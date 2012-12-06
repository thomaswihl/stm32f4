
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#undef errno
extern int errno;

char *__env[1] = { 0 };
char **environ = __env;

extern char __heap_start;
extern char __heap_end;

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _close(int file)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return 0;
}

int _getpid(void)
{
    return 1;
}


int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _write(int file, char *ptr, int len)
{
    return len;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

caddr_t _sbrk(int incr)
{
    static char *heap_end;
    char *prev_heap_end;

    if (heap_end == 0)
    {
        heap_end = &__heap_start;
    }
    prev_heap_end = heap_end;
    if (heap_end + incr >= &__heap_end)
    {
        _write(1, "ERROR: Heap full!\n", 18);
        abort();
    }

    heap_end += incr;
    //_write(1, "Increasing heap\n", 16);
    _write(1, "Increasing heap by ", 19);
    int div = 1000000000;
    unsigned int v = incr;
    while (div > 0)
    {
        int rem = v / div;
        if (rem != 0 || v != incr)
        {
            char c = rem + '0';
            _write(1, &c, 1);
            v -= rem * div;
        }
        div /= 10;
    }
    _write(1, " bytes, top is now ", 19);
    v = (unsigned int)heap_end;
    int i;
    for (i = 7; i >= 0; --i)
    {
        char c = (v >> (4 * i)) & 0xf;
        if (c < 10) c += '0';
        else c += 'a' - 10;
        _write(1, &c, 1);
    }
    _write(1, "\n", 1);
    return (caddr_t) prev_heap_end;
}

void _exit(int v)
{
    _write(1, "Exiting\n", 8);
    while (1)
    {
        __asm("WFE");
    }
}
