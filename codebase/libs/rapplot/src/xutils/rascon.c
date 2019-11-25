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
/*********************************************************************
 * RASCON.C : Raster Contourer for X displays - - no borders, labels etc.
 *          Handles variable type data, handle warped grids.
 *
 *    Created:  F. Hage    Jan 1991    NCAR - RAP From code
 *     originally written at UNC 1987.
 *
 *     Modified: Deirdre Roach Oct 1992        NCAR - RAP
 *                add filled contouring, contouring by row.
 *     Modified by F. Hage Mar 1993 to handle variable type data
 *     Modified by F. Hage May 1994 - Reimplement filled contouring,
 *       fix misc bugs, speed up labeling
 *    
 */

#include <toolsa/membuf.h>
#include <rapplot/rascon.h>    /* include file for this library */
#include <math.h>
#include <string.h>
#include <stdlib.h>

#if defined(SUNOS5) || defined(SUNOS5_64)
extern long random(void);
#endif

#define irint(x) ((int)rint((x)))

/* defines */
#define MAX_ROWS    8192 /* max size of data array */
#define MAX_COLS    8192
#define MAX_PTS         10   /* maximum number of points to define a polygon */
#define MAXPATHPOINTS   8 /* maximum number of points in a path (see DoBox*) */
#define MAXCHARS        8 /* maximum number of chars in a label */
#define NUMROWS_OF_LABELS  4 /* number of rows, interior to grid where labels appear */
#define NUMCOLS_OF_LABELS  4 /* number of columns, interior to grid where labels appear */
#define MAXLEVELS      256 /* array limit */

/* macros */
#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ?  (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)        (((a) > (b)) ?  (a) : (b))
#endif

void smooth_plane(double *data, int nx, int ny);
double* copy_plane(caddr_t    ptr);

/*****************************************************************************
 * Internal STRUCTURE definition for global data
 */
   
struct    cont_params {
    int    n_cols;      /* number of cell columns to contour - One less than the number of data points along X */
    int    n_rows;      /* number of cell rows to contour  - One less than the number of data points along Y */
    int    data_cols;   /* number of data points in x dim */
    int    data_rows;   /* number of data points in y dim */
    int    data_type;   /* The enumerated Type of the input data */
    int    num_levels;   /* number of contour levels */
    int    fill_flag;     /* 0 = line contours, 1 = filled contours */
    int    fullgrid;      /* 0 = contour part of grid, 1 = full grid */
    int    do_extras;     /* Bitwize control of extra features - Labeling, smoothing */

    void * data_ptr;    /* pointer to data grid data_ptr[rows][cols] */
    void * r1_ptr;      /* pointer to row1 of data values */ 
    void * r2_ptr;      /* pointer to row2 of data values */ 

    double  *con_level;  /* contour levels to draw at */

    double  label_scale;  /* amount to scale contour labels by */
    double  label_bias;   /* amount to scale bias labels by */

    double  *row1;       /* first row of data values      */
    double  *row2;       /* second row of data values     */
    double  *sm_plane;   /* Pointer to Temporary  work plane for smoothing */

    double  point0;      /* hold points within box    */
    double  point1;      /* hold points within box    */
    double  point2;      /* hold points within box    */
    double  point3;      /* hold points within box    */
    double  point4;      /* hold points within box    */
    double  miss_val;    /* missing value */

    /* used when contouring the entire grid */
    double  x_start[MAX_ROWS]; /* X,Y origin in drawable */
    double  y_start[MAX_COLS];
    double  x_delta[MAX_ROWS]; /* (drawable width) / ncols */
    double  y_delta[MAX_COLS]; /* (drawable height) / nrows */
    double  dynamic_range;     /* the dynamic range of the contour labeled levels */

    Display  *dpy;    /* X window display we are using */
    Drawable  xid;    /* X drawable handle */    
    GC  *gc_level;    /* gc's for each contour level */    

    /* used when contouring cells between two rows of data */
    RASCONBoxEdge *edges;
    MEMbuf *labelbuf;
};

/*
 * struct for label buffer entries
 */

typedef struct {
  int pix_x;
  int pix_y;
  GC  gc;
  char text[32];
} label_entry_t;

/*******************************************************************************
 * GLOBAL DATA AREA
 */

static struct   cont_params cp;    /* data area to place most contour parameters */

/*******************************************************************************
 * internal function prototypes
 */

static void CalcPixPos2Rows(double x, double y, int col, int *pixx, int *pixy);
static int CalcLinePath(int linenum, double level,double *xcoord, double *ycoord);
static int DoBox(int row, int col);
static int DoBoxFilled(int row, int col);
static int DoLinePath(int linenum, double level,GC gc,int row,int col,int pen);
static int DoPathFilled(int *path,double level,GC gc,int row,int col);
static int DoMap( void);
static int DoRow(int row);
static int FillRows(int row);
static int NeedLabel(int *path, int level_index, int colnum, int rownum, int ncols,int nrows);
#ifdef NOTNOW
static int PlotLabel(Display *disp, Drawable dw, int label_point, int col, int row,
		     double level, double label_scale, double label_bias, GC gc);
#endif
static void PlotLabelsFromBuf(Display *disp, Drawable dw);
static int AddLabel(int label_point, int col, int row,
		    double level, double label_scale, double label_bias, GC gc);


/************************** external routines ******************************/
/*******************************************************************************
 * RASCONcontour2Rows - contours the input row1 and row2 data using the specified
 *                      levels[]. Draws line or filled contours to the display
 *                      and drawable specified.
 */

