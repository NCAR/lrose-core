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
// GuiManager.hh
//
// GuiManager object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// GuiManager manages polar data gathering and dissemination
// for HORIZs and VERTs
//
// Rendering is delegated to HorizView and VertWidget
//
///////////////////////////////////////////////////////////////

#ifndef GuiManager_HH
#define GuiManager_HH

#include <string>
#include <vector>

#include "VlevelManager.hh"
#include "XyBox.hh"

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
class QHBoxLayout;
class QVBoxLayout;
class QDateTime;
class QDateTimeEdit;
class QFileDialog;
class QTableWidget;

class HorizView;
class VertView;
class VertManager;
class RadxPlatform;
class TimeScaleWidget;
class MapMenuItem;
class ProdMenuItem;
class WindMenuItem;
class ZoomMenuItem;
class TimeControl;
class VlevelSelector;

class GuiManager : public QMainWindow {
  
  Q_OBJECT

public:

  static GuiManager* Instance();

  // constructor
  
  GuiManager();
  
  // destructor
  
  virtual ~GuiManager();

  // run 

  int run(QApplication &app);

  // enable the unzoom actions
  
  void enableZoomBackButton() const;
  void enableZoomOutButton() const;
  
  // override event handling

  virtual void timerEvent(QTimerEvent * event);
  virtual void resizeEvent(QResizeEvent * event) override;
  virtual void keyPressEvent(QKeyEvent* event);

  // set/get archive mode
  
  void setArchiveMode(bool state);
  bool getArchiveMode() const { return _archiveMode; }

  // get selected name and units

  const string &getSelectedFieldLabel() const { return _selectedLabel; }
  const string &getSelectedFieldName() const { return _selectedName; }
  const string &getSelectedFieldUnits() const { return _selectedUnits; }

  // set the xy zoom

  void setXyZoom(double minY, double maxY, double minX, double maxX);

  // set flag to indicate changes

  void setVlevelHasChanged(bool val) { _vlevelHasChanged = val; }
  void setOverlaysHaveChanged(bool val) { _overlaysHaveChanged = val; }
  
  // boundary editor dialog
  QDialog *_boundaryEditorDialog;
  QGridLayout *_boundaryEditorDialogLayout;

public slots:

  // void colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
  //       			QColor gridColor,
  //       			QColor emphasisColor,
  //       			QColor annotationColor,
  //       			QColor backgroundColor);
  // void setVolume(); // const RadxVol &radarDataVolume);

signals:

private:

  int _mainTimerId;
  int _timerEventCount;
  bool _setupWindowsComplete;
  static GuiManager* m_pInstance;

  // main window frame

  QFrame *_main;

  // resize

  bool _resized;
  QTimer *_resizeTimer;
  
  // horizontal view windows
  
  QFrame *_horizFrame;
  HorizView *_horiz;
  
  // vertical view windows

  VertManager *_vertWindow;
  VertView *_vert;
  bool _vertWindowDisplayed;
  bool _vertMode;
  
  // times for data

  DateTime _plotStartTime;
  DateTime _plotEndTime;

  // vlevels

  VlevelManager _vlevelManager;
  VlevelSelector *_vlevelSelector;
  bool _vlevelHasChanged;

  // zooms

  XyBox _zoomXy;
  XyBox _prevZoomXy;
  
  // archive mode
  
  bool _archiveMode; // false for realtime mode
  DateTime _archiveStartTime; // start time for archive mode
  
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

  QAction *_exitAct;
  QAction *_clearAct;
  QAction *_zoomBackAct;
  QAction *_zoomOutAct;
  QAction *_reloadAct;
  QAction *_showClickAct;
  QAction *_showBoundaryEditorAct;
  QAction *_howtoAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;
  QAction *_openFileAct;
  QAction *_saveFileAct;
  QAction *_saveImageAct;

  // fields
  
  QDialog *_fieldMenu;
  QTableWidget *_fieldTable;
  bool _fieldMenuPlaced;
  QFrame *_fieldMenuPanel;
  QVBoxLayout *_fieldMenuLayout;

  int _fieldNum;
  int _prevFieldNum;

  int _fieldTableCol;
  int _fieldTableRow;

  int _prevFieldTableCol;
  int _prevFieldTableRow;

  string _selectedName;
  string _selectedLabel;
  string _selectedUnits;
  QLabel *_valueLabel;
  
  bool _fieldHasChanged;

  // maps and overlays

  QMenu *_mapsMenu;
  QAction *_mapsEnabledAct;
  vector<MapMenuItem *> _mapMenuItems;
  bool _mapsEnabled;
  bool _overlaysHaveChanged;
  
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
  
  // creating images in archive mode
  
  vector<string> _archiveFileList;
  bool _archiveFilesHaveDayDir;
  DateTime _archiveIntermediateTime;

  // saving images in real time mode

  DateTime _imagesScheduledTime;
  DateTime _imagesStartTime;
  DateTime _imagesEndTime;
  int _imagesScanIntervalSecs;

  // boundary editor
  
  string _boundaryDir;
  void setBoundaryDir();
  string getBoundaryFilePath(string boundaryFileName);

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
  string _getOutputPath(bool interactive, string &outputDir, string fileExt);

  // set top bar

  void _setTitleBar();
  
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

  // void _setVlevelPanelVisibility();

  // field menu

  QSize _getTableWidgetSize(QTableWidget *t,
                            bool includeVertHeader,
                            bool includeHorizHeader);
  void _createFieldMenu();
  
  // check for status change
  
  bool _checkForStateChange();
  bool _checkForZoomChange();
  void _handleFirstTimerEvent();
  void _readClickPoint();
    
  // for timer
  
  void _handleClientEvent();
  void _handleImageCreationEvent();

  void _checkForExpiredData(time_t tm);
  void _checkForDataUpdates(time_t tm);
  void _checkWhatNeedsRendering(int frame_index);
       
  // setting text

  void _setText(char *text, size_t maxTextLen, const char *format, int val);
  void _setText(char *text, size_t maxTextLen, const char *format, double val);

  // swap 2 int items

  void _swap(int &val1, int &val2);

  // legacy
  
  void _autoCreateFunc();
  // void _ciddTimerFunc(QTimerEvent *event);
  void _setField(int value);
  void _setDisplayTime(time_t utime);
  void _setEndFrame(int num_frames);
  void _updateMoviePopup();
                          
private slots:

  //////////////
  // Qt slots //
  //////////////

  void _howto();
  void _about();
  void _showClick();
  // void _freeze();
  void _zoomBack();
  void _zoomOut();
  void _refresh();
  // void _changeField(int fieldId, bool guiMode = true);
  void _openFile();
  void _saveFile();

  // vlevels

  // void _createVlevelFrame();
  // void _createVlevelRadioButtons();
  // void _clearVlevelRadioButtons();
  // void _changeVlevel(bool value);
  // void _changeVlevelRadioButton(int value);
  
  // local
  
// #ifdef NOTNOW
//   void _horizLocationClicked(double xkm, double ykm,
//                              const RadxRay *closestRay);
//   void _vertLocationClicked(double xkm, double ykm,
//                             const RadxRay *closestRay);
//   void _locationClicked(double xkm, double ykm,
//                         const RadxRay *ray);
// #endif
  
  // resize event
  
  void _resizeFinished();
  
  // field menu
  
  void _showFieldMenu();
  void _placeFieldMenu();
  void _fieldTableCellClicked(int row, int col);

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

