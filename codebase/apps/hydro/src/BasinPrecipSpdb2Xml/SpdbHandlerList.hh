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
 *   $Date: 2016/03/07 18:36:49 $
 *   $Id: SpdbHandlerList.hh,v 1.4 2016/03/07 18:36:49 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SpdbHandlerList: Class handling the output data for BasinPrecipSpdb2Xml.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SpdbHandlerList_HH
#define SpdbHandlerList_HH

#include <string>
#include <vector>

#include <toolsa/DateTime.hh>

#include "SpdbHandler.hh"
#include "XmlFile.hh"
using namespace std;


class SpdbHandlerList
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**********************************************************************
   * Constructor
   */

  SpdbHandlerList(const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~SpdbHandlerList(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /**********************************************************************
   * readSpdbDatabase() - Read the appropriate data from the SPDB database.
   *
   * Returns true on success, false on failure.
   */

  bool readSpdbDatabase(const DateTime &trigger_time,
			const int search_margin);
  

  /**********************************************************************
   * writeFieldToXmlFile() - Write the given field data to the given XML
   *                         file.
   *
   * Returns true on success, false on failure.
   */

  bool writeFieldToXmlFile(const XmlFile &xml_file,
			   const string &field_name) const;
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * addHandler() - Add the given SPDB handler to the list.
   */

  void addHandler(const SpdbHandler &handler)
  {
    _handlerList.push_back(handler);
  }
  

  /**********************************************************************
   * setDebugFlag() - Sets the debug flag for this object.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  mutable vector< SpdbHandler > _handlerList;
  mutable vector< int > _basinIdList;
  
};


#endif
