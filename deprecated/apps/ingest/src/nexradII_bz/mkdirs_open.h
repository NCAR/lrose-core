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
#ifndef _MKDIRS_H_
#define _MKDIRS_H_

/*
 * Like mkdir(2), but will create components as necessary.
 * The specified mode is used to create the directories.
 * Returns 0 if successful, -1 on failure.
 */
#ifdef __cplusplus
extern "C" int mkdirs(const char *path, mode_t mode);
#elif defined(__STDC__)
extern int mkdirs(const char *path, mode_t mode);
#else /* Old Style C */
extern int mkdirs();
#endif


/*
 * Like open(2), but will create components as necessary.
 * Returns valid file descriptor if successful, -1 on failure.
 */
#ifdef __cplusplus
extern "C" int mkdirs_open(const char *path, int flags, mode_t mode);
#elif defined(__STDC__)
extern int mkdirs_open(const char *path, int flags, mode_t mode);
#else /* Old Style C */
extern int mkdirs_open();
#endif

/*
 * Check to see if we have access to all components of 'path'
 * up to the last component. (Doesn't check the access of the full path)
 * If 'create' is no zero, attempt to create path components (directories)
 * as necessary.
 * Returns 0 if access is ok, -1 on error.
 */
#ifdef __cplusplus
extern "C" int diraccess(const char *path, int access_m, int create);
#elif defined(__STDC__)
extern int diraccess(const char *path, int access_m, int create);
#else /* Old Style C */
extern int diraccess();
#endif

#endif /* !_MKDIRS_H_ */
