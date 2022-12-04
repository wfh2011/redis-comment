`bytesToHuman`: 字节转换成可读形式
-------------------------------------------------------------------------------

```c

#include <stdio.h>

void bytesToHuman(char *s, long long n) {
    double d = 0.0;

    if (n < 0) {
        *s = '-';
        s++;
        n = -n;
    }
    if (n < 1024) {
        /* Bytes */
        sprintf(s, "%lluB", n);
        return;
    } else if (n < (1024 * 1024)) {
        d = (double) n / (1024);
        sprintf(s, "%.2fK", d);
    } else if (n < (1024LL * 1024 * 1024)) {
        d = (double) n / (1024 * 1024);
        sprintf(s, "%.2fM", d);
    } else if (n < (1024LL * 1024 * 1024 * 1024)) {
        d = (double) n / (1024LL * 1024 * 1024);
        sprintf(s, "%.2fG", d);
    }
}

int main() {
    char szHuman[64] = {0};
    bytesToHuman(szHuman, 10250);
    printf("%s", szHuman);
    return 0;
}
```


```golang
func BytesToHuman(n int64) string {
	d := float64(0)
	prefix := ""
	if n < 0 {
		prefix = "-"
		n = -n
	}
	ret := ""
	if n < 1024 {
		ret = fmt.Sprintf("%s%dB", prefix, n)
		return ret
	} else if n < (1024 * 1024) {
		d = float64(n) / 1024
		ret = fmt.Sprintf("%s%.2fK", prefix, d)
	} else if n < int64(1024*1024*1024) {
		d = float64(n) / (1024 * 1024)
		ret = fmt.Sprintf("%s%.2fM", prefix, d)
	} else if n < int64(1024*1024*1024*1024) {
		d = float64(n) / (1024 * 1024 * 1024)
		ret = fmt.Sprintf("%s%.2fG", prefix, d)
	} else if n < int64(1024*1024*1024*1024*1024) {
		d = float64(n) / (1024 * 1024 * 1024 * 1024)
		ret = fmt.Sprintf("%s%.2fT", prefix, d)
	} else if n < int64(1024*1024*1024*1024*1024*1024) {
		d = float64(n) / (1024 * 1024 * 1024 * 1024 * 1024)
		ret = fmt.Sprintf("%s%.2fP", prefix, d)
	} else {
		ret = "out of the range"
	}
	return ret
}

```