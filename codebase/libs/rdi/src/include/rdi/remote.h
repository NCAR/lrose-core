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
/****************************************************************
	File: remote.h
	header file for remoted and remote lib 

   Notes:

   A. General:

	The Remote software is a remote UNIX system call library, 
	which allows an application to make system calls on a remote
	host as if it were running on that host. The tool includes a 
	daemon and a library. The daemon acts as a server and runs on 
	the remote host. An application that needs to perform a remote 
	system call must link the library into its executable. The tool
	works in a simple client/server mode. When an application 
	calls a Remote library routine to make a remote system call,
	the Remote routine first looks for an existing connection to 
	that host, and, if the connection does not exist, it
	tries to make a connection to the daemon through a socket.
	Then it sends a request to the server and waits for a return
	message. The server then responds to the request by performing
	the required system call and sending the results to the
	client.

	The remote system call facility is built on top of UNIX TCP/IP 
	sockets. Thus the communication is reliable - except when the 
	connection is broken, one of the processes is dead or there 
	is a hardware problem. If this happens, the client's call
	fails and receives a message indicating a communication error. 
	The server closes all resources opened by that client and the 
	connection.

	The server, called remoted, is a concurrent server. It forks a
	child process for each client process. The connection will 
	remain open for 300 seconds after either all opened 
	resources have been closed or there is no action. Forking a
	child process to serve the client makes the server less
	sensitive to a faulty client and, thus, the server will be more
	robust. Retaining the connection for a certain time reduces the
	possibility of frequently forking processes. If a serious error
	occurs due to communication or a faulty client, the child 
	disconnects the socket and exits.

	If a system crach, or a communication hardware problem, 
	happens on the client side, the server
	uses the SO_KEEPALIVE feature to detect it and exits. If a 
	system crash happens on the server side, a client remote
	call will return after a timed out. In either case, the
	opened resources are lost. E.g. an opened fd becomes stale
	and the application needs to reopen that resource. 

	The default period of timed-out in the client side is 20
	seconds, which
	means that if a Remote call can not finish in 20 seconds,
	it will return assuming that the connection is broken.
	The user can call set_timed_out(n_seconds) to set a different
	timed-out period. If n_seconds=0 in the above call, 
	the timed-out period is set to the above default value again.

	On the server side, it may take a few hours for the server
	to detect a dead connection through keepalive prob messages
	in TCP/IP. The user can use -f option, when he/she starts the
	remoted, to turn on a faster detection feature build in 
	the tool, which takes about 8 minutes to detect a broken
	connection.

	It is probably possible to build a crash recovery scheme in 
	the remote tool. A possible way to do this is to store
	state information of the opened resources in the client side.
	When the client keeps trying to use a stale fd, the state
	information of all resources is sent to the server. After the
	server is restarted, it first tries to rebuild the broken 
	resources by accepting the state information, reopening
	lost resources and resetting their states. This kind of
	crash recovery is not implemented yet.

	Security:

	a. The user can only start remoted within a short period
	of time after login.

	b. The user can select a password based security mode. In 
	this mode, the user is asked to input a password when 
	the remoted is started. Then, at the time a Remote library 
	routine, which needs to connect to the daemon, is called in 
	the client application, the user is asked to input the same 
	password. To avoid frequent typing of the password, the remoted 
	sents a key to the client after validation of the password. 
	This key can then be used by the client to get access permission 
	for a certain period of time. The tasks of receiving, storing 
	and reusing the key is processed by the remote library. The key 
	is saved in the ~/.remote file by the remote library. The time 
	the key is valid is specified by the user when remoted is 
	started.

	If a client keeps trying to use invalid passwords or keys, the 
	server will respond by denying access from the client's host for 
	a certain time period. This prevents a rogue client from finding 
	the valid password or key by exhaustive search.

	c. Instead of using a password based security scheme, the 
	user can choose a configuration file scheme. In this case 
	the user prepares a configuration file for the remoted. In the 
	file host names that are permissible to access the remoted, as
	well as the local files and directories that can be accessed
	by the remoted, are listed. This protects all files that are 
	not listed and avoids unexpected accesses from unlisted hosts.
	The lists are echoed on the screen when the remoted is started 
	for further protection. This scheme is useful for an operational
	system.

	The configuration file's name must be remoted.conf and it must
	be in the directory from where the remoted starts. The 
	configuration file contains lines in the following format:

	Client: host_name
	directory

	Example configuration file:

	Client: everest
	Client: brightband

	/weather/ddp
	/tmp


	The Port Number:

	The Remote tool uses an encoded number of the user name for 
	the port number of the socket. This is convenient since 
	Remote is a user level tool. The number, however, generated
	may not be unique. In the case there is a port number conflict
	one of the following schemes can be used instead.

	Optionally the user can specify an environmental variable,
	PORTNAME, replacing the user name. PORTNAME must be defined 
	on both server and client machines. This is useful if there 
	is a port number conflict between different users.
	
	The user can also choose to directly specify a port number,
	when the server is started (as a command option for remoted), and 
	use set_port_number call for setting the port number on the
	client side.

   B. Message format: 

	The user needs this information only when adding new 
	functions to the tool.

	Request (message from the client): a description + data
	  description (char string): 
		msg_len(16 bytes) + command(1-byte-name + arguments:
		length= (strlen(command)+1)>>2)<<2 bytes) + data
	        ( byte array )
	  if msg_len < 128, 128 bytes are sent.
	Return (message to the client) a description + data
	  description (char string): 
	  	command (1-byte-name + return + status:
		length= (strlen(command)+1)>>2)<<2 bytes) + data
		( byte array )
	  if msg_len < 64, 64 bytes are sent.

   C. The user library:

	A version of program "remoted" must be running in 
	background on each of the machines in the target
	communication.

	remote.h must be included and remote.o (or libpipe.a) must 
	be linked into the user program. 
 
	The following system calls and C functions are currently 
	supported:

	open
	read 
	write
	lseek
	close
	write
	fcntl
	close
        kill
	system
	stat
	rename
	truncate
	unlink.

        These system functions use the same function name and the 
	same arguments as the original system calls. There are two ways 
	to specify the remote machine name: 

        a. Call set_remote(mach_name) before a call which needs a 
	mach_name to set a current default remote machine and subsequent
	remote calls will use the default mach_name. This setting
	will last until this function is called with a different 
	mach_name. If set_remote(NULL) or set_remote("") or 
	set_remote(local_machine_name) is called, local machine is set. 
	If set_remote() returns -1, the current remote machine is not 
	changed.

	b. Directly specify the mach_name in the path name such as:
        virga:/home/user/my_file. This specification
        ignores the current remote machine set by set_remote.

        The above functions return the same value and errno as their
	corresponding system calls do in the case when -1 is
        returned. Additional types of errors are indicated by 
	negative errno as listed later in this documentation.

	If the mach_name is missing or it is the local host name,
	the library will directly use the local UNIX system calls
	without using any daemon.

	The next set of functions are remote object pipes:

	int fopcreate(char *filename, int size, int lock);
	int fopopen(char *filename);
	int fopread(int obj, char *ray, int length);
	int fopwrite(int obj, char *ray, int length);
	int fopflush(int obj);
	int fopclose(int obj);
	int ropopen(char *mach, int type, int key, int size);  

	The first 6 functions are explained in fobpipe.h. The 
	difference here is that if the filename contains a machine
	name part, it will create/open a remote pipe (just like open()). 
	ropopen opens a remote shm pipe. It behaves like opopen (see
	obpipe.h) except for the machine name. The returned integer
	handle, then, is used for fopread, fopwrite and fopclose.
	These functions set a negative errno if a 
	communication error is detected. Another difference is that
	if fopwrite returns 0, which means pipe full, rewriting calls
	may use length=0 to save retransmission of the data in the
	following re-write calls. However, there must be no other 
	fopread and fopwrite calls in between.

	In the implementation the fopread, fopwrite and fopclose
        are shared with read, write and close. The conventional
	file descriptors returned by open are assumed to be between
	0 and 127. The opopen returns a descriptor from 128 to 143,
        and fopopen uses values from 144 to 159. If fopopen opens
	a local pipe, any fopread, fopwrite, fopflush and fopclose
	for that pipe will call local functions instead of going 
	through the remoted. However, ropopen requires a machine name, 
	and the pipe opened always goes through remoted even if
	the pipe is on the same host (where a local remoted must be
	running).

	The user may call set_remote_msg(1) to switch on (or
	set_remote_msg(0) for off) the error message printing in 
	the remote package.


   D. Remarks:

	How to add a new function:
	    add that to enum in remote.h;
	    add a new item in function process_msg() in remoted.c
	    add a p_?? () to remoted.c
	    add a r?? () to remote.c
	    modify the description in remote.h
	    recompile remoted and remote.c

	Bugs:
	    Further work on the MACRO transfer across different
	machines needs to be done (currently only the flags in
	in open() have been processed).
	    RPC can be added.
	    It is also possible to add signal registration (a 
	    special server message must be defined).

	    
    E. List of additional errno:

	    
	    -1: Failed in sending msg to the server.
	    -2: Failed in receiving the msg from the server.
	    -3: Too many connections built.
	    -4: Failed in making new connection; get_port_number().
	    -5: Failed in making new connection; gethostbyname().
	    -6: Failed in making new connection; socket().
	    -7: Failed in making new connection; connect().
	    -8: Failed in making new connection; fcntl(F_SETOWN)
	    -9: Failed in making new connection; set_sock_properties().
	    -10: Failed in build new connection; pass_security_check().
	    -11: Bad pipe descriptor.
	    -12: Corrupted data received.
	    -13: Bad machine name specified.
	    -14: Rename does not work across two machines.
	    -15: Failed in gethostname().
	    -16: Permission denied.
	    -17: Bad pipe name.

    F: An example:

	The following example shows how to open a file on a 
	the host virga and read a part of it.

	#include "remote.h"

	int fd;

	if((fd=open("virga:/home/jing/test/test_file",O_RDONLY,0))
				<0){
	    ...open failed...
	}
	if(lseek(fd,offset,0)<0) {
	    ...lseek failed...
	}
	if(read(fd,buf,len)<len){
	    ...read failed...
	}
	if(close(fd)<0){
	    ...close failed...
	}

	       


****************************************************************/

