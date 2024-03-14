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
        QDialog((QDialog *) parent),
        _parent(parent),
        _params(params)
        
{

  _nFrames = _params.n_movie_frames;
  _frameDurationSecs = _params.frame_duration_secs;
  _frameIndex = 0;
  _frameDwellMsecs = _params.movie_dwell_msecs;
  _loopDelayMsecs = _params.loop_delay_msecs;
  _startTime.set(_params.archive_start_time);
  _endTime = _startTime + _nFrames * _frameDurationSecs;
  _guiStartTime = _startTime;
  _guiEndTime = _endTime;
  _selectedTime = _startTime;

  populateGui();
  
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

  QFrame *timeUpper = new QFrame(_timePanel);
  QHBoxLayout *timeUpperLayout = new QHBoxLayout;
  timeUpperLayout->setSpacing(0);
  timeUpperLayout->setContentsMargins(0, 0, 0, 0);
  timeUpper->setLayout(timeUpperLayout);
  
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
  _timeSlider->setFixedWidth(200);
  _timeSlider->setToolTip("Drag to change time frame");

  _timeSlider->setMinimum(0);
  _timeSlider->setMaximum(_nFrames - 1);
  
  // active time
  
  _selectedTimeLabel = new QPushButton(_timePanel);
  _selectedTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  QPalette pal = _selectedTimeLabel->palette();
  pal.setColor(QPalette::Active, QPalette::Button, Qt::cyan);
  _selectedTimeLabel->setPalette(pal);
  _selectedTimeLabel->setToolTip("This is the selected data time");
  setGuiFromSelectedTime();

  // start time editor
  
  _startTimeEdit = new QDateTimeEdit(timeUpper);
  QFrame *startTimeFrame = new QFrame(timeUpper);
  QVBoxLayout *startTimeFrameLayout = new QVBoxLayout;
  startTimeFrameLayout->setSpacing(0);
  startTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  startTimeFrame->setLayout(startTimeFrameLayout);
  QLabel *startLabel = new QLabel(startTimeFrame);
  startLabel->setText("Movie start time");
  startTimeFrameLayout->addWidget(startLabel, 0, Qt::AlignTop);
  startTimeFrameLayout->addWidget(_startTimeEdit, 0, Qt::AlignBottom);
  _startTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  _startTimeEdit->setCalendarPopup(true);
  connect(_startTimeEdit, &QDateTimeEdit::dateTimeChanged,
          this, &TimeControl::setStartTimeFromGui);
  _startTimeEdit->setToolTip("Start time of movie interval");
  setGuiFromStartTime();
  
  // end time editor
  
  _endTimeEdit = new QDateTimeEdit(timeUpper);
  QFrame *endTimeFrame = new QFrame(timeUpper);
  QVBoxLayout *endTimeFrameLayout = new QVBoxLayout;
  endTimeFrameLayout->setSpacing(0);
  endTimeFrameLayout->setContentsMargins(0, 0, 0, 0);
  endTimeFrame->setLayout(endTimeFrameLayout);
  QLabel *endLabel = new QLabel(endTimeFrame);
  endLabel->setText("Movie end time");
  endTimeFrameLayout->addWidget(endLabel, 0, Qt::AlignTop);
  endTimeFrameLayout->addWidget(_endTimeEdit, 0, Qt::AlignBottom);
  _endTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  _endTimeEdit->setCalendarPopup(true);
  connect(_endTimeEdit, &QDateTimeEdit::dateTimeChanged, 
          this, &TimeControl::setEndTimeFromGui);
  _endTimeEdit->setToolTip("End time of movie interval");
  setGuiFromEndTime();

  // fwd and back buttons

  _back1 = new QPushButton(timeLower);
  _back1->setText("<");
  connect(_back1, &QPushButton::clicked, this, &TimeControl::goBack1);
  _back1->setToolTip("Go back by 1 frame");
  
  _fwd1 = new QPushButton(timeLower);
  _fwd1->setText(">");
  connect(_fwd1, &QPushButton::clicked, this, &TimeControl::goFwd1);
  _fwd1->setToolTip("Go forward by 1 frame");
    
  _backPeriod = new QPushButton(timeLower);
  _backPeriod->setText("<<");
  connect(_backPeriod, &QPushButton::clicked, this, &TimeControl::goBackPeriod);
  _backPeriod->setToolTip("Go back by the loop period");
  
  _fwdPeriod = new QPushButton(timeLower);
  _fwdPeriod->setText(">>");
  connect(_fwdPeriod, &QPushButton::clicked, this, &TimeControl::goFwdPeriod);
  _fwdPeriod->setToolTip("Go forward by the loop period");

  // accept cancel buttons

  QPushButton *acceptButton = new QPushButton(timeUpper);
  acceptButton->setText("Accept");
  QPalette acceptPalette = acceptButton->palette();
  acceptPalette.setColor(QPalette::Active, QPalette::Button, Qt::green);
  acceptButton->setPalette(acceptPalette);
  connect(acceptButton, &QPushButton::clicked, this, &TimeControl::acceptGuiTimes);
  acceptButton->setToolTip("Accept the selection");
  
  QPushButton *cancelButton = new QPushButton(timeUpper);
  cancelButton->setText("Cancel");
  QPalette cancelPalette = cancelButton->palette();
  cancelPalette.setColor(QPalette::Active, QPalette::Button, Qt::red);
  cancelButton->setPalette(cancelPalette);
  connect(cancelButton, &QPushButton::clicked, this, &TimeControl::cancelGuiTimes);
  cancelButton->setToolTip("Cancel the selection");
    
  // add time widgets to layout
  
  int stretch = 0;
  timeUpperLayout->addWidget(cancelButton, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(startTimeFrame, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(_selectedTimeLabel, stretch, Qt::AlignCenter);
  timeUpperLayout->addWidget(endTimeFrame, stretch, Qt::AlignLeft);
  timeUpperLayout->addWidget(acceptButton, stretch, Qt::AlignLeft);
  
  timeLowerLayout->addWidget(_backPeriod, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_back1, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_timeSlider, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(_fwd1, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(_fwdPeriod, stretch, Qt::AlignLeft);

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

void TimeControl::acceptGuiTimes()
{
  _startTime = _guiStartTime;
  _endTime = _guiEndTime;
  // _parent->loadArchiveFileList();
}

void TimeControl::cancelGuiTimes()
{
  cerr << "CCCCCCCCCCCCCCCCCCCC" << endl;
  setGuiFromStartTime();
  setGuiFromEndTime();
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void TimeControl::setGuiFromStartTime()
{
  if (!_startTimeEdit) {
    return;
  }
  QDateTime qtime = getQDateTime(_startTime);
  _startTimeEdit->setDateTime(qtime);
  _guiStartTime = _startTime;
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void TimeControl::setGuiFromEndTime()
{
  if (!_endTimeEdit) {
    return;
  }
  QDateTime qtime = getQDateTime(_endTime);
  _endTimeEdit->setDateTime(qtime);
  _guiEndTime = _endTime;
}

////////////////////////////////////////////////////////
// set gui selected time label

void TimeControl::setGuiFromSelectedTime()
{
  if (!_selectedTimeLabel) {
    return;
  }
  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           _selectedTime.getYear(),
           _selectedTime.getMonth(),
           _selectedTime.getDay(),
           _selectedTime.getHour(),
           _selectedTime.getMin(),
           _selectedTime.getSec());
  _selectedTimeLabel->setText(text);
}

////////////////////////////////////////////////////////
// set archive start time

void TimeControl::setStartTime(const RadxTime &rtime)

{
  _startTime = rtime;
  if (!_startTime.isValid()) {
    _startTime.set(RadxTime::NOW);
  }
  setGuiFromStartTime();
}

////////////////////////////////////////////////////////
// set archive end time

void TimeControl::setEndTime(const RadxTime &rtime)

{
  _endTime = rtime;
  if (!_endTime.isValid()) {
    _endTime.set(RadxTime::NOW);
  }
  setGuiFromEndTime();
}

////////////////////////////////////////////////////////
// change start time
// return true if retrieval is pending, false otherwise

void TimeControl::goBack1()
{
  if (_frameIndex <= 0) {
    _frameIndex = 0;
  } else {
    _frameIndex -= 1;
  }
  _timeSlider->setSliderPosition(_frameIndex);
  _selectedTime = _startTime + _frameIndex + _frameDurationSecs;
  setGuiFromSelectedTime();
}

void TimeControl::goBackPeriod()
{
  int archiveSpanSecs = _endTime - _startTime;
  _startTime -= archiveSpanSecs;
  _endTime -= archiveSpanSecs;
  _selectedTime -= archiveSpanSecs;
  setGuiFromStartTime();
  setGuiFromEndTime();
  setGuiFromSelectedTime();
}

void TimeControl::goFwd1()
{
  if (_frameIndex < _nFrames - 1) {
    _frameIndex += 1;
  } else {
    _frameIndex = _nFrames - 1;
  }
  _timeSlider->setSliderPosition(_frameIndex);
  _selectedTime = _startTime + _frameIndex + _frameDurationSecs;
  setGuiFromSelectedTime();
}

void TimeControl::goFwdPeriod()
{
  int archiveSpanSecs = _endTime - _startTime;
  _startTime += archiveSpanSecs;
  _endTime += archiveSpanSecs;
  _selectedTime += archiveSpanSecs;
  setGuiFromStartTime();
  setGuiFromEndTime();
  setGuiFromSelectedTime();
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
  // if (value < 0 || value > (int) _parent->getArchiveFileListSize() - 1) {
  //   return;
  // }
  // get path for this value
  // string path = _parent->getArchiveFileList()[value];
  // // get time for this path
  RadxTime pathTime;
  // NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  setGuiFromSelectedTime();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider changed, value: " << value << endl;
  }
}

void TimeControl::_timeSliderReleased() 
{
  int value = _timeSlider->value();
  // if (value < 0 || value > (int) _parent->getArchiveFileListSize() - 1) {
  //   return;
  // }
  // get path for this value
  // string path = _parent->getArchiveFileList()[value];
  // get time for this path
  RadxTime pathTime;
  // NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  setGuiFromSelectedTime();
  // request data
  if (_frameIndex != value) {
    _frameIndex = value;
    // _parent->setArchiveRetrievalPending();
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
}

void TimeControl::_timeSliderPressed() 
{
  int value = _timeSlider->value();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
}

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

