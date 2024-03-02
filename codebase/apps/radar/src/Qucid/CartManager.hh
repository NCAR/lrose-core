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
// CartManager.hh
//
// CartManager object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// CartManager manages polar data gathering and dissemination
// for HORIZs and VERTs
//
// Rendering is delegated to HorizWidget and VertWidget
//
///////////////////////////////////////////////////////////////

#ifndef CartManager_HH
#define CartManager_HH

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include "DisplayManager.hh"
// #include "RayLoc.hh"
// #include "ContextEditingView.hh"
// #include "BoundaryPointEditor.hh"
#include <QMainWindow>
#include <QListWidgetItem>
#include <euclid/SunPosn.hh>
#include <Radx/RadxRay.hh>

class QApplication;
class QActionGroup;
class QButtonGroup;
class QRadioButton;
class QPushButton;
class QListWidget;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class QDateTime;
class QDateTimeEdit;
class QFileDialog;
class QTableWidget;

class DisplayField;
class HorizWidget;
class VertWidget;
class VertWindow;
class RadxPlatform;
class TimeScaleWidget;
class WindWrapper;
class MapWrapper;
class ProdWrapper;

class CartManager : public DisplayManager {
  
  Q_OBJECT

public:
  static CartManager* Instance();

  // boundary editor dialog
  QDialog *_boundaryEditorDialog;
  QGridLayout *_boundaryEditorDialogLayout;

  // constructor
  
  CartManager(const Params &params,
              const vector<DisplayField *> &fields,
              bool haveFilteredFields);
  
  // destructor
  
  virtual ~CartManager();

  // run 

  int run(QApplication &app);

  // enable the zoom button - called by PolarWidget

  void enableZoomButton() const;

  // override event handling

  virtual void timerEvent (QTimerEvent * event);
  virtual void resizeEvent (QResizeEvent * event);
  virtual void keyPressEvent(QKeyEvent* event);

  // check on archive mode
  
  bool checkArchiveMode() const { return _archiveMode; }

  // input file list for archive mode

  void setArchiveFileList(const vector<string> &list,
                          bool fromCommandLine = true);
  
  // load archive file list by searching for files
  // returns 0 on success, -1 on failure
  
  int loadArchiveFileList();


  //  const RadxVol getDataVolume();

public slots:

  void colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
				QColor gridColor,
				QColor emphasisColor,
				QColor annotationColor,
				QColor backgroundColor);
  void setVolume(); // const RadxVol &radarDataVolume);

signals:

private:

  static CartManager* m_pInstance;
  string _openFilePath;
  string _boundaryDir;
  void setBoundaryDir();
  string getBoundaryFilePath(string boundaryFileName);

  bool _firstTime;
  bool _urlOK;

  // beam geometry
  
  int _nGates;
  double _maxRangeKm;

  // ray locations

  // vector<RayLoc> _rayLoc;

  // input data
  
  RadxTime _readerRayTime;
  RadxVol _vol;

  // sweeps

  // SweepManager _sweepManager;
  QVBoxLayout *_sweepVBoxLayout;
  QGroupBox *_sweepPanel;
  vector<QRadioButton *> *_sweepRButtons;

  // windows

  QFrame *_horizFrame;
  HorizWidget *_horiz;

  VertWindow *_vertWindow;
  VertWidget *_vert;
  bool _vertWindowDisplayed;
  bool _vertMode;
  
  // azimuths for current ray

  double _prevAz;
  double _prevEl;
  double _startAz;
  double _endAz;

  // times for rays

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;
  RadxTime _prevRayTime;
  
  // top-level menus

  QMenu *_fileMenu;
  QMenu *_timeMenu;
  QMenu *_overlaysMenu;
  QMenu *_helpMenu;

  // actions

  QAction *_showFieldMenuAct;
  QAction *_realtimeAct;
  QAction *_showTimeControlAct;
  QAction *_ringsAct;
  QAction *_gridsAct;
  QAction *_azLinesAct;
  QAction *_showVertAct;
  
  QAction *_timeControllerAct;
  QAction *_openFileAct;
  QAction *_saveFileAct;
  QAction *_saveImageAct;

  // archive mode
  
  bool _archiveMode;
  bool _archiveRetrievalPending;

  QDateTimeEdit *_archiveStartTimeEdit;
  RadxTime _guiStartTime;
  RadxTime _archiveStartTime;
  
  QDateTimeEdit *_archiveEndTimeEdit;
  RadxTime _guiEndTime;
  RadxTime _archiveEndTime;

  QPushButton *_selectedTimeLabel;
  RadxTime _selectedTime;

  QPushButton *_back1;
  QPushButton *_fwd1;
  QPushButton *_backPeriod;
  QPushButton *_fwdPeriod;
  QPushButton *_boundaryEditorClearBtn;
  QPushButton *_boundaryEditorHelpBtn;
  QPushButton *_boundaryEditorSaveBtn;
  QPushButton *_boundaryEditorPolygonBtn;
  QPushButton *_boundaryEditorCircleBtn;
  QPushButton *_boundaryEditorBrushBtn;
  QListWidget *_boundaryEditorList;
  QLabel *_boundaryEditorInfoLabel;
  bool forceHide = true;

  // field menu
  
  QDialog *_fieldMenu;
  QTableWidget *_fieldTable;
  bool _fieldMenuPlaced;
  QFrame *_fieldMenuPanel;
  QVBoxLayout *_fieldMenuLayout;
  int _fieldTableCurrentColumn;
  int _fieldTableCurrentRow;

  // maps

  QMenu *_mapsMenu;
  QAction *_mapsEnabledAct;
  bool _mapsEnabled;
  vector<MapWrapper *> _mapWrappers;
  
  // winds

  QMenu *_windsMenu;
  QAction *_windsEnabledAct;
  bool _windsEnabled;
  vector<WindWrapper *> _windWrappers;
  
  // prods

  QMenu *_productsMenu;
  QAction *_productsEnabledAct;
  bool _productsEnabled;
  vector<ProdWrapper *> _productWrappers;
  
  // time controller settings dialog
  
  QDialog *_timeControl;
  bool _timeControlPlaced;

  int _nArchiveScans;
  vector<string> _archiveFileList;
  int _archiveScanIndex;
  bool _archiveFilesHaveDayDir;

  // time slider

  QFrame *_timePanel;
  QVBoxLayout *_timeLayout;

  QSlider *_timeSlider;
  QSlider *_circleRadiusSlider;
  QSlider *_brushRadiusSlider;

  RadxTime _archiveIntermediateTime;

  RadxTime _startDisplayTime;
  RadxTime _currentDisplayTime;  // is this needed??
  RadxTime _endDisplayTime;
  RadxTime _imagesArchiveStartTime;
  RadxTime _imagesArchiveEndTime;
  int _imagesScanIntervalSecs;

  // saving images in real time mode

  RadxTime _imagesScheduledTime;

  //////////////////////////////
  // private methods

  // open File 

  void _openFile();
  void _saveFile();
  void _moveUpDown();
  string _getOutputPath(bool interactive, string &outputDir, string fileExt);

  // set top bar

  virtual void _setTitleBar(const string &radarName);
  
  // local methods

  void _clear();
  void _setupWindows();
  void _createActions();
  void _createMenus();
  void _populateMapsMenu();
  void _populateWindsMenu();
  void _populateProductsMenu();
  void _populateOverlaysMenu();

  // data retrieval

  void _handleRealtimeData(QTimerEvent * event);
  void _handleArchiveData(/* QTimerEvent * event */);
  int _getArchiveData();
  void _plotArchiveData();
  void _setupVolRead(RadxFile &file);
  //  int _applyDataEdits(RadxVol _editedVol);  // & or * ??
  void _applyDataEdits(); // const RadxVol &editedVol);

  // draw beam

  void _handleRay(RadxPlatform &platform, RadxRay *ray);

  // ray handling for display

  void _storeRayLoc(const RadxRay *ray, const double az,
		    const double beam_width);
  void _clearRayOverlap(const int start_index, const int end_index);

  // modes

  void _setArchiveMode(bool state);
  void _activateRealtimeRendering();
  void _activateArchiveRendering();

  // archive mode

  void _setGuiFromArchiveStartTime();
  void _setGuiFromArchiveEndTime();
  void _setGuiFromSelectedTime();
  void _setSweepPanelVisibility();

  // field menu

  QSize _getTableWidgetSize(QTableWidget *t,
                            bool includeVertHeader,
                            bool includeHorizHeader);
  void _createFieldMenu();
  
  // time controller

  void _createTimeControl();

  // override howto and boundaryEditor

  void _howto();

