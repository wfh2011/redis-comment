
# `Review`的声明
为了更清晰的阐述redis的相关业务逻辑，相关`api`的注解基于以下平台，若无明确的注释，则是`MacOs x86_64`:
- `MacOS`
```bash
# Darwin x86_64
➜  ws uname -a
Darwin wufeihu.local 21.6.0 Darwin Kernel Version 21.6.0: Wed Aug 10 14:25:27 PDT 2022; root:xnu-8020.141.5~2/RELEASE_X86_64 x86_64
➜  ws
```
- `Linux`

# 平台相关参数输出
## `MacOs`
```bash
# Darwin x86_64
➜  ws uname -a
Darwin wufeihu.local 21.6.0 Darwin Kernel Version 21.6.0: Wed Aug 10 14:25:27 PDT 2022; root:xnu-8020.141.5~2/RELEASE_X86_64 x86_64
➜  ws
```

```c++
#include <stdio.h>

static void platform() {
#ifdef __APPLE__
    printf("platform: apple \n");
#elif __linux__
    printf("platform: linux \n");
#elif __FreeBSD__
    printf("platform: free bsd \n");
#elif __OpenBSD__
    printf("platform: open bsd \n");
#elif __NetBSD__
    printf("platform: net bsd \n");
#else
    printf("platform: unknown \n");
#endif
}

static void archBits() {
    printf("arch bits: %d \n", sizeof(long) == 8 ? 64 : 32);
}

static void typeLen() {
    printf("type: unsigned short, len: %lu \n", sizeof(unsigned short));
    printf("type: short, len: %lu \n", sizeof(short));

    printf("type: int, len: %lu \n", sizeof(int));
    printf("type: unsigned int, len: %lu \n", sizeof(unsigned int));

    printf("type: float, len: %lu \n", sizeof(float));
    printf("type: double, len: %lu \n", sizeof(double));
    printf("type: size_t, len: %lu \n", sizeof(size_t));
    printf("type: long, len: %lu \n", sizeof(long));
    printf("type: long long, len: %lu \n", sizeof(long long));
}

static void basic() {
    archBits();
    platform();
    typeLen();
}

int main() {
    basic();

    return 0;
}

```

```shell
/Users/feihu/ws/c-proj-sample/cmake-build-debug/c_proj_sample
arch bits: 64 
platform: apple 
type: unsigned short, len: 2 
type: short, len: 2 
type: int, len: 4 
type: unsigned int, len: 4 
type: float, len: 4 
type: double, len: 8 
type: size_t, len: 8 
type: long, len: 8 
type: long long, len: 8 

Process finished with exit code 0
```

## `Linux`