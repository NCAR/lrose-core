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
// Field class
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2010
//
///////////////////////////////////////////////////////////////

#ifndef Field_hh
#define Field_hh

#include <string>
#include <vector>
#include <iostream>
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/DsRadarBeam.hh>
#include <toolsa/MemBuf.hh>
using namespace std;

class Field {

public:
  
  typedef enum {
    CHAN1,
    CHAN2,
    MEAN
  } source_t;
  
  // constructor
  
  Field(const string &inputName, const string &outputName,
        const DsRadarParams radarParams,
        source_t source);
  
  // destructor
  
  ~Field();

  // set methods

  void setIsOutput(bool val) { _isOutput = val; }
  void setParams(const DsFieldParams &params) { _params = params; }

  // get methods

  inline const string &getInputName() const { return _inputName; }
  inline const string &getOutputName() const { return _outputName; }
  inline bool getIsOutput() const { return _isOutput; }
  inline source_t getSource() const { return _source; }
  inline const DsFieldParams &getParams() const { return _params; }
  inline const void *getData() const { return _dataBuf.getPtr(); }

  // load up data

  void loadMissing(const DsRadarParams &radarParams);
  
  void loadUnchanged(const DsRadarParams &radarParams,
                     const vector<DsFieldParams*> &fp,
                     const DsRadarBeam &beam);
  
  void loadMean(const DsRadarParams &radarParams,
                const vector<DsFieldParams*> &fp1,
                const DsRadarBeam &beam1,
                const vector<DsFieldParams*> &fp2,
                const DsRadarBeam &beam2);

  // print

  void print(ostream &out) const;
  
  // get string representation of source
  
  static string sourceToStr(source_t source);

protected:
private:
  
  string _inputName;
  string _outputName;
  bool _isOutput;
  const DsRadarParams &_radarParams;
  source_t _source;
  DsFieldParams _params;
  MemBuf _dataBuf;

};

#endif

