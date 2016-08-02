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

#include "InputFile.hh"
#include <toolsa/pmu.h>
#include <iostream>
#include <cmath>
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
  _isRequired = input_url.is_required;
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

  // check if we need range field - used by closest

  bool alreadyGotRange = false;
  for (size_t ii = 0; ii < _fieldNames.size(); ii++) {
    if (strcmp(_fieldNames[ii].c_str(), _params.range_field_name) == 0) {
      alreadyGotRange = true;
    }
  }
  if (!alreadyGotRange) {
    for (int ii = 0; ii < _params.merge_fields_n; ii++) {
      if (_params._merge_fields[ii].merge_method == Params::MERGE_CLOSEST) {
        _fieldNames.push_back(_params.range_field_name);
        break;
      }
    }
  }

  // check field list

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

  // create lookup tables
  
  for (int ii = 0; ii < _params.merge_fields_n; ii++) {
    XyLookup *table =
      new XyLookup(_params,
		   _params._merge_fields[ii].name,
		   _params._merge_fields[ii].merge_method,
		   _outputProj);
    _lookupTables.push_back(table);
  }
    
}

/////////////
// Destructor

InputFile::~InputFile()

{
  for (size_t ii = 0; ii < _lookupTables.size(); ii++) {
    delete _lookupTables[ii];
  }
}

//////////////////////////////
// read in file for given time
//
// returns 0 on success, -1 on failure

int InputFile::process(const time_t& requestTime,
		       int fcstLeadTime,
		       int nxyOut,
                       int nzOut,
		       vector<void *> merged,
		       vector<ui08 *> count,
		       vector<time_t *> latest_time,
                       fl32 *closestRange,
                       int *closestFlag,
		       Mdvx::master_header_t &mhdr,
		       vector<Mdvx::field_header_t> &fhdrs)
  
