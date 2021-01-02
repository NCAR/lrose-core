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
// Properties.cc
//
// Case Props Array object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#include "CaseProps.hh"
#include "RandomList.hh"
#include <toolsa/str.h>
using namespace std;

//////////////
// Constructor

CaseProps::CaseProps (const char *prog_name,
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
                      int max_time_error_for_one_ended_interp)

{
  
  // set pointers & vals

  _progName = STRdup(prog_name);
  _debug = debug;
  _setMissingValInInterp = set_missing_val_in_interp;
  _allowOneEndedInterp = allow_one_ended_interp;
  _maxTimeErrorForOneEndedInterp = max_time_error_for_one_ended_interp;
  _propsFilesDir = STRdup(props_files_dir);
  _nCases = 0;

  // intialize cases array

  _casesMbuf = MEMbufCreate();
  _cases = (SeedCaseTracks::CaseTrack *) MEMbufPtr(_casesMbuf);

  // intialize seed flag array

  _seedFlagMbuf = MEMbufCreate();
  _seedFlags = (int *) MEMbufPtr(_seedFlagMbuf);

  // intialize active flag array

  _activeFlagMbuf = MEMbufCreate();
  _activeFlags = (int *) MEMbufPtr(_activeFlagMbuf);

  // initialize props array

  _nProps = 0;
  _propsMbuf = MEMbufCreate();
  _props = (Property **) MEMbufPtr(_propsMbuf);

  // initialize groups array

  _nextGroupStart = 0;
  _nGroups = 0;
  _groupsMbuf = MEMbufCreate();
  _groups = (prop_group_t *) MEMbufPtr(_groupsMbuf);

  // load up the global props

  for (int i = 0; i < n_global_props; i++) {
    _addGlobalProp(global_props[i]);
  }
  _addGroup((char *) "Global properties", n_global_props);

  // load up the time series props

  _nTseriesProps = n_tseries_props;
  if (_nTseriesProps > 0) {
    _tseriesProps = (char **) umalloc(_nTseriesProps * sizeof(char *));
    for (int i = 0; i < _nTseriesProps; i++) {
      _tseriesProps[i] = STRdup(tseries_props[i]);
    }
  }
  _nTseriesDtimes = n_tseries_dtimes;
  if (_nTseriesDtimes > 0) {
    _tseriesDtimes = (int *) umalloc(_nTseriesDtimes * sizeof(int));
  }
  for (int j = 0; j < _nTseriesDtimes; j++) {
    _tseriesDtimes[j] = tseries_dtimes[j];
  } // j
  
  for (int i = 0; i < n_tseries_props; i++) {
    for (int j = 0; j < _nTseriesDtimes; j++) {
      _addTseriesProp(tseries_props[i], _tseriesDtimes[j]);
    } // j
    char label[512];
    sprintf(label, "Time series property: %s", tseries_props[i]);
    _addGroup(label, _nTseriesDtimes);
  } // i

  // initialize conditional props array

  _nCondProps = 0;
  _condPropsMbuf = MEMbufCreate();
  _condProps = (CondProp **) MEMbufPtr(_condPropsMbuf);

  // load up the conditional properties

  for (int i = 0; i < n_conditional_props; i++) {
    _addConditionalProp(conditional_props + i);
  }


}

/////////////
// destructor

CaseProps::~CaseProps ()
  
{

  // cases

  MEMbufDelete(_casesMbuf);

  // properties

  for (int i = 0; i < _nProps; i++) {
    delete(_props[i]);
  }
  MEMbufDelete(_propsMbuf);

  // property groups

  for (int i = 0; i < _nGroups; i++) {
    STRfree(_groups[i].label);
  }
  MEMbufDelete(_groupsMbuf);

  // time series

  if (_nTseriesProps > 0) {
    for (int i = 0; i < _nTseriesProps; i++) {
      STRfree(_tseriesProps[i]);
    }
    ufree(_tseriesProps);
  }
  if (_nTseriesDtimes > 0) {
    ufree(_tseriesDtimes);
  }

  // conditional properties

  for (int i = 0; i < _nCondProps; i++) {
    delete(_condProps[i]);
  }
  MEMbufDelete(_condPropsMbuf);

  // seed flag array

  MEMbufDelete(_seedFlagMbuf);

  // active flag array

  MEMbufDelete(_activeFlagMbuf);

  // strings

  STRfree(_propsFilesDir);
  STRfree(_progName);

}

