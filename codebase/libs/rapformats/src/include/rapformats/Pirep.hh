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
//////////////////////////////////////////////////////////////////////////
// 
// Header:	Pirep
// 
// Author:	G. M. Cunning 
// 
// Date:	Wed Dec 28 10:29:37 2005
// 
// Description:	Class to contain pirep information. This is based on 
//		pirep.h by Mike Dixon.
// 


# ifndef    PIREP_H
# define    PIREP_H

// C++ include files
#include <string>
#include <cstring>
#include <iostream>

// System/RAP include files
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
// class TiXmlElement;

// Local include files

using namespace std;

class Pirep {
  
public:

  ////////////////////
  // public members //
  ////////////////////

  static const int REP_STR_LEN = 8;
  static const int AC_STR_LEN = 8;
  static const int PIREP_TEXT_LEN1 = 128;
  static const float MISSING_VALUE;

  //////////////////////////////////
  // public types & enumerations  //
  //////////////////////////////////

  //
  // pirep type enum
  //
  typedef enum 
  {
    FILL_PIREP  = -9,	// fill value
    VALID_PIREP = 0,	// valid pirep with no mention 
			//  of icing or turbulence
    ICING_PIREP = 1,	// icing data only
    TURB_PIREP  = 2,	// turbulence data only
    BOTH_PIREP  = 3,	// both icing and turbulence data
    CLEAR_PIREP = 4	// no icing or turbulence data, but clear skies
  } pirep_type_t;

  //
  // sky coverage enum
  //
  typedef enum 
  {
    NO_REPORT_SKY = -9,	// no report
    CLEAR_SKY     = 0,	// clear sky
    FEW_SKY       = 1,  // few sky
    SCT_SKY       = 2,  // scattered sky
    BKN_SKY       = 3,  // broken sky
    OVC_SKY       = 4,  // overcast sky
    OBSCURD_SKY   = 5  // obscured sky
  } sky_cvrg_t;
  /*
  typedef enum 
  {
    NO_REPORT_SKY = -9,	// no report
    CLEAR_SKY     = 0,	// clear sky
    LGT_SCT_SKY   = 1,	// light scattered sky
    SCT_SKY       = 2,  // scattered sky
    SCT_BKN_SKY   = 3,  // scattered to broken sky
    LGT_BKN_SKY   = 4,  // light broken sky
    BKN_SKY       = 5,  // broken sky
    BKN_OVC_SKY   = 6,  // broken to overcast sky
    LGT_OVC_SKY   = 7,  // light overcast sky
    OVC_SKY       = 8,  // overcast sky
    OBSCURD_SKY   = 9  // obscured sky
  } sky_cvrg_t;
  */
  //
  // observed weather enum
  //
  typedef enum 
  {
    NO_REPORT_WX       = -9,	// no report
    VFR_GOOD_UNLMTD_WX = 2,	// VFR good or unlimited visibility
    CLEAR_WX           = 3,	// clear
    SMOKE_WX           = 4,	// smoke
    HAZE_WX            = 5,	// haze
    DUST_ASH_WX        = 6,	// dust or ash
    TORNADO_WX         = 8,	// tornado
    SAND_WX            = 9,	// sand storm
    VIRGA_WX           = 14,	// virga
    LGHT_TSTRM_WX      = 17,	// lightning or thunderstorm
    FUNNEL_WX          = 19,	// funnel cloud 
    IFR_OBSCRD_WX      = 40,	// IFR obscured
    FOG_WX             = 45,	// fog or ground fog
    FRZ_FOG_WX         = 48,	// freezing fog
    DRZL_WX            = 51,	// drizzle
    FRZ_DRZL_WX        = 57,	// freezing drizzle
    RAIN_WX            = 63,	// rain
    FRZ_RAIN_WX        = 67,	// freezing rain
    SNOW_WX            = 73,	// snow
    GRAUPEL_WX         = 77,	// graupel
    RAIN_SHWR_WX       = 81,	// rain shower
    SNOW_SHWR_WX       = 86,	// snow shower
    HAIL_WX            = 87	// hail
  } obs_weather_t;


  //
  // icing intensity enum
  //
  typedef enum
  {
    FILL_II = -9,	// fill value
    NONE_II = -1,	// none
    TRC_II = 1,		// trace
    TRC_LGHT_II = 2,	// trace to light
    LGHT_II = 3,	// light
    LGHT_MOD_II = 4,	// light to moderate
    MOD_II = 5,		// moderate
    MOD_HVY_II = 6,	// moderate to heavy
    HVY_II = 7,		// heavy
    SEVR_II = 8		// severe
  } ice_intens_t;