int RASCONcontour2Rows(Display *disp, Drawable draw_xid,GC *draw_gc,
               int num_points_along_x, int rownum,
	       caddr_t row1_data, 
               caddr_t row2_data, caddr_t bad_data,int data_type, int nlevels, 
               double *levels,
               RASCONBoxEdge *bedge, int fill_flag, int do_extras, 
               double label_scale, double label_bias)
{
    int status;
    unsigned char    *char_ptr;
    unsigned short  *short_ptr;
    int     *int_ptr;
    float   *float_ptr;
    double  *dbl_ptr;

    /* set global variables */
    cp.gc_level = draw_gc;
    cp.dpy = disp;
    cp.xid = draw_xid;

    cp.fullgrid = 0;
    cp.n_rows = 1;
    cp.data_rows = 2;
    cp.n_cols = num_points_along_x -1;
    cp.data_cols = num_points_along_x;
    cp.data_type = data_type;

    switch(cp.data_type) {
        case RASCON_CHAR: char_ptr = (unsigned char *) bad_data;    
            cp.miss_val = *char_ptr;
        break;
        case RASCON_SHORT: short_ptr = (unsigned short *) bad_data;    
            cp.miss_val = *short_ptr;
        break;
        case RASCON_INT: int_ptr = (int *) bad_data;    
            cp.miss_val = *int_ptr;
        break;
        case RASCON_FLOAT: float_ptr = (float *) bad_data;    
            cp.miss_val = *float_ptr;
        break;
        case RASCON_DOUBLE: dbl_ptr = (double *) bad_data;    
            cp.miss_val = *dbl_ptr;
        break;
    }
    cp.edges = bedge;
    cp.dynamic_range =  fabs((levels[nlevels-1] -levels[0]) * label_scale);

    cp.con_level = levels;
    cp.num_levels = nlevels;
    cp.fill_flag = fill_flag;

    cp.r1_ptr = row1_data;
    cp.r2_ptr = row2_data;

    cp.label_scale = label_scale;
    cp.label_bias = label_bias;

    /* labeling flag. This is ignored for filled contours */
    cp.do_extras = do_extras;
    if (cp.fill_flag == RASCON_FILLED_CONTOURS) cp.do_extras &= ~(RASCON_DOLABELS);

    /* Smoothing across two rows does not make sense - Disable this */
    cp.do_extras &= ~(RASCON_DOSMOOTH | RASCON_DOSMOOTH2);

    /* do contouring */
    status = DoMap();
     
    return(status);
}
 
/*******************************************************************
 * RASCONinit : Calculate row, col coordinate to pixel
 *            coords, making sure it will fit in the given space.
 *              Initialize global variables
 */
int RASCONinit(Display *disp, Drawable draw_xid, int ncols, int nrows, int *x_grid, int *y_grid) 
{
    int    i;
    /* error check */
    if (ncols <=0 || nrows <= 0) {
      fprintf(stderr,"RASCON ERROR: rows and columns must be greater than zero \n");
      return(0);
    }

    /* fill global variables */

    cp.dpy = disp;
    cp.xid = draw_xid;
    
    cp.fullgrid = 1;                /* contour the entire grid */
    cp.n_cols = ncols -1;
    cp.data_cols = ncols;
     
    cp.n_rows = nrows -1;
    cp.data_rows = nrows;

    /* Record the starting and delta pixel coordinates of each contour cell */
    for(i=0; i < cp.n_cols; i++) {
        cp.x_start[i] = x_grid[i]; 
        cp.x_delta[i] = x_grid[i+1] - x_grid[i];
    }

    for(i=0; i < cp.n_rows ; i++) {
        cp.y_start[i] = y_grid[i];
        cp.y_delta[i] = y_grid[i+1] - y_grid[i];
    }    

    return(1);
}

/*******************************************************************************
 * RASCONcontour - contours the input data[] array using the specified
 *                 levels[]. Draws line or filled contours to the display
 *                 and drawable specified in RASCONinit().
 */

int RASCONcontour(GC *draw_gc, void * data, void * bad_data,int data_type, int nlevels,
          double *levels, int fill_flag,
          int do_extras,double label_scale,double label_bias)

{
    int    status;
    unsigned char    *char_ptr;
    unsigned short  *short_ptr;
    int     *int_ptr;
    float   *float_ptr;
    double  *dbl_ptr;

    /* set global variables */
    cp.gc_level = draw_gc;
    cp.data_type = data_type;
    switch(cp.data_type) {
        case RASCON_CHAR: char_ptr = (unsigned char *) bad_data;    
            cp.miss_val = *char_ptr;
        break;
        case RASCON_SHORT: short_ptr = (unsigned short *) bad_data;    
            cp.miss_val = *short_ptr;
        break;
        case RASCON_INT: int_ptr = (int *) bad_data;    
            cp.miss_val = *int_ptr;
        break;
        case RASCON_FLOAT: float_ptr = (float *) bad_data;    
            cp.miss_val = *float_ptr;
        break;
        case RASCON_DOUBLE: dbl_ptr = (double *) bad_data;    
            cp.miss_val = *dbl_ptr;
        break;
    }

    cp.con_level = levels;
    cp.num_levels = nlevels;
    cp.data_ptr = data;
    cp.fill_flag = fill_flag;
    cp.label_scale = label_scale;
    cp.label_bias = label_bias;
    cp.dynamic_range =  fabs((levels[nlevels-1] -levels[0]) * label_scale);


    /* labeling flag. This is ignored for filled contours */
    cp.do_extras = do_extras;
    if (cp.fill_flag == RASCON_FILLED_CONTOURS) cp.do_extras &= ~(RASCON_DOLABELS);

    status = DoMap();
     
    return(status);
}

/************************** internal routines ******************************/

/*********************************************************************
 * CALCPIXPOS2ROWS: Calculate the screen coords using simple bilinear 
 * interpolation. Input (x,y) position in box coordinates.
 * Output pixx,pixy in screen coordinates
 */

static void CalcPixPos2Rows(double x, double y, int col, int *pixx, int *pixy)
	/* double x, y;         x,y position in box coordinates */
	/* int col;             current column */
	/* int *pixx, *pixy;    returned */
{
  double x1, x2, y1, y2;

  /* if on one of the corners, don't bother to do calculations */
  if(x <= 0.0 ) {
     if(y <= 0.0) {
         *pixx = cp.edges[col].x1;
         *pixy = cp.edges[col].y1;
	 return;
     } 
     if(y >= 1.0) {
        *pixx = cp.edges[col].x2;
        *pixy = cp.edges[col].y2;
	return;
      }
  }

  if (x >= 1.0) { 
    if(y <= 0.0) {
        *pixx = cp.edges[col+1].x1;
        *pixy = cp.edges[col+1].y1;
	return;
    }
    if(y >= 1.0) {
        *pixx = cp.edges[col+1].x2;
        *pixy = cp.edges[col+1].y2;
	return;
    }
  }

  /* Find point along line at the top of the cell */
  x1 = cp.edges[col].x1 + ((cp.edges[col+1].x1 - cp.edges[col].x1) * x);
  y1 = cp.edges[col].y1 + ((cp.edges[col+1].y1 - cp.edges[col].y1) * x);

  /* Find point along line at the bottom of the cell */
  x2 = cp.edges[col].x2 + ((cp.edges[col+1].x2 - cp.edges[col].x2) * x);
  y2 = cp.edges[col].y2 + ((cp.edges[col+1].y2 - cp.edges[col].y2) * x);
   
  /* Calc point along line between the other two points */
  *pixx = x1 + ((x2 - x1) * y);
  *pixy = y1 + ((y2 - y1) * y);
  return;
}  

