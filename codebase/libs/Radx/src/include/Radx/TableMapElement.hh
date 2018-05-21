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
// BufrFile.hh
//
// BUFR file wrapper
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2017
//
///////////////////////////////////////////////////////////////

#ifndef TableMapElement_HH
#define TableMapElement_HH

#include <map>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>
#include <iterator>
#include <sstream>
//#include <string.h>  // contains strtok
#include <stdio.h>

using namespace std;

class TableMapElement {

public:

  /// Constructor
  
  TableMapElement();
  TableMapElement(vector<unsigned short> keys);
  TableMapElement(string fieldName, int scale, string units, int referenceValue,
		  int dataWidthBits);
  //(string fieldName, int scale);

  bool IsText();
  
  /// Destructor
  
  ~TableMapElement();

  typedef enum {
    KEYS,
    DESCRIPTOR
  } TableMapElementType;


  
  typedef struct {
    string fieldName;
    int scale;
    string units;
    int referenceValue;
    int dataWidthBits;
  } Descriptor;

  //private:

  TableMapElementType _whichType;
  vector<unsigned short> _listOfKeys;
  Descriptor _descriptor;  

};
#endif
