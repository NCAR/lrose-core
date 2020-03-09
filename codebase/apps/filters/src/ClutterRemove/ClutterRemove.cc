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
// ClutterRemove.cc
//
// Mike Dixon
// RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////
//
// ClutterRemove removes clutter from MDV radar files.
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include "ClutterRemove.hh"
using namespace std;

// Constructor

ClutterRemove::ClutterRemove(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "ClutterRemove";
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
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode, must specify start and end dates."
	   << endl << endl;
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
    if (_input.setRealtime(_params.input_url,
			   _params.max_realtime_valid_age,
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

ClutterRemove::~ClutterRemove()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ClutterRemove::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug);

  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");
    
    // read in clutter file
    
    if (_readClutterFile()) {
      iret = -1;
      continue;
    }
    
    // set up the Mdvx read
    
    _setupInputRead(mdvx);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Read for input file:" << endl;
      mdvx.printReadRequest(cerr);
    }
    
    // read the volume
    
    PMU_auto_register("Before read");
    if (_input.readVolumeNext(mdvx)) {
      cerr << "ERROR - ClutterRemove::Run" << endl;
      cerr << "  Cannot read in data." << endl;
      cerr << _input.getErrStr() << endl;
      iret = -1;
      continue;
    }
    
    if (mdvx.getField(_params.input_dbz_field_name) == NULL) {
      cerr << "ERROR - ClutterRemove::Run" << endl;
      cerr << "  " << _params.input_dbz_field_name
	   << " missing from input file." << endl;
      cerr << "  File: " << mdvx.getPathInUse() << endl;
      iret = -1;
      continue;
    }
  
    // process this file
    
    if (_processFile(mdvx)) {
      iret = -1;
    }
    
  } // while

  return iret;
  
}

///////////////////
// process the file

int ClutterRemove::_processFile(DsMdvx &mdvx)
  
