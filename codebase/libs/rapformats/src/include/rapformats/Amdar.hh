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
 *  $Id: Amdar.hh,v 1.13 2016/03/03 19:23:53 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	Amdar
// 
// Author:	G. M. Cunning
// 
// Date:	Mon Mar 19 12:06 2012
// 
// Description:	This class manages Aircraft Meteorological Data Relay
//		(AMDAR) messages. See WMO AMDAR Reference Manual for
//		details.
// 
//

# ifndef    AMDAR_H
# define    AMDAR_H

// C++ include files
#include <string>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <vector>
#include <toolsa/MemBuf.hh>

// System/RAP include files

// Local include files


class Amdar {
  
public:

  ////////////////////
  // public members //
  ////////////////////

  //
  // list variables based on FM-94 BUFR table B table references and the ASCII
  // (FM-42) formatted messages
  // 

  // BUFR (FM-94)

  // (3 11 001) aircraft bulletins (ASDAR)
  typedef struct {             // F X  Y
    std::string acIdentifier;  // 0 01 006
    std::string navSystem;     // 0 02 061 -- converted to string
    int year;                  // 0 04 001
    int month;                 // 0 04 002
    int day;                   // 0 04 003
    int hour;                  // 0 04 004
    int minute;                // 0 04 005
    double latitude;           // 0 05 001
    double longitude;          // 0 06 001
    std::string phaseFlight;   // 0 08 004 -- converted to string
    double altitude;           // 0 07 002
    double temperature;        // 0 12 001
    double windDirection;      // 0 11 001
    double windSpeed;          // 0 11 002
    std::string degreeTurb;    // 0 11 031 -- converted to string 
    double turbBaseHgt;        // 0 11 032
    double turbTopHgt;         // 0 11 033
    std::string airframeIcing; // 0 20 041 -- converted to string 
  } bufr_ac_bulletin_t; 

  // (3 01 065) ACARS identification
  typedef struct {               // F X  Y
    std::string flightNum;       // 0 01 006
    std::string acRegNum;        // 0 01 008
    std::string stationType;     // 0 02 001 -- converted to string 
    std::string windInstType;    // 0 02 002 -- converted to string 
    double precisionTemp;        // 0 02 005
    std::string acDataRelayType; // 0 02 062 -- converted to string
    std::string origLatLonSpec;  // 0 02 070 -- converted to string
    std::string grndRecvStatn;   // 0 02 065
  } bufr_acars_ident_t; 

  // (3 01 066) ACARS location
  typedef struct {              // F X  Y   
    int year;                   // 0 04 001
    int month;                  // 0 04 002
    int day;                    // 0 04 003
    int hour;                   // 0 04 004
    int minute;                 // 0 04 005
    int second;                 // 0 04 006
    double latitude;            // 0 05 001
    double longitude;           // 0 06 001
    double pressure;            // 0 07 004
    std::string phaseFlight;    // 0 08 004 -- converted to string
  } bufr_acars_loc_t; 

  // (3 11 003) ACARS reported variables
  typedef struct {           // F X  Y
    double indAcAlt;         // 0 10 070
    double windDirection;    // 0 11 001
    double windSpeed;        // 0 11 002
    double temperature;      // 0 12 001
    double mixingRatio;      // 0 13 002  
  } bufr_rep_acars_var_t;   

  // (3 11 004) ACARS supplementary reported variables
  typedef struct {                            // F X  Y
    std::vector<double> verticalGustVel;      // 0 11 034
    std::vector<double> verticalGustAccel;    // 0 11 035
    std::vector<double> meanTurbIntens;       // 0 11 075
    std::vector<double> peakTurbIntens;       // 0 11 076
    std::vector<std::string> acarsInterpVals; // 0 33 025 -- converted to string
    std::vector<std::string> moistureQuality; // 0 33 026 -- converted to string  
 } bufr_sup_acars_var_t; 

  // (3 11 002) standard ACARS bulletins
  typedef struct {
    bufr_acars_ident_t identification;
    bufr_acars_loc_t location;
    bufr_rep_acars_var_t bulletinVars;
    bufr_sup_acars_var_t suppBulletinVars;
  } bufr_std_acars_bulletin_t; 

  // (3 11 007) aircraft data for one level with lat/lon
  typedef struct {
    double flightLevel;         // 0 07 010
    double latitude;            // 0 05 001
    double longitude;           // 0 06 001
    double windDirection;       // 0 11 001
    double windSpeed;           // 0 11 002
    std::string rollAngQuality; // 0 02 064 -- converted to string
    double temperature;         // 0 12 001
    double dewpoint;            // 0 12 003 
  } bufr_acft_data_latlon_t; 

  // (3 11 006) AMDAR & aircraft ascent/descent profile data for one level without lat/lon
  typedef struct {              // F X  Y
    double flightLevel;         // 0 07 010
    double windDirection;       // 0 11 001
    double windSpeed;           // 0 11 002
    std::string rollAngQuality; // 0 02 064 -- converted to string
    double temperature;         // 0 12 001
    double dewpoint;            // 0 12 003 
  } bufr_acft_data_no_latlon_t; 

