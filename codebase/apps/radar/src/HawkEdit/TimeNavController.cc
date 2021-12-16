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
// TimeNavController.hh
//
// Coordinates navigation through archive file names indexed by date and time
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////
//
// coordinates movement through the archive file names
// 
///////////////////////////////////////////////////////////////

#include "TimeNavController.hh"

TimeNavController::TimeNavController(TimeNavView *view) {

	_view = view;
	_model = new TimeNavModel();

}

TimeNavController::~TimeNavController() {
  if (_view != NULL) delete _view;
  if (_model != NULL) delete _model;
}

void TimeNavController::fetchArchiveFiles(string seedPath, string seedFileName) {
  _model->findArchiveFileList(seedPath);
  _setGuiFromArchiveStartTime();  
  _setGuiFromArchiveEndTime();
  _model->setSelectedFile(seedFileName);
  _setGuiFromSelectedTime();

  _view->setNTicks(_model->getNArchiveFiles());

  _view->showTimeControl();
}


string &TimeNavController::getSelectedArchiveFile() {
  return _model->getSelectedArchiveFile();
}

////////////////////////////////////////////////////////
// set times from gui widgets

void TimeNavController::_setArchiveStartTimeFromGui(int year, int month, int day,
  int hour, int minute, int seconds)
{

  _model->setArchiveStartTime(year, month, day,
    hour, minute, seconds);
}

void TimeNavController::_setArchiveEndTimeFromGui(int year, int month, int day,
  int hour, int minute, int seconds)
{
  //QDate date = qdt.date();
  //QTime time = qdt.time();
  //_guiEndTime.set(date.year(), date.month(), date.day(),
  //                time.hour(), time.minute(), time.second());
  _model->setArchiveEndTime(year, month, day,
    hour, minute, seconds);  
}

/*
void TimeNavController::_acceptGuiTimes()
{
  _archiveStartTime = _guiStartTime;
  _archiveEndTime = _guiEndTime;
  loadArchiveFileList();
}

void TimeNavController::_cancelGuiTimes()
{
  _setGuiFromArchiveStartTime();
  _setGuiFromArchiveEndTime();
}
*/
////////////////////////////////////////////////////////
// set gui widget from archive start time

void TimeNavController::_setGuiFromArchiveStartTime()
{
  int year;
  int month; 
  int day;
  int hour; 
  int minute; 
  int seconds;
  _model->getArchiveStartTime(&year, &month, &day, &hour, &minute, &seconds);
  _view->setGuiFromArchiveStartTime(year, month, day, hour,
    minute, seconds);
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void TimeNavController::_setGuiFromArchiveEndTime()
{
  int year;
  int month; 
  int day;
  int hour; 
  int minute; 
  int seconds;
  _model->getArchiveEndTime(&year, &month, &day, &hour, &minute, &seconds);
  _view->setGuiFromArchiveEndTime(year, month, day, hour,
    minute, seconds);
}

////////////////////////////////////////////////////////
// set gui selected time label

void TimeNavController::_setGuiFromSelectedTime()
{
  int year;
  int month; 
  int day;
  int hour; 
  int minute; 
  int seconds;
  _model->getSelectedTime(&year, &month, &day, &hour, &minute, &seconds);
  /*
  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           _selectedTime.getYear(),
           _selectedTime.getMonth(),
           _selectedTime.getDay(),
           _selectedTime.getHour(),
           _selectedTime.getMin(),
           _selectedTime.getSec());
           */
  _view->setGuiFromSelectedTime(year, month, day, hour,
    minute, seconds);
  //_selectedTimeLabel->setText(text);
}

/*
////////////////////////////////////////////////////////
// set archive start time

void TimeNavController::_setArchiveStartTime(const RadxTime &rtime)

{
  _archiveStartTime = rtime;
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromArchiveStartTime();
}

////////////////////////////////////////////////////////
// set archive end time

void TimeNavController::_setArchiveEndTime(const RadxTime &rtime)

{
  _archiveEndTime = rtime;
  if (!_archiveEndTime.isValid()) {
    _archiveEndTime.set(RadxTime::NOW);
  }
  _setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// change start time

void TimeNavController::_goBack1()
{
  if (_archiveScanIndex > 0) {
    _archiveScanIndex -= 1;
    _setArchiveRetrievalPending();
  } else {

      LOG(DEBUG) << "At start of data, cannot go back";
    
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void TimeNavController::_goBackPeriod()
{

  int archiveSpanSecs = _archiveEndTime - _archiveStartTime;
  _archiveStartTime -= archiveSpanSecs;
  _archiveEndTime -= archiveSpanSecs;
  loadArchiveFileList();
  if (_archiveScanIndex > (int) _archiveFileList.size() - 1) {
    _archiveScanIndex = (int) _archiveFileList.size() - 1;
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);

}

void TimeNavController::_goFwd1()
{
  if (_archiveScanIndex < (int) _archiveFileList.size() - 1) {
    _archiveScanIndex += 1;
    _setArchiveRetrievalPending();
  } else {

      LOG(DEBUG) << "At end of data, cannot go forward";
    
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void TimeNavController::_goFwdPeriod()
{

  int archiveSpanSecs = _archiveEndTime - _archiveStartTime;
  _archiveStartTime += archiveSpanSecs;
  _archiveEndTime += archiveSpanSecs;
  loadArchiveFileList();
  if (_archiveScanIndex > (int) _archiveFileList.size() - 1) {
    _archiveScanIndex = (int) _archiveFileList.size() - 1;
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);

}


*/

void TimeNavController::timeSliderValueChanged(int value) 
{

  _model->setSelectedFile(value);
  /*
  size_t nFiles = _model->getNArchiveFiles();
  if (value < 0 || value > (int) nFiles - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  _setGuiFromSelectedTime();
  */
}

void TimeNavController::timeSliderReleased(int value) 
{

  _model->setSelectedFile(value);

  /*
  if (value < 0 || value > (int) _archiveFileList.size() - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  _setGuiFromSelectedTime();
  // request data
  if (_archiveScanIndex != value) {
    _archiveScanIndex = value;
  }
  */

}


