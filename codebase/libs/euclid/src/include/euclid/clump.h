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
/* clump.h - structures related to clumps */

#ifndef CLUMP_H
#define CLUMP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <sys/types.h>
#include <stdio.h>
#include <euclid/alloc.h>
#include <euclid/point.h>

/* defines */
#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#define NULL_ID 0	    /* clump ids should be initialized to this value */
#define OV_BEG_IN 0	    /* index for overlap array */
#define OV_END 1	    /* index for overlap array */

#define NORTH_INTERVAL 0    /* used for overlapping intervals */
#define SOUTH_INTERVAL 1    /* used for overlapping intervals */
#define UP_INTERVAL 2	    /* used for overlapping intervals */
#define DOWN_INTERVAL 3     /* used for overlapping intervals */

/* typedefs */

/* used for specifying 2d bounding box */
typedef struct box_2d
{
  int xmin;
  int ymin;
  int xmax;
  int ymax;
} Box_2d;

typedef struct box_2d box_2d_t;

/* used for specifying 3d bounding box */
typedef struct box_3d
{
  int xmin;
  int ymin;
  int zmin;
  int xmax;
  int ymax;
  int zmax;
} Box_3d;

typedef struct box_3d box_3d_t;

/* the interval structure is general enough to support 3d intervals */
typedef struct interval
{
  int id;               /* clump id */
  short overlaps[4][2]; /* overlaps[i][0] - index of begin overlap,
			 * overlaps[i][1] - index of end overlap */
  short plane;          /* plane index of interval */
  short row_in_vol;	/* row index of interval in volume -
			 * not row index in plane if there are
			 * multiple planes */
  short row_in_plane;   /* row in current plane. If only one
			 * plane, this is same as row_in_vol */
  short begin;          /* interval begins in this column */
  short end;            /* interval ends in this column */
  short len;            /* interval len = (end - begin + 1) */
} Interval;

/*
 * the interval link structure is used for creating linked lists of intervals
 */
typedef struct interval_link_hdr
{
  int size;			/* size of linked list */
  struct interval_link *link;	/* points to an interval link structure */
} Interval_link_hdr;

/*
 * the interval link structure is used for creating linked lists of intervals
 */
typedef struct interval_link
{
  Interval *iptr;		/* points to an interval */
  struct interval_link *link;	/* points to another interval link structure */
} Interval_link;

/*
 * short interval structure used for passing results to other machines
 * (see clump file below)
 */
typedef struct sinterval 
{
  int id;			/* clump id */
  short plane;			/* plane index of interval */
  short row;		    /* row index of interval in volume - not row index in plane if there are multiple planes */
  short begin;		    /* interval begins in this column */
  short end;		     /* interval ends in this column */
} Sinterval;

typedef struct row_hdr /* row header */
{
  int size;		  /* number of intervals in row */
  Interval *intervals;	  /* ptr to intervals in row */
} Row_hdr;

typedef struct srow_hdr /* row header for short intervals */
{
  int size;			/* number of intervals in row */
  Sinterval *intervals;	  /* ptr to intervals in row */
} Srow_hdr;

typedef struct clump_order /*clump order */
{
  int size;	           /* number of intervals in clump */
  int pts;		   /* number of points in the clump */
  Interval **ptr;     /* ptr to interval indices in clump */
} Clump_order;

typedef struct oclump_order /*clump order */
{
  int size;	        /* number of intervals in clump */
  int pts;		/* number of points in the clump */
  int xmin;		/* lower bound for x coord in clump */
  int xmax;		/* upper bound for x coord in clump */
  int ymin;		/* lower bound for y coord in clump */
  int ymax;		/* upper bound for y coord in clump */
  int zmin;		/* lower bound for z coord in clump */
  int zmax;		/* upper bound for z coord in clump */
  double mag;		/* user supplied magnitude for clump */
  Interval **ptr;	/* ptr to interval indices in clump */
} OClump_order;


/*
 * clump information structure
 *
 * Here are sizes of relevant arrays.  Such sizes should be used in
 * conjunction with cfree(array, size) for freeing space:
 *
 * volume - num_planes * num_rows * num_cols * sizeof(char)
 * intervals - isize * sizeof(Interval)
 * row_hdr - num_planes * num_rows * sizeof(Row_hdr)
 * interval_order - num_ints * sizeof(Interval *)
 * clump_order - num_clumps * sizeof(Clump_order)
 */
typedef struct clump_info
{
  char *volume;			/* pointer to volume information */
  int num_planes;		/* number of planes in volume */
  int num_rows;			/* number of rows in volume -
				 * NOT (number of rows in plane =
				 *      num_rows/num_planes) */
  int num_cols;			/* number of columns in volume */
  int threshold;		/* threshold used for clumping */
  int num_ints;			/* number of intervals in volume */
  int num_clumps;		/* number of clumps in volume */
  int isize;			/* size of intervals array */
  Interval *intervals;		/* array of intervals in volume */
  Row_hdr *row_hdr;		/* organizes intervals array */
  Interval **interval_order;	/* orders intervals according to clump */
  Clump_order *clump_order;	/* organizes interval_order array */
} Clump_info;

typedef struct oclump_info
{
  char *volume;			/* pointer to volume information */
  int num_planes;		/* number of planes in volume */
  int num_rows;			/* number of rows in volume -
				 * NOT (number of rows in plane =
				 *      num_rows/num_planes) */
  int num_cols;			/* number of columns in volume */
  int threshold;		/* threshold used for clumping */
  int num_ints;			/* number of intervals in volume */
  int num_clumps;		/* number of clumps in volume */
  int isize;			/* size of intervals array */
  Interval *intervals;		/* array of intervals in volume */
  Row_hdr *row_hdr;		/* organizes intervals array */
  Interval **interval_order;	/* orders intervals according to clump */
  OClump_order *clump_order;	/* organizes interval_order array */
} OClump_info;

/*
 * clump offset structure (See clump_order structure above.  This
 * structure is used for files
 */
typedef struct clump_offset
{
  int size;			/* number of intervals in clump */
  int pts;			/* number of points in clump */
  long int offset;	    /* offset of clump from beginning of file */
} Clump_offset;

/*
 * clump file header structure
 */
typedef struct clump_hdr
{
  int nx;			/* number of columns */
  int ny;			/* number of rows */
  int nz;			/* number of planes */
  int num_clumps;		/* number of clumps in the file */

  /*
   * this header is followed by:
   *
   * Clump_offsets clump_offsets[num_clumps];
   *
   * then by
   *
   * i=0, 1, ..., num_clumps-1 arrays of intervals each of size
   * clump_offsets[i].size
   *
   * Sinterval interval[num_clumps][clump_offsets[i].size]
   */
} Clump_hdr;


/* function declarations */

