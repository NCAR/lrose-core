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

#include <vector>
#include "Server.hh"
#include "ConvertUtil.hh"
#include "ConvertLocation.hh"
#include "tinyxml/tinyxml.h"
using namespace std;


//======================================================================

// Definitions

class ConvertLocation {
  public:

  double analysisYear;
  double analysisMonth;
  double analysisDay;
  double analysisHour;
  double analysisMinute;

  double forecastYear;
  double forecastMonth;
  double forecastDay;
  double forecastHour;
  double forecastMinute;

  double minHeightMeters;
  double maxHeightMeters;
}


ConvertLocation::ConvertLocation(
  int bugs,
  TiXmlElement ** p_cursor)          // updated
{
  TiXmlElement * cursor = *p_cursor;


  TiXmlElement * originCenter = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * analysisSignif = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * analysisYear = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * analysisMonth = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * analysisDay = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * analysisHour = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * analysisMinute = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * forecastSignif = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * forecastYear = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * forecastMonth = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * forecastDay = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * forecastHour = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * forecastMinute = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * minHeight = cursor;
  cursor = cursor->NextSiblingElement();

  TiXmlElement * maxHeight = cursor;
  cursor = cursor->NextSiblingElement();
  *p_cursor = cursor;

  if (bugs >= 5) {
    printNode("convertLocation: originCenter", originCenter);

    printNode("convertLocation: analysisSignif", analysisSignif);
    printNode("convertLocation: analysisYear", analysisYear);
    printNode("convertLocation: analysisMonth", analysisMonth);
    printNode("convertLocation: analysisDay", analysisDay);
    printNode("convertLocation: analysisHour", analysisHour);
    printNode("convertLocation: analysisMinute", analysisMinute);

    printNode("convertLocation: forecastSignif", forecastSignif);
    printNode("convertLocation: forecastYear", forecastYear);
    printNode("convertLocation: forecastMonth", forecastMonth);
    printNode("convertLocation: forecastDay", forecastDay);
    printNode("convertLocation: forecastHour", forecastHour);
    printNode("convertLocation: forecastMinute", forecastMinute);

    printNode("convertLocation: minHeightMeters", minHeightMeters);
    printNode("convertLocation: maxHeightMeters", maxHeightMeters);
  }
}








//======================================================================
