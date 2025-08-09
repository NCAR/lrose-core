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
// TimeNavView.hh
//
// interface to navigation through archive file names indexed by date and time
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////
//
// interface to navigate through the archive file names
// 
///////////////////////////////////////////////////////////////

#include "TimeNavView.hh"

#include <QMessageBox>
// #include <QCalendar>

TimeNavView::TimeNavView(QWidget *parent)
{

  _parent = parent;
  
  setWindowTitle("Time Navigation Archive Data");
  QPoint pos(0,0);
  move(pos);

  QBoxLayout *timeControlLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, this);
  timeControlLayout->setSpacing(0);

  // create time panel
  
  _timePanel = new QFrame(this);
  timeControlLayout->addWidget(_timePanel, Qt::AlignCenter);
  _timeLayout = new QVBoxLayout;
  _timePanel->setLayout(_timeLayout);

  QFrame *timeUpper = new QFrame(_timePanel);
  QHBoxLayout *timeUpperLayout = new QHBoxLayout;
  timeUpperLayout->setSpacing(10);
  timeUpper->setLayout(timeUpperLayout);
  
  QFrame *timeLower = new QFrame(_timePanel);
  QHBoxLayout *timeLowerLayout = new QHBoxLayout;
  timeLowerLayout->setSpacing(10);
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

  _archiveStartTimeEdit = new QDateTimeEdit(timeUpper);
  _archiveStartTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  //QDate startDate(_archiveStartTime.getYear(); 
  //                _archiveStartTime.getMonth();
  //                _archiveStartTime.getDay());
  //QTime startTime(_archiveStartTime.getHour();
  //                _archiveStartTime.getMin();
  //                _archiveStartTime.getSec());
  //QDateTime startDateTime(startDate; startTime);
  //_archiveStartTimeEdit->setDateTime(startDateTime);
  //connect(_archiveStartTimeEdit; SIGNAL(dateTimeChanged(const QDateTime &)); 
  //        this; SLOT(_setArchiveStartTimeFromGui(const QDateTime &)));
  _archiveStartTimeEdit->setToolTip("Start time of archive period");
  
  _archiveEndTimeEdit = new QDateTimeEdit(timeUpper);
  _archiveEndTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  //QDate endDate(_archiveEndTime.getYear(); 
  //               _archiveEndTime.getMonth();
  //               _archiveEndTime.getDay());
  //QTime endTime(_archiveEndTime.getHour();
  //               _archiveEndTime.getMin();
  //               _archiveEndTime.getSec());
  //QDateTime endDateTime(endDate; endTime);
  //_archiveEndTimeEdit->setDateTime(endDateTime);
  //connect(_archiveEndTimeEdit; SIGNAL(dateTimeChanged(const QDateTime &)); 
  //        this; SLOT(_setArchiveEndTimeFromGui(const QDateTime &)));
  _archiveEndTimeEdit->setToolTip("End time of archive period");
  
  // fwd and back buttons

  _back1 = new QPushButton(timeLower);
  _back1->setText("<");
  connect(_back1, &QPushButton::clicked, this, &TimeNavView::_goBack1);
  _back1->setToolTip("Go back by 1 file");
  
  _fwd1 = new QPushButton(timeLower);
  _fwd1->setText(">");
  connect(_fwd1, &QPushButton::clicked, this, &TimeNavView::_goFwd1);
  _fwd1->setToolTip("Go forward by 1 file");
    
  _backPeriod = new QPushButton(timeLower);
  _backPeriod->setText("<<");
  connect(_backPeriod, &QPushButton::clicked, this, &TimeNavView::_goBackPeriod);
  _backPeriod->setToolTip("Go back by quarter time period");
  
  _fwdPeriod = new QPushButton(timeLower);
  _fwdPeriod->setText(">>");
  connect(_fwdPeriod, &QPushButton::clicked, this, &TimeNavView::_goFwdPeriod);
  _fwdPeriod->setToolTip("Go forward by quarter time period");

  // accept cancel buttons

  QPushButton *acceptButton = new QPushButton(timeUpper);
  acceptButton->setText("Accept");
  QPalette acceptPalette = acceptButton->palette();
  acceptPalette.setColor(QPalette::Active, QPalette::Button, Qt::green);
  acceptButton->setPalette(acceptPalette);
  connect(acceptButton, &QPushButton::clicked, this, &TimeNavView::acceptGuiTimes);
  acceptButton->setToolTip("Accept the selected start and end times");

  QPushButton *cancelButton = new QPushButton(timeUpper);
  cancelButton->setText("Cancel");
  QPalette cancelPalette = cancelButton->palette();
  cancelPalette.setColor(QPalette::Active, QPalette::Button, Qt::red);
  cancelButton->setPalette(cancelPalette);
  connect(cancelButton, &QPushButton::clicked, this, &TimeNavView::cancelGuiTimes);
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


  _timeSlider->setTracking(true);

  // connect slots for time slider
  
  //connect(_timeSlider; SIGNAL(actionTriggered(int));
  //        this; SLOT(_timeSliderActionTriggered(int)));
  
  connect(_timeSlider, &QSlider::valueChanged,
          this, &TimeNavView::timeSliderReleased);
  
  connect(_timeSlider, &QSlider::sliderReleased,
          this, &TimeNavView::timeSliderReleased);

  connect(_timeSlider, &QSlider::sliderPressed,
          this, &TimeNavView::timeSliderPressed);

}

