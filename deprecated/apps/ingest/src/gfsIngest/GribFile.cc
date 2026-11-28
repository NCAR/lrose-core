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

// GribFile - Representative of a Grib file, includes the file
//              inventory, etc.
//
//
//////////////////////////////////////////////////////////////////
#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <dirent.h>
#include <values.h>
#include <sys/stat.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/utim.h>
#include <toolsa/file_io.h>
#include <rapmath/math_macros.h>
#include <toolsa/umisc.h>
#include <cstdio>

#include <toolsa/MemBuf.hh>

#include "GribFile.hh"
#include "GFSrecord.hh"
#include "GfsIngest.hh"
using namespace std;

GribFile::GribFile (Params &params)
{
   _eofFound         = false;
   _gribRec          = new MemBuf();
  _paramsPtr = &params;

  _lastId = -999;
  _gribLen = 0; 

}

GribFile::~GribFile()
{


}

//
// GFS data changed the summer of 05 - UGRD/VGRD data is now
// interlased.  To revert back to the old style (all UGRD planes
// followed by all VGRD planes) call the following method
// All parameters will be sorted by Id, vertical level type, and
// actual level - (1000 mb will appear before 950, etc)
// 
void
GribFile::sortInventory () {
	sort (_inventory.begin(), _inventory.end(), compareFunc);

}

//
// Inventory consists of all grib records in uncompressed
// format

int
GribFile::addToInventory (FILE *fp, int nextGribPos)
{
  //ui08 *gribPtr = NULL;
  int gribLen = 0;

  GFSrecord *grbRec = new GFSrecord();

  gribLen = grbRec->loadGribRec(fp, nextGribPos);
  if (grbRec->eof()) {
      _eofFound = TRUE;
      delete (grbRec);
      return (0);
  } 
  // GFS data changed the summer of 05 - UGRD/VGRD data is now
  // interlased.  Revert back to the old style (all UGRD planes
  // followed by all VGRD planes) by sorting the _inventory
  // list after all records have been inserted
  
  _inventory.push_back(grbRec);

  return(gribLen);
   
}

//
// Traverse the vector to find a particular grib record
// Return the index of that record
//

int GribFile::findGribRec (const int productId, const int levelType, const int levelNum)
{
  int count = 0;
  int level_number = 0;
  _inv = _inventory.begin();

  while (_inv != _inventory.end()) {
    if (((*_inv)->getParameterId() == productId) && ((*_inv)->getVerticalLevelType() == levelType)) {
      if (level_number == levelNum) {
        return(count);
      }
      level_number++;
    }
    _inv++;
    count++;
  }
  return (RI_FAILURE);
}

void
GribFile::clearInventory() {

  _inv = _inventory.begin();

  while (_inv != _inventory.end()) {
    if (*_inv) {
      delete (*_inv);
    }
    _inv++;
  }
  //_inventory.erase(_inventory.begin(), _inventory.end());
  _inventory.clear();
  _eofFound = FALSE;
  _gribLen = 0;

}

void
GribFile::printInventory(FILE *stream ) {

  int numZlevels = 0;
  int prod_id = -1;
  int levelType = -1;
  //string *level; -> no longer using MDV values for level identifier
  inventory_t gribParam;

  vector <int> Zlevels;

  cout << "Size of the inventory is " << _inventory.size() << endl;
  _inv = _inventory.begin();
  while (_inv != _inventory.end()) {
    //find the first parameter ID in the inventory
	if  ((*_inv)->getParameterId() != prod_id || (*_inv)->getVerticalLevelType() != levelType) { 
        if (prod_id >= 0) {
          //level = cnvtLvl2String((*_inv)->getMdvVerticalLevelType());
		  fprintf(stream, "\n\n");
		  fprintf(stream, "--------------------------\n");
		  fprintf(stream, "%s, %d\n", gribParam.Name.c_str(),
		                                             gribParam.LevelType);
		  fprintf(stream, "     Product ID %d\n", gribParam.Id);
		  fprintf(stream, "      long name %s\n", gribParam.LongName.c_str());
		  fprintf(stream, "          units %s\n", gribParam.Units.c_str());
		  fprintf(stream, "  number levels %d\n", numZlevels);
		  fprintf(stream, "  forecast time %d\n", gribParam.ForecastTime/3600);
		  for (unsigned int i = 0;  i < Zlevels.size(); i++)
		    fprintf(stream, "           %d\n", Zlevels[i] );


		}
        gribParam.Id = (*_inv)->getParameterId();
        gribParam.LongName =  (*_inv)->getLongName();
        gribParam.Name = (*_inv)->getName();
        gribParam.Units = (*_inv)->getUnits();
		// grab value from grib file
        gribParam.LevelType = (*_inv)->getVerticalLevelType();
        //gribParam.levelType = _p->ZlevelNum;
        gribParam.ForecastTime = (*_inv)->getForecastTime();
		Zlevels.erase(Zlevels.begin(), Zlevels.end());

        prod_id = gribParam.Id;
        levelType = gribParam.LevelType;
        numZlevels = 1;
		Zlevels.push_back((*_inv)->getLevelVal());
    }
    else {
        numZlevels ++;
		Zlevels.push_back((*_inv)->getLevelVal());
    }
      _inv++;
    }
}