/*********************************************************************
 * PlotLine : Map input (x,y) to screen coordinates. Draw a line from
 *            previous (x,y) to input (x,y)
 */

static int PlotLine(double x, double y, GC gc, int col, int row, int code)
{
    int pix_x, pix_y;
    static int    lastx,lasty;    /* storage for last coordinate used */

    /* map to screen coordinates */
    if (cp.fullgrid) {
      pix_x  = (cp.x_start[col] + (cp.x_delta[col] * x) + 0.5);
      pix_y  = (cp.y_start[row] + (cp.y_delta[row] * y) + 0.5);
    } else {
      CalcPixPos2Rows(x, y, col, &pix_x, &pix_y);
    }

    switch(code) {
        case 0:    /* Pen up moveto operation */
            lastx = pix_x;
            lasty = pix_y;
        break;

        default:    /* other draw codes */
            XDrawLine(cp.dpy,cp.xid,gc,lastx,lasty,pix_x,pix_y);
            lastx = pix_x;
            lasty = pix_y;
        break;
    }
    return 0;
}

/*********************************************************************
 * PlotFilled : Map input polygon points to screen coordinates.
 *              Draw a filled polygon.
 */

static int PlotFilled(double pgon[][2], int npts, GC gc, int col, int row)
{
    XPoint pts[MAX_PTS];
    int x, y;
    int i;

    if (npts > MAX_PTS) {
        fprintf(stderr,"RASCON ERROR: too many points in contour:%d \n", npts);
        return 1;
     }

    /* map to screen coordinates */
    for (i=0; i<npts; i++) {
        if (cp.fullgrid) {

          pts[i].x = (short)(cp.x_start[col] + (cp.x_delta[col] * pgon[i][0]) + 0.5);
          pts[i].y = (short)(cp.y_start[row] + (cp.y_delta[row] * pgon[i][1]) + 0.5);
        } else {

          CalcPixPos2Rows(pgon[i][0], pgon[i][1], col, &x, &y);
          pts[i].x = x;
          pts[i].y = y;
        }

      }

    XFillPolygon(cp.dpy, cp.xid, gc, pts, npts, Nonconvex, CoordModeOrigin);    
    
    return 0;
}
 
/******************************************************************************
 * DoMap :  Do this map row by row
 *
 */

static int DoMap(void)
{
    int    status,i;

    /*allocate space for vals */
    cp.row1 =(double *) calloc(cp.data_cols,sizeof(*cp.row1));
    cp.row2 =(double *) calloc(cp.data_cols,sizeof(*cp.row2));

    /* create mem buffer for label positions */

    cp.labelbuf = MEMbufCreate();

    if(cp.row1 == NULL || cp.row2 == NULL) {
	perror("RASCON Can't allocate memory for RASCON 2 rows\n");
	MEMbufDelete(cp.labelbuf);
	if(cp.row1) {
	    free(cp.row1);
	    cp.row1 = NULL;
	}
	if(cp.row2) {
	    free(cp.row2);
	    cp.row2 = NULL;
	}
	exit(-1);
    }


    if(cp.do_extras & (RASCON_DOSMOOTH | RASCON_DOSMOOTH2)) {  
      cp.sm_plane = copy_plane((caddr_t) cp.data_ptr);

      smooth_plane(cp.sm_plane, cp.data_cols, cp.data_rows);

      if(cp.do_extras & RASCON_DOSMOOTH2) 
	  smooth_plane(cp.sm_plane, cp.data_cols, cp.data_rows);
    }


    srandom(1); /* Seed the random number generator for repeatable labels */

    for(i = 0; i < cp.n_rows; i++) {    /* do all of the rows    */
        status = FillRows(i);    /* fill the raster arrays */
        if( status) {
	  MEMbufDelete(cp.labelbuf);
	  return(-1); 
	}

        status = DoRow(i);    /* contour one row of cells    */
        if(status) {
	  MEMbufDelete(cp.labelbuf);
	  return(-1);
	}
    }

    if(cp.row1) {
	free(cp.row1);
	cp.row1 = NULL;
    }
    if(cp.row2) {
	free(cp.row2);
	cp.row2 = NULL;
    }

    if(cp.do_extras & (RASCON_DOSMOOTH | RASCON_DOSMOOTH2)) {
        if(cp.sm_plane) {
	    free(cp.sm_plane);
	    cp.sm_plane = NULL;
	}
    }

    /* plot labels from buffer */

    PlotLabelsFromBuf(cp.dpy, cp.xid);

    /* free up labels */

    MEMbufDelete(cp.labelbuf);
    return(0);
}

/*******************************************************************************
 * COPY_PLANE : Allocate and fill array with values from the data array 
 *                Returns pointer to allocated array of doubles
 */

