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
////////////////////////////////////////////////////////////
// Mdv2NetCDF.cc
//
// Mdv2NetCDF object
//
// Sue Dettling, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2007
//
///////////////////////////////////////////////////////////////
//
// Mdv2NetCDF converts Mdv format to CF-1.0 compliant NetCDF files.
//
///////////////////////////////////////////////////////////////////////

#include "Mdv2NetCDF.hh"
#include <toolsa/Path.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxRadar.hh>
#include <Radx/DoradeRadxFile.hh>
using namespace std;

/////////////////////////////////////////////////
// Constructor

Mdv2NetCDF::Mdv2NetCDF(int argc, char **argv)
{

  // initialize

  isOK = true;
  _trigger = NULL;
  _commentWasSet = false;
  _comment = "";

  // set programe name

  _progName = "Mdv2NetCDF";

  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params

  _paramsPath = (char *) "unknown";

  if (_params.loadFromArgs(argc, argv, _args.override.list,
                           &_paramsPath))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
    return;
  }

  // If an additional TDRP file was specified, load that
  // over the existing params.

  if (NULL != _args.additional_tdrp_file){

    if (_params.debug){
      cerr << "Attempting to load additional param file "
           << _args.additional_tdrp_file << endl;
    }

    if (_params.load(_args.additional_tdrp_file, NULL, TRUE, FALSE)){
      cerr << "ERROR: " << _progName << endl;
      cerr << "Problem with TDRP parameters in file: "
           << _args.additional_tdrp_file << endl;
      isOK = false;
      return;
    }
  }

  // set up trigger

  if (_setUpTrigger() || _trigger == NULL) {
    cerr << "ERROR - Mdv2NetCDF" << endl;
    cerr << "  No input data" << endl;
    isOK = FALSE;
    return;
  }

// init process mapper registration
  int pmuRegSec = PROCMAP_REGISTER_INTERVAL;

  if(_params.procmap_register_interval_secs > PROCMAP_REGISTER_INTERVAL) {
    pmuRegSec = _params.procmap_register_interval_secs;
  }

  PMU_auto_init((char *) _progName.c_str(),
                _params.instance,
                pmuRegSec);

}

//////////////////////////////////////////////////////
//
// destructor

Mdv2NetCDF::~Mdv2NetCDF()

{

  // unregister process

  PMU_auto_unregister();

  if( _trigger) {
    delete _trigger;
  }

}

//////////////////////////////////////////////////
//
// Run

int Mdv2NetCDF::Run ()
{

  int iret = 0;

  // register with procmap

  PMU_auto_register("Run");
  
  while (!_trigger->endOfData())
    {
      TriggerInfo triggerInfo;
      _trigger->next(triggerInfo);
      
      if (_processData(triggerInfo.getIssueTime(), 
		       triggerInfo.getForecastTime() - triggerInfo.getIssueTime(), 
		       triggerInfo.getFilePath()))
	{
	  cerr << "Mdv2NetCDF::Run" <<"  Errors in processing time: "
	       <<  triggerInfo.getIssueTime()
	       << " input file: " << triggerInfo.getFilePath() << endl;
	  
	  iret = 1;
	}
    } // while
  
  return iret;

}


/////////////////////////////////////////
// Set up triggering mechanisme
//
// Returns 0 on success, 1 on failure

int Mdv2NetCDF::_setUpTrigger()