//
// printSummary is used to allow the user to have the system process a subset of the
// fields available in the grib file
//
void
GribFile::printSummary() {

  int prod_id = -1;
  int levelType = -1;
  inventory_t gribParam;

  _inv = _inventory.begin();
  while (_inv != _inventory.end()) {
    //find the first parameter ID in the inventory
	if  ((*_inv)->getParameterId() != prod_id || (*_inv)->getVerticalLevelType() != levelType) { 
        if (prod_id >= 0) {
		  fprintf(stdout, "%s, %d\n", gribParam.Name.c_str(),
		                                             gribParam.LevelType);


		}
        gribParam.Name = (*_inv)->getName();
        gribParam.LevelType = (*_inv)->getVerticalLevelType();

        prod_id = gribParam.Id;
        levelType = gribParam.LevelType;
    }
    _inv++;
  }
}

string
GribFile::uniqueParamNm(const string modelParam, const int levelType) {

  string *retString;

  switch (levelType) {
    case (1): 
      retString = new string("_SURFACE");
      break;
    case (2):
      retString = new string("_CLD_BASE");
      break;
    case (3):
      retString = new string("_CLD_TOP");
      break;
    case (4): 
      retString = new string("_0_ISO");
      break;
    case (5):
      retString = new string("_ADIABATIC_COND");
      break;
    case (6):
      retString = new string("_MAX_WIND");
      break;
    case (7):
      retString = new string("_TROPO");
      break;
    case (8):
      retString = new string("_NOM_TOP_OF_ATMOS");
      break;
    case (9):
      retString = new string("_SEA_BOTTOM");
      break;
    case (100): 
      retString = new string("_ISOB");
      break;
    case (101):
      retString = new string("_LAYER_BTWN_ISOB_LVLS");
      break;
    case (102):
      retString = new string("_MSL");
      break;
    case (103):
      retString = new string("_ABOVE_MSL");
      break;
    case (104):
      retString = new string("_ABV_MSL_V");
      break;
    case (105):
      retString = new string("_SPH_ABV_GRD");
      break;
    case (107):
      retString = new string("_99.5_SIG");
      break;
    case (108):
      retString = new string("_LAYER_SIGMA");
      break;
    case (109 ):
      retString = new string("_HYBRID");
      break;
    case (110):
      retString = new string("_LAYER_BTWN_HYBRID_LVSL");
      break;
    case (111):
      retString = new string("_BELOW_SURF");
      break;
    case (112):
      retString = new string("_BTN_DEPTHS");
      break;
    case (113):
      retString = new string("_ISENTROPIC");
      break;
    case (114):
      retString = new string("_LAYER_BTWN_ISENTROPIC");
      break;
    case (115):
      retString = new string("_SPECIFIED_PRESS_DIFF");
      break;
    case (116):
      retString = new string("_BTN_LVLS");
      break;
    case (117):
      retString = new string("_POT_VORTICITY_SURF");
      break;
    case (200):
      retString = new string("_ATMOSPHERE");
      break;
    case (204):
      retString = new string("_FRZE_LVL");
      break;
    case (211):
      retString = new string("_TDL_CLD_CVR");
      break;
    case (212):
      retString = new string("_LOW_CLD_BOT_LVL");
      break;
    case (213):
      retString = new string("_LOW_CLD_TOP_LVL");
      break;
    case (214):
      retString = new string("_LOW_CLD_LAYR");
      break;
    case (222):
      retString = new string("_MID_CLD_BOT_LVL");
      break;
    case (223):
      retString = new string("_MID_CLD_TOP_LVL");
      break;
    case (224):
      retString = new string("_MID_CLD_LAYER");
      break;
    case (232):
      retString = new string("_HIGH_CLD_BOT_LVL");
      break;
    case (233):
      retString = new string("_HIGH_CLD_TOP_LVL");
      break;
    case (234):
      retString = new string("_HIGH_CLD_LAYER");
      break;
    case (243):
      retString = new string("_CVT_CLDTOP");
      break;
    case (244):
      retString = new string("_CVT_TOTCLD");
      break;
    case (246):
      retString = new string("_MAX_EPOT");
      break;
    case (247):
      retString = new string("_EQUIL");
      break;

    default:
      {
      retString = new string("NO LEVEL STRING AVAILABLE");
      char lvlType[128]; 
      sprintf(lvlType, " -  %d",levelType);
      retString->append(lvlType);
      }
  }
  string MP = modelParam + *retString;
  delete retString;
  return (MP);

}

