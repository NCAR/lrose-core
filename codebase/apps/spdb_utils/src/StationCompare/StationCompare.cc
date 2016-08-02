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
// StationCompare.cc
//
// StationCompare object
//
// The One, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////
//
// StationCompare extracts data from gridded MDV fields
// at the times and locations of storm initiation. The time/space
// data are saved as an SPDB database of GenPts by the StormInitLocation
// program.
//
///////////////////////////////////////////////////////////////

#include "StationCompare.hh"

#include <toolsa/file_io.h>
#include <string>
#include <toolsa/umisc.h>
#include <titan/DsTitan.hh>
#include <cerrno>

#include <physics/physics.h>

#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>

#include <cstdio>
using namespace std;

// Constructor

StationCompare::StationCompare(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  
  return;

}

// destructor. That stuff which dreams are made of.

StationCompare::~StationCompare(){

}

//////////////////////////////////////////////////
// Run

int StationCompare::Run(time_t start, time_t end)
{

  if (_params->debug){
    cerr << "Processing data from " << utimstr(start);
    cerr << " to " << utimstr(end) << endl;
  }


  //
  // Open the output file.
  //
  char outFileName[MAX_PATH_LEN];
  date_time_t D;
  D.unix_time = start; uconvert_from_utime( &D );
  sprintf(outFileName,"%s/%d%02d%02d_%02d%02d%02d.dat",
	  _params->outDir,
	  D.year, D.month, D.day, D.hour, D.min, D.sec);
  
  FILE *ofp = fopen(outFileName,"wt");
  if (ofp == NULL){
    cerr << "Failed to create file " << outFileName;
    exit(-1);
  }

  for (int istn=0; istn < _params->stationIDs_n; istn++){

    if (_params->debug){
      cerr << "Looking at station " << _params->_stationIDs[istn] << endl;
    }

    int dataType = 0;
    //
    // If the station ID is not NONE then use it as the data type.
    //
    if (strcmp("NONE", _params->_stationIDs[istn])){
      dataType = Spdb::hash4CharsToInt32( _params->_stationIDs[istn] );
    }

    //
    // Read the SPDB database of stations.
    //
    DsSpdb stationReports;
    if (stationReports.getInterval(_params->spdbUrl, start, end, dataType)){
      cerr << "getInterval failed." << endl; // Unlikely.
      exit(-1);
    }
    if (_params->debug){
      cerr << stationReports.getNChunks() << " points found." << endl;
    }
    

    //
    // Loop through the station reports.
    //
    for (int point=0; point < stationReports.getNChunks(); point++){

      //
      // Dissassemble the data into a station report struct.
      //
      station_report_t S;
      
      memcpy(&S, stationReports.getChunks()[point].data, 
	     stationReports.getChunks()[point].len);

      station_report_from_be(&S);

      //
      // Get stuff from the structure.
      //
      time_t spdbDataTime = stationReports.getChunks()[point].valid_time;
      double lat = S.lat;
      double lon = S.lon;

      double U, V;

      if (
	  (S.windspd == STATION_NAN) ||
	  (S.winddir == STATION_NAN)
	  ){
	U = _params->badVal;
	V = _params->badVal;
      } else {
	U =  PHYwind_u(S.windspd, S.winddir);
	V =  PHYwind_v(S.windspd, S.winddir);
      }
      //
      // We now have everything we need from the
      // structure, start to print it out to file.
      //
      date_time_t T;
      T.unix_time = spdbDataTime;
      uconvert_from_utime( &T );


      char outputBuffer[4096];
      //
      // Timing stuff.
      //
      sprintf(outputBuffer,
	      "%d %02d %02d %02d %02d %02d\t",
	      T.year, T.month, T.day, T.hour, T.min, T.sec);
      //
      // Data pertaining to the SPDB point.
      //
      sprintf(outputBuffer, "%s%g\t%g\t",outputBuffer,
	      lat,lon);

      sprintf(outputBuffer, "%s%g\t%g\t", outputBuffer,
	      U, V);
    
      double temp, pres;

      if (S.temp == STATION_NAN){
	temp = _params->badVal;
      } else {
	temp = S.temp;
      }

      if (S.pres == STATION_NAN){
	pres = _params->badVal;
      } else {
	pres = S.pres;
      }


      sprintf(outputBuffer, "%s%g\t%g\t", outputBuffer,
	      temp, pres);

      bool OKtoPrint = true;
    
      //
      // We're done reading the GenPt - start
      // looking at the MDV data.
      //

      for (int imdv=0; imdv < _params->MDV_Fields_n; imdv++){

	DsMdvx Mdv;
	
	// Mdv.setDebug( _params->debug);
      
	if (_params->readFirstBefore){
	  Mdv.setReadTime(Mdvx::READ_FIRST_BEFORE, 
			  _params->_MDV_Fields[imdv].URL, 
			  _params->searchMargin,
			  spdbDataTime + _params->timeOffset );
	} else {
	  Mdv.setReadTime(Mdvx::READ_CLOSEST, 
			  _params->_MDV_Fields[imdv].URL, 
			  _params->searchMargin,
			  spdbDataTime + _params->timeOffset );
	}

	Mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
	Mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
	Mdv.addReadField( _params->_MDV_Fields[imdv].FieldName );
	//   Mdv.setDebug( _params->debug );
      
	if ( _params->_MDV_Fields[imdv].applyVlevelLimits ){
	  Mdv.setReadPlaneNumLimits( _params->_MDV_Fields[imdv].minVlevelPlaneNum,
				     _params->_MDV_Fields[imdv].maxVlevelPlaneNum);
	}
    

	if (Mdv.readVolume()){
	  cerr << "Failed to find MDV data for SPDB time ";
	  cerr << utimstr( spdbDataTime ) << endl;
	  cerr << "From URL " << _params->_MDV_Fields[imdv].URL << endl;
	  cerr << "Looking for field " << _params->_MDV_Fields[imdv].FieldName << endl;
	  OKtoPrint = false;
	  continue;
	} 

	MdvxField *InField = Mdv.getFieldByName( _params->_MDV_Fields[imdv].FieldName );
	
	if (InField == NULL){
	  cerr << "Field " << _params->_MDV_Fields[imdv].FieldName;
	  cerr << " not in dataset." << endl;
	  OKtoPrint = false;
	  continue;
	}
	
	Mdvx::master_header_t InMhdr = Mdv.getMasterHeader();
	Mdvx::field_header_t InFhdr = InField->getFieldHeader();
	Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
	
	MdvxProj Proj(InMhdr, InFhdr);

	fl32 *InData = (fl32 *) InField->getVol();

	//
	// Find the point in index space.
	//
	int centerX, centerY;
	
	if (0 != Proj.latlon2xyIndex(lat, lon, centerX, centerY)){
	  if (_params->debug){
	    cerr << "Data point is outside MDV grid." << endl;
	    OKtoPrint = false;
	    continue;
	  }
	}
	//
	// Loop through the MDV planes.
	//
	for (int iz = 0; iz < InFhdr.nz; iz++){
	  double total = 0.0; int num = 0;
	  for (int ix = centerX - _params->_MDV_Fields[imdv].gridWindow;
	       ix <= centerX + _params->_MDV_Fields[imdv].gridWindow;
	       ix++){
	
	    for (int iy = centerY - _params->_MDV_Fields[imdv].gridWindow;
		 iy <= centerY + _params->_MDV_Fields[imdv].gridWindow;
		 iy++){
	      if (
		  (ix >= 0)        && (iy >= 0) &&
		  (ix < InFhdr.nx) && (iy < InFhdr.ny)
		  ){
		int index = iz * InFhdr.nx*InFhdr.ny + iy * InFhdr.nx + ix;

		if (
		    (InData[index] != InFhdr.bad_data_value) &&
		    (InData[index] != InFhdr.missing_data_value)
		    ){
		  num++;
		  total = total + InData[index];
		}
	      }
	    }
	  }
	  //
	  // Done averaging for this plane.
	  //
	  double fieldVal;
	  if (num == 0){
	    fieldVal = _params->badVal;
	  } else {
	    fieldVal = total / double(num);
	  }
	  sprintf(outputBuffer,"%s%g\t", outputBuffer, fieldVal);
	} // End of loop through planes.
      } // End of loop through MDV fields.

      //
      // Print the line, if we got all the fields we needed.
      //
      if (OKtoPrint){
	fprintf(ofp,"%s\n",outputBuffer);
      }
    } // End of loop through all station reports.

  } // End of loop through station IDs.
  fclose( ofp );

  return 0;

}