double* copy_plane(caddr_t    ptr)
{
    int i;
    unsigned char    *char_ptr;
    unsigned short  *short_ptr;
    int     *int_ptr;
    float   *float_ptr;
    double  *dbl_ptr;
    double  *temp,*temp_ptr;

    temp  = (double *) calloc(cp.data_cols * cp.data_rows , sizeof(double));

    if(temp == NULL) {
	perror("RASCON Can't allocate memory for copy plane \n");
	exit(-1);
    }

    temp_ptr = temp;

    switch(cp.data_type) {
         case RASCON_CHAR:
                srandom(1); /* Seed  for repeatable noise */
		char_ptr = (unsigned char *) ptr;
                i = cp.data_cols * cp.data_rows;
		if(cp.do_extras & RASCON_ADDNOISE) {
                    while(i--) { 
		       *temp_ptr =  (double) *char_ptr++;
		    
		       if(*temp_ptr != cp.miss_val) { /* Add 1 part in 125  noise */
		         *temp_ptr += ((*temp_ptr * .008) * rand() /(RAND_MAX+1.0)) - (*temp_ptr * .004);
		       }
		       temp_ptr++;
		    };
		} else {
                    while(i--) { 
		       *temp_ptr++ = (double) *char_ptr++;
		    };
		}

            break;
            case RASCON_SHORT:
                short_ptr = (unsigned short *) ptr;    
                i = cp.data_cols * cp.data_rows;
                while(i--) { *temp_ptr++ =  *short_ptr++; };

            break;
            case RASCON_INT: 
		int_ptr = (int *) ptr;    
                i = cp.data_cols * cp.data_rows;
                while(i--) { *temp_ptr++ =  *int_ptr++; };

            break;
            case RASCON_FLOAT: 
		float_ptr = (float *) ptr;
                i = cp.data_cols * cp.data_rows;
                while(i--) { *temp_ptr++ =  *float_ptr++; };

            break;
            case RASCON_DOUBLE:
              dbl_ptr = (double *) ptr;    
              memcpy((void *) temp_ptr,(void*) dbl_ptr,
		      cp.data_cols * cp.data_rows * sizeof(double));
            break;
    }

    return temp;

}

/*******************************************************************************
 * SMOOTH_PLANE : Apply smoothing to the given data array.
 *   This version does a applys 3x3 weighted smoothing function:
 *                 0 1 0
 *                 1 3 1
 *                 0 1 0
 */

void smooth_plane(double *data, int nx, int ny)
{
    int i,j;
    double  *temp,*temp_ptr;

    double value, divisor;
    double  *row1, *row2, *row3;

    if((temp  = (double *) calloc(nx * ny, sizeof(double))) == NULL) {
	perror("RASCON Can't allocate memory for smooth plane \n");
	exit(-1);
    }
    temp_ptr = temp;

    /* Apply the smoothing function */
    for(j = 0 ; j < ny ; j++) {

      /* set up pointers to each row of data */
      if(j == 0) { /* First row case - Top is duplicated */
	  row1 = data + (nx * j);
	  row2 = data + (nx * j);
	  row3 = data + (nx * (j+1));

      } else if( j == ny -1) { /* last row case - Bottom is duplicated */
	  row1 = data + (nx * (j-1));
	  row2 = data + (nx * j);
	  row3 = data + (nx * j);

      } else { /* Internal Rows */
	  row1 = data + (nx * (j-1));
	  row2 = data + (nx * j);
	  row3 = data + (nx * (j+1));
      }

      for(i=0; i < nx; i++) {

	divisor = 7.0;
        if(i == 0) { /* First column special case */
	  if(row2[i] != cp.miss_val) {
            value = 0.0; 
            value += row2[i] * 3.0; /* center */
	    if( row2[i+1] != cp.miss_val) {
                value += row2[i] + row2[i+1]; /* center & right */
	    } else {
		value += row2[i];
	        divisor -= 1.0;
	    }
	    if( row3[i] != cp.miss_val) {
                value +=  row3[i]; /*  Down */
	    } else {
	        divisor -= 1.0;
	    }
	    if( row1[i] != cp.miss_val) {
                value += row1[i]; /* Top  */
	    } else {
	        divisor -= 1.0;
	    }
            value /= divisor; 
	    *temp_ptr++ = value;
	  } else {
	    *temp_ptr++ = cp.miss_val;
	  }

        } else if (i == nx -1) { /* last column special case */

	  if(row2[i] != cp.miss_val) {
            value = 0.0; 
            value += row2[i] * 3.0; /* center */
	    if( row2[i-1] != cp.miss_val) {
              value += row2[i-1] + row2[i]; /* left & center */
	    } else {
		value += row2[i];
	        divisor -= 1.0;
	    }
	    if( row3[i] != cp.miss_val) {
              value += row3[i]; /* Down */
	    } else {
	        divisor -= 1.0;
	    }
	    if( row1[i] != cp.miss_val) {
              value += row1[i]; /* Top  */
	    } else {
	        divisor -= 1.0;
	    }
            value /= divisor; 
	    *temp_ptr++ = value;
	  } else {
	    *temp_ptr++ = cp.miss_val;
	  }


         } else { /* regular case */

	  if(row2[i] != cp.miss_val) {
            value = 0.0; 
            value += row2[i] * 3.0; /* center */
	    if( row2[i-1] != cp.miss_val) {
              value += row2[i-1]; /* left  */
	    } else {
	        divisor -= 1.0;
	    }
	    if( row2[i+1] != cp.miss_val) {
              value += row2[i+1]; /* right */
	    } else {
	        divisor -= 1.0;
	    }
	    if( row1[i] != cp.miss_val) {
              value += row1[i]; /* Top  */
	    } else {
	        divisor -= 1.0;
	    }
	    if( row3[i] != cp.miss_val) {
              value += row3[i]; /* Down */
	    } else {
	        divisor -= 1.0;
	    }
            value /= divisor; 
	    *temp_ptr++ = value;
	  } else {
	    *temp_ptr++ = cp.miss_val;
	  }
         }
      }
    }

    /* copy our smoothed data into place */
    memcpy(data,temp,nx * ny * sizeof(double));

    free(temp); /* Free temporary work space */
}

/*******************************************************************************
 * FillRows : Fill the two raster arrays with values from the data array,
 *    in the case of a full grid, otherwise copy the data out of the two supplied data rows
 *
 */

