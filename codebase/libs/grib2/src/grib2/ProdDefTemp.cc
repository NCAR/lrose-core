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
///////////////////////////////////////////////////////////
// ProdDefTemp.cc : implementation for the ProdDefTemp class 
//                  This is an abstract base class to allow
//                  objects to represent various product 
//                  definition templates
//       
///////////////////////////////////////////////////////////

#include <grib2/ProdDefTemp.hh>
#include <iostream>
#include <map>

using namespace std;

namespace Grib2 {

/////////////////////////////////////////////////////////////////////////
//
// The following tables have been maintained from NCEP's web page at:
// http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml
//
//////////////////////////////////////////////////////////////////////////

/*                    GRIB2 Code Table 4.5                       */
/*                FIXED SURFACE TYPES AND UNITS                  */
const ProdDefTemp::_GRIB2SurfTable ProdDefTemp::_surface[] = {
   /* 1 */ {"SFC", "Ground or water surface", "-"},
   /* 2 */ {"CBL", "Cloud base level", "-"},
   /* 3 */ {"CTL", "Level of cloud tops", "-"},
   /* 4 */ {"0DEG", "Level of 0 degree C isotherm", "-"},
   /* 5 */ {"ADCL", "Level of adiabatic condensation lifted from the surface", "-"},
   /* 6 */ {"MWSL", "Maximum wind level", "-"},
   /* 7 */ {"TRO", "Tropopause", "-"},
   /* 8 */ {"NTAT", "Nominal top of atmosphere", "-"},
   /* 9 */ {"SEAB", "Sea bottom", "-"},
   /* 10 */ {"EATM", "Entire Atmosphere", "-"},
   /* 11 */ {"CBB", "Cumulonimbus Base", "m"},
   /* 12 */ {"CBT", "Cumulonimbus Top", "m"},
   /* 13 */ {"", "Lowest level where vertically integrated cloud cover exceeds the specified percentage (cloud base for a given percentage cloud cover)", "%"},
   /* 14 */ {"LFC", "Level of free convection", ""},
   /* 15 */ {"CCL", "Convection condensation level", ""},
   /* 16 */ {"LNB", "Level of neutral buoyancy or equilibrium", ""},
   /* 17 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 18 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 19 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 20 */ {"TMPL", "Isothermal level", "K"},
   /* 100 */ {"ISBL", "Isobaric surface", "Pa"},
   /* 101 */ {"MSL", "Mean sea level", "-"},
   /* 102 */ {"GPML", "Specific altitude above mean sea level", "m"},
   /* 103 */ {"HTGL", "Specified height level above ground", "m"},
   /* 104 */ {"SIGL", "Sigma level", "sigma value"},
   /* 105 */ {"HYBL", "Hybrid level", "-"},
   /* 106 */ {"DBLL", "Depth below land surface", "m"},
   /* 107 */ {"THEL", "Isentropic (theta) level", "K"},
   /* 108 */ {"SPDL", "Level at specified pressure difference from ground to level", "Pa"},
   /* 109 */ {"PVL", "Potential vorticity surface", "K m^2 kg^-1 s^-1"},
   /* 110 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 111 */ {"ETAL", "Eta level", "-"},
   /* 112 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 113 */ {"LOGHYB", "Logarithmic Hybrid Coordinate", "-"},
   /* 114 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 115 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 116 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 117 */ {"MLD", "Mixed layer depth", "m"}, 
   /* 118 */ {"HYBH", "Hybrid Height Level", "-"},
   /* 119 */ {"HYBP", "Hybrid Pressure Level", "-"},
   /* 120 */ {"PREST", "Pressure Thickness", "Pa"},
   /* 150 */ {"", "Generalized Vertical Height Coordinate", "-"},
   /* 160 */ {"DBSL", "Depth below sea level", "m"},
   /* 161 */ {"", "Depth Below Water Surface", "m"},
   /* 162 */ {"", "Lake or River Bottom", "-"},
   /* 163 */ {"", "Bottom Of Sediment Layer", "-"},
   /* 164 */ {"", "Bottom Of Thermally Active Sediment Layer", "-"},
   /* 165 */ {"", "Bottom Of Sediment Layer Penetrated By Thermal Wave", "-"},
   /* 166 */ {"", "Maxing Layer", "-"},
   /* 167 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 168 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 169 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 170 */ {"IOND", "Ionospheric D-region Level", "-"},
   /* 171 */ {"IONE", "Ionospheric E-region Level", "-"},
   /* 172 */ {"IONF1", "Ionospheric F1-region Level", "-"},
   /* 173 */ {"IONF2", "Ionospheric F2-region Level", "-"}, 
   /* 174 */ {"AGL", "Adjusted Ground Level", "ft"},
   /* 200 */ {"EATM", "Entire atmosphere (considered as a single layer)", "-"},
   /* 201 */ {"EOCN", "Entire ocean (considered as a single layer)", "-"},
   /* 202 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 203 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 204 */ {"HTFL", "Highest tropospheric freezing level", "-"},
   /* 205 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 206 */ {"GCBL", "Grid scale cloud bottom level", "-"},
   /* 207 */ {"GCTL", "Grid scale cloud top level", "-"},
   /* 208 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 209 */ {"BCBL", "Boundary layer cloud bottom level", "-"},
   /* 210 */ {"BCTL", "Boundary layer cloud top level", "-"},
   /* 211 */ {"BCY", "Boundary layer cloud layer", "-"},
   /* 212 */ {"LCBL", "Low cloud bottom level", "-"},
   /* 213 */ {"LCTL", "Low cloud top level", "-"},
   /* 214 */ {"LCY", "Low cloud layer", "-"},
   /* 215 */ {"CCEIL", "Cloud ceiling", "-"},
   /* 216 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 217 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 218 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 219 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 220 */ {"PBL", "Planetary Boundary Layer", "-"},
   /* 221 */ {"LBHYBL", "Layer Between Two Hybrid Levels", "-"},
   /* 222 */ {"MCBL", "Middle cloud bottom level", "-"},
   /* 223 */ {"MCTL", "Middle cloud top level", "-"},
   /* 224 */ {"MCY", "Middle cloud layer", "-"},
   /* 232 */ {"HCBL", "High cloud bottom level", "-"},
   /* 233 */ {"HCTL", "High cloud top level", "-"},
   /* 234 */ {"HCY", "High cloud layer", "-"},
   /* 235 */ {"OITL", "Ocean Isotherm Level (1/10 deg C)", "C"},
   /* 236 */ {"OLYR", "Layer between two depths below ocean surface", "-"},	
   /* 237 */ {"OBML", "Bottom of Ocean Mixed Layer", "m"},
   /* 238 */ {"OBIL", "Bottom of Ocean Isothermal Layer", "m"},
   /* 239 */ {"S26CY", "Layer Ocean Surface and 26C Ocean Isothermal Level", "-"},
   /* 240 */ {"OMXL", "Ocean Mixed Layer", "-"},
   /* 241 */ {"OSEQD", "Ordered Sequence of Data", "-"},
   /* 242 */ {"CCBL", "Convective cloud bottom level", "-"},
   /* 243 */ {"CCTL", "Convective cloud top level", "-"},
   /* 244 */ {"CCY", "Convective cloud layer", "-"},
   /* 245 */ {"LLTW", "Lowest level of the wet bulb zero", "-"},
   /* 246 */ {"MTHE", "Maximum equivalent potential temperature level", "-"},
   /* 247 */ {"EHLT", "Equilibrium level", "-"},
   /* 248 */ {"SCBL", "Shallow convective cloud bottom level", "-"},
   /* 249 */ {"SCTL", "Shallow convective cloud top level", "-"},
   /* 250 */ {"UNKNOWN", "unknown primary surface type", ""},
   /* 251 */ {"DCBL", "Deep convective cloud bottom level", "-"},
   /* 252 */ {"DCTL", "Deep convective cloud top level", "-"},
   /* 253 */ {"LBLSW", "Lowest bottom level of supercooled liquid water layer", "-"},
   /* 254 */ {"HTLSW", "Highest top level of supercooled liquid water layer", "-"},
   /* 255 */ {"MISSING", "", ""},
};


/*                       GRIB2 Code Table 4.2                    */
/* PARAMETER NUMBER BY PRODUCT DISCIPLINE AND PARAMETER CATEGORY */

// Note on comment style:  4.2 : 1.2
// Fields are unique to a discipline number a parameter category and parameter number.
// Discipline Number is octet 7 in section 0 (IS, Indicator Section).
// Parameter Category is octet 10 in section 4 (PDS, Product Definition Section).
// Parameter Number is octet 11 in section 4 (PDS, Product Definition Section).
//
// Numbers greater than 191 are for local use only, 
// these require checking the orginating/generating center and subcenter
// found in the IS, Identification Section, and using the appropriate local use table.
//
// For example the Temperature (TMP) field is 4.2 : 0.0 : 0
// 4.2 for Code Table 4.2
// 0.0 for Discipline 0, Category 0
// 0 for Parameter Number 0  (the index into the meteoTemp array)

/* Known Disciplines:
   0  Meteorological Products
   1  Hydrological Products
   2  Land Surface Products
   3  Space Products
   10 Oceanographic Products  */

/* Known Categories, by Discipline:
  Discipline 0, Meteorological products:
    Category 0 	 Temperature
    Category 1 	 Moisture
    Category 2 	 Momentum
    Category 3 	 Mass
    Category 4 	 Short wave radiation
    Category 5 	 Long wave radiation
    Category 6 	 Cloud
    Category 7 	 Thermodynamic stability indices
    Category 8   Kinematic stability indicies
    Category 9   Temperature probabilities 
    Category 10  Moisture probabilities 
    Category 11  Momentum probabilities
    Category 12  Mass probabilities 
    Category 13  Aerosols
    Category 14  Trace gases (e.g. Ozone, CO2)
    Category 15  Radar
    Category 18  Nuclear/radiology
    Category 19  Physical atmospheric properties
    Category 20-189  Reserved 
    Category 190 CCITT IA5 string
    Category 191 Miscellaneous
    Category 192-254  Reserved for Local Use
    Category 255 Missing 

  Discipline 1, Hydrologic products
    Category 0 	Hydrology basic products
    Category 1 	Hydrology probabilities
    Category 2-191  Reserved 
    Category 192-254  Reserved for Local Use
    Category 255 Missing 

  Discipline 2, Land Surface products
    Category 0 	Vegetation/Biomass
    Category 3  Soil products
    Category 4-191  Reserved 
    Category 192-254  Reserved for Local Use
    Category 255 Missing 

  Discipline 3, Space products
    Category 0 	Image format products
    Category 1  Quantitative products
    Category 2-191  Reserved 
    Category 192-254  Reserved for Local Use
    Category 255 Missing 

  Discipline 10, Oceanographic products
    Category 0 	Waves
    Category 1  Currents
    Category 2  Ice
    Category 3  Surface properties
    Category 4  Sub-surface properties
    Category 5-190  Reserved 
    Category 191 Miscellaneous
    Category 192-254  Reserved for Local Use
    Category 255 Missing 
*/

/* GRIB2 Code table 4.2 : 0.0 */
/* Meteorological products, temperature category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoTemp[30] = {
   /* 0 */ {"TMP", "Temperature", "K"},
   /* 1 */ {"VTMP", "Virtual temperature", "K"},
   /* 2 */ {"POT", "Potential temperature", "K"},
   /* 3 */ {"EPOT", "Pseudo-adiabatic potential temperature", "K"},
   /* 4 */ {"TMAX", "Maximum Temperature", "K"},
   /* 5 */ {"TMIN", "Minimum Temperature", "K"},
   /* 6 */ {"DPT", "Dew point temperature", "K"},
   /* 7 */ {"DEPR", "Dew point depression", "K"},
   /* 8 */ {"LAPR", "Lapse rate", "K/m"},
   /* 9 */ {"TMPA", "Temperature anomaly", "K"},
   /* 10 */ {"LHTFL", "Latent heat net flux", "W m^-2"},
   /* 11 */ {"SHTFL", "Sensible heat net flux", "W m^-2"},
   /* 12 */ {"HEATX", "Heat index", "K"},
   /* 13 */ {"WCF", "Wind chill factor", "K"},
   /* 14 */ {"MINDPD", "Minimum dew point depression", "K"},
   /* 15 */ {"VPTMP", "Virtual potential temperature", "K"},
   /* 16 */ {"SNOHF", "Snow phase change heat flux", "W m^-2"},
   /* 17 */ {"SKINT", "Skin Temperature", "K"},
   /* 18 */ {"SNOT", "Snow Temperature", "K"},
   /* 19 */ {"TTCHT", "Turbulent Transfer Coefficient for Heat", "numeric"}, 
   /* 20 */ {"TDCHT", "Turbulent Diffusion Coefficient for Heat", "m^2 s^-1"},
   /* 21 */ {"APTMP", "Apparent Temperature", "K"},
   /* 22 */ {"TTSWR", "Temperature Tendency due to Short-Wave Radiation", "K s^-1"},
   /* 23 */ {"TTLWR", "Temperature Tendency due to Long-Wave Radiation", "K s^-1"},
   /* 24 */ {"TTSWRCS", "Temperature Tendency due to Short-Wave Radiation, Clear Sky", "K s^-1"},
   /* 25 */ {"TTLWRCS", "Temperature Tendency due to Long-Wave Radiation, Clear Sky", "K s^-1"},
   /* 26 */ {"TTPARM", "Temperature Tendency due to parameterizations", "K s^-1"},
   /* 27 */ {"WETBT", "Wet Bulb Temperature", "K"},
   /* 28 */ {"UCTMP", "Unbalanced Component of Temperature", "K"},
   /* 29 */ {"TMPADV", "Temperature Advection", "K s-1"},
};
  
/* GRIB2 Code table 4.2 : 0.1 */
/* Meteorological products, moisture category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoMoist[122] = {
   /* 0 */ {"SPFH",   "Specific humidity", "kg kg^-1"},
   /* 1 */ {"RH",     "Relative Humidity", "%"},
   /* 2 */ {"MIXR",   "Humidity mixing ratio", "kg kg^-1"},
   /* 3 */ {"PWAT",   "Precipitable water", "kg m^-2"},
   /* 4 */ {"VAPP",   "Vapor Pressure", "Pa"},
   /* 5 */ {"SATD",   "Saturation deficit", "Pa"},
   /* 6 */ {"EVP",    "Evaporation", "kg m^-2"},
   /* 7 */ {"PRATE",  "Precipitation rate", "kg m^-2 s^-1"},
   /* 8 */ {"APCP",   "Total precipitation", "kg m^-2"},
   /* 9 */ {"NCPCP",  "Large scale precipitation", "kg m^-2"},
   /* 10 */ {"ACPCP", "Convective precipitation", "kg m^-2"},
   /* 11 */ {"SNOD",  "Snow depth", "m"},
   /* 12 */ {"SRWEQ", "Snowfall rate water equivalent", "kg m^-2 s^-1"},
   /* 13 */ {"WEASD", "Water equivalent of accumulated snow depth","kg m^-2"},
   /* 14 */ {"SNOC",  "Convective snow", "kg m^-2"},
   /* 15 */ {"SNOL",  "Large scale snow", "kg m^-2"},
   /* 16 */ {"SNOM",  "Snow melt", "kg m^-2"},
   /* 17 */ {"SNOAGE","Snow age", "day"},
   /* 18 */ {"ABSH",    "Absolute humidity", "kg m^-3"},
   /* 19 */ {"PTYPE", "Precipitation type", "categorical"}, //(1 Rain, 2 Thunderstorm, "
             //"3 Freezing Rain, 4 Mixed/ice, 5 snow, 255 missing)
   /* 20 */ {"ILIQW",   "Integrated liquid water", "kg m^-2"},
   /* 21 */ {"COND",  "Condensate", "kg kg^-1"},
   /* 22 */ {"CLWMR", "Cloud Water Mixing Ratio", "kg kg^-1"},
   /* 23 */ {"ICMR",  "Ice water mixing ratio", "kg kg^-1"},
   /* 24 */ {"RWMR",  "Rain Water Mixing Ratio", "kg kg^-1"},
   /* 25 */ {"SNMR",  "Snow Water Mixing Ratio", "kg kg^-1"},
   /* 26 */ {"MCONV",   "Horizontal moisture convergence", "kg kg^-1 s^-1"},
   /* 27 */ {"MAXRH", "Maximum relative humidity", "%"},
   /* 28 */ {"MAXAH", "Maximum absolute humidity", "kg m^-3"},
   /* 29 */ {"ASNOW", "Total snowfall", "m"},
   /* 30 */ {"PWCAT", "Precipitable water category", "-"},
   /* 31 */ {"HAIL",  "Hail", "m"},
   /* 32 */ {"GRLE", "Graupel (snow pellets)", "kg kg^-1"},
   /* 33 */ {"CRAIN", "Categorical Rain", "categorical"}, //(0 no, 1 yes)
   /* 34 */ {"CFRZR", "Categorical Freezing Rain", "categorical"}, //(0 no, 1 yes)
   /* 35 */ {"CICEP", "Categorical Ice Pellets", "categorical"}, //(0 no, 1 yes)
   /* 36 */ {"CSNOW", "Categorical Snow", "categorical"}, //(0 no, 1 yes)
   /* 37 */ {"CPRAT", "Convective Precipitation Rate", "kg m^-2 s^-1"},
   /* 38 */ {"MCONV", "Horizontal Moisture Divergence", "kg kg^-1 s^-1"},
   /* 39 */ {"CPOFP", "Percent frozen precipitation", "%"},
   /* 40 */ {"PEVAP", "Potential Evaporation", "kg m^-2"},
   /* 41 */ {"PEVPR", "Potential Evaporation Rate", "W m^-2"},
   /* 42 */ {"SNOWC", "Snow Cover", "%"},
   /* 43 */ {"FRAIN", "Rain Fraction of Total Liquid Water", "proportion"},
   /* 44 */ {"RIME",  "Rime Factor", "Numeric"},
   /* 45 */ {"TCOLR", "Total Column Integrated Rain", "kg m^-2"},
   /* 46 */ {"TCOLS", "Total Column Integrated Snow", "kg m^-2"},
   /* 47 */ {"LSWP", "Large Scale Water Precipitation (Non-Convective)", "kg m^-2"},
   /* 48 */ {"CWP", "Convective Water Precipitation", "kg m^-2"},
   /* 49 */ {"TWATP", "Total Water Precipitation", "kg m^-2"},
   /* 50 */ {"TSNOWP", "Total Snow Precipitation", "kg m^-2"},
   /* 51 */ {"TCWAT", "Total Column Water (Vertically integrated total water"
	     " (vapour+cloud water/ice)", "kg m^-2"},
   /* 52 */ {"TPRATE", "Total Precipitation Rate", "kg m^-2 s^-1"},
   /* 53 */ {"TSRWE", "Total Snowfall Rate Water Equivalent", "kg m^-2 s^-1"},
   /* 54 */ {"LSPRATE", "Large Scale Precipitation Rate", "kg m^-2 s^-1"},
   /* 55 */ {"CSRWE", "Convective Snowfall Rate Water Equivalent", "kg m^-2 s^-1"},
   /* 56 */ {"LSSRWE", "Large Scale Snowfall Rate Water Equivalent", "kg m^-2 s^-1"},
   /* 57 */ {"TSRATE", "Total Snowfall Rate", "m s^-1"},
   /* 58 */ {"CSRATE", "Convective Snowfall Rate", "m s^-1"},
   /* 59 */ {"LSSRATE", "Large Scale Snowfall Rate", "m s^-1"},
   /* 60 */ {"SDWE", "Snow Depth Water Equivalent", "kg m^-2"},
   /* 61 */ {"SDEN", "Snow Density", "kg m^-3"},
   /* 62 */ {"SEVAP", "Snow Evaporation", "kg m^-2"},
   /* 63 */ {"reserved", "Reserved", "-"},
   /* 64 */ {"TCIWV", "Total Column Integrated Water Vapour", "kg m^-2"},
   /* 65 */ {"RPRATE", "Rain Precipitation Rate", "kg m^-2 s^-1"},
   /* 66 */ {"SPRATE", "Snow Precipitation Rate", "kg m^-2 s^-1"},
   /* 67 */ {"FPRATE", "Freezing Rain Precipitation Rate", "kg m^-2 s^-1"},
   /* 68 */ {"IPPRATE", "Ice Pellets Precipitation Rate", "kg m^-2 s^-1"},
   /* 69 */ {"TCOLW", "Total Column Integrate Cloud Water", "kg m^-2"},
   /* 70 */ {"TCOLI", "Total Column Integrate Cloud Ice", "kg m^-2"},
   /* 71 */ {"HAILMXR", "Hail Mixing Ratio", "kg kg^-1"},
   /* 72 */ {"TCOLH", "Total Column Integrate Hail", "kg m^-2"},
   /* 73 */ {"HAILPR", "Hail Prepitation Rate", "kg m^-2 s^-1"},
   /* 74 */ {"TCOLG", "Total Column Integrate Graupel", "kg m^-2"},
   /* 75 */ {"GPRATE", "Graupel (Snow Pellets) Prepitation Rate", "kg m^-2 s^-1"},
   /* 76 */ {"CRRATE", "Convective Rain Rate", "kg m^-2 s^-1"},
   /* 77 */ {"LSRRATE", "Large Scale Rain Rate", "kg m^-2 s^-1"},
   /* 78 */ {"TCOLWA", "Total Column Integrate Water", "kg m^-2"},
   /* 79 */ {"EVARATE", "Evaporation Rate", "kg m^-2 s^-1"},
   /* 80 */ {"TOTCON", "Total Condensate", "kg kg^-1"},
   /* 81 */ {"TCICON", "Total Column-Integrate Condensate", "kg m^-2"},
   /* 82 */ {"CIMIXR", "Cloud Ice Mixing Ratio", "kg kg^-1"},
   /* 83 */ {"SCLLWC", "Specific Cloud Liquid Water Content", "kg kg^-1"},
   /* 84 */ {"SCLIWC", "Specific Cloud Ice Water Content", "kg kg^-1"},
   /* 85 */ {"SRAINW", "Specific Rain Water Content", "kg kg^-1"},
   /* 86 */ {"SSNOWW", "Specific Snow Water Content", "kg kg^-1"},
   /* 87 */ {"reserved", "Reserved", "-"},
   /* 88 */ {"reserved", "Reserved", "-"},
   /* 89 */ {"reserved", "Reserved", "-"},
   /* 90 */ {"TKMFLX", "Total Kinematic Moisture Flux", "kg kg^-1 m s^-1"},
   /* 91 */ {"UKMFLX", "U-component (zonal) Kinematic Moisture Flux", "kg kg^-1 m s^-1"},
   /* 92 */ {"VKMFLX", "V-component (meridional) Kinematic Moisture Flux", "kg kg^-1 m s^-1"},
   /* 93 */ {"RHWATER", "Relative Humidity With Respect to Water","%"},
   /* 94 */ {"RHICE", "Relative Humidity With Respect to Ice", "%"},
   /* 95 */ {"FZPRATE", "Freezing or Frozen Precipitation Rate", "kg m^-2 s^-1"},
   /* 96 */ {"MASSDR", "Mass Density of Rain", "kg m^-3"},
   /* 97 */ {"MASSDS", "Mass Density of Snow", "kg m^-3"},
   /* 98 */ {"MASSDG", "Mass Density of Graupel", "kg m^-3"},
   /* 99 */ {"MASSDH", "Mass Density of Hail", "kg m^-3"},
   /* 100 */ {"SPNCR", "Specific Number Concentration of Rain", "kg^-1"},
   /* 101 */ {"SPNCS", "Specific Number Concentration of Snow", "kg^-1"},
   /* 102 */ {"SPNCG", "Specific Number Concentration of Graupel", "kg^-1"},
   /* 103 */ {"SPNCH", "Specific Number Concentration of Hail", "kg^-1"},
   /* 104 */ {"NUMDR", "Number Density of Rain", "m^-3"},
   /* 105 */ {"NUMDS", "Number Density of Snow", "m^-3"},
   /* 106 */ {"NUMDG", "Number Density of Graupel", "m^-3"},
   /* 107 */ {"NUMDH", " Number Density of Hail", "m^-3"},
   /* 108 */ {"SHTPRM", "Specific Humidity Tendency due to Parameterizations", "kg kg^-1 s^-1"},
   /* 109 */ {"MDLWHVA", "Mass Density of Liquid Water Coating on Hail Expressed as Mass of Liquid Water per Unit Volume of Air", "kg m^-3"},
   /* 110 */ {"SMLWHMA", "Specific Mass of Liquid Water Coating on Hail Expressed as Mass of Liquid Water per Unit Mass of Moist Air", "kg kg^-1"},
   /* 111 */ {"MMLWHDA", "Mass Mixing Ratio of Liquid Water Coating on Hail Expressed as Mass of Liquid Water per Unit Mass of Dry Air", "kg kg^-1"},
   /* 112 */ {"MDLWGVA", "Mass Density of Liquid Water Coating on Graupel Expressed as Mass of Liquid Water per Unit Volume of Air", "kg m^-3"},
   /* 113 */ {"SMLWGMA", "Specific Mass of Liquid Water Coating on Graupel Expressed as Mass of Liquid Water per Unit Mass of Moist Air", "kg kg^-1"},
   /* 114 */ {"MMLWGDA", "Mass Mixing Ratio of Liquid Water Coating on Graupel Expressed as Mass of Liquid Water per Unit Mass of Dry Air", "kg kg^-1"},
   /* 115 */ {"MDLWSVA", "Mass Density of Liquid Water Coating on Snow Expressed as Mass of Liquid Water per Unit Volume of Air", "kg m^-3"},
   /* 116 */ {"SMLWSMA", "Specific Mass of Liquid Water Coating on Snow Expressed as Mass of Liquid Water per Unit Mass of Moist Air", "kg kg^-1"},
   /* 117 */ {"MMLWSDA", "Mass Mixing Ratio of Liquid Water Coating on Snow Expressed as Mass of Liquid Water per Unit Mass of Dry Air", "kg kg^-1"},
   /* 118 */ {"UNCSH", "Unbalanced Component of Specific Humidity", "kg kg-1"},
   /* 119 */ {"UCSCLW", "Unbalanced Component of Specific Cloud Liquid Water content", "kg kg-1"},
   /* 120 */ {"UCSCIW", "Unbalanced Component of Specific Cloud Ice Water content", "kg kg-1"},
   /* 121 */ {"FSNOWC", "Fraction of Snow Cover", "Proportion"},

};


