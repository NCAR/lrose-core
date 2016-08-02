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
//////////////////////////////////////////////

#include <typeinfo>
#include <cassert>
#include <cstdlib>
#include "GribMgr.hh"

#include <grib/GribVertType.hh>
#include <grib/LambertConformal.hh>
#include <Mdv/Mdvx.hh>
#include <toolsa/pmu.h>
#include <grib/EquidistantCylind.hh>
#include <grib/PolarStereographic.hh>
#include <grib/constants.h>

#include "Grib2Mdv.hh"

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

  // Unpack the grib indicator section
  if( _id->unpack( sectionPtr ) != GRIB_SUCCESS ) {
    cerr << "WARNING: Cannot unpack id section" << endl << flush;
    return( -2 );
  }
  sectionPtr += _id->getSize();

  // Unpack the product definition section
  int numBytes = GribSection::_upkUnsigned3(sectionPtr[0], sectionPtr[1], sectionPtr[2]);
  _pds->setExpectedSize(numBytes);
  if( _pds->unpack( sectionPtr ) != GRIB_SUCCESS ) {
    cerr << "ERROR: Cannot unpack pds section" << endl << flush;
    return( -1 );
  }
  sectionPtr += _pds->getSize();

  // try to only set time info once
  // Note: Grib2Mdv only processes a single forecast time,
  // and this condition ensures the the forecast time is
  // the first one encountered in the grib file.  Some
  // ensemble grib files contain multiple forecast times.
  // Those later forecasts are ignored by Grib2Mdv.
  // Carl Drews - June 13, 2006
  if( _forecastTime < 0 ) {	// not yet set
    _forecastTime = _pds->getForecastTime();
    _generateTime = _pds->getGenerateTime();
  }

  // Unpack the grid description section if it is present.
  if( _pds->gdsUsed() ) {
    if( _gds->unpack( sectionPtr ) != GRIB_SUCCESS ) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      _gds->print(stdout);
      return( -1 );
    }

    // Equidistant Cylindrical projection must be handled specially
    if (_gds->getProjType() == GDS::EQUIDISTANT_CYL_PROJ_ID) {
      getEquidistantCylindrical(sectionPtr);
    }

    // Polar Stereographic projection must be handled specially
    if (_gds->getProjType() == GDS::POLAR_STEREOGRAPHIC_PROJ_ID) {
      getPolarStereographic(sectionPtr);
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

  // Unpack the grib indicator section
  PMU_auto_register("Unpacking ID section");

  if( _id->unpack( sectionPtr ) != GRIB_SUCCESS ) {
    cerr << "WARNING: Cannot unpack id section" << endl << flush;
    return( -2 );
  }
  sectionPtr += _id->getSize();

  // Unpack the product definition section
  PMU_auto_register("Unpacking PDS");

  int numBytes = GribSection::_upkUnsigned3(sectionPtr[0], sectionPtr[1], sectionPtr[2]);
  _pds->setExpectedSize(numBytes);
  if( _pds->unpack( sectionPtr ) != GRIB_SUCCESS ) {
    cerr << "ERROR: Cannot unpack pds section" << endl << flush;
    return( -1 );
  }
  sectionPtr += _pds->getSize();

  //  cout << "the record is " << _pds->getLongName() << " for level "
  //  << _pds->getLevelVal() << endl << flush;

  // Unpack the grid description section if it is present.
  // If it isn't present, see if it is a predefined grid.
  // If it is, set the grid description section to look like
  // that predefined grid.  If it isn't, we don't know
  // what the grid looks like.  Error out in this case.
  PMU_auto_register("Unpacking GDS");

  if( _pds->gdsUsed() ) {
    if( _gds->unpack( sectionPtr ) != GRIB_SUCCESS ) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      _gds->print(stdout);
      return( -1 );
    }

    // Equidistant Cylindrical projection must be handled specially
    if (_gds->getProjType()  == GDS::EQUIDISTANT_CYL_PROJ_ID) {
      getEquidistantCylindrical(sectionPtr);
    }

    // Polar Stereographic projection must be handled specially
    if (_gds->getProjType() == GDS::POLAR_STEREOGRAPHIC_PROJ_ID) {
      getPolarStereographic(sectionPtr);
    }

    sectionPtr += _gds->getSize();
  }

  // If the bit map section is present, set it
  PMU_auto_register("Unpacking BMS");

  if( _pds->bmsUsed() ) {
    if( _bms->unpack( sectionPtr ) != GRIB_SUCCESS ) {
      cerr << "ERROR: Cannot unpack bms section" << endl << flush;
      return( -1 );
    }
    bitmapPtr = _bms->getBitMap();
    sectionPtr += _bms->getSize();
  }

  // for irregular grids, the number of points must be tallied up by the gds
  int nPts = nx*ny;
  if (_pds->gdsUsed()) {
    nPts = _gds->getNumGridPoints();
  }

  // determine what kind of BDS grid we have
  bool regularGrid = true;
  if (_pds->gdsUsed()) {
    regularGrid = _gds->isRegular();
  }

  // Unpack the binary data section
  PMU_force_register("Unpacking BDS");

  int bdsUnpackResult = 0;
  if (regularGrid) {
    bdsUnpackResult = _bds->unpack( sectionPtr, nPts, _pds->getDecimalScale(),
                    nx, bitmapPtr );
  } else {
    bdsUnpackResult = _bds->unpack( sectionPtr, nPts, _pds->getDecimalScale(),
                    _gds->getNumPtsPerRow(), bitmapPtr );
  }

  // did we get it right?
  if (bdsUnpackResult != GRIB_SUCCESS) {
    cerr << "ERROR: Cannot unpack data" << endl << flush;
    return( -1 );
  }

  return( 0 );
}


