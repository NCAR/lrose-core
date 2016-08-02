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
 *   $Date: 2016/03/03 19:23:53 $
 *   $Id: MapSimpleLabel.hh,v 1.3 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MapSimpleLabel: class representing a simple label in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MapSimpleLabel_HH
#define MapSimpleLabel_HH

#include <cstdio>
#include <string>

#include <rapformats/MapObject.hh>
#include <rapformats/MapPoint.hh>

using namespace std;

class MapSimpleLabel : public MapObject
{
 public:

  // Constructors

  MapSimpleLabel(void);
  
  MapSimpleLabel(const MapPoint icon_location,
		 const string &label = "");
  
  // Destructor

  ~MapSimpleLabel(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Read the label from the given file stream

  bool read(const char *header_line, FILE *stream);
  
  // Write the label to the given file stream.

  void write(FILE *stream) const;
  
 protected:

 private:

  // The label location.

  MapPoint _location;
  
  // The label information

  string _label;
  
};


#endif
