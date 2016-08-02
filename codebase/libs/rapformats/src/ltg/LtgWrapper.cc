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
// LtgWrapper.cc
//
// C++ wrapper class for ltg.h structs.
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Dec 2009
//////////////////////////////////////////////////////////////

#include <string>
#include <rapformats/LtgWrapper.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/TaXml.hh>
using namespace std;

////////////////////////////////
// print original strike struct

void LtgWrapper::print(FILE *out, const LTG_strike_t &strike)
{
  LTG_print_strike(out, &strike);
}

////////////////////////////////
// print extended strike struct

void LtgWrapper::print(FILE *out, const LTG_extended_t &strike)
{
  LTG_print_extended(out, &strike);
}

//////////////////////////////////////
// print original strike struct as XML

void LtgWrapper::printAsXml(ostream &out, const LTG_strike_t &strike,
                            int startIndentLevel)
{
  
  string xml = loadXml(strike, startIndentLevel);
  out << xml;

}

//////////////////////////////////////
// print extended strike struct as XML

void LtgWrapper::printAsXml(ostream &out, const LTG_extended_t &strike,
                            int startIndentLevel)
{

  string xml = loadXml(strike, startIndentLevel);
  out << xml;

}

//////////////////////////////////////
// load original strike struct as XML

string LtgWrapper::loadXml(const LTG_strike_t &strike,
                           int startIndentLevel)
{

  string xml;
  int sil = startIndentLevel;

  xml += TaXml::writeStartTag("ltg", sil+0);

  xml += TaXml::writeTime("time", sil+1 , strike.time);
  xml += TaXml::writeDouble("latitude", sil+1 , strike.latitude);
  xml += TaXml::writeDouble("longitude", sil+1 , strike.longitude);
  if (strike.amplitude != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("amplitude", sil+1 , strike.amplitude);
  }
  switch (strike.type) {
    case LTG_GROUND_STROKE:
      xml += TaXml::writeString("type", sil+1 , "cloud_to_ground");
      break;
    case LTG_CLOUD_STROKE:
      xml += TaXml::writeString("type", sil+1 , "cloud_to_cloud");
      break;
    default: {}
  }

  xml += TaXml::writeEndTag("ltg", sil+0);

  return xml;

}

//////////////////////////////////////
// load extended strike struct as XML

string LtgWrapper::loadXml(const LTG_extended_t &strike,
                           int startIndentLevel)
{

  string xml;
  int sil = startIndentLevel;

  xml += TaXml::writeStartTag("ltg", sil+0);

  xml += TaXml::writeTime("time", sil+1, strike.time);
  if (strike.nanosecs != LTG_MISSING_INT) {
    xml += TaXml::writeInt("nanosecs", sil+1, strike.nanosecs);
  }
  xml += TaXml::writeDouble("latitude", sil+1, strike.latitude);
  xml += TaXml::writeDouble("longitude", sil+1, strike.longitude);
  if (strike.altitude != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("altitude", sil+1, strike.altitude);
  }
  if (strike.amplitude != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("amplitude", sil+1, strike.amplitude);
  }
  switch (strike.type) {
    case LTG_GROUND_STROKE:
      xml += TaXml::writeString("type", sil+1, "cloud_to_ground");
      break;
    case LTG_CLOUD_STROKE:
      xml += TaXml::writeString("type", sil+1, "cloud_to_cloud");
      break;
    default: {}
  }

  if (strike.n_sensors != LTG_MISSING_INT) {
    xml += TaXml::writeInt("n_sensors", sil+1,
                              strike.n_sensors);
  }
  if (strike.degrees_freedom != LTG_MISSING_INT) {
    xml += TaXml::writeInt("degrees_freedom", sil+1,
                              strike.degrees_freedom);
  }

  if (strike.ellipse_angle != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("ellipse_angle", sil+1, 
                              strike.ellipse_angle);
  }
  if (strike.semi_major_axis != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("semi_major_axis", sil+1,
                              strike.semi_major_axis);
  }
  if (strike.semi_minor_axis != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("semi_minor_axis", sil+1,
                              strike.semi_minor_axis);
  }
  if (strike.chi_sq != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("chi_sq", sil+1, strike.chi_sq);
  }
  if (strike.rise_time != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("rise_time", sil+1, strike.rise_time);
  }
  if (strike.peak_to_zero_time != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("peak_to_zero_time", sil+1,
                              strike.peak_to_zero_time);
  }
  if (strike.max_rate_of_rise != LTG_MISSING_FLOAT) {
    xml += TaXml::writeDouble("max_rate_of_rise", sil+1,
                              strike.max_rate_of_rise);
  }

  if (strike.angle_flag != LTG_MISSING_INT) {
    xml += TaXml::writeInt("angle_flag", sil+1, strike.angle_flag);
  }
  if (strike.signal_flag != LTG_MISSING_INT) {
    xml += TaXml::writeInt("signal_flag", sil+1, strike.signal_flag);
  }
  if (strike.timing_flag != LTG_MISSING_INT) {
    xml += TaXml::writeInt("timing_flag", sil+1, strike.timing_flag);
  }

  xml += TaXml::writeEndTag("ltg", sil+0);

  return xml;

}

