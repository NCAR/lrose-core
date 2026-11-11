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
/*
 * pmu : proc mapper utilities
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Feb 1996
 *
 * Hacked from smu.c
 */

#include <toolsa/globals.h>
#include <toolsa/port.h>
#include <toolsa/procmap.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/udatetime.h>
#include <dataport/bigend.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <sys/stat.h>
#include <pthread.h>

#include <netinet/in.h> /* sin_family sin_addr */

#define MAX_WAIT_MSEC 10000 /* 10 secs */

static char Name[PROCMAP_NAME_MAX];
static char Instance[PROCMAP_INSTANCE_MAX];
static int Verbose = FALSE;
static int Debug_msgs = FALSE;
static time_t Prev_time_registered = 0;
static int Reg_interval = PROCMAP_REGISTER_INTERVAL;
static int Max_reg_interval = 2 * PROCMAP_REGISTER_INTERVAL;
static int Init_done = 0;
static long Start_time = 0;
static si32 Status = 0;
static int Port = -1;
static si32 N_reg = 0;
static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * byte ordering routines
 */

void PMU_ntohl_Info(PROCMAP_info_t *info)
{
  BE_to_array_32((ui32 *) info, PROCMAP_INFO_NBYTES_SWAP);
}

void PMU_htonl_Info(PROCMAP_info_t *info)
{
  BE_from_array_32((ui32 *) info, PROCMAP_INFO_NBYTES_SWAP);
}

void PMU_ntohl_Request(PROCMAP_request_t *req)
{
}

void PMU_htonl_Request(PROCMAP_request_t *req)
{
}

void PMU_ntohl_Reply(PROCMAP_reply_t *reply)
{
  BE_to_array_32((ui32 *) reply, PROCMAP_REPLY_NBYTES_SWAP);
}

void PMU_htonl_Reply(PROCMAP_reply_t *reply)
{
  BE_from_array_32((ui32 *) reply, PROCMAP_REPLY_NBYTES_SWAP);
}

/*------------- resolve hostname ------------------------------*/

static const char *Pmu_hostname(const char *hostname)
{
  static char local[PROCMAP_HOST_MAX];
  static int first = TRUE;

  if (first) {
    STRncopy(local, PORThostname(), PROCMAP_HOST_MAX);
    first = FALSE;
  }

  if ((hostname == NULL) || (strlen(hostname) == 0)) {
    return NULL;
  } else if (STRequal("local", hostname)) {
    return local;
  } else if (STRequal("none", hostname) || STRequal("null", hostname)) {
    return NULL;
  } else {
    return hostname;
  }

}

/*------------------ proc mapper notify -----------------------*/

static int Communicate(int type,
		       const char *host,
		       void *messin,
		       long messin_len, 
		       char **messout,
		       long *messout_len)
{
  static char *mess = NULL;
  static int mess_len = 0;
  static int port_set = FALSE;
  int cd, ret;
  SKU_header_t head;
  
  if (host == NULL) {
    return FALSE;
  }
 
  if (!port_set) {
    Port = PMU_get_default_port();
    port_set = TRUE;
  }

  cd = SKU_open_client (host, Port); 
  if (cd < 0) {
    return FALSE;
  }
 
  if (1 != SKU_writeh(cd, messin, messin_len, type, -1)) {
    SKU_close(cd);
    return FALSE;
  }
 
  ret = SKU_read_header(cd, &head, MAX_WAIT_MSEC);
 
  if(ret != 1) {
    SKU_close(cd);
    return(FALSE);
  }
 
  /* make sure messout is big enough */
  if (mess == NULL) {
    mess = (char *) malloc(head.len);
    if(!mess) {
      SKU_close(cd);
      return(FALSE);
    }
    mess_len = head.len;
  } else if (head.len > mess_len) {
    char * newmess = (char *) realloc(mess, head.len);
    if(!newmess) {
      SKU_close(cd);
      return(FALSE);
    }
    mess = newmess;
    mess_len = head.len;
  }
  
  SKU_read(cd, mess, head.len, -1);
  SKU_close(cd);

  *messout_len = head.len;
  *messout = mess;
  return (ret == 1);
}

/*------------------ PROCMAP_DIR file backend -----------------------*/

