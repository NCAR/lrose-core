// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*******************************************************************
 * shmem_output.c
 *
 * Routines writing a ray to shared memory.
 *
 * From Jing code, Jan. 29, 1992
 */

#include "ridds2mom.h"
#include <sys/ipc.h>
#include <sys/shm.h>
using namespace std;

static int Shmem_key;
static int Shmid = -1;
static int Debug;
static long *R_pt,*W_pt,B_size;
static ui08 *Dbuf;

/********************************************************************
 * init_output_shmem()
 *
 * Initialize shared memory output.
 */

int init_output_shmem(int shmem_key, int debug)

{

  int sz;
  ui08 *cpt;
  struct shmid_ds buf;

  Debug = debug;
  Shmem_key = shmem_key;
  
  /* the size */
  sz = (Shmem_key%10000) * 1024;
  if (sz < 2000) {
    fprintf(stderr,
	    "init_output_shmem: shmem key %d, size %d too small\n",
	    Shmem_key, sz);
    return(-1); /* exclude too small size */
  }

  /* create the shared memory */
  if((Shmid=shmget((key_t)Shmem_key,sz,0666|IPC_CREAT))<0){
    fprintf(stderr,
	    "init_output_shmem: shmem key %d, failed in shmget\n",
	    Shmem_key);
    return(-1);
  }
  
  /* attach the shared memory regions */
  if((long)(cpt=(ui08 *)shmat(Shmid,0,0))==-1){
    fprintf(stderr,
	    "init_output_shmem: shmem key %d, failed in shmat\n",
	    Shmem_key);
    return(-1);
  }
  
  /* check how many attached */
  shmctl (Shmid, IPC_STAT, &buf);
  if(buf.shm_nattch>2){
    fprintf(stderr,
	    "init_output_shmem: shmem key %d, too many connections - %d\n",
	    Shmem_key, (int) buf.shm_nattch);
    return(-1);
  }
  
  R_pt=(long *)cpt;
  cpt+=sizeof(long);
  W_pt=(long *)cpt;
  cpt+=sizeof(long);
  Dbuf=(ui08 *)cpt;
  
  /* make sure reset */
  if(*R_pt==-34009265){ /* client is running and waiting */
    *W_pt=0;
    *R_pt=0;
  } else {
    *W_pt= -34009265;
  }
  
  B_size=sz-2*sizeof(long)-sizeof(long);

  return (0);

}

/*******************************************************************
 * put a ray to the output shm.                     	          
 *
 * switch codes:
 *
 *  sw=2: write a data segment (ray).
 *     3: write without updating w_pt 
 *     4: update write pointer
 *     5: get a pointer to write (return cast to int)
 *     6: finish writing (must call after sw =5 call ?).
 * 
 * return codes:
 *  1 - client not running
 *  2 - client just started
 *  3 - buffer full
 * -1 - fatal error
 *  on success, returns pointer in shmem
 *
 *******************************************************************/

int write_output_ll_shmem(int sw, ui08 *ray, int length)
     
{
  int i,leng;
  static int wpt,wpg,pointor_returned=0;
  int rpt,rpg,wpto;
  static int not_updated=0,ww_pt;
  unsigned long *lpt,tail;
  
  leng=length+sizeof(long);
  leng=((leng+3)>>2)<<2;
  
  if(pointor_returned==1){
    if(sw==6) {
      pointor_returned=0;
      goto finish;
    }
    else {
      printf("Writing not finished.\n");
      return(-1);
    }
  }
  if(sw==6) return(0);
  
  if(not_updated==1){
    if(sw==2 || sw==4) {
      *W_pt=ww_pt;
      not_updated=0;
    }
  }
  if(sw==4)return (0);
  
  if(*R_pt== -34009265){ /* client just started */
    *W_pt=0;
    *R_pt=0;
    not_updated=0;
    return(2);
  }
  if(*W_pt==-34009265){ /* self locked */
    return(1);
  }
  
  /* if the client starts */
  rpt= *R_pt;
  if(rpt<0) return(1);
  if(not_updated==0) wpt= *W_pt;
  else wpt=ww_pt;
  
  /* is the buffer full? */
  rpg=rpt>>24;
  wpg=wpt>>24;
  rpt=rpt&0xffffff;
  wpt=wpt&0xffffff;
  if(rpg!=wpg && wpt+leng>=rpt) return (3); /* full */
  
  /* do we need fold back? */
  i=0;
  while(wpt+leng>B_size){
    if(i>=1){
      printf("Error: leng, size, wpt= %d %d %d\n",
	     (int) leng, (int) B_size, (int) wpt);
      return(-1);
    }
    wpto=wpt;
    wpt=0;
    wpg=(wpg+1)%2;
    i++;
    if(rpg!=wpg && wpt+leng>=rpt) return (3); /* full */
    tail=0;
    memcpy(&Dbuf[wpto],&tail,sizeof(long));
  }
  
  /* write the ray to the shm */
  if(sw==5){ /* we return the pointor */
    pointor_returned=1;
    return ((long)&Dbuf[wpt+sizeof(long)]);
  }
  
finish:
  wpto=wpt;
  lpt=(unsigned long *)&Dbuf[wpt];
  *lpt=leng;
  if(sw<=4) memcpy(&Dbuf[wpt+sizeof(long)],ray,length);
  
  wpt+=leng;
  wpt=wpt+(wpg<<24);
  if(sw==2 || sw==6) *W_pt=wpt;
  if(sw==3){
    ww_pt=wpt;
    not_updated=1;
  }
  
  return((long)(&Dbuf[wpto+sizeof(long)]));
  
}

/*************************************************************
 * delete_output_shmem()
 *
 */

void delete_output_shmem(void)

{

  struct shmid_ds buf;
  
  if (Shmid < 0) {
    return;
  }

  /* check if others attached */
  shmctl (Shmid, IPC_STAT, &buf);
  
  if(buf.shm_nattch>1){
    if (Debug) {
      fprintf(stderr, "There are other %d shm users - not removed.\n",
	      (int) buf.shm_nattch-1);
    }
    return;
  }

  shmctl(Shmid,IPC_RMID,&buf);
  return;

}
