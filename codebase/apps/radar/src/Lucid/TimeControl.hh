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
#include <QLabel>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>

class QFrame;
class QHBoxLayout;
class QVBoxLayout;
class GuiManager;

#include <toolsa/DateTime.hh>
#include "Params.hh"

using namespace std;

class DLL_EXPORT TimeControl : public QDialog {
  
  Q_OBJECT

 public:
  
  // constructor
  
  TimeControl(GuiManager *parent,
              const Params &params);
  
  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */
  
  static TimeControl *getInstance();
  
  // destructor
  
  virtual ~TimeControl();

  // set

  void setStartTimeFromEdit(const QDateTime &val);
  void setStartTime(const DateTime &rtime);
  void setEndTime(const DateTime &rtime);

  void setTimeSliderMinimum(int val) { _timeSlider->setMinimum(val); }
  void setTimeSliderMaximum(int val) { _timeSlider->setMaximum(val); }
  void setTimeSliderPosition(int val) { _timeSlider->setSliderPosition(val); }
  
  void setEnabled(bool val) {
    _startTimeEdit->setEnabled(val);
    _back1->setEnabled(val);
    _fwd1->setEnabled(val);
    _backDuration->setEnabled(val);
    _fwdDuration->setEnabled(val);
    _backMult->setEnabled(val);
    _fwdMult->setEnabled(val);
  }

  void setGuiStartTime(const DateTime &val);
  void setGuiEndTime(const DateTime &val);
  void setGuiSelectedTime(const DateTime &val);

  void setFrameIndex(int val) { _frameIndex = val; }

  void setSelectedTimeLabel(const string &text) {
    _selectedTimeLabel->setText(text.c_str());
  }
  
  // get
  
  const DateTime &getSelectedTime() const { return _selectedTime; }
  const DateTime &getStartTime() const { return _startTime; }
  const DateTime &getEndTime() const { return _endTime; }
  int getNFramesMovie() const { return _nFramesMovie; }
  double getMovieDurationSecs() const { return _movieDurationSecs; }
  int getFrameIndex() const { return _frameIndex; }
  double getLoopDwellMsecs() const { return _loopDwellMsecs; }
  double getFrameIntervalSecs() const { return _frameIntervalSecs; }

  // convert between Qt and Radx date/time objects

  static QDateTime getQDateTime(const DateTime &rtime);
  static DateTime getDateTime(const QDateTime &qtime);
  
  // populate the gui

  void populateGui();
  
 private:

  // Singleton instance pointer.

  static TimeControl *_instance;
  
  GuiManager *_manager;
  const Params &_params;
  
  QFrame *_timePanel;
  QVBoxLayout *_timeLayout;
  
  QDateTimeEdit *_startTimeEdit;
  QLabel *_endTimeLabel;
  QLabel *_selectedTimeLabel;
  
  QSlider *_timeSlider;
  bool _timeSliderInProgress;
  
  QPushButton *_back1;
  QPushButton *_fwd1;
  QPushButton *_backDuration;
  QPushButton *_fwdDuration;
  QPushButton *_backMult;
  QPushButton *_fwdMult;
  
  QSpinBox *_nFramesSelector;
  QDoubleSpinBox *_frameIntervalSelector;
  
  QCheckBox *_realtimeSelector;
  QCheckBox *_sweepSelector;
  
  QSpinBox *_loopDwellSelector;
  QSpinBox *_loopDelaySelector;
  
  // gui selections before accept
  
  DateTime _guiStartTime;
  DateTime _guiEndTime;
  DateTime _guiSelectedTime;
  int _guiNFramesMovie;
  double _guiFrameIntervalSecs;
  int _guiFrameIndex;

  // selections after accept
  
  DateTime _startTime;
  DateTime _endTime;
  DateTime _selectedTime;
  int _nFramesMovie;
  double _frameIntervalSecs;
  int _frameIndex;

  // other selections
  
  double _movieDurationSecs;
  int _loopDwellMsecs;
  int _loopDelayMsecs;
  bool _isRealtime;
  bool _isSweep;

  void _acceptSelectedTime();
  void _changeMovieLimits();

 public slots:

  // actions
  
  void _acceptGuiSelections();
  void _cancelGuiSelections();
  
  void _startMovie();
  void _stopMovie();
  void _outputMovieLoop();
  
  // move in time
  
  void _setSelectedTime(const DateTime &val);

  void _goBack1();
  void _goBackDuration();
  void _goBackMult();

  void _goFwd1();
  void _goFwdDuration();
  void _goFwdMult();

  // time slider
  
  void _timeSliderActionTriggered(int action);
  void _timeSliderValueChanged(int value);
  void _timeSliderReleased();
  void _timeSliderPressed();
  void _timeSliderSetNFrames(int val);
  void _setFrameIntervalSecs(double val);

#if QT_VERSION >= 0x067000
  void _setRealtime(Qt::CheckState val);
  void _setSweep(Qt::CheckState val);
#else
  void _setRealtime(int val);
  void _setSweep(int val);
#endif

  void _setLoopDwellMsecs(int val);
  void _setLoopDelayMsecs(int val);

};

#endif

