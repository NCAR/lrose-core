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
// $Id: MdvPatch.cc,v 1.4 2016/03/04 02:22:12 dixon Exp $
//
// MdvPatch
//
// Yan Chen, RAL, NCAR
//
// March 2008
//
///////////////////////////////////////////////////////////////
//
// MdvPatch finds bad and missing data for MDV files, replaces
// them with the data interpolated from the surrounding data.
// For instance, if a bad or missing data is found at point (i, j),
// it will be replaced with an average value from the surrounding
// points (i+1,j),(i-1,j),(i,j+1),(i,j-1).
//
////////////////////////////////////////////////////////////////

#include <string>
#include <fstream>
#include <sstream>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>
#include "MdvPatch.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

MdvPatch::MdvPatch(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "MdvPatch";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // parse the start and end time

  if (
    sscanf(
      _params.start_date_time, "%d %d %d %d %d %d",
      &_startTime.year, &_startTime.month, &_startTime.day,
      &_startTime.hour, &_startTime.min, &_startTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&_startTime);
  }

  if (
    sscanf(
      _params.end_date_time, "%d %d %d %d %d %d",
      &_endTime.year, &_endTime.month, &_endTime.day,
      &_endTime.hour, &_endTime.min, &_endTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&_endTime);
  }

  if (!isOK) {
    _args.usage(cerr);
    return;
  }

  // init process mapper registration
  
  PMU_auto_init(const_cast<char*>(_progName.c_str()), _params.instance, 
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

//////////////////////////
// destructor

MdvPatch::~MdvPatch()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvPatch::Run ()
{

  PMU_auto_register("MdvPatch::Run");

  bool fix_data = TRUE;

  if (_params.mode == Params::FIND)
    fix_data = FALSE;

  int ret = _performPatch(fix_data);

  return (ret);
}

//////////////////////////////////////////////////
// patch

int MdvPatch::_performPatch(bool fix_data) {

  if (_params.debug) {
    cerr << "Start: " << ctime(&(_startTime.unix_time));
    cerr << "End:   " << ctime(&(_endTime.unix_time));
  }

  DsMdvxTimes mdv_time_list;
  int iret = mdv_time_list.setArchive(
    _params.input_url_dir,
    _startTime.unix_time,
    _endTime.unix_time
  );
  if (iret) {
    cerr << "ERROR: MdvPatch::_performPatch()" << endl;
    cerr << "url: " << _params.input_url_dir << endl;
    cerr << mdv_time_list.getErrStr() << endl;
    return -1;
  }

  vector<time_t> _timelist = mdv_time_list.getArchiveList();
  int n_times = _timelist.size();
  if (n_times < 1)
    return -1;

  if (_params.debug) {
    cerr << "Total files: " << n_times << endl;
  }

  DateTime start(_startTime.unix_time);
  DateTime end(_endTime.unix_time);
  string logDir(_params.output_log_dir);
  string logPath;
  RapDataDir.fillPath(logDir, logPath);
  logDir = logPath;
  logPath += PATH_DELIM;
  logPath += start.getStrPlain();
  logPath += "-";
  logPath += end.getStrPlain();
  logPath += "_bad.log";

  if (_params.debug)
    cerr << "log path: " << logPath << endl;

  ofstream log;
  log.open(logPath.c_str(), ios::out);
  if (!log) {
    ta_makedir_recurse(logDir.c_str());
    log.open(logPath.c_str(), ios::out);
    if (!log) {
      cerr << "ERROR: MdvPatch::_performPatch()" << endl;
      cerr << "  Cannot create log file." << endl;
      return -1;
    }
  }

  for (int file_index = 0; file_index < n_times; file_index++) {

    time_t mdv_time = _timelist.at(file_index);

    DsMdvx inMdvx;
    inMdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
    inMdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
    inMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_url_dir,
      0,       // search margin
      mdv_time,// search time
      0        // forecast lead time
    );

    // read volume for all fields
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

    int nFields = 0;
    vector<string> fieldNames;

    if (_params.field_names_n > 0) {
      // if we have specified fields, check them
      nFields = _params.field_names_n;
      for (int i = 0; i < nFields; i++) {
        fieldNames.push_back(_params._field_names[i]);
      }

    } else {
      // otherwise, check all fields
      nFields = inMdvx.getNFields();
      for (int i = 0; i < nFields; i++) {
        fieldNames.push_back(inMdvx.getFieldName(i));
      }
    }

    int reported = FALSE;
    int have_fixed_data = FALSE;
    stringstream ss;

    // loop through all fields needed

    for (int field_index = 0; field_index < nFields; field_index++) {

      string field_name = fieldNames.at(field_index);

      MdvxField *inField = inMdvx.getFieldByName(field_name);

      const Mdvx::field_header_t &inFHdr = inField->getFieldHeader();

      // save important information for output purpose

      Mdvx::encoding_type_t encoding_type =
        (Mdvx::encoding_type_t)inFHdr.encoding_type;
      Mdvx::compression_type_t compression_type =
        (Mdvx::compression_type_t)inFHdr.compression_type;
      Mdvx::transform_type_t transform_type =
        (Mdvx::transform_type_t)inFHdr.transform_type;
      Mdvx::scaling_type_t scaling_type =
        (Mdvx::scaling_type_t)inFHdr.scaling_type;
      double scale = inFHdr.scale;
      double bias = inFHdr.bias;

      inField->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
      if (inFHdr.transform_type == Mdvx::DATA_TRANSFORM_LOG)
        inField->transform2Linear();
      inField->setPlanePtrs();

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

      Mdvx::vlevel_header_t vhdr = inField->getVlevelHeader();

      fl32 *in_data = NULL;

      for (int z = 0; z < nz; z++) {
        if (nz == 1) {
          in_data = (fl32 *) inField->getVol();
          has_vlevel = FALSE;

	} else {

          in_data = (fl32 *) inField->getPlane(z);
          has_vlevel = TRUE;
	}

        for (int y = 0; y < ny; y++) {

          for (int x = 0; x < nx; x++) {

            int i = x + (y * nx);
            if (
              in_data[i] != inFHdr.bad_data_value ||
              in_data[i] != inFHdr.missing_data_value
            ) {
              continue;
	    }

            double new_data = -9999;
            bool can_be_fixed = FALSE;
            if (fix_data) {
              double surrounding_data[4];
              if (x == 0) {
                surrounding_data[0] = inFHdr.bad_data_value;
	      } else {
                surrounding_data[0] = in_data[x-1 + (y * nx)];
	      }
              if (y == ny - 1) {
                surrounding_data[1] = inFHdr.bad_data_value;
	      } else {
                surrounding_data[1] = in_data[x + ((y+1) * nx)];
	      }
              if (x == nx - 1) {
                surrounding_data[2] = inFHdr.bad_data_value;
	      } else {
                surrounding_data[2] = in_data[x+1 + (y * nx)];
	      }
              if (y == 0) {
                surrounding_data[3] = inFHdr.bad_data_value;
	      } else {
                surrounding_data[3] = in_data[x + ((y-1) * nx)];
	      }
              int n_surroundings = 0;
              double sum = 0;
              for (int k = 0; k < 4; k++) {
                if (
                  surrounding_data[k] != inFHdr.bad_data_value ||
                  surrounding_data[k] != inFHdr.missing_data_value
                ) {
                  sum += surrounding_data[k];
                  n_surroundings++;
		}
              }
              if (n_surroundings > 0) {
                new_data = sum / n_surroundings;
                can_be_fixed = TRUE;
	      }
	    }

            double xLoc = minx + x * dx;
            double yLoc = miny + y * dy;

            if (xy_switched) {
              double tmp = xLoc;
              xLoc = yLoc;
              yLoc = tmp;
	    }
            if (!reported) {
              ss << "======================================" << endl;
              ss << "URL:  " << inMdvx.getPathInUse() << endl;
              ss << "======================================" << endl << endl;
              reported = TRUE;
            }

            ss << "Field Name:    " << field_name << endl;
            ss << "Bad value:     " << inFHdr.bad_data_value << " "
               << inFHdr.units << endl;
            ss << "Missing value: " << inFHdr.missing_data_value << " "
               << inFHdr.units << endl;
            ss << "Data value:    " << in_data[i] << " "
               << inFHdr.units << endl;
            if (fix_data) {
              if (can_be_fixed) {
                ss << "Replaced with: " << new_data << " "
                   << inFHdr.units << endl;
	      } else {
                ss << "Cannot be replaced." << endl;
	      }
	    }
            ss << "V level:       " << z << " - "
               << Mdvx::vertType2Str(vhdr.type[z]) << ", "
               << vhdr.level[z] << endl;
            ss << "Location(x, y): (" << xLoc << ", " << yLoc << ") "
               << xyunits << endl << endl;

            if (fix_data && can_be_fixed) {
              in_data[i] = new_data;
              have_fixed_data = TRUE;
	    }

          } // loop x

	} // loop y

        if (nz == 1)
          break;

      } // v levels

      if (fix_data) {

        // convert back to the type as the input

        inField->convertType(
          encoding_type,
          compression_type,
          scaling_type,
          scale,
          bias
        );
        if (transform_type == Mdvx::DATA_TRANSFORM_LOG)
          inField->transform2Log();
      }

    } // fields

    if (fix_data && have_fixed_data) {

      if (_params.debug) {
        cerr << "Fixed. Writing out..." << endl;
      }

      // write out

      if (inMdvx.writeToDir(_params.output_url_dir)) {
        cerr << "ERROR: MdvPatch::_performPatch()" << endl;
        cerr << "  Cannot write file, url: "
             << _params.output_url_dir << endl;
        cerr << inMdvx.getErrStr() << endl;
        return -1;
      }
    }

    log << ss.str();
    log.flush();
  } // files

  log << endl;
  log.close();

  return 0;
}

