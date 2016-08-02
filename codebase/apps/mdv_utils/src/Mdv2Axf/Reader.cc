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

#include <ctime>
#include <Mdv/DsMdvx.hh>

#include "Mdv2Axf.hh"

#include "Reader.hh"
using namespace std;

//
// Constructor.
//
Reader::Reader(){


}

//
// Destructor.
//
Reader::~Reader(){

}

//
// Process.
//
int Reader::Process(time_t t, Params *P){
  //
  // Read the URL.
  //
  DsMdvx MdvInput;

 
  //
  // Set the times on the URL.
  //
  MdvInput.setReadTime( Mdvx::READ_FIRST_BEFORE, P->InMdvURL, 
			P->TriggerTimeLookBack, t );
  

  //
  // Set compositing, if desired.
  //
  if (P->DoComposite){
    MdvInput.setReadVlevelLimits(P->CompositeMin, P->CompositeMax);
    MdvInput.setReadComposite();
  }

  //
  // Read the actual data into uncompressed floats.
  //
  MdvInput.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  MdvInput.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  //
  // Do a grid remapping, if requested.
  //

  if (P->DoGridRemap){

    switch (P->grid_projection){

    case Params::FLAT:
      MdvInput.setReadRemapFlat(P->grid_nx, P->grid_ny,
				 P->grid_minx, P->grid_miny,
				 P->grid_dx, P->grid_dy,
				 P->grid_origin_lat, P->grid_origin_lon,
				 P->grid_rotation);
      break;

    case Params::LATLON:
      MdvInput.setReadRemapLatlon(P->grid_nx, P->grid_ny,
				   P->grid_minx, P->grid_miny,
				   P->grid_dx, P->grid_dy);

      break;

    default:
      fprintf(stderr,"Unsupported projection.\n");
      return -1;
      break;

    }
  }

  if (MdvInput.readVolume()){
    cerr << "Mdv read failed from URL " << P->InMdvURL << " at ";
    cerr << utimstr( t ) << endl;
    return -1;
  }


  //
  // Get the field header that is representitative of.
  // the grid we will be writing to. If the fields do
  // not differ this can be any field header, otherwise,
  // only one field may be selected and we must use this field header.
  //
  MdvxField *field;

  int NumFields;
  if (P->UseFieldNumbers){
    NumFields = P->FieldNumbers_n;
  } else { 
    NumFields = P->FieldNames_n;
  }

  if (NumFields != P->FieldNames_n){
    cerr << "Wrong number of Field Names specified!" << endl;
    return -1;
  }

  Mdvx::master_header_t Mhdr = MdvInput.getMasterHeader();
  if ((Mhdr.field_grids_differ) && (NumFields > 1)){
    cerr << "Multiple  fields requested, but input grids differ." << endl;
    return -1;
  }

  if (P->UseFieldNumbers){
      field = MdvInput.getFieldByNum(P->_FieldNumbers[0]);
    } else { 
      field = MdvInput.getFieldByName(P->_FieldNames[0]);
    }

  if (field == NULL){
    cerr << "Could not find field ";
    if (P->UseFieldNumbers){
      cerr << "number " << P->_FieldNumbers[0] << endl;
    } else {
      cerr << P->_FieldNames[0] << endl;
    }
    cerr << "Exiting before main loop." << endl;
    return -1;
  }

  Mdvx::field_header_t Fhdr = field->getFieldHeader(); 
  
  Mdv2Axf M;

  M.Begin(Fhdr,t,P);

  for (int i=0; i < NumFields; i++){

    if (P->UseFieldNumbers){
      field = MdvInput.getFieldByNum(P->_FieldNumbers[i]);
    } else { 
      field = MdvInput.getFieldByName(P->_FieldNames[i]);
    }

    if (field == NULL){
      cerr << "Could not find field ";
      if (P->UseFieldNumbers){
	cerr << "number " << P->_FieldNumbers[0] << endl;
      } else {
	cerr << P->_FieldNames[0] << endl;
      }
      return -1;
    }

    //
    // Pass the field off to be processed.
    //
    M.Process(t, field, i, P->PlaneNumber, P->MissingVal, P->ConversionFactor,
	      P->_FieldNames[i]);

  }

  M.CloseOutput();
  return 0;

}