string
*GribFile::cnvtLvl2String(const int levelType) {

  string *retString;

  switch (levelType) {
    case (1): 
      retString = new string("GROUND_OR_WATER_SURFACE");
      break;
    case (2):
      retString = new string("CLOUD_BASE_LEVEL");
      break;
    case (3):
      retString = new string("CLOUD_TOP_LEVEL");
      break;
    case (4): 
      retString = new string("LEVEL_OF_0_DEGREE_ISOTHERM");
      break;
    case (5):
      retString = new string("ADIABATIC_CONDENSATION");
      break;
    case (6):
      retString = new string("MAXIMUM_WIND_LEVEL");
      break;
    case (7):
      retString = new string("TROPOPAUSE");
      break;
    case (8):
      retString = new string("NOMINAL_ATMOS_TOP");
      break;
    case (9):
      retString = new string("SEA_BOTTOM");
      break;
    case (100): 
      retString = new string("ISOBARIC");
      break;
    case (101): 
      retString = new string("BETWEEN_ISOB_LVLS");
      break;
    case (102):
      retString = new string("MEAN_SEA_LEVEL");
      break;
    case (103):
      retString = new string("ALTITUDE_ABOVE_MSL");
      break;
    case (104):
      retString = new string("ALTITUDE_ABOVE_MSL_V_COMP");
      break;
    case (105):
      retString = new string("SPECF_HEIGHT_ABOVE_GROUND");
      break;
    case (107):
      retString = new string("99.5_PERCENT_SIGMA_LEVELS");
      break;
    case (108):
      retString = new string("LAYER_SIGMA_LEVELS");
      break;
    case (109 ):
      retString = new string("HYBRID");
      break;
    case (110 ):
      retString = new string("LAYER_BETWEEN_HYBRID_LEVELS");
      break;
    case (111):
      retString = new string("DEPTH_BELOW_SURFACE");
      break;
    case (112):
      retString = new string("LAYER_BETWEEN_TWO_DEPTHS_BELOW_SURFACE");
      break;
    case (113):
      retString = new string("ISENTROPIC");
      break;
    case (114):
      retString = new string("LAYER_BETWEEN_ISENTROPIC");
      break;
    case (115):
      retString = new string("SPECIFIED_PRESS_DIFF");
      break;
    case (116):
      retString = new string("LAYER_BETWEEN_TWO_LEVELS_AT_SPECIFIED_PRESSURES");
      break;
    case (117):
      retString = new string("POTENTIAL_VORTICITY_SURF");
      break;
    case (200):
      retString = new string("ENTIRE_ATMOSPHERE");
      break;
    case (204):
      retString = new string("HIGHEST_TROPO_FREEZE_LEVEL");
      break;
    case (211):
      retString = new string("TOTAL_CLOUD_COVER");
      break;
    case (212):
      retString = new string("LOW_CLOUDS_BOTTOM_LEVEL");
      break;
    case (213):
      retString = new string("LOW_CLOUDS_TOP_LEVEL");
      break;
    case (214):
      retString = new string("LOW_CLOUD_LAYER");
      break;
    case (222):
      retString = new string("MID_CLOUD_BOTTOM_LEVEL");
      break;
    case (223):
      retString = new string("MID_CLOUD_TOP_LEVEL");
      break;
    case (224):
      retString = new string("MID_CLOUD_LAYER");
      break;
    case (232):
      retString = new string("HIGH_CLOUD_BOT_LEVEL");
      break;
    case (233):
      retString = new string("HIGH_CLOUD_TOP_LEVEL");
      break;
    case (234):
      retString = new string("HIGH_CLOUD_LAYER");
      break;
    case (243):
      retString = new string("CONVECTIVE_CLOUD_TOP_LEVEL");
      break;
    case (244):
      retString = new string("CONVECTIVE_TOTAL_CLOUD_COVER");
      break;
    case (246):
      retString = new string("MAXIMUM_EPOT_LEVEL");
      break;
    case (247):
      retString = new string("EQUILIBRIUM_LEVEL");
      break;

    default:
      {
      retString = new string("NO LEVEL STRING AVAILABLE");
      char lvlType[128]; 
      sprintf(lvlType, " -  %d",levelType);
      retString->append(lvlType);
      }
  }
  return retString;

}

