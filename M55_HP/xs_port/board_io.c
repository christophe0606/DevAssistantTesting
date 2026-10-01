/* Standard C streams for standalone AC6 firmware; no debugger host I/O. */
#ifndef XS_HOST
#include <rt_sys.h>
#include <string.h>
#include "xs_internal.h"

#pragma import(__use_no_semihosting)

const char __stdin_name[] = ":STDIN";
const char __stdout_name[] = ":STDOUT";
const char __stderr_name[] = ":STDERR";

FILEHANDLE _sys_open(const char *name, int mode) {
  (void)mode;
  if (!name) return -1;
  if (!strcmp(name, __stdin_name)) return 1;
  if (!strcmp(name, __stdout_name)) return 2;
  if (!strcmp(name, __stderr_name)) return 3;
  return -1;
}
int _sys_close(FILEHANDLE handle) { (void)handle; return 0; }
int _sys_write(FILEHANDLE handle, const unsigned char *data, unsigned length, int mode) {
  (void)data; (void)length; (void)mode;
  return handle == 2 || handle == 3 ? 0 : -1;
}
int _sys_read(FILEHANDLE handle, unsigned char *data, unsigned length, int mode) {
  (void)handle; (void)data; (void)mode;
  return (int)length;
}
int _sys_istty(FILEHANDLE handle) { return handle >= 1 && handle <= 3; }
int _sys_seek(FILEHANDLE handle, long position) { (void)handle; (void)position; return -1; }
int _sys_ensure(FILEHANDLE handle) { (void)handle; return 0; }
long _sys_flen(FILEHANDLE handle) { (void)handle; return 0; }
int _sys_tmpnam2(char *name, int id, unsigned length) { (void)name; (void)id; (void)length; return -1; }
char *_sys_command_string(char *command, int length) { if (length > 0) command[0] = 0; return command; }
void _ttywrch(int character) { (void)character; }
void _sys_exit(int status) { jwxyz_abort("C library exit %d", status); }
void __aeabi_assert(const char *expression, const char *file, int line) {
  jwxyz_abort("assert %s at %s:%d", expression, file, line);
}
#endif
