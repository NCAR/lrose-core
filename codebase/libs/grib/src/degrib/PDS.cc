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
//////////////////////////////////////////////////////
// PDS - Product Definition Section
//
// Used wgrib by Wesley Ebisuzaki at NOAA as
// reference (http://wesley.wwb.noaa.gov/wgrib.html) 
//
//////////////////////////////////////////////////////

#include <iostream>

#include <grib/constants.h>
#include <grib/GribVertType.hh>
#include <grib/PDS.hh>
#include <toolsa/utim.h>

using namespace std;

/*
 * parameter table for NCEP (operations)
 * center = 7, subcenter != 2 parameter table = 1, 2, 3 etc
 * note: see reanalysis parameter table for problems
 * updated 5/11/99
 */
const PDS::_ParmTable PDS::_parmTable[256] = {
          /* 0 */ {"var0", "undefined", "none"},
	  /* 1 */ {"PRES", "Pressure", "Pa"},
	  /* 2 */ {"PRMSL", "Pressure reduced to MSL", "Pa"},
	  /* 3 */ {"PTEND", "Pressure tendency", "Pa/s"},
	  /* 4 */ {"PVORT", "Pot. vorticity", "km^2/kg/s"},
	  /* 5 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height", "M"},
	  /* 6 */ {"GP", "Geopotential", "m^2/s^2"},
	  /* 7 */ {"HGT", "Geopotential height", "gpm"},
	  /* 8 */ {"DIST", "Geometric height", "m"},
	  /* 9 */ {"HSTDV", "Std dev of height", "m"},
	  /* 10 */ {"TOZNE", "Total ozone", "Dobson"},
	  /* 11 */ {"TMP", "Temperature", "K"},
	  /* 12 */ {"VTMP", "Virtual temperature", "K"},
	  /* 13 */ {"POT", "Potential temperature", "K"},
	  /* 14 */ {"EPOT", "Pseudo-adiabatic pot. temperature", "K"},
	  /* 15 */ {"TMAX", "Max. temperature", "K"},
	  /* 16 */ {"TMIN", "Min. temperature", "K"},
	  /* 17 */ {"DPT", "Dew point temperature", "K"},
	  /* 18 */ {"DEPR", "Dew point depression", "K"},
	  /* 19 */ {"LAPR", "Lapse rate", "K/m"},
	  /* 20 */ {"VIS", "Visibility", "m"},
	  /* 21 */ {"RDSP1", "Radar spectra (1)", "non-dim"},
	  /* 22 */ {"RDSP2", "Radar spectra (2)", "non-dim"},
	  /* 23 */ {"RDSP3", "Radar spectra (3)", "non-dim"},
	  /* 24 */ {"PLI", "Parcel lifted index (to 500 hPa)", "K"},
	  /* 25 */ {"TMPA", "Temperature anomaly", "K"},
	  /* 26 */ {"PRESA", "Pressure anomaly", "Pa"},
	  /* 27 */ {"GPA", "Geopotential height anomaly", "gpm"},
	  /* 28 */ {"WVSP1", "Wave spectra (1)", "non-dim"},
	  /* 29 */ {"WVSP2", "Wave spectra (2)", "non-dim"},
	  /* 30 */ {"WVSP3", "Wave spectra (3)", "non-dim"},
	  /* 31 */ {"WDIR", "Wind direction", "deg"},
	  /* 32 */ {"WIND", "Wind speed", "m/s"},
	  /* 33 */ {"UGRD", "u wind", "m/s"},
	  /* 34 */ {"VGRD", "v wind", "m/s"},
	  /* 35 */ {"STRM", "Stream function", "m^2/s"},
	  /* 36 */ {"VPOT", "Velocity potential", "m^2/s"},
	  /* 37 */ {"MNTSF", "Montgomery stream function", "m^2/s^2"},
	  /* 38 */ {"SGCVV", "Sigma coord. vertical velocity", "/s"},
	  /* 39 */ {"VVEL", "Pressure vertical velocity", "Pa/s"},
	  /* 40 */ {"DZDT", "Geometric vertical velocity", "m/s"},
	  /* 41 */ {"ABSV", "Absolute vorticity", "/s"},
	  /* 42 */ {"ABSD", "Absolute divergence", ""},
	  /* 43 */ {"RELV", "Relative vorticity", "/s"},
	  /* 44 */ {"RELD", "Relative divergence", "/s"},
	  /* 45 */ {"VUCSH", "Vertical u shear", "/s"},
	  /* 46 */ {"VVCSH", "Vertical v shear", "/s"},
	  /* 47 */ {"DIRC", "Direction of current", "deg"},
	  /* 48 */ {"SPC", "Speed of current", "m/s"},
	  /* 49 */ {"UOGRD", "u of current", "m/s"},
	  /* 50 */ {"VOGRD", "v of current", "m/s"},
	  /* 51 */ {"SPFH", "Specific humidity", "kg/kg"},
	  /* 52 */ {"RH", "Relative humidity", "%"},
	  /* 53 */ {"MIXR", "Humidity mixing ratio", "kg/kg"},
	  /* 54 */ {"PWAT", "Precipitable water", "kg/m^2"},
	  /* 55 */ {"VAPP", "Vapor pressure", "Pa"},
	  /* 56 */ {"SATD", "Saturation deficit", "Pa"},
	  /* 57 */ {"EVP", "Evaporation", "kg/m^2"},
	  /* 58 */ {"CICE", "Cloud Ice", "kg/m^2"},
	  /* 59 */ {"PRATE", "Precipitation rate", "kg/m^2/s"},
	  /* 60 */ {"TSTM", "Thunderstorm probability", "%"},
	  /* 61 */ {"APCP", "Total precipitation", "kg/m^2"},
	  /* 62 */ {"NCPCP", "Large scale precipitation", "kg/m^2"},
	  /* 63 */ {"ACPCP", "Convective precipitation", "kg/m^2"},
	  /* 64 */ {"SRWEQ", "Snowfall rate water equiv.", "kg/m^2/s"},
	  /* 65 */ {"WEASD", "Accumulated snow", "kg/m^2"},
	  /* 66 */ {"SNOD", "Snow depth", "m"},
	  /* 67 */ {"MIXHT", "Mixed layer depth", "m"},
	  /* 68 */ {"TTHDP", "Transient thermocline depth", "m"},
	  /* 69 */ {"MTHD", "Main thermocline depth", "m"},
	  /* 70 */ {"MTHA", "Main thermocline anomaly", "m"},
	  /* 71 */ {"TCDC", "Total cloud cover", "%"},
	  /* 72 */ {"CDCON", "Convective cloud cover", "%"},
	  /* 73 */ {"LCDC", "Low level cloud cover", "%"},
	  /* 74 */ {"MCDC", "Mid level cloud cover", "%"},
	  /* 75 */ {"HCDC", "High level cloud cover", "%"},
	  /* 76 */ {"CWAT", "Cloud water", "kg/m^2"},
	  /* 77 */ {"BLI", "Best lifted index (to 500 hPa)", "K"},
	  /* 78 */ {"SNOC", "Convective snow", "kg/m^2"},
	  /* 79 */ {"SNOL", "Large scale snow", "kg/m^2"},
	  /* 80 */ {"WTMP", "Water temperature", "K"},
	  /* 81 */ {"LAND", "Land cover (land=1;sea=0)", "fraction"},
	  /* 82 */ {"DSLM", "Deviation of sea level from mean", "m"},
	  /* 83 */ {"SFCR", "Surface roughness", "m"},
	  /* 84 */ {"ALBDO", "Albedo", "%"},
	  /* 85 */ {"TSOIL", "Soil temperature", "K"},
	  /* 86 */ {"SOILM", "Soil moisture content", "kg/m^2"},
	  /* 87 */ {"VEG", "Vegetation", "%"},
	  /* 88 */ {"SALTY", "Salinity", "kg/kg"},
	  /* 89 */ {"DEN", "Density", "kg/m^3"},
	  /* 90 */ {"WATR", "Water runoff", "kg/m^2"},
	  /* 91 */ {"ICEC", "Ice concentration (ice=1;no ice=0)", "fraction"},
	  /* 92 */ {"ICETK", "Ice thickness", "m"},
	  /* 93 */ {"DICED", "Direction of ice drift", "deg"},
	  /* 94 */ {"SICED", "Speed of ice drift", "m/s"},
	  /* 95 */ {"UICE", "u of ice drift", "m/s"},
	  /* 96 */ {"VICE", "v of ice drift", "m/s"},
	  /* 97 */ {"ICEG", "Ice growth rate", "m/s"},
	  /* 98 */ {"ICED", "Ice divergence", "/s"},
	  /* 99 */ {"SNOM", "Snow melt", "kg/m^2"},
	  /* 100 */ {"HTSGW", "Sig height of wind waves and swell", "m"},
	  /* 101 */ {"WVDIR", "Direction of wind waves", "deg"},
	  /* 102 */ {"WVHGT", "Sig height of wind waves", "m"},
	  /* 103 */ {"WVPER", "Mean period of wind waves", "s"},
	  /* 104 */ {"SWDIR", "Direction of swell waves", "deg"},
	  /* 105 */ {"SWELL", "Sig height of swell waves", "m"},
	  /* 106 */ {"SWPER", "Mean period of swell waves", "s"},
	  /* 107 */ {"DIRPW", "Primary wave direction", "deg"},
	  /* 108 */ {"PERPW", "Primary wave mean period", "s"},
	  /* 109 */ {"DIRSW", "Secondary wave direction", "deg"},
	  /* 110 */ {"PERSW", "Secondary wave mean period", "s"},
	  /* 111 */ {"NSWRS", "Net short wave (surface)", "W/m^2"},
	  /* 112 */ {"NLWRS", "Net long wave (surface)", "W/m^2"},
	  /* 113 */ {"NSWRT", "Net short wave (top)", "W/m^2"},
	  /* 114 */ {"NLWRT", "Net long wave (top)", "W/m^2"},
	  /* 115 */ {"LWAVR", "Long wave", "W/m^2"},
	  /* 116 */ {"SWAVR", "Short wave", "W/m^2"},
	  /* 117 */ {"GRAD", "Global radiation", "W/m^2"},
	  /* 118 */ {"BRTMP", "Brightness temperature", "K"},
	  /* 119 */ {"LWRAD", "Radiance with respect to wave no.", "W/m/sr"},
	  /* 120 */ {"SWRAD", "Radiance with respect ot wave len.", "W/m^3/sr"},
	  /* 121 */ {"LHTFL", "Latent heat flux", "W/m^2"},
	  /* 122 */ {"SHTFL", "Sensible heat flux", "W/m^2"},
	  /* 123 */ {"BLYDP", "Boundary layer dissipation", "W/m^2"},
	  /* 124 */ {"UFLX", "Zonal momentum flux", "N/m^2"},
	  /* 125 */ {"VFLX", "Meridional momentum flux", "N/m^2"},
	  /* 126 */ {"WMIXE", "Wind mixing energy", "J"},
	  /* 127 */ {"IMGD", "Image data", ""},
	  /* 128 */ {"MSLSA", "Mean sea level pressure (Std Atm)", "Pa"},
	  /* 129 */ {"MSLMA", "Mean sea level pressure (MAPS)", "Pa"},
	  /* 130 */ {"MSLET", "Mean sea level pressure (ETA model)", "Pa"},
	  /* 131 */ {"LFTX", "Surface lifted index", "K"},
	  /* 132 */ {"4LFTX", "Best (4-layer) lifted index", "K"},
	  /* 133 */ {"KX", "K index", "K"},
	  /* 134 */ {"SX", "Sweat index", "K"},
	  /* 135 */ {"MCONV", "Horizontal moisture divergence", "kg/kg/s"},
	  /* 136 */ {"VWSH", "Vertical speed shear", "1/s"},
	  /* 137 */ {"TSLSA", "3-hr pressure tendency (Std Atmos Red)", "Pa/s"},
	  /* 138 */ {"BVF2", "Brunt-Vaisala frequency^2", "1/s^2"},
	  /* 139 */ {"PVMW", "Potential vorticity (mass-weighted)", "1/s/m"},
	  /* 140 */ {"CRAIN", "Categorical rain", "yes=1;no=0"},
	  /* 141 */ {"CFRZR", "Categorical freezing rain", "yes=1;no=0"},
	  /* 142 */ {"CICEP", "Categorical ice pellets", "yes=1;no=0"},
	  /* 143 */ {"CSNOW", "Categorical snow", "yes=1;no=0"},
	  /* 144 */ {"SOILW", "Volumetric soil moisture", "fraction"},
	  /* 145 */ {"PEVPR", "Potential evaporation rate", "W/m^2"},
	  /* 146 */ {"CWORK", "Cloud work function", "J/kg"},
	  /* 147 */ {"U-GWD", "Zonal gravity wave stress", "N/m^2"},
	  /* 148 */ {"V-GWD", "Meridional gravity wave stress", "N/m^2"},
	  /* 149 */ {"PV", "Potential vorticity", "m^2/s/kg"},
	  /* 150 */ {"COVMZ", "Covariance between u and v", "m^2/s^2"},
	  /* 151 */ {"COVTZ", "Covariance between u and T", "K*m/s"},
	  /* 152 */ {"COVTM", "Covariance between v and T", "K*m/s"},
	  /* 153 */ {"CLWMR", "Cloud water", "kg/kg"},
	  /* 154 */ {"O3MR", "Ozone mixing ratio", "kg/kg"},
	  /* 155 */ {"GFLUX", "Ground heat flux", "W/m^2"},
	  /* 156 */ {"CIN", "Convective inhibition", "J/kg"},
	  /* 157 */ {"CAPE", "Convective Avail. Pot. Energy", "J/kg"},
	  /* 158 */ {"TKE", "Turbulent kinetic energy", "J/kg"},
	  /* 159 */ {"CONDP", "Lifted parcel condensation pressure", "Pa"},
	  /* 160 */ {"CSUSF", "Clear sky upward solar flux", "W/m^2"},
	  /* 161 */ {"CSDSF", "Clear sky downward solar flux", "W/m^2"},
	  /* 162 */ {"CSULF", "Clear sky upward long wave flux", "W/m^2"},
	  /* 163 */ {"CSDLF", "Clear sky downward long wave flux", "W/m^2"},
	  /* 164 */ {"CFNSF", "Cloud forcing net solar flux", "W/m^2"},
	  /* 165 */ {"CFNLF", "Cloud forcing net long wave flux", "W/m^2"},
	  /* 166 */ {"VBDSF", "Visible beam downward solar flux", "W/m^2"},
	  /* 167 */ {"VDDSF", "Visible diffuse downward solar flux", "W/m^2"},
	  /* 168 */ {"NBDSF", "Near IR beam downward solar flux", "W/m^2"},
	  /* 169 */ {"NDDSF", "Near IR diffuse downward solar flux", "W/m^2"},
	  /* 170 */ {"RWMR", "Rain water mixing ratio", "kg/kg"},
	  /* 171 */ {"SNMR", "Snow mixing ratio", "kg/kg"},
	  /* 172 */ {"MFLX", "Momentum flux", "N/m^2"},
	  /* 173 */ {"LMH", "Mass point model surface", "non-dim"},
	  /* 174 */ {"LMV", "Velocity point model surface", "non-dim"},
	  /* 175 */ {"MLYNO", "Model layer number (from bottom up)", "non-dim"},
	  /* 176 */ {"NLAT", "Latitude (-90 to +90)", "deg"},
	  /* 177 */ {"ELON", "East longitude (0-360)", "deg"},
	  /* 178 */ {"ICMR", "Ice mixing ratio", "kg/kg"},
	  /* 179 */ {"GRMR", "Graupel mixing ratio", ""},
	  /* 180 */ {"var180", "undefined", "none"},
	  /* 181 */ {"LPSX", "x-gradient of log pressure", "1/m"},
	  /* 182 */ {"LPSY", "y-gradient of log pressure", "1/m"},
	  /* 183 */ {"HGTX", "x-gradient of height", "m/m"},
	  /* 184 */ {"HGTY", "y-gradient of height", "m/m"},
	  /* 185 */ {"TURB", "Turbulence SIGMET/AIRMET", "non-dim"},
	  /* 186 */ {"ICNG", "Icing SIGMET/AIRMET", "non-dim"},
	  /* 187 */ {"LTNG", "Lightning", "non-dim"},
	  /* 188 */ {"var188", "undefined", "none"},
	  /* 189 */ {"VPTMP", "Virtual pot. temperature", "K"},
	  /* 190 */ {"HLCY", "Storm relative helicity", "m^2/s^2"},
	  /* 191 */ {"PROB", "Prob. from ensemble", "non-dim"},
	  /* 192 */ {"PROBN", "Prob. from ensemble norm. to clim. expect.", "non-dim"},
	  /* 193 */ {"POP", "Prob. of precipitation", "%"},
	  /* 194 */ {"CPOFP", "Prob. of frozen precipitation", "%"},
	  /* 195 */ {"CPOZP", "Prob. of freezing precipitation", "%"},
	  /* 196 */ {"USTM", "u-component of storm motion", "m/s"},
	  /* 197 */ {"VSTM", "v-component of storm motion", "m/s"},
	  /* 198 */ {"NCIP", "No. concen. ice particles", "N"},
	  /* 199 */ {"EVBS", "Direct evaporation from bare soil", "W/m^2"},
	  /* 200 */ {"EVCW", "Canopy water evaporation", "W/m^2"},
	  /* 201 */ {"ICWAT", "Ice-free water surface", "%"},
	  /* 202 */ {"var202", "undefined", "none"},
	  /* 203 */ {"var203", "undefined", "none"},
	  /* 204 */ {"DSWRF", "Downward short wave flux", "W/m^2"},
	  /* 205 */ {"DLWRF", "Downward long wave flux", "W/m^2"},
	  /* 206 */ {"UVI", "Ultra violet index (1 hour centered at solar noon)", "J/m^2"},
	  /* 207 */ {"MSTAV", "Moisture availability", "%"},
	  /* 208 */ {"SFEXC", "Exchange coefficient", "(kg/m^3)(m/s)"},
	  /* 209 */ {"MIXLY", "No. of mixed layers next to surface", "integer"},
	  /* 210 */ {"TRANS", "Transpiration", "W/m^2"},
	  /* 211 */ {"USWRF", "Upward short wave flux", "W/m^2"},
	  /* 212 */ {"ULWRF", "Upward long wave flux", "W/m^2"},
	  /* 213 */ {"CDLYR", "Non-convective cloud", "%"},
	  /* 214 */ {"CPRAT", "Convective precip. rate", "kg/m^2/s"},
	  /* 215 */ {"TTDIA", "Temperature tendency by all physics", "K/s"},
	  /* 216 */ {"TTRAD", "Temperature tendency by all radiation", "K/s"},
	  /* 217 */ {"TTPHY", "Temperature tendency by non-radiation physics", "K/s"},
	  /* 218 */ {"PREIX", "Precip index (0.0-1.00)", "fraction"},
	  /* 219 */ {"TSD1D", "Std. dev. of IR T over 1x1 deg area", "K"},
	  /* 220 */ {"NLGSP", "Natural log of surface pressure", "ln(kPa)"},
	  /* 221 */ {"HPBL", "Planetary boundary layer height", "m"},
	  /* 222 */ {"5WAVH", "5-wave geopotential height", "gpm"},
	  /* 223 */ {"CNWAT", "Plant canopy surface water", "kg/m^2"},
	  /* 224 */ {"SOTYP", "Soil type (Zobler)", "0..9"},
	  /* 225 */ {"VGTYP", "Vegetation type (as in SiB)", "0..13"},
	  /* 226 */ {"BMIXL", "Blackadar's mixing length scale", "m"},
	  /* 227 */ {"AMIXL", "Asymptotic mixing length scale", "m"},
	  /* 228 */ {"PEVAP", "Pot. evaporation", "kg/m^2"},
	  /* 229 */ {"SNOHF", "Snow phase-change heat flux", "W/m^2"},
	  /* 230 */ {"5WAVA", "5-wave geopot. height anomaly", "gpm"},
	  /* 231 */ {"MFLUX", "Convective cloud mass flux", "Pa/s"},
	  /* 232 */ {"DTRF", "Downward total radiation flux", "W/m^2"},
	  /* 233 */ {"UTRF", "Upward total radiation flux", "W/m^2"},
	  /* 234 */ {"BGRUN", "Baseflow-groundwater runoff", "kg/m^2"},
	  /* 235 */ {"SSRUN", "Storm surface runoff", "kg/m^2"},
	  /* 236 */ {"var236", "undefined", "none"},
	  /* 237 */ {"O3TOT", "Total ozone", "kg/m^2"},
	  /* 238 */ {"SNOWC", "Snow cover", "%"},
	  /* 239 */ {"SNOT", "Snow temperature", "K"},
	  /* 240 */ {"var240", "undefined", "none"},
	  /* 241 */ {"LRGHR", "Large scale condensation heating", "K/s"},
	  /* 242 */ {"CNVHR", "Deep convective heating", "K/s"},
	  /* 243 */ {"CNVMR", "Deep convective moistening", "kg/kg/s"},
	  /* 244 */ {"SHAHR", "Shallow convective heating", "K/s"},
	  /* 245 */ {"SHAMR", "Shallow convective moistening", "kg/kg/s"},
	  /* 246 */ {"VDFHR", "Vertical diffusion heating", "K/s"},
	  /* 247 */ {"VDFUA", "Vertical diffusion zonal accel", "m/s^2"},
	  /* 248 */ {"VDFVA", "Vertical diffusion meridional accel", "m/s^2"},
	  /* 249 */ {"VDFMR", "Vertical diffusion moistening", "kg/kg/s"},
	  /* 250 */ {"SWHR", "Solar radiative heating", "K/s"},
	  /* 251 */ {"LWHR", "Longwave radiative heating", "K/s"},
	  /* 252 */ {"CD", "Drag coefficient", "non-dim"},
	  /* 253 */ {"FRICV", "Friction velocity", "m/s"},
	  /* 254 */ {"RI", "Richardson number", "non-dim"},
	  /* 255 */ {"var255", "undefined", "none"} };

