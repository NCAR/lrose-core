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
#include <Mdv/MdvxChunk.hh>
#include <Mdv/DsMdvx.hh>

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

  //
  // Set the read time. Depends somewhat as to if these
  // are forecast data or not.
  //
  if (genTime == 0L){
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  } else {
    int leadTime = int(T - genTime);
    New.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, P->TriggerUrl, 86400, genTime, leadTime );
  }

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

  //
  // Only process specified field names, if desired.
  //
  if (P->useInputFieldNames){
    for (int i=0; i < P->InFieldNames_n; i++){
      if (P->Debug){
	cerr << "Adding read field name " << P->_InFieldNames[i] << endl;
      }
      New.addReadField( P->_InFieldNames[i] );
    }
  }

  //
  // Apply vlevel plane limits, if desired.
  //
  if (P->applyVlevelPlaneLimits){
    New.setReadPlaneNumLimits(P->minVlevelPlane, P->maxVlevelPlane);
  }
  //
  // Apply vlevel height limits, if desired.
  //
  if (P->applyVlevelHeightLimits){
    New.setReadVlevelLimits(P->minVlevelHeight, P->maxVlevelHeight);
  }

  //
  // Composite, if desired.
  //
  if (P->composite){
    New.setReadComposite();
  }

  //
  // Set horizontal limits, if desired.
  //
  if (P->applyHorizLimits){
    New.setReadHorizLimits(P->horiz_limits.min_lat,
			   P->horiz_limits.min_lon,
			   P->horiz_limits.max_lat,
			   P->horiz_limits.max_lon);
  }

  //
  // Set encoding type.
  //
  Mdvx::encoding_type_t encoding = Mdvx::ENCODING_ASIS;
  switch (P->encodingType) {

  case Params::ENCODING_MDV_ASIS :
    encoding = Mdvx::ENCODING_ASIS;
    break;

  case Params::ENCODING_MDV_INT8 :
    encoding = Mdvx::ENCODING_INT8;
    break;

  case Params::ENCODING_MDV_INT16 :
    encoding = Mdvx::ENCODING_INT16;
    break;

  case Params::ENCODING_MDV_FLOAT32 :
    encoding = Mdvx::ENCODING_FLOAT32;
    break;


  default :
    cerr << "Unrecognized encoding type " << P->encodingType << " I cannot cope." << endl;
    exit(-1);
    break;

  }

  New.setReadEncodingType( encoding );

  //
  // Compression type.
  //
  Mdvx::compression_type_t compression = Mdvx::COMPRESSION_ASIS;
  switch (P->compressionType) {

  case Params::COMPRESSION_ASIS :
    compression = Mdvx::COMPRESSION_ASIS;
    break;

  case Params::COMPRESSION_NONE :
    compression = Mdvx::COMPRESSION_NONE;
    break;

  case Params::COMPRESSION_RLE :
    compression = Mdvx::COMPRESSION_RLE;
    break;

  case Params::COMPRESSION_LZO :
    compression = Mdvx::COMPRESSION_LZO;
    break;

  case Params::COMPRESSION_ZLIB :
    compression = Mdvx::COMPRESSION_ZLIB;
    break;

  case Params::COMPRESSION_BZIP :
    compression = Mdvx::COMPRESSION_BZIP;
    break;

  case Params::COMPRESSION_GZIP :
    compression = Mdvx::COMPRESSION_GZIP;
    break;

  default :
    cerr << "Unrecognized compression type " << P->compressionType << " I cannot cope." << endl;
    exit(-1);
    break;

  }

  New.setReadCompressionType( compression );

  //
  // Scaling type. Optional since there is no "leave it alone" setting.
  //
  if (P->changeScalingType){
    Mdvx::scaling_type_t scaling = Mdvx::SCALING_ROUNDED;

    switch( P->scalingType ){

    case Params::SCALING_ROUNDED :
      scaling = Mdvx::SCALING_ROUNDED;
      break;

    case Params::SCALING_INTEGRAL :
      scaling = Mdvx::SCALING_INTEGRAL;
      break;

    case Params::SCALING_DYNAMIC :
      scaling = Mdvx::SCALING_DYNAMIC;
      break;

    case Params::SCALING_SPECIFIED :
      scaling = Mdvx::SCALING_SPECIFIED;
      break;

    default :
      cerr << "Unrecognized scaling type " << P->scalingType << " I cannot cope." << endl;
      exit(-1);
      break;

    }
    New.setReadScalingType(scaling, P->scale, P->bias );
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

  int numFieldsOut = 0;

  DsMdvx Out;

  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  //
  // Commence loop through fields.
  //
  for (int ifld=0; ifld < InMhdr.n_fields; ifld++){
    //
    // Get the desired field.
    //
    MdvxField *InField;

    InField = New.getFieldByNum( ifld ); 

    if (InField == NULL){
      cerr << "WARNING : input field number " << ifld << " not found - skipping..." << endl;
      continue;
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    if (P->overrideVlevels){
      if (InFhdr.nz != P->vlevels_n){
	cerr << "Warning : Field " << ifld << " has " << InFhdr.nz;
	cerr << " vertical levels, not " << P->vlevels_n;
	cerr << " - ignoring request to modify vlevels for this field." << endl;
      } else {
	for (int iz=0; iz < InFhdr.nz; iz++){
	  InVhdr.level[iz]=P->_vlevels[iz];
	}
      }
    }

    fl32 *InData = (fl32 *) InField->getVol();

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, InData);
        
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    

    numFieldsOut++;
    Out.addField(fld);
    if (P->Debug){
      cerr << "Adding input field " << ifld << endl;
    }
  } // End of loop through the fields.

  //
  // Add chunks, if desired.
  //
  if (P->addChunks){
    for (int ic=0; ic < New.getNChunks(); ic++){
      MdvxChunk *inChunk = New.getChunkByNum( ic );
      //
      // The chunk must be created using 'new'.
      // The chunk object becomes 'owned' by the Mdvx object, and will be
      // freed by it.
      //
      MdvxChunk *outChunk = new MdvxChunk( *inChunk );
      Out.addChunk( outChunk );
    }
    if (P->Debug){
      cerr << New.getNChunks() << " chunks were added." << endl;
    }
  }

  //
  // Do the write thing.
  //
  if (numFieldsOut == 0){
    cerr << "WARNING : Writing MDV file with ZERO FIELDS!" << endl; 
    // Could happen in bad setup of param file.
  }

  if (P->writeAsForecast) Out.setWriteAsForecast();

  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    exit(-1);
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