Params::param_id_t
GribFile::cnvtParamId2enum(const int paramType) {

  Params::param_id_t retVal;

  switch (paramType) {
    case (1): 
      retVal = Params::PRES;
      break;
    case (2):
      retVal = Params::PRMSL;
      break;
    case (3):
      retVal = Params::PRESSURE_TENDENCY;
      break;
    case (4): 
      retVal = Params::HGT;
      break;
    case (5):
      retVal = Params::CLOUD_HEIGHT;
      break;
    case (7):
      retVal = Params::HGT;
      break;
    case (10):
      retVal = Params::TOZNE;
      break;
    case (11):
      retVal = Params::TMP;
      break;
    case (13):
      retVal = Params::POT;
      break;
    case (14):
      retVal = Params::EQUIVALENT_POTENTIAL_TEMP;
      break;
    case (15):
      retVal = Params::MAX_TEMP;
      break;
    case (16):
      retVal = Params::MIN_TEMP;
      break;
    case (17):
      retVal = Params::DEW_POINT_TEMPERATURE;
      break;
    case (18):
      retVal = Params::DEW_POINT_DEPRESSION;
      break;
    case (20):
      retVal = Params::VISIBILITY;
      break;
    case (27):
      retVal = Params::GPA;
      break;
    case (33):
      retVal = Params::UGRD;
      break;
    case (34):
      retVal = Params::VGRD;
      break;
    case (39):
      retVal = Params::VVEL;
      break;
    case (41):
      retVal = Params::ABSV;
      break;
    case (51):
      retVal = Params::SPFH;
      break;
    case (52):
      retVal = Params::RH;
      break;
    case (53):
      retVal = Params::HUMIDITY_MIXING_RATIO;
      break;
    case (54):
      retVal = Params::PWAT;
      break;
    case (59):
      retVal = Params::PRECIPITATION_RATE;
      break;
    case (61):
      retVal = Params::TOTAL_PRECIPITATION;
      break;
    case (62):
      retVal = Params::LARGE_SCALE_PRECIPITATION;
      break;
    case (63):
      retVal = Params::CONVECTIVE_PRECIPITATION;
      break;
    case (65):
      retVal = Params::WEASD;
      break;
    case (66):
      retVal = Params::SNOW_DEPTH;
      break;
    case (71):
      retVal = Params::TCDC;
      break;
    case (73):
      retVal = Params::LOW_CLOUD_COVER;
      break;
    case (74):
      retVal = Params::MED_CLOUD_COVER;
      break;
    case (75):
      retVal = Params::HIGH_CLOUD_COVER;
      break;
    case (76):
      retVal = Params::CWAT;
      break;
    case (77):
      retVal = Params::BEST_LIFTED_INDEX;
      break;
    case (81):
      retVal = Params::LAND;
      break;
    case (84):
      retVal = Params::ALBEDO;
      break;
    case (85):
      retVal = Params::SOIL_TEMPERATURE;
      break;
    case (86):
      retVal = Params::SOIL_MOISTURE_CONTENT;
      break;
    case (90):
      retVal = Params::WATER_RUNOFF;
      break;
    case (91):
      retVal = Params::ICEC;
      break;
    case (99):
      retVal = Params::SNOW_MELT;
      break;
    case (111): 
      retVal = Params::SHORTWAVE_RAD_AT_SURFACE;
      break;
    case (112): 
      retVal = Params::LONGWAVE_RAD_AT_SURFACE;
      break;
    case (113):
      retVal = Params::SHORTWAVE_RAD_AT_TOP;
      break;
    case (114):
      retVal = Params::LONGWAVE_RAD_AT_TOP;
      break;
    case (121):
      retVal = Params::LATENT_HEAT_FLUX;
      break;
    case (122):
      retVal = Params::SENSIBLE_HEAT_FLUX;
      break;
    case (124):
      retVal = Params::ZONAL_MOMENTUM_FLUX;
      break;
    case (128):
      retVal = Params::MEAN_SEA_LEVEL_PRESS_SAR;
      break;
    case (129 ):
      retVal = Params::MEAN_SEA_LEVEL_PRESS_MAPS;
      break;
    case (131 ):
      retVal = Params::LFTX;
      break;
    case (132):
      retVal = Params::FOUR_LFTX;
      break;
    case (135):
      retVal = Params::HORZ_MOISTURE_CONVERGENCE;
      break;
    case (136):
      retVal = Params::VWSH;
      break;
    case (140):
      retVal = Params::CATEGORICAL_RAIN;
      break;
    case (141):
      retVal = Params::CATEGORICAL_FREEZING_RAIN;
      break;
    case (142):
      retVal = Params::CATEGORICAL_ICE_PELLETS;
      break;
    case (143):
      retVal = Params::CATEGORICAL_SNOW;
      break;
    case (144):
      retVal = Params::SOILW;
      break;
    case (145):
      retVal = Params::POTENTIAL_EVAP_RATE;
      break;
    case (146):
      retVal = Params::CLD_WORK_FUNC;
      break;
    case (147):
      retVal = Params::ZONAL_GRAVITY_WAVE_STRESS;
      break;
    case (149):
      retVal = Params::POTENTIAL_VORTICITY;
      break;
    case (153):
      retVal = Params::CLWMR;
      break;
    case (154):
      retVal = Params::O3MR;
      break;
    case (155):
      retVal = Params::GND_HEAT_FLUX;
      break;
    case (156):
      retVal = Params::CIN;
      break;
    case (157):
      retVal = Params::CAPE;
      break;
    case (158):
      retVal = Params::TURBULENT_KINETIC_ENERGY;
      break;
    case (170):
      retVal = Params::RAIN_WATER_MIXING_RATIO;
      break;
    case (171):
      retVal = Params::SNOW_MIXING_RATIO;
      break;
    case (172):
      retVal = Params::MOMENTUM_FLUX;
      break;
    case (178):
      retVal = Params::ICE_MIXING_RATIO;
      break;
    case (179):
      retVal = Params::GRAUPEL_MIXING_RATIO;
      break;
    //case (185):
    //  retVal = Params::WATER_VAPOR_MIXING_RATIO;
    //  break;
    case (185):
      retVal = Params::TURB_SIGMET_AIRMET;
      break;
    case (186):
      retVal = Params::ICING_SIGMET_AIRMET;
      break;
    case (187):
      retVal = Params::LIGHTNING;
      break;
    case (188):
      retVal = Params::RATE_WATER_CANOPY2GROUND;
      break;
    case (189):
      retVal = Params::VIRTUAL_POTENTIAL_TEMP;
      break;
    case (190):
      retVal = Params::STORM_RELATIVE_HELICITY;
      break;
    case (196):
      retVal = Params::U_STORM_MOTION;
      break;
    case (197):
      retVal = Params::V_STORM_MOTION;
      break;
    case (198):
      retVal = Params::NUM_CONCEN_ICE_PARTICLES;
      break;
    case (199):
      retVal = Params::DIRECT_EVAPORATION_BARE_SOIL;
      break;
    case (204):
      retVal = Params::DOWN_SHORTWAVE_RAD_FLUX;
      break;
    case (205):
      retVal = Params::DOWN_LONGWAVE_RAD_FLUX;
      break;
    case (211):
      retVal = Params::UP_SHORTWAVE_RAD_FLUX;
      break;
    case (212):
      retVal = Params::UP_LONGWAVE_RAD_FLUX;
      break;
    case (214):
      retVal = Params::CONV_PRECP_RATE;
      break;
    case (221):
      retVal = Params::HPBL;
      break;
    case (222):
      retVal = Params::FIVE_WAVH;
      break;
    case (223):
      retVal = Params::PLANT_CANOPY_SURFACE_WATER;
      break;
    case (224):
      retVal = Params::SOIL_TYPE;
      break;
    case (225):
      retVal = Params::VEGETATION_TYPE;
      break;
    case (229):
      retVal = Params::SNOW_PHASE_CHANGE_HEAT_FLUX;
      break;
    case (230):
      retVal = Params::FIVE_WAVA;
      break;
    case (234):
      retVal = Params::BASEFLOW_GRDWATER_RUNOFF;
      break;
    case (235):
      retVal = Params::STORM_SURFACE_RUNOFF;
      break;
    case (239):
      retVal = Params::SNOW_TEMPERATURE;
      break;
    case (253):
      retVal = Params::DRAG_COEFFICIENT;
      break;
    case (255):
      retVal = Params::GUST_WIND_SPEED;
      break;

    default:
      retVal = Params::UNKNOWN;
  }
  return retVal;

}

