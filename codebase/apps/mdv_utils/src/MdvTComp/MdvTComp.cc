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
// MdvTComp.cc
//
// MdvTComp object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////
//
// Performs temporal compositing on Mdv file data
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include "MdvTComp.hh"
using namespace std;

static void _updateAverage(fl32 *wff, fl32 wmissing, fl32 *pff, fl32 pmissing, 
			   fl32 *cff, int nPointsField);
static void _updateMax(fl32 *wff, fl32 wmissing, fl32 *pff, fl32 pmissing, 
		       int nPointsField);
static void _constrainField(const Params::field_t &field, int nPointsField,
			    DsMdvx &working);

// Constructor

MdvTComp::MdvTComp(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvTComp";
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

  return;

}

// destructor

MdvTComp::~MdvTComp()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvTComp::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");
  
  // create DsMdvx object
  
  DsMdvx inMdvx;
  inMdvx.setDebug(_params.debug);

  // loop until end of data

  time_t lastOutputTime = 0;

  while (!_input.endOfData()) {

    if (_processNewData(inMdvx, lastOutputTime) == -1) {
      return -1;
    }

  } // while

  return 0;

}

////////////////////////////////////////////////
// _processNewData

int  MdvTComp::_processNewData(DsMdvx &inMdvx, time_t &lastOutputTime)
{
  PMU_auto_register("In main loop");
    
  // set up the Mdvx read

  _setupRead(inMdvx);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read for main file" << endl;
    inMdvx.printReadRequest(cerr);
  }

  // read the volume
    
  PMU_auto_register("Before read");
  if (_input.readVolumeNext(inMdvx)) {
    cerr << "ERROR - MdvTComp::Run" << endl;
    cerr << "  Cannot read in data." << endl;
    cerr << _input.getErrStr() << endl;
    return 0;
  }
    
  if (inMdvx.getMasterHeader().n_fields == 0) {
    cerr << "ERROR - MdvTComp::Run" << endl;
    cerr << "  No fields in input data, cannot proceed." << endl;
    return 0;
  }

  time_t timeCentroid = inMdvx.getMasterHeader().time_centroid;
  if (_params.output_frequency == Params::SYNCH_OUTPUT_TO_COMPOSITE) {
    if (lastOutputTime > 0) {
      if (timeCentroid - lastOutputTime < _params.composite_period) {
	cerr << "Skipping file, did not exceed composite period dt="
	     << timeCentroid - lastOutputTime << endl;
	return 0;
      }
    }
  } else if (_params.output_frequency == Params::SYNCH_OUTPUT_TO_LIST) {
    if (!_isWantedTime(timeCentroid)) {
      cerr << "Skipping file, not a wanted time" << endl;
      return 0;
    }
  }  

  lastOutputTime = timeCentroid;

  if (_params.debug) {
    cerr << "Working on file: " << inMdvx.getPathInUse() << endl;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Input file fields:" << endl;
    for (int ifield = 0;
	 ifield < inMdvx.getMasterHeader().n_fields; ifield++) {
      MdvxField *fld = inMdvx.getField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
    }
  }

  // copy to the working Mdvx

  DsMdvx working(inMdvx);

  // if doing averaging make a second copy to hold accumulated counts

  DsMdvx workingCounts;
  if (_params.composite_type == Params::AVERAGE) {
    workingCounts = DsMdvx(inMdvx);
  }

  // check for uniform length fields
    
  int nPointsField = 0;
  if (!_uniformFields(working, nPointsField)) {
    return -1;
  }
    
  // if constrained, set field values to missing as appropriate
  // if averaging, set counts to 0

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    _prepareData(_params._fields[ifield], nPointsField, working, workingCounts);
  }
    
  // create time composite

  if (_doComposite(working, workingCounts, nPointsField)) {
    cerr << "ERROR - MdvTComp::Run" << endl;
    cerr << "  Cannot perform time composite." << endl;

    // return 0 so outer loop will continue
    return 0;
  }

  if (_params.composite_type == Params::AVERAGE) {

    // finish up averaging

    for (int ifield = 0; ifield < _params.fields_n; ifield++) {
      _finishAveraging(_params._fields[ifield], nPointsField, working,
		       workingCounts);
    }
  }

  // overwrite missing values as appropriate

  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    _prepareOutput(_params._fields[ifield], nPointsField, working);
  }
  
  // write out
    
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Output file fields:" << endl;
    for (int ifield = 0;
	 ifield < working.getMasterHeader().n_fields; ifield++) {
      MdvxField *fld = working.getField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
    }
  }

  PMU_auto_register("Before write");
  working.setWriteLdataInfo();
  if(working.writeToDir(_params.output_url)) {
    cerr << "ERROR - MdvTComp::Run" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << working.getErrStr() << endl;
  }
  return 0;
}

