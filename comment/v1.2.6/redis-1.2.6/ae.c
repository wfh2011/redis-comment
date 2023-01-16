/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * Copyright (c) 2006-2009, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "ae.h"
#include "zmalloc.h"
#include "config.h"

/*
 * 用epoll、select、kqueue取决于当前编译的平台
 * - linux => epoll
 * - 定义了MAC_OS_X_VERSION_10_6宏的Darwin系统 => kqueue
 * - 其他版本的Darwin系统 => select
 * - 以上条件均不符合 => select
 * */
/* Include the best multiplexing layer supported by this system.
 * The following should be ordered by performances, descending. */
#ifdef HAVE_EPOLL
#include "ae_epoll.c"
#else
    #ifdef HAVE_KQUEUE
    #include "ae_kqueue.c"
    #else
    #include "ae_select.c"
    #endif
#endif

aeEventLoop *aeCreateEventLoop(void) {
    aeEventLoop *eventLoop;
    int i;

    eventLoop = zmalloc(sizeof(*eventLoop));
    if (!eventLoop) return NULL;
    eventLoop->timeEventHead = NULL;
    eventLoop->timeEventNextId = 0;
    eventLoop->stop = 0;
    eventLoop->maxfd = -1;
    if (aeApiCreate(eventLoop) == -1) {
        zfree(eventLoop);
        return NULL;
    }
    /* Events with mask == AE_NONE are not set. So let's initialize the
     * vector with it. */
    for (i = 0; i < AE_SETSIZE; i++)
        eventLoop->events[i].mask = AE_NONE;
    return eventLoop;
}

void aeDeleteEventLoop(aeEventLoop *eventLoop) {
    aeApiFree(eventLoop);
    zfree(eventLoop);
}

void aeStop(aeEventLoop *eventLoop) {
    eventLoop->stop = 1;
}

/* 创建文件事件
 *
 * 文件事件:
 * - 使用multiplexing来同时监听多个套接字，并根据套接字的目前执行的任务来为套接字关联不同的事件处理器(callback)
 * - 当被监听的套接字准备好accept/read/write/close等操作时，与操作相对应的文件事件就会产生，执行callback
 *
 * */
int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask,
        aeFileProc *proc, void *clientData)
{
    if (fd >= AE_SETSIZE) return AE_ERR;
    aeFileEvent *fe = &eventLoop->events[fd];

    if (aeApiAddEvent(eventLoop, fd, mask) == -1)
        return AE_ERR;
    fe->mask |= mask;
    if (mask & AE_READABLE) fe->rfileProc = proc;
    if (mask & AE_WRITABLE) fe->wfileProc = proc;
    if (mask & AE_EXCEPTION) fe->efileProc = proc;
    fe->clientData = clientData;
    if (fd > eventLoop->maxfd)
        eventLoop->maxfd = fd;
    return AE_OK;
}


/* 删除文件事件
 * */
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask)
{
    if (fd >= AE_SETSIZE) return;
    aeFileEvent *fe = &eventLoop->events[fd];

    if (fe->mask == AE_NONE) return;
    fe->mask = fe->mask & (~mask);
    if (fd == eventLoop->maxfd && fe->mask == AE_NONE) {
        /* Update the max fd */
        int j;

        for (j = eventLoop->maxfd-1; j >= 0; j--)
            if (eventLoop->events[j].mask != AE_NONE) break;
        eventLoop->maxfd = j;
    }
    aeApiDelEvent(eventLoop, fd, mask);
}

/* 获取当前的时间，注意:
 * - seconds指的是秒，milliseconds指的是毫秒
 * - milliseconds<1000
 * - milliseconds表示当前时间的精度(在精度不高的场景，忽略此值也可以)
 * */
static void aeGetTime(long *seconds, long *milliseconds)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *milliseconds = tv.tv_usec/1000;
}

/*
 * 将当前时间加上milliseconds后的时间(毫秒)
 * 注意:
 * - ms的值<1000
 * - ms表示当前时间的精度
 * */
static void aeAddMillisecondsToNow(long long milliseconds, long *sec, long *ms) {
    long cur_sec, cur_ms, when_sec, when_ms;

    aeGetTime(&cur_sec, &cur_ms);
    when_sec = cur_sec + milliseconds/1000;
    when_ms = cur_ms + milliseconds%1000;
    if (when_ms >= 1000) {
        when_sec ++;
        when_ms -= 1000;
    }
    *sec = when_sec;
    *ms = when_ms;
}

