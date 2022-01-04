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
// The time slider uses this class to navigate through the 
// archive files.
// The script editor also uses this class to get a list
// of files on which to run a script.  The script editor
// sends a start and end time to specify the bounds of the
// archive data.
// 
///////////////////////////////////////////////////////////////

#ifndef TimeNavController_HH
#define TimeNavController_HH

#include "TimeNavView.hh"
#include "TimeNavModel.hh"


class TimeNavController {

public:

  TimeNavController(TimeNavView *view);

  // use this constructor for utility functions, without GUI.
  //TimeNavController();

  ~TimeNavController();

  void setSliderPosition();

  void timeSliderValueChanged(int value);
  void timeSliderReleased(int value);
  void setTimeSliderPosition(int value);

  void fetchArchiveFiles(string seedFileName);
  void fetchArchiveFiles(string seedPath, string seedFileName);

  string &getSelectedArchiveFile();
  string getSelectedArchiveFileName();

  string &getPath();

  // use this for the script editor ???
  // Hmmm, the script editor uses the singleton DataModel to 
  // get data.  The DataModel is the currently selected archive file.
  // Q: how to apply the script to a list of archive files?
  // 1. change the DataModel to each file in the list?
  // 2. maintain a separate DataModel? for working through the archive list???
  // NOTE: the edited files will go to a temp directory.  How to manage this??
  // Only one base directory is open/active at a time.
  string &getArchiveFile(size_t index) {}; 
  // or
  //vector<string> &getArchiveFileList(string path,
  //  string startTime, string endTime);

  vector<string> &getArchiveFileList(string path,
  int startYear, int startMonth, int startDay,
  int startHour, int startMinute, int startSecond,
  int endYear, int endMonth, int endDay,
  int endHour, int endMinute, int endSecond);



  void _setArchiveStartEndTimeFromGui(int startYear, int startMonth, int startDay,
                       int startHour, int startMinute, int startSecond,
                       int endYear, int endMonth, int endDay,
                       int endHour, int endMinute, int endSecond);
  void updateGui();

  bool moreFiles();

private:
  void _setGuiFromArchiveStartTime();
  void _setGuiFromArchiveEndTime();
  void _setGuiFromSelectedTime();



  TimeNavView *_view;
  TimeNavModel *_model;

};

#endif