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


#include <cstdio>
#include <toolsa/file_io.h>
#include <toolsa/umisc.h>

/**
 *
 * @file statistician.cc
 *
 * Implementation of statistician class.
 *
 * @author Niles Oien
 *
 */

#include "statistician.hh"
using namespace std;

statistician::statistician(string name,
			   time_t startTime,
			   string outDir,
			   int outputInterval){
  //
  // Set everything to 0 and make a local copy
  // of the name.
  //
  _totalNumNon = 0.0;
  _totalNumFail = 0.0; 
  _totalNumFalse = 0.0;
  _totalNumHit = 0.0;

  _subtotalNumNon = 0.0;
  _subtotalNumFail = 0.0; 
  _subtotalNumFalse = 0.0;
  _subtotalNumHit = 0.0;

  _num = 0; _subnum=1;

  _name = name;
  _startTime = startTime;
  _outDir = outDir + "/" + name;
  _outputInterval = outputInterval;

  _lastOutputTime = _startTime;

  _lastTruthTime = _startTime;

  return;

}

/////////////////////////////////////////////////
void statistician::reset(time_t newTime){

  //
  // Set everything to 0 and set the time as specified
  //
  _totalNumNon = 0.0;
  _totalNumFail = 0.0; 
  _totalNumFalse = 0.0;
  _totalNumHit = 0.0;

  _subtotalNumNon = 0.0;
  _subtotalNumFail = 0.0; 
  _subtotalNumFalse = 0.0;
  _subtotalNumHit = 0.0;

  _num = 0; _subnum=0;

  _startTime = newTime;
  _lastTruthTime = newTime;

  _lastOutputTime = _startTime;

  return;

}


/////////////////////////////////////////////////

void statistician::init(){
  //
  // Make a local copy of the start time and
  // open the output file.
  //
  char FileName[MAX_PATH_LEN], TimeStr[16];
  date_time_t BeginTime;

  BeginTime.unix_time = _startTime;
  uconvert_from_utime(&BeginTime);  

  sprintf(TimeStr,"%4d%02d%02d_%02d%02d%02d",
	  BeginTime.year, BeginTime.month, BeginTime.day,
	  BeginTime.hour, BeginTime.min, BeginTime.sec);

  /*
  fprintf(stderr,"Statistician %s started with %s\n",
	  _name.c_str(), utimstr(_startTime));
  */

  // Create the output directory

  if (ta_makedir_recurse(_outDir.c_str()) != 0)
  {
    fprintf(stderr, "Failed to create stats dir: %s\n", _outDir.c_str());
    exit(-1);
  }
  

  sprintf(FileName,"%s/Valid_%s_%s.dat", _outDir.c_str(),
	  TimeStr,_name.c_str());
  _sfp = fopen(FileName,"wt");
  if (_sfp == NULL){
    fprintf(stderr,"Failed to create stats file %s\n",FileName);
    exit(-1);
  }

  sprintf(FileName,"%s/Valid_%s_%s.final", _outDir.c_str(),
	  TimeStr,_name.c_str());
  _fsfp = fopen(FileName,"wt");
  if (_fsfp == NULL){
    fprintf(stderr,"Failed to create final stats file %s\n",FileName);
    exit(-1);
  }

  _lastOutputTime = _startTime;

  return;
}

