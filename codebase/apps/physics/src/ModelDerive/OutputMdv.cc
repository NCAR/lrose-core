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
 *   $Revision: 1.13 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * OutputMdv: OutputMdv program object.
 * Handles the output mdv data and vertical interpolation.
 *
 * Output derived fields will be added to the output url file if it
 * exists.  Thus using the same Input and Output Mdv url will add the 
 * derived fields to the input file. If vertical interpolation is on
 * the entire output files fields (with the same 3d dimensions, even
 * fields not output by this program), will be interpolated to the new 
 * vertical levels.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 *********************************************************************/

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

#include <grib2/ProdDefTemp.hh>

#include "OutputMdv.hh"
#include "Params.hh"
#include "InterpInterface.hh"

using namespace std;

OutputMdv::OutputMdv( Params *params, bool writeForecast )
{
  _params    = params;
  _mdvObj       = NULL;
  _forecast = writeForecast;
}

OutputMdv::~OutputMdv()
{
  clear();
}

void OutputMdv::clear() 
{
  fields.clear();
  if( _mdvObj )
    delete _mdvObj;
  _mdvObj = NULL;
}

int OutputMdv::writeVol(Mdvx::master_header_t masterHdr, const char *dataSetInfo, const char *dataSetSource)
{
  PMU_auto_register("In OutputMdv::writeVol");

  if( fields.size() == 0 ) {
    cerr << "ERROR: No fields added" << endl << flush;
    return(-1);
  }

  //
  // Vertical Interpolation is requested on output file.
  //
  if(_params->interpolate_levels_n > 0 && _params->interpolate_function_n > 0) {
    vector<float*> inputs;
    vector<float*> data;
    vector<float*> output_data;
    vector<int> data_index;

    Mdvx::field_header_t field_hdr;
    Mdvx::vlevel_header_t vlevel_hdr;

    float levels[MDV_MAX_VLEVELS];
    int nx, ny, nz;
    float missing, bad;
    int outz = _params->interpolate_levels_n;
    float *outlevels = _params->_interpolate_levels;
    Mdvx::grid_order_indices_t order = Mdvx::ORDER_XYZ;

    //
    // First gather up the input fields to the interpolation function
    //
    for (int b = 1; b < _params->interpolate_function_n; b++)
    {
      cout << "Processing function " << _params->_interpolate_function[0] << " ..." << endl;

      MdvxField *field = getField(_params->_interpolate_function[b]);
      if(field == 0) {
	cerr << "Error unable to retrieve field " << _params->_interpolate_function[b]
	     << " for interpolation " << _params->_interpolate_function[0] << endl;
	return (-4);
      }

      field_hdr = field->getFieldHeader();
      vlevel_hdr = field->getVlevelHeader();

      if(b == 1) {
	nx = field_hdr.nx;
	ny = field_hdr.ny;
	nz = field_hdr.nz;
	missing = field_hdr.missing_data_value;
	bad = field_hdr.bad_data_value;
	for(int c = 0; c < nz; c++)
	  levels[c] = vlevel_hdr.level[c];
      } else {
	if(field_hdr.nx != nx || field_hdr.ny != ny || field_hdr.nz != nz) {
	  cerr << "ERROR: Input fields for interpolation " << _params->_interpolate_function[0]
	       << " do not have identical dimensions." << endl;
	  return (-5);
	}
	if(field_hdr.missing_data_value != missing || field_hdr.bad_data_value != bad) {
	  cerr << "ERROR: Input fields for interpolation " << _params->_interpolate_function[0]
	       << " do not have identical missing/bad data values." << endl;
	  return (-5);
	}
      }
      inputs.push_back((float *)field->getVol());
    }
    if(_params->interpolate_function_n < 2)
      cerr << "WARNING: No input fields for interpolation " << _params->_interpolate_function[0]
	   << ". At least one input field is required for vertical interp functions." << endl;

    //
    // Gather all fields in the output file and stored local to be output
    // that match the input fields dimensions.
    //
    for(int a = 0; a < fields.size(); a++) {

      field_hdr = fields[a]->getFieldHeader();
      vlevel_hdr = fields[a]->getVlevelHeader();

      if(field_hdr.nz == 0)
	continue;

      if(field_hdr.nx != nx || field_hdr.ny != ny || field_hdr.nz != nz) {
	continue;
      }
      if(field_hdr.missing_data_value != missing || field_hdr.bad_data_value != bad) {
	cerr << "Warning: Fields for interpolation " << _params->_interpolate_function[0]
	     << " do not have identical missing/bad data values." << endl;
      }

      data.push_back((float *) fields[a]->getVol());
      data_index.push_back(a);
    }

    InterpBase *interpFunction = 
      InterpInterface::getInterpClassFromName(_params->_interpolate_function[0],
					      missing, bad, nx, ny, nz, order, levels, outz, outlevels);

    if(interpFunction == NULL) {
      cerr << "Error retreiving interpolation function class for function " << 
	_params->_interpolate_function[0] << endl;
      return (-5);
    }

    char *levelName = InterpBase::getOutputLevelName();
    char *units = InterpBase::getOutputUnits();

    interpFunction->interp(&inputs, &data, &output_data);

    delete interpFunction;

    int mdvLevelType = _convertGribLevel2MDVLevel(levelName, units);

    masterHdr.vlevel_type = mdvLevelType;
    masterHdr.vlevel_included = 1;

    //
    // Swap out the data in the field for the new interpolated data
    // If using Grib2 convetion vertical level field names, switch those
    //
    for(int a = 0; a < data_index.size(); a++) {
      fields[data_index[a]]->clearVolData();

      //
      // Set the vertical level header to the new values
      vlevel_hdr = fields[data_index[a]]->getVlevelHeader();
      for(int b = 0; b < _params->interpolate_levels_n; b++) {
	vlevel_hdr.type[b] = mdvLevelType;
	vlevel_hdr.level[b] = _params->_interpolate_levels[b];
      }
      for(int b = _params->interpolate_levels_n; b < nz; b++) {
	vlevel_hdr.type[b] = 0;
	vlevel_hdr.level[b] = 0;
      }

      //
      // Set new vertical information in the field header
      field_hdr = fields[data_index[a]]->getFieldHeader();
      field_hdr.nz = outz;
      field_hdr.grid_dz = 0.0;
      field_hdr.dz_constant = 0;
      field_hdr.grid_minz = levels[0];
      field_hdr.vlevel_type = mdvLevelType;
      field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  
      _switchVerticalFieldName(levelName, &field_hdr);

      cout << "Creating Field: " << field_hdr.field_name << endl;


      fields[data_index[a]]->setFieldHeader(field_hdr);
      fields[data_index[a]]->setVlevelHeader(vlevel_hdr);

      fields[data_index[a]]->setVolData(output_data[a], field_hdr.volume_size, Mdvx::ENCODING_FLOAT32);

      delete [] (output_data[a]);

    }
  }

  //
  // Check to see if this Mdv file already exists
  // If it does we want to copy out the fields from it
  //

  DsMdvx mdvx;
  mdvx.clearRead();
  mdvx.clearFields();
  mdvx.clearChunks();
  mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _params->output_path, 0,
		       masterHdr.time_gen, masterHdr.time_end - masterHdr.time_gen);
  mdvx.readAllHeaders();
  const Mdvx::master_header_t &master_header = mdvx.getMasterHeaderFile();
  if(master_header.time_gen == masterHdr.time_gen &&
     master_header.time_end == masterHdr.time_end)
  {

    if(mdvx.readVolume()) {
      cerr << "Error: Cannot read mdv data: " << "  " << mdvx.getErrStr() << endl;
      return(-3);
    }

    bool overwrite;
    //
    // Load previous mdv fields and add them to our list
    //
    vector<MdvxField *> f = mdvx.getFields();
    for(int a = 0; a < f.size(); a++) {
      overwrite = false;
      for(int b = 0; b < fields.size(); b++)
	if(strcmp(f[a]->getFieldHeader().field_name, fields[b]->getFieldHeader().field_name) == 0)
	  overwrite = true;
      if(!overwrite) {
	fields.push_back(new MdvxField(*(f[a])));
	cout << "Copying Field: " << f[a]->getFieldHeader().field_name << endl;
      }
    }

  }

  //
  // Create Mdvx object
  //
  _mdvObj = new DsMdvx();
  

  Mdvx::encoding_type_t encodingType = 
    (Mdvx::encoding_type_t)_params->encoding_type;
  Mdvx::compression_type_t compressionType = 
    (Mdvx::compression_type_t)_params->compression_type;
  Mdvx::scaling_type_t scalingType = 
    (Mdvx::scaling_type_t)_params->scaling_type;
  
  for(int a = 0; a < fields.size(); a++) {
    //
    // Convert to specified type and encode
    //
    fields[a]->convertType(encodingType, compressionType, scalingType);
    //
    // Add the fields to the Mdvx object
    //
    _mdvObj->addField( fields[a] );
  }
  
  
  strncpy(masterHdr.data_set_info, dataSetInfo, MDV_INFO_LEN-1);
  strncpy(masterHdr.data_set_source, dataSetSource, MDV_NAME_LEN-1);

  masterHdr.n_chunks = 0;

  _mdvObj->setMasterHeader( masterHdr );
  _mdvObj->updateMasterHeader();
  if(_forecast)
    _mdvObj->setWriteAsForecast();

  cout << "Writing " << fields.size() << " fields to dir: " << _params->output_path << endl; 
  if(_mdvObj->writeToDir(_params->output_path)) {
    cerr << " ERROR: Could not write file to " << _params->output_path << endl;
    return(-2);
  }
  
  //
  // Clean up
  //
  clear();
  
  return(0);
}

