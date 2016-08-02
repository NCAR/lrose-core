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
#ifdef __cplusplus                                                        
 extern "C" {                                                         
#endif
/*****************************************************************

	Header file defining the file object pipe structures

	File: fobpipe.h

*****************************************************************/

/*******************************************************************
	Object pipe for interprocess data communication
	File version
	Nov. 17, 1992

	Notes: 

	The file pipe is designed for large buffer size, static
	buffer and multiple access while the shared memory pipe
	is designed for maximum efficiency.

	The pipe is message queue implemented with a file. The file
	should be accessed only through the pipe library functions 
	described in this documentation. 

	To guarantee reliability, file locking scheme is used, which
	implies that only one user can access the pipe file at a time. 
	A user will be blocked if it tries to access while the pipe 
	file is in use. This allows multiple reader and writer access 
	of the pipe.

	Terminating signals are blocked to further protect the 
	integrity the queue. Thus a user should not use "kill -9"
	to terminate a program that calls this library.

	In a normal use, the user calls fopcreate once before any
	communication party is started. Then a communication party
	first calls fopopen to get a handle, and then calls fopread
	or fopwrite or other functions just like read and write 
	system calls. Since the pipe file is static, no
	fopcreate call is needed in programs that communicate.

	Note that One can not implement an automatic check of
	file status because the check and create is not an atom
	function. 
	
	The user should not try to remove the pipe file when any
	of the communicating parties is connected to it. Because
	after rm, the file still exists, if another process accessing
	it, although it does not show up in ls. So a new create will
	creates another file (with the same name) and the
	communication can not be built.

	All functions in this module return a negative number on
	failure. The number indicates why the function failed as 
	described in the following. Sometimes the errno may contain
	further information about the error.

	The user may call set_fobpipe_msg(1) to switch on (or
	set_fobpipe_msg(0) for off) the error message printing in 
	the fobpipe module.

	The routines:

	fopcreate(char *filename, int size, int flag) 
	
	creates the 
	file filename for the pipe and initialized it. The size
	is the file size to be created. The flag argument is not
	used currently. Normally, this function is used (in a 
	initialization program) before any of the communication 
	parties is started.
	If the file exists, this function checks if the file is
	consistent with the function arguments.
	It returns 0 on success and the following negative values 
	on failure:

	-101: The arguement, size, is less than 32.
	-102: The pipe file exists but not a valid pipe file.
	-103: The pipe file exists but its size is incorrect.
	-104: Permission of creating or opening the file denied.
	-105: Failed in locking the file.
	-106: Failed in actually generating the file for some reason
	    such as the file system full.
	-107: Failed in initializing the file header.

	fopopen(char *filename) opens an object pipe of filename. 
	The file must be created by fopcreate() before this call.
	It returns a non-negative integer (a handle) on success or
	the following negative values on failure:

	-108: Too many (more than 16) file pipes opened.
	-109: Failed in allocating a work space.
	-110: The pipe file can not be open. 
	-111: The pipe file has been corrupted.
	-105: Failed in locking the file. 

	fopread(int obj, char *buffer, int buf_size) 
	reads an object and puts it in buffer. buf_size is the number of
	bytes that buffer can hold. The arguement, buf_size,
	must be at least larger than the actual data
	size due the alignment. On success
	it returns the length of the object in number of bytes. 
	Otherwise, it returns 0 if
	the buffer is empty or the following negative values on 
	failure:

	-112: The ob arguement is not a valid pipe fd.
	-113: The pipe file is corrupted - read failed.
	-114: The message size is larger than the buf_size arguement.
	-115: The pipe file is corrupted - write failed.
	-117: The pipe file is corrupted - data error.
	-105: Failed in locking the file. 

	fopwrite(int obj, char *buffer, int length) 
	writes an object in *buffer of length bytes to 
	the buffer. It returns 

	1 if success;
        0 if buffer full;
        It returns the following negative values on failure:
        if return <= 0, that buffer is not written.

	-112: The ob arguement is not a valid pipe fd.
	-116: The message is larger than that the pipe can hold.
	-113: The pipe file is corrupted - read failed.
	-115: The pipe file is corrupted - write failed.
	-117: The pipe file is corrupted - data error.
	-105: Failed in locking the file. 


	fopflush(int obj) discards all data that is in
	the buffer and resets the queue. It returns 0 on success and 
	-1 on failure.

	-112: The ob arguement is not a valid pipe fd.
	-115: The pipe file is corrupted - write failed.
	-105: Failed in locking the file. 

	fopclose(int obj) closes the object. It returns
	0 if success or -1 if failed.
	
	-112: The ob arguement is not a valid pipe fd.

	Neither reader nor write can change the buffer size which
	is determined at the time of pipe file creation and
        initialization.

	In the file the first long word is a file identification
	number; the second is the buffer size; the
	the third and the forth are r_pt and w_pt respectively.
	The fifth is a flag word, which is not used currently.
        The sixth word is reserved.

	The object length is rounded to nearest 4 byte before it
	is written. This is useful if the file is to be mapped to 
	a memory segment. $$$ This should be fixed.

	The performance: For short objects of 16 bytes, about
	1000 objects piped in and out (SUN-SPARC II).

	

*******************************************************************/

#ifndef RKC

int fopopen (char *filename);
int fopcreate (char *filename, int size, int lock);
int fopread (int obj, char *buffer, int length);
int fopwrite (int obj, char *buffer, int length);
int fopclose (int obj);
int fopflush (int obj);
int foplen_needed(int ob);
void set_fobpipe_msg (int sw);

#else
void set_fobpipe_msg ();

#endif
#ifdef __cplusplus             
}
#endif
