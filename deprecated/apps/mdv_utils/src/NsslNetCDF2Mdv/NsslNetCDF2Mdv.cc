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
/**
 * @file NsslNetCDF2Mdv.cc
 */

#include "NsslNetCDF2Mdv.hh"
#include "Args.hh"
#include "Params.hh"
#include "VolumeTrigger.hh"
#include "ElevVolumeTrigger.hh"
#include "StartTimeVolumeTrigger.hh"
#include "ScanVolumeTrigger.hh"
#include "VolNumVolumeTrigger.hh"
#include "SweepFile.hh"
#include "NsslData.hh"
#include "TimeElev.hh"

#include <Mdv/MdvxField.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/umisc.h>

#include <assert.h>
#include <vector>
#include <algorithm>


// Global variables

NsslNetCDF2Mdv *NsslNetCDF2Mdv::_instance = (NsslNetCDF2Mdv *)NULL;

//------------------------------------------------------------------------
NsslNetCDF2Mdv::NsslNetCDF2Mdv(int argc, char **argv) :
  _dataTrigger(0), _volumeTrigger(0)
{
  static const string method_name = "NsslNetCDF2Mdv::NsslNetCDF2Mdv()";
  
  // Make sure the singleton wasn't already created.
  assert(_instance == (NsslNetCDF2Mdv *)NULL);
  
  // Initialize the okay flag.
  okay = true;
  
  // Set the singleton instance pointer
  _instance = this;

  // Set the program name.
  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = progname_parts.base;
  
  // Display copyright message.
  // ucopyright(_progName.c_str());

  // Get the command line arguments.
  _args = new Args(argc, argv, _progName.c_str());
  
  // Get TDRP parameters.
  _params = new Params();
  char *params_path;
  if (_params->loadFromArgs(argc, argv, _args->override.list, &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    okay = false;
    return;
  }

  if (_params->debug_mode == Params::DEBUG)
  {
    LOG_STREAM_INIT(true, false, true, true);    
  }
  else if (_params->debug_mode == Params::DEBUG_VERBOSE)
  {
    LOG_STREAM_INIT(true, false, true, true);    
  }
  else
  {
    LOG_STREAM_INIT(false, false, true, true);
  }
}

//------------------------------------------------------------------------
NsslNetCDF2Mdv::~NsslNetCDF2Mdv(void)
{
  delete _params;
  delete _args;
  delete _dataTrigger;
  delete _volumeTrigger;
}


//------------------------------------------------------------------------
NsslNetCDF2Mdv *NsslNetCDF2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (NsslNetCDF2Mdv *)NULL)
  {
    new NsslNetCDF2Mdv(argc, argv);
  }  
  return(_instance);
}

//------------------------------------------------------------------------
NsslNetCDF2Mdv *NsslNetCDF2Mdv::Inst(void)
{
  assert(_instance != (NsslNetCDF2Mdv *)NULL);
  return(_instance);
}


//------------------------------------------------------------------------
bool NsslNetCDF2Mdv::init(void)
{
  // Initialize the data trigger
  DsInputDirTrigger *trigger = new DsInputDirTrigger();
  if (trigger->init(_params->input_dir, _params->input_substring,
		    true, NULL, true, _params->exclude_substring) != 0)
  {
    LOG(ERROR) << "initializing ARCHIVE trigger";
    return false;
  }
  _dataTrigger = trigger;
  
  // Initialize the volume trigger
  switch (_params->volume_trigger)
  {
  case Params::ELEVATION_VOL_TRIGGER :
    _volumeTrigger = new ElevVolumeTrigger();
    break;
    
  case Params::START_TIME_VOL_TRIGGER :
    _volumeTrigger = new StartTimeVolumeTrigger();
    break;
    
  case Params::SCAN_VOL_TRIGGER :
    _volumeTrigger = new ScanVolumeTrigger();
    break;
    
  case Params::VOL_NUMBER_VOL_TRIGGER :
    _volumeTrigger = new VolNumVolumeTrigger();
    break;
  }

  // Initialize the field specifications
  for (int i=0; i<_params->field_n; ++i)
  {
    FieldSpec f;
    f.input_path = _params->_field[i].input_field_path;
    f.field_name = _params->_field[i].field_name;
    f.missing_value_is_global = _params->_field[i].missing_value_is_global;
    f.missing_data_value_att_name = 
      _params->_field[i].missing_data_value_att_name;
    f.bias_specified = _params->_field[i].bias_specified;
    f.bias_att_name = _params->_field[i].bias_att_name;
    f.scale_att_name = _params->_field[i].scale_att_name;
    f.units_att_name = _params->_field[i].units_att_name;
    f.override_missing = _params->_field[i].override_missing;
    f.missing_data_value = _params->_field[i].missing_data_value;
    f.fix_missing_beams = _params->_field[i].fix_missing_beams;

    //note allow more than one field to go to same output location
    _fields[_params->_field[i].output_field_path].push_back(f);
  }
  return true;
}

