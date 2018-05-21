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
 * MeanStatCalc: Class for calculating the mean statistic.
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
#include <toolsa/str.h>

using namespace std;


/**********************************************************************
 * Constructor
 */

MeanStatCalc::MeanStatCalc(const bool debug_flag, const bool check_z_levels) :
  StatCalc(debug_flag, check_z_levels)
{
}


/**********************************************************************
 * Destructor
 */

MeanStatCalc::~MeanStatCalc(void)
{
}
  

/**********************************************************************
 * calcStatistic() - Calculate the statistic field using the given
 *                   information.
 */

MdvxField *MeanStatCalc::calcStatistic(const DsMdvx &climo_file,
				       const MdvxField &data_field,
				       const DateTime &climo_time)
{
  // Construct the statistic field name

  string stat_field_name = data_field.getFieldHeader().field_name_long;
  
  // Extract the existing mean field from the climatolog file, if
  // there  is one.  If there is an existing field, we will update
  // the running mean.  If there isn't an existing field, we will
  // create a new one.

  string climo_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MEAN,
				stat_field_name).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *curr_mean_field =
    climo_file.getField(climo_field_name.c_str());
  MdvxField *new_mean_field = 0;
  
  if (curr_mean_field == 0)
  {
    if (_debug)
      cerr << "   Creating " << climo_field_name << " statistic" << endl;
    
    new_mean_field = _createField(data_field, climo_time);
  }
  else
  {
    if (_debug)
      cerr << "   Updating " << climo_field_name << " statistic" << endl;
    
    new_mean_field = _updateField(climo_file, data_field,
				  *curr_mean_field);
  }
  
  return new_mean_field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createField() - Create a new mean climatology field from the
 *                  given data field.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *MeanStatCalc::_createField(const MdvxField &data_field,
				      const DateTime &climo_time) const
{
  static const string method_name = "MeanStatCalc::_createField()";

  // Create the field header

  Mdvx::field_header_t field_hdr = data_field.getFieldHeader();;
  
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = climo_time.utime();
  
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;

  STRcopy(field_hdr.field_name_long,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MEAN,
				      field_hdr.field_name_long).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MEAN,
				      field_hdr.field_name).c_str(),
	  MDV_SHORT_FIELD_LEN);
  
  return new MdvxField(field_hdr,
		       data_field.getVlevelHeader(),
		       data_field.getVol());
}


/*********************************************************************
 * _updateField() - Create a new mean climatology field that is an
 *                  update of the current mean field using the given
 *                  information.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *MeanStatCalc::_updateField(const DsMdvx &climo_file,
				      const MdvxField &data_field,
				      const MdvxField &curr_mean_field) const
{
  static const string method_name = "MeanStatCalc::_updateField()";

  // Make sure that the current mean field and the new data field
  // match.  We can assume that all of the climo fields are consistent
  // so we don't need to check any other part of the climo file.

  if (!_fieldsMatch(data_field, curr_mean_field))
    return 0;
  
  // To calculate the new mean, we need the current mean and the
  // current number of observations.  The current mean field is
  // already given.

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
  
  // Create the new climo field as a copy of the current one.

  MdvxField *new_mean_field = new MdvxField(curr_mean_field);
  
  // Update the mean values in the new field.

  Mdvx::field_header_t num_obs_field_hdr = num_obs_field->getFieldHeader();
  
  fl32 *mean_vol = (fl32 *)new_mean_field->getVol();
  fl32 *num_obs_vol = (fl32 *)num_obs_field->getVol();
  fl32 *data_vol = (fl32 *)data_field.getVol();
  
  int vol_size = data_field_hdr.nx * data_field_hdr.ny * data_field_hdr.nz;
  
  for (int i = 0; i < vol_size; ++i)
  {
    if (data_vol[i] == data_field_hdr.missing_data_value ||
	data_vol[i] == data_field_hdr.bad_data_value)
      continue;
    
    if (num_obs_vol[i] <= 0 ||
	num_obs_vol[i] == num_obs_field_hdr.missing_data_value ||
	num_obs_vol[i] == num_obs_field_hdr.bad_data_value)
    {
      mean_vol[i] = data_vol[i];
    }
    else
    {
      double prev_total = mean_vol[i] * num_obs_vol[i];
      mean_vol[i] = (prev_total + data_vol[i]) / (num_obs_vol[i] + 1.0);
    }
  } /* endfor - i */
  
  return new_mean_field;
}