/*
 *  DESCRIPTION:    
 *
 *    Allocates or reallocates the row header array.
 *
 *  INPUTS:
 *
 *    int n_intervals - number of intervals to be catered for
 *
 *    int *n_intervals_alloc_p - pointer to N_intervals_alloc in
 *      calling routine, which should be static.
 *      Must be initialized to 0 before first call to alloc_clumps().
 *      Typical declaration in calling routine is:
 *        static int N_intervals_alloc = 0;
 *
 *    Clump_order **clumps_p - pointer to Clumps in calling routine which
 *      should be static and initialized to NULL.
 *      Typical declaration in calling routine is:
 *        static Clump_order *Clumps = NULL;
 *    
 *    Interval ***interval_order_p - pointer to Interval_order
 *      in calling routine which should be static and initialized to NULL.
 *      Typical declaration in calling routine is:
 *        static Interval **Interval_order = NULL;
 *    
 * OUTPUTS:
 *
 *    *n_intervals_alloc_p, *clumps_p and *interval_order_p are set to
 *    new values as necessary to meet the needs of reallocation.
 *
 * RETURNS:
 *
 *   void
 *
 */

extern void EG_alloc_clumps(int n_intervals,
			    int *n_intervals_alloc_p,
			    Clump_order **clumps_p,
			    Interval ***interval_order_p);

extern void OEG_alloc_clumps(int n_intervals,
			    int *n_intervals_alloc_p,
			    OClump_order **clumps_p,
			    Interval ***interval_order_p);

/*
 *  DESCRIPTION:    
 *
 *    Allocates or reallocates the row header array.
 *
 *  INPUTS:
 *
 *    int ny - number of rows needed
 *
 *    int *nrows_alloc_p - pointer to nrows_alloc in calling routine,
 *      which should be static. Must be initialized to 0 before first
 *      call to alloc_rowh().
 *      Typical declaration in calling routine is:
 *        static int Nrows_alloc = 0;
 *
 *    Row_hdr **rowh_p - pointer to rowh in calling routine which
 *      should be static and initialized to NULL.
 *      Typical declaration in calling routine is:
 *        static Row_hdr *Rowh = NULL;
 *    
 * OUTPUTS:
 *
 *    *nrows_alloc_p and *rowh_p are set to new values as necessary
 *    to meet the needs of reallocation.
 *
 * RETURNS:
 *
 *   void
 *
 */

extern void EG_alloc_rowh(int ny, int *nrows_alloc_p, Row_hdr **rowh_p);

extern void EG_clear_stack_2d(void);

extern int OEG_clump_volume_float
(
 float *grid,			/* I - input grid */
 int grid_nx,			/* I - number of columns in input grid */
 int grid_ny,			/* I - number of rows in input grid */
 int grid_nz,			/* I - number of planes in input grid */
 double threshold,		/* I - threshold to clump at */
 int min_overlap,		/* I - minimum interval overlap */
 OClump_info *clump_info	/* O - set of output clumps */
 );

extern int OEG_clump_grid_float
(
 float *grid,			/* I - input grid */
 int grid_nx,			/* I - number of columns in input grid */
 int grid_ny,			/* I - number of rows in input grid */
 double threshold,		/* I - threshold to clump at */
 OClump_info *clump_info		/* O - set of output clumps */
);

extern void EG_draw_bdry(Point_d *bdry_pts, int num_pts, int colr);

/************************************************************************

Function Name: 	OEG_free_clump_info

Description:  	free memory allocated in clump_info structure
    
Returns:    	void

Notes:

************************************************************************/

extern void OEG_free_clump_info(OClump_info *ci);

/************************************************************************

Function Name: 	OEG_init_clump_info

Description:  	Initialize clump_info structure
    
Returns:    	void

Notes:

************************************************************************/

extern void OEG_init_clump_info(OClump_info *ci);

extern void EG_free_clumps(int *n_intervals_alloc_p,
			   Clump_order **clumps_p,
			   Interval ***interval_order_p);

extern void OEG_free_clumps(int *n_intervals_alloc_p,
			   OClump_order **clumps_p,
			   Interval ***interval_order_p);

extern void EG_free_rowh(int *nrows_alloc_p, Row_hdr **rowh_p);

/*
 * Determine the set of intervals between begin and end in the array row
 * which are above threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */

int EG_get_intervals_float
(
 const float row[],
 const int begin,
 const int end,
 Interval interval_array[],
 const float threshold
);

/*
 * Determine the set of intervals between begin and end in the array row
 * which are below threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */
int EG_get_intervals_below_float
(
 const float row[],
 const int begin,
 const int end,
 Interval interval_array[],
 const float threshold
);

/*
 * DESCRIPTION:    
 *
 * EG_iclump_2d - assign clump_ids to clump of data points in a 2d data set.
 * The 2d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 2d data set.
 *
 * INPUTS:
 *
 * intervals - an array of intervals (see clump.h for definition)
 * num_intervals - the number of intervals in "intervals"
 *
 * ydim - the number of rows in the plane or equivalently, the dimension
 * of row_hdr 
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval in "intervals" will be set
 * with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_2d.  Seed_2d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int EG_iclump_2d(Interval *intervals, int num_intervals,
			int num_rows, int clear, int min_overlap,
			Interval **interval_order, Clump_order *clump_order);

/*
 * DESCRIPTION:    
 *
 * OEG_iclump_2d - assign clump_ids to clump of data points in a 2d data set.
 * The 2d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 2d data set.
 *
 * INPUTS:
 *
 * intervals - an array of intervals (see clump.h for definition)
 * num_intervals - the number of intervals in "intervals"
 *
 * ydim - the number of rows in the plane or equivalently, the dimension
 * of row_hdr 
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval in "intervals" will be set
 * with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(OClump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_2d.  Seed_2d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int OEG_iclump_2d(Interval *intervals, int num_intervals,
			 int num_rows, int clear, int min_overlap,
			 Interval **interval_order, OClump_order *clump_order);

/*
 * DESCRIPTION:    
 *
 * EG_rclump_2d - assign clump_ids to clump of data points in a 2d data set.
 * The 2d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 2d data set.  The function
 * rclump_2d differs from iclump_2d since the user must set up a row
 * header structure which provides organization to the set of intervals.
 * The function iclump_2d does this automatically for the user.
 *
 * INPUTS:
 *
 * row_hdr - an array of structures.  Each structure contains an array of
 * intervals along with the size of the array.  The array of intervals
 * corresponds to a row of intervals.
 *
 * ydim - the number of rows in the plane or equivalently, the dimension
 * of row_hdr 
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval pointed to by row_hdr will
 * be set with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_2d.  Seed_2d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int EG_rclump_2d(Row_hdr *row_hdr, int num_rows, int clear,
			int min_overlap,
			Interval **interval_order, Clump_order *clump_order);

/*
 * DESCRIPTION:    
 *
 * OEG_rclump_2d - assign clump_ids to clump of data points in a 2d data set.
 * The 2d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 2d data set.  The function
 * rclump_2d differs from iclump_2d since the user must set up a row
 * header structure which provides organization to the set of intervals.
 * The function iclump_2d does this automatically for the user.
 *
 * INPUTS:
 *
 * row_hdr - an array of structures.  Each structure contains an array of
 * intervals along with the size of the array.  The array of intervals
 * corresponds to a row of intervals.
 *
 * ydim - the number of rows in the plane or equivalently, the dimension
 * of row_hdr 
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval pointed to by row_hdr will
 * be set with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(OClump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_2d.  Seed_2d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int OEG_rclump_2d(Row_hdr *row_hdr, int num_rows,
			 int clear, int min_overlap,
			 Interval **interval_order, OClump_order *clump_order);

/*
 * DESCRIPTION:    
 * 	EG_seed_2d - find all intervals that are 1-connected to a particular
 * interval (see paper by Wiener, Goodrich, Dixon)
 *
 * INPUTS:
 * 	i - row index of initial seed
 * 	j - column index of initial seed
 * 	ydim - number of rows in 2d array
 * 	row_hdr - array of structures organizing intervals in a given row 
 * 	value - clump id for this seed
 * 	interval_index - index used for sorting intervals according to clump
 * 	interval_order - array ordering intervals according to clump
 *
 * OUTPUTS:
 * 	row_hdr - various subfields in the intervals fields are set
 * 	  including clump_id
 * 	interval_index - increased for each interval added to a clump
 * 	interval_order - set for each interval added to a clump
 *
 * RETURNS:
 *       The number of intervals in the clump or -1 on failure.
 *
 * METHOD:
 * 	Starting at a seed interval, classify all intervals in row_hdr
 * that are 1-connected to it using a seed-fill approach.
 */