/*
 * Compute the path of the registration file in proc_dir.
 * Returns 0 on success, -1 if the path does not fit.
 */

static int Proc_dir_path(char *path, size_t len, const char *proc_dir,
                         const char *prog_name, const char *instance)
{
  int n = snprintf(path, len, "%s/%s.%s", proc_dir, prog_name, instance);
  return (n < 0 || (size_t) n >= len) ? -1 : 0;
}

/*
 * Write the registration file for this process into proc_dir.
 * info is in host byte order. status_str is the caller's full string,
 * written as-is in the legacy format.
 *
 * If extended is set, the file is in 'key=value' format, otherwise the
 * legacy 3-line format.
 *
 * The file is written to a temporary name in the same directory and
 * then renamed onto the final name. rename() replaces the target
 * atomically, so a reader always sees either the previous complete
 * file or the new complete file - never an empty or partial one.
 * The temporary name starts with '.' so directory scanners can skip it.
 *
 * See pmu.h for the file format.
 */

static void Write_proc_dir_file(const char *proc_dir,
                                const PROCMAP_info_t *info,
                                const char *status_str,
                                int extended)
{
  char proc_path[MAX_PATH_LEN];
  char tmp_path[MAX_PATH_LEN];
  FILE *proc_file;
  int n;

  if (Proc_dir_path(proc_path, sizeof(proc_path),
                    proc_dir, info->name, info->instance)) {
    return;
  }
  n = snprintf(tmp_path, sizeof(tmp_path), "%s/.%s.%s.%d.tmp",
               proc_dir, info->name, info->instance, (int) info->pid);
  if (n < 0 || (size_t) n >= sizeof(tmp_path)) {
    return;
  }

  if ((proc_file = fopen(tmp_path, "w")) == NULL) {
    return;
  }

  if (extended) {

    /* status string must stay on one line */

    char one_line[PROCMAP_STATUS_MAX];
    char *cp;
    STRncopy(one_line, info->status_str, PROCMAP_STATUS_MAX);
    for (cp = one_line; *cp != '\0'; cp++) {
      if (*cp == '\n' || *cp == '\r') {
        *cp = ' ';
      }
    }

    fprintf(proc_file, "name=%s\n", info->name);
    fprintf(proc_file, "instance=%s\n", info->instance);
    fprintf(proc_file, "pid=%d\n", (int) info->pid);
    fprintf(proc_file, "host=%s\n", info->host);
    fprintf(proc_file, "user=%s\n", info->user);
    fprintf(proc_file, "start_time=%ld\n", (long) info->start_time);
    fprintf(proc_file, "heartbeat_time=%ld\n", (long) info->heartbeat_time);
    fprintf(proc_file, "max_reg_interval=%ld\n", (long) info->max_reg_interval);
    fprintf(proc_file, "status=%ld\n", (long) info->status);
    fprintf(proc_file, "n_reg=%ld\n", (long) info->n_reg);
    fprintf(proc_file, "status_str=%s\n", one_line);

  } else {

    fprintf(proc_file, "%d\n", (int) info->pid);
    fprintf(proc_file, "%ld\n", (long) info->max_reg_interval);
    fprintf(proc_file, "%s\n", status_str);

  }

  if (fclose(proc_file) != 0) {
    unlink(tmp_path);
    return;
  }

  if (rename(tmp_path, proc_path) != 0) {
    unlink(tmp_path);
  }
}

/*
 * Remove the registration file for this process from proc_dir.
 */

static void Remove_proc_dir_file(const char *proc_dir,
                                 const char *prog_name,
                                 const char *instance)
{
  char proc_path[MAX_PATH_LEN];
  if (Proc_dir_path(proc_path, sizeof(proc_path),
                    proc_dir, prog_name, instance) == 0) {
    unlink(proc_path);
  }
}

/**********************************************************************
 *
 * Communicate_both()
 *
 * Communicate with both hosts as appropriate
 *
 * Returns 1 on success, 0 on failure.
 */

