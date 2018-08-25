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
// MdvSurfLevelProj.cc
//
// MdvSurfLevelProj object
//
// Sue Dettling, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 2004
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/DsMdvxInput.hh>
#include "MdvSurfLevelProj.hh"
#include <set>

using namespace std;

const float  MdvSurfLevelProj::MISSING_DATA = -99999.0;

const float  MdvSurfLevelProj::BAD_DATA = -99999.0;

// Constructor

MdvSurfLevelProj::MdvSurfLevelProj(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvSurfLevelProj";

  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
      return;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_url, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_url,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  return;

}

// destructor

MdvSurfLevelProj::~MdvSurfLevelProj()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvSurfLevelProj::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // loop until end of data
  
  _input.reset();

  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // create input DsMdvx object
    
    DsMdvx inMdvx;
    if (_params.debug) {
      inMdvx.setDebug(true);
    }
    
    // do the read
  
    PMU_auto_register("Before read");
    
    if (_doRead(inMdvx)) {
      continue;
    }
    
    if (_params.debug) {
      cerr << "Read in file: " << inMdvx.getPathInUse() << endl;
    }
    
    // create output DsMdvx object
    
    DsMdvx outMdvx;
    if (_params.debug) {
      outMdvx.setDebug(true);
    }
    
    setNewMasterHeader (inMdvx, outMdvx);

    // compute the projection, adding the fields to the output object
    
    _doProjection(inMdvx, outMdvx);

    

 
    PMU_auto_register("Before write");
    
    // set output compression
    
    for (int i = 0; i < outMdvx.getNFields(); i++) {
      MdvxField *field = outMdvx.getFieldByNum(i);
      field->compress(_params.output_compression_type);
    }

    // add any chunks

    outMdvx.clearChunks();
    for (int i = 0; i < inMdvx.getNChunks(); i++) {
      MdvxChunk *chunk = new MdvxChunk(*inMdvx.getChunkByNum(i));
      outMdvx.addChunk(chunk);
    }
    
    // write out
    
    if(outMdvx.writeToDir(_params.output_url)) {
      cerr << "ERROR - MdvSurfLevelProj::Run" << endl;
      cerr << "  Cannot write data set." << endl;
      cerr << outMdvx.getErrStr() << endl;
      iret = -1;
    }
    
    
  } // while
  
  return iret;

}

/////////////////////////////////////////////////////////
// perform the read
//
// Returns 0 on success, -1 on failure.

int MdvSurfLevelProj::_doRead(DsMdvx &inMdvx)
  
{
  
  // read in
  
  PMU_auto_register("Before read");
  
  if (_input.readVolumeNext(inMdvx)) {
    cerr << "ERROR - MdvSurfLevelProj::_doRead" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Read in file: " << inMdvx.getPathInUse() << endl;
  }

  return 0;

}
  
/////////////////////////////////////////////////////////
// 
// compute the projection for the cape field.
// Record the data indices from which the projected value is 
// taken for each point in the grid. Project all other
// fields identically top the cape field using the array 
// of data indices.
//
int MdvSurfLevelProj::_doProjection(DsMdvx &inMdvx,
			       DsMdvx &outMdvx)  
{
  //
  // This is a one time use only application 
  // so just code for volumes in order xyz
  //

  Mdvx::master_header_t mhdr = inMdvx.getMasterHeader();

  if (mhdr.data_ordering != Mdvx::ORDER_XYZ)
    {
      cerr << "Exiting... Expecting data in ORDER_XYZ\n";
      return 1;
    }

  if (_params.debug) 
      cerr << "Creating projection for field CAPE\n";
  
  
  //
  // Set up read.
  //
  inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  inMdvx.setReadScalingType(Mdvx::SCALING_NONE);
  inMdvx.readVolume();

  //
  // Project the cape field first: get the cape field, field header, and data.
  //
  MdvxField *cape3Dfield = inMdvx.getFieldByName( "CAPE" );

  Mdvx::field_header_t cape3DFhdr = cape3Dfield->getFieldHeader();

  fl32 *cape3Ddata = (fl32 *) cape3Dfield->getVol();

  //
  // Get grid basic field header information
  //
  int nx = cape3DFhdr.nx;
  
  int ny = cape3DFhdr.ny;
  
  int nz = cape3DFhdr.nz;
  
  float bad = cape3DFhdr.bad_data_value;
  
  float missing = cape3DFhdr.missing_data_value;
 
  //
  // Create and init projected cape data.
  //
  fl32 *projectedCape = new float[nx * ny];
  
  for (int i = 0 ; i < nx * ny ; i++)
    projectedCape[i] = MISSING_DATA;

  //
  // Create and init array to hold indices of 
  // surface data for each point in the plane
  //
  int *surface_levels = new int[ nx * ny];

  for (int i = 0 ; i < nx * ny ; i++)
    surface_levels[i] = -1;

  //
  // Do projection
  // 
  for (int i = 0 ; i < nx; i++)
    {
      for (int j = 0; j < ny; j++)
	{
	  //
	  // For point in plane, check values up the column of
	  // data, stopping when we get a good value.
	  //
	  int index2D = nx * j  + i;
	  
	  for (int z = 0; z < nz; z++)
	    {

	      int index3D = z * nx * ny + nx * j  + i;
	      
	      if ( fabs(cape3Ddata[index3D] - missing) > .1 && fabs(cape3Ddata[index3D] - bad) > .1)
		{
		  projectedCape[index2D] = cape3Ddata[index3D];
		
		  surface_levels[index2D] = index3D;
		  
		  z = nz;
		}
	      
	    }
	}
    }

  //
  // Create and add new output field
  //
  createNewField(outMdvx, cape3DFhdr, projectedCape);
  
  //
  // Clean up
  //
  delete[] (projectedCape);


  //
  // Now project all other fields
  //
  for (int i = 0; i < inMdvx.getNFields(); i++) 
    {
      PMU_auto_register("Projecting field");

      MdvxField *inField = inMdvx.getFieldByNum(i);
      
      if ( strcmp( inField->getFieldName(), "CAPE") != 0 )
	{
	  if (_params.debug) 
	    {
	      cerr << "Creating projection for field: " 
		   << inField->getFieldName() << endl;
	    }
        
	  //
	  // Get input field header and data/
	  //
	  Mdvx::field_header_t inFhdr = inField->getFieldHeader();
	  
	  fl32 *data3D = (fl32 *) inField->getVol();
  
	  //
	  // Create and init new field.
	  //
	  fl32 *projectedData = new float[nx * ny];

	  for (int i = 0 ; i < nx * ny ; i++)
	    projectedData[i] = MISSING_DATA;

	  //
	  // At each point in the plane project the 
	  // value from the same zlevel as was projected
          // for the CAPE field.
	  //
	  for (int i = 0; i < nx; i++)
	    for (int j = 0; j < ny; j++) 
	    {
	      int index2D = j * nx + i;
	      
	      if (surface_levels[index2D] == -1)
		projectedData[index2D] = MISSING_DATA;
	      else
		projectedData[index2D]  = data3D[ surface_levels[index2D] ];
	    }
	  
	  //
	  // Create and add new output field
	  //
	  createNewField(outMdvx, inFhdr, projectedData);
	  
	  delete[] projectedData;
	}
    }

  delete[] surface_levels;

  return 0;
}