const int PDS::_parmTableSize = sizeof(_parmTable) / sizeof(PDS::_ParmTable);


PDS::PDS() :
  GribSection(),
  _expectedSize(EXPECTED_SIZE),
  _tableVersion(0),
  _centerId(0),
  _processGenId(0),
  _gridId(0),
  _parameterId(0),
  _parameterLongName(""),
  _parameterName(""),
  _parameterUnits(""),
  _gdsPresent(true),
  _bmsPresent(false),
  _year(1970),
  _month(1),
  _day(1),
  _hour(0),
  _min(0),
  _forecastUnitId(253),
  _forecastPeriod1(-1),
  _forecastPeriod2(-1),
  _timeRangeId(0),
  _avgNum(0),
  _avgNumMissing(0),
  _subCenterId(0),
  _decimalScale(0)
{
  _nBytes = NUM_SECTION_BYTES;
}

void
PDS::setTime(const DateTime &generate_time,
	     const int forecast_period,
	     const time_period_units_t forecast_period_units)
{
  _year = generate_time.getYear();
  _month = generate_time.getMonth();
  _day = generate_time.getDay();
  _hour = generate_time.getHour();
  _min = generate_time.getMin();
    
  _forecastUnitId = forecast_period_units;
  _forecastPeriod1 = forecast_period;
  _forecastPeriod2 = 0;
}
  