//////////////////////////////////////////////////
// Setup the read


void MdvTComp::_setupRead(DsMdvx &mdvx) const

{

  mdvx.clearRead();
  
  if (_params.set_vlevel_limits) {
    mdvx.setReadVlevelLimits(_params.lower_vlevel,
			     _params.upper_vlevel);
  } else if (_params.vert_composite) {
    mdvx.setReadComposite();
  }
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  for (int i = 0; i < _params.fields_n; i++) {
    mdvx.addReadField(_params._fields[i].name);
  }
  
}

//////////////////////////////////////////////////
// Prepare the data by evaluting constraints, and initializing the
// counts to 0 if doing averaging

void MdvTComp::_prepareData(const Params::field_t &field, int nPointsField,
			    DsMdvx &working, DsMdvx &workingCounts) const
{
  // constrain values within limits

  _constrainField(field, nPointsField, working);
	
  if (_params.composite_type == Params::AVERAGE) {
    MdvxField *fld = workingCounts.getField(field.name);
    fl32 *ff = (fl32 *) fld->getVol();
    for (int j = 0; j < nPointsField; j++, ff++) {
      *ff = 0.0;
    }
  }
}

//////////////////////////////////////////////////
// Run

int MdvTComp::_doComposite(DsMdvx &working, DsMdvx &workingCounts,
			   int nPointsField) const

{

  // get the available data times
  
  const Mdvx::master_header_t &mhdr = working.getMasterHeader();
  time_t lastTimeProcessed = mhdr.time_centroid;
  DsMdvx times;
  times.clearTimeListMode();
  times.setTimeListModeValid(_params.input_url,
			     mhdr.time_centroid - _params.composite_period,
			     mhdr.time_centroid - 1);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    times.printTimeListRequest(cerr);
  }

  if (times.compileTimeList()) {
    cerr << "ERROR - MdvTComp::_doComposite()" << endl;
    cerr << "  Cannot compile time list" << endl;
    cerr << times.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (int i = 0; i < times.getNTimesInList(); i++) {
      cerr << "  Time " << i << ": " <<
	DateTime::str(times.getTimeFromList(i)) << endl;
    }
  }

  if (times.getNTimesInList() == 0) {
    if (_params.debug) {
      cerr << "  No past times found for composite" << endl;
    }
    return 0;
  }

  // read the past files, in reverse order

  DsMdvx past;
  past.setDebug(_params.debug);
  
  for (int itime = times.getNTimesInList() - 1; itime >= 0; itime--) {
    
    time_t pastFileTime = times.getTimeFromList(itime);
    if (lastTimeProcessed - pastFileTime < _params.min_delta_time) {
      continue;
    }
    
    _accumulateComposite(pastFileTime, nPointsField, past,
			 working, workingCounts);
    lastTimeProcessed = pastFileTime;
  } // itime

  return 0;
}

//////////////////////////////////////////////////
// accumulate composite information using data from pastFileTime

