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
//   $Date: 2016/03/06 23:28:58 $
//   $Id: TemporalSmoother.cc,v 1.5 2016/03/06 23:28:58 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TemporalSmoother: Base class for ctrec classes that perform temporal
 *                   smoothing.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2002
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>

#include <Mdv/DsMdvx.hh>

#include "TemporalSmoother.hh"
using namespace std;



/**********************************************************************
 * Constructor
 */

TemporalSmoother::TemporalSmoother(const string &url,
				   const bool debug_flag) :
  _debug(debug_flag),
  _url(url)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

TemporalSmoother::~TemporalSmoother(void)
{
  // Do nothing
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _readField() - Read in the indicated field.
 *
 * Returns a pointer to the field on success, 0 on failure.
 */

MdvxField *TemporalSmoother::_readField(const string &field_name,
					const time_t data_time,
					const int searchMargin) const
{
  static const string method_name = "TemporalSmoother::_readField()";
  
  DsMdvx mdv_file;
  
  // Set up the request

  mdv_file.clearRead();
  mdv_file.clearReadFields();
  
  mdv_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
		       _url,
		       searchMargin, data_time);
  
  mdv_file.addReadField(field_name);
  
  mdv_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdv_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdv_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_debug)
  {
    cerr << "---> Read request for temporal smoothing field: "
	 << field_name << endl;
    mdv_file.printReadRequest(cerr);
  }
  
  // Read the data file

  if (mdv_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading temporal smoothing field: " << field_name << endl;
    cerr << "    url: " << _url << endl;
    cerr << "    data time: " << data_time << endl;
    cerr << mdv_file.getErrStr() << endl;
    
    return 0;
  }
  
  // Create a copy of the field to return

  MdvxField *field = mdv_file.getField(0);
  
  if (field == 0)
    return 0;
  
  return new MdvxField(*field);
}
