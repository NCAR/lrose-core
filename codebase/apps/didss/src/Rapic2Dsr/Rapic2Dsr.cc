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
// Rapic2Dsr.cc
//
// Rapic2Dsr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include <vector>
#include <cerrno>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <didss/DsInputPath.hh>
#include <toolsa/toolsa_macros.h>
#include <rapformats/DsRadarMsg.hh>
#include "Rapic2Dsr.hh"
#include "Beam.hh"
using namespace std;

// Constructor

Rapic2Dsr::Rapic2Dsr(int argc, char **argv)

{

  isOK = true;
  _input = NULL;
  _clearScanList();
  _volNum = 0;
  _scanType = -1;

  // set programe name

  _progName = (char *) "Rapic2Dsr";
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
  
  // create the data input object

  if (_params.mode == Params::REALTIME) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _params.max_realtime_valid_age,
			     PMU_auto_register,
			     _params.use_latest_data_info,
			     _params.latest_file_only);
  } else if (_params.mode == Params::ARCHIVE) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _params.input_dir,
			     _args.startTime, _args.endTime);
  } else if (_params.mode == Params::FILELIST) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  }

  // initialize the output queue

  if (_rQueue.init(_params.output_url,
		   _progName.c_str(),
		   _params.debug >= Params::DEBUG_VERBOSE,
		   DsFmq::READ_WRITE, DsFmq::END,
		   _params.output_compression != Params::NO_COMPRESSION,
		   _params.output_n_slots,
		   _params.output_buf_size)) {
    cerr << "WARNING - trying to create new fmq" << endl;
    if (_rQueue.init(_params.output_url,
		     _progName.c_str(),
		     _params.debug >= Params::DEBUG_VERBOSE,
		     DsFmq::CREATE, DsFmq::START,
		     _params.output_compression != Params::NO_COMPRESSION,
		     _params.output_n_slots,
		     _params.output_buf_size)) {
      cerr << "ERROR - Rapic2Dsr" << endl;
      cerr << "  Cannot open fmq, URL: " << _params.output_url << endl;
      isOK = false;
      return;
    }
  }
      
  _rQueue.setCompressionMethod((ta_compression_method_t)
			       _params.output_compression);

  if (_params.write_blocking) {
    _rQueue.setBlockingWrite();
  }
  if (_params.data_mapper_report_interval > 0) {
    _rQueue.setRegisterWithDmap(true, _params.data_mapper_report_interval);
  }

  return;

}

// destructor

Rapic2Dsr::~Rapic2Dsr()