  // (3 11 009) aircraft ascent/descent profile data for one level with lat/lon
  typedef struct {             // F X  Y
    std::string acIdent;       // 0 01 008
    int year;                  // 0 04 001
    int month;                 // 0 04 002
    int day;                   // 0 04 003
    int hour;                  // 0 04 004
    int minute;                // 0 04 005
    int second;                // 0 04 006
    double latitude;           // 0 05 001
    double longitude;          // 0 06 001
    std::string phaseFlight;   // 0 08 004 -- converted to string
    std::vector<bufr_acft_data_latlon_t> bulletins;
  } bufr_acft_profile_latlon_t; 

  // (3 11 008) aircraft ascent/descent profile data for one level with lat/lon
  typedef struct {             // F X  Y
    std::string acIdent;       // 0 01 008
    int year;                  // 0 04 001
    int month;                 // 0 04 002
    int day;                   // 0 04 003
    int hour;                  // 0 04 004
    int minute;                // 0 04 005
    int second;                // 0 04 006
    double latitude;           // 0 05 001
    double longitude;          // 0 06 001
    std::string phaseFlight;   // 0 08 004 -- converted to string
    std::vector<bufr_acft_data_no_latlon_t> bulletins;
  } bufr_acft_profile_no_latlon_t; 

  // (3 11 005) standard AMDAR bulletin
  typedef struct {                  // f X  Y
    std::string acIdent;            // 0 01 008
    int sequenceNum;                // 0 01 023
    double latitude;                // 0 05 001
    double longitude;               // 0 06 001
    time_t issueTime;
    double flightLevel;             // 0 07 010
    std::string detailPhaseFlight;  // 0 08 009  -- converted to string 
    double windDirection;           // 0 11 001
    double windSpeed;               // 0 11 002
    int degreeTurb;                 // 0 11 031 
    double derEqVertGustVel;        // 0 11 036
    double temperature;             // 0 12 101
    std::string acarsInterpVal;     // 0 33 025 -- converted to string
  } bufr_std_amdar_bulletin_t; 

  // ASCII (FM-42)

  // standard AMDAR
  typedef struct {
    std::string text;            // original bulletin text
    std::time_t issueTime;    // converted bulletin time
    std::string phaseFlight;     // phase of the flight
    std::string acIdent;         // aircraft identifier
    double latitude;             // minute precision in degrees
    double longitude;            // minute precision in degrees
    double pressAltitude;        // hundreds of feet precision in feet
    double temperature;          // tenth of degree C precision in degrees
    double dewPoint;             // tenth of degree C precision in degrees
    double relativeHumidity;     // percent 
    double windDirection;        // deg. from true north
    double windSpeed;            // knots
    int turbulenceCode;          // turbulence code  
    int navSys;                  // navigation system code.
    int typeNavSys;              // type navigation system code
    int tempPrec;                // temperature precision
    bool hasSection3;            // flag to indicate sec. 3 is present
    double sec3presAlt;          // section 3 pressure alt. from ACARS
    double verticalGust;         // tenths m/s precision in m/s
  } ascii_std_amdar_bulletin_t; 

  // code tables for the FM-42 messages

  // tubulence codes
  typedef enum {
    TURB_CODE_NONE = 0,
    TURB_CODE_LIGHT = 1,
    TURB_CODE_MODERATE = 2,
    TURB_CODE_SEVERE = 3
  } turb_code_t;

  // navigation system type
  typedef enum {
    NAV_SYS_INERTIAL = 0,
    NAV_SYS_OMEGA = 1
  } nav_sys_code_t;

  // reporting system configuration
  typedef enum {
    REP_SYS_ASDAR = 0,
    REP_SYS_ASDAR_ACARS_NO_OP = 1,
    REP_SYS_ASDAR_ACARS_OP = 2,
    REP_SYS_ACARS = 3,
    REP_SYS_ACARS_ASDAR_NO_OP = 4,
    REP_SYS_ACARS_ASDAR_OP = 5
  } report_sys_code_t;

  typedef enum {
    TEMP_PRECISION_LOW = 0, // 2 degrees C
    TEMP_PRECISION_HIGH = 1 // 1 degree C
  } temp_precision_code_t;

  // missing data value
  static const float MISSING_VALUE;

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  Amdar();

  Amdar(const ascii_std_amdar_bulletin_t& ascii_amdar);
  Amdar(const bufr_std_amdar_bulletin_t& bufr_amdar);

  Amdar(const Amdar &that);

  // destructor
  virtual ~Amdar();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar::clear
  //
  // Description:	clear the object
  //
  // Returns:		none
  //
  // Notes:	
  //
  void clear();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar::loadXml
  //
  // Description:	load from XML representation of object
  //
  // Returns:		none
  //
  // Notes:	
  //
  void loadXml(string &xml, int startIndentLevel = 0) const;
  

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar::disassemble
  //
  // Description:	Disassembles a buffer, sets the object values.
  //
  // Returns:		returns 0 for succes, and -1 on failure
  //
  // Notes:	
  //
  int disassemble(const void *buf, int len);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar::assemble
  //
  // Description:	Load up an XML buffer from the object.
  //
  // Returns:		none
  //
  // Notes:	
  //
  void assemble();


