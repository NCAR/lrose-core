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
////////////////////////////////////////////////////////////////////
// <titan/SeedCaseTracks.hh>
//
// Handling seed/no-seed cases
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// June 2008
//
////////////////////////////////////////////////////////////////////

#ifndef SeedCaseTracks_HH
#define SeedCaseTracks_HH

#include <ctime>
#include <vector>
#include <string>
#include <iostream>
using namespace std;

class SeedCaseTracks
{

public:

  class CaseTrack {

  public:
    
    int num;                /* case number */
    
    // seeding details
    
    bool seed_flag;         /* true/false */
    int num_flares;         /* number of flares used */
    int seed_duration;      /* secs */
    
    // decision time
    
    time_t decision_time;   /* when the seed/no-seed decision was made */

    int decision_minus_start; /* (secs) time before decision time at which
                               * the case starts */
    
    int end_minus_decision; /* (secs) time after decision time at which
                             * the case ends */
    
    // titan reference time
    // this is the time at which the complex and simple track
    // number are specified
    
    time_t ref_time;        /* reference time (as close as possible
                             * to decision time) */
    
    int ref_minus_start;    /* (secs) time before ref time at which
                             * the case starts */
    
    int end_minus_ref;      /* (secs) time after ref time at which
                             * the case ends */

    // start and end time of case track
    
    time_t start_time;      /* decision time - decision_minus_start =
                             * ref time - ref_minus_start */
    
    time_t end_time;        /* decision_time + end_time_minus_ref =
                             * ref_time + end_time_minus_ref */
    
    // track numbers
    
    int complex_track_num;
    int simple_track_num;
    
    // environmental data
    
    double cloud_base;      /* km */
    double mixing_ratio;    /* g/kg */
    double temp_ccl;        /* C */
    double deltat_500mb;    /* C */
    
  };

  // constructor
  
  SeedCaseTracks();
  
  // destructor
  
  virtual ~SeedCaseTracks();

  // set debug mode

  void setDebug(bool debug) { _debug = debug; }

  // clear the object
  
  void clear();
  
  // read in a case track file
  // returns 0 on success, -1 on failure

  int readCaseFile(const char *caseFilePath);

  // Find track case
  // Look through available case tracks for a match.
  // Loads thisCase arg.
  // Returns 0 on success, -1 on failure
  
  int findCase(time_t scan_time,
               int complex_track_num,
               int simple_track_num,
               CaseTrack &thisCase) const;

  // get methods

  int getNCases() const { return (int) _cases.size(); }
  const vector<CaseTrack> &getCases() const { return _cases; }

  // get case based on index
  // returns 0 on success, -1 on failure
  
  int getCase(int index, CaseTrack &caseTrack) const;

  // get next case, given a case
  // next case must have a higher case number
  // Returns 0 on success, -1 on failure
  
  int getNextCase(const CaseTrack &thisCase, CaseTrack &nextCase) const;

  // get next case, given a time
  // next case must have a later time
  // Returns 0 on success, -1 on failure
  
  int getNextCase(double thisTime, CaseTrack &nextCase) const;

  // get previous case, given a case
  // returns 0 on success, -1 on failure
  
  int getPrevCase(const CaseTrack &thisCase, CaseTrack &prevCase) const;

  // get prev case, given a time
  // prev case must have an earlier time
  // Returns 0 on success, -1 on failure
  
  int getPrevCase(double thisTime, CaseTrack &prevCase) const;

  // printing

  void print(ostream &out) const;
  void printCase(ostream &out, int index) const;
  static void printCase(ostream &out, const CaseTrack &caseTrack);

protected:

  bool _debug;
  string _filePath;
  vector<CaseTrack> _cases;

};

#endif
















