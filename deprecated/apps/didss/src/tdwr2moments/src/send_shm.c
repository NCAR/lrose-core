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
	routines writing a ray to the shared memory
	Jan. 29, 1992

	length=0  buffer end;

sw=0: rm shm; 
   1: get shm segment; 
   2: write a data segment (ray).
   3: write without updating w_pt 
   4: update write pointer
   5: get a pointor to write (reture casted to int)
   6: finish writing (must call after sw=5 call ?).

In writing call, key used for data length in bytes.

ray: The length in control the store is processed in send_shm
     and recv_shm. In ray data length is arbitrary.


if client is not running return 1.
if buffer full, return 3. if client just started, return 2. 
fatal error - return -1. On success, return the pointer in shm.
Note: if return <=3, that ray is not written	          


/******************************************************************/

#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>

static int get__ray(int sw, long *r_pt, long *w_pt,
                    unsigned char *dbuf, unsigned char *ray,
                    long b_size, int length);


/*******************************************************************
open shared memory                                                */

int send_shm(int sw, int shm_key, unsigned char *ray)
{
static long *r_pt,*w_pt,b_size;
static unsigned char *dbuf;
static int shmid;
int sz;
unsigned char *cpt;
struct shmid_ds buf;

if(sw>=2) return (get__ray(sw,r_pt,w_pt,dbuf,ray,b_size,shm_key));

if(sw==0){
	/* check if others attached */
	shmctl (shmid, IPC_STAT, &buf);
	if(buf.shm_nattch>1){
	    printf("There are other %d shm users - not removed.\n",
		buf.shm_nattch-1);
	    return(0);
	}
	shmctl(shmid,IPC_RMID,&buf);
	return (0);
}

/* the size */
sz=(shm_key%10000)*1024;
if(sz<2000) return(1); /* exclude too small size */

/* create the shared memory */
if((shmid=shmget((key_t)shm_key,sz,0666|IPC_CREAT))<0){
	printf("Failed in shmid - send_shm\n");
	exit;
}

/* attach the shared memory regions */
if((int)(cpt=(unsigned char *)shmat(shmid,0,0))==-1){
	printf("Failed in shmat _ send_shm\n");
	exit;
}

/* check how many attached */
shmctl (shmid, IPC_STAT, &buf);
if(buf.shm_nattch>2){
	printf("shared memory key %d in use - \n", shm_key);
	printf("Replace %d with %d in mpar.params\n", shm_key, shm_key + 1);
	return(-1);
}

r_pt=(long *)cpt;
cpt+=sizeof(long);
w_pt=(long *)cpt;
cpt+=sizeof(long);
dbuf=(unsigned char *)cpt;

/* make sure reset */
if(*r_pt==-34009265){ /* client is running and waiting */
	*w_pt=0;
	*r_pt=0;
}
else *w_pt= -34009265;

b_size=sz-2*sizeof(long)-sizeof(long);

return (0);

}

/*******************************************************************
put a ray to the output shm.                     	          */

static int get__ray(int sw, long *r_pt, long *w_pt,
                    unsigned char *dbuf, unsigned char *ray,
                    long b_size, int length)
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
        *w_pt=ww_pt;
        not_updated=0;
    }
}
if(sw==4)return (0);

if(*r_pt== -34009265){ /* client just started */
	*w_pt=0;
	*r_pt=0;
	not_updated=0;
	return(2);
}
if(*w_pt==-34009265){ /* self locked */
	return(1);
}

    /* if the client starts */
    rpt= *r_pt;
    if(rpt<0) return(1);
    if(not_updated==0) wpt= *w_pt;
    else wpt=ww_pt;

    /* is the buffer full? */
    rpg=rpt>>24;
    wpg=wpt>>24;
    rpt=rpt&0xffffff;
    wpt=wpt&0xffffff;
    if(rpg!=wpg && wpt+leng>=rpt) return (3); /* full */

    /* do we need fold back? */
    i=0;
    while(wpt+leng>b_size){
	if(i>=1){
	    printf("Error: leng, size, wpt= %d %d %d\n",
			leng,b_size,wpt);
	    return(-1);
	}
	wpto=wpt;
	wpt=0;
	wpg=(wpg+1)%2;
	i++;
        if(rpg!=wpg && wpt+leng>=rpt) return (3); /* full */
	tail=0;
	memcpy(&dbuf[wpto],&tail,sizeof(long));
    }

    /* write the ray to the shm */
    if(sw==5){ /* we return the pointor */
	pointor_returned=1;
	return ((int)&dbuf[wpt+sizeof(long)]);
    }

finish:
    wpto=wpt;
    lpt=(unsigned long *)&dbuf[wpt];
    *lpt=leng;
    if(sw<=4) memcpy(&dbuf[wpt+sizeof(long)],ray,length);

    wpt+=leng;
    wpt=wpt+(wpg<<24);
    if(sw==2 || sw==6) *w_pt=wpt;
    if(sw==3){
	ww_pt=wpt;
	not_updated=1;
    }

return((int)(&dbuf[wpto+sizeof(long)]));

}

