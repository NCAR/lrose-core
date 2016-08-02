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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Amdar.cc,v 1.15 2016/03/03 18:45:39 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	Amdar
//
// Author:	G. M. Cunning
//
// Date:	Sun Oct 19 09:55:23 2008
//
// Description: This class manages AMDAR SPDB messages.
//
//


// C++ include files

// System/RAP include files
#include <rapformats/Amdar.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>

// Local include files

using namespace std;

// define any constants
const string Amdar::_className    = "Amdar";
const float Amdar::MISSING_VALUE = -9999.0;


/////////////////////////////////////////////////////////////////////////
// Constructors
//

Amdar::Amdar()
{
  clear();
}

Amdar::Amdar(const ascii_std_amdar_bulletin_t& ascii_bulletin)
{
  _clearAsciiBulletin();
  _asciiBulletin = ascii_bulletin;
  _isAscii = true;
  _isBufr = false;

}

Amdar::Amdar(const bufr_std_amdar_bulletin_t& bufr_bulletin)
{
  _clearBufrBulletin();
  _bufrBulletin = bufr_bulletin;
  _isBufr = true;
  _isAscii = false;
}

Amdar::Amdar(const Amdar& from)
{
  _copy(from);
}

/////////////////////////////////////////////////////////////////////////
// Destructor
//
Amdar::~Amdar()
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
// operator=
//
Amdar&  
Amdar::operator=(const Amdar& from)
{
  if(this == &from) {
    return *this;
  }

  _copy(from);

  return *this;
}

/////////////////////////////////////////////////////////////////////////
// clear
//
void 
Amdar::clear()
{
  _clearAsciiBulletin();
  _clearBufrBulletin();
}

/////////////////////////////////////////////////////////////////////////
// loadXml
//
void 
Amdar::loadXml(string &xml, int startIndentLevel /* = 0 */ ) const
{
  int sil = startIndentLevel;

  xml = "";
  
  if (_isAscii == true) {
    _loadXmlAsciiBulletin(xml, sil);
  }
  else {
    _loadXmlBufrBulletin(xml, sil);
  }

}

