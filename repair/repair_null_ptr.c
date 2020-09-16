#include "repair.h"

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
    if (isnull)
    {
        asm volatile("mov $-1, %ebx\n"
                     "mov $1, %eax\n"
                     "int $0x80\n");
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

}

