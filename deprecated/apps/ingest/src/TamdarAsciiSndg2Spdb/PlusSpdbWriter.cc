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
//   $Id: PlusSpdbWriter.cc,v 1.2 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * PlusSpdbWriter: Class that writes Sndg information to a Sndg
 *                 SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>

#include "PlusSpdbWriter.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

PlusSpdbWriter::PlusSpdbWriter(const string &output_url,
			       const int expire_secs,
			       const bool debug_flag) :
  SpdbWriter(output_url, expire_secs, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

PlusSpdbWriter::~PlusSpdbWriter()
{
}


/*********************************************************************
 * writeSndg() - Write the given sounding to the output database.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool PlusSpdbWriter::writeSndg(const Sndg &sounding)
{
  static const string method_name = "PlusSpdbWriter::writeSndg()";
  
  Sndg output_sndg = sounding;
  
  output_sndg.assemble();
  Sndg::header_t sndg_hdr = output_sndg.getHeader();
  
  if (_outputSpdb.put(_outputUrl,
		      SPDB_SNDG_PLUS_ID,
		      SPDB_SNDG_PLUS_LABEL,
		      0,
		      sndg_hdr.launchTime,
		      sndg_hdr.launchTime + _expireSecs,
		      output_sndg.getBufLen(),
		      output_sndg.getBufPtr()) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing sounding to output URL: " << _outputUrl << endl;
    
    return false;
  }
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
