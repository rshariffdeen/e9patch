#include "instrument.h"

/************************************************************************/
/* TEMPLATES                                                            */
/************************************************************************/

// SF:ZF:0:AF:0:PF:1:CF:0:0:0:0:0:0:0:OF
#define OF  0x0001
#define CF  0x0100

static bool option_debug   = true;
static bool option_disable = true;

/*
 * Safe addition.
 */
static void safe_add(int64_t s1, int64_t s2, uint16_t *rflags, int64_t lb,
                        int64_t ub, const char *asm_str, const void *addr)
{
    __int128 d = (__int128)s1 + (__int128)s2;
    __int128 c = d;
    bool overflow = (d < lb || d > ub);

    if (option_debug && overflow)
    {
        struct stream_s stream_0 = {0};
        stream_t stream = &stream_0;

        write_string(stream, "\33[31mDETECT ADD OVERFLOW\33[0m: \33[32m");
        write_string(stream, asm_str);
        write_string(stream, "\33[0m @ ");
        write_hex(stream, (intptr_t)addr);
        write_string(stream, " (\33[33m");
        write_int(stream, s1);
        write_string(stream, " + ");
        write_int(stream, s2);
        write_string(stream, " = ");
        write_int(stream, (int64_t)c);
        write_string(stream, "\33[0m)\n");
        flush(stream);
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
 * Safe addition (64bit two operand form)
 */
void add_64(const int64_t *S1, const int64_t *S2, int64_t *D,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
    safe_add(*S1, *S2, rflags, INT64_MIN, INT64_MAX, asm_str, addr);
}


/*
 * Safe addition (32bit two operand form)
 */
void add_32(const int32_t *S1, const int32_t *S2, int32_t *D,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
    safe_add((int64_t)*S1, (int64_t)*S2, rflags, INT32_MIN,
                           INT32_MAX, asm_str, addr);
}


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

