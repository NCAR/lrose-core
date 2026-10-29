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
// MM5DataV2.hh
//
// Read in an MM5 V2 file
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1999
//
/////////////////////////////////////////////////////////////

#ifndef MM5DataV2_H
#define MM5DataV2_H


#include <mm5/MM5Data.hh>
using namespace std;

class MM5DataV2 : public MM5Data {
  
public:
  
  ///////////////
  // Constructor
  //
  // If flight_level is non-NULL, the freezing level is computed as
  // flight level.
  // If flight_level is NULL, the freezing level is computed as mb.
  // If you want registration with the procmap, set the heartbeat_func
  //    to PMU_auto_register() from toolsa.
  
  MM5DataV2 (const string &prog_name,
	     const string &path,
	     bool debug = false,
	     const heartbeat_t heartbeat_func = NULL,
	     bool dbzConstantIntercepts = true);

  /////////////
  // destructor

  virtual ~MM5DataV2();
  
  //////////////////////////
  // find the header records

  virtual void findHdrRecords();

  /////////////////
  // read file

  virtual int read();

  ///////////////////////////////////
  // get data set num, len and offset
  
  long getDataSetNum() { return _dataSetNum; }
  long getDataSetLen() { return _dataSetLen; }
  long getDataSetOffset() { return _dataSetOffset; }

  ////////////////
  // print headers

  virtual void printHeaders(FILE *out) const;

  // get the FDDA start and end times

  virtual double getFddaStartTime() const;
  virtual double getFddaEndTime() const;

  // get MIF or MRF values, given the label

  virtual int getMifVal(const char *label) const;
  virtual double getMrfVal(const char *label) const;

protected:

  static int MM5_HEADER_LEN;

  char *_mifc[20000];
  char *_mrfc[20000];

  int _n3d;
  int _n2d;
  
  int _fieldDataLen;
  int _dataSetNum;
  long _dataSetOffset;
  int _dataSetLen;
  int _nForecasts;

  // functions

  int _readMainHeaders();
  void _freeLabels();
  int _readLabels();
  int _readTimes();
  int _readDataSetTimes(int data_set_num, time_t &model_run_time,
			time_t &forecast_time, int &lead_time);
  int _readDataSet();

  long _offset3d(int field_num_3d);
  long _offset2d(int field_num_2d);

  int _read2dField(int field_num_2d,
		   const char *field_name, fl32 ***field_p);

  int _read3dField(int field_num_3d,
		   const char *field_name, fl32 ****field_p);

  int _readUv2dot(int field_num_3d,
		  const char *field_name, fl32 ****field_p);
  
  int _read2dField2dot(int field_num_2d,
		       const char *field_name, fl32 ***field_p);

  int _readwField();

  void _convertPstar();

private:

};

#endif

