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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:39:55 $
 *   $Id: SpdbReader.hh,v 1.2 2016/03/07 01:39:55 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SpdbReader: Base class for classes that read the Sndg information from
 *             an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SpdbReader_H
#define SpdbReader_H

#include <rapformats/Sndg.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>

using namespace std;


class SpdbReader
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  SpdbReader(const string &input_url,
	     const bool debug_flag);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~SpdbReader();


  /*********************************************************************
   * readSoundings() - Read the soundings for the given time from the
   *                   SPDB database and convert them to sounding plus
   *                   format.  Return the soundings in the given vector.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  virtual bool readSoundings(const DateTime &data_time,
			     vector< Sndg > &sndg_vector) = 0;


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _inputUrl;
  DsSpdb _spdb;
  
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _readSpdbChunks() - Read the chunks for the given time from the SPDB
   *                     database.
   *
   * Returns TRUE on success, FALSE on failure.
   */

  virtual bool _readSpdbChunks(const DateTime &data_time);
  
};

#endif
