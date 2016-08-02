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
/*********************************************************************
 * UfRecord.hh
 *
 * Object represting a single UF record
 *
 *********************************************************************/

#ifndef UF_RECORD_HH
#define UF_RECORD_HH

#include <rapformats/UfRadar.hh>
#include <vector>
using namespace std;
class DsRadarMsg;

class UfRecord

{

public:

  // constructor
  
  UfRecord();
  
  // destructor
  
  virtual ~UfRecord();

  // set debugging on

  void setDebug(bool state) { _debug = state; }
 
  // clear all data
  
  void clear();

  // free up data

  void free();

  // disassemble the object from a raw UF buffer
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int nBytes);

  // field translation

  typedef struct {
    string input_name;
    string uf_name;
    int scale;
  } field_tranlation_t;
  
  // load from DsRadarMsg
  // Returns 0 on success, -1 on failure
  
  int loadFromDsRadarMsg(const DsRadarMsg &radarMsg,
			 int rayNumInVol,
			 const vector<field_tranlation_t> &fieldTrans);
    
  // Write record to open file
  // Returns 0 on success, -1 on failure
  
  int write(FILE *fp, bool littleEndian = false);

  // print record
  
  void print(ostream &out,
	     bool print_headers,
	     bool print_data);

  // print data members derived from a disassemble

  void printDerived(ostream &out) const;

  // check for all missing data
  // returns true if all data is missing

  bool allDataMissing();

  // data
  
  UF_mandatory_header_t manHdr;
  UF_data_header_t dataHdr;
  vector<UF_field_info_t> fieldInfo;
  vector<UF_field_header_t> fieldHdrs;
  vector<string> fieldNames;
  vector<MemBuf> fieldData;
  
  int maxGates;
  int numSamples;
  int polarizationCode;
  double startRange;
  double gateSpacing;
  double radarConstant;
  double noisePower;
  double receiverGain;
  double peakPower;
  double antennaGain;
  double pulseWidth;
  double horizBeamWidth;
  double vertBeamWidth;
  double wavelength;
  double prf;
  double nyquistVel;

  int volNum;
  int tiltNum;
  double targetAngle;

  double elevation;
  double azimuth;
  time_t beamTime;

private:

  bool _debug;

};

#endif