////////////////////////////////////////////////
void statistician::accumulate(unsigned long numNon, 
			      unsigned long numFail, 
			      unsigned long numFalse, 
			      unsigned long numHit,
			      time_t _truthTime,
			      int leadTime,
			      char *forecastDataSetSource){




  _totalNumNon += numNon;
  _totalNumFail += numFail; 
  _totalNumFalse += numFalse;
  _totalNumHit += numHit;

  _subtotalNumNon += numNon;
  _subtotalNumFail += numFail; 
  _subtotalNumFalse += numFalse;
  _subtotalNumHit += numHit;


  date_time_t T;
  T.unix_time = _truthTime;
  uconvert_from_utime(&T);


  fprintf(_sfp,"%d\t%02d\t%02d\t",T.year,T.month,T.day);
  fprintf(_sfp,"%02d\t%02d\t%02d\t%d\t",
	  T.hour,T.min,T.sec,
	  leadTime );


  fprintf(_sfp,"1\t"); // For backward compatibility

  fprintf(_sfp,"%g\t%g\t%g\t%g\t",
	  _subtotalNumNon,_subtotalNumFail,
	  _subtotalNumFalse,_subtotalNumHit);

  double pod,far,csi,hssn,hssd,hss;
  double pod_no,bias;
  double num_fcasts, num_truths;

  num_truths = _subtotalNumHit + _subtotalNumFail;
  num_fcasts = _subtotalNumHit + _subtotalNumFalse;
  bias = num_fcasts / num_truths;
  
  pod_no = double(_subtotalNumNon) / 
    double(_subtotalNumNon + _subtotalNumFalse);
    
  pod = _subtotalNumHit / (_subtotalNumHit + _subtotalNumFail);
  far = _subtotalNumFalse / (_subtotalNumFalse + _subtotalNumHit);
  csi = _subtotalNumHit / (_subtotalNumFalse + 
				    _subtotalNumFail + _subtotalNumHit);

  hssn=2.0*(_subtotalNumHit*_subtotalNumNon-_subtotalNumFail*_subtotalNumFalse);
  
  hssd= _subtotalNumFail * _subtotalNumFail;
  hssd= hssd + _subtotalNumFalse * _subtotalNumFalse;
  hssd= hssd + 2.0*_subtotalNumHit*_subtotalNumNon;
  hssd= hssd + 
    (_subtotalNumFail + _subtotalNumFalse)*(_subtotalNumHit + _subtotalNumNon);
  
  hss = hssn / hssd;
  

  fprintf(_sfp,"%g\t%g\t%g\t%g\t",
	  _noNan(pod),_noNan(far),_noNan(csi),_noNan(hss));
  
  fprintf(_sfp,"%g\t%g\t",_noNan(pod_no),_noNan(bias));
  

  int sourceIndicator = -1;

  if (0==strcmp(forecastDataSetSource, "EXTRAP"))  sourceIndicator = 0;
  if (0==strcmp(forecastDataSetSource, "MODEL"))  sourceIndicator = 1;
  if (0==strcmp(forecastDataSetSource, "BLENDED"))  sourceIndicator = 2;


  _sourceIndicator = sourceIndicator;

  fprintf(_sfp, "%d\n", sourceIndicator);

  fflush(_sfp);

  _lastOutputTime = _truthTime;
  _lastTruthTime = _truthTime;

  _subtotalNumNon = 0.0;
  _subtotalNumFail = 0.0; 
  _subtotalNumFalse = 0.0;
  _subtotalNumHit = 0.0;
  _subnum=0;


  _subnum++; _num++;


  return;

}

////////////////////////////////////////////////

