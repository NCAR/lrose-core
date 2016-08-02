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
//////////////////////////////////////////////////////////
// Input file handling
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
//////////////////////////////////////////////////////////
//
// This module reads in files from the relevant input
// directory, if necessary computes the lookup table to
// convert this file to the output grid, and loads up the
// output grid with data from the file.
//
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <iostream>
#include <cmath>

#include "InputFile.hh"

using namespace std;

//////////////
// Constructor

InputFile::InputFile(const string &prog_name,
		     const Params &params,
		     Params::input_url_t &input_url,
		     const MdvxProj &output_proj) :
  _progName(prog_name),
  _params(params),
  _outputProj(output_proj)

{

  _url = input_url.url;
  _OK = true;

  // parse the field names / numbers

  vector<string> toks;
  _tokenize(input_url.field_names, ",", toks);

  if (toks.size() == 0) {

    // no fields specified for this url, use main field names

    for (int ii = 0; ii < _params.merge_fields_n; ii++) {
      _fieldNames.push_back(_params._merge_fields[ii].name);
    }

  } else {

    // use specified field names or numbers

    if ((int) toks.size() != _params.merge_fields_n) {
      cerr << "ERROR - MdvMerge2::InputFile constructor" << endl;
      cerr << "  Incorrect number of fields for url: " << _url << endl;
      cerr << "  Expecting nfields: " << _params.merge_fields_n << endl;
      cerr << "  Found     nfields: " << toks.size() << endl;
      _OK = false;
      return;
    }

    // do we have field numbers or names?

    bool useNumbers = true;
    for (size_t ii = 0; ii < toks.size(); ii++) {
      for (size_t jj = 0; jj < toks[ii].size(); jj++) {
	if (!isdigit(toks[ii][jj])) {
	  useNumbers = false;
	  break;
	}
      }
    }
    
    // load vectors
    
    if (useNumbers) {
      for (size_t ii = 0; ii < toks.size(); ii++) {
	int num = 0;
	sscanf(toks[ii].c_str(), "%d", &num);
	_fieldNums.push_back(num);
      }
    } else {
      for (size_t ii = 0; ii < toks.size(); ii++) {
	_fieldNames.push_back(toks[ii]);
      }
    }

  } // if (toks.size() == 0)

  if (_fieldNames.size() == 0 && _fieldNums.size() == 0) {
    cerr << "ERROR - MdvMerge2::InputFile constructor" << endl;
    cerr << "  No fields specified" << endl;
    _OK = false;
    return;
  }

  if (_params.debug) {
    cerr << "Creating InputFile for url: " << _url << endl;
    if (_fieldNames.size() > 0) {
      cerr << "  Field names:";
      for (size_t ii = 0; ii < _fieldNames.size(); ii++) {
	cerr << " " << _fieldNames[ii];
      }
      cerr << endl;
    } else {
      cerr << "  Field numbers:";
      for (size_t ii = 0; ii < _fieldNums.size(); ii++) {
	cerr << " " << _fieldNums[ii];
      }
      cerr << endl;
    }
  } // debug

}

/////////////
// Destructor

InputFile::~InputFile()

{

}

//////////////////////////////
// read in file for given time
//
// returns 0 on success, -1 on failure

int InputFile::process(const time_t& requestTime,
		       int nxyOut,
		       vector<void *> &merged,
		       vector<ui08 *> &count,
		       Mdvx::master_header_t &mhdr,
		       vector<Mdvx::field_header_t> &fhdrs)
  