void OutputMdv::addField(MdvxField *inputField)
{

  fields.push_back(inputField);

}

void OutputMdv::createMdvxField(Mdvx::field_header_t fieldHeader, Mdvx::vlevel_header_t vlevelHeader,
				 fl32 *data, const char *shortName, const char *longName, 
				const char *units, char *level) 
{

  strncpy(fieldHeader.field_name, shortName, MDV_SHORT_FIELD_LEN-1);
  if(level != NULL) {
    strcat(fieldHeader.field_name, "_");
    strncat(fieldHeader.field_name, level, MDV_SHORT_FIELD_LEN-2-strlen(fieldHeader.field_name));
  }
  cout << "Creating Field: " << fieldHeader.field_name << endl;

  strncpy(fieldHeader.field_name_long, longName, MDV_LONG_FIELD_LEN-1);
  strncpy(fieldHeader.units, units, MDV_UNITS_LEN-1);
  fieldHeader.field_code = 0;
  fieldHeader.min_value = 0.0;
  fieldHeader.max_value = 0.0;

  MdvxField *theField; 

  theField = new MdvxField(fieldHeader, vlevelHeader, data);

  addField(theField);

}


MdvxField *OutputMdv::getField(char *fieldName)
{
  MdvxField *field = NULL;
  for(int a = 0; a < fields.size(); a++) {
    field = fields[a];
    if(strcasecmp(field->getFieldName(), fieldName) == 0)
      return field;
    if(strcasecmp(field->getFieldNameLong(), fieldName) == 0)
      return field;

  }

  return NULL;
}



