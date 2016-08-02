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
 *   $Date: 2016/03/04 01:28:14 $
 *   $Id: TitanMaxGrid.hh,v 1.2 2016/03/04 01:28:14 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TitanMaxGrid: TitanMaxGrid program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2015
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TitanMaxGrid_HH
#define TitanMaxGrid_HH


#include <string>

#include <dsdata/DsTrigger.hh>
#include <dsdata/Tstorm.hh>
#include <dsdata/TstormGroup.hh>
#include <dsdata/TstormMgr.hh>
#include <Mdv/DsMdvx.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class TitanMaxGrid
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  // Flag indicating whether the program status is currently okay.

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Destructor
   */

  ~TitanMaxGrid(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static TitanMaxGrid *Inst(int argc, char **argv);
  static TitanMaxGrid *Inst();
  

  /*********************************************************************
   * init() - Initialize the local data.
   *
   * Returns true if the initialization was successful, false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /*********************************************************************
   * run() - run the program.
   */

  void run();
  

 private:

  ///////////////////
  // Private types //
  ///////////////////

  typedef struct
  {
    double lat;
    double lon;
  } location_t;
     
  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static TitanMaxGrid *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Current storm data

  TstormMgr _tstormMgr;
  vector< Tstorm* > _storms;
  
  // Current gridded dataset

  DsMdvx _grid;
  
  // The gridded polygon. These members are global just for
  // efficiency purposes.

  unsigned char *_polygonGrid;
  int _polygonGridSize;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  TitanMaxGrid(int argc, char **argv);
  

  /*********************************************************************
   * _allocPolygonGrid() - Allocate space for the polygon grid and
   *                       clear the data.
   */

  inline void _allocPolygonGrid(const int nx, const int ny)
  {
    int new_grid_size = nx * ny;
    
    if (_polygonGridSize >= new_grid_size)
    {
      memset(_polygonGrid, 0, _polygonGridSize * sizeof(unsigned char));
      return;
    }
    
    if (_polygonGrid != 0)
      delete [] _polygonGrid;

    _polygonGrid = new unsigned char[new_grid_size];
    _polygonGridSize = new_grid_size;

    memset(_polygonGrid, 0, _polygonGridSize * sizeof(unsigned char));
  }
  

  /*********************************************************************
   * _clean() - Clean up memory
   */

  inline void _clean()
  {
    // Clean up the storm data

    _tstormMgr.clearData();
    _storms.clear();
    
    // Clean up the gridded data

    _grid.clear();
  }
  

  /*********************************************************************
   * _processData() - Process data for the given time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &data_time);
  

  /*********************************************************************
   * _processStorms() - Process the storms.  When finished, write the
   *                    output to the SPDB database.
   *
   * Returns true on success, false on failure.
   */

  bool _processStorms();
  

  /*********************************************************************
   * _readGriddedData() - Read the gridded data for the given time.
   *
   * Returns true on success, false on failure.  Upon successful return,
   * the _grid member will contain the gridded data.
   */

  bool _readGriddedData(const DateTime &data_time);
  

  /*********************************************************************
   * _readTitanStorms() - Read and process the Titan storms for the
   *                      given time.
   *
   * Returns true on success, false on failure.  Upon successful return,
   * the _polygons vector will be updated to include all of the storms
   * read in for this data time.
   */

  bool _readTitanStorms(const DateTime &data_time);
  

//  /*********************************************************************
//   * _writePolygons() - Write the polygons to the output database.
//   *
//   * Returns true on success, false on failure.
//   */
//  
//  bool _writePolygons(const DateTime &data_time);
  

};


#endif