{

  DsMdvx in;                 // Mdvx input object
  
  // set up read
  
  in.setDebug(_params.debug);
  in.clearRead();
  
  // set time
  
  in.setReadTime(Mdvx::READ_CLOSEST, _url,
		 _params.time_trigger_margin, requestTime);
  
  // set fields
  
  if (_fieldNames.size() > 0) {
    for (size_t ii = 0; ii < _fieldNames.size(); ii++) {
      in.addReadField(_fieldNames[ii]);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Reading field name: " << _fieldNames[ii] << endl;
      }
    }
  } else {
    for (size_t ii = 0; ii < _fieldNums.size(); ii++) {
      in.addReadField(_fieldNums[ii]);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Reading field num: " << _fieldNums[ii] << endl;
      }
    }
  }
  
  //in.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  in.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading data for URL: " << _url << endl;
    in.printReadRequest(cerr);
  }

  // Apply vlevel limits, if requested.
  
  //if (_params.vlevelLimits.applyVlevelLimits){
  //in.setReadVlevelLimits(_params.vlevelLimits.vlevelMin,
  //		   _params.vlevelLimits.vlevelMax );
  //}

  // Apply horizontal remapping, if desired for this url.

  for (int ir=0; ir < _params.remap_input_grids_n; ir++){
    if (0==strcmp(_url.c_str(), _params._remap_input_grids[ir].url)){
      //
      // Yes, there is a remapping speficied for this URL.
      //
      bool remappingOK = false;

       if (0==strcmp("LATLON", _params._remap_input_grids[ir].gridType)){
	 remappingOK = true;
	 if (_params.debug >= Params::DEBUG_VERBOSE) {
	   cerr << "Remapping url " << _url << " to latlon grid" << endl;
	 }
	 in.setReadRemapLatlon(_params._remap_input_grids[ir].nx,
			       _params._remap_input_grids[ir].ny,
			       _params._remap_input_grids[ir].minx,
			       _params._remap_input_grids[ir].miny,
			       _params._remap_input_grids[ir].dx,
			       _params._remap_input_grids[ir].dy);
       }

       if (!(remappingOK)){
	 cerr << "ERROR - " <<  _progName << endl;
	 cerr << "Unrecognised grid type to remap to on input : ";
	 cerr << _params._remap_input_grids[ir].gridType;
	 cerr << " for url " << _url << endl;
	 return -1;
       }
       break;
    }
  }


  // perform the read
  
  if (in.readVolume()) {
    cerr << "ERROR - " <<  _progName << endl;
    cerr << "  Cannot read data for url: " << _url << endl;
    cerr << "  " << in.getErrStr() << endl;
    return -1;
  }
  
  _pathInUse = in.getPathInUse();
  if (_params.debug) {
    cerr << "Got data from file: " << _pathInUse << endl;
  }
  _timeStart = in.getMasterHeader().time_begin;
  _timeCentroid = in.getMasterHeader().time_centroid;
  _timeEnd = in.getMasterHeader().time_end;
  
  // load up field pointer vector, in the order as specified in the params
  
  vector<MdvxField *> fields;  // pointer to each field in returned data
  if (_fieldNames.size() > 0) {
    for (size_t ii = 0; ii < _fieldNames.size(); ii++) {
      fields.push_back(in.getField(_fieldNames[ii].c_str()));
    }
  } else {
    for (size_t ii = 0; ii < _fieldNums.size(); ii++) {
      fields.push_back(in.getField(_fieldNums[ii]));
    }
  }

  // set headers
  
  mhdr = in.getMasterHeader();
  fhdrs.clear();
  for (size_t ii = 0; ii < fields.size(); ii++) {
    fhdrs.push_back(fields[ii]->getFieldHeader());
  }
  
  // convert field encoding type as required

  //for (int ii = 0; ii < _params.merge_fields_n; ii++) {
  //if (_params._merge_fields[ii].merge_encoding == Params::INT8) {
  //  fields[ii]->convertType(Mdvx::ENCODING_INT8,
  //		      Mdvx::COMPRESSION_NONE,
  //		      Mdvx::SCALING_SPECIFIED,
  //		      _params._merge_fields[ii].merge_scale,
  //		      _params._merge_fields[ii].merge_bias);
  //} else if (_params._merge_fields[ii].merge_encoding == Params::INT16) {
  //  fields[ii]->convertType(Mdvx::ENCODING_INT16,
  //		      Mdvx::COMPRESSION_NONE,
  //		      Mdvx::SCALING_SPECIFIED,
  //		      _params._merge_fields[ii].merge_scale,
  //		      _params._merge_fields[ii].merge_bias);
  //}
  //}
  
  // set the plane pointers
  /*
  for (size_t ii = 0; ii < fields.size(); ii++) {
    fields[ii]->setPlanePtrs();
  }
  
  // check that the lookup tables are set up for each field
  // we use the first lookup table as the master.
  // The lookup table will only be computed if the grid geometry of
  // the field differs from the master.
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    
    // create input projection
    
    const Mdvx::master_header_t &mhdr = in.getMasterHeader();
    const Mdvx::field_header_t &fhdr = fields[ii]->getFieldHeader();

    //
    // Check if this is a polar projection and exit if it is (we can't
    // cope with this and the results are bizzare, leading to much debugging).
    // Niles.
    //
    if (
	(fhdr.proj_type != Mdvx::PROJ_LATLON) &&
	(fhdr.proj_type != Mdvx::PROJ_FLAT) &&
	(fhdr.proj_type != Mdvx::PROJ_LAMBERT_CONF)
	){
      cerr << "Unsupported projection. I cannot cope." << endl;
      exit(-1);
    }

    MdvxProj inputProj(mhdr, fhdr);

    // If this is a lat/lon projection, normalize the longitude value
    // of the input grid to the longitude range indicated by the output
    // grid specifications.  This is done to handle data where the output
    // grid includes the point where longitude changes from east to west
    // (180E == 180W).  We need to do this before comparing the new grid
    // with the old grid since we save and operate with the normalized
    // grid.
    
    if (_params.output_projection == Params::OUTPUT_PROJ_LATLON &&
	fhdr.proj_type == Mdvx::PROJ_LATLON) {
      
      Mdvx::coord_t inCoords = inputProj.getCoord();
      Mdvx::coord_t outCoords = _outputProj.getCoord();
      
      while (inCoords.minx < outCoords.minx - 360) {
	inCoords.minx += 360.0;
      }
      while (inCoords.minx > outCoords.minx + 360.0) {
        inCoords.minx -= 360.0;
      }

      inputProj.init(inCoords);
      
    }

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "----->> Input projection:" << endl;
      inputProj.print(cerr);
      cerr << "----->> Output projection:" << endl;
      _outputProj.print(cerr);
      cerr << "----------------------------------------------" << endl;
    }
    
    // check lookup table is valid
    
    _lookupTables[ii]->check(inputProj, _lookupTables[0]);
    
  }
  */

  if(merged.size() == 0)
    _initOutputGrids(fhdrs, merged);

  // add to merged data set

  _addToMerged(fields, nxyOut, merged, count);
  
  return 0;

}

