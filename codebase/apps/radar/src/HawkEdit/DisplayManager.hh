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
// DisplayManager.hh
//
// DisplayManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
///////////////////////////////////////////////////////////////
//
// Virtual base class for BscanManager and PolarManager
//
///////////////////////////////////////////////////////////////

#ifndef DisplayManager_HH
#define DisplayManager_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "ColorMap.hh"
#include <QMainWindow>
#include <euclid/SunPosn.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxPlatform.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxGeoref.hh>
#include "SweepManager.hh"
#include "DisplayFieldController.hh"

class QApplication;
class QButtonGroup;
class QRadioButton;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class QSlider;
class QWidget;

class DisplayField;
class Reader;

class DisplayManager : public QMainWindow {
  
  Q_OBJECT

public:

  // constructor
  
  DisplayManager(const Params &params,
                 Reader *reader,
		 DisplayFieldController *displayFieldController,
                 bool haveFilteredFields);

  /*
  DisplayManager(const Params &params,
                 Reader *reader,
		 vector<DisplayField *> &fields,
                 bool haveFilteredFields);
  */
  // destructor
  
  ~DisplayManager();

  // run 

  virtual int run(QApplication &app) = 0;
  
  // get selected name and units

  const string &getSelectedFieldLabel() const { return _selectedLabel; }
  const string &getSelectedFieldName() const { return _selectedName; }
  const string &getSelectedFieldUnits() const { return _selectedUnits; }
  // const DisplayField &getSelectedField() const { return _displayFieldController->getField(_fieldNum); }
  // const vector<DisplayField *> &getDisplayFields() const { return _fields; }
  //  const DisplayField &getSelectedField() const { return *_fields[_fieldNum]; }
  //  const vector<DisplayField *> &getDisplayFields() const { return _fields; }

  // location

  double getRadarLat() const { return _radarLat; }
  double getRadarLon() const { return _radarLon; }
  double getRadarAltKm() const { return _radarAltKm; }
  const RadxPlatform &getPlatform() const { return _platform; }

  // enable the zoom button
  
  virtual void enableZoomButton() const = 0;

  virtual double getSelectedSweepAngle() {return 0.0;}
  virtual size_t getSelectedFieldIndex() {return 0;}

public slots:

  void colorMapRedefineReceived(string fieldName, ColorMap newColorMap);


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

  
protected:
  
  const Params &_params;
  
  // reading data in
  
  Reader *_reader;
  vector<const RadxRay *> _rays;
  bool _initialRay;
  
  // instrument platform details 

  RadxPlatform _platform;
  
  // beam reading timer

  static bool _firstTimerEvent;
  int _beamTimerId;
  bool _frozen;

  // data fields
  //  vector <DisplayField *> _fields;
  DisplayFieldController *_displayFieldController;
  bool _haveFilteredFields;
  int _rowOffset;

  // windows

  QFrame *_main;

  // actions
  
  QAction *_exitAct;
  QAction *_freezeAct;
  QAction *_clearAct;
  QAction *_unzoomAct;
  QAction *_refreshAct;
  QAction *_showClickAct;
  QAction *_showBoundaryEditorAct;
  QAction *_howtoAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;
  QAction *_openFileAct;
  QAction *_saveFileAct;

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
  
  // field panel
  
  QGroupBox *_fieldPanel;
  QGridLayout *_fieldsLayout;
  QLabel *_selectedLabelWidget;
  QButtonGroup *_fieldGroup;
  vector<QRadioButton *> _fieldButtons;
  DisplayField *_selectedField;
  string _selectedName;
  string _selectedLabel;
  string _selectedUnits;
  QLabel *_valueLabel;
  //int _fieldNum;
  int _prevFieldNum;

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

  // set top bar

  virtual void _setTitleBar(const string &radarName) = 0;

  // panels
  
  void _createStatusPanel();
  void _createFieldPanel();
  void _updateFieldPanel(string newFieldName);
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

protected slots:

  //////////////
  // Qt slots //
  //////////////

  virtual void _howto();
  void _about();
  void _showClick();
  virtual void _freeze() = 0;
  virtual void _unzoom() = 0;
  virtual void _refresh() = 0;
  virtual void _changeField(int fieldId, bool guiMode) = 0;
  virtual void _openFile();
  virtual void _saveFile();

  void _changeFieldVariable(bool value);

};

#endif

