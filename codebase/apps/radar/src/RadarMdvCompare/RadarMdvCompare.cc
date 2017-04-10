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
// RadarMdvCompare object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////
//
// RadarMdvCompare compares data from 2 radars
//
////////////////////////////////////////////////////////////////

#include <vector>
#include <algorithm>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/Path.hh>
#include <toolsa/TaXml.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxField.hh>
#include <Spdb/DsSpdb.hh>
#include "RadarMdvCompare.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

RadarMdvCompare::RadarMdvCompare(int argc, char **argv)

{

  isOK = true;
  _statsFile = NULL;
  _tableFile = NULL;
  _summaryFile = NULL;

  // set programe name

  _progName = "RadarMdvCompare";
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

  // check params
  
  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  In ARCHIVE mode you must specify start and end times" << endl;
      cerr << "  Run '" << _progName << " -h' for usage" << endl;
      _args.usage(cerr);
      isOK = false;
    }
  }

  if (!isOK) {
    return;
  }
  
  // create trigger

  _createTrigger();

  // init process mapper registration
  
  PMU_auto_init(const_cast<char*>(_progName.c_str()),
		_params.instance, 
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

//////////////////////////
// destructor

RadarMdvCompare::~RadarMdvCompare()

{

  if (_statsFile) {
    fclose(_statsFile);
  }
  if (_tableFile) {
    fclose(_tableFile);
  }
  if (_summaryFile) {
    fclose(_summaryFile);
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadarMdvCompare::Run ()
{

  // create the directory for the output files, if needed

  if (_params.write_output_stats ||
      _params.write_output_table ||
      _params.append_to_summary_file) {
    if (ta_makedir_recurse(_params.output_dir)) {
      int errNum = errno;
      cerr << "ERROR - RadarMdvCompare::Run";
      cerr << "  Cannot create output dir: "
           << _params.output_dir << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  int iret = 0;
  PMU_auto_register("RadarMdvCompare::Run");
 
  // loop through times
  
  time_t triggerTime;
  while ((triggerTime = _trigger->next()) >= 0) {
    
    if (_params.debug) {
      cerr << "----> Trigger time: " << utimstr(triggerTime) << endl;
    }

    // In REALTIME mode, sleep after triggering, if desired.
    
    if (_params.mode == Params::REALTIME){
      
      if ((_params.sleep_after_trigger) && (_params.debug)) {
	cerr << " Sleeping for " << _params.sleep_after_trigger;
	cerr << " seconds before processing" << endl;
      }
      
      for (int iSleep=0; iSleep < _params.sleep_after_trigger; iSleep++){
	PMU_auto_register("Sleeping before processing data");
	umsleep(1000);
      }
      
    }

    // read in primary and secondary files

    if (_readPrimary(triggerTime) == 0) {
      
      _primaryTime = _primary.getMasterHeader().time_centroid;

      if (_openOutputFiles()) {
        return -1;
      }

      if (_readSecondary(_primaryTime) == 0) {
	_secondaryTime = _secondary.getMasterHeader().time_centroid;
	_compareFiles();
      }
      
    }

    if (_args.runOnce) {
      break;
    }

  } // while

  return iret;

}

//////////////////////////////////////////////////
//  create trigger

void RadarMdvCompare::_createTrigger()

{

  if (_params.mode == Params::REALTIME) {

    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new RealtimeTimeTrigger(_progName, _params);
    } else {
      _trigger = new RealtimeFileTrigger(_progName, _params);
    }

  } else {

    if (_params.trigger == Params::TIME_TRIGGER) {
      _trigger = new ArchiveTimeTrigger(_progName, _params,
				       _args.startTime,  _args.endTime);
    } else {
      _trigger = new ArchiveFileTrigger(_progName, _params,
				       _args.startTime, _args.endTime);
    }
  }
  
}

//////////////////////////////////////
// read in primary file for given time
//
// returns 0 on success, -1 on failure

int RadarMdvCompare::_readPrimary(time_t requestTime)
  
{
  
  // set up read
  
  _primary.clearRead();
  _primary.setDebug(_params.debug >= Params::DEBUG_EXTRA);
  
  // set time
  
  _primary.setReadTime(Mdvx::READ_FIRST_BEFORE, _params.primary_url,
		       _params.time_trigger_margin, requestTime);
  
  // set field
  
  _primary.addReadField(_params.primary_field_name);
  
  // floats, no compression
  
  _primary.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _primary.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Reading data for URL: " << _params.primary_url << endl;
    _primary.printReadRequest(cerr);
  }
  
  // Apply vlevel limits, if requested.
  
  if (_params.set_vlevel_limits) {
    _primary.setReadVlevelLimits(_params.lower_vlevel,
				 _params.upper_vlevel);
  }

  if (_params.request_composite) {
    _primary.setReadComposite();
  }
  
  // apply bounding box if requested
  
  if (_params.set_bounding_box) {
    _primary.setReadHorizLimits(_params.bounding_box.min_lat,
				_params.bounding_box.min_lon,
				_params.bounding_box.max_lat,
				_params.bounding_box.max_lon);
  }

  // perform the read
  
  if (_primary.readVolume()) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  Cannot read data for primary url: "
           << _params.primary_url << endl;
      cerr << "  " << _primary.getErrStr() << endl;
    }
    return -1;
  }
  
  const string &pathInUse = _primary.getPathInUse();
  if (_params.debug) {
    cerr << "Read primary file: " << pathInUse << endl;
  }
  
  return 0;

}

//////////////////////////////////////
// read in secondary file for given time
//
// returns 0 on success, -1 on failure

int RadarMdvCompare::_readSecondary(time_t requestTime)
  
{
  
  // set up read
  
  _secondary.clearRead();
  _secondary.setDebug(_params.debug >= Params::DEBUG_EXTRA);
  
  // set time
  
  _secondary.setReadTime(Mdvx::READ_CLOSEST, _params.secondary_url,
			 _params.max_time_diff, requestTime);
  
  // set field
  
  _secondary.addReadField(_params.secondary_field_name);
  
  // floats, no compression
  
  _secondary.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _secondary.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Reading data for URL: " << _params.secondary_url << endl;
    _secondary.printReadRequest(cerr);
  }
  
  // Apply vlevel limits, if requested.
  
  if (_params.set_vlevel_limits) {
    _secondary.setReadVlevelLimits(_params.lower_vlevel,
				 _params.upper_vlevel);
  }
  
  if (_params.request_composite) {
    _secondary.setReadComposite();
  }
  
  // apply bounding box if requested
  
  if (_params.set_bounding_box) {
    _secondary.setReadHorizLimits(_params.bounding_box.min_lat,
				_params.bounding_box.min_lon,
				_params.bounding_box.max_lat,
				_params.bounding_box.max_lon);
  }

  // perform the read
  
  if (_secondary.readVolume()) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "ERROR - " <<  _progName << endl;
      cerr << "  Cannot read data for secondary url: "
           << _params.secondary_url << endl;
      cerr << "  " << _secondary.getErrStr() << endl;
    }
    return -1;
  }
  
  const string &pathInUse = _secondary.getPathInUse();
  if (_params.debug) {
    cerr << "Read secondary file: " << pathInUse << endl;
  }
  
  return 0;

}