{

  if (_input) {
    delete _input;
  }
  _clearPpiFields();

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Rapic2Dsr::Run()
{
  
  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  // loop until end of data
  
  _input->reset();
  char *input_file_path;

  while ((input_file_path = _input->next()) != NULL) {
    
    PMU_auto_register("In main loop");

    if (_processFile(input_file_path)) {
      cerr << "ERROR - Rapic2Dsr::Run" << endl;
      cerr << "  Cannot process file: " << input_file_path << endl;
    }

  } // while
  
  return iret;

}

//////////////////////////////////////////////////
// process this file

int Rapic2Dsr::_processFile(const char *file_path)

{

  if (_params.debug) {
    cerr << "Processing file: " << file_path << endl;
  }

  // set up the input line buffer

  Linebuff lineBuf;
  
  if (lineBuf.openFile(file_path)) {
    cerr << "ERROR - Rapic2Dsr::_processFile" << endl;
    cerr << "  Cannot open file: " << file_path << endl;
    return -1;
  }

  // loop through all images in the file

  int imageNum = 0;

  while (!lineBuf.endOfFile()) {
    
    // find start of image
    
    if (_findImageStart(lineBuf)) {
      return 0;
    }

    if (_params.debug) {
      cerr << ">>>>>> start of image <<<<<<" << endl;
    }
    
    // process the image

    if (_processImage(file_path, lineBuf, imageNum)) {
      continue;
    }
    imageNum++;

    // find end of image
    
    if (_findImageEnd(lineBuf)) {
      continue;
    }
      
    if (_params.debug) {
      cerr << ">>>>>> end of image <<<<<<" << endl;
    }

  } // while
  
  // close input file

  lineBuf.closeFile();

  return 0;

}

//////////////////////////////////////////////////
// find start of image
//
// Return 0 on success, -1 on failure

int Rapic2Dsr::_findImageStart(Linebuff &lineBuf)

{
  const char *searchStr = "/IMAGE: ";
  int searchLen = strlen(searchStr);
  while (lineBuf.readNext() == 0) {
    if (!strncmp(lineBuf.line_buff, searchStr, searchLen)) {
      return 0;
    }
    lineBuf.reset();
  }
  return -1;
}

//////////////////////////////////////////////////
// find end of image
//
// Return 0 on success, -1 on failure

int Rapic2Dsr::_findImageEnd(Linebuff &lineBuf)

{
  const char *searchStr = "/IMAGEEND: ";
  int searchLen = strlen(searchStr);
  while (lineBuf.readNext() == 0) {
    if (!strncmp(lineBuf.line_buff, searchStr, searchLen)) {
      // rjp 4 Sep 2001 need to reset lineBuf or next scan is missed.
      lineBuf.reset();
      return 0;
    }
    lineBuf.reset();
  }
  return -1;
}

//////////////////////////////////////////////////
// load scan params
//
// Return 0 on success, -1 on failure

int Rapic2Dsr::_loadScanParams(ScanParams &sParams, Linebuff &lineBuf)

{

  while (lineBuf.readNext() == 0) {
    
    if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) {

      // into radial data, set the linebuf to repeat this line
      // and break out

      lineBuf.setRepeat();
      break;
      
    } else {
      
      // params info
	
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	cerr << "Params info >>> " << lineBuf.line_buff << endl;
      }
      if (sParams.set(lineBuf.line_buff, _nScans)) {
	cerr << "ERROR - Rapic2Dsr::_loadScanParams" << endl;
	cerr << "  Cannot load scan params" << endl;
	return -1;
      }
      
    } // if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) 
      
    lineBuf.reset();

  } // while

  if (!sParams.setDone || sParams.scan_num == 0) {
    return -1;
  }

  if (_params.debug) {
    //  if (_params.debug >= Params::DEBUG_VERBOSE) {
    sParams.print(cerr);
  }

  return 0;

}

//////////////////////////////////////////////////
// process the next image

int Rapic2Dsr::_processImage(const char *file_path, Linebuff &lineBuf,
			     int imageNum)

