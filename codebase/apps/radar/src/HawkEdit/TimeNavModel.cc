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


TimeNavModel::TimeNavModel() {
  changePath(".");
  _atEnd = true;
}

TimeNavModel::~TimeNavModel() {
  if (currentPath != NULL) 
    delete currentPath;
}
//
// TODO: set the selected time _selectedTime
//

bool TimeNavModel::moreFiles() {
  return !_atEnd;
}

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

void TimeNavModel::changePath(string archiveDataUrl) {
  currentPath = new RadxPath(archiveDataUrl);
}

///////////////////////////////////////////////
// get archive file list by searching for files
// starting point is the archiveDataUrl
// returns 0 on success, -1 on failure

int TimeNavModel::findArchiveFileList(string archiveDataUrl,
  bool keepTimeRange)
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

  // TODO: save current archiveUrl for use if start and end times change
  // must open new file to change the path
  changePath(archiveDataUrl);
  // RadxPath thePath(archiveDataUrl);
  if (currentPath->isDir()) {
    timeList.setDir(archiveDataUrl);
  } else {
    string dir = currentPath->getDirectory();
    timeList.setDir(dir);
  }

  if (keepTimeRange) {
    timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  } else {
    timeList.setModeAll();
  }
  timeList.compile();

  int nFilesFound = (int) timeList.getPathList().size();
  if (nFilesFound > 0) {
    _archiveStartTime = timeList.getValidTimes().front();
    _archiveEndTime = timeList.getValidTimes().back();
    setArchiveFileList(timeList.getPathList());
  } 
  return nFilesFound;

}

void TimeNavModel::recoverFromNoDatDirFormat(string archiveDataUrl) {

    // recover ... 
    // get the start and end times from the seedFileName
    setArchiveStartTimeDefault();
    setArchiveEndTimeDefault();  
    // put the fullUrl into a list
    vector<string> list;
    list.push_back(archiveDataUrl);  
    setArchiveFileList(list);
    RadxPath pathSent(archiveDataUrl);
    if (!pathSent.isDir()) {
      string dir = pathSent.getDirectory();
      changePath(dir);
    } else {
      changePath(archiveDataUrl);
    }
}

void TimeNavModel::findAndSetArchiveFileList(RadxTime startTime, RadxTime endTime,
  const string &absolutePath) {
  // string startTime, string endTime) { 
  
  //_archiveStartTime = _guiStartTime;
  //_archiveEndTime = _guiEndTime;
  //QFileInfo fileInfo(filename);
  //string absolutePath = fileInfo.absolutePath().toStdString();
  //if (_params->debug >= Params::DEBUG_VERBOSE) {
  //  cerr << "changing to path " << absolutePath << endl;
  //}
//  loadArchiveFileList(dir.absolutePath());

  // TODO: NO REPEATED CODE: use findArchiveFileList ... 
  RadxTimeList timeList;
  RadxPath thePath(absolutePath);
  if (!thePath.isDir()) {
    // if not fixable, then throw std::invalid_arg...
    throw std::invalid_argument("TimeNavModel::findArchiveFileList path is NOT a directory");
  } 
  timeList.setDir(thePath.getPath());
  timeList.setModeInterval(startTime, endTime);
  if (timeList.compile()) {
    string msg = "ERROR - TimeNavModel::findArchiveFileList() ";
    msg.append(timeList.getErrStr());
    throw std::invalid_argument(msg);
  }

  vector<string> pathList = timeList.getPathList();
  if (pathList.size() <= 0) {
    string msg = "ERROR - TimeNavModel::findArchiveFileList()\n";
    msg.append("  no files found\n");
    throw std::invalid_argument(msg);
  } 
  // else {
  //  if (_params->debug >= Params::DEBUG_VERBOSE) {
  //    cerr << "pathList is NOT empty" << endl;
  //    for(vector<string>::const_iterator i = pathList.begin(); i != pathList.end(); ++i) {
  //     cerr << *i << endl;
  //    }
  //    cerr << endl;
  //  }

  setArchiveFileList(timeList.getPathList());

    // now fetch the first time and last time from the directory
    // and set these values in the time controller display

    RadxTime firstTime;
    RadxTime lastTime;
    timeList.getFirstAndLastTime(firstTime, lastTime);
    //if (_params->debug >= Params::DEBUG_VERBOSE) {
    //  cerr << "first time " << firstTime << endl;
    //  cerr << "last time " << lastTime << endl;
    //}
    // convert RadxTime to QDateTime 
    _archiveStartTime = firstTime;
    _archiveEndTime = lastTime;
    setSelectedFile(0);

  //} // end else pathList is not empty

}

void TimeNavModel::setArchiveStartTimeDefault() {
  _archiveStartTime = RadxTime(RadxTime::ZERO);
}

void TimeNavModel::setArchiveEndTimeDefault() {
  _archiveEndTime = RadxTime(RadxTime::NOW);
}

