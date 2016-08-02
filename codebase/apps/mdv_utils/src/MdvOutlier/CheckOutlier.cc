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
// $Id: CheckOutlier.cc,v 1.8 2016/03/04 02:22:12 dixon Exp $
//
// Check outliers
//
// Yan Chen, RAL, NCAR
//
// Jan. 2008
//
//////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>
#include <didss/RapDataDir.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <toolsa/pmu.h>
#include "CheckOutlier.hh"
using namespace std;

// constructor

CheckOutlier::CheckOutlier(
  const string &prog_name,
  const Params &params
) : _progName(prog_name), _params(params) {

  _start_time = 0;
  _end_time = 0;
  _maxPrecipLog = "";

  // get fields from params

  for (int i = 0; i < _params.fields_interested_n; i++) {
    map<string, set<int> >::iterator iter = _fieldVlevelsMap.find(
      _params._fields_interested[i].field_name
    );
    if (iter == _fieldVlevelsMap.end()) {
      set<int> vlevels;
      vlevels.insert(_params._fields_interested[i].vertical_level - 1);
      _fieldVlevelsMap.insert(
        make_pair(_params._fields_interested[i].field_name, vlevels)
      );

      _fieldSigmaMap.insert(
        make_pair(
          _params._fields_interested[i].field_name,
          _params._fields_interested[i].sigma_threshold
        )
      );
      _fieldSpaceMap.insert(
        make_pair(
         _params._fields_interested[i].field_name,
         _params._fields_interested[i].space_to_report_outlier
        )
      );

    } else {

      (iter->second).insert(_params._fields_interested[i].vertical_level - 1);
    }
  }

}

// destructor

CheckOutlier::~CheckOutlier()
{
  
}

//////////////////////////////////////////////////
// Check

int CheckOutlier::check() {

  // register with procmap
  PMU_auto_register("CheckOutlier::check");

  int iret = _checkFiles();
  if (iret)
    return -1;

  iret = _checkOutliers();
  if (iret)
    return -1;

  if (_params.find_max_precip) {
    iret = _calcMaxPrecip();
    if (iret)
      return -1;
  }

  iret = _writeLogFile();
  if (iret)
    return -1;

  return 0;
}

//////////////////////////////////////////////////
// Check the files