static int
Communicate_both(int type,
		 PROCMAP_info_t *info,
		 const char *proc_host, const char *proc_host2)
{
  
  int ret = FALSE;
  char *mess;
  long len;

  /*
   * register if primary host is non-null
   */
  
  if (Pmu_hostname(proc_host) != NULL) {
    
    ret |= Communicate(type, Pmu_hostname(proc_host),
		       info, sizeof(PROCMAP_info_t), &mess, &len);
    
    /*
     * register with second host if non-null and it is
     * different from primary
     */
    
    if (Pmu_hostname(proc_host2) != NULL &&
	strcmp(Pmu_hostname(proc_host),
	       Pmu_hostname(proc_host2))) {
      
      ret |= Communicate(type, Pmu_hostname(proc_host2),
			 info, sizeof(PROCMAP_info_t), &mess, &len);
      
    }
    
  }

  return(ret);
  
}

/* NOTE: This mimics the behavior of DsLocator
 *   We don't us DsLocator here to avoid burdonsome library dependencies
 *   However, if the port offset of 3 for the procmap executable
 *   changes in DsLocator, it must also change here!
 */

int PMU_get_default_port()
{

  int   basePort    = 5430;
  int   portOffset  = 3;
  int   defaultPort = -1;
  char *portstr;

  /*
   * first try PROCMAP_PORT
   */
  
  portstr = getenv("PROCMAP_PORT");
  if (portstr != NULL) {
    int port;
    if (sscanf(portstr, "%d", &port) == 1 && port > 5000) {
      return port;
    }
  }

  /*
   * then try DS_BASE_PORT
   */

  portstr = getenv("DS_BASE_PORT");
  if (portstr == NULL) {
    defaultPort = basePort + portOffset;
  } else {
    int port;
    if (sscanf(portstr, "%d", &port) == 1 && port > 5000 && port < INT_MAX - portOffset) {
      defaultPort = port + portOffset;
    } else {
      fprintf(stderr, "ERROR - pmu::get_default_port\n"
	      " Env var DS_BASE_PORT is not valid.\n"
	      " Port number must be an integer greater than 5000\n" );
    }
  }

  return( defaultPort );

}

void PMU_register(const char *prog_name, const char *instance,
		  int max_reg_interval, const char *status_str)

{
  if (Start_time == 0) {
    Start_time = time(NULL);
  }

  if (pthread_mutex_trylock(&_mutex) == EBUSY) {
    /* already busy, do not need to register */
    return;
  }

  PMU_register_pid(prog_name, instance, max_reg_interval,
		   status_str, (int)getpid(), Start_time);

  /*  unlock */
  pthread_mutex_unlock(&_mutex);

  if (Verbose || Debug_msgs) {
    time_t t = time(0);
    fprintf(stderr, "-------------------------------------------------------\n");
    fprintf(stderr, "PMU_register - verbose mode\n");
    fprintf(stderr, "Now: %s\n", utimstr(t));
    fprintf(stderr, "Prog_name: '%s', instance '%s'\n", prog_name, instance);
    fprintf(stderr, "Max_reg_interval: %d\n", max_reg_interval);
    fprintf(stderr, "Status str: '%s'\n", status_str);
    fprintf(stderr, "Time: %s\n", utimstr(time((time_t *)NULL)));
    
  }
  
  return;
}

void PMU_register_pid(const char *prog_name, const char *instance,
		      int max_reg_interval, const char *status_str, int pid,
		      time_t start_time)

{

  static int first_call = TRUE;
  static char *proc_host;
  static char *proc_host2;
  static char *proc_dir;
  static char *proc_user;
  static int proc_dir_extended;
  char *env_str;
  PROCMAP_info_t info;

  /*
   * get environment variables
   */

  if (first_call) {
    proc_dir = getenv("PROCMAP_DIR");
    env_str = getenv("PROCMAP_DIR_EXTENDED");
    proc_dir_extended = (env_str != NULL && STRequal(env_str, "true"));
    proc_host = getenv("PROCMAP_HOST");
    proc_host2 = getenv("PROCMAP_HOST2");
    proc_user = getenv("USER");
    if(proc_user == NULL)
      proc_user = getenv("LOGNAME");
    first_call = FALSE;
  }

  /*
   * load up info struct
   */

  memset(&info, 0, sizeof(PROCMAP_info_t));
  info.heartbeat_time = time(NULL);
  info.start_time = start_time;
  info.pid = pid;
  info.max_reg_interval = max_reg_interval;
  info.status = Status;
  info.n_reg = ++N_reg;
  STRncopy(info.name, prog_name, PROCMAP_NAME_MAX);
  STRncopy(info.instance, instance, PROCMAP_INSTANCE_MAX);
  STRncopy(info.status_str, status_str, PROCMAP_STATUS_MAX);
  STRncopy(info.host, Pmu_hostname("local"), PROCMAP_HOST_MAX);
  STRncopy(info.user, proc_user ? proc_user : "", PROCMAP_USER_MAX);

  if (proc_dir != NULL) {
    Write_proc_dir_file(proc_dir, &info, status_str, proc_dir_extended);
  }
  
  PMU_htonl_Info(&info);
    
  Communicate_both(PROCMAP_REGISTER, &info, proc_host, proc_host2);

  return;

}

