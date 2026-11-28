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
///////////////////////////////////////////////
// GribMgr
//
// $Id: GribMgr.cc,v 1.10 2016/03/07 01:23:11 dixon Exp $
//
//////////////////////////////////////////////

#include <grib/GribVertType.hh>
#include <grib/LambertConformal.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/pmu.h>

#include "GribMgr.hh"
#include "RucIngest.hh"

using namespace std;

GribMgr::GribMgr() 
{
  _id  = new IdSec();
  _pds = new PDS();
  _gds = new GDS();
  _bms = new BMS();
  _bds = new BDS();
  _forecastTime = -1;
  _generateTime = -1;
}

GribMgr::~GribMgr() 
{
  delete _id;
  delete _pds;
  delete _gds;
  delete _bms;
  delete _bds;
}

int
GribMgr::inventoryRecord( ui08 *gribPtr )
{
  ui08 *sectionPtr = gribPtr;

  //
  // Unpack the grib identification section
  //
  if( _id->unpack( sectionPtr ) != 0 ) {
    cerr << "WARNING: Cannot unpack id section" << endl << flush;
    return( -2 );
  }
  //  sectionPtr += _pds->getSize();
  sectionPtr += 8;

  //
  // Unpack the product definition section
  //
  if( _pds->unpack( sectionPtr ) != 0 ) {
    cerr << "ERROR: Cannot unpack pds section" << endl << flush;
    return( -1 );
  }
  sectionPtr += _pds->getSize();
   
  //
  // try to only set time info once
  //
  if( _forecastTime < 0 ) {
    _forecastTime = _pds->getForecastTime();
    _generateTime = _pds->getGenerateTime();
  }
  //
  // Unpack the grid description section if it is present.
  //
  if( _pds->gdsUsed() ) {
    if( _gds->unpack( sectionPtr ) != 0 ) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      return( -1 );
    }
    sectionPtr += _gds->getSize();
  } else {
    cerr << "ERROR: No gds section" << endl << flush;
    return( -1 );
  }

   return( 0 );
}


int
GribMgr::unpackRecord( ui08 *gribPtr, int nx, int ny )
{
  ui08 *sectionPtr = gribPtr;
  ui08 *bitmapPtr  = NULL;

  int   nPts = 0;
  
  //
  // Unpack the grib identification section
  //

  PMU_auto_register("Unpacking ID section");

  if( _id->unpack( sectionPtr ) != 0 ) {
    cerr << "WARNING: Cannot unpack id section" << endl << flush;
    return( -2 );
  }
  sectionPtr += 8;

  //
  // Unpack the product definition section
  //

  PMU_auto_register("Unpacking PDS");

  if( _pds->unpack( sectionPtr ) != 0 ) {
    cerr << "ERROR: Cannot unpack pds section" << endl << flush;
    return( -1 );
  }
  sectionPtr += _pds->getSize();
   
  //  cout << "the record is " << _pds->getLongName() << " for level " 
  //  << _pds->getLevelVal() << endl << flush;

  //
  // Unpack the grid description section if it is present.
  // If it isn't present, see if it is a predefined grid.
  // If it is, set the grid description section to look like
  // that predefined grid.  If it isn't, we don't know
  // what the grid looks like.  Error out in this case.
  //

  PMU_auto_register("Unpacking GDS");

  if( _pds->gdsUsed() ) {
    if( _gds->unpack( sectionPtr ) != 0 ) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      return( -1 );
    }
    sectionPtr += _gds->getSize();
  } 

  //
  // If the bit map section is present, set it 
  //

  PMU_auto_register("Unpacking BMS");

  if( _pds->bmsUsed() ) {
    if( _bms->unpack( sectionPtr, nPts ) != 0 ) {
      cerr << "ERROR: Cannot unpack bms section" << endl << flush;
      return( -1 );
    }
    bitmapPtr = _bms->getBitMap();
    sectionPtr += _bms->getSize();
     
  }

  nPts = nx*ny;

  //
  // Unpack the binary data section
  //

  PMU_force_register("Unpacking BDS");

  if( _bds->unpack( sectionPtr, nPts, _pds->getDecimalScale(),
                    nx, bitmapPtr ) != 0 ) {
    cerr << "ERROR: Cannot unpack data" << endl << flush;
    return( -1 );
  }

  return( 0 );
}


int
GribMgr::getMdvVerticalLevelType()
{
  GribVertType::vert_type_t grib_type = _pds->getVerticalLevelType();
  
  switch (grib_type)
  {
  case GribVertType::SURFACE :
  case GribVertType::POTENTIAL_VORTICITY :
  case GribVertType::UNKNOWN_211 :
  case GribVertType::UNKNOWN_212 :
  case GribVertType::UNKNOWN_213 :
  case GribVertType::UNKNOWN_214 :
  case GribVertType::UNKNOWN_222 :
  case GribVertType::UNKNOWN_223 :
  case GribVertType::UNKNOWN_224 :
  case GribVertType::UNKNOWN_232 :
  case GribVertType::UNKNOWN_233 :
  case GribVertType::UNKNOWN_234 :
  case GribVertType::UNKNOWN_242 :
  case GribVertType::UNKNOWN_244 :
    return Mdvx::VERT_TYPE_SURFACE;
    
  case GribVertType::ISOBARIC :
  case GribVertType::BETWEEN_PRS_DIFF :
    return Mdvx::VERT_TYPE_PRESSURE;
    
  case GribVertType::MEAN_SEA_LEVEL :
  case GribVertType::ALTITUDE_ABOVE_MSL :
  case GribVertType::HEIGHT_ABOVE_GROUND :
  case GribVertType::SIGMA :
  case GribVertType::BETWEEN_SIGMA :
  case GribVertType::DEPTH_BELOW_LAND_SURFACE :
  case GribVertType::BETWEEN_DEPTH :
    return Mdvx::VERT_TYPE_Z;
    
  case GribVertType::HYBRID :
    return Mdvx::VERT_TYPE_SIGMA_Z;
    
  case GribVertType::CLOUD_BASE :
  case GribVertType::CLOUD_TOPS :
  case GribVertType::ZERO_ISOTHERM :
  case GribVertType::MAX_WIND :
  case GribVertType::TROPOPAUSE :
  case GribVertType::NOMINAL_ATM_TOP :
  case GribVertType::ENTIRE_ATMOSPHERE :
  case GribVertType::UNKNOWN_10 :
  case GribVertType::FREEZING_LEVEL :
  case GribVertType::CONV_CLD_TOP :
  case GribVertType::MAX_EQ_THETA_PRESSURE :
  case GribVertType::EQUILIBRIUM_LEVEL_HEIGHT :
    return Mdvx::VERT_FIELDS_VAR_ELEV;
    
  default:
    return -1;
    
  } /* endswitch - grib_type */
}

