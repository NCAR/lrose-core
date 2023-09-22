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
// SpriteMgr.hh
//
// SpriteMgr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// SpriteMgr manages BSCAN data - vert pointing etc
// Rendering is delegated to BscanWidget
//
///////////////////////////////////////////////////////////////

#ifndef SpriteMgr_HH
#define SpriteMgr_HH

#include <string>
#include <vector>
#include <deque>

#include "Args.hh"
#include "Params.hh"

#include <QMainWindow>

#include <toolsa/DateTime.hh>
#include <euclid/SunPosn.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxVol.hh>
#include <radar/ClickPointFmq.hh>

class TsReader;
class SpriteWidget;
class Beam;

class QApplication;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDateTime;
class QDateTimeEdit;
class QDialog;
class QFrame;
class QGridLayout;
class QGroupBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSlider;
class QVBoxLayout;
class QWidget;

class SpriteMgr : public QMainWindow {
  
  Q_OBJECT
  
public:

  // constructor

  SpriteMgr(const Params &params,
             TsReader *tsReader);
  
  // destructor
  
  ~SpriteMgr();

  // run 
  
  int run(QApplication &app);

  // enable the unzoom button - called by SpriteWidget
  
  virtual void enableUnzoomButton() const;

  // get methods

  string getWindowTitle() const { return _windowTitle; }

signals:

  ////////////////
  // Qt signals //
  ////////////////

  /**
   * @brief Signal emitted when the main frame is resized.
   *
   * @param[in] width    The new width of the frame.
   * @param[in] height   The new height of the frame.
   */
  
  void frameResized(const int width, const int height);
  
private:

  const Params &_params;
  
  // reading data in
  
  TsReader *_tsReader;
  bool _goForward;

  // beam reading timer

  static bool _firstTimerEvent;
  int _dataTimerId;
  bool _frozen;

  // windows

  QFrame *_main;
  string _windowTitle;
  
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

  QLabel *_sunElVal;
  QLabel *_sunAzVal;

  // bool _altitudeInFeet;

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
  
  double _radarLat, _radarLon, _radarAltM;
  SunPosn _sunPosn;

  // input data
  
  bool _plotStart;
  RadxTime _readerRayTime;

  // windows

  QFrame *_mainFrame;
  SpriteWidget *_widget;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  
  // menus

  QMenu *_fileMenu;
  QMenu *_configMenu;
  QMenu *_overlaysMenu;
  QMenu *_actionsMenu;
  QMenu *_helpMenu;

  // actions

  QAction *_xGridAct;
  QAction *_yGridAct;
  QAction *_legendsAct;

  QAction *_xAxisAct;
  QAction *_yAxisAct;

  double _timeSpanSecs;

  bool _archiveMode;
  bool _archiveRetrievalPending;

  RadxTime _archiveStartTime;

  RadxTime _archiveEndTime;
  
  double _xLocClicked;
  double _yLocClicked;

  // deque for beam memory

  deque<Beam *> _beamQueue;

  // user click Xml FMQ - from HawkEye
  
  ClickPointFmq _clickPointFmq;
  time_t _clickPointTimeSecs;
  int _clickPointNanoSecs;
  DateTime _clickPointTime;
  double _clickPointElevation;
  double _clickPointAzimuth;
  double _clickPointRangeKm;
  int _clickPointGateNum;

  // set top bar

  virtual void _setTitleBar(const string &radarName);

  // panels
  
  void _createStatusPanel();
  void _updateStatusPanel(const Beam *beam);

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
  
  // local methods
  
  void _setupWindows();
  void _createMenus();
  void _createActions();
  void _initActions();

  void _configureAxes();

  // retrieve data
  
  void _handleRealtimeData();
  void _handleArchiveData();
  void _followDisplay();

  int _readClickPointFmq(bool &gotNew);
  int _writeClickPointXml2Fmq();

private slots:

  // Qt slots //

  virtual void _howto();
  void _about();
  void _showClick();
  virtual void _openFile();
  
  virtual void _freeze();
  virtual void _unzoom();
  virtual void _refresh();
  
  void _manageBeamQueue(Beam *beam);

  void _clickPointChanged();

  // local slots

  void _widgetLocationClicked(double selectedRangeKm, int selectedGateNum);

  void _goBack();
  void _goFwd();
  void _changeRange(int deltaGates);
  
  void _performArchiveRetrieval();
  
};

#endif