{

  // read the scan list in volume header 

  _clearScanList();
  lineBuf.reset();
  while (!_scanListComplete && lineBuf.readNext() == 0) {
    if (_addToScanList(lineBuf)) {
      cerr << "ERROR - Rapic2Dsr::_processImage" << endl;
      cerr << "  Cannot load scan list" << endl;
      cerr << "    File path: " << file_path << endl;
      cerr << "    Image num: " << imageNum << endl;
      return -1;
    }
    lineBuf.reset();
  }

  if (_nScans < 1) {
    cerr << "ERROR - Rapic2Dsr::_processImage" << endl;
    cerr << "  File has no scans: " << file_path << endl;
    return -1;
  }
  
  // rjp 3 Sep 2001 - CompPPI scan with only one PPI
  if (_nScans ==1) {
    if (_params.debug) {
      cerr << "WARNING - Rapic2Dsr::_processImage" << endl;
      cerr << "  Scan is CompPPI: " << file_path << endl;
    }
    return -1;
  } 

  // get the elev list and n fields

  double prevElev = -9999;
  int nFieldsElev = 0;
  int nFieldsMax = 0;

  for (size_t ii = 0; ii < _scanList.size(); ii++) {
    double elev = _scanList[ii].elev_angle;
    if (fabs(elev - prevElev) > 0.00001) {
      _elevList.push_back(elev);
      prevElev = elev;
      nFieldsMax = MAX(nFieldsMax, nFieldsElev);
      nFieldsElev = 1;
    } else {
      nFieldsElev++;
    }
  } // ii
  nFieldsMax = MAX(nFieldsMax, nFieldsElev);
  _nFields = nFieldsMax;
  
  if (_nFields < 1) {
    cerr << "ERROR - Rapic2Dsr::_processImage" << endl;
    cerr << "  File has no fields: " << file_path << endl;
    return -1;
  }

  // compute the number of PPI's

  int nPPIs = _nScans / _nFields;
  if (_params.debug) {
    cerr << "nPPIs: " << nPPIs << endl;
  }

  // load scan params from scan header
  
  ScanParams sParams(_params);
  if (_loadScanParams(sParams, lineBuf)) {
    cerr << "ERROR - Rapic2Dsr::_processImage" << endl;
    cerr << "  Cannot load scan params" << endl;
    return -1;
  }
  
  // check radar name

  if (strlen(_params.radar_name) > 0) {
    if (strcmp(sParams.radar_name, _params.radar_name)) {
      if  (_params.debug) {
	cerr << "Incorrect radar name: " << sParams.radar_name << endl;
	cerr << "  Should be: " << _params.radar_name << endl;
	cerr << "  Skipping this image" << endl;
      }
      return -1;
    }
  }

  // Add check for volume_id. rjp 18 Jul 2006. 
  // check volume_id

  if (sParams.volume_id != _params.volume_id) {
    if (_params.debug) {
      cerr << "Incorrect volume_id: " << sParams.volume_id 
	   << "  Expecting: " << _params.volume_id << endl; 
      cerr << " Skipping this volume" << endl << endl;
    }
    return -1;
  }
  
  // start of volume

  _rQueue.putStartOfVolume(_volNum);

  // loop through the PPIs

  for (int ii = 0; ii < nPPIs; ii++) {
    
    if (_processPPI(ii, lineBuf, sParams)) {
      cerr << "ERROR - Rapic2Dsr::_processImage" << endl;
      cerr << "  Failed on PPI number: " << ii << endl;
      return -1;
    }

  }

  // end of volume
  
  _rQueue.putEndOfVolume(_volNum);
  _volNum++;
    
  return 0;

}


//////////////////////////////////////////////////
// process PPI

int Rapic2Dsr::_processPPI(int ppi_num, Linebuff &lineBuf, ScanParams &sParams)
  