///////////////////
// addCase()
//
// Add a case to the CaseProps array
//

int CaseProps::addCase(SeedCaseTracks::CaseTrack *this_case, FILE *out)

{

  // get value for each property for this case

  for (int i = 0; i < _nProps; i++) {

    Property *prop = _props[i];
    double val;
    int iret;

    if (prop->fromTseries()) {
      
      iret = _getTseriesProp(prop->name(), prop->deltaTime(),
			     this_case->num,
			     &val);
      if (iret == 0) {
	prop->addVal(val);
      }	

    } else {
      
      iret = _getGlobalProp(prop->name(), this_case->num, &val);
      if (iret == 0) {
	prop->addVal(val);
      }	

    }

    if (iret) {
      fprintf(stderr, "ERROR - %s:CaseProps::addCase\n", _progName);
      fprintf(stderr, "Cannot add case %d, prop %s\n",
	      this_case->num, prop->name());
      return (-1);
    }

  } // i

  // get value for each conditional property for this case and
  // determine whether the case should be active

  int active_flag = TRUE;

  for (int i = 0; i < _nCondProps; i++) {

    CondProp *prop = _condProps[i];
    double val;
    int iret;

    if (prop->fromTseries()) {
      
      iret = _getTseriesProp(prop->name(), prop->deltaTime(),
			     this_case->num, &val);
      if (iret == 0) {
	prop->addVal(val);
      }	

    } else {
      
      iret = _getGlobalProp(prop->name(), this_case->num, &val);
      if (iret == 0) {
	prop->addVal(val);
      }	

    }

    if (iret) {
      fprintf(stderr, "ERROR - %s:CaseProps::addCase\n", _progName);
      fprintf(stderr, "Cannot add case %d, conditional prop %s\n",
	      this_case->num, prop->name());
      return (-1);
    }

    // determine if this case should be active or not

    if (val != PROPERTY_MISSING_DATA_VAL &&
	!prop->withinLimits(_nCases)) {
      fprintf(out, "  Note: rejecting case %3d base on %s value of %0g\n",
	      this_case->num, prop->name(), val);
      active_flag = FALSE;
    }

  } // i

  // add seed flag to array
  
  int seed_flag = this_case->seed_flag;
  MEMbufAdd(_seedFlagMbuf, &seed_flag, sizeof(int));
  _seedFlags = (int *) MEMbufPtr(_seedFlagMbuf);
 
  // add active flag to array
  
  MEMbufAdd(_activeFlagMbuf, &active_flag, sizeof(int));
  _activeFlags = (int *) MEMbufPtr(_activeFlagMbuf);

  // cases array

  MEMbufAdd(_casesMbuf, this_case, sizeof(SeedCaseTracks::CaseTrack));
  _cases = (SeedCaseTracks::CaseTrack *) MEMbufPtr(_casesMbuf);
  _nCases++;

  return (0);

}

/////////////////
// computeStats()
//
// Compute stats for each property
//

void CaseProps::computeStats(int stat_type)
  
{

  for (int i = 0; i < _nProps; i++) {
    _props[i]->computeMeasuredStats(stat_type, _nCases,
				    _seedFlags, _activeFlags);
  }

}

/////////////
// doRerand()
//
// Perform rerandomization
//

void CaseProps::doRerand(int stat_type, int n_rerand,
			 int n_random_list, int max_split)

{

  RandomList *rList =
    new RandomList(_progName, _debug, n_random_list, max_split);

  for (int i = 0; i < n_rerand; i++) {

    // generate a new list

    int *list = rList->generate();

    // compute rerandomized stats

    for (int j = 0; j < _nProps; j++) {
      _props[j]->computeRerandStats(stat_type, _nCases,
				    list, _activeFlags);
    }

  } // i

  // compute Pfactors

  for (int j = 0; j < _nProps; j++) {
    _props[j]->computePfactor();
  }
  
  // clean up

  delete (rList);

}

/////////////////
// printStats()
//
// Print stats for each property
//

void CaseProps::printStats(FILE *out)
  