void PMU_unregister(const char *prog_name, const char *instance)

{
  PMU_unregister_pid(prog_name, instance, (int)getpid());
  
  return;
}

void PMU_unregister_pid(const char *prog_name, const char *instance, int pid)

{
  
  char *proc_host;
  char *proc_host2;
  char *proc_dir;
  char *proc_user;
  PROCMAP_info_t info;

  /*
   * get environment variables
   */
  
  proc_dir = getenv("PROCMAP_DIR");
  if (proc_dir != NULL) {
    Remove_proc_dir_file(proc_dir, prog_name, instance);
  }

  proc_host = getenv("PROCMAP_HOST");
  proc_host2 = getenv("PROCMAP_HOST2");
  proc_user = getenv("USER");
  if(proc_user == NULL)
    proc_user = getenv("LOGNAME");

  /*
   * load up info struct
   */
  
  memset(&info, 0, sizeof(info));
  
  info.heartbeat_time = time(NULL);
  info.pid = pid;
  STRncopy(info.name, prog_name, PROCMAP_NAME_MAX);
  STRncopy(info.instance, instance, PROCMAP_INSTANCE_MAX);
  STRncopy(info.host, Pmu_hostname("local"), PROCMAP_HOST_MAX);
  STRncopy(info.user, proc_user, PROCMAP_USER_MAX);
  
  PMU_htonl_Info(&info);
  
  Communicate_both(PROCMAP_UNREGISTER, &info, proc_host, proc_host2);

  Prev_time_registered = 0;

  return;

}

/**************************
 * request info - no relay
 */

static int _requestInfo(PROCMAP_request_t *req, int *n_procs,
			long *uptime, PROCMAP_info_t **info,
			const char *host1, const char *host2)
{

  char *mess;
  long mess_len = 0;

  PROCMAP_request_t req_copy = *req;
  PROCMAP_reply_t *reply;
  PROCMAP_info_t *infoptr;
  char *ptr;
  int ret, i;

  *n_procs = 0;
  *uptime = 0;
  
  /* put in network byte order */
  PMU_htonl_Request(&req_copy);
  
  /* Communicate request first to host1 and then host2 if failed */
  if (FALSE == (ret = Communicate(PROCMAP_GET_INFO,
				  Pmu_hostname(host1), &req_copy, 
				  sizeof(req_copy), &mess, &mess_len))) {
    ret = Communicate(PROCMAP_GET_INFO,
		      Pmu_hostname(host2), &req_copy, 
		      sizeof(req_copy), &mess, &mess_len);
  }
  
  if (!ret) {
    return -1;
  }
  
  reply = (PROCMAP_reply_t *) mess;
  PMU_ntohl_Reply(reply);

  *uptime = reply->uptime;
  *n_procs = reply->n_procs;

  if (PROCMAP_SUCCESS != reply->return_code) {
    return 0;
  }

  if (*n_procs == 0) {
    return 0;
  }

  /* byte order on all info messages */
  ptr = mess + sizeof(PROCMAP_reply_t);
  *info = (PROCMAP_info_t *) ptr;
  for (i=0; i < reply->n_procs; i++) {
    infoptr = (PROCMAP_info_t *) ptr;
    PMU_ntohl_Info(infoptr);
    ptr += sizeof(PROCMAP_info_t);
  }
  
  return 1;
}

/****************************
 * request info - with relay
 */

