#include "stdlib.c"

/************************************************************************/
/* TEMPLATES                                                            */
/************************************************************************/

// SF:ZF:0:AF:0:PF:1:CF:0:0:0:0:0:0:0:OF
#define OF  0x0001
#define CF  0x0100
#define RED     "\33[31m"
#define GREEN   "\33[32m"
#define YELLOW  "\33[33m"
#define WHITE   "\33[0m"

static bool option_debug   = true;
static bool option_disable = true;

/*
 * Safe multiplication.
 */
static void safe_mul(int64_t s1, int64_t s2, uint16_t *rflags, int64_t lb,
                        int64_t ub, const char *asm_str, const void *addr)
{
    __int128 d = (__int128)s1 * (__int128)s2;
    __int128 c = d;
    bool overflow = (d < lb || d > ub);

    if (option_debug && overflow)
    {
        fprintf(stderr, RED "DETECT MUL OVERFLOW" WHITE ": %s @ 0x%.16lx (%d * %d = %d)\n", asm_str, addr, s1, s2, c);
        fflush_unlocked(stderr);
    }

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
    safe_mul(*S1, *S2, rflags, INT64_MIN, INT64_MAX, asm_str, addr);
}

/*
 * Safe multiplication (64bit three operand form)
 */
void mul_imm32r64r64(const int32_t *S1, const int64_t *S2, int64_t *D,
                     uint16_t *rflags, const char *asm_str, const void *addr)
{
    safe_mul((int64_t)*S1, *S2, rflags, INT64_MIN, INT64_MAX, asm_str,
                  addr);
}

/*
 * Safe multiplication (32bit two operand form)
 */
void mul_r32r32(const int32_t *S1, const int32_t *S2, int32_t *D,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
    safe_mul((int64_t)*S1, (int64_t)*S2, rflags, INT32_MIN,
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
    setvbuf(stderr, NULL, _IOFBF, 0);
    for (; envp && *envp != NULL; envp++)
    {
        char *var = *envp;
    }
}