private slots:

  //////////////
  // Qt slots //
  //////////////

  // override

  virtual void _freeze();
  virtual void _unzoom();
  virtual void _refresh();
  virtual void _changeField(int fieldId, bool guiMode = true);

  // sweeps

  void _createSweepPanel();
  void _createSweepRadioButtons();
  void _clearSweepRadioButtons();
  void _changeSweep(bool value);
  void _changeSweepRadioButton(int value);

  // local

  void _horizLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _vertLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _locationClicked(double xkm, double ykm,
                        const RadxRay *ray);

  // modes
  
  void _setRealtime(bool enabled);

  // archive mode
  
  void _setArchiveStartTime(const RadxTime &rtime);
  void _setArchiveEndTime(const RadxTime &rtime);
  void _setArchiveStartTimeFromGui(const QDateTime &qdt);
  void _setArchiveEndTimeFromGui(const QDateTime &qdt);
  void _acceptGuiTimes();
  void _cancelGuiTimes();

  void _goBack1();
  void _goFwd1();
  void _goBackPeriod();
  void _goFwdPeriod();

  void _setArchiveRetrievalPending();
  
  // field menu
  
  void _showFieldMenu();
  void _placeFieldMenu();

  // maps

  void _setMapsEnabled(bool enable);

  // products

  void _setProductsEnabled(bool enable);

  // winds

  void _setWindsEnabled(bool enable);

  // time controller

  void _showTimeControl();
  void _placeTimeControl();

  // time slider

  void _timeSliderActionTriggered(int action);
  void _timeSliderValueChanged(int value);
  void _timeSliderReleased();
  void _timeSliderPressed();
  
  //circle radius slider for BoundaryPointEditor
  void _circleRadiusSliderValueChanged(int value);
  void _brushRadiusSliderValueChanged(int value);

  // images

  void _saveImageToFile(bool interactive = true);
  void _createRealtimeImageFiles();
  void _createArchiveImageFiles();
  void _createImageFilesAllSweeps();
  void _createImageFiles();

  // open file 

  void _createFileChooserDialog();
  void _refreshFileChooserDialog();
  void _showFileChooserDialog();

  // context editing (SOLO)
  void ShowContextMenu(const QPoint &pos);

  // boundary editor
  // void createBoundaryEditorDialog();
  // void showBoundaryEditor();
  // void refreshBoundaries();
  // void clearBoundaryEditorClick();
  // void helpBoundaryEditorClick();
  // void polygonBtnBoundaryEditorClick();
  // void circleBtnBoundaryEditorClick();
  // void brushBtnBoundaryEditorClick();
  // void onBoundaryEditorListItemClicked(QListWidgetItem* item);
  // void saveBoundaryEditorClick();
  // void selectBoundaryTool(BoundaryToolType tool);
};

#endif

