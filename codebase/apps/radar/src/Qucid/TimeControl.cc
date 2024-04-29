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
///////////////////////////////////////////////////////////////
// TimeControl.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2024
//
///////////////////////////////////////////////////////////////
//
// Wraps a map object, and provides the toggled() method
// for responding to menu selection.
//
///////////////////////////////////////////////////////////////

#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <Radx/NcfRadxFile.hh>
#include "TimeControl.hh"
#include "CartManager.hh"
#include "cidd.h"

// Constructor

TimeControl::TimeControl(CartManager *parent,
                         const Params &params) :
        QDialog(parent),
        _parent(parent),
        _params(params)
        
{

  _timePanel = NULL;
  _timeLayout = NULL;
  _startTimeEdit = NULL;
  _endTimeLabel = NULL;
  _selectedTimeLabel = NULL;
  _timeSlider = NULL;
  _back1 = NULL;
  _fwd1 = NULL;
  _backDuration = NULL;
  _fwdDuration = NULL;
  _backMult = NULL;
  _fwdMult = NULL;
  _nFramesSelector = NULL;
  _frameIntervalSelector = NULL;
  _realtimeSelector = NULL;
  _sweepSelector = NULL;
  
  _nFramesMovie = _params.n_movie_frames;
  _frameIntervalSecs = _params.frame_interval_secs;
  _movieDurationSecs = (_nFramesMovie - 1) * _frameIntervalSecs;
  _frameIndex = 0;
  
  _startTime.set(_params.archive_start_time);
  _endTime = _startTime + _movieDurationSecs;
  _selectedTime = _startTime;

  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _guiSelectedTime = _selectedTime;
  _guiNFramesMovie = _nFramesMovie;
  _guiFrameIntervalSecs = _frameIntervalSecs;
  _guiFrameIndex = _frameIndex;
  
  _frameDwellMsecs = _params.movie_dwell_msecs;
  _loopDelayMsecs = _params.loop_delay_msecs;

  _isRealtime = (_params.start_mode == Params::MODE_REALTIME);
  _isSweep = false;
  
  populateGui();

  setGuiStartTime(_guiStartTime);
  setGuiEndTime(_guiEndTime);
  setGuiSelectedTime(_guiSelectedTime);

}

// destructor

TimeControl::~TimeControl()
  
{

}

//////////////////////////////////////////////
// populate the time panel gui