/* 创建一个时间事件(链表节点)，并将其链表节点放到事件循环的链表头部
 *
 * 具体的链表节点包括以下数据:
 * - id, 每创建一个时间事件，id都会自增
 * - 期望此时间事件执行的时间(当前时间 + 传入的多少毫秒)
 * - 回调函数
 * - 回调函数处理的参数
 *
 * 时间事件分类:
 * - 定时事件: 程序在指定的时间之后执行一次
 * - 周期事件: 程序每隔指定时间就执行一次
 *
 * 时间事件链表:
 * - 事件按照id，从大到小排列，因为新的事件(链表节点, id自增)总是插入到链表头部
 * - 时间事件的期待执行的时间是无序，即when字段是无序的
 * - 虽然是遍历全部链表，但是性能不会低，因为整个链表的元素很少，当前redis版本只有一个，即serverCron
 * */
long long aeCreateTimeEvent(aeEventLoop *eventLoop, long long milliseconds,
        aeTimeProc *proc, void *clientData,
        aeEventFinalizerProc *finalizerProc)
{
    long long id = eventLoop->timeEventNextId++;
    aeTimeEvent *te;

    te = zmalloc(sizeof(*te));
    if (te == NULL) return AE_ERR;
    te->id = id;
    aeAddMillisecondsToNow(milliseconds,&te->when_sec,&te->when_ms);
    te->timeProc = proc;
    te->finalizerProc = finalizerProc;
    te->clientData = clientData;
    te->next = eventLoop->timeEventHead;
    eventLoop->timeEventHead = te;
    return id;
}

/* 删除某个时间事件
 *
 * 流程:
 * - 通过id对时间事件链表进行遍历，id匹配上的即为要删除的事件
 * - 将找到的链表节点从链表中剔除
 * - 如果找到的链表节点的finalizerProc回调函数赋值，那么需要执行
 * - 释放待删除的链表节点内存
 *
 * 返回值：
 * - 没有找到id所对应的时间事件，返回AE_ERR
 * */
int aeDeleteTimeEvent(aeEventLoop *eventLoop, long long id)
{
    aeTimeEvent *te, *prev = NULL;

    te = eventLoop->timeEventHead;
    while(te) {
        if (te->id == id) {
            if (prev == NULL)
                eventLoop->timeEventHead = te->next;
            else
                prev->next = te->next;
            if (te->finalizerProc)
                te->finalizerProc(eventLoop, te->clientData);
            zfree(te);
            return AE_OK;
        }
        prev = te;
        te = te->next;
    }
    return AE_ERR; /* NO event with the specified ID found */
}

/*
 * 对时间事件链表进行遍历，找到链表节点的UTC时间值最小的节点
 *
 * 原理:
 * - 链表节点的时间由秒+毫秒(毫秒 < 1000，毫秒可以理解成时间精度)组成
 * - 先比较秒部分，较小值优先
 * - 秒值相同，则比较毫秒值，毫秒值较小优先
 *
 * 返回值:
 * - 时间事件链表无元素，则返回NULL
 * */
/* Search the first timer to fire.
 * This operation is useful to know how many time the select can be
 * put in sleep without to delay any event.
 * If there are no timers NULL is returned.
 *
 * Note that's O(N) since time events are unsorted.
 * Possible optimizations (not needed by Redis so far, but...):
 * 1) Insert the event in order, so that the nearest is just the head.
 *    Much better but still insertion or deletion of timers is O(N).
 * 2) Use a skiplist to have this operation as O(1) and insertion as O(log(N)).
 */
static aeTimeEvent *aeSearchNearestTimer(aeEventLoop *eventLoop)
{
    aeTimeEvent *te = eventLoop->timeEventHead;
    aeTimeEvent *nearest = NULL;

    while(te) {
        if (!nearest || te->when_sec < nearest->when_sec ||
                (te->when_sec == nearest->when_sec &&
                 te->when_ms < nearest->when_ms))
            nearest = te;
        te = te->next;
    }
    return nearest;
}

/*
 * 处理时间事件
 * - 对时间事件链表进行遍历
 * - 如果链表节点记录的期望处理时间小于时间(表示时间到了，但是未处理)
 * - 执行符合条件的任务timeProc,并对返回值retval进行判断
 *  - retval == -1: 表明此任务处理完毕，从时间事件链表头重新遍历
 *  - retval >= 0: 表明再过retval后，任务再次执行
 * -
 * */