{

  // print cases used

  fprintf(out, "\n");
  fprintf(out, "Using cases:");
  int count = 0;
  for (int i = 0; i < _nCases; i++) {
    if (_activeFlags[i]) {
      fprintf(out, " %3d", _cases[i].num);
      count++;
    }
    if (count == 15) {
      fprintf(out, "\n");
      fprintf(out, "            ");
      count = 0;
    }
  }
  fprintf(out, "\n\n");

  // print property headers

  Property::printHeader(out, _setMissingValInInterp);

  // print property stats

  for (int i = 0; i < _nGroups; i++) {

    // print label and underline

    fprintf(out, "\n%s\n", _groups[i].label);
    int len = strlen(_groups[i].label);
    for (int j = 0; j < len; j++) {
      fprintf(out, "=");
    }
    fprintf(out, "\n\n");

    // print properties in this group

    for (int k = _groups[i].start_prop_num;
	 k <=  _groups[i].end_prop_num; k++) {
      _props[k]->printStats(out, _setMissingValInInterp);
    }
    
  } // i

  fprintf(out, "\n");
  fprintf(out, "\n");

}

///////////////
// printDebug()

void CaseProps::printDebug(FILE *out)
  
{

  fprintf(out, "\n\n");

  fprintf(out, "SEED FLAG ARRAY\n\n");
  for (int i = 0; i < _nCases; i++) {
    fprintf(out, "%d ", _seedFlags[i]);
  }
  fprintf(out, "\n\n");
  
  fprintf(out, "ACTIVE FLAG ARRAY\n\n");
  for (int i = 0; i < _nCases; i++) {
    fprintf(out, "%d ", _activeFlags[i]);
  }
  fprintf(out, "\n\n");
  
  fprintf(out, "CASEPROPS LIST\n\n");
  for (int i = 0; i < _nProps; i++) {
    _props[i]->printDebug(out, i);
  }

  fprintf(out, "\n\n");

}

/////////////////
// writeInterp()
//
// Write time-series interpolated files
//

void CaseProps::writeInterp()
  
{

  // loop through cases

  for (int icase = 0; icase < _nCases; icase++) {

    SeedCaseTracks::CaseTrack *this_case = _cases + icase;

    // compute file path

    char output_path[MAX_PATH_LEN];
    sprintf(output_path, "%s%s%s.%.3d",
	    _propsFilesDir, PATH_DELIM, "interp", this_case->num);
    
    // open interp output file

    FILE *interpFile;
    
    if ((interpFile = fopen(output_path, "w")) == NULL) {
      fprintf(stderr, "ERROR - %s:CaseProps::writeInterp\n", _progName);
      fprintf(stderr, "Cannot open interpolated time series output file\n");
      perror(output_path);
      continue;
    }
  
    // write out the time series props labels at top of file
    
    fprintf(interpFile, "ntimes: %d\n", _nTseriesDtimes);
    for (int iprop = 0; iprop < _nTseriesProps; iprop++) {
      fprintf(interpFile, "%s ", _tseriesProps[iprop]);
    }
    fprintf(interpFile, "\n");

    // write out the interpolated data for each time

    for (int itime = 0; itime < _nTseriesDtimes; itime++) {

      fprintf(interpFile, "%5d ", _tseriesDtimes[itime]);

      for (int igroup = 1; igroup < _nGroups; igroup++) {
	prop_group_t *group = _groups + igroup;
	int prop_num = group->start_prop_num + itime;
	fprintf(interpFile, "%9.4f ", _props[prop_num]->val(icase));
      } // igroup

      fprintf(interpFile, "\n");
      
    } // itime
    
    // close file

    fclose(interpFile);
    
  } // icase
  
}

///////////////////// PRIVATE ROUTINES /////////////////////////////

//////////////
// _addGroup()
//
// Add a property group
//

void CaseProps::_addGroup(char *label, int nprops)

{

  prop_group_t group;
  
  group.label = STRdup(label);
  group.start_prop_num = _nextGroupStart;
  group.end_prop_num = group.start_prop_num + nprops - 1;
  _nextGroupStart = group.end_prop_num + 1;

  // add to buffer

  MEMbufAdd(_groupsMbuf, &group, sizeof(prop_group_t));

  // update pointer
  
  _groups = (prop_group_t *) MEMbufPtr(_groupsMbuf);
  
  _nGroups++;

}


///////////////////
// _addGlobalProp()
//
// Add a global property to the CaseProps array
//

void CaseProps::_addGlobalProp(char *name)

{

  // create new property
  
  Property *prop = new Property(_progName, _debug, name);

  // add to buffer

  MEMbufAdd(_propsMbuf, &prop, sizeof(Property *));

  // update pointer
  
  _props = (Property **) MEMbufPtr(_propsMbuf);
  
  _nProps++;

}

