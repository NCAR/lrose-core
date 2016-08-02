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
//   $Id: SpdbHandlerList.cc,v 1.4 2016/03/07 18:36:49 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SpdbHandlerList: Class controlling a list of SpdbHandler objects.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "SpdbHandlerList.hh"
using namespace std;


/**********************************************************************
 * Constructor
 */

SpdbHandlerList::SpdbHandlerList(const bool debug_flag) :
  _debug(debug_flag)
{
}


/**********************************************************************
 * Destructor
 */

SpdbHandlerList::~SpdbHandlerList(void)
{
}
  

/**********************************************************************
 * readSpdbDatabase() - Read the appropriate data from the SPDB database.
 *
 * Returns true on success, false on failure.
 */

bool SpdbHandlerList::readSpdbDatabase(const DateTime &trigger_time,
				       const int search_margin)
{
  static const string method_name = "SpdbHandlerList::readSpdbDatabase()";
  
  vector< SpdbHandler >::iterator handler_iter;
  
  for (handler_iter = _handlerList.begin();
       handler_iter != _handlerList.end(); ++handler_iter)
  {
    if (!handler_iter->readSpdbDatabase(trigger_time,
					search_margin))
      return false;
  }


  if (_handlerList.size() > 0)
    _basinIdList = _handlerList[0].getBasinIdList();
  else
    _basinIdList.erase(_basinIdList.begin(), _basinIdList.end());
  
  return true;
}

  
/**********************************************************************
 * writeFieldToXmlFile() - Write the given field data to the given XML
 *                         file.
 *
 * Returns true on success, false on failure.
 */

bool SpdbHandlerList::writeFieldToXmlFile(const XmlFile &xml_file,
					  const string &field_name) const
{
  vector< int >::iterator basin_id_iter;
  
  for (basin_id_iter = _basinIdList.begin();
       basin_id_iter != _basinIdList.end(); ++ basin_id_iter)
  {
    int basin_id = *basin_id_iter;
    
    // Print the beginning basin tag

    xml_file.writeBasinBeginTag(basin_id);

    vector< SpdbHandler >::iterator spdb_iter;
    
    for (spdb_iter = _handlerList.begin();
	 spdb_iter != _handlerList.end(); ++ spdb_iter)
    {
      int time_offset = spdb_iter->getTimeOffset();
      double data_value = spdb_iter->getDataValue(basin_id, field_name);
      
      xml_file.writeDataValue(time_offset, data_value);
    } /* endfor - spdb_iter */
    
    // Print the ending basin tag

    xml_file.writeBasinEndTag();
      
  } /* endfor - basin_id_iter */
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