void
PDS::setTime(const DateTime &generate_time,
	     const int time_period1, const int time_period2,
	     const time_period_units_t forecast_period_units)
{
  _year = generate_time.getYear();
  _month = generate_time.getMonth();
  _day = generate_time.getDay();
  _hour = generate_time.getHour();
  _min = generate_time.getMin();
    
  _forecastUnitId = forecast_period_units;
  _forecastPeriod1 = time_period1;
  _forecastPeriod2 = time_period2;
}
  

void
PDS::getTime(DateTime &generate_time,
	     int &forecast_secs) const
{
  generate_time.set(_year, _month, _day, _hour, _min);
  forecast_secs = getForecastTime();
}
  

int
PDS::unpack( ui08 *pdsPtr)
{
  // reinitialize variables 

   _vertType.set();
  
   _gdsPresent      = true;
   _bmsPresent      = false;
   _year            = 1970;
   _month           = 1;
   _day             = 1;
   _hour            = 0;
   _min             = 0;
   _forecastUnitId  = 253;
   _forecastPeriod1 = -1;
   _forecastPeriod2 = -1;

   //
   // Length in bytes of this section
   //
   _nBytes = _upkUnsigned3( pdsPtr[0], pdsPtr[1], pdsPtr[2] );

   //
   // Check that section size is an expected value. GMC has encountered RUC files
   // with corrupt records. This check is useful in detecting bad records.
   //
//   if(_nBytes > _expectedSize) {
//     cout << "ERROR: Possible corrupt record. PDS size in bytes is " << 
//       _nBytes << endl << "expected size in bytes is " << _expectedSize << endl;
//     cout << "If PDS size is correct use setExpectedSize or GribRecord::setPdsExpectedSize method to pass test." 
//	  << endl;
//     return( GRIB_FAILURE );
//   }

   //
   // Various identification numbers - See Grib docmumentation
   //
   _tableVersion = (int) pdsPtr[3];
   _centerId     = (int) pdsPtr[4];
   _processGenId = (int) pdsPtr[5];
   _gridId       = (int) pdsPtr[6];

   //
   // Flags
   //
   if( pdsPtr[7] & 128 )
      _gdsPresent = true;
   if( pdsPtr[7] & 64 )
      _bmsPresent = true;

   // 
   // Identification numbers - See Grib documentation
   //
   _parameterId = (int) pdsPtr[8];
    int layer_type = (int) pdsPtr[9];
    int val1 = (int) pdsPtr[10];
    int val2 = (int) pdsPtr[11];

   //
   // Field information
   // 
   _parameterLongName = getLongName(_parameterId,layer_type,val1,val2);
   _parameterName = (_parmTable + _parameterId)->name;
   _parameterUnits = (_parmTable + _parameterId)->units;
   
   //
   // Level information
   // 
   _levels( pdsPtr[9], pdsPtr[10], pdsPtr[11] );
   
   //
   // Reference century 
   //
   int century  = (int) pdsPtr[24];

   //
   // Date and time
   //
   if( pdsPtr[12] == 0 ) {
      _year = (int) pdsPtr[12] + ( 100 * century );
   }
   else {
      _year = (int) pdsPtr[12] + ( 100 * (century - 1) );
   }
   
   _month = (int) pdsPtr[13];
   _day = (int) pdsPtr[14];
   _hour = (int) pdsPtr[15];
   _min = (int) pdsPtr[16];
   
   //
   // Forecast time unit id number - See Grib documentation
   //
   _forecastUnitId  = (int) pdsPtr[17];

   //
   // Forecast time periods - units depend on forecastUnitId
   //
   _forecastPeriod1 = (int) pdsPtr[18];
   _forecastPeriod2 = (int) pdsPtr[19];

   //
   // Time range indicator
   //
   _timeRangeId = (int) pdsPtr[20];

   //
   // Number included in average and number missing from averages
   // or accumulations when timeRangeId indicates an average
   // or accumulation
   //
   _avgNum = _upkUnsigned2( pdsPtr[21], pdsPtr[22] );
   _avgNumMissing = (int) pdsPtr[23];

   //
   // Id number for sub center
   //
   _subCenterId = (int) pdsPtr[25];

   //
   // Decimal scale factor - used in data packing
   //   Y * 10^D = R + (X*2^E)
   //  where Y = original value
   //        D = decimal scale factor
   //        R = reference value
   //        X = internal value
   //        E = binary scale factor
   //  "The WMO Format for the Storage of Weather Product
   //  Information and the Exchange of Weather Product Messages
   //  in Gridded Binary Form as Used by NCEP Central Operations",
   //  Clifford H. Dey, March 10, 1998, Sec. 0, page 4.
   //
   _decimalScale = _upkSigned2( pdsPtr[26], pdsPtr[27] );
   
   //
   // If there is anything left in this section, stick
   // it in the reserved buffer
   //
   if( _nBytes > 28 ) {
      _nReservedBytes = _nBytes - 28;
      _reserved = new ui08[_nReservedBytes];
      memcpy( (void *) _reserved, pdsPtr+28, _nReservedBytes );
   }

   return( GRIB_SUCCESS );
   
}