static int _requestInfoRelay(PROCMAP_request_t *req, int *n_procs,
			     long *uptime, PROCMAP_info_t **info,
			     const char *hostlist1,
			     const char *hostlist2)
{
  
  char *mess;
  long mess_len = 0;
  
  char contact_host[PROCMAP_HOST_RELAY_MAX];
  
  PROCMAP_request_relay_t req_relay;
  PROCMAP_reply_t *reply;
  PROCMAP_info_t *infoptr;
  char *ptr, *colon;
  int ret, i;

  /*
   * initialize
   */
  
  *n_procs = 0;
  *uptime = 0;
  
  STRncopy(req_relay.name, req->name, PROCMAP_NAME_MAX);
  STRncopy(req_relay.instance, req->instance, PROCMAP_INSTANCE_MAX);

  /*
   * try host 1 if you can
   */
  if (hostlist1 == NULL) {
    ret = FALSE;
  } else {
    colon = strstr(hostlist1, ":");
    if (colon == NULL) {
      STRncopy(contact_host, hostlist1, PROCMAP_HOST_RELAY_MAX);
      STRncopy(req_relay.relay_hosts, "", PROCMAP_HOST_RELAY_MAX);
    } else {
      STRncopy(contact_host, hostlist1, colon - hostlist1);
      STRncopy(req_relay.relay_hosts, colon, PROCMAP_HOST_RELAY_MAX);
    }
    ret = Communicate(PROCMAP_GET_INFO_RELAY,
		      Pmu_hostname(contact_host), &req_relay, 
		      sizeof(req_relay), &mess, &mess_len);
  }

  if (ret == FALSE) {
    /*
     * try second host, if you can
     */
    if (hostlist2 == NULL) {
      return -1;
    }
    
    colon = strstr(hostlist2, ":");
    if (colon == NULL) {
      STRncopy(contact_host, hostlist2, PROCMAP_HOST_RELAY_MAX);
      STRncopy(req_relay.relay_hosts, "", PROCMAP_HOST_RELAY_MAX);
    } else {
      STRncopy(contact_host, hostlist2, colon - hostlist2);
      STRncopy(req_relay.relay_hosts, colon, PROCMAP_HOST_RELAY_MAX);
    }
    
    ret = Communicate(PROCMAP_GET_INFO_RELAY,
		      Pmu_hostname(contact_host), &req_relay, 
		      sizeof(req_relay), &mess, &mess_len);
  }
  
  if (!ret) {
    return -1;
  }
  
  reply = (PROCMAP_reply_t *) mess;
  PMU_ntohl_Reply(reply);

  *uptime = reply->uptime;
  *n_procs = reply->n_procs;

  if (PROCMAP_SUCCESS != reply->return_code) {
    return 0;
  }

  if (*n_procs == 0) {
    return 0;
  }

  /* byte order on all info messages */
  ptr = mess + sizeof(PROCMAP_reply_t);
  *info = (PROCMAP_info_t *) ptr;
  for (i=0; i < reply->n_procs; i++) {
    infoptr = (PROCMAP_info_t *) ptr;
    PMU_ntohl_Info(infoptr);
    ptr += sizeof(PROCMAP_info_t);
  }
  
  return 1;
}

/**************
 * request info
 */

int PMU_requestInfo(PROCMAP_request_t *req, int *n_procs, long *uptime,
		    PROCMAP_info_t **info,
		    const char *host1, const char *host2)
{

  /*
   * check for presence of : in host names - if there, we have a 
   * request to relay the request to the first host in the list
   */
  
  if ((host1 == NULL || strstr(host1, ":") == NULL) &&
      (host2 == NULL || strstr(host2, ":") == NULL)) {

    return _requestInfo(req, n_procs, uptime,
			info, host1, host2);

  } else {

    return _requestInfoRelay(req, n_procs, uptime,
			     info, host1, host2);

  }

}

/******************************************
 * PMU_auto_init()
 *
 * Sets up statics for auto regsitration
 */

void PMU_auto_init(const char *prog_name,
		   const char *instance,
		   int reg_interval)
     
