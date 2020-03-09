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
/***************************************************************************
 * filter.c : performs the filtering from stdin to stdout
 *
 * Mike Dixon RAP NCAR Jan 1992
 *
 * returns 0 on success, -1 on failure
 *
 ***************************************************************************/

#include "ascii_to_xgraph.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define SAFELOG(a) ((a) <= 0.0 ? -10.0 : log(a))
#define EXP(a) (exp(a))

#define LABEL_MAX 64
#define LINE_MAX 4096

extern double gamma_(double);

static int compare_y_coords(const void *, const void *);

static int compute_percentiles(double minx,
			       double deltax,
			       double *x,
			       double *y,
			       int npoints,
			       int *n_bin_points,
			       double **percentile_vals);

static int compute_stats(double minx,
			 double deltax,
			 double hist_min,
			 double *x,
			 double *y,
			 int npoints,
			 double *xmean_p,
			 double *ymean_p,
			 double *xsdev_p,
			 double *ysdev_p,
			 double *chi_sq_p,
			 double *binx,
			 double *biny,
			 double *expected_y,
			 int *n_observed,
			 FILE *log_file);

static int copy_prologue(void);

static int draw_fit(int nfit,
		    double *xfit,
		    double *yfit,
		    double *aa,
		    double *binx,
		    FILE *log_file);

static int load_xy_data(int xpos,
			int ypos,
			int cond_pos,
			double **x_p,
			double **y_p,
			double *minx_p,
			double *maxx_p,
			double *miny_p,
			double *maxy_p,
			int *npoints_p);

static void print_acegraph_hdrs(char *xlabel,
				char *ylabel);

static void print_xgraph_hdrs(char *xlabel,
			      char *ylabel,
			      double deltax);

static int read_labels(int *xpos,
		       int *ypos,
		       int *cond_pos,
		       char *xlabel,
		       char *ylabel,
		       char *cond_label,
		       FILE* log_file);

int filter(FILE *log_file)