extern int EG_seed_2d(int i, int j, int rows, Row_hdr row_hdr[],
		      int value,
		      int *interval_index, Interval **interval_order);

/*
 * DESCRIPTION:    
 *
 * EG_iclump_3d - assign clump_ids to clump of data points in a 3d data set.
 * The 3d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 3d data set.
 *
 * INPUTS:
 *
 * intervals - an array of intervals (see clump.h for definition)
 *
 * num_intervals - the number of intervals in "intervals"
 *
 * zdim - the number of planes in the volume
 *
 * ydim - the number of rows in each plane of the volume
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval in "intervals" will be set
 * with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 3d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_3d.  Seed_3d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int EG_iclump_3d(Interval *intervals, int num_intervals,
			int num_rows, int num_planes,
			int clear, int min_overlap,
			Interval **interval_order, Clump_order *clump_order);

/*
 * DESCRIPTION:    
 *
 * OEG_iclump_3d - assign clump_ids to clump of data points in a 3d data set.
 * The 3d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 3d data set.
 *
 * INPUTS:
 *
 * intervals - an array of intervals (see clump.h for definition)
 *
 * num_intervals - the number of intervals in "intervals"
 *
 * zdim - the number of planes in the volume
 *
 * ydim - the number of rows in each plane of the volume
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval in "intervals" will be set
 * with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(OClump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 3d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_3d.  Seed_3d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int OEG_iclump_3d(Interval *intervals, int num_intervals,
			 int num_rows, int num_planes,
			 int clear, int min_overlap,
			 Interval **interval_order, OClump_order *clump_order);

/*
 * DESCRIPTION:    
 *
 * EG_rclump_3d - assign clump_ids to clump of data points in a 3d data set.
 * The 3d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 3d data set.  The function
 * rclump_3d differs from iclump_3d since the user must set up a row
 * header structure which provides organization to the set of intervals.
 * The function iclump_3d does this automatically for the user.
 *
 * INPUTS:
 *
 * row_hdr - an array of structures.  Each structure contains an array of
 * intervals along with the size of the array.  The array of intervals
 * corresponds to a row of intervals.
 *
 * zdim - the number of planes in the volume
 * ydim - the number of rows in each plane
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval pointed to by row_hdr will
 * be set with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 3d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_3d.  Seed_3d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int EG_rclump_3d(Row_hdr *row_hdr, int ydim, int zdim,
			int clear, int min_overlap,
			Interval **interval_order, Clump_order *clump_order);

/*
 * DESCRIPTION:    
 *
 * OEG_rclump_3d - assign clump_ids to clump of data points in a 3d data set.
 * The 3d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 3d data set.  The function
 * rclump_3d differs from iclump_3d since the user must set up a row
 * header structure which provides organization to the set of intervals.
 * The function iclump_3d does this automatically for the user.
 *
 * INPUTS:
 *
 * row_hdr - an array of structures.  Each structure contains an array of
 * intervals along with the size of the array.  The array of intervals
 * corresponds to a row of intervals.
 *
 * zdim - the number of planes in the volume
 * ydim - the number of rows in each plane
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval pointed to by row_hdr will
 * be set with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(OClump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEGIN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 3d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_3d.  Seed_3d will set the id for all intervals
 * that overlap in adjacent rows.
 */

extern int OEG_rclump_3d(Row_hdr *row_hdr, int ydim, int zdim,
			 int clear, int min_overlap,
			 Interval **interval_order, OClump_order *clump_order);

/*
 * DESCRIPTION:    
 * 	EG_seed_3d - find all intervals that are 1-connected to a particular
 * interval (see paper by Wiener, Goodrich, Dixon)
 *
 * INPUTS:
 * 	i - plane index of initial seed
 * 	j - row index of initial seed
 * 	k - column index of initial seed
 *      zdim - number of planes in volume
 *	ydim - number of rows in each plane
 * 	row_hdr - array of structures organizing intervals in a given row 
 * 	value - clump id for this seed
 * 	interval_index - index used for sorting intervals according to clump
 * 	interval_order - array ordering intervals according to clump
 *
 * OUTPUTS:
 * 	row_hdr - various subfields in the intervals fields are set
 * 	  including clump_id
 * 	interval_index - increased for each interval added to a clump
 * 	interval_order - set for each interval added to a clump
 *
 * RETURNS:
 *       The number of intervals in the clump or -1 on failure.
 *
 * METHOD:
 * 	Starting at a seed interval, classify all intervals in row_hdr
 * that are 1-connected to it using a seed-fill approach.
 */

extern int EG_seed_3d(int i, int j, int k,
		      int planes, int rows, Row_hdr *row_hdr,
		      int value,
		      int *interval_index, Interval **interval_order);

extern int EG_color_point_2d(int y, int x, Row_hdr row_hdr[], int value);

