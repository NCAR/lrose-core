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
// Mdv2Plain.cc
//
// Mdv2Plain object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
///////////////////////////////////////////////////////////////

#include <fstream>
#include <iomanip>
#include <cerrno>
#include <string>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaFile.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "Mdv2Plain.hh"
using namespace std;

// Constructor

Mdv2Plain::Mdv2Plain(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "Mdv2Plain";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = const_cast<char*>(string("unknown").c_str());
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
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

  // Initialize the Ldata info object

  _ldataInfo.setDir(_params.output_dir);
  _ldataInfo.setDataFileExt("plain");
  
  return;

}

// destructor

Mdv2Plain::~Mdv2Plain()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Mdv2Plain::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug);

  // loop until end of data

  while (!_input.endOfData()) {

    PMU_auto_register("In main loop");
    
    _processNextFile(mdvx);
    
  } // while

  return 0;

}

// Process the next input file

int Mdv2Plain::_processNextFile(DsMdvx &mdvx)

{
  // set up the Mdvx read

  mdvx.clearRead();
    
  if (_params.set_horiz_limits) {
    mdvx.setReadHorizLimits(_params.horiz_limits.min_lat,
			    _params.horiz_limits.min_lon,
			    _params.horiz_limits.max_lat,
			    _params.horiz_limits.max_lon);
  }
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
			     _params.upper_vlevel);
  }
  if (_params.set_plane_num_limits) {
    mdvx.setReadPlaneNumLimits(_params.lower_plane_num,
			       _params.upper_plane_num);
  }
  mdvx.setReadEncodingType((Mdvx::encoding_type_t)
			   _params.encoding_type);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  if (_params.composite) {
    mdvx.setReadComposite();
  }
  if (_params.set_field_names) {
    for (int i = 0; i < _params.field_names_n; i++) {
      mdvx.addReadField(_params._field_names[i]);
    }
  } else if (_params.set_field_nums) {
    for (int i = 0; i < _params.field_nums_n; i++) {
      mdvx.addReadField(_params._field_nums[i]);
    }
  }
  if (_params.remap_xy) {
    if (_params.remap_projection == Params::PROJ_LATLON) {
      mdvx.setReadRemapLatlon(_params.remap_grid.nx,
			      _params.remap_grid.ny,
			      _params.remap_grid.minx,
			      _params.remap_grid.miny,
			      _params.remap_grid.dx,
			      _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
      mdvx.setReadRemapLc2(_params.remap_grid.nx,
			   _params.remap_grid.ny,
			   _params.remap_grid.minx,
			   _params.remap_grid.miny,
			   _params.remap_grid.dx,
			   _params.remap_grid.dy,
			   _params.remap_origin_lat,
			   _params.remap_origin_lon,
			   _params.remap_lat1,
			   _params.remap_lat2);
    } else if (_params.remap_projection == Params::PROJ_FLAT) {
      mdvx.setReadRemapFlat(_params.remap_grid.nx,
			    _params.remap_grid.ny,
			    _params.remap_grid.minx,
			    _params.remap_grid.miny,
			    _params.remap_grid.dx,
			    _params.remap_grid.dy,
			    _params.remap_origin_lat,
			    _params.remap_origin_lon,
			    _params.remap_rotation);
    }
  } // if (_params.remap_xy)

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

  // perform the read

  PMU_auto_register("Before read");
    
  if (_input.readVolumeNext(mdvx)) {
    cerr << "ERROR - Mdv2Plain::Run" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << "  File: " << mdvx.getPathInUse() << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }

  if (mdvx.getMasterHeader().n_fields < 1) {
    cerr << "ERROR - Mdv2Plain::Run" << endl;
    cerr << "  No fields in data read in." << endl;
    cerr << "  File: " << mdvx.getPathInUse() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Working on file: " << mdvx.getPathInUse() << endl;
  }

  // print the file details

  int nBytesVol;
  _printInfo(mdvx, nBytesVol, cout);
    
  // write out

  if (_write(mdvx,nBytesVol)) {
    return -1;
  }

  return 0;
}

// print info about file

void Mdv2Plain::_printInfo(const DsMdvx &mdvx,
			   int &nBytesVol,
			   ostream &out)

