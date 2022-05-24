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
// PrecipSeries.cc
//
// PrecipSeries object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <vector>
#include <fstream>
#include <iomanip>
#include <toolsa/pmu.h>
#include "PrecipSeries.hh"
#include "InputMdv.hh"
#include "Trigger.hh"
using namespace std;

// Constructor

PrecipSeries::PrecipSeries(int argc, char **argv) 

{

  isOK = true;

  // set programe name

  _progName = "PrecipSeries";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // check start and end times are set

  if (_args.startTime == 0 || _args.endTime == 0) {
    cerr << "ERROR: " << _progName;
    cerr << "  Must set start and end times." << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

PrecipSeries::~PrecipSeries()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int PrecipSeries::Run ()
{

  // register with procmap

  PMU_auto_register("PrecipSeries::Run");

  // create input MDV file object

  InputMdv inputMdv(_progName, _params);
  if (!inputMdv.isOK) {
    return (-1);
  }

  // set up trigger object

  ArchiveTrigger trigger(_progName, _params,
			 _args.startTime, _args.endTime);

  // create vector for rates

  vector<rate_at_time_t> rateArray;
  
  // read through the files, loading up the accum array

  time_t thisTime;
  cerr << "Input times:" << endl;
  while ((thisTime = trigger.next()) != -1) {
    rate_at_time_t thisPrecip;
    if (_handleTime(thisTime, inputMdv, thisPrecip) == 0) {
      rateArray.push_back(thisPrecip);
    }
  }

  // debug print

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (unsigned int jj = 0; jj < rateArray.size(); jj++) {
      cout << utimstr(rateArray[jj].time) << ":  ";
      for (int ii = 0; ii < _params.locations_n; ii++) {
	cout << _params._locations[ii].name << ":"
	     << rateArray[jj].loc[ii] << "  ";
      } // ii
      cout << endl;
    } // jj
  }

  // compute accumulation array location by location

  for (int ii = 0; ii < _params.locations_n; ii++) {

    vector<accum_at_time_t> accumArray;

    // initialize at the start time

    accum_at_time_t start;
    start.time = rateArray[0].time;
    start.depth = 0.0;
    accumArray.push_back(start);

    // compute the depths for rest of times

    _computeAccum(ii, rateArray, accumArray);

    // debug print

    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "Accum for station: " << _params._locations[ii].name << endl;
      for (unsigned int jj = 0; jj < accumArray.size(); jj++) {
	cerr << "  " << utimstr(accumArray[jj].time) << ":  " 
	     << accumArray[jj].depth << endl;
      } // jj
    }

    // interpolate to constant delta-t

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "---------------------------------------------" << endl;
      cerr << "Deltas for station: " << _params._locations[ii].name << endl;
    }

    if (_interpAccum(_params._locations[ii].name, accumArray)) {
      return (-1);
    }

  } // ii

  return (0);

}

int PrecipSeries::_handleTime(time_t thisTime,
			      InputMdv &inputMdv,
			      rate_at_time_t &thisPrecip)

{

  // read in radar file

  if (inputMdv.read(thisTime)) {
    cerr << "ERROR - readMdv for time " << utimstr(thisTime);
    return (-1);
  }

  // compute precip at each point

  thisPrecip.time = thisTime;
  for (int i = 0; i < _params.locations_n; i++) {
    double rate =
      inputMdv.precipRateCompute(_params._locations[i].lat,
				 _params._locations[i].lon);
    thisPrecip.loc.push_back(rate);
  }

  return (0);

}


void PrecipSeries::_computeAccum(int locIndex,
				 vector<rate_at_time_t> &rateArray,
				 vector<accum_at_time_t> &accumArray)

{

  for (unsigned int i = 1; i < rateArray.size() - 1; i++) {

    double dtime = (rateArray[i+1].time - rateArray[i-1].time) / 2.0;

    if (dtime > _params.max_time_between_volumes) {
      dtime = _params.max_time_between_volumes;
    }

    accum_at_time_t accum;
    accum.time = rateArray[i].time;
    accum.depth = accumArray[i-1].depth +
      (dtime * rateArray[i].loc[locIndex]);

    accumArray.push_back(accum);
    
  } // i

}

int PrecipSeries::_interpAccum(string name,
			       vector<accum_at_time_t> &accumArray)

{

  unsigned inputIndex = 0;
  double interpDepth;
  double prevDepth = accumArray[0].depth;
  double checkAccum = 0.0;
  int prevDay = -1;

  // open output path

  string outPath = _params.output_precip_dir;
  outPath += PATH_DELIM;
  outPath += name;

  ofstream out(outPath.c_str());
  if (!out) {
    cerr << "ERROR - " << _progName << ":_interpAccum" << endl;
    cerr << "  Cannot open file for output: " << outPath << endl;
    return (-1);
  }

  for (time_t thisTime = _args.startTime;
       thisTime <= _args.endTime;
       thisTime += _params.output_delta_t) {

    // for start of new day, print out date
    
    int day = thisTime / 86400;
    if (day != prevDay) {
      date_time_t otime;
      otime.unix_time = thisTime;
      uconvert_from_utime(&otime);
      out << endl;
      out << otime.year << " " << otime.month << " "
	  << otime.day;
      prevDay = day;
    }

    while (accumArray[inputIndex].time < thisTime) {
      if (inputIndex == accumArray.size() - 1) {
	break;
      }
      inputIndex++;
    }

    if (inputIndex == 0) {

      interpDepth = accumArray[0].depth;

    } else if (inputIndex == accumArray.size()) {

      interpDepth = accumArray[accumArray.size()-1].depth;

    } else {

      double inDt =
	accumArray[inputIndex].time - accumArray[inputIndex-1].time;

      double inDd =
	accumArray[inputIndex].depth - accumArray[inputIndex-1].depth;

      double outDt = thisTime - accumArray[inputIndex-1].time;

      double fraction = outDt / inDt;

      interpDepth = accumArray[inputIndex-1].depth + fraction * inDd;
      
    }

    double accumThisPeriod = interpDepth - prevDepth;
    checkAccum += accumThisPeriod;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cout << utimstr(thisTime) << ": " << accumThisPeriod << " mm "
	   << "  checkAccum: " << checkAccum << endl;
    }

    prevDepth = interpDepth;

    // printout
    
    if (accumThisPeriod == 0.0) {
      out << " 0.0";
    } else {
      out << setprecision(2);
      out << " " << accumThisPeriod;
    }

  } // thisTime

  return (0);

}
