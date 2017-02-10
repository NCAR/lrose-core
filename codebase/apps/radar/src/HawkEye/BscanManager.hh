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
// BscanManager.hh
//
// BscanManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// BscanManager manages BSCAN data - vert pointing etc
// Rendering is delegated to BscanWidget
//
///////////////////////////////////////////////////////////////

#ifndef BscanManager_HH
#define BscanManager_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "DisplayManager.hh"
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

class BscanManager : public DisplayManager {
  
  Q_OBJECT

public:

  // constructor

  BscanManager(const Params &params,
               Reader *reader,
               const vector<DisplayField *> &fields,
               bool haveFilteredFields);
  
  // destructor
  
  ~BscanManager();

  // run 

  int run(QApplication &app);

  // enable the zoom button - called by BscanWidget
  
  virtual void enableZoomButton() const;
  
signals:

private:

  // input data
  
  bool _plotStart;
  RadxTime _readerRayTime;
  RadxVol _vol;

  // windows

  QFrame *_bscanFrame;
  BscanWidget *_bscan;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  
  // menus

  QMenu *_fileMenu;
  QMenu *_overlaysMenu;
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

  // range limits

  QCheckBox *_specifyRangeLimitsBox;
  QLineEdit *_minRangeEdit;
  QLineEdit *_maxRangeEdit;
  
  QPushButton *_rangeAccept;
  QPushButton *_rangeCancel;
  
  QPushButton *_rangeAxisSetRangeFromZoom;
  QPushButton *_rangeAxisResetRangeLimits;

  bool _specifyRangeLimits;
  double _minPlotRangeKm;
  double _maxPlotRangeKm;
  double _rayMinRangeKm;
  double _rayMaxRangeKm;

  // altitude units

  QCheckBox *_altitudeInFeetBox;
  QLabel *_minAltitudeLabel;
  QLabel *_maxAltitudeLabel;
  QLineEdit *_minAltitudeEdit;
  QLineEdit *_maxAltitudeEdit;
  
  QPushButton *_altitudeAccept;
  QPushButton *_altitudeCancel;
  
  QPushButton *_rangeAxisSetAltitudeFromZoom;
  QPushButton *_rangeAxisResetAltitudeLimits;

  double _altitudeUnitsMult;
  double _minPlotAltitudeKm;
  double _maxPlotAltitudeKm;
  string _altitudeUnits;

  // censoring data below surface

  QGroupBox *_censorDataBelowSurfaceBox;
  QCheckBox *_censorDataToggleBox;
  QLineEdit *_surfaceFieldEdit;
  QLineEdit *_minRangeToSurfaceEdit;
  QLineEdit *_surfaceRangeMarginEdit;
  QPushButton *_rangeAxisResetCensorDataBelowSurface;

  QPushButton *_censorDataBelowSurfaceAccept;
  QPushButton *_censorDataBelowSurfaceCancel;
  
  bool _censorDataBelowSurface;
  string _surfaceField;
  double _minRangeToSurfaceKm;
  double _surfaceRangeMarginKm;

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
  const RadxRay *_rayClicked;

  // saving images in real time mode

  RadxTime _imagesScheduledTime;

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

  void _createRangeAxisDialog();
  void _refreshRangeAxisDialog();

  void _createTimeAxisDialog();
  void _refreshTimeAxisDialog();

  // draw the beam
  
  void _handleRay(const RadxRay *ray);
  void _addRay(const RadxRay *ray);

  // retrieve data
  
  void _handleRealtimeData();
  void _handleArchiveData();
  int _getArchiveData();
  void _plotArchiveData();
  void _setupVolRead(RadxFile &file);
  void _setDwellAutoVal();

private slots:

  //////////////
  // Qt slots //
  //////////////

  // override

  virtual void _freeze();
  virtual void _unzoom();
  virtual void _refresh();
  virtual void _changeField(int fieldId, bool guiMode = true);

  // local

  void _showRangeAxisDialog();
  void _showTimeAxisDialog();

  void _bscanLocationClicked(double xkm, double ykm, const RadxRay *closestRay);
  void _locationClicked(double xkm, double ykm, const RadxRay *closestRay);

  void _setRangeAxisRangeUp();
  void _setRangeAxisRangeDown();
  void _setRangeAxisAltitude();

  void _cancelRangeAxisChanges();
  void _doneWithRangeAxis();
  void _cancelTimeAxisChanges();

  void _setAltitudeLimits();
  void _setAltitudeInFeet();
  void _setAltitudeLimitsFromZoom();
  void _setRangeLimitsFromZoom();
  void _setAltitudeLimitsToDefaults();
  void _resetAltitudeLimitsToDefaults();

  void _setRangeLimits();
  void _setSpecifyRangeLimits();
  void _setRangeLimitsToDefaults();
  void _resetRangeLimitsToDefaults();

  void _setDistScaleEnabled();

  void _setCensorDataBelowSurface();
  void _setCensorDataBelowSurfaceToDefaults();
  void _resetCensorDataBelowSurfaceToDefaults();
  void _cancelCensorDataBelowSurfaceChanges();

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

  double _getCensorRange(const RadxRay *ray);

  /// @brief Save the current image to a file
  /// 
  /// The destination path and file name for the image are controlled by 
  /// myriad _params.image_* parameters. See the source...
  /// @param interactive If true, a dialog box will be used report the file
  /// name for the saved image or to note errors
  void _saveImageToFile(bool interactive = true);
  
  void _createArchiveImageFiles();
  void _createRealtimeImageFiles();
  void _createImageFiles();

  // override howto

  void _howto();

};

#endif

