#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <syscall.h>

/************************************************************************/
/* I/O for debugging                                                    */
/************************************************************************/

#define STRING(x)   STRING_2(x)
#define STRING_2(x) #x

/*
 * System call implementation.
 */
asm
(
".globl syscall\n"
"syscall:\n"

// Disallow syscalls that MUST execute in the original context:
"cmp $" STRING(SYS_rt_sigreturn) ",%eax\n"
"je .Lno_sys\n"
"cmp $" STRING(SYS_clone) ",%eax\n"
"je .Lno_sys\n"

// Convert SYSV -> SYSCALL ABI:
"mov %edi,%eax\n"
"mov %rsi,%rdi\n"
"mov %rdx,%rsi\n"
"mov %rcx,%rdx\n"
"mov %r8,%r10\n"
"mov %r9,%r8\n"
"mov 0x8(%rsp),%r9\n"

// Execute the system call:
"syscall\n"

"retq\n"

// Handle errors:
".Lno_sys:\n"
"mov $-" STRING(ENOSYS) ",%eax\n"
"retq\n"
);

struct stream_s
{
    size_t pos;
    char buf[4096];
};
typedef struct stream_s *stream_t;

static void write_char(struct stream_s *stream, char c)
{
    if (stream->pos >= sizeof(stream->buf))
        return;
    stream->buf[stream->pos++] = c;
}

static void write_string(struct stream_s *stream, const char *s)
{
    for (; *s != '\0'; s++)
        write_char(stream, *s);
}

static void write_int(struct stream_s *stream, intptr_t x)
{
    if (x < 0)
    {
        write_char(stream, '-');
        x = -x;
    }
    if (x == 0)
    {
        write_char(stream, '0');
        return;
    }
    uintptr_t y = (uintptr_t)x;
    uintptr_t r = 10000000000000000000ull;
    bool seen = false;
    while (r != 0)
    {
        char c = '0' + y / r;
        y %= r;
        r /= 10;
        if (!seen && c == '0')
            continue;
        seen = true;
        write_char(stream, c);
    }
}

static const char xdigs[] = "0123456789abcdef";
static void write_hex(struct stream_s *stream, uintptr_t x)
{
    write_string(stream, "0x");
    if (x == 0)
    {
        write_char(stream, '0');
        return;
    }
    int shift = (15 * 4);
    bool seen = false;
    while (shift >= 0)
    {
        char c = xdigs[(x >> shift) & 0xF];
        shift -= 4;
        if (!seen && c == '0')
            continue;
        seen = true;
        write_char(stream, c);
    }
}

static void flush(struct stream_s *stream)
{
    syscall(SYS_write, STDOUT_FILENO, stream->buf, stream->pos);
    stream->pos = 0;
}