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
/*
  readObsAXF.h
  Class to parse axf format obs data into obsData class containers
  Passes fixedObsData instances to the obsDataMng singleton

  Reads Field declarations section to get fields available, names and 
  positions to locate required pre-defined field names

  This is a somewhat incomplete implementation, e.g. assumes "," delimiter
  and doesn't parse all of the meta data
*/

#ifndef __READAXFOBS_H
#define __READAXFOBS_H

#include <string>
#include <vector>
#include <Spdb/DsSpdb.hh>
#include "Params.hh"

enum axfFieldTypes { axf_undefined, axf_string, axf_int, axf_float };

class axfFieldData
{
 public:
  axfFieldData();
  axfFieldData(char *fieldname, axfFieldTypes fieldtype, 
	       int fieldpos = -1);
  void set(char *fieldname, axfFieldTypes fieldtype, 
	   int fieldpos = -1);
  void init();
  std::string fieldName;
  axfFieldTypes fieldType;
  int fieldPos;
  bool valid;
  bool readFieldData(char *fieldstr, int pos); // read description line
                                           // pos is line no./field pos
  bool fieldNameMatch(char *matchstr);
};
  

class obsFieldOfsWid
{
 public:
  int ofs, wid;
};

class readObsAXF
{
public:
  readObsAXF(const Params &params);
  virtual ~readObsAXF();
  int readFile(const char *axfname,
	       DsSpdb &spdb,
	       time_t start_time = 0,
	       time_t end_time = 0); // rd file return # of obs, -1 on err
private:
  const Params &_params;
  std::string axfName;
  FILE *axfFile;
  std::vector<axfFieldData> fieldData;
  int nameF, stationF, stnF, siteF, timeF, latF, longF, htF, 
    wdF, wsF, wgF, spF, tmpF, dpF, r9F, r10F, visF, relhF;
  void resetFPos();  // reset field positions
  bool readDescr();  // get field posns from AXF Description
  int readObs(DsSpdb &spdb,
	      time_t start_time = 0,
	      time_t end_time = 0);     // read AXF Surface obs entries
  bool readObsString(DsSpdb &spdb,
		     char *obsstr,
		     time_t start_time = 0,
		     time_t end_time = 0);
  bool checkFPos(char *obsstring);  // check 1st obs line for field pos 
  std::vector<obsFieldOfsWid> fieldOfs; // char offsets of each field in an Obs line
  std::string delimiter;
  int fieldCount;
  time_t lastReadStartTime, thisReadStartTime, lastReadEndTime, thisReadEndTime;
  int lastReadObsCount, lastReadAddedCount;
  int debug;
  int getFieldOfs(std::string &obsString);  // get offsets for each field of obs str
                                     // results in fieldsOfs
  int getFieldInt(std::string &obsString, 
		  int fieldnum);     // returns int val for field
  float getFieldFloat(std::string &obsString, 
		      int fieldnum); // returns flaot value for field
  std::string getFieldString(std::string &obsString, 
			     int fieldnum); // copy string
  time_t getFieldTime(std::string &obsString, 
		    int fieldnum); // copy string
};

#endif
