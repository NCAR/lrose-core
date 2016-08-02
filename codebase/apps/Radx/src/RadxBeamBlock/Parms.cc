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

/**
 * @file Parms.cc
 */
#include "Parms.hh"
#include <Mdv/MdvxProj.hh>
#include <toolsa/DateTime.hh>

//------------------------------------------------------------------
Parms::Parms(const Params &p, const std::string &progName) : Params(p), 
							     _progName(progName)
{
  DateTime dt(_time[0], _time[1], _time[2], _time[3], _time[4], _time[5]);
  _utime = dt.utime();
}

//------------------------------------------------------------------
Parms::~Parms(void)
{
}

//------------------------------------------------------------------
void Parms::latlonExtrema(std::pair<double,double> &sw,
			  std::pair<double,double> &ne) const
{
  double lat0 = radar_location.latitudeDeg;
  double lon0 = radar_location.longitudeDeg;

  MdvxProj proj;
  proj.initPolarRadar(lat0, lon0);
  
  sw.first = lat0;
  sw.second = lon0;
  ne = sw;

  double maxR = ithGate(ngates()-1);
  for (int i=0; i<azimuths.count; ++i)
  {
    // ignore elevation for now
    double a = ithAzimuth(i);
    double lati, loni;
    proj.xy2latlon(maxR, a, lati, loni);
    
    if (lati < sw.first)
    {
      sw.first = lati;
    }

    if (loni < sw.second)
    {
      sw.second = loni;
    }
    if (lati > ne.first)
    {
      ne.first = lati;
    }

    if (loni > ne.second)
    {
      ne.second = loni;
    }
  }
}

//------------------------------------------------------------------
std::string Parms::fieldName(Params::output_data_t t) const
{
  std::string ret = "";

  for (int i=0; i<output_fields_n; ++i)
  {
    if (_output_fields[i].type == t)
    {
      ret = _output_fields[i].name;
      return ret;
    }
  }
  printf("ERROR finding field\n");
  return ret;
}
