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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
#include <Spdb/DsSpdb.hh>
#include <rapformats/station_reports.h>
#include <physics/physics.h>
using namespace std;

//
// Constructor
//
Process::Process(){
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){


  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  New.addReadField("Speed10");
  New.addReadField("Wdir10");
  New.addReadField("RH2");
  New.addReadField("T2");
  New.addReadField("pressure2");
  New.addReadField("terrain");



  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  MdvxField *spdField =  New.getFieldByName( "Speed10" );
  MdvxField *dirField =  New.getFieldByName( "Wdir10" );
  MdvxField *rhField =  New.getFieldByName( "RH2" );
  MdvxField *tField =  New.getFieldByName( "T2" );
  MdvxField *pField =  New.getFieldByName( "pressure2" );
  MdvxField *hField =  New.getFieldByName( "terrain" );

  Mdvx::master_header_t InMhdr = New.getMasterHeader();

  fl32 *spdData = (fl32 *) spdField->getVol();
  fl32 *dirData = (fl32 *) dirField->getVol();
  fl32 *rhData = (fl32 *) rhField->getVol();
  fl32 *tData = (fl32 *) tField->getVol();
  fl32 *pData = (fl32 *) pField->getVol();
  fl32 *hData = (fl32 *) hField->getVol();

  Mdvx::field_header_t spdFhdr = spdField->getFieldHeader();
  Mdvx::field_header_t dirFhdr = dirField->getFieldHeader();
  Mdvx::field_header_t rhFhdr = rhField->getFieldHeader();
  Mdvx::field_header_t tFhdr = tField->getFieldHeader();
  Mdvx::field_header_t pFhdr = pField->getFieldHeader();
  Mdvx::field_header_t hFhdr = hField->getFieldHeader();

  MdvxProj Proj(InMhdr, spdFhdr );

  DsSpdb spdb;

  for (int is=0; is < P->stations_n; is++){

    int ix, iy;
    if (Proj.latlon2xyIndex(P->_stations[is].lat,
			    P->_stations[is].lon, 
			    ix, iy)){
      cerr << "Station " << P->_stations[is].name4char << " is out of range." << endl;
    } else {

      int index = iy * hFhdr.nx + ix;
      
      station_report_t R;
      
      memset(&R, 0, sizeof(R));
      //
      // Set fields to bad.
      //
      
      R.msg_id = STATION_REPORT;
      R.temp = tData[index] - 273.15;
      R.dew_point = STATION_NAN;
      R.relhum = rhData[index];
      R.windspd = spdData[index];
      R.winddir = dirData[index];
      R.windgust = STATION_NAN;
      R.pres = pData[index];
      R.liquid_accum = STATION_NAN;
      R.precip_rate = STATION_NAN;
      R.visibility = STATION_NAN;
      R.rvr = STATION_NAN;
      R.ceiling = STATION_NAN;
      R.shared.station.Spare1 = STATION_NAN;
      R.shared.station.Spare2 = STATION_NAN;
      R.shared.station.liquid_accum2 = STATION_NAN;
      
      R.dew_point  = PHYrhdp(R.temp, R.relhum); 
      
      R.time = InMhdr.time_centroid;
      
      R.lat = P->_stations[is].lat;
      R.lon = P->_stations[is].lon;
      R.alt = hData[index];
      
      station_report_to_be( &R );
      
      int stationID = Spdb::hash4CharsToInt32(P->_stations[is].name4char);
      
      spdb.addPutChunk( stationID,
			InMhdr.time_centroid,
			InMhdr.time_centroid,
			sizeof(station_report_t), &R, 0);
      
    }
    
  }

  if (spdb.put( P->OutUrl,
		SPDB_STATION_REPORT_ID,
		SPDB_STATION_REPORT_LABEL)){
    fprintf(stderr,"Failed to put data to %s\n",
            P->OutUrl );
    return -1;
  }



  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