/*
 * int EG_find_intervals_float()
 *
 * For each row in a plane or volume, determine the intervals that are >=
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane
 *     	  or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

extern int EG_find_intervals_float
(
 const int nrows,
 const int ncols,
 const float array[],
 Interval **intervals_p,
 int *n_intv_alloc_p,
 Row_hdr *row_hdr,
 const float threshold
);


/*
 * int EG_find_intervals_aux_float()
 *
 * This function is equivalent to find_intervals() except that it use an
 * auxiliary interval array determined by aux_row_hdr to assist in
 * determining the intervals above threshold.  For each row in plane,
 * determine the intervals that are above threshold.  Use an auxiliary
 * interval array for speedup.  Allocate array space to save these
 * intervals and copy the interval information to the allocated array.
 *
 * Inputs:
 * 	array - array of float
 * 	nrows - total number of rows (in plane or volume)
 * 	ncols - column dimension 
 * 	intervals_p - pointer to intervals array realloc'd if necessary
 * 	n_intv_alloc_p - initial size of intervals array
 * 	row_hdr - storage for the intervals above threshold for
 *        each row (dim nrows)
 * 	aux_row_hdr - auxiliary row header
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - intervals found are stored here 
 * 	row_hdr - the interval arrays and their sizes for each row
 *
 * Returns:
 * 	the number of intervals found
 */

extern int EG_find_intervals_aux_float
(
 int nrows,
 int ncols,
 float array[],
 Row_hdr *aux_row_hdr,
 Interval **intervals_p,
 int *n_intv_alloc_p,
 Row_hdr *row_hdr,
 float threshold
);

/*
 * int EG_find_intervals_3d_float()
 * 
 * For each row in a volume, determine the intervals that are >=
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane or volume)
 * 	ncols - column dimension 
 * 	array - array of float
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

extern int EG_find_intervals_3d_float
(
 int nplanes_in_vol,
 int nrows_in_vol,
 int nrows_in_plane,
 int ncols,
 float array[],
 Interval **intervals_p,
 int *n_intv_alloc_p,
 Row_hdr *row_hdr,
 float threshold
);

/*
 * int EG_find_intervals()
 *
 * For each row in a plane or volume, determine the intervals that are >=
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane
 *     	  or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

extern int EG_find_intervals(int ydim, int xdim, unsigned char array[],
			     Interval **intervals, int *isize,
			     Row_hdr *row_hdr, int threshold);

/*
 * int EG_find_intervals_3d()
 * 
 * For each row in a volume, determine the intervals that are >=
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

extern int EG_find_intervals_3d(int nplanes_in_vol, int nrows_in_vol,
				int nrows_in_plane, int ncols,
				unsigned char array[],
				Interval **intervals, int *isize,
				Row_hdr *row_hdr, int threshold);

/*
 * Inputs:
 * x - the xth interval from the intervals in row_hdr1
 * start   - the interval in row_cb2 to start looking for overlap
 * row_hdr1 - row header containing x
 * row_hdr2 - the other row header
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * Outputs:
 * overlap_begin - the first interval in row_hdr2 that overlaps x
 * overlap_end	- the last interval in row_hdr2 that overlaps x
 *
 * NOTE: The variable overlap_begin is initialized to be greater than
 * overlap_end.  It will be reset if there is an interval in row_hdr2 that
 * overlaps x.
 *
 * Returns:
 * 1 if x overlaps an interval in row_hdr2
 * 0 otherwise
 */

extern int EG_find_overlap(int x, int start,
			   Row_hdr *row_hdr1, Row_hdr *row_hdr2,
			   int min_overlap,
			   int *overlap_begin, int *overlap_end);

/*
 * Determine the set of intervals between begin and end in the array row
 * which are below threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */

extern int EG_get_intervals_below_float
(
 const float row[],
 const int begin,
 const int end,
 Interval interval_array[],
 const float threshold
);

/*
 * Determine the set of intervals between begin and end in the array row
 * which are above threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */

extern int EG_get_intervals_float
(
 const float row[],
 const int begin,
 const int end,
 struct interval interval_array[],
 const float threshold
);

extern int EG_init_stack_2d(void);

/*
 * NAME
 * 	free_interval
 *
 * PURPOSE
 * 	free up interval row control block array
 *
 * NOTES
 *
 *
 * HISTORY
 * 	wiener - Apr 30, 1992: Created.
 */

extern void EG_free_interval(Row_hdr *row_hdr, int interval_dim);

/*******************************************
 * EG_free_intervals()
 *
 * Free intervals allocated in this module.
 *
 */

extern void EG_free_intervals(Interval **intervals_p, int *n_intv_alloc_p);

extern void  EG_free_stack_2d(void);

extern int EG_init_stack_3d(void);

/*
 * DESCRIPTION:    
 * 	EG_edm_2d - Given an array of intervals corresponding to a set of
 * clumps, process the intervals using the euclidean distance map
 * algorithm.  This essentially assigns to each point in the underlying
 * input array, a value corresponding to its distance to the boundary of
 * the clump.  The underlying data is assumed to be offset in the
 * following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	array - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * IMPORTANT NOTE
 */

extern void EG_edm_2d(Row_hdr *row_hdr, unsigned char *array,
		      int xdim, int ydim, int offset);

/*
 * DESCRIPTION:
 * EG_simple_edm_2d - This has the same purpose as
 * EG_edm_2d. The difference is that instead of using intervals to
 * indicate the image shapes, it uses a template array.
 *
 * This essentially assigns to each point in the underlying input array,
 * a value corresponding to its distance to the boundary of the clump.
 * The underlying data is assumed to be offset in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	unsigned char *template_grid - input array, must have vals set 
 *             non-zero for shapes, 0 elsewhere.
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	unsigned char *edm_grid - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * Mike Dixon, from code by Gerry Wiener
 */

extern void EG_simple_edm_2d(unsigned char *template_grid, unsigned char *edm_grid,
			     int xdim, int ydim);

/*
 * DESCRIPTION:    
 * EG_inverse_edm_2d.
 * This is similar to EG_simple_edm_2d, except that it operates in the
 * inverse sense. The euclidean distances are the distance outside
 * the shape, from the closest shape border.
 *
 * This essentially assigns to each point in the underlying
 * input array, a value corresponding to its distance to the boundary of
 * the clump.  The underlying data is assumed to be offset in the
 * following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	unsigned char *template_grid - input array, must have vals set 
 *             non-zero for shapes, 0 elsewhere.
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *
 * OUTPUTS:
 * 	unsigned char *edm_grid - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * Mike Dixon, from code by Gerry Wiener
 */

extern void EG_inverse_edm_2d(unsigned char *template_grid, unsigned char *edm_grid,
			      int xdim, int ydim);

/*
 * DESCRIPTION:    
 * 	edm_3d - Given an array of intervals corresponding to a set of
 * clumps, process the intervals using the euclidean distance map
 * algorithm.  This essentially assigns to each point in the underlying
 * input array, a value corresponding to its distance to the boundary of
 * the clump.  The underlying data is assumed to be offset in the
 * following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back to an
 * array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 * 	int zdim - z dimension of input data set
 *
 * OUTPUTS:
 * 	array - output array, max edm = 255
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	1.  Assign 0 to background pixels, UCHAR_MAX, to pixels in
 * foreground
 * 	2.  In left to right, North to South, bottom to top scan
 * assign pixel in feature a value 1 greater than the smallest of its
 * neighbors
 * 	3.  Repeat 2. going from right to left, South to North, top to
 * bottom scan.
 *
 * This particular function uses the taxicab metric to determine
 * distance.
 *
 * IMPORTANT NOTE
 */

