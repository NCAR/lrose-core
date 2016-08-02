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
 *   $Date: 2016/03/04 02:22:12 $
 *   $Id: RwpHandler.hh,v 1.4 2016/03/04 02:22:12 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * RwpHandler: Base class for classes that supply the RWP field.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef RwpHandler_H
#define RwpHandler_H

#include <iostream>
#include <string>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>

using namespace std;


class RwpHandler
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  RwpHandler(const string &url,
	     const DateTime &gen_time, const DateTime &fcst_time);


  RwpHandler(const string &url,
	     const DateTime &time_centroid);
  
  /*********************************************************************
   * Destructor
   */

  virtual ~RwpHandler();


  /*********************************************************************
   * getRwpField() - Get the RWP field.
   *
   * Returns a pointer to the field on success, 0 on failure.  The pointer
   * is then owned by the calling method and must be deleted there.
   */

  virtual MdvxField *getRwpField() = 0;


  /*********************************************************************
   * getMasterHeader() - Get the master header from the input file.
   */

  Mdvx::master_header_t getMasterHeader()
  {
    return _mdvx.getMasterHeader();
  }

  /*********************************************************************
   * getNChunks() - return number of chunks
   */

  int getNChunks() const
  {
    return _mdvx.getNChunks();
  }

  /*********************************************************************
   * getChunkByNum() - return pointer to num'th chunk.
   */

  MdvxChunk *getChunkByNum(const int num)
  {
    return _mdvx.getChunkByNum(num);
  }


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  // Information about the input file

  string _url;
  DateTime _genTime;
  DateTime _fcstTime;
  DateTime _timeCentroid;

  DsMdvx _mdvx;
  
  bool _readForecast;

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _readInputData() - Read the indicated input data.
   *
   * Returns true on success, false on failure.
   */

  virtual bool _readInputData(const vector< string > &input_fields);
  

};

#endif