static int FillRows(int row)
{
    int    i,j;
    double    *temp;            /* temp pointer to contouring rows */
    caddr_t    ptr;
    unsigned char    *char_ptr;
    unsigned short  *short_ptr;
    int     *int_ptr;
    float   *float_ptr;
    double  *dbl_ptr;

    
  if(cp.fullgrid) {

    if(cp.do_extras & RASCON_DOSMOOTH) { /* Pull data from out smothed array */
      if(row > 0) row++;
      do {
        temp = cp.row1;      /* switch pointers to row1 and row2    */
        cp.row1 = cp.row2;   /* this allows us to read/copy a data row only once */
        cp.row2 = temp;

	/* set point to beginning of plane */
        dbl_ptr = (double *) cp.sm_plane;    
        dbl_ptr += cp.data_cols * row;        
        memcpy((void *) temp,(void*) dbl_ptr,cp.data_cols * sizeof(double));

        row++;
      } while(row <= 1);

    } else { /* No Smoothing - Gather two rows from raw data plane */

      if(row > 0) row++;
      do {
        temp = cp.row1;        /* switch pointers to row1 and row2    */
        cp.row1 = cp.row2;        /* this allows us to read/copy a data row only once */
        cp.row2 = temp;

        ptr = cp.data_ptr;
        i= cp.data_cols;
        switch(cp.data_type) {
            case RASCON_CHAR: char_ptr = (unsigned char *) ptr;    
                char_ptr += cp.data_cols * row;
                while(i--) {
                    *temp++ =  *char_ptr++;
                };
            break;
            case RASCON_SHORT: short_ptr = (unsigned short *) ptr;    
                short_ptr += cp.data_cols * row;
                while(i--) {
                    *temp++ =  *short_ptr++;
                };
                
            break;
            case RASCON_INT: int_ptr = (int *) ptr;    
                int_ptr += cp.data_cols * row;
                while(i--) {
                    *temp++ =  *int_ptr++;
                };
                
            break;
            case RASCON_FLOAT: float_ptr = (float *) ptr;    
                float_ptr += cp.data_cols * row;
                while(i--) {
                    *temp++ =  *float_ptr++;
                };
                
            break;
            case RASCON_DOUBLE: dbl_ptr = (double *) ptr;    
                dbl_ptr += cp.data_cols * row;        
                memcpy((void *) temp,(void*) dbl_ptr,cp.data_cols * sizeof(double));
            break;
        }
        row++;
      } while(row <= 1);
    }

  } else {  /* 2 ROW Contouring */
    for(j=0; j < 2; j++) {
     
        if (j == 0) {
	    ptr = cp.r1_ptr; 
	    temp = cp.row1;
	} else {
	    ptr = cp.r2_ptr;
	    temp = cp.row2;
	}
        i= cp.data_cols;
        switch(cp.data_type) {
            case RASCON_CHAR: char_ptr = (unsigned char *) ptr;    
                while(i--) {
                    *temp++ =  *char_ptr++;
                };
            break;
            case RASCON_SHORT: short_ptr = (unsigned short *) ptr;    
                while(i--) {
                    *temp++ =  *short_ptr++;
                };
                
            break;
            case RASCON_INT: int_ptr = (int *) ptr;    
                while(i--) {
                    *temp++ =  *int_ptr++;
                };
                
            break;
            case RASCON_FLOAT: float_ptr = (float *) ptr;    
                while(i--) {
                    *temp++ =  *float_ptr++;
                };
                
            break;
            case RASCON_DOUBLE: dbl_ptr = (double *) ptr;    
                memcpy((void *) temp,(void *) dbl_ptr,cp.data_cols * sizeof(double));
            break;
        }
    }
  }
  return(0);
}

/******************************************************************************
 * DoRow :  contour one row of cells
 *          Return -1 on error.
 */

static int DoRow(int row)
{
  int    i,status;

  for(i = 0; i < cp.n_cols; i++) {    /* do all of the columns    */
    cp.point0 =  (double)cp.row2[i+1];
    cp.point1 =  (double)cp.row2[i];
    cp.point2 =  (double)cp.row1[i];
    cp.point3 =  (double)cp.row1[i+1];

    /*  DEBUG 
    if(row == i || row == 0) 
	fprintf(stderr,"Row %d, col %d Points: %g, %g, %g, %g\n",
	    row,i,cp.point0,cp.point1,cp.point2,cp.point3);
    */

    /* Skip this cell if any of the points are "BAD" */
    if((cp.point1 != cp.miss_val) && (cp.point2 != cp.miss_val) && 
       (cp.point0 != cp.miss_val) && (cp.point3 != cp.miss_val)) {
  
      switch(cp.fill_flag) {
          case(RASCON_LINE_CONTOURS):
              status = DoBox(row,i);    /* contour one box */
          break;

          case(RASCON_FILLED_CONTOURS):
              status = DoBoxFilled(row, i);
          break;
      }
      if(status) return(-1);
    }
  }
  return(0);
}

/*******************************************************************************
 * DoBox : Compute all contour lines through box - call line drawing routines
 *         Return -1 on error.
 *
 */

static int    path[16][8] = {    /* All Possible paths through each box */
    { 0,0,0,0,0,0,0,0 },
    { 1,5,6,7,4,0,0,0 },
    { 1,8,7,6,2,0,0,0 },    /*     LINE NUMBERS        */
    { 2,6,7,4,0,0,0,0 },    /*     -----3------     */
    { 2,5,8,7,3,0,0,0 },    /*     |\        /|     */
    { 2,5,1,-1,3,7,4,0 },   /*     | 6      7 |     */
    { 3,7,8,1,0,0,0,0 },    /*     2  \    /  4     */
    { 3,7,4,0,0,0,0,0 },    /*     |  /   \   |     */
    { 3,6,5,8,4,0,0,0 },    /*     | 5     8  |     */
    { 3,6,5,1,0,0,0,0 },    /*     |/       \ |     */
    { 2,6,3,-1,1,8,4,0 },   /*     -----1-----      */
    { 2,6,3,0,0,0,0,0 },
    { 2,5,8,4,0,0,0,0 },    /*     POINT NUMBERS    */
    { 2,5,1,0,0,0,0,0 },    /*      2        3        */
    { 1,8,4,0,0,0,0,0 },    /*          4            */
    { 0,0,0,0,0,0,0,0 }     /*      1        0        */
};