/* Here is the actual header file detailing the function 
   templates */

#include <sys/stat.h>
#include <sys/types.h>

#define TEST_JUNK 254

#ifndef RKC

int ropen__r (char *path, int flag, mode_t mode);
int rread__r (int fd, char *buf, int nbyte);
int rseek__r (int fd, off_t offset, int whence);
int rclose__r (int fd);
int rwrite__r (int fd, char *buf, int nbyte);
int rfcntl__r (int fd, int cmd, int arg);
int rkill__r (int pid, int sig);
int rsystem__r (char *str);
int rstat__r (char *path, struct stat *);
int rrename__r (char *path1, char *path2);
int rtruncate__r (char *path, int length);
int runlink__r (char *path);

int set_remote (char *);
void set_port_number (int port);
void set_remote_msg (int sw);
void set_timed_out (int sw);

/* the remote object pipe */
int fopopen__r (char *filename);
int ropopen (char *mach, int type, int key, int size);
int fopflush__r (int obj);
int fopcreate__r (char *filename, int size, int lock);

#else
void set_remote_msg ();
void set_port_number ();
void set_timed_out ();

#endif

#ifdef LIB_MODULE

enum {
    Open = 33, Close, Lseek, Write, Read, Fcntl, Ioctl,
    Kill, System,
    Stat, Rename, Truncate, Unlink,
    Mopen, Mread, Mwrite,
    Fopopen, Ropopen, Fopflush, Fopcreate,
    Noop			/* no operation - testing communication */
};

#else

#define read(a,b,c) rread__r(a,b,c)
#define write(a,b,c) rwrite__r(a,b,c)
#define open(a,b,c) ropen__r(a,b,c)
#define lseek(a,b,c) rseek__r(a,b,c)
#define close(a) rclose__r(a)
#define fcntl(a,b,c) rfcntl__r(a,b,c)
#define kill(a,b) rkill__r(a,b)
#define system(a) rsystem__r(a)
#define stat(a,b) rstat__r(a,b)
#define rename(a,b) rrename__r(a,b)
#define truncate(a,b) rtruncate__r(a,b)
#define unlink(a) runlink__r(a)

#define fopopen(a) fopopen__r(a)
#define fopread(a,b,c) rread__r(a,b,c)
#define fopwrite(a,b,c) rwrite__r(a,b,c)
#define fopclose(a) rclose__r(a)
#define fopflush(a) fopflush__r(a)
#define fopcreate(a,b,c) fopcreate__r(a,b,c)

#endif

#ifdef __cplusplus             
}
#endif
