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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:47:03 $
//   $Id: BeamFilter.cc,v 1.9 2016/03/07 01:47:03 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BeamFilter: Class for filtering the beam data in a DsRadarMsg.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>

#include "BeamFilter.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

BeamFilter::BeamFilter(const bool debug) :
  _debug(debug),
  _numGates(0),
  _numFields(0),
  _fieldsInitialized(false)
{
}


/*********************************************************************
 * Destructor
 */

BeamFilter::~BeamFilter() 
{
  _clearInputFields();
}


/*********************************************************************
 * init() - Initialize the beam filter.
 */

bool BeamFilter::init(const bool debug) 
{
  _debug = debug;

  return true;
}


/*********************************************************************
 * addFilterField() - Add a field to use as a filter.
 */

void BeamFilter::addFilterField(const string &filter_field_name,
				const double filter_value,
				const bool keep_data_above_filter_value)
{
  filter_field_t filter_field;
  
  filter_field.field_name = filter_field_name;
  filter_field.field_index = -1;
  filter_field.filter_value = filter_value;
  filter_field.keep_data_above_filter_value = keep_data_above_filter_value;

  _filterFields.push_back(filter_field);
}


/*********************************************************************
 * filterBeam() - Filter the data in the beam contained in the given
 *                radar message.  This method assumes that there is
 *                beam data in the given message and doesn't check
 *                for this.
 *
 * Returns true on success, false on failure.
 */

bool BeamFilter::filterBeam(DsRadarBeam &radar_beam) const
{
  static const string method_name = "BeamFilter::filterBeam()";

  // Make sure we have received enough information to process
  // the beam.

  if (!_fieldsInitialized || _numGates <= 0)
    return false;

  // Make sure the beam contains the amount of data that we are
  // expecting

  if (radar_beam.dataLen() != (int)(_numGates * _numFields))
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Beam is a different size than expected" << endl;
    cerr << "Expected " << (_numGates * _numFields) << " bytes" << endl;
    cerr << "Got " << radar_beam.dataLen() << " bytes" << endl;
    cerr << "Not filtering beam..." << endl;

    return false;
  }

  // Loop through all of the data in the beam, updating values
  // where appropriate

  ui08 *beam_buffer = radar_beam.data();

  for (unsigned int gate = 0; gate < _numGates; ++gate)
  {
    // Look through each of the filter fields to see if this gate
    // contains noise

    vector< filter_field_t >::const_iterator filter_field_info;
    bool gate_is_noise = true;
    
    for (filter_field_info = _filterFields.begin();
	 filter_field_info != _filterFields.end(); ++filter_field_info)
    {
      // Make sure we have access to the indicated filter field.  If we
      // don't, we can't filter the field.

      if (filter_field_info->field_index < 0)
      {
	if (_debug)
	  cerr << "Filter field index = " << filter_field_info->field_index
	       << ", no filtering will be applied..." << endl;
	
	gate_is_noise = false;
	break;
      }
      
      // Get a pointer to the filter field

      DsFieldParams *filter_field =
	_inputFields[filter_field_info->field_index];

      // If the filter value is missing, we can't classify this gate
      // as noise

      ui08 scaled_filter_value =
	beam_buffer[gate * _numFields + filter_field_info->field_index];

      if (scaled_filter_value ==
	  _inputFields[filter_field_info->field_index]->missingDataValue)
      {
	if (_debug)
	  cerr << "Missing filter value, no filtering will be applied..." << endl;

	gate_is_noise = false;
	break;
      }

      // Now check the filter value itself

      double unscaled_filter_value =
	scaled_filter_value * filter_field->scale + filter_field->bias;

      if ((filter_field_info->keep_data_above_filter_value &&
	   unscaled_filter_value > filter_field_info->filter_value) ||
	  (!filter_field_info->keep_data_above_filter_value &&
	   unscaled_filter_value < filter_field_info->filter_value))
      {
	if (_debug)
	  cerr << "Filter value indicates not noise: " << unscaled_filter_value
	       << " (scaled value: " << (int)scaled_filter_value << ")" << endl;
	gate_is_noise = false;
	break;
      }
    } /* endfor - filter_field_info */
    
    if (gate_is_noise)
    {
      for (unsigned int field = 0; field < _numFields; ++field)
      {
	// Don't filter the filter fields

	vector< filter_field_t >::const_iterator filter_field_info;
	bool filter_field_flag = false;
	
	for (filter_field_info = _filterFields.begin();
	     filter_field_info != _filterFields.end(); ++filter_field_info)
	{
	  if (filter_field_info->field_name == _inputFields[field]->name)
	  {
	    filter_field_flag = true;
	    break;
	  }
	}
	
	if (!filter_field_flag)
	  beam_buffer[gate * _numFields + field] =
	    _inputFields[field]->missingDataValue;
      } /* endfor - field */
    } /* endif - gate_is_noise */
    
  } /* endfor - gate */

  return true;
}


