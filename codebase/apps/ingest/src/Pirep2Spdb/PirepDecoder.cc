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
 *  $Id: PirepDecoder.cc,v 1.11 2016/03/07 01:23:04 dixon Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: PirepDecoder.cc,v 1.11 2016/03/07 01:23:04 dixon Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	PirepDecoder
//
// Author:	G. M. Cunning
//
// Date:	Tue Jan  3 16:31:09 2006
//
// Description: Decodes pireps following Greg Thompson's conventions.
//
//


// C++ include files

// System/RAP include files
#include <boost/regex.hpp>
#include<boost/tokenizer.hpp>

// Local include files
#include "PirepDecoder.hh"

using namespace std;

// define any constants
const string PirepDecoder::_className    = "PirepDecoder";

typedef boost::tokenizer<boost::char_separator<char> > tokenizer;



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

PirepDecoder::PirepDecoder()
{
  _initialize();
}

PirepDecoder::PirepDecoder(const PirepDecoder &)
{

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
PirepDecoder::~PirepDecoder()
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
void PirepDecoder::_initialize()
{
  // fill out _turbulenceKeys
  //NEG | SMTH-LGT | LGT | LGT-MOD | MOD | MOD-SEV | SEV | SEV-EXTM | EXTM
  _turbulenceKeys.insert(pair<string,int>("EXTM",8));
  _turbulenceKeys.insert(pair<string,int>("SEV-EXTM",7));
  _turbulenceKeys.insert(pair<string,int>("SEV",6));
  _turbulenceKeys.insert(pair<string,int>("MOD-SEV",5));
  _turbulenceKeys.insert(pair<string,int>("MOD",4));
  _turbulenceKeys.insert(pair<string,int>("LGT-MOD",3));
  _turbulenceKeys.insert(pair<string,int>("LGT",2));
  _turbulenceKeys.insert(pair<string,int>("SMTH-LGT",1));
  _turbulenceKeys.insert(pair<string,int>("NEG",0));

  //CAT | CHOP | LLWS | MWAVE
  _turbulenceTypes.insert(pair<string,int>("CHOP",1));
  _turbulenceTypes.insert(pair<string,int>("CAT",2));
  _turbulenceTypes.insert(pair<string,int>("LLWS",3));
  _turbulenceTypes.insert(pair<string,int>("MWAVE",4));

 //NEG | NEGclr | TRC | TRC-LGT | LGT | LGT-MOD | MOD | MOD-SEV | HVY | SEV
  _icingKeys.insert(pair<string,int>("SEV",8));
  _icingKeys.insert(pair<string,int>("HVY",7));
  _icingKeys.insert(pair<string,int>("MOD-SEV",6));
  _icingKeys.insert(pair<string,int>("MOD",5));
  _icingKeys.insert(pair<string,int>("LGT-MOD",4));
  _icingKeys.insert(pair<string,int>("LGT",3));
  _icingKeys.insert(pair<string,int>("TRC-LGT",2));
  _icingKeys.insert(pair<string,int>("TRC",1));
  _icingKeys.insert(pair<string,int>("NEGclr",-1));
  _icingKeys.insert(pair<string,int>("NEG",-1));
 
  // SKC | CLEAR | CAVOC | FEW | SCT | BKN | OVC | OVX
  // Not used: VMC, VFR, IFR, IMC
  _skyKeys.insert(pair<string,int>("CLEAR",0));
  _skyKeys.insert(pair<string,int>("SKC",0));
  _skyKeys.insert(pair<string,int>("CAVOC",0));
  _skyKeys.insert(pair<string,int>("FEW",1));
  _skyKeys.insert(pair<string,int>("SCT",2));
  _skyKeys.insert(pair<string,int>("BKN",3));
  _skyKeys.insert(pair<string,int>("OVC",4));
  _skyKeys.insert(pair<string,int>("OVX",5));

 // RIME | CLEAR | MIXED
  _icingTypes.insert(pair<string,int>("RIME",1));
  _icingTypes.insert(pair<string,int>("CLEAR",2));
  _icingTypes.insert(pair<string,int>("MIXED",3));

 //ISOL | OCNL | CONT
  _turbulenceFreq.insert(pair<string,int>("ISOL",1));
  _turbulenceFreq.insert(pair<string,int>("OCNL",2));
  _turbulenceFreq.insert(pair<string,int>("CONT",3));
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	PirepDecoder::addXmlSpdb
//
// Description:	preforms tokenizing.
//
// Returns:	
//
// Notes:
//
//

void 
PirepDecoder::addXmlSpdb(DsSpdb* spdb, Pirep* p, int& expire_secs)
{
  time_t obsTime = p->getObsTime();
  const time_t expireTime = obsTime + expire_secs;

  string xml = p->getXml(); 
  cout << "str:" << xml << endl;
  
  spdb->addPutChunk(0,
         	    obsTime,
		    expireTime,
         	    xml.size()+1,
		    xml.c_str());  
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	PirepDecoder::addRawSpdb
//
// Description:	preforms tokenizing.
//
// Returns:	
//
// Notes:
//
//

void 
PirepDecoder::addRawSpdb(DsSpdb* spdb, Pirep* p, const time_t& expire_secs)
{
  time_t obsTime = p->getObsTime();
  const time_t expireTime = obsTime + expire_secs;

  string raw = p->getRaw();  

  spdb->addPutChunk(0,
		    obsTime,
		    expireTime,
                    raw.size()+1,
                    raw.c_str());
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

int PirepDecoder::createPirepObject(string in, Pirep& out, Params::xml_names_t xNames)
{
  bool first;
  time_t obsTime;
  if(TaXml::readTime(in, xNames.time, obsTime)) {
    cerr << "ERROR - PirepDecoder::createPirepObject" << endl;
    cerr << "  No obs_time tag" << endl;
    return -2;
  }
  out.setObsTime(obsTime);

  double latitude;
  if (TaXml::readDouble(in, xNames.lat, latitude)) {
    cerr << "ERROR - PirepDecoder::createPirepObject" << endl;
    cerr << "  No latitude tag" << endl;
    return -3;
  }
  out.setLatitude(latitude);

  double longitude;
  if (TaXml::readDouble(in, xNames.lon, longitude)) {
    cerr << "ERROR - PirepDecoder::createPirepObject" << endl;
    cerr << "  No longitude tag" << endl;
    return -3;
  }
  out.setLongitude(longitude);


  double altitude;
  if (TaXml::readDouble(in, xNames.altitude, altitude)) {
    cerr << "ERROR - PirepDecoder::createPirepObject" << endl;
    cerr << "  No altitude tag" << endl;
    return -3;
  }
  out.setAltitude(altitude);

  // is valid if contains above fields
  out.setType(Pirep::VALID_PIREP);

  string raw;
  if (TaXml::readString(in, xNames.raw, raw) == 0) {
    out.setRaw(raw.c_str());
    cout << "or: " << out.getRaw() << endl;
  }

  cout << "raw: " << raw << endl;

  string aid;
  if (TaXml::readString(in, xNames.aircraft, aid) == 0) {
    out.setAircraftId(aid.c_str());
    cout << "oa: " << out.getAircraftId() << endl;
  }

  cout << "aid: " << aid << endl;
  string reportType;
  if (TaXml::readString(in, xNames.type, reportType) == 0) {
    if(strcmp(reportType.c_str(),"PIREP") != 0)
    {
      return -1; // only process pireps?
    }
    out.setReportType(reportType.c_str());
    cout << "ort: " << out.getReportType() << endl;
  }

  cout << "type: " << reportType << endl;

  first = true;
  vector<string> iceXmlVec;
  if (TaXml::readTagBufArray(in, xNames.ice_cond, iceXmlVec) == 0)
  {
    for(unsigned int i=0; i< iceXmlVec.size(); i++)
    {
      out.setType(Pirep::ICING_PIREP);

      Pirep::ice_obs_t ice;
      ice.type = Pirep::FILL_IT;
      ice.intensity = Pirep::FILL_II;
      ice.top = Pirep::MISSING_VALUE;
      ice.base = Pirep::MISSING_VALUE;

      const string t = iceXmlVec[i];
      string temp;
      vector<TaXml::attribute> attrs;
      TaXml::readString(t,xNames.ice_cond,temp,attrs);
      for(unsigned int j=0; j<attrs.size(); j++)
      {
        if(attrs[j].getName() == xNames.ice_type)
	{
          map<string,int>::const_iterator it;
          if((it = _icingTypes.find(attrs[j].getVal())) != _icingTypes.end())
	    {
	      ice.type = (*it).second;
	    }
        }
        else if(attrs[j].getName() == xNames.ice_int)
	{
          map<string,int>::const_iterator it;
          if((it = _icingKeys.find(attrs[j].getVal())) != _icingKeys.end())
	    {
	      ice.intensity = (*it).second;
	    }
        }
        else if(attrs[j].getName() == xNames.ice_top)
	{
	  ice.top = atoi(attrs[j].getVal().c_str());
        }
        else if(attrs[j].getName() == xNames.ice_base)
	{
	  ice.base = atoi(attrs[j].getVal().c_str());
        }
      }
      if(first)
      {
        out.setIceObs1(ice);
        first = false;
      }
      else
      {
        out.setIceObs2(ice);
      }
    }
  }

  // get turb obs
  first = true;
  vector<string> turbXmlVec;
  if (TaXml::readTagBufArray(in, xNames.turb_cond, turbXmlVec) == 0) {
    for(unsigned int i=0; i< turbXmlVec.size(); i++)
    {
      Pirep::turb_obs_t turb;
      turb.top = Pirep::MISSING_VALUE;
      turb.base = Pirep::MISSING_VALUE;

      if(out.getType() == Pirep::ICING_PIREP)
      {
        out.setType(Pirep::BOTH_PIREP);
      }
      else if(out.getType() == Pirep::VALID_PIREP)
      {
        out.setType(Pirep::TURB_PIREP);
      }

      cout << "T: " << turbXmlVec[i] << endl;
      const string t = turbXmlVec[i];
      string temp;
      vector<TaXml::attribute> attrs;
      TaXml::readString(t,xNames.turb_cond,temp,attrs);
      for(unsigned int j=0; j<attrs.size(); j++)
      {
        if(attrs[j].getName() == xNames.turb_type)
	{
          map<string,int>::const_iterator it;
          if((it = _turbulenceTypes.find(attrs[j].getVal())) != _turbulenceTypes.end())
	    {
	      turb.type = (*it).second;
	    }
        }
        else if(attrs[j].getName() == xNames.turb_int)
	{
          map<string,int>::const_iterator it;
          if((it = _turbulenceKeys.find(attrs[j].getVal())) != _turbulenceKeys.end())
	    {
	      turb.intensity = (*it).second;
	    }
        }
        else if(attrs[j].getName() == xNames.turb_freq)
	{
          map<string,int>::const_iterator it;
          if((it = _turbulenceFreq.find(attrs[j].getVal())) != _turbulenceFreq.end())
	    {
	      turb.frequency = (*it).second;
	    }
        }
        else if(attrs[j].getName() == xNames.turb_top)
	{
	  turb.top = atoi(attrs[j].getVal().c_str());
        }
        else if(attrs[j].getName() == xNames.turb_base)
	{
	  turb.base = atoi(attrs[j].getVal().c_str());
        }
      }
      if(first)
      {
        out.setTurbObs1(turb);
        first = false;
      }
      else
      {
        out.setTurbObs2(turb);
      }
    }
  }

  // get sky obs
  first = true;
  vector<string> skyXmlVec;
  if (TaXml::readTagBufArray(in, xNames.sky_cond, skyXmlVec) == 0) {
    for(unsigned int i=0; i< skyXmlVec.size(); i++)
    {
      Pirep::sky_cond_t sky;
      sky.top = Pirep::MISSING_VALUE;
      sky.base = Pirep::MISSING_VALUE;
      if(out.getType() == Pirep::ICING_PIREP)
      {
        out.setType(Pirep::BOTH_PIREP);
      }
      else if(out.getType() == Pirep::VALID_PIREP)
      {
        out.setType(Pirep::TURB_PIREP);
      }

      cout << "T: " << skyXmlVec[i] << endl;
      const string t = skyXmlVec[i];
      string temp;
      vector<TaXml::attribute> attrs;
      TaXml::readString(t,xNames.sky_cond,temp,attrs);
      for(unsigned int j=0; j<attrs.size(); j++)
      {
        if(attrs[j].getName() == xNames.sky_cover)
	{
          map<string,int>::const_iterator it;
          if((it = _skyKeys.find(attrs[j].getVal())) != _skyKeys.end())
	    {
	      sky.coverage = (*it).second;
	    }
          
          if(sky.coverage == Pirep::CLEAR_SKY && out.getType() == Pirep::VALID_PIREP)
          {
            out.setType(Pirep::CLEAR_PIREP);
          }
        }
        else if(attrs[j].getName() == xNames.cloud_top)
	{
          sky.top = atoi(attrs[j].getVal().c_str());
        }
        else if(attrs[j].getName() == xNames.cloud_base)
	{
	  sky.base = atoi(attrs[j].getVal().c_str());
        }
      }
      if(first)
      {
        out.setSkyCondition1(sky);
        first = false;
      }
      else
      {
        out.setSkyCondition2(sky);
      }
    }
  }

  // get weather obs
  Pirep::wx_obs_t weather;
  weather.clear_above = Pirep::MISSING_VALUE;
  weather.temperature = Pirep::MISSING_VALUE;
  weather.wind_dir = Pirep::MISSING_VALUE;
  weather.wind_speed = Pirep::MISSING_VALUE;
  weather.visibility = Pirep::MISSING_VALUE;
  bool hasWeather = false;

  string item;
  if (TaXml::readString(in, xNames.clear_above, item) == 0) {
    weather.clear_above = atoi(item.c_str());
    hasWeather = true;
  }

  if (TaXml::readString(in, xNames.temp, item) == 0) {
    weather.temperature = atoi(item.c_str());
    hasWeather = true;
  }

  if (TaXml::readString(in, xNames.wind_dir, item) == 0) {
    weather.wind_dir = atoi(item.c_str());
    hasWeather = true;
  }

  if (TaXml::readString(in, xNames.wind_speed, item) == 0) {
    weather.wind_speed = atoi(item.c_str());
    hasWeather = true;
  }

  if (TaXml::readString(in, xNames.visibility, item) == 0) {
    weather.visibility = atoi(item.c_str());
    hasWeather = true;
  }

  if(hasWeather)
  {
    out.setWeatherObs(weather);
  }

  out.toXml();
  return 0;
}

int PirepDecoder::createPirepObject(vector<string> in, Pirep& out)
{
  // check if item exists before entering it
  if(in[1].size() == 0)
  {
    return -1;
  }
  out.setObsTime(convertStringToTimeT(in[1]));

  if(in[4].size() == 0)
  {
    return -1;
  }
  out.setLatitude(atof(in[4].c_str()));

  if(in[5].size() == 0)
  {
    return -1;
  }
  out.setLongitude(atof(in[5].c_str()));

  if(in[6].size() == 0)
  {
    return -1;
  }
  out.setAltitude(atoi(in[6].c_str()));

  // is valid if contains above fields
  out.setType(Pirep::VALID_PIREP);

  out.setRaw(in[37]);

  out.setAircraftId(in[3]);

  out.setReportType(in[38]);

  // get icing obs
  if(in[21].size() != 0)
  {
    Pirep::ice_obs_t ice;
    ice.type = Pirep::FILL_IT;
    ice.intensity = Pirep::FILL_II;
    ice.top = Pirep::MISSING_VALUE;
    ice.base = Pirep::MISSING_VALUE;
    out.setType(Pirep::ICING_PIREP);
   
    map<string,int>::const_iterator it;
    if((it = _icingTypes.find(in[21])) != _icingTypes.end())
    {
      ice.type = (*it).second;
    }

    if(in[20].size() != 0)
    {
      map<string,int>::const_iterator it;
      if((it = _icingKeys.find(in[20])) != _icingKeys.end())
      {
        ice.intensity = (*it).second;
      }

    }

    if(in[19].size() != 0)
    {
      ice.top = atoi(in[19].c_str());
    }

    if(in[18].size() != 0)
    {
      ice.base = atoi(in[18].c_str());
    }
    
    out.setIceObs1(ice);
  }

  if(in[25].size() != 0)
  {
    Pirep::ice_obs_t ice;
    ice.type = Pirep::FILL_IT;
    ice.intensity = Pirep::FILL_II;
    ice.top = Pirep::MISSING_VALUE;
    ice.base = Pirep::MISSING_VALUE;
    out.setType(Pirep::ICING_PIREP);

    map<string,int>::const_iterator it;
    if((it = _icingTypes.find(in[25])) != _icingTypes.end())
    {
      ice.type = (*it).second;
    }

    if(in[24].size() != 0)
    {
      map<string,int>::const_iterator it;
      if((it = _icingKeys.find(in[24])) != _icingKeys.end())
      {
        ice.intensity = (*it).second;
      }
    }

    if(in[23].size() != 0)
    {
      ice.top = atoi(in[23].c_str());
    }

    if(in[22].size() != 0)
    {
      ice.base = atoi(in[22].c_str());
    }

    out.setIceObs2(ice);
  }

  // get turb obs
  if(in[29].size() != 0)
  {
    Pirep::turb_obs_t turb;
    turb.type = Pirep::FILL_TT;
    turb.intensity = Pirep::FILL_TI;
    turb.top = Pirep::MISSING_VALUE;
    turb.base = Pirep::MISSING_VALUE;

    if(out.getType() == Pirep::ICING_PIREP)
    {
      out.setType(Pirep::BOTH_PIREP);
    }
    else if(out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::TURB_PIREP);
    }

    map<string,int>::const_iterator it;
    if((it = _turbulenceTypes.find(in[29])) != _turbulenceTypes.end())
    {
      turb.type = (*it).second;
    }
    //  turb.type = in[29];

    if((it = _turbulenceKeys.find(in[28])) != _turbulenceKeys.end())
    {
      turb.intensity = (*it).second;
    }
    //  turb.intensity = in[28];
    turb.top = atoi(in[27].c_str());
    turb.base = atoi(in[26].c_str());

    out.setTurbObs1(turb);
  }

  if(in[34].size() != 0)
  {
    Pirep::turb_obs_t turb;
    turb.type = Pirep::FILL_TT;
    turb.intensity = Pirep::FILL_TI;
    turb.top = Pirep::MISSING_VALUE;
    turb.base = Pirep::MISSING_VALUE;
    if(out.getType() == Pirep::ICING_PIREP)
    {
      out.setType(Pirep::BOTH_PIREP);
    }
    else if(out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::TURB_PIREP);
    }

    map<string,int>::const_iterator it;
    if((it = _turbulenceTypes.find(in[34])) != _turbulenceTypes.end())
    {
      turb.type = (*it).second;
    }

    if((it = _turbulenceKeys.find(in[33])) != _turbulenceKeys.end())
    {
      turb.intensity = (*it).second;
    }
    turb.top = atoi(in[32].c_str());
    turb.base = atoi(in[31].c_str());

    out.setTurbObs2(turb);
  }

  // get sky obs
  if(in[9].size() != 0)
  {
    Pirep::sky_cond_t sky;
    sky.top = Pirep::MISSING_VALUE;
    sky.base = Pirep::MISSING_VALUE;
    sky.coverage = Pirep::NO_REPORT_SKY;

    map<string,int>::const_iterator it;
    if((it = _skyKeys.find(in[9])) != _skyKeys.end())
    {
      sky.coverage = (*it).second;
    }

    if(sky.coverage == Pirep::CLEAR_SKY && out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::CLEAR_PIREP);
    }

    if(in[8].size() != 0)
    {
      sky.top = atoi(in[8].c_str());
    }

    if(in[7].size() != 0)
    {
      sky.base = atoi(in[7].c_str());
    }

    out.setSkyCondition1(sky);    
  }

  if(in[12].size() != 0)
  {
    Pirep::sky_cond_t sky;
    sky.top = Pirep::MISSING_VALUE;
    sky.base = Pirep::MISSING_VALUE;
    sky.coverage = Pirep::NO_REPORT_SKY;

    map<string,int>::const_iterator it;
    if((it = _skyKeys.find(in[12])) != _skyKeys.end())
    {
      sky.coverage = (*it).second;
    }

    if(sky.coverage == Pirep::CLEAR_SKY && out.getType() == Pirep::VALID_PIREP)
    {
      out.setType(Pirep::CLEAR_PIREP);
    }

    if(in[11].size() != 0)
    {
      sky.top = atoi(in[11].c_str());
    }

    if(in[10].size() != 0)
    {
      sky.base = atoi(in[10].c_str());
    }

    out.setSkyCondition2(sky);    
  }

  // get weather obs
  Pirep::wx_obs_t weather;
  weather.visibility = Pirep::MISSING_VALUE;
  weather.temperature = Pirep::MISSING_VALUE;
  weather.wind_dir = Pirep::MISSING_VALUE;
  weather.wind_speed = Pirep::MISSING_VALUE;
  weather.clear_above = Pirep::MISSING_VALUE;

  bool hasWeather = false;

  if(in[13].size() != 0)
  {
    weather.visibility = atoi(in[13].c_str());
    hasWeather = true;
  }

  if(in[15].size() != 0)
  {
    weather.temperature = atoi(in[15].c_str());
    hasWeather = true;
  }

  if(in[16].size() != 0)
  {
    weather.wind_dir = atoi(in[16].c_str());
    hasWeather = true;
  }

  if(in[17].size() != 0)
  {
    weather.wind_speed = atoi(in[17].c_str());
    hasWeather = true;
  }
  // no clear above field in csv files?

  if(hasWeather)
  {
    out.setWeatherObs(weather);
  }

  out.toXml();

  return 0;
}

time_t PirepDecoder::convertStringToTimeT(string in)
{
  int year, month, day, hour, min, sec;
  if (sscanf(in.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
         &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime dt(year, month, day, hour, min, sec);
    return dt.utime();
  }
  time_t tval;
  if (sscanf(in.c_str(), "%ld", &tval) == 1) {
    return tval;
  }

  return 0;
}