///////////////////
// _addTseriesProp()
//
// Add a time_series property to the CaseProps array
//

void CaseProps::_addTseriesProp(char *name, int delta_time)

{

  // create new property
  
  Property *prop = new Property(_progName, _debug, name, delta_time);

  // add to buffer

  MEMbufAdd(_propsMbuf, &prop, sizeof(Property *));

  // update pointer
  
  _props = (Property **) MEMbufPtr(_propsMbuf);
  
  _nProps++;

}

////////////////////////
// _addConditionalProp()
//
// Add a conditional property to the CaseProps array
//

void CaseProps::_addConditionalProp(Params::condition *cond)

{

  // create new property
  
  Property *prop =
    new CondProp(_progName, _debug,
		 cond->prop_name, cond->min_val, cond->max_val);

  // add to buffer

  MEMbufAdd(_condPropsMbuf, &prop, sizeof(CondProp *));

  // update pointer
  
  _condProps = (CondProp **) MEMbufPtr(_condPropsMbuf);
  
  _nCondProps++;

}

/////////////////////////////////////////////
// _getGlobalProp()
//
// Get a global property for the case
//
// Returns 0 on success, -1 on failure
//

int CaseProps::_getGlobalProp(char *prop_name,
			      int case_num,
			      double *val_p)