void TimeNavModel::setArchiveStartEndTime(int startYear, int startMonth, int startDay,
                       int startHour, int startMinute, int startSeconds,
                       int endYear, int endMonth, int endDay,
                       int endHour, int endMinute, int endSeconds) {

  RadxTime requestStartTime;
  requestStartTime.set(startYear, startMonth, startDay,
    startHour, startMinute, startSeconds);  
  RadxTime requestEndTime;
  requestEndTime.set(endYear, endMonth, endDay,
    endHour, endMinute, endSeconds);
  const string mypath = currentPath->getPath();
  cerr << "looking in this directory: " << mypath << endl;
  //std::remove_const<const string>::type mypath;
  findAndSetArchiveFileList(requestStartTime, requestEndTime, mypath);
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

string TimeNavModel::getArchiveFilePath(int idx) {
  if ((idx < 0) || (idx > _archiveFileList.size())) {
    stringstream ss;
    ss << "Error, invalid index " << idx << " TimeNavModel::getArchiveFilePath";
    throw std::invalid_argument(ss.str());
  }
  return _archiveFileList.at(idx);
}  


string TimeNavModel::getArchiveFileName(int idx) { 
  if (_archiveFileList.size() <= 0) {
    string empty;
    return empty;
  } else {
    string p = _archiveFileList.at(idx);
    size_t idx = p.find_last_of("/\\");
    
    if (idx != string::npos) {
      string result = p.substr(idx+1);
      return result;
    } 
    return p;
  }
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
    _atEnd = true;
  } else {
    _atEnd = false;
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
}

// I think this assumes all files are in the same folder??
// TODO: check if the folder is different for this file,
// set the currentPath as needed.
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

string TimeNavModel::getSelectedArchiveFileName() { 
  if (_archiveFileList.size() <= 0) {
    string empty;
    return empty;
  } else {
    string p = _archiveFileList.at(_archiveScanIndex);
    size_t idx = p.find_last_of("/\\");
    
    if (idx != string::npos) {
      string result = p.substr(idx+1);
      return result;
    } 
    return p;
  }
}

// 
string TimeNavModel::no_yyyymmdd(string s) {
  if ((s.size() > 8) && _archiveFilesHaveDayDir) {
    return s.substr(0, s.size()-8);
  }
}

string TimeNavModel::getCurrentPath() {
  return currentPath->getPath();
}

/*
// return a string composed of the current base directory
// plus internally named temporary directory .tmp_N
// where N is the next available sequence number
string TimeNavModel::getTempDir() {

  std::stringstream ss;

  string tmp = ".tmp_";
  //tmp.append(_archiveStartTime.getDateStrPlain());
  //string dir = currentPath->getDirectory();
  // get list of current .tmp_N directories
  string path = currentPath->getPath(); // computeTmpPath(yyyymmdd.c_str());
  // if we are in a temp dir, then move up one dir
  size_t pos = path.find(tmp);
  if (pos != string::npos) {
    path = path.substr(0, pos-1);
  }

  //path.append("/");
  //path.append(tmp);
  string yyyymmdd = _archiveStartTime.getDateStrPlain();

  int max = 10;
  int idx = 0;
  do {
    idx += 1;
    ss.str(""); // reset the starting path
    ss << path << "/" << tmp << idx;
    if (_archiveFilesHaveDayDir) {
      ss << "/" << yyyymmdd;
    }
  } while (RadxPath::exists(ss.str()) && (idx < max));

  if (idx >= max) {
    string msg = "max temporary directories ";
    msg.append(std::to_string(max));
    throw std::invalid_argument(msg);    
  }

  string workingPath = ss.str();

  RadxPath pathToCreate;
  pathToCreate.setDirectory(workingPath);

  if (pathToCreate.makeDirRecurse()) {
    string msg = "cannot create temporary directory ";
    msg.append(workingPath);
    throw std::invalid_argument(msg);
  }

  return workingPath;
}
*/

bool TimeNavModel::archiveFilesHaveDayDir() {
  return _archiveFilesHaveDayDir;
}

int TimeNavModel::getPositionOfSelection() {
  return _archiveScanIndex;
}

//--------

//RadxTime &TimeNavModel::convertDateTimePieces(string dateTime) {
  // ???
//}
// fetch the list of archive files only; DOES NOT set any GUI info
vector<string> &TimeNavModel::getArchiveFileListOnly(string path,
  int startYear, int startMonth, int startDay,
  int startHour, int startMinute, int startSecond,
  int endYear, int endMonth, int endDay,
  int endHour, int endMinute, int endSecond) {
  //string startTime, string endTime) {


 //setArchiveStartEndTime(int startYear, int startMonth, int startDay,
 //                      int startHour, int startMinute, int startSeconds,
 //                      int endYear, int endMonth, int endDay,
 //                      int endHour, int endMinute, int endSeconds) {

  RadxTime requestStartTime;
  requestStartTime.set(startYear, startMonth, startDay,
    startHour, startMinute, startSecond);  
  RadxTime requestEndTime;
  requestEndTime.set(endYear, endMonth, endDay,
    endHour, endMinute, endSecond);

  if (path.length() <= 0) {
    path = currentPath->getPath();
  }
  cerr << "looking in this directory: " << path << endl;
  //std::remove_const<const string>::type mypath;
  vector<string> fileList = findArchiveFileList(requestStartTime, requestEndTime, path);

  return fileList;
}

const vector<string> &TimeNavModel::findArchiveFileList(RadxTime startTime, RadxTime endTime,
  const string &absolutePath) {


  RadxTimeList timeList;
  RadxPath thePath(absolutePath);
  if (!thePath.isDir()) {
    // if not fixable, then throw std::invalid_arg...
    throw std::invalid_argument("TimeNavModel::findArchiveFileList path is NOT a directory");
  } 
  timeList.setDir(thePath.getPath());
  timeList.setModeInterval(startTime, endTime);
  if (timeList.compile()) {
    string msg = "ERROR - TimeNavModel::findArchiveFileList() ";
    msg.append(timeList.getErrStr());
    throw std::invalid_argument(msg);
  }

  vector<string> pathList = timeList.getPathList();
  if (pathList.size() <= 0) {
    string msg = "ERROR - TimeNavModel::findArchiveFileList()\n";
    msg.append("  no files found\n");
    throw std::invalid_argument(msg);
  } 


  return timeList.getPathList();

}