//------------------------------------------------------------------------
void NsslNetCDF2Mdv::run(void)
{
  // for each file build up its field, time, and elevation
  vector<NsslData> nsslData;
  vector<TimeElev> timeElev;

  _formInputs(nsslData, timeElev);

  // loop through the output urls
  for (field_it i=_fields.begin(); i!=_fields.end(); ++i)
  {
    string outPath = i->first;
    vector<FieldSpec> field = i->second;
    LOG(DEBUG) << "Processing all data that goes to " << outPath;
    for (size_t j=0; j<field.size(); ++j)
    {
      LOG(DEBUG) << "     " << field[j].input_path;
    }
    _processFiles(outPath, field, nsslData, timeElev);
  }
}

//------------------------------------------------------------------------
void NsslNetCDF2Mdv::_formInputs(std::vector<NsslData> &nsslData,
				 std::vector<TimeElev> &timeElev) const
{
  vector<string> pathNames;
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      LOG(ERROR) << "getting next trigger information";
      continue;
    }
    pathNames.push_back(trigger_info.getFilePath());
  }
  LOG(DEBUG) << "Sorting";
  sort(pathNames.begin(), pathNames.end());

  for (size_t i=0; i<pathNames.size(); ++i)
  {
    NsslData data(*_params, pathNames[i]);
    if (data.ok())
    {
      nsslData.push_back(data);
      TimeElev te(data);
      if (find(timeElev.begin(), timeElev.end(), te) == timeElev.end())
      {
	timeElev.push_back(te);
      }
    }
  }

  for (size_t i=0; i<timeElev.size(); ++i)
  {
    timeElev[i].print();
  }
  sort(timeElev.begin(), timeElev.end());
  printf("After sort\n");
  for (size_t i=0; i<timeElev.size(); ++i)
  {
    timeElev[i].print();
  }
}

//------------------------------------------------------------------------
void NsslNetCDF2Mdv::_processFiles(const std::string &outPath, 
				   const std::vector<FieldSpec> &field, 
				   const std::vector<NsslData> &data,
				   const std::vector<TimeElev> &timeElev)
{
  if (data.empty() || timeElev.empty())
  {
    return;
  }

  // for each time/elev, see if there is any data that is wanted, and if so,
  // process it.  Keep track if data is missing.

  bool firstForThisOutput = true;
  for (size_t i=0; i<timeElev.size(); ++i)
  {
    if (_processTimeElev(timeElev[i], outPath, field, data,
			 firstForThisOutput, i+1 == timeElev.size()))
    {
      firstForThisOutput = false;
    }
  }
}

//------------------------------------------------------------------------
bool
NsslNetCDF2Mdv::_processTimeElev(const TimeElev timeElev,
				 const std::string &outPath,
				 const std::vector<FieldSpec> &field, 
				 const std::vector<NsslData> &data,
				 bool firstForThisOutput, bool lastTimeElev)
{
  // look through all fields for match to input time/elev
  // expect either no matchs, or a complete match
  vector<pair<NsslData,FieldSpec> > match;
  for (size_t j=0; j<field.size(); ++j)
  {
    string name = field[j].input_path;
    for (size_t k=0; k<data.size(); ++k)
    {
      if (data[k].isMatch(name, timeElev._time, timeElev._elev))
      {
	match.push_back(pair<NsslData,FieldSpec>(data[k],field[j]));
	break;
      }
    }
  }
  if (match.empty())
  {
    return false;
  }
  return _processFilesAtTime(outPath, match, firstForThisOutput,
			     lastTimeElev);
}