//////////////////////////////////////
// compare primary and secondary files
//
// returns 0 on success, -1 on failure

int RadarMdvCompare::_compareFiles()

{

  // get fields

  const Mdvx::master_header_t &primaryMhdr = _primary.getMasterHeader();
  MdvxField *primaryFld = _primary.getField(_params.primary_field_name);
  if (primaryFld == NULL) {
    cerr << "ERROR - RadarMdvCompare::_compareFiles" << endl;
    cerr << "  Primary field missing: "
	 << _params.primary_field_name << endl;
    return -1;
  }
  const Mdvx::field_header_t &primaryFhdr = primaryFld->getFieldHeader();
  const Mdvx::vlevel_header_t &primaryVhdr = primaryFld->getVlevelHeader();
  const fl32 *primaryData = (fl32 *) primaryFld->getVol();
  int primaryNptsPlane = primaryFhdr.nx * primaryFhdr.ny;
  fl32 primaryMissing = primaryFhdr.missing_data_value;

  const Mdvx::master_header_t &secondaryMhdr = _secondary.getMasterHeader();
  MdvxField *secondaryFld = _secondary.getField(_params.secondary_field_name);
  if (secondaryFld == NULL) {
    cerr << "ERROR - RadarMdvCompare::_compareFiles" << endl;
    cerr << "  Secondary field missing: "
	 << _params.secondary_field_name << endl;
    return -1;
  }
  const Mdvx::field_header_t &secondaryFhdr = secondaryFld->getFieldHeader();
  const Mdvx::vlevel_header_t &secondaryVhdr = secondaryFld->getVlevelHeader();
  const fl32 *secondaryData = (fl32 *) secondaryFld->getVol();
  int secondaryNptsPlane = secondaryFhdr.nx * secondaryFhdr.ny;
  fl32 secondaryMissing = secondaryFhdr.missing_data_value;

  // set up vector to match the vert levels

  vector<int> vlevelMatch;
  if (!_params.request_composite) {
    for (int iz = 0; iz < primaryFhdr.nz; iz++) {
      double primaryVlevel = primaryVhdr.level[iz];
      int match = -999;
      for (int jz = 0; jz < primaryFhdr.nz; jz++) {
	double secondaryVlevel = secondaryVhdr.level[jz];
	double diff = primaryVlevel - secondaryVlevel; 
	if (fabs(diff) < _params.max_vlevel_diff) {
	  match = jz;
	}
      } // jz
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Vlevel match, iz, jz: " << iz << ", " << match << endl;
      }
      vlevelMatch.push_back(match);
    } // iz
  } else {
    // if composite is set, there is only one plane
    vlevelMatch.push_back(0);
  }

  // get projections

  MdvxProj primaryProj(primaryMhdr, primaryFhdr);
  MdvxProj secondaryProj(secondaryMhdr, secondaryFhdr);

  // get lat/lon of origins

  double primaryOriginLat, primaryOriginLon;
  primaryProj.xy2latlon(0.0, 0.0, primaryOriginLat, primaryOriginLon);

  double secondaryOriginLat, secondaryOriginLon;
  secondaryProj.xy2latlon(0.0, 0.0, secondaryOriginLat, secondaryOriginLon);

  // loop through the (x,y) points in the primary data
  // accumulate difference data

  vector<double> diffs;
  double nptsAboveThreshold = 0.0;
  
  for (int ix = 0; ix < primaryFhdr.nx; ix++) {
    
    for (int iy = 0; iy < primaryFhdr.ny; iy++) {

      // get lat/lon of this point
      
      double xx = primaryFhdr.grid_minx + ix * primaryFhdr.grid_dx;
      double yy = primaryFhdr.grid_miny + iy * primaryFhdr.grid_dy;
      double lat, lon;
      primaryProj.xy2latlon(xx, yy, lat, lon);

      // is this on the secondary grid?

      int secondaryIx, secondaryIy;
      if (secondaryProj.latlon2xyIndex(lat, lon, secondaryIx, secondaryIy)) {
	// data outside grid
	continue;
      }

      // check range if required

      if (_params.set_range_limits) {

	double primaryRangeKm, primaryTheta;
	PJGLatLon2RTheta(lat, lon,
			 primaryOriginLat, primaryOriginLon,
			 &primaryRangeKm, &primaryTheta);
	
	if (primaryRangeKm < _params.min_range_km || 
	    primaryRangeKm > _params.max_range_km) {
	  continue;
	}

	double secondaryRangeKm, secondaryTheta;
	PJGLatLon2RTheta(lat, lon,
			 secondaryOriginLat, secondaryOriginLon,
			 &secondaryRangeKm, &secondaryTheta);
	
	if (secondaryRangeKm < _params.min_range_km || 
	    secondaryRangeKm > _params.max_range_km) {
	  continue;
	}

      } // if (_params.set_range_limit)

      // range is OK
      // loop through vlevels

      for (int iz = 0; iz < primaryFhdr.nz; iz++) {
	
	// check for match in secondary
	
	int jz = vlevelMatch[iz];
	if (jz < 0) {
	  continue;
	}

	// get primary and secondary values

	int primaryIndex = (iz * primaryNptsPlane) +
	  (iy * primaryFhdr.nx) + ix;

	int secondaryIndex = (jz * secondaryNptsPlane) +
	  (secondaryIy * secondaryFhdr.nx) + secondaryIx;

	fl32 primaryVal = primaryData[primaryIndex];
	fl32 secondaryVal = secondaryData[secondaryIndex];
	
 	if (primaryVal != primaryMissing &&
	    secondaryVal != secondaryMissing &&
	    primaryVal >= _params.min_value_for_diff &&
	    secondaryVal >= _params.min_value_for_diff) {
	  
	  double diff = primaryVal - secondaryVal;

          if (diff >= _params.min_valid_diff &&
              diff <= _params.max_valid_diff) {
            
            if (_tableFile) {
              fprintf(_tableFile,
                      "%10.4f %10.4f %10.4f %10.4f %10.4f %10.4f\n",
                      lat, lon, primaryVhdr.level[iz],
                      primaryVal, secondaryVal, diff);
            }

            diffs.push_back(diff);
            
            if (primaryVal >= _params.threshold_value_for_min_pts &&
                secondaryVal >= _params.threshold_value_for_min_pts) {
              nptsAboveThreshold++;
            }
	  }

	}
	  
      }
      
    } // iy

  } // ix

  // compute stats on diffs

  if (_params.debug) {
    cerr << "nptsAboveThreshold: " << nptsAboveThreshold << endl;
  }
  
  if (nptsAboveThreshold >= _params.min_number_of_pts_above_threshold) {
    _computeDiffStats(diffs);
  } else {
    if (_params.debug) {
      cerr << "NOTE - too few points above threshold, not computing stats" 
	   << endl;
    }
  }

  return 0;

}
  