{
  
  if (_params.mode == Params::TIME_INTERVAL)
  {

    // Create archive timelist trigger

    if (_params.debug)
    {
      cerr << "Mdv2NetCDF::Run(): Creating ARCHIVE trigger\n"
           << "     start time: " << _args.startTime
           << "     end time:   " << _args.endTime << endl;
    }

    DsTimeListTrigger *archive_trigger = new DsTimeListTrigger();

    if (archive_trigger->init(_params.mdv_url,
                              _args.startTime,
                              _args.endTime) != 0)
    {
      cerr << archive_trigger->getErrStr();
      _trigger = 0;
      return 1;
    }
    else
    {
      _trigger = archive_trigger;
    }
  }
  else  if ( _params.mode == Params::FILELIST )
  {

    // Archive filelist mode.

    if ( _args.inputFileList.size() )
    {
      if (_params.debug)
        cerr << "FileInput::init: Initializing archive FILELIST mode." << endl;

      DsFileListTrigger *file_trigger = new DsFileListTrigger();

      if  (file_trigger->init( _args.inputFileList ) )
      {
        cerr << file_trigger->getErrStr();
        _trigger = 0;
        return 1;
      }
      else
        _trigger = file_trigger;
    }
  }
  else if (_params.mode == Params::REALTIME || 
           _params.mode == Params::REALTIME_FCST_DATA)
  {
    if (_params.debug)
    {
      cerr << "Mdv2NetCDF::Run(): Creating REALTIME trigger" << endl;
    }

    // realtime mode

    DsLdataTrigger *realtime_trigger = new DsLdataTrigger();

    if (realtime_trigger->init(_params.mdv_url,
                               _params.max_valid_realtime_age,
                               PMU_auto_register))
    {
      cerr << realtime_trigger->getErrStr();
      _trigger = 0;
      return 1;
    }
    else
    {
      _trigger = realtime_trigger;
    }
  }

  else if (_params.mode == Params::SPEC_FCST_REALTIME)
  {
    if (_params.debug)
    {
      cerr << "Mdv2NetCDF::Run(): Creating SPEC_FCAST_REALTIME trigger for "
           <<  _params.mdv_url  << endl;
    }
      
    DsSpecificFcstLdataTrigger *spec_trigger =
      new DsSpecificFcstLdataTrigger();
      
    vector< int > fcast_lead_times;

    fcast_lead_times.push_back(_params.fcast_lead_time.lead_time_secs);

    if (spec_trigger->init(_params.mdv_url,
                           fcast_lead_times,
                           _params.fcast_lead_time.use_gen_time,
                           7200, PMU_auto_register) != 0)
    {
      cerr << spec_trigger->getErrStr() << endl;
	  
      _trigger = 0;
    }
    else
    {
      _trigger = spec_trigger;
    }
  }
  else
    return 1;

  return 0;

}

///////////////////////////////////
//  process data at trigger time