//Params::level_id_t
//GribFile::cnvtLevelId2enum(const int levelType) {

//  Params::level_id_t retVal;

//  switch (levelType) {
//    case (1): 
//      retVal = Params::GROUND_OR_WATER_SURFACE;
//      break;
//    case (2):
//      retVal = Params::CLOUD_BASE_LEVEL;
//      break;
//    case (3):
//      retVal = Params::CLOUD_TOP_LEVEL;
//      break;
//    case (4): 
//      retVal = Params::LEVEL_OF_0_DEGREE_ISOTHERM;
//      break;
//    case (5):
//      retVal = Params::ADIABATIC_CONDENSATION;
//      break;
//    case (6):
//      retVal = Params::MAXIMUM_WIND_LEVEL;
//      break;
//    case (7):
//      retVal = Params::TROPOPAUSE;
//      break;
//    case (8):
//      retVal = Params::NOMINAL_ATMOS_TOP;
//      break;
//    case (9):
//      retVal = Params::SEA_BOTTOM;
//      break;
//    case (100): 
//      retVal = Params::ISOBARIC;
//      break;
//    case (101): 
//      retVal = Params::BETWEEN_ISOB_LVLS;
//      break;
//    case (102):
//      retVal = Params::MEAN_SEA_LEVEL;
//      break;
//    case (103):
//      retVal = Params::ALTITUDE_ABOVE_MSL;
//      break;
//    case (104):
//      retVal = Params::ALTITUDE_ABOVE_MSL_V_COMP;
//      break;
//    case (105):
//      retVal = Params::SPECF_HEIGHT_ABOVE_GROUND;
//      break;
//    case (107):
//      retVal = Params::PERCENT_SIGMA_LEVELS;
//      break;
//    case (108):
//      retVal = Params::LAYER_SIGMA_LEVELS;
//      break;
//    case (109 ):
//      retVal = Params::HYBRID;
//      break;
//    case (110 ):
//      retVal = Params::LAYER_BETWEEN_HYBRID_LEVELS;
//      break;
//    case (111):
//      retVal = Params::DEPTH_BELOW_SURFACE;
//      break;
//    case (112):
//      retVal = Params::LAYER_BETWEEN_TWO_DEPTHS_BELOW_SURFACE;
//      break;
//    case (113):
//      retVal = Params::ISENTROPIC;
//      break;
//    case (114):
//      retVal = Params::LAYER_BETWEEN_ISENTROPIC;
//      break;
//    case (115):
//      retVal = Params::SPECIFIED_PRESS_DIFF;
//      break;
//    case (116):
//      retVal = Params::LAYER_BETWEEN_TWO_LEVELS_AT_SPECIFIED_PRESSURES;
//      break;
//    case (117):
//      retVal = Params::POTENTIAL_VORTICITY_SURF;
//      break;
//    case (200):
//      retVal = Params::ENTIRE_ATMOSPHERE;
//      break;
//    case (204):
//      retVal = Params::HIGHEST_TROPO_FREEZE_LEVEL;
//      break;
//    case (211):
//      retVal = Params::TOTAL_CLOUD_COVER;
//      break;
//    case (212):
//      retVal = Params::LOW_CLOUDS_BOTTOM_LEVEL;
//    break;
//  case (213):
//    retVal = Params::LOW_CLOUDS_TOP_LEVEL;
//    break;
//  case (214):
//    retVal = Params::LOW_CLOUD_LAYER;
//    break;
//  case (222):
//    retVal = Params::MID_CLOUD_BOTTOM_LEVEL;
//    break;
//  case (223):
//    retVal = Params::MID_CLOUD_TOP_LEVEL;
//    break;
//  case (224):
//    retVal = Params::MID_CLOUD_LAYER;
//    break;
//  case (232):
//    retVal = Params::HIGH_CLOUD_BOT_LEVEL;
//    break;
//  case (233):
//    retVal = Params::HIGH_CLOUD_TOP_LEVEL;
//    break;
//  case (234):
//    retVal = Params::HIGH_CLOUD_LAYER;
//    break;
//  case (243):
//    retVal = Params::CONVECTIVE_CLOUD_TOP_LEVEL;
//    break;
//  case (244):
//      retVal = Params::CONVECTIVE_TOTAL_CLOUD_COVER;
//      break;
//    case (246):
//      retVal = Params::MAXIMUM_EPOT_LEVEL;
//      break;
//    case (247):
//      retVal = Params::EQUILIBRIUM_LEVEL;
//      break;