/* GRIB2 Code table 4.2 : 0.2 */
/* Meteorological products, momentum category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoMoment[47] = {
   /* 0 */ {"WDIR", "Wind direction (from which blowing)", "deg true"},
   /* 1 */ {"WIND", "Wind speed", "m s^-1"},
   /* 2 */ {"UGRD", "u-component of wind", "m s^-1"},
   /* 3 */ {"VGRD", "v-component of wind", "m s^-1"},
   /* 4 */ {"STRM", "Stream function", "m^2 s^-1"},
   /* 5 */ {"VPOT", "Velocity potential", "m^2 s^-1"},
   /* 6 */ {"MNTSF", "Montgomery stream function", "m^2 s^-2"},
   /* 7 */ {"SGCVV", "Sigma coordinate vertical velocity", "1 s^-1"},
   /* 8 */ {"VVEL", "Vertical velocity (pressure)", "Pa s^-1"},
   /* 9 */ {"DZDT", "Vertical velocity (geometric)", "m s^-1"},
   /* 10 */ {"ABSV", "Absolute vorticity", "1 s^-1"},
   /* 11 */ {"ABSD", "Absolute divergence", "1 s^-1"},
   /* 12 */ {"RELV", "Relative vorticity", "1 s^-1"},
   /* 13 */ {"RELD", "Relative divergence", "1 s^-1"},
   /* 14 */ {"PVORT", "Potential vorticity", "K m^2 kg^-1 s^-1"},
   /* 15 */ {"VUCSH", "Vertical u-component shear", "1 s^-1"},
   /* 16 */ {"VVCSH", "Vertical v-component shear", "1 s^-1"},
   /* 17 */ {"UFLX", "Momentum flux, u component", "N m^-2"},
   /* 18 */ {"VFLX", "Momentum flux, v component", "N m^-2"},
   /* 19 */ {"WMIXE", "Wind mixing energy", "J"},
   /* 20 */ {"BLYDP", "Boundary layer dissipation", "W m^-2"},
   /* 21 */ {"MAXGUST", "Maximum wind speed", "m s^-1"},
   /* 22 */ {"GUST", "Wind speed (gust)", "m s^-1"},
   /* 23 */ {"UGUST", "u-component of wind (gust)", "m s^-1"},
   /* 24 */ {"VGUST", "v-component of wind (gust)", "m s^-1"},
   /* 25 */ {"VWSH", "Vertical speed sheer", "s^-1"},
   /* 26 */ {"MFLX", "Horizontal Momentum Flux", "N m^-2"},
   /* 27 */ {"USTM", "U-Component Storm Motion", "m s^-1"},
   /* 28 */ {"VSTM", "V-Component Storm Motion", "m s^-1"},
   /* 29 */ {"CD", "Drag Coefficient", "numeric"},
   /* 30 */ {"FRICV", "Frictional Velocity", "m s^-1"},
   /* 31 */ {"TDCMOM", "Turbulent Diffusion Coefficient for Momentum", "m^2 s^-1"},
   /* 32 */ {"ETACVV", "Eta Coordinate Vertical Velocity", "s^-1"},
   /* 33 */ {"WINDF", "Wind Fetch", "m"},
   /* 34 */ {"NWIND", "Normal Wind Component", "m s^-1"},
   /* 35 */ {"TWIND", "Tangential Wind Component", "m s^-1"},
   /* 36 */ {"AFRWE", "Amplitude Function for Rossby Wave Envelope for Meridional Wind", "m s^-1"},
   /* 37 */ {"NTSS", "Northward Turbulent Surface Stress", "N m^-2 s"},
   /* 38 */ {"ETSS", "Eastward Turbulent Surface Stress", "N m^-2 s"},
   /* 39 */ {"EWTPARM", "Eastward Wind Tendency Due to Parameterizations", "m s^-2"},
   /* 40 */ {"NWTPARM", "Northward Wind Tendency Due to Parameterizations", "m s^-2"},
   /* 41 */ {"UGWIND", "U-Component of Geostrophic Wind", "m s^-1"},
   /* 42 */ {"VGWIND", "V-Component of Geostrophic Wind", "m s^-1"},
   /* 43 */ {"GEOWD", "Geostrophic Wind Direction", "degree true"},
   /* 44 */ {"GEOWS", "Geostrophic Wind Speed", "m s^-1"},
   /* 45 */ {"UNDIV", "Unbalanced Component of Divergence", "s-1"},
   /* 46 */ {"VORTADV", "Vorticity Advection", "s-2"},
};

/* GRIB2 Code table 4.2 : 0.3 */
/* Meteorological products, mass category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoMass[32] = {
   /* 0 */ {"PRES", "Pressure", "Pa"},
   /* 1 */ {"PRMSL", "Pressure reduced to MSL", "Pa"},
   /* 2 */ {"PTEND", "Pressure tendency", "Pa s^-1"},
   /* 3 */ {"ICAHT", "ICAO Standard Atmosphere Reference Height", "m"},
   /* 4 */ {"GP", "Geopotential", "m^2 s^-2"},
   /* 5 */ {"HGT", "Geopotential height", "gpm"},
   /* 6 */ {"DIST", "Geometric Height", "m"},
   /* 7 */ {"HSTDV", "Standard deviation of height", "m"},
   /* 8 */ {"PRESA", "Pressure anomaly", "Pa"},
   /* 9 */ {"GPA", "Geopotential height anomally", "gpm"},
   /* 10 */ {"DEN", "Density", "kg m^-3"},
   /* 11 */ {"ALTS", "Altimeter setting", "Pa"},
   /* 12 */ {"THICK", "Thickness", "m"},
   /* 13 */ {"PRESALT", "Pressure altitude", "m"},
   /* 14 */ {"DENALT", "Density altitude", "m"},
   /* 15 */ {"5WAVH", "5-Wave Geopotential Height", "gpm"},
   /* 16 */ {"U-GWD", "Zonal Flux of Gravity Wave Stress", "N m^-2"},
   /* 17 */ {"V-GWD", "Meridional Flux of Gravity Wave Stress", "N m^-2"},
   /* 18 */ {"HPBL", "Planetary Boundary Layer Height", "m"},
   /* 19 */ {"5WAVA", "5-Wave Geopotential Height Anomaly", "gpm"},
   /* 20 */ {"SDSGSO", "Standard Deviation Of Sub-Grid Scale Orography", "m"},
   /* 21 */ {"AOSGSO", "Angle Of Sub-Grid Scale Orography", "Rad"},
   /* 22 */ {"SSGSO", "Slope Of Sub-Grid Scale Orography", "numeric"},
   /* 23 */ {"GSGSO", "Gravity Of Sub-Grid Scale Orography", "W m^-2"},
   /* 24 */ {"ASGSO", "Anisotropy Of Sub-Grid Scale Orography", "numeric"},
   /* 25 */ {"NLPRES", "Natural Logarithm of Pressure in Pa", "numeric"},
   /* 26 */ {"EXPRES", "Exner Pressure", "numeric"},
   /* 27 */ {"UMFLX", "Updraught Mass Flux", "kg m^-2 s^-1"},
   /* 28 */ {"DMFLX", "Downdraught Mass Flux", "kg m^-2 s^-1"},
   /* 29 */ {"UDRATE", "Updraught Detrainment Rate", "kg m^-3 s^-1"},
   /* 30 */ {"DDRATE", "Downdraught Detrainment Rate", "kg m^-3 s^-1"},
   /* 31 */ {"UCLSPRS", "Unbalanced Component of Logarithm of Surface Pressure", "-"},
};

/* GRIB2 Code table 4.2 : 0.4 */
/* Meteorological products, short-wave radiation category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoShortRadiate[54] = {
   /* 0 */ {"NSWRS", "Net short-wave radiation flux (surface)", "W m^-2"},
   /* 1 */ {"NSWRT", "Net short-wave radiation flux (top of atmosphere)", "W m^-2"},
   /* 2 */ {"SWAVR", "Short wave radiation flux", "W m^-2"},
   /* 3 */ {"GRAD", "Global radiation flux", "W m^-2"},
   /* 4 */ {"BRTMP", "Brightness temperature", "K"},
   /* 5 */ {"LWRAD", "Radiance (with respect to wave number)", "W m^-1 sr^-1"},
   /* 6 */ {"SWRAD", "Radiance (with respect to wave length)", "W m^-3 sr^-1"},
   /* 7 */ {"DSWRF", "Downward Short-Wave Rad. Flux", "W m^-2"},
   /* 8 */ {"USWRF", "Upward Short-Wave Rad. Flux", "W m^-2"},
   /* 9 */ {"NSWRF", "Net Short Wave Radiation Flux", "W m^-2"},
   /* 10 */ {"PHOTAR", "Photosynthetically Active Radiation", "W m^-2"},
   /* 11 */ {"NSWRFCS", "Net Short-Wave Radiation Flux, Clear Sky", "W m^-2"},
   /* 12 */ {"DWUVR", "Downward UV Radiation", "W m^-2"},
   /* 13 */ {"reserved", "Reserved", "-"},
   /* 14 */ {"reserved", "Reserved", "-"},
   /* 15 */ {"reserved", "Reserved", "-"},
   /* 16 */ {"reserved", "Reserved", "-"},
   /* 17 */ {"reserved", "Reserved", "-"},
   /* 18 */ {"reserved", "Reserved", "-"},
   /* 19 */ {"reserved", "Reserved", "-"},
   /* 20 */ {"reserved", "Reserved", "-"},
   /* 21 */ {"reserved", "Reserved", "-"},
   /* 22 */ {"reserved", "Reserved", "-"},
   /* 23 */ {"reserved", "Reserved", "-"},
   /* 24 */ {"reserved", "Reserved", "-"},
   /* 25 */ {"reserved", "Reserved", "-"},
   /* 26 */ {"reserved", "Reserved", "-"},
   /* 27 */ {"reserved", "Reserved", "-"},
   /* 28 */ {"reserved", "Reserved", "-"},
   /* 29 */ {"reserved", "Reserved", "-"},
   /* 30 */ {"reserved", "Reserved", "-"},
   /* 31 */ {"reserved", "Reserved", "-"},
   /* 32 */ {"reserved", "Reserved", "-"},
   /* 33 */ {"reserved", "Reserved", "-"},
   /* 34 */ {"reserved", "Reserved", "-"},
   /* 35 */ {"reserved", "Reserved", "-"},
   /* 36 */ {"reserved", "Reserved", "-"},
   /* 37 */ {"reserved", "Reserved", "-"},
   /* 38 */ {"reserved", "Reserved", "-"},
   /* 39 */ {"reserved", "Reserved", "-"},
   /* 40 */ {"reserved", "Reserved", "-"},
   /* 41 */ {"reserved", "Reserved", "-"},
   /* 42 */ {"reserved", "Reserved", "-"},
   /* 43 */ {"reserved", "Reserved", "-"},
   /* 44 */ {"reserved", "Reserved", "-"},
   /* 45 */ {"reserved", "Reserved", "-"},
   /* 46 */ {"reserved", "Reserved", "-"},
   /* 47 */ {"reserved", "Reserved", "-"},
   /* 48 */ {"reserved", "Reserved", "-"},
   /* 49 */ {"reserved", "Reserved", "-"},
   /* 50 */ {"UVIUCS", "UV Index (Under Clear Sky)", "numeric"},
   /* 51 */ {"UVI", "UV Index", "J m^-2"},
   /* 52 */ {"DSWRFCS", "Downward Short-Wave Radiation Flux, Clear Sky", "W m^-2"},
   /* 53 */ {"USWRFCS", "Upward Short-Wave Radiation Flux, Clear Sky", "W m^-2"},
};

/* GRIB2 Code table 4.2 : 0.5 */
/* Meteorological products, long-wave radiation category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoLongRadiate[9] = {
   /* 0 */ {"NLWRS", "Net long wave radiation flux (surface)", "W m^-2"},
   /* 1 */ {"NLWRT", "Net long wave radiation flux (top of atmosphere)", "W m^-2"},
   /* 2 */ {"LWAVR", "Long wave radiation flux", "W m^-2"},
   /* 3 */ {"DLWRF", "Downward Long-Wave Rad. Flux", "W m^-2"},
   /* 4 */ {"ULWRF", "Upward Long-Wave Rad. Flux", "W m^-2"},
   /* 5 */ {"NLWRF", "Net Long-Wave Radiation Flux", "W m^-2"},
   /* 6 */ {"NLWRCS", "Net Long-Wave Radiation Flux, Clear Sky", "W m^-2"},
   /* 7 */ {"BRTEMP", "Brightness Temperature", "K"},
   /* 8 */ {"DLWRFCS", "Downward Long-Wave Radiation Flux, Clear Sky", "W m^-2"},
};

/* GRIB2 Code table 4.2 : 0.6 */
/* Meteorological products, cloud category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoCloud[50] = {
   /* 0 */ {"CICE", "Cloud Ice", "kg m^-2"},
   /* 1 */ {"TCDC", "Total cloud cover", "%"},
   /* 2 */ {"CDCON", "Convective cloud cover", "%"},
   /* 3 */ {"LCDC", "Low cloud cover", "%"},
   /* 4 */ {"MCDC", "Medium cloud cover", "%"},
   /* 5 */ {"HCDC", "High cloud cover", "%"},
   /* 6 */ {"CWAT", "Cloud water", "kg m^-2"},
   /* 7 */ {"CDCA", "Cloud amount", "%"},
   /* 8 */ {"CDCT", "Cloud type", "categorical"},  //"(0 clear, 1 Cumulonimbus, 2 Stratus, "
            //"3 Stratocumulus, 4 Cumulus, 5 Altostratus, 6 Nimbostratus, "
            //"7 Altocumulus, 8 Cirrostratus, 9 Cirrocumulus, 10 Cirrus, "
            //"11 Cumulonimbus (fog), 12 Stratus (fog), 13 Stratocumulus (fog),"
            //" 14 Cumulus (fog), 15 Altostratus (fog), 16 Nimbostratus (fog), "
            //"17 Altocumulus (fog), 18 Cirrostratus (fog), "
            //"19 Cirrocumulus (fog), 20 Cirrus (fog), 191 unknown, "
            //"255 missing)"},
   /* 9 */ {"TMAXT", "Thunderstorm maximum tops", "m"},
   /* 10 */ {"THUNC", "Thunderstorm coverage", "categorical"}, //"(0 none, 1 isolated (1%-2%), "
             //"2 few (3%-15%), 3 scattered (16%-45%), 4 numerous (> 45%), "
             //"255 missing)"},
   /* 11 */ {"CDCB", "Cloud base", "m"},
   /* 12 */ {"CDCT", "Cloud top", "m"},
   /* 13 */ {"CEIL", "Ceiling", "m"},
   /* 14 */ {"CDLYR", "Non-Convective Cloud Cover", "%"},
   /* 15 */ {"CWORK", "Cloud Work Function", "J kg^-1"},
   /* 16 */ {"CUEFI", "Convective Cloud Efficiency", "proportion"},
   /* 17 */ {"TCOND", "Total Condensate", "kg kg^-1"},
   /* 18 */ {"TCOLW", "Total Column-Integrated Cloud Water", "kg m^-2"},
   /* 19 */ {"TCOLI", "Total Column-Integrated Cloud Ice", "kg m^-2"},
   /* 20 */ {"TCOLC", "Total Column-Integrated Condensate", "kg m^-2"},
   /* 21 */ {"FICE", "Ice fraction of total condensate", "proportion"},
   /* 22 */ {"CDCC", "Cloud Cover", "%"},
   /* 23 */ {"CDCIMR", "Cloud Ice Mixing Ratio", "kg kg^-1"},
   /* 24 */ {"SUNS", "Sunshine", "numeric"},
   /* 25 */ {"CBHE", "Horizontal Extent of Cumulonimbus (CB)", "%"},
   /* 26 */ {"HCONCB", "Height of Convective Cloud Base", "m"},
   /* 27 */ {"HCONCT", "Height of Convective Cloud Top", "m"},
   /* 28 */ {"NCONCD", "Number Concentration of Cloud Droplets", "kg^-1"},
   /* 29 */ {"NCCICE", "Number Concentration of Cloud Ice", "kg^-1"},
   /* 30 */ {"NDENCD", "Number Density of Cloud Droplets", "m^-3"},
   /* 31 */ {"NDCICE", "Number Density of Cloud Ice", "m^-3"},
   /* 32 */ {"FRACCC", "Fraction of Cloud Cover", "numeric"},
   /* 33 */ {"SUNSD", "Sunshine Duration", "s"},
   /* 34 */ {"SLWTC", "Surface Long Wave Effective Total Cloudiness", "numeric"},
   /* 35 */ {"SSWTC", "Surface Short Wave Effective Total Cloudiness", "numeric"},
   /* 36 */ {"FSTRPC", "Fraction of Stratiform Precipitation Cover", "proportion"},
   /* 37 */ {"FCONPC", "Fraction of Convective Precipitation Cover", "proportion"},
   /* 38 */ {"MASSDCD", "Mass Density of Cloud Droplets", "kg m^-3"},
   /* 39 */ {"MASSDCI", "Mass Density of Cloud Ice", "kg m^-3"},
   /* 40 */ {"MDCCWD", "Mass Density of Convective Cloud Water Droplets", "kg m^-3"},
   /* 41 */ {"reserved", "reserved", "-"},
   /* 42 */ {"reserved", "reserved", "-"},
   /* 43 */ {"reserved", "reserved", "-"},
   /* 44 */ {"reserved", "reserved", "-"},
   /* 45 */ {"reserved", "reserved", "-"},
   /* 46 */ {"reserved", "reserved", "-"},
   /* 47 */ {"VFRCWD", "Volume Fraction of Cloud Water Droplets", "Numeric"},
   /* 48 */ {"VFRCICE", "Volume Fraction of Cloud Ice Particles", "Numeric"},
   /* 49 */ {"VFRCIW", "Volume Fraction of Cloud (Ice and/or Water)", "Numeric"},
};

/* GRIB2 Code table 4.2 : 0.7 */
/* Meteorological products, Thermodynamic Stability category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoStability[20] = {
   /* 0 */ {"PLI", "Parcel lifted index (to 500 hPa)", "K"},
   /* 1 */ {"BLI", "Best lifted index (to 500 hPa)", "K"},
   /* 2 */ {"KX", "K index", "K"},
   /* 3 */ {"KOX", "KO index", "K"},
   /* 4 */ {"TOTALX", "Total totals index", "K"},
   /* 5 */ {"SX", "Sweat index", "numeric"},
   /* 6 */ {"CAPE", "Convective Available Potential Energy", "J kg^-1"},
   /* 7 */ {"CIN", "Convective Inhibition", "J kg^-1"},
   /* 8 */ {"HLCY", "Storm Relative Helicity", "J kg^-1"},
   /* 9 */ {"EHLX", "Energy helicity index", "numeric"},
   /* 10 */ {"LFTX", "Surface Lifted Index", "K"},
   /* 11 */ {"4LFTX", "Best (4 layer) Lifted Index", "K"},
   /* 12 */ {"RI", "Richardson Number", "numeric"},
   /* 13 */ {"SHWINX", "Showalter Index", "K"},
   /* 14 */ {"reserved", "reserved", "-"},
   /* 15 */ {"UPHL", "Updraft Helicity", "m^2 s^-2"},
   /* 16 */ {"BLKRN", "Bulk Richardson Number", "numeric"},
   /* 17 */ {"GRDRN", "Gradient Richardson Number", "numeric"},
   /* 18 */ {"FLXRN", "Flux Richardson Number", "numeric"},
   /* 19 */ {"CONAPES", "Convective Available Potential Energy Shear", "m2 s-2"},
};

/* GRIB2 Code table 4.2 : 0.8 */
/* Meteorological products, Kinematic stability category */

/* GRIB2 Code table 4.2 : 0.9 */
/* Meteorological products, Temperature probabilities category */

/* GRIB2 Code table 4.2 : 0.10 */
/* Meteorological products, Moisture probabilities category */

/* GRIB2 Code table 4.2 : 0.11 */
/* Meteorological products, Momentum probabilities category */

/* GRIB2 Code table 4.2 : 0.12 */
/* Meteorological products, Mass probabilities category */

/* GRIB2 Code table 4.2 : 0.13 */
/* Meteorological products, Aerosols category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoAerosols[1] = {
   /* 0 */ {"AEROT", "Aerosol type", "categorical"}, //"(0 Aerosol not present, 1 Aerosol present, "
            //"255 missing)"}
};

/* GRIB2 Code table 4.2 : 0.14 */
/* Meteorological products, Trace Gases category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoGases[3] = {
  /* 0 */ {"TOZNE", "Total ozone", "Dobson"},
  /* 1 */ {"O3MR", "Ozone Mixing Ratio", "kg kg^-1"},
  /* 2 */ {"TCIOZ", "Total Column Integrated Ozone", "Dobson"},
};

/* GRIB2 Code table 4.2 : 0.15 */
/* Meteorological products, Radar category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoRadar[17] = {
   /* 0 */ {"BSWID", "Base spectrum width", "m s^-1"},
   /* 1 */ {"BREF", "Base reflectivity", "dB"},
   /* 2 */ {"BRVEL", "Base radial velocity", "m s^-1"},
   /* 3 */ {"VERIL", "Vertically-integrated liquid", "kg m^-2"},
   /* 4 */ {"LMAXBR", "Layer-maximum base reflectivity", "dB"},
   /* 5 */ {"PREC", "Precipitation", "kg m^-2"},
   /* 6 */ {"RDSP1", "Radar spectra (1)", "-"},
   /* 7 */ {"RDSP2", "Radar spectra (2)", "-"},
   /* 8 */ {"RDSP3", "Radar spectra (3)", "-"},
   /* 9 */ {"RFCD", "Reflectivity of Cloud Droplets", "dB"},
   /* 10 */ {"RFCI", "Reflectivity of Cloud Ice", "dB"},
   /* 11 */ {"RFSNOW", "Reflectivity of Snow", "dB"},
   /* 12 */ {"RFRAIN", "Reflectivity of Rain", "dB"},
   /* 13 */ {"RFGRPL", "Reflectivity of Graupel", "dB"},
   /* 14 */ {"RFHAIL", "Reflectivity of Hail", "dB"},
   /* 15 */ {"HSR", "Hybrid Scan Reflectivity", "dB"},
   /* 16 */ {"HSRHT", "Hybrid Scan Reflectivity Height", "m"},
};

/* GRIB2 Code table 4.2 : 0.16 */
/* Meteorological products, Forecast Radar Imagery category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoRadarForecast[6] = {
   /* 0 */ {"REFZR", "Equivalent radar reflectivity factor for rain", "m m^6 m^-3"},
   /* 1 */ {"REFZI", "Equivalent radar reflectivity factor for snow", "m m^6 m^-3"},
   /* 2 */ {"REFZC", "Equivalent radar reflectivity factor for parameterized convection", "m m^6 m^-3"},
   /* 3 */ {"RETOP", "Echo Top", "m"},
   /* 4 */ {"REFD", "Reflectivity", "dB"},
   /* 5 */ {"REFC", "Composite reflectivity", "dB"}
};

/* GRIB2 Code table 4.2 : 0.17 */
/* Meteorological products, Electrodynamics category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoElectro[5] = {
  /* 0 */ {"LTNGSD", "Lightning Strike Density", "m^-2 s^-1"},
  /* 1 */ {"LTPINX", "Lightning Potential Index", "J kg^-1"},
  /* 2 */ {"CDGDLTFD", "Cloud-to-Ground Lightning Flash Density", "km-2 day-1"},
  /* 3 */ {"CDCDLTFD", "Cloud-to-Cloud Lightning Flash Density", "km-2 day-1"},
  /* 4 */ {"TLGTFD", "Total Lightning Flash Density (see Note 2)", "km-2 day-1"},
};

/* GRIB2 Code table 4.2 : 0.18 */
/* Meteorological products, Nuclear/Radiology category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoNuclear[19] = {
   /* 0 */ {"ACCES", "Air concentration of Caesium 137", "Bq m^-3"},
   /* 1 */ {"ACIOD", "Air concentration of Iodine 131", "Bq m^-3"},
   /* 2 */ {"ACRADP ", "Air concentration of radioactive pollutant", "Bq m^-3"},
   /* 3 */ {"GDCES", "Ground deposition of Caesium 137", "Bq m^-2"},
   /* 4 */ {"GDIOD", "Ground deposition of Iodine 131", "Bq m^-2"},
   /* 5 */ {"GDRADP", "Ground deposition of radioactive pollutant", "Bq m^-2"},
   /* 6 */ {"TIACCP", "Time-integrated air concentration of caesium pollutant",
            "Bq s m^-3"},
   /* 7 */ {"TIACIP", "Time-integrated air concentration of iodine pollutant",
            "Bq s m^-3"},
   /* 8 */ {"TIACRP", "Time-integrated air concentration of radioactive pollutant",
            "(Bq s m^-3"}, 
   /* 9 */ {"reserved", "reserved", "-"},
   /* 10 */ {"AIRCON", "Air Concentration", "Bq m^-3"},
   /* 11 */ {"WETDEP", "Wet Deposition", "Bq m^-2"},
   /* 12 */ {"DRYDEP", "Dry Deposition", "Bq m^-2"},
   /* 13 */ {"TOTLWD", "Total Deposition (Wet + Dry)", "Bq m^-2"},
   /* 14 */ {"SACON", "Specific Activity Concentration", "Bq kg^-1"},
   /* 15 */ {"MAXACON", "Maximum of Air Concentration in Layer", "Bq m^-3"},
   /* 16 */ {"HMXACON", "Height of Maximum of Air Concentration", "m"},
   /* 17 */ {"CIAIRC", "Column-Integrated Air Concentration", "Bq m-2"},
   /* 18 */ {"CAACL", "Column-Averaged Air Concentration in Layer", "Bq m-3"},
};

/* GRIB2 Code table 4.2 : 0.19 */
/* Meteorological products, Physical Atmospheric category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoAtmos[37] = {
   /* 0 */ {"VIS", "Visibility", "m"},
   /* 1 */ {"ALBDO", "Albedo", "%"},
   /* 2 */ {"TSTM", "Thunderstorm", "%"},
   /* 3 */ {"MIXHT", "Mixed layer depth", "m"},
   /* 4 */ {"VOLASH", "Volcanic ash", "catagorical"},
   /* 5 */ {"ICIT", "Icing top", "m"},
   /* 6 */ {"ICIB", "Icing base", "m"},
   /* 7 */ {"ICI", "Icing", "categorical"},
   /* 8 */ {"TURBT", "Turbulance top", "m"},
   /* 9 */ {"TURBB", "Turbulence base", "m"},
   /* 10 */ {"TURB", "Turbulance", "catagorical"},
   /* 11 */ {"TKE", "Turbulent Kinetic Energy", "J kg^-1"},
   /* 12 */ {"PBLREG", "Planetary boundary layer regime", "catagorical"},
   /* 13 */ {"CONTI", "Contrail intensity", "catagorical"},
   /* 14 */ {"CONTET", "Contrail engine type", "catagorical"},
   /* 15 */ {"CONTT", "Contrail top", "m"},
   /* 16 */ {"CONTB", "Contrail base", "m"},
   /* 17 */ {"MXSALB", "Maximum Snow Albedo", "%"},
   /* 18 */ {"SNFALB", "Snow-Free Albedo", "%"},
   /* 19 */ {"SALBD", "Snow Albedo", "%"},
   /* 20 */ {"ICIP", "Icing", "%"},
   /* 21 */ {"CTP", "In-Cloud Turbulence", "%"},
   /* 22 */ {"CAT", "Clear Air Turbulence", "%"},
   /* 23 */ {"SLDP", "Supercooled Large Droplet Probability", "%"},
   /* 24 */ {"CONTKE", "onvective Turbulent Kinetic Energy", "J kg^-1"},
   /* 25 */ {"WIWW", "Weather", "numeric"},
   /* 26 */ {"CONVO", "Convective Outlook", "numeric"},
   /* 27 */ {"ICESC", "Icing Scenario", "numeric"},
   /* 28 */ {"MWTURB", "Mountain Wave Turbulence", "m^2/3 s^-1"},
   /* 29 */ {"CATEDR", "Clear Air Turbulence", "m^2/3 s^-1"},
   /* 30 */ {"EDPARM", "Eddy Dissipation Parameter", "m^2/3 s^-1"},
   /* 31 */ {"MXEDPRM", "Maximum of Eddy Dissipation Parameter in Layer", "m^2/3 s^-1"},
   /* 32 */ {"HIFREL", "Highest Freezing Level", "m"},
   /* 33 */ {"VISLFOG", "Visibility Through Liquid Fog", "m"},
   /* 34 */ {"VISIFOG", "Visibility Through Ice Fog", "m"},
   /* 35 */ {"VISBSN", "Visibility Through Blowing Snow", "m"},
   /* 36 */ {"ICESEV", "Icing Severity", "non-dim"},
};