//////////////////////////////////////
// compare primary and secondary files

void RadarMdvCompare::_computeDiffStats(vector<double> &diffs)

{
  
  double count = 0.0;
  double mean = -9999;
  double sdev = -9999;
  double median = -9999;
  
  // sort the vector, compute the median
  
  sort(diffs.begin(), diffs.end());
  median = diffs[diffs.size() / 2];

  // compute mean and stdev

  double sum = 0.0;
  double sumsq = 0.0;
  for (int ii = 0; ii < (int) diffs.size(); ii++) {
    count++;
    double diff = diffs[ii];
    sum += diff;
    sumsq += diff * diff;
  }

  if (count > 0) {
    mean = sum / count;
  }
  
  if (count > 2) {
    double var = (sumsq - (sum * sum) / count) / (count - 1.0);
    if (var >= 0.0) {
      sdev = sqrt(var);
    } else {
      sdev = 0.0;
    }
  }

  if (_statsFile) {
    fprintf(_statsFile, "============= RadarMdvCompare ============\n");
    fprintf(_statsFile, "PRIMARY: \n");
    fprintf(_statsFile, "  time: %s\n", DateTime::strm(_primaryTime).c_str());
    fprintf(_statsFile, "  url: %s\n", _params.primary_url);
    fprintf(_statsFile, "  field name: %s\n", _params.primary_field_name);
    fprintf(_statsFile, "SECONDARY:\n");
    fprintf(_statsFile, "  time: %s\n", DateTime::strm(_secondaryTime).c_str());
    fprintf(_statsFile, "  radar url: %s\n", _params.secondary_url);
    fprintf(_statsFile, "  field name: %s\n", _params.secondary_field_name);
    fprintf(_statsFile, "RESULTS:\n");
    fprintf(_statsFile, "  Point count: %g\n", count);
    fprintf(_statsFile, "  Mean of diffs: %g\n", mean);
    fprintf(_statsFile, "  Sdev of diffs: %g\n", sdev);
    if (median > -9990) {
      fprintf(_statsFile, "  Median of diffs: %g\n", median);
    }
    fprintf(_statsFile, "Note: Diffs are primary minus secondary\n");
    fprintf(_statsFile, "=======================================\n");
    if (_params.debug) {
      cerr << "Writing to stats file: " << _statsPath << endl;
    }
  }

  if (_summaryFile) {
    DateTime ptime(_primaryTime);
    fprintf(_summaryFile,
            "%.4d %.2d %.2d %.2d %.2d %.2d %12ld %10.5f "
            "%7.0f %10.4f %10.4f %10.4f\n",
            ptime.getYear(),
            ptime.getMonth(),
            ptime.getDay(),
            ptime.getHour(),
            ptime.getMin(),
            ptime.getSec(),
            (long int) _primaryTime,
            (double) _primaryTime / 86400.0,
            count,
            mean,
            sdev,
            median);
    if (_params.debug) {
      cerr << "Writing to summary file: " << _summaryPath << endl;
    }
  }
  
  if (_params.write_results_to_spdb) {
    _writeResultsToSpdb(count, mean, sdev, median);
  }
  
}

