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
 *   $Id: Tstorms2GenPoly.hh,v 1.3 2016/03/04 01:28:14 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Tstorms2GenPoly: Tstorms2GenPoly program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Tstorms2GenPoly_HH
#define Tstorms2GenPoly_HH


#include <string>

#include <dsdata/DsTrigger.hh>
#include <dsdata/Tstorm.hh>
#include <dsdata/TstormGroup.hh>
#include <dsdata/TstormMgr.hh>
#include <rapformats/GenPoly.hh>

#include "Args.hh"
#include "Params.hh"

using namespace std;


class Tstorms2GenPoly
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

  ~Tstorms2GenPoly(void);
  

  /*********************************************************************
   * Inst() - Retrieve the singleton instance of this class.
   */

  static Tstorms2GenPoly *Inst(int argc, char **argv);
  static Tstorms2GenPoly *Inst();
  

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

  /////////////////////
  // Private members //
  /////////////////////

  // Singleton instance pointer

  static Tstorms2GenPoly *_instance;
  
  // Program parameters.

  char *_progName;
  Args *_args;
  Params *_params;
  
  // Triggering object

  DsTrigger *_dataTrigger;
  
  // Current list of polygons created from Titan storms

  vector< GenPoly* > _polygons;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * Constructor -- private because this is a singleton object
   */

  Tstorms2GenPoly(int argc, char **argv);
  

  /*********************************************************************
   * _clearPolygons() - Clear out the current polygons vector.
   */

  inline void _clearPolygons()
  {
    vector< GenPoly*>::iterator poly_iter;
    
    for (poly_iter = _polygons.begin();
	 poly_iter != _polygons.end(); ++poly_iter)
      delete *poly_iter;
    
    _polygons.erase(_polygons.begin(), _polygons.end());
  }
  

  /*********************************************************************
   * _processData() - Process data for the given time.
   *
   * Returns true on success, false on failure.
   */

  bool _processData(const DateTime &data_time);
  

  /*********************************************************************
   * _processTitanStorm() - Convert the given Titan storm to GenPoly
   *                        format and add the polygon to the _polygons
   *                        vector.
   *
   * Returns true on success, false on failure.
   */

  bool _processTitanStorm(const Tstorm &tstorm);
  

  /*********************************************************************
   * _readTitanStorms() - Read and process the Titan storms for the
   *                      given time.
   *
   * Returns true on success, false on failure.  Upon successful return,
   * the _polygons vector will be updated to include all of the storms
   * read in for this data time.
   */

  bool _readTitanStorms(const DateTime &data_time);
  

  /*********************************************************************
   * _writePolygons() - Write the polygons to the output database.
   *
   * Returns true on success, false on failure.
   */
  
  bool _writePolygons(const DateTime &data_time);
  

};


#endif