/*********************************************************************
 * updateFieldInfo() - Update the field information for the filter.
 *                     This method assumes that there is field information
 *                     in the given message and doesn't check for this.
 *                     This method should be called every time a message
 *                     is received that contains field parameters.
 *
 * Returns true on success, false on failure.
 */

bool BeamFilter::updateFieldInfo(const vector< DsFieldParams* > input_fields)
{
  static const string method_name = "BeamFilter::updateFieldInfo()";

  _fieldsInitialized = false;

  // Clear out the field index numbers for the filter fields

  vector< filter_field_t >::iterator filter_field_info;
  for (filter_field_info = _filterFields.begin();
       filter_field_info != _filterFields.end(); ++filter_field_info)
    filter_field_info->field_index = -1;
  
  // Do a quick check to make sure that the number of fields
  // seems correct.

  if (_numFields != input_fields.size())
  {
    if (_numFields > 0)
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Number of fields in field params doesn't match number of fields specified in radar params" << endl;
      cerr << "Radar params indicates " << _numFields << " fields" << endl;
      cerr << "Field params has " << input_fields.size() << " fields" << endl;
      cerr << "Will assume the field params is correct" << endl;
    }

    _numFields = input_fields.size();
  }

  // Clear out the old local field information

  _clearInputFields();

  // Find the filter field indices

  for (unsigned int i = 0; i < _numFields; ++i)
  {
    // Copy the field information into our local vector.

    _inputFields.push_back(new DsFieldParams(*input_fields[i]));

    // See if this is a filter field

    for (filter_field_info = _filterFields.begin();
	 filter_field_info != _filterFields.end(); ++filter_field_info)
    {
      if (input_fields[i]->name == filter_field_info->field_name)
      {
	filter_field_info->field_index = i;
	break;
      }
    } /* endfor - filter_field_info */

  } /* endfor - i */

  _fieldsInitialized = true;

  return true;
}


/*********************************************************************
 * updateParamInfo() - Update the radar parameters for the filter.
 *                     This method assumes that there is param information
 *                     in the given message and doesn't check for this.
 *                     This method should be called every time a message
 *                     is received that contains radar parameters.
 *
 * Returns true on success, false on failure.
 */

bool BeamFilter::updateRadarInfo(const DsRadarParams &radar_params)
{
  // Set the internal values based on the given parameters

  _numFields = radar_params.numFields;
  _numGates = radar_params.numGates;

  return true;
}


/*********************************************************************
 * _clearInputFields() - Clear out the current input field information.
 */

void BeamFilter::_clearInputFields()
{
  vector< DsFieldParams* >::iterator field_iter;

  for (field_iter = _inputFields.begin();
       field_iter != _inputFields.end(); ++field_iter)
    delete *field_iter;

  _inputFields.erase(_inputFields.begin(), _inputFields.end());
}