int PDS::pack(ui08 *pdsPtr) 
{
  // Length in bytes of this section

  _pkUnsigned3(_nBytes, &(pdsPtr[0]));
  
  // Various identification numbers - See Grib docmumentation

  pdsPtr[3] = (ui08)_tableVersion;
  pdsPtr[4] = (ui08)_centerId;
  pdsPtr[5] = (ui08)_processGenId;
  pdsPtr[6] = (ui08)_gridId;
  
  // Flags

  pdsPtr[7] = 0;
  if (_gdsPresent)
    pdsPtr[7] = 128;
  if (_bmsPresent)
    pdsPtr[7] = pdsPtr[7] + 64;

  // Identification numbers - See Grib documentation

  pdsPtr[8] = (ui08)_parameterId;
  
  // Level information

  _packLevels(&(pdsPtr[9]));
  
  // Date and time

  if (_year > 2000)
  {
    pdsPtr[12] = _year - 2000;
    pdsPtr[24] = 21;
  }
  else
  {
    pdsPtr[12] = _year - 1900;
    pdsPtr[24] = 20;
  }
  
  pdsPtr[13] = _month;
  pdsPtr[14] = _day;
  pdsPtr[15] = _hour;
  pdsPtr[16] = _min;
   
  // Forecast time unit id number - See Grib documentation

  pdsPtr[17] = _forecastUnitId;

  // Forecast time periods - units depend on forecastUnitId

  pdsPtr[18] = _forecastPeriod1;
  pdsPtr[19] = _forecastPeriod2;

  // Time range indicator

  pdsPtr[20] = _timeRangeId;
  
  // Number included in average and number missing from averages
  // or accumulations when timeRangeId indicates an average
  // or accumulation

  _pkUnsigned2(_avgNum, &(pdsPtr[21]));
  pdsPtr[23] = _avgNumMissing;

  // Id number for sub center

  pdsPtr[25] = _subCenterId;

  // Decimal scale factor - used in data packing

  _pkSigned2(_decimalScale, &(pdsPtr[26]));
   
  return GRIB_SUCCESS;
}

