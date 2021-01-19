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
// DataSet.cc
//
// DataSet object - reads in and serves out DataSet
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "DataSet.hh"
#include <cerrno>
#include <cstdlib>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <rapmath/math_macros.h>
using namespace std;

static int time_dur_compare(const void *v1, const void *v2);

// Constructor

DataSet::DataSet(const char *prog_name,
                 const Params &params) :
        _params(params)

{

  OK = TRUE;
  
  _progName = STRdup(prog_name);

  if (_readLabels()) {
    fprintf(stderr, "ERROR - %s:DataSet::_readLabels()\n", _progName);
    OK = FALSE;
  }

  if (_readData()) {
    fprintf(stderr, "ERROR - %s:DataSet::_readData()\n", _progName);
    OK = FALSE;
  }
  
  return;
  
}

// destructor

DataSet::~DataSet()

{

  STRfree(_progName);
  ufree(_inputData);

}

////////////////
// _readLabels()
//
// load data label positions from stdin
//
// Errors go to log file
//

#define LINE_MAX_N 1024

int DataSet::_readLabels()
  
{

  char *token;
  char *paren_p;
  char line[LINE_MAX_N];
  int retval = 0;
  int n_match;
  int i;

  _timePos = -1;
  _durPos = -1;
  _condPos = -1;

  while (!feof(stdin)) {

    if (fgets(line, LINE_MAX_N, stdin) != NULL) {

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
	  
	  if (!strncmp(_params.time_label, token, n_match)) {
	    STRncopy(_timeLabel, token, DATASET_LABEL_MAX);
	    _timePos = i;
	  }

	  if (!strncmp(_params.dur_label, token, n_match)) {
	    STRncopy(_durLabel, token, DATASET_LABEL_MAX);
	    _durPos = i;
	  }

	  if (!strncmp(_params.conditional_label, token, n_match)) {
	    STRncopy(_condLabel, token, DATASET_LABEL_MAX);
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

  if (_timePos < 0) {
    fprintf(stderr, "ERROR - %s:_readLabels\n", _progName);
    fprintf(stderr, "time label '%s' not found\n", _params.time_label);
    retval = -1;
  }

  if (_durPos < 0) {
    fprintf(stderr, "ERROR - %s:_readLabels\n", _progName);
    fprintf(stderr, "dur label '%s' not found\n", _params.dur_label);
    retval = -1;
  }

  if (_params.condition_input_data && _condPos < 0) {
    fprintf(stderr, "ERROR - %s:_readLabels\n", _progName);
    fprintf(stderr, "cond label '%s' not found\n",
	    _params.conditional_label);
    retval = -1;
  }

  return (retval);
  
}

//////////////////
// _readData()
//
// read time and dur data arrays
//

typedef struct time_dur_list_s {
  double time, dur;
  struct time_dur_list_s *next;
} time_dur_list_t;

int DataSet::_readData()

{

  char line[LINE_MAX_N];
  char *token, *end_pt;
  char *time_str, *dur_str, *cond_str;
  
  int i;
  
  double ttime, ddur, cond_val;
  
  double mintime = LARGE_DOUBLE, maxtime = -LARGE_DOUBLE;
  double mindur = LARGE_DOUBLE, maxdur = -LARGE_DOUBLE;
  
  time_dur_list_t *this_point = NULL, *prev_point = NULL;
  time_dur_list_t *first_point = NULL; 
  
  _nData = 0;

  while (!feof(stdin)) {
    
    if (fgets(line, LINE_MAX_N, stdin) != NULL) {
      
      if (line[0] != '#') {
	
	token = strtok(line, " \n");
	i = 0;
	time_str = (char *) NULL;
	dur_str = (char *) NULL;
	cond_str = (char *) NULL;
	
	while (token != NULL) {
	  
	  if (i == _timePos)
	    time_str = token;
	  
	  if (i == _durPos)
	    dur_str = token;
	  
	  if (_params.condition_input_data && i == _condPos)
	    cond_str = token;

	  if (_params.condition_input_data) {
	    
	    if (time_str != NULL && dur_str != NULL && cond_str != NULL)
	      break;
	    
	  } else {
	    
	    if (time_str != NULL && dur_str != NULL)
	      break;
	    
	  } // if (_params.condition_input_data)

	  i++;
	  token = strtok((char *) NULL, " \n");
	  
	} // while (token ...
	
	if (time_str == NULL || dur_str == NULL) {
	  fprintf(stderr, "WARNING - %s:DataSet::_readData\n", _progName);
	  if (time_str == NULL) {
	    fprintf(stderr, "time value not found in data\n");
	  }
	  fprintf(stderr, "dur value not found in data\n");
	  continue;
	}
	
	if (_params.condition_input_data && cond_str == NULL) {
	  fprintf(stderr, "WARNING - %s:DataSet::_readData\n", _progName);
	  fprintf(stderr, "cond value not found in data\n");
	  continue;
	}
	
	// ignore if either data val is missing

	if (!strncmp(time_str, "-9999", 5) || !strncmp(dur_str, "-9999", 5)) {
	  continue;
	}
	
	errno = 0;
	ttime = strtod(time_str, &end_pt);
	if (errno) {
	  fprintf(stderr, "WARNING - %s:DataSet::_readData\n", _progName);
	  fprintf(stderr, "Error in data stream, reading time\n");
	  perror(time_str);
	  continue;
	}

	errno = 0;
	ddur = strtod(dur_str, &end_pt);
	if (errno) {
	  fprintf(stderr, "WARNING - %s:DataSet::_readData\n", _progName);
	  fprintf(stderr, "Error in data stream, reading dur\n");
	  perror(dur_str);
	  continue;
	}
	
	// check limits as required
	
	if (_params.limit_time_data) {
	  if (ttime < _params.time_min || ttime > _params.time_max) {
	    continue;
	  }
	}

	if (_params.limit_dur_data) {
	  if (ddur < _params.dur_min || ddur > _params.dur_max) {
	    continue;
	  }
	}
	
	// check conditional data
	
	if (_params.condition_input_data) {
	  
	  errno = 0;
	  cond_val = strtod(cond_str, &end_pt);
	  if (errno) {
	    fprintf(stderr, "WARNING - %s:DataSet::_readData\n", _progName);
	    fprintf(stderr, "Error in data stream, reading cond val\n");
	    perror(cond_str);
	    continue;
	  }
	  
	  if (cond_val < _params.cond_min ||
	      cond_val > _params.cond_max) {
	    continue;
	  }
	  
	} // if (_params.condition_input_data)

	// add point to linked list
	
	this_point = (time_dur_list_t *) umalloc
	  (sizeof(time_dur_list_t));
	this_point->next = (time_dur_list_t *) NULL;
	
	if (_nData == 0)
	  first_point = this_point;
	else
	  prev_point->next = this_point;
	
	this_point->time = ttime;
	this_point->dur = ddur;
	
	_nData++;
	prev_point = this_point;
	
      } // if (line[0] ...
      
    } // if (fgets ...
    
  } // while

  // allocate the time and dur arrays

  _inputData = (time_dur_t *) umalloc (_nData * sizeof(time_dur_t));

  // set min and max vals

  _timeMin = mintime;
  _timeMax = maxtime;
  _durMin = mindur;
  _durMax = maxdur;
  
  // load up data array, free up linked list

  this_point = first_point;

  time_dur_t *id = _inputData;
  for (i = 0; i < _nData; i++, id++) {
    id->time = this_point->time;
    id->dur = this_point->dur;
    prev_point = this_point;
    this_point = this_point->next;
    ufree((char *) prev_point);
  }

  // sort data according to time

  qsort(_inputData, _nData, sizeof(time_dur_t), time_dur_compare);

  return (0);

}

// define function to be used for sorting

static int time_dur_compare(const void *v1, const void *v2)

{

    time_dur_t *td1 = (time_dur_t *) v1;
    time_dur_t *td2 = (time_dur_t *) v2;
    double diff = td1->time - td2->time;
    
    if (diff > 0) {
      return (1);
    } else if (diff < 0) {
      return (-1);
    } else {
      return(0);
    }

}

//////////////////////////////////////////////////
// Reset
//
// Reset pos pointer to start of data.
//

void DataSet::Reset()
{
  _dataPos = 0;
}

//////////////////////////////////////////////////
// Reset(int)
//
// Reset pos pointer to given point in data.
//

void DataSet::Reset(int pos)
{
  _dataPos = pos;
}

//////////////////////////////////////////////////
// Next
//
// Get next data point, advance pos pointer by 1.
//
// Returns 0 on success, -1 on failure

int DataSet::Next(double *time_p, double *dur_p)
{

  if (_dataPos >= _nData) {
    return (-1);
  }

  *time_p = _inputData[_dataPos].time;
  *dur_p = _inputData[_dataPos].dur;

  _dataPos++;

  return (0);

}

//////////////////////////////////////////////////
// Get
//
// Get specified data point
//
// Returns 0 on success, -1 on failure

int DataSet::Get(int pos, double *time_p, double *dur_p)
{

  if (pos >= _nData) {
    return (-1);
  }

  *time_p = _inputData[pos].time;
  *dur_p = _inputData[pos].dur;

  return (0);

}

//////////////////////////////////////////////////
// Size
//
// Returns data set size

int DataSet::Size()
{
  return (_nData);
}

//////////////////////////////////////////////////
// GetActivePeriod ()
//
// Computes end of first active period after
// the start_pos.
//
// Returns 0 on success, -1 on failure

int DataSet::GetActivePeriod(int start_pos, int *end_pos_p)

{

  if (start_pos >= _nData) {
    // past end of data
    return (-1);
  }
  
  if (start_pos == _nData - 1) {
    // last point - period is one point long
    *end_pos_p = start_pos;
    return (0);
  }

  // search for gap

  double time, dur;
  Get(start_pos, &time, &dur);
  
  double activity_end_time = time + dur;
  double max_gap = _params.activity_gap_max;
  
  // search for gap

  for (int i = start_pos + 1; i < _nData; i++) {
    
    Get(i, &time, &dur);

    if (time - activity_end_time > max_gap) {

      // period of activity has ended

      *end_pos_p = i - 1;
      return (0);

    }

    activity_end_time = time +  dur;

  } // i

  // period goes to end of data

  *end_pos_p = _nData - 1;
  return (0);

}

