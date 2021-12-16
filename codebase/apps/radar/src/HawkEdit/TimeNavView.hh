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
// Widget to navigate the time dimension
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2021
//
///////////////////////////////////////////////////////////////
//
//
///////////////////////////////////////////////////////////////

#ifndef TimeNavView_HH
#define TimeNavView_HH

#include <vector>
#include <string>
#include <QListWidgetItem>
#include <QStringList>
#include <QDialog>
#include <QDateTime>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDateTimeEdit>

/*
class QActionGroup;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class QListWidget;
class QFrame;

class QLabel;
class QGroupBox;
class QGridLayout;
class QDateTime;
class QDateTimeEdit;
*/

using namespace std;

class TimeNavView : public QDialog {

  Q_OBJECT

public:

  TimeNavView(QWidget *parent);

  int _nArchiveScans;
  vector<string> _archiveFileList;
  int _archiveScanIndex;
  bool _archiveFilesHaveDayDir;

  void setNTicks(int nFiles);
  void setSliderPosition(int value);
  void setGuiFromArchiveStartTime(int year, int month, int day,
    int hour, int minute, int seconds);
  void setGuiFromArchiveEndTime(int year, int month, int day,
    int hour, int minute, int seconds);
  void setGuiFromSelectedTime(int year, int month, int day, int hour,
    int minute, int seconds);

  void timeSliderValueChanged(int value);
  void timeSliderReleased();
  void timeSliderPressed();
  void timeSliderActionTriggered(int action);

  void placeTimeControl();
  void showTimeControl();

signals:

  void newTimeIndexSelected(int value);
  void timeSliderMoved(int value);

public slots:

  void acceptGuiTimes();
  void cancelGuiTimes();

private:

  QWidget *_parent;
  
  // time slider
  QDialog *_timeControl;
  QFrame *_timePanel;
  //QVBoxLayout *_timeLayout;

  QSlider *_timeSlider;

  QDateTimeEdit *_archiveStartTimeEdit;
  
  QDateTimeEdit *_archiveEndTimeEdit;

  QPushButton *_selectedTimeLabel;

  QPushButton *_back1;
  QPushButton *_fwd1;
  QPushButton *_backPeriod;
  QPushButton *_fwdPeriod;

  QVBoxLayout *_timeLayout;

};

#endif