/* GRIB2 Code table 4.2 : 0.20 */
/* Meteorological products, Atmospheric Chemical Constituents category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoChem[113] = {
   /* 0 */ {"MASSDEN", "Mass Density (Concentration)", "kg m^-3"},
   /* 1 */ {"COLMD", "Column-Integrated Mass Density", "kg m^-2"},
   /* 2 */ {"MASSMR", "Mass Mixing Ratio (Mass Fraction in Air)", "kg kg^-1"},
   /* 3 */ {"AEMFLX", "Atmosphere Emission Mass Flux", "kg m^-2 s^-1"},
   /* 4 */ {"ANPMFLX", "Atmosphere Net Production Mass Flux", "kg m^-2 s^-1"},
   /* 5 */ {"ANPEMFLX", "Atmosphere Net Production And Emision Mass Flux", "kg m^-2 s^-1"},
   /* 6 */ {"SDDMFLX", "Surface Dry Deposition Mass Flux", "kg m^-2 s^-1"},
   /* 7 */ {"SWDMFLX", "Surface Wet Deposition Mass Flux", "kg m^-2 s^-1"},
   /* 8 */ {"AREMFLX", "Atmosphere Re-Emission Mass Flux", "kg m^-2 s^-1"},
   /* 9 */ {"WLSMFLX", "Wet Deposition by Large-Scale Precipitation Mass Flux", "kg m^-2 s^-1"},
   /* 10 */ {"WDCPMFLX", "Wet Deposition by Convective Precipitation Mass Flux", "kg m^-2 s^-1"},
   /* 11 */ {"SEDMFLX", "Sedimentation Mass Flux", "kg m^-2 s^-1"},
   /* 12 */ {"DDMFLX", "Dry Deposition Mass Flux", "kg m^-2 s^-1"},
   /* 13 */ {"TRANHH", "Transfer From Hydrophobic to Hydrophilic", "kg kg^-1 s^-1"},
   /* 14 */ {"TRSDS", "Transfer From SO2 (Sulphur Dioxide) to SO4 (Sulphate)", "kg kg^-1 s^-1"},
   /* 15 */ {"reserved", "Reserved", "-"},
   /* 16 */ {"reserved", "Reserved", "-"},
   /* 17 */ {"reserved", "Reserved", "-"},
   /* 18 */ {"reserved", "Reserved", "-"},
   /* 19 */ {"reserved", "Reserved", "-"},
   /* 20 */ {"reserved", "Reserved", "-"},
   /* 21 */ {"reserved", "Reserved", "-"},
   /* 22 */ {"reserved", "Reserved", "-"},
   /* 23 */ {"reserved", "Reserved", "-"},
   /* 24 */ {"reserved", "Reserved", "-"},
   /* 25 */ {"reserved", "Reserved", "-"},
   /* 26 */ {"reserved", "Reserved", "-"},
   /* 27 */ {"reserved", "Reserved", "-"},
   /* 28 */ {"reserved", "Reserved", "-"},
   /* 29 */ {"reserved", "Reserved", "-"},
   /* 30 */ {"reserved", "Reserved", "-"},
   /* 31 */ {"reserved", "Reserved", "-"},
   /* 32 */ {"reserved", "Reserved", "-"},
   /* 33 */ {"reserved", "Reserved", "-"},
   /* 34 */ {"reserved", "Reserved", "-"},
   /* 35 */ {"reserved", "Reserved", "-"},
   /* 36 */ {"reserved", "Reserved", "-"},
   /* 37 */ {"reserved", "Reserved", "-"},
   /* 38 */ {"reserved", "Reserved", "-"},
   /* 39 */ {"reserved", "Reserved", "-"},
   /* 40 */ {"reserved", "Reserved", "-"},
   /* 41 */ {"reserved", "Reserved", "-"},
   /* 42 */ {"reserved", "Reserved", "-"},
   /* 43 */ {"reserved", "Reserved", "-"},
   /* 44 */ {"reserved", "Reserved", "-"},
   /* 45 */ {"reserved", "Reserved", "-"},
   /* 46 */ {"reserved", "Reserved", "-"},
   /* 47 */ {"reserved", "Reserved", "-"},
   /* 48 */ {"reserved", "Reserved", "-"},
   /* 49 */ {"reserved", "Reserved", "-"},
   /* 50 */ {"AIA", "Amount in Atmosphere", "mol"},
   /* 51 */ {"CONAIR", "Concentration In Air", "mol m^-3"},
   /* 52 */ {"VMXR", "Volume Mixing Ratio (Fraction in Air)", "mol mol^-1"},
   /* 53 */ {"CGPRC", "Chemical Gross Production Rate of Concentration", "mol m^-3 s^-1"},
   /* 54 */ {"CGDRC", "Chemical Gross Destruction Rate of Concentration", "mol m^-3 s^-1"},
   /* 55 */ {"SFLUX", "Surface Flux", "mol m^-2 s^-1"},
   /* 56 */ {"COAIA", "Changes Of Amount in Atmosphere (See Note 1)", "mol s^-1"},
   /* 57 */ {"TYABA", "Total Yearly Average Burden of The Atmosphere", "mol"},
   /* 58 */ {"TYAAL", "Total Yearly Average Atmospheric Loss (See Note 1)", "mol s^-1"},
   /* 59 */ {"ANCON", "Aerosol Number Concentration", "m^-3"},
   /* 60 */ {"ASNCON", "Aerosol Specific Number Concentration", "kg-1"},
   /* 61 */ {"MXMASSD", "Maximum of Mass Density", "kg m-3"},
   /* 62 */ {"HGTMD", "Height of Mass Density", "m"},
   /* 63 */ {"CAVEMDL", "Column-Averaged Mass Density in Layer", "kg m-3"},
   /* 64 */ {"MOLRDRYA", "Mole fraction with respect to dry air", "mol mol-1"},
   /* 65 */ {"MOLRWETA", "Mole fraction with respect to wet air", "mol mol-1"},
   /* 66 */ {"CINCLDSP", "Column-integrated in-cloud scavenging rate by precipitation", "kg m-2 s-1"},
   /* 67 */ {"CBLCLDSP", "Column-integrated below-cloud scavenging rate by precipitation", "kg m-2 s-1"},
   /* 68 */ {"CIRELREP", "Column-integrated release rate from evaporating precipitation", "kg m-2 s-1"},
   /* 69 */ {"CINCSLSP", "Column-integrated in-cloud scavenging rate by large-scale precipitation", "kg m-2 s-1"},
   /* 70 */ {"CBECSLSP", "Column-integrated below-cloud scavenging rate by large-scale precipitation", "kg m-2 s-1"},
   /* 71 */ {"CRERELSP", "Column-integrated release rate from evaporating large-scale precipitation", "kg m-2 s-1"},
   /* 72 */ {"CINCSRCP", "Column-integrated in-cloud scavenging rate by convective precipitation", "kg m-2 s-1"},
   /* 73 */ {"CBLCSRCP", "Column-integrated below-cloud scavenging rate by convective precipitation", "kg m-2 s-1"},
   /* 74 */ {"CIRERECP", "Column-integrated release rate from evaporating convective precipitation", "kg m-2 s-1"},
   /* 75 */ {"WFIREFLX", "Wildfire flux", "kg m-2 s-1"},
   /* 76 */ {"reserved", "Reserved", "-"},
   /* 77 */ {"reserved", "Reserved", "-"},
   /* 78 */ {"reserved", "Reserved", "-"},
   /* 79 */ {"reserved", "Reserved", "-"},
   /* 80 */ {"reserved", "Reserved", "-"},
   /* 81 */ {"reserved", "Reserved", "-"},
   /* 82 */ {"reserved", "Reserved", "-"},
   /* 83 */ {"reserved", "Reserved", "-"},
   /* 84 */ {"reserved", "Reserved", "-"},
   /* 85 */ {"reserved", "Reserved", "-"},
   /* 86 */ {"reserved", "Reserved", "-"},
   /* 87 */ {"reserved", "Reserved", "-"},
   /* 88 */ {"reserved", "Reserved", "-"},
   /* 89 */ {"reserved", "Reserved", "-"},
   /* 90 */ {"reserved", "Reserved", "-"},
   /* 91 */ {"reserved", "Reserved", "-"},
   /* 92 */ {"reserved", "Reserved", "-"},
   /* 93 */ {"reserved", "Reserved", "-"},
   /* 94 */ {"reserved", "Reserved", "-"},
   /* 95 */ {"reserved", "Reserved", "-"},
   /* 96 */ {"reserved", "Reserved", "-"},
   /* 97 */ {"reserved", "Reserved", "-"},
   /* 98 */ {"reserved", "Reserved", "-"},
   /* 99 */ {"reserved", "Reserved", "-"},
   /* 100 */ {"SADEN", "Surface Area Density (Aerosol)", "m^-1"},
   /* 101 */ {"ATMTK", "Atmosphere Optical Thickness", "m"},
   /* 102 */ {"AOTK", "Aerosol Optical Thickness", "numeric"},
   /* 103 */ {"SSALBK", "Single Scattering Albedo", "numeric"},
   /* 104 */ {"ASYSFK", "Asymmetry Factor", "numeric"},
   /* 105 */ {"AECOEF", "Aerosol Extinction Coefficient", "m^-1"},
   /* 106 */ {"AACOEF", "Aerosol Absorption Coefficient", "m^-1"},
   /* 107 */ {"ALBSAT", "Aerosol Lidar Backscatter from Satellite", "m^-1 sr-1"},
   /* 108 */ {"ALBGRD", "Aerosol Lidar Backscatter from the Ground", "m^-1 sr-1"},
   /* 109 */ {"ALESAT", "Aerosol Lidar Extinction from Satellite", "m^-1"},
   /* 110 */ {"ALEGRD", "Aerosol Lidar Extinction from the Ground", "m^-1"},
   /* 111 */ {"ANGSTEXP", "Angstrom Exponent", "numeric"},
   /* 112 */ {"SCTAOTK", "Scattering Aerosol Optical Thickness", "numeric"},
};

/* GRIB2 Code table 4.2 : 0.190 */
/* Meteorological products, ASCII category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoText[1] = {
   /* 0 */ {"TEXT", "Arbitrary text string", "CCITTIA5"}   /* No Official Abbrev */
};

/* GRIB2 Code table 4.2 : 0.191 */
/* Meteorological products, Miscellaneous category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_meteoMisc[4] = {
  /* 0 */ {"TSEC", "Seconds prior to initial reference time", "s"},
  /* 1 */ {"GEOLAT", "Geographical Latitude", "degrees"},
  /* 2 */ {"GEOLON", "Geographical Longitude", "degrees"},
  /* 3 */ {"DSLOBS", "Days Since Last Observation", "days"},
};

/* GRIB2 Code table 4.2 : 1.0 */
/* Hydrological products, Basic Hydrology category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_hydroBasic[17] = {
   /* 0 */ {"FFLDG", "Flash flood guidance", "kg m^-2"},
   /* 1 */ {"FFLDRO", "Flash flood runoff", "kg m^-2"},
   /* 2 */ {"RSSC", "Remotely sensed snow cover", "categorical"}, //"(50 no-snow/no-cloud, "
            //"100 Clouds, 250 Snow, 255 missing)"},
   /* 3 */ {"ESCT", "Elevation of snow covered terrain", "categorical"}, //"(0-90 elevation in "
            //"increments of 100m, 254 clouds, 255 missing)"},
   /* 4 */ {"SWEPON", "Snow water equivalent percent of normal", "%"},
   /* 5 */ {"BGRUN", "Baseflow-Groundwater Runoff", "kg m^-2"},
   /* 6 */ {"SSRUN", "Storm Surface Runoff", "kg m^-2"},
   /* 7 */ {"DISRS", "Discharge from Rivers or Streams", "m^3 s^-1"},
   /* 8 */ {"GWUPS", "Group Water Upper Storage", "kg m^-2"},
   /* 9 */ {"GWLOWS", "Group Water Lower Storage", "kg m^-2"},
   /* 10 */ {"SFLORC", "Side Flow into River Channel", "m^3 s^-1 m^-1"},
   /* 11 */ {"RVERSW", "River Storage of Water", "m^3"},
   /* 12 */ {"FLDPSW", "Flood Plain Storage of Water", "m^3"},
   /* 13 */ {"DEPWSS", "Depth of Water on Soil Surface", "kg m^-2"},
   /* 14 */ {"UPAPCP", "Upstream Accumulated Precipitation", "kg m^-2"},
   /* 15 */ {"UPASM", "Upstream Accumulated Snow Melt", "kg m^-2"},
   /* 16 */ {"PERRATE", "Percolation Rate", "kg m^-2 s^-1"},
};

/* GRIB2 Code table 4.2 : 1.1 */
/* Hydrological products, Hydrology Inland Water and Sediment Properties category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_hydroProb[3] = {
   /* 0 */ {"CPPOP", "Conditional percent precipitation amount fractile for an "
            "overall period", "kg m^-2"},
   /* 1 */ {"PPOSP", "Percent precipitation in a sub-period of an overall period",
            "%"},
   /* 2 */ {"POP", "Probability of 0.01 inch of precipitation (POP)", "%"},
};

/* GRIB2 Code table 4.2 : 1.2 */
/* Hydrological products, Hydrology Probabilities category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_hydroWaterSediment[14] = {
   /* 0 */ {"WDPTHIL", "Water Depth", "m"},
   /* 1 */ {"WTMPIL", "Water Temperature", "K"},
   /* 2 */ {"WFRACT", "Water Fraction", "proportion"},
   /* 3 */ {"SEDTK", "Sediment Thickness", "m"},
   /* 4 */ {"SEDTMP", "Sediment Temperature", "K"},
   /* 5 */ {"ICTKIL", "Ice Thickness", "m"},
   /* 6 */ {"ICETIL", "Ice Temperature", "K"},
   /* 7 */ {"ICECIL", "Ice Cover", "proportion"},
   /* 8 */ {"LANDIL", "Land Cover (0=water, 1=land)", "proportion"},
   /* 9 */ {"SFSAL", "Shape Factor with Respect to Salinity Profile", "-"},
   /* 10 */ {"SFTMP", "Shape Factor with Respect to Temperature Profile in Thermocline", "-"},
   /* 11 */ {"ACWSR", "Attenuation Coefficient of Water with Respect to Solar Radiation", "m^-1"},
   /* 12 */ {"SALTIL", "Salinity", "kg kg^-1"},
   /* 13 */ {"CSAFC", "Cross Sectional Area of Flow in Channel", "m^2"},
};

/* GRIB2 Code table 4.2 : 2.0 */
/* Land Surface products, Vegetation/Biomass category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_landVeg[39] = {
   /* 0 */ {"LAND", "Land cover (0=sea, 1=land)", "proportion"},
   /* 1 */ {"SFCR", "Surface roughness", "m"},
   /* 2 */ {"TSOIL", "Soil temperature", "K"},
   /* 3 */ {"SOILM", "Soil moisture content", "kg m^-2"},
   /* 4 */ {"VEG", "Vegetation", "%"},
   /* 5 */ {"WATR", "Water runoff", "kg m^-2"},
   /* 6 */ {"EVAPT", "Evapotranspiration", "1 kg^-2 s^-1"},
   /* 7 */ {"MTERH", "Model terrain height", "m"},
   /* 8 */ {"LANDU", "Land use", "categorical"},  //"(1 Urban land, 2 agriculture, 3 Range Land, "
            //"4 Deciduous forest, 5 Coniferous forest, 6 Forest/wetland, "
            //"7 Water, 8 Wetlands, 9 Desert, 10 Tundra, 11 Ice, "
            //"12 Tropical forest, 13 Savannah)"},
   /* 9 */ {"SOILW", "Volumetric Soil Moisture Content", "proportion"},
   /* 10 */ {"GFLUX", "Ground Heat Flux", "W m^-2"},
   /* 11 */ {"MSTAV", "Moisture Availability", "%"},
   /* 12 */ {"SFEXC", "Exchange Coefficient", "kg m^-2 s^-1"},
   /* 13 */ {"CNWAT", "Plant Canopy Surface Water", "kg m^-2"},
   /* 14 */ {"BMIXL", "Blackadar's Mixing Length Scale", "m"},
   /* 15 */ {"CCOND", "Canopy Conductance", "m s^-1"},
   /* 16 */ {"RSMIN", "Minimal Stomatal Resistance", "s m^-1"},
   /* 17 */ {"WILT", "Wilting Point", "proportion"},
   /* 18 */ {"RCS", "Solar parameter in canopy conductance", "proportion"},
   /* 19 */ {"RCT", "Temperature parameter in canopy conductance", "proportion"},
   /* 20 */ {"RCSOL", "Soil moisture parameter in canopy conductance", "proportion"},
   /* 21 */ {"RCQ", "Humidity parameter in canopy conductance", "proportion"},
   /* 22 */ {"SOILM", "Soil Moisture", "kg m^-3"},
   /* 23 */ {"CISOILW", "Column-Integrated Soil Water", "kg m^-2"},
   /* 24 */ {"HFLUX", "Heat Flux", "W m^-2"},
   /* 25 */ {"VSOILM", "Volumetric Soil Moisture", "m^-3 m^-3"},
   /* 26 */ {"WILT", "Wilting Point", "kg m^-3"},
   /* 27 */ {"VWILTM", "Volumetric Wilting Moisture", "m^-3 m^-3"},
   /* 28 */ {"LEAINX", "Leaf Area Index", "numeric"},
   /* 29 */ {"EVGFC", "Evergreen Forest Cover", "proportion"},
   /* 30 */ {"DECFC", "Deciduous Forest Cover", "proportion"},
   /* 31 */ {"NDVINX", "Normalized Differential Vegetation Index (NDVI)", "numeric"},
   /* 32 */ {"RDVEG", "Root Depth of Vegetation", "m"},
   /* 33 */ {"WROD", "Water Runoff and Drainage", "kg m^-2"},
   /* 34 */ {"SFCWRO", "Surface Water Runoff", "kg m^-2"},
   /* 35 */ {"TCLASS", "Tile Class", "numeric"},
   /* 36 */ {"TFRCT", "Tile Fraction", "proportion"},
   /* 37 */ {"TPERCT", "Tile Percentage", "%"},
   /* 38 */ {"SOILVIC", "Soil Volumetric Ice Content", "m3 m^-3"},
};

/* GRIB2 Code table 4.2 : 2.1 */
/* Land Surface products, Agri/aquacultural special  category */

/* GRIB2 Code table 4.2 : 2.2 */
/* Land Surface products, Transportation related category */

/* GRIB2 Code table 4.2 : 2.3 */
/* Land Surface products, Soil category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_landSoil[28] = {
   /* 0 */ {"SOTYP", "Soil type", "categorical"}, //"(1 Sand, 2 Loamy sand, 3 Sandy loam, "
            //"4 Silt loam, 5 Organic (redefined), 6 Sandy clay loam, "
            //"7 Silt clay loam, 8 Clay loam, 9 Sandy clay, 10 Silty clay, "
            //"11 Clay)"},
   /* 1 */ {"UPLST", "Upper layer soil temperature", "K"},
   /* 2 */ {"UPLSM", "Upper layer soil moisture", "kg m^-3"},
   /* 3 */ {"LOWLSM", "Lower layer soil moisture", "kg m^-3"},
   /* 4 */ {"BOTLST", "Bottom layer soil temperature", "K"},
   /* 5 */ {"SOILL", "Liquid Volumetric Soil Moisture (non-frozen)", "proportion"},
   /* 6 */ {"RLYRS", "Number of Soil Layers in Root Zone", "numeric"},
   /* 7 */ {"SMREF", "Transpiration Stress-onset (soil moisture)", "proportion"},
   /* 8 */ {"SMDRY", "Direct Evaporation Cease (soil moisture)", "proportion"},
   /* 9 */ {"POROS", "Soil Porosity", "proportion"},
   /* 10 */ {"LIQVSM", "Liquid Volumetric Soil Moisture (Non-Frozen)", "m^3 m^-3"},
   /* 11 */ {"VOLTSO", "Volumetric Transpiration Stree-Onset(Soil Moisture)", "m^3 m^-3"},
   /* 12 */ {"TRANSO", "Transpiration Stree-Onset(Soil Moisture)", "kg m^-3"},
   /* 13 */ {"VOLDEC", "Volumetric Direct Evaporation Cease(Soil Moisture)", "m^3 m^-3"},
   /* 14 */ {"DIREC", "Direct Evaporation Cease(Soil Moisture)", "kg m^-3"},
   /* 15 */ {"SOILP", "Soil Porosity", "m^3 m^-3"},
   /* 16 */ {"VSOSM", "Volumetric Saturation Of Soil Moisture", "m^3 m^-3"},
   /* 17 */ {"SATOSM", "Saturation Of Soil Moisture", "m^3 m^-3"},
   /* 18 */ {"SOILTMP", "Soil Temperature", "K"},
   /* 19 */ {"SOILMOI", "Soil Moisture", "kg m^-3"},
   /* 20 */ {"CISOILM", "Column-Integrated Soil Moisture", "kg m^-2"},
   /* 21 */ {"SOILICE", "Soil Ice", "kg m^-3"},
   /* 22 */ {"CISICE", "Column-Integrated Soil Ice", "kg m^-2"},
   /* 23 */ {"LWSNOWP", "Liquid Water in Snow Pack", "kg m^-2"},
   /* 24 */ {"FRSTINX", "Frost Index", "kg day^-1"},
   /* 25 */ {"SNWDEB", "Snow Depth at Elevation Bands", "kg m^-2"},
   /* 26 */ {"SHFLX", "Soil Heat Flux", "W m^-2"},
   /* 27 */ {"SOILDEP", "Soil Depth", "m"},
};

/* GRIB2 Code table 4.2 : 2.4 */
/* Land Surface products, Fire Weather category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_landFire[12] = {
  /* 0 */ {"FIREOLK", "Fire Outlook", "numeric"},
  /* 1 */ {"FIREODT", "Fire Outlook Due to Dry Thunderstorm", "numeric"},
  /* 2 */ {"HINDEX", "Haines Index", "numeric"},
  /* 3 */ {"FBAREA", "Fire Burned Area", "%"},
  /* 4 */ {"FOSINDX", "Fosberg Index", "numeric"},
  /* 5 */ {"FWINX", "Fire Weath Index", "numeric"},
  /* 6 */ {"FFMCODE", "Fine Fuel Moisture Code", "numeric"},
  /* 7 */ {"DUFMCODE", "Duff Moisture Code", "numeric"},
  /* 8 */ {"DRTCODE", "Drought Code", "numeric"},
  /* 9 */ {"INFSINX", "Initial Fire Spread Index", "numeric"},
  /* 10 */ {"FBUPINX" "Fire Build Up Index", "numeric"},
  /* 11 */ {"FDSRTE", "Fire Daily Severity Rating", "numeric"},
};

/* GRIB2 Code table 4.2 : 2.5 */
/* Land Surface products, Glaciers and Inland Ice category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_landIce[2] = {
  /* 0 */ {"reserved", "Reserved", "-"},
  /* 1 */ {"GLACTMP", "Glacier Temperature", "K"},
};

/* GRIB2 Code table 4.2 : 3.0 */
/* Space products, Image Format category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_spaceImage[10] = {
   /* 0 */ {"SRAD", "Scaled radiance", "numeric"},
   /* 1 */ {"SALBDO", "Scaled albedo", "numeric"},
   /* 2 */ {"SBRTMP", "Scaled brightness temperature", "numeric"},
   /* 3 */ {"SPWAT", "Scaled precipitable water", "numeric"},
   /* 4 */ {"SLIFTX", "Scaled lifted index", "numeric"},
   /* 5 */ {"SCTOPP", "Scaled cloud top pressure", "numeric"},
   /* 6 */ {"SKINTMP", "Scaled skin temperature", "numeric"},
   /* 7 */ {"CLOUDMASK", "Cloud mask", "categorical"}, //"(0 clear over water, 1 clear over land, "
            //"2 cloud, 3 No data)"},
   /* 8 */ {"PIXST", "Pixel scene type", "index"},
   /* 9 */ {"FIREDI", "Fire Detection Indicator", "index"},
};

/* GRIB2 Code table 4.2 : 3.1 */
/* Space products, Quantitative category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_spaceQuantitative[30] = {
   /* 0 */ {"ESTP", "Estimated precipitation", "kg m^-2"},
   /* 1 */ {"IRRATE", "Instantaneous Rain Rate", "kg m^-2 s^-1"},
   /* 2 */ {"CTOPH", "Cloud Top Height", "m"},
   /* 3 */ {"CTOPHQI", "Cloud Top Height Quality Indicator", "index"},
   /* 4 */ {"ESTUGRD", "Estimated u-Component of Wind", "m^-1"},
   /* 5 */ {"ESTVGRD", "Estimated v-Component of Wind", "m^-1"},
   /* 6 */ {"NPIXU", "Number Of Pixels Used", "numeric"},
   /* 7 */ {"SOLZA", "Solar Zenith Angle", "degrees"},
   /* 8 */ {"RAZA", "Relative Azimuth Angle", "degrees"},
   /* 9 */ {"RFL06", "Reflectance in 0.6 Micron Channel", "%"},
   /* 10 */ {"RFL08", "Reflectance in 0.8 Micron Channel", "%"},
   /* 11 */ {"RFL16", "Reflectance in 1.6 Micron Channel", "%"},
   /* 12 */ {"RFL39", "Reflectance in 3.9 Micron Channel", "%"},
   /* 13 */ {"ATMDIV", "Atmospheric Divergence", "s^-1"},
   /* 14 */ {"CBTMP", "Cloudy Brightness Temperature", "K"},
   /* 15 */ {"CSBTMP", "Clear Sky Brightness Temperature", "K"},
   /* 16 */ {"CLDRAD", "Cloudy Radiance (with respect to wave number)", "W m^-1 sr^-1"},
   /* 17 */ {"CSKYRAD", "Clear Sky Radiance (with respect to wave number)", "W m^-1 sr^-1"},
   /* 18 */ {"reserved", "reserved", "-"},
   /* 19 */ {"WINDS", "Wind Speed", "m s^-1"},
   /* 20 */ {"AOT06", "Aerosol Optical Thickness at 0.635 µm", "-"},
   /* 21 */ {"AOT08", "Aerosol Optical Thickness at 0.810 µm", "-"},
   /* 22 */ {"AOT16", "Aerosol Optical Thickness at 1.640 µm", "-"},
   /* 23 */ {"ANGCOE", "Angstrom Coefficient", "-"},
   /* 24 */ {"reserved", "reserved", "-"},
   /* 25 */ {"reserved", "reserved", "-"},
   /* 26 */ {"reserved", "reserved", "-"},
   /* 27 */ {"BRFLF", "Bidirectional Reflecance Factor", "numeric"},
   /* 28 */ {"SPBRT", "Brightness Temperture", "K"},
   /* 29 */ {"SRAD", "Scaled Radiance", "numeric"},
};

