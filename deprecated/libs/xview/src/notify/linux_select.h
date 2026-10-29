/* Header for linux versions of system calls which are
 * called through syscall() in XView. We don't use syscall,
 * but the unistd.h macros in the kernel sources. */

#if defined(__linux) && !defined(__LINUX_SELECT_H)
#define __LINUX_SELECT_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>

int linux_select(int width, fd_set *readfds, fd_set *writefds,
                 fd_set *exceptfds, struct timeval *timeout);
#ifndef __GLIBC__
int sys_fcntl(int fildes, int cmd, int arg);
int sys_read(int fildes, char * buf, off_t cnt);
#endif

#endif /* __LINUX_SELECT_H */