{

  char *env_str;

  /*
   * make sure sigpipe is turned off
   */

  PORTsignal(SIGPIPE, SIG_IGN);
  
  STRncopy(Name, prog_name, PROCMAP_NAME_MAX);
  STRncopy(Instance, instance, PROCMAP_NAME_MAX);
  Max_reg_interval = reg_interval * 2;
  Reg_interval = reg_interval;

  /* In verbose mode, the PMU_auto_register funciton will register with procmap on */
  /* every call without checking how long it's been since the previous registration. */
  /* This is useful in an operational system when firguring out where a process is */
  /* spending too much time without registering and so is being restarted. */

  env_str = getenv("PROCMAP_VERBOSE");
  if (env_str && STRequal(env_str, "true")) {
    Verbose = TRUE;
  } else {
    Verbose = FALSE;
  }

  /* In debug_msgs mode, the PMU functions will print out some debug messages while */
  /* they work.  This is meant for debugging the PMU functions. */

  env_str = getenv("PROCMAP_DEBUG_MSGS");
  if (env_str && STRequal(env_str, "true")) {
    Debug_msgs = TRUE;
  } else {
    Debug_msgs = FALSE;
  }

  Start_time = time(NULL);
  Status = 0;
  Init_done = 1;
  PMU_auto_register("PMU_auto_init");
  return;
  
}

/******************************************
 * PMU_clear_init()
 *
 * Clears flag set up by init.
 * Use this to prevent any further registrations.
 */

void PMU_clear_init()
     
{
  Init_done = 0;
}

/******************************************
 * PMU_auto_register()
 *
 * Automatically registers if Reg_interval secs have expired
 * since the previous registration.
 *
 * This routine may be called frequently - registration will
 * only occur at the specified Reg_interval.
 */

void PMU_auto_register(const char *status_str)

{

  if (!Init_done) {
    return;
  }

  if (Verbose) {

    /* In Verbose mode, we register with procmap on every call, regardless of */
    /* when the previous call was made */

    PMU_register(Name, Instance, Max_reg_interval, status_str);

  } else {

    /*
     * get the time
     */
    
    time_t this_time = time((time_t *) NULL);

    if (this_time - Prev_time_registered > 4397 &&
	this_time - Prev_time_registered <= 4399) {
      /*
       * A known LINUX kernel bug in which time jumps forward around 4398 seconds
       */
      if (Debug_msgs)
	fprintf(stderr, "**LINUX BUG** Time jumped forward %d seconds..ignore\n",
		(int)(this_time - Prev_time_registered));
      return;
    }
    
    /*
     * register process if time has arrived
     */
    
    if ((this_time - Prev_time_registered) > Reg_interval) {
      
      if (Debug_msgs)
	fprintf(stderr, "Before register prev time=%s  this=%s  interval=%d\n",
		utimstr(Prev_time_registered),
		utimstr(this_time), Reg_interval);

      PMU_register(Name, Instance, Max_reg_interval, status_str);

      if (Debug_msgs)
	fprintf(stderr, "After register prev time=%s  this=%s  interval=%d\n",
		utimstr(Prev_time_registered),
		utimstr(this_time), Reg_interval);

      Prev_time_registered = this_time;
      
    }
    else {
      if (Debug_msgs)
	fprintf(stderr, "register but wait.. %s dt=%d %s\n", 
		utimstr(this_time), (int)(this_time - Prev_time_registered),
		status_str);
    }

  } /* if (Verbose) */

  return;
}

/******************************************
 * PMU_force_register()
 *
 * Forced registration.
 *
 * This routine should only be called from places in the code which do
 * not run frequently. Call PMU_auto_register() from most places
 * in the code.
 */

void PMU_force_register(const char *status_str)

{

  if (!Init_done) {
    if (Debug_msgs)
    {
      fprintf(stderr, "WARNING - PMU_force_register()\n");
      fprintf(stderr, "Cannot register - init not done\n");
    }
    return;
  }

  PMU_register(Name, Instance, Max_reg_interval, status_str);
  if (Debug_msgs)
    fprintf(stderr, "After Forced register, prev time=%s\n",
	    utimstr(Prev_time_registered));

  return;

}

/******************************************
 * PMU_auto_unregister()
 *
 * Automatically unregisters - remembers process name
 * and instance
 */

void PMU_auto_unregister(void)

{

  if (Init_done) {
    PMU_unregister(Name, Instance);
  }

  return;

}