{

  // Due to uncertainty about integrity of the radar volume there is need 
  // to add a test to check that station_id for each PPI scan matches what is 
  // defined in volume header. (rjp 21/5/2008)

  _clearPpiFields();

  // read in the PPI scan

  sRadl radial;
  int maxGates = 0;
  int maxBeams = 0;

  if (_params.debug) {
    cerr << "==========================" << endl;
    cerr << "-->> Starting ppi num: " << ppi_num << endl;
  }

  // read in all fields

  for (int ifield = 0; ifield < _nFields; ifield++) {
    
    int scan_num = ppi_num * _nFields + ifield ;
    double targetElev = _scanList[scan_num].elev_angle;
    
    PPIField *ppiField = new PPIField(_params);
    
    while (lineBuf.readNext() == 0) {
      
      if (lineBuf.IsEndOfRadarImage()) {
	
	if (_checkScanParams(sParams)) {
	  delete ppiField;
	  return -1;
	}

	ppiField->fieldName = sParams.field_name;
	ppiField->time = sParams.time;
	// To give index from 0 instead of 1. 
	ppiField->scanNum = sParams.scan_num - 1;
	if (ppiField->scanNum != scan_num) {
	  cerr << "WARNING - scanNum mismatch" << endl;
	  cerr << "  Header scanNum: " << ppiField->scanNum << endl;
	  cerr << "  Calculated scanNum: " << scan_num << endl;
	}
	ppiField->rangeRes = sParams.range_res;
	ppiField->startRange = sParams.start_range;
	lineBuf.reset();
	break;
	
      } // if (lineBuf.IsEndOfRadarImage())
      
      if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) {

	bool isBinary;
	if (_decodeRadial(lineBuf, radial, isBinary, sParams.video_res) == 0) {
	  ppiField->addBeam(_params, &radial, sParams, isBinary, targetElev);
	}
	
      } else {
	
	// scan params info
	
	if (_params.debug >= Params::DEBUG_VERBOSE) {
	  cerr << "Params info >>> " << lineBuf.line_buff << endl;
	}
	if (sParams.set(lineBuf.line_buff, _nScans)) {
	  cerr << "ERROR - Rapic2Dsr::_processPPI" << endl;
	  cerr << "  Cannot load scan params" << endl;
	  return -1;
	}
	
      } // if (lineBuf.IsBinRadl() || lineBuf.IsRadl()) 
      
      lineBuf.reset();

    } // while (lineBuf ...

    maxBeams = MAX(maxBeams, ((int) ppiField->beams.size()));
    maxGates = MAX(maxGates, ppiField->maxGates);
    
    //rjp 19/8/2006 test for scan with 0 beams
    if (maxBeams == 0) {
      if (_params.debug) {
	cerr << "WARNING - Rapic2Dsr::_processPPI" << endl;
	cerr << " Scan has 0 beams" << endl;
	cerr << " Scan will not be written to fmq" << endl;
      }
    }
 
    //rjp 5 Jul 2006 for scan with all null radials set maxGates > 0 to 
    // ensure scan is sent to fmq
    if (maxGates == 0) {
      maxGates=4;
      if (_params.debug) {
	cerr << "WARNING - Rapic2Dsr::_processPPI" << endl;
	cerr << "  Scan has null radials only" << endl;
	cerr << "  set maxGates > 0 to ensure scan sent to fmq" << endl; 
      }
    }

    if (_params.debug) {
      // cerr << "---->> scanNum: " << ppiField->scanNum << endl;
      cerr << "---->> scanNum: " << sParams.scan_num << endl; // rjp set idx for scan num same as data file. 
      cerr << "       time: " << DateTime::str(ppiField->time) << endl;
      cerr << "       fieldNum: " << ifield << endl;
      cerr << "       field name: " << ppiField->fieldName << endl;
      cerr << "       elev angle: " << sParams.elev_angle << endl;  // rjp 11 Sep 2001
      cerr << "          ppi.nBeams: " << ppiField->beams.size() << endl;
      cerr << "          ppi.maxGates: " << ppiField->maxGates << endl;
      cerr << "          ppiField->rangeRes: " << ppiField->rangeRes << endl;
      cerr << "          ppiField->startRange: " << ppiField->startRange << endl;
    }
    
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "=====================================" << endl;
      ppiField->printFull(cerr);
    }

    // add field to vector

    _ppiFields.push_back(ppiField);
    
  } // ifield

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "-------->> _nFields: " << _nFields << endl;
    cerr << "-------->> maxBeams: " << maxBeams << endl;
    cerr << "-------->> maxGates: " << maxGates << endl;
  }

  // check startRange / rangeRes is same for multiple fields for Beijing data
  if (_nFields > 1) {
    if (_ppiFields[0]->fieldName == "Refl" && _ppiFields[1]->fieldName == "Vel") {
      if (_ppiFields[0]->rangeRes != _ppiFields[1]->rangeRes) {
	if (_params.debug) {
	  cerr << "******************************************" << endl;
	  cerr << "WARNING - nFields > 1 and range resolution different for fields" << endl; 
	  for ( int ifield = 0; ifield < _nFields; ifield++) {
	    cerr << "  field name: " << _ppiFields[ifield]->fieldName 
		 << "  maxGates: " << _ppiFields[ifield]->maxGates  
		 << "  start range: " << _ppiFields[ifield]->startRange 
		 << "  range res:  " << _ppiFields[ifield]->rangeRes << endl; 
	  }
          cerr << " maxGates:  " << maxGates << endl;
	  cerr << "******************************************" << endl;
	}

	// remap refl data to same resolution as vel data 

	_ppiFields[0]->convertNexradResolution(maxGates); 

	if (_params.debug) {
	  cerr << "Remap refl to same rangeRes as vel" << endl;
	  cerr << "  field name: " << _ppiFields[0]->fieldName
               << "  maxGates: " << _ppiFields[0]->maxGates 
               << "  startRange: " << _ppiFields[0]->startRange 
               << "  rangeRes: " << _ppiFields[0]->rangeRes << endl;
	  cerr << endl;
	  cerr << "******************************************" << endl;
	}
      }
    }
  }
 

  // write out the ppi

  // new scan type?

  if (_scanType != _scanList[_ppiFields[0]->scanNum].flag1) {
    _scanType = _scanList[_ppiFields[0]->scanNum].flag1;
    _rQueue.putNewScanType(_scanType);
  }

  // start of tilt

  _rQueue.putStartOfTilt(ppi_num, _ppiFields[0]->time);

  // write out params

  if (_writeRadarAndFieldParams(maxGates, sParams)) {
    cerr << "ERROR - Rapic2Dsr::_processPPI" << endl;
    cerr << "  Cannot write out params" << endl;
    return -1;
  }

  // write out the beams

  if (_writeBeams(ppi_num, maxGates, maxBeams, sParams)) {
    cerr << "ERROR - Rapic2Dsr::_processPPI" << endl;
    cerr << "  Cannot write out beams" << endl;
    return -1;
  }

  // end of tilt

  _rQueue.putEndOfTilt(ppi_num);

  if (_params.debug) {
    cerr << "-->> Ending ppi num: " << ppi_num << endl;
  }

  return 0;

}

