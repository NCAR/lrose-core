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
 *   $Date: 2016/03/06 23:15:37 $
 *   $Revision: 1.3 $
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/************************************************************************
 * DeriveInterface: Class for interfacing with DeriveBase sub classes.
 *
 * How to add a new derivable variable:
 *  1) Create a new function class declaration. (in Wind.hh or Temperature.hh etc.)
 *  2) Create the constructor/derive/destruct and setOutputNames class functions
 *     in the accompanying .cc file. 
 *  3) Add the class to the getDeriveClassFromName and setOutputNamesFromClassName 
 *     functions in this file.
 *  4) Class can now be called by name from the parameter file. Add documentation
 *     about the derive function in paramdef.ModelDerive derive_functions variable.
 *
 * RAP, NCAR, Boulder CO
 * Jason Craig
 * Nov 2007
 *
 ************************************************************************/

#ifndef DeriveInterface_HH
#define DeriveInterface_HH

#include "DeriveBase.hh"
#include "Temperature.hh"
#include "Wind.hh"

namespace DeriveInterface {

  DeriveBase *getDeriveClassFromName(char *className, const float missing, const float bad, int nx, int ny, int nz)
  {
    int charLength = strlen( className );
    DeriveBase *classPtr = NULL;
    if( charLength >= 24 && strcasecmp( className, "WindSpeedDirectionFromUV" ) == 0 ) {
      classPtr = (DeriveBase *) new WindSpeedDirectionFromUV(missing, bad, nx, ny, nz);
    } else if( charLength >= 24 && strcasecmp( className, "AirTempFromVptmpMixrPres" ) == 0 ) {
      classPtr = (DeriveBase *) new AirTempFromVptmpMixrPres(missing, bad, nx, ny, nz);
    } else if( charLength >= 17 && strcasecmp( className, "RhFromTmpMixrPres" ) == 0 ) {
      classPtr = (DeriveBase *) new RhFromTmpMixrPres(missing, bad, nx, ny, nz);
    } else if( charLength >= 17 && strcasecmp( className, "RhFromTmpSpecPres" ) == 0 ) {
      classPtr = (DeriveBase *) new RhFromTmpSpecPres(missing, bad, nx, ny, nz);
    } else if( charLength >= 19 && strcasecmp( className, "RhFromVptmpMixrPres" ) == 0 ) {
      classPtr = (DeriveBase *) new RhFromVptmpMixrPres(missing, bad, nx, ny, nz);
    }

    return classPtr;
  }

  bool setOutputNamesFromClassName(char *className)
  {
    int charLength = strlen( className );
    if( charLength >= 24 && strcasecmp( className, "WindSpeedDirectionFromUV" ) == 0 ) {
      WindSpeedDirectionFromUV::setOutputNames();
    } else if( charLength >= 24 && strcasecmp( className, "AirTempFromVptmpMixrPres" ) == 0 ) {
      AirTempFromVptmpMixrPres::setOutputNames();
    } else if( charLength >= 17 && strcasecmp( className, "RhFromTmpMixrPres" ) == 0 ) {
      RhFromTmpMixrPres::setOutputNames();
    } else if( charLength >= 17 && strcasecmp( className, "RhFromTmpSpecPres" ) == 0 ) {
      RhFromTmpSpecPres::setOutputNames();
    } else if( charLength >= 19 && strcasecmp( className, "RhFromVptmpMixrPres" ) == 0 ) {
      RhFromVptmpMixrPres::setOutputNames();
    } else {
      return false;
    }

    return true;
  }


}

#endif
