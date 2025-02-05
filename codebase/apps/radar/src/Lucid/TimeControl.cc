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

#include <cassert>
#include <QFrame>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "TimeControl.hh"
#include "GuiManager.hh"
#include "cidd.h"

// initialize instance

TimeControl *TimeControl::_instance = nullptr;

// Constructor

TimeControl::TimeControl(GuiManager *parent,
                         const Params &params) :
        QDialog(parent),
        _manager(parent),
        _params(params)
        
{

  if (_instance != nullptr) {
    return;
  }

  _timePanel = NULL;
  _timeLayout = NULL;
  _startTimeEdit = NULL;
  _endTimeLabel = NULL;
  _selectedTimeLabel = NULL;
  _timeSlider = NULL;
  _timeSliderInProgress = false;
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
  _loopDwellSelector = NULL;
  _loopDelaySelector = NULL;
  
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
  
  _loopDwellMsecs = _params.movie_dwell_msecs;
  _loopDelayMsecs = _params.loop_delay_msecs;

  _isRealtime = (_params.start_mode == Params::MODE_REALTIME);
  _isSweep = false;
  
  _populateGui();

  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);

}

/*********************************************************************
 * Inst()
 */

TimeControl *TimeControl::getInstance()
{
  return _instance;
}


////////////////////////////
// destructor

TimeControl::~TimeControl()
  
{

}

//////////////////////////////////////////////
// populate the time panel gui

void TimeControl::_populateGui()
{
  
  setWindowTitle("Time and movie loop controller");
  // QPoint pos(0,0);
  // move(pos);
  // show();

  QVBoxLayout *timeControlLayout = new QVBoxLayout(this);
  timeControlLayout->setSpacing(0);
  timeControlLayout->setContentsMargins(0, 0, 0, 0);
  
  // create time panel
  
  _timePanel = new QFrame(this);
  timeControlLayout->addWidget(_timePanel, Qt::AlignCenter);
  _timeLayout = new QVBoxLayout;
  _timePanel->setLayout(_timeLayout);
  _timePanel->setStyleSheet("font: bold 10px;"
                            /* "padding: 1px 1px 1px 1px;" */);
  
  // upper section
  
  QFrame *timeUpper = new QFrame(_timePanel);
  QHBoxLayout *timeUpperLayout = new QHBoxLayout;
  timeUpperLayout->setSpacing(3);
  timeUpperLayout->setContentsMargins(0, 0, 0, 0);
  timeUpper->setLayout(timeUpperLayout);
  // timeUpper->setStyleSheet("border: 1px solid black; padding: 1px 1px 1px 1px;");

  // lower section
  
  QFrame *timeLower = new QFrame(_timePanel);
  QHBoxLayout *timeLowerLayout = new QHBoxLayout;
  timeLower->setLayout(timeLowerLayout);
  timeLowerLayout->setSpacing(3);
  timeLowerLayout->setContentsMargins(0, 0, 0, 0);
  // timeLower->setStyleSheet("border: 1px solid black; padding: 1px 1px 1px 1px;");
  
  _timeLayout->addWidget(timeUpper);
  _timeLayout->addWidget(timeLower);
  
  //////////////////////////////////////////////////////////////////////
  // upper panel contents
  //////////////////////////////////////////////////////////////////////

  // start/stop buttons
  
  QFrame *startStopFrame = new QFrame(timeUpper);
  QVBoxLayout *startStopFrameLayout = new QVBoxLayout;
  startStopFrame->setLayout(startStopFrameLayout);
  startStopFrameLayout->setSpacing(2);
  startStopFrameLayout->setContentsMargins(2, 2, 2, 2);
  
  QPushButton *startButton = new QPushButton(startStopFrame);
  startButton->setText("Start");
  connect(startButton, &QPushButton::clicked, this,
          &TimeControl::_startMovie);
  startButton->setToolTip("Start the movie");
  startButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                             "background-color: seagreen;");
  
  QPushButton *stopButton = new QPushButton(startStopFrame);
  stopButton->setText("Stop");
  connect(stopButton, &QPushButton::clicked, this,
          &TimeControl::_stopMovie);
  stopButton->setToolTip("Stop the movie");
  stopButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                            "background-color: red;");

  startStopFrameLayout->addWidget(startButton, 0, Qt::AlignTop);
  startStopFrameLayout->addWidget(stopButton, 0, Qt::AlignTop);

  // realtime?

  QFrame *realtimeFrame = new QFrame(timeUpper);
  QVBoxLayout *realtimeFrameLayout = new QVBoxLayout;
  realtimeFrame->setLayout(realtimeFrameLayout);
  // realtimeFrame->setStyleSheet("background-color: lightgray;");
  realtimeFrameLayout->setSpacing(5);
  realtimeFrameLayout->setContentsMargins(5, 5, 5, 5);
  
  QLabel *realtimeTitle = new QLabel(realtimeFrame);
  realtimeTitle->setText("Realtime?");
  realtimeTitle->setAlignment(Qt::AlignHCenter);

  _realtimeSelector = new QCheckBox(realtimeFrame);
  _realtimeSelector->setChecked(_isRealtime);
  _manager->setArchiveMode(!_isRealtime);

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
  sweepFrame->setLayout(sweepFrameLayout);
  sweepFrameLayout->setSpacing(5);
  sweepFrameLayout->setContentsMargins(5, 5, 5, 5);
  // sweepFrame->setStyleSheet("background-color: lightgray;");

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
  
  // loop dwell (msecs)
  
  QFrame *loopDwellFrame = new QFrame(timeUpper);
  QVBoxLayout *loopDwellFrameLayout = new QVBoxLayout;
  loopDwellFrameLayout->setSpacing(2);
  loopDwellFrameLayout->setContentsMargins(2, 2, 2, 2);
  loopDwellFrame->setLayout(loopDwellFrameLayout);
  
  QLabel *loopDwellTitle = new QLabel(loopDwellFrame);
  loopDwellTitle->setText("Dwell (msecs)");
  loopDwellTitle->setAlignment(Qt::AlignHCenter);

  _loopDwellSelector = new QSpinBox(loopDwellFrame);
  _loopDwellSelector->setMinimum(_params.movie_min_dwell_msecs);
  _loopDwellSelector->setMaximum(_params.movie_max_dwell_msecs);
  _loopDwellSelector->setValue(_loopDwellMsecs);