//////////////////////////////
// add data to merged field

void InputFile::_addToMerged(const vector<MdvxField *> &fields,
			     int nxyOut,
			     vector<void *> &merged,
			     vector<ui08 *> &count)
  
{

  for (size_t ifld = 0; ifld < fields.size(); ifld++) {
    
    Mdvx::coord_t outCoord = _outputProj.getCoord();

    MdvxField *fld = fields[ifld];
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = fld->getVlevelHeader();

    if(fhdr.nz != outCoord.nz || fabs(fhdr.grid_dz - outCoord.dz) > .0001 ||
       fabs(fhdr.grid_dx - outCoord.dx) > .0001 || fabs(fhdr.grid_dy - outCoord.dy) > .0001) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  Input url  " << _url << ", Field  " << fld->getFieldName() <<
	" does not have same spacing (dx,dy,dz) or does not have" << endl;
      cerr << " same number of vertical levels (nz). Cannot stitch. " << endl;
      continue;
    }
    if(fhdr.encoding_type != _params._merge_fields[ifld].output_encoding) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  Input url  " << _url << ", Field  " << fld->getFieldName() <<
	" does not have expected encoding type" << endl;
      continue;
    }

    const void * vol = fld->getVol();

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--->> Merging field: " << fld->getFieldName() << endl;
    }
    
    int xDiff = (int)(((fhdr.grid_minx - outCoord.minx) / fhdr.grid_dx) + .5);
    int yDiff = (int)(((fhdr.grid_miny - outCoord.miny) / fhdr.grid_dy) + .5);

    if(xDiff > outCoord.nx || yDiff > outCoord.ny)
      return;

    int yStart = 0;
    if(yDiff < 0) {
      yStart = 0 - yDiff;
      yDiff = 0;
    }

    int numCopy = fhdr.nx;

    if(xDiff + fhdr.nx >= outCoord.nx) {
      numCopy = outCoord.nx - xDiff;
    }

    if(xDiff < 0) {
      numCopy -= 0 - xDiff;
      xDiff = 0;
    }

    if(numCopy < 0)
      return;

    for (int iz = 0; iz < fhdr.nz; iz++) {
      for (int iy = yStart; iy < fhdr.ny; iy++) {

	if(iy + yDiff > outCoord.ny)
	  break;

	long int inputOffset = (iz * fhdr.nx * fhdr.ny) + (iy * fhdr.nx);

	long int outputOffset = (iz * outCoord.nx * outCoord.ny) + ((iy + yDiff) * outCoord.nx) + xDiff;

	if (fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {

	  memcpy( (void *)((fl32 *)merged[ifld] + outputOffset), (void *)((fl32 *)vol + inputOffset), 
		  numCopy * sizeof(fl32));

	} else if (fhdr.encoding_type == Mdvx::ENCODING_INT16) {

	  memcpy( (void *)((ui16 *)merged[ifld] + outputOffset), (void *)((ui16 *)vol + inputOffset), 
		  numCopy * sizeof(ui16));

	} else { // INT8

	  memcpy( (void *)((ui08 *)merged[ifld] + outputOffset), (void *)((ui08 *)vol + inputOffset), 
		  numCopy * sizeof(ui08));

	}
	
      } // iy 
    } // iz

  } // ifld

}