int CheckOutlier::_checkFiles() {

  // register with procmap
  PMU_auto_register("CheckOutlier::_checkFiles");

  stddevMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  stddevMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  meanMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  meanMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  bool read_std_success = true;
  bool read_mean_success = true;

  if (_params.debug) {
    cerr << "Reading Std Dev file..." << endl;
  }

  if (stddevMdvx.readVolume()) {
    cerr << "ERROR: readVolume() " << endl;
    cerr << stddevMdvx.getErrStr() << endl;
    read_std_success = false;
  }

  if (_params.debug)
    cerr << "Reading Mean file..." << endl;

  if (meanMdvx.readVolume()) {
    cerr << "ERROR: readVolume() " << endl;
    cerr << meanMdvx.getErrStr() << endl;
    read_mean_success = false;
  }

  if (!read_std_success || !read_mean_success) {
    return -1;
  }

  Mdvx::master_header_t std_master_hdr = stddevMdvx.getMasterHeader();
  Mdvx::master_header_t mean_master_hdr = meanMdvx.getMasterHeader();

  // check if two files are stats files we want

  if (std_master_hdr.data_collection_type != Mdvx::DATA_CLIMO_ANA) {
    cerr << "ERROR: Not a statistics file: " << endl;
    cerr << "  " << stddevMdvx.getPathInUse() << endl;
    cerr << "Checking outliers failed." << endl;
    return -1;
  }
  if (mean_master_hdr.data_collection_type != Mdvx::DATA_CLIMO_ANA) {
    cerr << "ERROR: Not a statistics file: " << endl;
    cerr << "  " << meanMdvx.getPathInUse() << endl;
    cerr << "Checking outliers failed." << endl;
    return -1;
  }

  // check if two stat files match

  string err_str = "";
  if (!_files_match(std_master_hdr, mean_master_hdr, err_str)) {
    cerr << "ERROR: Files do not match: " << endl;
    cerr << stddevMdvx.getPathInUse() << endl;
    cerr << meanMdvx.getPathInUse() << endl;
    cerr << err_str << endl;
    cerr << "Checking outliers failed." << endl;
    return -1;
  }

  // check if both files have the fields we want

  map<string, set<int> >::iterator pos;
  for (pos = _fieldVlevelsMap.begin(); pos != _fieldVlevelsMap.end(); pos++) {
    MdvxField *stdField = stddevMdvx.getFieldByName(pos->first);
    if (stdField == NULL) {
      cerr << "ERROR: Field: " << pos->first << endl;
      cerr << "  Could not be found in file: " << endl;
      cerr << "  " << stddevMdvx.getPathInUse() << endl;
      cerr << "Checking outliers failed." << endl;
      return -1;
    }

    MdvxField *meanField = meanMdvx.getFieldByName(pos->first);
    if (meanField == NULL) {
      cerr << "ERROR: Field: " << pos->first << endl;
      cerr << "  Could not be found in file: " << endl;
      cerr << "  " << meanMdvx.getPathInUse() << endl;
      cerr << "Checking outliers failed." << endl;
      return -1;
    }

    // if both found, check to see if they are the fields we expect

    Mdvx::field_header_t stdFHdr = stdField->getFieldHeader();
    if (stdFHdr.transform_type != Mdvx::DATA_TRANSFORM_STDDEV) {
      cerr << "ERROR: Field: " << stdField->getFieldName() << endl;
      cerr << "  in file: " << stddevMdvx.getPathInUse() << endl;
      cerr << "  is not a standard deviation field." << endl;
      cerr << "Checking outliers failed." << endl;
      return -1;
    }
    Mdvx::field_header_t meanFHdr = meanField->getFieldHeader();
    if (meanFHdr.transform_type != Mdvx::DATA_TRANSFORM_MEAN) {
      cerr << "ERROR: Field: " << meanField->getFieldName() << endl;
      cerr << " in file: " << meanMdvx.getPathInUse() << endl;
      cerr << " is not a mean field." << endl;
      cerr << "Checking outliers failed." << endl;
      return -1;
    }

    // if both found, check to see if they match

    err_str = "";
    if (!_fields_match(stdField, meanField, err_str)) {
      cerr << "ERROR: Fields do not match: " << stdField->getFieldName()
           << endl;
      cerr << err_str << endl;
      cerr << "Checking outliers failed." << endl;
      return -1;
    }
  }

  // check if we have all MDV files available

  DsMdvxTimes mdv_time_list;
  int iret = mdv_time_list.setArchive(
    _params.input_url_dir,
    _start_time,
    _end_time
  );
  if (iret) {
    cerr << "ERROR: CheckOutliers::check()" << endl;
    cerr << "url: " << _params.input_url_dir << endl;
    cerr << mdv_time_list.getErrStr() << endl;
    return -1;
  }

  _timelist = mdv_time_list.getArchiveList();
  int n_times = _timelist.size();
  if (n_times != std_master_hdr.n_chunks) {
    cerr << "ERROR: File numbers do not match." << endl;
    cerr << n_times << " files found in: " << _params.input_url_dir << endl;
    cerr << std_master_hdr.n_chunks << " files were calculated towards "
         << "standard deviation and mean." << endl;
    return -1;
  }

  return 0;
}

//////////////////////////////////////////////////
// Check the outliers