int GribMgr::getEquidistantCylindrical(ui08 *sectionPtr)
{
  // unpack into temporary GDS
  EquidistantCylind *temp = new EquidistantCylind();
  if (temp->unpack(sectionPtr) != GRIB_SUCCESS) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      temp->print(stdout);
      return( -1 );
    }

  // temp->print(stdout);

  // replace the existing GDS (probably the base class GDS)
  delete _gds;
  _gds = temp;

  return 0;
}


int GribMgr::getPolarStereographic(ui08 *sectionPtr)
{
  // unpack into temporary GDS
  PolarStereographic *temp = new PolarStereographic();
  if (temp->unpack(sectionPtr) != GRIB_SUCCESS) {
      cerr << "ERROR: Cannot unpack gds section" << endl << flush;
      temp->print(stdout);
      return( -1 );
    }

  temp->print(stdout);

  // replace the existing GDS (probably the base class GDS)
  delete _gds;
  _gds = temp;

  return 0;
}



bool GribMgr::needsEnsembleChange()
{
  if (!_pds->isEnsemble())
    return false;

  // if no params set, what can we do?
  if (_paramsPtr == NULL) {
    cerr << "ERROR: No parameters set for ensemble map." << endl << flush;
    return false;
  }

  return true;
}


Params::code_name_map_t *GribMgr::getEnsembleMap()
{
  if (_paramsPtr == NULL)
    return NULL;

  return _paramsPtr->_ncep_code_name_map;
}


int GribMgr::getEnsembleMapSize()
{
  if (_paramsPtr == NULL)
    return 0;

  return _paramsPtr->ncep_code_name_map_n;
}


string GribMgr::getLongName()
{
  if (!needsEnsembleChange()) {
    return( _pds->getLongName() );
  }

  // retrieve the grib code
  int gribCode = _pds->getParameterId();

  // look for code in the NCEP ensemble table
  Params::code_name_map_t *ensembleMap = getEnsembleMap();
  for (int i = 0; i < getEnsembleMapSize(); i++) {
    if (gribCode == ensembleMap[i].code) {
      return( ensembleMap[i].description);
    }
  }

  // not found - use the standard description
  return( _pds->getLongName() );
}


string GribMgr::getName()
{
  if (!needsEnsembleChange()) {
    return( _pds->getName() );
  }

  // retrieve the grib code
  int gribCode = _pds->getParameterId();

  // look for code in the NCEP ensemble table
  Params::code_name_map_t *ensembleMap = getEnsembleMap();
  for (int i = 0; i < getEnsembleMapSize(); i++) {
    if (gribCode == ensembleMap[i].code) {
      return( ensembleMap[i].name);
    }
  }

  // not found - use the standard name
  return( _pds->getName() );
}


