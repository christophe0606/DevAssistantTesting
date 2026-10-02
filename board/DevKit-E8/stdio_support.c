/* The LCD demo has no console transport. SDK diagnostics must not stop it. */
#include "RTE_Components.h"
#include CMSIS_device_header
#include "retarget_stdin.h"
#include "retarget_stdout.h"
#include "retarget_stderr.h"
#include "retarget_tty.h"

int stdin_getchar(void) { return -1; }
int stdout_putchar(int ch) { return ch; }
int stderr_putchar(int ch) { return ch; }
void ttywrch(int ch) { (void)ch; }

#if defined(__ARMCC_VERSION)
/* Turn accidental use of a semihosted library routine into a link error. */
__asm(".global __use_no_semihosting");
__asm(".global __ARM_use_no_argv");

char *_sys_command_string(char *buffer, int length)
{
    (void)buffer;
    (void)length;
    return 0;
}

__attribute__((noreturn)) void _sys_exit(int status)
{
    (void)status;
    for (;;) __WFI();
}
#endif