  //
  // icing type enum
  //
  typedef enum
  {
    FILL_IT = -9,	// fill value
    RIME_IT = 1,	// rime ice
    CLEAR_IT = 2,	// clear ice
    MIXED_IT = 3	// rime and clear ice
  } ice_type_t;

  //
  // turbulence frequency enum
  //
  typedef enum
  {
    FILL_TF = -9,	// fill value
    ISOL_TF = 1,	// isolated
    OCCNL_TF = 2,	// occasional
    CONTNS_TF = 3	// continuous
  } turb_freq_t;


  //
  // turbulence intensity enum
  //
  typedef enum
  {
    FILL_TI = -9,	// fill value
    NONE_SMOOTH_TI = 0, // none or smooth
    SMOOTH_LGHT_TI = 1, // smooth to light
    LGHT_TI = 2,	// light
    LGHT_MOD_TI = 3,	// light to moderate
    MOD_TI = 4,		// moderate
    MOD_SEVR_TI = 5,	// moderate to severe
    SEVR_TI = 6,	// severe
    SEVR_EXTRM_TI = 7,	// severe to extreme
    EXTRM_TI = 8	// extreme
  } turb_intens_t;


  //
  // turbulence type enum
  //
  typedef enum
  {
    FILL_TT = -9,		// fill value 
    CHOP_TT = 1,		// chop 
    CAT_TT = 2,			// clear air 
    LOWLVL_WNDSHR_TT = 3,	// low-level wind shear 
    MTN_WAVE_TT = 4		// mountain wave
  } turb_type_t;

  //
  // reported sky condition
  //
  typedef struct
  {
    fl32 base;      // feet
    fl32 top;       // feet
    si08 coverage;  // see sky_cvrg enum for definitions
    bool isSet;
  } sky_cond_t;

  //
  // reported weather observation
  //
  typedef struct 
  {
    fl32 clear_above; // feet
    fl32 visibility;  // nautical miles
    si08 observation; // wx_obs enum for definition
    fl32 temperature; // degrees C
    fl32 wind_dir;    // degrees (true north?)
    fl32 wind_speed;  // degrees (true north?)
    bool isSet;
  } wx_obs_t;


  //
  // reported icing observation
  //
  typedef struct
  {
    fl32 base;       // feet
    fl32 top;        // feet
    si08 intensity;  // see ice_intens enum for definitions
    si08 type;       // see ice_type enum for definitions   
    bool isSet;
  } ice_obs_t;


  //
  // reported turbulence observation
  //
  typedef struct
  {
    fl32 base;       // feet
    fl32 top;        // feet
    si08 frequency;  // see turb_freq enum for definitions
    si08 intensity;  // see turb_intens enum for definitions
    si08 type;       // see turb_type enum for definitions   
    bool isSet;
  } turb_obs_t;


  //
  // the pirep
  //
  /*
  typedef struct
  {
    ti32 obs_time;			// unix time
    char report_type[REP_STR_LEN];	// report type -- PIREP or AIREP
    char acft_id[AC_STR_LEN];		// aircraft ID or tail number
    fl32 latitude;			// degrees
    fl32 longitude;			// degrees
    fl32 altitude;			// feet
    si08 type;				// see pirep_type enum for definitions
    
    sky_cond_t sky_cond_1;
    sky_cond_t sky_cond_2;

    wx_obs_t wx_obs;

    ice_obs_t ice_obs_1;
    ice_obs_t ice_obs_2;

    turb_obs_t turb_obs_1;
    turb_obs_t turb_obs_2;

    char text[PIREP_TEXT_LEN];
  } pirep_t;
  */
  
  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  Pirep();
  Pirep(const Pirep &);

  // destructor
  virtual ~Pirep();

  // assignment
  Pirep& operator=(const Pirep &rhs);

  // clear the object
  void clear();

  // parse XML methods
  // TiXmlElement* getItem(string name, TiXmlElement* xml);
  // TiXmlElement* getNextItem(string name, TiXmlElement* xml);
  // bool parseObsTime(string name, TiXmlElement* xml);
  // bool parseLatitude(string name, TiXmlElement* xml);
  // bool parseLongitude(string name, TiXmlElement* xml);
  // bool parseAltitude(string name, TiXmlElement* xml);
  // bool parseRaw(string name, TiXmlElement* xml);
  // bool parseAircraftId(string name, TiXmlElement* xml);
  // bool parseReportType(string name, TiXmlElement* xml);

