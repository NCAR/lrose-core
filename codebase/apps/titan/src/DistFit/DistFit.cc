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
///////////////////////////////////////////////////////////////
// DistFit.cc
//
// DistFit object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
///////////////////////////////////////////////////////////////

#include "DistFit.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/mem.h>
#include <toolsa/ucopyright.h>
#include <rapmath/math_macros.h>
#include <rapmath/stats.h>
using namespace std;

// Constructor

DistFit::DistFit(int argc, char **argv)

{

  OK = TRUE;

  // set programe name

  _progName = STRdup("DistFit");
  ucopyright((char *) _progName.c_str());

  // parse args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = false;
    return;
  }

  if (!OK) {
    return;
  }

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

DistFit::~DistFit()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int DistFit::Run ()
{

  if (_loadLabels()) {
    fprintf(stderr, "ERROR - %s:_loadLabels()\n", _progName.c_str());
    return -1;
  }

  if (_loadXYData()) {
    fprintf(stderr, "ERROR - %s:_loadXYData()\n", _progName.c_str());
    return -1;
  }

  fprintf(stdout, "\n\n  **** %s ****\n\n", _progName.c_str());
  fprintf(stdout, "    xlabel: %s\n", _xLabel);
  if (_params.log_x_data) {
    fprintf(stdout, "    X data has log transform\n");
  }
  if (_params.distribution_class == Params::BI_VARIATE) {
    fprintf(stdout, "    ylabel: %s\n", _yLabel);
  }
  if (_params.condition_input_data) {
    fprintf(stdout, "    cond_label: %s\n", _condLabel);
  }


  if (_params.distribution_name == Params::DIST_NORMAL ||
      _params.distribution_name == Params::DIST_ALL) {
    fprintf(stdout, "\nNormal fit:\n\n");
    double mean, sdev;
    if (STATS_normal_fit(_nData, _xData, &mean, &sdev)) {
      fprintf(stdout, "    Normal does not fit\n");
    } else {
      fprintf(stdout, "    Normal params: mean = %g, sdev = %g\n",
	      mean, sdev);
      double chisq;
      if (STATS_normal_chisq(_nData, _xData, mean, sdev,
			     _params.chisq_nbins, &chisq) == 0) {
	fprintf(stdout, "    Normal chisq: %g, nbins %d\n",
		chisq, (int) _params.chisq_nbins);
      }
    }
  }

  if (_params.distribution_name == Params::DIST_EXPONENTIAL ||
      _params.distribution_name == Params::DIST_ALL) {
    fprintf(stdout, "\nExponential fit:\n\n");
    double b;
    if (STATS_exponential_fit(_nData, _xData, &b)) {
      fprintf(stdout, "    Exponential does not fit\n");
    } else {
      fprintf(stdout, "    Exponential params: b = %g\n", b);
      double chisq;
      if (STATS_exponential_chisq(_nData, _xData, b,
				  _params.chisq_nbins, &chisq) == 0) {
	fprintf(stdout, "    Exponential chisq: %g, nbins %d\n",
		chisq, (int) _params.chisq_nbins);
      }
    }
  }

  if (_params.distribution_name == Params::DIST_GAMMA ||
      _params.distribution_name == Params::DIST_ALL) {
    fprintf(stdout, "\nGamma fit:\n\n");
    double a, b;
    if (STATS_gamma_fit(_nData, _xData, &a, &b)) {
      fprintf(stdout, "    Gamma does not fit\n");
    } else {
      fprintf(stdout, "    Gamma params: a = %g, b = %g\n", a, b);
      double chisq;
      if (STATS_gamma_chisq(_nData, _xData, a, b,
			    _params.chisq_nbins, &chisq) == 0) {
	fprintf(stdout, "    Gamma chisq: %g, nbins %d\n",
		chisq, (int) _params.chisq_nbins);
      }
    }
  }

  if (_params.distribution_name == Params::DIST_WEIBULL ||
      _params.distribution_name == Params::DIST_ALL) {
    fprintf(stdout, "\nWeibull fit:\n\n");
    double a, b;
    if (STATS_weibull_fit(_nData, _xData, &a, &b)) {
      fprintf(stdout, "    Weibull does not fit\n");
    } else {
      fprintf(stdout, "    Weibull params: a = %g, b = %g\n", a, b);
      double chisq;
      if (STATS_weibull_chisq(_nData, _xData, a, b,
			      _params.chisq_nbins, &chisq) == 0) {
	fprintf(stdout, "    Weibull chisq: %g, nbins %d\n",
		chisq, (int) _params.chisq_nbins);
      }
    }
  }

  return 0;

}

