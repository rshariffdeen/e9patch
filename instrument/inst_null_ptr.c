#include "instrument.h"

/************************************************************************/
/* TEMPLATES                                                            */
/************************************************************************/

// SF:ZF:0:AF:0:PF:1:CF:0:0:0:0:0:0:0:OF
#define ZF  0x0040
#define OF  0x0001
#define CF  0x0100

static bool option_debug   = true;
static bool option_disable = true;

/*
 * Safe division
 */
static void safe_ptr(void* s, void* d, uint16_t *rflags, const char *asm_str, const void *addr)
{

    bool isnull = (s == NULL || d == NULL);
    if (option_debug && isnull)
    {
        struct stream_s stream_0 = {0};
        stream_t stream = &stream_0;

        write_string(stream, "\33[31mDETECT NULL PTR\33[0m: \33[32m");
        write_string(stream, asm_str);
        write_string(stream, "\33[0m @ ");
        write_hex(stream, (intptr_t)addr);

        write_string(stream, " (");
        write_string(stream, " ");
        write_hex(stream, (intptr_t)s);
        write_string(stream, " - ");
        write_hex(stream, (intptr_t)d);
        write_string(stream, " )\33[0m\n");
        flush(stream);
    }

}


/*
 * Safe pointer reference (32bit)
 */
void ptr_r32(const int32_t *S, const int32_t *D,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
   safe_ptr((void*)S,(void*)D, rflags, asm_str, addr);
}

/*
 * Safe pointer reference (64bit)
 */
void ptr_r64(const int64_t *S,const int64_t *D,
             uint16_t *rflags, const char *asm_str, const void *addr)
{
    safe_ptr((void*)S,(void*)D, rflags, asm_str, addr);
}


/*
 * Init.
 */
void init(int argc, char **argv, char **envp)
{
    for (; envp && *envp != NULL; envp++)
    {
        char *var = *envp;
        if (var[0] == 'D' &&
            var[1] == 'E' &&
            var[2] == 'B' &&
            var[3] == 'U' &&
            var[4] == 'G' &&
            var[5] == '=' &&
            var[6] != '0')
        {
            option_debug = true;
        }
        if (var[0] == 'D' &&
            var[1] == 'I' &&
            var[2] == 'S' &&
            var[3] == 'A' &&
            var[4] == 'B' &&
            var[5] == 'L' &&
            var[6] == 'E' &&
            var[7] == '=' &&
            var[8] != '0')
        {
            option_disable = true;
        }
    }
}