/* GRIB2 Code table 4.2 : 10.0 */
/* Oceanographic products, Waves category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_oceanWaves[46] = {
   /* 0 */ {"WVSP1", "Wave spectra (1)", "-"},
   /* 1 */ {"WVSP2", "Wave spectra (2)", "-"},
   /* 2 */ {"WVSP3", "Wave spectra (3)", "-"},
   /* 3 */ {"HTSGW", "Significant height of combined wind waves and swell", "m"},
   /* 4 */ {"WVDIR", "Direction of wind waves", "degree true"},
   /* 5 */ {"WVHGT", "Significant height of wind waves", "m"},
   /* 6 */ {"WVPER", "Mean period of wind waves", "s"},
   /* 7 */ {"SWDIR", "Direction of swell waves", "degree true"},
   /* 8 */ {"SWELL", "Significant height of swell waves", "m"},
   /* 9 */ {"SWPER", "Mean period of swell waves", "s"},
   /* 10 */ {"DIRPW", "Primary wave direction", "degree true"},
   /* 11 */ {"PERPW", "Primary wave mean period", "s"},
   /* 12 */ {"DIRSW", "Secondary wave direction", "degree true"},
   /* 13 */ {"PERSW", "Secondary wave mean period", "s"},
   /* 14 */ {"WWSDIR", "Direction of Combined Wind Waves and Swell", "degree true"},
   /* 15 */ {"MWSPER", "Mean Period of Combined Wind Waves and Swell", "s"},
   /* 16 */ {"CDWW", "Coefficient of Drag With Waves", "-"},
   /* 17 */ {"FRICV", "Friction Velocity", "m s^-1"},
   /* 18 */ {"WSTR", "Wave Stress", "N m^-2"},
   /* 19 */ {"NWSTR", "Normalised Waves Stress", "-"},
   /* 20 */ {"MSSW", "Mean Square Slope of Waves", "-"},
   /* 21 */ {"USSD", "U-component Surface Stokes Drift", "m s^-1"},
   /* 22 */ {"VSSD", "V-component Surface Stokes Drift", "m s^-1"},
   /* 23 */ {"PMAXWH", "Period of Maximum Individual Wave Height", "s"},
   /* 24 */ {"MAXWH", "Maximum Individual Wave Height", "m"},
   /* 25 */ {"IMWF", "Inverse Mean Wave Frequency", "s"},
   /* 26 */ {"IMFWW", "Inverse Mean Frequency of The Wind Waves", "s"},
   /* 27 */ {"IMFTSW", "Inverse Mean Frequency of The Total Swell", "s"},
   /* 28 */ {"MZWPER", "Mean Zero-Crossing Wave Period", "s"},
   /* 29 */ {"MZPWW", "Mean Zero-Crossing Period of The Wind Waves", "s"},
   /* 30 */ {"MZPTSW", "Mean Zero-Crossing Period of The Total Swell", "s"},
   /* 31 */ {"WDIRW", "Wave Directional Width", "-"},
   /* 32 */ {"DIRWWW", "Directional Width of The Wind Waves", "-"},
   /* 33 */ {"DIRWTS", "Directional Width of The Total Swell", "-"},
   /* 34 */ {"PWPER", "Peak Wave Period", "s"},
   /* 35 */ {"PPERWW", "Peak Period of The Wind Waves", "s"},
   /* 36 */ {"PPERTS", "Peak Period of The Total Swell", "s"},
   /* 37 */ {"ALTWH", "Altimeter Wave Height", "m"},
   /* 38 */ {"ALCWH", "Altimeter Corrected Wave Height", "m"},
   /* 39 */ {"ALRRC", "Altimeter Range Relative Correction", "-"},
   /* 40 */ {"MNWSOW", "10 Metre Neutral Wind Speed Over Waves", "m s^-1"},
   /* 41 */ {"MWDIRW", "10 Metre Wind Direction Over Waves", "degree true"},
   /* 42 */ {"WESP", "Wave Engery Spectrum", "m^-2 s rad^-1"},
   /* 43 */ {"KSSEW", "Kurtosis of The Sea Surface Elevation Due to Waves", "-"},
   /* 44 */ {"BENINX", "Benjamin-Feir Index", "-"},
   /* 45 */ {"SPFTR", "Spectral Peakedness Factor", "s^-1"},
};

/* GRIB2 Code table 4.2 : 10.1 */
/* Oceanographic products, Currents category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_oceanCurrents[4] = {
   /* 0 */ {"DIRC", "Current direction", "degree true"},
   /* 1 */ {"SPC", "Current speed", "m s^-1"},
   /* 2 */ {"UOGRD", "u-component of current", "m s^-1"},
   /* 3 */ {"VOGRD", "v-component of current", "m s^-1"},
};

/* GRIB2 Code table 4.2 : 10.2 */
/* Oceanographic products, Ice category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_oceanIce[13] = {
   /* 0 */ {"ICEC", "Ice cover", "proportion"},
   /* 1 */ {"ICETK", "Ice thinkness", "m"},
   /* 2 */ {"DICED", "Direction of ice drift", "degree true"},
   /* 3 */ {"SICED", "Speed of ice drift", "m s^-1"},
   /* 4 */ {"UICE", "u-component of ice drift", "m s^-1"},
   /* 5 */ {"VICE", "v-component of ice drift", "m s^-1"},
   /* 6 */ {"ICEG", "Ice growth rate", "m s^-1"},
   /* 7 */ {"ICED", "Ice divergence", "s^-1"},
   /* 8 */ { "ICETMP", "ce Temperatur", "K"},
   /* 9 */ { "ICEPRS", "Ice Internal Pressure", "Pa m"},
   /* 10 */ { "ZVCICEP", "Zonal Vector Component of Vertically Integrated Ice Internal Pressure", "Pa m"},
   /* 11 */ { "MVCICEP", "Meridional Vector Component of Vertically Integrated Ice Internal Pressure", "Pa m"},
   /* 12 */ { "CICES", "Compressive Ice Strength", "N m^-1"},
};

/* GRIB2 Code table 4.2 : 10.3 */
/* Oceanographic products, Surface Properties category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_oceanSurface[3] = {
   /* 0 */ {"WTMP", "Water temperature", "K"},
   /* 1 */ {"DSLM", "Deviation of sea level from mean", "m"},
   /* 2 */ {"CH","Heat Exchange Coefficient", "-"},
};

/* GRIB2 Code table 4.2 : 10.4 */
/* Oceanographic products, Sub-Surface Properties category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_oceanSubSurface[16] = {
   /* 0 */ {"MTHD", "Main thermocline depth", "m"},
   /* 1 */ {"MTHA", "Main thermocline anomaly", "m"},
   /* 2 */ {"TTHDP", "Transient thermocline depth", "m"},
   /* 3 */ {"SALTY", "Salinity", "kg kg^-1"},
   /* 4 */ {"OVHD", "Ocean Vertical Heat Diffusivity", "m^2 s^-1"},
   /* 5 */ {"OVSD", "Ocean Vertical Salt Diffusivity", "m^2 s^-1"},
   /* 6 */ {"OVMD", "Ocean Vertical Momentum Diffusivity", "m^2 s^-1"},
   /* 7 */ {"BATHY", "athymetry", "m"},
   /* 8 */ {"reserved", "reserved", "-"},
   /* 9 */ {"reserved", "reserved", "-"},
   /* 10 */ {"reserved", "reserved", "-"},
   /* 11 */ {"SFSALP", "Shape Factor With Respect To Salinity Profile", "-"},
   /* 12 */ {"SFTMPP", "Shape Factor With Respect To Temperature Profile In Thermocline", "-"},
   /* 13 */ {"ACWSRD", "Attenuation Coefficient Of Water With Respect to Solar Radiation", "m^-1"},
   /* 14 */ {"WDEPTH", "Water Depth", "m"},
   /* 15 */ {"WTMPSS", "Water Temperature", "K"},
};

/* GRIB2 Code table 4.2 : 10.191 */
/* Meteorological products, Miscellaneous category */
const ProdDefTemp::_GRIB2ParmTable ProdDefTemp::_oceanMisc[4] = {
  /* 0 */ {"TSEC", "Seconds prior to initial reference time", "s"},
  /* 1 */ {"MOSF", "Meridional Overturning Stream Function", "m^3 s^-1"},
  /* 2 */ {"reserved", "reserved", "-"},
  /* 3 */ {"DSLOBSO", "Days Since Last Observation", "days"},
};

/* NCAR/RAL Local Use Table, Center 60, SubCenter 0? */
/* 
   This is a list of known NCAR/RAL created products that are
   being created and sent via grib2 to other institutions.
   Only create a new variable here if the variable cannot be
   found in any of the standard tables above.
*/
const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_NCAR_RALlocalTable[] = {
  {0, 19, 205, "FLGHT", "Flight Category", "none"},
  {0, 19, 206, "CICEL", "Confidence - Ceiling", "-"},
  {0, 19, 207, "CIVIS", "Confidence - Visibility", "-"},
  {0, 19, 208, "CIFLT", "Confidence - Flight Category", "-"},
  {0, 19, 217, "SLDICE", "Supercooled Large Droplet Icing", "categorical"},
  {0, 19, 219, "TPFI", "Turbulence Potential Forecast Index", "-"},
  {0, 19, 233, "ICPRB", "Icing probability", "-"},
  {0, 19, 234, "ICSEV", "Icing severity", "-"},
};


