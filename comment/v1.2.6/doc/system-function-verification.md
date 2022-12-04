`malloc_size` 
-------------------------------------------------------------------------------
```c++
#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <malloc/malloc.h>
#endif

int main() {
    char *p = NULL;
    int tmp = 0;
    for (int i = 1; i < 128; i++) {
        p = (char *) malloc(i);
        if (NULL == p) {
            continue;
        }
#ifdef __APPLE__
        // 16B的倍数
        tmp = malloc_size(p);
        printf("real malloc, in: %lu, real: %lu \n", i, tmp);
#endif
    }
    return 0;
}
```
执行结果:
```shell
/Users/feihu/ws/c-proj-sample/cmake-build-debug/c_proj_sample
real malloc, in: 1, real: 16 
real malloc, in: 2, real: 16 
real malloc, in: 3, real: 16 
real malloc, in: 4, real: 16 
real malloc, in: 5, real: 16 
real malloc, in: 6, real: 16 
real malloc, in: 7, real: 16 
real malloc, in: 8, real: 16 
real malloc, in: 9, real: 16 
real malloc, in: 10, real: 16 
real malloc, in: 11, real: 16 
real malloc, in: 12, real: 16 
real malloc, in: 13, real: 16 
real malloc, in: 14, real: 16 
real malloc, in: 15, real: 16 
real malloc, in: 16, real: 16 
real malloc, in: 17, real: 32 
real malloc, in: 18, real: 32 
real malloc, in: 19, real: 32 
real malloc, in: 20, real: 32 
real malloc, in: 21, real: 32 
real malloc, in: 22, real: 32 
real malloc, in: 23, real: 32 
real malloc, in: 24, real: 32 
real malloc, in: 25, real: 32 
real malloc, in: 26, real: 32 
real malloc, in: 27, real: 32 
real malloc, in: 28, real: 32 
real malloc, in: 29, real: 32 
real malloc, in: 30, real: 32 
real malloc, in: 31, real: 32 
real malloc, in: 32, real: 32 
real malloc, in: 33, real: 48 
real malloc, in: 34, real: 48 
real malloc, in: 35, real: 48 
real malloc, in: 36, real: 48 
real malloc, in: 37, real: 48 
real malloc, in: 38, real: 48 
real malloc, in: 39, real: 48 
real malloc, in: 40, real: 48 
real malloc, in: 41, real: 48 
real malloc, in: 42, real: 48 
real malloc, in: 43, real: 48 
real malloc, in: 44, real: 48 
real malloc, in: 45, real: 48 
real malloc, in: 46, real: 48 
real malloc, in: 47, real: 48 
real malloc, in: 48, real: 48 
real malloc, in: 49, real: 64 
real malloc, in: 50, real: 64 
real malloc, in: 51, real: 64 
real malloc, in: 52, real: 64 
real malloc, in: 53, real: 64 
real malloc, in: 54, real: 64 
real malloc, in: 55, real: 64 
real malloc, in: 56, real: 64 
real malloc, in: 57, real: 64 
real malloc, in: 58, real: 64 
real malloc, in: 59, real: 64 
real malloc, in: 60, real: 64 
real malloc, in: 61, real: 64 
real malloc, in: 62, real: 64 
real malloc, in: 63, real: 64 
real malloc, in: 64, real: 64 
real malloc, in: 65, real: 80 
real malloc, in: 66, real: 80 
real malloc, in: 67, real: 80 
real malloc, in: 68, real: 80 
real malloc, in: 69, real: 80 
real malloc, in: 70, real: 80 
real malloc, in: 71, real: 80 
real malloc, in: 72, real: 80 
real malloc, in: 73, real: 80 
real malloc, in: 74, real: 80 
real malloc, in: 75, real: 80 
real malloc, in: 76, real: 80 
real malloc, in: 77, real: 80 
real malloc, in: 78, real: 80 
real malloc, in: 79, real: 80 
real malloc, in: 80, real: 80 
real malloc, in: 81, real: 96 
real malloc, in: 82, real: 96 
real malloc, in: 83, real: 96 
real malloc, in: 84, real: 96 
real malloc, in: 85, real: 96 
real malloc, in: 86, real: 96 
real malloc, in: 87, real: 96 
real malloc, in: 88, real: 96 
real malloc, in: 89, real: 96 
real malloc, in: 90, real: 96 
real malloc, in: 91, real: 96 
real malloc, in: 92, real: 96 
real malloc, in: 93, real: 96 
real malloc, in: 94, real: 96 
real malloc, in: 95, real: 96 
real malloc, in: 96, real: 96 
real malloc, in: 97, real: 112 
real malloc, in: 98, real: 112 
real malloc, in: 99, real: 112 
real malloc, in: 100, real: 112 
real malloc, in: 101, real: 112 
real malloc, in: 102, real: 112 
real malloc, in: 103, real: 112 
real malloc, in: 104, real: 112 
real malloc, in: 105, real: 112 
real malloc, in: 106, real: 112 
real malloc, in: 107, real: 112 
real malloc, in: 108, real: 112 
real malloc, in: 109, real: 112 
real malloc, in: 110, real: 112 
real malloc, in: 111, real: 112 
real malloc, in: 112, real: 112 
real malloc, in: 113, real: 128 
real malloc, in: 114, real: 128 
real malloc, in: 115, real: 128 
real malloc, in: 116, real: 128 
real malloc, in: 117, real: 128 
real malloc, in: 118, real: 128 
real malloc, in: 119, real: 128 
real malloc, in: 120, real: 128 
real malloc, in: 121, real: 128 
real malloc, in: 122, real: 128 
real malloc, in: 123, real: 128 
real malloc, in: 124, real: 128 
real malloc, in: 125, real: 128 
real malloc, in: 126, real: 128 
real malloc, in: 127, real: 128 

Process finished with exit code 0


```

`anetResolve`
-------------------------------------------------------------------------------
```cpp

#include <stdio.h>
#include <unistd.h>
#include "anet.h"
#include "zmalloc.h"
#include "sds.h"

static checkHost2Ip() {
    char hostName[] = "www.baidu.com";
    int lRet = -1;
    char *ip = zmalloc(32);
    lRet = anetResolve(NULL, hostName, ip);
    printf("ip: %s \n", ip);

    fprintf(stdout, "host: %s, ret: %d, ip: %s \n", hostName, lRet, ip);
    zfree(ip);
    return 0;
}

int main() {
    checkHost2Ip();
}
```
执行结果:
```c++
# 程序结果
/Users/feihu/ws/c-proj-sample/cmake-build-debug/c_proj_sample
ip: 112.80.248.75
host: www.baidu.com, ret: 0, ip: 112.80.248.75

Process finished with exit code 0

# ping命令结果
➜  c-proj-sample ping www.baidu.com
PING www.a.shifen.com (112.80.248.75): 56 data bytes
64 bytes from 112.80.248.75: icmp_seq=0 ttl=56 time=15.711 ms
64 bytes from 112.80.248.75: icmp_seq=1 ttl=56 time=18.836 ms
^C
--- www.a.shifen.com ping statistics ---
2 packets transmitted, 2 packets received, 0.0% packet loss
round-trip min/avg/max/stddev = 15.711/17.273/18.836/1.563 ms


```