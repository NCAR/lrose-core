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
// AScopeManager.hh
//
// AScopeManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// AScopeManager manages BSCAN data - vert pointing etc
// Rendering is delegated to BscanWidget
//
///////////////////////////////////////////////////////////////

#ifndef AScopeManager_HH
#define AScopeManager_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "TsReader.hh"
#include <QMainWindow>
#include <Radx/RadxTime.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxVol.hh>

class QCheckBox;
class QPushButton;
class QDateTimeEdit;
class QDateTime;
class QComboBox;
class BscanWidget;

class AScopeManager {
  
  Q_OBJECT
  
public:

  // constructor

  AScopeManager(const Params &params,
                TsReader *reader);
  
  // destructor
  
  ~AScopeManager();

  // run 
  
  int run(QApplication &app);

  // enable the zoom button - called by AscopeWidget
  
  virtual void enableZoomButton() const;
  
signals:

private:

  const Params &_params;
  
  // reading data in
  
  TsReader *_reader;
  
  // beam reading timer

  static bool _firstTimerEvent;
  int _dataTimerId;
  bool _frozen;

  // windows

  QFrame *_main;

  // actions
  
  QAction *_exitAct;
  QAction *_freezeAct;
  QAction *_clearAct;
  QAction *_unzoomAct;
  QAction *_refreshAct;
  QAction *_showClickAct;
  QAction *_howtoAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;
  QAction *_openFileAct;

  // status panel

  QGroupBox *_statusPanel;
  QGridLayout *_statusLayout;

  QLabel *_radarName;
  QLabel *_dateVal;
  QLabel *_timeVal;

  QLabel *_volNumVal;
  QLabel *_sweepNumVal;

  QLabel *_fixedAngVal;
  QLabel *_elevVal;
  QLabel *_azVal;

  QLabel *_nSamplesVal;
  QLabel *_nGatesVal;
  QLabel *_gateSpacingVal;

  QLabel *_pulseWidthVal;
  QLabel *_prfVal;
  QLabel *_nyquistVal;
  QLabel *_maxRangeVal;
  QLabel *_unambigRangeVal;
  QLabel *_powerHVal;
  QLabel *_powerVVal;

  QLabel *_scanNameVal;
  QLabel *_sweepModeVal;
  QLabel *_polModeVal;
  QLabel *_prfModeVal;

  QLabel *_latVal;
  QLabel *_lonVal;

  QLabel *_altVal;
  QLabel *_altLabel;

  QLabel *_altRateVal;
  QLabel *_altRateLabel;
  double _prevAltKm;
  RadxTime _prevAltTime;
  double _altRateMps;

  QLabel *_speedVal;
  QLabel *_headingVal;
  QLabel *_trackVal;

  QLabel *_sunElVal;
  QLabel *_sunAzVal;

  bool _altitudeInFeet;

  vector<QLabel *> _valsRight;
  
  // click location report dialog

  QDialog *_clickReportDialog;
  QGridLayout *_clickReportDialogLayout;
  QLabel *_dateClicked;
  QLabel *_timeClicked;
  QLabel *_elevClicked;
  QLabel *_azClicked;
  QLabel *_gateNumClicked;
  QLabel *_rangeClicked;
  QLabel *_altitudeClicked;

  // sun position calculator
  
  double _radarLat, _radarLon, _radarAltKm;
  SunPosn _sunPosn;

  // input data
  
  bool _plotStart;
  RadxTime _readerRayTime;

  // windows

  QFrame *_bscanFrame;
  AscopeWidget *_widget;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  
  // menus

  QMenu *_fileMenu;
  QMenu *_configMenu;
  QMenu *_overlaysMenu;
  QMenu *_actionsMenu;
  QMenu *_vertMenu;
  QMenu *_helpMenu;

  // actions

  QAction *_rangeGridAct;
  QAction *_timeGridAct;
  QAction *_instHtLineAct;

  QAction *_rangeAxisAct;
  QAction *_timeAxisAct;
  QAction *_saveImageAct;

  QAction *_latlonLegendAct;
  QAction *_speedTrackLegendAct;
  QAction *_distScaleAct;

  // range axis settings dialog

  QDialog *_rangeAxisDialog;
  QGroupBox *_rangeAxisModeBox;
  QGroupBox *_rangeAxisAltitudeBox;
  QGroupBox *_rangeAxisRangeBox;
  QGroupBox *_rangeAxisDoneBox;
  
  QRadioButton *_rangeAxisModeUpButton;
  QRadioButton *_rangeAxisModeDownButton;
  QRadioButton *_rangeAxisModeAltitudeButton;