int CheckOutlier::_checkOutliers() {

  // register with procmap
  PMU_auto_register("CheckOutlier::_checkOutliers");

  // all match, check outliers

  // loop through all mdv files

  int n_times = _timelist.size();
  if (_params.debug)
    cerr << "Checking " << n_times << " files..." << endl;

  int file_to_start = 0;
  if (
    _params.num_of_first_files_to_ignore > 0 &&
    _params.num_of_first_files_to_ignore < n_times
  ) {
    file_to_start = _params.num_of_first_files_to_ignore;
  }

  for (int file_index = file_to_start; file_index < n_times; file_index++) {

    time_t mdv_time = _timelist.at(file_index);

    DsMdvx inMdvx;
    inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
    inMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_url_dir,
      0,       // search margin
      mdv_time,// search time
      0        // forecast lead time
    );
    inMdvx.setReadNoChunks();

    // set up fields to be read in

    map<string, set<int> >::iterator pos;

    for (
      pos = _fieldVlevelsMap.begin();
      pos != _fieldVlevelsMap.end();
      pos++
    ) {
      inMdvx.addReadField(pos->first);
    }

    // read volume for all fields needed
    if (inMdvx.readVolume()) {
      cerr << "ERROR: readVolume()" << endl;
      cerr << inMdvx.getErrStr() << endl;
      continue; // continue to next file
    }

    if (_params.debug) {
      cerr << "Checking file: " << inMdvx.getPathInUse() << endl;
    }

    Mdvx::master_header_t inMHdr = inMdvx.getMasterHeader();
    Mdvx::grid_order_indices_t data_order =
      (Mdvx::grid_order_indices_t) inMHdr.data_ordering;

    time_t start_time = inMHdr.time_begin;
    time_t end_time = inMHdr.time_end;

    vector<LogRecord> logRecords;

    // loop through all fields

    for (
      pos = _fieldVlevelsMap.begin();
      pos != _fieldVlevelsMap.end();
      pos++
    ) {

      string field_name(pos->first);

      MdvxField *inField = inMdvx.getFieldByName(field_name);
      MdvxField *stdField = stddevMdvx.getFieldByName(field_name);
      MdvxField *meanField = meanMdvx.getFieldByName(field_name);

      // all should not be NULL, since we have checked before

      Mdvx::field_header_t inFHdr = inField->getFieldHeader();
      Mdvx::field_header_t stdFHdr =stdField->getFieldHeader();
      Mdvx::field_header_t meanFHdr = meanField->getFieldHeader();

      if (inFHdr.transform_type == Mdvx::DATA_TRANSFORM_LOG)
        inField->transform2Linear();
      inField->setPlanePtrs();

      if (stdFHdr.transform_type == Mdvx::DATA_TRANSFORM_LOG)
        stdField->transform2Linear();
      stdField->setPlanePtrs();

      if (meanFHdr.transform_type == Mdvx::DATA_TRANSFORM_LOG)
        meanField->transform2Linear();
      meanField->setPlanePtrs();

      MdvxProj proj(inFHdr);
      Mdvx::coord_t coord = proj.getCoord();
      string xyunits(coord.unitsx);

      int nx, ny;
      double minx, miny;
      double dx, dy;
      bool xy_switched = FALSE;

      if (
        data_order == Mdvx::ORDER_XYZ ||
        data_order == Mdvx::ORDER_XZY ||
        data_order == Mdvx::ORDER_ZXY
      ) {
        nx = inFHdr.nx;
        ny = inFHdr.ny;
        minx = inFHdr.grid_minx;
        miny = inFHdr.grid_miny;
        dx = inFHdr.grid_dx;
        dy = inFHdr.grid_dy;

      } else {
        nx = inFHdr.ny;
        ny = inFHdr.nx;
        minx = inFHdr.grid_miny;
        miny = inFHdr.grid_minx;
        dx = inFHdr.grid_dy;
        dy = inFHdr.grid_dx;

        xy_switched = TRUE;

      }

      int nz = inFHdr.nz;

      bool has_vlevel;
      int v_level = 0;

      Mdvx::vlevel_header_t vhdr = inField->getVlevelHeader();

      map<string, double>::iterator sigma_iter =
        _fieldSigmaMap.find(field_name);
      double sigma_threshold = sigma_iter->second;

      map<string, double>::iterator space_iter =
        _fieldSpaceMap.find(field_name);
      double space_to_report = space_iter->second;

      int nx_to_skip = (int) round(space_to_report / dx);
      int ny_to_skip = (int) round(space_to_report / dy);

      set<int> vlevels = pos->second;
      set<int>::iterator iter;

      fl32 *in_data = NULL;
      fl32 *std_data = NULL;
      fl32 *mean_data = NULL;

      for (iter = vlevels.begin(); iter != vlevels.end(); iter++) {
        if (nz == 1) {
          in_data = (fl32 *) inField->getVol();
          std_data = (fl32 *) stdField->getVol();
          mean_data = (fl32 *) meanField->getVol();
          has_vlevel = FALSE;

	} else {

          if (*iter < nz) {
            in_data = (fl32 *) inField->getPlane(*iter);
            std_data = (fl32 *) stdField->getPlane(*iter);
            mean_data = (fl32 *) meanField->getPlane(*iter);
            v_level = *iter;
	  } else {
            break;
	  }

          has_vlevel = TRUE;
	}

        int last_reported_x = -1;
        int last_reported_y = -1;

        for (int y = 0; y < ny; y++) {

          for (int x = 0; x < nx; x++) {

	    /* This is not quite right, but at least we do not
	     * need to report every single outlier.
	     */
            if (last_reported_x >= 0) {
              if (
                (x - last_reported_x) < nx_to_skip &&
                (y - last_reported_y) < ny_to_skip
              ) {

		// in the nearby of last reported location, skip it
                continue;
	      }
	    }

            int i = x + (y * nx);
            if (
              in_data[i] == inFHdr.bad_data_value ||
              in_data[i] == inFHdr.missing_data_value ||
              std_data[i] == stdFHdr.bad_data_value ||
              std_data[i] == stdFHdr.missing_data_value ||
              mean_data[i] == meanFHdr.bad_data_value ||
              mean_data[i] == meanFHdr.missing_data_value
            ) {
              continue;
	    }

            double inData;
            if (_params.use_absolute_values) {
              inData = fabs(in_data[i]);
	    } else {
              inData = in_data[i];
	    }

            double upper_limit = mean_data[i] +
              sigma_threshold * fabs(std_data[i]);
            double lower_limit = mean_data[i] -
              sigma_threshold * fabs(std_data[i]);

            if (inData > upper_limit || inData < lower_limit) {

              // outlier, report it

              LogRecord logRec(field_name, string(inFHdr.units), xyunits);

              logRec.vol_value = inData;
              logRec.mean = mean_data[i];
              logRec.std_dev = std_data[i];
              logRec.has_v_levels = has_vlevel;
              logRec.v_level = v_level;
              logRec.v_type = Mdvx::vertType2Str(vhdr.type[v_level]);
              logRec.v_value = vhdr.level[v_level];

              double xLoc = minx + x * dx;
              double yLoc = miny + y * dy;

              if (xy_switched) {
                logRec.x_location = yLoc;
                logRec.y_location = xLoc;
	      } else {
                logRec.x_location = xLoc;
                logRec.y_location = yLoc;
	      }

              logRecords.push_back(logRec);

              last_reported_x = x;
              last_reported_y = y;

	    }

          } // loop x

	} // loop y

        if (nz == 1)
          break;

      } // v levels

    } // fields

    if (logRecords.size() > 0) {

      if (_params.debug) {
        cerr << "Found " << logRecords.size() << " outliers." << endl;
      }

      // put into file records

      LogFile log_file(mdv_time, start_time, end_time, inMdvx.getPathInUse());
      log_file.logRecords = logRecords;

      _logFiles.push_back(log_file);

    } else {

      if (_params.debug) {
        cerr << "No outliers found." << endl;
      }
    }

  } // files

  return 0;
}