#if QT_VERSION >= 0x067000
  connect(_loopDwellSelector, &QSpinBox::valueChanged,
          this, &TimeControl::_setLoopDwellMsecs);
#else
  connect(_loopDwellSelector, SIGNAL(valueChanged(int)),
          this, SLOT(_setLoopDwellMsecs(int)));
#endif
  
  loopDwellFrameLayout->addWidget(loopDwellTitle, 0, Qt::AlignTop);
  loopDwellFrameLayout->addWidget(_loopDwellSelector, 0, Qt::AlignTop);
  
  // loop delay (msecs)
  
  QFrame *loopDelayFrame = new QFrame(timeUpper);
  QVBoxLayout *loopDelayFrameLayout = new QVBoxLayout;
  loopDelayFrameLayout->setSpacing(2);
  loopDelayFrameLayout->setContentsMargins(2, 2, 2, 2);
  loopDelayFrame->setLayout(loopDelayFrameLayout);
  
  QLabel *loopDelayTitle = new QLabel(loopDelayFrame);
  loopDelayTitle->setText("Delay (msecs)");
  loopDelayTitle->setAlignment(Qt::AlignHCenter);

  _loopDelaySelector = new QSpinBox(loopDelayFrame);
  _loopDelaySelector->setMinimum(0);
  _loopDelaySelector->setMaximum(9999);
  _loopDelaySelector->setValue(_loopDelayMsecs);
#if QT_VERSION >= 0x067000
  connect(_loopDelaySelector, &QSpinBox::valueChanged,
          this, &TimeControl::_setLoopDelayMsecs);
#else
  connect(_loopDelaySelector, SIGNAL(valueChanged(int)),
          this, SLOT(_setLoopDelayMsecs(int)));
