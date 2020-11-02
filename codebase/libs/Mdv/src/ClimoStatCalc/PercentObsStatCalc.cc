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
/*********************************************************************
 * PercentObsStatCalc: Class for calculating the percentage of
 * observations statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/climo/PercentObsStatCalc.hh>
#include <toolsa/str.h>

using namespace std;


/**********************************************************************
 * Constructor
 */

PercentObsStatCalc::PercentObsStatCalc(const bool debug_flag,
				       const bool check_z_levels) :
  StatCalc(debug_flag, check_z_levels)
{
}


/**********************************************************************
 * Destructor
 */

PercentObsStatCalc::~PercentObsStatCalc(void)
{
}
  

/**********************************************************************
 * calcStatistic() - Calculate the statistic field using the given
 *                   information.
 */

MdvxField *PercentObsStatCalc::calcStatistic(const DsMdvx &climo_file,
					     const MdvxField &data_field,
					     const DateTime &climo_time)
{
  // Construct the statistic field name

  string data_field_name = data_field.getFieldHeader().field_name_long;
  
  // Extract the existing percent field from the climatolog file, if
  // there  is one.  If there is an existing field, we will update
  // the running percent.  If there isn't an existing field, we will
  // create a new one.

  string climo_field_name =
    _getStatFieldName(data_field_name).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *curr_percent_field =
    climo_file.getField(climo_field_name.c_str());
  MdvxField *new_percent_field = 0;
  
  if (curr_percent_field == 0)
  {
    if (_debug)
      cerr << "   Creating " << climo_field_name << " statistic " << endl;
    
    new_percent_field = _createField(data_field, climo_time);
  }
  else
  {
    if (_debug)
      cerr << "   Updating " << climo_field_name << " statistic " << endl;
    
    new_percent_field = _updateField(climo_file, data_field,
				     *curr_percent_field);
  }
  
  return new_percent_field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createField() - Create a new percent climatology field from the
 *                  given data field.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *PercentObsStatCalc::_createField(const MdvxField &data_field,
					    const DateTime &climo_time) const
{
  static const string method_name = "PercentObsStatCalc::_createField()";

  // Create the field header

  Mdvx::field_header_t field_hdr = data_field.getFieldHeader();;
  
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = climo_time.utime();
  
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny *
    field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.bad_data_value = -1.0;
  field_hdr.missing_data_value = -1.0;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  
  STRcopy(field_hdr.field_name_long,
	  _getStatFieldName(field_hdr.field_name_long).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,
	  _getStatFieldName(field_hdr.field_name).c_str(),
	  MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "%", MDV_UNITS_LEN);
  
  MdvxField *percent_field = new MdvxField(field_hdr,
					   data_field.getVlevelHeader(),
					   NULL, true);
  if (percent_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating percent field" << endl;
    
    return 0;
  }
  
  // Fill in the values

  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  
  fl32 *data_vol = (fl32 *)data_field.getVol();
  fl32 *percent_vol = (fl32 *)percent_field->getVol();
  
  int vol_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  
  for (int i = 0; i < vol_size; ++i)
  {
    if (data_vol[i] == data_field_hdr.missing_data_value ||
	data_vol[i] == data_field_hdr.bad_data_value)
    {
      percent_vol[i] = 0.0;
    }
    else
    {
      if (_meetsCondition(data_vol[i]))
	percent_vol[i] = 100.0;
      else
	percent_vol[i] = 0.0;
    }
    
  } /* endfor - i */
  
  return percent_field;
}


/*********************************************************************
 * _updateField() - Create a new percent climatology field that is an
 *                  update of the current percent field using the given
 *                  information.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *PercentObsStatCalc::_updateField(const DsMdvx &climo_file,
					    const MdvxField &data_field,
					    const MdvxField &curr_percent_field) const
{
  static const string method_name = "PercentObsStatCalc::_updateField()";

  // Make sure the fields match so we can update the climos

  if (!_fieldsMatch(data_field, curr_percent_field))
    return 0;
  
  // To calculate the new percent, we need the current num obs and the
  // current num times fields.

  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  
  string num_obs_field_name =
    _getNumObsStatFieldName(data_field_hdr.field_name_long).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *num_obs_field =
    climo_file.getField(num_obs_field_name.c_str());
  
  if (num_obs_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting num obs field (" << num_obs_field_name
	 << ") from climo file" << endl;
    
    return 0;
  }
  
  string num_times_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_NUM_TIMES,
				data_field_hdr.field_name_long).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *num_times_field =
    climo_file.getField(num_times_field_name.c_str());
  
  if (num_times_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting num times field (" << num_times_field_name
	 << ") from climo file" << endl;
    
    return 0;
  }
  
  // Create the new field as a copy of the original field

  MdvxField *new_percent_field = new MdvxField(curr_percent_field);
  
  // Loop through the new field values, calculating the percentage
  // as we go along

  Mdvx::field_header_t percent_field_hdr = new_percent_field->getFieldHeader();
  Mdvx::field_header_t num_obs_field_hdr = num_obs_field->getFieldHeader();
  Mdvx::field_header_t num_times_field_hdr = num_times_field->getFieldHeader();
  
  fl32 *percent_data = (fl32 *)new_percent_field->getVol();
  fl32 *data = (fl32 *)data_field.getVol();
  fl32 *num_obs_data = (fl32 *)num_obs_field->getVol();
  fl32 *num_times_data = (fl32 *)num_times_field->getVol();
  
  int volume_size =
    percent_field_hdr.nx * percent_field_hdr.ny * percent_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    fl32 curr_num_obs = num_obs_data[i];
    fl32 curr_num_times = num_times_data[i];
    
    // Reinitialize the current numbers if they're missing

    if (curr_num_obs == num_obs_field_hdr.missing_data_value ||
	curr_num_obs == num_obs_field_hdr.bad_data_value)
      curr_num_obs = 0;

    if (curr_num_times == num_times_field_hdr.missing_data_value ||
	curr_num_times == num_times_field_hdr.bad_data_value)
      curr_num_times = 0;

    // Update the num obs and num times values based on the current
    // data values

    if (data[i] != data_field_hdr.missing_data_value &&
	data[i] != data_field_hdr.bad_data_value &&
	_meetsCondition(data[i]))
      ++curr_num_obs;
    
    ++curr_num_times;
    
    // Calculate the new percentage

    if (curr_num_times > 0)
      percent_data[i] = curr_num_obs / curr_num_times * 100.0;
    else
      percent_data[i] = 0.0;
    
  } /* endfor - i */
  
  return new_percent_field;
}
