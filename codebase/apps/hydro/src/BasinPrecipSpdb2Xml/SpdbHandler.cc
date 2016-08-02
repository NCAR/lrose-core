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
//   $Date: 2016/03/07 18:36:49 $
//   $Id: SpdbHandler.cc,v 1.4 2016/03/07 18:36:49 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SpdbHandler: Class handling the output data for BasinPrecipSpdb2Xml.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <Spdb/DsSpdb.hh>

#include "SpdbHandler.hh"
using namespace std;


/**********************************************************************
 * Constructor
 */

SpdbHandler::SpdbHandler(const string &input_url,
			 const int time_offset_minutes,
			 const bool debug_flag) :
  _debug(debug_flag),
  _inputUrl(input_url),
  _timeOffsetSecs(time_offset_minutes * 60)
{
}


/**********************************************************************
 * Destructor
 */

SpdbHandler::~SpdbHandler(void)
{
}
  

/**********************************************************************
 * getDataValue() - Retrieves the data value for the given basin id
 *                  and field name.
 */

double SpdbHandler::getDataValue(const int basin_id,
				 const string &field_name)
{
  static const string method_name = "SpdbHandler::getDataValue()";
  
  // First get a pointer to the proper basin information

  vector< GenPt >::iterator basin_iter;
  vector< GenPt >::iterator basin;
  bool found = false;
  
  for (basin_iter = _basinList.begin();
       basin_iter != _basinList.end(); ++basin_iter)
  {
    if (basin_iter->getId() == basin_id)
    {
      basin = basin_iter;
      found = true;
      break;
    }
  } /* endfor - basin_iter */
  
  if (!found)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find basin ID " << basin_id
	 << " in SPDB data for time offset "
	 << (_timeOffsetSecs / 60) << " minutes." << endl;
    
    return -1.0;
  }
  
  // Now find the field information in the basin.

  int field_index = basin->getFieldNum(field_name);
      
  if (field_index < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Couldn't find field <" << field_name
	 << "> in database for time offset "
	 << (_timeOffsetSecs / 60) << " minutes." << endl;
	
    return -1.0;
  }

  return basin->get1DVal(field_index);
}


/**********************************************************************
 * readSpdbDatabase() - Read the appropriate data from the SPDB database.
 *
 * Returns true on success, false on failure.
 */

bool SpdbHandler::readSpdbDatabase(const DateTime &trigger_time,
				   const int search_margin)
{
  static const string method_name = "SpdbHandler::readSpdbDatabase()";
  
  // Empty the basin list

  _basinList.erase(_basinList.begin(), _basinList.end());
  
  // Read the data from the SPDB database

  DsSpdb spdb_input;
  
  if (spdb_input.getClosest(_inputUrl,
			    trigger_time.utime() - _timeOffsetSecs,
			    search_margin) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving data from SPDB database:" << endl;
    cerr << "    URL: " << _inputUrl << endl;
    cerr << "    Time: " << trigger_time << endl;
    cerr << "    search margin: " << search_margin << endl;
    
    return false;
  }
  
  if (_debug)
    cerr << "    Read " << spdb_input.getNChunks()
	 << " chunks for time offset " << _timeOffsetSecs << " seconds"
	 << endl;

  // Convert the chunks to GenPt objects and save them.

  vector< Spdb::chunk_t > spdb_chunks = spdb_input.getChunks();
  
  vector< Spdb::chunk_t >::iterator chunk_iter;
  
  for (chunk_iter = spdb_chunks.begin();
       chunk_iter != spdb_chunks.end(); ++chunk_iter)
  {
    GenPt basin_info;
    
    basin_info.disassemble(chunk_iter->data, chunk_iter->len);
    
    if (_debug)
      basin_info.print(cerr);
    
    _basinList.push_back(basin_info);
    
  } /* endfor - chunk_iter */
  
  return true;
}

  
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