string GribMgr::getUnits()
{
  if (!needsEnsembleChange()) {
    return( _pds->getUnits() );
  }

  // retrieve the grib code
  int gribCode = _pds->getParameterId();

  // look for code in the NCEP ensemble table
  Params::code_name_map_t *ensembleMap = getEnsembleMap();
  for (int i = 0; i < getEnsembleMapSize(); i++) {
    if (gribCode == ensembleMap[i].code) {
      return( ensembleMap[i].units);
    }
  }

  // not found - use the standard units
  return( _pds->getUnits() );
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
  case GribVertType::DEPTH_BELOW_LAND_SURFACE :
  case GribVertType::BETWEEN_DEPTH :
    return Mdvx::VERT_TYPE_Z;

  case GribVertType::SIGMA :
  case GribVertType::BETWEEN_SIGMA :
    return Mdvx::VERT_TYPE_SIGMA_P;

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


void GribMgr::swapGridOrientationNS_2_SN()
// Ideally this swapping should be a virtual method of GDS and its derivatives.
{
  // cast to the correct projection
  if ((_gds->getProjType()  != GDS::EQUIDISTANT_CYL_PROJ_ID) &&
      (_gds->getProjType()  != GDS::GAUSSIAN_LAT_LON_PROJ_ID) &&
      (_gds->getProjType()  != GDS::LAMBERT_CONFORMAL_PROJ_ID)
      ){
    cerr << "GribMgr::swapGridOrientationNS_2_SN() : ";
    cerr << "Unsupported projection type : ";
    cerr << _gds->getProjType() << endl;
    cerr << "Lat lon is " << GDS::GAUSSIAN_LAT_LON_PROJ_ID;
    cerr << ", cylindrical is " << GDS::EQUIDISTANT_CYL_PROJ_ID;
    cerr << " and lambert is " << GDS::LAMBERT_CONFORMAL_PROJ_ID << endl;
    exit(-1);
  }

// I prefer dynamic_cast, but it doesn't compile.  Carl Drews - March 24, 2004
//  EquidistantCylind *equiDist = dynamic_cast<EquidistantCylind *>(_gds);


//
// The following code has a somewhat tortured history. Originally
// the 'equiDist' object was used to do some math. This seemed
// to cause problems - in 2007, Frank Hage removed some of
// the operations that used 'equiDist' (although the code is
// still in this file, commented out). In 2009 Niles Oien
// removed all references to 'equiDist' (again, the code is
// still here, commented out). The result is that the code
// is heavy with comments and somewhat confusing. I do feel that
// it is best to leave the original code in place, though, even
// though it can be recalled via CVS - it is at least an accurate
// reflection of the confusion surrounding the code.
//
// Niles Oien January 2009.
//

//
// equiDist is now not used, but I will leave it in
// as commented out code - Niles Oien January 2009 
// 
//  EquidistantCylind *equiDist = (EquidistantCylind *)_gds;
//  assert(equiDist != NULL);
//
// End of commented out code - Niles.

  // store grid data in a buffer
  fl32 *dataPtr = getData();
  MemBuf *gribData = new MemBuf();
  int numX = _gds->getNx();
  int numY = _gds->getNy();
  fl32 *bufPtr = (fl32 *)gribData->load((void *)dataPtr, numX * numY * sizeof (fl32));

  // swap while copying back into data
  int j = 0;
  for (int y = numY - 1; y >= 0; y--) {
    for (int x = 0;  x < numX; x++) {
      dataPtr[j] = bufPtr[(y * numX) + x];
      j++;
    }
  }

  // Note: The class instance of equiDist contains a Pjg class.
  // Why is this reversed many times.?
  // This should describe the data in the Grib arrays.
  // Removed in an attempt to find out where the min Lat is getting reversed
  // after 2 fields are output. - F. Hage May 2007.
  // 
  //
  // switch the beginning and end latitude to reflect reordering
  //double oldStartLat = equiDist->getStartLat();
  //equiDist->setStartLat(equiDist->getEndLat());
  //equiDist->setEndLat(oldStartLat);

  gribData->free();

  // Set the projection Grid Minimums
  Pjg projection(*getProjection());

  //
  // equiDist is now not used in the setting of the grid minimums - again,
  // I will leave the original code in, although commented out. Niles Oien
  // January 2009.
  //
  //  projection.setGridMins(projection.getMinx(), equiDist->getEndLat(), projection.getMinz());
  //
  // End of commented out code - new call to setGridMins() follows. Niles.
  //

  projection.setGridMins(projection.getMinx(), projection.getMiny(), projection.getMinz());

  setProjection(projection);

}


string GribMgr::uniqueFieldName(const string name, const int levelType)
{
  string suffix;

  switch (levelType) {
    case (1):
      suffix = string("_SURFACE");
      break;
    case (2):
      suffix = string("_CLD_BASE");
      break;
    case (3):
      suffix = string("_CLD_TOP");
      break;
    case (4):
      suffix = string("_0_ISO");
      break;
    case (5):
      suffix = string("_ADIABATIC_COND");
      break;
    case (6):
      suffix = string("_MAX_WIND");
      break;
    case (7):
      suffix = string("_TROPO");
      break;
    case (8):
      suffix = string("_NOM_TOP_OF_ATMOS");
      break;
    case (9):
      suffix = string("_SEA_BOTTOM");
      break;
    case (100):
      suffix = string("_ISOB");
      break;
    case (101):
      suffix = string("_LAYER_BTWN_ISOB_LVLS");
      break;
    case (102):
      suffix = string("_MSL");
      break;
    case (103):
      suffix = string("_ABOVE_MSL");
      break;
    case (104):
      suffix = string("_ABV_MSL_V");
      break;
    case (105):
      suffix = string("_SPH_ABV_GRD");
      break;
    case (107):
      suffix = string("_SIGMA");
      break;
    case (108):
      suffix = string("_LAYER_SIGMA");
      break;
    case (109 ):
      suffix = string("_HYBRID");
      break;
    case (110):
      suffix = string("_LAYER_BTWN_HYBRID_LVSL");
      break;
    case (111):
      suffix = string("_BELOW_SURF");
      break;
    case (112):
      suffix = string("_BTN_DEPTHS");
      break;
    case (113):
      suffix = string("_ISENTROPIC");
      break;
    case (114):
      suffix = string("_LAYER_BTWN_ISENTROPIC");
      break;
    case (115):
      suffix = string("_SPECIFIED_PRESS_DIFF");
      break;
    case (116):
      suffix = string("_BTN_LVLS");
      break;
    case (117):
      suffix = string("_POT_VORTICITY_SURF");
      break;
    case (200):
      suffix = string("_ATMOSPHERE");
      break;
    case (204):
      suffix = string("_FRZE_LVL");
      break;
    case (211):
      suffix = string("_TDL_CLD_CVR");
      break;
    case (212):
      suffix = string("_LOW_CLD_BOT_LVL");
      break;
    case (213):
      suffix = string("_LOW_CLD_TOP_LVL");
      break;
    case (214):
      suffix = string("_LOW_CLD_LAYR");
      break;
    case (222):
      suffix = string("_MID_CLD_BOT_LVL");
      break;
    case (223):
      suffix = string("_MID_CLD_TOP_LVL");
      break;
    case (224):
      suffix = string("_MID_CLD_LAYER");
      break;
    case (232):
      suffix = string("_HIGH_CLD_BOT_LVL");
      break;
    case (233):
      suffix = string("_HIGH_CLD_TOP_LVL");
      break;
    case (234):
      suffix = string("_HIGH_CLD_LAYER");
      break;
    case (243):
      suffix = string("_CVT_CLDTOP");
      break;
    case (244):
      suffix = string("_CVT_TOTCLD");
      break;
    case (246):
      suffix = string("_MAX_EPOT");
      break;
    case (247):
      suffix = string("_EQUIL");
      break;

    default:
      {
      suffix = string("_NO_LEVEL_STRING_AVAILABLE");
      char lvlType[128];
      sprintf(lvlType, "_%d",levelType);
      suffix.append(lvlType);
      }
  }

  return name + suffix;
}