int Mdv2NetCDF::_processData(time_t inputTime, int leadTime,
                             const string filepath)
{

  // registration with procmap

  PMU_force_register("Processing data");

  if (_params.debug)
  {
    cerr << "Mdv2NetCDF::_processData: Processing time: "
         << DateTime::strm(inputTime) << endl;
    cerr << " file trigger: " << filepath << endl;
  }
  
  // Read mdv file.
  
  DsMdvx mdvx;
  mdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
  if (_readMdv(inputTime, leadTime, filepath, mdvx)) {
    cerr << "ERROR - Mdv2NetCDF::_processData" << endl;
    return 1;
  }
  if (mdvx.getMasterHeader().n_fields < 1) {
    cerr << "ERROR - Mdv2NetCDF::_processData" << endl;
    cerr << "  No fields in file: " << mdvx.getPathInUse() << endl;
    return 1;
  }
  
  // initialize translation
  if (!_commentWasSet) {
    _comment = _params.global_attributes.comment;
  }

  mdvx.setMdv2NcfAttr(_params.global_attributes.institution,
                      _params.global_attributes.references,
                      _comment);
  
  for (int ii = 0; ii < _params.mdv_ncf_fields_n; ii++) {
    const Params::mdv_ncf_field_t &mnfld = _params._mdv_ncf_fields[ii];
    DsMdvx::ncf_pack_t packing = DsMdvx::NCF_PACK_ASIS;
    if (mnfld.packed_data_type == Params::DATA_PACK_NONE) {
      packing = DsMdvx::NCF_PACK_FLOAT;
    } else if (mnfld.packed_data_type == Params::DATA_PACK_BYTE) {
      packing = DsMdvx::NCF_PACK_BYTE;
    } else if (mnfld.packed_data_type == Params::DATA_PACK_SHORT) {
      packing = DsMdvx::NCF_PACK_SHORT;
    }
    mdvx.addMdv2NcfTrans(mnfld.mdv_field_name,
                         mnfld.ncf_field_name,
                         mnfld.ncf_standard_name,
                         mnfld.ncf_long_name,
                         mnfld.ncf_units,
                         mnfld.do_linear_transform,
                         mnfld.linear_multiplier,
                         mnfld.linear_const,
                         packing);
  } // ii
  
  if (_params.file_format == Params::CLASSIC) {
    mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_CLASSIC);
  } else if (_params.file_format == Params::NC64BIT) {
    mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_OFFSET64BITS);
  } else if  (_params.file_format == Params::NETCDF4_CLASSIC) {
    mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCFD4_CLASSIC);
  } else {
    mdvx.setMdv2NcfFormat(DsMdvx::NCF_FORMAT_NETCDF4);
  }

  mdvx.setMdv2NcfCompression(_params.compress_data,
                             _params.compression_level);
  
  mdvx.setMdv2NcfOutput(_params.output_latlon_arrays,
                        _params.output_mdv_attributes,
                        _params.output_mdv_chunks);

  // perform the translation

  if (mdvx.getProjection() == Mdvx::PROJ_POLAR_RADAR && 
      _params.radial_file_type != Params::FILE_TYPE_CF) {

    // specified radial data type
    
    Mdv2NcfTrans trans;
    trans.setDebug(_params.debug);
    if (_params.radial_file_type == Params::FILE_TYPE_DORADE) {
      trans.setRadialFileType(DsMdvx::RADIAL_TYPE_DORADE);
    } else if (_params.radial_file_type == Params::FILE_TYPE_UF) {
      trans.setRadialFileType(DsMdvx::RADIAL_TYPE_UF);
    } else {
      trans.setRadialFileType(DsMdvx::RADIAL_TYPE_CF_RADIAL);
    }
    _outputDir = _params.output_dir;
    if (trans.translateToCfRadial(mdvx, _outputDir)) {
      cerr << "ERROR - Mdv2NetCDF::_processData()" << endl;
      cerr << trans.getErrStr() << endl;
      return 1;
    }
    string outputPath = trans.getNcFilePath();
    // write latest data info
    _writeLdataInfo(mdvx, outputPath);

  } else {

    // basic CF

    string outputPath = _computeOutputPath(mdvx);
    Mdv2NcfTrans trans;
    trans.setDebug(_params.debug);
    trans.setRadialFileType(DsMdvx::RADIAL_TYPE_CF);
    if (trans.translate(mdvx, outputPath)) {
      cerr << "ERROR - Mdv2NetCDF::_processData()" << endl;
      cerr << trans.getErrStr() << endl;
      return 1;
    }
    
    // write latest data info
    _writeLdataInfo(mdvx, outputPath);
    
  }
    
  return 0;

}

/////////////////////////////////////////////////////////
// Read mdv file.

int Mdv2NetCDF::_readMdv(time_t requestTime, int leadTime,
                         const string filepath, DsMdvx &mdvx)

{

  // Set up for reading mdv data, reinitialize DsMdvx object

  mdvx.clearRead();
  mdvx.setReadFieldFileHeaders();
  
  if (_params.debug == Params::DEBUG_VERBOSE) {
    mdvx.setDebug(_params.debug);
  }

  if ( _params.mode == Params::FILELIST )
  {
    mdvx.setReadPath(filepath);
  }
  else if (_params.mode == Params::REALTIME_FCST_DATA)
  {
   mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,_params.mdv_url, 0, requestTime, leadTime);
  }
  else
  {
    mdvx.setReadTime(Mdvx::READ_CLOSEST, _params.mdv_url, 300, requestTime, leadTime);
  }

  // Read only the fields listed in the parameter file

  for (int i = 0; i < _params.mdv_ncf_fields_n; i++)
  {
    mdvx.addReadField(_params._mdv_ncf_fields[i].mdv_field_name);
  }
  
  if (_params.debug == Params::DEBUG_VERBOSE) 
  {
    cerr << "Mdv2NetCDF::_readMdv() : Reading data for URL: "
         << _params.mdv_url << endl;
    mdvx.printReadRequest(cerr);
  }
  
  // perform the read
  if (mdvx.readVolume()) 
  {
    cerr << "Mdv2NetCDF::readMdv(): ERROR: Cannot read data for url: "
         << _params.mdv_url << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return 1;
  }
  // see if we have the comment chunk, if so replace the ncf comment

  _checkForComment(mdvx);
  return 0;

}

