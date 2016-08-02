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
// RainGauge.hh
//
// Class representing a RainGauge.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2000
//
///////////////////////////////////////////////////////////////

#ifndef RainGauge_H
#define RainGauge_H

#include <iostream>
#include <string>

#include <euclid/WorldPoint2D.hh>
#include <shapelib/shapefil.h>
using namespace std;


class RainGauge
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructors

  RainGauge(const bool debug_flag = false);

  RainGauge(const WorldPoint2D &location,
	    const int id,
	    const string &afos_id,
	    const string &name,
	    const int elevation,
	    const int instrument,
	    const bool debug_flag = false);


  // destructor
  
  ~RainGauge();


  // Load the gauge information from the given shape file.
  //
  // Returns true if the gauge information was successfully loaded, false
  // otherwise.

  bool loadShapeInfo(const string shape_file_base,
		     const int record_number);
  
  bool loadShapeInfo(const DBFHandle dbf_handle,
		     const int record_number);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the current RainGauge information to the given stream for
  // debugging purposes.

  void print(ostream &stream) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Retrieve the gauge id number

  int getId(void) const
  {
    return _id;
  }
  

  // Retrieve the gauge AFOS id

  string getAfosId(void) const
  {
    return _afosId;
  }
  

  // Retrieve the gauge location

  WorldPoint2D getLocation(void) const
  {
    return _location;
  }
  

protected:
  
private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const string LAT_FIELD_NAME;
  static const string LON_FIELD_NAME;
  static const string ID_FIELD_NAME;
  static const string AFOS_ID_FIELD_NAME;
  static const string NAME_FIELD_NAME;
  static const string ELEV_FIELD_NAME;
  static const string INST_FIELD_NAME;
  

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  WorldPoint2D _location;
  int _id;
  string _afosId;
  string _name;
  int _elevation;
  int _instrument;
  
  bool _dataLoaded;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("RainGauge");
  }
  
};

#endif