void MdvTComp::_accumulateComposite(const time_t pastFileTime, int nPointsField,
				    DsMdvx &past, DsMdvx &working,
				    DsMdvx &workingCounts) const
{
  // set up the read

  _setupRead(past);
  past.setReadTime(Mdvx::READ_CLOSEST, _params.input_url, 0, pastFileTime);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Read for past file, time: "
	 << DateTime::str(pastFileTime) << endl;
    past.printReadRequest(cerr);
  }
    
  // read the volume
    
  PMU_auto_register("Before read past");
  if (past.readVolume()) {
    cerr << "ERROR - MdvTComp::_doComposite()" << endl;
    cerr << "  Cannot read volume" << endl;
    cerr << past.getErrStr() << endl;
    return;
  }
    
  // check field lengths
    
  int nPointsThisFile = 0;
  if (!_uniformFields(past, nPointsThisFile)) {
    return;
  }
  if (nPointsThisFile != nPointsField) {
    cerr << "ERROR - MdvTComp::_doComposite()" << endl;
    cerr << "  Fields not of correct length" << endl;
    cerr << "  This file: " << past.getPathInUse() << endl;
    cerr << "    Npoints in this file: " << nPointsThisFile << endl;
    cerr << "  Main file: " << working.getPathInUse() << endl;
    cerr << "    Npoints in main file: " << nPointsField << endl;
    return;
  }
    
  // loop through the fields, accumulate into each of them
    
  for (int ifield = 0; ifield < _params.fields_n; ifield++) {
    _accumulateFieldComposite(_params._fields[ifield], nPointsField,
			      past, working, workingCounts);
  } 
}

//////////////////////////////////////////////////
// accumulate composite information for one field,
// pulling data out of 'past'

void MdvTComp::_accumulateFieldComposite(const Params::field_t &field,
					 int nPointsField,
					 DsMdvx &past, DsMdvx &working,
					 DsMdvx &workingCounts) const
{
  MdvxField *wfld = working.getField(field.name);
  const Mdvx::field_header_t &wfhdr = wfld->getFieldHeader();

  MdvxField *pfld = past.getField(field.name);
  const Mdvx::field_header_t &pfhdr = pfld->getFieldHeader();

  fl32 wmissing = wfhdr.missing_data_value;
  fl32 pmissing = pfhdr.missing_data_value;

  // constrain data within limits if appropriate

  _constrainField(field, nPointsField, past);

  // update max or accumulate into average
  fl32 *wff = (fl32 *) wfld->getVol();
  fl32 *pff = (fl32 *) pfld->getVol();

  if (_params.composite_type == Params::AVERAGE) {
    MdvxField *cfld = workingCounts.getField(field.name);
    fl32 *cff = (fl32 *) cfld->getVol();
    _updateAverage(wff, wmissing, pff, pmissing, cff, nPointsField);
  } else {
    _updateMax(wff, wmissing, pff, pmissing, nPointsField);
  }
}

//////////////////////////////////////////////////
// Finish the averaging by dividing sums by counts

void MdvTComp::_finishAveraging(const Params::field_t &field, int nPointsField,
				DsMdvx &working, DsMdvx &workingCounts) const
{
  MdvxField *fld = working.getField(field.name);
  const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
  fl32 missing = fhdr.missing_data_value;
  fl32 *ff = (fl32 *) fld->getVol();

  MdvxField *cfld = workingCounts.getField(field.name);
  fl32 *cc = (fl32 *) cfld->getVol();
	
  for (int j = 0; j < nPointsField; j++, ff++, cc++) {
    if (*ff != missing) {
      if (*cc != 0) {
	*ff = (*ff) / (*cc);
      } else {
	cerr << "ERROR - MdvTComp::Run" << endl;
	cerr << "Unexpected divide by zero, set data to misssing" << endl;
	*ff = missing;
      } 
    }
  }
}


//////////////////////////////////////////////////
// reset missing data to fill value if configured to do so, and convert
// output field data