#endif
  
  loopDelayFrameLayout->addWidget(loopDelayTitle, 0, Qt::AlignTop);
  loopDelayFrameLayout->addWidget(_loopDelaySelector, 0, Qt::AlignTop);
  
  // start time editor
  
  QFrame *startTimeFrame = new QFrame(timeUpper);
  QVBoxLayout *startTimeFrameLayout = new QVBoxLayout;
  startTimeFrame->setLayout(startTimeFrameLayout);
  startTimeFrameLayout->setSpacing(0);
  startTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  startTimeFrame->setStyleSheet("border: 1px solid black; "
                                "padding: 2px 2px 2px 2px; ");

  QLabel *startTitle = new QLabel(startTimeFrame);
  startTitle->setText("Movie start time");
  startTitle->setAlignment(Qt::AlignHCenter);
  startTitle->setStyleSheet("background-color: lightgray;");
  
  _startTimeEdit = new QDateTimeEdit(timeUpper);
  _startTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  _startTimeEdit->setCalendarPopup(true);
  connect(_startTimeEdit, &QDateTimeEdit::dateTimeChanged,
          this, &TimeControl::_setStartTimeFromEdit);
  _startTimeEdit->setToolTip("Start time of movie");
  
  startTimeFrameLayout->addWidget(startTitle, 0, Qt::AlignTop);
  startTimeFrameLayout->addWidget(_startTimeEdit, 0, Qt::AlignTop);
  
  // end time label

  QFrame *endTimeFrame = new QFrame(_timePanel);
  QVBoxLayout *endTimeFrameLayout = new QVBoxLayout;
  endTimeFrame->setLayout(endTimeFrameLayout);
  endTimeFrameLayout->setSpacing(0);
  endTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  endTimeFrame->setStyleSheet("border: 1px solid black; "
                              "padding: 2px 2px 2px 2px; ");

  QLabel *endTitle = new QLabel(endTimeFrame);
  endTitle->setText("Movie end time");
  endTitle->setAlignment(Qt::AlignHCenter);
  endTitle->setStyleSheet("background-color: lightgray;");

  _endTimeLabel = new QLabel(_timePanel);
  _endTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  _endTimeLabel->setToolTip("End time of movie");
  
  endTimeFrameLayout->addWidget(endTitle, 0, Qt::AlignTop);
  endTimeFrameLayout->addWidget(_endTimeLabel, 0, Qt::AlignTop);

  // selected time label
  
  QFrame *selectedTimeFrame = new QFrame(_timePanel);
  QVBoxLayout *selectedTimeFrameLayout = new QVBoxLayout;
  selectedTimeFrame->setLayout(selectedTimeFrameLayout);
  selectedTimeFrameLayout->setSpacing(0);
  selectedTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  selectedTimeFrame->setLineWidth(1);
  selectedTimeFrame->setStyleSheet("border: 1px solid black; "
                                   "padding: 2px 2px 2px 2px; ");

  QLabel *selectedTitle = new QLabel(selectedTimeFrame);
  selectedTitle->setText("Selected time");
  selectedTitle->setAlignment(Qt::AlignHCenter);
  selectedTitle->setStyleSheet("background-color: lightgray;");

  _selectedTimeLabel = new QLabel(_timePanel);
  _selectedTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  _selectedTimeLabel->setToolTip("This is the selected data time");
  _setGuiSelectedTime(_guiSelectedTime);
  
  selectedTimeFrameLayout->addWidget(selectedTitle, 0, Qt::AlignTop);
  selectedTimeFrameLayout->addWidget(_selectedTimeLabel, 0, Qt::AlignTop);
  
  // accept/cancel buttons

  QFrame *acceptCancelFrame = new QFrame(timeUpper);
  QVBoxLayout *acceptCancelFrameLayout = new QVBoxLayout;
  acceptCancelFrame->setLayout(acceptCancelFrameLayout);
  acceptCancelFrameLayout->setSpacing(2);
  acceptCancelFrameLayout->setContentsMargins(2, 2, 2, 2);
  
  QPushButton *acceptButton = new QPushButton(acceptCancelFrame);
  acceptButton->setText("Accept");
  connect(acceptButton, &QPushButton::clicked, this,
          &TimeControl::_acceptGuiSelections);
  acceptButton->setToolTip("Accept the selection");
  acceptButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                              "background-color: seagreen;");
  
  QPushButton *cancelButton = new QPushButton(acceptCancelFrame);
  cancelButton->setText("Cancel");
  connect(cancelButton, &QPushButton::clicked, this,
          &TimeControl::_cancelGuiSelections);
  cancelButton->setToolTip("Cancel the selection");
  cancelButton->setStyleSheet("padding: 2px 4px 2px 4px; "
                              "background-color: red;");

  acceptCancelFrameLayout->addWidget(acceptButton, 0, Qt::AlignTop);
  acceptCancelFrameLayout->addWidget(cancelButton, 0, Qt::AlignTop);

  //////////////////////////////////////////////////////////////////////
  // lower panel contents
  //////////////////////////////////////////////////////////////////////

  // output loop button
  
  QFrame *outputLoopFrame = new QFrame(timeUpper);
  QVBoxLayout *outputLoopFrameLayout = new QVBoxLayout;
  outputLoopFrame->setLayout(outputLoopFrameLayout);
  outputLoopFrameLayout->setSpacing(2);
  outputLoopFrameLayout->setContentsMargins(2, 2, 2, 2);
  
  QPushButton *outputButton = new QPushButton(timeLower);
  outputButton->setText("Output Loop");
  connect(outputButton, &QPushButton::clicked, this,
          &TimeControl::_outputMovieLoop);
  outputButton->setToolTip("Output movie loop to file");
  outputButton->setStyleSheet("padding: 3px 6px 3px 6px; "
                              "background-color: seagreen;");
  
  outputLoopFrameLayout->addWidget(outputButton, 0, Qt::AlignLeft);
  
  // fwd and back buttons
  
  QString buttonStyle("padding: 3px 6px 3px 6px; "
                      "background-color: seagreen;");
  
  QFrame *backButtonFrame = new QFrame(timeLower);
  QHBoxLayout *backButtonFrameLayout = new QHBoxLayout;
  backButtonFrame->setLayout(backButtonFrameLayout);
  backButtonFrameLayout->setSpacing(3);
  backButtonFrameLayout->setContentsMargins(2, 2, 2, 2);
  
  _back1 = new QPushButton(backButtonFrame);
  _back1->setText("<");
  connect(_back1, &QPushButton::clicked, this, &TimeControl::goBack1);
  _back1->setToolTip("Go back by 1 frame");
  _back1->setStyleSheet(buttonStyle);

  _backDuration = new QPushButton(backButtonFrame);
  _backDuration->setText("<<");
  connect(_backDuration, &QPushButton::clicked, this, &TimeControl::_shiftBack1);
  _backDuration->setToolTip("Go back by 1 movie duration");
  _backDuration->setStyleSheet(buttonStyle);
  
  _backMult = new QPushButton(backButtonFrame);
  _backMult->setText("<<<");
  connect(_backMult, &QPushButton::clicked, this, &TimeControl::_shiftBack3);
  _backMult->setToolTip("Go back by 3 movie durations");
  _backMult->setStyleSheet(buttonStyle);
  
  backButtonFrameLayout->addWidget(_backMult, 0, Qt::AlignLeft);
  backButtonFrameLayout->addWidget(_backDuration, 0, Qt::AlignLeft);
  backButtonFrameLayout->addWidget(_back1, 0, Qt::AlignLeft);

  QFrame *fwdButtonFrame = new QFrame(timeLower);
  QHBoxLayout *fwdButtonFrameLayout = new QHBoxLayout;
  fwdButtonFrame->setLayout(fwdButtonFrameLayout);
  fwdButtonFrameLayout->setSpacing(3);
  fwdButtonFrameLayout->setContentsMargins(2, 2, 2, 2);
  
  _fwd1 = new QPushButton(fwdButtonFrame);
  _fwd1->setText(">");
  connect(_fwd1, &QPushButton::clicked, this, &TimeControl::goFwd1);
  _fwd1->setToolTip("Go forward by 1 frame");
  _fwd1->setStyleSheet(buttonStyle);
    
  _fwdDuration = new QPushButton(fwdButtonFrame);
  _fwdDuration->setText(">>");
  connect(_fwdDuration, &QPushButton::clicked, this, &TimeControl::_shiftFwd1);
  _fwdDuration->setToolTip("Go forward by 1 movie duration");
  _fwdDuration->setStyleSheet(buttonStyle);

  _fwdMult = new QPushButton(fwdButtonFrame);
  _fwdMult->setText(">>>");
  connect(_fwdMult, &QPushButton::clicked, this, &TimeControl::_shiftFwd3);
  _fwdMult->setToolTip("Go forward by 3 movie durations");
  _fwdMult->setStyleSheet(buttonStyle);

  fwdButtonFrameLayout->addWidget(_fwd1, 0, Qt::AlignRight);
  fwdButtonFrameLayout->addWidget(_fwdDuration, 0, Qt::AlignRight);
  fwdButtonFrameLayout->addWidget(_fwdMult, 0, Qt::AlignRight);

  // time slider
  
  QFrame *timeSliderFrame = new QFrame(timeLower);
  QHBoxLayout *timeSliderFrameLayout = new QHBoxLayout;
  timeSliderFrameLayout->setSpacing(2);
  timeSliderFrameLayout->setContentsMargins(2, 2, 2, 2);
  timeSliderFrame->setLayout(timeSliderFrameLayout);
  // timeSliderFrame->setStyleSheet("border: 1px solid black; ");
  
  _timeSlider = new QSlider(Qt::Horizontal);
  _timeSlider->setFocusPolicy(Qt::StrongFocus);
  _timeSlider->setTickPosition(QSlider::TicksBothSides);
  _timeSlider->setTickInterval(1);
  _timeSlider->setTracking(true);
  _timeSlider->setSingleStep(1);
  _timeSlider->setPageStep(0);
  _timeSlider->setFixedWidth(200);
  _timeSlider->setToolTip("Drag to change time frame");
  _timeSlider->setMinimum(0);
  _timeSlider->setMaximum(_nFramesMovie - 1);
  
  // nframes and frame interval

  _nFramesSelector = new QSpinBox(timeSliderFrame);
  _nFramesSelector->setMinimum(1);
  _nFramesSelector->setMaximum(MAX_FRAMES);
  _nFramesSelector->setPrefix("N frames: ");
  _nFramesSelector->setValue(_nFramesMovie);
  _nFramesSelector->setContentsMargins(2, 2, 2, 2);
