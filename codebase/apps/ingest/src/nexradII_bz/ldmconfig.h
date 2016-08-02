/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* config/ldmconfig.h.  Generated automatically by configure.  */
#ifndef LDM_CONFIG_H
#define LDM_CONFIG_H

/* #undef BSD */
/* No database support in pqact/filel.c */
#define NO_DB 1
/* Concatenate products in pqact/filel.c. Mutually exclusive with DB_XPROD */
#define DB_CONCAT 1
/* XDR DB products in pqact/filel.c. Mutually exclusive with DB_CONCAT */
#define DB_XPROD 0
/* Use gdbm interface in pqact/filel.c; default is ndbm */
/* #undef USE_GDBM */
#define HAVE_ST_BLKSIZE 1
#define LOGNAME_ISSOCK 1
/* #undef NO_ATEXIT */
#define NO_FSYNC 1
#define NO_FTRUNCATE 1
#define NO_MEMCMP 1
#define NO_MEMMOVE 1
#define NO_MMAP 1
#define NO_POSIXSIGNALS 1
#define NO_RENAME 1
#define NO_REPLACE_SYSLOG 1
#define NO_SETENV 1
#define NO_SETEUID 1
#define NO_STRDUP 1
#define NO_STRERROR 1
#define NO_WAITPID 1
/* #undef PORTMAP */
#define SIZEOF_INT 0
#define SIZEOF_LONG 0
#define SYSLOG_PIDFILE "/var/run/syslogd.pid"
/* #undef SYSLOG_RETURNS_INT */
/* #undef TV_INT */
/* #undef UD_SETPROCTITLE */
/* #undef _DEV_CONSLOG */
#define _MAPRGNS 1
#define _NOGROW 1
/* #undef _SYS_USER_INCLUDED */
/* #undef _XOPEN_SOURCE_EXTENDED */
/* #undef const */
/* #undef off_t */
#define ptrdiff_t int
/* #undef sig_atomic_t */
/* #undef ssize_t */
/* #undef socklen_t */
#define LDMHOME "/d2/oien/ldm-6.0.14"
#define LOG_LDM LOG_LOCAL0
#define LDM_PORT 388
#define LDM_PROG 300029

#endif
