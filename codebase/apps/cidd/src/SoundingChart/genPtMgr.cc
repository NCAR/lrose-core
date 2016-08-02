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
//
// Object that creates and stores GenPt objects.
//
using namespace std;


#include <Spdb/Spdb.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pjg_flat.h>
#include <toolsa/umisc.h>

#include "genPtMgr.hh"

//////////////////////////////////////////////////////////////////////////
// Constructor. Reads the data and fills up the vector.
// Determines which sounding to read.
//
genPtMgr::genPtMgr( string url, int debug){

  Url = url;
  return;
}

//////////////////////////////////////////////////////////////////////////
// Destructor
//
genPtMgr::~genPtMgr(){
  return;
}

//////////////////////////////////////////////////////////////////////////
// CLEAR_DATABASE
//
void genPtMgr::clear_database(void){

  DsSpdb spdb;

  spdb.compileTimeList(Url,0,time(0) + 900000,0);

  cerr << "Erasing " << spdb.getNTimesInList() << " Points" << endl;
  for(int i = 0; i < spdb.getNTimesInList(); i++) {
	  spdb.erase(Url,spdb.getTimeFromList(i));
  }

  return;
}

//////////////////////////////////////////////////////////////////////////
// OUTPUT_POINT
//
void genPtMgr::output_point(double lat, double lon, time_t t,char *name, char * units, double value) {
  GenPt gpt;

  gpt.clear();

  // Add A time stamp as a text label
  // time_t now = time(0);
  // char txt_buf[16];
  // strftime(txt_buf,16,"%H:%M:%S",localtime(&now));
  // gpt.setText(txt_buf); // Label the click time 

  gpt.setName("SoundingChart");
  gpt.setNLevels(1);
  gpt.setLat(lat);
  gpt.setLon(lon);
  gpt.setTime(t);
  gpt.setId(0);

  gpt.addVal(value);
  gpt.addFieldInfo(name,units);

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeAdd); 
  spdb.clearUrls();
  spdb.addUrl(Url.c_str());

  gpt.assemble();  // Load up a byte swapped buffer

  if(spdb.put(SPDB_GENERIC_POINT_ID,
			  SPDB_GENERIC_POINT_LABEL,
			  0,
			  t,
			  t + 3600,
			  gpt.getBufLen(),
			  (void *) gpt.getBufPtr(),
			  0)) {

      cerr << "Problems putting Point to " << Url << endl;
  }

  return;
}
