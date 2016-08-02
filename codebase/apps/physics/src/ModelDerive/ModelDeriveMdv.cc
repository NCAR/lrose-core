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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/* RCS info
 *   $Author: dixon $
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.8 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ModelDeriveMdv: Object for handling MDV files with the ModelDerive
 * program.  This object handles input and calling derive field 
 * functions while OutputMdv handles writing and vertical interpolation.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 *********************************************************************/

#include "ModelDeriveMdv.hh"
#include "DeriveInterface.hh"

ModelDeriveMdv::ModelDeriveMdv(Params *params)
{
  _params = params;
}

ModelDeriveMdv::~ModelDeriveMdv()
{

}


/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool ModelDeriveMdv::processData(TriggerInfo &trigger_info)
{
  static const string method_name = "ModelDerive::_processData()";

  //
  // Read in the input file
  //
  DsMdvx input_mdv;

  if (!_readMdvFile(input_mdv, trigger_info))
    return false;

  bool forecast = isForecastFile((input_mdv.getPathInUse()).c_str());

  OutputMdv output_mdv(_params, forecast);

  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeader();


  //
  // Loop over the requested derive functions processing each
  //
  for (int a = 0; a < _params->derive_functions_n1; a++)
  {
    cout << "Processing function " << _params->__derive_functions[a][0] << " ..." << endl;

    vector<float *> input_data;
    vector<float *> output_data;

    Mdvx::field_header_t field_hdr;
    Mdvx::vlevel_header_t vlevel_hdr;

    int nx, ny, nz;
    float missing, bad;
    char *input_units;

    for(int b = 1; b < _params->derive_functions_n2; b++)
    {
      if(strlen(_params->__derive_functions[a][b]) == 0)
	continue;

      //
      // get the field
      MdvxField *field = input_mdv.getField(_params->__derive_functions[a][b]);
      if(field == 0)
      {
	field = output_mdv.getField(_params->__derive_functions[a][b]);
	if(field == 0) {
	  cerr << "Error unable to retrieve field " << _params->__derive_functions[a][b]
	       << " for derive function " << _params->__derive_functions[a][0] << endl;
	  return false;
	}
      }
      
      //
      // verify the field
      field_hdr = (*field).getFieldHeader();
      vlevel_hdr = (*field).getVlevelHeader();

      if(b == 1) {
	nx = field_hdr.nx;
	ny = field_hdr.ny;
	nz = field_hdr.nz;
	missing = field_hdr.missing_data_value;
	bad = field_hdr.bad_data_value;
	input_units = field_hdr.units;
      } else {
	if(field_hdr.nx != nx || field_hdr.ny != ny || field_hdr.nz != nz) {
	  cerr << "Error input fields for function " << _params->__derive_functions[a][0]
	       << " do not have identical dimensions. " << endl;
	  return false;
	}
	if(field_hdr.missing_data_value != missing || field_hdr.bad_data_value != bad) {
	  cerr << "Warning input fields for function " << _params->__derive_functions[a][0]
	       << " do not have identical missing/bad data values." << endl;
	}
      }
     
      input_data.push_back((float *)field->getVol());
      
    }

    DeriveBase *deriveFunction = 
      DeriveInterface::getDeriveClassFromName(_params->__derive_functions[a][0],
					      missing, bad, nx, ny, nz);

    if(deriveFunction == NULL) {
      cerr << "Error retreiving derive function class for function " << _params->__derive_functions[a][0]
	   << endl;
      return false;
    }

    const vector<string> *shortNames = DeriveBase::getOutputShortNames();
    const vector<string> *longNames = DeriveBase::getOutputLongNames();
    const vector<string> *units = DeriveBase::getOutputUnits();

    //
    // Run the derive function
    deriveFunction->derive(&input_data, &output_data);

    //
    // Look for the vertical level name from the input field name
    char *levelName = stripLevelName(_params->__derive_functions[a][1]);


    //
    // Save the output fields 
    for(int d = 0; d < output_data.size(); d++) {
      //
      // If the output field units are "inputs" set its units to the inputs units
      if(strcasecmp((*units)[d].c_str(), "inputs") == 0) {
	output_mdv.createMdvxField(field_hdr, vlevel_hdr, output_data[d],
				   (*shortNames)[d].c_str(), (*longNames)[d].c_str(), 
				   input_units, levelName);
      } else {
	output_mdv.createMdvxField(field_hdr, vlevel_hdr, output_data[d],
				   (*shortNames)[d].c_str(), (*longNames)[d].c_str(), 
				   (*units)[d].c_str(), levelName);
      }
      delete[] output_data[d];
    }

    delete deriveFunction;

  }

  //
  // Copy over requested fields
  //
  for(int a = 0; a < fields_to_copy.size(); a++) {

      MdvxField *field = input_mdv.getField(fields_to_copy[a]);
      if(field == 0)
      {
	cerr << "Error unable to retrieve field " << fields_to_copy[a]
	     << " from input mdv file." << endl;
	  return false;
      }
      output_mdv.addField(new MdvxField(*field));
  }

  input_mdv.clear();

  char dataSetInfo[] = "Produced by ModelDerive";

  output_mdv.writeVol(master_hdr, dataSetInfo, input_mdv.getPathInUse().c_str());

  fields_to_copy.clear();

  return true;
}