/******************************************
 * PMU_init_done()
 *
 * Check whether the PMU_auto_init() function has been called.
 * Returns: 1 if init has been called, 0 if init has not been done.
 *
 */

int PMU_init_done()
{
   return (Init_done == 1 ? 1 : 0);
}

/******************************************
 * PMU_set_status()
 *
 * Set status, force register.
 *
 */

void PMU_set_status(si32 status, const char *status_str)

{

  Status = status;
  PMU_force_register(status_str);
  return;

}

  

/******************************************
 * PMU_read_dir_entry()
 *
 * Read one $PROCMAP_DIR registration file into info (host byte order).
 * See pmu.h for details.
 */

int PMU_read_dir_entry(const char *path, PROCMAP_info_t *info)

{

  FILE *fp;
  char line[PROCMAP_STATUS_MAX + 64];
  struct stat sbuf;
  const char *base;
  const char *dot;
  int nlines = 0;
  int keyed = 0;
  int have_pid = 0;

  memset(info, 0, sizeof(PROCMAP_info_t));

  if (stat(path, &sbuf) != 0) {
    return -1;
  }

  /* default name / instance from file name 'name.instance' */

  base = strrchr(path, '/');
  base = (base == NULL) ? path : base + 1;
  dot = strchr(base, '.');
  if (dot != NULL && dot != base) {
    size_t nlen = (size_t) (dot - base);
    if (nlen >= PROCMAP_NAME_MAX) {
      nlen = PROCMAP_NAME_MAX - 1;
    }
    memcpy(info->name, base, nlen);
    info->name[nlen] = '\0';
    STRncopy(info->instance, dot + 1, PROCMAP_INSTANCE_MAX);
  } else {
    STRncopy(info->name, base, PROCMAP_NAME_MAX);
  }

  /* default heartbeat from file modify time */

  info->heartbeat_time = (si32) sbuf.st_mtime;
  info->last_registered = (si32) sbuf.st_mtime;

  if ((fp = fopen(path, "r")) == NULL) {
    return -1;
  }

  while (fgets(line, sizeof(line), fp) != NULL) {

    char *eq;
    char *val;
    size_t len = strlen(line);
    while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
      line[--len] = '\0';
    }

    if (nlines == 0) {
      keyed = (strchr(line, '=') != NULL);
    }
    nlines++;

    if (!keyed) {
      /* legacy 3-line format: pid, max_reg_interval, status_str */
      if (nlines == 1) {
        info->pid = atoi(line);
        have_pid = 1;
      } else if (nlines == 2) {
        info->max_reg_interval = atoi(line);
      } else if (nlines == 3) {
        STRncopy(info->status_str, line, PROCMAP_STATUS_MAX);
      }
      continue;
    }

    if ((eq = strchr(line, '=')) == NULL) {
      continue;
    }
    *eq = '\0';
    val = eq + 1;

    if (STRequal(line, "name")) {
      STRncopy(info->name, val, PROCMAP_NAME_MAX);
    } else if (STRequal(line, "instance")) {
      STRncopy(info->instance, val, PROCMAP_INSTANCE_MAX);
    } else if (STRequal(line, "pid")) {
      info->pid = atoi(val);
      have_pid = 1;
    } else if (STRequal(line, "host")) {
      STRncopy(info->host, val, PROCMAP_HOST_MAX);
    } else if (STRequal(line, "user")) {
      STRncopy(info->user, val, PROCMAP_USER_MAX);
    } else if (STRequal(line, "start_time")) {
      info->start_time = (si32) atol(val);
    } else if (STRequal(line, "heartbeat_time")) {
      info->heartbeat_time = (si32) atol(val);
      info->last_registered = info->heartbeat_time;
    } else if (STRequal(line, "max_reg_interval")) {
      info->max_reg_interval = (si32) atol(val);
    } else if (STRequal(line, "status")) {
      info->status = (si32) atol(val);
    } else if (STRequal(line, "n_reg")) {
      info->n_reg = (si32) atol(val);
    } else if (STRequal(line, "status_str")) {
      STRncopy(info->status_str, val, PROCMAP_STATUS_MAX);
    }
    /* unknown keys are ignored, for forward compatibility */

  }

  fclose(fp);

  return have_pid ? 0 : -1;

}
