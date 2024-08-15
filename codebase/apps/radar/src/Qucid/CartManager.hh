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

// #include "Args.hh"
// #include "Params.hh"
#include "DisplayManager.hh"

// #include "RayLoc.hh"
// #include "ContextEditingView.hh"
// #include "BoundaryPointEditor.hh"
#include <QMainWindow>
#include <QListWidgetItem>
#include <euclid/SunPosn.hh>
#include <Radx/RadxRay.hh>

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

// class DisplayField;
class HorizWidget;
class VertWidget;
class VertWindow;
class RadxPlatform;
class TimeScaleWidget;
class MapMenuItem;
class ProdMenuItem;
class WindMenuItem;
class ZoomMenuItem;
class TimeControl;

class CartManager : public DisplayManager {
  
  Q_OBJECT

public:
  static CartManager* Instance();

  // boundary editor dialog
  QDialog *_boundaryEditorDialog;
  QGridLayout *_boundaryEditorDialogLayout;

  // constructor
  
  CartManager();
  // CartManager(const Params &params,
  //             const vector<DisplayField *> &fields,
  //             bool haveFilteredFields);
  
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

  // set/get archive mode
  
  void setArchiveMode(bool state);
  bool getArchiveMode() const { return _archiveMode; }

  // input file list for archive mode

  // void setArchiveFileList(const vector<string> &list,
  //                         bool fromCommandLine = true);

  // const vector<string> &getArchiveFileList() const {
  //   return _archiveFileList;
  // }
  // size_t getArchiveFileListSize() const {
  //   return _archiveFileList.size();
  // }
  
  // load archive file list by searching for files
  // returns 0 on success, -1 on failure
  
  // int loadArchiveFileList();
  // void setArchiveRetrievalPending();

  //  const RadxVol getDataVolume();

public slots:

  // void colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
  //       			QColor gridColor,
  //       			QColor emphasisColor,
  //       			QColor annotationColor,
  //       			QColor backgroundColor);
  // void setVolume(); // const RadxVol &radarDataVolume);

signals:

private:

  static CartManager* m_pInstance;
  // // string _openFilePath;
  string _boundaryDir;
  void setBoundaryDir();
  // string getBoundaryFilePath(string boundaryFileName);

  // // bool _firstTime;
  // // bool _urlOK;

  // // beam geometry
  
  // // int _nGates;
  // // double _maxRangeKm;

  // // ray locations

  // // vector<RayLoc> _rayLoc;

  // // input data
  
  // // RadxTime _readerRayTime;
  // // RadxVol _vol;

  // // sweeps

  // // SweepManager _sweepManager;
  // QVBoxLayout *_sweepVBoxLayout;
  // QGroupBox *_sweepPanel;
  // vector<QRadioButton *> *_sweepRButtons;

  // horizontal view windows

  QFrame *_horizFrame;
  HorizWidget *_horiz;
  
  // vertical view windows

  VertWindow *_vertWindow;
  VertWidget *_vert;
  bool _vertWindowDisplayed;
  bool _vertMode;
  
  // times for data

  RadxTime _plotStartTime;
  RadxTime _plotEndTime;

  // archive mode
  
  bool _archiveMode; // false for realtime mode
  RadxTime _archiveStartTime;// start time for archive mode
  
  // top-level menus

  QMenu *_fileMenu;
  QMenu *_timeMenu;
  QMenu *_overlaysMenu;
  QMenu *_helpMenu;

  // actions

  QAction *_showFieldMenuAct;
  QAction *_showTimeControlAct;
  QAction *_ringsAct;
  QAction *_gridsAct;
  QAction *_azLinesAct;
  QAction *_showVertAct;
  QAction *_openFileAct;
  QAction *_saveFileAct;
  QAction *_saveImageAct;

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
  vector<MapMenuItem *> _mapMenuItems;
  
  // winds

  QMenu *_windsMenu;
  QAction *_windsEnabledAct;
  bool _windsEnabled;
  vector<WindMenuItem *> _windMenuItems;
  
  // prods

  QMenu *_productsMenu;
  QAction *_productsEnabledAct;
  bool _productsEnabled;
  vector<ProdMenuItem *> _productMenuItems;
  
  // zooms