void TimeControl::populateGui()
{
  
  setWindowTitle("Time and movie loop controller");
  QPoint pos(0,0);
  move(pos);
  
  QVBoxLayout *timeControlLayout = new QVBoxLayout(this);
  timeControlLayout->setSpacing(0);
  timeControlLayout->setContentsMargins(0, 0, 0, 0);
  
  // create time panel
  
  _timePanel = new QFrame(this);
  timeControlLayout->addWidget(_timePanel, Qt::AlignCenter);
  _timeLayout = new QVBoxLayout;
  _timePanel->setLayout(_timeLayout);

  // upper section
  
  QFrame *timeUpper = new QFrame(_timePanel);
  QHBoxLayout *timeUpperLayout = new QHBoxLayout;
  timeUpperLayout->setSpacing(0);
  timeUpperLayout->setContentsMargins(0, 0, 0, 0);
  timeUpper->setLayout(timeUpperLayout);

  // lower section
  
  QFrame *timeLower = new QFrame(_timePanel);
  QHBoxLayout *timeLowerLayout = new QHBoxLayout;
  timeLowerLayout->setSpacing(0);
  timeLowerLayout->setContentsMargins(0, 0, 0, 0);
  timeLower->setLayout(timeLowerLayout);
  
  _timeLayout->addWidget(timeUpper);
  _timeLayout->addWidget(timeLower);
  
  // create slider
  
  _timeSlider = new QSlider(Qt::Horizontal);
  _timeSlider->setFocusPolicy(Qt::StrongFocus);
  _timeSlider->setTickPosition(QSlider::TicksBothSides);
  _timeSlider->setTickInterval(1);
  _timeSlider->setTracking(true);
  _timeSlider->setSingleStep(1);
  _timeSlider->setPageStep(0);
  _timeSlider->setFixedWidth(250);
  _timeSlider->setToolTip("Drag to change time frame");
  _timeSlider->setMinimum(0);
  _timeSlider->setMaximum(_nFramesMovie - 1);
  
  // start time editor
  
  QFrame *startTimeFrame = new QFrame(timeUpper);
  QVBoxLayout *startTimeFrameLayout = new QVBoxLayout;

  startTimeFrameLayout->setSpacing(0);
  startTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  startTimeFrame->setLayout(startTimeFrameLayout);
  startTimeFrame->setStyleSheet("border: 1px solid black; "
                                "padding: 2px 2px 2px 2px; "
                                "background-color: lightgray;");

  QLabel *startTitle = new QLabel(startTimeFrame);
  startTitle->setText("Movie start time");
  startTitle->setAlignment(Qt::AlignHCenter);
  
  _startTimeEdit = new QDateTimeEdit(timeUpper);
  _startTimeEdit->setDisplayFormat("yyyy/mm/dd hh:mm:ss");
  _startTimeEdit->setCalendarPopup(true);
  connect(_startTimeEdit, &QDateTimeEdit::dateTimeChanged,
          this, &TimeControl::setStartTimeFromEdit);
  _startTimeEdit->setToolTip("Start time of movie");
  
  startTimeFrameLayout->addWidget(startTitle, 0, Qt::AlignTop);
  startTimeFrameLayout->addWidget(_startTimeEdit, 0, Qt::AlignTop);
  
  // end time editor

  QFrame *endTimeFrame = new QFrame(_timePanel);
  QVBoxLayout *endTimeFrameLayout = new QVBoxLayout;
  
  endTimeFrameLayout->setSpacing(0);
  endTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  endTimeFrame->setLayout(endTimeFrameLayout);
  endTimeFrame->setStyleSheet("border: 1px solid black; "
                              "padding: 2px 2px 2px 2px; "
                              "background-color: lightgray;");

  QLabel *endTitle = new QLabel(endTimeFrame);
  endTitle->setText("Movie end time");
  endTitle->setAlignment(Qt::AlignHCenter);

  _endTimeLabel = new QLabel(_timePanel);
  _endTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  _endTimeLabel->setToolTip("End time of movie");
  
  endTimeFrameLayout->addWidget(endTitle, 0, Qt::AlignTop);
  endTimeFrameLayout->addWidget(_endTimeLabel, 0, Qt::AlignTop);

  // selected time
  
  QFrame *selectedTimeFrame = new QFrame(_timePanel);
  QVBoxLayout *selectedTimeFrameLayout = new QVBoxLayout;
  
  selectedTimeFrameLayout->setSpacing(0);
  selectedTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  selectedTimeFrame->setLayout(selectedTimeFrameLayout);
  selectedTimeFrame->setLineWidth(1);
  selectedTimeFrame->setStyleSheet("border: 1px solid black; "
                                   "padding: 2px 2px 2px 2px; "
                                   "background-color: lightgray;");

  QLabel *selectedTitle = new QLabel(selectedTimeFrame);
  selectedTitle->setText("Selected time");
  selectedTitle->setAlignment(Qt::AlignHCenter);

  _selectedTimeLabel = new QLabel(_timePanel);
  _selectedTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  _selectedTimeLabel->setToolTip("This is the selected data time");
  setGuiSelectedTime(_guiSelectedTime);
  
  selectedTimeFrameLayout->addWidget(selectedTitle, 0, Qt::AlignTop);
  selectedTimeFrameLayout->addWidget(_selectedTimeLabel, 0, Qt::AlignTop);
  
  // fwd and back buttons

  QString buttonStyle("padding: 2px 4px 2px 4px; "
                      "background-color: seagreen;");
  
  _back1 = new QPushButton(timeLower);
  _back1->setText("<");
  connect(_back1, &QPushButton::clicked, this, &TimeControl::goBack1);
  _back1->setToolTip("Go back by 1 frame");
  _back1->setStyleSheet(buttonStyle);

  _fwd1 = new QPushButton(timeLower);
  _fwd1->setText(">");
  connect(_fwd1, &QPushButton::clicked, this, &TimeControl::goFwd1);
  _fwd1->setToolTip("Go forward by 1 frame");
  _fwd1->setStyleSheet(buttonStyle);
    
  _backDuration = new QPushButton(timeLower);
  _backDuration->setText("<<");
  connect(_backDuration, &QPushButton::clicked, this, &TimeControl::goBackDuration);
  _backDuration->setToolTip("Go back by 1 movie duration");
  _backDuration->setStyleSheet(buttonStyle);
  
  _fwdDuration = new QPushButton(timeLower);
  _fwdDuration->setText(">>");
  connect(_fwdDuration, &QPushButton::clicked, this, &TimeControl::goFwdDuration);
  _fwdDuration->setToolTip("Go forward by 1 movie duration");
  _fwdDuration->setStyleSheet(buttonStyle);

  _backMult = new QPushButton(timeLower);
  _backMult->setText("<<<");
  connect(_backMult, &QPushButton::clicked, this, &TimeControl::goBackMult);
  _backMult->setToolTip("Go back by 6 movie durations");
  _backMult->setStyleSheet(buttonStyle);
  
  _fwdMult = new QPushButton(timeLower);
  _fwdMult->setText(">>>");
  connect(_fwdMult, &QPushButton::clicked, this, &TimeControl::goFwdMult);
  _fwdMult->setToolTip("Go forward by 6 movie durations");
  _fwdMult->setStyleSheet(buttonStyle);

  // nframes and frame interval

  _nFramesSelector = new QSpinBox(timeLower);
  _nFramesSelector->setMinimum(1);
  _nFramesSelector->setPrefix("N frames: ");
  _nFramesSelector->setValue(_nFramesMovie);
  connect(_nFramesSelector, &QSpinBox::valueChanged,
          this, &TimeControl::_timeSliderSetNFrames);

  _frameIntervalSelector = new QDoubleSpinBox(timeLower);
  _frameIntervalSelector->setMinimum(1.0);
  _frameIntervalSelector->setMaximum(_params.max_frame_interval_secs);
  _frameIntervalSelector->setDecimals(0);
  _frameIntervalSelector->setPrefix("Frame interval (secs): ");
  _frameIntervalSelector->setValue(_frameIntervalSecs);
  connect(_frameIntervalSelector, &QDoubleSpinBox::valueChanged,
          this, &TimeControl::_setFrameIntervalSecs);

  // accept cancel buttons

  QFrame *acceptCancelFrame = new QFrame(timeUpper);
  QVBoxLayout *acceptCancelFrameLayout = new QVBoxLayout;
  acceptCancelFrameLayout->setSpacing(0);
  acceptCancelFrameLayout->setContentsMargins(0, 0, 0, 0);
  acceptCancelFrame->setLayout(acceptCancelFrameLayout);
  
  QPushButton *acceptButton = new QPushButton(acceptCancelFrame);
  acceptButton->setText("Accept");
  connect(acceptButton, &QPushButton::clicked, this,
          &TimeControl::acceptGuiSelections);
  acceptButton->setToolTip("Accept the selection");
  acceptButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                              "background-color: darkgreen;");
  
  QPushButton *cancelButton = new QPushButton(acceptCancelFrame);
  cancelButton->setText("Cancel");
  connect(cancelButton, &QPushButton::clicked, this,
          &TimeControl::cancelGuiSelections);
  cancelButton->setToolTip("Cancel the selection");
  cancelButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                              "background-color: darkred;");

  acceptCancelFrameLayout->addWidget(acceptButton, 0, Qt::AlignTop);
  acceptCancelFrameLayout->addWidget(cancelButton, 0, Qt::AlignTop);
  
  // start stop buttons

  QFrame *startStopFrame = new QFrame(timeUpper);
  QVBoxLayout *startStopFrameLayout = new QVBoxLayout;
  startStopFrameLayout->setSpacing(0);
  startStopFrameLayout->setContentsMargins(0, 0, 0, 0);
  startStopFrame->setLayout(startStopFrameLayout);
  
  QPushButton *startButton = new QPushButton(startStopFrame);
  startButton->setText("Start");
  connect(startButton, &QPushButton::clicked, this,
          &TimeControl::startMovie);
  startButton->setToolTip("Start the movie");
  startButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                             "background-color: darkgreen;");
  
  QPushButton *stopButton = new QPushButton(startStopFrame);
  stopButton->setText("Stop");
  connect(stopButton, &QPushButton::clicked, this,
          &TimeControl::stopMovie);
  stopButton->setToolTip("Stop the movie");
  stopButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                            "background-color: darkred;");

  startStopFrameLayout->addWidget(startButton, 0, Qt::AlignTop);
  startStopFrameLayout->addWidget(stopButton, 0, Qt::AlignTop);

  // realtime?

  QFrame *realtimeFrame = new QFrame(timeUpper);
  QVBoxLayout *realtimeFrameLayout = new QVBoxLayout;
  realtimeFrameLayout->setSpacing(0);
  realtimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  realtimeFrame->setLayout(realtimeFrameLayout);

  QLabel *realtimeTitle = new QLabel(realtimeFrame);
  realtimeTitle->setText("Realtime?");
  realtimeTitle->setAlignment(Qt::AlignHCenter);

  _realtimeSelector = new QCheckBox(realtimeFrame);
  _realtimeSelector->setChecked(_isRealtime);

  realtimeFrameLayout->addWidget(realtimeTitle, 0,
                                 Qt::AlignTop | Qt::AlignCenter);
  realtimeFrameLayout->addWidget(_realtimeSelector, 0,
                                 Qt::AlignBottom | Qt::AlignCenter);
  