//////////////////////////////////////////////////////
// calculate max average hourly precipitation

int CheckOutlier::_calcMaxPrecip() {

  // register with procmap
  PMU_auto_register("CheckOutlier::_calcMaxPrecip");

  if (_params.debug)
    cerr << "Calculating max average hourly precipitation..." << endl;

  string field_name("hrain_total");

  DsMdvx inMdvx; // mean results
  inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  inMdvx.setReadPath(_params.mean_path);
  inMdvx.setReadNoChunks();

  inMdvx.addReadField(field_name);

  // read volume for all fields needed
  if (inMdvx.readVolume()) {
    cerr << "ERROR: readVolume()" << endl;
    cerr << inMdvx.getErrStr() << endl;
    return -1;
  }

  Mdvx::master_header_t inMHdr = inMdvx.getMasterHeader();
  Mdvx::grid_order_indices_t data_order =
    (Mdvx::grid_order_indices_t) inMHdr.data_ordering;

  time_t start_time = inMHdr.time_begin;
  time_t end_time = inMHdr.time_end;

  MdvxField *rainField = inMdvx.getFieldByName(field_name);
  rainField->transform2Linear();
  rainField->setPlanePtrs();

  Mdvx::field_header_t rainFHdr = rainField->getFieldHeader();

  MdvxProj proj(rainFHdr);
  Mdvx::coord_t coord = proj.getCoord();
  string xyunits(coord.unitsx);

  int nx, ny;
  double minx, miny;
  double dx, dy;
  bool xy_switched = FALSE;

  if (
    data_order == Mdvx::ORDER_XYZ ||
    data_order == Mdvx::ORDER_XZY ||
    data_order == Mdvx::ORDER_ZXY
  ) {
    nx = rainFHdr.nx;
    ny = rainFHdr.ny;
    minx = rainFHdr.grid_minx;
    miny = rainFHdr.grid_miny;
    dx = rainFHdr.grid_dx;
    dy = rainFHdr.grid_dy;

  } else {
    nx = rainFHdr.ny;
    ny = rainFHdr.nx;
    minx = rainFHdr.grid_miny;
    miny = rainFHdr.grid_minx;
    dx = rainFHdr.grid_dy;
    dy = rainFHdr.grid_dx;

    xy_switched = TRUE;
  }

  fl32 *rain_data = NULL;
  if (rainFHdr.nz == 1) {
    rain_data = (fl32 *) rainField->getVol();
  } else {
    rain_data = (fl32 *) rainField->getPlane(0);
  }

  double max = -1.0e99;
  int max_xIndex = -1;
  int max_yIndex = -1;

  for (int y = 0; y < ny; y++) {
    for (int x = 0; x < nx; x++) {
      fl32 rain_val = rain_data[x + y * nx];
      if (
        rain_val != rainFHdr.bad_data_value &&
        rain_val != rainFHdr.missing_data_value
      ) {
        if (rain_val > max) {
          max = rain_val;
          max_xIndex = x;
          max_yIndex = y;
	}
      }
    }
  }

  if (max_xIndex != -1 && max_yIndex != -1) {
    double xLoc = minx + max_xIndex * dx;
    double yLoc = miny + max_yIndex * dy;
    if (xy_switched) {
      double tmp = xLoc;
      xLoc = yLoc;
      yLoc = tmp;
    }

    stringstream ss;
    ss << "======================================" << endl;
    ss << "Average Hourly Precipitation" << endl;
    ss << "From: " << ctime(&start_time);
    ss << "To:   " << ctime(&end_time);
    ss << "URL:  " << inMdvx.getPathInUse() << endl;
    ss << "======================================" << endl;
    ss << "Field Name: " << field_name << endl;
    ss << "Max Value:  " << max << " " << rainFHdr.units << endl;
    ss << "Location(x, y): (" << xLoc << ", " << yLoc << ") "
       << xyunits << endl << endl;

    _maxPrecipLog = ss.str();
  }

  if (max_xIndex != -1 && max_yIndex != -1 && _params.write_hrain_values) {

    stringstream ss;

    int n_times = _timelist.size();
    for (int file_index = 0; file_index < n_times; file_index++) {

      time_t mdv_time = _timelist.at(file_index);

      DsMdvx iMdvx;
      iMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
      iMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
      iMdvx.setReadTime(
        Mdvx::READ_CLOSEST,
        _params.input_url_dir,
        0,       // search margin
        mdv_time,// search time
        0        // forecast lead time
      );
      iMdvx.setReadNoChunks();

      iMdvx.addReadField(field_name);

      if (iMdvx.readVolume()) {
        cerr << "ERROR: readVolume()" << endl;
        cerr << iMdvx.getErrStr() << endl;
        continue;
      }

      Mdvx::master_header_t iMHdr = iMdvx.getMasterHeader();
      Mdvx::grid_order_indices_t data_ordering =
        (Mdvx::grid_order_indices_t) iMHdr.data_ordering;

      MdvxField *iField = iMdvx.getFieldByName(field_name);
      iField->transform2Linear();
      iField->setPlanePtrs();

      Mdvx::field_header_t iFHdr = iField->getFieldHeader();
      if (
        data_ordering == Mdvx::ORDER_XYZ ||
        data_ordering == Mdvx::ORDER_XZY ||
        data_ordering == Mdvx::ORDER_ZXY
      ) {
        nx = iFHdr.nx;
      } else {
        nx = iFHdr.ny;
      }

      fl32 *i_data = NULL;
      if (iFHdr.nz == 1) {
        i_data = (fl32 *) iField->getVol();
      } else {
        i_data = (fl32 *) iField->getPlane(0);
      }
      fl32 i_val = i_data[max_xIndex + max_yIndex * nx];

      ss << iMdvx.getPathInUse() << "  "
         << i_val << " " << iFHdr.units << endl;
    }

    ss << endl;
    _precipValuesLog = ss.str();
  }

  return 0;
}