/* NCEP Local Use Table, Center 7, SubCenter 0 */
/* 
   This list is from NCEPs listing of their local use variables
   at http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_table4-1.shtml
*/
const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_NCEPlocalTable[] = {

  /* 4.2 : 0.0 Meteorological products, temperature category */
  {0, 0, 192, "SNOHF", "Snow Phase Change Heat Flux", "W m^-2"},    /* also 4.2 : 0.0 : 16  */
  {0, 0, 193, "TTRAD", "Temperature tendency by all radiation", "K s^-1"},
  {0, 0, 194, "REV",   "Relative Error Variance", "-"},
  {0, 0, 195, "LRGHR", "Large Scale Condensate Heat rate", "K s^-1"},
  {0, 0, 196, "CNVHR", "Deep Convective Heat rate", "K s^-1"},
  {0, 0, 197, "THFLX", "Total Downward Heat Flux at Surface", "W m^-2"},
  {0, 0, 198, "TTDIA", "Temperature Tendency By All Physics", "K s^-1"},
  {0, 0, 199, "TTPHY", "Temperature Tendency By Non-radiation Physics", "K s^-1"},
  {0, 0, 200, "TSD1D", "Standard Dev. of IR Temp. over 1x1 deg. area", "K"},
  {0, 0, 201, "SHAHR", "Shallow Convective Heating rate", "K s^-1"},
  {0, 0, 202, "VDFHR", "Vertical Diffusion Heating rate", "K s^-1"},
  {0, 0, 203, "THZ0", "Potential temperature at top of viscous sublayer", "K"},
  {0, 0, 204, "TCHP", "Tropical Cyclone Heat Potential", "J m^-2 K^-1"},

  /* 4.2 : 0.1  Meteorological products, moisture category */
  {0, 1, 192, "CRAIN", "Categorical Rain", "categorical"},            /* also 4.2 : 0.1 : 33  */
  {0, 1, 193, "CFRZR", "Categorical Freezing Rain", "categorical"},   /* also 4.2 : 0.1 : 34  */
  {0, 1, 194, "CICEP", "Categorical Ice Pellets", "categorical"},     /* also 4.2 : 0.1 : 35  */
  {0, 1, 195, "CSNOW", "Categorical Snow", "categorical"},            /* also 4.2 : 0.1 : 36  */
  {0, 1, 196, "CPRAT", "Convective Precipitation Rate", "kg s m^-2"},/* also 4.2 : 0.1 : 37  */
  {0, 1, 197, "MCONV", "Horizontal Moisture Divergence", "kg s kg^-1"},  /* also 4.2 : 0.1 : 38  */
  {0, 1, 198, "MINRH", "Minimum Relative Humidity", "%"},
  {0, 1, 199, "PEVAP", "Potential Evaporation", "kg m^-2"},            /* also 4.2 : 0.1 : 40  */
  {0, 1, 200, "PEVPR", "Potential Evaporation Rate", "W m^-2"},        /* also 4.2 : 0.1 : 41  */
  {0, 1, 201, "SNOWC", "Snow Cover", "%"},                              /* also 4.2 : 0.1 : 42  */
  {0, 1, 202, "FRAIN", "Rain Fraction of Total Liquid Water", ""},      /* also 4.2 : 0.1 : 43  */
  {0, 1, 203, "FRIME", "Rime Factor", "-"},                             /* also 4.2 : 0.1 : 44  */
  {0, 1, 204, "TCOLR", "Total Column Integrated Rain", "kg m^-2"},     /* also 4.2 : 0.1 : 45  */
  {0, 1, 205, "TCOLS", "Total Column Integrated Snow", "kg m^-2"},     /* also 4.2 : 0.1 : 46  */
  {0, 1, 206, "TIPD", "Total Icing Potential Diagnostic", "-"},
  {0, 1, 207, "NCIP", "Number concentration for ice particles", "-"},
  {0, 1, 208, "SNOT", "Snow temperature", "K"},
  {0, 1, 209, "TCLSW", "Total column-integrated supercooled liquid water", "kg m^-2"},
  {0, 1, 210, "TCOLM", "Total column-integrated melting ice", "kg m^-2"},
  {0, 1, 211, "EMNP", "Evaporation - Precipitation", "cm day^-1"},
  {0, 1, 212, "SBSNO", "Sublimation (evaporation from snow)", "W m^-2"},
  {0, 1, 213, "CNVMR", "Deep Convective Moistening Rate", "kg kg^-1 s^-1"},
  {0, 1, 214, "SHAMR", "Shallow Convective Moistening Rate", "kg kg^-1 s^-1"},
  {0, 1, 215, "VDFMR", "Vertical Diffusion Moistening Rate", "kg kg^-1 s^-1"},
  {0, 1, 216, "CONDP", "Condensation Pressure of Parcali Lifted From Indicate Surface", "Pa"},
  {0, 1, 217, "LRGMR", "Large scale moistening rate", "kg kg^-1 s^-1"},
  {0, 1, 218, "QZ0", "Specific humidity at top of viscous sublayer", "kg kg^-1"},
  {0, 1, 219, "QMAX", "Maximum specific humidity at 2m", "kg kg^-1"},
  {0, 1, 220, "QMIN", "Minimum specific humidity at 2m", "kg kg^-1"},
  {0, 1, 221, "ARAIN", "Liquid precipitation (rainfall)", "kg m^-2"},
  {0, 1, 222, "SNOWT", "Snow temperature, depth-avg", "K"},
  {0, 1, 223, "APCPN", "Total precipitation (nearest grid point)", "kg m^-2"},
  {0, 1, 224, "ACPCPN", "Convective precipitation (nearest grid point)", "kg m^-2"},
  {0, 1, 225, "FRZR", "Freezing Rain", "kg m^-2"},
  {0, 1, 226, "PWTHER", "Predominant Weather", "numeric"},
  {0, 1, 227, "FROZR", "Frozen Rain", "kg m^-2"},
  {0, 1, 241, "TSNOW", "Total Snow", "kg m^-2"},
  {0, 1, 242, "RHPW", "Relative Humidity with Respect to Precipitable Water", "%"},

  /* 4.2 : 0.2 Meteorological products, momentum category */
  {0, 2, 192, "VWSH", "Vertical speed sheer", "1 s^-1"},             /* also 4.2 : 0.2 : 25  */
  {0, 2, 193, "MFLX", "Horizontal Momentum Flux", "N m^-2"},     /* also 4.2 : 0.2 : 26  */
  {0, 2, 194, "USTM", "U-Component Storm Motion", "m s^-1"},         /* also 4.2 : 0.2 : 27  */
  {0, 2, 195, "VSTM", "V-Component Storm Motion", "m s^-1"},         /* also 4.2 : 0.2 : 28  */
  {0, 2, 196, "CD", "Drag Coefficient", "-"},                     /* also 4.2 : 0.2 : 29  */
  {0, 2, 197, "FRICV", "Frictional Velocity", "m s^-1"},             /* also 4.2 : 0.2 : 30  */
  {0, 2, 198, "LAUV", "Latitude of U Wind Component of Velocity", "degrees"},
  {0, 2, 199, "LOUV", "Latitude of U Wind Component of Velocity", "degrees"},
  {0, 2, 200, "LAVV", "Latitude of V Wind Component of Velocity", "degrees"},
  {0, 2, 201, "LOUV", "Longitude of V Wind Component of Velocity", "degrees"},
  {0, 2, 202, "LAPP", "Latitude of Presure Point", "degrees"},
  {0, 2, 203, "LOPP", "Longitude of Presure Point", "degrees"},
  {0, 2, 204, "VEDH", "Vertical Eddy Diffusivity Heat exchange", "m^2 s^-1"},
  {0, 2, 205, "COVMZ", "Covariance between Meridional and Zonal Components of the wind","m^2 s^-2"},
  {0, 2, 206, "COVTZ", "Covariance between Temperature and Zonal Components of the wind", "K m s^-1"},
  {0, 2, 207, "COVTM", "Covariance between Temperature and Meridional Components of the wind", "K m s^-1"},
  {0, 2, 208, "VDFUA", "Vertical Diffusion Zonal Acceleration", "m s^-2"},
  {0, 2, 209, "VDFVA", "Vertical Diffusion Meridional Acceleration", "m s^-2"},
  {0, 2, 210, "GWDU", "Gravity wave drag zonal acceleration", "m s^-2"},
  {0, 2, 211, "GWDV", "Gravity wave drag meridional acceleration", "m s^-2"},
  {0, 2, 212, "CNVU", "Convective zonal momentum mixing acceleration", "m s^-2"},
  {0, 2, 213, "CNVV", "Convective meridional momentum mixing acceleration", "m s^-2"},
  {0, 2, 214, "WTEND", "Tendency of vertical velocity", "m s^-2"},
  {0, 2, 215, "OMGALF", "Omega (Dp/Dt) divide by density", "K"},
  {0, 2, 216, "CNGWDU", "Convective Gravity wave drag zonal acceleration", "m s^-2"},
  {0, 2, 217, "CNGWDV", "Convective Gravity wave drag meridional acceleration", "m s^-2"},
  {0, 2, 218, "LMV", "Velocity Point Model Surface", "-"},
  {0, 2, 219, "PVMWW", "Potential Vorticity (Mass-Weighted)", "1 s^-1 m^-1"},
  {0, 2, 220, "MAXUVV", "Hourly Maximum of Upward Vertical Velocity in the lowest 400hPa", "m s^-1"},
  {0, 2, 221, "MAXDVV", "Hourly Maximum of Downward Vertical Velocity in the lowest 400hPa", "m s^-1"},
  {0, 2, 222, "MAXUW", "U Component of Hourly Maximum 10m Wind Speed", "m s^-1"},
  {0, 2, 223, "MAXVW", "V Component of Hourly Maximum 10m Wind Speed", "m s^-1"},
  {0, 2, 224, "VRATE", "Ventilation Rate", "m^2 s^-1"},

  {0, 2, 225, "TRWSPD", "Transport Wind Speed", "m s-1       "},
  {0, 2, 226, "TRWDIR", "Transport Wind Direction", "Deg     "},
  {0, 2, 227, "TOA10", "Earliest Reasonable Arrival Time (10% exceedance)", "s      "},
  {0, 2, 228, "TOA50", "Most Likely Arrival Time (50% exceedance)", "s      "},
  {0, 2, 229, "TOD50", "Most Likely Departure Time (50% exceedance)", "s    "},
  {0, 2, 230, "TOD90", "Latest Reasonable Departure Time (90% exceedance)", "s      "},
  {0, 2, 231, "TPWDIR", "Tropical Wind Direction", "°"},
  {0, 2, 232, "TPWSPD", "Tropical Wind Speed", "m s-1"},


  /* 4.2 : 0.3 Meteorological products, mass category */
  {0, 3, 192, "MSLET", "Mean Sea Level Pressure (Eta Reduction)", "Pa"},
  {0, 3, 193, "5WAVH", "5-Wave Geopotential Height", "gpm"},                 /* also 4.2 : 0.3 : 15  */
  {0, 3, 194, "U-GWD", "Zonal Flux of Gravity Wave Stress", "N m^-2"},      /* also 4.2 : 0.3 : 16  */
  {0, 3, 195, "V-GWD", "Meridional Flux of Gravity Wave Stress", "N m^-2"}, /* also 4.2 : 0.3 : 17  */
  {0, 3, 196, "HPBL", "Planetary Boundary Layer Height", "m"},               /* also 4.2 : 0.3 : 18  */
  {0, 3, 197, "5WAVA", "5-Wave Geopotential Height Anomaly", "gpm"},         /* also 4.2 : 0.3 : 19  */
  {0, 3, 198, "MSLMA", "MSLP (MAPS System Reduction)", "Pa"},
  {0, 3, 199, "TSLSA", "3-hr pressure tendency (Std. Atmos. Reduction)", "Pa s^-1"},
  {0, 3, 200, "PLPL", "Pressure of level from which parcel was lifted", "Pa"},
  {0, 3, 201, "LPSX", "X-gradient of Log Pressure", "1 m^-1"},
  {0, 3, 202, "LPSY", "Y-gradient of Log Pressure", "1 m^-1"},
  {0, 3, 203, "HGTX", "X-gradient of Height", "1 m^-1"},
  {0, 3, 204, "HGTY", "Y-gradient of Height", "1 m^-1"},
  {0, 3, 205, "LAYTH", "Layer Thickness", "m"},
  {0, 3, 206, "NLGSP", "Natural Log of Surface Pressure", "ln(kPa)"},
  {0, 3, 207, "CNVUMF", "Convective updraft mass flux", "kg m^-2 s^-1"},
  {0, 3, 208, "CNVDMF", "Convective downdraft mass flux", "kg m^-2 s^-1"},
  {0, 3, 209, "CNVDEMF", "Convective detrainment mass flux", "kg m^-2 s^-1"},
  {0, 3, 210, "LMH", "Mass Point Model Surface", "-"},
  {0, 3, 211, "HGTN", "Geopotential Height (nearest grid point)", "gpm"},
  {0, 3, 212, "PRESN", "Pressure (nearest grid point)", "Pa"},
  {0, 3, 213, "ORCONV", "Orographic Convexity", "-"},
  {0, 3, 214, "ORASW", "Orographic Asymmetry, W Component", "-"},
  {0, 3, 215, "ORASS", "Orographic Asymmetry, S Component", "-"},
  {0, 3, 216, "ORASSW", "Orographic Asymmetry, SW Component", "-"},
  {0, 3, 217, "ORASNW", "Orographic Asymmetry, NW Component", "-"},	
  {0, 3, 218, "ORLSW", "Orographic Length Scale, W Component", "-"},	
  {0, 3, 219, "ORLSS", "Orographic Length Scale, S Component", "-"},
  {0, 3, 220, "ORLSSW", "Orographic Length Scale, SW Component", "-"},
  {0, 3, 221, "ORLSNW", "Orographic Length Scale, NW Component", "-"},

  /* 4.2 : 0.4 Meteorological products, short-wave radiation category */
  {0, 4, 192, "DSWRF", "Downward Short-Wave Rad. Flux", "W m^-2"},  /* also 4.2 : 0.4 : 7  */
  {0, 4, 193, "USWRF", "Upward Short-Wave Rad. Flux", "W m^-2"},    /* also 4.2 : 0.4 : 8  */
  {0, 4, 194, "DUVB",  "UV-B downward solar flux", "W m^-2"},
  {0, 4, 195, "CDUVB", "Clear sky UV-B downward solar flu", "W m^-2"},
  {0, 4, 196, "CSDSF", "Clear Sky Downward Solar Flux", "W m^-2"},
  {0, 4, 197, "SWHR", "Solar Radiative Heating Rate", "K s^-1"},
  {0, 4, 198, "CSUSF", "Clear Sky Upward Solar Flux", "W m^-2"},
  {0, 4, 199, "CFNSF", "Cloud Forcing Net Solar Flux", "W m^-2"},
  {0, 4, 200, "VBDSF", "Visible Beam Downward Solar Flux", "W m^-2"},
  {0, 4, 201, "VDDSF", "Visible Diffuse Downward Solar Flux", "W m^-2"},
  {0, 4, 202, "NBDSF", "Near IR Beam Downward Solar Flux", "W m^-2"},
  {0, 4, 203, "NDDSF", "Near IR Diffuse Downward Solar Flux", "W m^-2"},
  {0, 4, 204, "DTRF", "Downward Total radiation Flux", "W m^-2"},
  {0, 4, 205, "UTRF", "Upward Total radiation Flux", "W m^-2"},

  /* 4.2 : 0.5 Meteorological products, long-wave radiation category */
  {0, 5, 192, "DLWRF", "Downward Long-Wave Rad. Flux", "W m^-2"},  /* also 4.2 : 0.5 : 3  */
  {0, 5, 193, "ULWRF", "Upward Long-Wave Rad. Flux", "W m^-2"},    /* also 4.2 : 0.5 : 4  */
  {0, 5, 194, "LWHRF", "Long-Wave Radiative Heating Rate", "W m^-2"},
  {0, 5, 195, "CSULF", "Clear Sky Upward Long Wave Flux", "W m^-2"},
  {0, 5, 196, "CSDLF", "Clear Sky Downward Long Wave Flux", "W m^-2"},
  {0, 5, 197, "CFNLF", "Cloud Forcing Net Long Wave Flux", "W m^-2"},

  /* 4.2 : 0.6 Meteorological products, cloud category */
  {0, 6, 192, "CDLYR", "Non-Convective Cloud Cover", "%"},
  {0, 6, 193, "CWORK", "Cloud Work Function", "J/kg"},
  {0, 6, 194, "CUEFI", "Convective Cloud Efficiency", "-"},
  {0, 6, 195, "TCOND", "Total Condensate", "kg kg^-1"},
  {0, 6, 196, "TCOLW", "Total Column-Integrated Cloud Water", "kg m^-2"},
  {0, 6, 197, "TCOLI", "Total Column-Integrated Cloud Ice", "kg m^-2"},
  {0, 6, 198, "TCOLC", "Total Column-Integrated Condensate", "kg m^-2"},
  {0, 6, 199, "FICE", "Ice fraction of total condensate", "-"},
  {0, 6, 200, "MFLUX", "Convective Cloud Mass Flux", "Pa s^-1"},
  {0, 6, 201, "SUNSD", "Sunshine Duration", "s"},

  /* 4.2 : 0.7 Meteorological products, Thermodynamic Stability category */
  {0, 7, 192, "LFTX", "Surface Lifted Index", "K"},         /* also 4.2 : 0.7 : 10  */
  {0, 7, 193, "4LFTX", "Best (4 layer) Lifted Index", "K"}, /* also 4.2 : 0.7 : 11  */
  {0, 7, 194, "RI", "Richardson Number", "-"},              /* also 4.2 : 0.7 : 12  */
  {0, 7, 195, "CWDI", "Convective Weather Detection Index", "-"},
  {0, 7, 196, "UVI", "Ultra Violet Index", "J m^-2"},
  {0, 7, 197, "UPHL", "Updraft Helicity", "m^2 s^-2"},
  {0, 7, 198, "LAI", "Leaf Area Index", "-"},
  {0, 7, 199, "MXUPHL", "Hourly Maximum of Updraft Helicity over Layer 2km to 5 km AGL", "m^2 s^-2"},
  {0, 7, 200, "MNUPHL", "Hourly Minimum of Updraft Helicity", "m2 s-2"},
  {0, 7, 201, "BNEGELAY", "Bourgoiun Negative Energy Layer (surface to freezing level)", "J kg-1"},
  {0, 7, 202, "BPOSELAY", "Bourgoiun Positive Energy Layer (2k ft AGL to 400 hPa)", "J kg-1"},
  {0, 7, 203, "DCAPE", "Downdraft CAPE", "J kg-1"},
  {0, 7, 204, "EFHL", "Effective Storm Relative Helicity", "m2 s-2"},
  {0, 7, 205, "ESP", "Enhanced Stretching Potential", "Numeric"},
  {0, 7, 206, "CANGLE", "Critical Angle", "Degree"},

  /* 4.2 : 0.13 Meteorological products, Aerosols category */
  {0, 13, 192, "PMTC", "Particulate matter (coarse)", "µg m^-3"},
  {0, 13, 193, "PMTF", "Particulate matter (fine)", "µg m^-3"},
  {0, 13, 194, "LPMTF", "Particulate matter (fine)", "log10(µg m^-3)"},
  {0, 13, 195, "LIPMF", "Integrated column particulate matter (fine)", "log10(µg m^-3)"},

  /* 4.2 : 0.14 Meteorological products, Trace Gases category */
  {0, 14, 192, "O3MR", "Ozone Mixing Ratio", "kg kg^-1"},  /* also 4.2 : 0.14 : 1  */
  {0, 14, 193, "OZCON", "Ozone Concentration", "PPB"},
  {0, 14, 194, "OZCAT", "Categorical Ozone Concentration", "unknown"},
  {0, 14, 195, "VDFOZ", "Ozone vertical diffusion", "kg kg^-1 s^-1"},
  {0, 14, 196, "POZ", "Ozone production", "kg kg^-1 s^-1"},
  {0, 14, 197, "TOZ", "Ozone tendency", "kg kg^-1 s^-1"},
  {0, 14, 198, "POZT", "Ozone production from temperature term", "kg kg^-1 s^-1"},
  {0, 14, 199, "POZO", "Ozone production from col ozone term", "kg kg^-1 s^-1"},
  {0, 14, 200, "OZMAX1", "Ozone Daily Max from 1-hour Average", "ppbV"},
  {0, 14, 201, "OZMAX8", "Ozone Daily Max from 8-hour Average", "ppbV"},
  {0, 14, 202, "PDMAX1", "PM 2.5 Daily Max from 1-hour Average", "ug m^-3"},
  {0, 14, 203, "PDMAX24", "PM 2.5 Daily Max from 24-hour Average", "ug m^-3"},

  /* 4.2 : 0.16 Meteorological products, Forecast Radar Imagery category */
  {0, 16, 192, "REFZR", "Equivalent radar reflectivity factor for rain", "m m^6 m^-3"},
  {0, 16, 193, "REFZI", "Equivalent radar reflectivity factor for snow", "m m^6 m^-3"},
  {0, 16, 194, "REFZC", "Equivalent radar reflectivity factor for parameterized convection", "m m^6 m^-3"},
  {0, 16, 195, "REFD", "Reflectivity", "dB"},
  {0, 16, 196, "REFC", "Composite radar reflectivity", "dB"},
  {0, 16, 197, "RETOP", "Echo Top", "m"},
  {0, 16, 198, "MAXREF", "Hourly Maximum of Simulated Reflectivity at 1 km AGL", "dB"},

  /* 4.2 : 0.17 Meteorological products, Electrodynamics category */
  {0, 17, 192, "LTNG", "Lightning", "-"},

  /* 4.2 : 0.19 Meteorological products, Physical Atmospheric category */
  {0, 19, 192, "MXSALB", "Maximum Snow Albedo", "%"},       /* also 4.2 : 0.19 : 17  */
  {0, 19, 193, "SNFALB", "Snow-Free Albedo", "%"},          /* also 4.2 : 0.19 : 18  */
  {0, 19, 194, "SlightRisk", "Slight risk convective outlook", "categorical"},
  {0, 19, 195, "ModerateRisk", "Moderate risk convective outlook", "categorical"},
  {0, 19, 196, "HighRisk", "High risk convective outlook", "categorical"},
  {0, 19, 197, "TornadoProb", "Tornado probability", "%"},
  {0, 19, 198, "HailProb", "Hail probability", "%"},
  {0, 19, 199, "WindProb", "Wind probability", "%"},
  {0, 19, 200, "SigTornadoProb", "Significant Tornado probability", "%"},
  {0, 19, 201, "SigHailProb", "Significant Hail probability", "%"},
  {0, 19, 202, "SigWindProb", "Significant Wind probability", "%"},
  {0, 19, 203, "CatThunder", "Categorical Thunderstorm", "categorical"},
  {0, 19, 204, "MIXLY", "Number of mixed layers next to surface", "integer"},
  {0, 19, 205, "FLGHT", "Flight Category", "none"},
  {0, 19, 206, "CICEL", "Confidence - Ceiling", "-"},
  {0, 19, 207, "CIVIS", "Confidence - Visibility", "-"},
  {0, 19, 208, "CIFLT", "Confidence - Flight Category", "-"},
  {0, 19, 209, "LAVNI", "Low-Level aviation interest", "-"},
  {0, 19, 210, "HAVNI", "High-Level aviation interest", "-"},
  {0, 19, 211, "SBSALB", "Visible, Black Sky Albedo", "%"},
  {0, 19, 212, "SWSALB", "Visible, White Sky Albedo", "%"},
  {0, 19, 213, "NBSALB", "Near IR, Black Sky Albedo", "%"},
  {0, 19, 214, "NWSALB", "Near IR, White Sky Albedo", "%"},
  {0, 19, 215, "PRSVR", "Total Probability of Severe Thunderstorms (Days 2,3)", "%"},
  {0, 19, 216, "PRSIGSVR", "Total Probability of Extreme Severe Thunderstorms (Days 2,3)", "%"},
  {0, 19, 217, "SLDICE", "Supercooled Large Droplet Icing", "categorical"},
  {0, 19, 218, "EPSR", "Radiative emissivity", "-"},
  {0, 19, 219, "TPFI", "Turbulence Potential Forecast Index", "-"},
  {0, 19, 220, "SVRTS", "Categorical Severe Thunderstorm", "categorical"},
  {0, 19, 221, "PROCON", "Probability of Convection", "%"},
  {0, 19, 222, "CONVP", "Convection Potential", "categorical"},
  /*   These have been taken out of the NCEP local tables
  {0, 19, 217, "MEIP", "Mean Icing Potential", "-"},
  {0, 19, 218, "MAIP", "Maximum Icing Potential", "-"},
  {0, 19, 219, "MECTP", "Mean in-Cloud Turbulence Potential", "-"},
  {0, 19, 220, "MACTP", "Max in-Cloud Turbulence Potential", "-"},
  {0, 19, 221, "MECAT", "Mean Cloud Air Turbulence Potentia", "-"},
  {0, 19, 222, "MACAT", "Maximum Cloud Air Turbulence Potentia", "-"},
  {0, 19, 223, "CBHE", "Cumulonimbus Horizontal Extent", "%"},
  {0, 19, 224, "PCBB", "Pressure at Cumulonimbus Base", "Pa"},
  {0, 19, 225, "PCBT", "Pressure at Cumulonimbus Top", "Pa"},
  {0, 19, 226, "PECBB", "Pressure at Embedded Cumulonimbus Base", "Pa"},
  {0, 19, 227, "PECBT", "Pressure at Embedded Cumulonimbus Top", "Pa"},
  {0, 19, 228, "HCBB", "ICAO Height at Cumulonimbus Base", "m"},
  {0, 19, 229, "HCBT", "ICAO Height at Cumulonimbus Top", "m"},
  {0, 19, 230, "HECBB", "ICAO Height at Embedded Cumulonimbus Base", "m"},
  {0, 19, 231, "HECBT", "ICAO Height at Embedded Cumulonimbus Top", "m"},
  */
  {0, 19, 232, "VAFTD", "Volcanic Ash Forecast Transport and Dispersion", "log10(kg m^-3)"},
  {0, 19, 233, "ICPRB", "Icing probability", "-"},
  {0, 19, 234, "ICSEV", "Icing severity", "-"},
  {0, 19, 235, "JFWPRB", "Joint Fire Weather Probability", "%"},
  {0, 19, 236, "SNOWLVL", "Snow Level", "m"},
  {0, 19, 237, "DRYTPROB", "Dry Thunderstorm Probability", "%"},
  {0, 19, 238, "ELLINX", "Ellrod Index", "-"},

  /* 4.2 : 0.191 Meteorological products, Miscellaneous category */
  {0, 191, 192, "NLAT", "Latitude (-90 to 90)", "degrees"},
  {0, 191, 193, "ELON", "East Longitude (0 to 360)", "degrees"},
  {0, 191, 194, "TSEC", "Seconds prior to initial reference time", "sec"},  /* also 4.2 : 0.191 : 1  */
  {0, 191, 195, "MLYNO", "Model Layer number (From bottom up)", "-"},
  {0, 191, 196, "NLATN", "Latitude (nearest neighbor) (-90 to +90)", "degrees"},
  {0, 191, 197, "ELONN", "East Longitude (nearest neighbor) (0 - 360)", "degrees"},

  /* 4.2 : 0.192 Meteorological products, Covariance category */
  {0, 192, 1, "COVZM", "Covariance between zonal and meridional components of the wind. Defined as [uv]-[u][v]", "m^2 s^-2"},
  {0, 192, 2, "COVTZ", "Covariance between izonal component of the wind and temperature. Defined as [uT]-[u][T]", "K m s^-1"},
  {0, 192, 3, "COVTM", "Covariance between meridional component of the wind and temperature. Defined as [vT]-[v][T]", "K m s^-1"},
  {0, 192, 4, "COVTW", "Covariance between temperature and vertical component of the wind. Defined as [wT]-[w][T]", "K m s^-1"},
  {0, 192, 5, "COVZZ", "Covariance between zonal and zonal components of the wind. Defined as [uu]-[u][u]", "m^2 s^-2"},
  {0, 192, 6, "COVMM", "Covariance between meridional and meridional components of the wind. Defined as [vv]-[v][v]", "m^2 s^-2"},
  {0, 192, 7, "COVQZ", "Covariance between specific humidity and zonal components of the wind. Defined as [uq]-[u][q]", "kg kg^-1 m s^-1"},
  {0, 192, 8, "COVQM", "Covariance between specific humidity and meridional components of the wind. Defined as [vq]-[v][q]", "kg kg^-1 m s^-1"},
  {0, 192, 9, "COVTVV", "Covariance between temperature and vertical components of the wind. Defined as [\u03a9T]-[\u03a9][T]", "K Pa s^-1"},
  {0, 192, 10, "COVQVV", "ovariance between specific humidity and vertical components of the wind. Defined as [\u03a9q]-[\u03a9][q]", "kg kg^-1 Pa s^-1"},
  {0, 192, 11, "COVPSPS", "Covariance between surface pressure and surface pressure. Defined as [Psfc]-[Psfc][Psfc]", "Pa Pa"},
  {0, 192, 12, "COVQQ", "Covariance between specific humidity and specific humidy. Defined as [qq]-[q][q]", "kg kg^-1 kg kg^-1"},
  {0, 192, 13, "COVVVVV", "Covariance between vertical and vertical components of the wind. Defined as [\u03a9\u03a9]-[\u03a9][\u03a9]", "Pa^2 s^-2"},
  {0, 192, 14, "COVTT", "Covariance between temperature and temperature. Defined as [TT]-[T][T]", "K K"},

  /* 4.2 : 1.0 Hydrological products, Basic Hydrology category */
  {1, 0, 192, "BGRUN", "Baseflow-Groundwater Runoff", "kg m^-2"}, /* also 4.2 : 1.0 : 5  */
  {1, 0, 193, "SSRUN", "Storm Surface Runoff", "kg m^-2"},        /* also 4.2 : 1.0 : 6  */

  /* 4.2 : 1.1 Hydrological products, Hydrology Probabilities category */
  {1, 1, 192, "PropFreezPre", "Probability of Freezing Precipitation", "%"},
  {1, 1, 193, "PropFrozenPre", "Probability of Frozen Precipitation", "%"},	
  {1, 1, 194, "PPFFG", "Probability of precipitation exceeding flash flood guidance values", "%"},
  {1, 1, 195, "CWR", "Probability of Wetting Rain, exceeding in 0.10 in a given time period", "%"},	

  /* 4.2 : 2.0 Land Surface products, Vegetation/Biomass category */
  {2, 0, 192, "SOILW", "Volumetric Soil Moisture Content", "fraction"},             /* also 4.2 : 2.0 : 9  */
  {2, 0, 193, "GFLUX", "Ground Heat Flux", "W m^-2"},                              /* also 4.2 : 2.0 : 10  */
  {2, 0, 194, "MSTAV", "Moisture Availability", "%"},                               /* also 4.2 : 2.0 : 11  */
  {2, 0, 195, "SFEXC", "Exchange Coefficient", "kg m^-3 m s^-1"},                  /* also 4.2 : 2.0 : 12  */
  {2, 0, 196, "CNWAT", "Plant Canopy Surface Water", "kg m^-2"},                   /* also 4.2 : 2.0 : 13  */
  {2, 0, 197, "BMIXL", "Blackadar's Mixing Length Scale", "m"},                     /* also 4.2 : 2.0 : 14  */
  {2, 0, 198, "VGTYP", "Vegetation Type", "categorical"},
  {2, 0, 199, "CCOND", "Canopy Conductance", "m s^-1"},                                /* also 4.2 : 2.0 : 15  */
  {2, 0, 200, "RSMIN", "Minimal Stomatal Resistance", "s m^-1"},                       /* also 4.2 : 2.0 : 16  */
  {2, 0, 201, "WILT", "Wilting Point", "fraction"},                                 /* also 4.2 : 2.0 : 17  */
  {2, 0, 202, "RCS", "Solar parameter in canopy conductance", "fraction"},          /* also 4.2 : 2.0 : 18  */
  {2, 0, 203, "RCT", "Temperature parameter in canopy conductance", "fraction"},    /* also 4.2 : 2.0 : 19  */
  {2, 0, 204, "RCQ", "Humidity parameter in canopy conductance", "fraction"},       /* also 4.2 : 2.0 : 21  */
  {2, 0, 205, "RCSOL", "Soil moisture parameter in canopy conductance", "fraction"},/* also 4.2 : 2.0 : 20  */
  {2, 0, 206, "RDRIP", "Rate of water dropping from canopy to ground", "-"},
  {2, 0, 207, "ICWAT", "Ice-free water surface", "%"},
  {2, 0, 208, "AKHS", "Surface exchange coefficients for T and Q divided by delta z", "m s^-1"},
  {2, 0, 209, "AKMS", "Surface exchange coefficients for U and V divided by delta z", "m s^-1"},
  {2, 0, 210, "VEGT", "Vegetation canopy temperature", "K"},
  {2, 0, 211, "SSTOR", "Surface water storage", "Kg m^-2"},
  {2, 0, 212, "LSOIL", "Liquid soil moisture content (non-frozen)", "Kg m^-2"},
  {2, 0, 213, "EWATR", "Open water evaporation (standing water)", "W m^-2"},
  {2, 0, 214, "GWREC", "Groundwater recharge", "kg m^-2"},
  {2, 0, 215, "QREC", "Flood plain recharge", "kg m^-2"},
  {2, 0, 216, "SFCRH", "Roughness length for heat", "m"},
  {2, 0, 217, "NDVI", "Normalized Difference Vegetation Index", "-"},
  {2, 0, 218, "LANDN", "Land-sea coverage (nearest neighbor) [land=1,sea=0]", "categorical"},
  {2, 0, 219, "AMIXL", "Asymptotic mixing length scale", "m"},
  {2, 0, 220, "WVINC", "Water vapor added by precip assimilation", "kg m^-2"},
  {2, 0, 221, "WCINC", "Water condensate added by precip assimilation", "kg m^-2"},
  {2, 0, 222, "WVCONV", "Water Vapor Flux Convergance (Vertical Int)", "kg m^-2"},
  {2, 0, 223, "WCCONV", "Water Condensate Flux Convergance (Vertical Int)", "kg m^-2"},
  {2, 0, 224, "WVUFLX", "Water Vapor Zonal Flux (Vertical Int)", "kg m^-2"},
  {2, 0, 225, "WVVFLX", "Water Vapor Meridional Flux (Vertical Int)", "kg m^-2"},
  {2, 0, 226, "WCUFLX", "Water Condensate Zonal Flux (Vertical Int)", "kg m^-2"},
  {2, 0, 227, "WCVFLX", "Water Condensate Meridional Flux (Vertical Int)", "kg m^-2"},
  {2, 0, 228, "ACOND", "Aerodynamic conductance", "m s^-1"},
  {2, 0, 229, "EVCW", "Canopy water evaporation", "W m^-2"},
  {2, 0, 230, "TRANS", "Transpiration", "W m^-2"},

  /* 4.2 : 2.1  Land Surface products, Agricultural category */
  {2, 1, 192, "CANL", "Cold Advisory for Newborn Livestock", "numeric"},

  /* 4.2 : 2.3  Land Surface products, Soil category */
  {2, 3, 192, "SOILL", "Liquid Volumetric Soil Moisture", "fraction"},            /* also 4.2 : 2.3 : 5 */
  {2, 3, 193, "RLYRS", "Number of Soil Layers in Root Zone", "-"},                /* also 4.2 : 2.3 : 6 */
  {2, 3, 194, "SLTYP", "Surface Slope Type", "index"},
  {2, 3, 195, "SMREF", "Transpiration Stress-onset (soil moisture)", "fraction"}, /* also 4.2 : 2.3 : 7 */
  {2, 3, 196, "SMDRY", "Direct Evaporation Cease (soil moisture)", "fraction"},   /* also 4.2 : 2.3 : 8 */
  {2, 3, 197, "POROS", "Soil Porosity", "fraction"},                              /* also 4.2 : 2.3 : 9 */
  {2, 3, 198, "EVBS", "Direct evaporation from bare soil", "W m^-2"},
  {2, 3, 199, "LSPA", "Land Surface Precipitation Accumulation", "kg m^-2"},
  {2, 3, 200, "BARET", "Bare soil surface skin temperature", "K"},
  {2, 3, 201, "AVSFT", "Average surface skin temperature", "K"},
  {2, 3, 202, "RADT", "Effective radiative skin temperature", "K"},
  {2, 3, 203, "FLDCP", "Field Capacity", "fraction"},

  /* 4.2 : 3.1 Space products, Quantitative category */
  {3, 1, 192, "USCT", "Scatterometer Estimated U Wind", "-"},
  {3, 1, 193, "VSCT", "Scatterometer Estimated V Wind", "-"},
  {3, 1, 194, "SWQI", "Scatterometer Wind Quality","-"},

  /* 4.2 : 3.192 Space products, Forecast Satellite Imagery */
  {3, 192, 0, "SBT122", "Simulated Brightness Temperature for GOES 12, Channel 2", "K"},
  {3, 192, 1, "SBT123", "Simulated Brightness Temperature for GOES 12, Channel 3", "K"},
  {3, 192, 2, "SBT124", "Simulated Brightness Temperature for GOES 12, Channel 4", "K"},
  {3, 192, 3, "SBT125", "Simulated Brightness Temperature for GOES 12, Channel 5", "K"},
  {3, 192, 4, "SBC123", "Simulated Brightness Counts for GOES 12, Channel 3", "byte"},
  {3, 192, 5, "SBC124", "Simulated Brightness Counts for GOES 12, Channel 4", "byte"},
  {3, 192, 6, "SBT112", "Simulated Brightness Temperature for GOES 11, Channel 2", "K"},
  {3, 192, 7, "SBT113", "Simulated Brightness Temperature for GOES 11, Channel 3", "K"},
  {3, 192, 8, "SBT114", "Simulated Brightness Temperature for GOES 11, Channel 4", "K"},
  {3, 192, 9, "SBT115", "Simulated Brightness Temperature for GOES 11, Channel 5", "K"},
  {3, 192, 10, "AMSRE9", "Simulated Brightness Temperature for AMSRE on Aqua, Channel 9", "K"},
  {3, 192, 11, "AMSRE10", "Simulated Brightness Temperature for AMSRE on Aqua, Channel 10", "K"},
  {3, 192, 12, "AMSRE11", "Simulated Brightness Temperature for AMSRE on Aqua, Channel 11", "K"},
  {3, 192, 13, "AMSRE12", "Simulated Brightness Temperature for AMSRE on Aqua, Channel 12", "K"},
  {3, 192, 14, "SRFA161", "Simulated Reflectance Factor for ABI GOES-16, Band-1", "-"},
  {3, 192, 15, "SRFA162", "Simulated Reflectance Factor for ABI GOES-16, Band-2", "-"},
  {3, 192, 16, "SRFA163", "Simulated Reflectance Factor for ABI GOES-16, Band-3", "-"},
  {3, 192, 17, "SRFA164", "Simulated Reflectance Factor for ABI GOES-16, Band-4", "-"},
  {3, 192, 18, "SRFA165", "Simulated Reflectance Factor for ABI GOES-16, Band-5", "-"},
  {3, 192, 19, "SRFA166", "Simulated Reflectance Factor for ABI GOES-16, Band-6", "-"},
  {3, 192, 20, "SBTA167", "Simulated Brightness Temperature for ABI GOES-16, Band-7", "K"},
  {3, 192, 21, "SBTA168", "Simulated Brightness Temperature for ABI GOES-16, Band-8", "K"},
  {3, 192, 22, "SBTA169", "Simulated Brightness Temperature for ABI GOES-16, Band-9", "K"},
  {3, 192, 23, "SBTA1610", "imulated Brightness Temperature for ABI GOES-16, Band-10", "K"},
  {3, 192, 24, "SBTA1611", "Simulated Brightness Temperature for ABI GOES-16, Band-11", "K"},
  {3, 192, 25, "SBTA1612", "Simulated Brightness Temperature for ABI GOES-16, Band-12", "K"},
  {3, 192, 26, "SBTA1613", "Simulated Brightness Temperature for ABI GOES-16, Band-13", "K"},
  {3, 192, 27, "SBTA1614", "Simulated Brightness Temperature for ABI GOES-16, Band-14", "K"},
  {3, 192, 28, "SBTA1615", "Simulated Brightness Temperature for ABI GOES-16, Band-15", "K"},
  {3, 192, 29, "SBTA1616", "Simulated Brightness Temperature for ABI GOES-16, Band-16", "K"},
  {3, 192, 30, "SRFA171", "Simulated Reflectance Factor for ABI GOES-17, Band-1", "-"},
  {3, 192, 31, "SRFA172", "Simulated Reflectance Factor for ABI GOES-17, Band-2", "-"},
  {3, 192, 32, "SRFA173", "Simulated Reflectance Factor for ABI GOES-17, Band-3", "-"},
  {3, 192, 33, "SRFA174", "Simulated Reflectance Factor for ABI GOES-17, Band-4", "-"},
  {3, 192, 34, "SRFA175", "Simulated Reflectance Factor for ABI GOES-17, Band-5", "-"},
  {3, 192, 35, "SRFA176", "Simulated Reflectance Factor for ABI GOES-17, Band-6", "-"},
  {3, 192, 36, "SBTA177", "Simulated Brightness Temperature for ABI GOES-17, Band-7", "K"},
  {3, 192, 37, "SBTA178", "Simulated Brightness Temperature for ABI GOES-17, Band-8", "K"},
  {3, 192, 38, "SBTA179", "Simulated Brightness Temperature for ABI GOES-17, Band-9", "K"},
  {3, 192, 39, "SBTA1710", "Simulated Brightness Temperature for ABI GOES-17, Band-10", "K"},
  {3, 192, 40, "SBTA1711", "Simulated Brightness Temperature for ABI GOES-17, Band-11", "K"},
  {3, 192, 41, "SBTA1712", "Simulated Brightness Temperature for ABI GOES-17, Band-12", "K"},
  {3, 192, 42, "SBTA1713", "Simulated Brightness Temperature for ABI GOES-17, Band-13", "K"},
  {3, 192, 43, "SBTA1714", "Simulated Brightness Temperature for ABI GOES-17, Band-14", "K"},
  {3, 192, 44, "SBTA1715", "Simulated Brightness Temperature for ABI GOES-17, Band-15", "K"},
  {3, 192, 45, "SBTA1716", "Simulated Brightness Temperature for ABI GOES-17, Band-16", "K"},

  /* 4.2 : 10.0 Oceanographic products, Waves category */
  {10, 0, 192, "WSTP", "Wave Steepness", "-"},
  {10, 0, 193, "WLENG", "Wave Length", "-"},

  /* 4.2 : 10.1 Oceanographic products, Currents category */
  {10, 1, 192, "OMLU", "Ocean Mixed Layer U Velocity", "m s^-1"},
  {10, 1, 193, "OMLV", "Ocean Mixed Layer V Velocity", "m s^-1"},
  {10, 1, 194, "UBARO", "Barotropic U velocity", "m s^-1"},
  {10, 1, 195, "VBARO", "Barotropic V velocity", "m s^-1"},

  /* 4.2 : 10.3 Oceanographic products, Surface Properties category */
  {10, 3, 192, "SURGE", "Storm Surge", "m"},
  {10, 3, 193, "ETSRG", "Extra Tropical Storm Surge", "m"},
  {10, 3, 194, "ELEV", "Ocean Surface Elevation Relative to Geoid", "m"},
  {10, 3, 195, "SSHG", "Sea Surface Height Relative to Geoid", "m"},
  {10, 3, 196, "P2OMLT", "Ocean Mixed Layer Potential Density (Reference 2000m)", "kg m^-3"},
  {10, 3, 197, "AOHFLX", "Net Air-Ocean Heat Flux", "W m^-2"},
  {10, 3, 198, "ASHFL", "Assimilative Heat Flux", "W m^-2"},
  {10, 3, 199, "SSTT", "Surface Temperature Trend", "degree day^-1"},
  {10, 3, 200, "SSST", "Surface Salinity Trend", "psu day^-1"},
  {10, 3, 201, "KENG", "Kinetic Energy", "J kg^-1"},
  {10, 3, 202, "SLTFL", "Salt Flux", "kg m^-2 s^-1"},
  {10, 3, 242, "TCSRG20", "20% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 243, "TCSRG30", "30% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 244, "TCSRG40", "40% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 245, "TCSRG50", "50% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 246, "TCSRG60", "60% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 247, "TCSRG70", "70% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 248, "TCSRG80", "80% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 249, "TCSRG90", "90% Tropical Cyclone Storm Surge Exceedanc", "m"},
  {10, 3, 250, "ETCWL", "Extra Tropical Storm Surge Combined Surge and Tide", "m"},

  /* 4.2 : 10.4 Oceanographic products, Sub-Surface Properties category) */
  {10, 4, 192, "WTMPC", "3-D Temperature", "degrees c"},
  {10, 4, 193, "SALIN", "3-D Salinity", "-"},
  {10, 4, 194, "BKENG", "Barotropic Kinectic Energy", "J kg^-1"},
  {10, 4, 195, "DBSS", "Geometric Depth Below Sea Surface", "m"},
  {10, 4, 196, "INTFD", "Interface Depths", "m"},
  {10, 4, 197, "OHC", "Ocean Heat Content", "J m^-2"},

};

/* SPC (Storm Prediction Center) Local Use Table, Center 7, SubCenter 9 or 255 */

const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_SPClocalTable[] = {

  {0, 19, 205, "MUCP", "Most Unstable Parcel Cape", "J kg^-1"},
  {0, 19, 206, "MUCN", "Most Unstable Parcel CIN", "J kg^-1"},
  {0, 19, 207, "M1CP", "Mean Layer Parcel Cape", "J kg^-1"},
  {0, 19, 208, "SBCP", "Surface Based Parcel Cape", "J kg^-1"},
  {0, 19, 209, "ML3K", "Mean Layer Cape 0-3km", "J kg^-1"},
  {0, 19, 210, "DNCP", "Downdraft Cape", "J kg^-1"},
  {0, 19, 211, "NCAP", "Normalized Cape", "m s^-2"},
  {0, 19, 212, "MLCH", "Mean Layer parcel LCL Height", "m"},
  {0, 19, 213, "MLFH", "Mean Layer parcel LFC Height", "m"},
  {0, 19, 214, "MFCN", "Moisture Flux Convergence", "g kg^-1"},
  {0, 19, 215, "INPW", "Integrated Precipitable Water", "in"},
  {0, 19, 216, "FRNT", "Surface Frontogenesis", "K"},
  {0, 19, 217, "U6SV", "0-6km Shear U comp", "m s^-1"},
  {0, 19, 218, "V6SV", "0-6km Shear V comp", "m s^-1"},
  {0, 19, 219, "U8SV", "0-8km Shear U comp", "m s^-1"},
  {0, 19, 220, "V8SV", "0-8km Shear V comp", "m s^-1"},
  {0, 19, 221, "PVOR", "H4-H25 Potential Vorticity Advection", "m s^-1"},
  {0, 19, 222, "LLLR", "Low Level Lapse Rate", "K km^-1"},
  {0, 19, 223, "TL75", "H7-H5 Temp Difference", "K"},
  {0, 19, 224, "M1CN", "Mean Layer Parcel CIN", "J kg^-1"},
  {0, 19, 225, "SBCN", "Surface Based Parcel CIN", "J kg^-1"},
  {0, 19, 226, "MLPH", "Lifted Parcel Height (Mean Layer Parcel)", "m"},
  {0, 19, 227, "MCAV", "Deep Moisture Flux Convergence", "g kg^-1"},
  {0, 19, 228, "FCRH", "Mean RH of LCL-LFC Layer", "%"},
  {0, 19, 229, "ESHR", "Effective Shear Magnitude", "M s^-1"},
  {0, 19, 230, "ALT2", "2hr Altimeter Change", "in 2hr^-1"},
  {0, 19, 231, "EDCP", "Derecho Composite Index", "m"},
};

/* Candian Meteorological Service - Montreal - Local Use Table, Center 54, SubCenter 2 */
/* 
   This list matches NCEP's but is only products we have seen from MSC
*/
const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_MSC_MONTREAL_localTable[] = {
  /* 4.2 : 0.4 Meteorological products, short-wave radiation category */
  {0, 4, 192, "DSWRF", "Downward Short-Wave Rad. Flux", "W m^-2"},        /* also 4.2 : 0.4 : 7  */
  {0, 4, 193, "USWRF", "Upward Short-Wave Radiation Flux", "W m^-2"},     /* also 4.2 : 0.4 : 8  */

  /* 4.2 : 0.5 Meteorological products, long-wave radiation category */
  {0, 5, 192, "DLWRF", "Downward Long-Wave Rad. Flux", "W m^-2"},         /* also 4.2 : 0.5 : 3  */
  {0, 5, 193, "ULWRF", "Upward Long-Wave Rad. Flux", "W m^-2"},           /* also 4.2 : 0.5 : 4  */
};

/* NOAA Forecast Systems Lab Local Use Table, Center 59, SubCenter 0 */
/* 
   This list matches NCEP's but is only products we have seen from FSL
*/
const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_NOAA_FSLlocalTable[] = {

  /* 4.2 : 0.0 Meteorological products, temperature category */
  {0, 0, 192, "SNOHF", "Snow Phase Change Heat Flux", "W m^-2"},    /* also 4.2 : 0.0 : 16  */
  {0, 0, 195, "LRGHR", "Large Scale Condensate Heat rate", "K s^-1"},

  /* 4.2 : 0.1  Meteorological products, moisture category */
  {0, 1, 192, "CRAIN", "Categorical Rain", "categorical"},            /* also 4.2 : 0.1 : 33  */
  {0, 1, 193, "CFRZR", "Categorical Freezing Rain", "categorical"},   /* also 4.2 : 0.1 : 34  */
  {0, 1, 194, "CICEP", "Categorical Ice Pellets", "categorical"},     /* also 4.2 : 0.1 : 35  */
  {0, 1, 195, "CSNOW", "Categorical Snow", "categorical"},            /* also 4.2 : 0.1 : 36  */
  {0, 1, 196, "CPRAT", "Convective Precipitation Rate", "kg s m^-2"},/* also 4.2 : 0.1 : 37  */
  {0, 1, 198, "MINRH", "Minimum Relative Humidity", "%"},
  {0, 1, 201, "SNOWC", "Snow Cover", "%"},                              /* also 4.2 : 0.1 : 42  */
  {0, 1, 207, "NCIP", "Number concentration for ice particles", "-"},
  {0, 1, 208, "SNOT", "Snow temperature", "K"},

  /* 4.2 : 0.3 Meteorological products, mass category */
  {0, 2, 192, "VWSH", "Vertical speed sheer", "s^-1"},                      /* also 4.2 : 0.2 : 25  */
  {0, 2, 194, "USTM", "U-Component Storm Motion", "m s^-1"},                  /* also 4.2 : 0.2 : 27  */
  {0, 2, 195, "VSTM", "V-Component Storm Motion", "m s^-1"},                  /* also 4.2 : 0.2 : 28  */
  {0, 2, 197, "FRICV", "Frictional Velocity", "m s^-1"},                      /* also 4.2 : 0.2 : 30  */

  /* 4.2 : 0.3 Meteorological products, mass category */
  {0, 3, 192, "MSLET", "Mean Sea Level Pressure (Eta Reduction)", "Pa"},
  {0, 3, 193, "ECHOT", "Echo top height of 18dBZ surface", "m"},           /* does not match NCEPs */
  {0, 3, 194, "VIL",   "Vertically integrated liquid,", "kg m^-2,"},        /* does not match NCEPs */
  {0, 3, 195, "RDVIL", "Radar-derived vertically integrated liquid", "kg m^-2,"}, /* does not match NCEPs */
  {0, 3, 196, "HPBL", "Planetary Boundary Layer Height", "m"},             /* also 4.2 : 0.3 : 18  */
  {0, 3, 197, "5WAVA", "5-Wave Geopotential Height Anomaly", "gpm"},       /* also 4.2 : 0.3 : 19  */
  {0, 3, 198, "MSLMA", "MSLP (MAPS System Reduction)", "Pa"},
  {0, 3, 200, "PLPL", "Pressure of level from which parcel was lifted", "Pa"},
  {0, 3, 206, "NLGSP", "Natural Log of Surface Pressure", "ln(kPa)"},

  /* 4.2 : 0.4 Meteorological products, short-wave radiation category */
  {0, 4, 192, "DSWRF", "Downward Short-Wave Rad. Flux", "W m^-2"},        /* also 4.2 : 0.4 : 7  */
  {0, 4, 193, "USWRF", "Upward Short-Wave Radiation Flux", "W m^-2"},     /* also 4.2 : 0.4 : 8  */

  /* 4.2 : 0.5 Meteorological products, long-wave radiation category */
  {0, 5, 192, "DLWRF", "Downward Long-Wave Rad. Flux", "W m^-2"},         /* also 4.2 : 0.5 : 3  */
  {0, 5, 193, "ULWRF", "Upward Long-Wave Rad. Flux", "W m^-2"},           /* also 4.2 : 0.5 : 4  */

  /* 4.2 : 0.7 Meteorological products, Thermodynamic Stability category */
  {0, 7, 192, "LFTX", "Surface Lifted Index", "K"},                        /* also 4.2 : 0.7 : 10  */
  {0, 7, 193, "4LFTX", "Best (4 layer) Lifted Index", "K"},                /* also 4.2 : 0.7 : 11  */

  /* 4.2 : 0.16 Meteorological products, Forecast Radar Imagery category */
  {0, 16, 195, "REFD", "Derived radar reflectivity", "dB"},
  {0, 16, 196, "REFC", "Maximum/Composite radar reflectivity", "dB"},

  /* 4.2 : 0.17 Meteorological products, Electrodynamics category */
  {0, 17, 192, "LTNG", "Lightning", "-"},

  /* 4.2 : 1.0 Hydrological products, Basic Hydrology category */
  {1, 0, 192, "BGRUN", "Baseflow-Groundwater Runoff", "kg m^-2"}, /* also 4.2 : 1.0 : 5  */
  {1, 0, 193, "SSRUN", "Storm Surface Runoff", "kg m^-2"},        /* also 4.2 : 1.0 : 6  */

  /* 4.2 : 2.0 Land Surface products, Vegetation/Biomass category */
  {2, 0, 192, "SOILW", "Volumetric Soil Moisture Content", "fraction"},    /* also 4.2 : 2.0 : 9  */
  {2, 0, 193, "GFLUX", "Ground Heat Flux", "W m^-2"},                     /* also 4.2 : 2.0 : 10  */
  {2, 0, 194, "MSTAV", "Moisture Availability", "%"},                      /* also 4.2 : 2.0 : 11  */
  {2, 0, 196, "CNWAT", "Plant Canopy Surface Water", "kg m^-2"},          /* also 4.2 : 2.0 : 13  */
  {2, 0, 198, "VGTYP", "Vegetation Type", "categorical"},
  {2, 0, 229, "EVCW", "Canopy water evaporation", "W m^-2"},
  {2, 0, 230, "TRANS", "Transpiration", "W m^-2"},

  /* 4.2 : 2.3  Land Surface products, Soil category */
  {2, 3, 198, "EVBS", "Direct evaporation from bare soil", "W m^-2"},

  /* 4.2 : 3.192 Space products, Forecast Satellite Imagery */
  {3, 192, 2, "SBT124", "Simulated Brightness Temperature for GOES 12, Channel 4", "K"},

  /* 4.2 : 10.3 Oceanographic products, Surface Properties category */
  {10, 3, 192, "SURGE", "Storm Surge", "m"},
};


/* MRMS Local Use Table, Center 161, SubCenter 0, Discipline 209 */
/* 
   This list is from NOAAs listing of MRMS operation local use variables
   at http://www.nssl.noaa.gov/projects/mrms/operational/tables.php

*/
const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_MRMSlocalTable[] = {
  {209,2,0, "NLDN1min", "CG Lightning Density 1-min - NLDN", "flashes km^-2 min^-1"},
  {209,2,1, "NLDN5min", "CG Lightning Density 5-min - NLDN", "flashes km^-2 min^-1"},
  {209,2,2, "NLDN15min", "CG Lightning Density 15-min - NLDN", "flashes km^-2 min^-1"},
  {209,2,3, "NLDN30min", "CG Lightning Density 30-min - NLDN", "flashes km^-2 min^-1"},
  {209,2,4, "NLDNProbability30min", "Lightning Probability 0-30 minutes - NLDN", "%"},
  {209,3,0, "MergedAzShear0to2kmAGL", "Azimuth Shear 0-2km AGL", "0.001 s^-1"},
  {209,3,1, "MergedAzShear3to6kmAGL", "Azimuth Shear 3-6km AGL", "0.001 s^-1"},
  {209,3,2, "RotationTrack30min", "Rotation Track 0-2km AGL 30-min", "0.001 s^-1"},
  {209,3,3, "RotationTrack60min", "Rotation Track 0-2km AGL 60-min", "0.001 s^-1"},
  {209,3,4, "RotationTrack120min", "Rotation Track 0-2km AGL 120-min", "0.001 s^-1"},
  {209,3,5, "RotationTrack240min", "Rotation Track 0-2km AGL 240-min", "0.001 s^-1"},
  {209,3,6, "RotationTrack360min", "Rotation Track 0-2km AGL 360-min", "0.001 s^-1"},
  {209,3,7, "RotationTrack1440min", "Rotation Track 0-2km AGL 1440-min", "0.001 s^-1"},
  {209,3,14, "RotationTrackML30min", "Rotation Track 0-3km AGL 30-min", "0.001 s^-1"},
  {209,3,15, "RotationTrackML60min", "Rotation Track 0-3km AGL 60-min", "0.001 s^-1"},
  {209,3,16, "RotationTrackML120min", "Rotation Track 0-3km AGL 120-min", "0.001 s^-1"},
  {209,3,17, "RotationTrackML240min", "Rotation Track 0-3km AGL 240-min", "0.001 s^-1"},
  {209,3,18, "RotationTrackML360min", "Rotation Track 0-3km AGL 360-min", "0.001 s^-1"},
  {209,3,19, "RotationTrackML1440min", "Rotation Track 0-3km AGL 1440-min", "0.001 s^-1"},
  {209,3,26, "SHI", "Severe Hail Index", "index"},
  {209,3,27, "POSH", "Prob of Severe Hail", "%"},
  {209,3,28, "MESH", "Maximum Estimated Size of Hail (MESH)", "mm"},
  {209,3,29, "MESHMax30min", "MESH Hail Swath 30-min", "mm"},
  {209,3,30, "MESHMax60min", "MESH Hail Swath 60-min", "mm"},
  {209,3,31, "MESHMax120min", "MESH Hail Swath 120-min", "mm"},
  {209,3,32, "MESHMax240min", "MESH Hail Swath 240-min", "mm"},
  {209,3,33, "MESHMax360min", "MESH Hail Swath 360-min", "mm"},
  {209,3,34, "MESHMax1440min", "MESH Hail Swath 1440-min", "mm"},
  {209,3,41, "VIL", "Vertically Integrated Liquid", "kg m^-2"},
  {209,3,42, "VILD", "Vertically Integrated Liquid Density", "g m^-3"},
  {209,3,43, "VII", "Vertically Integrated Ice", "kg m^-2"},
  {209,3,44, "ETP18", "Echo Top - 18 dBZ", "km MSL"},
  {209,3,45, "ETP30", "Echo Top - 30 dBZ", "km MSL"},
  {209,3,46, "ETP50", "Echo Top - 50 dBZ", "km MSL"},
  {209,3,47, "ETP60", "Echo Top - 60 dBZ", "km MSL"},
  {209,3,48, "H50AboveM20C", "Thickness [50 dBZ top - (-20C)]", "km"},
  {209,3,49, "H50Above0C", "Thickness [50 dBZ top - 0C]", "km"},
  {209,3,50, "H60AboveM20C", "Thickness [60 dBZ top - (-20C)]", "km"},
  {209,3,51, "H60Above0C", "Thickness [60 dBZ top - 0C]", "km"},
  {209,3,52, "Reflectivity0C", "Isothermal Reflectivity at 0C", "dBZ"},
  {209,3,53, "ReflectivityM5C", "Isothermal Reflectivity at -5C", "dBZ"},
  {209,3,54, "ReflectivityM10C", "Isothermal Reflectivity at -10C", "dBZ"},
  {209,3,55, "ReflectivityM15C", "Isothermal Reflectivity at -15C", "dBZ"},
  {209,3,56, "ReflectivityM20C", "Isothermal Reflectivity at -20C", "dBZ"},
  {209,3,57, "ReflectivityAtLowestAltitude", "ReflectivityAtLowestAltitude", "dBZ"},
  {209,3,58, "MergedReflectivityAtLowestAltitude", "Non Quality Controlled Reflectivity At Lowest Altitude", "dBZ"},
  {209,4,0, "IRband4", "Infrared (E/W blend)", "K"},
  {209,4,1, "Visible", "Visible (E/W blend)", "non-dim"},
  {209,4,2, "WaterVapor", "Water Vapor (E/W blend)", "K"},
  {209,4,3, "CloudCover", "Cloud Cover", "K"},
  {209,6,0, "PCPFLAG", "Surface Precipitation Type (Convective 6, Stratiform 1-2 10, Tropical 91-96, Hail 7, Snow 3-4)", "categorical"},
  {209,6,1, "PRATE", "Radar Precipitation Rate", "mm hr^-1"},
  {209,6,2, "RadarOnlyQPE01H", "Radar precipitation accumulation 1-hour", "mm"},
  {209,6,3, "RadarOnlyQPE03H", "Radar precipitation accumulation 3-hour", "mm"},
  {209,6,4, "RadarOnlyQPE06H", "Radar precipitation accumulation 6-hour", "mm"},
  {209,6,5, "RadarOnlyQPE12H", "Radar precipitation accumulation 12-hour", "mm"},
  {209,6,6, "RadarOnlyQPE24H", "Radar precipitation accumulation 24-hour", "mm"},
  {209,6,7, "RadarOnlyQPE48H", "Radar precipitation accumulation 48-hour", "mm"},
  {209,6,8, "RadarOnlyQPE72H", "Radar precipitation accumulation 72-hour", "mm"},
  {209,6,9, "GaugeCorrQPE01H", "Local gauge bias corrected radar precipitation accumulation 1-hour", "mm"},
  {209,6,10, "GaugeCorrQPE03H", "Local gauge bias corrected radar precipitation accumulation 3-hour", "mm"},
  {209,6,11, "GaugeCorrQPE06H", "Local gauge bias corrected radar precipitation accumulation 6-hour", "mm"},
  {209,6,12, "GaugeCorrQPE12H", "Local gauge bias corrected radar precipitation accumulation 12-hour", "mm"},
  {209,6,13, "GaugeCorrQPE24H", "Local gauge bias corrected radar precipitation accumulation 24-hour", "mm"},
  {209,6,14, "GaugeCorrQPE48H", "Local gauge bias corrected radar precipitation accumulation 48-hour", "mm"},
  {209,6,15, "GaugeCorrQPE72H", "Local gauge bias corrected radar precipitation accumulation 72-hour", "mm"},
  {209,6,16, "GaugeOnlyQPE01H", "Gauge only precipitation accumulation 1-hour", "mm"},
  {209,6,17, "GaugeOnlyQPE03H", "Gauge only precipitation accumulation 3-hour", "mm"},
  {209,6,18, "GaugeOnlyQPE06H", "Gauge only precipitation accumulation 6-hour", "mm"},
  {209,6,19, "GaugeOnlyQPE12H", "Gauge only precipitation accumulation 12-hour", "mm"},
  {209,6,20, "GaugeOnlyQPE24H", "Gauge only precipitation accumulation 24-hour", "mm"},
  {209,6,21, "GaugeOnlyQPE48H", "Gauge only precipitation accumulation 48-hour", "mm"},
  {209,6,22, "GaugeOnlyQPE72H", "Gauge only precipitation accumulation 72-hour", "mm"},
  {209,6,23, "MountainMapperQPE01H", "Mountain Mapper precipitation accumulation 1-hour", "mm"},
  {209,6,24, "MountainMapperQPE03H", "Mountain Mapper precipitation accumulation 3-hour", "mm"},
  {209,6,25, "MountainMapperQPE06H", "Mountain Mapper precipitation accumulation 6-hour", "mm"},
  {209,6,26, "MountainMapperQPE12H", "Mountain Mapper precipitation accumulation 12-hour", "mm"},
  {209,6,27, "MountainMapperQPE24H", "Mountain Mapper precipitation accumulation 24-hour", "mm"},
  {209,6,28, "MountainMapperQPE48H", "Mountain Mapper precipitation accumulation 48-hour", "mm"},
  {209,6,29, "MountainMapperQPE72H", "Mountain Mapper precipitation accumulation 72-hour", "mm"},
  {209,7,0, "ModelSurfaceTemp", "Model Surface temperature [RAP 13km]", "C"},
  {209,7,1, "ModelWetBulbTemp", "Model Surface wet bulb temperature [RAP 13km]", "C"},
  {209,7,2, "WarmRainProbability", "Probability of warm rain [RAP 13km derived]", "%"},
  {209,7,3, "ModelHeight0C", "Model Freezing Level Height [RAP 13km]", "m MSL"},
  {209,7,4, "BrightBandTopHeight", "Brightband Top Radar [RAP 13km derived]", "m AGL"},
  {209,7,5, "BrightBandBottomHeight", "Brightband Bottom Radar [RAP 13km derived]", "m AGL"},
  {209,8,0, "RadarQualityIndex", "Radar Quality Index", "non-dim"},
  {209,8,1, "GaugeInflIndex01H", "Gauge Influence Index for 1-hour QPE", "non-dim"},
  {209,8,2, "GaugeInflIndex03H", "Gauge Influence Index for 3-hour QPE", "non-dim"},
  {209,8,3, "GaugeInflIndex06H", "Gauge Influence Index for 6-hour QPE", "non-dim"},
  {209,8,4, "GaugeInflIndex12H", "Gauge Influence Index for 12-hour QPE", "non-dim"},
  {209,8,5, "GaugeInflIndex24H", "Gauge Influence Index for 24-hour QPE", "non-dim"},
  {209,8,6, "GaugeInflIndex48H", "Gauge Influence Index for 48-hour QPE", "non-dim"},
  {209,8,7, "GaugeInflIndex72H", "Gauge Influence Index for 72-hour QPE", "non-dim"},
  {209,8,8, "SHSR", "Seamless Hybrid Scan Reflectivity with VPR correction", "dBZ"},
  {209,8,9, "SHSRH", "Height of Seamless Hybrid Scan Reflectivity", "km AGL"},
  {209,9,0, "MREF", "WSR-88D 3D Reflectivty Mosaic", "dBZ"},
  {209,9,1, "MREF", "All Radar 3D Reflectivty Mosaic", "dBZ"},
  {209,10,0, "CREF", "Composite Reflectivity Mosaic (optimal method)", "dBZ"},
  {209,10,1, "CREFH", "Height of Composite Reflectivity Mosaic (optimal method)", "m MSL"},
  {209,10,2, "LCREF", "Low-Level Composite Reflectivity Mosaic (0-4km)", "dBZ"},
  {209,10,3, "LCREFH", "Height of Low-Level Composite Reflectivity Mosaic (0-4km)", "m MSL"},
  {209,10,4, "LCR_LOW", "Layer Composite Reflectivity Mosaic 0-24kft (low altitude)", "dBZ"},
  {209,10,5, "LCR_HIGH", "Layer Composite Reflectivity Mosaic 24-60 kft (highest altitude)", "dBZ"},
  {209,10,6, "LCR_SUSER", "Layer Composite Reflectivity Mosaic 33-60 kft (super high altitude)", "dBZ"},
  {209,10,7, "CREFMAX", "Composite Reflectivity Hourly Maximum", "dBZ"},
  {209,10,8, "CREFM10C", "Maximum Reflectivity at -10 deg C height and above", "dBZ"},
  {209,11,0, "BASE_REFL", "Mosaic Base Reflectivity (optimal method)", "dBZ"},
  {209,11,1, "CREF", "UnQcd Composite Reflectivity Mosaic (max ref)", "dBZ"},
  {209,11,2, "CREF", "Composite Reflectivity Mosaic (max ref)", "dBZ"}
};

/* COSMO - Local Use Table, Center 250, SubCenter 2 */
const ProdDefTemp::_GRIB2LocalTable ProdDefTemp::_COSMO_localTable[] = {
  {192, 0, 0,   "TDEF", "Total Deformation", "s^-1"},
  {0, 1, 197,   "TUHFLX", "Total Upward Heat Flux", "W m^-2"},
  {0, 191, 207, "BLH", "Boundary Layer Height", "m"},
  {2, 0, 203,   "IMOL", "Inverse of Monin-Obukov length", "-"},
  {0, 2, 207,   "USTAR", "U-star", "-"},
  {2, 0, 208,   "VOROG", "Variance of Orography", "-"},
  {2, 0, 206,   "RHEIGHT", "Roughness Height", "-"},
};

/**************************************************************************
 * Constructor.
 */

ProdDefTemp::ProdDefTemp() 
{
  _parameterLongName = NULL;
  _parameterName = NULL;
  _parameterUnits = NULL;
  _processID = 255;
}

ProdDefTemp::ProdDefTemp(Grib2Record::Grib2Sections_t sectionsPtr) 
{
  setSectionsPtr(sectionsPtr);
  _parameterLongName = NULL;
  _parameterName = NULL;
  _parameterUnits = NULL;
  _processID = 255;
}

/**************************************************************************
 * Destructor
 */

ProdDefTemp::~ProdDefTemp() 
{
  if(_parameterLongName != NULL)
    delete _parameterLongName;
  if(_parameterName != NULL)
    delete _parameterName;
  if(_parameterUnits != NULL)
    delete _parameterUnits;

  _parameterLongName = NULL;
  _parameterName = NULL;
  _parameterUnits = NULL;
}

void ProdDefTemp::setSectionsPtr(Grib2Record::Grib2Sections_t sectionsPtr)
{
  _sectionsPtr = sectionsPtr;
  _centerId = _sectionsPtr.ids->getCenterId();
  _subCenterId = _sectionsPtr.ids->getSubCenterId();
  _disciplineNum = _sectionsPtr.is->getDisciplineNum();
}

void ProdDefTemp::setParamStrings () 
{
  if(_parameterLongName != NULL)
    delete _parameterLongName;
  if(_parameterName != NULL)
    delete _parameterName;
  if(_parameterUnits != NULL)
    delete _parameterUnits;

  _parameterLongName = NULL;
  _parameterName = NULL;
  _parameterUnits = NULL;

  if (_paramNumber >= 255 || _parameterCategory >= 255) 
  {
     _parameterName = new string ("Missing");
     if(_paramNumber >= 255)
       _parameterLongName = new string ("Missing parameter number");
     else
       _parameterLongName = new string ("Missing category number");
     _parameterUnits = new string ("-");

  // Parameter categories 192 through 254 are Local Use Only
  // This is constant among the various parameter Disciplines
  } else if (_paramNumber >= 192 || _parameterCategory >= 192 || _disciplineNum >= 192) 
  {

     // override defaults with local values where appropriate
     switch (_centerId) {
        // NCEP local tables
        case 7:
	  // Sub-center values for Center 7, US National Centers for Environmental Prediction 
	  //1	NCEP Re-Analysis Project
	  //2	NCEP Ensemble Products
	  //3	NCEP Central Operations
	  //4	Environmental Modeling Center
	  //5	Hydrometeorological Prediction Center
	  //6	Marine Prediction Center
	  //7	Climate Prediction Center
	  //8	Aviation Weather Center
	  //9	Storm Prediction Center
	  //10	National Hurricane Prediction Center
	  //11	NWS Techniques Development Laboratory
	  //12	NESDIS Office of Research and Applications
	  //13	Federal Aviation Administration
	  //14	NWS Meteorological Development Laboratory
	  //15	North American Regional Reanalysis Project
	  //16	Space Weather Prediction Center
          if (_subCenterId == 0 || _subCenterId == 1 ||_subCenterId == 2 || _subCenterId == 3) {
            // set local parameter information

            for (ui32 i = 0; i < _NCEPlocalTable_numElements; i++ ) {

               if ((_NCEPlocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _NCEPlocalTable[i].category) &&
                         (_paramNumber == _NCEPlocalTable[i].paramNumber)) {
                    _parameterName = new string (_NCEPlocalTable[i].name);   
                    _parameterLongName = new string (_NCEPlocalTable[i].comment);
                    _parameterUnits = new string (_NCEPlocalTable[i].unit);
                    break;
               }
            } // for
          }  // NCEP / Environmental Modeling Center (EMC), for now same as basic NCEP local tables
	  else if (_subCenterId == 4) {
            for (ui32 i = 0; i < _NCEPlocalTable_numElements; i++ ) {

               if ((_NCEPlocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _NCEPlocalTable[i].category) &&
                         (_paramNumber == _NCEPlocalTable[i].paramNumber)) {
                    _parameterName = new string (_NCEPlocalTable[i].name);   
                    _parameterLongName = new string (_NCEPlocalTable[i].comment);
                    _parameterUnits = new string (_NCEPlocalTable[i].unit);
                    break;
               }
            } // for
          }  // NCEP / Aviation Weather Center (AWC) local tables, for now same as basic NCEP local tables
	  else if (_subCenterId == 8) {
            for (ui32 i = 0; i < _NCEPlocalTable_numElements; i++ ) {

               if ((_NCEPlocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _NCEPlocalTable[i].category) &&
                         (_paramNumber == _NCEPlocalTable[i].paramNumber)) {
                    _parameterName = new string (_NCEPlocalTable[i].name);   
                    _parameterLongName = new string (_NCEPlocalTable[i].comment);
                    _parameterUnits = new string (_NCEPlocalTable[i].unit);
                    break;
               }
            } // for
	  } // NCEP / MDL (Meteorological Developmental Lab) local tables, for now same as basic NCEP local tables
	  else if (_subCenterId == 14) {
            for (ui32 i = 0; i < _NCEPlocalTable_numElements; i++ ) {

               if ((_NCEPlocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _NCEPlocalTable[i].category) &&
                         (_paramNumber == _NCEPlocalTable[i].paramNumber)) {
                    _parameterName = new string (_NCEPlocalTable[i].name);   
                    _parameterLongName = new string (_NCEPlocalTable[i].comment);
                    _parameterUnits = new string (_NCEPlocalTable[i].unit);
                    break;
               }
            } // for
	  } // SPC (Storm Prediction Center) local tables
	  else if (_subCenterId == 9 || _subCenterId == 255) {
            for (ui32 i = 0; i < _SPClocalTable_numElements; i++ ) {

               if ((_SPClocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _SPClocalTable[i].category) &&
                         (_paramNumber == _SPClocalTable[i].paramNumber)) {
                    _parameterName = new string (_SPClocalTable[i].name.c_str());   
                    _parameterLongName = new string (_SPClocalTable[i].comment.c_str());
                    _parameterUnits = new string (_SPClocalTable[i].unit.c_str());
                    break;
               }
            } // for
	  }
          else {
	    cerr << "ERROR: ProdDefTemp::setParamStrings()" << endl;
            cerr << "New sub-center found for NCEP, cannot decode local parameter numbers" << endl;
	  }
          break;

	// NWS Local Tables
        case 8:  
	  cerr << "ProdDefTemp::setParamStrings()" << endl;
          cerr << "NWS Telecomunications gateway local parameter tables not implemented" << endl;
          break;

       // Meteorological Service of Canada
       case 54:
	  // Montreal
          if (_subCenterId == 2 || _subCenterId == 0) {
            for (ui32 i = 0; i < _MSC_MONTREAL_localTable_numElements; i++ ) {

               if ((_MSC_MONTREAL_localTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _MSC_MONTREAL_localTable[i].category) &&
                         (_paramNumber == _MSC_MONTREAL_localTable[i].paramNumber)) {
                    _parameterName = new string (_MSC_MONTREAL_localTable[i].name);   
                    _parameterLongName = new string (_MSC_MONTREAL_localTable[i].comment);
                    _parameterUnits = new string (_MSC_MONTREAL_localTable[i].unit);
                    break;
               }
            } // for
	    if(_parameterName == NULL) {
	      for (ui32 i = 0; i < _NCEPlocalTable_numElements; i++ ) {
		if ((_NCEPlocalTable[i].prodDiscipline == _disciplineNum) && 
		    (_parameterCategory == _NCEPlocalTable[i].category) &&
		    (_paramNumber == _NCEPlocalTable[i].paramNumber)) {
		  _parameterName = new string (_NCEPlocalTable[i].name);   
		  _parameterLongName = new string (_NCEPlocalTable[i].comment);
		  _parameterUnits = new string (_NCEPlocalTable[i].unit);
		  break;
		}
	      } // for
	    }

          }
          else {
	    cerr << "ERROR: ProdDefTemp::setParamStrings()" << endl;
            cerr << "New sub-center found for NOAA FSL, cannot decode local parameter numbers" << endl;
	  }
          break;



	// NOAA Forecast Systems Lab
        case 59:
	  // Global Systems Division (GSD)
          if (_subCenterId == 0) {
            for (ui32 i = 0; i < _NOAA_FSLlocalTable_numElements; i++ ) {

               if ((_NOAA_FSLlocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _NOAA_FSLlocalTable[i].category) &&
                         (_paramNumber == _NOAA_FSLlocalTable[i].paramNumber)) {
                    _parameterName = new string (_NOAA_FSLlocalTable[i].name);   
                    _parameterLongName = new string (_NOAA_FSLlocalTable[i].comment);
                    _parameterUnits = new string (_NOAA_FSLlocalTable[i].unit);
                    break;
               }
            } // for
	    if(_parameterName == NULL) {
	      for (ui32 i = 0; i < _NCEPlocalTable_numElements; i++ ) {
		if ((_NCEPlocalTable[i].prodDiscipline == _disciplineNum) && 
		    (_parameterCategory == _NCEPlocalTable[i].category) &&
		    (_paramNumber == _NCEPlocalTable[i].paramNumber)) {
		  _parameterName = new string (_NCEPlocalTable[i].name);   
		  _parameterLongName = new string (_NCEPlocalTable[i].comment);
		  _parameterUnits = new string (_NCEPlocalTable[i].unit);
		  break;
		}
	      } // for
	    }

          }
          else {
	    cerr << "ERROR: ProdDefTemp::setParamStrings()" << endl;
            cerr << "New sub-center found for NOAA FSL, cannot decode local parameter numbers" << endl;
	  }
          break;

        // NCAR local tables
        case 60:
	  // Research Aplications Laboratory local table
          if (_subCenterId == 0) {
            for (ui32 i = 0; i < _NCAR_RALlocalTable_numElements; i++ ) {

               if ((_NCAR_RALlocalTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _NCAR_RALlocalTable[i].category) &&
                         (_paramNumber == _NCAR_RALlocalTable[i].paramNumber)) {
                    _parameterName = new string (_NCAR_RALlocalTable[i].name);   
                    _parameterLongName = new string (_NCAR_RALlocalTable[i].comment);
                    _parameterUnits = new string (_NCAR_RALlocalTable[i].unit);
                    break;
               }
            } // for
          }
          else {
	    cerr << "ERROR: ProdDefTemp::setParamStrings()" << endl;
            cerr << "New sub-center found for NCAR, cannot decode local parameter numbers" << endl;
	  }
          break;

        // US NOAA Office of Oceanic and Atmospheric Research
        case 161:
	  // Subcenter: 0 (National Severe Storms Lab)
	  // Operational MRMS GRIB2 Tables
	  if(_subCenterId == 0) {
	    for (ui32 i = 0; i < _MRMSlocalTable_numElements; i++ ) {
	      
	      if ((_MRMSlocalTable[i].prodDiscipline == _disciplineNum) && 
		  (_parameterCategory == _MRMSlocalTable[i].category) &&
		  (_paramNumber == _MRMSlocalTable[i].paramNumber)) {
		_parameterName = new string (_MRMSlocalTable[i].name);   
		_parameterLongName = new string (_MRMSlocalTable[i].comment);
		_parameterUnits = new string (_MRMSlocalTable[i].unit);
		break;
	      }
	    } // for
	  }
	  break;
	  
        // COSMO local tables
        case 250:
	  // COnsortium for Small scall MOdelling (COSMO) local table
          if (_subCenterId == 2) {
            for (ui32 i = 0; i < _COSMO_localTable_numElements; i++ ) {

               if ((_COSMO_localTable[i].prodDiscipline == _disciplineNum) && 
                         (_parameterCategory == _COSMO_localTable[i].category) &&
                         (_paramNumber == _COSMO_localTable[i].paramNumber)) {
                    _parameterName = new string (_COSMO_localTable[i].name);   
                    _parameterLongName = new string (_COSMO_localTable[i].comment);
                    _parameterUnits = new string (_COSMO_localTable[i].unit);
                    break;
               }
            } // for
          }
          else {
	    cerr << "ERROR: ProdDefTemp::setParamStrings()" << endl;
            cerr << "New sub-center found for COSMO, cannot decode local parameter numbers" << endl;
	  }
          break;

      default:
	cerr << "ERROR: ProdDefTemp::setParamStrings()" << endl;
	cerr << "New center ID found, cannot decode local parameter numbers" << endl;
	break;
     }  // switch


     if(_parameterName == NULL) {
       // set default values
       _parameterName = new string ("Reserved");
       _parameterLongName = new string ("Unknown local use paramater number");
       _parameterUnits = new string ("-");
     }

  }
  else 
  {
    switch (_disciplineNum)
    {
    case 0:
      // Product Discipline 0, Meteorological products

      switch (_parameterCategory) 
      {
      case 0:
        // GRIB2 Code table 4.2 : 0.0
 	// parameterCategory is temperature
        if (_paramNumber < (si32) sizeof (_meteoTemp) / (si32)sizeof(_GRIB2ParmTable)) {
	  _parameterName = new string (_meteoTemp[_paramNumber].name);
	  _parameterLongName = new string (_meteoTemp[_paramNumber].comment);
	  _parameterUnits = new string (_meteoTemp[_paramNumber].unit);
        }
        break;

      case 1:
        // GRIB2 Code table 4.2 : 0.1 
 	// parameterCategory is Moisture
        if (_paramNumber < (si32) sizeof (_meteoMoist) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoMoist[_paramNumber].name);
          _parameterLongName = new string (_meteoMoist[_paramNumber].comment);
          _parameterUnits = new string (_meteoMoist[_paramNumber].unit);
        }
        break;

      case 2:
        // GRIB2 Code table 4.2 : 0.2         
 	// parameterCategory is Momentum
        if (_paramNumber < (si32) sizeof (_meteoMoment) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoMoment[_paramNumber].name);
          _parameterLongName = new string (_meteoMoment[_paramNumber].comment);
          _parameterUnits = new string (_meteoMoment[_paramNumber].unit);
        }
        break;

     case 3:
        // GRIB2 Code table 4.2 : 0.3
 	// parameterCategory is Mass
        if (_paramNumber < (si32) sizeof (_meteoMass) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoMass[_paramNumber].name);
          _parameterLongName = new string (_meteoMass[_paramNumber].comment);
          _parameterUnits = new string (_meteoMass[_paramNumber].unit);
        }
        break;

     case 4:
        // GRIB2 Code table 4.2 : 0.4
 	// parameterCategory is Short-wave Radiation
        if (_paramNumber < (si32) sizeof (_meteoShortRadiate) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoShortRadiate[_paramNumber].name);
          _parameterLongName =new string (_meteoShortRadiate[_paramNumber].comment);
          _parameterUnits = new string (_meteoShortRadiate[_paramNumber].unit);
        }
        break;

     case 5:
        // GRIB2 Code table 4.2 : 0.5
 	// parameterCategory is Long-wave Radiation
        if (_paramNumber < (si32) sizeof (_meteoLongRadiate) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoLongRadiate[_paramNumber].name);
          _parameterLongName =new string (_meteoLongRadiate[_paramNumber].comment);
          _parameterUnits = new string (_meteoLongRadiate[_paramNumber].unit);
        }
        break;

     case 6:
        // GRIB2 Code table 4.2 : 0.6 
 	// parameterCategory is Cloud
        if (_paramNumber < (si32) sizeof (_meteoCloud) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoCloud[_paramNumber].name);
          _parameterLongName =new string (_meteoCloud[_paramNumber].comment);
          _parameterUnits = new string (_meteoCloud[_paramNumber].unit);
        }
        break;

     case 7:
        // GRIB2 Code table 4.2 : 0.7
 	// parameterCategory is Thermodynamic Stability indices
        if (_paramNumber < (si32) sizeof (_meteoStability) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoStability[_paramNumber].name);
          _parameterLongName =new string (_meteoStability[_paramNumber].comment);
          _parameterUnits = new string (_meteoStability[_paramNumber].unit);
        }
        break;

