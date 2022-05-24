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
// PropsList.cc: PropsList handling
//
// Processes a properties file for track matching.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 1998
//
//////////////////////////////////////////////////////////

#include "PropsList.hh"
#include <toolsa/str.h>
using namespace std;

static int _props_compare(const void *v1, const void *v2);

//////////////
// Constructor

PropsList::PropsList (const char *prog_name,
		      const Params &params) :
        _params(params)

{

  // initialize
  
  _progName = STRdup(prog_name);

  // list array

  _nList = _params.n_candidates;
  _list = (initial_props_t *) ucalloc(_nList,
				      sizeof(initial_props_t));
  for (int i = 0; i < _nList; i++) {
    _list[i].delta_prop = 1.0e99;
  }

  return;

}

/////////////
// Destructor

PropsList::~PropsList()

{

  // free up

  ufree(_list);
  STRfree(_progName);

}

/////////////////////////////////////////
// scan
//
// scan a line from a properties file,
// filling in the property struct.
//

int PropsList::scan(char *line, initial_props_t *props)

{

  if (line[0] == '#') {
    return (-1);
  }

  if (sscanf(line,
	     "%d "
	     "%d %d "
	     "%d %d %d "
	     "%d %d %d "
	     "%lg %lg %lg "
	     "%lg %lg %lg %lg "
	     "%lg %lg %lg %lg",
	     &props->n_simple_tracks,
	     &props->complex_track_num, &props->simple_track_num,
	     &props->year, &props->month, &props->day,
	     &props->hour, &props->min, &props->sec,
	     &props->range, &props->xx, &props->yy,
	     &props->area, &props->volume,
	     &props->mass, &props->precip_flux,
	     &props->da_dt, &props->dv_dt,
	     &props->dm_dt, &props->dp_dt) != 20) {
    return -1;
  }

  // compute the unix time

  date_time_t ttime;
  ttime.year = props->year;
  ttime.month = props->month;
  ttime.day = props->day;
  ttime.hour = props->hour;
  ttime.min = props->min;
  ttime.sec = props->sec;
  uconvert_to_utime(&ttime);
  props->time = ttime.unix_time;

  // compute the delta between the case track this track

  return 0;

}

/////////////////////////////////////////
// update
//
// scan a line from a properties file. If it is
// a valid entry add it in the last position, sort.
//

int PropsList::update(char *line, initial_props_t *case_props)

{

  // scan in the properties

  initial_props_t props;
  if (scan(line, &props)) {
    return (-1);
  }

  // check time
  
  if (_params.time_margin >= 0) {
    int max_time_diff = (int) (_params.time_margin * 3600.0);
    if (fabs((double) props.time - (double) case_props->time) > max_time_diff) {
      return (-1);
    }
  }
  
  // check range
  
  if (_params.range_margin >= 0) {
    if (fabs(props.range - case_props->range) > _params.range_margin) {
      return (-1);
    }
  }
  
  // compute the delta between the case props and these props

  switch (_params.match_property) {
  case Params::VOLUME:
    if (_params.use_rate_for_match) {
      props.delta_prop = fabs(props.dv_dt - case_props->dv_dt);
    } else {
      props.delta_prop = fabs(props.volume - case_props->volume);
    }
    break;
  case Params::AREA:
    if (_params.use_rate_for_match) {
      props.delta_prop = fabs(props.da_dt - case_props->da_dt);
    } else {
      props.delta_prop = fabs(props.area - case_props->area);
    }
    break;
  case Params::MASS:
    if (_params.use_rate_for_match) {
      props.delta_prop = fabs(props.dm_dt - case_props->dm_dt);
    } else {
      props.delta_prop = fabs(props.mass - case_props->mass);
    }
    break;
  case Params::PRECIP_FLUX:
    if (_params.use_rate_for_match) {
      props.delta_prop = fabs(props.dp_dt - case_props->dp_dt);
    } else {
      props.delta_prop = fabs(props.precip_flux - case_props->precip_flux);
    }
    break;
  } // switch

  // add as last item in list, then sort

  _list[_nList-1] = props;

  _sort();

  return(0);

}

/////////////////////////////////////////
// print
//
// Print the list.
//

void PropsList::print(FILE *out)

{

  initial_props_t *props = _list;
  for (int i = 0; i < _nList; i++, props++) {
    print(out, props);
  }

}

/////////////////////////////////////////
// print
//
// Print a properties entry
//

void PropsList::print(FILE *out, initial_props_t *props)

{

  fprintf(out, 
	  "%5d "
	  "%5d %5d "
	  "%4d %2d %2d "
	  "%2d %2d %2d "
	  "%7.1f %7.1f %7.1f "
	  "%7.1f %7.1f %7.1f %7.1f "
	  "%7.1f %7.1f %7.1f %7.1f "
	  "\n",
	  props->n_simple_tracks,
	  props->complex_track_num, props->simple_track_num,
	  props->year, props->month, props->day,
	  props->hour, props->min, props->sec,
	  props->range, props->xx, props->yy,
	  props->area, props->volume, props->mass, props->precip_flux,
	  props->da_dt, props->dv_dt, props->dm_dt, props->dp_dt);

}

////////////////
// sort
//
// sort the list

void PropsList::_sort()

{

  // sort data according to the difference between the property of
  // the case to be matched and the actual track property

  qsort(_list, _nList, sizeof(initial_props_t), _props_compare);

}

/////////////////////////////////////////
// define function to be used for sorting

static int _props_compare(const void *v1, const void *v2)

{

  initial_props_t *ip1 = (initial_props_t *) v1;
  initial_props_t *ip2 = (initial_props_t *) v2;
  double diff = ip1->delta_prop - ip2->delta_prop;
  
  if (diff > 0) {
    return (1);
  } else if (diff < 0) {
    return (-1);
  } else {
    return(0);
  }

}

