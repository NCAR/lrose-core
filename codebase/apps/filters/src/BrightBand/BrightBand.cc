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
//
// BrightBand object
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1997
//
///////////////////////////////////////////////////////////////

#include <signal.h>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>

#include "BrightBand.h"
#include "Interest_MDV.h"
#include "Filter.h"

using namespace std;

// Constructor

BrightBand::BrightBand(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;

  // set programe name

  name = STRdup("BrightBand");

  // print ucopyright

  ucopyright(name);

  // get command line args

  args = new Args(argc, argv, name);
  if (!args->OK) {
    cerr << "ERROR: " << name << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }
  if (args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  params = new Params();
  char *params_path = (char *) "unknown";
  
  if (params->loadFromArgs(argc, argv,
			   args->override.list,
			   &params_path))
  {
    cerr << "ERROR: " << name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    OK = FALSE;
    
    return;
  }

}

// destructor

BrightBand::~BrightBand()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  delete trigger;
  delete params;
  delete args;

  // delete other members

  STRfree(name);

  vector< Template_Interest* >::iterator ti_iter;
  for (ti_iter = _templateInterest.begin(); ti_iter != _templateInterest.end();
       ++ti_iter)
    delete *ti_iter;
  _templateInterest.clear();
  
  delete _reflInterest;
  delete _incrReflInterest;
  delete _maxInterest;
  delete _textureInterest;
  delete _clumps;
}


bool BrightBand::init()
{
  static const string method_name = "BrightBand::init()";
  
  // Check the TDRP parameters for consistency

  if (!_checkParams())
    return false;
  
  // initialize interest fields

  if (!_initInterestFields())
    return false;
  
  // input trigger object

  if (!_initTrigger())
    return false;
  
  // initialize process registration
  
  PMU_auto_init(name, params->instance, PROCMAP_REGISTER_INTERVAL);

  // Make the output directory. Exit if we can't do this.

  if ( ta_makedir_recurse(params->output_dir) ){
    cerr << "Failed to create output directory " << params->output_dir << endl;
    exit(-1);
  }

  return true;
}


// Run

bool BrightBand::Run ()
{
  while (!trigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (trigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    DsMdvx input_file;
    
    string input_file_path;

    if (time_trigger)
    {
      if (!_readFile(input_file, DateTime(trigger_info.getIssueTime())))
	return false;
    }
    else
    {
      if (!_readFile(input_file, trigger_info.getFilePath()))
	return false;
    }
    
    if (!_processFile(input_file))
      return false;
  }
  
  return true;
}


// _checkDbzField

bool BrightBand::_checkDbzField(MdvxField &dbz_field) 
{
  static const string method_name = "BrightBand::_checkDbzField()";
  
  // The field must contain constant Z levels

  Mdvx::field_header_t field_hdr = dbz_field.getFieldHeader();
  Mdvx::vlevel_header_t vlevel_hdr = dbz_field.getVlevelHeader();
  
  for (int z = 0; z < field_hdr.nz; ++z)
  {
    if (vlevel_hdr.type[z] != Mdvx::VERT_TYPE_Z)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "vlevel type not supported" << endl;

      return false;
    }
  }
  
  _inputEncoding = (Mdvx::encoding_type_t) field_hdr.encoding_type;
  _inputCompression = (Mdvx::compression_type_t) field_hdr.compression_type;

  if (params->output_encoding == Params::ENCODING_ASIS) {
    _outputEncoding = _inputEncoding;
  } else {
    switch (params->output_encoding) {
      case Params::ENCODING_INT8:
        _outputEncoding = Mdvx::ENCODING_INT8;
        break;
      case Params::ENCODING_FLOAT32:
        _outputEncoding = Mdvx::ENCODING_FLOAT32;
        break;
      default:
        _outputEncoding = Mdvx::ENCODING_INT16;
    }
  }

  if (params->output_compression == Params::COMPRESSION_ASIS) {
    _outputCompression = _inputCompression;
  } else {
    switch (params->output_compression) {
      case Params::COMPRESSION_NONE:
        _outputCompression = Mdvx::COMPRESSION_NONE;
        break;
      case Params::COMPRESSION_BZIP2:
        _outputCompression = Mdvx::COMPRESSION_BZIP;
        break;
      default:
        _outputCompression = Mdvx::COMPRESSION_GZIP;
    }
  }

  // uncompress dbz field, convert to float 32

  dbz_field.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  return true;

}