#ifdef NOTNOW
     case 8:
        // GRIB2 Code table 4.2 : 0.8
 	// parameterCategory is Kinematic Stability indices
        break;
     case 9:
        // GRIB2 Code table 4.2 : 0.9 
 	// parameterCategory is Temperature Probabilities
        break;
     case 10:
        // GRIB2 Code table 4.2 : 0.10 
 	// parameterCategory is Moisture Probabilities
        break;
     case 11:
        // GRIB2 Code table 4.2 : 0.11
 	// parameterCategory is Momentum Probabilities
        break;
     case 12:
        // GRIB2 Code table 4.2 : 0.12
 	// parameterCategory is Mass Probabilities
        break;
#endif
     case 13:
        // GRIB2 Code table 4.2 : 0.13
 	// parameterCategory is Aerosols
        if (_paramNumber < (si32) sizeof (_meteoAerosols) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoAerosols[_paramNumber].name);
          _parameterLongName =new string (_meteoAerosols[_paramNumber].comment);
          _parameterUnits = new string (_meteoAerosols[_paramNumber].unit);
        }
        break;

     case 14:
        // GRIB2 Code table 4.2 : 0.14
 	// parameterCategory is Trace gases (e.g., ozone, CO2)
        if (_paramNumber < (si32) sizeof (_meteoGases) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoGases[_paramNumber].name);
          _parameterLongName =new string (_meteoGases[_paramNumber].comment);
          _parameterUnits = new string (_meteoGases[_paramNumber].unit);
        }
        break;

     case 15:
        // GRIB2 Code table 4.2 : 0.15
 	// parameterCategory is Radar
        if (_paramNumber < (si32) sizeof (_meteoRadar) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoRadar[_paramNumber].name);
          _parameterLongName =new string (_meteoRadar[_paramNumber].comment);
          _parameterUnits = new string (_meteoRadar[_paramNumber].unit);
        }
        break;

     case 16:
        // GRIB2 Code table 4.2 : 0.16
 	// parameterCategory is Forecast Radar Imagery
        if (_paramNumber < (si32) sizeof (_meteoRadarForecast) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoRadarForecast[_paramNumber].name);
          _parameterLongName =new string (_meteoRadarForecast[_paramNumber].comment);
          _parameterUnits = new string (_meteoRadarForecast[_paramNumber].unit);
        }
        break;

     case 17:
        // GRIB2 Code table 4.2 : 0.17
 	// parameterCategory is Electro-dynamics
        if (_paramNumber < (si32) sizeof (_meteoElectro) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoElectro[_paramNumber].name);
          _parameterLongName =new string (_meteoElectro[_paramNumber].comment);
          _parameterUnits = new string (_meteoElectro[_paramNumber].unit);
        }
        break;

     case 18:
        // GRIB2 Code table 4.2 : 0.18
 	// parameterCategory is Nuclear/radiology
        if (_paramNumber < (si32) sizeof (_meteoNuclear) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoNuclear[_paramNumber].name);
          _parameterLongName =new string (_meteoNuclear[_paramNumber].comment);
          _parameterUnits = new string (_meteoNuclear[_paramNumber].unit);
        }
        break;

     case 19:
        // GRIB2 Code table 4.2 : 0.19 
 	// parameterCategory is Physical atmospheric properties
        if (_paramNumber < (si32) sizeof (_meteoAtmos) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoAtmos[_paramNumber].name);
          _parameterLongName =new string (_meteoAtmos[_paramNumber].comment);
          _parameterUnits = new string (_meteoAtmos[_paramNumber].unit);
        }
        break;

     case 20:
        // GRIB2 Code table 4.2 : 0.20 
 	// parameterCategory is Atmospheric Chemical Constituents
        if (_paramNumber < (si32) sizeof (_meteoChem) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoChem[_paramNumber].name);
          _parameterLongName =new string (_meteoChem[_paramNumber].comment);
          _parameterUnits = new string (_meteoChem[_paramNumber].unit);
        }
        break;

     case 190:
        // GRIB2 Code table 4.2 : 0.190 
 	// parameterCategory is Arbitrary text string
        if (_paramNumber < (si32) sizeof (_meteoText) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoText[_paramNumber].name);
          _parameterLongName =new string (_meteoText[_paramNumber].comment);
          _parameterUnits = new string (_meteoText[_paramNumber].unit);
        }
        break;

     case 191:
        // GRIB2 Code table 4.2 : 0.191 
 	// parameterCategory is Miscellaneous
        if (_paramNumber < (si32) sizeof (_meteoMisc) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_meteoMisc[_paramNumber].name);
          _parameterLongName =new string (_meteoMisc[_paramNumber].comment);
          _parameterUnits = new string (_meteoMisc[_paramNumber].unit);
        }
        break;

      case 255:
 	// parameterCategory is Missing
        _parameterName = new string ("Missing");
        _parameterLongName =new string ("Missing Category Number");
        _parameterUnits = new string ("-");	
        break;

      default:
        _parameterName = new string ("Missing");
        _parameterLongName =new string ("Missing Category Number");
        _parameterUnits = new string ("-");
      } // end switch (parameterCategory) 

      break;   // end Discipline 0

    case 1:
      // Product Discipline 1, Hydrologic products

      switch (_parameterCategory) 
      {
      case 0:
        // GRIB2 Code table 4.2 : 1.0
 	// Hydrology basic products
        if (_paramNumber < (si32) sizeof (_hydroBasic) / (si32)sizeof(_GRIB2ParmTable)) {
	  _parameterName = new string (_hydroBasic[_paramNumber].name);
	  _parameterLongName = new string (_hydroBasic[_paramNumber].comment);
	  _parameterUnits = new string (_hydroBasic[_paramNumber].unit);
        }
        break;

      case 1:
        // GRIB2 Code table 4.2 : 1.1 
 	// Hydrology probabilities
        if (_paramNumber < (si32) sizeof (_hydroProb) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_hydroProb[_paramNumber].name);
          _parameterLongName = new string (_hydroProb[_paramNumber].comment);
          _parameterUnits = new string (_hydroProb[_paramNumber].unit);
        }
        break;

      case 2:
        // GRIB2 Code table 4.2 : 1.2
 	// Hydrology Inland water and sediment properties
        if (_paramNumber < (si32) sizeof (_hydroWaterSediment) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_hydroWaterSediment[_paramNumber].name);
          _parameterLongName = new string (_hydroWaterSediment[_paramNumber].comment);
          _parameterUnits = new string (_hydroWaterSediment[_paramNumber].unit);
        }
        break;

      default:
        _parameterName = new string ("Missing");
        _parameterLongName =new string ("Missing Category Number");
        _parameterUnits = new string ("-");
      } // end switch (parameterCategory) 
      break;

    case 2:
      // Product Discipline 2, Land Surface products

      switch (_parameterCategory) 
      {
      case 0:
        // GRIB2 Code table 4.2 : 2.0
 	// Vegetation/Biomass
        if (_paramNumber < (si32) sizeof (_landVeg) / (si32)sizeof(_GRIB2ParmTable)) {
	  _parameterName = new string (_landVeg[_paramNumber].name);
	  _parameterLongName = new string (_landVeg[_paramNumber].comment);
	  _parameterUnits = new string (_landVeg[_paramNumber].unit);
        }
        break;

      case 3:
        // GRIB2 Code table 4.2 : 2.3 
 	// Soil products
        if (_paramNumber < (si32) sizeof (_landSoil) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_landSoil[_paramNumber].name);
          _parameterLongName = new string (_landSoil[_paramNumber].comment);
          _parameterUnits = new string (_landSoil[_paramNumber].unit);
        }
        break;

      case 4:
        // GRIB2 Code table 4.2 : 2.4 
 	// Fire Weather products
        if (_paramNumber < (si32) sizeof (_landFire) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_landFire[_paramNumber].name);
          _parameterLongName = new string (_landFire[_paramNumber].comment);
          _parameterUnits = new string (_landFire[_paramNumber].unit);
        }
        break;

      default:
        _parameterName = new string ("Missing");
        _parameterLongName =new string ("Missing Category Number");
        _parameterUnits = new string ("-");
      } // end switch (parameterCategory) 
      break;

    case 3:
      // Product Discipline 3, Space products

      switch (_parameterCategory) 
      {
      case 0:
        // GRIB2 Code table 4.2 : 3.0
 	// Image format products
        if (_paramNumber < (si32) sizeof (_spaceImage) / (si32)sizeof(_GRIB2ParmTable)) {
	  _parameterName = new string (_spaceImage[_paramNumber].name);
	  _parameterLongName = new string (_spaceImage[_paramNumber].comment);
	  _parameterUnits = new string (_spaceImage[_paramNumber].unit);
        }
        break;

      case 1:
        // GRIB2 Code table 4.2 : 3.1 
 	// Quantitative products
        if (_paramNumber < (si32) sizeof (_spaceQuantitative) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_spaceQuantitative[_paramNumber].name);
          _parameterLongName = new string (_spaceQuantitative[_paramNumber].comment);
          _parameterUnits = new string (_spaceQuantitative[_paramNumber].unit);
        }
        break;

      default:
        _parameterName = new string ("Missing");
        _parameterLongName =new string ("Missing Category Number");
        _parameterUnits = new string ("-");
      } // end switch (parameterCategory) 

      break;

    case 10:
      // Product Discipline 10, Oceanographic products

      switch (_parameterCategory) 
      {
      case 0:
        // GRIB2 Code table 4.2 : 10.0
 	// Waves
        if (_paramNumber < (si32) sizeof (_oceanWaves) / (si32)sizeof(_GRIB2ParmTable)) {
	  _parameterName = new string (_oceanWaves[_paramNumber].name);
	  _parameterLongName = new string (_oceanWaves[_paramNumber].comment);
	  _parameterUnits = new string (_oceanWaves[_paramNumber].unit);
        }
        break;

      case 1:
        // GRIB2 Code table 4.2 : 10.1 
 	// Currents
        if (_paramNumber < (si32) sizeof (_oceanCurrents) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_oceanCurrents[_paramNumber].name);
          _parameterLongName = new string (_oceanCurrents[_paramNumber].comment);
          _parameterUnits = new string (_oceanCurrents[_paramNumber].unit);
        }
        break;

      case 2:
        // GRIB2 Code table 4.2 : 10.2
 	// Ice
        if (_paramNumber < (si32) sizeof (_oceanIce) / (si32)sizeof(_GRIB2ParmTable)) {
	  _parameterName = new string (_oceanIce[_paramNumber].name);
	  _parameterLongName = new string (_oceanIce[_paramNumber].comment);
	  _parameterUnits = new string (_oceanIce[_paramNumber].unit);
        }
        break;

      case 3:
        // GRIB2 Code table 4.2 : 10.3 
 	// Surface properties
        if (_paramNumber < (si32) sizeof (_oceanSurface) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_oceanSurface[_paramNumber].name);
          _parameterLongName = new string (_oceanSurface[_paramNumber].comment);
          _parameterUnits = new string (_oceanSurface[_paramNumber].unit);
        }
        break;

      case 4:
        // GRIB2 Code table 4.2 : 10.4
 	// Sub-surface properties
        if (_paramNumber < (si32) sizeof (_oceanSubSurface) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_oceanSubSurface[_paramNumber].name);
          _parameterLongName = new string (_oceanSubSurface[_paramNumber].comment);
          _parameterUnits = new string (_oceanSubSurface[_paramNumber].unit);
        }
        break;

     case 191:
        // GRIB2 Code table 4.2 : 10.191 
 	//  Miscellaneous
        if (_paramNumber < (si32) sizeof (_oceanMisc) / (si32)sizeof(_GRIB2ParmTable)) {
          _parameterName = new string (_oceanMisc[_paramNumber].name);
          _parameterLongName =new string (_oceanMisc[_paramNumber].comment);
          _parameterUnits = new string (_oceanMisc[_paramNumber].unit);
        }
        break;

      default:
        _parameterName = new string ("Missing");
        _parameterLongName =new string ("Missing Category Number");
        _parameterUnits = new string ("-");
      } // end switch (parameterCategory) 

      break;


    default:
      _parameterName = new string ("Missing");
      _parameterLongName =new string ("Missing Discipline Number");
      _parameterUnits = new string ("-");      
    } // end switch (_disciplineNum)

    if(_parameterName == NULL) {
      _parameterName = new string ("Unknown");
      _parameterLongName = new string ("Unknown Parameter Number");
      _parameterUnits = new string ("-");
    } 

  } // end else

}