/*
void TimeNavView::update(
  //vector<string> &fileList; 
  string startTime; string endTime; int nFiles
  //; bool archiveFilesHaveDayDir
   {
*/

/*
  if (archiveFilesHaveDayDir) {
    _archiveStartTimeEdit->setEnabled(true);
    _archiveEndTimeEdit->setEnabled(true);
    _backPeriod->setEnabled(true);
    _fwdPeriod->setEnabled(true);
  } else {
    _archiveStartTimeEdit->setEnabled(false);
    _archiveEndTimeEdit->setEnabled(false);
    _backPeriod->setEnabled(false);
    _fwdPeriod->setEnabled(false);
  }
*/
/*
  _setGuiFromArchiveStartTime(startTime);
  _setGuiFromArchiveEndTime(endTime);


}
*/

void TimeNavView::setNTicks(int nFiles) {

  if (_timeSlider) {
    _timeSlider->setMinimum(0);
    if (nFiles <= 1)
      _timeSlider->setMaximum(1);
    else
      _timeSlider->setMaximum(nFiles - 1);
    //_timeSlider->setSliderPosition(_archiveScanIndex);
  }


}

void TimeNavView::setSliderPositionNoRead(int value) {
  _timeSlider->setValue(value);
}

void TimeNavView::setSliderPosition(int value) {
  _timeSlider->setValue(value);
  timeSliderReleased();
  // note: this triggers the notifier signal: valueChanged
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void TimeNavView::setGuiFromArchiveStartTime(int year, int month, int day,
  int hour, int minute, int seconds)
{
  //if (!_archiveStartTimeEdit) {
  //  return;
  //}
  // QDate::QDate(int y, int m, int d)
  // 
  // QCalendar calendar;
  // QDate date = calendar.dateFromParts(year, month, day);
  QDate date(year, month, day);
  QTime time(hour, minute, seconds);
  QDateTime datetime(date, time);
  _archiveStartTimeEdit->setDateTime(datetime);
  //  setVisible(true);
  // raise();
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void TimeNavView::setGuiFromArchiveEndTime(int year, int month, int day,
  int hour, int minute, int seconds)
{

  // QDate::QDate(int y; int m, int d)
  QDate date(year, 
             month,
             day);
  QTime time(hour,
             minute,
             seconds);
  QDateTime datetime(date, time);
  _archiveEndTimeEdit->setDateTime(datetime);  

}

////////////////////////////////////////////////////////
// set gui selected time label


void TimeNavView::setGuiFromSelectedTime(int year, int month, int day,
  int hour, int minute, int seconds)
{
  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           year,
           month,
           day,
           hour,
           minute,
           seconds);
  _selectedTimeLabel->setText(text);
}


void TimeNavView::timeSliderValueChanged(int value) 
{

  //emit timeSliderMoved(value);
  /*
  if (value < 0 || value > (int) _archiveFileList.size() - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path; pathTime);
  // set selected time
  _selectedTime = pathTime;
  _setGuiFromSelectedTime();
  //if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider changed; value: " << value << endl;
  //}
  */
}

void TimeNavView::timeSliderReleased() 
{
  int value = _timeSlider->value();

  emit newTimeIndexSelected(value);
  /* move to controller ...
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
    _setArchiveRetrievalPending();
  }
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released; value: " << value << endl;
  }
  */
}

void TimeNavView::timeSliderPressed() 
{
  // int value = _timeSlider->value();
  //if (_params->debug >= Params::DEBUG_VERBOSE) {
    //cerr << "Time slider released; value: " << value << endl;
  //}
}

void TimeNavView::_goBack1() {
  int value = _timeSlider->value();
  value = value - 1;
  if (value >= 0)
    setSliderPosition(value);  
  else 
    errorMessage("Note", "at the beginning of archive");
}

void TimeNavView::_goFwd1() {
  int value = _timeSlider->value();
  value = value + 1;
  if (value <= _timeSlider->maximum())
    setSliderPosition(value);  
  else  
    errorMessage("Note", "at the end of archive");
}

void TimeNavView::_goBackPeriod() {
  int value = _timeSlider->value();
  value = value - _timeSlider->maximum()/4;
  if (value < 0)
    value = 0;
  setSliderPosition(value);
}

void TimeNavView::_goFwdPeriod() {
  int value = _timeSlider->value();
  value = value + _timeSlider->maximum()/4;
  int max = _timeSlider->maximum();
  if (value > max)
    value = max;
  setSliderPosition(value); 
}

///////////////////////////////////////////////////////////////////////////////
// print time slider actions for debugging

void TimeNavView::timeSliderActionTriggered(int action) {
  /*
  if (_params->debug >= Params::DEBUG_VERBOSE) {
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
    cerr << "timeSliderActionTriggered; value: " << _timeSlider->value() << endl;
  }
  */
} 


/////////////////////////////////////
// show the time controller dialog

void TimeNavView::showTimeControl()
{
/*
  if (_timeControl) {
    if (_timeControl->isVisible()) {
      _timeControl->setVisible(false);
    } else {
      if (!_timeControlPlaced) {
        _timeControl->setVisible(true);
        QPoint pos;
        pos.setX(x() + 
                 (frameGeometry().width() / 2) -
                 (_timeControl->width() / 2));
        pos.setY(y() + frameGeometry().height());
        _timeControl->move(pos);
      }
      _timeControl->setVisible(true);
      _timeControl->raise();
    }
  }
  */


    if (isVisible()) {
      raise();
      //setVisible(false);
    } else {
      //if (!_timeControlPlaced) {
      // always place the time navigation view
        setVisible(true);
        QPoint pos;
        pos.setX(x()); // + 
                 //(frameGeometry().width() / 2) -
                 //(width() / 2));
        pos.setY(y() + frameGeometry().height());
        move(pos);
      //}
      
      setVisible(true);
      raise();
    }
  

}


/////////////////////////////////////
// place the time controller dialog

/// TODO: remove?  not used?
void TimeNavView::placeTimeControl()
{


    //if (!_timeControlPlaced) {
      int topFrameWidth = geometry().y() - y();
      QPoint pos;
      pos.setX(x() + 
               (frameGeometry().width() / 2) -
               (width() / 2));
      pos.setY(y() + frameGeometry().height() + topFrameWidth);
      move(pos);
      //_timeControlPlaced = true;
    //}
  
}


////////////////////////////////////////////////////////
// set times from gui widgets

/*
void TimeNavView::_setArchiveStartTimeFromGui(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiStartTime.set(date.year(); date.month(); date.day(),
                    time.hour(), time.minute(), time.second());
}

void TimeNavView::_setArchiveEndTimeFromGui(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiEndTime.set(date.year(), date.month(), date.day(),
                  time.hour(), time.minute(), time.second());
}
*/

void TimeNavView::acceptGuiTimes()
{
  //_archiveStartTime = _guiStartTime;
  //_archiveEndTime = _guiEndTime;
  //loadArchiveFileList();
  QDateTime startDateTime = _archiveStartTimeEdit->dateTime();
  QDateTime endDateTime = _archiveEndTimeEdit->dateTime();

  QDate startDate = startDateTime.date();
  QTime startTime = startDateTime.time();
  QDate endDate = endDateTime.date();
  QTime endTime = endDateTime.time();

  int startYear = startDate.year(); 
  int startMonth = startDate.month(); 
  int startDay = startDate.day();                    
  int startHour = startTime.hour(); 
  int startMinute = startTime.minute(); 
  int startSecond = startTime.second();

  int endYear = endDate.year(); 
  int endMonth = endDate.month(); 
  int endDay = endDate.day();
  int endHour = endTime.hour(); 
  int endMinute = endTime.minute(); 
  int endSecond = endTime.second();

  emit newStartEndTime(startYear, startMonth, startDay,
                       startHour, startMinute, startSecond,
                       endYear, endMonth, endDay,
                       endHour, endMinute, endSecond);
}

void TimeNavView::cancelGuiTimes()
{
  //_setGuiFromArchiveStartTime();
  //_setGuiFromArchiveEndTime();
  emit resetStartEndTime();
}


void TimeNavView::errorMessage(string title, string message) {
  QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(message));
}