//////////////////////////////////////
// Compute output path for netCDF file

string Mdv2NetCDF::_computeOutputPath(const DsMdvx &mdvx)
{

  // Get the proper time to assign to filename

  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();
  DateTime validTime(mhdr.time_centroid);
  DateTime genTime(mhdr.time_gen);
  
  bool isForecast = false;
  int year, month, day, hour, minute, seconds;
  
  if (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
      mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
      mhdr.forecast_time > 0) {
    year = genTime.getYear();
    month = genTime.getMonth();
    day =  genTime.getDay();
    hour = genTime.getHour();
    minute = genTime.getMin();
    seconds = genTime.getSec();
    isForecast = true;
  } else {
    year = validTime.getYear();
    month = validTime.getMonth();
    day =  validTime.getDay();
    hour = validTime.getHour();
    minute = validTime.getMin();
    seconds = validTime.getSec();
  }

  if(!_params.output_as_forecast){
    isForecast = false;
  }

  // determine if we have polar radar data

  bool isPolar = false;
  bool isRhi = false;
  bool isSector = false;
  int nSweeps = 1;
  double fixedAngle = 0.0;
  if (mdvx.getNFields() > 0) {
    const MdvxField *field = mdvx.getField(0);
    const Mdvx::field_header_t &fhdr = field->getFieldHeader();
    if (fhdr.proj_type == Mdvx::PROJ_POLAR_RADAR) {
      isPolar = true;
      if (fhdr.vlevel_type == Mdvx::VERT_TYPE_AZ) {
        isRhi = true;
      } else {
        double angleCoverage = fhdr.ny * fhdr.grid_dy;
        if (angleCoverage < 330) {
          isSector = true;
        }
      }
      nSweeps = fhdr.nz;
      if (nSweeps == 1) {
        const Mdvx::vlevel_header_t &vhdr = field->getVlevelHeader();
        fixedAngle = vhdr.level[0];
      }
    }
  } // if (mdvx.getNFields() > 0) 

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  isPolar: " << (isPolar?"Y":"N") << endl;
    cerr << "  isRhi: " << (isRhi?"Y":"N") << endl;
    cerr << "  isSector: " << (isSector?"Y":"N") << endl;
    cerr << "  fixedAngle: " << fixedAngle << endl;
  }

  // compute output dir

  _outputDir = _params.output_dir;
  if (_params.write_to_day_dir) {
    char dayStr[128];
    sprintf(dayStr, "%.4d%.2d%.2d", year, month, day);
    _outputDir += PATH_DELIM;
    _outputDir += dayStr;
  }

  // ensure output dir exists
  
  if (ta_makedir_recurse(_outputDir.c_str())) {
    cerr << "ERROR - Mdv2NetCDF::_initNc3File()" << endl;
    cerr << "  Cannot make output dir: " << _outputDir;
  }

  // Create output filepath
  int leadTime = mhdr.forecast_delta;
  char outputPath[1024];
  
  if (_params.use_output_filename) {
    
    sprintf(outputPath, "%s/%s", _outputDir.c_str(), _params.output_filename);
    
  } else if (_params.use_iso8601_filename_convention) {
    
    if (isForecast) { 
      int leadTimeHrs = leadTime/3600;
      int leadTimeMins = (leadTime % 3600 )/ 60;
      sprintf(outputPath, "%s/%s%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d.PT%.2d:%.2d%s.nc",
              _outputDir.c_str(), _params.output_file_prefix, _params.basename,
              year, month, day, hour, minute, seconds,
              leadTimeHrs, leadTimeMins, _params.output_file_suffix);   
    } else {
      sprintf(outputPath, "%s/%s%s%.4d-%.2d-%.2dT%.2d:%.2d:%.2d%s.nc",
              _outputDir.c_str(), _params.output_file_prefix, _params.basename,
              year, month, day, hour, minute, seconds, _params.output_file_suffix);
    }
    
  } else if (_params.use_TDS_filename_convention) {
    
    if (isForecast) { 
      int leadTimeHrs = leadTime/3600;
      int leadTimeMins = (leadTime % 3600 )/ 60;
      sprintf(outputPath, "%s/%s%s%.4d%.2d%.2d_%.2d%.2d%.2d_f%.2d%.2d%s.nc",
              _outputDir.c_str(), _params.output_file_prefix, _params.basename,
              year, month, day, hour, minute, seconds,
              leadTimeHrs, leadTimeMins, _params.output_file_suffix);   
    } else {
      sprintf(outputPath, "%s/%s%s%.4d%.2d%.2d_%.2d%.2d%.2d%s.nc",
              _outputDir.c_str(), _params.output_file_prefix, _params.basename,
              year, month, day, hour, minute, seconds, _params.output_file_suffix);
    }
    
  } else {
    
    string basename = string(_params.basename);
    string v_yyyymmdd = validTime.getDateStrPlain();
    string v_hhmmss = validTime.getTimeStrPlain();
    char filename[1024];
    
    if (isForecast) { 
      if(basename.empty()) {
	sprintf(filename, "%sf_%08i%s.nc",
		_params.output_file_prefix,
		leadTime,
		_params.output_file_suffix);
      } else {
	sprintf(filename, "%s%sf_%08i%s.nc",
		_params.output_file_prefix, _params.basename,  
		leadTime,
		_params.output_file_suffix);
      }
      string g_hhmmss = genTime.getTimeStrPlain();
      sprintf(outputPath, "%s/g_%s/%s",
              _outputDir.c_str(), g_hhmmss.c_str(), filename);
    } else {
      if(basename.empty()) {
	sprintf(filename, "%s%s_%s%s.nc",
		_params.output_file_prefix,
		v_yyyymmdd.c_str(), v_hhmmss.c_str(),
		_params.output_file_suffix);
      } else {
	sprintf(filename, "%s%s%s_%s%s.nc",
		_params.output_file_prefix, _params.basename,  
		v_yyyymmdd.c_str(), v_hhmmss.c_str(),
		_params.output_file_suffix);
      }
      sprintf(outputPath, "%s/%s", _outputDir.c_str(), filename);
    }
    
  }

  return outputPath;
  
}