statistician::~statistician(){


  date_time_t T;
  T.unix_time = _lastOutputTime + _outputInterval;
  uconvert_from_utime(&T);

  if (_subnum > 0){ // Only print out if we got something.
    fprintf(_sfp,"%d\t%02d\t%02d\t",T.year,T.month,T.day);
    fprintf(_sfp,"%02d\t%02d\t%02d\t%ld\t",
	    T.hour,T.min,T.sec,
	    _lastOutputTime + _outputInterval - _startTime );

    fprintf(_sfp,"%ld\t",_subnum);

    fprintf(_sfp,"%g\t%g\t%g\t%g\t",
	    _subtotalNumNon,_subtotalNumFail,
	    _subtotalNumFalse,_subtotalNumHit);

    double pod,far,csi,hssn,hssd,hss;
    double pod_no,bias;
    double num_fcasts, num_truths;

    num_truths = _subtotalNumHit + _subtotalNumFail;
    num_fcasts = _subtotalNumHit + _subtotalNumFalse;
    bias = num_fcasts / num_truths;

    pod_no = double(_subtotalNumNon) / 
      double(_subtotalNumNon + _subtotalNumFalse);
    
    pod = _subtotalNumHit / (_subtotalNumHit + _subtotalNumFail);
    far = _subtotalNumFalse / (_subtotalNumFalse + _subtotalNumHit);
    csi = _subtotalNumHit / (_subtotalNumFalse + 
				  _subtotalNumFail + _subtotalNumHit);

    hssn=2.0*(_subtotalNumHit*_subtotalNumNon-_subtotalNumFail*_subtotalNumFalse);

    hssd= _subtotalNumFail * _subtotalNumFail;
    hssd= hssd + _subtotalNumFalse * _subtotalNumFalse;
    hssd= hssd + 2.0*_subtotalNumHit*_subtotalNumNon;
    hssd= hssd + 
      (_subtotalNumFail + _subtotalNumFalse)*(_subtotalNumHit + _subtotalNumNon);

    hss = hssn / hssd;


    fprintf(_sfp,"%g\t%g\t%g\t%g\t",
	    _noNan(pod),_noNan(far),_noNan(csi),_noNan(hss));

    fprintf(_sfp,"%g\t%g\t",_noNan(pod_no),_noNan(bias));

    fprintf(_sfp, "%d\n", _sourceIndicator);

    fflush(_sfp);
  } // End of only print out if we got something.


  /////////
  fclose(_sfp);

  fprintf(_fsfp,"\nFinal statistics for %s :\n\n",_name.c_str());

  fprintf(_fsfp,"%s Total non-events : %g\n",_name.c_str(),_totalNumNon);
  fprintf(_fsfp,"%s Total failures : %g\n",_name.c_str(),_totalNumFail);
  fprintf(_fsfp,"%s Total false alarms : %g\n",_name.c_str(),_totalNumFalse);
  fprintf(_fsfp,"%s Total successes : %g\n\n",_name.c_str(),_totalNumHit);
  fprintf(_fsfp,"%s Total number of forecast files : %ld\n",_name.c_str(),_num);

  double pod,far,csi,hssn,hssd,hss;

  pod = _totalNumHit / (_totalNumHit + _totalNumFail);
  far = _totalNumFalse / (_totalNumFalse + _totalNumHit);
  csi = _totalNumHit / (_totalNumFalse + 
			     _totalNumFail + _totalNumHit);

  hssn=2.0*(_totalNumHit*_totalNumNon - _totalNumFail*_totalNumFalse);

  hssd= _totalNumFail * _totalNumFail;
  hssd= hssd + _totalNumFalse * _totalNumFalse;
  hssd= hssd + 2.0*_totalNumHit*_totalNumNon;
  hssd= hssd + 
    (_totalNumFail + _totalNumFalse)*(_totalNumHit + _totalNumNon);

  hss = hssn / hssd;

  double pod_no,bias;
  double num_fcasts, num_truths;

  num_truths = _totalNumHit + _totalNumFail;
  num_fcasts = _totalNumHit + _totalNumFalse;
  bias = num_fcasts / num_truths;

  pod_no = double(_totalNumNon) / 
    double(_totalNumNon + _totalNumFalse);

  fprintf(_fsfp,"%s POD : %g\n",_name.c_str(),_noNan(pod));
  fprintf(_fsfp,"%s FAR : %g\n",_name.c_str(),_noNan(far));
  fprintf(_fsfp,"%s CSI : %g\n",_name.c_str(),_noNan(csi));
  fprintf(_fsfp,"%s HSS : %g\n",_name.c_str(),_noNan(hss));
  fprintf(_fsfp,"%s POD_NO : %g\n",_name.c_str(),_noNan(pod_no));
  fprintf(_fsfp,"%s BIAS : %g\n",_name.c_str(),_noNan(bias));
  fprintf(_fsfp,"%s TRUTHS : %g\n",_name.c_str(),_noNan(num_truths));
  fprintf(_fsfp,"%s FORECASTS : %g\n",_name.c_str(),_noNan(num_fcasts));

  fclose(_fsfp);

}

/////////////////////////////////////////////////

double statistician::_noNan(double val){

  if (std::isnan(val)){
    return -1000.0;
  }
  return val;
}
































