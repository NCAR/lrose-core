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
// CaseProps.h
//
// Case Props Array object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef CaseProps_h
#define CaseProps_h

#include "Property.hh"
#include "CondProp.hh"
#include "Params.hh"
#include <titan/SeedCaseTracks.hh>

typedef struct {

  char *label;
  int start_prop_num;
  int end_prop_num;
  
} prop_group_t;

class CaseProps {
  
public:

  // constructor
  
  CaseProps (const char *prog_name,
	     bool debug,
	     const char *props_files_dir,
	     int n_global_props,
	     char **global_props,
	     int n_tseries_props,
	     char **tseries_props,
	     int n_tseries_dtimes,
	     int *tseries_dtimes,
	     int n_conditional_props,
	     Params::condition *conditional_props,
	     bool set_missing_val_in_interp,
             bool allow_one_ended_interp,
             int max_time_error_for_one_ended_interp);
  
  // destructor
  
  ~CaseProps();
  
  // add a case

  int addCase(SeedCaseTracks::CaseTrack *this_case, FILE *out);

  // compute stats

  void computeStats(int stat_type);

  // do rerandomization

  void doRerand(int stat_type, int n_rerand,
		int n_random_list, int max_split);

  // print stats

  void printStats(FILE *out);

  // debug print

  void printDebug(FILE *out);

  // write time-series interpolated files

  void writeInterp();
  
protected:
  
private:

  bool _debug;
  char *_progName;
  char *_propsFilesDir;
  bool _setMissingValInInterp;
  bool _allowOneEndedInterp;
  int _maxTimeErrorForOneEndedInterp;
  
  // cases

  int _nCases;
  SeedCaseTracks::CaseTrack *_cases;
  MEMbuf *_casesMbuf;

  // time-series details
  
  int _nTseriesProps;
  char **_tseriesProps;
  int _nTseriesDtimes;
  int *_tseriesDtimes;

  // property groups

  int _nextGroupStart;
  int _nGroups;
  prop_group_t *_groups;
  MEMbuf *_groupsMbuf; // MEMbuf for groups array

  // properties array

  int _nProps;
  Property **_props;
  MEMbuf *_propsMbuf; // MEMbuf for props array

  // conditional properties array

  int _nCondProps;
  CondProp **_condProps;
  MEMbuf *_condPropsMbuf; // MEMbuf for props array

  // seed flag array

  int *_seedFlags;
  MEMbuf *_seedFlagMbuf;  // MEMbuf for seed flag array

  // active flag array

  int *_activeFlags;
  MEMbuf *_activeFlagMbuf;  // MEMbuf for active flag array

  // add a global property
  
  void _addGlobalProp(char *name);

  // add a time_series property

  void _addTseriesProp(char *name, int delta_time);

  // add a conditional property

  void _addConditionalProp(Params::condition *cond);

  // add a property group

  void _addGroup(char *label, int nprops);

  // get global property

  int _getGlobalProp(char *prop_name, int case_num,
		     double *val_p);

  // get time series property

  int _getTseriesProp(char *prop_name, int delta_time,
		      int case_num, double *val_p);

  // read time series file header

  int _readTseriesHeader(FILE *fp,
			 char *prop_name,
			 int *nscans_p,
			 int *dtime_index_p,
			 int *prop_index_p);

  // get time series interpolated value

  int _getTseriesInterpVal(const vector<int> &dtimes,
                           const vector<double> &vals,
                           int requested_delta_time,
                           double *interp_val_p);

  // read time series data line

  int _readTseriesLine(FILE *fp,
		       int dtime_index,
		       int prop_index,
		       int *delta_time_p,
		       double *val_p);

};

#endif

