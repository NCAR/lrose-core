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
using namespace std;

//
// Constructor
//
Process::Process(){
  OutputUrl = (char *)NULL;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T, time_t genTime){

  if (P->Debug){
    if (genTime == 0L){
      cerr << "Data at " << utimstr(T) << endl;
    } else {
      cerr << "Data at " << utimstr(T) << " with gen time " << utimstr(genTime) << endl;
    }
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;
  
  if (genTime == 0L){
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  } else {
    int leadTime = int(T - genTime);
    New.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, P->TriggerUrl, 86400, genTime, leadTime );
  }
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  New.addReadField(P->InFieldName); // Only read one field

 if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
                         P->grid_minx, P->grid_miny,
                         P->grid_dx, P->grid_dy,
                         P->grid_origin_lat, P->grid_origin_lon,
                         P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
                           P->grid_minx, P->grid_miny,
                           P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
                        P->grid_minx, P->grid_miny,
                        P->grid_dx, P->grid_dy,
                        P->grid_origin_lat, 
                        P->grid_origin_lon,
                        P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }




  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     
  //
  // Get the desired field.
  //

  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  MdvxField *InField;

  InField = New.getFieldByNum( 0 ); // Only one field there

  if (InField == NULL){
    cerr << "New field " << P->InFieldName << " not found." << endl;
    return -1;
  }
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  if (InFhdr.nz != 1){
    cerr << "WARNING : Encountered 3D data, this is inherently a 2D program" << endl;
  }

  MdvxProj Proj(InMhdr, InFhdr);
  
  fl32 *InData = (fl32 *) InField->getVol();

  double ll_x, ll_y, ur_x, ur_y;

  if (P->wholeGrid){
    ll_x = 0; ll_y = 0; ur_x = InFhdr.nx-1; ur_y = InFhdr.ny-1;
  } else {
    if (
	(Proj.latlon2xyIndex(P->ll_lat, P->ll_lon, ll_x, ll_y))
	){
      cerr << "Lower left lat/lon point specified is not within grid - skipping ...." << endl;
      return -1;
    }
    
    if (
	(Proj.latlon2xyIndex(P->ur_lat, P->ur_lon, ur_x, ur_y))
	){
      cerr << "Upper right lat/lon point specified is not within grid - skipping ...." << endl;
      return -1;
    }
  }

  if (P->Debug){
    cerr << "Range : X : " << ll_x << " to " << ur_x << ", ";
    cerr <<        " Y : " << ll_y << " to " << ur_y << endl;
  }

  
  int numExceeded = 0;
  int numConsidered = 0;
  for(int iz=0; iz < InFhdr.nz; iz++){
    for(int iy=(int)rint(ll_y); iy < (int)rint(ur_y)+1; iy++){
      for(int ix=(int)rint(ll_x); ix < (int)rint(ur_x)+1; ix++){
	int index = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	//
	// If this data point is bad, leave it.
	//
	numConsidered++;
	if (
	    (InData[index] == InFhdr.missing_data_value) ||
	    (InData[index] == InFhdr.bad_data_value) ||
	    (InData[index] < P->threshold)
	    ) {
	  continue;
	}
	numExceeded++;
      }
    }
  }

  double answer = 0.0;
  if (numConsidered != 0){
    answer = 100.0*double(numExceeded) / double(numConsidered);
  } else {
    cerr << "No data considered!" << endl;
    return -1;
  }

  FILE *fp = fopen(P->outFileName,"a");
  if (fp == NULL){
    cerr << "Failed to create " << P->outFileName << endl;
    return -1;
  }

  if (P->printLeadTime){
    fprintf(fp, "%s %g %d\n",utimstr(T), answer, 
	    int(InMhdr.time_centroid - InMhdr.time_gen) );
  } else {
    fprintf(fp, "%s %g\n",utimstr(T), answer);
  }
  fclose(fp);


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