#if QT_VERSION >= 0x067000
  connect(_realtimeSelector, &QCheckBox::checkStateChanged, this,
          &TimeControl::_setRealtime);
#else
  connect(_realtimeSelector, &QCheckBox::stateChanged, this,
          &TimeControl::_setRealtime);
#endif
  
  // sweep?

  QFrame *sweepFrame = new QFrame(timeUpper);
  QVBoxLayout *sweepFrameLayout = new QVBoxLayout;
  sweepFrameLayout->setSpacing(0);
  sweepFrameLayout->setContentsMargins(0, 0, 0, 0);
  sweepFrame->setLayout(sweepFrameLayout);

  QLabel *sweepTitle = new QLabel(sweepFrame);
  sweepTitle->setText("Sweep?");
  sweepTitle->setAlignment(Qt::AlignHCenter);

  _sweepSelector = new QCheckBox(sweepFrame);
  _sweepSelector->setChecked(_isSweep);
  
  sweepFrameLayout->addWidget(sweepTitle, 0,
                              Qt::AlignTop | Qt::AlignCenter);
  sweepFrameLayout->addWidget(_sweepSelector, 0,
                              Qt::AlignBottom | Qt::AlignCenter);
  
#if QT_VERSION >= 0x067000
  connect(_sweepSelector, &QCheckBox::checkStateChanged, this,
          &TimeControl::_setSweep);