static int DoBox(int row, int col)
{
    int    i,index; /* index into array above */
    GC      gc; /* GC to use for this level */
    double    level; /* contour level */
    int    *pth; /* path (see diagram above */
    int    status; /* return status */
    int     lpt;  /* label point */

    /* average corner points to get  value for fifth point */
    cp.point4 = (cp.point0 + cp.point1 + cp.point2 + cp.point3) / 4.0;

    for( i = 0; i < cp.num_levels; i++) {     /* do all cont levels */
        index = 0;            /* reset index */
        status = 0;
        gc = cp.gc_level[i];

        level = cp.con_level[i];
         
        /* Determine path through cell */
        if(cp.point0 >= level) index |= 1;
        if(cp.point1 >= level) index |= 2;
        if(cp.point2 >= level) index |= 4;
        if(cp.point3 >= level) index |= 8;
        if(cp.point4 <= level) index = ~index & 0x0000000f;

        pth = path[index];
         
        if(*pth != 0 ) {
            /* move to beginning of line - pen up */
             status = DoLinePath(*pth,level,gc,row,col,0);
        }

        pth++;

        while(*pth != 0 ) {
            if(*pth == -1 ) { /* is a dual path box */
                 pth++;
                 /* move to beg of 2nd line - pen up */
                  status = DoLinePath(*pth++,level,gc,row,col,0);
            }

            /* Move to end of line - pen down */
             status = DoLinePath(*pth++,level,gc,row,col,1);
        }
        if(status) return(-1);

        /* do labeling on top of contour line */
        if ((cp.do_extras & RASCON_DOLABELS) && (*path[index] != 0)) {

          lpt = NeedLabel(path[index], i, col, row, cp.data_cols-1, cp.data_rows-1);
          if (lpt > 0) {
	    /* PlotLabel(cp.dpy, cp.xid, lpt, col, row, level, */
	    /* cp.label_scale, cp.label_bias, gc); */
	    AddLabel(lpt, col, row, level, cp.label_scale, cp.label_bias, gc);
	  }
        }
    }
    return(0);
}
 
/*******************************************************************************
 * DoBoxFilled : Compute all contour lines through box.
 *                 Call polygon fill drawing routines.
 *                 Return -1 on error.
 *
 */

static int    fpath[33][10] = {    /* All Possible paths through each box */
    /* FILL INTERNAL - CONVEX PATHS */
    { 0,0,0,0,0,0,0,0,0,0 },       
    { 1,5,6,7,4,9,0,0,0,0 },
    { 1,8,7,6,2,10,0,0,0,0 },       /*     LINE NUMBERS     */
    { 2,6,7,4,9,10,0,0,0,0 },       /*   11-----3------12   */
    { 2,5,8,7,3,11,0,0,0,0 },       /*     |\        /|     */
    { 2,5,1,9,4,7,3,11,0,0 },       /*     | 6      7 |     */
    { 3,7,8,1,10,11,0,0,0,0 },      /*     2  \    /  4     */
    { 3,7,4,9,10,11,0,0,0,0 },      /*     |  /   \   |     */
    { 3,6,5,8,4,12,0,0,0,0 },       /*     | 5     8  |     */
    { 3,6,5,1,9,12,0,0,0,0 },       /*     |/       \ |     */
    { 2,6,3,12,4,8,1,10,0,0 },      /*    10----1-----9     */
    { 2,6,3,12,9,10,0,0,0,0 },
    { 2,5,8,4,12,11,0,0,0,0 },      /*     POINT NUMBERS    */
    { 2,5,1,9,12,11,0,0,0,0 },      /*      2        3      */
    { 1,8,4,12,11,10,0,0,0,0 },     /*          4           */
    { 0,0,0,0,0,0,0,0,0,0 },     
    /* FILL "EXTERNAL"  - CONCAVE PATHS */
    { 0,0,0,0,0,0,0,0,0,0 },
    { 1,5,6,7,4,12,11,10,0,0 },
    { 1,8,7,6,2,11,12,9,0,0 },
    { 2,6,7,4,12,11,0,0,0,0 },
    { 2,5,8,7,3,12,9,10,0,0 },
    { 2,5,1,10,-1,3,7,4,12,0 },
    { 3,7,8,1,9,12,0,0,0,0 },
    { 3,7,4,12,0,0,0,0,0,0 },
    { 3,6,5,8,4,9,10,11,0,0 },
    { 3,6,5,1,10,11,0,0,0,0 },
    { 2,6,3,11,-1,1,8,4,9,0 },
    { 2,6,3,11,0,0,0,0,0,0 },
    { 2,5,8,4,9,10,0,0,0,0 },
    { 2,5,1,10,0,0,0,0,0,0 },
    { 1,8,4,9,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0 },       /* fill none */
    { 9,10,11,12,0,0,0,0,0,0}      /* fill all */  
};


static int DoBoxFilled(int row, int col)
{
  int    i,index;
  double    level;
  int    *pth;
  int    status;
  int    last_index;
  GC    gc;

  /* average corner points to get  value for fifth point */
  cp.point4 = (cp.point0 + cp.point1 + cp.point2 + cp.point3) / 4.0;

  last_index = 0;
  for( i = 0; i < cp.num_levels; i++) {     /* do all cont levels */
    index = 0;            /* reset index */
    status = 0;

    level = cp.con_level[i];
    gc = cp.gc_level[i];
 
    /* Determine path through cell */
    if(cp.point0 >= level) index |= 1;
    if(cp.point1 >= level) index |= 2;
    if(cp.point2 >= level) index |= 4;
    if(cp.point3 >= level) index |= 8;
    if(cp.point4 < level)  index = (~index & 0x000f) + 16; 

    if(last_index == 15) { /* All points were above last contour level */
	if(index == 31) {  /* Now all points are below level -  Whole cell needs filled */
	    index = 32;
	    gc = cp.gc_level[i-1];
            pth = fpath[index];
            status = DoPathFilled(pth,level,gc,row,col);
	} else {           /* Need to fill both sides of cell */
            pth = fpath[index];

	    /* Fill section of cell above the contour level */
	    if(*pth)status = DoPathFilled(pth,level,gc,row,col);


	    /* Fill section of cell below the contour level */
 	    pth = fpath[((index + 16)  & 0x001f)]; /*  */
 	    gc = cp.gc_level[i-1]; /*  */
 	    if(*pth)status = DoPathFilled(pth,level,gc,row,col); /*  */
	}
    } else {
     pth = fpath[index];
     if(*pth)status = DoPathFilled(pth,level,gc,row,col); /*  */
    }
     
    last_index = index;

    if(status) return(-1);
  }
  return(0);
}
 
/*******************************************************************************
 * Lterp : Returns fractional distance from point 1
 *
 */

static double Lterp(double pt1, double pt2, double level)
{
    if((pt2 - pt1) == 0.0) return(0.5);
    return (double)(level - pt1) / (double)(pt2 - pt1);
}

/*******************************************************************************
 * CalcLinePath : Do appropriate linear interpolation between points of line.
 *                Return -1 on error.
 *
 */

