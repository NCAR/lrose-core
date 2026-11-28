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
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  if (P->Debug){
    cerr << "Data at " << utimstr(T) << ", remapping height data" << endl;
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.addReadField(P->fieldName);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Read the height field.
  //
  DsMdvx ht;
  ht.addReadField(P->heightFieldName);

  ht.setReadTime(Mdvx::READ_CLOSEST, P->heightUrl, P->temporalTolerance, T);
  ht.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  ht.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  //
  // Have to remap to same proj as field to be thresholded.
  //
  Mdvx::master_header_t threshMhdr = New.getMasterHeader();
  MdvxField *threshField = New.getFieldByNum( 0 ); // Only one field
  if (threshField == NULL){
    cerr << "Threshold field " << P->fieldName << " not found." << endl;
    return -1;
  }
  Mdvx::field_header_t threshFhdr = threshField->getFieldHeader();
  Mdvx::vlevel_header_t threshVhdr = threshField->getVlevelHeader();


  MdvxProj threshProj( threshMhdr, threshFhdr);
  ht.setReadRemap( threshProj );

  if (ht.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->heightUrl  << endl;
    return -1;
  }

  if (P->Debug) cerr << "Thresholding using " << ht.getPathInUse() << endl;


  MdvxField *htField = ht.getFieldByNum( 0 ); // Only one field
  if (htField == NULL){
    cerr << "Height field " << P->heightFieldName << " not found." << endl;
    return -1;
  }
  Mdvx::field_header_t htFhdr = htField->getFieldHeader();
  Mdvx::vlevel_header_t htVhdr = htField->getVlevelHeader();

  fl32 *threshData = (fl32 *)threshField->getVol();
  fl32 *htData = (fl32 *)htField->getVol();
  fl32 *OutData = (fl32 *) malloc(sizeof(float) * threshFhdr.nx * threshFhdr.ny );
  
  for(int l=0; l < threshFhdr.nx * threshFhdr.ny;  l++){
    OutData[l]=threshFhdr.missing_data_value;
  }

  //
  // Do the actual processing.
  //
  for(int ix=0; ix < threshFhdr.nx; ix++){
    for(int iy=0; iy < threshFhdr.ny; iy++){

      int index2D = ix + threshFhdr.nx * iy;

      if ((htData[index2D] == htFhdr.bad_data_value) ||
	  (htData[index2D] == htFhdr.missing_data_value)){
	continue; // Without height data whole column is marked missing
      }

      double threshHeight = htData[index2D]*P->heightAdjust.scale + P->heightAdjust.bias;

       for(int iz=0; iz < threshFhdr.nz -1; iz++){

	 int index3D = index2D + iz * threshFhdr.nx * threshFhdr.ny;

	 if ((threshData[index3D] == threshFhdr.bad_data_value) ||
	     (threshData[index3D] == threshFhdr.missing_data_value)){
	   continue; // Data values that are bad/missing stay that way
	 }

	 double dataHeight = threshVhdr.level[iz];
	 if (  dataHeight >=  threshHeight - P->heightTolerance &&   
	    dataHeight < threshHeight + P->heightTolerance)
	{
	    OutData[index2D] = threshData[index3D]*P->layerAdjust.scale + P->layerAdjust.bias;
	 }
       }
    }
  }

  //
  // Set up the output.
  //
  string dsetInfo = "Ht threshed by " + ht.getPathInUse();
  sprintf(threshMhdr.data_set_info, "%s", dsetInfo.c_str());
  
  //
  // Makes some mods the thresh headers for 3d-> 2d data
  //
  threshMhdr.data_dimension = 2;
  threshMhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  threshMhdr.n_fields = 2;
  threshMhdr.max_nz  = 1;
  Out.setMasterHeader(threshMhdr);
  
  //
  // Makes some mods the field header for 3d-> 2d data
  //
  threshFhdr.nz = 1;
  threshFhdr.volume_size =  threshFhdr.nx * threshFhdr.ny * sizeof(threshFhdr.data_element_nbytes);
  threshFhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  threshFhdr.data_dimension = 2;
  threshFhdr.grid_dz = 0;

  //
  // Makes some mods the vlevle header for 3d-> 2d data
  //
  threshVhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  threshVhdr.level[0] = P->vlevelVal;

  MdvxField *fld = new MdvxField(threshFhdr, threshVhdr, OutData);
  free(OutData);
  
  if (fld->convertRounded(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  Out.addField(fld);


  htFhdr.forecast_delta=0;
  htFhdr.forecast_time=0;

  MdvxField *fld2 = new MdvxField(htFhdr, htVhdr, htData);
  if (fld2->convertRounded(Mdvx::ENCODING_INT16,
			   Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  Out.addField(fld2);



  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    exit(-1);
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










