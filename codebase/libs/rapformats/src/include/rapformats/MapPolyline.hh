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
 *   $Id: MapPolyline.hh,v 1.6 2016/03/03 19:23:53 dixon Exp $
 *   $Revision: 1.6 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MapPolyline.hh: class representing a polyline in a map.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MapPolyline_HH
#define MapPolyline_HH

#include <string>
#include <vector>

#include <cstdio>

#include <rapformats/MapObject.hh>
#include <rapformats/MapPoint.hh>
using namespace std;

class MapPolyline : public MapObject
{
 public:

  // Constructor

  MapPolyline(void);
  
  // Destructor

  ~MapPolyline(void);
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////

  // Read the polyline from the given file stream

  bool read(const char *header_line, FILE *stream);
  
  // Write the polyline to the given file stream.

  void write(FILE *stream) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  inline void addVertex(MapPoint& vertex)
  {
    MapPoint new_vertex(vertex);
    
    _vertexList.push_back(new_vertex);
  }
  
  inline size_t nVertices() const
  {
    return _vertexList.size();
  }
  
  inline MapPoint getVertex(const size_t index) const
  {
    if (index >= _vertexList.size())
    {
      MapPoint return_pt;
      
      return_pt.lat = MapPoint::PEN_UP;
      return_pt.lon = MapPoint::PEN_UP;
      
      return return_pt;
    }
    
    return _vertexList[index];
  }
  
 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  // The name associated with the polyline.

  string _polylineName;
  
  // The list of vertices in the polyline.

  vector< MapPoint > _vertexList;
  

private:

  /////////////////////
  // Private methods //
  /////////////////////

  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("MapPolyline");
  }
  
};


#endif