void MdvSurfLevelProj::createNewField(DsMdvx &outMdvx, Mdvx::field_header_t &input3Dfhdr, float *data)		   
{
  //
  // Setup new field header
  //
  Mdvx::field_header_t projectedFhdr;

  MEM_zero(projectedFhdr);
  projectedFhdr.nx = input3Dfhdr.nx;
  projectedFhdr.ny = input3Dfhdr.ny;
  projectedFhdr.nz = 1;
  projectedFhdr.data_element_nbytes = sizeof(fl32);
  projectedFhdr.volume_size = projectedFhdr.nx * projectedFhdr.ny *sizeof(fl32);
  projectedFhdr.proj_origin_lat = input3Dfhdr.proj_origin_lat;
  projectedFhdr.proj_origin_lon = input3Dfhdr.proj_origin_lon;
  projectedFhdr.grid_minx =  input3Dfhdr.grid_minx;
  projectedFhdr.grid_miny =  input3Dfhdr.grid_miny;
  projectedFhdr.grid_minz = 0;
  projectedFhdr.grid_dx =  input3Dfhdr.grid_dx;
  projectedFhdr.grid_dy =  input3Dfhdr.grid_dy;
  projectedFhdr.grid_dz = 0;
  projectedFhdr.missing_data_value = MISSING_DATA;
  projectedFhdr.bad_data_value = BAD_DATA;
  projectedFhdr.encoding_type = input3Dfhdr.encoding_type;
  projectedFhdr.compression_type = input3Dfhdr.compression_type;
  projectedFhdr.compression_type = input3Dfhdr.scaling_type;
  projectedFhdr.proj_type = input3Dfhdr.proj_type;
  sprintf(projectedFhdr.field_name_long,"%s",input3Dfhdr.field_name_long);
  sprintf(projectedFhdr.field_name,"%s",input3Dfhdr.field_name);
  sprintf(projectedFhdr.units,"%s",input3Dfhdr.units);

  //
  // Set up new vlevel header
  //
  Mdvx::vlevel_header_t vhdr;
  
  MEM_zero(vhdr);
  
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  
  vhdr.level[0] = 0;

  // create field

  MdvxField *field = new MdvxField(projectedFhdr, vhdr, data);

  outMdvx.addField(field);


} 

void MdvSurfLevelProj::setNewMasterHeader (const DsMdvx &inMdvx, DsMdvx &outMdvx)
{

  //
  // Get master header of inMdvx
  //
  
  Mdvx::master_header_t inmhdr = inMdvx.getMasterHeader();


  // Set up master header of outMdvx.
  //
  Mdvx::master_header_t outmhdr;

  outmhdr.time_gen = time(0);

  
  outmhdr.time_begin = inmhdr.time_begin;

  outmhdr.time_end = inmhdr.time_end;
  
  outmhdr.time_centroid = inmhdr.time_centroid;

  outmhdr.time_expire = inmhdr.time_expire;

  sprintf(outmhdr.data_set_info,"%s","Surface Level Projected Data");

  sprintf(outmhdr.data_set_name,"%s","Surface Level Projected Data");

  sprintf(outmhdr.data_set_source,"%s", _params.input_url );
 
  outmhdr.max_nz = 1;
 
  sprintf(outmhdr.data_set_info,"Composites computed using MdvSurfLevelProj");
  
  outMdvx.setMasterHeader(outmhdr);

  outMdvx.clearFields();

}








