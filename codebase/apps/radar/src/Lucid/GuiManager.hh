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

#include "GlobalData.hh"
#include "Params.hh"
#include "VlevelManager.hh"
#include "ClickReport.hh"
#include "VsectionRoute.hh"

#include <QMainWindow>
#include <euclid/SunPosn.hh>

class QActionGroup;
class QPushButton;
class QFrame;
class QDialog;
class QLabel;
class QGroupBox;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;
class QDateTime;
class QDateTimeEdit;
class QByteArray;
class QFileDialog;
class QImage;
class QTableWidget;
class QString;
class QTimer;

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
class ImageGenerationManager;
class MovieManager;
class WorldPlot;
struct TimelineFrame;

class GuiManager : public QMainWindow {
  
  Q_OBJECT

public:

  // constructor
  
  GuiManager();
  
  // destructor
  
  virtual ~GuiManager();

  // run 

  int run(QApplication &app);

  // enable the unzoom actions
  
  void enableUnzoomButtons(bool val) const;
  
  // set the zoom level

  void setZoomIndex(int zoomIndex);

  // void setXyZoom(double minY, double maxY, double minX, double maxX);

  // get selected name and units

  const string &getSelectedFieldLabel() const;
  int getSelectedFieldIndex() const { return _fieldNum; }
  bool mapsEnabled() const { return _mapsEnabled; }

  // set flag to indicate changes

  void setVlevelHasChanged(bool val) { _vlevelHasChanged = val; }
  void setOverlaysHaveChanged(bool val) { _overlaysHaveChanged = val; }
  void mapDisplayChanged();
  
  // set title bar str
  
  void setTitleBarStr(const string &titleStr);

  // update the click report from a horizontal-view selection

  void updateClickReport(const ClickReport &report);

  // Coordinate asynchronous data retrieval with TimeControl's movie builder.
  void prepareMovieBuild(int frameCount);
  void beginMovieFrameBuild(int frameIndex);
  void cancelMovieBuild();
  bool showMovieFrame(int frameIndex);
  bool exportMovieLoop(QString &message);
  void stopMoviePlayback();
  void restoreLiveDisplayAfterMovie();
  void stopMovieForDisplayChange();
  bool isMovieBuilding() const;
  bool isMoviePlaying() const;
  bool isMovieActive() const;
  int getTimelineFrameCount() const;
  int getTimelineFrameIndex() const;
  const TimelineFrame *getTimelineFrame(int frameIndex) const;
  void movieStateChanged();
  void showMovieInteractionBlocked();

  // Change the optional ground overlay without rebuilding either field view.
  void setTerrainEnabled(bool enabled);
  
  // override event handling

  virtual void timerEvent(QTimerEvent * event) override;
  virtual void resizeEvent(QResizeEvent * event) override;
  virtual void keyPressEvent(QKeyEvent* event) override;

  // get pos at top right of main window

  QPoint getTopRightPos();

public slots:

  void showClickDialog();
  void vertWindowClosed(QCloseEvent *event);
  
signals:

private:

  Params &_params;
  GlobalData &_gd;

  int _mainTimerId;
  int _timerEventCount;

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
  // Keep the most recently completed route independently of the canvas so a
  // later field or time change can request the same vertical section.

  VsectionRoute _vsectionRoute;
  
  // vlevels

  VlevelManager _vlevelManager;
  VlevelSelector *_vlevelSelector;
  bool _vlevelHasChanged;

  // top-level menus

  QMenu *_fileMenu;
  QMenu *_timeMenu;
  QMenu *_helpMenu;

  // actions

  QAction *_openFileAct;
  QAction *_saveFileAct;
  QAction *_saveImageAct;

  QAction *_showFieldMenuAct;
  QAction *_showTimeControlAct;

  QAction *_showValueAct;
  QAction *_showClickAct;

  QAction *_gridsAct;
  QAction *_fixedRingsAct;
  QAction *_dataRingsAct;
  QAction *_showVertAct;

  QAction *_exitAct;
  QAction *_clearAct;
  QAction *_zoomBackAct;
  QAction *_zoomOutAct;
  QAction *_reloadAct;

  QAction *_howtoAct;
  QAction *_aboutAct;
  QAction *_aboutQtAct;

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

  bool _fieldHasChanged;
  bool _horizontalRequestHasChanged;

  MovieManager *_movieManager;

  // Terrain uses independent horizontal and vertical MDV requests. Failed
  // completions are settled so optional context cannot stall captured output.
  int _terrainHorizontalRequestGeneration;
  bool _terrainHorizontalSettled;
  bool _terrainHorizontalRequestPending;
  int _terrainVerticalRequestGeneration;
  bool _terrainVerticalSettled;
  void _checkMovieFrameReady();

  // The image manager owns unattended scheduling and publication. GuiManager
  // retains only the live display state which must be restored between
  // realtime catalog batches.
  ImageGenerationManager *_imageGenerationManager;

