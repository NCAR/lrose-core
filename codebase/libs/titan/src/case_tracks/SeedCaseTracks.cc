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
////////////////////////////////////////////////////////////////
// SeedCaseTracks.cc
//
// Handling seed/no-seed cases
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2008
//
////////////////////////////////////////////////////////////////

#include <titan/SeedCaseTracks.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <cstdio>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

SeedCaseTracks::SeedCaseTracks()
{
  
}

////////////////////////////////////////////////////////////
// destructor

SeedCaseTracks::~SeedCaseTracks()
  
{
  clear();
}

////////////////////////////////////////////////////////////
// clear the object

void SeedCaseTracks::clear()

{
  _cases.clear();
}

////////////////////////////////////////////////////////////
// read in a case track file
//
// Returns 0 on success, -1 on failure
  
int SeedCaseTracks::readCaseFile(const char *caseFilePath)

{

  // open case file
  
  FILE *case_file;
  if ((case_file = fopen(caseFilePath, "r")) == NULL) {
    fprintf(stderr, "ERROR - SeedCaseTracks::readCaseFile\n");
    fprintf(stderr, "Cannot open case file\n");
    perror(caseFilePath);
    return(-1);
  }
  
  _filePath = caseFilePath;
  if (_debug) {
    cerr << "Reading case file: " << _filePath << endl;
  }
  
  // initialize
  
  clear();

  // read in cases
  
  char line[BUFSIZ];
  while (fgets(line, BUFSIZ, case_file) != NULL) {
    
    if (line[0] == '#') {
      continue;
    }
    
    int num;
    int num_flares;
    int seed_duration;
    int complex_track_num;
    int simple_track_num;
    int decision_minus_start;
    int end_minus_decision;
    
    double cloud_base;
    double mixing_ratio;
    double temp_ccl;
    double deltat_500mb;

    char seed_flag[8];

    int dyear, dmonth, dday, dhour, dmin, dsec;
    int ryear, rmonth, rday, rhour, rmin, rsec;

    if (sscanf(line,
	       "%d "
	       "%s %d %d"
	       "%d %d %d %d %d %d "
	       "%d %d %d %d %d %d "
	       "%d %d "
	       "%d %d "
	       "%lg %lg "
	       "%lg %lg",
	       &num,
	       seed_flag, &num_flares, &seed_duration,
	       &dyear, &dmonth, &dday, &dhour, &dmin, &dsec,
	       &ryear, &rmonth, &rday, &rhour, &rmin, &rsec,
	       &decision_minus_start, &end_minus_decision,
	       &complex_track_num, &simple_track_num,
	       &cloud_base, &mixing_ratio,
	       &temp_ccl, &deltat_500mb) != 24) {

      // try old format, which did not specify decision time

      if (sscanf(line,
                 "%d "
                 "%s %d %d"
                 "%d %d %d %d %d %d "
                 "%d %d "
                 "%d %d "
                 "%lg %lg "
                 "%lg %lg",
                 &num,
                 seed_flag, &num_flares, &seed_duration,
                 &ryear, &rmonth, &rday, &rhour, &rmin, &rsec,
                 &decision_minus_start, &end_minus_decision,
                 &complex_track_num, &simple_track_num,
                 &cloud_base, &mixing_ratio,
                 &temp_ccl, &deltat_500mb) == 18) {
        dyear = ryear;
        dmonth = rmonth;
        dday = rday;
        dhour = rhour;
        dmin = rmin;
        dsec = rsec;
      } else {
        continue;
      }

    }

    if (complex_track_num < 0 || simple_track_num < 0) {
      continue;
    }
    
    DateTime decisionTime(dyear, dmonth, dday, dhour, dmin, dsec);
    DateTime refTime(ryear, rmonth, rday, rhour, rmin, rsec);

    // set up case
    
    CaseTrack thisCase;
    thisCase.num = num;

    if (seed_flag[0] == 'Y' || seed_flag[0] == 'y') {
      thisCase.seed_flag = TRUE;
    } else {
      thisCase.seed_flag = FALSE;
    }

    thisCase.num_flares = num_flares;
    thisCase.seed_duration = seed_duration * 60;
    
    thisCase.decision_time = decisionTime.utime();
    thisCase.decision_minus_start = decision_minus_start * 60;
    thisCase.end_minus_decision = end_minus_decision * 60;

    thisCase.start_time = thisCase.decision_time - thisCase.decision_minus_start;
    thisCase.end_time = thisCase.decision_time + thisCase.end_minus_decision;
    
    thisCase.ref_time = refTime.utime();
    thisCase.ref_minus_start = thisCase.ref_time - thisCase.start_time;
    thisCase.end_minus_ref = thisCase.end_time - thisCase.ref_time;

    thisCase.complex_track_num = complex_track_num;
    thisCase.simple_track_num = simple_track_num;

    thisCase.cloud_base = cloud_base;
    thisCase.mixing_ratio = mixing_ratio;
    thisCase.temp_ccl = temp_ccl;
    thisCase.deltat_500mb = deltat_500mb;

    if (_debug) {
      SeedCaseTracks::printCase(cerr, thisCase);
    }

    _cases.push_back(thisCase);
    
  } /* while */

  fclose(case_file);

  if (_cases.size() > 0) {
    return 0;
  } else {
    return -1;
  }

}