int
PDS::getForecastTime() const
{
   int nSecsPerUnit;
   int forecastTime;
   
   switch( _forecastUnitId ) {
       case 0:
	  nSecsPerUnit = 60;
	  break;
	  
       case 1:
	  nSecsPerUnit = 60*60;
	  break;
	  
       case 2:
	  nSecsPerUnit = 24*60*60;
	  break;
	  
       case 10:
	  nSecsPerUnit = 3*60*60;
	  break;
	  
       case 11:
	  nSecsPerUnit = 6*60*60;
	  break;
	  
       case 12:
	  nSecsPerUnit = 12*60*60;
	  break;
	  
       case 254:
	  nSecsPerUnit = 1;
	  break;
	  
       default:
	  return( -1 );
	  break;
   }

   // the usual case (P1 in octet 19)
   forecastTime = nSecsPerUnit * _forecastPeriod1;

  // sometimes the period is in octets 19 and 20
  if (_timeRangeId == P1_OCCUPIES_19_AND_20) {
    forecastTime = nSecsPerUnit * ((_forecastPeriod1 << 8) + _forecastPeriod2);
  }
 
   return( forecastTime );
}

time_t
PDS::getGenerateTime() const
{
   UTIMstruct genTime;
   
   genTime.year = _year;
   genTime.month = _month;
   genTime.day = _day;
   genTime.hour = _hour;
   genTime.min = _min;
   genTime.sec = 0;
   
   return( UTIMdate_to_unix( &genTime ) );
}