/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/


/*********************************************************************
 * _readMdvFile() - Read the MDV file for the given time.
 */

bool ModelDeriveMdv::_readMdvFile(DsMdvx &input_mdv,
                            TriggerInfo &trigger_info)
{
  static const string method_name = "ModelDerive::_readMdvFile()";

  input_mdv.clearRead();

  //
  // Set up the read request
  if(_params->trigger_mode == Params::FILE_LIST)
  {
    input_mdv.setReadPath(trigger_info.getFilePath());
  }
  else if(_params->trigger_mode == Params::LATEST_DATA )
  {
    input_mdv.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params->input_path, 0,
			  trigger_info.getIssueTime(), 
			  trigger_info.getForecastTime() - trigger_info.getIssueTime());
  } else {
    input_mdv.setReadTime(Mdvx::READ_CLOSEST,
			  _params->input_path,
			  0, trigger_info.getIssueTime());
  }

  //
  // Read the file's headers
  if(input_mdv.readAllHeaders() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input MDV file:" << endl;
    cerr << "   URL: " << _params->input_path << endl;
    cerr << "   Request time: " << DateTime::str(trigger_info.getIssueTime()) << endl;
    cerr << "   msg: " << input_mdv.getErrStr() << endl;

    return false;
  }

  //
  // Get a list of needed input fields from the params file
  // Also checks that every field requested is available
  if(_create_verify_input_field_list(input_mdv) != 0) {
    return false;
  }

  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);


  if (_params->debug)
    input_mdv.printReadRequest(cerr);

  //
  // Read the MDV file
  if (input_mdv.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input MDV file:" << endl;
    cerr << "   URL: " << _params->input_path << endl;
    cerr << "   Request time: " << DateTime::str(trigger_info.getIssueTime()) << endl;
    cerr << "   msg: " << input_mdv.getErrStr() << endl;

    return false;
  }

  return true;
}