#else
  connect(_sweepSelector, &QCheckBox::stateChanged, this,
          &TimeControl::_setSweep);
#endif
  
  // add widgets to layouts
  
  int stretch = 0;
  timeUpperLayout->addWidget(startStopFrame, stretch, Qt::AlignLeft);
  timeUpperLayout->addWidget(realtimeFrame, stretch,
                             Qt::AlignLeft | Qt::AlignTop);
  timeUpperLayout->addWidget(sweepFrame, stretch,
                             Qt::AlignLeft | Qt::AlignTop);
  timeUpperLayout->addWidget(startTimeFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(endTimeFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(selectedTimeFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(acceptCancelFrame, stretch, Qt::AlignRight);
  
  timeLowerLayout->addWidget(_backMult, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(_backDuration, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(_back1, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(_timeSlider, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(_nFramesSelector, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(_frameIntervalSelector, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(_fwd1, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_fwdDuration, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_fwdMult, stretch, Qt::AlignRight);

  // connect signals and slots
  
  connect(_timeSlider, &QSlider::actionTriggered,
          this, &TimeControl::_timeSliderActionTriggered);
  
  connect(_timeSlider, &QSlider::valueChanged,
          this, &TimeControl::_timeSliderValueChanged);
  
  connect(_timeSlider, &QSlider::sliderReleased,
          this, &TimeControl::_timeSliderReleased);
  
  connect(_timeSlider, &QSlider::sliderPressed,
          this, &TimeControl::_timeSliderPressed);
  
}

/////////////////////////////////////////////////////////////////////
// accept or cancel gui selections
// data retrieval does not change until the selections are accepted

void TimeControl::acceptGuiSelections()
{
  _startTime = _guiStartTime;
  _endTime = _guiEndTime;
  _selectedTime = _guiSelectedTime;
  _nFramesMovie = _guiNFramesMovie;
  _frameIntervalSecs = _guiFrameIntervalSecs;
  _frameIndex = _guiFrameIndex;
  _movieDurationSecs = (_nFramesMovie - 1) * _frameIntervalSecs;
  _endTime = _startTime + _movieDurationSecs;
  _selectedTime = _startTime + _frameIndex * _frameIntervalSecs;
  setGuiFromSelections();
}

void TimeControl::cancelGuiSelections()
{
  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _guiSelectedTime = _selectedTime;
  _guiNFramesMovie = _nFramesMovie;
  _guiFrameIntervalSecs = _frameIntervalSecs;
  _guiFrameIndex = _frameIndex;
  setGuiFromSelections();
}

////////////////////////////////////////////////////////
// start/stop the movie

void TimeControl::startMovie()
{
  cerr << "Start the movie" << endl;
}

void TimeControl::stopMovie()
{
  cerr << "Stop the movie" << endl;
}

/////////////////////////////////////////////////////////////////////
// grab the start time from the editor widget

void TimeControl::setStartTimeFromEdit(const QDateTime &val) {
  QDate dd = val.date();
  QTime tt = val.time();
  _guiStartTime.set(dd.year(), dd.month(), dd.day(),
                    tt.hour(), tt.minute(), tt.second());
  _guiEndTime = _guiStartTime + _movieDurationSecs;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  setGuiEndTime(_guiEndTime);
  setGuiSelectedTime(_guiSelectedTime);
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void TimeControl::setGuiStartTime(const RadxTime &val)
{
  if (!_startTimeEdit) {
    return;
  }
  QDateTime qtime = getQDateTime(val);
  _startTimeEdit->setDateTime(qtime);
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void TimeControl::setGuiEndTime(const RadxTime &val)
{
  if (!_endTimeLabel) {
    return;
  }
  QDateTime qtime = getQDateTime(val);
  _endTimeLabel->setText(val.asString(0).c_str());
}

////////////////////////////////////////////////////////
// set gui selected time label

void TimeControl::setGuiSelectedTime(const RadxTime &val)
{
  if (!_selectedTimeLabel) {
    return;
  }
  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           val.getYear(),
           val.getMonth(),
           val.getDay(),
           val.getHour(),
           val.getMin(),
           val.getSec());
  _selectedTimeLabel->setText(text);
}

////////////////////////////////////////////////////////
// set gui from gui times

void TimeControl::setGuiFromSelections()

{
  setGuiStartTime(_guiStartTime);
  setGuiEndTime(_guiEndTime);
  setGuiSelectedTime(_guiSelectedTime);
  if (_timeSlider) {
    _timeSlider->setSliderPosition(_guiFrameIndex);
    _timeSlider->setMaximum(_guiNFramesMovie - 1);
  }
  if (_nFramesSelector) {
    _nFramesSelector->setValue(_guiNFramesMovie);
  }
  if (_frameIntervalSelector) {
    _frameIntervalSelector->setValue(_guiFrameIntervalSecs);
  }
}

////////////////////////////////////////////////////////
// set archive start time

void TimeControl::setStartTime(const RadxTime &rtime)

{
  _startTime = rtime;
  if (!_startTime.isValid()) {
    _startTime.set(RadxTime::NOW);
  }
  _endTime = _startTime + _movieDurationSecs;
  _selectedTime = _startTime + _frameIndex * _frameIntervalSecs;
  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _guiSelectedTime = _selectedTime;
  setGuiFromSelections();
}

////////////////////////////////////////////////////////
// set archive end time

void TimeControl::setEndTime(const RadxTime &rtime)

{
  _endTime = rtime;
  if (!_endTime.isValid()) {
    _endTime.set(RadxTime::NOW);
  }
  _startTime = _endTime - _movieDurationSecs;
  _selectedTime = _startTime + _frameIndex * _frameIntervalSecs;
  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _guiSelectedTime = _selectedTime;
  setGuiFromSelections();
}

////////////////////////////////////////////////////////
// change start time
// return true if retrieval is pending, false otherwise

void TimeControl::goBack1()
{
  if (_guiFrameIndex <= 0) {
    _guiFrameIndex = 0;
  } else {
    _guiFrameIndex -= 1;
  }
  _timeSlider->setSliderPosition(_guiFrameIndex);
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  acceptGuiSelections();
}

void TimeControl::goBackDuration()
{
  int deltaSecs = _endTime - _startTime;
  _guiStartTime -= deltaSecs;
  _guiEndTime -= deltaSecs;
  _guiSelectedTime -= deltaSecs;
  acceptGuiSelections();
}

void TimeControl::goBackMult()
{
  int deltaSecs = (_endTime - _startTime) * 6;
  _guiStartTime -= deltaSecs;
  _guiEndTime -= deltaSecs;
  _guiSelectedTime -= deltaSecs;
  acceptGuiSelections();
}

void TimeControl::goFwd1()
{
  if (_guiFrameIndex < _nFramesMovie - 1) {
    _guiFrameIndex += 1;
  } else {
    _guiFrameIndex = _nFramesMovie - 1;
  }
  _timeSlider->setSliderPosition(_guiFrameIndex);
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  acceptGuiSelections();
}

void TimeControl::goFwdDuration()
{
  int deltaSecs = _endTime - _startTime;
  _guiStartTime += deltaSecs;
  _guiEndTime += deltaSecs;
  _guiSelectedTime += deltaSecs;
  acceptGuiSelections();
}

void TimeControl::goFwdMult()
{
  int deltaSecs = (_endTime - _startTime) * 6;
  _guiStartTime += deltaSecs;
  _guiEndTime += deltaSecs;
  _guiSelectedTime += deltaSecs;
  acceptGuiSelections();
}

void TimeControl::_timeSliderActionTriggered(int action) {
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    switch (action) {
      case QAbstractSlider::SliderNoAction:
        cerr << "SliderNoAction action in _timeSliderActionTriggered" << endl;
        break;
      case QAbstractSlider::SliderSingleStepAdd: 
        cerr << "SliderSingleStepAdd action in _timeSliderActionTriggered" << endl;
        break; 
      case QAbstractSlider::SliderSingleStepSub:	
        cerr << "SliderSingleStepSub action in _timeSliderActionTriggered" << endl;
        break;
      case QAbstractSlider::SliderPageStepAdd:
        cerr << "SliderPageStepAdd action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderPageStepSub:
        cerr << "SliderPageStepSub action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderToMinimum:
        cerr << "SliderToMinimum action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderToMaximum:
        cerr << "SliderToMaximum action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderMove:
        cerr << "SliderMove action in _timeSliderActionTriggered" << endl;
        break;
      default: 
        cerr << "unknown action in _timeSliderActionTriggered" << endl;
    }
    cerr << "timeSliderActionTriggered, value: "
         << _timeSlider->value() << endl;
  }
} 

void TimeControl::_timeSliderValueChanged(int value) 
{
  if (value < 0 || value > _nFramesMovie - 1) {
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider changed, value: " << value << endl;
  }
  _guiFrameIndex = value;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  acceptGuiSelections();
}

void TimeControl::_timeSliderReleased() 
{
  int value = _timeSlider->value();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
  if (value < 0 || value > _nFramesMovie - 1) {
    return;
  }
  if (value == _guiFrameIndex) {
    return;
  }
  _guiFrameIndex = value;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  acceptGuiSelections();
}

void TimeControl::_timeSliderPressed() 
{
  int value = _timeSlider->value();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
}

void TimeControl::_timeSliderSetNFrames(int val) 
{
  _guiNFramesMovie = val;
  _timeSlider->setMaximum(_guiNFramesMovie - 1);
  _movieDurationSecs = (_guiNFramesMovie - 1) * _guiFrameIntervalSecs;
  if (_guiFrameIndex >= _guiNFramesMovie) {
    _guiFrameIndex = _guiNFramesMovie - 1;
  }
  _guiEndTime = _guiStartTime + _movieDurationSecs;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  setGuiEndTime(_guiEndTime);
  setGuiSelectedTime(_guiSelectedTime);
}

void TimeControl::_setFrameIntervalSecs(double val) 
{
  _guiFrameIntervalSecs = val;
  _movieDurationSecs = (_guiNFramesMovie - 1) * _guiFrameIntervalSecs;
  _guiEndTime = _guiStartTime + _movieDurationSecs;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  setEndTime(_guiEndTime);
  setGuiSelectedTime(_guiSelectedTime);
}

// realtime mode?

#if QT_VERSION >= 0x067000
void TimeControl::_setRealtime(Qt::CheckState val)
{
  if (val == Qt::Checked) {
    _isRealtime = true;
    cerr << "Realtime mode" << endl;
  } else {
    _isRealtime = false;
    cerr << "Archive mode" << endl;
  }
}
#else
void TimeControl::_setRealtime(int val)
{
  if (val == 0) {
    _isRealtime = false;
    cerr << "Archive mode" << endl;
  } else {
    _isRealtime = true;
    cerr << "Realtime mode" << endl;
  }
}
#endif

// sweep mode?

#if QT_VERSION >= 0x067000
void TimeControl::_setSweep(Qt::CheckState val)
{
  if (val == Qt::Checked) {
    _isSweep = true;
    cerr << "Sweep mode" << endl;
  } else {
    _isSweep = false;
    cerr << "Not sweep mode" << endl;
  }
}
#else
void TimeControl::_setSweep(int val)
{
  if (val == 0) {
    _isSweep = false;
    cerr << "Not sweep mode" << endl;
  } else {
    _isSweep = true;
    cerr << "Sweep mode" << endl;
  }
}
#endif


////////////////////////////////////////////////////////
// convert between Qt and Radx date/time objects

QDateTime TimeControl::getQDateTime(const RadxTime &rtime)
{
  QDate date(rtime.getYear(), 
             rtime.getMonth(),
             rtime.getDay());
  QTime time(rtime.getHour(),
             rtime.getMin(),
             rtime.getSec());
  QDateTime qtime(date, time);
  return qtime;
}

RadxTime TimeControl::getRadxTime(const QDateTime &qtime)
{
  RadxTime rtime(qtime.date().year(),
                 qtime.date().month(),
                 qtime.date().day(),
                 qtime.time().hour(),
                 qtime.time().minute(),
                 qtime.time().second());
  return rtime;
}

