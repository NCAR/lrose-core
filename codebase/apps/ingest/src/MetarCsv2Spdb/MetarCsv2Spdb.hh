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
 *
 *  @file MetarCsv2Spdb.hh
 *
 *  @class MetarCsv2Spdb
 *
 *  Converts CSV PIREPs to SPDB
 *
 *  @author G McCabe
 * 
 *  @date March, 2013
 *
 *  @version $Id: MetarCsv2Spdb.hh,v 1.4 2016/03/07 01:23:02 dixon Exp $
 */


#ifndef MetarCsv2Spdb_HH
#define MetarCsv2Spdb_HH

// C++ include files
#include <string>
#include <map>
#include <vector>

// System/RAP include files
#include <toolsa/udatetime.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/WxObs.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>

// Local include files
#include "Args.hh"
#include "Params.hh"

using namespace std;

// Forward class declarations
class DsInputPath;

class MetarCsv2Spdb
{

 public:

  static const int MAX_LINE = 256;
  static const int MISSING_INDEX = -9;

 /** constructor */
  MetarCsv2Spdb(int argc, char **argv);
  
  /** destructor */
  ~MetarCsv2Spdb(void);
  
  /** Entry point for execution */
  int run();

	/** Add XML METAR chunk to SPDB */
  void addXmlSpdb(DsSpdb* spdb, WxObs* m, int& expire_secs);

	/**
	 *  fill Metar object with data
	 *
	 * @param[in] in a collection of one tokenized line from the CSV
	 * 
	 * @param[out] out the Metar object to fill 
	 */
  int fillMetarObject(const vector<string>& in, WxObs& out);
  
	/** Flag indicating whether the program status is currently okay. 
	 * set when constructor exits in a bad state.
	 */
  bool isOK;

 private:
  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  // Input objects

  DsInputPath *_inputPath;
	bool _useStdin;

	// field indicies
	int _iIcao;
	int _iLat;
	int _iLon;
	int _iAlt;
	int _iObsTime;
	int _iReportTime;
	int _iSurfaceTemp;
	int _iDewPtTemp;
	int _iWindDir;
	int _iWindSpeed;
	int _iWindGust;
	int _iHorizVisibility;
	int _iAltimeter;
	int _iSeaLvlPressure;
	int _iQcField;
	int _iCorrected;
	int _iAuto;
	int _iAutoStation;
	int _iMaintenanceOn;
	int _iNoSignal;
	int _iLightningOff;
	int _iFreezingRainOff;
	int _iPresentWeatherOff;
	int _iPresentWeather;
	int _iCloudCoverage1;
	int _iCloudCoverage2;
	int _iCloudCoverage3;
	int _iCloudCoverage4;
	int _iCloudCoverage5;
	int _iCloudCoverage6;
	int _iCloudBase1;
	int _iCloudBase2;
	int _iCloudBase3;
	int _iCloudBase4;
	int _iCloudBase5;
	int _iCloudBase6;
	int _iPressureTendency;
	int _iMaxT;
	int _iMinT;
	int _iMaxT24hr;
	int _iMinT24hr;
	int _iPrecip;
	int _iPrecip3hr;
	int _iPrecip6hr;
	int _iPrecip24hr;
	int _iSnow;
	int _iVerticalVisibility;
        int _iCeilingLow;
	int _iMetarType;
	int _iRawText;
	int _iRemarks;

  multimap <string,double> _skyKeys;

  // functions
	void _clearIndicies();
	void _initializeKeys();
  int _writeSpdb(DsSpdb &spdb, string url);
	/** Parse the header line of the CSV file to find indices of fields */
	bool _parseHeader(string line);
  time_t convertStringToTimeT(string in);
  string trimWhitespace(string s);
};



#endif