int ProdDefTemp::_getSurfaceIndex (int value) const
{
  if(value <= 20)                   /* 0 - 19 */
    return (value-1);
  if(value >= 100 && value <= 120)  /* 20 - 40 */
    return (value -100 + 20);
  if(value == 150)                  /* 41 */
    return (value - 150 + 41);
  if(value >= 160 && value <= 174)  /* 42 - 56 */
    return (value -160 + 42);
  if(value >= 200 && value <= 224)  /* 57 - 81 */
    return (value -200 + 57);
  if(value >= 232 && value <= 255)  /* 82 - 104 */
    return (value -232 + 82);
	    
  return (-1);
 
}

void ProdDefTemp::_printTimeIncrementType(FILE *stream, int timeIncrement) const
{
  fprintf(stream, "    Type of time increment between successive fields: \n");
  switch( timeIncrement ) {
  case 1:
    fprintf(stream, "        Successive times processed have same forecast time, start time of forecast is incremented.\n");
    break;
  case 2:
    fprintf(stream, "        Successive times processed have same start time of forecast, forecast time is incremented.\n");
    break;
  case 3:
    fprintf(stream, "        Successive times processed have start time of forecast incremented and forecast time decremented so that valid time remains constant.\n");
    break;
  case 4:
    fprintf(stream, "        Successive times processed have start time of forecast decremented and forecast time incremented so that valid time remains constant.\n");
    break;
  case 5:
    fprintf(stream, "        Floating subinterval of time between forecast time and end of overall time interval.\n");
    break;
  default:
    fprintf(stream, "        Unknown\n");
    break;
  }
}

