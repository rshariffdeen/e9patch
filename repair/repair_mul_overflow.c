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

/************************************************************************/
/* TEMPLATES                                                            */
/************************************************************************/

// SF:ZF:0:AF:0:PF:1:CF:0:0:0:0:0:0:0:OF
#define OF  0x0001
#define CF  0x0100

/*
 * Safe multiplication.
 */
static int64_t safe_mul(int64_t s1, int64_t s2, uint16_t *rflags, int64_t lb,
                        int64_t ub, const char *asm_str, const void *addr)
{
    __int128 d = (__int128)s1 * (__int128)s2;
    __int128 c = d;
    bool overflow = (d < lb || d > ub);

    if (c > ub)
        c = ub;
    else if (c < lb)
        c = lb;
    d = c;
    overflow = (d < lb || d > ub);
    if (overflow)
        *rflags |= (CF | OF);
    else
        *rflags &= ~(CF | OF);

    return (int64_t)d;
}

/*

COMMAND:

./e9compile.sh template.c
./e9tool \
     -M 'mnemonic=imulq' -M 'op.length=2' \
     -A 'call[replace] mul_r64r64(src[0],src[1],dst[0],&rflags,asmStr,staticAddr)@template' \
     -M 'mnemonic=imulq' -M 'op.length=3' \
     -A 'call[replace] mul_imm32r64r64(src[0],src[1],dst[0],&rflags,asmStr,staticAddr)@template' \
     -M 'mnemonic=imull' -M 'op.length=2' \
     -A 'call[replace] mul_r32r32(src[0],src[1],dst[0],&rflags,asmStr,staticAddr)@template' \
     -M 'mnemonic=imull' -M 'op.length=3' \
     -A 'call[replace] mul_imm32r32r32(src[0],src[1],dst[0],&rflags,asmStr,staticAddr)@template' \
     `which xterm`
DEBUG=1 ./a.out

*/

/*
 * Safe multiplication (64bit two operand form)
 */
void mul_r64r64(const int64_t *S1, const int64_t *S2, int64_t *D,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
    *D = safe_mul(*S1, *S2, rflags, INT64_MIN, INT64_MAX, asm_str, addr);
}

/*
 * Safe multiplication (64bit three operand form)
 */
void mul_imm32r64r64(const int32_t *S1, const int64_t *S2, int64_t *D,
                     uint16_t *rflags, const char *asm_str, const void *addr)
{
    *D = safe_mul((int64_t)*S1, *S2, rflags, INT64_MIN, INT64_MAX, asm_str,
                  addr);
}

/*
 * Safe multiplication (32bit two operand form)
 */
void mul_r32r32(const int32_t *S1, const int32_t *S2, int32_t *D,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
    *D = (int32_t)safe_mul((int64_t)*S1, (int64_t)*S2, rflags, INT32_MIN,
                           INT32_MAX, asm_str, addr);
}

/*
 * Safe multiplication (32bit three operand form)
 */
void mul_imm32r32r32(const int32_t *S1, const int32_t *S2, int32_t *D,
                     uint16_t *rflags, const char *asmStr, const void *addr)
__attribute__((__alias__("mul_r32r32")));

/*
 * Init.
 */
void init(int argc, char **argv, char **envp)
{
    for (; envp && *envp != NULL; envp++)
    {
        char *var = *envp;
    }
}