//////////////////////////////////////////////////////
// check if master headers from two files match or not

bool CheckOutlier::_files_match(
  Mdvx::master_header_t &mhdr1,
  Mdvx::master_header_t &mhdr2,
  string &err_str
) {

  bool iret = true;

  if (mhdr1.time_begin != mhdr2.time_begin) {
    stringstream ss;
    ss << "Begin times do not match:" << endl;
    ss << "  " << _timeStr(mhdr1.time_begin) << endl;
    ss << "  " << _timeStr(mhdr2.time_begin) << endl << endl;
    err_str += ss.str();

    iret = false;

  } else {

    _start_time = mhdr1.time_begin;
  }

  if (mhdr1.time_end != mhdr2.time_end) {
    stringstream ss;
    ss << "End times do not match:" << endl;
    ss << "  " << _timeStr(mhdr1.time_end) << endl;
    ss << "  " << _timeStr(mhdr2.time_end) << endl << endl;
    err_str += ss.str();

    iret = false;

  } else {

    _end_time = mhdr1.time_end;
  }

  if (mhdr1.n_chunks != mhdr2.n_chunks) {
    stringstream ss;
    ss << "Two stat files have different number of chunks: ";
    ss << mhdr1.n_chunks << "," << mhdr2.n_chunks << endl;
    ss << "This means the two stat files calculate over different "
       << "number of files." << endl << endl;
    err_str += ss.str();

    iret = false;
  }

  return(iret);
}