//////////////////////////////////////////////////
// initialize the output grids

void InputFile::_initOutputGrids(vector<Mdvx::field_header_t> &fhdrs, vector<void *> &merged)
{
  
  // alloc gridded arrays
  Mdvx::coord_t outCoord = _outputProj.getCoord();

  int nxy = outCoord.nx * outCoord.ny;
  int nxyz = nxy * outCoord.nz;

  for (int ii = 0; ii < fhdrs.size(); ii++) {
    
    ui08 *ui08Merged = NULL;
    ui16 *ui16Merged = NULL;
    fl32 *fl32Merged = NULL;
    int jj = 0;

    switch ( _params._merge_fields[ii].output_encoding ){

    case Mdvx::ENCODING_FLOAT32 :

      if(fhdrs[ii].missing_data_value == 0.0)
	fl32Merged = (fl32 *)calloc(nxyz, sizeof(fl32));
      else
	fl32Merged = (fl32 *)malloc(nxyz * sizeof(fl32));
      if (fl32Merged == NULL){
	cerr << "Malloc fl32 failed! size : " << nxyz << endl;
	cerr << "Field number " << ii+1 << endl;
	exit(-1);
      }
      if(fhdrs[ii].missing_data_value != 0.0)
	for (jj = 0; jj < nxyz; jj++)
	  fl32Merged[jj] = fhdrs[ii].missing_data_value;
      
      merged.push_back((void *) fl32Merged);
      break;

    case Mdvx::ENCODING_INT16 :

      if(fhdrs[ii].missing_data_value == 0)
	ui16Merged = (ui16 *)calloc(nxyz, sizeof(ui16));
      else
	ui16Merged = (ui16 *)malloc(nxyz * sizeof(ui16));
      if (ui16Merged == NULL){
	cerr << "Malloc ui16 failed! size : " << nxyz << endl;
	cerr << "Field number " << ii+1 << endl;
	exit(-1);
      }

      if(fhdrs[ii].missing_data_value != 0.0) {
	//ui16 miss_value = (fhdrs[ii].missing_data_value * fhdrs[ii].scale) + fhdrs[ii].bias;
	for (jj = 0; jj < nxyz; jj++)
	  ui16Merged[jj] = (ui16)fhdrs[ii].missing_data_value;
      }
  
      merged.push_back((void *) ui16Merged);
      break;

    default :
      if(fhdrs[ii].missing_data_value == 0)
	ui08Merged = (ui08 *)calloc(nxyz, sizeof(ui08));
      else
	ui08Merged = (ui08 *)malloc(nxyz * sizeof(ui08));
      if (ui08Merged == NULL){
	cerr << "Malloc ui08 failed! size : " << nxyz << endl;
	cerr << "Field number " << ii+1 << endl;
	exit(-1);
      }
      if(fhdrs[ii].missing_data_value != 0.0) {
	//ui08 miss_value = (fhdrs[ii].missing_data_value * fhdrs[ii].scale) + fhdrs[ii].bias;
	for (jj = 0; jj < nxyz; jj++)
	  ui16Merged[jj] = fhdrs[ii].missing_data_value;
      }

      merged.push_back((void *) ui08Merged);
      break;
    }

  } // ii
  return;
}


//////////////////////////////////////////////
// tokenize a string into a vector of strings

void InputFile::_tokenize(const string &str,
			  const string &spacer,
			  vector<string> &toks)
  
{
    
  toks.clear();
  size_t pos = 0;
  bool done = false;
  while (!done) {
    size_t start = str.find_first_not_of(spacer, pos);
    size_t end = str.find_first_of(spacer, start);
    if (start == string::npos) {
      done = true;
    } else {
      string tok;
      tok.assign(str, start, end - start);
      toks.push_back(tok);
    }
    pos = end;
  }
}







