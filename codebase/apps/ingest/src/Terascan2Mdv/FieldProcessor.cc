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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FieldProcessor : Base class for objects that convert the data in
 *                  a TeraScan field into an Mdv field.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <errno.h>
#include <new>

#include <toolsa/pjg_flat.h>

#include "DataProcessor.hh"
#include "ByteDataProcessor.hh"
#include "ShortDataProcessor.hh"
#include "FloatDataProcessor.hh"

#include "FieldProcessor.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

FieldProcessor::FieldProcessor(SETP input_dataset) :
  _inputDataset(input_dataset),
  _initialized(false)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

FieldProcessor::~FieldProcessor()
{
  // Do nothing
}


/*********************************************************************
 * createField() - Create the MDV field from the TeraScan satellite data.
 */

MdvxField *FieldProcessor::createField(const string &sat_field_name,
			const string &mdv_field_name,
			const string &field_units,
			const int field_code,
                        const Params::scaling_type_t scaling_type,
                        const double scale,
                        const double bias)

{
  const string method_name = "FieldProcessor::createField()";
  
  VARP input_var;

  //  unsigned char *uc_ptr;
  //  short *short_ptr;
  //  float *float_ptr;

  float *var_ptr;
  float *var_ptr_out;

  float *flddata;
  float *flddata_out;

  // Initialize local variables

  if (!_initialized)
  {
    if (!_init())
      return 0;
    
    _initialized = true;
  }
  
  if ((input_var = gpvar(_inputDataset,
			 (char *)sat_field_name.c_str())) == NULL)
  {
    cerr <<  "ERROR: " << method_name << endl;
    cerr << "Requested variable is not in Terascan dataset: " <<
      sat_field_name << endl;
    msgout(terrno);
      
    return 0;
  }
 
  /* Now allocate the field arrays (header and data)  and fill the field header */

  Mdvx::field_header_t fld_hdr;
  memset(&fld_hdr, 0, sizeof(fld_hdr));
    
  fld_hdr.field_code = field_code;
  fld_hdr.nx = _numSamples;
  fld_hdr.ny = _numLines;
  fld_hdr.nz = 1;
  fld_hdr.data_element_nbytes = 4;
  fld_hdr.proj_origin_lat = _centerLat;
  fld_hdr.proj_origin_lon = _centerLon;
  
  fld_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fld_hdr.volume_size = fld_hdr.nx * fld_hdr.ny * fld_hdr.nz * fld_hdr.data_element_nbytes; 
  fld_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fld_hdr.scaling_type = Mdvx::SCALING_NONE;

  _setProjectionInfo(fld_hdr);
  
  fld_hdr.bad_data_value = 999999.;  
  fld_hdr.missing_data_value = 999999.;  
  fld_hdr.proj_rotation = 0.0;  

  strcpy(fld_hdr.field_name_long,  sat_field_name.c_str());
  strcpy(fld_hdr.field_name,       mdv_field_name.c_str());
  strcpy(fld_hdr.units,            field_units.c_str());
  strcpy(fld_hdr.transform,        "none");

  fld_hdr.scale = 1;
  fld_hdr.bias  = 0;

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
    
  vlevel_hdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
  vlevel_hdr.level[0] = 0.0;
    
  /* 
     Loop to read, process, and write the data.  The data will be 
     processed 1 (scan) line at a time (gpgetvar(3), gpputvar(3)).
     */

  // Create the data processor object

  DataProcessor *data_processor;
    
  switch (input_var->type)
  {
  case GP_BYTE :
    if(!(data_processor = new(nothrow) ByteDataProcessor())) {
      cerr << "new failed" << endl;
      return 0;
    }
    break;
      
  case GP_SHORT :
    if(!(data_processor = new(nothrow) ShortDataProcessor())) {
      cerr << "new failed" << endl;
      return 0;
    }
    break;
      
  case GP_FLOAT :
    if(!(data_processor = new(nothrow) FloatDataProcessor())) {
      cerr << "new failed" << endl;
      return 0;
    }
    break;
      
  default:
    cerr << "ERROR: " << method_name << endl;
    cerr << "Cannot process satellite data type: " << input_var->type << endl;
    cerr << "Skipping satellite file" << endl;
      
    
    return 0;
  } /* endswitch - input_var->type */

  // Convert the satellite data to the format needed for the MDV file.
  // We do this line-by-line.


  if(!(flddata = new(nothrow) float[_numLines * _numSamples])) {
     cerr << "new failed to allocate " << sizeof(float)*_numLines*_numSamples << " bytes." << endl;
     return 0;
  }
  var_ptr = flddata;
  
  for (int i = _numLines - 1; i >= 0; --i)
  { 
    if (!data_processor->convertData(input_var,
				     i, 0, 1, _numSamples,
				     var_ptr))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting satellite data to output format" << endl;
      cerr << "Skipping file" << endl;
	
      delete [] flddata;
      
      return false;
    }
      
    var_ptr += _numSamples;
  } /* endfor - i */

  if(!(flddata_out = new(nothrow) float[_numLines * _numSamples])) {
    cerr << "new failed to allocate " << sizeof(float)*_numLines*_numSamples << " bytes." << endl;
    return 0;
  }
  var_ptr_out = flddata_out;

  // Now loop through the native grid and move the values to the appropriate
  // place in the output grid.

  for (int i = 0; i < _numLines; i++)
  {
    for (int j = 0; j < _numSamples; j++)
      var_ptr_out[j] = flddata[_calcIndex(i, j)];

    var_ptr_out += _numSamples;
  } /* endfor - i */

  // Create the field to be returned
  MdvxField *output_field;
  if(!(output_field = new(nothrow) MdvxField(fld_hdr, vlevel_hdr,
					     flddata_out))) {
    cerr << "errno is " << errno << endl;
    cerr << "MdvxField new failed" << endl;
    return 0;
  }

  // Finally, free allocated temporary space

  switch (scaling_type) {
    case Params::ROUNDED:
      // Dynamic scale and bias but constrained to 0.2, 0.5 or 1.0 multiplied by power of 10
      output_field->convertRounded(Mdvx::ENCODING_INT8);
      break;
    // Dynamic scale and bias constrained to integral values
    case Params::INTEGRAL:
      output_field->convertIntegral(Mdvx::ENCODING_INT8);
      break;
    case Params::DYNAMIC:
      output_field->convertDynamic(Mdvx::ENCODING_INT8);
      break;
    // Scale and Bias specified in the parameter file
    case Params::SPECIFIED:
      output_field->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_NONE,
                            Mdvx::SCALING_SPECIFIED, scale, bias);
      break;
  }
  
  delete [] flddata;
  delete [] flddata_out;
  delete data_processor;

  return output_field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _init() - Initialize the internal variables.
 */