/////////////////////////////////////////////////////////////////////////
// disassemble
//
int
Amdar::disassemble(const void *buf, int len)
{
  clear();
  _xml.clear();
  _xml.append(static_cast<const char*>(buf), len - 1);

  string amdarStr;
  if (TaXml::readString(_xml, "AMDAR", amdarStr)) {
    return -1;
  }

  string contentStr;
  if (TaXml::readString(_xml, "bulletin_type", contentStr)) {
    return -1;
  }

  if(contentStr == "ASCII") {
    _isAscii = true;
    _isBufr = false;
    _disassembleAsciiBulletin(amdarStr);
  }
  else if(contentStr == "BUFR") {
    _isAscii = false;
    _isBufr = true;
    _disassembleBufrBulletin(amdarStr);
  }
  else {
    return -1;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////
// assemble
//
void
Amdar::assemble()
{
    // check mem buffer is free
  
  _memBuf.free();
  
  // convert to XML string
  
  loadXml(_xml);

  // add xml string to buffer, including trailing null
  
  _memBuf.add(_xml.c_str(), _xml.size() + 1);

}

/////////////////////////////////////////////////////////////////////////
// print
//
void 
Amdar::print(ostream &out, string spacer /* = "" */) const
{
  if (_isAscii == true) {
    _printAsciiBulletin(out, spacer);
  }
  else {
    _printBufrBulletin(out, spacer);
  }
}

/////////////////////////////////////////////////////////////////////////
// printAsXml
//
void 
Amdar::printAsXml(ostream &out, int startIndentLevel) const
{

  if (_xml.size() == 0 || startIndentLevel != 0) {
    loadXml(_xml, startIndentLevel);
  }

  out << _xml;

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// _copy
//
void 
Amdar::_copy(const Amdar &from)
{
}

/////////////////////////////////////////////////////////////////////////
// _clearAsciiBulletin
//
void 
Amdar::_clearAsciiBulletin()
{
  _asciiBulletin.text.erase();       
  _asciiBulletin.issueTime = 0;    
  _asciiBulletin.phaseFlight.erase();  
  _asciiBulletin.acIdent.erase();    
  _asciiBulletin.latitude = MISSING_VALUE;        
  _asciiBulletin.longitude = MISSING_VALUE;     
  _asciiBulletin.pressAltitude = MISSING_VALUE;   
  _asciiBulletin.temperature = MISSING_VALUE;    
  _asciiBulletin.dewPoint = MISSING_VALUE;         
  _asciiBulletin.relativeHumidity = MISSING_VALUE;    
  _asciiBulletin.windDirection = MISSING_VALUE;      
  _asciiBulletin.windSpeed = MISSING_VALUE;        
  _asciiBulletin.turbulenceCode = static_cast<int>(MISSING_VALUE);        
  _asciiBulletin.navSys = static_cast<int>(MISSING_VALUE);                
  _asciiBulletin.typeNavSys = static_cast<int>(MISSING_VALUE);                
  _asciiBulletin.tempPrec = static_cast<int>(MISSING_VALUE);                
  _asciiBulletin.hasSection3 = false;                
  _asciiBulletin.sec3presAlt = MISSING_VALUE;      
  _asciiBulletin.verticalGust = MISSING_VALUE;         
}

/////////////////////////////////////////////////////////////////////////
// _clearBufrBulletin
//
void 
Amdar::_clearBufrBulletin()
{
  _bufrBulletin.acIdent.erase();
  _bufrBulletin.sequenceNum = static_cast<int>(MISSING_VALUE);
  _bufrBulletin.latitude = MISSING_VALUE;
  _bufrBulletin.longitude = MISSING_VALUE; 
  _bufrBulletin.issueTime = 0; 
  _bufrBulletin.flightLevel = MISSING_VALUE;   
  _bufrBulletin.detailPhaseFlight.erase();  
  _bufrBulletin.windDirection = MISSING_VALUE; 
  _bufrBulletin.windSpeed = MISSING_VALUE;      
  _bufrBulletin.degreeTurb = static_cast<int>(MISSING_VALUE);   
  _bufrBulletin.derEqVertGustVel = MISSING_VALUE;  
  _bufrBulletin.temperature = MISSING_VALUE;         
  _bufrBulletin.acarsInterpVal.erase();  
}

/////////////////////////////////////////////////////////////////////////
// _printAsciiBulletin
//
void 
Amdar::_printAsciiBulletin(ostream &out, string spacer /* = "" */) const
{
  out << spacer << "=================== AMDAR =================" << endl;
  out << spacer << "bulletin type: ASCII" << endl; 
  out << spacer << "bulletin_time: " << utimstr(_asciiBulletin.issueTime) << endl;
  out << spacer << "flight_phase: " << _asciiBulletin.phaseFlight << endl;
  out << spacer << "ac_id: " << _asciiBulletin.acIdent << endl;
  out << spacer << "latitude: " << _asciiBulletin.latitude << endl;
  out << spacer << "longitude: " << _asciiBulletin.longitude << endl;
  out << spacer << "altitude: " << _asciiBulletin.pressAltitude << endl;
  out << spacer << "temperature: " << _asciiBulletin.temperature << endl;
  out << spacer << "dewpoint: " << _asciiBulletin.dewPoint << endl;
  out << spacer << "relative_humidity: " << _asciiBulletin.relativeHumidity << endl;
  out << spacer << "wind_dir: " << _asciiBulletin.windDirection << endl;
  out << spacer << "wind_speed: " << _asciiBulletin.windSpeed << endl;
  out << spacer << "turb_code: " << _asciiBulletin.turbulenceCode << endl;
  out << spacer << "turb_cat: " << _turbCode2Str(_asciiBulletin.turbulenceCode) << endl;
  out << spacer << "nav_sys_code: " << _asciiBulletin.navSys << endl;
  out << spacer << "nav_sys_type: " << _asciiBulletin.typeNavSys << endl;
  out << spacer << "temp_precision: " << _asciiBulletin.tempPrec << endl;
  out << spacer << "has_sec3: " << _asciiBulletin.hasSection3 << endl;
  out << spacer << "altitude_sec3: " << _asciiBulletin.sec3presAlt << endl;
  out << spacer << "vert_gust: " << _asciiBulletin.verticalGust << endl;
  out << spacer << "orig_text: " << _asciiBulletin.text << endl;
  out << spacer << "=========================================" << endl; 
}

/////////////////////////////////////////////////////////////////////////
// _printBufrBulletin
//
void 
Amdar::_printBufrBulletin(ostream &out, string spacer /* = "" */) const
{
  out << spacer << "=================== AMDAR =================" << endl;
  out << spacer << "bulletin type: BUFR" << endl; 
  out << spacer << "bulletin_time: " << utimstr(_bufrBulletin.issueTime) << endl;
  out << spacer << "flight_phase: " << _bufrBulletin.detailPhaseFlight << endl;
  out << spacer << "ac_id: " << _bufrBulletin.acIdent << endl;
  out << spacer << "latitude: " << _bufrBulletin.latitude << endl;
  out << spacer << "longitude: " << _bufrBulletin.longitude << endl;
  out << spacer << "flightLevel: " << _bufrBulletin.flightLevel << endl;
  out << spacer << "temperature: " << _bufrBulletin.temperature << endl;
  out << spacer << "wind_dir: " << _bufrBulletin.windDirection << endl;
  out << spacer << "wind_speed: " << _bufrBulletin.windSpeed << endl;
  out << spacer << "deg_turb: " << _bufrBulletin.degreeTurb << endl;
  out << spacer << "der_vert_gust: " << _bufrBulletin.derEqVertGustVel << endl;
  out << spacer << "=========================================" << endl; 
}

/////////////////////////////////////////////////////////////////////////
// _loadXmlAsciiBulletin
//
void 
Amdar::_loadXmlAsciiBulletin(string &xml, int startIndentLevel /* = 0 */) const
{
  int sil = startIndentLevel;

  // print object to string as XML
  
  xml = "";
  
  xml += TaXml::writeStartTag("AMDAR", sil+0);
  xml += TaXml::writeString("bulletin_type", sil+0, "ASCII");  
  xml += TaXml::writeTime("issue_time", sil+1, _asciiBulletin.issueTime);
  xml += TaXml::writeString("flight_phase", sil+1, _asciiBulletin.phaseFlight);
  xml += TaXml::writeString("ac_id", sil+1, _asciiBulletin.acIdent);
  xml += TaXml::writeDouble("latitude", sil+1, _asciiBulletin.latitude);
  xml += TaXml::writeDouble("longitude", sil+1, _asciiBulletin.longitude);
  xml += TaXml::writeDouble("altitude", sil+1, _asciiBulletin.pressAltitude);
  xml += TaXml::writeDouble("temperature", sil+1, _asciiBulletin.temperature);
  xml += TaXml::writeDouble("dewpoint", sil+1, _asciiBulletin.dewPoint);
  xml += TaXml::writeDouble("relative_humidity", sil+1, _asciiBulletin.relativeHumidity);
  xml += TaXml::writeDouble("wind_dir", sil+1, _asciiBulletin.windDirection);
  xml += TaXml::writeDouble("wind_speed", sil+1, _asciiBulletin.windSpeed);
  xml += TaXml::writeInt("turb_code", sil+1, _asciiBulletin.turbulenceCode);
  xml += TaXml::writeString("turb_cat", sil+1, _turbCode2Str(_asciiBulletin.turbulenceCode));
  xml += TaXml::writeInt("nav_sys_code", sil+1, _asciiBulletin.navSys);
  xml += TaXml::writeInt("nav_sys_type", sil+1, _asciiBulletin.typeNavSys);
  xml += TaXml::writeInt("temp_precision", sil+1, _asciiBulletin.tempPrec);
  xml += TaXml::writeBoolean("has_sec3", sil+1, _asciiBulletin.hasSection3);
  xml += TaXml::writeDouble("altitude_sec3", sil+1, _asciiBulletin.sec3presAlt);
  xml += TaXml::writeDouble("vert_gust", sil+1, _asciiBulletin.verticalGust);
  xml += TaXml::writeString("orig_text", sil+1, _asciiBulletin.text);
  xml += TaXml::writeDouble("missing", sil+1, MISSING_VALUE);
  xml += TaXml::writeEndTag("AMDAR", sil+0);
}

/////////////////////////////////////////////////////////////////////////
// _loadXmlBufrBulletin
//
void 
Amdar::_loadXmlBufrBulletin(string &xml, int startIndentLevel /* = 0 */) const
{
  int sil = startIndentLevel;

  // print object to string as XML
  
  xml = "";
  xml += TaXml::writeStartTag("AMDAR", sil+0);
  xml += TaXml::writeString("bulletin_type", sil+0, "BUFR");  
  xml += TaXml::writeTime("issue_time", sil+1, _bufrBulletin.issueTime);
  xml += TaXml::writeString("flight_phase", sil+1, _bufrBulletin.detailPhaseFlight);
  xml += TaXml::writeString("ac_id", sil+1, _bufrBulletin.acIdent);
  xml += TaXml::writeDouble("latitude", sil+1, _bufrBulletin.latitude);
  xml += TaXml::writeDouble("longitude", sil+1, _bufrBulletin.longitude);
  xml += TaXml::writeDouble("altitude", sil+1, _bufrBulletin.flightLevel);
  xml += TaXml::writeDouble("temperature", sil+1, _bufrBulletin.temperature);
  xml += TaXml::writeDouble("wind_dir", sil+1, _bufrBulletin.windDirection);
  xml += TaXml::writeDouble("wind_speed", sil+1, _bufrBulletin.windSpeed);
  xml += TaXml::writeInt("deg_turb", sil+1, _bufrBulletin.degreeTurb);
  xml += TaXml::writeDouble("der_vert_gust", sil+1, _bufrBulletin.derEqVertGustVel);
  xml += TaXml::writeDouble("missing", sil+1, MISSING_VALUE);
  xml += TaXml::writeEndTag("AMDAR", sil+0);
  
}

 
/////////////////////////////////////////////////////////////////////////
// _disassembleAsciiBulletin
//
void
Amdar::_disassembleAsciiBulletin(const string& amdar_str)
{
  // find the tags
  
  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(amdar_str, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = amdar_str.substr(start, len);
    
    if (tag == "issue_time") {
      TaXml::readTime(content, tag, _asciiBulletin.issueTime);
    } else if (tag == "flight_phase") {
      TaXml::readString(content, tag, _asciiBulletin.phaseFlight);
    } else if (tag == "ac_id") {
      TaXml::readString(content, tag, _asciiBulletin.acIdent);
    } else if (tag == "latitude") {
      TaXml::readDouble(content, tag, _asciiBulletin.latitude);
    } else if (tag == "longitude") {
      TaXml::readDouble(content, tag, _asciiBulletin.longitude);
    } else if (tag == "altitude") {
      TaXml::readDouble(content, tag, _asciiBulletin.pressAltitude);
    } else if (tag == "temperature") {
      TaXml::readDouble(content, tag, _asciiBulletin.temperature);
    } else if (tag == "dewpoint") {
      TaXml::readDouble(content, tag, _asciiBulletin.dewPoint);
    } else if (tag == "relative_humidity") {
      TaXml::readDouble(content, tag, _asciiBulletin.relativeHumidity);
    } else if (tag == "wind_dir") {
      TaXml::readDouble(content, tag, _asciiBulletin.windDirection);
    } else if (tag == "wind_speed") {
      TaXml::readDouble(content, tag, _asciiBulletin.windSpeed);
    } else if (tag == "turb_code") {
      TaXml::readInt(content, tag, _asciiBulletin.turbulenceCode);
    } else if (tag == "nav_sys_code") {
      TaXml::readInt(content, tag, _asciiBulletin.navSys);
    } else if (tag == "nav_sys_type") {
      TaXml::readInt(content, tag, _asciiBulletin.typeNavSys);
    } else if (tag == "temp_precision") {
      TaXml::readInt(content, tag, _asciiBulletin.tempPrec);
    } else if (tag == "has_sec3") {
      TaXml::readBoolean(content, tag, _asciiBulletin.hasSection3);
    } else if (tag == "altitude_sec3") {
      TaXml::readDouble(content, tag, _asciiBulletin.sec3presAlt);
    } else if (tag == "vert_gust") {
      TaXml::readDouble(content, tag, _asciiBulletin.verticalGust);
    } else if (tag == "orig_text") {
      TaXml::readString(content, tag, _asciiBulletin.text);
    }
  }

}

/////////////////////////////////////////////////////////////////////////
// _disassembleBufrBulletin
//
void
Amdar::_disassembleBufrBulletin(const string& amdar_str)
{
  // find the tags
  
  vector<TaXml::TagLimits> tags;
  TaXml::readTagLimits(amdar_str, 0, tags);
  
  for (int ii = 0; ii < (int) tags.size(); ii++) {
    
    string tag = tags[ii].getTag();
    size_t start = tags[ii].getStartPosn();
    size_t end = tags[ii].getEndPosn();
    size_t len = end - start;
    string content = amdar_str.substr(start, len);
    
    if (tag == "issue_time") {
      TaXml::readTime(content, tag, _bufrBulletin.issueTime);
    } else if (tag == "flight_phase") {
      TaXml::readString(content, tag, _bufrBulletin.detailPhaseFlight);
    } else if (tag == "ac_id") {
      TaXml::readString(content, tag, _bufrBulletin.acIdent);
    } else if (tag == "latitude") {
      TaXml::readDouble(content, tag, _bufrBulletin.latitude);
    } else if (tag == "longitude") {
      TaXml::readDouble(content, tag, _bufrBulletin.longitude);
    } else if (tag == "altitude") {
      TaXml::readDouble(content, tag, _bufrBulletin.flightLevel);
    } else if (tag == "temperature") {
      TaXml::readDouble(content, tag, _bufrBulletin.temperature);
    } else if (tag == "wind_dir") {
      TaXml::readDouble(content, tag, _bufrBulletin.windDirection);
    } else if (tag == "wind_speed") {
      TaXml::readDouble(content, tag, _bufrBulletin.windSpeed);
    } else if (tag == "deg_turb") {
      TaXml::readInt(content, tag, _bufrBulletin.degreeTurb);
    } else if (tag == "der_vert_gust") {
      TaXml::readDouble(content, tag, _bufrBulletin.derEqVertGustVel);
    }
  }


}

string
Amdar::_getAsciiTranslatedText() const
{
	
	string retString = "AMDAR\n";

	retString += "    flight#: " + getAircraftId() + "\n";

	//enough to hold date/time format below
	char timeBuff[20];
	
	const int maxBuffSize = 32;
	char numBuff[maxBuffSize];
	time_t issueT = getIssueTime();

	strftime(timeBuff, 20, "%Y-%m-%d %H:%M:%S", gmtime(&issueT));

	retString += "    Issue Time: ";
	retString += timeBuff;
	retString += "\n";

	retString += "    Flight Phase: ";
	retString += getFlightPhase();
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getLatitude());
	retString += "    Lat: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getLongitude());
	retString += "    Lon: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getPressAltitude());
	retString += "    Pres Alt: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getTemperature());
	retString += "    Temp: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getWindSpeed());
	retString += "    Wind Speed: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getWindDir());
	retString += "    Wind Dir: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getDewPoint());
	retString += "    Dew Point: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getRelativeHumidity());
	retString += "    RH: ";
	retString += numBuff;
	retString +=  "\n";

	
	retString += "    Turb: ";
	retString += _turbCode2Str(getTurbulenceCode());
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",getVerticalGust());
	retString += "    Vertical Gust: ";
	retString += numBuff;
	retString += "\n";

	return retString;

	/*

	      _asciiBulletin.navSys);

      _asciiBulletin.typeNavSys);

      _asciiBulletin.tempPrec);

      _asciiBulletin.hasSection3);

      _asciiBulletin.sec3presAlt);

      _asciiBulletin.verticalGust);

      _asciiBulletin.text);

	*/
}  

string
Amdar::_getBufrTranslatedText() const
{
	
	string retString = "AMDAR\n";

	retString += "    flight#: " +  _bufrBulletin.acIdent + "\n";

	//enough to hold date/time format below
	char timeBuff[20];
	
	const int maxBuffSize = 32;
	char numBuff[maxBuffSize];
	time_t issueT = _bufrBulletin.issueTime;

	strftime(timeBuff, 20, "%Y-%m-%d %H:%M:%S", gmtime(&issueT));

	retString += "    Issue Time: ";
	retString += timeBuff;
	retString += "\n";

	retString += "    Flight Phase: ";
	retString += _bufrBulletin.detailPhaseFlight;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.latitude);
	retString += "    Lat: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.longitude);
	retString += "    Lon: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.flightLevel);
	retString += "    Pres Alt: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.temperature);
	retString += "    Temp: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.windSpeed);
	retString += "    Wind Speed: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.windDirection);
	retString += "    Wind Dir: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%2d",_bufrBulletin.degreeTurb);
	retString += "    Deg. Turb: ";
	retString += numBuff;
	retString += "\n";

	snprintf(numBuff,maxBuffSize,"%.2f",_bufrBulletin.derEqVertGustVel);
	retString += "    Der. Vertical Gust: ";
	retString += numBuff;
	retString += "\n";

	return retString;
}  

/////////////////////////////////////////////////////////////////////////
// _turbCode2Str
//

string 
Amdar::_turbCode2Str(int code) const
{
  if(code == TURB_CODE_NONE) {
    return "NONE";
  }
  else if (code == TURB_CODE_LIGHT) {
    return "LIGHT";
  } 
  else if(code == TURB_CODE_MODERATE) {
    return "MODERATE";
  } 
  else if(code == TURB_CODE_SEVERE) {
    return "SEVERE";
  }
  else {
    return "UNKNOWN";
  }
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Helper Functions
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