void MdvTComp::_prepareOutput(const Params::field_t &field, int nPointsField,
			      DsMdvx &working) const
{
  MdvxField *fld = working.getField(field.name);
  const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      
  if (field.reset_missing) {
	
    fl32 *ff = (fl32 *) fld->getVol();
    fl32 missing = fhdr.missing_data_value;
    fl32 fillval = field.missing_fill_value;
	
    for (int j = 0; j < nPointsField; j++, ff++) {
      if (*ff == missing) {
	*ff = fillval;
      }
    }
  }
	
  // convert output
      
  fld->computeMinAndMax(true);
  fld->convertRounded((Mdvx::encoding_type_t) _params.output_encoding_type,
		      Mdvx::COMPRESSION_ZLIB);
}

////////////////////////////////////////////
// check that the fields are uniform in size

bool MdvTComp::_uniformFields(DsMdvx &mdvx, int &n_points) const
  
{

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  n_points = 0;

  const MdvxField *fld0 = mdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = fld0->getFieldHeader();
  int nPoints0 = fhdr0.nx * fhdr0.ny * fhdr0.nz;

  for (int i = 1; i < mhdr.n_fields; i++) {
    const MdvxField *fld = mdvx.getField(i);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    int nPoints = fhdr.nx * fhdr.ny * fhdr.nz;
    if (nPoints != nPoints0) {
      cerr << "ERROR - MdvTComp::_uniformFields()" << endl;
      cerr << "  Fields are not uniform in length" << endl;
      cerr << "  Npoints in first field: " << nPoints0 << endl;
      cerr << "  Npoints in field " << i << ": " << nPoints0 << endl;
      cerr << "  File: " << mdvx.getPathInUse() << endl;
      return false;
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "nfields: " << mhdr.n_fields << endl;
    cerr << "nx, ny, nz, npoints: "
	 << fhdr0.nx << ", " << fhdr0.ny << ", "
	 << fhdr0.nz << ", " << nPoints0 << endl;
    cerr << "vol len: " << fld0->getVolLen() << endl;
  }

  n_points = nPoints0;
  return true;

}

bool MdvTComp::_isWantedTime(const time_t &t) const
{
  DateTime dt(t);
  int hour = dt.getHour();
  int min = dt.getMin();
  int sec = dt.getSec();

  for (int i=0; i<_params.output_times_n; ++i) {
    if (hour == _params._output_times[i].hour &&
	min == _params._output_times[i].minute &&
	sec == _params._output_times[i].second) {
      return true;
    }
  }
  return false;
}

void _constrainField(const Params::field_t &field, int nPointsField,
		      DsMdvx &working)
{
  if (field.constrain_values) {
	
    MdvxField *fld = working.getField(field.name);

    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    fl32 *ff = (fl32 *) fld->getVol();
    fl32 missing = fhdr.missing_data_value;
    fl32 minval = field.min_value;
    fl32 maxval = field.max_value;
	
    for (int j = 0; j < nPointsField; j++, ff++) {
      fl32 fff = *ff;
      if (fff != missing) {
	if (fff < minval || fff > maxval) {
	  *ff = missing;
	}
      }
    } 
  } 
}


void _updateAverage(fl32 *wff, fl32 wmissing, fl32 *pff, fl32 pmissing, 
			   fl32 *cff, int nPointsField)
{
  for (int j = 0; j < nPointsField; j++, wff++, pff++, cff++) {
    fl32 wfff = *wff;
    fl32 pfff = *pff;
    fl32 cfff = *cff;
    if (pfff != pmissing) {
      if (wfff != wmissing) { 
	*wff = wfff + pfff;
      } else {
	*wff = pfff;
      }
      *cff = cfff + 1;
    }
  } // j
}

void _updateMax(fl32 *wff, fl32 wmissing, fl32 *pff, fl32 pmissing, 
		       int nPointsField)
{
  for (int j = 0; j < nPointsField; j++, wff++, pff++) {
    fl32 wfff = *wff;
    fl32 pfff = *pff;
    if (pfff != pmissing) {
      if (wfff != wmissing) { 
	if (pfff > wfff) {
	  *wff = pfff;
	}
      } else {
	*wff = pfff;
      }
    }
  } // j
}


