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
/////////////////////////////////////////////////////////////
// RainGaugeList.hh
//
// Class representing a group of rain gauges.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2000
//
///////////////////////////////////////////////////////////////

#ifndef RainGaugeList_H
#define RainGaugeList_H

#include <iostream>
#include <map>

#include <hydro/RainGauge.hh>
using namespace std;


class RainGaugeList
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  RainGaugeList(const bool debug_flag = false);


  // destructor
  
  ~RainGaugeList();


  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // Add all of the gauges in the given shape file to the gauge list.
  // shape_file_base is the path of the shape files not including the
  // extensions.
  //
  // Returns true if successful, false otherwise.

  bool addFromShapeFile(const string shape_file_base);
  

  ///////////////////////
  // Iteration methods //
  ///////////////////////

  // Gets the first gauge in the gauge list.
  //
  // If successful, returns a pointer the the first gauge in the list.
  // This pointer points directly to the RainGauge object in the list so
  // any changes to this object will change the object in the list.
  // Also, this pointer should not be deleted.
  // If there are no gauges in the list, returns 0.

  RainGauge *getFirstGauge(void);
  
  // Gets the next gauge in the gauge list.  Note that getFirstGauge()
  // MUST be called before this method can be called.
  //
  // If successful, returns a pointer the the next gauge in the list.
  // This pointer points directly to the RainGauge object in the list so
  // any changes to this object will change the object in the list.
  // Also, this pointer should not be deleted.
  // If there are no more gauges in the list or if getFirstGauge()
  // hasn't been called, returns 0.

  RainGauge *getNextGauge(void);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the current gauge list information to the given stream for
  // debugging purposes.

  void print(ostream &stream);
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Retrieve the first gauge in the list with the given AFOS id.
  //
  // Returns a pointer to the first gauge in the list with the given
  // AFOS id, or 0 if no gauge in the list has the given id.
  //
  // Note that this method returns a pointer that points directly
  // into the rain gauge list so any changes made to this pointer
  // will change the internal list and this pointer should NOT be
  // freed by the client.

  RainGauge *getByAfosId(const string &afos_id)
  {
    map< int, RainGauge*, less<int> >::iterator gauge_iter;
    
    for (gauge_iter = _rainGaugeList.begin();
	 gauge_iter != _rainGaugeList.end();
	 ++gauge_iter)
    {
      if ((*gauge_iter).second->getAfosId() == afos_id)
	return (*gauge_iter).second;
    }
    
    return 0;
  }
  
protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  // The list of gauges

  map< int, RainGauge*, less<int> > _rainGaugeList;
  
  // The iterator for the gauge list

  map< int, RainGauge*, less<int> >::iterator _rainGaugeListIter;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("RainGaugeList");
  }
  
};

#endif