{

  if (_params.debug) {
    cerr << "Processing file: " << mdvx.getPathInUse() << endl;
  }

  // check the xy geometry
  
  const MdvxField *cfld = _clutter.getField(_params.clutter_dbz_field_name);
  const Mdvx::field_header_t &cfhdr = cfld->getFieldHeader();
  const Mdvx::vlevel_header_t &cvhdr = cfld->getVlevelHeader();
  
  MdvxField *dfld = (MdvxField *) mdvx.getField(_params.input_dbz_field_name);
  const Mdvx::field_header_t &dfhdr = dfld->getFieldHeader();
  const Mdvx::vlevel_header_t &dvhdr = dfld->getVlevelHeader();
  
  if (dfhdr.nx != cfhdr.nx ||	dfhdr.ny != cfhdr.ny) {
    cerr << "ClutterRemove::_processFile" << endl;
    cerr << "Input file geometry does not match clutter file." << endl;
    cerr << "Input   nx, ny: " << dfhdr.nx << ", " << dfhdr.ny << endl;
    cerr << "Clutter nx, ny: " << cfhdr.nx << ", " << cfhdr.ny << endl;
    return -1;
  }

  vector<MdvxField *> iflds;
  vector<const Mdvx::field_header_t *> ifhdrs;
  
  if (_params.remove_from_all_fields) {
    
    for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {

      const MdvxField *ifld = mdvx.getField(ii);
      const Mdvx::field_header_t &ifhdr = ifld->getFieldHeader();
      if (strcmp(ifhdr.field_name, dfhdr.field_name)) {
	if (ifhdr.nx != cfhdr.nx ||	ifhdr.ny != cfhdr.ny) {
	  cerr << "ClutterRemove::_processFile" << endl;
	  cerr << "Input file geometry does not match clutter file." << endl;
	  cerr << "Input   nx, ny: " << ifhdr.nx << ", " << ifhdr.ny << endl;
	  cerr << "Clutter nx, ny: " << cfhdr.nx << ", " << cfhdr.ny << endl;
	  return -1;
	}
	if (ifhdr.nz != dfhdr.nz) {
	  cerr << "ClutterRemove::_processFile" << endl;
	  cerr << "Input file nz values not constant." << endl;
	  cerr << "Dbz field nz: " << dfhdr.nz << endl;
	  cerr << ifhdr.field_name << " field nz: " << ifhdr.nz << endl;
	  return -1;
	}
      } // if (strcmp(ifhdr.field_name ...

      iflds.push_back((MdvxField *) ifld);
      ifhdrs.push_back(&ifhdr);
      
    } // ii

  } // if (_params.remove_from_all_fields)
  
  // make a copy of the dbz field and convert to fl32

  MdvxField floatDbz(*dfld);
  floatDbz.convertType(Mdvx::ENCODING_FLOAT32);
  
  // loop through the planes in the clutter file

  for (int clutterPlane = 0; clutterPlane < cfhdr.nz; clutterPlane++) {

    // find the relevant plane in the input file

    int dbzPlane = -1;
    double clutterVlevel = cvhdr.level[clutterPlane];
    for (int iz = 0; iz < dfhdr.nz; iz++) {
      double dbzVlevel = dvhdr.level[iz];
      if (fabs(clutterVlevel - dbzVlevel) < _params.vlevel_tolerance) {

	dbzPlane = iz;
	break;
      }
    } // iz

    if (dbzPlane < 0) {
      // no plane matches, ignore this one
      if (_params.debug) {
	cerr << "No plane matches clutter vlevel: " << clutterVlevel << endl;
      }
      continue;
    }

    // remove clutter for this plane

    _doRemoval(cfld, cfhdr,
	       dfld, dfhdr, &floatDbz,
	       iflds, ifhdrs,
	       clutterPlane, dbzPlane);

  } // clutterPlane
  
  // compress the fields

  for (size_t ii = 0; ii < mdvx.getNFields(); ii++) {
    mdvx.getField(ii)->requestCompression(_params.output_compression);
  }

  // append to the info in the master header

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  char info[2048];
  sprintf(info, "%s\n%s\n%s%g\n%s%g\n%s%g",
	  mhdr.data_set_info,
	  "Clutter removed using ClutterRemove.",
	  "clutter threshold: ", _params.clutter_threshold,
	  "dBZ threshold: ", _params.dbz_threshold,
	  "dBZ margin: ", _params.dbz_margin);
  mdvx.setDataSetInfo(info);
  
  // write out the file
  
  PMU_auto_register("Before write");
  mdvx.setWriteLdataInfo();
  if(mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - ClutterRemove::_processFile" << endl;
    cerr << "  Cannot write clutter-removed file, url:"
	 << _params.output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }

  return 0;

}

///////////////////
// do the removal

void ClutterRemove::_doRemoval(const MdvxField *cfld,
			       const Mdvx::field_header_t &cfhdr,
			       MdvxField *dfld,
			       const Mdvx::field_header_t &dfhdr,
			       const MdvxField *floatDbz,
			       vector<MdvxField *> &iflds,
			       vector<const Mdvx::field_header_t *> &ifhdrs,
			       int clutterPlane,
			       int dbzPlane)
  