//
// Returns the Mdv level appropriate to a Grib level
int OutputMdv::_convertGribLevel2MDVLevel(char *gribLevel, char *units)
{
  if(strlen(gribLevel) == 0)
     return Mdvx::VERT_TYPE_Z;

  if (!strcmp(gribLevel, "SFC"))
    return Mdvx::VERT_TYPE_SURFACE;
  else if (!strcmp(gribLevel, "CBL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(gribLevel, "CTL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(gribLevel, "0DEG"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(gribLevel, "ADCL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV; //??
  else if (!strcmp(gribLevel, "MWSL"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(gribLevel, "TRO"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(gribLevel, "NTAT"))
    return Mdvx::VERT_FIELDS_VAR_ELEV;
  else if (!strcmp(gribLevel, "SEAB"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(gribLevel, "TMPL"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(gribLevel, "ISBL"))
    return Mdvx::VERT_TYPE_PRESSURE;
  else if (!strcmp(gribLevel, "MSL"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(gribLevel, "GPML"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(gribLevel, "HTGL")) {
    if(!strcmp(units, "km"))
      return Mdvx::VERT_TYPE_Z;
    else if(!strcmp(units, "100ft"))
      return Mdvx::VERT_FLIGHT_LEVEL;
    else if(!strcmp(units, "ft"))
      return Mdvx::VERT_TYPE_ZAGL_FT;
    else
      return Mdvx::VERT_TYPE_Z;
  }
  else if (!strcmp(gribLevel, "SIGL"))
    return Mdvx::VERT_TYPE_SIGMA_Z;
  else if (!strcmp(gribLevel, "HYBL"))
    return Mdvx::VERT_TYPE_SIGMA_Z;
  else if (!strcmp(gribLevel, "DBLL"))
    return Mdvx::VERT_TYPE_Z;
  else if (!strcmp(gribLevel, "THEL"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(gribLevel, "SPDL"))
    return Mdvx::VERT_TYPE_PRESSURE;
  else if (!strcmp(gribLevel, "PVL"))
    return Mdvx::VERT_TYPE_SURFACE;
  else if (!strcmp(gribLevel, "EtaL"))
    return Mdvx::VERT_TYPE_Z;          //??
  else if (!strcmp(gribLevel, "DBSL"))
    return Mdvx::VERT_TYPE_Z;
  else
    return Mdvx::VERT_TYPE_Z;
}


void OutputMdv::_switchVerticalFieldName(char *outLevelName, Mdvx::field_header_t *field_hdr)
{
  if(outLevelName == NULL)
    return;

  char *inLevelName = NULL;
  inLevelName = stripLevelName(field_hdr->field_name);

  if(inLevelName != NULL) {
    char *pos = strstr(field_hdr->field_name, inLevelName);
    if(pos != NULL)
      strncpy(pos, outLevelName, MDV_SHORT_FIELD_LEN-2-strlen(field_hdr->field_name)+strlen(pos));
    delete[] inLevelName;
  }

}



char *stripLevelName(char *fieldName)
{
  int a = 0;
  // Loop through the grib2 surface levels untill you get to the
  // last surface level witch is always "MISSING"
  while(Grib2::ProdDefTemp::_surface[a].name.compare("MISSING") != 0)
  {
    char *pos = strstr(fieldName, Grib2::ProdDefTemp::_surface[a].name.c_str());
    // If we found one and it is preceded by a underscore then it is assumed
    // to be the vertical level name.
    if(pos != NULL && pos[-1] == '_') {
      char *level = new char[strlen(pos)+1];
      strcpy(level, pos);
      return level;
    }
    a++;
  }
  return NULL;
}


bool isForecastFile(const char *mdvFileName)
{
  const char *pos = strstr(mdvFileName, "/g_");
  if(pos != NULL) {
    const char *pos2 = strstr(pos, "/f_"); 
    if(pos2 != NULL)
      return true;
  }
  return false;
}
