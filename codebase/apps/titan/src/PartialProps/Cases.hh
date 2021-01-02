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
// Cases.h: Case handling
//
// Reads in the cases, supplies them to the program one at
// a time.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef Cases_h
#define Cases_h

#include <toolsa/umisc.h>
#include <titan/storm.h>
#include <titan/track.h>
#include <titan/partial_track.h>
#include <titan/SeedCaseTracks.hh>

class Cases {
  
public:

  // constructor

  Cases (const char *prog_name,
	 bool debug,
         bool verbose,
	 const char *case_file_path,
	 double altitude_threshold,
	 const char *output_dir);
  
  // Destructor

  virtual ~Cases();

  // reset the case num to 0

  void reset();

  // get next file

  int next(SeedCaseTracks::CaseTrack *this_case);

  // process the case

  int process(SeedCaseTracks::CaseTrack *this_case,
	      char *storm_file_path);
     
  // flag to indicate construction was successful

  int isOK;

protected:
  
private:

  char *_caseFilePath;
  char *_progName;
  char *_outputDir;

  SeedCaseTracks _cases;
  int _caseIndex;

  bool _debug;
  bool _verbose;
  int _gridType;

  double _altitudeThreshold;

  storm_file_handle_t _s_handle;
  track_file_handle_t _t_handle;
  rf_partial_track_t _pTrack;

  int _init_indices();

  int _open_storm_and_track_files(char *storm_file_path,
				  char *track_file_path);


};

#endif