//////////////////////////////////////

void Mdv2NetCDF::_writeLdataInfo(const DsMdvx &mdvx, const string &outputPath)
{
  
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  // Write LdataInfo file

  DsLdataInfo ldata(_params.output_dir, _params.debug);
 
  ldata.setWriter("Mdv2NetCDF");
  ldata.setDataFileExt("nc");
  ldata.setDataType("netCDF");

  string fileName;
  Path::stripDir(_params.output_dir, outputPath, fileName);
  ldata.setRelDataPath(fileName);
  
  if ( (mhdr.data_collection_type == Mdvx::DATA_EXTRAPOLATED ||
        mhdr.data_collection_type == Mdvx::DATA_FORECAST ||
	mhdr.forecast_time > 0) 
      && !_params.force_ldata_as_valid)
  {
    ldata.setIsFcast(true);
    int leadtime = mhdr.forecast_delta;
    ldata.setLeadTime(leadtime);
    ldata.write(mhdr.time_gen);
  }
  else
  {
    ldata.setIsFcast(false);
    ldata.write(mhdr.time_centroid);
  }
  
  if (_params.debug) {
    cerr << "Mdv2NetCDF::_writeLdataInfo(): Data written to "
         << outputPath << endl;
  }

}

//////////////////////////////////////

void Mdv2NetCDF::_checkForComment(DsMdvx &mdvx)
{
  _commentWasSet = false;
  _comment = "";

  int nchunks = mdvx.getNChunks();
  for (int i=0; i<nchunks; ++i)
  {
    MdvxChunk *c = mdvx.getChunkByNum(i);
    if (c->getId() == Mdvx::CHUNK_COMMENT)
    {
      // make sure it is null terminated by doing a copy
      int n = c->getSize();
      const char *d = (const char *)c->getData();
      char *buf = new char[n+1];
      strncpy(buf, d, n);
      _commentWasSet = true;
      _comment = buf;
      delete [] buf;
      return;
    }
  }
}