//
// GRIB2 - CODE TABLE 4.3
// TYPE OF GENERATING PROCESS
void ProdDefTemp::_printGeneratingProcessType(FILE *stream, int processType) const
{
  fprintf(stream, "Type of generating process: ");
  switch( processType ) {
  case 0:
    fprintf(stream, "Analysis\n");
    break;
  case 1:
    fprintf(stream, "Initialization\n");
    break;
  case 2:
    fprintf(stream, "Forecast\n");
    break;
  case 3:
    fprintf(stream, "Bias Corrected Forecast\n");
    break;
  case 4:
    fprintf(stream, "Ensemble Forecast\n");
    break;
  case 5:
    fprintf(stream, "Probability Forecast\n");
    break;
  case 6:
    fprintf(stream, "Forecast Error\n");
    break;
  case 7:
    fprintf(stream, "Analysis Error\n");
    break;
  case 8:
    fprintf(stream, "Observation\n");
    break;
  case 9:
    fprintf(stream, "Climatological\n");
    break;
  case 10:
    fprintf(stream, "Probability-Weighted Forecast\n");
    break;
  case 11:
    fprintf(stream, "Bias-Corrected Ensemble Forecast\n");
    break;
  case 12:
    fprintf(stream, "Post-processed Analysis\n");
    break;
  case 13:
    fprintf(stream, "Post-processed Forecast\n");
    break;
  case 14:
    fprintf(stream, "Nowcast\n");
    break;
  case 15:
    fprintf(stream, "Hindcast\n");
    break;
  case 192:
    fprintf(stream, "Forecast Confidence Indicator\n");
    break;
  case 255:
    fprintf(stream, "Missing\n");
    break;
  default:
    fprintf(stream, "Reserved\n");
    break;
  }
}

//
// GRIB2 -CODE TABLE 4.4
// INDICATOR OF UNIT OF TIME RANGE
void ProdDefTemp::_printTimeUnits(FILE *stream, int timeUnits) const
{
  switch( timeUnits ) {
  case 0:
    fprintf(stream, "Minutes\n");
    break;	  
  case 1:
    fprintf(stream, "Hours\n");
    break;
  case 2:
    fprintf(stream, "Days\n");
    break;
  case 3:
    fprintf(stream, "Months\n");
    break;
  case 4:
    fprintf(stream, "Years\n");
    break;
  case 5:
    fprintf(stream, "Decades\n");
    break;
  case 6:
    fprintf(stream, "Normals (30 Years)\n"); 
    break;    
  case 7:
    fprintf(stream, "Centurys\n");
    break;
  case 10:
    fprintf(stream, "3 hour periods\n");
    break;
  case 11:
    fprintf(stream, "6 hours periods\n");
    break;
  case 12:
    fprintf(stream, "12 hours periods\n");
    break;
  case 13:	  
  case 254:
    fprintf(stream, "Seconds\n");
    break;
  default:
    fprintf(stream, "Unknown units\n");
    break;
  }
}

long int ProdDefTemp::_getTimeUnits( long int timeUnits) const
{
  switch( timeUnits ) {
  // Minutes
  case 0:
    return(60);
    break;
  // Hours    
  case 1:
    return(60*60);
    break;
  // Days
  case 2:
    return(24*60*60);
    break;
  // Time period of months poses a problem... variable days per month
  case 3:
    return(24*60*60);
    break;
  // Time period of years is set to 365 days which could be incorrect by one day (leap year).
  case 4:
    return((long int)24*60*60*(long int)365);
    break;
  // Decade can be off by one day as well (unknown number of leap years in decade).
  case 5:
    return((long int)24*60*60*(long int)3652);
    break;
  // Normal 30 years, hopefully never used.
  case 6:
    return((long int)24*60*60*(long int)10957);
    break;
  // Century
  case 7:
    return((long int)24*60*60*(long int)36524);
    break;
  // 3 hours
  case 10:
    return(3*60*60);
    break;
  // 6 hours    
  case 11:
    return(6*60*60);
    break;
  // 12 hours    
  case 12:
    return(12*60*60);
    break;
  // Seconds    
  case 13:
  case 254:
    return (1);
    break;
    
  default:
    return( -1 );
    break;
  }
}

string ProdDefTemp::_getTimeUnitName(int time, int timeUnits) const
{
  string timeName;
  switch( timeUnits ) {
  case 0:
    timeName = "Min";
    break;
  case 1:
    timeName = "Hr";
    break;    
  case 2:
    timeName = "Day";
    break;
  case 3:
    timeName = "Mon";
    break;
  case 4:
    timeName = "Yr";
    break;
  case 5:
    timeName = "Yr";
    time *= 10;
    break;
  case 6:
    timeName = "Yr";
    time *= 30;
    break;
  case 7:
    timeName = "Yr";
    time *= 100;
    break;
  case 10:
    timeName = "Hr";
    time *= 3;
    break;    
  case 11:
    timeName = "Hr";
    time *= 6;
    break;
  case 12:
    timeName = "Hr";
    time *= 12;
    break;
  case 13:
  case 254:
    timeName = "Sec";
    break;
  default:
    timeName = "Unk";
    break;
  }
  char buf[50];
  sprintf(buf, "%d", time);
  timeName = buf + timeName;
  return timeName;
}

//
// GRIB2 - CODE TABLE 4.6
// TYPE OF ENSEMBLE FORECAST
void ProdDefTemp::_printEnsembleForecastType(FILE *stream, int ensembleId) const
{
  fprintf(stream, "Type of ensemble forecast: ");
  switch( ensembleId ) {
  case 0:
    fprintf(stream, "Unperturbed High-Resolution Control Forecast\n");
    break;	  
  case 1:
    fprintf(stream, "Unperturbed Low-Resolution Control Forecast\n");
    break;
  case 2:
    fprintf(stream, "Negatively Perturbed Forecast\n");
    break;
  case 3:
    fprintf(stream, "Positively Perturbed Forecast\n");
    break;
  case 4:
    fprintf(stream, "Multi-Model Forecast\n");
    break;
  case 192:
    fprintf(stream, "Perturbed Ensemble Member\n");
    break;
  default:
    fprintf(stream, "Unknown \n");
    break;
  }
}

//
// GRIB2 - CODE TABLE 4.7
// DERIVED FORECAST TYPE
void ProdDefTemp::_printDerivedForecastType(FILE *stream, int derivedId) const
{
  fprintf(stream, "Type of derived forecast: ");
  switch( derivedId ) {
  case 0:
    fprintf(stream, "Unweighted Mean of All Members\n");
    break;	  
  case 1:
    fprintf(stream, "Weighted Mean of All Members\n");
    break;
  case 2:
    fprintf(stream, "Standard Deviation with respect to Cluster Mean\n");
    break;
  case 3:
    fprintf(stream, "Standard Deviation with respect to Cluster Mean, Normalized\n");
    break;
  case 4:
    fprintf(stream, "Spread of All Members\n");
    break;
  case 5:
    fprintf(stream, "Large Anomaly Index of All Members\n");
    break;
  case 6:
    fprintf(stream, "Unweighted Mean of the Cluster Members\n");
    break;
  case 7:
    fprintf(stream, "Interquartile Range (Range between the 25th and 75th quantile)\n");
    break;
  case 8:
    fprintf(stream, "Minimum Of All Ensemble Members\n");
    break;
  case 9:
    fprintf(stream, "Maximum Of All Ensemble Members\n");
    break;
  case 192:
    fprintf(stream, "Unweighted Mode of All Members\n");
    break;
  case 193:
    fprintf(stream, "Percentile value (10%%) of All Members\n");
    break;
  case 194:
    fprintf(stream, "Percentile value (50%%) of All Members\n");
    break;
  case 195:
    fprintf(stream, "Percentile value (90%%) of All Members\n");
    break;
  case 196:
    fprintf(stream, "Statistically decided weights for each ensemble member\n");
    break;
  case 197:
    fprintf(stream, "Climate Percentile (percentile values from climate distribution)\n");
    break;
  default:
    fprintf(stream, "Unknown \n");
    break;
  }
}

string ProdDefTemp::_getDerivedForecastType(int derivedId) const
{
  string ensemble;
  switch( derivedId ) {
  case 0:
    ensemble = "Ensemble Mean";
    break;	  
  case 1:
    ensemble = "Ensemble Weighted Mean";
    break;
  case 2:
    ensemble = "Ensemble Standard Deviation";
    break;
  case 3:
    ensemble = "Ensemble Standard Deviation Normalized";
    break;
  case 4:
    ensemble = "Ensemble Spread";
    break;
  case 5:
    ensemble = "Ensemble Large Anomaly";
    break;
  case 6:	  
    ensemble = "Ensemble Mean";
    break;
  case 7:	  
    ensemble = "Ensemble Interquartile Range";
    break;
  case 8:	  
    ensemble = "Ensemble Minimum";
    break;
  case 9:	  
    ensemble = "Ensemble Maximum";
    break;
  case 192:
    ensemble = "Unweighted Mode";
    break;
  case 193:
    ensemble = "Percentile value (10%)";
    break;
  case 194:
    ensemble = "Percentile value (50%)";
    break;
  case 195:
    ensemble = "Percentile value (90%)";
    break;
  case 196:
    ensemble = "Statistically decided weights for each ensemble member";
    break;
  case 197:
    ensemble = "Climate Percentile";
    break;
  default:
    ensemble = "";
    break;
  }
  return ensemble;
}

//
// GRIB2 - CODE TABLE 4.9
// PROBABILITY TYPE
void ProdDefTemp::_printProbabilityType(FILE *stream, int probId) const
{
  fprintf(stream, "Probability Type: \n");
  switch( probId ) {
  case 0:
    fprintf(stream, "        Probability of event below lower limit\n");
    break;
  case 1:
    fprintf(stream, "        Probability of event above upper limit\n");
    break;
  case 2:
    fprintf(stream, "        Probability of event between upper and lower limits (range includes lower limit but no the upper limit)\n");
    break;
  case 3:
    fprintf(stream, "        Probability of event above lower limit\n");
    break;
  case 4:
    fprintf(stream, "        Probability of event below upper limit\n");
    break;
  default:
    fprintf(stream, "        Unknown Probability type\n");
    break;
  }
}

//
// GRIB2 - CODE TABLE 4.10
// TYPE OF STATISTICAL PROCESSING 
void ProdDefTemp::_printStatisticalProcess(FILE *stream, int processId) const
{
  fprintf(stream, "Statistical process: ");
  switch( processId ) {
  case 0:
    fprintf(stream, "Average\n");
    break;
  case 1:
    fprintf(stream, "Accumulation\n");
    break;
  case 2:
    fprintf(stream, "Maximum\n");
    break;
  case 3:
    fprintf(stream, "Minimum\n");
    break;
  case 4:
    fprintf(stream, "Difference (value at the end of the time range minus value at the beginning)\n");
    break;
  case 5:
    fprintf(stream, "Root Mean Square\n");
    break;
  case 6:
    fprintf(stream, "Standard Deviation\n");
    break;
  case 7:
    fprintf(stream, "Covariance (temporal variance)\n");
    break;
  case 8:
    fprintf(stream, "Difference (value at the beginning of the time range minus value at the end)\n");
    break;
  case 9:
    fprintf(stream, "Ratio\n");
    break;
  case 10:
     fprintf(stream, "Standardized Anomaly\n");
     break;
  case 11:
     fprintf(stream, "Summation\n");
     break;
  case 12:
     fprintf(stream, "Confidence Index\n");
     break;
  case 13:
     fprintf(stream, "Quality Indicator\n");
     break;
  case 192:
     fprintf(stream, "Climatological Mean Value\n");
     break;
    case 193:
     fprintf(stream, "Average of forecasts or initialized analyses\n");
     break;
    case 194:
     fprintf(stream, "Average of uninitialized analyses\n");
     break;
    case 195:
     fprintf(stream, "Average of forecast accumulations\n");
     break;
    case 196:
     fprintf(stream, "Average of successive forecast accumulations\n");
     break;
    case 197:
     fprintf(stream, "Average of forecast averages\n");
     break;
    case 198:
     fprintf(stream, "Average of successive forecast averages\n");
     break;
    case 199:
     fprintf(stream, "Climatological Average of N analyses, each a year apart\n");
     break;
    case 200:
     fprintf(stream, "Climatological Average of N forecasts, each a year apart\n");
     break;
    case 201:
     fprintf(stream, "Climatological Root Mean Square difference between N forecasts and their verifying analyses, each a year apart\n");
     break;
    case 202:
     fprintf(stream, "Climatological Standard Deviation of N forecasts from the mean of the same N forecasts, for forecasts one year apart\n");
     break;
    case 203:
     fprintf(stream, "Climatological Standard Deviation of N analyses from the mean of the same N analyses, for analyses one year apart\n");
     break;
    case 204:
     fprintf(stream, "Average of forecast accumulations at 6-hour intervals\n");
     break;
    case 205:
     fprintf(stream, "Average of forecast averages at 6-hour intervals\n");
     break;
    case 206:
     fprintf(stream, "Average of forecast accumulations at 12-hour intervals\n");
     break;
    case 207:
     fprintf(stream, "Average of forecast averages at 12-hour intervals\n");
     break;
  default:
    fprintf(stream, "Unknown\n");
    break;
  }
}

string ProdDefTemp::_getStatisticalProcess(int processId) const
{
  string process;
  switch( processId ) {
  case 0:
  case 193:
  case 194:
  case 195:
  case 196:
  case 197:
  case 198:
  case 199:
  case 200:
  case 204:
  case 205:
  case 206:
  case 207:
    process = "_AVG";
    break;
  case 1:  // Do not append Accumulation to var name
    process = "";
    break;
  case 2:
    process = "_MAX";
    break;
  case 3:
    process = "_MIN";
    break;
  case 4:
 case 8:
    process = "_DIFF";
    break;
  case 5:
 case 201:
    process = "_RMS";
    break;
  case 6:
 case 202:
 case 203:
    process = "_SDEV";
    break;
  case 7:
    process = "_COV";
    break;
  case 9:
    process = "_RATIO";
    break;
  case 10:
     process = "_ANOM";
     break;
  case 11:
     process = "_SUM";
     break;
  case 12:
     process = "_CONF";
     break;
  case 13:
     process = "_QUAL";
     break;
  case 192:
     process = "_MEAN";
     break;
  default:
    process = "";
    break;
  }
  return process;
}

//
// GRIB2 - CODE TABLE 4.15
// TYPE OF SPATIAL PROCESSING USED TO ARRIVE AT A GIVEN
// DATA VALUE FROM THE SOURCE DATA
void ProdDefTemp::_printSpatialProcess(FILE *stream, int processId) const
{
  fprintf(stream, "Type of spatial interpolation: \n");
  switch( processId ) {
  case 0:
    fprintf(stream, "      Data is calculated directly from the source grid with no interpolation\n");
    break;
  case 1:
    fprintf(stream, "      Bilinear interpolation using the 4 source grid grid-point values surrounding the nominal grid-point\n");
    break;
  case 2:
    fprintf(stream, "      Bicubic interpolation using the 4 source grid grid-point values surrounding the nominal grid-point\n");
    break;
  case 3:
    fprintf(stream, "      Using the value from the source grid grid-point which is nearest to the nominal grid-point\n");
    break;
  case 4:
    fprintf(stream, "      Budget interpolation using the 4 source grid grid-point values surrounding the nominal grid-point\n");
    break;
  case 5:
    fprintf(stream, "      Spectral interpolation using the 4 source grid grid-point values surrounding the nominal grid-point\n");
    break;
  case 6:
    fprintf(stream, "      Neighbor-budget interpolation using the 4 source grid grid-point values surrounding the nominal grid-point\n");
    break;
  default:
    fprintf(stream, "      Unknown\n");
    break;
  }
}

//
// ON388 - TABLE A
// Generating Process or Model from Originating Center 7 (US-NWS, NCEP)
string ProdDefTemp::getGeneratingProcess() const
{
  string process;
  switch( _processID ) {
  case 02:
    process = "Ultra Violet Index Model";
    break;
  case 03:
    process = "NCEP/ARL Transport and Dispersion Model";
    break;
  case 04:
    process = "NCEP/ARL Smoke Model";
    break;
  case 05:
    process = "Satellite Derived Precipitation and temperatures, from IR";
    break;
  case 06:
    process = "NCEP/ARL Dust Model";
    break;
  case 10:
    process = "Global Wind-Wave Forecast Model";
    break;
  case 11:
    process = "Global Multi-Grid Wave Model (Static Grids)";
    break;
  case 12:
    process = "Probabilistic Storm Surge";
    break;
  case 13:
    process = "Hurricane Multi-Grid Wave Model";
    break;
  case 14:
    process = "Extra-tropical Storm Surge Atlantic Domain";
    break;
  case 15:
    process = "Nearshore Wave Prediction System (NWPS)";
    break;
  case 17:
    process = "Extra-tropical Storm Surge Pacific Domain";
    break;
  case 19:
    process = "Limited-area Fine Mesh (LFM) analysis";
    break;
  case 25:
    process = "Snow Cover Analysis";
    break;
  case 30:
    process = "Forecaster generated field";
    break;
  case 31:
    process = "Value added post processed field";
    break;
  case 42:
    process = "Global Optimum Interpolation Analysis (GOI) from GFS model";
    break;
  case 43:
    process = "Global Optimum Interpolation Analysis (GOI) from Final run ";
    break;
  case 44:
    process = "Sea Surface Temperature Analysis";
    break;
  case 45:
    process = "Coastal Ocean Circulation Model";
    break;
  case 46:
    process = "HYCOM - Global";
    break;
  case 47:
    process = "HYCOM - North Pacific basin";
    break;
  case 48:
    process = "HYCOM - North Atlantic basin";
    break;
  case 49:
    process = "Ozone Analysis from TIROS Observations ";
    break;
  case 52:
    process = "Ozone Analysis from Nimbus 7 Observations ";
    break;
  case 53:
    process = "LFM-Fourth Order Forecast Model";
    break;
  case 64:
    process = "Regional Optimum Interpolation Analysis (ROI)";
    break;
  case 68:
    process = "80 wave triangular, 18-layer Spectral model from GFS model";
    break;
  case 69:
    process = "80 wave triangular, 18 layer Spectral model from Medium Range Forecast run";
    break;
  case 70:
    process = "Quasi-Lagrangian Hurricane Model (QLM)";
    break;
  case 73:
    process = "Fog Forecast model - Ocean Prod. Center";
    break;
  case 74:
    process = "Gulf of Mexico Wind/Wave";
    break;
  case 75:
    process = "Gulf of Alaska Wind/Wave";
    break;
  case 76:
    process = "Bias corrected Medium Range Forecast";
    break;
  case 77:
    process = "126 wave triangular, 28 layer Spectral model from GFS model";
    break;
  case 78:
    process = "126 wave triangular, 28 layer Spectral model from Medium Range Forecast run";
    break;
  case 79:
    process = "Backup from the previous run";
    break;
  case 80:
    process = "62 wave triangular, 28 layer Spectral model from Medium Range Forecast run";
    break;
  case 81:
    process = "Analysis from GFS (Global Forecast System)";
    break;
  case 82:
    process = "Analysis from GDAS (Global Data Assimilation System)";
    break;
  case 83:
    process = "High Resolution Rapid Refresh (HRRR)";
    break;
  case 84:
    process = "MESO NAM Model";
    break;
  case 85:
    process = "Real Time Ocean Forecast System (RTOFS)";
    break;
  case 87:
    process = "CAC Ensemble Forecasts from Spectral (ENSMB)";
    break;
  case 88:
    process = "NOAA Wave Watch III (NWW3) Ocean Wave Model";
    break;
  case 89:
    process = "Non-hydrostatic Meso Model (NMM) (Currently 8 km)";
    break;
  case 90:
    process = "62 wave triangular, 28 layer spectral model extension of the Medium Range Forecast run";
    break;
  case 91:
    process = "62 wave triangular, 28 layer spectral model extension of the GFS model";
    break;
  case 92:
    process = "62 wave triangular, 28 layer spectral model run from the Medium Range Forecast final analysis";
    break;
  case 93:
    process = "62 wave triangular, 28 layer spectral model run from the T62 GDAS analysis of the Medium Range Forecast run";
    break;
  case 94:
    process = "T170/L42 Global Spectral Model from MRF run";
    break;
  case 95:
    process = "T126/L42 Global Spectral Model from MRF run";
    break;
  case 96:
    process = "Global Forecast System Model";
    break;
  case 98:
    process = "Climate Forecast System Model -- Atmospheric model (GFS) coupled to a multi level ocean model";
    break;
  case 105:
    process = "Rapid Refresh (RAP)";
    break;
  case 107:
    process = "Global Ensemble Forecast System (GEFS)";
    break;
  case 108:
    process = "LAMP";
    break;
  case 109:
    process = "RTMA (Real Time Mesoscale Analysis)";
    break;
  case 110:
    process = "NAM Model - 15km version";
    break;
  case 111:
    process = "NAM model, generic resolution (Used in SREF processing)";
    break;
  case 112:
    process = "WRF-NMM model, generic resolution (Used in various runs) NMM=Nondydrostatic Mesoscale Model (NCEP)";
    break;
  case 113:
    process = "Products from NCEP SREF processing";
    break;
  case 114:
    process = "NAEFS Products from joined NCEP, CMC global ensembles";
    break;
  case 115:
    process = "Downscaled GFS from NAM eXtension";
    break;
  case 116:
    process = "WRF-EM model, generic resolution (Used in various runs) EM - Eulerian Mass-core (NCAR - aka Advanced Research WRF)";
    break;
  case 117:
    process = "NEMS GFS Aerosol Component";
    break;
  case 118:
    process = "URMA (UnRestricted Mesoscale Analysis)";
    break;
  case 119:
    process = "WAM (Whole Atmosphere Model)";
    break;
  case 120:
    process = "Ice Concentration Analysis";
    break;
  case 121:
    process = "Western North Atlantic Regional Wave Model";
    break;
  case 122:
    process = "Alaska Waters Regional Wave Model";
    break;
  case 123:
    process = "North Atlantic Hurricane Wave Model";
    break;
  case 124:
    process = "Eastern North Pacific Regional Wave Model";
    break;
  case 125:
    process = "North Pacific Hurricane Wave Model";
    break;
  case 126:
    process = "Sea Ice Forecast Model";
    break;
  case 127:
    process = "Lake Ice Forecast Model";
    break;
  case 128:
    process = "Global Ocean Forecast Model";
    break;
  case 129:
    process = "Global Ocean Data Analysis System (GODAS)";
    break;
  case 130:
    process = "Merge of fields from the RUC, NAM, and Spectral Model ";
    break;
  case 131:
    process = "Great Lakes Wave Model";
    break;
  case 140:
    process = "North American Regional Reanalysis (NARR)";
    break;
  case 141:
    process = "Land Data Assimilation and Forecast System";
    break;
  case 150:
    process = "NWS River Forecast System (NWSRFS)";
    break;
  case 151:
    process = "NWS Flash Flood Guidance System (NWSFFGS)";
    break;
  case 152:
    process = "WSR-88D Stage II Precipitation Analysis";
    break;
  case 153:
    process = "WSR-88D Stage III Precipitation Analysis";
    break;
  case 180:
    process = "Quantitative Precipitation Forecast generated by NCEP";
    break;
  case 181:
    process = "River Forecast Center Quantitative Precipitation Forecast mosaic generated by NCEP";
    break;
  case 182:
    process = "River Forecast Center Quantitative Precipitation estimate mosaic generated by NCEP";
    break;
  case 183:
    process = "NDFD product generated by NCEP/HPC";
    break;
  case 184:
    process = "Climatological Calibrated Precipitation Analysis - CCPA";
    break;
  case 190:
    process = "National Convective Weather Diagnostic generated by NCEP/AWC";
    break;
  case 191:
    process = "Current Icing Potential automated product genterated by NCEP/AWC";
    break;
  case 192:
    process = "Analysis product from NCEP/AWC";
    break;
  case 193:
    process = "Forecast product from NCEP/AWC";
    break;
  case 195:
    process = "Climate Data Assimilation System 2 (CDAS2)";
    break;
  case 196:
    process = "Climate Data Assimilation System 2 (CDAS2) - used for regeneration runs";
    break;
  case 197:
    process = "Climate Data Assimilation System (CDAS)";
    break;
  case 198:
    process = "Climate Data Assimilation System (CDAS) - used for regeneration runs";
    break;
  case 199:
    process = "Climate Forecast System Reanalysis (CFSR) -- Atmospheric model (GFS) coupled to a multi level ocean, land and seaice model";
    break;
  case 200:
    process = "CPC Manual Forecast Product";
    break;
  case 201:
    process = "CPC Automated Product";
    break;
  case 210:
    process = "EPA Air Quality Forecast - Currently North East US domain";
    break;
  case 211:
    process = "EPA Air Quality Forecast - Currently Eastern US domain";
    break;
  case 215:
    process = "SPC Manual Forecast Product";
    break;
  case 220:
    process = "NCEP/OPC automated product";
    break;
  default:
    process = "Unknown";
    break;
  }
  return process;
}

} // namespace Grib2

