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
//////////////////////////////////////////////////////////
// Property.cc
//
// Property object - contains array of property values
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#include "Property.hh"
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <cassert>
#include <math.h>
#include "Params.hh"
using namespace std;

//////////////////////////////////////////
// Empty constructor - for derived classes

Property::Property ()
  
{
  
}

//////////////////////////////////////////
// Constructor - non-time_series property

Property::Property (char *prog_name,
		    int debug,
		    char *name)
  
{

  _init(prog_name, debug, name);
  _fromTseries = FALSE;
  _deltaTime = 0;
  
}

/////////////////////////////////////
// Constructor - time_series property

Property::Property (char *prog_name,
		    int debug,
		    char *name,
		    int delta_time)
  
{
  
  _init(prog_name, debug, name);
  _fromTseries = TRUE;
  _deltaTime = delta_time;
  
}

/////////////
// destructor

Property::~Property ()
  
{

  // free up memory

  MEMbufDelete(_mbuf);
  STRfree(_progName);
  STRfree(_name);

}

///////////////////////////
// addVal()
// Add a value to the array
//

void Property::addVal(double value)
  
{
  _nVals++;
  MEMbufAdd(_mbuf, &value, sizeof(double));
  _vals = (double *) MEMbufPtr(_mbuf);
}

///////////////////////////
// computeMeasuredStats()
//
// Compute stats for measured data
//

void Property::computeMeasuredStats(int stat_type,
				    int n_cases,
				    int *seed_flags,
				    int *active_flags)
  
{

  if (stat_type == Params::ARITH_MEAN || stat_type == Params::GEOM_MEAN) {
    _computeMeans(stat_type, n_cases,
		  seed_flags, active_flags,
		  &_measuredResult);
  } else {
    _computeQuartiles(stat_type, n_cases,
		      seed_flags, active_flags,
		      &_measuredResult);
  }

}

///////////////////////////
// computeRerandStats()
//
// Compute stats for re-randomized data
//

void Property::computeRerandStats(int stat_type,
				  int n_cases,
				  int *seed_flags,
				  int *active_flags)
  
{

  if (stat_type == Params::ARITH_MEAN || stat_type == Params::GEOM_MEAN) {
    _computeMeans(stat_type, n_cases,
		  seed_flags, active_flags,
		  &_rerandResult);
  } else {
    _computeQuartiles(stat_type, n_cases,
		      seed_flags, active_flags,
		      &_rerandResult);
  }

  if (_measuredResult.diffStat != _rerandResult.diffStat) {
    _nRerandResults++;
    if (_measuredResult.diffStat >= _rerandResult.diffStat) {
      _nPResults++;
    }
  }

  if (_measuredResult.diffActivity != _rerandResult.diffActivity) {
    _nRerandActivity++;
    if (_measuredResult.diffActivity > _rerandResult.diffActivity) {
      _nPActivity++;
    }
  }

}

///////////////////////////
// computePfactor()
//
// Compute P-factor from rerandomization
//

void Property::computePfactor()
  
{

  if (_nRerandResults > 0) {
    _pFactorResults = ((double) _nPResults / (double) _nRerandResults) * 100.0;
  }
  
  if (_nRerandActivity > 0) {
    _pFactorActivity = ((double) _nPActivity / (double) _nRerandActivity) * 100.0;
  }

}

//////////////////////////////////////////////////
// print header
//

void Property::printHeader(FILE *out, int print_activity)

{
 
  fprintf(out, "%32s %6s %9s %10s %12s %13s %10s %10s",
	  "Property name",
	  "n_seed", "n_no_seed",
	  "stat_seed", "stat_no_seed",
	  "diff_in_stats", "% diff", "Pfactor");

  if (print_activity) {
    fprintf(out, " %10s", "PActivity");
  }
  
  fprintf(out, "\n");

  fprintf(out, "%32s %6s %9s %10s %12s %10s %10s %10s",
	  "",
	  "======", "=========",
	  "=========", "============",
	  "=============", "=====", "=======");

  if (print_activity) {
    fprintf(out, " %10s", "=========");
  }
  
  fprintf(out, "\n");

  fflush(out);

}

/////////////////////////////////////
// printStats()
//

void Property::printStats(FILE *out, int print_activity)

{

  char prop_name[256];

  if (_fromTseries) {
    sprintf(prop_name, "%s@%3g", _name, _deltaTime / 60.0);
  } else {
    STRncopy(prop_name, _name, 256);
  }
  
  fprintf(out,
	  "%32s %6d %9d %10.3f %12.3f %13.3f %10.1f",
	  prop_name,
	  _measuredResult.nYesSeed,
	  _measuredResult.nNoSeed,
	  _measuredResult.yesSeedStat,
	  _measuredResult.noSeedStat,
	  _measuredResult.diffStat,
	  _measuredResult.diffPercent);

  if (_nRerandResults > 0) {
    fprintf(out, " %10.1f", _pFactorResults);
  } else {
    fprintf(out, " %10s", " ");
  }

  if (print_activity && (_nRerandActivity > 0) && _fromTseries) {
    fprintf(out, " %10.1f", _pFactorActivity);
  } else {
    fprintf(out, " %10s", " ");
  }

  fprintf(out, "\n");

}

