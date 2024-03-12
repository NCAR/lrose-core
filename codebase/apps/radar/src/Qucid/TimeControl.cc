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
  _populate();
}

// destructor

TimeControl::~TimeControl()
  
{

}

//////////////////////////////////////////////
// populate the time panel

void TimeControl::_populate()
{
  
  setWindowTitle("Time and movie controller");
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
  _timeSlider->setFixedWidth(400);
  _timeSlider->setToolTip("Drag to change time selection");
  
  // active time
  
  // _selectedTimeLabel = new QLabel("yyyy/MM/dd hh:mm:ss", _timePanel);
  _selectedTimeLabel = new QPushButton(_timePanel);
  _selectedTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  QPalette pal = _selectedTimeLabel->palette();
  pal.setColor(QPalette::Active, QPalette::Button, Qt::cyan);
  _selectedTimeLabel->setPalette(pal);
  _selectedTimeLabel->setToolTip("This is the selected data time");

  // time editing

  _archiveScanIndex = 0;

  _archiveStartTimeEdit = new QDateTimeEdit(timeUpper);
  _archiveStartTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  QDate startDate(_archiveStartTime.getYear(), 
                  _archiveStartTime.getMonth(),
                  _archiveStartTime.getDay());
  QTime startTime(_archiveStartTime.getHour(),
                  _archiveStartTime.getMin(),
                  _archiveStartTime.getSec());
  QDateTime startDateTime(startDate, startTime);
  _archiveStartTimeEdit->setDateTime(startDateTime);
  _archiveStartTimeEdit->setCalendarPopup(true);
  connect(_archiveStartTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), 
          this, SLOT(_setArchiveStartTimeFromGui(const QDateTime &)));
  _archiveStartTimeEdit->setToolTip("Start time of archive period");
  
  _archiveEndTimeEdit = new QDateTimeEdit(timeUpper);
  _archiveEndTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  QDate endDate(_archiveEndTime.getYear(), 
                _archiveEndTime.getMonth(),
                _archiveEndTime.getDay());
  QTime endTime(_archiveEndTime.getHour(),
                _archiveEndTime.getMin(),
                _archiveEndTime.getSec());
  QDateTime endDateTime(endDate, endTime);
  _archiveEndTimeEdit->setDateTime(endDateTime);
  _archiveEndTimeEdit->setCalendarPopup(true);
  connect(_archiveEndTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), 
          this, SLOT(_setArchiveEndTimeFromGui(const QDateTime &)));
  _archiveEndTimeEdit->setToolTip("End time of archive period");
  
  // fwd and back buttons

  _back1 = new QPushButton(timeLower);
  _back1->setText("<");
  connect(_back1, SIGNAL(clicked()), this, SLOT(_goBack1()));
  _back1->setToolTip("Go back by 1 file");
  
  _fwd1 = new QPushButton(timeLower);
  _fwd1->setText(">");
  connect(_fwd1, SIGNAL(clicked()), this, SLOT(_goFwd1()));
  _fwd1->setToolTip("Go forward by 1 file");
    
  _backPeriod = new QPushButton(timeLower);
  _backPeriod->setText("<<");
  connect(_backPeriod, SIGNAL(clicked()), this, SLOT(_goBackPeriod()));
  _backPeriod->setToolTip("Go back by the archive time period");
  
  _fwdPeriod = new QPushButton(timeLower);
  _fwdPeriod->setText(">>");
  connect(_fwdPeriod, SIGNAL(clicked()), this, SLOT(_goFwdPeriod()));
  _fwdPeriod->setToolTip("Go forward by the archive time period");

  // accept cancel buttons

  QPushButton *acceptButton = new QPushButton(timeUpper);
  acceptButton->setText("Accept");
  QPalette acceptPalette = acceptButton->palette();
  acceptPalette.setColor(QPalette::Active, QPalette::Button, Qt::green);
  acceptButton->setPalette(acceptPalette);
  connect(acceptButton, SIGNAL(clicked()), this, SLOT(_acceptGuiTimes()));
  acceptButton->setToolTip("Accept the selected start and end times");

  QPushButton *cancelButton = new QPushButton(timeUpper);
  cancelButton->setText("Cancel");
  QPalette cancelPalette = cancelButton->palette();
  cancelPalette.setColor(QPalette::Active, QPalette::Button, Qt::red);
  cancelButton->setPalette(cancelPalette);
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(_cancelGuiTimes()));
  cancelButton->setToolTip("Cancel the selected start and end times");
    
  // add time widgets to layout
  
  int stretch = 0;
  timeUpperLayout->addWidget(cancelButton, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(_archiveStartTimeEdit, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(_selectedTimeLabel, stretch, Qt::AlignCenter);
  timeUpperLayout->addWidget(_archiveEndTimeEdit, stretch, Qt::AlignLeft);
  timeUpperLayout->addWidget(acceptButton, stretch, Qt::AlignLeft);
  
  timeLowerLayout->addWidget(_backPeriod, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_back1, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_timeSlider, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(_fwd1, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(_fwdPeriod, stretch, Qt::AlignLeft);

  // connect slots for time slider
  
  connect(_timeSlider, SIGNAL(actionTriggered(int)),
          this, SLOT(_timeSliderActionTriggered(int)));
  
  connect(_timeSlider, SIGNAL(valueChanged(int)),
          this, SLOT(_timeSliderValueChanged(int)));
  
  connect(_timeSlider, SIGNAL(sliderReleased()),
          this, SLOT(_timeSliderReleased()));
  
  connect(_timeSlider, SIGNAL(sliderPressed()),
          this, SLOT(_timeSliderPressed()));
  
}

////////////////////////////////////////////////////////
// set times from gui widgets

void TimeControl::setArchiveStartTime(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiStartTime.set(date.year(), date.month(), date.day(),
                    time.hour(), time.minute(), time.second());
}

void TimeControl::setArchiveEndTime(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiEndTime.set(date.year(), date.month(), date.day(),
                  time.hour(), time.minute(), time.second());
}

void TimeControl::acceptGuiTimes()
{
  _archiveStartTime = _guiStartTime;
  _archiveEndTime = _guiEndTime;
  _parent->loadArchiveFileList();
}

void TimeControl::cancelGuiTimes()
{
  setGuiFromArchiveStartTime();
  setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void TimeControl::setGuiFromArchiveStartTime()
{
  if (!_archiveStartTimeEdit) {
    return;
  }
  QDate date(_archiveStartTime.getYear(), 
             _archiveStartTime.getMonth(),
             _archiveStartTime.getDay());
  QTime time(_archiveStartTime.getHour(),
             _archiveStartTime.getMin(),
             _archiveStartTime.getSec());
  QDateTime datetime(date, time);
  _archiveStartTimeEdit->setDateTime(datetime);
  _guiStartTime = _archiveStartTime;
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void TimeControl::setGuiFromArchiveEndTime()
{
  if (!_archiveEndTimeEdit) {
    return;
  }
  QDate date(_archiveEndTime.getYear(), 
             _archiveEndTime.getMonth(),
             _archiveEndTime.getDay());
  QTime time(_archiveEndTime.getHour(),
             _archiveEndTime.getMin(),
             _archiveEndTime.getSec());
  QDateTime datetime(date, time);
  _archiveEndTimeEdit->setDateTime(datetime);
  _guiEndTime = _archiveEndTime;
}

////////////////////////////////////////////////////////
// set gui selected time label

void TimeControl::setGuiFromSelectedTime()
{
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

void TimeControl::setArchiveStartTime(const RadxTime &rtime)

{
  _archiveStartTime = rtime;
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  setGuiFromArchiveStartTime();
}

////////////////////////////////////////////////////////
// set archive end time

void TimeControl::setArchiveEndTime(const RadxTime &rtime)

{
  _archiveEndTime = rtime;
  if (!_archiveEndTime.isValid()) {
    _archiveEndTime.set(RadxTime::NOW);
  }
  setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// change start time
// return true if retrieval is pending, false otherwise

void TimeControl::goBack1()
{
  if (_archiveScanIndex > 0) {
    _archiveScanIndex -= 1;
    _parent->setArchiveRetrievalPending();
  } else {
    if (_params.debug) {
      cerr << "At start of data, cannot go back" << endl;
    }
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void TimeControl::goBackPeriod()
{
  int archiveSpanSecs = _archiveEndTime - _archiveStartTime;
  _archiveStartTime -= archiveSpanSecs;
  _archiveEndTime -= archiveSpanSecs;
  _parent->loadArchiveFileList();
  if (_archiveScanIndex > (int) _parent->getArchiveFileListSize() - 1) {
    _archiveScanIndex = (int) _parent->getArchiveFileListSize() - 1;
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void TimeControl::goFwd1()
{
  if (_archiveScanIndex < (int) _parent->getArchiveFileListSize() - 1) {
    _archiveScanIndex += 1;
    _parent->setArchiveRetrievalPending();
  } else {
    if (_params.debug) {
      cerr << "At end of data, cannot go forward" << endl;
    }
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void TimeControl::goFwdPeriod()
{
  int archiveSpanSecs = _archiveEndTime - _archiveStartTime;
  _archiveStartTime += archiveSpanSecs;
  _archiveEndTime += archiveSpanSecs;
  _parent->loadArchiveFileList();
  if (_archiveScanIndex > (int) _parent->getArchiveFileListSize() - 1) {
    _archiveScanIndex = (int) _parent->getArchiveFileListSize() - 1;
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
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
  if (value < 0 || value > (int) _parent->getArchiveFileListSize() - 1) {
    return;
  }
  // get path for this value
  string path = _parent->getArchiveFileList()[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
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
  if (value < 0 || value > (int) _parent->getArchiveFileListSize() - 1) {
    return;
  }
  // get path for this value
  string path = _parent->getArchiveFileList()[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  setGuiFromSelectedTime();
  // request data
  if (_archiveScanIndex != value) {
    _archiveScanIndex = value;
    _parent->setArchiveRetrievalPending();
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