#if QT_VERSION >= 0x067000
  connect(_nFramesSelector, &QSpinBox::valueChanged,
          this, &TimeControl::_timeSliderSetNFrames);
#else
  connect(_nFramesSelector, SIGNAL(valueChanged(int)),
          this, SLOT(_timeSliderSetNFrames(int)));
#endif

  _frameIntervalSelector = new QDoubleSpinBox(timeSliderFrame);
  _frameIntervalSelector->setMinimum(1.0);
  _frameIntervalSelector->setMaximum(_params.max_frame_interval_secs);
  _frameIntervalSelector->setDecimals(0);
  _frameIntervalSelector->setPrefix("Interval (secs): ");
  _frameIntervalSelector->setValue(_frameIntervalSecs);
  _frameIntervalSelector->setContentsMargins(2, 2, 2, 2);
#if QT_VERSION >= 0x067000
  connect(_frameIntervalSelector, &QDoubleSpinBox::valueChanged,
          this, &TimeControl::_setFrameIntervalSecs);
#else
  connect(_frameIntervalSelector, SIGNAL(valueChanged(double)),
          this, SLOT(_setFrameIntervalSecs(double)));
#endif

  // connect signals and slots
  
  connect(_timeSlider, &QSlider::actionTriggered,
          this, &TimeControl::_timeSliderActionTriggered);
  
  connect(_timeSlider, &QSlider::valueChanged,
          this, &TimeControl::_timeSliderValueChanged);
  
  connect(_timeSlider, &QSlider::sliderReleased,
          this, &TimeControl::_timeSliderReleased);
  
  connect(_timeSlider, &QSlider::sliderPressed,
          this, &TimeControl::_timeSliderPressed);
  
  timeSliderFrameLayout->addWidget(_nFramesSelector, 0, Qt::AlignLeft);
  timeSliderFrameLayout->addWidget(_timeSlider, 0, Qt::AlignCenter);
  timeSliderFrameLayout->addWidget(_frameIntervalSelector, 0, Qt::AlignRight);

  // add widgets to layouts
  
  int stretch = 0;
  timeUpperLayout->addWidget(startStopFrame, stretch, Qt::AlignLeft);
  timeUpperLayout->addWidget(realtimeFrame, stretch,
                             Qt::AlignLeft | Qt::AlignTop);
  timeUpperLayout->addWidget(sweepFrame, stretch,
                             Qt::AlignLeft | Qt::AlignTop);
  timeUpperLayout->addWidget(loopDwellFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(loopDelayFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(startTimeFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(endTimeFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(selectedTimeFrame, stretch, Qt::AlignLeft);
  timeUpperLayout->addWidget(acceptCancelFrame, stretch, Qt::AlignRight);
  
  timeLowerLayout->addWidget(outputLoopFrame, 0, Qt::AlignLeft);
  timeLowerLayout->addWidget(backButtonFrame, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(timeSliderFrame, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(fwdButtonFrame, stretch, Qt::AlignRight);
  
}

/////////////////////////////////////////
// set flags for change in selected time

void TimeControl::_acceptSelectedTime()
{
  cerr << "====>> acceptSelectedTime()" << endl;
  gd.data_request_time = _selectedTime.utime();
  invalidate_all_data(); // TODO later - check if this is correct
  set_redraw_flags(1, 1);
}

/////////////////////////////////////////
// set flags for change in movie limits

void TimeControl::_changeMovieLimits()
{

  cerr << "====>> changeMovieLimits()" << endl;
  gd.epoch_start = _startTime.utime();
  gd.epoch_end = _endTime.utime();
  gd.data_request_time = _selectedTime.utime();
  gd.movie.num_frames = _nFramesMovie;
  gd.movie.start_frame = 0;
  gd.movie.start_frame = _nFramesMovie - 1;
  gd.movie.display_time_msec = _loopDwellMsecs;
  gd.movie.delay = _loopDelayMsecs / _loopDwellMsecs;

  _resetMovieFrameTimes();
  invalidate_all_data();
  set_redraw_flags(1, 1);
  
  if (gd.prod_mgr) {
    gd.prod_mgr->reset_times_valid_flags();
  }
  
  // if(gd.time_plot) {
  //   gd.time_plot->Set_times(gd.epoch_start,
  //                           gd.epoch_end,
  //                           gd.movie.frame[gd.movie.cur_frame].time_start,
  //                           gd.movie.frame[gd.movie.cur_frame].time_end,
  //                           (gd.movie.time_interval_mins * 60.0) + 0.5,
  //                           gd.movie.num_frames);
  //   gd.time_plot->Draw(); 
  // }
    
}

/////////////////////////////////////////////////////////////////////
// accept or cancel gui selections
// data retrieval does not change until the selections are accepted

void TimeControl::_acceptGuiSelections()
{
  _startTime = _guiStartTime;
  _endTime = _guiEndTime;
  _setSelectedTime(_guiSelectedTime);
  _nFramesMovie = _guiNFramesMovie;
  _frameIntervalSecs = _guiFrameIntervalSecs;
  _frameIndex = _guiFrameIndex;
  _movieDurationSecs = (_nFramesMovie - 1) * _frameIntervalSecs;
  _endTime = _startTime + _movieDurationSecs;
  _setSelectedTime(_startTime + _frameIndex * _frameIntervalSecs);
  // set redraw flags
  _changeMovieLimits();
}

void TimeControl::_cancelGuiSelections()
{
  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _guiSelectedTime = _selectedTime;
  _guiNFramesMovie = _nFramesMovie;
  _guiFrameIntervalSecs = _frameIntervalSecs;
  _guiFrameIndex = _frameIndex;
  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

////////////////////////////////////////////////////////
// start/stop the movie, output movie loop to file

void TimeControl::_startMovie()
{
  cerr << "Start the movie" << endl;
}

void TimeControl::_stopMovie()
{
  cerr << "Stop the movie" << endl;
}

void TimeControl::_outputMovieLoop()
{
  cerr << "Output movie loop" << endl;
}

//////////////////////////////
// set number of movie frames

void TimeControl::_setNFramesMovie(int val)
{
  _nFramesMovie = val;
}

////////////////////////////////
// set movie frame interval secs

void TimeControl::_setIntervalSecs(double val)
{
  _frameIntervalSecs = val;
}

////////////////////////////////
// enable the gui elements

void TimeControl::setEnabled(bool val)
{
  _startTimeEdit->setEnabled(val);
  _back1->setEnabled(val);
  _fwd1->setEnabled(val);
  _backDuration->setEnabled(val);
  _fwdDuration->setEnabled(val);
  _backMult->setEnabled(val);
  _fwdMult->setEnabled(val);
}

/////////////////////////////////////////////////////////////////////
// grab the start time from the editor widget

void TimeControl::_setStartTimeFromEdit(const QDateTime &val) {
  QDate dd = val.date();
  QTime tt = val.time();
  _guiStartTime.set(dd.year(), dd.month(), dd.day(),
                    tt.hour(), tt.minute(), tt.second());
  _guiEndTime = _guiStartTime + _movieDurationSecs;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void TimeControl::_setGuiStartTime(const DateTime &val)
{
  if (!_startTimeEdit) {
    return;
  }
  QDateTime qtime = getQDateTime(val);
  _startTimeEdit->setDateTime(qtime);
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void TimeControl::_setGuiEndTime(const DateTime &val)
{
  if (!_endTimeLabel) {
    return;
  }
  QDateTime qtime = getQDateTime(val);
  _endTimeLabel->setText(val.asString(0).c_str());
}

////////////////////////////////////////////////////////
// set selected time

void TimeControl::_setSelectedTime(const DateTime &val)
{
  gd.prev_time = _selectedTime.utime();
  _selectedTime = val;
  gd.selected_time = _selectedTime.utime();
  gd.time_has_changed = true;
}

////////////////////////////////////////////////////////
// set gui selected time label

void TimeControl::_setGuiSelectedTime(const DateTime &val)
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
// set movie start time

void TimeControl::_setStartTime(const DateTime &stime)

{
  _startTime = stime;
  if (!_startTime.isValid()) {
    _startTime.setToNow();
  }
  _endTime = _startTime + _movieDurationSecs;
  _setSelectedTime(_startTime + _frameIndex * _frameIntervalSecs);
  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _guiSelectedTime = _selectedTime;
  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

// ////////////////////////////////////////////////////////
// // set movie end time

// void TimeControl::_setEndTime(const DateTime &rtime)

// {
//   _endTime = rtime;
//   if (!_endTime.isValid()) {
//     _endTime.setToNow();
//   }
//   _startTime = _endTime - _movieDurationSecs;
//   _setSelectedTime(_startTime + _frameIndex * _frameIntervalSecs);
//   _guiStartTime = _startTime;
//   _guiEndTime = _endTime;
//   _guiSelectedTime = _selectedTime;
//   setGuiStartTime(_guiStartTime);
//   setGuiEndTime(_guiEndTime);
//   setGuiSelectedTime(_guiSelectedTime);
// }

////////////////////////////////////////////////////////
// go back or fwd

// go back by 1 time step

void TimeControl::goBack1()
{
  // change index
  if (_guiFrameIndex == 0) {
    return;
  } else if (_guiFrameIndex < 0) {
    _guiFrameIndex = 0;
  } else {
    _guiFrameIndex -= 1;
  }
  // move slider
  _timeSliderInProgress = true;
  _timeSlider->setSliderPosition(_guiFrameIndex);
  _timeSliderInProgress = false;
  // update GUI
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  _setSelectedTime(_guiSelectedTime);
  _setGuiSelectedTime(_guiSelectedTime);
  // set redraw flags
  _acceptSelectedTime();
}

// shift back by movie duration

void TimeControl::_shiftBack1()
{
  int deltaSecs = _endTime - _startTime;
  _guiStartTime -= deltaSecs;
  _guiEndTime -= deltaSecs;
  _guiSelectedTime -= deltaSecs;
  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

// shift back by 3 movie durations

void TimeControl::_shiftBack3()
{
  int deltaSecs = (_endTime - _startTime) * 3;
  _guiStartTime -= deltaSecs;
  _guiEndTime -= deltaSecs;
  _guiSelectedTime -= deltaSecs;
  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

// go fwd by 1 time step

void TimeControl::goFwd1()
{
  // change index
  if (_guiFrameIndex == _nFramesMovie - 1) {
    return;
  } else if (_guiFrameIndex >= _nFramesMovie) {
    _guiFrameIndex = _nFramesMovie - 1;
  } else {
    _guiFrameIndex += 1;
  }
  // move slider
  _timeSliderInProgress = true;
  _timeSlider->setSliderPosition(_guiFrameIndex);
  _timeSliderInProgress = false;
  // update GUI
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  _setSelectedTime(_guiSelectedTime);
  _setGuiSelectedTime(_guiSelectedTime);
  // set redraw flags
  _acceptSelectedTime();
}

// shift fwd by one movie duration

void TimeControl::_shiftFwd1()
{
  int deltaSecs = _endTime - _startTime;
  _guiStartTime += deltaSecs;
  _guiEndTime += deltaSecs;
  _guiSelectedTime += deltaSecs;
  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

// shoft fwd by 3 movie durations

void TimeControl::_shiftFwd3()
{
  int deltaSecs = (_endTime - _startTime) * 3;
  _guiStartTime += deltaSecs;
  _guiEndTime += deltaSecs;
  _guiSelectedTime += deltaSecs;
  _setGuiStartTime(_guiStartTime);
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

// trap time slider trigger - no action for now

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

// trap change in slider position

void TimeControl::_timeSliderValueChanged(int value) 
{
  if (value < 0 || value > _nFramesMovie - 1) {
    return;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider changed, value: " << value << endl;
  }
  _guiSelectedTime = _guiStartTime + value * _guiFrameIntervalSecs;
  _setGuiSelectedTime(_guiSelectedTime);
  if (_timeSliderInProgress) {
    return;
  }
  // set redraw flags
  // _changeMovieLimits();
  _setSelectedTime(_guiSelectedTime);
  _manager->setSelectedTime(_selectedTime);
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
  _timeSliderInProgress = false;
  _guiFrameIndex = value;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  _setGuiSelectedTime(_guiSelectedTime);
  _manager->setSelectedTime(_selectedTime);
  // set redraw flags
  // _changeMovieLimits();
}

void TimeControl::_timeSliderPressed() 
{
  int value = _timeSlider->value();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider pressed, value: " << value << endl;
  }
  _timeSliderInProgress = true;
}

// set number of slider frames

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
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
}

// set frame interval

void TimeControl::_setFrameIntervalSecs(double val) 
{
  _guiFrameIntervalSecs = val;
  _movieDurationSecs = (_guiNFramesMovie - 1) * _guiFrameIntervalSecs;
  _guiEndTime = _guiStartTime + _movieDurationSecs;
  _guiSelectedTime = _guiStartTime + _guiFrameIndex * _guiFrameIntervalSecs;
  _setGuiEndTime(_guiEndTime);
  _setGuiSelectedTime(_guiSelectedTime);
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
  _manager->setArchiveMode(!_isRealtime);
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
  _manager->setArchiveMode(!_isRealtime);
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

////////////////////////////////////////////////
// loop dwell and delay

void TimeControl::_setLoopDwellMsecs(int val) 
{
  _loopDwellMsecs = val;
  cerr << "_loopDwellMsecs: " << _loopDwellMsecs << endl;
}

void TimeControl::_setLoopDelayMsecs(int val) 
{
  _loopDelayMsecs = val;
  cerr << "_loopDelayMsecs: " << _loopDelayMsecs << endl;
}

////////////////////////////////////////////////////////
// convert between Qt and Radx date/time objects

QDateTime TimeControl::getQDateTime(const DateTime &rtime)
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

DateTime TimeControl::getDateTime(const QDateTime &qtime)
{
  DateTime rtime(qtime.date().year(),
                 qtime.date().month(),
                 qtime.date().day(),
                 qtime.time().hour(),
                 qtime.time().minute(),
                 qtime.time().second());
  return rtime;
}

/************************************************************************
 * RESET_TIME_POINTS: Calc the beginning and ending time points for
 * each movie frame
 */

void TimeControl::_resetMovieFrameTimes()
{

  if(gd.debug2) {
    printf("Resetting time points - mode: %d\n",gd.movie.mode);
  }
  
  switch(gd.movie.climo_mode) {
    case REGULAR_INTERVAL:
      for (int i = 0; i < gd.movie.num_frames; i++)
      {
	double time_interval_secs = gd.movie.time_interval_mins * 60.0;
	
	gd.movie.frame[i].time_start =
	  (time_t)(gd.movie.start_time + (i * time_interval_secs) + 0.5);
        
	gd.movie.frame[i].time_mid =
	  (time_t)(gd.movie.start_time + ((i + 0.5) * time_interval_secs) + 0.5);
        
	gd.movie.frame[i].time_end =
	  (time_t)(gd.movie.frame[i].time_start + time_interval_secs + 0.5);
        
      }
      break;
      
    case DAILY_INTERVAL:
      for (int i = 0; i < gd.movie.num_frames; i++) {
        gd.movie.frame[i].time_start = (time_t)
          (gd.movie.start_time + (i * 1440 * 60.0));
        
        gd.movie.frame[i].time_mid = (time_t)
          (gd.movie.start_time + (gd.movie.frame_span * 30.0));
        
        gd.movie.frame[i].time_end =  (time_t)
          (gd.movie.frame[i].time_start + (gd.movie.frame_span * 60.0));
        
      }
      break;
      
    case YEARLY_INTERVAL:
      time_t    ft;
      UTIMstruct t;
      UTIMunix_to_date(gd.movie.start_time,&t);
      for (int i = 0; i < gd.movie.num_frames; i++, t.year++) {
        ft = UTIMdate_to_unix(&t);
        gd.movie.frame[i].time_start = (time_t) ft;
        
        gd.movie.frame[i].time_mid = (time_t)
          (gd.movie.frame[i].time_start + (gd.movie.frame_span * 30.0));
        
        gd.movie.frame[i].time_end =  (time_t)
          (gd.movie.frame[i].time_start + (gd.movie.frame_span * 60.0));
        
      }
      break;
  }
  reset_time_list_valid_flags();
  
  if (gd.prod_mgr) gd.prod_mgr->reset_times_valid_flags();
  
  gd.epoch_start = (time_t) gd.movie.start_time;
  gd.epoch_end =  (time_t) (gd.movie.frame[gd.movie.num_frames -1].time_end);
  
  if(gd.coord_expt != NULL) {
    gd.coord_expt->epoch_start = gd.epoch_start;
    gd.coord_expt->epoch_end = gd.epoch_end;
    gd.coord_expt->click_type = CIDD_OTHER_CLICK;
    gd.coord_expt->pointer_seq_num++;
    gd.coord_expt->time_min = gd.movie.frame[gd.movie.cur_frame].time_start;
    gd.coord_expt->time_max = gd.movie.frame[gd.movie.cur_frame].time_end;
    if(gd.movie.movie_on) { 
      gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
    } else {
      gd.coord_expt->time_cent = gd.coord_expt->time_min +
        ((gd.coord_expt->time_max - gd.coord_expt->time_min)/2);
    }
  }
  
}

