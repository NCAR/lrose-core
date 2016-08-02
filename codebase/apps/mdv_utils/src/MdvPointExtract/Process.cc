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
#include <Spdb/SoundingPut.hh>
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
int Process::Derive(Params *P, time_t T, string argUrl){


  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.setDebug( P->Debug);

  if (argUrl == "NONE"){
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  } else {
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, argUrl, 0, T);
  }

  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


 

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    if (argUrl == "NONE"){
      cerr << P->TriggerUrl  << endl;
    } else {
      cerr << argUrl << endl;
    }
    return -1;
  }     

 
  //
  // Get the desired fields.
  //
  MdvxField *UField, *VField, *TField;

  UField = New.getFieldByName( "U" );
  VField = New.getFieldByName( "V" );
  TField = New.getFieldByName( "T" );

  if ((UField == NULL) || (VField == NULL) || (TField == NULL)){
    cerr << "Fields not found." << endl;
    return -1;
  }

  //
  // Get the field and vlevel headers.
  //
  Mdvx::field_header_t UFhdr = UField->getFieldHeader();
  Mdvx::vlevel_header_t UVhdr = UField->getVlevelHeader();
  //
  Mdvx::field_header_t VFhdr = VField->getFieldHeader();
  Mdvx::vlevel_header_t VVhdr = VField->getVlevelHeader();
  //
  Mdvx::field_header_t TFhdr = TField->getFieldHeader();
  Mdvx::vlevel_header_t TVhdr = TField->getVlevelHeader();
  //

  Mdvx::master_header_t InMhdr = New.getMasterHeader();


  MdvxProj Proj(InMhdr, UFhdr);

  fl32 *UData = (fl32 *) UField->getVol();
  fl32 *VData = (fl32 *) VField->getVol();
  fl32 *TData = (fl32 *) TField->getVol();

  for (int ip=0; ip < P->stationDef_n; ip++){

    cerr << "Station " << P->_stationDef[ip].name << endl;
    

    double Lat, Lon;
    Lat = P->_stationDef[ip].lat;
    Lon = P->_stationDef[ip].lon;
    int iz0 =  P->_stationDef[ip].planeNum;

    int ix, iy;

    if (Proj.latlon2xyIndex(Lat, Lon, ix, iy)){
      cerr << "Outside grid." << endl;
      continue;
    }

    int numPoints =  UFhdr.nz - iz0;

    double *U = (double *) malloc(numPoints * sizeof(double));
    double *V = (double *) malloc(numPoints * sizeof(double));
    double *Temp = (double *) malloc(numPoints * sizeof(double));
    double *ht = (double *) malloc(numPoints * sizeof(double));


    if ((U == NULL) || (V == NULL) || (Temp == NULL) || (ht == NULL)){
      cerr << "Malloc" << endl;
      exit(-1);
    }
    //
    // Fill arrays in the vewrtical and print.
    //
    for (int iz = iz0; iz < UFhdr.nz; iz++){
      int index = iz * UFhdr.nx * UFhdr.ny +
	iy * UFhdr.nx + ix;

      cerr << TVhdr.level[iz] << " : \t{";
      cerr << UData[index] << ", ";
      cerr << VData[index] << "} \t";
      cerr << TData[index] - 273.16 << endl;

      ht[iz] = TVhdr.level[iz];
      U[iz] = UData[index];
      V[iz] = VData[index];
      Temp[iz] = TData[index] - 273.16;
    }
    //
    // Write out the lowest point to an SPDB METAR struct.
    //
    station_report_t S;
    memset( &S, 0, sizeof(station_report_t));

    int dataType = Spdb::hash4CharsToInt32(  P->_stationDef[ip].name );

    S.alt = ht[0];
    S.lat = Lat; 
    S.lon = Lon;
    S.time = T;
    S.temp = Temp[0];

    double dir, spd;

    dir = PHYwind_dir( U[0], V[0] );
    spd = PHYwind_speed( U[0], V[0] );

    S.windspd = spd;
    S.winddir = dir;

    S.dew_point = STATION_NAN;
    S.relhum = STATION_NAN;
    S.windgust = STATION_NAN;
    S.pres = STATION_NAN;
    S.liquid_accum = STATION_NAN;
    S.precip_rate = STATION_NAN;
    S.visibility = STATION_NAN;
    S.rvr = STATION_NAN;
    S.ceiling = STATION_NAN;
    S.shared.station.Spare1 = STATION_NAN;
    S.shared.station.Spare2 = STATION_NAN;
    S.shared.station.liquid_accum2 = STATION_NAN;

    sprintf(S.station_label, "%s", P->_stationDef[ip].name );

    station_report_to_be( &S );

    DsSpdb Station;

    Station.put(P->OutSurfaceUrl,
		SPDB_STATION_REPORT_ID,
		SPDB_STATION_REPORT_LABEL,
		dataType,
		T,
		T + P->Expiry,
		sizeof(station_report_t),
		&S);

    //
    // Similarly for the sounding data.
    //
    SoundingPut SP;
    
    vector< string* > urlVec;
    string Url( P->OutSoundingUrl );
    urlVec.push_back( &Url );

    SP.init(urlVec, 
	   Sounding::DEFAULT_ID, 
	   "MDV data", 
	   dataType,
	   P->_stationDef[ip].name,
	   Lat, Lon, 
	   ht[0], -9999.0 );
    
    SP.set(T,
	  numPoints,
	  ht,
	  U,
	  V,
	  NULL,
	  NULL,
	  NULL,
	  Temp);
    
    
    SP.writeSounding( T, T + P->Expiry );
    
    free(U); free(V); free(Temp); free(ht);

  }
	
  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){

}