  // get the assembled buffer pointer

  void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }


	

  // getters
  time_t getIssueTime() const {
    if(_isAscii) {
      return _asciiBulletin.issueTime;
    }
    else {
      return _bufrBulletin.issueTime;
    }
  } 

  double getLatitude() const {
    if(_isAscii) {
      return _asciiBulletin.latitude;
    }
    else {
      return _bufrBulletin.latitude;
    }
  } 

  double getLongitude() const {
    if(_isAscii) {
      return _asciiBulletin.longitude;
    }
    else {
      return _bufrBulletin.longitude;
    }
  } 

  string getAircraftId() const {
    if(_isAscii) {
      return _asciiBulletin.acIdent;
    }
    else {
      return _bufrBulletin.acIdent;
    }
  }

	double getPressAltitude() const {
    if(_isAscii) {
      return _asciiBulletin.pressAltitude;
    }
    else {
      return _bufrBulletin.flightLevel;
    }
  }

	int getTurbulenceCode() const {
    if(_isAscii) {
      return _asciiBulletin.turbulenceCode;
    }
    else {
      return 0;
    }
  }
	double getTemperature() const {
    if(_isAscii) {
      return _asciiBulletin.temperature;
    }
    else {
      return _bufrBulletin.temperature;
    }
  }
	double getDewPoint() const {
    if(_isAscii) {
      return _asciiBulletin.dewPoint;
    }
    else {
      return 0;
    }
  }
	double getRelativeHumidity() const {
    if(_isAscii) {
      return _asciiBulletin.relativeHumidity;
    }
    else {
      return 0;
    }
  }
	double getWindSpeed() const {
    if(_isAscii) {
      return _asciiBulletin.windSpeed;
    }
    else {
      return _bufrBulletin.windSpeed;
    }
  }
	double getWindDir() const {
    if(_isAscii) {
      return _asciiBulletin.windDirection;
    }
    else {
      return _bufrBulletin.windDirection;
    }
  }

	double getVerticalGust() const {
    if(_isAscii) {
      return _asciiBulletin.verticalGust;
    }
    else {
      return 0;
    }
  }

  string getFlightPhase() const {
    if(_isAscii) {
      return _asciiBulletin.phaseFlight;
    }
    else {
      return _bufrBulletin.detailPhaseFlight;
    }
  }

  string getText() const {
    if(_isAscii) {
      return _asciiBulletin.text;
    }
    else {
      return 0;
    }
  }

  string getTranslatedText() const {
    if (_isAscii) {
      return _getAsciiTranslatedText();
    }
    else {
      return _getBufrTranslatedText();
    }
  }

  // setters

  void setLatitude(const double lat) {
    if(_isAscii) {
      _asciiBulletin.latitude = lat;
    }
    else {
      _bufrBulletin.latitude = lat;
    }
  } 

  void setLongitude(const double lon) {
    if(_isAscii) {
      _asciiBulletin.longitude = lon;
    }
    else {
      _bufrBulletin.longitude = lon;
    }
  } 

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar::print
  //
  // Description:	Prints the message 
  //
  // Returns:		none
  //
  // Notes:	
  //
  void print(ostream &out, string spacer = "") const;

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	Amdar::printAsXml
  //
  // Description:	Prints and XML-formatted representation of 
  //			the object 
  //
  // Returns:		none
  //
  // Notes:	
  //
  void printAsXml(ostream &out, int startIndentLevel = 0) const;

  Amdar &operator=(const Amdar & amdar);

  // use defualt
  //  friend bool operator==(const Amdar &left, const Amdar &right);


protected:
private:

  /////////////////////
  // private members //
  /////////////////////

  static const string _className;
  
  // ASCII (FM-42) bulletins
  bool _isAscii;
  ascii_std_amdar_bulletin_t _asciiBulletin;

  // BUFR (FM-94) bulletins
  bool _isBufr;
  bufr_std_amdar_bulletin_t _bufrBulletin;

  // xml

  mutable string _xml;
  
  // buffer for assemble / disassemble

  MemBuf _memBuf;

  /////////////////////
  // private methods //
  /////////////////////

  void _copy(const Amdar &from);

  void _clearAsciiBulletin();

  void _clearBufrBulletin();

  void _printAsciiBulletin(ostream &out, string spacer = "") const ;

  void _printBufrBulletin(ostream &out, string spacer = "") const ;

  void _loadXmlAsciiBulletin(string &xml, int startIndentLevel = 0) const;

  void _loadXmlBufrBulletin(string &xml, int startIndentLevel = 0) const;

  void _disassembleAsciiBulletin(const string& amdar_str);

  void _disassembleBufrBulletin(const string& amdar_str);

  string _turbCode2Str(int code) const ;

  string _getAsciiTranslatedText() const ;

  string _getBufrTranslatedText() const ;

};

bool operator==(const Amdar &left, 
		const Amdar &right);

inline bool operator!=(const Amdar &left, 
		       const Amdar &right)
{
  return !(left == right);
}

# endif     /* AMDAR_H */
