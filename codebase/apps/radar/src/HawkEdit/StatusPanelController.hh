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
// StatusPanelController.hh
//
// StatusPanel object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// StatusPanel manages the display of metadata for a selected
// (file, sweep, ray, range) = (time, elevation, azimuth, range)
//
///////////////////////////////////////////////////////////////

#ifndef StatusPanelController_HH
#define StatusPanelController_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
//#include "DisplayManager.hh"
#include "RayLocationController.hh"
#include "ContextEditingView.hh"
#include "ClickableLabel.hh"
#include "ParameterColorView.hh"
//#include "BatchFileSaveView.hh"
#include "FieldColorController.hh"
#include "SweepView.hh"
#include "SweepController.hh"
#include "DisplayFieldView.hh"
#include "SpreadSheetController.hh"
#include "SpreadSheetView.hh"
#include "ScriptEditorController.hh"
#include "ScriptEditorView.hh"
#include "BoundaryPointEditor.hh"
#include "BoundaryPointEditorView.hh"
#include "TimeNavView.hh"
#include "TimeNavController.hh"
#include "UndoRedoController.hh"
#include <QMainWindow>
#include <QListWidgetItem>
#include <QStringList>
#include <QCheckBox>
#include <euclid/SunPosn.hh>
#include <Radx/RadxRay.hh>

class QApplication;
class QActionGroup;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class QProgressDialog;
class QListWidget;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class QDateTime;
class QDateTimeEdit;
class QFileDialog;

class DisplayField;
class PpiWidget;
class RhiWidget;
class RhiWindow;
class Reader;
class RadxPlatform;
class TimeScaleWidget;

class StatusPanelController : public QMainWindow {
  
  Q_OBJECT

public:

  // constructor
  
  StatusPanelController(DisplayFieldController *displayFieldController,
	       //               const vector<DisplayField *> &fields,
               bool haveFilteredFields, bool interactiv = true);
  
  // destructor
  
  ~StatusPanelController();

  // override event handling

  //void timerEvent (QTimerEvent * event);
  void resizeEvent (QResizeEvent * event);

  // location

  double getRadarLat() const { return _radarLat; }
  double getRadarLon() const { return _radarLon; }
  double getRadarAltKm() const { return _radarAltKm; }
  const RadxPlatform &getPlatform() const { return _platform; }

  bool evaluateCursor(bool isShiftKeyDown);

  bool evaluateRange(double xRange);


  void closeEvent(QEvent *event);

public slots:
 
  void setDataMissing(string fieldName, float missingValue);

  void selectedFieldChanged(QString newFieldName);
  void selectedFieldChanged(string fieldName);
  //void _updateField(size_t fieldId);

  void selectedSweepChanged(int sweepNumber);


signals:

  ////////////////
  // Qt signals //
  ////////////////

  
  void frameResized(const int width, const int height);
  //void setParamsFile();

  void addField(QString fieldName);

  void newSweepData(int sweepNumber);

private:

  // from DisplayManager ...
  ParamFile *_params;
  
  // instrument platform details 

  RadxPlatform _platform;

  // data fields
  //  vector <DisplayField *> _fields;
  DisplayFieldController *_displayFieldController;
  bool _haveFilteredFields;
  int _rowOffset;

  // windows

  QFrame *_main;

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

  QLabel *_georefsApplied;
  QLabel *_geoRefRotationVal;
  QLabel *_geoRefRollVal;
  QLabel *_geoRefTiltVal;
  QLabel *_cfacRotationVal;
  QLabel *_cfacRollVal;
  QLabel *_cfacTiltVal;

  QLabel *_geoRefTrackRelRotationVal;
  QLabel *_geoRefTrackRelAzVal;
  QLabel *_geoRefTrackRelTiltVal;
  QLabel *_geoRefTrackRelElVal;
      

  QLabel *_georefsAppliedLabel;
  QLabel *_geoRefRotationLabel;
  QLabel *_geoRefRollLabel;
  QLabel *_geoRefTiltLabel;
  QLabel *_cfacRotationLabel;
  QLabel *_cfacRollLabel;
  QLabel *_cfacTiltLabel;  

  QLabel *_geoRefTrackRelRotationLabel;
  QLabel *_geoRefTrackRelAzLabel;
  QLabel *_geoRefTrackRelElLabel;
  QLabel *_geoRefTrackRelTiltLabel;  
  

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

  QCheckBox *_applyCfacToggle;
  
  // sun position calculator
  double _radarLat, _radarLon, _radarAltKm;
  SunPosn _sunPosn;

  // set top bar

  //virtual void _setTitleBar(const string &radarName) = 0;

  // panels
  
  void _createStatusPanel();
  void _createFieldPanel();
  void _updateFieldPanel(string newFieldName);

  void _createClickReportDialog();
  void _updateStatusPanel(const RadxRay *ray);
  double _getInstHtKm(const RadxRay *ray);

  void _applyCfac();
  void hideCfacs();

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

  bool _firstTime;

  // beam geometry
  
  int _nGates;
  double _maxRangeKm;

  // windows

  QFrame *_ppiFrame;
  PolarWidget *_ppi;

  RhiWindow *_rhiWindow;
  RhiWidget *_rhi;
  bool _rhiWindowDisplayed;
  bool _rhiMode;
  
  // azimuths for current ray

  double _prevAz;
  double _prevEl;
  double _startAz;
  double _endAz;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  
  // actions

  QAction *_realtimeAct;
  QAction *_showTimeControlAct;
  QAction *_ringsAct;
  QAction *_gridsAct;
  QAction *_azLinesAct;
  QAction *_showRhiAct;
  
  QAction *_timeControllerAct;
  QAction *_openFileAct;
  QAction *_saveFileAct;
  QAction *_saveImageAct;

  QAction *_examineAct;
  QAction *_editAct;

  // archive mode
  
  bool _archiveMode;
  bool _archiveRetrievalPending;

  bool _cancelled;
  bool _batchEditing;


  //////////////////////////////
  // private methods

  // set top bar

  void _setTitleBar(const string &radarName);
  
  // local methods

  //void _clear();
  void _setupWindows();
  void _createActions();
  void _createMenus();

private slots:

  //////////////
  // Qt slots //
  //////////////

  void _refresh();

};

#endif

