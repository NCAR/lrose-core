// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2021                                         
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
// TimeNavModel.cc
//
// Storage of archive file names indexed by date and time
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////
//
// Holds the archive file names
// 
///////////////////////////////////////////////////////////////

#include <string>

#include "TimeNavModel.hh"
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxPath.hh>

//
// TODO: set the selected time _selectedTime
//

///////////////////////////////////////
// set input file list for archive mode

void TimeNavModel::setArchiveFileList(const vector<string> &list)
{

  if (list.size() <= 0) {
    throw std::invalid_argument("empty archive list");
  } 

  // determine start and end time from file list
  RadxTime startTime, endTime;
  NcfRadxFile::getTimeFromPath(list[0], startTime);
  NcfRadxFile::getTimeFromPath(list[list.size()-1], endTime);
  // round to nearest five minutes
  time_t startTimeSecs = startTime.utime();
  startTimeSecs =  (startTimeSecs / 300) * 300;
  time_t endTimeSecs = endTime.utime();
  endTimeSecs =  (endTimeSecs / 300) * 300 + 300;
  _archiveStartTime.set(startTimeSecs);
  _archiveEndTime.set(endTimeSecs);
  _archiveScanIndex = 0;
  _selectedTime.set(startTimeSecs);
  
  _archiveFileList = list;

  /*
  if (_archiveScanIndex < 0) {
    _archiveScanIndex = 0;
  } else if (_archiveScanIndex > (int) _archiveFileList.size() - 1) {
    _archiveScanIndex = _archiveFileList.size() - 1;
  }
  */
/*
  if (_timeSlider) {
    _timeSlider->setMinimum(0);
    if (_archiveFileList.size() <= 1)
      _timeSlider->setMaximum(1);
    else
      _timeSlider->setMaximum(_archiveFileList.size() - 1);
    _timeSlider->setSliderPosition(_archiveScanIndex);
  }
*/
  // check if the paths include a day dir

  _archiveFilesHaveDayDir = false;
  if (list.size() > 0) {
    RadxPath path0(list[0]);
    RadxPath parentPath(path0.getDirectory());
    string parentDir = parentPath.getFile();
    int year, month, day;
    if (sscanf(parentDir.c_str(), "%4d%2d%2d", &year, &month, &day) == 3) {
      _archiveFilesHaveDayDir = true;
    }
  }


}

///////////////////////////////////////////////
// get archive file list by searching for files
// starting point is the archiveDataUrl
// returns 0 on success, -1 on failure

int TimeNavModel::findArchiveFileList(string archiveDataUrl)
{

  bool _urlOK;

  // get the first day/time under this starting point 
  RadxTimeList timeList;
  // need to separate directory from the filename
  // then, send the directory to the timeList
  // need to find a pattern like this *<separator>YYYYMMDD
  // then send the string from start to end of pattern
  // to the timeList
  //
  RadxPath thePath(archiveDataUrl);
  if (thePath.isDir()) {
    timeList.setDir(archiveDataUrl);
  } else {
    string dir = thePath.getDirectory();
    timeList.setDir(dir);
  }

  timeList.setModeAll();
  timeList.compile();

  // TODO: how to report error? throw exception???
  if (timeList.getPathList().size() < 1) {
    string msg("No archive files found at  ");
    msg.append(archiveDataUrl);
    throw std::invalid_argument(msg);
    //cerr << "ERROR - TimeNavModel::loadArchiveFileList()" << endl;
    //cerr << "  Cannot load file list for url: " 
    //     << archiveDataUrl << endl;
    //cerr << "  Start time: " << _archiveStartTime.getStr() << endl;
    //cerr << "  End time: " << _archiveEndTime.getStr() << endl;
    //_urlOK = false;
    //return -1;
  }

  _archiveStartTime = timeList.getValidTimes().front();

/*
  // get the last day/time under this starting point 
  //RadxTimeList timeList;
  //timeList.setDir(archiveDataUrl);
  timeList.setModeLast();
  timeList.compile(); 

  // TODO: how to report error? throw exception???
  if (timeList.getPathList().size() < 1) {
    cerr << "ERROR - TimeNavModel::loadArchiveFileList()" << endl;
    cerr << "  Cannot load file list for url: " 
         << archiveDataUrl << endl;
    cerr << "  Start time: " << _archiveStartTime.getStr() << endl;
    cerr << "  End time: " << _archiveEndTime.getStr() << endl;
    _urlOK = false;
    return -1;
  } 

*/
  _archiveEndTime = timeList.getValidTimes().back();


/*
  // get the last day/time under this starting point 
  //RadxTimeList timeList;
  //timeList.setDir(archiveDataUrl);
  timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  timeList.compile();  
  _urlOK = true;

// TODO: how to report error? throw exception???
  if (timeList.getPathList().size() < 1) {
    cerr << "ERROR - TimeNavModel::loadArchiveFileList()" << endl;
    cerr << "  Cannot load file list for url: " 
         << archiveDataUrl << endl;
    cerr << "  Start time: " << _archiveStartTime.getStr() << endl;
    cerr << "  End time: " << _archiveEndTime.getStr() << endl;
    _urlOK = false;
    return -1;

  }
*/

  setArchiveFileList(timeList.getPathList());
  
  return 0;

}

