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
int Process::Derive(Params *P, time_t T){

  OutputUrl = STRdup(P->OutUrl);

  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  New.addReadField( P->inFieldName );

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
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;


  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  //
  // Get the desired field - field 0
  //
  MdvxField *InField;

  InField = New.getFieldByNum( 0 ); 


  if (InField == NULL){
    cerr << "New field " << P->inFieldName << " not found." << endl;
    return -1;
  }
   
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();


  fl32 *InData = (fl32 *) InField->getVol();

  fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				    InFhdr.nx *
				    InFhdr.ny );

  if (OutData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  // Init to missing.
  //
  for(int l=0; l < InFhdr.nx*InFhdr.ny; l++){
    OutData[l]=InFhdr.missing_data_value;
  }
  

  for(int iy=0; iy < InFhdr.ny; iy++){
    for(int ix=0; ix < InFhdr.nx; ix++){
      
      int index2D = iy*InFhdr.nx + ix;
      //
      // Descend in Z.
      //
      for(int iz=InFhdr.nz-1; iz > -1; iz--){
	
	int index3D = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	//
	// If this data point is bad, leave it.
	//
	if (
	    (InData[index3D] == InFhdr.missing_data_value) ||
	    (InData[index3D] == InFhdr.bad_data_value)
	    ) {
	  continue;
	}
	
	//
	// If it is less than the threshold, leave it.
	//
	if (InData[index3D] < P->threshold){
	  continue;
	}
	
	//
	// OK - if we are here we have data that exceeds
	// the threshold, so record the height and then
	// break out of the Z loop.
	//
	OutData[index2D] = InVhdr.level[iz];
	//
	// If we are doing translations, do it now.
	//
	if (P->doTranslation){
	  for (int it=0; it < P->transTable_n; it++){
	    if ((OutData[index2D] > P->_transTable[it].minVal) &&
		(OutData[index2D] <= P->_transTable[it].maxVal)
		){
	      OutData[index2D] = P->_transTable[it].subsVal;
	      break;
	    }
	  }
	}
	break;
      }
    }
  }
  
  //
  // Write out these data.
  //

  InFhdr.nz = 1;
  InFhdr.volume_size = sizeof(float)*InFhdr.nx*InFhdr.ny;

  MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);
    
  free(OutData);

  fld->setFieldName( "Height" );
  fld->setUnits( P->heightUnits );
  fld->setFieldNameLong( "Height" );
    
  if (fld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
    
  Out.addField(fld);

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
  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
}