/* Process time events */
static int processTimeEvents(aeEventLoop *eventLoop) {
    int processed = 0;
    aeTimeEvent *te;
    long long maxId;

    te = eventLoop->timeEventHead;
    maxId = eventLoop->timeEventNextId-1;
    while(te) {
        long now_sec, now_ms;
        long long id;

        if (te->id > maxId) {
            te = te->next;
            continue;
        }
        aeGetTime(&now_sec, &now_ms);
        if (now_sec > te->when_sec ||
            (now_sec == te->when_sec && now_ms >= te->when_ms))
        {
            int retval;

            id = te->id;
            retval = te->timeProc(eventLoop, id, te->clientData);
            processed++;
            /* After an event is processed our time event list may
             * no longer be the same, so we restart from head.
             * Still we make sure to don't process events registered
             * by event handlers itself in order to don't loop forever.
             * To do so we saved the max ID we want to handle.
             *
             * FUTURE OPTIMIZATIONS:
             * Note that this is NOT great algorithmically. Redis uses
             * a single time event so it's not a problem but the right
             * way to do this is to add the new elements on head, and
             * to flag deleted elements in a special way for later
             * deletion (putting references to the nodes to delete into
             * another linked list). */
            if (retval != AE_NOMORE) {
                aeAddMillisecondsToNow(retval,&te->when_sec,&te->when_ms);
            } else {
                aeDeleteTimeEvent(eventLoop, id);
            }
            te = eventLoop->timeEventHead;
        } else {
            te = te->next;
        }
    }
    return processed;
}

/* 处理事件
 *
 * 核心思想:
 * - 通过aeCreateFileEvent函数将accept/write/read/close等事件与其相关回调进行映射
 * - 在aeProcessEvents中，accept/write/read/close等文件事件出发，执行注册的回调函数
 * - 文件事件优先级 > 时间事件优先级
 * - 消息处理/定时任务等被集成到主线程中(这是说redis单线程的原因)
 * - epoll/kevent里面的等待时间参数取决于最小的时间事件时间(when字段)与当前时间之差:
 *  - when - now < 0: epoll/kevent不等待，即等待时间为0
 *  - when - now > 0: epoll/kevent等待,等待时间为when - now
 *
 *  时间/文件事件与AE_DONT_WAIT关系:
 *  - AE_DONT_WAIT指的是epoll/kevent的时间参数为0(不等待，没有文件事件，直接进入下一步操作)的情况
 *  - 文件事件的epoll/kevent必有等待时间的参数
 *  - 文件事件 + 有时间事件(时间事件链表不为空) => 必然会有等待 => 文件事件+时间事件 => flag != AE_DONT_WAIT => flag == WAIT
 *  - 无时间事件(时间事件链表为空) + WAIT => 只有文件事件 + WAIT => 一直等待
 *  - 无时间事件(时间事件链表为空) + DONT_WAIT => 只有文件事件 + DONT_WAIT => 文件事件等待时间为0
 *
 * 注意:
 * - aeProcessEvents只有一个地方调用，且flags为AE_ALL_EVENTS(AE_FILE_EVENTS|AE_TIME_EVENTS)
 * - flags必须包括AE_FILE_EVENTS或者AE_TIME_EVENTS其一
 * */
/* Process every pending time event, then every pending file event
 * (that may be registered by time event callbacks just processed).
 * Without special flags the function sleeps until some file event
 * fires, or when the next time event occurrs (if any).
 *
 * If flags is 0, the function does nothing and returns.
 * if flags has AE_ALL_EVENTS set, all the kind of events are processed.
 * if flags has AE_FILE_EVENTS set, file events are processed.
 * if flags has AE_TIME_EVENTS set, time events are processed.
 * if flags has AE_DONT_WAIT set the function returns ASAP until all
 * the events that's possible to process without to wait are processed.
 *
 * The function returns the number of events processed. */