///////////////////////
// clear out ppi fields

void Rapic2Dsr::_clearPpiFields()

{
  for (int ii = 0; ii < (int) _ppiFields.size(); ii++) {
    delete _ppiFields[ii];
  }
  _ppiFields.clear();
}

//////////////////
// decode a radial

int Rapic2Dsr::_decodeRadial(Linebuff &lineBuf, sRadl &radial, bool &isBinary, int rLevels)

{

  isBinary = true;

  if (lineBuf.IsBinRadl()) {

    if (sRadl::DecodeBinaryRadl((unsigned char *) lineBuf.line_buff,
				&radial) < 0) {
      cerr << "ERROR - Rapic2Dsr::_decodeRadial" << endl;
      cerr << "  calling DecodeBinaryRadl" << endl;
      return -1;
    }

    isBinary = true;
    
  } else if (lineBuf.IsRadl()) {
    if (rLevels == 6) {
      if (sRadl::RLE_6L_radl(lineBuf.line_buff,
			     &radial) < 0) {
	cerr << "ERROR - Rapic2Dsr::_decodeRadial" << endl;
	cerr << "  calling RLE_6L_radl" << endl;
	return -1;
      }
 
    }
    else {
      int NumLevels = 256;
      if (sRadl::RLE_16L_radl(lineBuf.line_buff,
			      &radial, NumLevels-1) < 0) {
	cerr << "ERROR - Rapic2Dsr::_decodeRadial" << endl;
	cerr << "  calling RLE_16L_radl" << endl;
	return -1;
      }
    }
    isBinary = false;
    
  }
  
  return 0;

}

//////////////////////
// clear the scan list

void Rapic2Dsr::_clearScanList()

{
  _scanListComplete = false;
  _nScans = 0;
  _nFields = 0;
  _scanList.clear();
  _elevList.clear();
}

////////////////////////
// add to the scan list

int Rapic2Dsr::_addToScanList(Linebuff &lineBuf)

{

  if (_params.debug) {
    cerr << "Scan list >>>  " << lineBuf.line_buff << endl;
  }

  if (_nScans == 0) {
    int nscans;
    if (sscanf(lineBuf.line_buff, "/IMAGESCANS: %d", &nscans) == 1) {
      _nScans = nscans;
    }
    return 0;
  }

  scan_description_t scan;
  MEM_zero(scan);
  if (sscanf(lineBuf.line_buff,
	     "/SCAN%d:%d%s%d%lg%d%d%d%d",
	     &scan.scan_num,
	     &scan.station_id,
	     scan.time_str,
	     &scan.flag1,
	     &scan.elev_angle,
	     &scan.field_num,
	     &scan.n_fields,
	     &scan.flag2,
	     &scan.flag3) == 9) {
    _scanList.push_back(scan);
    //  _nFields = MAX(_nFields, scan.field_num + 1);
    return 0;
  }
  
  if (strstr(lineBuf.line_buff, "/IMAGEHEADER END") != NULL) {
    if ((int) _scanList.size() == _nScans) {
      _scanListComplete = true;
      if (_params.debug >= Params::DEBUG_VERBOSE) {
	_printScanList(cerr);
      }
    } else {
      cerr << "ERROR - Rapic2Dsr::_loadScanList" << endl;
      cerr << "  IMAGEHEADER END found, but not all scans found yet" << endl;
      return -1;
    }
  }

  return 0;

}

