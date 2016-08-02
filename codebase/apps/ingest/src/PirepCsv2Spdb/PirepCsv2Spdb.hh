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
 *  @file PirepCsv2Spdb.hh
 *
 *  @class PirepCsv2Spdb
 *
 *  Converts CSV PIREPs to SPDB
 *
 *  @author G McCabe
 * 
 *  @date March, 2013
 *
 *  @version $Id: PirepCsv2Spdb.hh,v 1.10 2016/03/07 01:23:04 dixon Exp $
 */


#ifndef PirepCsv2Spdb_HH
#define PirepCsv2Spdb_HH

// C++ include files
#include <string>
#include <map>
#include <vector>

// System/RAP include files
#include <toolsa/udatetime.h>
#include <Spdb/DsSpdb.hh>
#include <rapformats/Pirep.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>

// Local include files
#include "Args.hh"
#include "Params.hh"

using namespace std;

// Forward class declarations
class DsInputPath;

class PirepCsv2Spdb
{

 public:

  static const int MAX_LINE = 256;
  static const int MISSING_INDEX = -9;

 /** constructor */
  PirepCsv2Spdb(int argc, char **argv);
  
  /** destructor */
  ~PirepCsv2Spdb(void);
  
  /** Entry point for execution */
  int run();

	/** Add XML PIREP chunk to SPDB */
  void addXmlSpdb(DsSpdb* spdb, Pirep* p, int& expire_secs);
	/** Add ASCII PIREP chunk to SPDB */
  void addRawSpdb(DsSpdb* spdb, Pirep* p, const time_t& expire_secs);

  void addAssumptions(Pirep& out);
	/**
	 *  fill Pirep object with data
	 *
	 * @param[in] in a collection of one tokenized line from the CSV
	 * 
	 * @param[out] out the Pirep object to fill 
	 */
  int fillPirepObject(const vector<string>& in, Pirep& out);
  
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
	int _iTime;
	int _iLat;
	int _iLon;
	int _iAlt;
	int _iRaw;
	int _iAircraft;
	int _iType;
	int _iIceType;
	int _iIceInt;
	int _iIceTop;
	int _iIceBase;
	int _iIceType2;
	int _iIceInt2;
	int _iIceTop2;
	int _iIceBase2;
	int _iTurbType;
	int _iTurbInt;
	int _iTurbFreq;
	int _iTurbTop;
	int _iTurbBase;
	int _iTurbType2;
	int _iTurbInt2;
	int _iTurbFreq2;
	int _iTurbTop2;
	int _iTurbBase2;
	int _iSkyCover;
	int _iCloudTop;
	int _iCloudBase;
	int _iSkyCover2;
	int _iCloudTop2;
	int _iCloudBase2;
	int _iTemp;
	int _iWindDir;
	int _iWindSpeed;
	int _iVisibility;

  // Decoder
  multimap < string, int > _turbulenceKeys;
  multimap < string, int > _turbulenceTypes;
  multimap < string, int > _turbulenceFreq;
  multimap <string,int> _icingKeys;
  multimap <string,int> _icingTypes;
  multimap <string,int> _skyKeys;

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