  Params::range_axis_mode_t _rangeAxisMode;
  Params::range_axis_mode_t _requestedRangeAxisMode;

  // time axis settings dialog
  
  QDialog *_timeAxisDialog;
  QLabel *_timeAxisInfo;

  QLineEdit *_timeSpanEdit;
  double _timeSpanSecs;

  bool _archiveMode;
  bool _archiveRetrievalPending;
  QRadioButton *_realtimeModeButton;
  QRadioButton *_archiveModeButton;

  QGroupBox *_archiveTimeBox;
  QDateTimeEdit *_archiveStartTimeEdit;
  RadxTime _archiveStartTime;

  QLabel *_archiveEndTimeEcho;
  RadxTime _archiveEndTime;
  
  RadxTime _archiveImagesStartTime;
  RadxTime _archiveImagesEndTime;

  QGroupBox *_dwellSpecsBox;
  QCheckBox *_dwellAutoBox;
  QLabel *_dwellAutoVal;
  bool _dwellAuto;
  double _dwellAutoSecs;
  double _dwellSpecifiedSecs;
  double _dwellSecs;
  QLineEdit *_dwellSpecifiedEdit;
  QFrame *_dwellSpecifiedFrame;
  QComboBox *_dwellStatsComboBox;
  RadxField::StatsMethod_t _dwellStatsMethod;

  double _xSecsClicked;
  double _yKmClicked;

  // set top bar

  virtual void _setTitleBar(const string &radarName) = 0;

  // panels
  
  void _createStatusPanel();
  void _createClickReportDialog();
  void _updateStatusPanel(const RadxRay *ray);
  double _getInstHtKm(const RadxRay *ray);

  // setting text

  void _setText(char *text, const char *format, int val);
  void _setText(char *text, const char *format, double val);
  
  // adding vals / labels

  QLabel *_newLabelRight(const string &text);

  QLabel *_createStatusVal(const string &leftLabel,
                           const string &rightLabel,
                           int row, 
                           int fontSize,
                           QLabel **label = NULL);
  
  QLabel *_addLabelRow(QWidget *widget,
                       QGridLayout *layout,
                       const string &leftLabel,
                       const string &rightLabel,
                       int row,
                       int fontSize = 0);

  QLineEdit *_addInputRow(QWidget *widget,
                          QVBoxLayout *layout,
                          const string &leftLabel,
                          const string &rightContent,
                          int fontSize = 0,
                          QLabel **label = NULL);

  QLineEdit *_addInputRow(QWidget *parent,
                          QVBoxLayout *layout,
                          const string &leftLabel,
                          const string &rightContent,
                          int fontSize = 0,
                          QFrame **framePtr = NULL);


  // override event handling
  
  virtual void timerEvent (QTimerEvent * event);
  virtual void resizeEvent (QResizeEvent * event);
  virtual void keyPressEvent(QKeyEvent* event);
  
  // set top bar

  virtual void _setTitleBar(const string &radarName);
  
  // local methods
  
  void _setupWindows();
  void _createMenus();
  void _createActions();
  void _initActions();

  void _configureAxes();

  // retrieve data
  
  void _handleRealtimeData();
  void _handleRealtimeDataForImages();
  void _handleArchiveData();
  int _getArchiveData();
  void _plotArchiveData();
  void _setupVolRead(RadxFile &file);
  void _setDwellAutoVal();

private slots:

  //////////////
  // Qt slots //
  //////////////

  virtual void _howto();
  void _about();
  void _showClick();
  virtual void _openFile();
  
  virtual void _freeze();
  virtual void _unzoom();
  virtual void _refresh();
  
  // local

  void _locationClicked(double xkm, double ykm, const RadxRay *closestRay);

  void _setTimeSpan();
  void _resetTimeSpanToDefault();
  
  void _setStartTimeFromGui(const QDateTime &datetime1);
  void _setGuiFromStartTime();
  void _setArchiveStartTimeToDefault();
  void _setArchiveStartTime(const RadxTime &rtime);
  void _setArchiveEndTime();
  void _setDataRetrievalMode();
  void _goBack1();
  void _goFwd1();
  void _goBack5();
  void _goFwd5();
  void _changeRange(int deltaGates);

  void _performArchiveRetrieval();
  
  void _setDwellToDefaults();
  void _setDwellSpecified();
  void _resetDwellSpecifiedToDefault();
  void _setDwellAuto();
  void _setDwellStats();

  // override howto

  void _howto();

};

#endif

