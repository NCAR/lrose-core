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
 * MaxDateStatCalc: Class for calculating the date of the maximum statistic.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/climo/MaxDateStatCalc.hh>
#include <toolsa/str.h>

using namespace std;


/**********************************************************************
 * Constructor
 */

MaxDateStatCalc::MaxDateStatCalc(const bool debug_flag,
				 const bool check_z_levels) :
  StatCalc(debug_flag, check_z_levels)
{
}


/**********************************************************************
 * Destructor
 */

MaxDateStatCalc::~MaxDateStatCalc(void)
{
}
  

/**********************************************************************
 * calcStatistic() - Calculate the statistic field using the given
 *                   information.
 *
 * Note that this method gets the data time from the forecast_time
 * field in the data_field field header, so this must be set appropriately
 * in the incoming data field.
 */

MdvxField *MaxDateStatCalc::calcStatistic(const DsMdvx &climo_file,
					  const MdvxField &data_field,
					  const DateTime &climo_time)
{
  // Construct the statistic field name

  string stat_field_name = data_field.getFieldHeader().field_name_long;
  
  // Extract the existing maximum date field from the climatology
  // file, if there  is one.  If there is an existing field, we will
  // update the running maximum date.  If there isn't an existing
  // field, we will create a new one.

  string climo_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MAX_DATE,
				stat_field_name).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *curr_max_date_field =
    climo_file.getField(climo_field_name.c_str());
  MdvxField *new_max_date_field = 0;
  
  if (curr_max_date_field == 0)
  {
    if (_debug)
      cerr << "   Creating " << climo_field_name << " statistic" << endl;
    
    new_max_date_field = _createField(data_field, climo_time);
  }
  else
  {
    if (_debug)
      cerr << "   Updating " << climo_field_name << " statistic" << endl;
    
    new_max_date_field = _updateField(climo_file, data_field,
				      *curr_max_date_field);
  }
  
  return new_max_date_field;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createField() - Create a new maximum climatology field from the
 *                  given data field.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *MaxDateStatCalc::_createField(const MdvxField &data_field,
					 const DateTime &climo_time) const
{
  static const string method_name = "MaxDateStatCalc::_createField()";

  // Create the field header
  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  Mdvx::field_header_t field_hdr = data_field_hdr;
  
  field_hdr.forecast_delta = 0;
  field_hdr.forecast_time = climo_time.utime();
  
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;

  field_hdr.missing_data_value = 0.0;
  field_hdr.bad_data_value = 0.0;
  
  STRcopy(field_hdr.field_name_long,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MAX_DATE,
				      field_hdr.field_name_long).c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name,
	  StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MAX_DATE,
				      field_hdr.field_name).c_str(),
	  MDV_SHORT_FIELD_LEN);
  
  // Create the new grid filled with the input data time.

  int grid_size = field_hdr.nx * field_hdr.ny * field_hdr.nz;
  ti32 *max_date_data = new ti32[grid_size];
  fl32 *data = (fl32 *)data_field.getVol();
  
  for (int i = 0; i < grid_size; ++i)
  {
    if (data[i] == data_field_hdr.bad_data_value ||
	data[i] == data_field_hdr.missing_data_value)
      max_date_data[i] = (ti32)field_hdr.missing_data_value;
    else
      max_date_data[i] = field_hdr.forecast_time;
  }
  
  return new MdvxField(field_hdr,
		       data_field.getVlevelHeader(),
		       (void *)max_date_data);
}


/*********************************************************************
 * _updateField() - Create a new maximum climatology field that is an
 *                  update of the current maximum field using the given
 *                  information.
 *
 * Return the newly created field on success, 0 on failure.
 */

MdvxField *MaxDateStatCalc::_updateField(const DsMdvx &climo_file,
					 const MdvxField &data_field,
					 const MdvxField &curr_max_date_field) const
{
  static const string method_name = "MaxDateStatCalc::_updateField()";

  // Make sure the fields match so we can update the climos

  if (!_fieldsMatch(data_field, curr_max_date_field))
    return 0;
  
  // To calculate the new max date, we need the current max date and the
  // current max.  The current max date field is already given.

  Mdvx::field_header_t data_field_hdr = data_field.getFieldHeader();
  
  string curr_max_field_name =
    StatNamer::getStatFieldName(Mdvx::CLIMO_TYPE_MAX,
				data_field_hdr.field_name_long).substr(0, MDV_LONG_FIELD_LEN);
  
  MdvxField *curr_max_field =
    climo_file.getField(curr_max_field_name.c_str());
  
  if (curr_max_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error extracting maximum field (" << curr_max_field_name
	 << ") from climo file" << endl;
    
    return 0;
  }
  
  // Create the new field as a copy of the original field

  MdvxField *new_max_date_field = new MdvxField(curr_max_date_field);
  
  // Loop through the new field values, updating the stat value
  // wherever we have new data.

  Mdvx::field_header_t max_date_field_hdr =
    new_max_date_field->getFieldHeader();
  
  ti32 *max_date_data = (ti32 *)new_max_date_field->getVol();
  fl32 *max_data = (fl32 *)curr_max_field->getVol();
  fl32 *data = (fl32 *)data_field.getVol();
  
  int volume_size =
    max_date_field_hdr.nx * max_date_field_hdr.ny * max_date_field_hdr.nz;
  
  for (int i = 0; i < volume_size; ++i)
  {
    if (data[i] == data_field_hdr.missing_data_value ||
	data[i] == data_field_hdr.bad_data_value)
      continue;
    
    if (max_data[i] == max_date_field_hdr.missing_data_value ||
	max_data[i] == max_date_field_hdr.bad_data_value ||
	data[i] > max_data[i])
      max_date_data[i] = data_field_hdr.forecast_time;
    
  } /* endfor - i */
  
  return new_max_date_field;
}
