/*
 *      OFFICIAL NOTIFICATION: the following CONFIDENTIAL and PROPRIETARY 
 * 	property legend shall not be removed from this source code module 
 * 	for any reason.
 *
 *	This program is the CONFIDENTIAL and PROPRIETARY property 
 *	of FairCom(R) Corporation. Any unauthorized use, reproduction or
 *	transfer of this computer program is strictly prohibited.
 *
 *      Copyright (c) 1984 - 1997 FairCom Corporation.
 *	This is an unpublished work, and is subject to limited distribution and
 *	restricted disclosure only. ALL RIGHTS RESERVED.
 *
 *			RESTRICTED RIGHTS LEGEND
 *	Use, duplication, or disclosure by the Government is subject to
 *	restrictions set forth in subparagraph (c)(1)(ii) of the Rights in
 * 	Technical Data and Computer Software clause at DFARS 252.227-7013.
 *	FairCom Corporation, 4006 West Broadway, Columbia, MO 65203.
 *
 *	c-tree PLUS(tm)	Version 6.7
 *			Release A2
 *			August 1, 1997
 */

#ifndef ctSORTH
#define ctSORTH

#define MAX_HANDLES 5   /* this is the maximum number of file handles */
			/* the sort will try to allocate min=3 the more */
			/* the better.  if it can't get this many it */
			/* use what it can get (min 3) */
#define MAX_K_TO_USE 1000 /* max number of kilobytes the sort will try to */
			/* allocate.  the more the better.  if it can't */
			/* get this much it will use less */
#define MAX_INT_VALUE 65000 /*max value that can be held in an integer */
#define WORK_DRIVE_OR_PATH ""

/* Structure that maps global variables */
typedef struct globalv {
	LONG	outpos;		/* file position */
	LONG	wf_pos[MAX_HANDLES];/* file position */
	ppUTEXT ptr_area_p;	/* pointer to pointer area */
	pUTEXT 	out_buf;	/* ptr to output buffer */
	pUTEXT 	data_area[MAX_HANDLES];/* ptrs to data areas */
	pUTEXT 	in_buf;		/* address of current input buffer */
	pUTEXT 	m_ptr[MAX_HANDLES];/* pointers to cur rec in merge buf */
	UINT	last_rec_given;	/* last record given by givrec */
	NINT	last_buf;	/* merge buffer assignment */
	NINT	last_buf_moved;	/* last buffer moved from */
	UCOUNT	next_ptr;  	/* next pointer number to be used */
	UCOUNT	bytes_in_buf;	/* number of bytes in data buf */
	UCOUNT	cur_data_area;	/* number of current data area */
	UCOUNT	data_area_num;	/* number of data areas allocated */
	UCOUNT	data_area_size; /* data area size in bytes */
	UCOUNT	first_mergefile;/* number of first file to merge */
	UCOUNT	m_bytes_in_buf[MAX_HANDLES];/* num of bytes in each merge buf */
	UCOUNT	num_ptrs;  	/* number of pointers that can be held */
	UCOUNT	number_handles; /* number of handles available */
	UCOUNT	out_buf_cur_off;/* current offset into output buffer */
	UCOUNT	out_buf_size;	/* output buffer size in bytes */
	UCOUNT	recl;  		/* record length */
	UCOUNT	return_code;  	/* main return code */
	UCOUNT	wf_num;		/* num of current workfile (rel 1) */
	COUNT	outnum;
	COUNT   wfnum[MAX_HANDLES];	/* ctree fcb's */
	UTEXT 	wf_root[13];	/* init sets to "swtttttt.00 z"  */
	TEXT 	filename_hold[64];/* hold area for filename */
	UTEXT 	m_eof[MAX_HANDLES];/* eof indicators */
	UTEXT 	givrec_subsequent;/* indicates if subsequent givrec */
	UTEXT 	in_memory_sort;	  /* indicates if in_memory sort */
	UTEXT 	m_intermediate;	  /* intermediate merge indicator */
	UTEXT 	merge1_subsequent;/* indicates subsequent merge1 called */
	UTEXT	no_more_recs;  	/* indicates if all recs given */
	UTEXT 	sinit_done;	/* flag indicating if sinit done */
	UTEXT 	sreturn_started;/* indicates if return phase started */
	} SGLOBV;
typedef SGLOBV ctMEM *	pSGLOBV;

#ifdef PROTOTYPE
extern pSGLOBV ctsinit(NINT rec_size,NINT max_hndl);
extern NINT ctsadd(pSGLOBV gv,pVOID rec_area);
extern NINT ctsval(pSGLOBV gv,pVOID rec_area);
extern NINT ctsabort(pSGLOBV gv);
#else /* PROTOTYPE */
extern pSGLOBV ctsinit();
extern NINT ctsadd();
extern NINT ctsval();
extern NINT ctsabort();
#endif /* PROTOTYPE */

#endif /* ctSORTH */

/* end of ctsort.h */
