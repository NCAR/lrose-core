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
	Shared memory version
	Nov. 16, 1992

*******************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <rdi/obpipe.h>

/* we store the object handle locally. This has the following
   advantages: Better handle check (closed handle does not cause
   memory error; possibility of being differentiated from regular
   file fd; Possible to add a commu. fd number in higher byte, which
   is required by the remote software */
/* fd+128 for file pipe, at most 16 pipes opened */
static int obj_pt[16] =
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static int msg_on = 0;

static int get_pointer (int ob);

/*******************************************************************
get spaces and Initialize communication.                          */

int 
opopen (int type, int key, int size)
{
    char *cpt;
    struct shmid_ds buf;
    Object_pipe *obj;
    long *ow_pt, *op_pt;
    int i;

    for (i = 0; i < 16; i++)
	if (obj_pt[i] == 0)
	    break;
    if (i >= 16) {
	if (msg_on == 1)
	    fprintf (stderr, "Too many shm pipes opened (max=16).\n");
	return (-1);
    }

    if (size < 32)
	return (-1);

    /* the space for the handle */
    if ((obj = (Object_pipe *) malloc (sizeof (Object_pipe))) == NULL) {
	if (msg_on == 1)
	    printf ("Failed in allocating space for handle\n");
	return (-1);
    }
    obj->shm_key = key;
    obj->type = type;

    /* create the shared memory */
    if ((obj->shmid = shmget ((key_t) key, size, 0666 | IPC_CREAT)) < 0) {
	if (msg_on == 1)
	    printf ("Failed in shmget\n");
	return (-1);
    }

    /* attach the shared memory regions */
    cpt = (char *) shmat (obj->shmid, 0, 0);
    if ((int) cpt == -1) {
	if (msg_on == 1)
	    printf ("Failed in shmat\n");
	return (-1);
    }

    /* check how many attached */
    shmctl (obj->shmid, IPC_STAT, &buf);
    if (buf.shm_nattch > 2) {
	if (msg_on == 1)
	  fprintf (stderr, "Too many (%d) connections to shm\n",
		   buf.shm_nattch);
	return (-1);
    }

    obj->r_pt = (long *) cpt;
    cpt += sizeof (long);
    obj->w_pt = (long *) cpt;
    cpt += sizeof (long);
    obj->dbuf = (char *) cpt;
    obj->b_size = size - 3 * sizeof (long);
    /* an extra long is needed at the end */
    obj->rrpt = -1;
    obj->pointer_returned = 0;
    obj->not_updated = 0;

    /* initialize */
    obj->init = 0;
    if ((type & 1) == 0) {	/* read */
	ow_pt = obj->r_pt;
	op_pt = obj->w_pt;
    }
    else {
	ow_pt = obj->w_pt;
	op_pt = obj->r_pt;
    }

    if (*op_pt == -34009265) {	/* the other party is not active */
	*ow_pt = 0;
	*op_pt = 0;
    }
    else
	*ow_pt = -34009265;

    obj_pt[i] = (int) obj;

    return (i + 128);
}


/*******************************************************************
return length if success, 0 if no data,
-1 if error. Because We return the pointer, 
we must reset r_pt later after we finish using the data. Here we 
reset last read pointer before current read                        */

int 
opread (int ob, char **ray)
{
    int len;
    long rpt, wpt, rpg, wpg;
    Object_pipe *obj;
    int pt;

    if ((pt = get_pointer (ob)) == -1)
	return (-1);

    obj = (Object_pipe *) pt;

    if ((obj->type & 1) != 0)
	return (-1);

    if (obj->rrpt >= 0)
	*obj->rr_pt = obj->rrpt;
    obj->rrpt = -1;

    if (*obj->w_pt == -34009265) {	/* writer is just started */
	*obj->r_pt = 0;
	*obj->w_pt = 0;
    }
    rpt = *obj->r_pt;
    if (rpt < 0)
	return (0);		/* self locked */
    wpt = *obj->w_pt;

    rpg = rpt >> 24;
    rpt = rpt & 0xffffff;
    wpg = wpt >> 24;
    wpt = wpt & 0xffffff;

    if (rpg == wpg && rpt >= wpt)
	return (0);		/* data not available */

    len = ((unsigned long *) (&obj->dbuf[rpt]))[0];

    if (len == 0) {
	rpt = 0;
	rpg = (rpg + 1) % 2;
	len = ((unsigned long *) (&obj->dbuf[rpt]))[0];
	if (rpg == wpg && rpt >= wpt)
	    return (0);		/* data not available */
    }

    if (len < 4 || len + rpt > obj->b_size || (len + rpt > wpt && rpg == wpg)) {
	if (msg_on == 1)
	  fprintf (stderr, "Fatal shared memory data error.\n");
	if (msg_on == 1)
	  fprintf (stderr, "%d %ld %ld %ld %ld\n",
		   len, rpg, wpg, rpt, wpt);
	return (-1);
    }

    *ray = &(obj->dbuf[rpt + sizeof (long)]);
    rpt += len;

    obj->rrpt = rpt + (rpg << 24);
    obj->rr_pt = obj->r_pt;

    return (len - sizeof (long));

}


/*******************************************************************
put a ray to the output shm.                     	          */

