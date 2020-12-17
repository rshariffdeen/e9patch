#include "stdlib.c"

/************************************************************************/
/* TEMPLATES                                                            */
/************************************************************************/

// SF:ZF:0:AF:0:PF:1:CF:0:0:0:0:0:0:0:OF
#define ZF  0x0040
#define OF  0x0001
#define CF  0x0100

#define RED     "\33[31m"
#define GREEN   "\33[32m"
#define YELLOW  "\33[33m"
#define WHITE   "\33[0m"

static bool option_debug   = true;
static bool option_disable = true;

/*
 * Safe division
 */
static void safe_div(int64_t s1, uint16_t *rflags, const char *asm_str, const void *addr)
{
    __int128 d = (__int128)s1;
    bool iszero = (d == 0);

    if (option_debug && iszero)
    {
        fprintf(stderr, RED "DETECT DIV ZERO" WHITE ": %s @ %.16lx (%.16lx)\n", asm_str, addr);
        fflush_unlocked(stderr);
    }

}


/*
 * Safe division (32bit)
 */
void div_r32(const int32_t *S1,
                uint16_t *rflags, const char *asm_str, const void *addr)
{
   safe_div((int64_t)*S1, rflags, asm_str, addr);
}

/*
 * Safe division (64bit)
 */
void div_r64(const int64_t *S1,
             uint16_t *rflags, const char *asm_str, const void *addr)
{
    safe_div(*S1, rflags, asm_str, addr);
}


/*
 * Init.
 */
void init(int argc, char **argv, char **envp)
{
    setvbuf(stderr, NULL, _IOFBF, 0);
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