////////////////
// _loadLabels()
//
// load data label positions from stdin
//

#define LINE_MAX_LEN 1024

int DistFit::_loadLabels()
  
{

  char *token;
  char *paren_p;
  char line[LINE_MAX_LEN];
  int retval = 0;
  int n_match;
  si32 i;

  _xPos = -1;
  _yPos = -1;
  _condPos = -1;

  while (!feof(stdin)) {

    if (fgets(line, LINE_MAX_LEN, stdin) != NULL) {

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
	    n_match = (si32) (paren_p - token);
	  } else {
	    n_match = strlen(token);
	  }
	  
	  if (!strncmp(_params.x_label, token, n_match)) {
	    STRncopy(_xLabel, token, LABEL_MAX);
	    _xPos = i;
	  }

	  if (!strncmp(_params.y_label, token, n_match)) {
	    STRncopy(_yLabel, token, LABEL_MAX);
	    _yPos = i;
	  }

	  if (!strncmp(_params.conditional_label, token, n_match)) {
	    STRncopy(_condLabel, token, LABEL_MAX);
	    _condPos = i;
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

  if (_xPos < 0) {

    fprintf(stderr, "ERROR - %s:_loadLabels\n", _progName.c_str());
    fprintf(stderr, "x label '%s' not found\n", _params.x_label);
    retval = -1;
    
  }

  if (_yPos < 0) {
    if (_params.distribution_class == Params::BI_VARIATE) {
      fprintf(stderr, "ERROR - %s:_loadLabels\n", _progName.c_str());
      fprintf(stderr, "y label '%s' not found\n", _params.y_label);
      retval = -1;
    }
    _yPos = 0;
  }

  if (_params.condition_input_data && _condPos < 0) {
    fprintf(stderr, "ERROR - %s:_loadLabels\n", _progName.c_str());
    fprintf(stderr, "cond label '%s' not found\n", _params.conditional_label);
    retval = -1;
  }

  return retval;
  
}

//////////////////
// _loadXYData()
//
// loads x and y data arrays
//

typedef struct xy_list_s {
  double x, y;
  struct xy_list_s *next;
} xy_list_t;

int DistFit::_loadXYData()

{

  char line[LINE_MAX_LEN];
  char *token, *end_pt;
  char *x_str, *y_str, *cond_str;
  
  int extreme_point;
  int perform_attrition = _params.perform_attrition;
  
  si32 i;
  si32 count = 0;
  si32 attrition_count = _params.attrition_count;
  
  double *x, *y;
  double xx, yy, cond_val;
  
  double minx = LARGE_DOUBLE, maxx = -LARGE_DOUBLE;
  double miny = LARGE_DOUBLE, maxy = -LARGE_DOUBLE;
  
  xy_list_t *this_point, *prev_point = NULL;
  xy_list_t *first_point = NULL; 

  _nData = 0;

  while (!feof(stdin)) {
    
    if (fgets(line, LINE_MAX_LEN, stdin) != NULL) {
      
      if (line[0] != '#') {
	
	token = strtok(line, " \n");
	i = 0;
	x_str = (char *) NULL;
	y_str = (char *) NULL;
	cond_str = (char *) NULL;
	
	while (token != NULL) {
	  
	  if (i == _xPos)
	    x_str = token;
	  
	  if (i == _yPos)
	    y_str = token;
	  
	  if (_params.condition_input_data && i == _condPos)
	    cond_str = token;

	  if (_params.condition_input_data) {
	    
	    if (x_str != NULL && y_str != NULL && cond_str != NULL)
	      break;
	    
	  } else {
	    
	    if (x_str != NULL && y_str != NULL)
	      break;
	    
	  } // if (_params.condition_input_data)

	  i++;
	  token = strtok((char *) NULL, " \n");
	  
	} // while (token ...
	
	if (x_str == NULL || y_str == NULL) {
	  fprintf(stderr, "WARNING - %s:_loadXYData\n", _progName.c_str());
	  if (x_str == NULL) {
	    fprintf(stderr, "x value not found in data\n");
	  }
	  if (_params.distribution_class == Params::BI_VARIATE && y_str == NULL) {
	    fprintf(stderr, "y value not found in data\n");
	  }
	  continue;
	}
	
	if (_params.condition_input_data && cond_str == NULL) {
	  fprintf(stderr, "WARNING - %s:_loadXYData\n", _progName.c_str());
	  fprintf(stderr, "cond value not found in data\n");
	  continue;
	}
	
	// ignore if either data val is missing

	if (!strncmp(x_str, "-9999", 5) || !strncmp(y_str, "-9999", 5)) {
	  continue;
	}
	
	errno = 0;
	xx = strtod(x_str, &end_pt);
	if (_params.log_x_data) {
	  xx = log(xx);
	}
	if (errno) {
	  fprintf(stderr, "WARNING - %s:_loadXYData\n", _progName.c_str());
	  fprintf(stderr, "Error in data stream, reading x\n");
	  perror(x_str);
	  continue;
	}

	errno = 0;
	yy = strtod(y_str, &end_pt);
	if (_params.log_y_data) {
	  yy = log(yy);
	}
	if (errno) {
	  fprintf(stderr, "WARNING - %s:_loadXYData\n", _progName.c_str());
	  fprintf(stderr, "Error in data stream, reading y\n");
	  perror(y_str);
	  continue;
	}
	
	// check limits as required
	
	if (_params.limit_x_data) {
	  if (xx < _params.x_min || xx > _params.x_max) {
	    continue;
	  }
	}

	if (_params.limit_y_data) {
	  if (yy < _params.y_min || yy > _params.y_max) {
	    continue;
	  }
	}
	
	// check conditional data
	
	if (_params.condition_input_data) {
	  
	  errno = 0;
	  cond_val = strtod(cond_str, &end_pt);
	  if (errno) {
	    fprintf(stderr, "WARNING - %s:_loadXYData\n", _progName.c_str());
	    fprintf(stderr, "Error in data stream, reading cond val\n");
	    perror(cond_str);
	    continue;
	  }
	  
	  if (cond_val < _params.cond_min ||
	      cond_val > _params.cond_max) {
	    continue;
	  }
	  
	} // if (_params.condition_input_data)

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

	// if attrition is to be performed, continue to end of loop
	// except for extreme points or every 'attrition_count' points
	
	if (!extreme_point && perform_attrition) {
	  count++;
	  if (count > attrition_count) {
	    count = 0;
	  } else {
	    continue;
	  }
	}
	
	// add point to linked list
	
	this_point = (xy_list_t *) umalloc
	  ((ui32) sizeof(xy_list_t));
	this_point->next = (xy_list_t *) NULL;
	
	if (_nData == 0)
	  first_point = this_point;
	else
	  prev_point->next = this_point;
	
	this_point->x = xx;
	this_point->y = yy;
	
	_nData++;
	prev_point = this_point;
	
      } // if (line[0] ...
      
    } // if (fgets ...
    
  } // while

  // allocate the x and y arrays

  _xData = (double *) umalloc ((ui32) (_nData * sizeof(double)));
  _yData = (double *) umalloc ((ui32) (_nData * sizeof(double)));

  // set min and max vals

  _xMin = minx;
  _xMax = maxx;
  _yMin = miny;
  _yMax = maxy;
  
  // load up x and y arrays, free up linked list

  this_point = first_point;

  x = _xData;
  y = _yData;
  for (i = 0; i < _nData; i++, x++, y++) {
    *x = this_point->x;
    *y = this_point->y;
    prev_point = this_point;
    this_point = this_point->next;
    ufree((char *) prev_point);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {

    x = _xData;
    y = _yData;
    for (i = 0; i < _nData; i++, x++, y++) {
      if (_params.distribution_class == Params::UNI_VARIATE) {
	fprintf(stdout, "index, x: %10d %10g\n", i, *x);
      } else {
	fprintf(stdout, "index, x, y: %10d %10g %10g\n",
		i, *x, *y);
      }
    } // i

  }

  return 0;

}