int 
opwrite (int sw, int ob, char *ray, int length)
{
    int i, leng;
    int rpt, rpg, wpto, wpt, wpg;
    unsigned long *lpt, tail;
    Object_pipe *obj;
    int pt;

    if ((pt = get_pointer (ob)) == -1)
	return (-1);

    obj = (Object_pipe *) pt;

    leng = length + sizeof (long);
    leng = ((leng + 3) >> 2) << 2;

    if (sw < 2 || sw > 6 || (sw != 4 && length <= 0) || (obj->type & 1) != 1 ||
	leng > obj->b_size)
	return (-1);

    if (obj->pointer_returned == 1) {
	if (sw == 6) {
	    obj->pointer_returned = 0;
	    wpt = obj->wpt5;
	    wpg = obj->wpg5;
	    goto finish;
	}
	else
	    return (-1);
    }
    if (sw == 6)
	return (-1);

    if (obj->not_updated == 1) {
	if (sw == 2 || sw == 4) {
	    *obj->w_pt = obj->ww_pt;
	    obj->not_updated = 0;
	}
	else if (sw >= 5)
	    return (-1);
    }
    if (sw == 4)
	return (1);

    if (*obj->r_pt == -34009265) {	/* reader just started */
	*obj->w_pt = 0;
	*obj->r_pt = 0;
	obj->not_updated = 0;
    }
    if (*obj->w_pt == -34009265)
	return (0);		/* self_locked */

    rpt = *obj->r_pt;
    if (obj->not_updated == 0)
	wpt = *obj->w_pt;
    else
	wpt = obj->ww_pt;

    /* is the buffer full? */
    rpg = rpt >> 24;
    wpg = wpt >> 24;
    rpt = rpt & 0xffffff;
    wpt = wpt & 0xffffff;
    if (rpg != wpg && wpt + leng > rpt)
	return (0);		/* full */

    /* do we need fold back? */
    i = 0;
    while (wpt + leng > obj->b_size) {
	if (i >= 1) {
	    if (msg_on == 1)
	      fprintf (stderr, "Error: leng, size, wpt= %d %ld %d\n",
		       leng, obj->b_size, wpt);
	    return (-1);
	}
	wpto = wpt;
	wpt = 0;
	wpg = (wpg + 1) % 2;
	i++;
	if (rpg != wpg && wpt + leng > rpt)
	    return (0);		/* full */
	tail = 0;
	memcpy (&obj->dbuf[wpto], &tail, sizeof (long));
    }

    if (sw == 5) {		/* we return the pointer */
	obj->pointer_returned = 1;
	obj->wpt5 = wpt;
	obj->wpg5 = wpg;
	return ((int) &obj->dbuf[wpt + sizeof (long)]);
    }

  finish:
    /* write the ray to the shm */
    wpto = wpt;
    lpt = (unsigned long *) &obj->dbuf[wpt];
    *lpt = leng;
    if (sw <= 4)
	memcpy (&obj->dbuf[wpt + sizeof (long)], ray, length);

    wpt += leng;
    wpt = wpt + (wpg << 24);
    if (sw == 2 || sw == 6)
	*obj->w_pt = wpt;
    if (sw == 3) {
	obj->ww_pt = wpt;
	obj->not_updated = 1;
    }

    if (sw == 3)
	return ((int) &obj->dbuf[wpto + sizeof (long)]);
    return (1);

}

/*******************************************************************
close the object                                                 */

int 
opclose (int ob)
{
    struct shmid_ds buf;
    Object_pipe *obj;
    int pt;

    if ((pt = get_pointer (ob)) == -1)
	return (-1);

    obj = (Object_pipe *) pt;

    /* check if others attached */
    shmctl (obj->shmid, IPC_STAT, &buf);
    if (buf.shm_nattch <= 1)
	shmctl (obj->shmid, IPC_RMID, &buf);
    free (obj);
    obj_pt[ob - 128] = 0;
    return (0);
}

/*******************************************************************
get pointer for a pipe. Returns the pointer or -1 if failed       */

static int 
get_pointer (int ob)
{

    if (ob - 128 < 0 || ob - 128 >= 16 || obj_pt[ob - 128] == 0) {
	if (msg_on == 1)
	    printf ("Bad pipe descriptor (%d, 128-143 only)\n", ob);
	return (-1);
    }

    return (obj_pt[ob - 128]);
}


/*******************************************************************
for compatibility                                                 */

int 
recv_shm (int sw, int key, char **ray)
	/* sw=0: rm shm; 1: get shm segment; 2: read a data segm */
{
    int size;
    static int op;

    if (sw == 2)
	return (opread (op, ray));
    if (sw == 0) {
	opclose (op);
	return (0);
    }

    /* the size */
    size = (key % 10000) * 1024;
    if (size < 128)
	return (1);		/* too small to hold rays */

    if ((op = opopen (2, key, size)) == -1)
	return (-1);
    return (0);
}

/********************************************************************
for compatibility                                                  */

int 
send_shm (int sw, int shm_key, char *ray)
{
    int sz;
    static int op;

    if (sw >= 2) {
	sz = opwrite (sw, op, ray, shm_key);
	if (sz == -1)
	    return (-1);
	if (sz == 0)
	    return (1);
	if (sw == 5 || sw == 3)
	    return (sz);
	else
	    return (0);
    }

    if (sw == 0) {
	opclose (op);
	return (0);
    }

    /* the size */
    sz = (shm_key % 10000) * 1024;
    if (sz < 128)
	return (1);		/* exclude too small size */
    if ((op = opopen (3, shm_key, sz)) == -1)
	return (1);
    return (0);
}


/******************************************************************
switch on the message                                            */

void 
set_obpipe_msg (int sw)
{

    msg_on = sw;
    return;
}
