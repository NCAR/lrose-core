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
/*********************************************************************
 * read_params.c: reads the parameters, loads up the globals
 *
 * RAP, NCAR, Boulder CO
 *
 * Jan 1992
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "ascii_to_xgraph.h"

void read_params(void)

{

  char *resource_str;
  char *str_copy, *token, *end_pt;
  int i;

  Glob->xgraph_prologue_path = uGetParamString(Glob->prog_name,
					       "xgraph_prologue_path",
					       XGRAPH_PROLOGUE_PATH);
 
  Glob->acegr_prologue_path = uGetParamString(Glob->prog_name,
					      "acegr_prologue_path",
					      ACEGR_PROLOGUE_PATH);
 
  Glob->printer_name = uGetParamString(Glob->prog_name,
				       "printer_name", PRINTER_NAME);

  Glob->log_file_path = uGetParamString(Glob->prog_name,
					"log_file_path",
					LOG_FILE_PATH);

  Glob->title = uGetParamString(Glob->prog_name,
				"title", TITLE);

  Glob->xlabel = uGetParamString(Glob->prog_name,
				 "xlabel", XLABEL);
  
  Glob->ylabel = uGetParamString(Glob->prog_name,
				 "ylabel", YLABEL);
  
  /*
   * percentiles
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "percentiles", PERCENTILES);

  str_copy = (char *) umalloc (strlen(resource_str) + 1);
  strcpy(str_copy, resource_str);

  Glob->n_percentiles = 0;
  token = strtok(str_copy, " \n\r\t");

  while (token != NULL) {
    Glob->n_percentiles++;
    token = strtok((char *) NULL, " \n\r\t");
  }

  Glob->percentiles = (double *)
    umalloc(Glob->n_percentiles * sizeof(double));

  strcpy(str_copy, resource_str);

  token = strtok(str_copy, " \n\r\t");

  for (i = 0; i < Glob->n_percentiles; i++) {
    Glob->percentiles[i] = strtod(token, &end_pt);
    token = strtok((char *) NULL, " \n\r\t");
  }

  ufree((char *) str_copy);

  Glob->min_points_for_percentile =
    uGetParamLong(Glob->prog_name,
		  "min_points_for_percentile",
		  MIN_POINTS_FOR_PERCENTILE);
  
  /*
   * set data_fit option
   */
  
  resource_str = uGetParamString(Glob->prog_name, "data_fit", DATA_FIT);
  
  if (uset_triple_param(Glob->prog_name,
			"read_params",
			Glob->params_path_name,
			resource_str, &Glob->data_fit, 
			"false", FALSE,
			"polynomial", DATA_POLYNOMIAL,
			"exponential", DATA_EXPONENTIAL,
			"data_fit"))
    exit(-1);

  Glob->polynomial_order =
    uGetParamLong(Glob->prog_name, "polynomial_order", POLYNOMIAL_ORDER);

  /*
   * set hist_fit option
   */
  
  resource_str = uGetParamString(Glob->prog_name, "hist_fit", HIST_FIT);
  
  if (uset_quin_param(Glob->prog_name,
		      "read_params",
		      Glob->params_path_name,
		      resource_str, &Glob->hist_fit, 
		      "false", FALSE,
		      "normal", HIST_NORMAL,
		      "exponential", HIST_EXPONENTIAL,
		      "gamma", HIST_GAMMA,
		      "weibull", HIST_WEIBULL,
		      "hist_fit"))
    exit(-1);
  
  Glob->hist_fit_param1 =
    uGetParamDouble(Glob->prog_name, "hist_fit_param1", HIST_FIT_PARAM1);

  if (Glob->hist_fit == HIST_WEIBULL) {
    fprintf(stderr, "WARNING: %s\n", Glob->prog_name);
    fprintf(stderr,
	    "Weibull distribution not yet implemented.\n");
    Glob->hist_fit = FALSE;
  }

  if (Glob->hist_fit == HIST_EXPONENTIAL) {
    Glob->hist_fit_param1 = 1.0;
  }
  
  resource_str = uGetParamString(Glob->prog_name, "hist_bar", HIST_BAR);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->hist_bar,
			    "hist_bar"))
    exit(-1);

  /*
   * set data attrition option
   */

  resource_str = uGetParamString(Glob->prog_name,
				 "perform_attrition",
				 PERFORM_ATTRITION);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->perform_attrition,
			    "perform_attrition"))
    exit(-1);

  Glob->attrition_count =
    uGetParamLong(Glob->prog_name, "attrition_count",
		  ATTRITION_COUNT);
  
  /*
   * set log transform for input data
   */

  resource_str = uGetParamString(Glob->prog_name, "logx", LOGX);

  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->logx, "logx"))
    exit(-1);

  resource_str = uGetParamString(Glob->prog_name, "logy", LOGY);

  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->logy, "logy"))
    exit(-1);

  /*
   * set log scale option for plotting
   */

  resource_str = uGetParamString(Glob->prog_name, "plot_logx", PLOT_LOGX);

  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->plot_logx, "plot_logx"))
    exit(-1);

  resource_str = uGetParamString(Glob->prog_name, "plot_logy", PLOT_LOGY);

  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->plot_logy, "plot_logy"))
    exit(-1);

  /*
   * set data and axis limits
   */

  resource_str = uGetParamString(Glob->prog_name,
				 "limitx_data", LIMITX_DATA);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->limitx_data,
			    "limitx_data"))
    exit(-1);

  resource_str = uGetParamString(Glob->prog_name,
				 "limity_data", LIMITY_DATA);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->limity_data,
			    "limity_data"))
    exit(-1);

  resource_str = uGetParamString(Glob->prog_name,
				 "limitx_axis", LIMITX_AXIS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->limitx_axis,
			    "limitx_axis"))
    exit(-1);

  resource_str = uGetParamString(Glob->prog_name,
				 "limity_axis", LIMITY_AXIS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->limity_axis,
			    "limity_axis"))
    exit(-1);

  Glob->multx = uGetParamDouble(Glob->prog_name, "multx", MULTX);
  Glob->multy = uGetParamDouble(Glob->prog_name, "multy", MULTY);
  Glob->lowx = uGetParamDouble(Glob->prog_name, "lowx", LOWX);
  Glob->lowy = uGetParamDouble(Glob->prog_name, "lowy", LOWY);
  Glob->highx = uGetParamDouble(Glob->prog_name, "highx", HIGHX);
  Glob->highy = uGetParamDouble(Glob->prog_name, "highy", HIGHY);

  /*
   * set the histogram options
   */
  
  resource_str = uGetParamString(Glob->prog_name,
				 "set_x_intervals", SET_X_INTERVALS);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->set_x_intervals,
			    "set_x_intervals"))
    exit(-1);

  Glob->minx = uGetParamDouble(Glob->prog_name, "minx", MINX);
  Glob->deltax = uGetParamDouble(Glob->prog_name, "deltax", DELTAX);
  Glob->n_x_intervals = uGetParamLong(Glob->prog_name,
				      "n_x_intervals", N_X_INTERVALS);

  /*
   * set conditional data parameters
   */

  resource_str = uGetParamString(Glob->prog_name,
				 "conditional_data", CONDITIONAL_DATA);
  
  if (uset_true_false_param(Glob->prog_name,
			    "read_params",
			    Glob->params_path_name,
			    resource_str, &Glob->conditional_data,
			    "conditional_data"))
    exit(-1);

  Glob->cond_label = uGetParamString(Glob->prog_name,
				     "cond_label", COND_LABEL);

  Glob->cond_low = uGetParamDouble(Glob->prog_name, "cond_low", COND_LOW);
  Glob->cond_high = uGetParamDouble(Glob->prog_name, "cond_high", COND_HIGH);

}
