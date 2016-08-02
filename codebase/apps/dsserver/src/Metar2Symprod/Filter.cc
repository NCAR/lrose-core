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
/////////////////////////////////////////////////////////////
// Filter.cc
//
// Abstract base class for filter objects.  Employed by the server
// to restrict which metars are returned and assign colors.
// The filters are setup via the auxillary XML.
//
// Paul Prestopnik, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2009
//
///////////////////////////////////////////////////////////////
#include "Filter.hh"
#include "SimpleFilter.hh"
#include "SequenceFilter.hh"


// Example XML for SimpleFilter:
/*
<ogc:Filter>

<!-- valid field names are:
    visibilityKm
    ceilingKm
    temperatureC
    rhPercent
    rvrKm
    windSpeedMps -->

 <ogc:PropertyIsGreaterThan>
   <ogc:PropertyName>temperatureC</ogc:PropertyName>
   <ogc:Literal>26.5</ogc:Literal>
 </ogc:PropertyIsGreaterThan>

</ogc:Filter>
*/

// Example XML for SequenceFilter:
/*
  <ogc:Sequence>
	<ogc:Filter color="#FF0000">
	    <ogc:PropertyIsGreaterThan>
	       <ogc:PropertyName>temperatureC</ogc:PropertyName>
	       <ogc:Literal>7.5</ogc:Literal>
	     </ogc:PropertyIsGreaterThan>
	</ogc:Filter>     

	<ogc:Filter color="#00FF00">
	    <ogc:PropertyIsGreaterThan>
	       <ogc:PropertyName>visibilityM</ogc:PropertyName>
	       <ogc:Literal>2000</ogc:Literal>
	     </ogc:PropertyIsGreaterThan>
	</ogc:Filter>     

	<ogc:Filter color="#0000FF">
	</ogc:Filter>     
  </ogc:Sequence>
*/

Filter* Filter::getCorrectFilter(string XML, Params* params)
{
  if (strstr(XML.c_str(),"Sequence"))
    {
    cerr << "Using SequenceFilter\n";
    return new SequenceFilter(params);
    }
  else
    {
    cerr << "Using SimpleFilter\n";
    return new SimpleFilter(params);
    }
}

Filter::Filter(Params* params)
{
  _latestObsColorString = "";
  _isDebug = params->debug >= Params::DEBUG_NORM;
  _isVerbose = params->debug >=  Params::DEBUG_VERBOSE;
  
}

const char* 
Filter::getColor() 
{ return _latestObsColorString.c_str();}


Filter::~Filter()
{
}