//////////////////////////////////////////////////////
// check if field headers from two files match or not

bool CheckOutlier::_fields_match(
  MdvxField *field1,
  MdvxField *field2,
  string &err_str
) {

  bool iret = true;

  const Mdvx::field_header_t &fhdr1 = field1->getFieldHeader();
  const Mdvx::field_header_t &fhdr2 = field2->getFieldHeader();

  MdvxProj proj1(fhdr1);
  MdvxProj proj2(fhdr2);
  if (proj1 != proj2) {
    err_str += "Field projections do not match:\n";
    err_str += fhdr1.field_name_long;
    err_str += ":\n";
    ostringstream oss;
    proj2.print(oss);
    err_str += oss.str();
    err_str += fhdr2.field_name_long;
    err_str += ":\n";
    oss.flush();
    proj2.print(oss);
    err_str += oss.str();

    iret = false;
  }

  if (fhdr1.vlevel_type != fhdr2.vlevel_type) {
    stringstream ss;
    ss << "Field vlevel types do not match:" << endl;
    ss << fhdr1.field_name_long << endl;
    ss << "   vert_type = "
       << Mdvx::vertType2Str(fhdr1.vlevel_type) << endl;
    ss << fhdr2.field_name_long << endl;
    ss << "   vert_type = "
       << Mdvx::vertType2Str(fhdr2.vlevel_type) << endl;
    err_str += ss.str();

    iret = false;
  }

  // Only check the vertical levels if they aren't constant

  if (!fhdr1.dz_constant || !fhdr2.dz_constant) {
    const Mdvx::vlevel_header_t &vlevel_hdr1 = field1->getVlevelHeader();
    const Mdvx::vlevel_header_t &vlevel_hdr2 = field2->getVlevelHeader();

    for (int z = 0; z < fhdr1.nz; ++z) {
      // Make sure all of the v levels match.
      //  I'm assuming that if the type is set to 0,
      // then it really wasn't set at all and the check should be ignored.

      if (
        vlevel_hdr1.type[z] != 0 &&
        vlevel_hdr2.type[z] != 0 &&
        vlevel_hdr1.type[z] != vlevel_hdr2.type[z] ||
        vlevel_hdr1.level[z] != vlevel_hdr2.level[z]
      ) {
        stringstream ss;
        ss << "Field vertical levels do not match:" << endl;
        ss << fhdr1.field_name_long << endl;
        ss << "   type[" << z << "] = "
           << Mdvx::vertType2Str(vlevel_hdr1.type[z]) << endl;
        ss << "   level[" << z << "] = " << vlevel_hdr1.level[z] << endl;
        ss << fhdr2.field_name_long << endl;
        ss << "   type[" << z << "] = "
           << Mdvx::vertType2Str(vlevel_hdr2.type[z]) << endl;
        ss << "   level[" << z << "] = " << vlevel_hdr2.level[z] << endl;
        err_str += ss.str();

        iret = false;
      }
    } /* endfor - z */
  }

  return(iret);
}