//    default:
//      retVal = Params::NOT_KNOWN;
//  }
//  return retVal;

// }

//
// Used when the parameter process_everything is set to
// TRUE - gets the list from the Grib file
//

list <Params::out_field_t> 
  GribFile::getFieldList() {

  list <Params::out_field_t> field_list;

  Params::out_field_t new_field;
  Params::out_field_t last_field;
  MEM_zero(new_field);
  MEM_zero(last_field);
  
  _inv = _inventory.begin();
  while (_inv != _inventory.end()) {
    new_field.param_id  =  cnvtParamId2enum((*_inv)->getParameterId());
    new_field.level_id  =  (*_inv)->getVerticalLevelType();

    //Filter out z_level field list
    if (new_field.param_id != last_field.param_id || new_field.level_id != last_field.level_id) {
      //cout << "              Accepting " << new_field.param_id << "  " << new_field.level_id << " " << new_field.out_units << endl;
      field_list.push_back(new_field);
    }
    last_field = new_field;
    _inv++;
  }
    
  return (field_list);

}

/*****************************************************/

int
GribFile::getNumZlevels(const int productId, const int levelType) {

  int numZlevels = 0;

  _inv = _inventory.begin();
  while (_inv != _inventory.end()) {
    // find the first parameter ID in the inventory
    if ((*_inv)->getParameterId() == productId && (*_inv)->getVerticalLevelType() == levelType) {
      numZlevels = 1;
      _inv++;
      break;
    }
    _inv++;
  }

  while (_inv != _inventory.end()) {
    // find how many vertical levels for this field
    if ((*_inv)->getParameterId() != productId || (*_inv)->getVerticalLevelType() != levelType) {
      return (numZlevels);;
    }
    else
        numZlevels ++;
    _inv++;

  }

  if (numZlevels == 0) {
    cout << "WARNING productID " << productId << " with level type ";
    cout << levelType << " NOT FOUND " << endl;
    return RI_FAILURE; 
  }
  return (numZlevels);
}

/*****************************************************/
bool
compareFunc(GFSrecord *lhs, GFSrecord *rhs) {
   // do we have a volume of data?
   if ((*lhs).getParameterId() == (*rhs).getParameterId() &&
                  (*lhs).getVerticalLevelType() == (*rhs).getVerticalLevelType()) {
      // sort by vertical level
      return ((*rhs).getLevelVal() < (*lhs).getLevelVal() ? true : false);
   }
   if ((*lhs).getParameterId() == (*rhs).getParameterId()) {
      // sort by vertical level type
      return ((*lhs).getVerticalLevelType() < (*rhs).getVerticalLevelType() ? true : false);
   }
   // sort just by Id number
   return ((*lhs).getParameterId() < (*rhs).getParameterId() ? true : false);
}

