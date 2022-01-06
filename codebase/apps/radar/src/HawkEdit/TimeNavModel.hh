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
// TimeNavModel.hh
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

#ifndef TimeNavModel_HH
#define TimeNavModel_HH

#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>

class TimeNavModel {

public:

  TimeNavModel();
  ~TimeNavModel();

  void setArchiveFileList(const vector<string> &list);

  int findArchiveFileList(string archiveDataUrl);

  const vector<string> &findArchiveFileList(RadxTime startTime, RadxTime endTime,
  const string &absolutePath);
  void findAndSetArchiveFileList(RadxTime startTime, RadxTime endTime,
    const string &absolutePath);
  vector<string> &getArchiveFileListOnly(string path,
  int startYear, int startMonth, int startDay,
  int startHour, int startMinute, int startSecond,
  int endYear, int endMonth, int endDay,
  int endHour, int endMinute, int endSecond);

  int getNArchiveFiles() { return  (int) _archiveFileList.size(); };

  //void setArchiveStartTime(int year, int month, int day,
  //  int hour, int minute, int seconds);
  //void setArchiveEndTime(int year, int month, int day,
  //  int hour, int minute, int seconds);

  void setArchiveStartEndTime(int startYear, int startMonth, int startDay,
                       int startHour, int startMinute, int startSecond,
                       int endYear, int endMonth, int endDay,
                       int endHour, int endMinute, int endSecond);

  void getArchiveStartTime(int *year, int *month, int *day,
    int *hour, int *minute, int *seconds);
  void getArchiveEndTime(int *year, int *month, int *day,
    int *hour, int *minute, int *seconds);
  void getSelectedTime(int *year, int *month, int *day,
    int *hour, int *minute, int *seconds);

  //void changeSelectedTime(int value);
  void setSelectedFile(int value);
  void setSelectedFile(string fileName);

  string &getSelectedArchiveFile();
  string getSelectedArchiveFileName();
  string getTempDir();
  int getPositionOfSelection();

  bool moreFiles();

private:

  void changePath(string archiveDataUrl);

  RadxTimeList timeList;

  //RadxTime _archiveIntermediateTime;

  //RadxTime _startDisplayTime;
  //RadxTime _currentDisplayTime;  // is this needed??
  //RadxTime _endDisplayTime;
  //RadxTime _imagesArchiveStartTime;
  //RadxTime _imagesArchiveEndTime;

  RadxTime _archiveStartTime;  
  RadxTime _archiveEndTime;

  RadxTime _selectedTime;

  int _archiveScanIndex;

  vector<string> _archiveFileList;

  bool _archiveFilesHaveDayDir;

  RadxPath *currentPath;

  bool _atEnd;

};

#endif