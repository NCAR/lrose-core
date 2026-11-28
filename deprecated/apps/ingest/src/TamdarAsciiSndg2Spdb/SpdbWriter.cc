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
//   $Date: 2016/03/07 01:23:06 $
//   $Id: SpdbWriter.cc,v 1.2 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SpdbWriter: Base class for classes that write Sndg information to an
 *             SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
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
  _debug(debug_flag),
  _outputUrl(output_url),
  _expireSecs(expire_secs)
{
  _outputSpdb.setPutMode(Spdb::putModeAdd);
}

  
/*********************************************************************
 * Destructor
 */

SpdbWriter::~SpdbWriter()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
