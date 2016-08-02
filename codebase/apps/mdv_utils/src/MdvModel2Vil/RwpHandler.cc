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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:12 $
//   $Id: RwpHandler.cc,v 1.4 2016/03/04 02:22:12 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RwpHandler: Base class for classes that supply the RWP field.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "RwpHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

RwpHandler::RwpHandler(const string &url,
		       const DateTime &gen_time,
		       const DateTime &fcst_time) :
  _url(url),
  _genTime(gen_time),
  _fcstTime(fcst_time)
{
  _readForecast = true;
}
RwpHandler::RwpHandler(const string &url,
		       const DateTime &time_centroid):

  _url(url),
  _timeCentroid(time_centroid)
{  
  _readForecast = false;
}
  
/*********************************************************************
 * Destructor
 */

RwpHandler::~RwpHandler()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _readInputData() - Read the indicated input data.
 *
 * Returns true on success, false on failure.
 */

bool RwpHandler::_readInputData(const vector< string > &input_fields)
{

  static const string method_name = "RwpHandler::_readInputData()";
  
  // Set up the read request.  Note that the rest of the code relies on
  // the order of the read fields so don't change this order unless you
  // change the other code.

  if (_readForecast)
    _mdvx.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, _url, 0,
		      _genTime.utime(), _fcstTime.utime() - _genTime.utime());
  else
    _mdvx.setReadTime(Mdvx::READ_CLOSEST, _url, 0,_timeCentroid.utime());

  vector< string >::const_iterator field_name;
  for (field_name = input_fields.begin(); field_name != input_fields.end();
       ++field_name)
    _mdvx.addReadField(*field_name);
  
  _mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Read the data

  if (_mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << _url << endl;
    cerr << _mdvx.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