{

  // loop though the points

  int nPointsPlane = dfhdr.nx * dfhdr.ny;
  fl32 *clut = (fl32 *) cfld->getVol() + clutterPlane * nPointsPlane;
  fl32 *dbz = (fl32 *) floatDbz->getVol() + dbzPlane * nPointsPlane;

  int nExceedClutterThreshold = 0;
  int nExceedDbzThreshold = 0;
  int nRemoved = 0;

  for (int ii = 0; ii < nPointsPlane; ii++) {

    // test for clutter

    if (clut[ii] < _params.clutter_threshold) {
      continue;
    }

    nExceedClutterThreshold++;

    if (dbz[ii] < _params.dbz_threshold) {
      continue;
    }

    nExceedDbzThreshold++;

    if (dbz[ii] > clut[ii] + _params.dbz_margin) {
      continue;
    }

    nRemoved++;

    // clutter found, remove from dbz field

    switch (dfhdr.encoding_type) {
    case Mdvx::ENCODING_INT8: {
      ui08 *val = (ui08 *) dfld->getVol() + dbzPlane * nPointsPlane + ii;
      *val = (ui08) dfhdr.missing_data_value;
    }
    break;
    case Mdvx::ENCODING_INT16: {
      ui16 *val = (ui16 *) dfld->getVol() + dbzPlane * nPointsPlane + ii;
      *val = (ui16) dfhdr.missing_data_value;
    }
    break;
    case Mdvx::ENCODING_FLOAT32: {
      fl32 *val = (fl32 *) dfld->getVol() + dbzPlane * nPointsPlane + ii;
      *val = dfhdr.missing_data_value;
    }
    break;
    } // switch

    // remove from other fields

    for (size_t jj = 0; jj < iflds.size(); jj++) {

      switch (ifhdrs[jj]->encoding_type) {
      case Mdvx::ENCODING_INT8: {
	ui08 *val =
	  (ui08 *) iflds[jj]->getVol() + dbzPlane * nPointsPlane + ii;
	*val = (ui08) ifhdrs[jj]->missing_data_value;
      }
      break;
      case Mdvx::ENCODING_INT16: {
	ui16 *val =
	  (ui16 *) iflds[jj]->getVol() + dbzPlane * nPointsPlane + ii;
	*val = (ui16) ifhdrs[jj]->missing_data_value;
      }
      break;
      case Mdvx::ENCODING_FLOAT32: {
	fl32 *val =
	  (fl32 *) iflds[jj]->getVol() + dbzPlane * nPointsPlane + ii;
	*val = ifhdrs[jj]->missing_data_value;
      }
      break;
      } // switch

    } // jj

  } // ii

  if (_params.debug) {
    cerr << "n points exceed clutter threshold: "
	 << nExceedClutterThreshold << endl;
    cerr << "n points exceed dBZ threshold: "
	 << nExceedDbzThreshold << endl;
    cerr << "n points removed: " << nRemoved << endl;
  }

}
  
/////////////////////////////////////
// set up the read for the input file
  
void ClutterRemove::_setupInputRead(DsMdvx &mdvx)
  
{
  
  mdvx.clearRead();
  mdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.specify_field_names) {
    mdvx.addReadField(_params.input_dbz_field_name);
    for (int i = 0; i < _params.field_names_n; i++) {
      if (strcmp(_params._field_names[i],
		 _params.input_dbz_field_name)) {
	mdvx.addReadField(_params._field_names[i]);
      }
    } // i
  }
  
}

/////////////////////////
// read the clutter file

int ClutterRemove::_readClutterFile()
  
{
  
  _clutter.clearRead();
  _clutter.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _clutter.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _clutter.addReadField(_params.clutter_dbz_field_name);

  if (_params.use_latest_clutter_file) {
    _clutter.setReadTime(Mdvx::READ_LAST,
			 _params.clutter_path_url);
  } else {
    _clutter.setReadPath(_params.clutter_path_url);
  }

  if (_params.remap_xy) {
    if (_params.remap_projection == Params::PROJ_LATLON) {
      _clutter.setReadRemapLatlon(_params.remap_grid.nx,
				  _params.remap_grid.ny,
				  _params.remap_grid.minx,
				  _params.remap_grid.miny,
				  _params.remap_grid.dx,
				  _params.remap_grid.dy);
    } else if (_params.remap_projection == Params::PROJ_LAMBERT_CONF) {
      _clutter.setReadRemapLc2(_params.remap_grid.nx,
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
      _clutter.setReadRemapFlat(_params.remap_grid.nx,
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

  if (_clutter.readVolume()) {
    cerr << "ERROR - ClutterRemove::_readClutterFile" << endl;
    cerr << "  Cannot read in clutter data." << endl;
    cerr << _clutter.getErrStr() << endl;
    return -1;
  }

  return 0;
  
}
