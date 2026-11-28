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
 *   $Date: 2016/03/07 01:23:06 $
 *   $Id: FlatLookupTable.hh,v 1.4 2016/03/07 01:23:06 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FlatLookupTable.hh : Class controlling the lookup table used for flat
 *                      projections in Terascan2Mdv.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FlatLookupTable_HH
#define FlatLookupTable_HH


#include <string>

// Include the needed Terascan include files.  Note that they were
// written assuming that people would only write in C, not in C++.

#ifdef __cplusplus
extern "C" {
#endif

#include <etx.h>
#include <terrno.h>

#ifdef __cplusplus
}
#endif

using namespace std;



class FlatLookupTable
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  FlatLookupTable(const string &lookup_filename = "");
  

  /*********************************************************************
   * Destructor
   */

  ~FlatLookupTable(void);
  

  /*********************************************************************
   * setFilename() - Sets the lookup table filename to the given value.
   */

  void setFilename(const string &lookup_filename)
  {
    _lookupFilename = lookup_filename;
  }
  

  /*********************************************************************
   * generate() - Generate a lookup table with the given parameters.
   *              If there is a lookup table saved to disk, read that
   *              table and use it.  Otherwise, generate the table and
   *              write it to disk.
   */

  bool generate(const int num_lines, const int num_samples,
		const double delta_x, const double delta_y,
		ETXFORM mxfm);
  
  /*********************************************************************
   * getIndex() - 
   */

  int getIndex(int i0, int j0);
  

 private:

  /////////////////////
  // Private members //
  /////////////////////

  string _lookupFilename;
  
  bool _lookupGenerated;
  
  int _numLines;
  int _numSamples;
  
  int *_mapOutput;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /*********************************************************************
   * _loadTableFromFile() - Load the lookup table from the file.
   *
   * Returns true if the table was loaded successfully, false otherwise.
   */

  bool _loadTableFromFile();
  

  /*********************************************************************
   * _writeTableToFile() - Writes the lookup table to the file.
   *
   * Returns true if the table was loaded successfully, false otherwise.
   */

  bool _writeTableToFile();
  

};


#endif