{
  
  char xlabel[LABEL_MAX], ylabel[LABEL_MAX], cond_label[LABEL_MAX];

  int i, ibin, iperc;
  int npoints = 0;
  int xpos, ypos, cond_pos;
  int nfit;
  int *n_observed;

  double xmean = 0, ymean = 0;
  double xsdev = 0, ysdev = 0;
  double chi_sq = 0;
  double minx, maxx;
  double miny, maxy;
  double rangex, deltax;
  double hist_min = 0;
  double *binx, *biny;
  double *expected_y;
  double *xfit, *yfit;
  double *x, *y, *xp, *yp;
  double *aa = NULL;
  double **percentile_vals = NULL;

  /*
   * set debugging
   */

  if (Glob->data_fit) {
    uCurveFitDebug(Glob->debug);
  }

  /*
   * copy the prologue file
   */

  if (copy_prologue())
    return (-1);
  
  /*
   * alloc memory for polynomial coefficients
   */
  
  if (Glob->data_fit == DATA_POLYNOMIAL) {
    aa = (double *)
      umalloc ((Glob->polynomial_order + 1) * sizeof(double));
  } else if (Glob->data_fit == DATA_EXPONENTIAL) {
    aa = (double *)
      umalloc (3 * sizeof(double));;
  }
  
  /*
   * read the axis labels
   */
  
  if (read_labels(&xpos, &ypos, &cond_pos,
		  xlabel, ylabel, cond_label,
		  log_file)) {
    if (aa != NULL)
      ufree((char *) aa);
    return (-1);
  }
      
  /*
   * read in data, load up linked list
   */

  if (load_xy_data(xpos, ypos, cond_pos,
		   &x, &y,
		   &minx, &maxx,
		   &miny, &maxy,
		   &npoints)) {
    if (aa != NULL)
      ufree((char *) aa);
    return (-1);
  }
  
  /*
   * allocate
   */

  binx = (double *) umalloc
    (Glob->n_x_intervals * sizeof(double));
  
  biny = (double *) umalloc
    (Glob->n_x_intervals * sizeof(double));
  
  expected_y = (double *) umalloc
    (Glob->n_x_intervals * sizeof(double));
  
  n_observed = (int *) ucalloc
    (Glob->n_x_intervals, sizeof(int));
  
  if (Glob->set_x_intervals) {
    
    deltax = Glob->deltax;
    minx = Glob->minx;
    
  } else {

    if (Glob->mode == LOG_HIST_MODE) {

      hist_min = 0.0;
      rangex = SAFELOG(maxx - minx + 1.0);
      deltax = rangex / (double) Glob->n_x_intervals;

    } else {

      hist_min = minx;
      rangex = maxx - minx;
      deltax = rangex / (double) Glob->n_x_intervals;

    }
    
  }
  
  fprintf(stderr, "---> deltax: %g\n", deltax);
  fprintf(stderr, "     minx, maxx: %g, %g\n", minx, maxx);
  fprintf(stderr, "     n_intervals: %d\n", Glob->n_x_intervals);

  compute_stats(minx, deltax,
		hist_min,
		x, y, npoints,
		&xmean, &ymean,
		&xsdev, &ysdev,
		&chi_sq,
		binx, biny,
		expected_y,
		n_observed,
		log_file);
  
  /*
   * in percentile mode, compute the percentile vals
   */

  if (Glob->mode == PERCENTILE_MODE) {

    /*
     * allocate memory
     */

    percentile_vals = (double **) umalloc2
      (Glob->n_percentiles,
       Glob->n_x_intervals,
       sizeof(double));

    compute_percentiles(minx, deltax,
			x, y, npoints,
			n_observed,
			percentile_vals);
      
  } /* if (Glob->mode == PERCENTILE_MODE) */

  /*
   * write stats to log file
   */
  
  fprintf(log_file, "npoints: %ld\n", (long) npoints);
  fprintf(log_file, "%s mean, sdev: %g, %g\n", xlabel, xmean, xsdev);
  fprintf(log_file, "%s minx, maxx: %g, %g\n", xlabel, minx, maxx);

  if (Glob->mode == SCATTER_MODE ||
      Glob->mode == PERCENTILE_MODE) {
    fprintf(log_file, "%s mean, sdev: %g, %g\n", ylabel, ymean, ysdev);
    fprintf(log_file, "%s miny, maxy: %g, %g\n", ylabel, miny, maxy);
  }

  fprintf(stderr, "Chi_sq: %g\n", chi_sq);
  fprintf(log_file, "\n");
  
  /*
   * set up titles etc
   */

  if (Glob->client == XGRAPH)
    print_xgraph_hdrs(xlabel, ylabel, deltax);
  else
    print_acegraph_hdrs(xlabel, ylabel);

  /*
   * write data set
   */

  switch (Glob->mode) {

  case SCATTER_MODE:

    if (Glob->client == XGRAPH)
      fprintf(stdout, "%s", "\n\n\" \"\n");
    
    xp = x;
    yp = y;

    for (i = 0; i < npoints; i++) {

      if (Glob->client == XGRAPH) {
	fprintf(stdout, "move %g %g\n", *xp, *yp);
	fprintf(stdout, "draw %g %g\n", *xp, *yp);
      } else {
	fprintf(stdout, "%g %g\n", *xp, *yp);
	fprintf(stdout, "%g %g\n", *xp, *yp);
      }

      xp++;
      yp++;
      
    } /* i */

    if (Glob->data_fit) {
      fprintf(log_file, "SCATTER PLOT REGRESSION\n");
      draw_fit(npoints, x, y, aa,
	       binx, log_file);
    } /* if (Glob->data_fit) */

    break;

  case HIST_MODE:

    /*
     * put in dummy data sets to get gray hardcopy instead of black - 
     * the first set is printed black
     */

    if (Glob->client == XGRAPH)
      fprintf(stdout, "\n\n\"\"\n");

    for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {
      if (Glob->hist_bar) {
	fprintf(stdout, "%g %g\n", binx[ibin], biny[ibin] * 100.0);
      } else {
	fprintf(stdout, "%g %g\n", binx[ibin], biny[ibin] / deltax);
      }
    }

    if (Glob->hist_fit) {

      /*
       * new data set
       */

      fprintf(stdout, "@    s1 type xy\n");
      fprintf(stdout, "@    s1 symbol 0\n");
      fprintf(stdout, "@    s1 linestyle 2\n");
      fprintf(stdout, "&\n");
      
      for (ibin = 0; ibin < Glob->n_x_intervals; ibin++)
	if (Glob->hist_bar) {
	  fprintf(stdout, "%g %g\n", binx[ibin], expected_y[ibin] * 100.0);
	} else {
	  fprintf(stdout, "%g %g\n", binx[ibin],
		  expected_y[ibin] / deltax);
	}
    }

    break;

  case LOG_HIST_MODE:

    /*
     * put in dummy data sets to get gray hardcopy instead of black - 
     * the first set is printed black
     */

    if (Glob->client == XGRAPH)
      fprintf(stdout, "\n\n\"\"\n");

    for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {
      if (Glob->hist_bar) {
	fprintf(stdout, "%g %g\n", EXP(binx[ibin]) + minx - 1.0,
		biny[ibin] * 100.0);
      } else {
	fprintf(stdout, "%g %g\n", EXP(binx[ibin]) + minx - 1.0,
		biny[ibin] / deltax);
      }
    }

    if (Glob->hist_fit) {

      /*
       * new data set
       */

      fprintf(stdout, "@    s1 type xy\n");
      fprintf(stdout, "@    s1 symbol 0\n");
      fprintf(stdout, "@    s1 linestyle 2\n");
      fprintf(stdout, "&\n");
      
      for (ibin = 0; ibin < Glob->n_x_intervals; ibin++)
	if (Glob->hist_bar) {
	  fprintf(stdout, "%g %g\n", EXP(binx[ibin]) + minx - 1.0,
		  expected_y[ibin] * 100.0);
	} else {
	  fprintf(stdout, "%g %g\n", EXP(binx[ibin]) + minx - 1.0,
		  expected_y[ibin] / deltax);
	}
    }

    break;

  case PERCENTILE_MODE:

    xfit = (double *) umalloc
      (Glob->n_x_intervals * sizeof(double));
    
    yfit = (double *) umalloc
      (Glob->n_x_intervals * sizeof(double));

    for (iperc = 0; iperc < Glob->n_percentiles; iperc++) {

      if (Glob->client == XGRAPH)
	fprintf(stdout, "\n\n\"%gth percentile\"\n",
		Glob->percentiles[iperc]);
      else
	fprintf(stdout, "&\n");

      nfit = 0;

      for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {

	if (n_observed[ibin] > Glob->min_points_for_percentile) {

	  fprintf(stdout, "%g %g\n", binx[ibin],
		  percentile_vals[iperc][ibin]);

	  xfit[nfit] = binx[ibin];
	  yfit[nfit] = percentile_vals[iperc][ibin];
	  nfit++;
	  
	}

      } /* ibin */
      
      if (Glob->data_fit) {
	
	fprintf(log_file,
		"%gTH PERCENTILE REGRESSION\n",
		Glob->percentiles[iperc]);
	
	draw_fit(nfit, xfit, yfit, aa,
		 binx, log_file);

      }
      
    } /* iperc */

    ufree((char *) xfit);
    ufree((char *) yfit);

    break;
    
  } /* switch (Glob->mode)*/

  if (aa != NULL)
    ufree((char *) aa);

  return (0);

}

