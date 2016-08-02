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
 * get_titan_data.c
 *
 * get track data from the DsTitanServer
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * Feb 2001
 *
 *************************************************************************/

#include "Rview.hh"
using namespace std;

void get_titan_data()

{

  // set up the titan server object

  Glob->_dsTitan.clearRead();

  if (Glob->mode == REALTIME) {
    Glob->_dsTitan.setReadLatest();
  } else {
    Glob->_dsTitan.setReadFirstBefore
      (Glob->time, Glob->track_shmem->track_data_time_margin);
  }

  if (Glob->track_shmem->track_set == TDATA_ALL_IN_FILE) {
    Glob->_dsTitan.setReadAllInFile();
  } else {
    Glob->_dsTitan.setReadAllAtTime();
  }
  
  if (Glob->runs_included) {
    Glob->_dsTitan.setReadRuns();
  }
  
  if (Glob->debug) {
    Glob->_dsTitan.printReadRequest(cerr);
  }

  // do the read

  if (Glob->_dsTitan.read(Glob->titan_server_url)) {
    if (Glob->debug) {
      cerr << "ERROR - Rview::get_titan_data" << endl;
      cerr << Glob->_dsTitan.getErrStr() << endl;
    }
    Glob->track_data_available = FALSE;
    return;
  }
    
  Glob->time = Glob->_dsTitan.getTimeInUse();
  Glob->track_data_available = TRUE;
  
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
	return;
	
      } // ientry
      
    } // isimple
    
  } // icomplex

}