// _checkInputFile

bool BrightBand::_checkInputFile(DsMdvx &input_file)
{
  static const string method_name = "BrightBand::_checkInputFile()";
  
  // The file must use XYZ data ordering

  Mdvx::master_header_t master_hdr = input_file.getMasterHeader();
  
  if (master_hdr.data_ordering != Mdvx::ORDER_XYZ)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "MDV file must have XYZ data ordering" << endl;
    
    return false;
  }
  
  return true;
}


// _checkParams()

bool BrightBand::_checkParams()
{
  static const string method_name = "BrightBand::_checkParams()";
  
  // Check to make sure that our band bases aren't above our band tops

  if (params->input_template1_info.band_base_idex >
      params->input_template1_info.band_top_idex)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Template 1 band base index must not be greater than band top index" << endl;
    cerr << "base index = " << params->input_template1_info.band_base_idex << endl;
    cerr << "top index = " << params->input_template1_info.band_top_idex << endl;
    cerr << "Fix parameters and try again" << endl;
    
    return false;
  }
  
  if (params->input_template2_info.band_base_idex >
      params->input_template2_info.band_top_idex)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Template 2 band base index must not be greater than band top index" << endl;
    cerr << "base index = " << params->input_template2_info.band_base_idex << endl;
    cerr << "top index = " << params->input_template2_info.band_top_idex << endl;
    cerr << "Fix parameters and try again" << endl;
    
    return false;
  }
  
  if (params->input_template3_info.band_base_idex >
      params->input_template3_info.band_top_idex)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Template 3 band base index must not be greater than band top index" << endl;
    cerr << "base index = " << params->input_template3_info.band_base_idex << endl;
    cerr << "top index = " << params->input_template3_info.band_top_idex << endl;
    cerr << "Fix parameters and try again" << endl;
    
    return false;
  }
  
  return true;
}


// _createUrl()

string BrightBand::_createUrl(const string &input_source)
{
  // See if the string contains "mdvp:".  If it does, then it's assumed to
  // already be a URL.

  if (input_source.find("mdvp:") != string::npos)
    return input_source;
  
  // If we don't find the "mdvp:" substring, assume this is a directory
  // and add the URL stuff to the beginning

  return "mdvp:://localhost::" + input_source;
}


// _initInterestFields()

bool BrightBand::_initInterestFields()
{
  static const string method_name = "BrightBand::_initInterestFields()";
  
  _reflInterest =
    new Refl_Interest(params->input_template1_info,
		      params->input_template2_info,
		      params->input_template3_info,
		      params->debug);

  _incrReflInterest =
    new Incr_Refl_Interest(params->max_down,
			   params->max_up,
			   params->debug);
     
  int band_base1_idex = params->input_template1_info.band_base_idex - 1;
  int band_base2_idex = params->input_template2_info.band_base_idex - 1;
  int band_base3_idex = params->input_template3_info.band_base_idex - 1;
  int band_top1_idex = params->input_template1_info.band_top_idex - 1;
  int band_top2_idex = params->input_template2_info.band_top_idex - 1;
  int band_top3_idex = params->input_template3_info.band_top_idex - 1;

  Template_Interest *template_interest;
  vector< double > template_values;
  
  template_values.clear();
  for (int i = 0; i < params->input_template1_n; ++i)
    template_values.push_back(params->_input_template1[i]);
  template_interest = 
    new Template_Interest(template_values,
			  band_base1_idex,
			  band_top1_idex,
			  params->input_template1_info.min_refl_in_band,
			  params->input_template1_info.max_refl_in_band,
			  params->input_template1_info.compute_interest,
			  params->max_down, params->max_up,
			  params->debug);
  _templateInterest.push_back(template_interest);
     
  template_values.clear();
  for (int i = 0; i < params->input_template2_n; ++i)
    template_values.push_back(params->_input_template2[i]);
  template_interest = 
    new Template_Interest(template_values,
			  band_base2_idex,
			  band_top2_idex,
			  params->input_template2_info.min_refl_in_band,
			  params->input_template2_info.max_refl_in_band,
			  params->input_template2_info.compute_interest,
			  params->max_down, params->max_up,
			  params->debug);
  _templateInterest.push_back(template_interest);

  template_values.clear();
  for (int i = 0; i < params->input_template3_n; ++i)
    template_values.push_back(params->_input_template3[i]);
  template_interest = 
    new Template_Interest(template_values,
			  band_base3_idex,
			  band_top3_idex,
			  params->input_template3_info.min_refl_in_band,
			  params->input_template3_info.max_refl_in_band,
			  params->input_template3_info.compute_interest,
			  params->max_down, params->max_up,
			  params->debug);
  _templateInterest.push_back(template_interest);
  
  _maxInterest = new Max_Interest(params->debug);
     
  _textureInterest = new Texture_Interest(params->debug);

  _clumps = new Clump(params->interest_threshold,
		      params->area_threshold_sq_km,
		      params->debug);

  return true;
}


