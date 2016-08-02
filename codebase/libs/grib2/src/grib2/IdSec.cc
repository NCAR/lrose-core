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
////////////////////////////////////////////////////////////////////
//
// Based on documentation located:
//     http://www.nws.noaa.gov/tdl/iwt/grib2
//
// Gary Blackburn,  June 2004
//
// The Identification section is a header new to grib 2.  Most of
// the information is located in the PDS in grib 1
//
////////////////////////////////////////////////////////////////////

#include <grib2/IdSec.hh>
#include <toolsa/utim.h> // For unix_to_date
#include <iostream>

using namespace std;

namespace Grib2 {

/*          ON388 - TABLE 0                   */
/* NATIONAL/INTERNATIONAL ORIGINATING CENTERS */
/*         (Assigned By The WMO)              */
const string IdSec::_centers[256] = {
   /* 00 */ "",
   /* 01 */ "Melbourne",
   /* 02 */ "Melbourne",
   /* 03 */ "Melbourne",
   /* 04 */ "Moscow",
   /* 05 */ "Moscow",
   /* 06 */ "Moscow",
   /* 07 */ "US NWS - NCEP",
   /* 08 */ "US NWS - NWSTG",
   /* 09 */ "US NWS - Other",
   /* 10 */ "Cairo",
   /* 11 */ "Cairo",
   /* 12 */ "Dakar",
   /* 13 */ "Dakar",
   /* 14 */ "Nairobi",
   /* 15 */ "Nairobi",
   /* 16 */ "Casablanca",
   /* 17 */ "Tunis",
   /* 18 */ "Tunis-Casablanca",
   /* 19 */ "Tunis-Casablanca",
   /* 20 */ "Las Palmas",
   /* 21 */ "Algiers",
   /* 22 */ "ACMAD",
   /* 23 */ "Mozambique",
   /* 24 */ "Pretoria",
   /* 25 */ "La Reunion",
   /* 26 */ "Khabarovsk",
   /* 27 */ "Khabarovsk",
   /* 28 */ "New Delhi",
   /* 29 */ "New Delhi",
   /* 30 */ "Novosibirsk",
   /* 31 */ "Novosibirsk",
   /* 32 */ "Tashkent",
   /* 33 */ "Jeddah",
   /* 34 */ "Tokyo, Japanese Meteorological Agency",
   /* 35 */ "Tokyo, Japanese Meteorological Agency",
   /* 36 */ "Bankok",
   /* 37 */ "Ulan Bator",
   /* 38 */ "Beijing",
   /* 39 */ "Beijing",
   /* 40 */ "Seoul",
   /* 41 */ "Buenos Aires",
   /* 42 */ "Buenos Aires",
   /* 43 */ "Brasilia",
   /* 44 */ "Brasilia",
   /* 45 */ "Santiago",
   /* 46 */ "Brazilian Space Agency - INPE",
   /* 47 */ "Columbia",
   /* 48 */ "Ecuador",
   /* 49 */ "Peru",
   /* 50 */ "Venezuela",
   /* 51 */ "Miami",
   /* 52 */ "Miami, National Hurricane Center",
   /* 53 */ "Canadian Meteorological Service - Montreal",
   /* 54 */ "Canadian Meteorological Service - Montreal",
   /* 55 */ "San Francisco",
   /* 56 */ "ARINC Center",
   /* 57 */ "US Air Force - Air Force Global Weather Center",
   /* 58 */ "Fleet Numerical Meteorology and Oceanography Center,Monterey,CA,USA",
   /* 59 */ "The NOAA Forecast Systems Lab, Boulder, CO, USA",
   /* 60 */ "National Center for Atmospheric Research (NCAR), Boulder, CO",
   /* 61 */ "Service ARGOS - Landover, MD, USA",
   /* 62 */ "US Naval Oceanographic Office",
   /* 63 */ "International Research Institude for Climate and Society",
   /* 64 */ "Honolulu",
   /* 65 */ "Darwin",
   /* 66 */ "Darwin",
   /* 67 */ "Melbourne",
   /* 68 */ "Reserved",
   /* 69 */ "Wellington",
   /* 70 */ "Wellington",
   /* 71 */ "Nadi",
   /* 72 */ "Singapore",
   /* 73 */ "Malaysia",
   /* 74 */ "U.K. Met Office - Exeter",
   /* 75 */ "U.K. Met Office - Exeter",
   /* 76 */ "Moscow",
   /* 77 */ "Reserved",
   /* 78 */ "Offenbach",
   /* 79 */ "Offenbach",
   /* 80 */ "Rome",
   /* 81 */ "Rome",
   /* 82 */ "Norrkoping",
   /* 83 */ "Norrkoping",
   /* 84 */ "French Weather Service - Toulouse",
   /* 85 */ "French Weather Service - Toulouse",
   /* 86 */ "Helsinki",
   /* 87 */ "Belgrade",
   /* 88 */ "Oslo",
   /* 89 */ "Prague",
   /* 90 */ "Episkopi",
   /* 91 */ "Ankara",
   /* 92 */ "Frankfurt/Main",
   /* 93 */ "London (WAFC)",
   /* 94 */ "Copenhagen",
   /* 95 */ "Rota",
   /* 96 */ "Athens",
   /* 97 */ "European Space Agency (ESA)",
   /* 98 */ "European Center for Medium-Range Weather Forecasts",
   /* 99 */ "De Bilt, Netherlands",
   /* 100 */ "Brazzaville",
   /* 101 */ "Abidjan",
   /* 102 */ "Libyan Arab Jamahiriya",
   /* 103 */ "Madagascar",
   /* 104 */ "Mauritius",
   /* 105 */ "Niger",
   /* 106 */ "Seychelles",
   /* 107 */ "Uganda",
   /* 108 */ "United Republic of Tanzania",
   /* 109 */ "Zimbabwe",
   /* 110 */ "Hong-Kong",
   /* 111 */ "Afghanistan",
   /* 112 */ "Bahrain",
   /* 113 */ "Bangladesh",
   /* 114 */ "Bhutan",
   /* 115 */ "Cambodia",
   /* 116 */ "Democratic People's Republic of Korea",
   /* 117 */ "Islamic Republic of Iran",
   /* 118 */ "Iraq",
   /* 119 */ "Kazakhstan",
   /* 120 */ "Kuwait",
   /* 121 */ "Kyrgyz Republic",
   /* 122 */ "Lao People's Democratic Republic",
   /* 123 */ "Macao, China",
   /* 124 */ "Maldives",
   /* 125 */ "Myanmar",
   /* 126 */ "Nepal",
   /* 127 */ "Oman",
   /* 128 */ "Pakistan",
   /* 129 */ "Qatar",
   /* 130 */ "Yemen",
   /* 131 */ "Sri Lanka",
   /* 132 */ "Tajikistan",
   /* 133 */ "Turkmenistan",
   /* 134 */ "United Arab Emirates",
   /* 135 */ "Uzbekistan",
   /* 136 */ "Viet Nam",
   /* 137 */ "Reserved",
   /* 138 */ "Reserved",
   /* 139 */ "Reserved",
   /* 140 */ "Bolivia",
   /* 141 */ "Guyana",
   /* 142 */ "Paraguay",
   /* 143 */ "Suriname",
   /* 144 */ "Uruguay",
   /* 145 */ "French Guyana",
   /* 146 */ "Brazilian Navy Hydrographic Center",
   /* 147 */ "National Commission on Space Activities - Argentina",
   /* 148 */ "Reserved",
   /* 149 */ "Reserved",
   /* 150 */ "Antigua and Barbuda",
   /* 151 */ "Bahamas",
   /* 152 */ "Barbados",
   /* 153 */ "Belize",
   /* 154 */ "British Caribbean Territories Center",
   /* 155 */ "San Jose",
   /* 156 */ "Cuba",
   /* 157 */ "Dominica",
   /* 158 */ "Dominican Republic",
   /* 159 */ "El Salvador",
   /* 160 */ "US NOAA/NESDIS",
   /* 161 */ "US NOAA Office of Oceanic and Atmospheric Research",
   /* 162 */ "Guatemala",
   /* 163 */ "Haiti",
   /* 164 */ "Honduras",
   /* 165 */ "Jamaica",
   /* 166 */ "Mexico City",
   /* 167 */ "Netherlands Antilles and Aruba",
   /* 168 */ "Nicaragua",
   /* 169 */ "Panama",
   /* 170 */ "Saint Lucia",
   /* 171 */ "Trinidad and Tobago",
   /* 172 */ "French Departments in RA IV",
   /* 173 */ "US National Aeronautics and Space Administration (NASA)",
   /* 174 */ "Integrated System Data Management/Marine Environmental Data Service (ISDM/MEDS) - Canada",
   /* 175 */ "Reserved",
   /* 176 */ "US Cooperative Institude for Meteorological Satellite Studies",
   /* 177 */ "Reserved",
   /* 178 */ "Reserved",
   /* 179 */ "Reserved",
   /* 180 */ "Reserved",
   /* 181 */ "Reserved",
   /* 182 */ "Reserved",
   /* 183 */ "Reserved",
   /* 184 */ "Reserved",
   /* 185 */ "Reserved",
   /* 186 */ "Reserved",
   /* 187 */ "Reserved",
   /* 188 */ "Reserved",
   /* 189 */ "Reserved",
   /* 190 */ "Cook Islands",
   /* 191 */ "French Polynesia",
   /* 192 */ "Tonga",
   /* 193 */ "Vanuatu",
   /* 194 */ "Brunei",
   /* 195 */ "Indonesia",
   /* 196 */ "Kiribati",
   /* 197 */ "Federated States of Micronesia",
   /* 198 */ "New Caledonia",
   /* 199 */ "Niue",
   /* 200 */ "Papua New Guinea",
   /* 201 */ "Philippines",
   /* 202 */ "Samoa",
   /* 203 */ "Solomon Islands",
   /* 204 */ "Narional Institude of Water and Atmospheric Research - New Zealand",
   /* 205 */ "Reserved",
   /* 206 */ "Reserved",
   /* 207 */ "Reserved",
   /* 208 */ "Reserved",
   /* 209 */ "Reserved",
   /* 210 */ "Frascati (ESA/ESRIN)",
   /* 211 */ "Lanion",
   /* 212 */ "Lisbon",
   /* 213 */ "Reykjavik",
   /* 214 */ "Madrid",
   /* 215 */ "Zurich",
   /* 216 */ "Service ARGOS - Toulouse",
   /* 217 */ "Bratislava",
   /* 218 */ "Budapest",
   /* 219 */ "Ljubljana",
   /* 220 */ "Warsaw",
   /* 221 */ "Zagreb",
   /* 222 */ "Albania",
   /* 223 */ "Armenia",
   /* 224 */ "Austria",
   /* 225 */ "Azerbaijan",
   /* 226 */ "Belarus",
   /* 227 */ "Belgium",
   /* 228 */ "Bosnia and Herzegovina",
   /* 229 */ "Bulgaria",
   /* 230 */ "Cyprus",
   /* 231 */ "Estonia",
   /* 232 */ "Georgia",
   /* 233 */ "Dublin",
   /* 234 */ "Israel",
   /* 235 */ "Jordan",
   /* 236 */ "Latvia",
   /* 237 */ "Lebanon",
   /* 238 */ "Lithuania",
   /* 239 */ "Luxembourg",
   /* 240 */ "Malta",
   /* 241 */ "Monaco",
   /* 242 */ "Romania",
   /* 243 */ "Syrian Arab Republic",
   /* 244 */ "The former Yugoslav Republic of Macedonia",
   /* 245 */ "Ukraine",
   /* 246 */ "Republic of Moldova",
   /* 247 */ "Operational Programme for the Exchange of Weather RAdar Information (OPERA) - EUMETNET",
   /* 248 */ "Reserved",
   /* 249 */ "Reserved",
   /* 250 */ "COnsortium for Small scale MOdelling (COSMO)",
   /* 251 */ "Reserved",
   /* 252 */ "Reserved",
   /* 253 */ "Reserved",
   /* 254 */ "EUMETSAT Operations Center",
   /* 255 */ ""
};

const IdSec::_GRIB2SubCenter IdSec::_subCenters[] = {
  {7, 1, "NCEP Re-Analysis Project"},
  {7, 2, "NCEP Ensemble Products"},
  {7, 3, "NCEP Central Operations"},
  {7, 4, "Environmental Modeling Center"},
  {7, 5, "Hydrometeorological Prediction Center"},
  {7, 6, "Marine Prediction Center"},
  {7, 7, "Climate Prediction Center"},
  {7, 8, "Aviation Weather Center"},
  {7, 9, "Storm Prediction Center"},
  {7, 10, "National Hurricane Prediction Center"},
  {7, 11, "NWS Techniques Development Laboratory"},
  {7, 12, "NESDIS Office of Research and Applications"},
  {7, 13, "Federal Aviation Administration"},
  {7, 14, "NWS Meteorological Development Laboratory"},
  {7, 15, "North American Regional Reanalysis Project"},
  {7, 16, "Space Weather Prediction Center"}
};

IdSec::IdSec() : 
  GribSection()
{
  _sectionNum = 1;
  _sectionLen = 21;
}

int IdSec::pack( ui08 *idPtr ) 
{
  _pkUnsigned4(_sectionLen, &(idPtr[0]));

  idPtr[4] = (ui08) _sectionNum;

  _pkUnsigned2(_generatingCenter, &(idPtr[5]));

  _pkUnsigned2(_subCenter, &(idPtr[7]));

  idPtr[9] = (ui08) _masterTableVer;

  idPtr[10] = (ui08) _localTableVer;

  idPtr[11] = (ui08) _referenceTimeSig;

  _pkUnsigned2(_year, &(idPtr[12]));

  idPtr[14] = (ui08) _month;

  idPtr[15] = (ui08) _day;

  idPtr[16] = (ui08) _hour;

  idPtr[17] = (ui08) _min;

  idPtr[18] = (ui08) _sec;

  idPtr[19] = (ui08) _productionStatus;

  idPtr[20] = (ui08) _proccesedDataType;

   return (GRIB_SUCCESS );
}

int IdSec::unpack( ui08 *idPtr ) 
{
   
   // bytes 1-4 -> Length of section (21 or nn)
   _sectionLen = _upkUnsigned4 (idPtr[0], idPtr[1], idPtr[2], idPtr[3]);

   _sectionNum = (int) idPtr[4];

   if (_sectionNum != 1) {
    cerr << "ERROR: IdSec::unpack()" << endl;
     cerr << "Detecting incorrect section number, should be 1 but found section "
           << _sectionNum << endl;
     return( GRIB_FAILURE );
   }

   //
   // Identification of originating/generating centre
   //
   _generatingCenter = (int) _upkUnsigned2 (idPtr[5], idPtr[6]);

   //
   // Identification of originating/generating sub-centre
   //
   _subCenter = (int) _upkUnsigned2 (idPtr[7], idPtr[8]);

   //
   // GRIB Master Tables Version Number 
   //
   _masterTableVer = (int) idPtr[9];

   //
   // GRIB Local Tables Version Number
   //
   _localTableVer = (int) idPtr[10];

   //
   // Significance of Reference Time 
   //
   _referenceTimeSig = (int) idPtr[11];

   //
   // Date and time
   //
   _year = (int) _upkUnsigned2(idPtr[12], idPtr[13]);

   _month = (int) idPtr[14];
   _day = (int) idPtr[15];
   _hour = (int) idPtr[16];
   _min = (int) idPtr[17];
   _sec = (int) idPtr[18];

   //
   // Production status of processed data in this GRIB message
   //
   _productionStatus = (int) idPtr[19];

   //
   // Type of processed data in this GRIB message
   //
   _proccesedDataType = (int) idPtr[20];

   // Bytes from 22 to nn are reserved and need not be present

   return (GRIB_SUCCESS );
}

string IdSec::getGeneratingCenterName()
{
  string name = "";
  if(_generatingCenter > 0 && _generatingCenter < 255) 
  {
    name = _centers[_generatingCenter];
    for (ui32 i = 0; i < _subCenters_numElements; i++ ) {
      if(_subCenters[i].center == _generatingCenter &&
	 _subCenters[i].subCenter == _subCenter) {
	name.append(" - ");
	name.append(_subCenters[i].name);
      }
    }
  }
  return name;
}

void IdSec::setGenerateTime(time_t generateTime)
{
  UTIMstruct tmpTimePtr;
  UTIMunix_to_date(generateTime, &tmpTimePtr);
  _year = tmpTimePtr.year;
  _month = tmpTimePtr.month;
  _day = tmpTimePtr.day;
  _hour = tmpTimePtr.hour;
  _min = tmpTimePtr.min;
  _sec = tmpTimePtr.sec;
}

time_t IdSec::getGenerateTime() const
{
   UTIMstruct genTime;
   
   genTime.year = _year;
   genTime.month = _month;
   genTime.day = _day;
   genTime.hour = _hour;
   genTime.min = _min;
   genTime.sec = _sec;
   
   return( UTIMdate_to_unix( &genTime ) );
}

void
IdSec::print (FILE *stream)  {

  fprintf(stream, "\n\n");
  fprintf(stream, "Identification section:\n");
  fprintf(stream, "----------------------------------------------------\n");
  fprintf(stream, "Section Length %d\n", _sectionLen);
  fprintf(stream, "Section number %d\n", _sectionNum);
  fprintf(stream, "Originating/Generating centre Id %d\n", _generatingCenter);
  fprintf(stream, "Originating/Generating sub-centre Id %d\n", _subCenter);
  fprintf(stream, "      name: '%s'\n", getGeneratingCenterName().c_str());
  fprintf(stream, "GRIB Master Tables Version Number Id %d\n", _masterTableVer);
  if (_masterTableVer == 0)
    fprintf(stream, "      Experimental\n");
  else 
    if (_masterTableVer == 255)
      fprintf(stream, "      Local table used\n");
      
  fprintf(stream, "GRIB Local Tables Version Number Id %d\n", _localTableVer);
  fprintf(stream, "Significance of Reference Time %d\n", _referenceTimeSig);
  switch (_referenceTimeSig) {
    case 0:
      fprintf(stream, "      Analysis\n");
      break;
    case 1:
      fprintf(stream, "      Start of forecast\n");
      break;
    case 2:
      fprintf(stream, "      Verifying time of forecast\n");
      break;
    case 3:
      fprintf(stream, "      Observation time\n");
      break;
  }

  fprintf(stream, "Year %d\n", _year);
  fprintf(stream, "Month %d\n", _month);
  fprintf(stream, "Day %d\n", _day);
  fprintf(stream, "Hour %d\n", _hour);
  fprintf(stream, "Minute %d\n", _min);
  fprintf(stream, "Second %d\n", _sec);
  fprintf(stream, "Production status of processed data %d\n", _productionStatus);
  switch (_productionStatus) {
    case 0:
      fprintf(stream, "      Operational products\n");
      break;
    case 1:
      fprintf(stream, "      Operational test products \n");
      break;
    case 2:
      fprintf(stream, "      Research products\n");
      break;
    case 3:
      fprintf(stream, "      Re-analysis products\n");
      break;
  }
  fprintf(stream, "Type of processed data in this GRIB message %d\n", _proccesedDataType);
  switch (_proccesedDataType) {
    case 0:
      fprintf(stream, "      Analysis products\n");
      break;
    case 1:
      fprintf(stream, "      Forecast products\n");
      break;
    case 2:
      fprintf(stream, "      Analysis and forecast products\n");
      break;
    case 3:
      fprintf(stream, "      Control forecast products\n");
      break;
    case 4:
      fprintf(stream, "      Perturbed forecast products\n");
      break;
    case 5:
      fprintf(stream, "      Control and perturbed forecast products\n");
      break;
    case 6:
      fprintf(stream, "      Processed satellite observations\n");
      break;
    case 7:
      fprintf(stream, "      Processed radar observations\n");
      break;
  }
}

} // namespace

