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
 *   $Id: SpdbHandler.hh,v 1.4 2016/03/07 18:36:49 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * SpdbHandler: Class handling the output data for BasinPrecipSpdb2Xml.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef SpdbHandler_HH
#define SpdbHandler_HH

#include <string>

#include <rapformats/GenPt.hh>
#include <toolsa/DateTime.hh>
using namespace std;


class SpdbHandler
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**********************************************************************
   * Constructor
   */

  SpdbHandler(const string &input_url,
	      const int time_offset_minutes,
	      const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~SpdbHandler(void);
  

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
  

  //////////////////////
  // Accessor methods //
  //////////////////////

  /**********************************************************************
   * getBasinIdList() - Generate a list of the basin IDs in the latest
   *                    SPDB data read in.
   */

  vector< int > getBasinIdList(void)
  {
    vector< int > basin_id_list;
    
    vector< GenPt >::iterator basin_iter;
    
    for (basin_iter = _basinList.begin();
	 basin_iter != _basinList.end(); ++ basin_iter)
      basin_id_list.push_back(basin_iter->getId());
    
    return basin_id_list;
  }
  

  /**********************************************************************
   * getDataValue() - Retrieves the data value for the given basin id
   *                  and field name.
   */

  double getDataValue(const int basin_id,
		      const string &field_name);
  

  /**********************************************************************
   * getTimeOffset() - Retrieves the time offset value for this object
   *                   in minutes.
   */

  int getTimeOffset(void) const
  {
    return _timeOffsetSecs / 60;
  }
  

  /**********************************************************************
   * setDebugFlag() - Sets the debug flag for this object.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

  /**********************************************************************
   * setTimeOffset() - Sets the time offset value for this object.  This
   *                   value is given in minutes.
   */

  void setTimeOffset(const int time_offset_minutes)
  {
    _timeOffsetSecs = time_offset_minutes * 60;
  }
  

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _inputUrl;
  int _timeOffsetSecs;
  
  vector< GenPt > _basinList;
  
};


#endif