{

  // initialize in case of error return

  *val_p = 0.0;

  // open global props file

  char file_path[MAX_PATH_LEN];
  
  sprintf(file_path, "%s%s%s.%.3d",
	  _propsFilesDir, PATH_DELIM, "global", case_num);

  FILE *fp;
  
  if ((fp = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:CaseProps::_getGlobalProp\n", _progName);
    fprintf(stderr, "Cannot open global props file\n");
    perror(file_path);
    return (-1);
  }

  // create format
  
  char format[256];
  sprintf(format, "%s:%%lg", prop_name);

  // read in file, match format

  char line[1024];

  while (fgets(line, 1024, fp) != NULL) {

    double val;
    if (sscanf(line, format, &val) == 1) {
      *val_p = val;
      fclose(fp);
      return (0);
    }

  } // while

  // not found - error return

  fclose(fp);
  return (-1);

}

/////////////////////////////////////////////
// _getTseriesProp()
//
// Get a time series property for the case
//

int CaseProps::_getTseriesProp(char *prop_name,
			       int delta_time,
			       int case_num,
			       double *val_p)

{

  // initialize in case of error return

  *val_p = 0.0;

  // open tseries props file

  char file_path[MAX_PATH_LEN];
  
  sprintf(file_path, "%s%s%s.%.3d",
	  _propsFilesDir, PATH_DELIM, "tseries", case_num);

  FILE *fp;
  
  if ((fp = fopen(file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s:CaseProps::_getTseriesProp\n", _progName);
    fprintf(stderr, "Cannot open time series props file\n");
    perror(file_path);
    return (-1);
  }

  // read in header

  int nscans;
  int dtime_index;
  int prop_index;
  
  if (_readTseriesHeader(fp, prop_name,
			 &nscans, &dtime_index, &prop_index)) {
    fprintf(stderr, "ERROR - %s:CaseProps::_getTseriesProp\n", _progName);
    fprintf(stderr, "Cannot read header in time series props file %s\n",
	    file_path);
    fclose(fp);
    return (-1);
  }

  // read in array of dtimes and prop vals

  vector<int> dtimes;
  vector<double> vals;
  int dtime;
  double val;
  while (_readTseriesLine(fp, dtime_index, prop_index,
			  &dtime, &val) == 0) {
    dtimes.push_back(dtime);
    vals.push_back(val);
  }
  
  // close file

  fclose(fp);

  if (vals.size() > 0) {

    // get the interpolated value - if missing the MISSING_DATA_VAL
    // will be interted instead
    
    _getTseriesInterpVal(dtimes, vals, delta_time, val_p);

  }
  
  return (0);

}

/////////////////////////////////////////////
// _readTseriesHeaders()
//
// Read the time series file header, to get
// number of scans and the index position in
// each line for the delta_time and property.
//
// Returns 0 on success, -1 on error
//

int CaseProps::_readTseriesHeader(FILE *fp,
				  char *prop_name,
				  int *nscans_p,
				  int *dtime_index_p,
				  int *prop_index_p)

{

  char line[1024];
  int nscans;
  int dtime_index = 0;
  int prop_index = 0;
  
  // read in number of scans

  if(fgets(line, 1024, fp) == NULL) {
    return (-1);
  }
  if (sscanf(line, "nscans:%d", &nscans) != 1) {
    return (-1);
  }
  
  // read in variable name line

  if(fgets(line, 1024, fp) == NULL) {
    return (-1);
  }

  // tokenize line to get index positions

  int dtime_found = FALSE;
  int prop_found = FALSE;
  int index = 0;

  char *tok = strtok(line, " \t\n");
  while (tok != NULL) {
    if (!strcmp(tok, "delta_time")) {
      dtime_found = TRUE;
      dtime_index = index;
    }
    if (!strcmp(tok, prop_name)) {
      prop_found = TRUE;
      prop_index = index;
    }
    tok = strtok(NULL, " \t\n");
    index++;
  }

  if (dtime_found && prop_found) {

    // success

    *nscans_p = nscans;
    *dtime_index_p = dtime_index;
    *prop_index_p = prop_index;

    return (0);

  } else {

    // error - indices not found

    return (-1);

  }

}

/////////////////////////////////////////////
// _getTseriesInterpVal()
//
// Get the data from the time series file,
// and interpolate to the required delta time.
//
// If there is not data at delta_time,
// the value is returned as PROPERTY_MISSING_DATA_VAL.
//
// Returns 0 on success, -1 on failure.
//

int CaseProps::_getTseriesInterpVal(const vector<int> &dtimes,
                                    const vector<double> &vals,
				    int requested_delta_time,
				    double *interp_val_p)

{

  // Set missing value for storms which do not exist at interp time.
  // This is either the missing val flag, or 0.0
  
  if (_setMissingValInInterp) {
    *interp_val_p = PROPERTY_MISSING_DATA_VAL;
  } else {
    *interp_val_p = 0.0;
  }

  int nTimes = (int) dtimes.size();

  if (nTimes < 1) {
    return -1;
  }
  
  // look for times which straddle the requested delta time

  for (int ii = 1; ii < nTimes; ii++) {

    if (dtimes[ii-1] <= requested_delta_time && dtimes[ii] >= requested_delta_time) {
      
      // check end points, which also prevents divide-by-zero
      // during interpolation
      
      if (dtimes[ii-1] == requested_delta_time) {
	*interp_val_p = vals[ii-1];
	return 0;
      }

      if (dtimes[ii] == requested_delta_time) {
	*interp_val_p = vals[ii];
	return 0;
      }

      // interpolate
      
      double tperiod = dtimes[ii] - dtimes[ii-1];
      double tdiff = requested_delta_time - dtimes[ii-1];
      double fraction = tdiff / tperiod;
      *interp_val_p = vals[ii-1] + fraction * (vals[ii] - vals[ii-1]);
      return 0;

    }

  } // ii

  if (_allowOneEndedInterp) {

    // check for end points
    
    if (fabs(dtimes[0] - requested_delta_time) < _maxTimeErrorForOneEndedInterp) {
      *interp_val_p = vals[0];
      return 0;
    }


    if (fabs(dtimes[nTimes-1] - requested_delta_time) < _maxTimeErrorForOneEndedInterp) {
      *interp_val_p = vals[nTimes-1];
      return 0;
    }

  }

  return -1;

}

/////////////////////////////////////////////
// _readTseriesLine()
//
// Reads a line of data from the time series file.
// Loads up the delta_time and prop_val.
//
// Returns 0 on success, -1 on failure.
//

int CaseProps::_readTseriesLine(FILE *fp,
				int dtime_index,
				int prop_index,
				int *delta_time_p,
				double *val_p)

{

  char line[1024];

  // read in line
  
  if(fgets(line, 1024, fp) == NULL) {
    return (-1);
  }

  // tokenize line to get index positions

  int dtime_found = FALSE;
  int prop_found = FALSE;
  int index = 0;
  
  char *tok = strtok(line, " \t\n");

  while (tok != NULL) {

    if (index == dtime_index) {
      if (sscanf(tok, "%d", delta_time_p) != 1) {
	return (-1);
      }
      dtime_found = TRUE;
    }

    if (index == prop_index) {
      if (sscanf(tok, "%lg", val_p) != 1) {
	return (-1);
      }
      prop_found = TRUE;
    }

    tok = strtok(NULL, " \t\n");
    index++;

  } // while

  if (dtime_found && prop_found) {
    return (0);
  } else {
    return (-1);
  }

}

