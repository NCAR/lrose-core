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
// BasinList.hh
//
// Class representing a group of watershed basins.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2000
//
///////////////////////////////////////////////////////////////

#ifndef BasinList_H
#define BasinList_H

#include <iostream>

#include <hydro/Basin.hh>
using namespace std;


class BasinList
{
  
public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  BasinList(const bool debug_flag = false);

  // destructor
  
  ~BasinList();


  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // Add all of the basins in the given shape file to the basin list.
  // shape_file_base is the path of the shape files not including the
  // extensions.
  //
  // Returns true if successful, false otherwise.

  bool addFromShapeFile(const string shape_file_base);
  

  ///////////////////////
  // Iteration methods //
  ///////////////////////

  // Gets the first basin in the basin list.
  //
  // If successful, returns a pointer the the first basin in the list.
  // This pointer points directly to the Basin object in the list so
  // any changes to this object will change the object in the list.
  // Also, this pointer should not be deleted.
  // If there are no basins in the list, returns 0.

  Basin *getFirstBasin(void);
  
  // Gets the next basin in the basin list.  Note that getFirstBasin()
  // MUST be called before this method can be called.
  //
  // If successful, returns a pointer the the next basin in the list.
  // This pointer points directly to the Basin object in the list so
  // any changes to this object will change the object in the list.
  // Also, this pointer should not be deleted.
  // If there are no more basins in the list or if getFirstBasin()
  // hasn't been called, returns 0.

  Basin *getNextBasin(void);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the current basin list information to the given stream for
  // debugging purposes.

  void print(ostream &stream) const;
  

protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  // The list of basins

  vector< Basin* > _basinList;
  
  // The iterator for the basin list

  vector< Basin* >::iterator _basinListIter;
  

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("Basin");
  }
  
};

#endif