//////////////////////////////////////////////////
// open the output files

int RadarMdvCompare::_openOutputFiles()
{

  // create the directory for the output files, if needed

  if (_params.write_output_stats ||
      _params.write_output_table ||
      _params.append_to_summary_file) {
    if (ta_makedir_recurse(_params.output_dir)) {
      int errNum = errno;
      cerr << "ERROR - RadarMdvCompare::_openOutputFiles";
      cerr << "  Cannot create output dir: "
           << _params.output_dir << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  // compute base
  
  DateTime ptime(_primaryTime);
  char base[1024];
  sprintf(base, "%s/%s%.4d%.2d%.2d_%.2d%.2d%.2d",
          _params.output_dir,
	  _params.output_file_prefix,
          ptime.getYear(),
          ptime.getMonth(),
          ptime.getDay(),
          ptime.getHour(),
          ptime.getMin(),
          ptime.getSec());

  // open stats file

  if (_params.write_output_stats && _statsFile == NULL) {

    char path[1024];
    sprintf(path, "%s.stats.%s", base, _params.output_file_ext);
    _statsPath = path;

    if ((_statsFile = fopen(path, "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - RadarMdvCompare::_openOutputFiles";
      cerr << "  Cannot open stats file for writing: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (_params.debug) {
      cerr << "Opening stats file: " << path << endl;
    }

  }
  
  // open table file

  if (_params.write_output_table && _tableFile == NULL) {

    char path[1024];
    sprintf(path, "%s.table.%s", base, _params.output_file_ext);
    _tablePath = path;
    
    if ((_tableFile = fopen(path, "w")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - RadarMdvCompare::_openOutputFiles";
      cerr << "  Cannot open table file for writing: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (_params.debug) {
      cerr << "Opening table file: " << path << endl;
    }

  }
  
  // open summary file

  if (_params.append_to_summary_file && _summaryFile == NULL) {

    char path[1024];
    sprintf(path, "%s.summary.%s", base, _params.output_file_ext);
    _summaryPath = path;
    
    if ((_summaryFile = fopen(path, "a")) == NULL) {
      int errNum = errno;
      cerr << "ERROR - RadarMdvCompare::_openOutputFiles";
      cerr << "  Cannot open summary file for appending: " << path << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (_params.debug) {
      cerr << "Opening summary file: " << path << endl;
    }

    fprintf(_summaryFile,
            "# year month day hour min sec utime uday count mean sdev median\n");

  }
  
  return 0;

}

//////////////////////////////////////////////////
// write the results to SPDB

int RadarMdvCompare::_writeResultsToSpdb(double count, 
                                         double mean,
                                         double sdev,
                                         double median)

{

  string xml;

  xml += TaXml::writeStartTag("RadarComparison", 0);
  
  Path primaryPath(_primary.getPathInUse());
  xml += TaXml::writeString("PrimaryFile", 1, primaryPath.getFile());

  Path secondaryPath(_secondary.getPathInUse());
  xml += TaXml::writeString("SecondaryFile", 1, secondaryPath.getFile());
  
  DateTime primaryTime(_primaryTime);
  xml += TaXml::writeString("PrimaryTime", 1, primaryTime.getW3cStr());

  DateTime secondaryTime(_secondaryTime);
  xml += TaXml::writeString("SecondaryTime", 1, secondaryTime.getW3cStr());
  
  xml += TaXml::writeInt("nPts", 1, (int) count);

  xml += TaXml::writeDouble("medianDiff", 1, median);
  xml += TaXml::writeDouble("meanDiff", 1, mean);
  xml += TaXml::writeDouble("sdevDiff", 1, sdev);

  xml += TaXml::writeDouble("minValueForDiff", 1, _params.min_value_for_diff);
  xml += TaXml::writeDouble("minValidDiff", 1, _params.min_valid_diff);
  xml += TaXml::writeDouble("maxValidDiff", 1, _params.max_valid_diff);

  if (_params.set_bounding_box) {
    xml += TaXml::writeDouble("boundingBoxMinLat", 1, _params.bounding_box.min_lat);
    xml += TaXml::writeDouble("boundingBoxMinLon", 1, _params.bounding_box.min_lon);
    xml += TaXml::writeDouble("boundingBoxMaxLat", 1, _params.bounding_box.max_lat);
    xml += TaXml::writeDouble("boundingBoxMaxLon", 1, _params.bounding_box.max_lon);
  }

  if (_params.set_vlevel_limits) {
    xml += TaXml::writeDouble("lowerVlevelKm", 1, _params.lower_vlevel);
    xml += TaXml::writeDouble("upperVlevelKm", 1, _params.upper_vlevel);
  }

  if (_params.set_range_limits) {
    xml += TaXml::writeDouble("minRangeKm", 1, _params.max_range_km);
    xml += TaXml::writeDouble("maxRangeKm", 1, _params.min_range_km);
  }

  xml += TaXml::writeEndTag("RadarComparison", 0);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Writing XML results to SPDB:" << endl;
    cerr << xml << endl;
  }
  
  DsSpdb spdb;
  time_t validTime = primaryTime.utime();
  int dataType = 0;
  spdb.addPutChunk(dataType, validTime, validTime, xml.size() + 1, xml.c_str());
  if (spdb.put(_params.spdb_output_url,
               SPDB_XML_ID, SPDB_XML_LABEL)) {
    cerr << "ERROR - RadarMdvCompare::_writeResultsToSpdb" << endl;
    cerr << spdb.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote to spdb, url: " << _params.spdb_output_url << endl;
  }

  return 0;

}

