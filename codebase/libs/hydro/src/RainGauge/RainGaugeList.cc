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
///////////////////////////////////////////////////////////////
// RainGaugeList.cc
//
// Class representing a group of rain gauges.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>

#include <hydro/RainGauge.hh>
#include <hydro/RainGaugeList.hh>
#include <shapelib/shapefil.h>
using namespace std;


/*********************************************************************
 * Constructors
 */

RainGaugeList::RainGaugeList(const bool debug_flag) :
  _debugFlag(debug_flag)
{
  _rainGaugeListIter = _rainGaugeList.end();
}


/*********************************************************************
 * Destructor
 */

RainGaugeList::~RainGaugeList()
{
  // Reclaim space for the gauges

  map< int, RainGauge*, less<int> >::iterator gauge_iter;
  
  for (gauge_iter = _rainGaugeList.begin();
       gauge_iter != _rainGaugeList.end();
       ++gauge_iter)
    delete (*gauge_iter).second;
  
}


/*********************************************************************
 * addFromShapeFile() - Add all of the rain gauges in the given shape file
 *                      to the gauge list.  shape_file_base is the path
 *                      of the shape files not including the extensions.
 *
 * Returns true if successful, false otherwise.
 */

bool RainGaugeList::addFromShapeFile(const string shape_file_base)
{
  const string routine_name = "addFromShapeFile()";
  
  bool return_code = true;
  
  if (_debugFlag)
    cerr << "*** Reading shapes from " << shape_file_base << endl;

  // Open the database file

  DBFHandle dbf_handle;
  string dbf_file_name = shape_file_base + ".dbf";
  
  if ((dbf_handle = DBFOpen(dbf_file_name.c_str(), "rb")) == 0)
  {
    cerr << "ERROR: " << _className() << "::" << routine_name << endl;
    cerr << "Error opening database file: " << dbf_file_name << endl;
    
    return false;
  }
  
  // Retrieve the shape information from the file

  int n_records = DBFGetRecordCount(dbf_handle);
  
  if (_debugFlag)
    cerr << "   Database file contains " << n_records << " records" << endl;
  
  // Process each shape in the file

  for (int i = 0; i < n_records; ++i)
  {
    RainGauge *gauge = new RainGauge(_debugFlag);
    
    if (!gauge->loadShapeInfo(dbf_handle, i))
    {
      cerr << "ERROR: " << _className() << "::" << routine_name << endl;
      cerr << "Error loading gauge data from record " << i <<
	" in dbf file " << shape_file_base << endl;
      cerr << "--- Skipping rain gauge ---" << endl;
      
      delete gauge;
      return_code = false;

      continue;
    }
    
    _rainGaugeList[gauge->getId()] = gauge;
//    _rainGaugeList.push_back(gauge);
    
  } /* endfor - i */

  // Close the shape files since we're done with them.

  DBFClose(dbf_handle);
  
  return return_code;
}


/*********************************************************************
 * getFirstGauge() - Gets the first gauge in the gauge list.
 *
 * If successful, returns a pointer the the first gauge in the list.
 * This pointer points directly to the RainGauge object in the list so
 * any changes to this object will change the object in the list.
 * Also, this pointer should not be deleted.
 * If there are no gauges in the list, returns 0.
 */

RainGauge *RainGaugeList::getFirstGauge(void)
{
  _rainGaugeListIter = _rainGaugeList.begin();
  
  if (_rainGaugeListIter == _rainGaugeList.end())
    return 0;
  
  return (*_rainGaugeListIter).second;
}


/*********************************************************************
 * getNextGauge() - Gets the next gauge in the gauge list.  Note that
 *                  getFirstGauge() MUST be called before this method
 *                  can be called.
 *
 * If successful, returns a pointer the the next gauge in the list.
 * This pointer points directly to the RainGauge object in the list so
 * any changes to this object will change the object in the list.
 * Also, this pointer should not be deleted.
 * If there are no more gauges in the list, returns 0.
 */

RainGauge *RainGaugeList::getNextGauge(void)
{
  if (_rainGaugeListIter == _rainGaugeList.end())
    return 0;
  
  ++_rainGaugeListIter;
  
  if (_rainGaugeListIter == _rainGaugeList.end())
    return 0;
  
  return (*_rainGaugeListIter).second;
}


/*********************************************************************
 * print() - Print the current gauge list information to the given stream
 *           for debugging purposes.
 */

void RainGaugeList::print(ostream &stream)
{
  stream << "RainGaugeList information:" << endl;
  stream << "==========================" << endl;
  stream << endl;
  stream << "gauge list contains " << _rainGaugeList.size() <<
    " gauges." << endl;
  stream << endl;
  
  map< int, RainGauge*, less<int> >::iterator gauge_iter;
  int gauge_num;
  
  for (gauge_iter = _rainGaugeList.begin(), gauge_num = 0;
       gauge_iter != _rainGaugeList.end();
       ++gauge_iter, ++gauge_num)
  {
    stream << "Rain Gauge #" << gauge_num << ":" << endl;
    (*gauge_iter).second->print(stream);
    stream << endl;
  } /* endfor - gauge_iter */
  
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/