extern void EG_edm_3d(Row_hdr *row_hdr, unsigned char *array,
		      int xdim, int ydim, int zdim, int offset);

/*
 * DESCRIPTION:    
 * 	erode_clump_2d
 *
 * INPUTS:
 *      intervals - array of intervals
 *      num_ints - size of intervals array
 *      xdim - number of columns in underlying array
 *      ydim - number of rows in underlying array
 *      threshold - erode all points within threshold distance of the
 *        boundary if they are not adjacent to points further in from the
 *        boundary
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * OUTPUTS:
 * 	new_clump_array - array of new clumps (This array will also be
 * dimensioned similar to the 2D Example above.
 *      num_clumps - number of output clumps
 *
 * RETURNS:
 *         0 successful
 *       -1 on error
 *
 * NOTES:
 * 	
 */

extern int EG_erode_clump_2d(Row_hdr *row_hdr, int num_intervals,
			     int xdim, int ydim, int threshold,
			     unsigned char *new_clump_array, int *num_clumps);

/*
 * DESCRIPTION:    
 * 	erode_level_2d - Remove all points of a given value that do
 * not have a neighboring point having a value one greater.
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *      int value - test value - points with this value
 *      are candidates.
 *
 * IN_OUT:
 * 	edm_array - this is modified by the erosion process.
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	
 */

extern void EG_erode_level_2d(Row_hdr *row_hdr, unsigned char *edm_array,
			      int xdim, int ydim, int value);

/*
 * DESCRIPTION:    
 * 	erode_lesser_2d - Remove all points of a given value for which
 * all neigbors have lesser values.
 * Similar to erode_level_2d, except the test is >= instead of ==v+1
 *
 * For other details, see erode_below_2d 
 */

extern void EG_erode_lesser_2d(Row_hdr *row_hdr, unsigned char *val_array,
			       int xdim, int ydim, int value);

/*
 * DESCRIPTION:    
 * 	erode_lesser_or_equal_2d - Remove all points of a given value
 *      for which all neigbors have lesser or equal values.
 * Similar to erode_level_2d, except the test is > instead of ==v+1
 *
 * For other details, see erode_below_2d 
 */

extern void EG_erode_lesser_or_equal_2d(Row_hdr *row_hdr, unsigned char *val_array,
					int xdim, int ydim, int value);

/*
 * DESCRIPTION:    
 * 	erode_bridges_2d - remove bridges in grid of width 1
 *      Bridges are single-width connections with a value of
 *      1, and 0's on opposite sides.
 *
 * EXAMPLES:
 *
 *   0000000000             000000000000
 *   0111111110             011110111110
 *   0111111110   vertical  011110111110
 *   0000100000 <- bridge   011111111110
 *   0111111110             011110111110
 *   0111111110             011110111110
 *   0000000000             000000000000
 *                               ^
 *                           Horizontal
 *                             Bridge
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *      are candidates.
 *
 * IN_OUT:
 * 	val_array - this is modified by the erosion process.
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 
 * Mike Dixon, May 1995 	
 */

extern void EG_erode_bridges_2d(Row_hdr *row_hdr, unsigned char *val_array,
				int xdim, int ydim);

/*
 * DESCRIPTION:    
 * 	erode_clump_3d
 *
 * INPUTS:
 * 	intervals - array of intervals
 *      num_ints - size of intervals array
 *      xdim - number of columns in underlying array
 *      ydim - number of rows in underlying array
 *      threshold - erode all points within threshold distance of the
 *        boundary if they are not adjacent to points further in from the
 *        boundary
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array parray back
 * to an array without the extra rows and columns of zeroes.
 *
 * OUTPUTS:
 * 	new_clump_array - array of new clumps (This array will also be
 * dimensioned similar to the 2D Example above.
 *      num_clumps - number of output clumps
 *
 * RETURNS:
 *         0 successful 
 *       -1 on error
 *
 * NOTES:
 * 	
 */

extern int EG_erode_clump_3d(Row_hdr *row_hdr, int num_intervals,
			     int xdim, int ydim, int zdim, int threshold,
			     unsigned char *new_clump_array, int *num_clumps);

/*
 * DESCRIPTION:    
 *	
 *
 * IN:
 *	
 *
 * IN-OUT:
 *	
 *
 * OUT:
 *	
 *
 * RETURNS:
 *       
 *
 * NOTES:
 *	
 */

extern void EG_erode_level_3d(Row_hdr *row_hdr, unsigned char *edm_array,
			      int xdim, int ydim, int zdim, int value);

/*
 * DESCRIPTION:    
 * 	extend_int_2d - extend each interval in a given array of
 * intervals.  This function reallocates space if necessary.
 *
 * INPUTS:
 * 	interval - array of input intervals
 *      in_size - size of interval array
 *      x - displacement in x direction
 *      y - displacement in y direction
 *      box - bounding box for extension
 *
 * OUTPUTS:
 * 	pout_interval - pointer to array of output intervals
 *      (realloc'd if necessary)
 *      pout_size - size of pout_interval
 *      
 * RETURNS:
 *       the number of intervals in pout_interval
 *
 * NOTES:
 * 	This function extends intervals in both positive and negative
 * x and y directions.  It does not do the extension beyond the limits of
 * the bounding box.  Note that xmin, ymin should not be smaller than 0
 * and xmax, ymax should not be larger than the maximum x and y values
 * for the underlying grid for the intervals.
 */

extern int EG_extend_int_2d(Interval *interval, int in_size,
			    int x, int y,
			    Interval **pout_interval, int *pout_size,
			    Box_2d *box);

extern int EG_extend_pint_2d(Interval **interval, int in_size,
			     int x, int y,
			     Interval **pout_interval, int *pout_size,
			     Box_2d *box);

extern int EG_extend_int_3d(Interval *interval, int in_size,
			    int x, int y, int z,
			    Interval **pout_interval, int *pout_size,
			    Box_3d *box);

extern int EG_extend_pint_3d(Interval **interval, int in_size,
			     int x, int y, int z,
			     Interval **pout_interval, int *pout_size,
			     Box_3d *box);

extern void EG_find_2d_bbox
(
 Interval **iarray,		/* I - array of intervals */
 int n_iarray,			/* I - size of iarray */
 box_2d_t *box_2d		/* O - bounding box containing intervals */
);

void OEG_find_ci_2d_bbox
(
 OClump_info *ci		/* I/O - clump information structure */
);