{

  DsMdvx in;                 // Mdvx input object
  
  // set up read
  
  in.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
  in.clearRead();
  
  // set time
  

  if(_params.trigger == Params::FCST_FILES_TRIGGER)
  {
    if (_params.debug) {
      cerr << "READ_SPECIFIED_FORECST\n";
    }
    in.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _url,
		   _params.time_trigger_margin, requestTime,
		   fcstLeadTime);
  }
  else
  {
    if(_params.past_only)
    {
      if (_params.debug) {
        cerr << "READ_FIRST_BEFORE\n";
      }
      in.setReadTime(Mdvx::READ_FIRST_BEFORE, _url,
		     _params.time_trigger_margin, requestTime);
    }
    else
    {
      if (_params.debug) {
        cerr << "READ_CLOSEST\n";
      }
      in.setReadTime(Mdvx::READ_CLOSEST, _url,
		     _params.time_trigger_margin, requestTime);
    }
  }
  
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
  
  in.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  in.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  // Apply vlevel limits, if requested.
  
  if (_params.vlevelLimits.applyVlevelLimits){
    in.setReadVlevelLimits(_params.vlevelLimits.vlevelMin,
			   _params.vlevelLimits.vlevelMax );
  }

  // Apply vertical plane number limits, if requested.
  
  if (_params.planeNumLimits.applyPlaneNumLimits){
    in.setReadPlaneNumLimits(_params.planeNumLimits.planeNumMin,
			     _params.planeNumLimits.planeNumMax );
  }



  // Apply horizontal remapping, if desired for this url.

  for (int ir=0; ir < _params.remap_input_grids_n; ir++){
    if (0==strcmp(_url.c_str(), _params._remap_input_grids[ir].url)){
      //
      // Yes, there is a remapping speficied for this URL.
      //
      bool remappingOK = false;

       if (0==strcmp("FLAT", _params._remap_input_grids[ir].gridType)){
	 remappingOK = true;
	 if (_params.debug >= Params::DEBUG_VERBOSE) {
	   cerr << "Remapping url " << _url << " to flat grid" << endl;
	 }
	 in.setReadRemapFlat(_params._remap_input_grids[ir].nx,
			     _params._remap_input_grids[ir].ny,
			     _params._remap_input_grids[ir].minx,
			     _params._remap_input_grids[ir].miny,
			     _params._remap_input_grids[ir].dx,
			     _params._remap_input_grids[ir].dy,
			     _params._remap_input_grids[ir].originLat,
			     _params._remap_input_grids[ir].originLon,
			     _params._remap_input_grids[ir].flatEarthRotation);
       }

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

       if (0==strcmp("LAMBERT", _params._remap_input_grids[ir].gridType)){
	 remappingOK = true;
	 if (_params.debug >= Params::DEBUG_VERBOSE) {
	   cerr << "Remapping url " << _url << " to lambert grid" << endl;
	 }
	 in.setReadRemapLc2(_params._remap_input_grids[ir].nx,
			    _params._remap_input_grids[ir].ny,
			    _params._remap_input_grids[ir].minx,
			    _params._remap_input_grids[ir].miny,
			    _params._remap_input_grids[ir].dx,
			    _params._remap_input_grids[ir].dy,
			    _params._remap_input_grids[ir].originLat,
			    _params._remap_input_grids[ir].originLon,
			    _params._remap_input_grids[ir].lambertTruelat1,
			    _params._remap_input_grids[ir].lambertTruelat2);
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

  PMU_force_register("Reading file");
  
  if (_params.debug) {
    cerr << "Reading data for URL: " << _url << endl;
    in.printReadRequest(cerr);
  }

  if (in.readVolume()) {
    if (_params.debug) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  Cannot read data for url: " << _url << endl;
      cerr << "  " << in.getErrStr() << endl;
    }
    return -1;
  }
  
  _pathInUse = in.getPathInUse();
  if (_params.debug) {
    cerr << "  Read data from file: " << _pathInUse << endl;
  }
  _timeStart = in.getMasterHeader().time_begin;
  _timeCentroid = in.getMasterHeader().time_centroid;
  _timeEnd = in.getMasterHeader().time_end;
  
  // load up field pointer vector, in the order as specified in the params
  
  vector<MdvxField *> fields;  // pointer to each field in returned data
  if (_fieldNames.size() > 0) {
    for (size_t ii = 0; ii < _fieldNames.size(); ii++) {
      MdvxField *fld = in.getField(_fieldNames[ii].c_str());
      if (fld == NULL) {
        cerr << "ERROR - InputFile::process" << endl;
        cerr << "  Cannot find field name: " << _fieldNames[ii] << endl;
        cerr << "  File: " << _pathInUse << endl;
        return -1;
      } else {
        fields.push_back(in.getField(_fieldNames[ii].c_str()));
      }
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

  for (int ii = 0; ii < _params.merge_fields_n; ii++) {
    if (_params._merge_fields[ii].merge_encoding == Params::INT8) {
      fields[ii]->convertType(Mdvx::ENCODING_INT8,
			      Mdvx::COMPRESSION_NONE,
			      Mdvx::SCALING_SPECIFIED,
			      _params._merge_fields[ii].merge_scale,
			      _params._merge_fields[ii].merge_bias);
    } else if (_params._merge_fields[ii].merge_encoding == Params::INT16) {
      fields[ii]->convertType(Mdvx::ENCODING_INT16,
			      Mdvx::COMPRESSION_NONE,
			      Mdvx::SCALING_SPECIFIED,
			      _params._merge_fields[ii].merge_scale,
			      _params._merge_fields[ii].merge_bias);
    }
  }
  
  // set the plane pointers
  
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

    // Check if this is a polar projection and exit if it is (we can't
    // cope with this and the results are bizzare, leading to much debugging).

    if ((fhdr.proj_type != Mdvx::PROJ_LATLON) &&
	(fhdr.proj_type != Mdvx::PROJ_LAMBERT_CONF) &&
	(fhdr.proj_type != Mdvx::PROJ_MERCATOR) &&
	(fhdr.proj_type != Mdvx::PROJ_POLAR_STEREO) &&
	(fhdr.proj_type != Mdvx::PROJ_CYL_EQUIDIST) &&
	(fhdr.proj_type != Mdvx::PROJ_FLAT) &&
	(fhdr.proj_type != Mdvx::PROJ_OBLIQUE_STEREO) &&
	(fhdr.proj_type != Mdvx::PROJ_TRANS_MERCATOR) &&
	(fhdr.proj_type != Mdvx::PROJ_ALBERS) &&
	(fhdr.proj_type != Mdvx::PROJ_LAMBERT_AZIM)) {
      cerr << "Unsupported projection. I cannot cope." << endl;
      return -1;
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
      
      while (inCoords.minx < outCoords.minx) {
	inCoords.minx += 360.0;
      }
      while (inCoords.minx >= outCoords.minx + 360.0) {
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

  // set the data time for each of the fields.  This is only needed
  // if using MERGE_LATEST but it doesn't hurt to set it for all
  // merge methods, just in case it's needed for something that is
  // added later.
  
  for (size_t ii = 0; ii < fields.size(); ii++) {
    Mdvx::field_header_t fhdr = fields[ii]->getFieldHeader();
    if (fhdr.forecast_time == 0)
    {
      fhdr.user_time1 = _timeCentroid;
    }
    else 
    {
      if (_params.fcst_data_time_type == Params::GENTIME)
	fhdr.user_time1 = fhdr.forecast_time - fhdr.forecast_delta;
      else
	fhdr.user_time1 = fhdr.forecast_time;
    }
    fields[ii]->setFieldHeader(fhdr);
  }
  
  // add to merged data set

  _addToMerged(fields, nxyOut, nzOut,
               merged, count, latest_time, closestRange, closestFlag);
  
  return 0;

}

//////////////////////////////
// add data to merged field

void InputFile::_addToMerged(const vector<MdvxField *> &fields,
			     int nxyOut,
                             int nzOut,
			     vector<void *> merged,
			     vector<ui08 *> count,
			     vector<time_t *> latest_time,
                             fl32 *closestRange,
                             int *closestFlag)
 
{

  // set up closest array, if needed

  if (closestRange != NULL) {
    _setClosestFlag(fields, nxyOut, nzOut, closestRange, closestFlag);
  }
  
  for (size_t ifld = 0; ifld < fields.size(); ifld++) {
    
    Mdvx::coord_t outCoord = _outputProj.getCoord();
    MdvxField *fld = fields[ifld];
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    const Mdvx::vlevel_header_t &vhdr = fld->getVlevelHeader();

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "--->> Merging field: " << fld->getFieldName() << endl;
    }
    
    // loop through input levels
    
    for (int iz = 0; iz < fhdr.nz; iz++) {

      // compute the output plane number
      
      int outPlaneNum = _computeOutputPlaneIndex(iz, outCoord, fhdr, vhdr);

      if (outPlaneNum >= 0) {
        
        // do the merge

	int offset = outPlaneNum * nxyOut;
	ui08 *cc = NULL;
	if (count[ifld] != NULL) {
	  cc = count[ifld] + offset;
	}
	int *cFlag = NULL;
	if (closestFlag != NULL) {
	  cFlag = closestFlag + offset;
	}

	time_t *time_ptr = NULL;
	if (latest_time[ifld] != NULL) {
	  time_ptr = latest_time[ifld] + offset;
	}

	if (fhdr.encoding_type == Mdvx::ENCODING_FLOAT32) {
	  fl32 *mm = ((fl32 *) merged[ifld]) + offset;
	  fl32 *plane = (fl32 *) fld->getPlane(iz);
	  _lookupTables[ifld]->merge(outPlaneNum,
                                     plane,
				     fhdr.missing_data_value,
				     fhdr.bad_data_value,
				     fhdr.user_time1,
				     mm, cc, time_ptr, cFlag);
	} else if (fhdr.encoding_type == Mdvx::ENCODING_INT16) {
	  ui16 *mm = ((ui16 *) merged[ifld]) + offset;
	  ui16 *plane = (ui16 *) fld->getPlane(iz);
	  _lookupTables[ifld]->merge(outPlaneNum,
                                     plane,
				     fhdr.missing_data_value,
				     fhdr.bad_data_value,
				     fhdr.scale, fhdr.bias,
				     fhdr.user_time1,
				     mm, cc, time_ptr, cFlag);
	} else { // INT8
	  ui08 *plane = (ui08 *) fld->getPlane(iz);
	  ui08 *mm = ((ui08 *) merged[ifld]) + offset;
	  _lookupTables[ifld]->merge(outPlaneNum,
                                     plane,
				     fhdr.missing_data_value,
				     fhdr.bad_data_value,
				     fhdr.scale, fhdr.bias,
				     fhdr.user_time1,
				     mm, cc, time_ptr, cFlag);
	}
	
      }
      
    } // iz

  } // ifld

}

////////////////////////////////////
// set closest array

void InputFile::_setClosestFlag(const vector<MdvxField *> &fields,
                                int nxyOut,
                                int nzOut,
                                fl32 *closestRange,
                                int *closestFlag)
  
{

  // initialize closest array to 0 - i.e. to use existing data
  
  for (int ii = 0; ii < nxyOut * nzOut; ii++) {
    closestFlag[ii] = 0;
  }
  
  // get range field in data file, if it exists
  
  MdvxField *rangeFld = NULL;
  size_t rangeIfld = 0;
  for (size_t ifld = 0; ifld < fields.size(); ifld++) {
    MdvxField *fld = fields[ifld];
    if (strcmp(fld->getFieldName(), _params.range_field_name) == 0) {
      rangeFld = fld;
      rangeIfld = ifld;
      break;
    }
  }
  if (rangeFld == NULL) {
    cerr << "WARNING - InputFile::_setClosestFlag" << endl;
    cerr << "  Cannot find range field in file: " << _pathInUse << endl;
    cerr << "  Closest cannot be performed" << endl;
    // no range field, cannot do this operation
    // use all points, override existing data
    for (int ii = 0; ii < nxyOut; ii++) {
      closestFlag[ii] = 1;
    }
    return;
  }

  Mdvx::coord_t outCoord = _outputProj.getCoord();
  const Mdvx::field_header_t &fhdr = rangeFld->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = rangeFld->getVlevelHeader();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "--->> Using range field for closest: " << rangeFld->getFieldName() << endl;
  }
    
  // loop through input levels
  
  for (int iz = 0; iz < fhdr.nz; iz++) {
    
    // compute the output plane number

    int outPlaneNum = _computeOutputPlaneIndex(iz, outCoord, fhdr, vhdr);

    if (outPlaneNum >= 0) {

      // set closest range and flag for this plane

      int offset = outPlaneNum * nxyOut;
      fl32 *inRange = (fl32 *) rangeFld->getPlane(iz);
      fl32 rangeMissing = fhdr.missing_data_value;
      fl32 *cRange = closestRange + offset;
      int *cflag = closestFlag + offset;

      _lookupTables[rangeIfld]->setClosestFlag(outPlaneNum,
                                               inRange, rangeMissing,
                                               cRange, cflag);

    }
      
  } // iz

}

/////////////////////////////////////////////
// compute the output plane number

int InputFile::_computeOutputPlaneIndex(int iz,
                                        const Mdvx::coord_t &outCoord,
                                        const Mdvx::field_header_t &fhdr,
                                        const Mdvx::vlevel_header_t &vhdr)

{
  
  int planeIndex = 0;
  
  if (outCoord.nz == 1 && fhdr.nz == 1) {
    
    // special case if only 1 plane for both input and output
    // in which case we use plane 0 for both
    
    planeIndex = 0;
    
  } else {
    
    double ht = vhdr.level[iz];
    
    if(_params.use_specified_vlevels) {
      // Use user-specified output vlevels (probably irregularly spaced)
      double delta_vlevel = fabs(ht - _params._vlevel_array[0]);
      int iht = 0;
      // Loop through 2nd through nz output vlevels
      for(int out_iz=1; out_iz<_params.vlevel_array_n; out_iz++) {
        if(fabs(_params._vlevel_array[out_iz] - ht) < delta_vlevel) {
          // vlevel difference for current output level is less than last
          delta_vlevel = fabs(_params._vlevel_array[out_iz] - ht);
          iht = out_iz;
        }
      }
      if (iht >= 0 && iht < _params.vlevel_array_n) {
        planeIndex = iht;
      } else {
        planeIndex = -1;
      }
    } else {
      // Use regularly-spaced output levels specified by minz, dz and nz
      int iht = (int) ((ht - outCoord.minz) / outCoord.dz + 0.5);
      if (iht >= 0 && iht < outCoord.nz) {
        planeIndex = iht;
      } else {
        planeIndex = -1;
      }
    }
    
  }

  return planeIndex;

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