int aeProcessEvents(aeEventLoop *eventLoop, int flags)
{
    int processed = 0, numevents;

    /* Nothing to do? return ASAP */
    if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) return 0;

    /* Note that we want call select() even if there are no
     * file events to process as long as we want to process time
     * events, in order to sleep until the next time event is ready
     * to fire. */
    if (eventLoop->maxfd != -1 ||
        ((flags & AE_TIME_EVENTS) && !(flags & AE_DONT_WAIT))) {
        int j;
        aeTimeEvent *shortest = NULL;
        struct timeval tv, *tvp;

        if (flags & AE_TIME_EVENTS && !(flags & AE_DONT_WAIT))
            shortest = aeSearchNearestTimer(eventLoop);
        if (shortest) {
            long now_sec, now_ms;

            /* Calculate the time missing for the nearest
             * timer to fire. */
            aeGetTime(&now_sec, &now_ms);
            tvp = &tv;
            tvp->tv_sec = shortest->when_sec - now_sec;
            if (shortest->when_ms < now_ms) {
                tvp->tv_usec = ((shortest->when_ms+1000) - now_ms)*1000;
                tvp->tv_sec --;
            } else {
                tvp->tv_usec = (shortest->when_ms - now_ms)*1000;
            }
            if (tvp->tv_sec < 0) tvp->tv_sec = 0;
            if (tvp->tv_usec < 0) tvp->tv_usec = 0;
        } else {
            /* If we have to check for events but need to return
             * ASAP because of AE_DONT_WAIT we need to se the timeout
             * to zero */
            if (flags & AE_DONT_WAIT) {
                tv.tv_sec = tv.tv_usec = 0;
                tvp = &tv;
            } else {
                /* Otherwise we can block */
                tvp = NULL; /* wait forever */
            }
        }

        numevents = aeApiPoll(eventLoop, tvp);
        for (j = 0; j < numevents; j++) {
            aeFileEvent *fe = &eventLoop->events[eventLoop->fired[j].fd];
            int mask = eventLoop->fired[j].mask;
            int fd = eventLoop->fired[j].fd;

	    /* note the fe->mask & mask & ... code: maybe an already processed
             * event removed an element that fired and we still didn't
             * processed, so we check if the event is still valid. */
            if (fe->mask & mask & AE_READABLE)
                fe->rfileProc(eventLoop,fd,fe->clientData,mask);
            if (fe->mask & mask & AE_WRITABLE && fe->wfileProc != fe->rfileProc)
                fe->wfileProc(eventLoop,fd,fe->clientData,mask);
            if (fe->mask & mask & AE_EXCEPTION &&
				       fe->efileProc != fe->wfileProc &&
                                       fe->efileProc != fe->rfileProc)
                fe->efileProc(eventLoop,fd,fe->clientData,mask);
            processed++;
        }
    }
    /* Check time events */
    if (flags & AE_TIME_EVENTS)
        processed += processTimeEvents(eventLoop);

    return processed; /* return the number of processed file/time events */
}

/* Wait for millseconds until the given file descriptor becomes
 * writable/readable/exception */
int aeWait(int fd, int mask, long long milliseconds) {
    struct timeval tv;
    fd_set rfds, wfds, efds;
    int retmask = 0, retval;

    tv.tv_sec = milliseconds/1000;
    tv.tv_usec = (milliseconds%1000)*1000;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_ZERO(&efds);

    if (mask & AE_READABLE) FD_SET(fd,&rfds);
    if (mask & AE_WRITABLE) FD_SET(fd,&wfds);
    if (mask & AE_EXCEPTION) FD_SET(fd,&efds);
    if ((retval = select(fd+1, &rfds, &wfds, &efds, &tv)) > 0) {
        if (FD_ISSET(fd,&rfds)) retmask |= AE_READABLE;
        if (FD_ISSET(fd,&wfds)) retmask |= AE_WRITABLE;
        if (FD_ISSET(fd,&efds)) retmask |= AE_EXCEPTION;
        return retmask;
    } else {
        return retval;
    }
}

/* 事件循环主流程
 * */
void aeMain(aeEventLoop *eventLoop) {
    eventLoop->stop = 0;
    while (!eventLoop->stop)
        aeProcessEvents(eventLoop, AE_ALL_EVENTS);
}

/* 返回使用的事件api，以下的一种:
 * - epoll
 * - select
 * - kqueue
 * */
char *aeGetApiName(void) {
    return aeApiName();
}

/* Questions:
 * - processTimeEvents中对maxId判断的理由?
 * - processTimeEvents遍历到符合条件的时间事件，从链表头遍历的原因？
 *
 * */