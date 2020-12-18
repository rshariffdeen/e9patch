#include "repair.h"
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

    if (iszero)
    {
        fprintf(stderr, GREEN "AVOIDED DIV ZERO" WHITE ": %s @ 0x%.16lx (%.16lx)\n", asm_str, addr, s1);
        fflush_unlocked(stderr);
        asm volatile("mov $0x1, %edi\n"
                     "callq $510\n");
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

}

