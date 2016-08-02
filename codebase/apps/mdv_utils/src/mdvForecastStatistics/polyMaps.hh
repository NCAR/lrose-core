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
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#ifndef POLY_MAPS_HH
#define POLY_MAPS_HH

#include <string>
#include <vector>
#include <ctime>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>


/**
 *
 * @file polyMaps.hh
 *
 * @class polyMaps
 *
 * Class to generate a map mask. The general idea here is that
 * you pass in an MDV master and field header - thereby defining
 * a grid - and the name of an ascii map file. This map file contains
 * a set of lat/lon polygons. For each polygon, it can be required
 * that a grid point be inside, or outside, the polygon (this is
 * specified in the map file).
 *
 * You get out a two dimensional array of ui08 (bytes) which are
 * set to polyMaps::polyMapRelevant if the grid point has passed
 * all the criteria (ie. it is inside the polygons it needs to
 * be inside of, and outside the ones it needs to be outside of).
 *
 * If the grid point has failed one or more criteria, the byte
 * value for that grid point is set to polyMaps::polyMapNotRelevant.
 *
 * The format of the map file is discussed in the param file for
 * the app.
 *
 * @author Niles Oien
 *
 */
using namespace std;

class polyMaps {
  
public:

  const static ui08 polyMapNotRelevant = 0;  
  const static ui08 polyMapRelevant = 1;

  typedef struct {
    int ix;
    int iy;
  } intPoint_t;

  typedef struct {
    bool retainPointsInside;
    vector <intPoint_t> intPoints;
  } indexMap_t;


/**
 * The constructor. The bulk of the work is done here - the
 * parsing of the map file and the generation of the mask bytes.
 *
 * @param mapFilename  path for the input ascii map file.
 * @param mhdr MDV master header type, for grid definition.
 * @param fhdr MDV field header type, for grid definition.
 *
 * @return  No return value.
 *
 * @todo cope more gracefully with points outside of the MDV grid.
 * At the moment they are simply ignored, after printing a warning
 * to that effect.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  polyMaps(string mapFilename,
	   Mdvx::master_header_t mhdr,
	   Mdvx::field_header_t fhdr);


/**
 * Allow access to the mask bytes.
 *
 * @return  Returns a pointer to the mask bytes, which are
 * set to either polyMaps::polyMapNotRelevant or polyMaps::polyMapRelevant.
 * Array is two dimensional, of size nx by ny as defined in field header.
 *
 * NULL is returned if the map file is not found.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  ui08 *getMapMask();

/**
 * Print the maps to stderr. Note that the lat/lon points have
 * been translated to integer values by the time they are printed.
 *
 * @return  void, no return value.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void printMaps();

/**
 * Destructor. Frees up memory.
 *
 * @return  void, no return value.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  ~polyMaps();
  
protected:
  
private:

  vector< indexMap_t > _Maps;
  ui08 *_mapMask;


/**
 * Internal protected method to determine if a grid point is
 * inside a certain polygon or outside it.
 *
 *
 * @param ix Integer X co-ordinate of point to test
 * @param iy Integer Y co-ordinate of point to test
 * @param intPoints Vector of points defining the boundary polygon.
 *
 * @return  A boolean, true if the point is inside the polygon,
 *          otherwise false.
 *
 * @todo This uses the routine in the euclid library to determine
 * if the point at ix,iy is inside the polygon. This means that
 * the vector has to be converted into an array of a similar but
 * slightly different type prior to calling the euclid routine.
 * It would be nice to pull that routine out of euclid and make
 * it compatible with the types we have here to avoid that type conversion
 * which is repeated a lot.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  bool _isInside(int ix, int iy, vector <polyMaps::intPoint_t> intPoints);

};

#endif
