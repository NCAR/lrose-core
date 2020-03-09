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
//   $Date: 2016/03/04 02:22:11 $
//   $Id: InputFcst.cc,v 1.2 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InputFcst: Class controlling access to an input forecast
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxField.hh>

#include "InputFcst.hh"

using namespace std;

/*********************************************************************
 * Constructor
 */

InputFcst::InputFcst(const string &url, const int fcst_lead_secs,
		     const bool fcst_stored_by_gen_time,
		     const bool debug_flag) :
  _debug(debug_flag),
  _url(url),
  _fcstLeadSecs(fcst_lead_secs),
  _fcstStoredByGenTime(fcst_stored_by_gen_time)
{
}


/*********************************************************************
 * Destructor
 */

InputFcst::~InputFcst()
{
}


/*********************************************************************
 * readData() - Read the indicated data for this forecast.
 */

bool InputFcst::readData(const DateTime &data_time, Mdvx &fcst_mdvx)
{
  static const string method_name = "InputFcst::readData()";
  
  // Set up the read request

  fcst_mdvx.clearRead();
  
  fcst_mdvx.setReadTime(Mdvx::READ_CLOSEST,
			_url, 0, data_time.utime());
  
  vector< string >::const_iterator field_name;
  for (field_name = _fieldNames.begin(); field_name != _fieldNames.end();
       ++field_name)
    fcst_mdvx.addReadField(*field_name);
  
  if (_debug)
    fcst_mdvx.printReadRequest(cerr);
  
  if (fcst_mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV data" << endl;
    cerr << "URL: " << _url << endl;
    cerr << "search time: " << data_time << endl;
    cerr << fcst_mdvx.getErrStr() << endl;
    
    return false;
  }
  
  _updateTimes(fcst_mdvx);
  
  return true;
}

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _updateTimes() - Update the times in the given file to make it
 *                  a true forecast file.
 */

void InputFcst::_updateTimes(Mdvx &fcst_mdvx)
{
  // Update the times in the master header

  Mdvx::master_header_t master_hdr = fcst_mdvx.getMasterHeader();
  
  if (_fcstStoredByGenTime)
  {
    master_hdr.time_gen = master_hdr.time_centroid;
    master_hdr.time_centroid = master_hdr.time_gen + _fcstLeadSecs;
  }
  else
  {
    master_hdr.time_gen = master_hdr.time_centroid - _fcstLeadSecs;
  }

  fcst_mdvx.setMasterHeader(master_hdr);
  
  // Update the times in each of the fields

  for (size_t i = 0; i < fcst_mdvx.getNFields(); ++i)
  {
    MdvxField *field = fcst_mdvx.getField(i);
    
    Mdvx::field_header_t field_hdr = field->getFieldHeader();
    
    field_hdr.forecast_time = master_hdr.time_centroid;
    field_hdr.forecast_delta = _fcstLeadSecs;
    
    field->setFieldHeader(field_hdr);
  } /* endfor - i */
  
}
