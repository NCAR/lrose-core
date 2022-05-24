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
/////////////////////////////////////////////////////////////
// Property.h
//
// Property object - contains array of property values
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef Property_h
#define Property_h

#include <toolsa/umisc.h>
#include <toolsa/membuf.h>

#define PROPERTY_MISSING_DATA_VAL -999.0

// struct for case flags

typedef struct {

  int seed;   // seed or no-seed
  int active; // is this case active - is set by conditionals

} case_flags_t;

// struct for stats results

typedef struct {

  int nYesSeed;          // number of seed cases
  int nNoSeed;           // number of no-seed cases

  int diffActivity;      // nYesSeed - nNoSeed

  double yesSeedStat;    // stat for seed cases
  double noSeedStat;     // stat for no-seed cases

  double diffStat;       // yesSeedStat - noSeedStat
  double diffPercent;    // (diffStat / noSeedStat) * 100.0

} stat_result_t;

class Property {
  
public:

  // empty constructor for derived classes

  Property ();
  
  // constructor for global props
       
  Property (char *prog_name, int debug, char *name);
  
  // constructor for time_series props
       
  Property (char *prog_name, int debug, char *name,
	    int delta_time);
  
  // destructor
  
  virtual ~Property();

  // add value

  void addVal(double val);
	      
  // compute stats for measured values

  void computeMeasuredStats(int stat_type,
			    int n_cases,
			    int *seed_flags,
			    int *active_flags);

  // compute rerandomized stats

  void computeRerandStats(int stat_type,
			  int n_cases,
			  int *seed_flags,
			  int *active_flags);

  // compute Pfactor

  void computePfactor();
  
  // print header

  static void printHeader(FILE *out, int print_activity);

  // print stats

  void printStats(FILE *out, int print_activity);

  // debug print

  void printDebug(FILE *out, int prop_num);

  // data member access

  char *name() {return (_name);}
  int fromTseries() {return (_fromTseries);}
  int deltaTime() {return (_deltaTime);}
  double val(int icase) {return(_vals[icase]);}

protected:
  
  // private data members

  char *_progName;   // program name

  int _debug;        // debug flag
  
  MEMbuf *_mbuf;     // MEMbuf for vals array

  char *_name;       // property name

  int _fromTseries;  // flag to indicate whether the property is from
                    // a time series or not

  int _deltaTime;    // for tseries property, this is the delta time
                    // from decision time

  int _nVals;        // number of vals in the array
  
  double *_vals;     // property value array

  stat_result_t _measuredResult; // actual result measured in field

  stat_result_t _rerandResult;   // rerandomized result
  
  int _nPResults; // number of times the real result diff exceeds the
                  // re-randomized result
  
  int _nRerandResults;  // number of re-randomizations in which results differed

  double _pFactorResults; // P-factor for results

  int _nPActivity; // number of times the diff between active seed and no-seed
                   // cases exceeds the re-randomized value

  int _nRerandActivity; // number of re-randomizations in which nYesSeed and nNoSeed
                        // were not equal

  double _pFactorActivity; // P-factor for activity

  // protected functions

  void _init(char *prog_name, int debug, char *name);
  
  void _computeMeans(int stat_type, int n_cases,
		     int *seed_flags, int *active_flags,
		     stat_result_t *result);

  void _computeQuartiles(int stat_type, int n_cases,
			 int *seed_flags, int *active_flags,
			 stat_result_t *result);

  static int _compareDoubles(const void *v1, const void *v2);

private:

};

#endif
