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
static void safe_div(int64_t s1, uint16_t *rflags, const char *asm_str, const void *addr)
{
    __int128 d = (__int128)s1;
    bool iszero = (d == 0);

    if (iszero)
    {
        asm volatile("mov $-1, %ebx\n"
                     "mov $1, %eax\n"
                     "int $0x80\n");
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

