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
 * SpdbWriter: Base class for classes that write observation information to an
 *             SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2006
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <cstdio>

#include "SpdbWriter.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

SpdbWriter::SpdbWriter(const string &output_url,
		       const int expire_secs,
		       const bool debug_flag) :
  Writer(output_url, expire_secs, debug_flag)
{

  _outputSpdb.setPutMode(Spdb::putModeAdd);
}

  
/*********************************************************************
 * Destructor
 */

SpdbWriter::~SpdbWriter()
{
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool SpdbWriter::init()
{
  static const string method_name = "SpdbWriter::init()";
  return true;
}


/*********************************************************************
 * writeInfo() - Write the observation information.
 *
 * Returns true if the write was successful, false otherwise.
 */

bool SpdbWriter::writeInfo()
{
  static const string method_name = "SpdbWriter::writeInfo()";

  if (_outputSpdb.put(_outputUrl, SPDB_GENERIC_POINT_ID, SPDB_GENERIC_POINT_LABEL) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing obs info to spdb database." << endl;
      return false;
    }

  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