static int CalcLinePath(int linenum, double level,double *xcoord, double *ycoord)
{
    double    fract;    
    double    pt5 = 0.5;
    
    switch(linenum) {
        
        case 1:    fract = Lterp(cp.point1,cp.point0,level);
            *xcoord = fract;
            *ycoord = 1.0;
        break;

        case 2:    fract = Lterp(cp.point2,cp.point1,level);
            *xcoord = 0.0;
            *ycoord = fract;
        break;

        case 3:    fract = Lterp(cp.point2,cp.point3,level);
            *xcoord = fract;
            *ycoord = 0.0;
        break;

        case 4:    fract = Lterp(cp.point3,cp.point0,level);
            *xcoord = 1.0;
            *ycoord = fract;
        break;

        case 5:    fract = Lterp(cp.point4,cp.point1,level);
            *xcoord = pt5 - (pt5 * fract);
            *ycoord = pt5 + (pt5 * fract);
        break;

        case 6:    fract = Lterp(cp.point2,cp.point4,level);
            *xcoord = pt5 * fract;
            *ycoord = pt5 * fract;
        break;

        case 7:    fract = Lterp(cp.point4,cp.point3,level);
            *xcoord = pt5 + (pt5 * fract);
            *ycoord = pt5 - (pt5 * fract);
        break;

        case 8:    fract = Lterp(cp.point4,cp.point0,level);
            *xcoord = pt5 + (pt5 * fract);
            *ycoord = pt5 + (pt5 * fract);
        break;

        case 9: *xcoord = (double)1.0;
            *ycoord = (double)1.0;
        break;

        case 10:*xcoord = (double)0.0;
            *ycoord = (double)1.0;
        break;

        case 11:*xcoord = (double)0.0;
            *ycoord = (double)0.0;
        break;

        case 12:*xcoord = (double)1.0;
            *ycoord = (double)0.0;
        break;
    }
    return(1);
}

/*******************************************************************************
 * DoLinePath : Do appropriate linear interpolation between points of line.
 *              Return -1 on error.
 *
 */

static int DoLinePath(int linenum, double level,GC gc,int row,int col,int pen)
{
    double    xcoord,ycoord;
    int    status;

    status = CalcLinePath(linenum, level, &xcoord, &ycoord);
    
    status = PlotLine(xcoord,ycoord,gc,col,row,pen);

    if(status) return(-1);
    return(0);
}

/*******************************************************************************
 * DoPathFilled :Follow path and build polygon vertices - then plot filled
 *                         Return -1 on error.
 */

static int DoPathFilled(int *path,double level,GC gc,int row,int col)
{
    int status;
    int npts = 0;
    double    xcoord,ycoord;
    double pgon[MAX_PTS][2];

    /* Follow path and build polygon vertices - then plot filled */
    while(*path != 0) {
	if(*path == -1) {  /* Is a dual path box; fill first polygon */

          /* draw first polygon */
          status = PlotFilled(pgon, npts, gc, col, row);
	  npts = 0;

	} else {  /* Compute coords for the vertex on this line segment segment */

            status = CalcLinePath(*path, level, &xcoord, &ycoord);
    
            pgon[npts][0] = xcoord;
            pgon[npts][1] = ycoord;
            npts++;
        }
	path++;
    }
   /* draw remaining polygon */
   status = PlotFilled(pgon, npts, gc, col, row);
      
   if(status) return(-1);
   return(0);
}
 
/*------------------------------------------------------------------------*/
static int NeedLabel(int *ppath, int level_index, int colnum, int rownum, int ncols,int nrows)
/*
 * Determine whether or not to label a particular contour line.
 * Returns the "path line number" (see diagram below) 1-8 of where
 * to label line. Returns 0 if line should not be labeled.
 *
 * Label contour lines in a box if:
 *   - in the top row and line exits the box to the top
 *   - in the bottom row and line exits the box to the bottom
 *   - in the first box of a row and line exits to the left
 *   - in the last box of a row and line exits to the right
 *   - whenever the level changes or after MAXBEFORELABEL boxes with
 *     the same level have been drawn
 *
 * All Possible paths through each box:
 *
 *     -----3------ 
 *     |\        /|
 *     | 6      7 | 
 *     2  \    /  4 
 *     |  /   \   | 
 *     | 5     8  |
 *     |/       \ |
 *     -----1----- 
 */
{
  int i;
  int do_label = 0;  /* flag to label this line, true or false */
  int label_point = 0; /* point (1-8) from diagram above where label should be placed */
  int *ptr;
  static int maxcols_b4label = -1; /* number of columns between labels */
  static int skip = -1;
  static int last_row = -1;
  static int last_col = -1;
  static int last_col_level[MAXLEVELS];

  skip = nrows / NUMROWS_OF_LABELS;
  if(skip == 0) skip = 1;
  /* set the maximum number of columns between labels, this depends on grid size */
  maxcols_b4label = ncols / NUMCOLS_OF_LABELS + ((cp.x_start[ncols-1] - cp.x_start[0]) / (ncols * 10)) ;    
  if(maxcols_b4label < 2) maxcols_b4label = 2;

  /* exit if no path through box */
  if (*ppath <= 0) return(0);

  /* If a label has already been placed in this grid cell - return */
  if(rownum == last_row && colnum == last_col) return(0);

  /* return if not a multiple of the skip count row (except for the first and last row)*/
/*   if ((rownum != nrows) && (rownum != 0) && ((rownum % skip) != 0)) return(0);  */
   if ((rownum != nrows) && (rownum != 0) && (((rownum + (random()&ncols)) % skip) != 0)) return(0);    /*  */

  /* reset counters at start of each row */
  if ((colnum == 0) || (rownum != last_row)) {
    for (i=0; i < cp.num_levels; i++) last_col_level[i] = 1 - maxcols_b4label;
  }

  ptr = ppath;
  /* first row of full grid contour */
  if (rownum == 0 && cp.fullgrid) {
    while(*ptr > 0) {
      /* label at point 6 or 7 */
      switch(*ptr) {
        case(3):do_label = 1;
	break;

        case(6):
        case(7):
              label_point = *ptr;
        break;
      }
      ptr++;
    } 
  } else if (rownum == nrows && cp.fullgrid) { /* last row  of full grid contour*/
    while(*ptr > 0) {
      switch(*ptr) {
        case(1):do_label = 1;
	break;

        case(5): /* label at point 8 or 5 */
        case(8):
              label_point = *ptr;
        break;
      }
      ptr++;
    }
  }

  ptr = ppath;
  /* first box in row */
  if (colnum == 0) {
    while(*ptr > 0) {
      switch(*ptr) {
        case(2):do_label = 1;
	break;

        case(5): /* label at point 6 or 5 */
        case(6):
              label_point = *ptr;
        break;
      }
      ptr++;
    }
  } else if (colnum == ncols) { /* last box in row */
    while(*ptr > 0) {
      switch(*ptr) {
        case(4):do_label = 1;
	break;

        case(7): /* label at point 7 or 8 */
        case(8):
              label_point = *ptr;
        break;
      }
      ptr++;
    }
  }

  /* an interior box */
  ptr = ppath;
  if (!do_label) {
    /* have MAXBEFORELABEL boxes with the same level gone by? OR has the level changed? */
    if (colnum - last_col_level[level_index] >= maxcols_b4label) {
      do_label = 1;

      while(*ptr > 0) {
        switch(*ptr) {
          case(5): 
          case(6):
          case(7):
          case(8):
              label_point = *ptr;
          break;
        }
        ptr++;
      }
    }
  } 

  /* update counters */

  last_row = rownum;
  last_col = colnum;

  if (do_label) {
    last_col_level[level_index] = colnum;

    return(label_point);
  }
  else return(0);
}
 