///////////////////////////////////////
// Find track case
// Look through available case tracks for a match.
// Loads thisCase arg.
// Returns 0 on success, -1 on failure

int SeedCaseTracks::findCase(time_t scan_time,
                             int complex_track_num,
                             int simple_track_num,
                             CaseTrack &thisCase) const

{

  for (int ii = 0; ii < (int) _cases.size(); ii++) {

    const CaseTrack &ct = _cases[ii];

    if (ct.start_time <= scan_time && ct.end_time >= scan_time) {
      
      // time is good
      
      if (ct.complex_track_num == complex_track_num &&
	  ct.simple_track_num == simple_track_num) {

	// track numbers are good, accept
	
	thisCase = ct;
	return 0;
	
      }
      
    }
    
  } // ii

  return -1;

}

///////////////////////////////////////
// Get a case given an index
// Returns 0 on success, -1 on failure

int SeedCaseTracks::getCase(int index, CaseTrack &caseTrack) const

{

  if (index < (int) _cases.size()) {
    
    caseTrack = _cases[index];
    return 0;

  } else {

    cerr << "ERROR - SeedCaseTracks::getCase" << endl;
    cerr << "  Case index invalid: " << index << endl;
    cerr << "  Max case index: " << _cases.size() - 1 << endl;
    return -1;
    
  }

}

///////////////////////////////////////////
// get next case, given a case
// next case must have a higher case number
// Returns 0 on success, -1 on failure

int SeedCaseTracks::getNextCase(const CaseTrack &thisCase,
                                CaseTrack &nextCase) const

{

  bool nextCaseFound = false;
  int minDiff = 1000000000;
  for (int ii = 0; ii < (int) _cases.size(); ii++) {
    int diff = _cases[ii].num - thisCase.num;
    if (diff > 0 && diff < minDiff) {
      nextCase = _cases[ii];
      minDiff = diff;
      nextCaseFound = true;
    }
  }

  if (nextCaseFound) {
    return 0;
  } else {
    return -1;
  }
    
}

///////////////////////////////////////////
// get next case, given a time
// next case must have a later time
// Returns 0 on success, -1 on failure

int SeedCaseTracks::getNextCase(double thisTime,
                                CaseTrack &nextCase) const

{

  bool nextCaseFound = false;
  double minDiff = 1.0e99;
  for (int ii = 0; ii < (int) _cases.size(); ii++) {
    double diff = _cases[ii].ref_time - thisTime;
    if (diff > 0 && diff < minDiff) {
      nextCase = _cases[ii];
      minDiff = diff;
      nextCaseFound = true;
    }
  }

  if (nextCaseFound) {
    return 0;
  } else {
    return -1;
  }
    
}

