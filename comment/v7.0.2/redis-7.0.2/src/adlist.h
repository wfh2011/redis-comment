/* adlist.h - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

/* 链表节点
 * 包括：
 * 1. 节点在链表的位置
 * 2. 节点所持有的数据，数据一般为malloc后的返回值(指针)
 */
typedef struct listNode {
    struct listNode *prev;  /* 后继 */
    struct listNode *next;  /* 前驱 */
    void *value;            /* 链表节点数据 */
} listNode;

/* 链表迭代器
 * 包括:
 * 1. 下一个迭代的节点
 * 2. 迭代方向: 从前到后(AL_START_HEAD)、从后到前(AL_START_TAIL)
 */
typedef struct listIter {
    listNode *next;         /* 下一个节点 */
    int direction;          /* 方向 */
} listIter;

/* 链表的元数据
 */
typedef struct list {
    listNode *head;                     /* 链表第一个节点指针 */
    listNode *tail;                     /* 链表最后一个节点指针 */
    void *(*dup)(void *ptr);            /* 复制listNode的value的回调函数 */
    void (*free)(void *ptr);            /* 释放listNode的value的回调函数 */
    int (*match)(void *ptr, void *key); /* 匹配listNode的value的回调函数 */
    unsigned long len;                  /* 链表元素个数 */
} list;

/* Functions implemented as macros */
#define listLength(l) ((l)->len)    /* 返回list的元素个数(unsigned long) */
#define listFirst(l) ((l)->head)    /* 返回list的第一个节点(listNode*) */
#define listLast(l) ((l)->tail)     /* 返回list的最后一个节点(listNode*) */
#define listPrevNode(n) ((n)->prev) /* 返回当前节点(listNode*)前一个节点(listNode*) */
#define listNextNode(n) ((n)->next) /* 返回当前节点(listNode*)下一个节点(listNode*) */
#define listNodeValue(n) ((n)->value) /* 返回当前节点(listNode*)值(void *)，使用的时候需要类型转换 */

#define listSetDupMethod(l,m) ((l)->dup = (m))      /* 设置链表节点value的复制回调函数 */
#define listSetFreeMethod(l,m) ((l)->free = (m))    /* 设置链表节点value的释放回调函数 */
#define listSetMatchMethod(l,m) ((l)->match = (m))  /* 设置链表节点value的匹配回调函数 */

#define listGetDupMethod(l) ((l)->dup)          /* 获取链表节点value的复制回调函数 */
#define listGetFreeMethod(l) ((l)->free)        /* 获取链表节点value的释放回调函数 */
#define listGetMatchMethod(l) ((l)->match)      /* 获取链表节点value的匹配回调函数 */

/* Prototypes */
list *listCreate(void);
void listRelease(list *list);
void listEmpty(list *list);
list *listAddNodeHead(list *list, void *value);
list *listAddNodeTail(list *list, void *value);
list *listInsertNode(list *list, listNode *old_node, void *value, int after);
void listDelNode(list *list, listNode *node);
listIter *listGetIterator(list *list, int direction);
listNode *listNext(listIter *iter);
void listReleaseIterator(listIter *iter);
list *listDup(list *orig);
listNode *listSearchKey(list *list, void *key);
listNode *listIndex(list *list, long index);
void listRewind(list *list, listIter *li);
void listRewindTail(list *list, listIter *li);
void listRotateTailToHead(list *list);
void listRotateHeadToTail(list *list);
void listJoin(list *l, list *o);

/* Directions for iterators */
#define AL_START_HEAD 0     /* 迭代器方向: 从头到尾 */
#define AL_START_TAIL 1     /* 迭代器方向: 从尾到头 */

#endif /* __ADLIST_H__ */