/***********************************************************************
 * define function to be used for sorting points
 * based on the y coordinate (lowest to highest)
 */

static int compare_y_coords(const void *vv1, const void *vv2)

{

  double yy1 = *((double *) vv1);
  double yy2 = *((double *) vv2);

  double dy;

  dy = yy1 - yy2;

  if (dy > 0)
    return 1;
  else if (dy < 0)
    return -1;
  else
    return 0;

}

/***********************************************************************
 * compute_percentiles()
 *
 * perform percentile calculations
 */

static int compute_percentiles(double minx,
			       double deltax,
			       double *x,
			       double *y,
			       int npoints,
			       int *n_observed,
			       double **percentile_vals)

{
  
  int i, ibin, iperc, ipoint;
  int *n_in_bin;

  double **bin_y;
  
  /*
   * allocate memory
   */
  
  n_in_bin = (int *) ucalloc
    (Glob->n_x_intervals, sizeof(int));
  
  bin_y = (double **) umalloc
    (Glob->n_x_intervals * sizeof(double *));
  
  for (ibin = 0; ibin < Glob->n_x_intervals; ibin++)
    bin_y[ibin] = (double *) umalloc
      (n_observed[ibin] * sizeof(double));
  
  /*
   * loop through the points, loading up arrays of pointers to
   * the points in each bin
   */
  
  for (i = 0; i < npoints; i++) {
    
    ibin = (int) floor ((*x - minx) / deltax);
    
    if (ibin >= 0 && ibin < Glob->n_x_intervals) {
      bin_y[ibin][n_in_bin[ibin]] = *y;
      n_in_bin[ibin]++;
    }
    
    x++;
    y++;
    
  } /* i */
  
  /*
   * sort the points in each bin, and compute the percentile vals
   * if there are enough points
   */
  
  for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {
    
    if (n_in_bin[ibin] > Glob->min_points_for_percentile) {
      
      qsort((void *) bin_y[ibin],
	    (size_t) n_in_bin[ibin],
	    sizeof(double),
	    compare_y_coords);
      
      for (iperc = 0; iperc < Glob->n_percentiles; iperc++) {
	
	ipoint = (int)
	  ((double) n_observed[ibin] *
	   (Glob->percentiles[iperc] / 100.0) - 0.5);
	
	percentile_vals[iperc][ibin] = bin_y[ibin][ipoint];
	
      } /* iperc */
      
    } /* if (n_observed[ibin] > 0) */
    
  } /* ibin */

  /*
   * free up memory
   */

  ufree((char *) n_in_bin);
  
  for (ibin = 0; ibin < Glob->n_x_intervals; ibin++)
    ufree((char *) bin_y[ibin]);

  ufree((char *) bin_y);
    
  return (0);
  
}

/***********************************************************************
 * compute_stats()
 *
 * perform stats calculations
 */

static int compute_stats(double minx,
			 double deltax,
			 double hist_min,
			 double *x,
			 double *y,
			 int npoints,
			 double *xmean_p,
			 double *ymean_p,
			 double *xsdev_p,
			 double *ysdev_p,
			 double *chi_sq_p,
			 double *binx,
			 double *biny,
			 double *expected_y,
			 int *n_observed,
			 FILE *log_file)