void
PDS::_levels(ui08 pds_b9, ui08 pds_b10, ui08 pds_b11) 
{
  int kpds7 = (int) _upkUnsigned2(pds_b10,  pds_b11);

  /* octets 11 and 12 */
  int o11 = kpds7 / 256;
  int o12 = kpds7 % 256;

  _vertType.set((GribVertType::vert_type_t)pds_b9, o11, o12, kpds7);
}

void
PDS::_packLevels(ui08 *buffer) 
{
  // Initialize values

  buffer[0] = (ui08)_vertType.getLevelType();

  if (_vertType.isSingleLevelValue())
  {
    _pkUnsigned2(_vertType.getLevelValue(), &(buffer[1]));
  }
  else
  {
    buffer[1] = (ui08)_vertType.getLevelValueTop();
    buffer[2] = (ui08)_vertType.getLevelValueBottom();
  }
}

void
PDS::print(FILE *stream) const
{
  fprintf(stream, "\n\n");
  fprintf(stream, "Grib Product Definition Section:\n");
  fprintf(stream, "--------------------------\n");
  fprintf(stream, "PDS length %d\n", _nBytes);
  fprintf(stream, "Parameter Table Version %d\n", _tableVersion);
  fprintf(stream, "Center Id %d\n", _centerId);
  fprintf(stream, "Generating Process Id %d\n", _processGenId);
  fprintf(stream, "Grid Id %d\n", _gridId);
  if (_gdsPresent)
    fprintf(stream, "Grid Description Section present\n");
  else
    fprintf(stream, "Grid Description Section not included\n");
  if (_bmsPresent)
    fprintf(stream, "Bit Map Section present\n");
  else
    fprintf(stream, "Bit Map Section not included\n");
  fprintf(stream, "Parameter/Units raw field %d\n", _parameterId);
  if (_parameterId < _parmTableSize)
  {
    fprintf(stream, "    long name %s\n", _parameterLongName.c_str());
    fprintf(stream, "    name %s\n", (_parmTable + _parameterId)->name.c_str());
    fprintf(stream, "    units %s\n", (_parmTable + _parameterId)->units.c_str());
  }
  
  _vertType.print(stream);
  
  fprintf(stream, "Year %d\n", _year);
  fprintf(stream, "Month %d\n", _month);
  fprintf(stream, "Day %d\n", _day);
  fprintf(stream, "Hour %d\n", _hour);
  fprintf(stream, "Minute %d\n", _min);
  fprintf(stream, "Forecast time unit id number %d\n", _forecastUnitId);
  fprintf(stream, "Forecast time period 1 %d\n", _forecastPeriod1);
  fprintf(stream, "Forecast time period 2 %d\n", _forecastPeriod2);
  fprintf(stream, "Time range indicator %d\n", _timeRangeId);
  fprintf(stream, "Number included in average %d\n",  _avgNum);
  fprintf(stream, "Number missing from averages %d\n", _avgNumMissing);
  fprintf(stream, "Sub-center identification %d\n", _subCenterId);
  fprintf(stream, "Decimal scale factor D %d\n", _decimalScale);

}

