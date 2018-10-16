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
 * @file AsciiOutput.cc
 */
#include "AsciiOutput.hh"

#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/LogStream.hh>

#include <cstdio>
#include <cstdlib>

using std::string;

/*----------------------------------------------------------------*/
AsciiOutput::AsciiOutput(const std::string &name,
			 const string &dir, const time_t &t)
{
  _name = name;

  _time = t;
  DateTime dt(_time);
  string ymd = dt.getDateStrPlain();
  string hms = dt.getTimeStrPlain();

  _dir = dir;

  _fileName = ymd;
  _fileName += "_";
  _fileName += hms;
  _fileName += ".humidity.ascii";

  _relPath = ymd;
  _relPath += PATH_DELIM;
  _relPath += _fileName;

  _path = dir + PATH_DELIM;
  _path += _relPath;

  Path P(_path);
  P.makeDirRecurse();

}

/*----------------------------------------------------------------*/
AsciiOutput::~AsciiOutput()
{
}

/*----------------------------------------------------------------*/
void AsciiOutput::clear(void) const
{
  Path P(_path);
  if (P.pathExists())
  {
    string cmd = "\\rm " + _path;
    system(cmd.c_str());
  }
}

/*----------------------------------------------------------------*/
void AsciiOutput::append(const string &s) const
{
  FILE *fp = fopen(_path.c_str(), "a");
  if (fp == NULL)
  {
    LOG(ERROR) << "error opening " << _path;
    return;
  }
  fprintf(fp, "%s\n", s.c_str());
  fclose(fp);
}

/*----------------------------------------------------------------*/
void AsciiOutput::appendNoCr(const string &s) const
{
  FILE *fp = fopen(_path.c_str(), "a");
  if (fp == NULL)
  {
    LOG(ERROR) << "error opening " << _path;
    return;
  }
  fprintf(fp, "%s", s.c_str());
  fclose(fp);
}

/*----------------------------------------------------------------*/
void AsciiOutput::writeLdataInfo()
{

  if (!Path::exists(_path.c_str())) {
    return;
  }

  DsLdataInfo ldata;
  ldata.setDir(_dir);
  ldata.setRelDataPath(_relPath);
  ldata.setWriter("REPoH");
  ldata.setDataFileExt("ascii");
  ldata.setDataType("ascii");
  if (ldata.write(_time)) {
    cerr << "ERROR - AsciiOutput::write_ldata_info()" << endl; 
    cerr << "  Cannot write _latest_data_info" << endl;
    cerr << ldata.getErrStr() << endl;
  }
  
}