/////////////////////////////
// string for rendering time

char *CheckOutlier::_timeStr(const time_t ttime)

{
  if (ttime == -1 || ttime == 0) {
    return "not set";
  } else {
    return (utimstr(ttime));
  }
}

//////////////////////////////////////////////////
// Output

int CheckOutlier::_writeLogFile() {

  // get the path of the log file
  DateTime end(_end_time);
  string logDir(_params.output_log_dir);
  string logPath;
  RapDataDir.fillPath(logDir, logPath);
  logDir = logPath;
  logPath += PATH_DELIM;
  logPath += end.getDateStrPlain();
  logPath += "-";
  logPath += end.getTimeStrPlain();
  logPath += ".log";

  if (_params.debug)
    cerr << "log path: " << logPath << endl;

  ofstream log;
  log.open(logPath.c_str(), ios::out);
  if (!log) {
    ta_makedir_recurse(logDir.c_str());
    log.open(logPath.c_str(), ios::out);
    if (!log) {
      cerr << "ERROR: CheckOutlier::_writeLogFile()" << endl;
      cerr << "  Cannot create log file." << endl;
      return -1;
    }
  }

  size_t size = _logFiles.size();
  for (size_t i = 0; i < size; i++) {
    LogFile file = _logFiles.at(i);
    size_t rec_size = file.logRecords.size();

    log << "======================================" << endl;
    log << "URL:  " << file.mdv_url << endl;
    log << "Time: " << ctime(&(file.mdv_time));
    log << "Total outliers: " << rec_size << endl;
    log << "======================================" << endl << endl;

    for (size_t rec_index = 0; rec_index < rec_size; rec_index++) {
      LogRecord rec = file.logRecords.at(rec_index);

      log << "Field Name: " << rec.field_name << endl;
      if (_params.use_absolute_values) {
        log << "Abs. Value: " << rec.vol_value << " "
            << rec.field_units << endl;
      } else {
        log << "Value:      " << rec.vol_value << " "
            << rec.field_units << endl;
      }
      log << "Mean:       " << rec.mean << " " << rec.field_units << endl;
      log << "Std Dev:    " << rec.std_dev << " " << rec.field_units << endl;
      log << "V level:    " << rec.v_level + 1 << " - " << rec.v_type
          << ", " << rec.v_value << endl;
      log << "Location(x, y): (" << rec.x_location << ", "
          << rec.y_location << ") " << rec.xy_units << endl << endl;
    }
  }

  if (_params.find_max_precip) {

    if (!_params.write_hrain_values) {

      log << _maxPrecipLog;

    } else {

      string rainLogDir(_params.rain_log_dir);
      string rainLogPath;
      RapDataDir.fillPath(rainLogDir, rainLogPath);
      rainLogDir = rainLogPath;
      rainLogPath += PATH_DELIM;
      rainLogPath += "hrain";
      rainLogPath += "_";
      rainLogPath += end.getDateStrPlain();
      rainLogPath += ".log";

      if (_params.debug)
        cerr << "rain path: " << rainLogPath << endl;

      ofstream rainLog;
      rainLog.open(rainLogPath.c_str(), ios::out);
      if (!rainLog) {
        ta_makedir_recurse(rainLogDir.c_str());
        rainLog.open(rainLogPath.c_str(), ios::out);
        if (!rainLog) {
          cerr << "ERROR: CheckOutlier::_writeLogFile()" << endl;
          cerr << "  Cannot create rain log file." << endl;
          return -1;
	}
      }

      rainLog << _maxPrecipLog << endl;
      rainLog << _precipValuesLog << endl;

      rainLog.close();
    }
  }

  log.close();

  // write the event list

  if (_params.write_event_list) {

    string eventDir(_params.event_list_dir);
    string eventPath;
    RapDataDir.fillPath(eventDir, eventPath);
    eventDir = eventPath;
    eventPath += PATH_DELIM;
    eventPath += end.getDateStrPlain();
    eventPath += "-";
    eventPath += end.getTimeStrPlain();
    eventPath += ".event";

    if (_params.debug)
      cerr << "event path: " << eventPath << endl << endl;

    ofstream event;
    event.open(eventPath.c_str(), ios::out);
    if (!event) {
      ta_makedir_recurse(eventDir.c_str());
      event.open(eventPath.c_str(), ios::out);
      if (!event) {
        cerr << "ERROR: CheckOutlier::_writeLogFile()" << endl;
        cerr << "  Cannot create event file." << endl;
        return -1;
      }
    }

    for (size_t i = 0; i < size; i++) {
      LogFile file = _logFiles.at(i);

      size_t rec_size = file.logRecords.size();

      struct tm e_gmt;
      struct tm s_gmt;

      char part1_buf[128];
      char part2_buf[128];

      gmtime_r(&file.start_time,&s_gmt);
      gmtime_r(&file.end_time,&e_gmt);

      event << "###############################################################"
            << endl;
      event << "# Title:" << ctime(&(file.mdv_time)) << endl;
      event << "# Notes: " << rec_size << " outliers found."
            << "# Only the first outlier of each field at each level is reported here."
            << "# See log file for more details:"
            << "# " << logPath << "#";

      string last_field_name = "";
      int last_v_level = 0;

      for (size_t rec_index = 0; rec_index < rec_size; rec_index++) {
        LogRecord rec = file.logRecords.at(rec_index);

        if (
          rec.field_name.compare(last_field_name) == 0 &&
          rec.v_level == last_v_level
        )
          continue;

        event << "# Field Name: " << rec.field_name;
        if (_params.use_absolute_values) {
          event << "# Abs. Value: " << rec.vol_value << " "
                << rec.field_units;
        } else {
          event << "# Value:      " << rec.vol_value << " "
                << rec.field_units;
        }
        event << "# Mean:       " << rec.mean << " " << rec.field_units
              << "# Std Dev:    " << rec.std_dev << " " << rec.field_units
              << "# V level:    " << rec.v_level + 1 << " - " << rec.v_type
	      << ", " << rec.v_value
	      << "# Location(x,y): (" << rec.x_location << ", "
              << rec.y_location << ") " << rec.xy_units
              << "#";

        last_field_name = rec.field_name;
        last_v_level = rec.v_level;
      }
      event << "#" << endl;

      strftime(part1_buf,128,"start %Y/%m/%d %T ",&s_gmt);
      strftime(part2_buf,128,"end %Y/%m/%d %T",&e_gmt);
      event << part1_buf << "    " << part2_buf << endl;
    }

    event.close();
  }

  return 0;
}



