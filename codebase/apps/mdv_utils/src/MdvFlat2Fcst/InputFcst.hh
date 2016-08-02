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
 *   $Date: 2016/03/04 02:22:11 $
 *   $Id: InputFcst.hh,v 1.2 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * InputFcst: Class controlling access to an input forecast
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef InputFcst_HH
#define InputFcst_HH

#include <string>
#include <vector>

#include <Mdv/Mdvx.hh>

using namespace std;

class InputFcst
{
 public:

  /*********************************************************************
   * Constructor
   */

  InputFcst(const string &url = "", const int fcst_lead_secs = 0,
	    const bool fcst_stored_by_gen_time = true,
	    const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~InputFcst(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  /*********************************************************************
   * readData() - Read the indicated data for this forecast.
   */

  bool readData(const DateTime &data_time, Mdvx &fcst_mdvx);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * addField() - Add a field to the field name list.
   */

  void addField(const string &field_name)
  {
    _fieldNames.push_back(field_name);
  }
  

  /*********************************************************************
   * getNumFields() - Get the number of fields used in this input.
   */

  int getNumFields()
  {
    return _fieldNames.size();
  }
  

  /*********************************************************************
   * getUrl() - Get the URL for this input.
   */

  string getUrl()
  {
    return _url;
  }
  

  /*********************************************************************
   * getFcstLeadSecs() - Get the forecast lead seconds for this input.
   */

  int getFcstLeadSecs()
  {
    return _fcstLeadSecs;
  }
  

  /*********************************************************************
   * getFcstStoredByGenTime() - Get the forecast stored by gen time flag
   *                            for this input.
   */

  bool getFcstStoredByGenTime()
  {
    return _fcstStoredByGenTime;
  }
  

  /*********************************************************************
   * setDebugFlag() - Set the debug flag for this input.
   */

  void setDebugFlag(const bool debug_flag)
  {
    _debug = debug_flag;
  }
  

  /*********************************************************************
   * setUrl() - Set the URL for this input.
   */

  void setUrl(const string &url)
  {
    _url = url;
  }
  

  /*********************************************************************
   * setFcstLeadSecs() - Set the forecast lead seconds for this input.
   */

  void setFcstLeadSecs(const int fcst_lead_secs)
  {
    _fcstLeadSecs = fcst_lead_secs;
  }
  

  /*********************************************************************
   * setFcstStoredByGenTime() - Set the forecast stored by gen time flag
   *                            for this input.
   */

  void setFcstStoredByGenTime(const bool fcst_stored_by_gen_time)
  {
    _fcstStoredByGenTime = fcst_stored_by_gen_time;
  }
  

 protected:

  bool _debug;
  
  string _url;
  int _fcstLeadSecs;
  bool _fcstStoredByGenTime;
  
  vector< string > _fieldNames;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * updateTimes() - Update the times in the given file to make it
   *                 a true forecast file.
   */

  void _updateTimes(Mdvx &fcst_mdvx);
  

};


#endif