extern void  EG_free_stack_3d();

/*
 * Determine the set of intervals between begin and end in the array row
 * which are above threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */

extern int EG_get_intervals(unsigned char row[], int begin, int end,
			    Interval interval_array[], int threshold);

/*
 * Determine the set of intervals between begin and end in the array row
 * which are below threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */

extern int EG_get_intervals_below(unsigned char row[], int begin, int end,
				  Interval interval_array[], int threshold);

extern int EG_image_dim(
	      FILE *fp,		/* in file pointer */
	      int *xdim,	/* out x dimension */
	      int *ydim,	/* out y dimension */
	      int *zdim 	/* out z dimension */
	      );

extern int EG_image_read(
	       FILE 	*fp,		    /* in file pointer */
	       unsigned char 	*volume	       /* out volume */
	       );

extern int EG_image_write(
		unsigned char *volume,		/* in volume */
		int xdim,	/* in x dimension */
		int ydim,	/* in y dimension */
		int zdim,	/* in z dimension */
		FILE *fp       /* out file pointer */
		);

/*
 * DESCRIPTION:    
 * 	link_intervals
 *
 * INPUTS:
 * 	int_array - array of intervals
 * 	int_array_size - dimension of int_array
 * 	num_rows - number of rows
 *
 * OUTPUTS:
 * 	link_array - array of ptrs to linked lists of intervals for each row
 * 			    (has dimension num_rows)
 * 	links - members of link_array containing interval ptrs and links
 * 		   (has dimension int_array_size)
 * RETURNS:
 *  	void     
 *
 * METHOD:
 * 	For each interval in int_array, determine its row, then link
 * it to the appropriate row.  This has the effect of sorting the
 * intervals in int_array by row.
 *
 * row1 -> link -> link -> 0
 * row2 -> link -> link -> 0
 * row3 -> link -> link -> 0
 * etc.
 */

extern void EG_link_intervals(Interval *int_array, int int_array_size,
			      int num_rows,
			      Interval_link_hdr *link_array,
			      Interval_link *links);

/*
 * DESCRIPTION:    
 * 	link_pintervals - This function differs from link_intervals
 * only in terms of the input array pint_array which is an array of
 * pointers to intervals as opposed to an array of intervals
 *
 * INPUTS:
 * 	pint_array - array of pointers to intervals
 * 	int_array_size - dimension of int_array
 * 	num_rows - number of rows
 *
 * OUTPUTS:
 * 	link_array - array of ptrs to linked lists of intervals for each row
 * 			    (has dimension num_rows)
 * 	links - members of link_array containing interval ptrs and links
 * 		   (has dimension int_array_size)
 * RETURNS:
 *  	void     
 *
 * METHOD:
 * 	For each interval in int_array, determine its row, then link
 * it to the appropriate row.  This has the effect of sorting the
 * intervals in int_array by row.
 *
 * row1 -> link -> link -> 0
 * row2 -> link -> link -> 0
 * row3 -> link -> link -> 0
 * etc.
 */

extern void EG_link_pintervals(Interval **pint_array, int int_array_size,
			       int num_rows,
			       Interval_link_hdr *link_array,
			       Interval_link *links);

/*
 * DESCRIPTION:    
 * 	dump_links
 *
 * INPUTS:
 * 	link_array - array of linked list headers for each row
 * 	num_rows - dimension of link_array
 *
 * OUTPUTS:
 * 	int_array - place to store dumped interval pointers (must be
 * large enough to hold all intervals that are linked - this size is
 * usually known when allocating the links array)
 *
 * RETURNS:
 *       the number of links dumped
 *
 * METHOD:
 * 	Dumps the linked lists contained in link_array to
 * int_array.  
 */

extern int EG_dump_links(Interval_link_hdr *link_array, int num_rows,
			 Interval **int_array);

/*
 * DESCRIPTION:    
 * 	make_row_hdr - generate a row header structure for an array of
 * intervals (similar to gen_row_hdr but does not allocate row header array
 *
 * IN:
 * 	intervals - array of intervals
 * 	num_intervals - size of intervals array
 * 	ydim - total number of rows covered by the intervals array (may
 * 	cover multiple planes)
 *
 * OUT:
 * 	row_hdr - contains output row_hdr array
 *
 * RETURNS:
 *      size of row_hdr array or -1 on failure
 *
 * NOTES:
 *      The intervals are assumed to be sorted according to row, column order.
 */

extern int EG_make_row_hdr(Interval *intervals, int num_intervals,
			   int ydim, Row_hdr *row_hdr);

/*
 * overlap() returns info on whether two intervals overlap.
 *
 * If min_overlap is +n, then the intervals must overlap by n
 * grid positions.
 * If min_overlap is 0, the intervals 'overlap' if their corners 
 * touch diagonally.
 * If min_overlap is -n, the intervals are considered to
 * overlap if there is a gap of n grid positions or less between them.
 *
 * If interval1 is before interval2 including min overlap
 * 	return -1
 *
 * If interval1 and interval2 overlap including min overlap
 * 	return  0
 *
 * If interval1 is after interval2 including min overlap
 * 	return  1
 */

extern int EG_overlap(Interval *interval1_ptr, Interval *interval2_ptr,
		      int min_overlap);

extern void EG_overlap_plane(int ydim, Row_hdr *row_hdr, int min_overlap);

/*
 * ALGORITHM:
 * 	
 * 1.  Get the next interval, next_interal, in row_cb1.
 * 2.  Find the set of intervals it overlaps in row_cb2.
 * 3.  Record the (begin, end) indices of the overlapping intervals from
 *     row_cb2 in the appropriate direction subfield of next_interval
 * 4.  Go back to 1.
 *
 * INPUTS:
 *
 * row_hdr1 - the first row header
 * row_hdr2 - the second row header
 * direction  - the direction to update.  0 - north, 1 - south,
 * 2 - up, 3 - down
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * the intervals array in the row header row_hdr1 are
 * adjusted to reflect the overlap of intervals of row_hdr1 and row_hdr2
 *
 * RETURNS:
 *
 * no return
 */

extern void EG_overlap_rows(Row_hdr *row_hdr1, Row_hdr *row_hdr2,
			    int direct, int min_overlap);

/*
 * DESCRIPTION:    
 *	overlap_volume
 *
 * INPUTS:
 *	zdim - number of planes in volume
 *	ydim - number of rows in each plane
 *	row_hdr - array of row header information for each row
 *	(has dimension zdim * ydim)
 *
 * OUTPUTS:
 *	row_hdr - overlap field in intervals array for each row header
 *	struct is set
 *
 * RETURNS:
 *  	void     
 *
 * METHOD:
 *	
 */

extern void EG_overlap_volume(int zdim, int ydim,
			      Row_hdr *row_hdr, int min_overlap);

extern void OEG_print_clump_info(OClump_info *ci);

