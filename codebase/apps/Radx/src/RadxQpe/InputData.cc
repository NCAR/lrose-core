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
 * @file InputData.cc
 */
#include "InputData.hh"
#include "Args.hh"
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/DateTime.hh>
#include <didss/DataFileNames.hh>

//----------------------------------------------------------------
InputData::InputData(const Parms &params, const Args &args) :
  Data(params),  _trigger(NULL)
{
  _fileListPos = 0;
  if (args.inputFileList.size() > 0) 
  {
    // filelist mode
    _inputFileList = args.inputFileList;
  }
  else
  {
    if (args._isArchive)
    {
      // archive mode
      _trigger = new DsUrlTrigger(args._archiveT0, args._archiveT1, 
                                  params.data_url, DsUrlTrigger::OBS, false);
    }
    else
    {
      // realtime mode
      _trigger = new DsUrlTrigger(params.data_url, DsUrlTrigger::OBS, false);
    }
  }
}

//----------------------------------------------------------------
InputData::~InputData(void)
{
  if (_trigger != NULL)
  {
    delete _trigger;
  }
}

//----------------------------------------------------------------
bool InputData::nextVolume(time_t &t)
{

  // set up field list

  vector<string> fields;
  for (int i=0; i<_params.numRainRate(); ++i)
  {
    fields.push_back(_params.ithInputPrecipName(i));
  }
  if (_params.hasSnr())
  {
    fields.push_back(_params.snr_field);
  }
  fields.push_back(_params.pid_field);

  if (_inputFileList.size() > 0)
  {
    // filelist mode
    if (_fileListPos >= _inputFileList.size()) {
      return false;
    }
    string path = _inputFileList[_fileListPos];
    _fileListPos++;
    bool dateOnly;
    if (DataFileNames::getDataTime(path, t, dateOnly)) {
      LOG(LogMsg::ERROR, "ERROR - InputData::nextVolume");
      LOG(LogMsg::ERROR, "  Cannot get time from file path");
      LOG(LogMsg::ERROR, path);
    }
    return read(path, fields);
  }
  else
  {
    if (_trigger->nextTime(t))
    {
      LOGF(LogMsg::PRINT, "---Triggered %s ---",  DateTime::strn(t).c_str());
      
      return read(t, fields);
    }
    else
    {
      return false;
    }
  }
  return false;
}