{

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  
  if (_params.debug)
  {
    out << "Mdv2Plain - writing MDV data to plain arrays" << endl;
    out << "  File: " << mdvx.getPathInUse() << endl;
    out << "  time: " << DateTime::str(mhdr.time_centroid) << endl;
    out << "  n_fields: " << mhdr.n_fields << endl;
  
    for (int  i = 0; i < mhdr.n_fields; i++) {
      MdvxField *fld = mdvx.getField(i);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      out << "    field " << i << ", name: " << fhdr.field_name
	  << ", units: " << fhdr.units << endl;
    } // i
  }
  
  MdvxField *fld = mdvx.getField(0);
  const Mdvx::field_header_t &fhdr = fld->getFieldHeader();

  if (_params.debug)
  {
    out << "  nx: " << fhdr.nx << endl;
    out << "  ny: " << fhdr.ny << endl;
    out << "  nz: " << fhdr.nz << endl;
    out << "  proj_type: " << Mdvx::projType2Str(fhdr.proj_type) << endl;
    out << "  grid_minx: " << fhdr.grid_minx << endl;
    out << "  grid_miny: " << fhdr.grid_miny << endl;
    out << "  grid_minz: " << fhdr.grid_minz << endl;
    out << "  grid_dx: " << fhdr.grid_dx << endl;
    out << "  grid_dy: " << fhdr.grid_dy << endl;
    out << "  grid_dz: " << fhdr.grid_dz << endl;

    out << "  encoding_type: "
	<< Mdvx::encodingType2Str(fhdr.encoding_type) << endl;
    if (fhdr.encoding_type != Mdvx::ENCODING_FLOAT32) {
      out << "  Note: scale and bias will be written as 2 fl32 words," << endl
	  << "        immediately preceding the field data array." << endl;
    }
    out << "  data_element_nbytes: " << fhdr.data_element_nbytes << endl;
    out << "  nbytes per field: " << fhdr.volume_size << endl;
  }
  
  nBytesVol = fhdr.volume_size;
  
  if (_params.debug)
  {
    const Mdvx::vlevel_header_t &vhdr = fld->getVlevelHeader();
    out << "  vlevel_type: " << Mdvx::vertType2Str(vhdr.type[0]) << endl;
    for (int  i = 0; i < fhdr.nz; i++) {
      out << "    vlevel: " << setw(3) << i << ", val: "
	  << vhdr.level[i] << endl;
    } // i
  }
  
}

// write output

int Mdv2Plain::_write(const DsMdvx &mdvx, int nBytesVol)