  // set methods
  void setObsTime(const time_t &obs_time);
  void setReportType(const string &report_type);
  void setAircraftId(const string &acft_id);
  void setLatitude(const double &lat);
  void setLongitude(const double &lon);
  void setAltitude(const double &alt);
  void setType(const int &type);
  void setSkyCondition1(const sky_cond_t &sky_cond);
  void setSkyCondition2(const sky_cond_t &sky_cond);
  void setWeatherObs(const wx_obs_t &wx_obs);
  void setIceObs1(const ice_obs_t &ice_obs);
  void setIceObs2(const ice_obs_t &ice_obs);
  void setTurbObs1(const turb_obs_t &turb_obs);
  void setTurbObs2(const turb_obs_t &turb_obs);
  void setRaw(const string &text);

  void setXml(const string &text);

  // get methods
  time_t getObsTime() const { return _obs_time; };
  const char* getReportType() const { return _report_type; };
  const char* getAircraftId() const { return _acft_id; };
  double getLatitude() const { return _latitude; };
  double getLongitude() const { return _longitude; };
  double getAltitude() const { return _altitude; };
  int getType() const { return _type; };
  sky_cond_t getSkyCondition1() const { return _sky_cond_1; };
  sky_cond_t getSkyCondition2() const { return _sky_cond_2; };
  wx_obs_t getWeatherObs() const { return _wx_obs; };
  ice_obs_t getIceObs1() const { return _ice_obs_1; };
  ice_obs_t getIceObs2() const { return _ice_obs_2; };
  turb_obs_t getTurbObs1() const { return _turb_obs_1; };
  turb_obs_t getTurbObs2() const { return _turb_obs_2; };
  const char* getRaw() const { return _raw; };
  string getXml() const { return _xml; }

  int fromXml();
  void toXml();


  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  void assemble();


  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns true on success, false on failure
  bool disassemble(const void *buf, int len);


  // get the assembled buffer info
  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }
  

  /////////////////////////
  // printing
  void print(ostream &out, string spacer = "") const;

  static const char *typeToString(pirep_type_t type);
  static const char *skyCovrgToString(sky_cvrg_t sky_cvrg);
  static const char *wxObsToString(obs_weather_t obs_weather);
  static const char *iceIntensityToString(ice_intens_t ice_intens);
  static const char *iceTypeToString(ice_type_t ice_type);
  static const char *turbFreqToString(turb_freq_t turb_freq);
  static const char *turbIntensityToString(turb_intens_t turb_intens);
  static const char *turbTypeToString(turb_type_t turb_type);

  friend bool operator==(const Pirep &, const Pirep &);
  friend bool operator!=(const Pirep &, const Pirep &);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  static const string _className;

  ti32 _obs_time;			// unix time
  char _report_type[REP_STR_LEN];	// report type -- PIREP or AIREP
  char _acft_id[AC_STR_LEN];		// aircraft ID or tail number
	string _acft_id_string;
  fl32 _latitude;			// degrees
  fl32 _longitude;			// degrees
  fl32 _altitude;			// feet
  si08 _type;				// see pirep_type enum for definitions
    
  sky_cond_t _sky_cond_1;
  sky_cond_t _sky_cond_2;

  wx_obs_t _wx_obs;

  ice_obs_t _ice_obs_1;
  ice_obs_t _ice_obs_2;

  turb_obs_t _turb_obs_1;
  turb_obs_t _turb_obs_2;

  char _raw[PIREP_TEXT_LEN1];
	string _raw_string;

  string _xml;
  MemBuf _memBuf;

  /////////////////////
  // private methods //
  /////////////////////

  bool _checkPirepType(const pirep_type_t& type);
  bool _checkSkyCvrg(const sky_cvrg_t& cvrg);
  bool _checkObsWx(const obs_weather_t& obs);
  bool _checkIceIntens(const ice_intens_t& intens);
  bool _checkIceType(const ice_type_t& type);
  bool _checkTurbFreq(const turb_freq_t& freq);
  bool _checkTurbIntens(const turb_intens_t& intens);
  bool _checkTurbType(const turb_type_t& type);

  // time_t convertStringToTimeT(string in);
  // int _itemToInt(TiXmlElement* in);
  // double _itemToDouble(TiXmlElement* in);
  // time_t _itemToTimeT(TiXmlElement* in);

  void _setSkyCondition(const sky_cond_t &source, sky_cond_t &target);
  void _setIceObs(const ice_obs_t &source, ice_obs_t &target);
  void _setTurbObs(const turb_obs_t &source, turb_obs_t &target);

};

inline bool operator!=(
		       const Pirep &left, 
		       const Pirep &right )
{
  return !(left == right);
}

# endif     /* PIREP_H */