{

  int i, ibin;
  int n_end_bins;

  double xmin = 1.0e99;
  double sumx = 0.0, sumx2 = 0.0;
  double sumy = 0.0, sumy2 = 0.0;
  double dn;
  double xmean, ymean;
  double xsdev, ysdev;
  double chi_sq = 0.0;
  double xval, yval;
  double expected, observed;
  double x_diff, chi_diff;
  double pxi = 0, xx;
  double beta, beta1, beta2, eta;
  double one_over_beta;
  double gamma_k;
  double shape, shape_minus_one;
  
  if (npoints < 2)
    return (-1);

  dn = (double) npoints;

  /*
   * compute x vals for each interval
   */
  
  for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {
    
    biny[ibin] = 0.0;
    binx[ibin] = hist_min + ((double) ibin + 0.5) * deltax;
    
  } /* ibin */
  
  /*
   * sum the various terms
   */

  for (i = 0; i < npoints; i++) {

    if (Glob->mode == LOG_HIST_MODE) {
      xval = *x++ - minx + 1.0;
      yval = *y++ - minx + 1.0;
      xval = SAFELOG(xval);
      yval = SAFELOG(xval);
    } else {
      xval = *x++;
      yval = *y++;
    }

    xmin = MIN(xmin, xval);
    sumx += xval;
    sumx2 += xval * xval;
    sumy += yval;
    sumy2 += yval * yval;

    ibin = (int) floor ((xval - hist_min) / deltax);

    if (ibin >= 0 && ibin < Glob->n_x_intervals)
      n_observed[ibin]++;
    
  } /* i */
  
  /*
   * compute mean and standard deviation
   */
  
  xmean = sumx / dn;
  ymean = sumy / dn;

  xsdev = sqrt(fabs(dn * sumx2 - sumx * sumx)) / (dn - 1.0);
  ysdev = sqrt(fabs(dn * sumy2 - sumy * sumy)) / (dn - 1.0);

  /*
   * exponential and gamma distribution parameters
   */

  shape = Glob->hist_fit_param1;
  shape_minus_one = shape - 1.0;
  beta1 = (xmean - xmin) / shape;
  beta2 = sqrt((xsdev * xsdev) / shape);
  beta = (beta1 + beta2) / 2.0;
  one_over_beta = 1.0 / beta;
  gamma_k = 1.0 / (pow(beta, shape) * gamma_(shape));
  eta = xmin;

  if (Glob->hist_fit == HIST_EXPONENTIAL) {

    fprintf(log_file, "EXPONENTIAL FIT\n");
    fprintf(log_file, "mean, sdev: %g, %g\n", xmean, xsdev);
    fprintf(log_file, "min: %g\n", xmin);
    fprintf(log_file, "beta: %g\n", beta);
    
  } else if (Glob->hist_fit == HIST_GAMMA) {

    fprintf(log_file, "GAMMA FIT\n");
    fprintf(log_file, "mean, sdev: %g, %g\n", xmean, xsdev);
    fprintf(log_file, "min: %g\n", xmin);
    fprintf(log_file, "beta: %g\n", beta);
    fprintf(log_file, "shape: %g\n", shape);
    
  } else if (Glob->hist_fit == HIST_NORMAL) {

    fprintf(log_file, "NORMAL FIT\n");
    fprintf(log_file, "mean, sdev: %g, %g\n", xmean, xsdev);
    
  }
    
  /*
   * compute fraction in each bin and chi_sq val
   */

  for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {
    biny[ibin] = ((double) n_observed[ibin] / dn);
  }
    
  /*
   * compute chi_sq for center bins
   */

  n_end_bins = 0;

  for (ibin = n_end_bins;
       ibin < Glob->n_x_intervals - n_end_bins; ibin++) {

    biny[ibin] = ((double) n_observed[ibin] / dn);
    
    x_diff = (binx[ibin] - xmean) / xsdev;

    if (Glob->hist_fit == HIST_EXPONENTIAL) {

      xx = binx[ibin] - eta;
      pxi = (one_over_beta * exp(-one_over_beta * xx));

    } else if (Glob->hist_fit == HIST_GAMMA) {

      xx = binx[ibin] - eta;
      pxi = gamma_k * pow(xx, shape_minus_one) * exp(-one_over_beta * xx);

    } else if (Glob->hist_fit == HIST_NORMAL) {

      /*
       * normal, lognormal
       */

      pxi = ((1.0 / (xsdev * sqrt(2.0 * M_PI))) *
	     EXP(-0.5 * x_diff * x_diff));
      
    }
    
    expected = pxi * (double) npoints * deltax;
    expected_y[ibin] = (pxi * deltax);
    observed = (double) n_observed[ibin];

    chi_diff = expected - observed;

    chi_sq += (chi_diff * chi_diff) / expected;

  } /* ibin */

  /*
   * set return vals
   */

  *xmean_p = xmean;
  *ymean_p = ymean;
  *xsdev_p = xsdev;
  *ysdev_p = ysdev;
  *chi_sq_p = chi_sq;

  return (0);
  
}

