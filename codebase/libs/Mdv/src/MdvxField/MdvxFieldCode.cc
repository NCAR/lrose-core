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
//////////////////////////////////////////////////////////
// MdvxFieldCode.cc
//
// Class for providing handling Mdvx field codes
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxFieldCode.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxFieldCode.hh>
#include <string.h>
using namespace std;

const MdvxFieldCode::entry_t MdvxFieldCode::_entries[MDVX_N_FIELD_CODES] = {
  { 0, "Reserved", "UNDEFINED", "UNDEFINED"},
  { 1, "Pressure", "Pa", "PRES"},
  { 2, "Pressure reduced to MSL", "Pa", "PRMSL"},
  { 3, "Pressure tendency", "Pa/s", "PTEND"},
  { 4, "Potential vorticity", "Km2kg/s", "PVORT"},
  { 5, "ICAO Standard Atmosphere Reference Height", "m", "ICAHT"},
  { 6, "Geopotential", "m2/s2", "GP"},
  { 7, "Geopotential height", "gpm", "HGT"},
  { 8, "Geometric height", "m" ,"DIST"},
  { 9, "Standard deviation of height", "m", "HSTDV"},
  { 10, "Total ozone", "Dobson", "TOZONE"},
  { 11, "Temperature", "K", "TMP"},
  { 12, "Virtual temperature", "K", "VTMP"},
  { 13, "Potential temperature", "K", "POT"},
  { 14, "Pseudo-adiabatic (equivalent) potential temp", "K", "EPOT"},
  { 15, "Maximum temperature", "K", "T MAX"},
  { 16, "Minimum temperature", "K", "T MIN"},
  { 17, "Dew point temperature", "K", "DPT"},
  { 18, "Dew point depression /or deficit/", "K", "DEPR"},
  { 19, "Lapse rate", "K/m", "LAPR"},
  { 20, "Visibility", "m", "VIS"},
  { 21, "Radar Spectra /1/", "UNDEFINED", "RDSP1"},
  { 22, "Radar Spectra /2/", "UNDEFINED", "RDSP2"},
  { 23, "Radar Spectra /3/", "UNDEFINED", "RDSP3"},
  { 24, "Parcel lifted index /to 500 hPa", "K", "PLI"},
  { 25, "Temperature anomaly", "K", "TMP A"},
  { 26, "Pressure anomaly", "Pa", "PRESA"},
  { 27, "Geopotential height anomaly", "gpm", "GP A"},
  { 28, "Wave Spectra /1/", "UNDEFINED", "WVSP1"},
  { 29, "Wave Spectra /2/", "UNDEFINED", "WVSP2"},
  { 30, "Wave Spectra /3/", "UNDEFINED", "WVSP3"},
  { 31, "Wind direction /from which blowing/", "deg true", "WDIR"},
  { 32, "Wind speed", "m/s", "WIND"},
  { 33, "u-component of wind", "m/s", "U GRD"},
  { 34, "v-component of wind", "m/s", "V GRD"},
  { 35, "Stream function", "m2/s", "STRM"},
  { 36, "Velocity potential", "m2/s", "V POT"},
  { 37, "Montgomery stream function", "m2/s2", "MNTSF"},
  { 38, "Sigma coord. vertical velocity", "/s", "SGCVV"},
  { 39, "Pressure Vertical velocity", "Pa/s", "V VEL"},
  { 40, "Geometric Vertical velocity", "m/s", "DZDT"},
  { 41, "Absolute vorticity", "/s", "ABS V"},
  { 42, "Absolute divergence", "/s", "ABS D"},
  { 43, "Relative vorticity", "/s", "REL V"},
  { 44, "Relative divergence", "/s", "REL D"},
  { 45, "Vertical u-component shear", "/s", "VUCSH"},
  { 46, "Vertical v-component shear", "/s", "VVCSH"},
  { 47, "Direction of current", "deg true", "DIR C"},
  { 48, "Speed of current", "m/s", "SP C"},
  { 49, "u-component of current", "m/s", "UOGRD"},
  { 50, "v-component of current", "m/s", "VOGRD"},
  { 51, "Specific humidity", "kg/kg", "SPF H"},
  { 52, "Relative humidity", "%", "R H"},
  { 53, "Humidity mixing ratio", "kg/kg", "MIXR"},
  { 54, "Precipitable water", "kg/m2", "P WAT"},
  { 55, "Vapor pressure", "Pa", "VAPP"},
  { 56, "Saturation deficit", "Pa", "SAT D"},
  { 57, "Evaporation", "kg/m2", "EVP"},
  { 58, "Cloud Ice", "kg/m2", "C ICE"},
  { 59, "Precipitation rate", "kg/m2/s", "PRATE"},
  { 60, "Thunderstorm probability", "%", "TSTM"},
  { 61, "Total precipitation", "kg/m2", "A PCP"},
  { 62, "Large scale precipitation /non-conv./", "kg/m2", "NCPCP"},
  { 63, "Convective precipitation", "kg/m2", "ACPCP"},
  { 64, "Snowfall rate water equivalent", "kg/m2/s", "SRWEQ"},
  { 65, "Water equiv. of accum. snow depth", "kg/m2", "WEASD"},
  { 66, "Snow depth", "m", "SNO D"},
  { 67, "Mixed layer depth", "m", "MIXHT"},
  { 68, "Transient thermocline depth", "m", "TTHDP"},
  { 69, "Main thermocline depth", "m", "MTHD"},
  { 70, "Main thermocline anomaly", "m", "MTH A"},
  { 71, "Total cloud cover", "%", "T CDC"},
  { 72, "Convective cloud cover", "%", "CDCON"},
  { 73, "Low cloud cover", "%", "L CDC"},
  { 74, "Medium cloud cover", "%", "M CDC"},
  { 75, "High cloud cover", "%", "H CDC"},
  { 76, "Cloud water", "kg/m2", "C WAT"},
  { 77, "Best lited index /to 500 hPa/", "K", "BLI"},
  { 78, "Convective snow", "kg/m2", "SNO C"},
  { 79, "Large scale snow", "kg/m2", "SNO L"},
  { 80, "Water Temperature", "K", "WTMP"},
  { 81, "Land-sea mask /land=1;sea=0/", "fraction", "LAND"},
  { 82, "Deviation of sea level from mean", "m", "DSL M"},
  { 83, "Surface roughness", "m", "SFC R"},
  { 84, "Albedo", "%", "ALBDO"},
  { 85, "Soil temperature", "K", "TSOIL"},
  { 86, "Soil moisture content", "kg/m2", "SOIL M"},
  { 87, "Vegetation", "%", "VEG"},
  { 88, "Salinity", "kg/kg", "SALTY"},
  { 89, "Density", "kg/m3", "DEN"},
  { 90, "Water runoff", "kg/m2", "WATR"},
  { 91, "Ice concentration /ice=1;no ice=0/", "fraction", "ICE C"},
  { 92, "Ice thickness", "m", "ICETK"},
  { 93, "Direction of ice drift", "deg. true", "DICED"},
  { 94, "Speed of ice drift", "m/s", "SICED"},
  { 95, "u-component of ice drift", "m/s", "U ICE"},
  { 96, "v-component of ice drift", "m/s", "V ICE"},
  { 97, "Ice growth rate", "m/s", "ICE G"},
  { 98, "Ice divergence", "/s", "ICE D"},
  { 99, "Snow melt", "kg/m2", "SNO M"},
  {100, "Significant ht of combined wind waves and swell", "m", "HTSGW"},
  {101, "Direction of wind waves /from which/", "deg true", "WVDIR"},
  {102, "Significant height of wind waves", "m", "WVHGT"},
  {103, "Mean period of wind waves", "s", "WVPER"},
  {104, "Direction of swell waves", "deg true", "SWDIR"},
  {105, "Significant height of swell waves", "m", "SWELL"},
  {106, "Mean period of swell waves", "s", "SWPER"},
  {107, "Primary wave direction", "deg true", "DIRPW"},
  {108, "Primary wave mean period", "s", "PERPW"},
  {109, "Secondary wave direction", "deg true", "DIRSW"},
  {110, "Secondary wave mean period", "s", "PERSW"},
  {111, "Net short-wave radiation /surface/", "W/m2", "NSWRS"},
  {112, "Net long wave radiation /surface/", "W/m2", "NLWRS"},
  {113, "Net short-wave radiation /top of atmos./", "W/m2", "NSWRT"},
  {114, "Net long wave radiation /top of atmos./", "W/m2", "NLWRT"},
  {115, "Long wave radiation flux", "W/m2", "LWAVR"},
  {116, "Short wave radiation flux", "W/m2", "SWAVR"},
  {117, "Global radiation flux", "W/m2", "G RAD"},
  {118, "Brightness temperature", "K", "BRTMP"},
  {119, "Radiance /with respect to wave number/", "W/m/sr", "LWRAD"},
  {120, "Radiance /with respect to wave length/", "W/m3/sr", "SWRAD"},
  {121, "Latent heat net flux", "W/m2", "LHTFL"},
  {122, "Sensible heat net flux", "W/m2", "SHTFL"},
  {123, "Boundary layer dissipation", "W/m2", "BLYDP"},
  {124, "Momentum flux, u component", "N/m2", "U FLX"},
  {125, "Momentum flux, v component", "N/m2", "V FLX"},
  {126, "Wind mixing energy", "J", "WMIXE"},
  {127, "Image data", "UNDEFINED", "IMG D"},
  {128, "Mean Sea Level Pres /Std Atmos Reduction/", "Pa", "MSLSA"},
  {129, "Mean Sea Level Pres /MAPS System Reduction/", "Pa", "MSLMA"},
  {130, "Mean Sea Level Pres /ETA Model Reduction/", "Pa", "MSLET"},
  {131, "Surface lifted index", "K", "LFT X"},
  {132, "Best /4 layer/ lifted index", "K", "4LFTX"},
  {133, "K index", "K", "K X"},
  {134, "Sweat index", "K", "S X"},
  {135, "Horizontal moisture divergence", "kg/kg/s", "MCONV"},
  {136, "Vertical speed shear", "/s", "VW SH"},
  {137, "3-hr pressure tendency /Std Atmos Reduction/", "Pa/s", "TSLSA"},
  {138, "Brunt-Vaisala frequency /squared/", "/s2", "BVF 2"},
  {139, "Potential vorticity /density weighted/", "/s/m", "PV MW"},
  {140, "Categorical rain /yes=1; no=0/", "non-dim", "CRAIN"},
  {141, "Categorical freezing rain /yes=1; no=0/", "non-dim", "CFRZR"},
  {142, "Categorical ice pellets /yes=1; no=0/", "non-dim", "CICEP"},
  {143, "Categorical snow /yes=1; no=0/", "non-dim", "CSNOW"},
  {144, "Volumetric soil moisture content", "fraction", "SOILW"},
  {145, "Potential evaporation rate", "W/m2", "RNWSH"},
  {146, "Cloud workfunction", "J/kg", "CWORK"},
  {147, "Zonal flux of gravity wave stress", "N/m2", "U-GWD"},
  {148, "Meridional flux of gravity wave stress", "N/m2", "V-GWD"},
  {149, "Potential vorticity", "m2/s/kg", "PV"},
  {150, "Wind Covariance [uv]-[u][v] ([]=mean/span)", "m2/s2", "COVMZ"},
  {151, "Uwind-Temp Covariance [uT]-[u][T]", "K*m/s", "COVTZ"},
  {152, "Vwind-Temp Covariance [vT]-[v][T]", "K*m/s", "COVTM"},
  {153, "Cloud water", "Kg/kg", "CLWMR"},
  {154, "Ozone mixing ratio", "Kg/kg", "O3MR"},
  {155, "Ground Heat Flux", "W/m2", "GFLUX"},
  {156, "Convective inhibition", "J/kg", "CIN"},
  {157, "Convective Available Potential Energy", "J/kg", "CAPE"},
  {158, "Turbulent Kinetic Energy", "J/kg", "TKE"},
  {159, "Condensation pressure of parcel", "Pa", "CONDP"},
  {160, "Clear Sky Upward Solar Flux", "W/m2", "CSUSF"},
  {161, "Clear Sky Downward Solar Flux", "W/m2", "CSDSF"},
  {162, "Clear Sky upward long wave flux", "W/m2", "CSULF"},
  {163, "Clear Sky downward long wave flux", "W/m2", "CSDLF"},
  {164, "Cloud forcing net solar flux", "W/m2", "CFNSF"},
  {165, "Cloud forcing net long wave flux", "W/m2", "CFNLF"},
  {166, "Visible beam downward solar flux", "W/m2", "VBDSF"},
  {167, "Visible diffuse downward solar flux", "W/m2", "VDDSF"},
  {168, "Near IR beam downward solar flux", "W/m2", "NBDSF"},
  {169, "Near IR diffuse downward solar flux", "W/m2", "NDDSF"},
  {170, "Rain water mixing ratio", "Kg/Kg", "RWMR"},
  {171, "Snow mixing ratio", "Kg/Kg", "SNMR"},
  {172, "Momentum flux", "N/m2", "M FLX"},
  {173, "Mass point model surface", "non-dim", "LMH"},
  {174, "Velocity point model surface", "non-dim", "LMV"},
  {175, "Model layer number /from bottom up/", "non-dim", "MLYNO"},
  {176, "latitude /-90 to +90/", "deg", "NLAT"},
  {177, "east longitude /0-360/", "deg", "ELON"},
  {178, "Ice mixing ratio", "Kg/Kg", "ICMR"},
  {179, "Graupel mixing ratio", "Kg/Kg", "GRMR"},
  {180, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {181, "x-gradient of log pressure", "/m", "LPS X"},
  {182, "y-gradient of log pressure", "/m", "LPS Y"},
  {183, "x-gradient of height", "m/m", "HGT X"},
  {184, "y-gradient of height", "m/m", "HGT Y"},
  {185, "Turbulence SIGMET/AIRMET", "non-dim", "TURB"},
  {186, "Icing SIGMET/AIRMET", "non-dim", "ICNG"},
  {187, "Lightning", "non-dim", "LTNG"},
  {188, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {189, "Virtual potential temperature", "K", "VPTMP"},
  {190, "Storm relative helicity", "m2/s2", "HLCY"},
  {191, "Probability from ensemble", "numeric", "PROB"},
  {192, "Probability from ensemble normalized", "numeric", "PROBN"},
  {193, "Probability of precipitation", "%", "POP"},
  {194, "Probability of frozen precipitation", "%", "CPOFP"},
  {195, "Probability of freezing precipitation", "%", "CPOSP"},
  {196, "u-component of storm motion", "m/s", "USTM"},
  {197, "v-component of storm motion", "m/s", "VSTM"},
  {198, "Number concentration for ice particles", "UNDEFINED", "NCIP"},
  {199, "Direct evaporation from bare soil", "W/m2", "EVBS"},
  {200, "Canopy water evaporation", "W/m2", "EVCW"},
  {201, "Ice-free water surface", "%", "ICWAT"},
  {202, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {203, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {204, "downward short wave rad. flux", "W/m2", "DSWRF"},
  {205, "downward long wave rad. flux", "W/m2", "DLWRF"},
  {206, "Ultra violet index /1hr int. cent. solar noon/", "J/m2", "UVI"},
  {207, "Moisture availability", "%", "MSTAV"},
  {208, "Exchange coefficient", "(kg/m3)(m/s)", "SFEXC"},
  {209, "No. of mixed layers next to surface", "integer", "MIXLY"},
  {210, "Transpiration", "W/m2", "TRANS"},
  {211, "upward short wave rad. flux", "W/m2", "USWRF"},
  {212, "upward long wave rad. flux", "W/m2", "ULWRF"},
  {213, "Amount of non-convective cloud", "%", "CDLYR"},
  {214, "Convective Precipitation rate", "kg/m2/s", "CPRAT"},
  {215, "Temperature tendency by all physics", "K/s", "TTDIA"},
  {216, "Temperature tendency by all radiation", "K/s", "TTRAD"},
  {217, "Temperature tendency by non-radiation physics", "K/s", "TTPHY"},
  {218, "precip.index /0.0-1.00/ /see note/", "fraction", "PREIX"},
  {219, "Std. dev. of IR T over 1x1 deg area", "K", "TSD1D"},
  {220, "Natural log of surface pressure", "ln/kPa/", "NLGSP"},
  {221, "Planetary boundary layer height", "m", "HPBL"},
  {222, "5-wave geopotential height", "gpm", "5WAVH"},
  {223, "Plant canopy surface water", "kg/m2", "CNWAT"},
  {224, "Soil type /as in Zobler/", "Integer /0-9/", "SOTYP"},
  {225, "Vegitation type /as in SiB", "Integer /0-13/", "VGTYP"},
  {226, "Blackadar's mixing length scale", "m", "BMIXL"},
  {227, "Asymptotic mixing length scale", "m", "AMIXL"},
  {228, "Potential evaporation", "kg/m2", "PEVAP"},
  {229, "Snow phase-change heat flux", "W/m2", "SNOHF"},
  {230, "5-wave geopotential height anomaly", "gpm", "5WAVA"},
  {231, "Convective cloud mass flux", "Pa/s", "MFLUX"},
  {232, "Downward total radiation flux", "W/m2", "DTRF"},
  {233, "Upward total radiation flux", "W/m2", "UTRF"},
  {234, "Baseflow-groundwater runoff", "kg/m2", "BGRUN"},
  {235, "Storm surface runoff", "kg/m2", "SSRUN"},
  {236, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {237, "Total Ozone", "Kg/m2", "O3TOT"},
  {238, "Snow cover", "percent", "SNOWC"},
  {239, "Snow temperature", "K", "SNO T"},
  {240, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {241, "Large scale condensat. heat rate", "K/s", "LRGHR"},
  {242, "Deep convective heating rate", "K/s", "CNVHR"},
  {243, "Deep convective moistening rate", "kg/kg/s", "CNVMR"},
  {244, "Shallow convective heating rate", "K/s", "SHAHR"},
  {245, "Shallow convective moistening rate", "kg/kg/s", "SHAMR"},
  {246, "Vertical diffusion heating rate", "K/s", "VDFHR"},
  {247, "Vertical diffusion zonal acceleration", "m/s2", "VDFUA"},
  {248, "Vertical diffusion meridional accel", "m/s2", "VDFVA"},
  {249, "Vertical diffusion moistening rate", "kg/kg/s", "VDFMR"},
  {250, "Solar radiative heating rate", "K/s", "SWHR"},
  {251, "long wave radiative heating rate", "K/s", "LWHR"},
  {252, "Drag coefficient", "non-dim", "CD"},
  {253, "Friction velocity", "m/s", "FRICV"},
  {254, "Richardson number", "non-dim", "RI"},
  {255, "Missing", "UNDEFINED", "UNDEFINED"},
  {256, "Radar Reflectivity", "dBZ", "DBZ"},
  {257, "Radar Composite Reflectivity", "dBZ", "C DBZ"},
  {258, "Radar Layer Composite Reflectivity", "dBZ", "LC DBZ"},
  {259, "Radar Radial Velocity", "m/s", "R VEL"},
  {260, "Radar Spectral Width", "Hz", "S WIDTH"},
  {261, "Radar Signal/Noise", "dB", "S/N"},
  {262, "Radar Echo Tops", "m", "E TOPS"},
  {263, "Vertical Integrated Liquid", "kg/m^2", "VIL"},
  {264, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {265, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {266, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {267, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {268, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {269, "UNDEFINED", "UNDEFINED", "UNDEFINED"},
  {270, "Storm Precip Accumulation", "mm", "ST ACCUM"},
  {271, "1Hr Precip Accumulation", "mm", "1H ACCUM"},
  {272, "3Hr Precip Accumulation", "mm", "3H ACCUM"},
};

////////////////////////////////////////////////////////////////////////
// Basic constructor - only a master header reference
//

MdvxFieldCode::MdvxFieldCode()

{
  
}

/////////////////////////////
// Destructor

MdvxFieldCode::~MdvxFieldCode()

{

}

//////////////////////////
// lookup entry from code
//
// Sets entry.
// Returns 0 on success, -1 on failure.

int MdvxFieldCode::getEntryByCode(int code,
				  MdvxFieldCode::entry_t &entry)
  
{
  if (code < 0 || code > MDVX_MAX_FIELD_CODE) {
    return -1;
  }
  entry = _entries[code];
  return 0;
}


//////////////////////////
// lookup entry from name
//
// Sets entry.
// Returns 0 on success, -1 on failure.

int MdvxFieldCode::getEntryByName(const char *name,
				  MdvxFieldCode::entry_t &entry)
  
{
  for (int i = 0; i < MDVX_N_FIELD_CODES; i++) {
    if (!strcmp(name, _entries[i].name)) {
      entry = _entries[i];
      return 0;
    }
  }
  return -1;
}

//////////////////////////
// lookup entry from abbrev
//
// Sets entry.
// Returns 0 on success, -1 on failure.

int MdvxFieldCode::getEntryByAbbrev(const char *abbrev,
				    MdvxFieldCode::entry_t &entry)
  
{
  for (int i = 0; i < MDVX_N_FIELD_CODES; i++) {
    if (!strcmp(abbrev, _entries[i].abbrev)) {
      entry = _entries[i];
      return 0;
    }
  }
  return -1;
}



