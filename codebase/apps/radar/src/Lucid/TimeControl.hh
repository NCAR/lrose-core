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

  // get info

  bool timeHasChanged();
  DateTime getSelectedTime() const;
  const DateTime &getStartTime() const { return _startTime; }
  DateTime getEndTime() const;
  int getNFramesMovie() const { return _nFramesMovie; }
  double getFrameIntervalSecs() const { return _frameIntervalSecs; }
  double getMovieDurationSecs() const {
    return (_nFramesMovie - 1) * _frameIntervalSecs;
  }
  int getFrameIndex() const { return _frameIndex; }
  double getLoopDwellMsecs() const { return _loopDwellMsecs; }

  // convert between Qt and Radx date/time objects
  
  static QDateTime getQDateTime(const DateTime &rtime);
  static DateTime getDateTime(const QDateTime &qtime);

  // set enabled on the GUI
  // controls greyed-out behavior
  
  void setEnabled(bool val); // for archive image generation

  // go back and fwd by 1 time step
  
  void goBack1();
  void goFwd1();

 private:

  // Singleton instance pointer.

  static TimeControl *_instance;
  
  GuiManager *_manager;
  const Params &_params;

  // GUI elements
  
  QFrame *_timePanel;
  QVBoxLayout *_timeLayout;

  QPushButton *_startButton;
  QPushButton *_stopButton;
  QPushButton *_outputButton;
  
  QDateTimeEdit *_startTimeEdit;
  QLabel *_endTimeLabel;
  QLabel *_selectedTimeLabel;
  
  QSlider *_timeSlider;
  bool _timeSliderInProgress;
  
  QPushButton *_goBack1Button;
  QPushButton *_goFwd1Button;
  QPushButton *_shiftBack1Button;
  QPushButton *_shiftFwd1Button;
  QPushButton *_shiftBack3Button;
  QPushButton *_shiftFwd3Button;
  
  QSpinBox *_nFramesSelector;
  QDoubleSpinBox *_frameIntervalSelector;
  
  QCheckBox *_realtimeSelector;
  QCheckBox *_sweepSelector;
  
  QSpinBox *_loopDwellSelector;
  QSpinBox *_loopDelaySelector;
  
  // gui state - the 'view'
  
  DateTime _guiStartTime;
  int _guiNFramesMovie;
  double _guiFrameIntervalSecs;
  int _guiFrameIndex;

  // accepted state - i.e. the 'model'

  bool _timeHasChanged;
  DateTime _startTime;
  int _nFramesMovie;
  double _frameIntervalSecs;
  int _frameIndex;
  vector<DateTime> _frameTimes;

  // parameters
  
  // double _movieDurationSecs;
  int _loopDwellMsecs;
  int _loopDelayMsecs;
  bool _isRealtime;
  bool _isSweep;
  
  // private methods
  
  void _populateGui();
  
  void _setStartTimeFromEdit(const QDateTime &val);
  void _setStartTime(const DateTime &rtime);
  
  void _setGuiNFramesMovie(int val);
  void _setGuiIntervalSecs(double val);
  void _setTimeSliderMinimum(int val) { _timeSlider->setMinimum(val); }
  void _setTimeSliderMaximum(int val) { _timeSlider->setMaximum(val); }
  void _setTimeSliderPosition(int val) { _timeSlider->setSliderPosition(val); }
  
  void _setGuiStartTime(const DateTime &val);
  void _updateTimesInGui();
  void _updateSelectedTimeInGui();
  
  void _setFrameIndex(int val) { _frameIndex = val; }

  void _setSelectedTimeLabel(const string &text) {
    _selectedTimeLabel->setText(text.c_str());
  }
  
  void _setEndTime();
  void _acceptSelectedTime();
  // void _changeMovieLimits();
  void _resetMovieFrameTimes();

  double _getGuiMovieDurationSecs() const {
    return (_guiNFramesMovie - 1) * _guiFrameIntervalSecs;
  }

  DateTime _getGuiEndTime() const;
  DateTime _getGuiSelectedTime() const;
                                      
 private slots:

  // actions
  
  void _acceptGuiSelections();
  void _cancelGuiSelections();
  
  void _startMovie();
  void _stopMovie();
  void _outputMovieLoop();
  
  // move in time
  
  // void _setSelectedTime(const DateTime &val);
  
  void _shiftBack1();
  void _shiftBack3();

  void _shiftFwd1();
  void _shiftFwd3();

  // time slider
  
  void _timeSliderActionTriggered(int action);
  void _timeSliderValueChanged(int value);
  void _timeSliderReleased();
  void _timeSliderPressed();

  void _setNFrames(int val);
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

