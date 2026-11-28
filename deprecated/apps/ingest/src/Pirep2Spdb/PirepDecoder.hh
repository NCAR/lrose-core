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
 *  $Id: PirepDecoder.hh,v 1.8 2016/03/07 01:23:04 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	PirepDecoder
// 
// Author:	G. M. Cunning
// 
// Date:	Mon Jan  2 11:07:15 2006
// 
// Description:	Decodes pireps following Greg Thompson's conventions.
// 
//              This decoder can handle voice reports (voicereps), 
//		air reports (aireps) and AMDAR reports (amdar).
//
//		Here is a example of each meassage type:
// voicerep:
//
// ^A^M
// 051 ^M
// UBUS1 KNKA 022300^M
// SZL UA /OV BUM 030020/TM 2256/FL350/TP LJ60/TB MOD/RM FM ZKC =^M
// DCA UA /OV OTT180030 /TM 2300 /FL 390 /TP GLF3 /TB LGTR-MDT=^M
// FAY UA /OV FAY270020 /TM 2255 /FL 370 /TP B757 /TB LGT=^M
// ^M
// ^C
//
// SEQ_NUM = sequence number
// SOME_ID = some kind of ID
// ICAO_ID = ICAO identification number
// BLT_TS = bulletin timestamp
//
// airep:
//
// ^A^M
// 830 ^M
// UANT01 CWAO 022303^M
// ARP AAL141 4919N02741W 2303 F380 MS65 247/62 KT^M
// N775AN QXT AOE2 022303 F34A^M
// ^M
// ^M
// ^C
//
// amdar:




# ifndef    PIREP_DECODER_H
# define    PIREP_DECODER_H

// C++ include files
#include <string>
#include <map>
#include <vector>
#include <iostream>

// System/RAP include files
#include <rapformats/Pirep.hh>
#include<toolsa/udatetime.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaXml.hh>

// Local include files
#include "Params.hh"

using namespace std;

class PirepDecoder {
  
public:

  static const int MAX_LINE = 256;
  static const float MISSING_ALT = -999.0;
  static const float FT_TO_M = 0.3048; 

  static const char START_DELIMITER;
  static const char END_DELIMITER;

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  PirepDecoder();
  PirepDecoder(const PirepDecoder &);

  // destructor
  ~PirepDecoder();

  void _initialize();

  // isOK -- check on class
  bool isOK() { return _isOk; }

  // getErrStr -- returns error message
  string getErrStr() const { return _errStr; }

  //  int getDebug() const { return _debug; }

  PirepDecoder &operator=(const PirepDecoder &);

  void addXmlSpdb(DsSpdb* spdb, Pirep* p, int& expire_secs);

  void addRawSpdb(DsSpdb* spdb, Pirep* p, const time_t& expire_secs);

  int createPirepObject(string in, Pirep& out, Params::xml_names_t xNames);

  int createPirepObject(vector<string> in, Pirep& out);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  
  char fgets_buffer[MAX_LINE];
  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  bool _isOk;
  string _errStr;
  static const string _className;
  //  const int& _debug;

  string _header;
  vector<string> _reps;

  map < string, float > _compassPoints;
  multimap < string, int > _turbulenceKeys;
  multimap < string, int > _turbulenceTypes;
  multimap < string, int > _turbulenceFreq;
  multimap <string,int> _icingKeys;
  multimap <string,int> _icingTypes;
  multimap <string,int> _skyKeys;

  map< string, string > _items;

  /////////////////////
  // private methods //
  /////////////////////
  time_t convertStringToTimeT(string in);
};

# endif     /* PIREP_DECODER_H */