/////////////////////////////////////
// printDebug()
//

void Property::printDebug(FILE *out, int prop_num)

{

  fprintf(out, "  Prop %d, name %s ", prop_num, _name);
  if (_fromTseries) {
    fprintf(out, " TSERIES_TYPE, dtime %d", _deltaTime);
  }
  fprintf(out, ":");
  for (int j = 0; j < _nVals; j++) {
    fprintf(out, " %g", _vals[j]);
  }
  fprintf(out, "\n");
  
}

//////////////////////////////////////////
// PRIVATE FUNCTIONS
//////////////////////////////////////////


//////////////////////////////////////////
// Constructor initializer

void Property::_init(char *prog_name,
		     int debug,
		     char *name)
  
{
  
  _progName = STRdup(prog_name);
  _debug = debug;
  _name = STRdup(name);
  _nVals = 0;
  _mbuf = MEMbufCreate();
  _vals = (double *) MEMbufPtr(_mbuf);
  memset(&_measuredResult, 0, sizeof(stat_result_t));
  memset(&_rerandResult, 0, sizeof(stat_result_t));
  _nPResults = 0;
  _nRerandResults = 0;
  _pFactorResults = 50.0;
  _nPActivity = 0;
  _nRerandActivity = 0;
  _pFactorActivity = 50.0;
  
}

/////////////////////////////////////////////////////////////////
// _computeMeans()
//
// Compute the seed and no-seed arithmetic or geometric means.
//
// stat_type must be ARITH_MEAN or GEOM_MEAN
//

void Property::_computeMeans(int stat_type, int n_cases,
			     int *seed_flags, int *active_flags,
			     stat_result_t *result)

{

  assert(stat_type == Params::ARITH_MEAN || stat_type == Params::GEOM_MEAN);

  int n_yes_seed = 0;
  int n_no_seed = 0;

  double sum_yes_seed = 0.0;
  double sum_no_seed = 0.0;

  double mean_yes_seed;
  double mean_no_seed;
  double diff_in_means;
  double diff_percent;

  // sum up

  for (int i = 0; i < n_cases; i++) {

    if (active_flags[i]) {

      double val = _vals[i];
      
      if (seed_flags[i]) {
	
	if (val != PROPERTY_MISSING_DATA_VAL) {
	  if (stat_type == Params::ARITH_MEAN) {
	    sum_yes_seed += val;
	    n_yes_seed++;
	  } else {
	    if (val > 0) {
	      sum_yes_seed += log(val);
	      n_yes_seed++;
	    }
	  }
	}
	
      } else { // if (_seed_flags[i])
	
	if (val != PROPERTY_MISSING_DATA_VAL) {
	  if (stat_type == Params::ARITH_MEAN) {
	    sum_no_seed += val;
	    n_no_seed++;
	  } else {
	    if (val > 0) {
	      sum_no_seed += log(val);
	      n_no_seed++;
	    }
	  }
	}
      
      } // if (_seed_flags[i]) 

    } // if (active_flags[i]) 

  } // i

  // compute means

  if (n_yes_seed > 0) {
    if (stat_type == Params::ARITH_MEAN) {
      mean_yes_seed = sum_yes_seed / (double) n_yes_seed;
    } else {
      mean_yes_seed = exp(sum_yes_seed / (double) n_yes_seed);
    }
  } else {
    mean_yes_seed = PROPERTY_MISSING_DATA_VAL;
  }

  if (n_no_seed > 0) {
    if (stat_type == Params::ARITH_MEAN) {
      mean_no_seed = sum_no_seed / (double) n_no_seed;
    } else {
      mean_no_seed = exp(sum_no_seed / (double) n_no_seed);
    }
  } else {
    mean_no_seed = PROPERTY_MISSING_DATA_VAL;
  }

  // compute diff

  if (mean_yes_seed != PROPERTY_MISSING_DATA_VAL &&
      mean_no_seed != PROPERTY_MISSING_DATA_VAL) {
    diff_in_means = mean_yes_seed - mean_no_seed;
    if (mean_no_seed != 0.0) {
      diff_percent = (diff_in_means / fabs(mean_no_seed)) * 100.0;
    } else {
      diff_percent = 0.0;
    }
  } else {
    diff_in_means = PROPERTY_MISSING_DATA_VAL;
    diff_percent = PROPERTY_MISSING_DATA_VAL;
  }

  // set return vals

  result->nYesSeed = n_yes_seed;
  result->nNoSeed = n_no_seed;

  result->yesSeedStat = mean_yes_seed;
  result->noSeedStat = mean_no_seed;

  result->diffActivity = n_yes_seed - n_no_seed;

  result->diffStat = diff_in_means;
  result->diffPercent = diff_percent;

}

//////////////////////////////////
// _computeQuartiles()
//
// Compute the first, second or third quartiles.
//
// stat_type must be FIRST_QUARTILE, SECOND_QUARTILE or THIRD_QUARTILE
//