void
PDS::print(ostream &stream) const
{
  stream << endl << endl;
  stream << "Grib Product Definition Section:" << endl;
  stream << "--------------------------" << endl;
  stream << "PDS length " <<  _nBytes  << endl;
  stream << "Parameter Table Version " <<  _tableVersion  << endl;
  stream << "Center Id " <<  _centerId  << endl;
  stream << "Generating Process Id " <<  _processGenId  << endl;
  stream << "Grid Id " <<  _gridId  << endl;
  if (_gdsPresent)
    stream << "Grid Description Section present" << endl;
  else
    stream << "Grid Description Section not included" << endl;
  if (_bmsPresent)
    stream << "Bit Map Section present" << endl;
  else
    stream << "Bit Map Section not included" << endl;
  stream << "Parameter/Units raw field " <<  _parameterId << endl;
  if (_parameterId < _parmTableSize)
  {
    stream << "    long name " <<  
	    (_parmTable + _parameterId)->long_name << endl;
    stream << "    name " <<  
	    (_parmTable + _parameterId)->name << endl;
    stream << "    units " <<  
      (_parmTable + _parameterId)->units << endl;
  }
  
  _vertType.print(stream);
  
  stream << "Year " <<  _year << endl;
  stream << "Month " <<  _month << endl;
  stream << "Day " <<  _day << endl;
  stream << "Hour " <<  _hour << endl;
  stream << "Minute " <<  _min << endl;
  stream << "Forecast time unit id number " <<  _forecastUnitId << endl;
  stream << "Forecast time period 1 " <<  _forecastPeriod1 << endl;
  stream << "Forecast time period 2 " <<  _forecastPeriod2 << endl;
  stream << "Time range indicator " <<  _timeRangeId << endl;
  stream << "Number included in average " <<   _avgNum << endl;
  stream << "Number missing from averages " <<  _avgNumMissing << endl;
  stream << "Sub-center identification " <<  _subCenterId << endl;
  stream << "Decimal scale factor D " <<  _decimalScale << endl;

}

