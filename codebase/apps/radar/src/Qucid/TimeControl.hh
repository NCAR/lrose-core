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
// TimeControl.hh
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2024
//
///////////////////////////////////////////////////////////////
//
// Time controller dialog
//
///////////////////////////////////////////////////////////////

#ifndef TimeControl_HH
#define TimeControl_HH

#ifndef DLL_EXPORT
#ifdef WIN32
#ifdef QT_PLUGIN
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
#else
#define DLL_EXPORT
#endif
#endif

#include <string>
#include <vector>

#include <QWidget>
#include <QDialog>
#include <QSlider>
#include <QDateTimeEdit>
#include <QPushButton>

class QFrame;
class QHBoxLayout;
class QVBoxLayout;
class CartManager;

#include <Radx/RadxTime.hh>
#include "Params.hh"

using namespace std;

class DLL_EXPORT TimeControl : public QDialog {
  
  Q_OBJECT

 public:
  
  // constructor
  
  TimeControl(CartManager *parent,
              const Params &params);
  
  // destructor
  
  virtual ~TimeControl();

  // set

  void setArchiveStartTime(string timeStr) {
    _archiveStartTime.set(timeStr);
    setGuiFromArchiveStartTime();
  }
  void setArchiveEndTime(string timeStr) {
    _archiveEndTime.set(timeStr);
    setGuiFromArchiveEndTime();
  }
  
  void setArchiveStartTime(time_t val) { _archiveStartTime.set(val); }
  void setArchiveEndTime(time_t val) { _archiveEndTime.set(val); }

  void setArchiveStartTime(const RadxTime &rtime);
  void setArchiveEndTime(const RadxTime &rtime);

  void setTimeSliderMinimum(int val) { _timeSlider->setMinimum(val); }
  void setTimeSliderMaximum(int val) { _timeSlider->setMaximum(val); }
  void setTimeSliderPosition(int val) { _timeSlider->setSliderPosition(val); }

  void setArchiveEnabled(bool val) {
    _archiveStartTimeEdit->setEnabled(val);
    _archiveEndTimeEdit->setEnabled(val);
    _backPeriod->setEnabled(val);
    _fwdPeriod->setEnabled(val);
  }

  void setGuiFromArchiveStartTime();
  void setGuiFromArchiveEndTime();
  void setGuiFromSelectedTime();

  void setArchiveScanIndex(int val) { _archiveScanIndex = val; }

  void setSelectedTimeLabel(const string &text) {
    _selectedTimeLabel->setText(text.c_str());
  }
  
  void setArchiveStartTimeFromGui() {
    setArchiveStartTime(_guiStartTime);
  }
  void setArchiveEndTimeFromGui() {
    setArchiveEndTime(_guiEndTime);
  }

  void setArchiveStartTime(const QDateTime &qdt);
  void setArchiveEndTime(const QDateTime &qdt);
  void acceptGuiTimes();
  void cancelGuiTimes();
  
  // get

  const RadxTime &getArchiveStartTime() const { return _archiveStartTime; }
  const RadxTime &getArchiveEndTime() const { return _archiveEndTime; }
  int getArchiveScanIndex() const { return _archiveScanIndex; }
  QSlider *getTimeSlider() { return _timeSlider; }
  
 protected:

  CartManager *_parent;
  const Params &_params;
  
  QFrame *_timePanel;
  QVBoxLayout *_timeLayout;
  QSlider *_timeSlider;

  QPushButton *_back1;
  QPushButton *_fwd1;
  QPushButton *_backPeriod;
  QPushButton *_fwdPeriod;
  
  QDateTimeEdit *_archiveStartTimeEdit;
  QDateTimeEdit *_archiveEndTimeEdit;

  RadxTime _guiStartTime;
  RadxTime _archiveStartTime;
  
  RadxTime _guiEndTime;
  RadxTime _archiveEndTime;

  QPushButton *_selectedTimeLabel;
  RadxTime _selectedTime;
  int _archiveScanIndex;

  // populate the time controller

  void _populate();
  
 public slots:

  // void toggled(bool checked); // connected to menu button

  // move in time
  
  void goBack1();
  void goBackPeriod();
  void goFwd1();
  void goFwdPeriod();

  // time slider
  
  void _timeSliderActionTriggered(int action);
  void _timeSliderValueChanged(int value);
  void _timeSliderReleased();
  void _timeSliderPressed();

};

#endif