{

  PMU_auto_register("Before write");

  // compute output dir and path
  
  char dir[MAX_PATH_LEN];
  char path[MAX_PATH_LEN];
  char logPath[MAX_PATH_LEN];
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  DateTime ftime(mhdr.time_centroid);

  sprintf(dir, "%s%s%.4d%.2d%.2d",
	  _params.output_dir, PATH_DELIM,
	  ftime.getYear(), ftime.getMonth(), ftime.getDay());

  sprintf(path, "%s%s%.2d%.2d%.2d.plain",
	  dir, PATH_DELIM,
	  ftime.getHour(), ftime.getMin(), ftime.getSec());

  sprintf(logPath, "%s%s%.2d%.2d%.2d.log",
	  dir, PATH_DELIM,
	  ftime.getHour(), ftime.getMin(), ftime.getSec());

  if (_params.debug) {
    cerr << "Writing plain file: " << path << endl;
  }

  // make directory

  if (ta_makedir_recurse(dir)) {
    cerr << "ERROR - Mdv2Plain::_write" << endl;
    cerr << "  Cannot make dir: " << dir << endl;
    return -1;
  }

  // write info to the log file

  ofstream logFile;
  logFile.open( logPath );
  if (logFile.rdbuf()->is_open()) {
    int volsize;
    _printInfo(mdvx, volsize, logFile);
    logFile.close();
  } else {
    cerr << "ERROR - Mdv2Plain::_write" << endl;
    cerr << "  Cannot open log file: " << logPath << endl;
  }
  
  // open file

  TaFile out;
  if (out.fopen(path, "w") == NULL) {
    int errNum = errno;
    cerr << "ERROR - Mdv2Plain::_write" << endl;
    cerr << "  Cannot open file : " << path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // write the fields

  for (int  ifield = 0; ifield < mhdr.n_fields; ifield++) {

    MdvxField *fld = mdvx.getField(ifield);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();

    if (fhdr.encoding_type != Mdvx::ENCODING_FLOAT32) {

      // scale and bias
      
      fl32 scale = fhdr.scale;
      fl32 bias = fhdr.bias;
      if (_params.big_endian_output) {
	BE_from_array_32(&scale, sizeof(fl32));
	BE_from_array_32(&bias, sizeof(fl32));
      }

      if (_writeFl32(out, scale)) {
	return -1;
      }
      if (_writeFl32(out, bias)) {
	return -1;
      }

    } // if (fhdr.encoding_type != Mdvx::ENCODING_FLOAT32)

    // field data

    if (fld->getVolLen() != nBytesVol) {
      cerr << "ERROR - Mdv2Plain::_write" << endl;
      cerr << "  Field: " << fhdr.field_name << endl;
      cerr << "  nbytes: " << fld->getVolLen() << endl;
      cerr << "  Should be" << nBytesVol << endl;
      return -1;
    }

    MemBuf fldData;
    switch (_params.output_row_ordering)
    {
    case Params::SOUTH_ROW_FIRST :
      fldData.add(fld->getVol(), fld->getVolLen());
      break;
      
    case Params::NORTH_ROW_FIRST :
      _addRowsNorthToSouth(fldData, *fld);
      break;
    }
    
    if (_params.big_endian_output) {
      if (fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {
	BE_from_array_32(fldData.getPtr(), fldData.getLen());
      } else if (fhdr.encoding_type == Mdvx::ENCODING_INT16) {
	BE_from_array_16(fldData.getPtr(), fldData.getLen());
      }
    }

    if (_params.fortran_output) {
      if (_writeFortranReclen(out, fldData.getLen())) {
	return -1;
      }
    }

    if (out.fwrite(fldData.getPtr(), 1, fldData.getLen()) !=
	fldData.getLen()) {
      int errNum = errno;
      cerr << "ERROR - Mdv2Plain::_write" << endl;
      cerr << "  Write failed for field: " << fhdr.field_name << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (_params.fortran_output) {
      if (_writeFortranReclen(out, fldData.getLen())) {
	return -1;
      }
    }
    
  } // ifield

  // close the file
  
  out.fclose();

  if (_params.gzip_output) {

    string sysStr = "gzip -f ";
    sysStr += path;

    system(sysStr.c_str());

  }

  // Write an Ldata file

  _ldataInfo.write(ftime.utime(), "plain");
  
  return 0;

}

// add the field data to the data buffer from north to south

void Mdv2Plain::_addRowsNorthToSouth(MemBuf &fldData, MdvxField &fld)
{
  Mdvx::field_header_t field_hdr = fld.getFieldHeader();
  
  switch (field_hdr.encoding_type)
  {
  case Mdvx::ENCODING_INT8 :
  {
    ui08 *data = (ui08 *)fld.getVol();
    for (int z = 0; z < field_hdr.nz; ++z)
    {
      for (int y = field_hdr.ny - 1; y >= 0; --y)
      {
	int row_offset = (z * field_hdr.nx * field_hdr.ny) +
	  (y * field_hdr.nx);
	
	fldData.add(&(data[row_offset]),
		    field_hdr.nx * sizeof(ui08));
      } /* endfor - y */
    } /* endfor - z */
  }
  break;
  
  case Mdvx::ENCODING_INT16 :
  {
    ui16 *data = (ui16 *)fld.getVol();
    for (int z = 0; z < field_hdr.nz; ++z)
    {
      for (int y = field_hdr.ny - 1; y >= 0; --y)
      {
	int row_offset = (z * field_hdr.nx * field_hdr.ny) +
	  (y * field_hdr.nx);
	
	fldData.add(&(data[row_offset]),
		    field_hdr.nx * sizeof(ui16));
      } /* endfor - y */
    } /* endfor - z */
  }
  break;
  
  case Mdvx::ENCODING_FLOAT32 :
  {
    fl32 *data = (fl32 *)fld.getVol();
    for (int z = 0; z < field_hdr.nz; ++z)
    {
      for (int y = field_hdr.ny - 1; y >= 0; --y)
      {
	int row_offset = (z * field_hdr.nx * field_hdr.ny) +
	  (y * field_hdr.nx);
	
	fldData.add(&(data[row_offset]),
		    field_hdr.nx * sizeof(fl32));
      } /* endfor - y */
    } /* endfor - z */
  }
  break;
  
  case Mdvx::ENCODING_RGBA32 :
  {
    ui32 *data = (ui32 *)fld.getVol();
    for (int z = 0; z < field_hdr.nz; ++z)
    {
      for (int y = field_hdr.ny - 1; y >= 0; --y)
      {
	int row_offset = (z * field_hdr.nx * field_hdr.ny) +
	  (y * field_hdr.nx);
	
	fldData.add(&(data[row_offset]),
		    field_hdr.nx * sizeof(ui32));
      } /* endfor - y */
    } /* endfor - z */
  }
  break;
  
  default:
    cerr << "ERROR - Mdv2Plain::_addRowsNorthToSouth()" << endl;
    cerr << "Invalid encoding type" << endl;
    cerr << "Data not added to output buffer" << endl;
    break;
  } /* endswitch - field_hdr.encoding_type */
  
}

// write output

int Mdv2Plain::_writeFortranReclen(TaFile &out, si32 rec_len)

{

  si32 out_rec_len = rec_len;
  
  if (_params.big_endian_output) {
    BE_from_array_32(&out_rec_len, sizeof(si32));
  }
  
  if (out.fwrite(&out_rec_len, sizeof(si32), 1) != 1) {
    int errNum = errno;
    cerr << "ERROR - Mdv2Plain::_writeFortranReclen" << endl;
    cerr << "  Write failed" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}

int Mdv2Plain::_writeFl32(TaFile &out, fl32 val)

{

  if (_params.fortran_output) {
    if (_writeFortranReclen(out, sizeof(fl32))) {
      return -1;
    }
  }
  if (out.fwrite(&val, sizeof(fl32), 1) != 1) {
    int errNum = errno;
    cerr << "ERROR - Mdv2Plain::_write" << endl;
    cerr << "  Write failed" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
  if (_params.fortran_output) {
    if (_writeFortranReclen(out, sizeof(fl32))) {
      return -1;
    }
  }

  return 0;

}