/*
 *
 * INPUTS:
 *
 * interval_order - array of pointers to intervals ordered by clump
 *
 * clump_order - array of structures per clump containing information as
 * to the intervals in each clump.  The clump_order structure contains
 * the number of intervals in the clump as well as a pointer into the
 * interval_order array.
 *
 * num_clumps - size of clump_order array
 *
 * RETURNS:
 *
 * void
 */

extern void EG_print_clump_intervals(Interval **interval_order, Clump_order *clump_order, int num_clumps);

/*
 *
 * INPUTS:
 *
 * interval_order - array of pointers to intervals ordered by clump
 *
 * clump_order - array of structures per clump containing information as
 * to the intervals in each clump.  The clump_order structure contains
 * the number of intervals in the clump as well as a pointer into the
 * interval_order array.
 *
 * num_clumps - size of clump_order array
 *
 * RETURNS:
 *
 * void
 */

extern void OEG_print_clump_intervals(OClump_order *clump_order,
				      int num_clumps);

extern void EG_print_interval(Interval *iptr);

/*
 *
 * INPUTS:
 *
 * intervals 	- array of intervals
 * num_ints	- dimension of intervals
 *
 * RETURNS:
 *
 * void
 */

extern void EG_print_intervals(Interval *intervals, int num_ints);

/*
 *
 * INPUTS:
 *
 * intervals 	- array of intervals
 * num_ints	- dimension of intervals
 *
 * RETURNS:
 *
 * void
 */

extern void EG_print_pintervals(Interval **pintervals, int num_ints);

/*
 * Print out relevant interval information in the array interval_row_cb.
 *
 * INPUTS:
 *
 * row_hdr - array of row_hdr structures
 * num_rows - the number of elements in intervals
 *
 * RETURNS:
 *
 * void
 */

extern void EG_print_row_hdr(Row_hdr *row_hdr, int num_rows);

extern int EG_pop_2d(int *x, int *y);

extern int EG_push_2d(int x, int y);

extern void EG_reset_clump_id(Interval *intervals, int num_ints);

extern void EG_reset_arrays(Interval intervals[], Interval **interval_order,
			    Clump_order *clump_order, int num_ints);

extern void OEG_reset_arrays(Interval intervals[], Interval **interval_order,
			     OClump_order *clump_order, int num_ints);

/*
 * DESCRIPTION:    
 *	set_rp - set the row and plane fields in a given array of intervals
 *
 * INPUTS:
 *	row_hdr - array of row header structures
 *	num_rows - size of array row_hdr
 *	num_planes - number of planes
 *
 * OUTPUTS:
 *	row_hdr - the row and plane fields in the intervals array are set
 *
 * RETURNS:
 *       void
 *
 * METHOD:
 */

extern void EG_set_rp(Row_hdr *row_hdr, int num_rows, int num_planes);

extern void EG_get_perp(Point_d *a, Point_d *b, Point_d *c, Point_d *d);

extern double EG_get_perp_dist(Point_d *a, Point_d *b, Point_d *c);

extern double EG_get_perp_sign_dist(Point_d *a, Point_d *b, Point_d *c,
				    int *sign);

extern void OEG_set_interval_float
(
 float *grid,			/* I/O - grid */
 int grid_nx,			/* I - number of columns in grid */
 Interval *iptr,		/* I - intervals to set */
 float val			/* I - value to set */
);

/*
 * DESCRIPTION:    
 * 	set_intervals - set all elements in an array corresponding to
 * intervals to a specified value.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 * 	interval_array - array of underlying intervals
 *      ncols - number of columns in 2d matrix corresponding to array
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals(unsigned char *array, int ncols,
			     Interval **interval_array, int size,
			     unsigned char value);

/*
 * DESCRIPTION:    
 * 	set_intervals_clump - set all elements in an array
 * corresponding to intervals to the clump value.  This function is
 * similar to set_intervals_row_hdr except that clump values are used in
 * setting the underlying array.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_clump(unsigned char *array,
				   Row_hdr *row_hdr, int nrows, int ncols);

/*
 * DESCRIPTION:    
 * 	set_intervals_row_hdr - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals except that it uses a row_hdr array as
 * opposed to an array of intervals.    Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_row_hdr(unsigned char *array,
				     Row_hdr *row_hdr, int nrows, int ncols,
				     unsigned char value);

/*
 * DESCRIPTION:    
 * 	set_intervals_int16 - set all elements in an array corresponding to
 * intervals to a specified value.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 * 	interval_array - array of underlying intervals
 *      ncols - number of columns in 2d matrix corresponding to array
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_int16(unsigned short *array, int ncols,
				   Interval **interval_array, int size,
				   unsigned short value);

/*
 * DESCRIPTION:    
 * 	set_intervals_row_hdr - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals except that it uses a row_hdr array as
 * opposed to an array of intervals.    Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_row_hdr_int16(unsigned short *array,
					   Row_hdr *row_hdr, int nrows,
					   int ncols, unsigned short value);

/*
 * DESCRIPTION:    
 * 	set_intervals_clump - set all elements in an array
 * corresponding to intervals to the clump value.  This function is
 * similar to set_intervals_row_hdr except that clump values are used in
 * setting the underlying array.  Applies to both 2d and 3d arrays.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      nrows - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_clump(unsigned char *array,
				   Row_hdr *row_hdr, int nrows, int ncols);

/*
 * DESCRIPTION:    
 * 	set_intervals_translate_2d - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals_row_hdr except that it sets array values
 * using an offset.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      new_xdim - number of columns in array
 *      ydim - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 *      xoff - x offset of interval information from beginning of array
 *      yoff - y offset of interval information from beginning of array
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_translate_2d(unsigned char *array, Row_hdr *row_hdr,
					  int ydim, int new_xdim,
					  int xoff, int yoff, unsigned char value);

/*
 * DESCRIPTION:    
 * 	set_intervals_translate_3d - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals_translate_2d except that it uses 3
 * dimensions.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      new_xdim - number of columns in array
 *      ydim - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 *      xoff - x offset of interval information from beginning of array
 *      yoff - y offset of interval information from beginning of array
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_translate_3d(unsigned char *array, Row_hdr *row_hdr,
					  int ydim, int zdim,
					  int new_xdim, int new_ydim,
					  int xoff, int yoff, int zoff,
					  unsigned char value);

/*
 * DESCRIPTION:    
 * 	set_intervals_translate_2d - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals_row_hdr except that it sets array values
 * using an offset.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      new_xdim - number of columns in array
 *      ydim - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 *      xoff - x offset of interval information from beginning of array
 *      yoff - y offset of interval information from beginning of array
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_translate_2d(unsigned char *array, Row_hdr *row_hdr,
					  int ydim, int new_xdim,
					  int xoff, int yoff, unsigned char value);

/*
 * DESCRIPTION:    
 * 	set_intervals_translate_3d - set all elements in an array
 * corresponding to intervals to a specified value.  This function is
 * similar to set_intervals_translate_2d except that it uses 3
 * dimensions.
 *
 * INPUTS:
 *      ncols - number of columns in array
 *      new_xdim - number of columns in array
 *      ydim - number of rows in array
 * 	row_hdr - specifies array of underlying intervals
 * 	size - number of intervals in interval_array
 * 	value - specified value
 *      xoff - x offset of interval information from beginning of array
 *      yoff - y offset of interval information from beginning of array
 * 	
 * OUTPUTS:
 * 	array - array of elements to be set	
 *
 * RETURNS:
 *  	void	
 *
 * METHOD:
 * 	
 */

