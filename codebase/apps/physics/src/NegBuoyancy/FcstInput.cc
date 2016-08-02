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
/**
 *
 * @file FcstInput.cc
 *
 * @class FcstInput
 *
 * Class that retrieve input files from RAP forecast directories.
 *  
 * @date 6/11/2010
 *
 */

#include "FcstInput.hh"


// Global variables


/*********************************************************************
 * Constructor
 */

FcstInput::FcstInput(const bool debug_flag) :
  Input(debug_flag)
{
}


/*********************************************************************
 * Destructor
 */

FcstInput::~FcstInput()
{
}


/*********************************************************************
 * init()
 */

bool FcstInput::init()
{
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _setReadTime()
 */

void FcstInput::_setReadTime(DsMdvx &request,
			     const string &url,
			     const string &field_name,
			     const int max_input_valid_secs,
			     const TriggerInfo &trigger_info) const
{
  time_t data_time = trigger_info.getIssueTime();
  int lead_time =
    trigger_info.getForecastTime() - trigger_info.getIssueTime();
    
  request.setReadTime(Mdvx::READ_SPECIFIED_FORECAST,
		      url,
		      max_input_valid_secs,
		      data_time,
		      lead_time);

  if (_debug)
  {
    cerr << "Reading " << field_name << " field" << endl;
    cerr << "   Issue time: " << DateTime::str(trigger_info.getIssueTime()) << endl;
    cerr << "   Valid time: " << DateTime::str(trigger_info.getForecastTime()) << endl;
  }
  
}