//------------------------------------------------------------------------
bool NsslNetCDF2Mdv::
_processFilesAtTime(const std::string &outPath,
		    std::vector<std::pair<NsslData,FieldSpec> > &data,
		    bool firstForThisOutput, bool lastTimeElev)
{
  LOG(DEBUG) << "Process files at time";
  for (size_t i=0; i<data.size(); ++i)
  {
    if (!_processFileAtTime(data[i].first, data[i].second, outPath,
			    firstForThisOutput, i==0,
			    lastTimeElev, i+1 == data.size()))
    {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------------
bool 
NsslNetCDF2Mdv::_processFileAtTime(NsslData &data, const FieldSpec &spec,
				   const std::string &outPath,
				   bool firstForThisOutput, 
				   bool firstField, bool lastTimeElev,
				   bool lastField)
{
  // Construct the sweep file paths.  If the given input file doesn't
  // exist, try prepending the input_dir specified in the parameter
  // file.
  if (!data.pathAdjust(_params->input_dir))
  {
    return false;
  }

  LOG(DEBUG) << " " << data.getPath();
  SweepFile sweep_file(data, spec, _params->num_gates_dim_name,
		       _params->data_driven_beamwidth,
		       _params->data_driven_allowed_beamwidth_n,
		       _params->_data_driven_allowed_beamwidth,
		       _params->output_beamwidth,
		       _params->force_negative_longitude);
  if (!sweep_file.initialize())
  {
    return false;
  }


  // See if this is a new volume.  If it is, we need to write out the old
  // one and start accumulating a new one.
  if (firstField)
  {
    if (firstForThisOutput)
    {
      // Clear out the MDV file to prepare for a new volume
      _mdvFile = DsMdvx();
    }
    else if (_volumeTrigger->isNewVolume(sweep_file))
    {
      if (!_finishVolume(sweep_file, outPath))
      {
	return false;
      }
    }
  }

  // Add the new sweep to the MDV file
  if (!sweep_file.addSweepToMdv(_mdvFile))
  {
    return false;
  }

  if (lastTimeElev && lastField)
  {
    return _finishVolume(sweep_file, outPath);
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------------
bool NsslNetCDF2Mdv::_finishVolume(const SweepFile &sweep_file,
				   const std::string &outPath)
{
  // Write the previous MDV file
  DateTime volume_end_time;
    
  if (!sweep_file.getVolumeStartTime(volume_end_time))
    return false;
    
  string radarName;
  if (!sweep_file.getRadarName(radarName))
    return false;
    
  if (!_writeMdvFile(_mdvFile, volume_end_time, radarName, outPath))
    return false;
  
  // Clear out the MDV file to prepare for a new volume
  _mdvFile = DsMdvx();
  return true;
}

//------------------------------------------------------------------------
bool NsslNetCDF2Mdv::_writeMdvFile(DsMdvx &mdv_file,
				   const DateTime &volume_end_time,
				   const std::string &radarName,
				   const std::string &outPath)
{
  Mdvx::master_header_t mh = mdv_file.getMasterHeader();

  // Compress all of the fields

  for (int field_num = 0; field_num < mdv_file.getMasterHeader().n_fields;
       ++field_num)
  {
    MdvxField *field = mdv_file.getField(field_num);
    
    if (_params->output_encoding == Params::INT8)
    {
      field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_GZIP);
    }
    else if (_params->output_encoding == Params::FLOAT32)
    {
      field->convertType(Mdvx::ENCODING_FLOAT32,
			 Mdvx::COMPRESSION_GZIP);
    }

  } /* endfor - field_num */
  
  // Update the times in the master header.  We are doing this here because
  // some of the files we got had bad time offset values.  So, rather than
  // relying on the time offset values we'll just assume that each volume
  // ends at the start of the next volume.

  Mdvx::master_header_t master_hdr = mdv_file.getMasterHeader();
  master_hdr.time_end = volume_end_time.utime();
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.time_centroid =
    (master_hdr.time_begin / 2) + (master_hdr.time_end / 2);
  mdv_file.setMasterHeader(master_hdr);
  
  // Write out the file

  string outname = "";
  if (_params->output_url_add_radar_name)
  {
    outname = _params->out_url_0;
    outname += radarName;
    outname += _params->out_url_1;
  }
  else
  {
    outname = _params->output_url;
  }

  outname += "/" + outPath;

  LOG(DEBUG) << DateTime::str(mh.time_centroid)  << "," << outname;
  if (mdv_file.writeToDir(outname) != 0)
  {
    LOG(ERROR) << "writing MDV file to URL: " << outname;
    LOG(ERROR) << mdv_file.getErrStr();
    return false;
  }
  
  return true;
}