char* 
PDS::getLongName(const int& id, const int &layer_type, const int &v1, const int &v2)
{

  static char buf[4096];
  char buf2[1024];

  strncpy(buf,(_parmTable + id)->long_name.c_str(),1024);

  switch (layer_type) {
    default:  
        // Do nothing;
    break;

    case 1:
      strncat(buf," at surface",1024);
    break;

    case 2:
      strncat(buf," cloud base",1024);
    break;

    case 3:
      strncat(buf," cloud top",1024);
    break;

    case 4:
      strncat(buf," freezing level",1024);
    break;

    case 5:
      strncat(buf," adiabatic condensation level",1024);
    break;

    case 6:
      strncat(buf," maximum wind level",1024);
    break;

    case 7:
      strncat(buf," tropopause level",1024);
    break;

    case 8:
      strncat(buf," top of atmosphere",1024);
    break;

    case 9:
      strncat(buf," sea bottom",1024);
    break;

    case 100:
      strncat(buf, " isobaric hectoPascals",1024);
    break;

    case 101:
      sprintf(buf2," %d kpa to %d kpa",v2,v1);
      strncat(buf,buf2,1024);
    break;

    case 102:
      strncat(buf," mean sea level",1024);
    break;

    case 103:
      sprintf(buf2," %dm MSL",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 104:
      sprintf(buf2," between %dm and %dm MSL",v1 * 100,v2 * 100);
      strncat(buf,buf2,1024);
    break;

    case 105:
      sprintf(buf2," %dm AGL ",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 106:
      sprintf(buf," %dm to %dm AGL",v1 * 100,v2 * 100);
      strncat(buf,buf2,1024);
    break;

    case 107:
      sprintf(buf2," sigma level %g",((v1 * 256) + v2) / 10000.0);
      strncat(buf,buf2,1024);
    break;

    case 108:
      sprintf(buf2," %g to %g sigma",v1 / 100.0,v2 / 100.0);
      strncat(buf,buf2,1024);
    break;

    case 109:
      sprintf(buf2," hybrid level %d",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 110:
      sprintf(buf2," %d to %d hybrid",v2,v1);
      strncat(buf,buf2,1024);
    break;

    case 111:
      sprintf(buf2," %dcm below Surface",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 112:
      sprintf(buf2," %dcm to %dcm below Surface",v1,v2);
      strncat(buf,buf2,1024);
    break;

    case 113:
      sprintf(buf2," %d isentropic level (K)",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 114:
      sprintf(buf2," %d to %d  isentropic levels (K)",475 - v1, 475 -  v2);
      strncat(buf,buf2,1024);
    break;

    case 121:
      sprintf(buf2," %dmb to %dmb",1100 - v1, 1100 -  v2);
      strncat(buf,buf2,1024);
    break;

    case 125:
      sprintf(buf2," %d cm high ",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 128:
      sprintf(buf2," %g to %g sigma ",1.1 - (v1/1000.0), 1.1 - (v2/1000.0));
      strncat(buf,buf2,1024);
    break;

    case 141:
      sprintf(buf2," %dmb to %dmb",v1, 1100 -  v2);
      strncat(buf,buf2,1024);
    break;

    case 160:
      sprintf(buf2," %dm below Sea Surface",((v1 * 256) + v2));
      strncat(buf,buf2,1024);
    break;

    case 200:
      strncat(buf," entire atmosphere ",1024);
    break;

    case 201:
      strncat(buf," entire ocean ",1024);
    break;
  }

  return(buf);
}

const char* 
PDS::getName(const int& id)
{
  return((_parmTable + id)->name.c_str());
}

const char* 
PDS::getUnits(const int& id)
{
  return((_parmTable + id)->units.c_str());
}


static const int _kEnsembleBase = 29;

// Return if this record contains ensemble data.
bool PDS::isEnsemble()
{
  if (_reserved == NULL)
    return false;
  if (_nReservedBytes < 45 - _kEnsembleBase + 1)
    return false;

  // If octet 44 is 1 then we have ensemble data
  return (_reserved[41 - _kEnsembleBase] == 1);
}


// Return the ensemble product identifier.
int PDS::getEnsembleProduct()
{
  return (int)_reserved[44 - _kEnsembleBase];
}