// _initTrigger()

bool BrightBand::_initTrigger()
{
  static const string method_name = "BrightBand::_initTrigger()";
  
  if (params->mode == Params::ARCHIVE)
  {
    if (args->nFiles > 0)
    {
      DsFileListTrigger *file_list_trigger = new DsFileListTrigger();
    
      vector< string > file_list;
      for (int i = 0; i < args->nFiles; ++i)
	file_list.push_back(args->filePaths[i]);
      
      if (file_list_trigger->init(file_list) != 0)
      {
	cerr << "ERROR: " << name << endl;
	cerr << "Error initializing file list trigger" << endl;

	return false;
      }

      trigger = file_list_trigger;
      time_trigger = false;
      
    }
    else if (args->startTime != 0 && args->endTime != 0)
    {
      DsTimeListTrigger *time_list_trigger = new DsTimeListTrigger();
      string input_url = _createUrl(params->input_data_dir);
      
      if (time_list_trigger->init(input_url,
				  args->startTime, args->endTime) != 0)
      {
	cerr << "ERROR: " << name << endl;
	cerr << "Error initializing time list trigger" << endl;
	cerr << "URL = " << input_url << endl;
	cerr << "start time = " << DateTime::str(args->startTime) << endl;
	cerr << "end time = " << DateTime::str(args->endTime) << endl;
	
	return false;
      }
      
      trigger = time_list_trigger;
      time_trigger = true;
      
    }
    else
    {
      cerr << "ERROR: " << name << endl;
      cerr << "In ARCHIVE mode you must either specify a file list using" << endl;
      cerr << "-f or the start and end times using -start and -end" << endl;

      return false;
    }

  }
  else
  {
    DsLdataTrigger *ldata_trigger = new DsLdataTrigger();
      
    if (ldata_trigger->init(params->input_data_dir,
			    params->max_realtime_valid_age,
			    PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << name << endl;
      cerr << "Error initializing realtime trigger." << endl;

      return false;
    }
    
    trigger = ldata_trigger;
    time_trigger = true;
  }

  return true;
}


// _processFile

bool BrightBand::_processFile(DsMdvx &input_file)
{
  if (params->debug)
    cerr << endl
	 << "*** Processing file " << input_file.getPathInUse() << endl;
  
  // Find the DBZ field in the file

  MdvxField *dbz_field = input_file.getField(params->dbz_label);
  if (dbz_field == NULL) {
    cerr << "ERROR - BrightBand::_processFile" << endl;
    cerr << "  Input file does not have dbz field, label: "
         << params->dbz_label << endl;
    cerr << "  File: " << input_file.getPathInUse() << endl;
    return false;
  }

  if (!_checkDbzField(*dbz_field))
    return false;
  
  // Output data classes
  Interest_MDV *interest_mdv;

  // processing variables
  int num_big_clumps;
  int band_base1_idex = params->input_template1_info.band_base_idex - 1;
  int band_base2_idex = params->input_template2_info.band_base_idex - 1;
  int band_base3_idex = params->input_template3_info.band_base_idex - 1;
  int band_top1_idex = params->input_template1_info.band_top_idex - 1;
  int band_top2_idex = params->input_template2_info.band_top_idex - 1;
  int band_top3_idex = params->input_template3_info.band_top_idex - 1;
  
  // timing variables
  unsigned long end_time, start_time = UTIMtime_diff(0);
  
  PMU_auto_register("Input new file");
    
  // reflectivity interest
  
  if (!_reflInterest->calcInterestFields(dbz_field))
    return false;
  
  // reflectivity interest

  if (!_incrReflInterest->calcInterestFields(dbz_field))
    return false;
  
  // template interest

  vector< Template_Interest*> ::iterator ti_iter;
  
  for (ti_iter = _templateInterest.begin(); ti_iter != _templateInterest.end();
       ++ti_iter)
  {
    if (!(*ti_iter)->calcInterestFields(dbz_field,
					_reflInterest->interest_values,
					_incrReflInterest->interest_values))
      return false;
  }
  
  // max interest based on template interest values

  if (!_maxInterest->calcInterestFields(dbz_field,
					_templateInterest))
    return false;
  
  // texture interest based on max interest

  if (!_textureInterest->calcInterestFields(dbz_field,
					    _maxInterest->interest_values,
					    _maxInterest->ht))
    return false;
  
  // find clumps in the combined interest field

  if (!_clumps ->calcInterestFields(dbz_field,
				    _textureInterest->interest_values,
				    _textureInterest->ht))
    return false;

  num_big_clumps = _clumps->Get_num_big_clumps();

  if (params->debug)
  {
    cerr << "number of big clumps = " << num_big_clumps << endl;
  }

  // write interest fields to mdv file
  switch (params->write_interest)
  {
  case Params::ALWAYS:

    interest_mdv = 
      new Interest_MDV(params->interest_dir, &input_file,
		       _reflInterest, 
		       _incrReflInterest,
		       _templateInterest,
		       _maxInterest,
		       _textureInterest,
		       _clumps,
		       params->debug);

    delete(interest_mdv);
    break;
	
  case Params::BRIGHTBAND_FOUND:

    if (num_big_clumps > 0)
    {
      interest_mdv = 
	new Interest_MDV(params->interest_dir, &input_file,
			 _reflInterest,
			 _incrReflInterest, 
			 _templateInterest,
			 _maxInterest,
			 _textureInterest,
			 _clumps,
			 params->debug);
      
      delete(interest_mdv);
    }
    break;
	  
  case Params::NEVER:
        
    break;
	
  } /* endswitch - params->write_interest */

  // filter data and write new mdv file

  Filter *filter_data = new Filter(params->output_dir, &input_file,
				   dbz_field,
				   num_big_clumps,
				   _clumps->interest_values,
				   _maxInterest->field_used,
				   _maxInterest->ht,
				   band_base1_idex,
				   band_top1_idex,
				   band_base2_idex,
				   band_top2_idex,
				   band_base3_idex,
				   band_top3_idex,
				   params->debug,
                                   _outputEncoding,
                                   _outputCompression);
     
     
  delete filter_data;
     
  if (params->debug)
  {
    end_time = UTIMtime_diff(0);
    cerr << "Loop time in milliseconds: "
	 << (UTIMtime_diff(0) - start_time) << endl;
    start_time = end_time;
  }
     
  return true;
}


// _readFile

bool BrightBand::_readFile(DsMdvx &input_file, const DateTime &data_time)
{
  static const string method_name = "BrightBand::_readFile()";
  
  // Set up the read request

  input_file.setReadTime(Mdvx::READ_CLOSEST,
			 params->input_data_dir,
			 0, data_time.utime());
  
  // Read the volume

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file:" << endl;
    cerr << "   URL = " << params->input_data_dir << endl;
    cerr << "   time = " << data_time << endl;
    cerr << input_file.getErrStr() << endl;

    return false;
  }
  
  // Check to make sure we can process this file

  if (!_checkInputFile(input_file))
    return false;
  
  return true;
}


bool BrightBand::_readFile(DsMdvx &input_file, const string &input_path)
{
  static const string method_name = "BrightBand::_readFile()";
  
  // Set up the read request

  input_file.setReadPath(input_path);
  
  // Read the volume

  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input file:" << endl;
    cerr << "   path = " << input_path << endl;
    cerr << input_file.getErrStr() << endl;
    
    return false;
  }
  
  // Check to make sure we can process this file

  if (!_checkInputFile(input_file))
    return false;
  
  return true;
}
