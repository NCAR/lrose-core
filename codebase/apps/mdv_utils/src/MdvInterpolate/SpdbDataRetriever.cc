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
//   $Id: SpdbDataRetriever.cc,v 1.2 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SpdbDataRetriever: Class for retrieving SPDB data.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "SpdbDataRetriever.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

SpdbDataRetriever::SpdbDataRetriever(const string &input_url,
				     const int product_id) :
  _inputUrl(input_url)
{
  _spdb.setProdId(product_id);
}


/**********************************************************************
 * Destructor
 */

SpdbDataRetriever::~SpdbDataRetriever(void)
{
}
  

/**********************************************************************
 * getData() - Retrieve the SPDB data for the given time range.
 */

const vector< Spdb::chunk_t > SpdbDataRetriever::getData(const DateTime start_time,
							 const DateTime end_time,
							 const int data_type,
							 const int data_type2)
{
  static const string method_name = "SpdbDataRetriever::getData()";
  
  // Retrieve the data from the database

  if (_spdb.getInterval(_inputUrl,
			start_time.utime(),
			end_time.utime(),
			data_type,
			data_type2) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving data from SPDB database:" << endl;
    cerr << "   URL: " << _inputUrl << endl;
    cerr << "   Start time: " << start_time << endl;
    cerr << "   End time: " << end_time << endl;
    
    return _spdb.getChunks();
  }
  
  return _spdb.getChunks();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
