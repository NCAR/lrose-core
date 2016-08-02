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
/*******************************************************************
  Object pipe for interprocess data communication
  File version
  Nov. 17, 1992
  
  *******************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef AIX
#include <sys/select.h>
#endif 

#include <rdi/fobpipe.h>

#define FD_SET_P (fd_set *)

struct fobject_pipe {
  long idn; /* a file identification number: 324578594 */
  long b_size;/*buffer size for data excluding header(24+4 bytes)*/
  int h_size; /* header size: 6*sizeof(long)=24 */
  int r_off, w_off; /* offset for read and write pointers */
  int fd;
  long flag; /* status flag of the pipe */
};
typedef struct fobject_pipe Fobject_pipe;

static int set_lock(int sw, int fd);
static int get_pointer(int ob);
static int sig_block(int sw);

/* we store the object handle locally. This has the following
   advantages: Better handle check (closed handle does not cause
   memory error; possibility of being differentiated from regular
   file fd; Possible to add a commu. fd number in higher byte, which
   is required by the remote software */
/* fd+144 for file pipe, at most 16 pipes opened */
static int Obj_pt[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int Length_needed[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static int Msg_on=0;

/*************************
 * sleep for micro-seconds
 */

static void micro_sleep(unsigned int usecs)

{
  
  struct timeval sleep_time;
  fd_set read_value;
  
  sleep_time.tv_sec = usecs / 1000000;
  sleep_time.tv_usec = usecs % 1000000;
  
  memset ((void *)  &read_value,
          (int) 0, (size_t)  (sizeof(fd_set)));
  
  select(30, FD_SET_P &read_value, FD_SET_P 0,
         FD_SET_P 0, &sleep_time);
  
}

/*******************************************************************
  creating the file                                                 */

int fopcreate(char *filename, int size, int lock)
{
  unsigned long id[6];
  char buf[4096];
  int fd,ret;
  int i,len,cnt;
  
  if(size<32) {
    fprintf(stderr, "WARNING - fopcreate\n");
    fprintf(stderr, "size is too small (%d)\n",size);
    return (-101);
  }
  
  /* open the file */
  if((fd=open(filename, O_RDWR, 0))>=0){
    /* check consistancy */
    if(lseek(fd,0,SEEK_SET)<0 ||
       read(fd,id,6*sizeof(long))<6*sizeof(long) ||
       id[0]!=324578594){
      fprintf(stderr, "WARNING - fopcreate\n");
      fprintf(stderr, "File %s exists, but it is not a pipe\n",
	      filename);
      close(fd);
      return (-102);
    }
    if(size!=id[1]){
      fprintf(stderr, "WARNING - fopcreate\n");
      fprintf(stderr, "File %s exists, but size=%ld\n",
	      filename, id[1]);
      close(fd);
      return (-103);
    }
    close(fd);
    return(0);
  }
  if((fd=open(filename, O_RDWR | O_CREAT, 0666))<0){
    fprintf(stderr, "ERROR - fopcreate - Failed in creating %s\n",filename);
    return(-104);
  }
  
  if(set_lock(1,fd)<0) return (-105);
  
  /* write zeros to the file */
  for(i=0;i<4096;i++) buf[i]=0;
  cnt=0;
  while(1){
    if(cnt>=size) break;
    i=size-cnt;
    if(i>4096) i=4096;
    len=write(fd,buf,i);
    if(len<0) {
      fprintf(stderr, "ERROR - fopcreate - writing buffer failed\n");
      ret=-106;
      goto next;
    }
    cnt += len;
  }
  
  /* initialize the header */
  id[0]=324578594;
  id[1]=size;
  id[2]=id[3]=id[5]=0;
  sig_block(1);
  if(lseek(fd,0,SEEK_SET)<0 ||
     write(fd,id,6*sizeof(long))<6*sizeof(long)){
    sig_block(0);
    fprintf(stderr, "ERROR - fopcreate - writing id failed\n");
    ret=-107;
    goto next;
  }
  sig_block(0);
  ret=0;
  
 next:
  if(set_lock(0,fd)<0) return (-105);
  close(fd);
  return(ret);
  
}

/*******************************************************************
  opens the file and initialize Fobjest_pipe.                       */

int fopopen(char *filename)
{

  Fobject_pipe *obj;
  long tt[6],i;

  for(i=0;i<16;i++) if(Obj_pt[i]==0) break;
  if(i>=16){
    fprintf(stderr, "ERROR - fopopen\n");
    fprintf(stderr, "Too many file pipes opened (max=16).\n");
    return (-108);
  }
  
  /* the space for the handle */
  obj = (Fobject_pipe *) malloc(sizeof(Fobject_pipe));

  if(obj==NULL){
    fprintf(stderr, "ERROR - fopopen\n");
    fprintf(stderr, "Failed in allocating space for handle\n");
    return (-109);
  }
  obj->h_size=6*sizeof(long);
  obj->r_off=2*sizeof(long);
  obj->w_off=3*sizeof(long);
  
  /* open the file */
  if((obj->fd=open(filename, O_RDWR, 0))<0){
    fprintf(stderr, "ERROR - fopopen\n");
    fprintf(stderr, "File %s does not exist? - failed in opening\n",
	    filename);
    return(-110);
  }
  
  if(set_lock(1,obj->fd)<0) return (-105);
  
  /* the id and buffer size */
  if(read(obj->fd,tt,6*sizeof(long))<6*sizeof(long)) {
    fprintf(stderr, "ERROR - fopopen\n");
    fprintf(stderr, "Reading file header failed\n");
    if(set_lock(0,obj->fd)<0) return (-105);
    close(obj->fd);
    return (-111);
  }
  if(set_lock(0,obj->fd)<0) return (-105);
  
  if(tt[0]!=324578594){
    fprintf(stderr, "ERROR - fopopen\n");
    fprintf(stderr, "File %s is not created by fopcreate?\n",
	    filename);
    close(obj->fd);
    return (-111);
  }
  obj->b_size=tt[1]-7*sizeof(long);
  Obj_pt[i]=(int)obj;
  
  return (i+144);
}

/*******************************************************************
  return length if success, 0 if no data,
  -1 if error.                                                      */

int fopread(int ob, char *ray, int length)
{
  long rpt,wpt,rpg,wpg,tmp[2],len;
  int ret;
  Fobject_pipe *obj;
  int pt;
  
  if((pt=get_pointer(ob))==-1) return (-112);
  
  obj=(Fobject_pipe *)pt;
  
  if(set_lock(1,obj->fd)<0) return (-105);
  
  /* get the pointers */
  if(lseek(obj->fd,obj->r_off,SEEK_SET)<0 ||
     read(obj->fd,tmp,2*sizeof(long))<2*sizeof(long)){
    fprintf(stderr, "ERROR - fopread\n");
    fprintf(stderr, "Reading pointers failed\n");
    fopflush(ob);
    ret=-113;
    goto next;
  }
  
  rpt=tmp[0];
  wpt=tmp[1];
  
  rpg=rpt>>24;
  rpt=rpt&0xffffff;
  wpg=wpt>>24;
  wpt=wpt&0xffffff;
  
  if(rpg==wpg && rpt>=wpt) {
    ret=0;
    goto next;  /* data not available */
  }
  
  /* get length */
  if(lseek(obj->fd,obj->h_size+rpt,SEEK_SET)<0 ||
     read(obj->fd,&len,sizeof(long))<sizeof(long)){
    fprintf(stderr, "ERROR - fopread\n");
    fprintf(stderr, "Reading length failed\n");
    fopflush(ob);
    ret=-113;
    goto next;
  }
  
  if(len==0){
    rpt=0; 
    rpg=(rpg+1)%2;
    if(lseek(obj->fd,obj->h_size+rpt,SEEK_SET)<0 ||
       read(obj->fd,&len,sizeof(long))<sizeof(long)){
      fprintf(stderr, "ERROR - fopread\n");
      fprintf(stderr, "Reading length failed 1\n");
      fopflush(ob);
      ret=-113;
      goto next;
    }
    if(rpg==wpg && rpt>=wpt) {
      ret=0;
      goto next;/* data not available */
    }
  }
  
  if(len<4 || len+rpt>obj->b_size || (len+rpt>wpt && rpg==wpg)){
    fprintf(stderr, "ERROR - fopread\n");
    fprintf(stderr, "Fatal data error:\n");
    fprintf(stderr, "%ld %ld %ld %ld %ld\n",len,rpg,wpg,rpt,wpt);
    fopflush(ob);
    ret=-117;
    goto next;
  }
  
  len -= sizeof(long);
  if(len>length){
    if (Msg_on) {
      fprintf(stderr, "WARNING - fopread\n");
      fprintf(stderr, "The buffer is too small (%ld %d)\n",
	      len,length);
      fprintf(stderr, "Realloc read buffer to %ld bytes\n", len);
    }
    Length_needed[ob] = len;
    ret=-114;
    goto next;
  }    
  
  /* read data */
  if(lseek(obj->fd,obj->h_size+rpt+sizeof(long),SEEK_SET)<0 ||
     read(obj->fd,ray,len)<len){
    fprintf(stderr, "ERROR - fopread\n");
    fprintf(stderr, "Reading data failed\n");
    fopflush(ob);
    ret=-113;
    goto next;
  }
  
  rpt=rpt+len+sizeof(long);
  
  /* update the pointer */
  rpt=rpt+(rpg<<24);
  sig_block(1);
  if(lseek(obj->fd,obj->r_off,SEEK_SET)<0 ||
     write(obj->fd,&rpt,sizeof(long))<sizeof(long)){
    sig_block(0);
    fprintf(stderr, "ERROR - fopread\n");
    fprintf(stderr, "Writing pointer failed\n");
    fopflush(ob);
    ret=-115;
    goto next;
  }
  sig_block(0);
  ret=len;
  
 next:
  
  if(set_lock(0,obj->fd)<0) return (-105);
  return (ret);
  
}

/********************************************************************
 * returns length needed to prevent failure on read. Length_needed array
 * is loaded after read failure due to short read buffer. The idea
 * is that the client program will realloc the read buffer and
 * try again
 */

int foplen_needed(int ob)

{
  return Length_needed[ob];
}

/*******************************************************************
  discard data in the buffer                                        */

int fopflush(int ob)
{
  Fobject_pipe *obj;
  int pt;
  unsigned long id[2];
  
  if((pt=get_pointer(ob))==-1) return (-112);
  
  obj=(Fobject_pipe *)pt;
  
  if(set_lock(1,obj->fd)<0) return (-105);
  
  id[0]=id[1]=0;
  sig_block(1);
  if(lseek(obj->fd,obj->r_off,SEEK_SET)<0 ||
     write(obj->fd,id,2*sizeof(long))<2*sizeof(long)){
    sig_block(0);
    if(Msg_on==1) printf("resetting control failed\n");
    if(set_lock(0,obj->fd)<0) return (-105);
    return(-115);
  }
  sig_block(0);
  if(set_lock(0,obj->fd)<0) return (-105);
  return (0);
  
}

/*******************************************************************
  put a ray to the output shm.                     	          */

int fopwrite(int ob, char *ray,int length)
{
  long i,leng;
  long rpt,rpg,wpto,wpt,wpg,tmp[2];
  unsigned long tail;
  int ret;
  Fobject_pipe *obj;
  int pt;
  
  if((pt=get_pointer(ob))==-1) return (-112);
  
  obj=(Fobject_pipe *)pt;
  
  leng=length+sizeof(long);
  leng=((leng+3)>>2)<<2;
  
  if(length<=0 || leng>obj->b_size) 
    return (-116);
  
  if(set_lock(1,obj->fd)<0) return (-105);
  
  /* get the pointers */
  if(lseek(obj->fd,obj->r_off,SEEK_SET)<0 ||
     read(obj->fd,tmp,2*sizeof(long))<2*sizeof(long)){
    fprintf(stderr, "ERROR - fopwrite\n");
    fprintf(stderr, "Reading pointers failed\n");
    fopflush(ob);
    ret=-113;
    goto next;
  }
  
  rpt=tmp[0];
  wpt=tmp[1];
  /* used for testing file lock: sleep(1); */
  
  /* is the buffer full? */
  rpg=rpt>>24;
  wpg=wpt>>24;
  rpt=rpt&0xffffff;
  wpt=wpt&0xffffff;
  if(rpg!=wpg && wpt+leng>rpt){
    ret=0; /* full */
    goto next;
  }
  
  /* do we need fold back? */
  i=0;
  while(wpt+leng>obj->b_size){
    if(i>=1){
      fprintf(stderr, "ERROR - fopwrite\n");
      fprintf(stderr, "Error: leng, size, wpt= %ld %ld %ld\n",
	      leng,obj->b_size,wpt);
      fopflush(ob);
      ret=-117;
      goto next;
    }
    wpto=wpt;
    wpt=0;
    wpg=(wpg+1)%2;
    i++;
    if(rpg!=wpg && wpt+leng>rpt){
      if(wpto!=rpt || leng>obj->b_size){
	/* if there is no lock,
	   The size may need to be as twice as the max msg 
	   size. However, if there is lock we may move the
	   read point if the buffer is empty */
	ret=0; 
	goto next;
      }    
      /* the cure by moving the read pointer */
      rpt=0;
      rpg=wpg;
      rpt=(rpg<<24)+rpt;
      if(lseek(obj->fd,obj->r_off,SEEK_SET)<0 ||
	 write(obj->fd,&rpt,sizeof(long))<sizeof(long)){
	fprintf(stderr, "ERROR - fopwrite\n");
	fprintf(stderr, "Writing pointer failed 3\n");
	fopflush(ob);
	ret=-115;
	goto next;
      }
      
    }
    tail=0;
    if(lseek(obj->fd,obj->h_size+wpto,SEEK_SET)<0 ||
       write(obj->fd,&tail,sizeof(long))<sizeof(long)){
      fprintf(stderr, "ERROR - fopwrite\n");
      fprintf(stderr, "Writing tail failed\n");
      fopflush(ob);
      ret=-115;
      goto next;
    }
  }
  
  /* write the ray to the file */
  if(lseek(obj->fd,obj->h_size+wpt,SEEK_SET)<0 ||
     write(obj->fd,&leng,sizeof(long))<sizeof(long) ||
     write(obj->fd,ray,length)<length){
    fprintf(stderr, "ERROR - fopwrite\n");
    fprintf(stderr, "Writing data failed\n");
    fopflush(ob);
    ret=-115;
    goto next;
  }
  
  /* update the pointer */
  wpt +=leng;
  wpt=wpt+(wpg<<24);
  sig_block(1);
  if(lseek(obj->fd,obj->w_off,SEEK_SET)<0 ||
     write(obj->fd,&wpt,sizeof(long))<sizeof(long)){
    sig_block(0);
    fprintf(stderr, "ERROR - fopwrite\n");
    fprintf(stderr, "Writing pointer failed\n");
    fopflush(ob);
    ret=-115;
    goto next;
  }
  sig_block(0);
  ret=1;
  
 next:
  if(set_lock(0,obj->fd)<0) return (-105);
  return(ret);
  
}

/*******************************************************************
  close the object                                                 */

int fopclose(int ob)
{
  Fobject_pipe *obj;
  int pt;
  
  if((pt=get_pointer(ob))==-1) return (-112);
  
  obj=(Fobject_pipe *)pt;
  
  close(obj->fd);
  free(obj);
  Obj_pt[ob-144]=0;
  return (0);
}

/*******************************************************************
  get pointer for a pipe. Returns the pointer or -1 if failed       */

static int get_pointer(int ob)
{
  
  if(ob-144<0 || ob-144>=16 || Obj_pt[ob-144]==0) {
    if(Msg_on==1)
      printf("Bad pipe descriptor (%d, 144-159 only)\n",ob);
    return (-1);
  }
  
  return (Obj_pt[ob-144]);
}

/*******************************************************************
  set(sw=1) and reset(sw=0) file lock                               */

#include <errno.h>

static int set_lock(int sw, int fd)
{
  static struct flock fl;
  static int init=0;
  int err;
  
  if(init==0){
    init=1;
    fl.l_whence=SEEK_SET;
    fl.l_start=0;
    fl.l_len=0;
  }
  
  if(sw==1) fl.l_type=F_WRLCK;
  else fl.l_type=F_UNLCK;
  
  while((err=fcntl(fd,F_SETLKW,&fl))==-1 && errno==EINTR) {
    micro_sleep(50000);
  }
  
  if(err==-1 && Msg_on==1) 
    printf("Locking file failed. errno=%d\n",errno);
  
  return (err);
}


/*******************************************************************
  block(sw=1) and unblock(sw=0) termination signals                 */

#include <errno.h>
#include <signal.h>

static int sig_block(int sw)
{

  static sigset_t oldmask, newmask;

  if(sw==1) {

    /*
     * suspend signals
     */
    
    sigfillset(&newmask);

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
      fprintf(stderr, "SIG_BLOCK error\n");
    }

  } else {

    /*
     * activate signals
     */
    
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
      fprintf(stderr, "SIG_SETMASK error\n");
    }

  }

  return (0);
}

/******************************************************************
  switch on the message                                            */

void set_fobpipe_msg(int sw)
{
  
  Msg_on=sw;
  return;
}