int ModelDeriveMdv::_create_verify_input_field_list(DsMdvx &input_mdv)
{
  Mdvx::master_header_t master_hdr = input_mdv.getMasterHeaderFile();
  vector<Mdvx::field_header_t> fhdrsFile;
  for(int a = 0; a < master_hdr.n_fields; a++)
    fhdrsFile.push_back(input_mdv.getFieldHeaderFile(a));

  vector<string> deriveFields;
  //
  // Loop over the request derive functions from the params.
  // Look at what each function needs and verify we have the
  // field available.
  //
  for(int a = 0; a < _params->derive_functions_n1; a++)
  {
    
    if(! DeriveInterface::setOutputNamesFromClassName(_params->__derive_functions[a][0]))
    {
      cerr << "ERROR: Unknown derive function " << 
	_params->__derive_functions[a][0] << endl;
      return -1;
    }

    for(int b = 1; b < _params->derive_functions_n2; b++)
    {
      if(strlen(_params->__derive_functions[a][b]) == 0)
	continue;

      int field_index = -1;
      for(int d = 0; d < fhdrsFile.size(); d++) {
	if( strcasecmp( (fhdrsFile[d]).field_name_long, _params->__derive_functions[a][b]) == 0 ||
	    strcasecmp( (fhdrsFile[d]).field_name, _params->__derive_functions[a][b]) == 0) {
	  field_index = d;
	  break;
	}
      }
      
      if (field_index == -1) 
      {
	bool found = false;
	for(int c = 0; c < deriveFields.size(); c++) {
	  if(strcasecmp(deriveFields[c].c_str(), _params->__derive_functions[a][b]) == 0) {
	    found = true;
	    break;
	  }
	}
	if(found == false) {
          cerr << "ERROR: Field " << _params->__derive_functions[a][b]
               << " is not in input MDV file, nor available from previous"
	       << "derive functions." << endl;
	  return -1;
	}

      } else {
	input_mdv.addReadField(field_index);
      }
      
    }
    const vector<string> *shortNames = DeriveBase::getOutputShortNames();
    for(int d = 0; d < shortNames->size(); d++)
      deriveFields.push_back( (*shortNames)[d] );

    const vector<string> *longNames = DeriveBase::getOutputLongNames();
    for(int d = 0; d < longNames->size(); d++)
      deriveFields.push_back( (*longNames)[d] );

  }

  //
  // Verify fields needed for the vertical interpolation function are available.
  //
  if(_params->interpolate_levels_n > 0 && _params->interpolate_function_n > 0) {

    for (int b = 1; b < _params->interpolate_function_n; b++)
    {
      if(strlen(_params->_interpolate_function[b]) == 0)
	continue;

      bool found = false;
      for(int c = 0; c < deriveFields.size(); c++) {
	if(strcasecmp(deriveFields[c].c_str(), _params->_interpolate_function[b]) == 0) {
	  found = true;
	  break;
	}
      }
      if(found == false) {
	int field_index = -1;
	for(int d = 0; d < fhdrsFile.size(); d++) {
	  if( strcasecmp( (fhdrsFile[d]).field_name_long, _params->_interpolate_function[b]) == 0 ||
	      strcasecmp( (fhdrsFile[d]).field_name, _params->_interpolate_function[b]) == 0) {
	    field_index = d;
	    break;
	  }
	}
	
	if (field_index == -1) 
	{
	  cerr << "ERROR: Field " <<  _params->_interpolate_function[b]
	       << " is not in input MDV file, nor available from previous"
	       << "derive functions." << endl;
	  return false;
	} else {
	  input_mdv.addReadField(field_index);
	  fields_to_copy.push_back(_params->_interpolate_function[b]);
	}
      }

    }

  }

  //
  // Also verify we have the fields requested to be copied
  //
  if(_params->copy_fields_n > 0) {

    for (int b = 0; b < _params->copy_fields_n; b++)
    {
      if(strlen(_params->_copy_fields[b]) == 0)
	continue;

      int field_index = -1;
      for(int d = 0; d < fhdrsFile.size(); d++) {
	if( strcasecmp( (fhdrsFile[d]).field_name_long, _params->_copy_fields[b]) == 0 ||
	    strcasecmp( (fhdrsFile[d]).field_name, _params->_copy_fields[b]) == 0) {
	  field_index = d;
	  break;
	}
      }
      
      if (field_index == -1) 
      {
	cerr << "ERROR: Field " << _params->_copy_fields[b]
	     << " is not in input MDV file, nor available from previous"
	     << "derive functions." << endl;
	return false;
      } else {
	input_mdv.addReadField(field_index);
	fields_to_copy.push_back(_params->_copy_fields[b]);
      }
    }

  }

  return 0;
}