//////////////////////
// print the scan list

void Rapic2Dsr::_printScanList(ostream &out)

{

  if (!_scanListComplete) {
    cerr << "ERROR - scan list not yet complete" << endl;
    return;
  }

  out << "SCAN LIST" << endl;
  out << "=========" << endl;
  out << endl;

  out << "_nScans: " << _nScans << endl;
  for (int ii = 0; ii < _nScans; ii++) {
    out << "#" << _scanList[ii].scan_num
	<< " id: " << _scanList[ii].station_id
	<< " time: " << _scanList[ii].time_str
	<< " flag1: " << _scanList[ii].flag1
	<< " elev: " << _scanList[ii].elev_angle
	<< " field_num: " << _scanList[ii].field_num
	<< " n_fields: " << _scanList[ii].n_fields
	<< " flag2: " << _scanList[ii].flag2
	<< " flag3: " << _scanList[ii].flag3
	<< endl;
  } // ii

}

////////////////////////////////////
// check that the scan params match

int Rapic2Dsr::_checkScanParams(const ScanParams &sParams)

{

  if (!sParams.setDone || sParams.scan_num == 0) {
    cerr << "ERROR - Rapic2Dsr::_checkScanParams" << endl;
    cerr << "  Scan params not set" << endl;
    return -1;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    sParams.print(cerr);
  }

  // check elevation

  scan_description_t &scan = _scanList[sParams.scan_num - 1];

  //  rjp 7 Sep 2001. Make same as wxwdss version.
  //  if (sParams.elev_angle != scan.elev_angle) {
  if (fabs(sParams.elev_angle - scan.elev_angle) > (_params.beam_width / 2.0))  {
    cerr << "ERROR - Rapic2Dsr::_checkScanParams" << endl;
    cerr << "  Scan number: " << sParams.scan_num << endl;
    cerr << "  Incorrect elevation angle" << endl;
    cerr << "  Should be: " << scan.elev_angle << endl;
    cerr << "  Found: " << sParams.elev_angle << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// write out the radar and field params


int Rapic2Dsr::_writeRadarAndFieldParams(int maxGates,
					 const ScanParams &sParams)

{

  // load up radar params and field params message
  
  // radar params

  DsRadarMsg msg;
  DsRadarParams &rParams = msg.getRadarParams();
  
  rParams.radarId = sParams.station_id;
  rParams.radarType = _scanList[_ppiFields[0]->scanNum].flag1;
  rParams.numFields = _nFields;
  rParams.numGates = maxGates;
  rParams.samplesPerBeam = _params.samples_per_beam;
  rParams.scanType = _params.scan_type;
  rParams.scanMode = DS_RADAR_SURVEILLANCE_MODE;
  rParams.polarization = _params.polarization_code;
  
  rParams.radarConstant = _params.radar_constant;
  rParams.altitude = sParams.ht / 1000.0;
  if (_params.change_latitude_sign) {
    rParams.latitude = sParams.lat * -1.0;
  } else {
    rParams.latitude = sParams.lat;
  }
  rParams.longitude = sParams.lon;
  rParams.gateSpacing = sParams.range_res / 1000.0;
  rParams.startRange = sParams.start_range / 1000.0;
  rParams.horizBeamWidth = _params.beam_width;
  rParams.vertBeamWidth = _params.beam_width;
  rParams.pulseWidth = sParams.pulse_length;
  rParams.pulseRepFreq = sParams.prf;
  rParams.wavelength = (3.0e8 / (sParams.freq_mhz * 1.0e6)) * 100.0;
  rParams.xmitPeakPower = sParams.peak_power;
  rParams.receiverMds = _params.receiver_mds;
  rParams.receiverGain = _params.receiver_gain;
  rParams.antennaGain = _params.antenna_gain;
  rParams.systemGain = _params.system_gain;
  rParams.unambigVelocity = sParams.nyquist;
  rParams.unambigRange = (3.0e8 / (2.0 * sParams.prf)) / 1000.0;

  rParams.radarName = sParams.radar_name;
  rParams.scanTypeName = "";

  // field params

  vector< DsFieldParams* > &fieldParams = msg.getFieldParams();
  for (int ifield = 0; ifield < (int) _ppiFields.size(); ifield++) {
    
    if (_ppiFields[ifield]->fieldName == "Refl") {
      double scale = 0.5;
      double bias = -32.0;
      DsFieldParams *fparams = new DsFieldParams( "DBZ", "dBZ", scale, bias);
      fieldParams.push_back( fparams );
    } else if (_ppiFields[ifield]->fieldName == "Vel") {
      double scale = sParams.nyquist / 127.0;
      double bias = (-1.0 * sParams.nyquist * 128.0) / 127.0;
      DsFieldParams *fparams = new DsFieldParams( "VEL", "m/s", scale, bias);
      fieldParams.push_back( fparams );
    } else if (_ppiFields[ifield]->fieldName == "Width") {
      double scale = sParams.nyquist / 255.0;
      double bias = 0;
      DsFieldParams *fparams = new DsFieldParams( "WIDTH", "m/s", scale, bias);
      fieldParams.push_back( fparams );
    } else if (_ppiFields[ifield]->fieldName == "ZDR") {
      double scale = 0.1;
      double bias = (-12.7 * 128.0) / 127.0;
      DsFieldParams *fparams = new DsFieldParams( "ZDR", "dB", scale, bias);
      fieldParams.push_back( fparams );
    } else if (_ppiFields[ifield]->fieldName == "PHIDP") {
      double scale = 180.0 / 127.0;
      double bias = (-1.0 * 180 * 128.0) / 127.0;
      DsFieldParams *fparams = new DsFieldParams( "PHIDP", "deg", scale, bias);
      fieldParams.push_back( fparams );
    } else if (_ppiFields[ifield]->fieldName == "RHOHV") {
      double scale = 1.0 / 255.0;
      double bias = 0.1;
      DsFieldParams *fparams = new DsFieldParams( "RHOHV", "", scale, bias);
      fieldParams.push_back( fparams );
    } else {
      DsFieldParams *fparams = new
	DsFieldParams( _ppiFields[ifield]->fieldName.c_str(),
		       "", 1.0, 0.0);
      fieldParams.push_back( fparams );
    }
    
  } // ifield

  // send the params

  if (_rQueue.putDsMsg
      (msg,
       DS_DATA_TYPE_RADAR_PARAMS | DS_DATA_TYPE_RADAR_FIELD_PARAMS)) {
    cerr << "ERROR - Rapic2Dsr::_writeRadarAndFieldParams" << endl;
    cerr << "  Cannot put radar and field params message to FMQ" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////
// write out the beams


int Rapic2Dsr::_writeBeams(int ppi_num,
			   int maxGates,
			   int maxBeams,
			   const ScanParams &sParams)

{

  // check ppi fields have correct number of beams
  
  int nBeams = (int) ((360.0 / sParams.angle_res) + 0.5);
  
  for (int ifield = 0; ifield < _nFields; ifield++) {
    // Rapic format data does not send blank radls,
    // so size will often be < nBeams
    if (_params.debug) {
      if ((int) _ppiFields[ifield]->beams.size() != nBeams) {
	cerr << "WARNING - Rapic2Dsr::_processPPI" << endl;
	cerr << "  Field does not have correct number of beams" << endl;
	cerr << "  Field " << ifield << " has "
	     << _ppiFields[ifield]->beams.size() << " beams" << endl;
	cerr << "  Expected number: " << nBeams << endl;
      }
    }
  }

  // loop through the azimuths
  
  int dataLen = _nFields * maxGates;
  //if (dataLen == 0) dataLen = 1;   // rjp 5 Sep 2001
  //rjp 5 Jul 2006 commented out above.  maxGates set in _processPPI to 
  //    allow for all null radials. 
  TaArray<ui08> data_;
  ui08 *data = data_.alloc(dataLen);
  
  TaArray<size_t> beamIndex_;
  size_t *beamIndex = beamIndex_.alloc(_nFields);
  for (int ifield = 0; ifield < _nFields; ifield++) {
    beamIndex[ifield] = 0;
  }

  for (int ibeam = 0; ibeam < maxBeams; ibeam++) {

    // find the next azimuth and elevation in sequence

    double az = 0;
    double el = 0;
    bool dataFound = false;
    
    for (int ifield = 0; ifield < (int) _ppiFields.size(); ifield++) {
      const PPIField *ppiField = _ppiFields[ifield];
      size_t index = beamIndex[ifield];
      if (index < ppiField->beams.size()) {

	Beam *beam = ppiField->beams[index];

	if (!dataFound) {
	  az = beam->azimuth;
	  el = beam->elevation;
	  dataFound = true;
	} else {
	  double az1 = beam->azimuth;
	  if (_azLessThan(az1, az)) {
	    az = az1;
	    el = beam->elevation;
	  }
	}

      } // if (index ...
    } // ifield

    if (!dataFound) {
      // all fields done
      break;
    }

    // clear the data area

    memset(data, 0, dataLen);
    int beamTime = 0;
    
    // for each field which has a matching azimuth, load up the data

    for (int ifield = 0; ifield < (int) _ppiFields.size(); ifield++) {
      
      const PPIField *ppiField = _ppiFields[ifield];
      size_t index = beamIndex[ifield];
      
      if (index < ppiField->beams.size()) {
	
	Beam *beam = ppiField->beams[index];
	double az1 = beam->azimuth;
	
	if (az == az1) {
	  
	  // load the data from this field beam
	  
	  ui08 *dd = data + ifield;
	  for (int igate = 0; igate < beam->nGates; igate++) {
	    *dd = beam->vals[igate];
	    dd += _nFields;
	  }

	  beamIndex[ifield]++;
	  beamTime = ppiField->time;

	} // if (az == az1)

      } // if (index < ppiField->beams.size())

    } // ifield

    // load the message

    DsRadarMsg msg;
    DsRadarBeam &radarBeam = msg.getRadarBeam();
    radarBeam.loadData(data, dataLen);
    if (_params.use_wallclock_time) {
      radarBeam.dataTime = time(NULL);
    } else {
      radarBeam.dataTime = beamTime + _params.time_correction;
    }
    radarBeam.azimuth = az;
    radarBeam.elevation = el;
    radarBeam.targetElev = _scanList[_ppiFields[0]->scanNum].elev_angle;
    radarBeam.tiltNum = ppi_num;
    radarBeam.volumeNum = _volNum;

    // rjp 5 Jul 2006
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-----------------------------" << endl;
      radarBeam.print(cerr);
      cerr << endl;
      cerr << "ByteData" << endl;
      radarBeam.printByteData(stdout);
      cerr << "-----------------------------" << endl;
    }
 
    // send the params
    
    if (_rQueue.putDsBeam
	(msg, DS_DATA_TYPE_RADAR_BEAM_DATA)) {
      cerr << "ERROR - Rapic2Dsr::_writeBeam" << endl;
      cerr << "  Cannot put beam message to FMQ" << endl;
      return -1;
    }

    if (_params.write_delay_msecs > 0) {
      umsleep(_params.write_delay_msecs);
    }
    
  } // ibeam

  return 0;

}


//////////////////////////
// test for azimiuth order
//
// Returns true if az1 < az2

bool Rapic2Dsr::_azLessThan(double az1, double az2)

{
  
  if (az1 < az2) {

    if (az2 - az1 < 180.0) {
      return true;
    }

  } else {

    if (az1 - az2 > 180.0) {
      return true;
    }

  }

  return false;

}
