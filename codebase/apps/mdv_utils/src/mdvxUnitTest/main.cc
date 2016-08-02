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

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include <iostream>

int main(int argc, char *argv[]){

  //
  // Prepare to write MDV files. Declare a master, field
  // and vlevel header.
  //
  Mdvx::field_header_t fhdr;
  Mdvx::vlevel_header_t vhdr;
  Mdvx::master_header_t Mhdr;
  //
  // Use 'memset' to set all the bytres in these structures to 0.
  //
  memset(&fhdr, 0, sizeof(fhdr));
  memset(&vhdr, 0, sizeof(vhdr));
  memset(&Mhdr, 0, sizeof(Mhdr));
  
  ///////////////////////////////////////////////////////////////////
  //
  // Set up the master header. The times MUST be set.
  time_t dataTime = time(NULL);

  Mhdr.time_centroid = dataTime;
  Mhdr.time_gen = dataTime;
  Mhdr.time_begin = dataTime;
  Mhdr.time_end = dataTime;
  Mhdr.time_expire = dataTime + 3600; // Won't expire for an hour
  Mhdr.time_centroid = dataTime;

  // These master header fields need not necessairly be set - if they
  // are not I believe they are set in the construction
  // of the DsMdvx object. I'm doing it here to show some
  // more fields. To see them all, look in 
  // libs/Mdv/src/include/Mdv/Mdvx_typedefs.hh 
  // which shows the field and vlevel header definitions too.

  Mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  Mhdr.num_data_times = 1;
  Mhdr.data_ordering = Mdvx::ORDER_XYZ;  
  Mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  Mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;

  sprintf(Mhdr.data_set_info,"%s","unitTest");
  sprintf(Mhdr.data_set_name,"%s","unitTest");
  sprintf(Mhdr.data_set_source,"%s", "unitTest");



  ////////////////////////////////////////////////////////////
  //
  // Set up the field header. This MDV object will have only one
  // field (MDV supports multiple fields per object, definitely, but let's be simple).
  // Again, some of these don't have to be set, if they are not set they are
  // set appropriately in the DsMdvx object constructor. And again (just
  // like the master header above) this is not an exhaustive list
  // of the field header's elements.
  //
  fhdr.nx = 100;
  fhdr.ny = 100;
  fhdr.nz = 5;
  //
  fhdr.grid_dx = 1;
  fhdr.grid_dy = 1; // Horizontal incement is 1 Km either direction
  //
  fhdr.proj_type = Mdvx::PROJ_FLAT; 
  //
  fhdr.proj_origin_lat =  40.0;
  fhdr.proj_origin_lon =  -95.0; // Arbitrary, somewhere in the USA
  //
  // Calculate the distance from the origin to the middle of the lower left grid point
  // to get the minx, miny distances in Kilometers. For flat earth projections the
  // origin is typically in the middle of the grid to minimize the distortion inherrent
  // in this projection (the earth is not flat - sorry).
  //
  //
  fhdr.grid_minx = -fhdr.nx * fhdr.grid_dx + fhdr.grid_dx/2.0;
  fhdr.grid_miny = -fhdr.ny * fhdr.grid_dy + fhdr.grid_dy/2.0;
  //
  //
  // Set up an uncompressed grid of floating point values.
  //
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     

  fhdr.forecast_time = Mhdr.time_centroid;


  //
  // State what vlevel type we have.
  //
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.dz_constant = 1; // Indicates that vertical increments are constant, 0=> they vary

  // Set names, units etc.

  sprintf( fhdr.field_name_long,"%s", "Test field");
  sprintf( fhdr.field_name,"%s", "test");
  sprintf( fhdr.units,"%s", "noUnits");
  sprintf( fhdr.transform,"%s","none");

  fhdr.grid_minz = 1.0;
  fhdr.grid_dz = 0.5;
  
  ///////////////////////////////////////////////////////////////////
  //
  // Set up the vlevel header. Set the type to be
  // kilometers above sea level.
  //
  vhdr.type[0] = Mdvx::VERT_TYPE_Z;

  // Set it up so that height = 1.0 + 0.5 * planeNumber
  // Heights in Km are thus 1.0, 1.5, 2.0, 2.5, 3.0

  for (int i=0; i < fhdr.nz; i++){
    vhdr.level[i] = 1.0 + 0.5 * i;
  }


  //
  // Set up some data. Make the data value at each point equal to
  // 100000*iz + 10 * iy + ix
  //
  fl32 *data = (fl32 *)malloc(fhdr.nx*fhdr.ny*fhdr.nz*sizeof(fl32));
  if (data==NULL){
    cerr << "Malloc failed!" << endl;
    exit(-1);
  }

  for (int iz=0; iz < fhdr.nz; iz++){
    for (int iy=0; iy < fhdr.ny; iy++){
      for (int ix=0; ix < fhdr.nx; ix++){
	data[iz * fhdr.nx * fhdr.nx + iy * fhdr.nx + ix] = 100000*iz + 10 * iy + ix;
	}
    }
  }

  //
  // Construct a field object from the data, the field header and the vlevel header.
  //
  MdvxField *field = new MdvxField(fhdr, vhdr, data);

  //
  // Convert the field to be quantized, compressed.
  //
  if (field->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
    fprintf(stderr, "conversion of field failed - I cannot go on.\n");
    exit(-1);
  }


  free(data); // Don't need initial data workspace anymore.

  //
  // Now, construct a DsMdvx object that holds this one field.
  //
  DsMdvx testObj;
  testObj.setMasterHeader( Mhdr );
  testObj.addField(field);

  //
  // At this point it turns out that we do have to write a
  // data file - the reason being that the selection of a
  // subset of verical levels only happens when we read a file, and that's what
  // we want to test.
  //
  if (testObj.writeToDir("./testData")) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write test data : " << endl;
    cerr << testObj.getErrStr() << endl;
    exit(-1);
  }

  //
  // Now, set up a new Mdvx object, set it up so that it will
  // only read some of the vertical planes, and then read the data
  // off disk.
  //
  DsMdvx New;

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, "./testData", 0, dataTime);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  New.setReadVlevelLimits(1.4, 2.6); // Should get the middle 3 planes at heights 1.5, 2.0 and 2.5 Km

  if (New.readVolume()){
    cerr << "Read failed." << endl;
    exit(-1);
  }     

  Mdvx::master_header_t InMhdr = New.getMasterHeader();

  // TEST : Should have one field
  if (InMhdr.n_fields != 1){
    cerr << "Got wrong number of fields! " << InMhdr.n_fields  << endl;
  } else {
    cerr << "Number of fields correct" << endl;
  }

  MdvxField *InField = New.getFieldByNum( 0 ); // More common to look up fields by name but there is only one field here
  if (InField == NULL){
    cerr << "Failed to get field." << endl;
    exit(-1);
  }


  Mdvx::field_header_t InFhdr = InField->getFieldHeader();

  // TEST : Should have 3 planes
  if (InFhdr.nz != 3){
    cerr << "Got wrong number of planes! " << InFhdr.nz << endl;
  } else {
    cerr << "Number of planes correct." << endl;
  }


  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
  
  // TEST - Vertical levels should be about 1.5, 2.0, 2.5
  bool vTestOK = true;
  for (int i=0; i < InFhdr.nz; i++){
    double expectedLevel = 1.5 + 0.5 * i;
    if (fabs(InVhdr.level[i]-expectedLevel) > 0.001){
      vTestOK = false;
      break;
    }
  }

  if (vTestOK){
    cerr << "Vertical plane heights as expected." << endl;
  } else {
    cerr << "Vertical plane height test failed!" << endl;
    for (int i=0; i < InFhdr.nz; i++){
      cerr << InVhdr.level[i] << endl;
    }
  }



  fl32 *InData = (fl32 *) InField->getVol();

  // TEST : Data values should approximate what they were in the original volume.
  bool dataTestOK = true;
  double dataError = 0.0;
  for (int iz=0; iz < InFhdr.nz; iz++){
    for (int iy=0; iy < InFhdr.ny; iy++){
      for (int ix=0; ix < InFhdr.nx; ix++){
	double expectedValue = 100000*(iz+1) + 10 * iy + ix; // First plane here is the old second plane.
	if (fabs(InData[iz * InFhdr.nx * InFhdr.nx + iy * InFhdr.nx + ix] - expectedValue) > 10.0){
	  dataTestOK = false;
	  dataError = fabs(InData[iz * InFhdr.nx * InFhdr.nx + iy * InFhdr.nx + ix] - expectedValue);
	  break;
	}
      }
    }
  }

  if (dataTestOK){
    cerr << "Data as expected." << endl;
  } else {
    cerr << "Data test failed! Value was " << dataError << endl;
  }


  return 0;

}
