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

/**************************************************************************
 * ascii_to_xgraph.h - header file for ascii_to_xgraph program
 *
 * Mike Dixon RAP NCAR Boulder CO USA
 *
 * March 1991
 *
 **************************************************************************/

/*
 * includes
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>

/*
 * defines
 */

#define XGRAPH_PROLOGUE_PATH "ascii_to_xgraph.xgraph"
#define ACEGR_PROLOGUE_PATH "ascii_to_xgraph.acegr"
#define PRINTER_NAME "lp"
#define LOG_FILE_PATH "null"

#define TITLE "ASCII_TO_XGRAPH UTILITY"

#define XLABEL "null-x"
#define YLABEL "null-y"

#define SCATTER_MODE 0
#define HIST_MODE 1
#define LOG_HIST_MODE 2
#define PERCENTILE_MODE 3

#define PERCENTILES "25 50 75"
#define MIN_POINTS_FOR_PERCENTILE 10L

#define MODE "scatter"
#define CLIENT "xgraph"

#define PERFORM_ATTRITION "false"
#define ATTRITION_COUNT 10L

#define XGRAPH 0
#define ACEGR 1

#define DATA_FIT "false"
#define DATA_POLYNOMIAL 1
#define DATA_EXPONENTIAL 2
#define POLYNOMIAL_ORDER 2L

#define HIST_FIT "false"
#define HIST_NORMAL 1
#define HIST_EXPONENTIAL 2
#define HIST_GAMMA 3
#define HIST_WEIBULL 4
#define HIST_FIT_PARAM1 1.0
#define HIST_BAR "true"

#define LIMITX_DATA "false"
#define LIMITY_DATA "false"
#define LIMITX_AXIS "false"
#define LIMITY_AXIS "false"
#define LOWX 0.0
#define HIGHX 1.0
#define LOWY 0.0
#define HIGHY 1.0

#define MULTX 1.0
#define MULTY 1.0

#define LOGX "false"
#define LOGY "false"

#define PLOT_LOGX "false"
#define PLOT_LOGY "false"

#define SET_X_INTERVALS "false"
#define N_X_INTERVALS 7L
#define MINX 0.0
#define DELTAX 1.0

#define CONDITIONAL_DATA "false"
#define COND_LABEL "null"
#define COND_LOW 0.0
#define COND_HIGH 1.0

/*
 * global struct
 */

typedef struct {

  char *prog_name;            /* program name */

  char *params_path_name;     /* params file path name */

  char *title;                /* the line to appear as the title to
			       * the graph */

  char *xgraph_prologue_path; /* name of xgraph prologue file */
  char *acegr_prologue_path;  /* name of acegr prologue file */

  char *printer_name;

  char *log_file_path;

  int debug;

  int mode;                   /* SCATTER_MODE,
			       * HIST_MODE,
			       * LOG_HIST_MODE or
			       * PERCENTILE_MODE */

  int client;                 /* XGRAPH or ACEGR */

  int data_fit;               /* FALSE, DATA_POLYNOMIAL or DATA_EXPONENTIAL */

  int polynomial_order;       /* 1 = LINEAR, 2 = QUADRATIC,
			       * 3 = CUBIC etc. */

  int hist_fit;               /* FALSE, HIST_NORMAL, HIST_EXPONENTIAL,
			       * HIST_GAMMA or HIST_WEIBULL */

  int hist_bar;               /* TRUE or FALSE. If FALSE, points are used */

  double hist_fit_param1;     /* first param for hist fit distribution */

  int limitx_data;            /* TRUE or FALSE */
  int limity_data;            /* TRUE or FALSE */
  int limitx_axis;            /* TRUE or FALSE */
  int limity_axis;            /* TRUE or FALSE */
  int logx;                   /* TRUE or FALSE */
  int logy;                   /* TRUE or FALSE */
  int plot_logx;              /* TRUE or FALSE */
  int plot_logy;              /* TRUE or FALSE */
  int set_x_intervals;        /* TRUE or FALSE */
  int conditional_data;       /* TRUE or FALSE */
  int perform_attrition;      /* TRUE or FALSE */

  int attrition_count;

  char *xlabel;               /* label of x val in data */
  char *ylabel;               /* label of y val in data */
  char *cond_label;           /* label for conditional data */

  int n_x_intervals;          /* number of hist intervals */

  double multx, multy;
  double lowx, highx;
  double lowy, highy;
  double cond_low, cond_high;

  double minx, deltax;

  int n_percentiles;
  int min_points_for_percentile;
  double *percentiles;

} global_t;

/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN

global_t *Glob = NULL;

#else

extern global_t *Glob;

#endif

/*
 * function prototypes
 */

extern int filter(FILE *log_file);
extern void parse_args(int argc, char **argv);
extern void read_params(void);
#ifdef __cplusplus
}
#endif
