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

#ifndef TimeNavController_HH
#define TimeNavController_HH

#include "TimeNavView.hh"
#include "TimeNavModel.hh"


class TimeNavController {

public:

  TimeNavController(TimeNavView *view);
  ~TimeNavController();

  void setSliderPosition();
  void fetchArchiveFiles(string seedFileName);
  void timeSliderValueChanged(int value);
  void timeSliderReleased(int value);
  void fetchArchiveFiles(string seedPath, string seedFileName);
  string &getSelectedArchiveFile();

private:

  void _setArchiveStartTimeFromGui(int year, int month, int day,
    int hour, int minute, int seconds);
  void _setArchiveEndTimeFromGui(int year, int month, int day,
  int hour, int minute, int seconds);

  void _setGuiFromArchiveStartTime();
  void _setGuiFromArchiveEndTime();
  void _setGuiFromSelectedTime();



  TimeNavView *_view;
  TimeNavModel *_model;

};

#endif