  // Realtime scheduling temporarily drives the display through the catalog
  // matrix. Preserve the accepted interactive state so the interval between
  // batches remains a normal live display rather than the last output job.
  bool _automaticDisplayStateSaved;
  int _automaticSavedField;
  int _automaticSavedZoom;
  double _automaticSavedHeight;
  DateTime _automaticSavedStartTime;
  int _automaticSavedFrameCount;
  double _automaticSavedFrameIntervalSecs;
  int _automaticSavedFrameIndex;
  bool _automaticSavedRealtime;
  bool _automaticSavedVertWindowVisible;
  VsectionRoute _automaticSavedVsectionRoute;

  void _saveAutomaticDisplayState();
  void _restoreAutomaticDisplayState();
  void _startAutomaticImageJob();
  void _startAutomaticHorizontalJob();
  void _startAutomaticVerticalJob();
  void _selectAutomaticImageField(int fieldIndex,
                                  bool requestHorizontal = true);
  void _checkAutomaticImageReady();
  bool _captureAutomaticImage(QImage &image,
                              DateTime &dataTime,
                              const WorldPlot *&world,
                              bool &writeGeoXml,
                              QString &error);
  void _cancelTimedOutImageRequest(int fieldIndex,
                                   bool vertical,
                                   int expectedGeneration);
  bool _buildAutomaticVsectionRoute(VsectionRoute &route,
                                    QString &error) const;

  // maps

  QMenu *_mapsMenu;
  QAction *_mapsEnabledAct;
  vector<MapMenuItem *> _mapMenuItems;
  bool _mapsEnabled;
  
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

  // overlays menu
  
  QMenu *_overlaysMenu;
  bool _overlaysHaveChanged;

  // show menu
  
  QMenu *_showMenu;

  // click location report dialog

  QDialog *_clickReportDialog;
  QGridLayout *_clickReportDialogLayout;
  QLabel *_clickDate;
  QLabel *_clickTime;
  QLabel *_clickLatitude;
  QLabel *_clickLongitude;
  QLabel *_clickHeightName;
  QLabel *_clickHeight;
  QLabel *_clickXName;
  QLabel *_clickX;
  QLabel *_clickYName;
  QLabel *_clickY;
  QTableWidget *_clickFieldsTable;
  bool _clickReportPlaced;

  //////////////////////////////
  // private methods
  
  // open File 

  // void _openFile();
  // void _saveFile();

  // local methods
  
  void _setupWindows();
  void _configureAutomaticImageUi();
  void _createActions();
  void _createMenus();
  void _populateMapsMenu();
  void _populateProductsMenu();
  void _populateWindsMenu();
  void _populateZoomsMenu();
  void _populateOverlaysMenu();
  void _populateShowMenu();
  void _createClickReportDialog();
  QLabel *_addClickReportRow(const string &name, int row,
                             QLabel **nameLabelOut = nullptr);

  // field menu

  QSize _getTableWidgetSize(QTableWidget *t,
                            bool includeVertHeader,
                            bool includeHorizHeader);
  void _createFieldMenu();
  
  // check for status change
  
  void _checkAndReadH(MdvReader *mr);
  bool _checkForStateChange();
  // bool _checkForZoomChange();
  void _handleFirstTimerEvent();
    
  // for timer
  
  void _handleClientEvent();

       
  int _requestVerticalSection();
  int _requestVerticalWinds();
  int _requestVerticalTerrain();
  int _requestVerticalTerrain(const VsectionRoute &route);
  bool _horizontalTerrainApplies() const;
  int _requestHorizontalTerrain();
                          
private slots:

  //////////////
  // Qt slots //
  //////////////

  // file menu
  
  void _openFile();
  void _saveFile();
  void _reloadData();

  // help menu
  
  void _howto();
  void _about();

  // resize event
  
  void _resizeFinished();
  
  // field menu
  
  void _showFieldMenu();
  void _placeFieldMenu();
  void _fieldTableCellClicked(int row, int col);
  void _showFieldContextMenu(const QPoint &pos);
  void _applyFieldRenderingChange(int fieldNum);

  // maps

  void _setMapsEnabled(bool enable);

  // products

  void _setProductsEnabled(bool enable);

  // winds

  void _setWindsEnabled(bool enable);

  // time controller

  void _showTimeControl();
  void _placeTimeControl();
  
  // zoom menu
  
  void _zoomBack();
  void _zoomOut();

  // show menu

  void _showValue();

  // retain a completed route for the forthcoming MDV vertical-section read

  void _verticalRouteCompleted(const VsectionRoute &route);
  void _verticalReadComplete(MdvReader *reader);
  void _terrainHorizontalReadComplete(MdvReader *reader, int generation,
                                      bool success);
  void _terrainVerticalReadComplete(MdvReader *reader);
  
  // images

  void _saveViewImages();

  // open file 

  // void _createFileChooserDialog();
  // void _refreshFileChooserDialog();
  // void _showFileChooserDialog();
  
  // interaction help

  void _showInteractionHelp(const QPoint &pos);
  
};

#endif

