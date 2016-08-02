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
	routines reading a ray from the shared memory
	Jan. 29, 1992

	length=0  buffer end;

*******************************************************************/

#include "tdwr2moments.h"

static int get_a__rays(long *r_pt,
                       long *w_pt,
                       unsigned char *dbuf,
                       unsigned char **ray);

/*******************************************************************
get shm spaces for input. Initialize communication.             
get a data segment from shared memory. return length if success,
0 if no data, -1 if error. 
Note that the returned length may be actually bigger
then the true data length due to alignment in data store.
Because We return the pointer, 
we must reset r_pt later after we finish using the data. Here we 
reset last read pointer before current read                       */


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int recv_cmd (int sw, int key, unsigned char **ray)
     /* sw=0: rm shm; 1: get shm segment; 2: read a data segm */

{
static long *r_pt,*w_pt;
static unsigned char *dbuf;
static int shmid;
int size;
char *cpt;
struct shmid_ds buf;

    if(sw==2) return (get_a__rays (r_pt, w_pt, dbuf, ray));

    if(sw==0){
		/* check if others attached */
		shmctl (shmid, IPC_STAT, &buf);
		if (buf.shm_nattch > 1){
	   		printf ("There are other %d shm users - not removed.\n",
			buf.shm_nattch-1);
	   		return(0);
		}
		shmctl (shmid,IPC_RMID,&buf);
		return (0);
    }

    /* the size */
    size = (key % 10000) * 1024;
    if (size<2000) return(1); /* too small to hold rays */

    /* create the shared memory */
    if ((shmid = shmget ((key_t) key, size, 0666 | IPC_CREAT))<0){
		printf ("Failed in shmid - recv_cmd\n");
		return (1);
    }

    /* attach the shared memory regions */
    if((cpt = (char *) shmat (shmid, 0, 0)) < 0){
		printf ("Failed in shmat _ recv_cmd\n");
		return (1);
    }

    /* check how many attached */
    shmctl (shmid, IPC_STAT, &buf);
    if (buf.shm_nattch > 2){
  		printf("shared memory key %d in use - \n", key);
		printf("Replace %d with %d in mpar.params\n", key, key + 1);
		return (-1);
    }

    r_pt = (long *)cpt;
    cpt += sizeof(long);
    w_pt = (long *)cpt;
    cpt += sizeof(long);
    dbuf = (unsigned char *)cpt;

    /* make sure reset */
    if(*w_pt == -34009265){ /* writer is running and waiting */
		*r_pt = 0;
		*w_pt = 0;
    }
    else *r_pt =- 34009265;

return (0);
}


/*******************************************************************
get a ray from shared memory. return length if success, 0 if no data,
-1 if error. Because We return the pointer, 
we must reset r_pt later after we finish using the data. Here we 
reset last read pointer before current read                        */

static int get_a__rays(long *r_pt,                              
                       long *w_pt,
                       unsigned char *dbuf,
                       unsigned char **ray)

{
int len;
int rpt,wpt,rpg,wpg;
static long rrpt=-1,*rr_pt;

if(rrpt>=0) *rr_pt=rrpt;
rrpt=-1;

rpt=*r_pt;
wpt=*w_pt;
if(wpt==-34009265){   /* writer is just started */
	*r_pt=0;
	*w_pt=0;
	rpt=0;
	wpt=0;
}

if(rpt<0) return (0); /* writer not running */

rpg=rpt>>24;
rpt=rpt&0xffffff;
wpg=wpt>>24;
wpt=wpt&0xffffff;

if(rpg==wpg && rpt>=wpt) return (0); /* data not available */

len=((unsigned long *)(&dbuf[rpt]))[0];
					
if(len==0){
	rpt=0; 
	rpg=(rpg+1)%2;
	len=((unsigned long *)(&dbuf[rpt]))[0];
	if(rpg==wpg && rpt>=wpt) return (0); /* data not available */
}

if(len<4 || len>10000 || (len+rpt>wpt && rpg==wpg)){
	printf("Fatal shared memory data error.\n");
	printf("%d %d %d %d %d\n",len,rpg,wpg,rpt,wpt);
	return (-1);
}

*ray=&dbuf[rpt+sizeof(long)];
rpt+=len;

rrpt=rpt+(rpg<<24);
rr_pt=r_pt;

return (len-sizeof(long));

}