/*------------------------------------------------------------------------*/
#ifdef NOTNOW
static int PlotLabel(Display *disp, Drawable dw, int label_point, int col, int row,
	double level, double label_scale, double label_bias, GC gc)

/*
 * Label the contour line. Return 1 if able to do label. Return 0 on error.
 */
{
  char string[MAXCHARS];
  int len; /* length of string (number of chars) */
  int width; /* width of string (pixels) */
  int okay; 
  int ilevel;
  int pix_x, pix_y; /* pixel location to draw label */
  double dlevel;
  double x, y; /* interpolated x,y location to draw label */
  
  /* scale level by label factor" */
  if (cp.do_extras & RASCON_DATA_IS_LOG) {
    dlevel = exp(level * label_scale + label_bias);
  } else {
    dlevel = level * label_scale + label_bias;
  }

  /* convert level number to string */
  if(cp.dynamic_range < 2.0) {
     sprintf(string, "%.2f", dlevel);
  } else {
      ilevel =  irint(dlevel);
    sprintf(string, "%d", ilevel);
  }
  len = strlen(string);

#ifdef NOT_NOW
  /* determine string extent (in pixels) */
  xfont = XQueryFont(disp, (XGContextFromGC(gc)));
  width = XTextWidth(xfont, string, len);
#else 
  width = 8 * len;
#endif

  /* convert label_point to an x,y interpolated location */
  okay = CalcLinePath(label_point, level, &x, &y);
  if (!okay) {
    printf("DoLabel: cannot convert label_point:%d to pixels\n", label_point);
    return(0);
  }

  /* map (x,y) to screen coordinates */
  if (cp.fullgrid) {
    pix_x  = (cp.x_start[col] + (cp.x_delta[col] * x) + 0.5);
    pix_y  = (cp.y_start[row] + (cp.y_delta[row] * y) + 0.5);
  } else  {
      CalcPixPos2Rows(x, y, col, &pix_x, &pix_y);
  }

  /* shift to center the string */
  pix_x = pix_x - (int) (width/2.0);

  /* draw label */
  XDrawImageString(disp, dw, gc, pix_x, pix_y, string, len);
  /* XDrawString(disp, dw, gc, pix_x, pix_y, string, len); */

  return(1);
}
#endif

/*------------------------------------------------------------------------*/

static void PlotLabelsFromBuf(Display *disp, Drawable dw)

/*
 * Label the contour line.
 */

{

  int i;
  int len; /* length of string (number of chars) */
  int height; /* height of string (pixels) */
  int width; /* width of string (pixels) */
  int nlabels;
  label_entry_t *entry = (label_entry_t *) MEMbufPtr(cp.labelbuf);
  XFontStruct *xfont;
  
  nlabels = MEMbufLen(cp.labelbuf) / sizeof(label_entry_t);

  if (nlabels < 1) {
    return;
  }
  
  xfont = XQueryFont(disp, (XGContextFromGC(entry->gc)));
  
  for (i = 0; i < nlabels; i++, entry++) {
    
    len = strlen(entry->text);
    height = xfont->ascent;
    width = XTextWidth(xfont, entry->text, len);
    
    /* draw label */
    XDrawImageString(disp, dw, entry->gc,
		     entry->pix_x - width/2, entry->pix_y + height/2,
		     entry->text, len);

  } /* i */

}

/*------------------------------------------------------------------------*/

static int AddLabel(int label_point, int col, int row,
		    double level, double label_scale, double label_bias, GC gc)
     
/*
 * Add a label to the labelbuf
 */

{

  label_entry_t entry;
  int okay; 
  int pix_x, pix_y; /* pixel location to draw label */
  double dlevel;
  double x, y; /* interpolated x,y location to draw label */
  
  /* scale level by label factor" */

  if (cp.do_extras & RASCON_DATA_IS_LOG) {
    dlevel = exp(level * label_scale + label_bias);
  } else {
    dlevel = level * label_scale + label_bias;
  }

  /* convert level number to string */
  if(cp.dynamic_range < 2.0) {
    sprintf(entry.text, "%.2f", dlevel);
  } else {
    int ilevel = irint(dlevel);
    sprintf(entry.text, "%d", ilevel);
  }

  /* convert label_point to an x,y interpolated location */
  
  okay = CalcLinePath(label_point, level, &x, &y);
  if (!okay) {
    printf("AddLabel: cannot convert label_point:%d to pixels\n", label_point);
    return(0);
  }
  
  /* map (x,y) to screen coordinates */
  if (cp.fullgrid) {
    pix_x  = (cp.x_start[col] + (cp.x_delta[col] * x) + 0.5);
    pix_y  = (cp.y_start[row] + (cp.y_delta[row] * y) + 0.5);
  } else  {
    CalcPixPos2Rows(x, y, col, &pix_x, &pix_y);
  }

  /* fill struct */

  entry.pix_x = pix_x;
  entry.pix_y = pix_y;
  entry.gc = gc;
  
  /* add it to the buffer */

  MEMbufAdd(cp.labelbuf, &entry, sizeof(entry));

  return(1);
}

/********************************* END OF FILE *********************************/