bool FieldProcessor::_init()
{
  const string method_name = "FieldProcessor::_init()";
  
  /*
   * Retrieve number of line and samples in the dataset (gpdim(3)).
   * Images usually have line and sample dimension names, however the 
   * x and y dimensions may have any names but are required to have 
   * their cooord attributes be set to GP_X_COORD (sample) and GP_Y_COORD
   * (line), see datasets(7).
   * In that case you would search the dimensions for those with the 
   * coord dimension attribute set to GP_X_COORD and GP_Y_COORD.
   */

  DIMP line_dimension;
  
  if ((line_dimension = gpdim(_inputDataset, "gvar_ch1_line")) == NULL)
  {
    if ((line_dimension = gpdim(_inputDataset, "line")) == NULL) 
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting line dimension from dataset" << endl;
      msgout(terrno);
      
      return false;
    }
  }
  _numLines = line_dimension->size;

  DIMP sample_dimension;
  
  if ((sample_dimension = gpdim(_inputDataset, "gvar_ch1_samp")) == NULL)
  {
    if ((sample_dimension = gpdim(_inputDataset, "sample")) == NULL) 
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting sample dimension from dataset" << endl;
      msgout(terrno);
      
      return false;
    }
  }

  _numSamples = sample_dimension->size;

  // Initialize the satellite data projection calculator

  ETXFORM mxfm;

  gpetready(_inputDataset, 0, mxfm);

  // Find the center lat/lon

  if (gpgetatt(_inputDataset, C_CENTERLAT, (double *) &_centerLat) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting time from dataset" << endl;
    msgout(terrno);
    return false;
  }

  if (gpgetatt(_inputDataset, C_CENTERLON, (double *) &_centerLon) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting time from dataset" << endl;
    msgout(terrno);
    return false;
  }

  if (gpgetatt(_inputDataset, C_PROJPARAM, (double *) &_projParam) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting time from dataset" << endl;
    msgout(terrno);
    return false;
  }

  if (gpgetatt(_inputDataset, C_PROJECTION, (long *) &_projection) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting time from dataset" << endl;
    msgout(terrno);
    return false;
  }


  // Find the lower left lat/lon

  etxll(mxfm, _numLines, 1,
	&_lowerLeftLat, &_lowerLeftLon);
  
  // Initialize any variables needed locally by the derived classes

  return _initLocal(mxfm);
}