extern void EG_set_intervals_translate_3d(unsigned char *array, Row_hdr *row_hdr,
					  int ydim, int zdim,
					  int new_xdim, int new_ydim,
					  int xoff, int yoff, int zoff,
					  unsigned char value);

extern int EG_translate_int_2d(Interval *interval, int in_size,
			       double x, double y,
			       Interval **pout_interval, int *pout_size,
			       Box_2d *box);

/*
 * DESCRIPTION:    
 * 	translate_array_2d - Suppose that a larger grid A overlaps a
 * smaller grid B.  Set the values in B according to the overlapping
 * values in A.  We assume that A is offset from B using the values xoff,
 * yoff.  Hence if the dimensions for A are xdim, ydim then the
 * dimensions for B are xdim - xoff, ydim - yoff.  This function may be
 * useful in conjunction with the function edm_2d.
 *
 * INPUTS:
 * 	in_array - input array corresponding to A above
 *      xdim - the x dimension for in_array
 *      ydim - the y dimension for in_array
 *      xoff - the x offset of A from B
 *      yoff - the y offset of A from B
 *
 * OUTPUTS:
 * 	out_array - output array corresponding to B above
 *
 * RETURNS:
 *       void
 *
 * NOTES:
 * 	
 */

extern void EG_translate_array_2d(unsigned char *in_array, int xdim, int ydim,
				  int xoff, int yoff, unsigned char *out_array);

/*
 * DESCRIPTION:    
 * 	translate_array_3d - Suppose that a larger grid A overlaps a
 * smaller grid B.  Set the values in B according to the overlapping
 * values in A.  We assume that A is offset from B using the values xoff,
 * yoff, zoff.  Hence if the dimensions for A are xdim, ydim, zdim then
 * the dimensions for B are xdim - xoff, ydim - yoff and zdim - zoff.
 * This function may be useful in conjunction with the function edm_3d.
 *
 * INPUTS:
 * 	in_array - input array corresponding to A above
 *      xdim - the x dimension for in_array
 *      ydim - the y dimension for in_array
 *      zdim - the z dimension for in_array
 *      xoff - the x offset of A from B
 *      yoff - the y offset of A from B
 *      zoff - the z offset of A from B
 *
 * OUTPUTS:
 * 	out_array - output array corresponding to B above
 *
 * RETURNS:
 *       void
 *
 * NOTES:
 * 	
 */

extern void EG_translate_array_3d(unsigned char *in_array,
				  int xdim, int ydim, int zdim,
				  int xoff, int yoff, int zoff,
				  unsigned char *out_array);

extern int EG_translate_bdry(Point_d *in_bdry, int in_bdry_size,
			     Point_d vect, Box_2d bbox,
			     Point_d *out_bdry, int out_bdry_size);

/*
 * DESCRIPTION:    
 * 	union_row - given an array of pointers to intervals all of
 * which belong to one row, form the union of the intervals
 *
 * INPUTS:
 * 	interval_order - array of pointers to intervals
 * 	size - size of interval_order array
 *
 * OUTPUTS:
 * 	out - output intervals after taking union (the dimension of
 * the output array should be at least as big as the input array and must
 * be allocated by the user)
 *
 * RETURNS:
 *       size of output array or -1 on error
 *
 * NOTES:
 * 	the input array is assumed to be sorted in ascending order
 */

extern int EG_union_row(Interval **interval_order, int size, Interval *out);

/* compare intervals in the same row */

extern int EG_compare_ints_1d(const void *p, const void *q);

/* compare intervals in potentially different rows but the same plane */

extern int EG_compare_ints_2d(const void *p, const void *q);

/* sort intervals contained in one row into ascending column order */

extern void EG_sort_ints_1d(Interval **interval_order, int size);

/*
 * sort a 2d array of pointers to intervals into first row then column
 * ascending order
 */

extern void EG_sort_ints_2d(Interval **interval_order, int size);

/*
 * DESCRIPTION:    
 * 	sort_ints1_2d - like sort_ints, this function sorts an array of
 * pointers to intervals in row-column order, i.e., intervals with
 * smaller row indices precede intervals with larger row indices and
 * intervals having the same row indices are then sorted into ascending
 * column indices.  This function differs from sort_ints_2d in that it links
 * intervals into rows first, then sorts each row whereas sort_ints_2d
 * compares two intervals by comparing both rows and columns.  Hence
 * sort_ints_2d has order n(log(n)) whereas sort_ints1_2d should have order
 * k(log(n/k))+cn where k is the number of rows inhabited by intervals
 * and hence is less than n.
 *
 * INPUTS:
 * 	interval_order - input array of pointers to intervals to be sorted
 *	num_ints - size of interval_order
 *	num_rows - total number of rows in plane containing intervals
 *
 * OUTPUTS:
 * 	out_interval - output array of intervals (not pointers to intervals)
 *
 * RETURNS:
 *       num_ints if successful, -1 on failure
 *
 * NOTES:
 * 	
 */

extern int EG_sort_ints1_2d(Interval **interval_order, int num_ints,
			    int num_rows, Interval *out_interval);

/* compare intervals in potentially different rows and different planes */

extern int EG_compare_ints_3d(const void *p, const void *q);

/*
 * sort a 3d array of pointers to intervals into first plane, then row
 * then column ascending order
 */

extern void EG_sort_ints_3d(Interval **interval_order, int size);

/* zero out clumps that are
   a.  too small
   b.  not strong enough in magnitude
   c.  horizontal lines
   d.  vertical lines
   */

extern int OEG_zero_clump_float
(
 float *grid,			/* I/O - input/output grid */
 OClump_info *ci,		/* I - clump information structure */
 int min_size,			/* I - minimum clump size */
 double min_mag			/* I - minimum clump magnitude */
);

extern int EG_init_stack_2d();
extern void EG_free_stack_2d();
extern void EG_clear_stack_2d();
extern int EG_push_2d(int x, int y);
extern int EG_pop_2d(int *x, int *y);

extern int EG_init_stack_3d();
extern void EG_free_stack_3d();
extern void EG_clear_stack_3d();
extern int EG_push_3d(int x, int y, int z);
extern int EG_pop_3d(int *x, int *y, int *z);

#ifdef __cplusplus
}
#endif

#endif