/*****************************************************************
 * copy the xgraph prologue file to stdout
 */

static int copy_prologue(void)

{
  
  char line[LINE_MAX];
  char *prologue_path;
  FILE *prologue_file;
  
  if (Glob->client == XGRAPH)
    prologue_path = Glob->xgraph_prologue_path;
  else
    prologue_path = Glob->acegr_prologue_path;
  
  /*
   * open prologue file
   */
  
  if ((prologue_file = fopen(prologue_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:filters.\n", Glob->prog_name);
    fprintf(stderr, "Opening prologue file.\n");
    perror(prologue_path);
    return(-1);
  }
  
  /*
   * copy contents of prologue file to stdout and
   * close prologue file
   */
  
  while (!feof(prologue_file)){
    if (fgets(line, LINE_MAX, prologue_file) != NULL)
      fputs(line, stdout);
  }

  fclose(prologue_file);

  return (0);

}

/******************************************************************
 * draw_fit()
 *
 * fits a curve to the data, and draws the curve
 */

static int draw_fit(int nfit,
		    double *xfit,
		    double *yfit,
		    double *aa,
		    double *binx,
		    FILE *log_file)

{

  long i, ibin;
  double xx, yy;
  double xfac;
  double std_error_est, r_squared;

  if (Glob->client == ACEGR) {
    fprintf(stdout, "DATA_FIT\n");
  }
  
  if (Glob->data_fit == DATA_POLYNOMIAL) {
    
    if (uPolyFit(nfit, xfit, yfit,
		 aa, Glob->polynomial_order,
		 &std_error_est, &r_squared)) {
    
      fprintf(stderr, "ERROR - %s:filter\n", Glob->prog_name);
      fprintf(stderr, "Cannot fit polynomial function to data\n");
      
    } else {
      
      fprintf(log_file,
	      "Polynomial order %ld fit :-\n",
	      (long) Glob->polynomial_order);
    
      for (i = 0; i <= Glob->polynomial_order; i++)
	fprintf(log_file,
		"    coefficient a%ld = %g\n", i, aa[i]);
    
      fprintf(log_file,
	      "    corr_coeff = %g\n", sqrt(r_squared));
      fprintf(log_file,
	      "    r_squared = %g\n", r_squared);
      fprintf(log_file,
	      "    stderr_est= %g\n", std_error_est);
      fprintf(log_file, "\n");
      
      for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {
	
	xx = binx[ibin];
	yy = aa[0];
	xfac = 1.0;
	for (i = 1; i <= Glob->polynomial_order; i++) {
	  xfac *= xx;
	  yy += aa[i] * xfac;
	}
	if (ibin == 0 && Glob->client == XGRAPH)
	  fprintf(stdout, "move ");
	fprintf(stdout, "%g %g\n", xx, yy);

      } /* ibin */

    } /* if (uPolyFit ... */
    
  } else if (Glob->data_fit == DATA_EXPONENTIAL) {
    
    if (uExponentialFit(nfit, xfit, yfit, aa,
			&std_error_est, &r_squared)) {
      
      fprintf(stderr, "ERROR - %s:filter\n", Glob->prog_name);
      fprintf(stderr, "Cannot fit exponential function to data\n");
      
    } else {
      
      fprintf(log_file, "Exponential fit :-\n");

      for (i = 0; i < 3; i++)
	fprintf(log_file,
		"    coefficient a%ld = %g\n", i, aa[i]);
      
      fprintf(log_file,
	      "    corr_coeff = %g\n", sqrt(r_squared));
      fprintf(log_file,
	      "    r_squared = %g\n", r_squared);
      fprintf(log_file,
	      "    stderr_est= %g\n", std_error_est);
      fprintf(log_file, "\n");
      
      for (ibin = 0; ibin < Glob->n_x_intervals; ibin++) {

	xx = binx[ibin];
	yy = aa[0] + aa[1] * exp(xx * aa[2]);
	if (ibin == 0 && Glob->client == XGRAPH)
	  fprintf(stdout, "move ");
	fprintf(stdout, "%g %g\n", xx, yy);
	
      } /* ibin */
      
    } /* if (uExponentialFit ... */
    
  } /* if (Glob->data_fit ... */
  
  return (0);
}

/*****************************************************************
 * load_xy_data()
 *
 * loads x and y data arrays
 */

typedef struct xy_list_s {
  double x, y;
  struct xy_list_s *next;
} xy_list_t;

static int load_xy_data(int xpos,
			int ypos,
			int cond_pos,
			double **x_p,
			double **y_p,
			double *minx_p,
			double *maxx_p,
			double *miny_p,
			double *maxy_p,
			int *npoints_p)

{

  char line[LINE_MAX];
  char *token, *end_pt;
  char *x_str, *y_str, *cond_str;

  int extreme_point;
  int perform_attrition = Glob->perform_attrition;

  int i;
  int npoints = 0;
  int count = 0;
  int attrition_count = Glob->attrition_count;

  double *x, *y;
  double xx, yy, cond_val;
  double minx = LARGE_DOUBLE, maxx = -LARGE_DOUBLE;
  double miny = LARGE_DOUBLE, maxy = -LARGE_DOUBLE;

  xy_list_t *this_point = NULL, *prev_point = NULL;
  xy_list_t *first_point = NULL;

  while (!feof(stdin)) {

    if (fgets(line, LINE_MAX, stdin) != NULL) {

      if (line[0] != '#') {
	
	token = strtok(line, " \n");
	i = 0;
	x_str = (char *) NULL;
	y_str = (char *) NULL;
	cond_str = (char *) NULL;

	while (token != NULL) {
	  
	  if (i == xpos)
	    x_str = token;
	  
	  if (i == ypos)
	    y_str = token;

	  if (Glob->conditional_data && i == cond_pos)
	    cond_str = token;

	  if (Glob->conditional_data) {
	    
	    if (x_str != NULL && y_str != NULL && cond_str != NULL)
	      break;
	    
	  } else {
	    
	    if (x_str != NULL && y_str != NULL)
	      break;

	  } /* if (Glob->conditional_data) */

	  i++;
	  token = strtok((char *) NULL, " \n");
	  
	} /* while (token ... */
	
	if (x_str == NULL || y_str == NULL) {
	  
	  fprintf(stderr, "WARNING - %s:filter:load_xy_list\n",
		  Glob->prog_name);
	  
	  if (x_str == NULL)
	    fprintf(stderr, "x value not found in data\n");
	  
	  if ((Glob->mode == SCATTER_MODE ||
	       Glob->mode == PERCENTILE_MODE) &&
	      (y_str == NULL))
	    fprintf(stderr, "y value not found in data\n");
	
	  continue;
	  
	}
	
	if (Glob->conditional_data && cond_str == NULL) {
	  fprintf(stderr, "WARNING - %s:filter:load_xy_list\n",
		  Glob->prog_name);
	  fprintf(stderr, "cond value not found in data\n");
	  continue;
	}
	
	/*
	 * ignore if either data val is missing
	 */
	
	if (!strncmp(x_str, "-9999", 5) ||
	    !strncmp(y_str, "-9999", 5))
	  continue;
	
	errno = 0;
	xx = strtod(x_str, &end_pt);
	if (errno) {
	  fprintf(stderr, "WARNING - %s:filter:load_xy_list\n",
		  Glob->prog_name);
	  fprintf(stderr, "Error in data stream, reading x\n");
	  perror(x_str);
	  continue;
	}

	errno = 0;
	yy = strtod(y_str, &end_pt);
	if (errno) {
	  fprintf(stderr, "WARNING - %s:filter:load_xy_list\n",
		  Glob->prog_name);
	  fprintf(stderr, "Error in data stream, reading y\n");
	  perror(y_str);
	  continue;
	}

	/*
	 * check limits as required
	 */

	if (Glob->limitx_data)
	  if (xx < Glob->lowx || xx > Glob->highx)
	    continue;

	if (Glob->limity_data)
	  if (yy < Glob->lowy || yy > Glob->highy)
	    continue;

	/*
	 * check conditional data
	 */

	if (Glob->conditional_data) {

	  errno = 0;
	  cond_val = strtod(cond_str, &end_pt);
	  if (errno) {
	    fprintf(stderr, "WARNING - %s:filter:load_xy_list\n",
		    Glob->prog_name);
	    fprintf(stderr, "Error in data stream, reading cond val\n");
	    perror(cond_str);
	    continue;
	  }

	  if (cond_val < Glob->cond_low ||
	      cond_val > Glob->cond_high)
	    continue;
	  
	} /* if (Glob->conditional_data) */

	/*
	 * scalar and log transform
	 */
	
	xx *= Glob->multx;
	yy *= Glob->multy;
	
	if (Glob->logx) {
	  xx = log(xx);
	}
	if (Glob->logy) {
	  yy = log(yy);
	}

	/*
	 * set extreme points
	 */

	extreme_point = FALSE;
	
	if (xx < minx) {
	  minx = xx;
	  extreme_point = TRUE;
	}

	if (xx > maxx) {
	  maxx = xx;
	  extreme_point = TRUE;
	}

	if (yy < miny) {
	  miny = yy;
	  extreme_point = TRUE;
	}

	if (yy > maxy) {
	  maxy = yy;
	  extreme_point = TRUE;
	}

	/*
	 * if attrition is to be performed, continue to end of loop
	 * except for extreme points or every 'attrition_count' points
	 */
	
	if (!extreme_point && perform_attrition) {
	  count++;
	  if (count > attrition_count) {
	    count = 0;
	  } else {
	    continue;
	  }
	}

	/*
	 * add point to linked list
	 */

	this_point = (xy_list_t *) umalloc
	  (sizeof(xy_list_t));
	this_point->next = (xy_list_t *) NULL;
	
	if (npoints == 0)
	  first_point = this_point;
	else
	  prev_point->next = this_point;
	
	this_point->x = xx;
	this_point->y = yy;
	
	npoints++;
	prev_point = this_point;
	
      } /* if (line[0] ... */
      
    } /* if (fgets ... */
    
  } /* while */

  /*
   * allocate the x and y arrays
   */

  x = (double *) umalloc (npoints * sizeof(double));
  y = (double *) umalloc (npoints * sizeof(double));

  /*
   * set return values
   */

  *x_p = x;
  *y_p = y;
  *npoints_p = npoints;
  *minx_p = minx;
  *maxx_p = maxx;
  *miny_p = miny;
  *maxy_p = maxy;
  
  /*
   * load up x and y arrays, free up linked list
   */

  this_point = first_point;
  
  for (i = 0; i < npoints; i++) {
    
    *x = this_point->x;
    *y = this_point->y;
    prev_point = this_point;
    this_point = this_point->next;

    ufree((char *) prev_point);

    x++;
    y++;

  } /* i */

  return (0);

}

/**********************************************************************
 * print_acegraph_hdrs()
 */

static void print_acegraph_hdrs(char *xlabel,
				char *ylabel)

{

  /*
   * acegr
   */

  if (Glob->mode == SCATTER_MODE) {

    fprintf(stdout, "@    s0 symbol 2\n");
    fprintf(stdout, "@    s0 linestyle 0\n");

  } else {
    
    fprintf(stdout, "@    s0 symbol 0\n");
    fprintf(stdout, "@    s0 linestyle 1\n");

  }

  fprintf(stderr, "----> title: %s\n", Glob->title);

  if (strlen(Glob->title) > 2) {
    fprintf(stdout, "@    subtitle \"%s\"\n", Glob->title);
  }

  if (Glob->logx) {
    fprintf(stdout, "@    xaxis  label \"Log_%s\"\n", xlabel);
  } else {
    fprintf(stdout, "@    xaxis  label \"%s\"\n", xlabel);
  }
    
  if (Glob->mode == SCATTER_MODE || Glob->mode == PERCENTILE_MODE) {
    if (Glob->logy) {
      fprintf(stdout, "@    yaxis  label \"Log_%s\"\n", ylabel);
    } else {
      fprintf(stdout, "@    yaxis  label \"%s\"\n", ylabel);
    }
  } else if (Glob->mode == HIST_MODE || 
	     Glob->mode == LOG_HIST_MODE) {
    if (Glob->hist_bar) {
      fprintf(stdout, "@    yaxis  label \"%s\"\n", "Percent");
    } else {
      fprintf(stdout, "@    yaxis  label \"%s\"\n", "Density");
    }
  }
  
  if (Glob->mode == SCATTER_MODE) {
      
    fprintf(stdout, "@g0 type xy\n");
      
  } else if (Glob->mode == HIST_MODE ||
	     Glob->mode == LOG_HIST_MODE) {
      
    if (Glob->hist_bar) {
      fprintf(stdout, "@g0 type bar\n");
    } else {
      fprintf(stdout, "@g0 type xy\n");
    }
      
  }

  if (Glob->mode == HIST_MODE) {

    fprintf(stdout, "@    s0 color 1\n");
    if (Glob->hist_bar) {
      fprintf(stdout, "@    s0 fill 1\n");
      fprintf(stdout, "@    s0 fill with color\n");
      fprintf(stdout, "@    s0 fill color 1\n");
      fprintf(stdout, "@    s0 fill pattern 0\n");
    }

  } else if (Glob->mode == LOG_HIST_MODE) {
      
    fprintf(stdout, "@    s0 color 1\n");

  }

  if (Glob->plot_logx && Glob->plot_logy)
    fprintf(stdout, "@g0 type LOGXY\n");
  else if (Glob->plot_logx)
    fprintf(stdout, "@g0 type LOGX\n");
  else if (Glob->plot_logy)
    fprintf(stdout, "@g0 type LOGY\n");

  if (Glob->limitx_axis) {
    fprintf(stdout, "@    world xmin %g\n", Glob->lowx);
    fprintf(stdout, "@    world xmax %g\n", Glob->highx);
  }
    
  if (Glob->limity_axis) {
    fprintf(stdout, "@    world ymin %g\n", Glob->lowy);
    fprintf(stdout, "@    world ymax %g\n", Glob->highy);
  }
    
}
  
/**********************************************************************
 * print_xgraph_hdrs()
 */

static void print_xgraph_hdrs(char *xlabel,
			      char *ylabel,
			      double deltax)

{

  fprintf(stdout, "\n\nTitleText: %s\n", Glob->title);
  fprintf(stdout, "FileorDev: %s\n", Glob->printer_name);
  
  fprintf(stdout, "XUnitText: %s\n", xlabel);
    
  if (Glob->mode == SCATTER_MODE || Glob->mode == PERCENTILE_MODE)
    fprintf(stdout, "YUnitText: %s\n", ylabel);
  else if (Glob->mode == HIST_MODE ||
	   Glob->mode == LOG_HIST_MODE)
    fprintf(stdout, "YUnitText: Percent\n");
  
  if (Glob->mode == SCATTER_MODE) {
    
    fprintf(stdout, "Markers: true\n");
      
  } else if (Glob->mode == HIST_MODE ||
	     Glob->mode == LOG_HIST_MODE) {
      
    fprintf(stdout, "BarGraph: true\n");
    fprintf(stdout, "BarWidth: %g\n", deltax * 1.10);
    fprintf(stdout, "NoLines: true\n");
    
  }
    
  if (Glob->limitx_axis) {
    fprintf(stdout, "XLowLimit: %g\n", Glob->lowx);
    fprintf(stdout, "XHighLimit: %g\n", Glob->highx);
  }
    
  if (Glob->limity_axis) {
    fprintf(stdout, "YLowLimit: %g\n", Glob->lowy);
    fprintf(stdout, "YHighLimit: %g\n", Glob->highy);
  }
    
  if (Glob->plot_logx)
    fprintf(stdout, "LogX: true\n");
  
  if (Glob->plot_logy)
    fprintf(stdout, "LogY: true\n");
  
}

/**********************************************************************
 * read_labels()
 *
 * read axis labels and conditional data label
 */

static int read_labels(int *xpos,
		       int *ypos,
		       int *cond_pos,
		       char *xlabel,
		       char *ylabel,
		       char *cond_label,
		       FILE* log_file)

{

  char *token;
  char *paren_p;
  char line[LINE_MAX];
  int retval = 0;
  int n_match;
  int i;

  *xpos = -1;
  *ypos = -1;
  *cond_pos = -1;

  while (!feof(stdin)) {

    if (fgets(line, LINE_MAX, stdin) != NULL) {

      if (!strncmp(line, "#labels: ", 9)) {
      
	token = strtok(line + 9, ",\n");

	i = 0;

	while (token != NULL) {
	  
	  /* 
	   *  ignore units in parens, perhaps preceded by spaces
	   */ 
	  
	  paren_p = strchr(token, '(');
	  if (paren_p != NULL) {
	    while (*(paren_p - 1) == ' ')
	      paren_p--;
	    n_match = (int) (paren_p - token);
	  } else {
	    n_match = strlen(token);
	  }
	  
	  if (!strncmp(Glob->xlabel, token, n_match)) {
	    strncpy(xlabel, token, LABEL_MAX);
	    *xpos = i;
	    fprintf(log_file, "xlabel: %s\n", xlabel);
	  }

	  if (!strncmp(Glob->ylabel, token, n_match)) {
	    strncpy(ylabel, token, LABEL_MAX);
	    *ypos = i;
	    fprintf(log_file, "ylabel: %s\n", ylabel);
	  }

	  if (!strncmp(Glob->cond_label, token, n_match)) {
	    strncpy(cond_label, token, LABEL_MAX);
	    *cond_pos = i;
	    fprintf(log_file, "cond_label: %s\n", cond_label);
	  }

	  i++;
	  token = strtok((char *) NULL, ",\n");

	} /* while (token ... */

	break;

      } /* if (!strncmp(line, "#labels: ", 9)) */

    } /* if (fgets ... */

  } /* while */

  /*
   * return error if labels not found
   */

  if (*xpos < 0) {

    fprintf(stderr, "ERROR - %s:filter\n", Glob->prog_name);
    fprintf(stderr, "x label '%s' not found\n", Glob->xlabel);
    retval = -1;
    
  }

  if (*ypos < 0) {
    
    if (Glob->mode == SCATTER_MODE || Glob->mode == PERCENTILE_MODE) {
      fprintf(stderr, "ERROR - %s:filter\n", Glob->prog_name);
      fprintf(stderr, "y label '%s' not found\n", Glob->ylabel);
      retval = -1;
    }

    *ypos = 0;

  }

  if (Glob->conditional_data && *cond_pos < 0) {

    fprintf(stderr, "ERROR - %s:filter\n", Glob->prog_name);
    fprintf(stderr, "cond label '%s' not found\n", Glob->cond_label);
    retval = -1;

  }

  return (retval);

}
