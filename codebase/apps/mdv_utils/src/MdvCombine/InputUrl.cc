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
/*
 *  $Id: InputUrl.cc,v 1.11 2016/03/04 02:22:10 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	InputUrl
//
// Author:	G M Cunning
//
// Date:	Wed Jan 17 13:53:04 2001
//
// Description: this class is a wrapper around the DsMdvxTimes class.
//


// C++ include files

// System/RAP include files
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/Mdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>

// Local include files
#include "InputUrl.hh"
#include "Params.hh"
#include "Args.hh"
using namespace std;


// define any constants
const string InputUrl::_className    = "InputUrl";


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

InputUrl::InputUrl(const string &url,
		   const int field_number, const string &field_name,
		   const int time_trigger_interval,
		   const bool debug) :
  _debug(debug),
  _url(url),
  _fieldNumber(field_number),
  _fieldName(field_name),
  _timeTriggerInterval(time_trigger_interval),
  _isOk(true)
{
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
InputUrl::~InputUrl() 
{
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	InputUrl::getNext
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void
InputUrl::getNext(const time_t& request_time)
{
  const string methodName = _className + string("::getNext");
  _readSuccess = false;

  _handle.setDebug(_debug);
  _handle.clearRead();
  _handle.setReadTime(Mdvx::READ_CLOSEST, _url, 
		      _timeTriggerInterval, request_time);

  _handle.clearReadFields();

  if (_fieldName == "")
    _handle.addReadField(_fieldNumber);
  else
    _handle.addReadField(_fieldName);
  
  if (_debug)
    _handle.printReadRequest(cerr);

  if (_handle.readVolume() < 0)
  {
    cerr << "ERROR: " << methodName << endl;
    cerr << _handle.getErrStr() << endl;

    return;
  }
  _path = _handle.getPathInUse();

  _readSuccess = true;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	InputUrl::getField
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

MdvxField* 
InputUrl::getField(void)
{
  MdvxField *field = _handle.getFieldByNum(0);
  return new MdvxField(*field);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	InputUrl::getMasterHeader
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

Mdvx::master_header_t*
InputUrl::getMasterHeader()
{
  _masterHeader = _handle.getMasterHeader();
  
  return &_masterHeader;
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	InputUrl::updateTimes
//
// Description:	
//
// Returns:	
//
// Globals:	
//
// Notes:
//
//

void 
InputUrl::updateTimes(time_t *start_time_p, time_t *end_time_p)
{
  Mdvx::master_header_t mhdr = _handle.getMasterHeader();

  if (*start_time_p < 0)
    *start_time_p = mhdr.time_begin;
  else
    *start_time_p = MIN((*start_time_p), mhdr.time_begin);

  if (*end_time_p < 0)
    *end_time_p = mhdr.time_end;
  else
    *end_time_p = MAX((*end_time_p), mhdr.time_end);

}
