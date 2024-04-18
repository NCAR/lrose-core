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

  void setStartTime(const RadxTime &rtime);
  void setEndTime(const RadxTime &rtime);

  void setTimeSliderMinimum(int val) { _timeSlider->setMinimum(val); }
  void setTimeSliderMaximum(int val) { _timeSlider->setMaximum(val); }
  void setTimeSliderPosition(int val) { _timeSlider->setSliderPosition(val); }

  void setEnabled(bool val) {
    _startTimeEdit->setEnabled(val);
    _endTimeEdit->setEnabled(val);
    _backPeriod->setEnabled(val);
    _fwdPeriod->setEnabled(val);
  }

  void setGuiStartTime(const RadxTime &val);
  void setGuiEndTime(const RadxTime &val);
  void setGuiSelectedTime(const RadxTime &val);
  void setGuiFromTimes();

  void acceptGuiTimes();
  void cancelGuiTimes();
  
  void setFrameIndex(int val) { _frameIndex = val; }

  void setSelectedTimeLabel(const string &text) {
    _selectedTimeLabel->setText(text.c_str());
  }
  
  void setStartTimeFromEdit(const QDateTime &val);
  void setEndTimeFromEdit(const QDateTime &val);

  // get
  
  const RadxTime &getStartTime() const { return _startTime; }
  const RadxTime &getEndTime() const { return _endTime; }
  int getNFramesMovie() const { return _nFramesMovie; }
  double getMovieDurationSecs() const { return _movieDurationSecs; }
  int getFrameIndex() const { return _frameIndex; }
  double getFrameDwellMsecs() const { return _frameDwellMsecs; }

  // convert between Qt and Radx date/time objects

  static QDateTime getQDateTime(const RadxTime &rtime);
  static RadxTime getRadxTime(const QDateTime &qtime);
  
  // populate the gui

  void populateGui();
  
 protected:

  CartManager *_parent;
  const Params &_params;
  
  QFrame *_timePanel;
  QVBoxLayout *_timeLayout;
  
  QDateTimeEdit *_startTimeEdit;
  QDateTimeEdit *_endTimeEdit;
  QPushButton *_selectedTimeLabel;

  QSlider *_timeSlider;
  QPushButton *_back1;
  QPushButton *_fwd1;
  QPushButton *_backPeriod;
  QPushButton *_fwdPeriod;
  

  // gui times before 'accept'
  
  RadxTime _guiStartTime;
  RadxTime _guiEndTime;
  RadxTime _guiSelectedTime;

  // times after 'accept'
  
  RadxTime _startTime;
  RadxTime _endTime;
  RadxTime _selectedTime;

  int _nFramesMovie;
  int _frameIndex;
  double _frameDurationSecs;
  double _movieDurationSecs;
  double _frameDwellMsecs;
  double _loopDelayMsecs;
                         
 public slots:

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