void Property::_computeQuartiles(int stat_type, int n_cases,
				 int *seed_flags, int *active_flags,
				 stat_result_t *result)

{

  assert(stat_type == Params::FIRST_QUARTILE ||
	 stat_type == Params::SECOND_QUARTILE ||
	 stat_type == Params::THIRD_QUARTILE);

  double quartile_fract;

  switch (stat_type) {
    
  case Params::FIRST_QUARTILE:
    quartile_fract = 0.25;
    break;
    
  case Params::SECOND_QUARTILE:
    quartile_fract = 0.50;
    break;
    
  case Params::THIRD_QUARTILE:
    quartile_fract = 0.75;
    break;

  } // switch

  // load up seed and no-seed arrays

  int n_yes_seed = 0;
  int n_no_seed = 0;

  MEMbuf *yesSeedBuf = MEMbufCreate();
  MEMbuf *noSeedBuf = MEMbufCreate();
  
  for (int i = 0; i < n_cases; i++) {

    if (active_flags[i]) {

      double val = _vals[i];
      
      if (seed_flags[i]) {
	
	if (val != PROPERTY_MISSING_DATA_VAL) {
	  MEMbufAdd(yesSeedBuf, &val, sizeof(double));
	  n_yes_seed++;
	}
	
      } else { // if (_seed_flags[i])
	
	if (val != PROPERTY_MISSING_DATA_VAL) {
	  MEMbufAdd(noSeedBuf, &val, sizeof(double));
	  n_no_seed++;
	}
	
      } // if (_seed_flags[i]) 

    } // if (active_flags[i])

  } // i
  
  double quart_yes_seed;
  double quart_no_seed;
  double diff_in_quarts;
  double diff_percent;

  double *noSeedVals = (double  *) MEMbufPtr(noSeedBuf);
  if (n_no_seed > 0) {
    qsort((void *) noSeedVals, n_no_seed, 
	  sizeof(double), _compareDoubles);
  }

  // compute seed quartiles

  if (n_yes_seed > 2) {
    
    // sort seed array

    double *yesSeedVals = (double  *) MEMbufPtr(yesSeedBuf);

    qsort((void *) yesSeedVals, n_yes_seed, 
	  sizeof(double), _compareDoubles);

    double pp = quartile_fract * (n_yes_seed - 1.0);
    int n1 = (int) pp;
    int n2 = n1 + 1;
    double partial = pp - n1;
    quart_yes_seed = yesSeedVals[n1] +
      partial * (yesSeedVals[n2] - yesSeedVals[n1]);

#ifdef OBSOLETE
    int nq = (int) (n_yes_seed * quartile_fract);
    quart_yes_seed = yesSeedVals[nq];
#endif

  } else {
    
    quart_yes_seed = PROPERTY_MISSING_DATA_VAL;

  }

  // compute no-seed quartiles

  if (n_no_seed > 2) {
    
    // sort no-seed array

    double *noSeedVals = (double  *) MEMbufPtr(noSeedBuf);
    
    qsort((void *) noSeedVals, n_no_seed, 
	  sizeof(double), _compareDoubles);

    double pp = quartile_fract * (n_no_seed - 1.0);
    int n1 = (int) pp;
    int n2 = n1 + 1;
    double partial = pp - n1;
    quart_no_seed = noSeedVals[n1] +
      partial * (noSeedVals[n2] - noSeedVals[n1]);

#ifdef OBSOLETE
    int nq = (int) (n_no_seed * quartile_fract);
    quart_no_seed = noSeedVals[nq];
#endif

  } else {
    
    quart_no_seed = PROPERTY_MISSING_DATA_VAL;

  }

  // compute diff
  
  if (quart_yes_seed != PROPERTY_MISSING_DATA_VAL &&
      quart_no_seed != PROPERTY_MISSING_DATA_VAL) {
    
    diff_in_quarts = quart_yes_seed - quart_no_seed;

    if (quart_no_seed != 0.0) {
      diff_percent = (diff_in_quarts / quart_no_seed) * 100.0;
    } else {
      diff_percent = 0.0;
    }

  } else {

    diff_in_quarts = PROPERTY_MISSING_DATA_VAL;
    diff_percent = PROPERTY_MISSING_DATA_VAL;

  }

  // free up

  MEMbufDelete(yesSeedBuf);
  MEMbufDelete(noSeedBuf);

  // set returns

  result->nYesSeed = n_yes_seed;
  result->nNoSeed = n_no_seed;

  result->yesSeedStat = quart_yes_seed;
  result->noSeedStat = quart_no_seed;

  result->diffStat = diff_in_quarts;
  result->diffPercent = diff_percent;

}


/////////////////////////////////////////////////////////
// define function to be used for sorting double values
//

int Property::_compareDoubles(const void *v1, const void *v2)

{

  double d1 = *((double *) v1);
  double d2 = *((double *) v2);
  double dd = d1 - d2;

  if (dd > 0) {
    return (1);
  } else if (dd < 0) {
    return (-1);
  } else {
    return (0);
  }

}