/*
void TimeNavModel::DoSomething() { 

  // now update the time controller window
  QDateTime now = QDateTime::currentDateTime();  
  //QDateTime epoch(QDate(1970, 1, 1), QTime(0, 0, 0));
  _setArchiveStartTimeFromGui(now);

  _setArchiveEndTimeFromGui(now);
  
  _archiveStartTime = _guiStartTime;
  _archiveEndTime = _guiEndTime;
  QFileInfo fileInfo(filename);
  string absolutePath = fileInfo.absolutePath().toStdString();
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "changing to path " << absolutePath << endl;
  }
//  loadArchiveFileList(dir.absolutePath());

  RadxTimeList timeList;
  timeList.setDir(absolutePath);
  timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  if (timeList.compile()) {
    cerr << "ERROR - TimeNavModel::openFile()" << endl;
    cerr << "  " << timeList.getErrStr() << endl;
  }

  //vector<string> pathList = timeList.getPathList();
  //if (pathList.size() <= 0) {
  //  cerr << "ERROR - TimeNavModel::openFile()" << endl;
  //  cerr << "  pathList is empty" << endl;
  //  cerr << "  " << timeList.getErrStr() << endl;
  //} else {
  //  if (_params->debug >= Params::DEBUG_VERBOSE) {
  //    cerr << "pathList is NOT empty" << endl;
  //    for(vector<string>::const_iterator i = pathList.begin(); i != pathList.end(); ++i) {
  //     cerr << *i << endl;
  //    }
  //    cerr << endl;
  //  }
  
    setArchiveFileList(fileList, false); // pathList, false);

    // now fetch the first time and last time from the directory
    // and set these values in the time controller display

    RadxTime firstTime;
    RadxTime lastTime;
    timeList.getFirstAndLastTime(firstTime, lastTime);
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      cerr << "first time " << firstTime << endl;
      cerr << "last time " << lastTime << endl;
    }
    // convert RadxTime to QDateTime 
    _archiveStartTime = firstTime;
    _archiveEndTime = lastTime;
    _setGuiFromArchiveStartTime();
    _setGuiFromArchiveEndTime();
  //} // end else pathList is not empty

}
*/

void TimeNavModel::setArchiveStartTime(int year, int month, int day,
  int hour, int minute, int seconds) {

  _archiveStartTime.set(year, month, day, hour, minute, seconds);

}

void TimeNavModel::setArchiveEndTime(int year, int month, int day,
  int hour, int minute, int seconds) {

  _archiveEndTime.set(year, month, day, hour, minute, seconds);

}

void TimeNavModel::getArchiveStartTime(int *year, int *month, int *day,
  int *hour, int *minute, int *seconds) {

  _archiveStartTime.getAll(year, month, day, hour, minute, seconds);

}

void TimeNavModel::getArchiveEndTime(int *year, int *month, int *day,
  int *hour, int *minute, int *seconds) {

  _archiveEndTime.getAll(year, month, day, hour, minute, seconds);

}

void TimeNavModel::getSelectedTime(int *year, int *month, int *day,
  int *hour, int *minute, int *seconds) {

  _selectedTime.getAll(year, month, day, hour, minute, seconds);

}

/* void TimeNavModel::changeSelectedTime(int value) 
{
  if (value < 0 || value > (int) _archiveFileList.size() - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList.at(value);
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;

}
*/

void TimeNavModel::setSelectedFile(int value) 
{

  if (value < 0 || value > (int) _archiveFileList.size() - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList.at(value);
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;

  // request data
  //if (_archiveScanIndex != value) {
    _archiveScanIndex = value;
  //}

}

void TimeNavModel::setSelectedFile(string fileName) 
{

  vector<string>::iterator it;
  bool found = false;
  it = _archiveFileList.begin();
  while ( (it != _archiveFileList.end()) && !found ) {
    RadxPath thePath(*it);
    if (thePath.getFile().compare(fileName) == 0) {
      _archiveScanIndex = it - _archiveFileList.begin();
      found = true;
    }
    ++it;
  }  

  if (found) {
    // get time for this path
    RadxTime pathTime;
    NcfRadxFile::getTimeFromPath(fileName, pathTime);
    // set selected time
    _selectedTime = pathTime;
  }

}

string &TimeNavModel::getSelectedArchiveFile() { 
  if (_archiveFileList.size() <= 0) {
    string empty;
    return empty;
  } else {
    return _archiveFileList.at(_archiveScanIndex);
  }
}