  QMenu *_zoomsMenu;
  QAction *_zoomsEnabledAct;
  QActionGroup *_zoomsActionGroup;
  bool _zoomsEnabled;
  vector<ZoomMenuItem *> _zoomMenuItems;
  
  // time controller settings dialog
  
  TimeControl *_timeControl;
  bool _timeControlPlaced;

  int _nArchiveScans;
  vector<string> _archiveFileList;
  bool _archiveFilesHaveDayDir;
  RadxTime _archiveIntermediateTime;
  RadxTime _startDisplayTime;
  RadxTime _currentDisplayTime;  // is this needed??
  RadxTime _endDisplayTime;

  // saving images in real time mode

  RadxTime _imagesScheduledTime;
  RadxTime _imagesStartTime;
  RadxTime _imagesEndTime;
  int _imagesScanIntervalSecs;

  // polygon 

  QPushButton *_boundaryEditorClearBtn;
  QPushButton *_boundaryEditorHelpBtn;
  QPushButton *_boundaryEditorSaveBtn;
  QPushButton *_boundaryEditorPolygonBtn;
  QPushButton *_boundaryEditorCircleBtn;
  QPushButton *_boundaryEditorBrushBtn;
  QListWidget *_boundaryEditorList;
  QLabel *_boundaryEditorInfoLabel;
  bool forceHide = true;

  QSlider *_circleRadiusSlider;
  QSlider *_brushRadiusSlider;

  // for timer
  
  int redraw_interv;
  int update_interv;
  int update_due;

  struct timeval cur_tm;
  struct timeval last_frame_tm;
  struct timeval last_dcheck_tm;
  time_t last_tick = 0;
  long client_seq_num = 0;
  
  //////////////////////////////
  // private methods

  // open File 

  // void _openFile();
  // void _saveFile();
  void _moveUpDown();
  string _getOutputPath(bool interactive, string &outputDir, string fileExt);

  // set top bar

  virtual void _setTitleBar();
  
  // local methods

  void _clear();
  void _setupWindows();
  void _createActions();
  void _createMenus();
  void _populateMapsMenu();
  void _populateProductsMenu();
  void _populateOverlaysMenu();
  void _populateWindsMenu();
  void _populateZoomsMenu();

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

  void _activateRealtimeRendering();
  void _activateArchiveRendering();

  // archive mode

  // void _setSweepPanelVisibility();

  // field menu

  QSize _getTableWidgetSize(QTableWidget *t,
                            bool includeVertHeader,
                            bool includeHorizHeader);
  void _createFieldMenu();
  
  // time controller

  void _createTimeControl();

  // override howto and boundaryEditor

  void _howto();

  void _checkForFieldChange();
  void _handleFirstTimerEvent();
  void _readClickPoint();
    
  // for timer
  
  void _handleClientEvent();
  void _checkForExpiredData(time_t tm);
  void _checkForDataUpdates(time_t tm);
  void _checkWhatNeedsRendering(int frame_index);
  void _ciddTimerFunc(QTimerEvent *event);
       
private slots:

  //////////////
  // Qt slots //
  //////////////

  // override

  virtual void _freeze();
  virtual void _zoomBack();
  virtual void _refresh();
  virtual void _changeField(int fieldId, bool guiMode = true);

  // sweeps

  // void _createSweepPanel();
  // void _createSweepRadioButtons();
  // void _clearSweepRadioButtons();
  // void _changeSweep(bool value);
  // void _changeSweepRadioButton(int value);

  // local

  void _horizLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _vertLocationClicked(double xkm, double ykm,
                           const RadxRay *closestRay);
  void _locationClicked(double xkm, double ykm,
                        const RadxRay *ray);

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

  // circle radius slider for BoundaryPointEditor
  void _circleRadiusSliderValueChanged(int value);
  void _brushRadiusSliderValueChanged(int value);

  // images

  void _saveImageToFile(bool interactive = true);
  void _createRealtimeImageFiles();
  void _createArchiveImageFiles();
  void _createImageFilesAllLevels();
  void _createImageFiles();

  // open file 

  // void _createFileChooserDialog();
  // void _refreshFileChooserDialog();
  // void _showFileChooserDialog();
  
  // context editing (SOLO)
  void ShowContextMenu(const QPoint &pos);

  // void _createBoundaryEditorDialog();
  
};

#endif

