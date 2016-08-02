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
//   $Date: 2016/03/07 01:39:55 $
//   $Id: SndgPlusReader.cc,v 1.2 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SndgPlusReader: Class that reads the Sndg information from an SPDB
 *                 sounding plus database.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "SndgPlusReader.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

SndgPlusReader::SndgPlusReader(const string &input_url,
			       const bool debug_flag) :
  SpdbReader(input_url, debug_flag)
{
}

  
/*********************************************************************
 * Destructor
 */

SndgPlusReader::~SndgPlusReader()
{
}


/*********************************************************************
 * readSoundings() - Read the soundings for the given time from the
 *                   SPDB database and convert them to sounding plus
 *                   format.  Return the soundings in the given vector.
 *
 * Returns TRUE on success, FALSE on failure.
 */

bool SndgPlusReader::readSoundings(const DateTime &data_time,
				   vector< Sndg > &sndg_vector)
{
  static const string method_name = "SndgPlusReader::readSoundings()";
  
  // Read the chunks from the database

  if (!_readSpdbChunks(data_time))
    return false;
  
  // Process the soundings

  const vector< Spdb::chunk_t > sndg_chunks = _spdb.getChunks();
  
  vector< Spdb::chunk_t >::const_iterator sndg_chunk;
  
  for (sndg_chunk = sndg_chunks.begin(); sndg_chunk != sndg_chunks.end();
       ++sndg_chunk)
  {
    // Convert the chunk to a sounding

    Sndg sounding;

    if (sounding.disassemble(sndg_chunk->data, sndg_chunk->len) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error converting SPDB chunk to Sndg format" << endl;
      cerr << "Skipping chunk" << endl;
      
      continue;
    }
    
    Sndg output_sndg(sounding);
    
    sndg_vector.push_back(output_sndg);
    
  } /* endfor - sndg_chunk */
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
