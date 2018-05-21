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
 * StdDevStatCalc: Class for calculating the standard deviation statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/climo/MeanStatCalc.hh>
#include <Mdv/climo/NumObsStatCalc.hh>
#include <Mdv/climo/StdDevStatCalc.hh>
#include <toolsa/str.h>
#include <cmath>
using namespace std;


/**********************************************************************
 * Constructor
 */

StdDevStatCalc::StdDevStatCalc(const bool debug_flag,
			       const bool check_z_levels) :
  StatCalc(debug_flag, check_z_levels)
{
}


/**********************************************************************
 * Destructor
 */

StdDevStatCalc::~StdDevStatCalc(void)
{
}
  

/**********************************************************************
 * calcStatistic() - Calculate the statistic field using the given
 *                   information.
 */

MdvxField *StdDevStatCalc::calcStatistic(const DsMdvx &climo_file,
					 const MdvxField &data_field,
					 const DateTime &climo_time)
{
  // Construct the statistic field name

  string stat_field_name = data_field.getFieldHeader().field_name_long;
  
  // Extract the existing std dev field from the climatolog file, if
  // there  is one.  If there is an existing field, we will update
  // the running std dev.  If there isn't an existing field, we will
  // create a new one.

  string climo_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_STD_DEV,
				stat_field_name).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *curr_std_field =
    climo_file.getField(climo_field_name.c_str());
  MdvxField *new_std_field = 0;
  
  if (curr_std_field == 0)
  {
    if (_debug)
      cerr << "   Creating " << climo_field_name << " statistic" << endl;
    
    new_std_field = _createField(data_field, climo_time);
  }
  else
  {
    if (_debug)
      cerr << "   Updating " << climo_field_name << " statistic" << endl;
    
    new_std_field = _updateField(climo_file, data_field,
				 *curr_std_field);
  }
  
  return new_std_field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createField() - Create a new std dev climatology field from the
 *                  given data field.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *StdDevStatCalc::_createField(const MdvxField &data_field,
				      const DateTime &climo_time) const
{
  static const string method_name = "StdDevStatCalc::_createField()";

  // Create the field header

  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();;
  Mdvx::field_header_t std_field_hdr = data_field_hdr;
  
  std_field_hdr.forecast_delta = 0;
  std_field_hdr.forecast_time = climo_time.utime();
  
  std_field_hdr.bad_data_value = -9999.0;
  std_field_hdr.missing_data_value = -9999.0;
  
  std_field_hdr.compression_type = Mdvx::COMPRESSION_NONE;

  STRcopy(std_field_hdr.field_name_long,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_STD_DEV,
				      std_field_hdr.field_name_long).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(std_field_hdr.field_name,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_STD_DEV,
				      std_field_hdr.field_name).c_str(),
	  MDV_SHORT_FIELD_LEN);
  
  // Create the data.  The standard deviation value will start off
  // always being 0 since our data values are equal to our means
  // with a single observation.

  int volume_size = std_field_hdr.nx * std_field_hdr.ny * std_field_hdr.nz;
  fl32 *std_data = new fl32[volume_size];
  fl32 *raw_data = (fl32 *)data_field.getVol();
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (raw_data[i] == data_field_hdr.missing_data_value ||
	raw_data[i] == data_field_hdr.bad_data_value)
      std_data[i] = std_field_hdr.bad_data_value;
    else
      std_data[i] = 0.0;
  }
  
  MdvxField *std_field = new MdvxField(std_field_hdr,
				       data_field.getVlevelHeader(),
				       std_data);

  delete [] std_data;
  
  return std_field;
}


/*********************************************************************
 * _updateField() - Create a new std dev climatology field that is an
 *                  update of the current std dev field using the given
 *                  information.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *StdDevStatCalc::_updateField(const DsMdvx &climo_file,
					const MdvxField &data_field,
					const MdvxField &curr_std_field) const
{
  static const string method_name = "StdDevStatCalc::_updateField()";

  // Make sure that the current std dev field and the new data field
  // match.  We can assume that all of the climo fields are consistent
  // so we don't need to check any other part of the climo file.

  if (!_fieldsMatch(data_field, curr_std_field))
    return 0;
  
  // To calculate the new std dev, we need the current std dev, the
  // current mean and the current number of observations.  The current
  // std dev field is already given.

  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  
  string num_obs_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_NUM_OBS,
				data_field_hdr.field_name_long).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *num_obs_field =
    climo_file.getField(num_obs_field_name.c_str());
  
  if (num_obs_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting num obs field (" << num_obs_field_name
	 << ") from climo file" << endl;
    
    return 0;
  }
  
  string mean_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MEAN,
				data_field_hdr.field_name_long).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *mean_field =
    climo_file.getField(mean_field_name.c_str());
  
  if (mean_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting mean field (" << mean_field_name
	 << ") from climo file" << endl;
    
    return 0;
  }
  
  // Create the new climo field as a copy of the current one.

  MdvxField *new_std_field = new MdvxField(curr_std_field);
  
  // Update the std dev values in the new field.

  Mdvx::field_header_t num_obs_field_hdr = num_obs_field->getFieldHeader();
  
  fl32 *std_vol = (fl32 *)new_std_field->getVol();
  fl32 *mean_vol = (fl32 *)mean_field->getVol();
  fl32 *num_obs_vol = (fl32 *)num_obs_field->getVol();
  fl32 *data_vol = (fl32 *)data_field.getVol();
  
  int vol_size = data_field_hdr.nx * data_field_hdr.ny * data_field_hdr.nz;
  
  for (int i = 0; i < vol_size; ++i)
  {
    if (data_vol[i] == data_field_hdr.missing_data_value ||
	data_vol[i] == data_field_hdr.bad_data_value)
      continue;
    
    if (num_obs_vol[i] == 0 ||
	num_obs_vol[i] == num_obs_field_hdr.missing_data_value ||
	num_obs_vol[i] == num_obs_field_hdr.bad_data_value)
    {
      std_vol[i] = 0.0;
    }
    else
    {
      double m = num_obs_vol[i];
      double n = m + 1.0;
      
      double prev_mean = mean_vol[i];
      double prev_total = prev_mean * m;
      double new_total = prev_total + data_vol[i];
      
      double prev_std = std_vol[i];
      
      double new_std =
	sqrt((((m - 1.0) * prev_std * prev_std) +
	      (m * prev_mean * prev_mean) + (data_vol[i] * data_vol[i]) -
	      ((new_total * new_total) / n)) / m);

      std_vol[i] = new_std;
    }
  } /* endfor - i */
  
  return new_std_field;
}