///////////////////////////////////////
// get prev case, given a case
// prev case must have a lower case number
// Returns 0 on success, -1 on failure

int SeedCaseTracks::getPrevCase(const CaseTrack &thisCase,
                                CaseTrack &prevCase) const

{

  bool prevCaseFound = false;
  int minDiff = 1000000000;
  for (int ii = 0; ii < (int) _cases.size(); ii++) {
    int diff = thisCase.num - _cases[ii].num;
    if (diff > 0 && diff < minDiff) {
      prevCase = _cases[ii];
      minDiff = diff;
      prevCaseFound = true;
    }
  }

  if (prevCaseFound) {
    return 0;
  } else {
    return -1;
  }
    
}

///////////////////////////////////////////
// get prev case, given a time
// prev case must have an earlier time
// Returns 0 on success, -1 on failure

int SeedCaseTracks::getPrevCase(double thisTime,
                                CaseTrack &prevCase) const

{

  bool prevCaseFound = false;
  double minDiff = 1.0e99;
  for (int ii = 0; ii < (int) _cases.size(); ii++) {
    double diff = thisTime - _cases[ii].ref_time;
    if (diff > 0 && diff < minDiff) {
      prevCase = _cases[ii];
      minDiff = diff;
      prevCaseFound = true;
    }
  }

  if (prevCaseFound) {
    return 0;
  } else {
    return -1;
  }
    
}

///////////////////////////////////////
// Print a case

void SeedCaseTracks::print(ostream &out) const 

{

  out << "==========================================================" << endl;
  out << "CASE TRACKS:" << endl;
  out << "File: " << _filePath << endl;
  out << "Number of cases: " << _cases.size() << endl;
  for (int ii = 0; ii < (int) _cases.size(); ii++) {
    printCase(out, _cases[ii]);
  }
  out << "==========================================================" << endl;
  

}

///////////////////////////////////////
// Print a case

void SeedCaseTracks::printCase(ostream &out, int index) const

{

  if (index < (int) _cases.size()) {

    printCase(out, _cases[index]);

  } else {

    cerr << "ERROR - SeedCaseTracks::printCase" << endl;
    cerr << "  Case index invalid: " << index << endl;
    cerr << "  Max case index: " << _cases.size() - 1 << endl;

  }

}

///////////////////////////////////////
// Print a case

void SeedCaseTracks::printCase(ostream &out, const CaseTrack &thisCase)

{

  out << "  ---------------------------------------------------------" << endl;
  out << "  CASE TRACK:" << endl;
  
  out << "    num: " << thisCase.num << endl;
  out << "    seed_flag: " << (thisCase.seed_flag? "Y":"N") << endl;
  out << "    num_flares: " << thisCase.num_flares << endl;
  out << "    seed_duration: " << thisCase.seed_duration << endl;
  
  out << "    decision_time: " << DateTime::strm(thisCase.decision_time) << endl;
  out << "    decision_minus_start: " << thisCase.decision_minus_start << endl;
  out << "    end_minus_decision: " << thisCase.end_minus_decision << endl;

  out << "    ref_time: " << DateTime::strm(thisCase.ref_time) << endl;
  out << "    ref_minus_start: " << thisCase.ref_minus_start << endl;
  out << "    end_minus_ref: " << thisCase.end_minus_ref << endl;

  out << "    start_time: " << DateTime::strm(thisCase.start_time) << endl;
  out << "    end_time: " << DateTime::strm(thisCase.end_time) << endl;
  
  out << "    complex_track_num: " << thisCase.complex_track_num << endl;
  out << "    simple_track_num: " << thisCase.simple_track_num << endl;
  
  out << "    cloud_base: " << thisCase.cloud_base << endl;
  out << "    mixing_ratio: " << thisCase.mixing_ratio << endl;
  out << "    temp_ccl: " << thisCase.temp_ccl << endl;
  out << "    deltat_500mb: " << thisCase.deltat_500mb << endl;

}

