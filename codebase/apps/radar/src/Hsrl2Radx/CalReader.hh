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
// CalReader.hh
//
// CalReader object
//
// Brad Schoenrock, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2017
//
///////////////////////////////////////////////////////////////

#ifndef CalReader_HH
#define CalReader_HH

#include <Radx/RadxTime.hh>
#include <vector>
#include <ostream>

using namespace std;

class CalReader{
private: 
  string varName;
  string varUnits;
  vector<RadxTime> time;
  vector<string> dataStr;
  vector< vector<double> > dataNum;
  bool isNum, isStr;
public:

  CalReader();
  CalReader(string inName, string inUnits, vector< RadxTime > inTime, 
	    vector<string> inDataStr);//constructor for string type data
  CalReader(string inName, string inUnits, vector< RadxTime > inTime, 
	    vector< vector<double> > inDataNum);//constructor for num type data
 
  void setVarName(string inName);
  void setVarUnits(string inUnits);
  void setTime(vector<RadxTime> inTime);
  void addTime(RadxTime inTime);
  void setDataStr(vector<string> inDataStr);
  void addDataStr(string inDataStr);
  void setDataNum(vector< vector<double> > inDataNum);
  void addDataNum(vector<double> inDataNum);
  void setIsStr();
  void setIsNum();
  bool dataTypeisNum();
  bool dataTypeisStr(); 
  string getVarName();
  string getVarUnits();
  vector<RadxTime> getTime();
  vector<string> getDataStr();
  const vector< vector<double> > &getDataNum() const;
  void printBlock(ostream &out);
  CalReader sortTime(CalReader toSort);
  int dateMatch(const CalReader &calIn, const RadxTime &check) const;

  CalReader readCalVals(const char* file, const char* variable);
  string removeWhitespace(string s);
  int checkForChar(string subSt, string s);


  ~CalReader();
    
};
#endif
