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
/************************************************************************
 * get_titan_data.cc
 *
 * get track data from the DsTitanServer
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Feb 2001
 *
 *************************************************************************/

#include "TimeHist.hh"
using namespace std;

void get_titan_data(void)

{

  if (Glob->verbose) {
    fprintf(stderr, "** TimeHist - get_titan_data **\n");
  }

  // set up the titan server object

  Glob->_dsTitan.clearRead();

  // set delta time to be used if TITAN data read fails
  // This is derived from the shared memory segment

  int delta_time = 0;

  if (Glob->track_shmem->mode == TDATA_REALTIME) {

    Glob->_dsTitan.setReadLatest();

  } else {

    if (Glob->scan_delta > 0) {
      
      Glob->_dsTitan.setReadNext
	(Glob->track_shmem->time,
	 Glob->track_shmem->track_data_time_margin);

      delta_time = Glob->track_shmem->scan_delta_t;
      
    } else if (Glob->scan_delta < 0) {
      
      Glob->_dsTitan.setReadPrev
	(Glob->track_shmem->time,
	 Glob->track_shmem->track_data_time_margin);
      
      delta_time = -Glob->track_shmem->scan_delta_t;

    } else {
      
      Glob->_dsTitan.setReadFirstBefore
	(Glob->track_shmem->time,
	 Glob->track_shmem->track_data_time_margin);
      
      delta_time = 0;

    }

  }

  Glob->scan_delta = 0;
  
  if (Glob->track_shmem->track_set == TDATA_ALL_IN_FILE) {
    Glob->_dsTitan.setReadAllInFile();
  } else {
    Glob->_dsTitan.setReadAllAtTime();
  }
  
  Glob->_dsTitan.setReadLprops();
  Glob->_dsTitan.setReadDbzHist();
  
  if (Glob->debug) {
    Glob->_dsTitan.printReadRequest(cerr);
  }

  // do the read

  if (Glob->_dsTitan.read(Glob->track_shmem->titan_server_url)) {
    Glob->_dsTitan.setReadAllInFile();
    if (Glob->_dsTitan.read(Glob->track_shmem->titan_server_url)) {
      if (Glob->debug) {
	cerr << "ERROR - TimeHist::get_track_data" << endl;
	cerr << Glob->_dsTitan.getErrStr() << endl;
      }
      Glob->track_shmem->time += delta_time;
      Glob->data_start_time = (Glob->track_shmem->time / SECS_IN_DAY) * SECS_IN_DAY;
      Glob->data_end_time = Glob->data_start_time + SECS_IN_DAY;
      Glob->track_shmem->complex_track_num = -1;
      Glob->track_data_avail = FALSE;
      return;
    }
  }
  
  Glob->track_data_avail = TRUE;
  Glob->track_shmem->time = Glob->_dsTitan.getTimeInUse();
  Glob->data_start_time = Glob->_dsTitan.getDataStartTime();
  Glob->data_end_time = Glob->_dsTitan.getDataEndTime();

  // check that we actually have some data

  for (size_t icomplex = 0;
       icomplex < Glob->_dsTitan.complex_tracks().size(); icomplex++) {
    
    const TitanComplexTrack *ctrack =
      Glob->_dsTitan.complex_tracks()[icomplex];

    for (size_t isimple = 0;
	 isimple < ctrack->simple_tracks().size(); isimple++) {

      const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
      
      for (size_t ientry = 0; ientry < strack->entries().size(); ientry++) {

	const TitanTrackEntry *tentry = strack->entries()[ientry];
	Glob->titan_grid = tentry->scan().grid;
	compute_track_num();

	return;
	
      } // ientry
      
    } // isimple
    
  } // icomplex

  // fell through - no actual data

  Glob->track_shmem->complex_track_num = -1;
  return;

}
      
