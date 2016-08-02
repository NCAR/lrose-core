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
 * @file LdataPathParseWriter.cc
 */
#include <toolsa/DateTime.hh>
#include <dsserver/DsLdataInfo.hh>
#include "LdataPathParseWriter.hh"
using std::string;

//-------------------------------------------------------------------
LdataPathParseWriter::LdataPathParseWriter(int argc, char **argv)
{
  isOK = true;

  // set programe name
  _progName = "LdataPathParseWriter";

  // get command line args
  if (_args.parse(argc, argv, _progName))
  {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }
  return;
}

//-------------------------------------------------------------------
LdataPathParseWriter::~LdataPathParseWriter()
{
}

//-------------------------------------------------------------------
int LdataPathParseWriter::run(void)
{
  // parse the path to get the needed fields
  string s = _args._path;
  size_t suffixSize = _args._suffix.size();
  size_t timeSize;
  if (_args._isFcast)
  {
    // yyyymmdd/g_hhmmss/f_vvvvvvvv.<suffix>
    timeSize = 28;
  }
  else
  {
    // yyyymmdd/hhmmss.<suffix>
    timeSize = 15;
  }

  // the part of the path that precedes time information is leadPath,
  // add 1 for the '.' between the time information and suffix
  string leadPath = s.substr(0, s.size() - (timeSize + suffixSize + 1));

  // The part of the path that is after the leadPath:
  string relPath = s.substr(leadPath.size());

  // parse the time information 
  string timeInfo = relPath.substr(0, relPath.size()- suffixSize - 1);

  int y, m, d, h, min, sec, lt;
  if (_args._isFcast)
  {
    if (sscanf(timeInfo.c_str(), "%04d%02d%02d/g_%02d%02d%02d/f_%08d",
	       &y, &m, &d, &h, &min, &sec, &lt) != 7)
    {
      cerr << "ERROR -  cannot parse forecast time info" << endl;
      return -1;
    }      
  }
  else
  {
    if (sscanf(timeInfo.c_str(), "%04d%02d%02d/%02d%02d%02d",
	       &y, &m, &d, &h, &min, &sec) != 6)
    {
      cerr << "ERROR -  cannot parse forecast time info" << endl;
      return -1;
    }      
    lt = 0;
  }

  DateTime dt(y, m, d, h, min, sec);
  time_t t = dt.utime();

  // create Ldata file object in leadPath (top) directory
  DsLdataInfo ldata(leadPath, _args._debug);
  if (_args._debug)
  {
   ldata.setDebug(true);
  }

  // set object from command line args
  _setFromArgs(ldata, lt, relPath);

  // write the latest data info 
  int iret = 0;
  if (ldata.write(t, _args._dataType))
  {
    cerr << "ERROR -  Cannot write ldata file to dir: " << leadPath << endl;
    iret = -1;
  }
  else
  {
    if (_args._debug)
    {
      cerr << "Writing to " << leadPath 
           << ", time: " << DateTime::str(t) << endl;
    }
  }
  return iret;
}

//-------------------------------------------------------------------
void LdataPathParseWriter::_setFromArgs(DsLdataInfo &ldata,
					const int lt,
					const std::string &relPath)
{
  
  ldata.setDataFileExt(_args._fileExt.c_str());
  ldata.setRelDataPath(relPath);
  ldata.setWriter("LdataPathParseWriter");

  if (_args._dataType.size() > 0)
  {
    ldata.setDataType(_args._dataType.c_str());
  }

  if (_args._isFcast)
  {
    ldata.setIsFcast();
    ldata.setLeadTime(lt);
  }
}

