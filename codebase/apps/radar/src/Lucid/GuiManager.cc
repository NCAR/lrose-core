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
///////////////////////////////////////////////////////////////
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Sept 2023
//
///////////////////////////////////////////////////////////////
//
// GuiManager manages cartesian rendering - plan view
//
///////////////////////////////////////////////////////////////

#include <string>
#include <cmath>
#include <iostream>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QFrame>
#include <QFont>
#include <QLabel>
#include <QToolTip>
#include <QMenu>
#include <QMenuBar>
#include <QGroupBox>
#include <QMessageBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QRadioButton>
#include <QStatusBar>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QErrorMessage>
#include <QFileDialog>
#include <QSlider>
#include <QGraphicsScene>
#include <QGraphicsAnchorLayout>
#include <QGraphicsProxyWidget>
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QWidgetAction>
#include <QTimer>
#include <QDesktopServices>
#include <QTableWidget>
#include <QHeaderView>

#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaStr.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>
#include <Radx/RadxFile.hh>
#include <Radx/NcfRadxFile.hh>
#include <Radx/RadxSweep.hh>
#include <toolsa/DateTime.hh>
#include <Radx/RadxPath.hh>
#include <Ncxx/H5x.hh>

#include "GuiManager.hh"
#include "ProductMgr.hh"
#include "FieldTableItem.hh"
#include "HorizView.hh"
#include "MapMenuItem.hh"
#include "ProdMenuItem.hh"
#include "TimeControl.hh"
#include "VlevelSelector.hh"
#include "VertView.hh"
#include "VertManager.hh"
#include "WindMenuItem.hh"
#include "ZoomMenuItem.hh"

// GlobalData.hh must be included AFTER qt includes because of #define None in X11/X.h

#include "GlobalData.hh"

using namespace std;
using namespace H5x;

GuiManager* GuiManager::m_pInstance = NULL;

GuiManager* GuiManager::Instance()
{
  return m_pInstance;
}

// Constructor

GuiManager::GuiManager() :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _vertWindowDisplayed(false)
{

  _timerEventCount = 0;
  _archiveMode = true;
  
  m_pInstance = this;

  // initialize

  _main = NULL;
  _resizeTimer = NULL;
  _resized = false;
  
  _horizFrame = NULL;
  _horiz = NULL;

  _vertWindow = NULL;
  _vert = NULL;

  // _vlevelVBoxLayout = NULL;
  // _vlevelFrame = NULL;
  // _vlevelPanel = NULL;
  
  _fieldMenu = NULL;
  _fieldTable = NULL;
  _fieldMenuPlaced = false;
  _fieldMenuPanel = NULL;
  _fieldMenuLayout = NULL;

  _fieldNum = 0;
  _prevFieldNum = -1;
  _fieldHasChanged = true;
  
  _fieldTableCol = 0;
  _fieldTableRow = 0;
  
  _prevFieldTableCol = -1;
  _prevFieldTableRow = -1;

  _vlevelManager.requestLevel(_params.start_ht);
  _vlevelHasChanged = true;
  
  _overlaysHaveChanged = true;
  
  _timeControl = NULL;
  _timeControlPlaced = false;

  setArchiveMode(_params.start_mode == Params::MODE_ARCHIVE);
  _archiveStartTime.set(_params.archive_start_time);

  _imagesStartTime.set(_params.images_archive_start_time);
  _imagesEndTime.set(_params.images_archive_end_time);
  _imagesScanIntervalSecs = _params.images_scan_interval_secs;

  // set up ray locators

  // _rayLoc.resize(RayLoc::RAY_LOC_N);

  // set up windows

  _setupWindows();

  // set initial field to 0

  // _changeField(0, false);

  // init for timer
  
  redraw_interv = _params.redraw_interval;
  update_interv = _params.update_interval;
  update_due = 0;
  last_frame_tm = {0,0};
  last_dcheck_tm = {0,0};
  last_tick = 0;
  client_seq_num = 0;

  setAttribute(Qt::WA_TranslucentBackground);
  setAutoFillBackground(false);

}

// destructor

GuiManager::~GuiManager()

{

  if (_horiz) {
    delete _horiz;
  }

  if (_vert) {
    delete _vert;
  }

}

//////////////////////////////////////////////////
// Run

int GuiManager::run(QApplication &app)
{

  // make window visible
  
  show();

  // move to starting point
  
  QPoint pos;
  pos.setX(_params.horiz_window_x_pos);
  pos.setY(_params.horiz_window_y_pos);
  move(pos);
  
  // set timer running
  
  _mainTimerId = startTimer(2); // msecs
  
  return app.exec();

}

//////////////////////////////////////////////////
// enable the zoom button - called by Widgets

void GuiManager::enableZoomBackButton() const
{
  _zoomBackAct->setEnabled(true);
}

void GuiManager::enableZoomOutButton() const
{
  _zoomOutAct->setEnabled(true);
}

//////////////////////////////////////////////////
// set the XY zoom limits

void GuiManager::setXyZoom(double minY, double maxY,
                           double minX, double maxX)
{
  _prevZoomXy = _zoomXy;
  _zoomXy.setLimits(minY, maxY, minX, maxX);
  _gd.h_win.cmin_y = minY;
  _gd.h_win.cmax_y = maxY;
  _gd.h_win.cmin_x = minX;
  _gd.h_win.cmax_x = maxX;
}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void GuiManager::timerEvent(QTimerEvent *event)
{

  // register with procmap

  PMU_auto_register("timerEvent");
  
  // check ID
  
  if (event->timerId() != _mainTimerId) {
    return;
  }
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.
  
  if (_timerEventCount == 0) {
    _handleFirstTimerEvent();
  }    
  _timerEventCount++;

  // get the time control into the right place
  // we need to let the windows fully draw
  
  if (_timerEventCount == 10) {
    _placeTimeControl();
  }

  // read click point info from FMQ
  
  _readClickPoint();
  
  // handle client events
  
  if(_gd.coord_expt->client_event != NO_MESSAGE) {
    _handleClientEvent();
  }

  // is previous read still busy?

  MdvReader *mr = _gd.mread[_fieldNum];
  if (!mr->getReadBusyH()) {
    // read the H data if needed
    _checkAndReadH(mr);
  }
  
}

//////////////////////////////////////////////////////////////
// check if we need new H data, and read accordingly
// then trigger rendering if appropriate
  
void GuiManager::_checkAndReadH(MdvReader *mr)
{

  // check for state change
  
  bool stateChanged = _checkForStateChange();
  
  // get new data if needed - data is retrieved in a thread
  
  int frameIndex = _gd.movie.cur_frame;
  if (_gd.movie.cur_frame < 0) {
    frameIndex = _gd.movie.num_frames - 1;
  }

  if (stateChanged) {
    mr->requestHorizPlane(_timeControl->getSelectedTime().utime(),
                          _vlevelManager.getRequestedLevel(),
                          _gd.h_win.page);
  }
  
  // check for new data
  
  if (mr->isNewH()) {
    if (!mr->isValidH()) {
      cerr << "ERROR - GuiManager::timerEvent" << endl;
      cerr << "  mr->requestHorizPlane" << endl;
      cerr << "  time_start: " << DateTime::strm(_gd.movie.frame[frameIndex].time_start) << endl;
      cerr << "  time_end: " << DateTime::strm(_gd.movie.frame[frameIndex].time_end) << endl;
      cerr << "  page: " << _gd.h_win.page << endl;
    }
    int frameIndex = _gd.movie.cur_frame;
    if (_gd.movie.cur_frame < 0) {
      frameIndex = _gd.movie.num_frames - 1;
    }
    _horiz->triggerGridRendering(_gd.h_win.page, frameIndex);
    if (_gd.h_win.page < _gd.num_datafields) {
      _vlevelManager.set(*_gd.mread[_gd.h_win.page]);
    }
    _vlevelSelector->update();
  }

}

///////////////////////////////////////////
// check for state changes

bool GuiManager::_checkForStateChange()

{

  bool stateHasChanged = false;

  if (_fieldHasChanged) {
    stateHasChanged = true;
    _fieldHasChanged = false;
  }
  
  // zoom change?
  
  if (_checkForZoomChange()) {
    stateHasChanged = true;
  }

  // vlevel change?
  
  if (_vlevelHasChanged) {
    stateHasChanged = true;
    _vlevelHasChanged = false;
  }

  // time change

  if (_timeControl->timeHasChanged()) {
    stateHasChanged = true;
  }

  // resize?

  if (_resized) {
    stateHasChanged = true;
    _resized = false;
  }

  // overlays?

  if (_overlaysHaveChanged) {
    stateHasChanged = true;
    _overlaysHaveChanged = false;
  }

  return stateHasChanged;

}

///////////////////////////////////////////////
// override resize event
// we use a timer to debounce the resize event

void GuiManager::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  // Restart the timer on each resize event
  _resizeTimer->start();
}

void GuiManager::_resizeFinished()
{
  // Called after the resize events have "settled"
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent, width, height: "
         << _horizFrame->width() << ", " << _horizFrame->height() << endl;
  }
  _horiz->resize(_horizFrame->width(), _horizFrame->height());
  _resized = true;
}

////////////////////////////////////////////////////////////////
void GuiManager::keyPressEvent(QKeyEvent * e)
{

  // get key pressed
  
  // Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Clicked char: " << keychar << ":" << (int) keychar << endl;
    cerr << "         key: " << hex << key << dec << endl;
  }
  
  // for '.', swap with previous field
  
  if (keychar == '.') {
    if (_prevFieldNum >= 0) {
      _fieldTable->setCurrentCell(_prevFieldTableRow, _prevFieldTableCol);
      emit _fieldTable->cellClicked(_prevFieldTableRow, _prevFieldTableCol);
    }
    return;
  }
  
  // for ESC, freeze / unfreeze

  // if (keychar == 27) {
  //   _freezeAct->trigger();
  //   return;
  // }

  // check for up/down in vlevels
  
  if (key == Qt::Key_Left) {
    
    if (_params.debug) {
      cerr << "Clicked left arrow, go back in time" << endl;
    }
    _timeControl->goBack1();
    
  } else if (key == Qt::Key_Right) {

    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _timeControl->goFwd1();
    
  } else if (key == Qt::Key_Up) {
    
    if (_vlevelManager.getSelectedIndex() > 0) {
      if (_params.debug) {
        cerr << "Clicked up arrow, go up a vlevel" << endl;
      }
      _vlevelSelector->keyPressEvent(e);
      // _changeVlevelRadioButton(-1);
    }
    
  } else if (key == Qt::Key_Down) {
    
    if (_vlevelManager.getSelectedIndex() < (int) _vlevelManager.getNLevels() - 1) {
      if (_params.debug) {
        cerr << "Clicked down arrow, go down a vlevel" << endl;
      }
      _vlevelSelector->keyPressEvent(e);
      // _changeVlevelRadioButton(+1);
    }
    
  }

}

///////////////////////////////////
// swap 2 ints

void GuiManager::_swap(int &val1, int &val2) 
{
  int tmp = val1;
  val1 = val2;
  val2 = tmp;
}

// void GuiManager::_moveUpDown() 
// {
//   this->setCursor(Qt::WaitCursor);
//   this->setCursor(Qt::ArrowCursor);
// }

//////////////////////////////////////////////////
// Set radar name in title bar

void GuiManager::setTitleBarStr(const string &titleStr)
{
  setWindowTitle(tr(titleStr.c_str()));
}
  
//////////////////////////////////////////////////
// set up windows and widgets
  
void GuiManager::_setupWindows()
{

  // set up windows
  
  _main = new QFrame(this);
  QHBoxLayout *mainLayout = new QHBoxLayout;
  _main->setLayout(mainLayout);
  mainLayout->setSpacing(0);
  mainLayout->setContentsMargins(2,2,2,2);
  setCentralWidget(_main);

  // horiz window

  _horizFrame = new QFrame(_main);
  mainLayout->addWidget(_horizFrame);
  _horizFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _horizFrame->resize(_params.horiz_plot_width + _params.horiz_color_scale_width,
                      _params.horiz_plot_height);

  // configure the HORIZ

  _horiz = new HorizView(_horizFrame, *this);
  
  // Create the VERT window

  _vertWindow = new VertManager(this);
  _vertWindow->setRadarName("unknown");
  
  // set pointer to the vertWidget
  
  _vert = _vertWindow->getView();
  
  // connect slots for location

#ifdef NOTNOW
  connect(_horiz, &HorizView::locationClicked,
          this, &GuiManager::_horizLocationClicked);
  connect(_vert, &VertWidget::locationClicked,
          this, &GuiManager::_vertLocationClicked);
#endif

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);

  // customContextMenuRequested(e->pos());
  connect(_horiz, &HorizView::customContextMenuRequested,
	  this, &GuiManager::ShowContextMenu);

  // vlevel selector

  _vlevelSelector = new VlevelSelector(_params.vlevel_selector_width,
                                       _vlevelManager, this);
  mainLayout->addWidget(_vlevelSelector);
                                       
  // resize timer for debouncing resize events
  
  _resizeTimer = new QTimer(this);
  _resizeTimer->setInterval(250); // 0.25s delay
  _resizeTimer->setSingleShot(true);
  connect(_resizeTimer, &QTimer::timeout, this, &GuiManager::_resizeFinished);

  // field menu

  _createFieldMenu();
  
  // time panel
  
  _timeControl = new TimeControl(this);
  
  // fill out menu bar

  _createActions();
  _createMenus();

  // title bar

  setTitleBarStr(_params.horiz_frame_label);

  setMinimumSize(_params.horiz_window_min_width, _params.horiz_window_min_height);
  // resize(_params.main_window_width, _params.main_window_height);
  // connect(this, &GuiManager::frameResized, _horiz, &HorizView::resize);
  resize(_params.horiz_plot_width + _params.horiz_color_scale_width,
         _params.horiz_plot_height);
  
  // set location on screen

  QPoint pos;
  pos.setX(_params.horiz_window_x_pos);
  pos.setY(_params.horiz_window_y_pos);
  move(pos);
  show();

  // set up field status dialog
  // _createClickReportDialog();

  // createBoundaryEditorDialog();

  // if (_archiveMode) {
  //   _showTimeControl();
  // }
  // _setVlevelPanelVisibility();

}

//////////////////////////////////////////////////
// add/remove  vlevel panel (archive mode only)

// void GuiManager::_setVlevelPanelVisibility()
// {
//   if (_vlevelPanel != NULL) {
//     _vlevelPanel->setVisible(true);
//   }
// }

//////////////////////////////
// create actions for menus

void GuiManager::_createActions()
{

  // show field menu
  _showFieldMenuAct = new QAction(tr("Fields"), this);
  _showFieldMenuAct->setStatusTip(tr("Show field menu"));
  connect(_showFieldMenuAct, &QAction::triggered,
          this, &GuiManager::_showFieldMenu);
  
  // freeze display
  // _freezeAct = new QAction(tr("Freeze"), this);
  // _freezeAct->setShortcut(tr("Esc"));
  // _freezeAct->setStatusTip(tr("Freeze display"));
  // connect(_freezeAct, &QAction::triggered, this, &GuiManager::_freeze);
  
  // show user click in dialog
  _showClickAct = new QAction(tr("Values"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, &QAction::triggered, this, &GuiManager::_showClick);

  // show boundary editor dialog
  // _showBoundaryEditorAct = new QAction(tr("Boundary Editor"), this);
  // _showBoundaryEditorAct->setStatusTip(tr("Show boundary editor dialog"));
  // connect(_showBoundaryEditorAct, &QAction::triggered, this, &GuiManager::showBoundaryEditor);
  
  // show time control window
  _showTimeControlAct = new QAction(tr("Movie"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, &QAction::triggered,
          this, &GuiManager::_showTimeControl);
  
  // unzoom display
  
  // _zoomBackAct = new QAction(tr("Back"), this);
  // _zoomBackAct->setStatusTip(tr("Unzoom to previous view"));
  // _zoomBackAct->setEnabled(false);
  // connect(_zoomBackAct, &QAction::triggered, this, &GuiManager::_zoomBack);

  // reload data
  
  _reloadAct = new QAction(tr("Reload"), this);
  _reloadAct->setStatusTip(tr("Reload data"));
  connect(_reloadAct, &QAction::triggered, this, &GuiManager::_refresh);

  // clear display
  _clearAct = new QAction(tr("Clear"), this);
  _clearAct->setStatusTip(tr("Clear data"));
  connect(_clearAct, &QAction::triggered, _horiz, &HorizView::clear);
  connect(_clearAct, &QAction::triggered, _vert, &VertView::clear);

  // exit app
  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, &QAction::triggered, this, &GuiManager::close);

  // file chooser for Open
  _openFileAct = new QAction(tr("O&pen"), this);
  _openFileAct->setShortcut(tr("Ctrl+F"));
  _openFileAct->setStatusTip(tr("Open File"));
  connect(_openFileAct, &QAction::triggered, this, &GuiManager::_openFile);

  // file chooser for Save
  _saveFileAct = new QAction(tr("S&ave"), this);
  //_saveFileAct->setShortcut(tr("Ctrl+S"));
  _saveFileAct->setStatusTip(tr("Save File"));
  connect(_saveFileAct, &QAction::triggered, this, &GuiManager::_saveFile);

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.horiz_grids_on_at_startup);
  connect(_gridsAct, &QAction::triggered, _horiz, &HorizView::setGrids);

  // show range rings

  _fixedRingsAct = new QAction(tr("Fixed range Rings"), this);
  _fixedRingsAct->setStatusTip(tr("Turn fixed range rings on/off"));
  _fixedRingsAct->setCheckable(true);
  _fixedRingsAct->setChecked(_params.plot_range_rings_fixed);
  connect(_fixedRingsAct, &QAction::triggered, _horiz, &HorizView::setRingsFixed);

  _dataRingsAct = new QAction(tr("Data-driven range Rings"), this);
  _dataRingsAct->setStatusTip(tr("Turn data-driven range rings on/off"));
  _dataRingsAct->setCheckable(true);
  _dataRingsAct->setChecked(_params.plot_range_rings_from_data);
  connect(_dataRingsAct, &QAction::triggered, _horiz, &HorizView::setRingsDataDriven);
  
  // show VERT window

  _showVertAct = new QAction(tr("Show VERT Window"), this);
  _showVertAct->setStatusTip(tr("Show the VERT Window"));
  connect(_showVertAct, &QAction::triggered, _vertWindow, &VertView::show);

  // howto and about
  
  _howtoAct = new QAction(tr("Howto"), this);
  _howtoAct->setStatusTip(tr("Show the application's Howto box"));
  connect(_howtoAct, &QAction::triggered, this, &GuiManager::_howto);

  _aboutAct = new QAction(tr("About"), this);
  _aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAct, &QAction::triggered, this, &GuiManager::_about);

  _aboutQtAct = new QAction(tr("About &Qt"), this);
  _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(_aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

  // save image
  
  _saveImageAct = new QAction(tr("Save-Image"), this);
  _saveImageAct->setStatusTip(tr("Save image to png file"));
  connect(_saveImageAct, &QAction::triggered, this, &GuiManager::_saveImageToFile);

}

////////////////
// create menus

void GuiManager::_createMenus()
{

  // file menu
  
  _fileMenu = menuBar()->addMenu(tr("File"));
  _fileMenu->addSeparator();
  _fileMenu->addAction(_openFileAct);
  _fileMenu->addAction(_saveFileAct);
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  // field menu
  
  menuBar()->addAction(_showFieldMenuAct);
  
  // add maps menu
  
  _mapsMenu = menuBar()->addMenu(tr("Maps"));
  _populateMapsMenu();

  // add products menu
  
  _productsMenu = menuBar()->addMenu(tr("Products"));
  _populateProductsMenu();

  // add winds menu
  
  _windsMenu = menuBar()->addMenu(tr("Winds"));
  _populateWindsMenu();

  // add zooms menu
  
  _zoomsMenu = menuBar()->addMenu(tr("Zooms"));
  _populateZoomsMenu();
  // menuBar()->addAction(_zoomBackAct);
  
  // time selector
  
  menuBar()->addAction(_showTimeControlAct);

  // add overlay menu
  
  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _populateOverlaysMenu();

  // misc actions
  
  // menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showClickAct);
  // menuBar()->addAction(_showBoundaryEditorAct);
  menuBar()->addAction(_reloadAct);

  _helpMenu = menuBar()->addMenu(tr("Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

/////////////////////////////////////////////////////////////
// populate maps menu

void GuiManager::_populateMapsMenu()
{

  // maps enabled

  _mapsEnabled = _params.maps_enabled_at_startup;
  _mapsEnabledAct = new QAction(tr("Maps enabled"), this);
  _mapsEnabledAct->setStatusTip(tr("Enable / disable all maps"));
  _mapsEnabledAct->setCheckable(true);
  _mapsEnabledAct->setChecked(_mapsEnabled);
  connect(_mapsEnabledAct, &QAction::toggled,
	  this, &GuiManager::_setMapsEnabled);

  _mapsMenu->addAction(_mapsEnabledAct);
  _mapsMenu->addSeparator();
  
  // loop through map entries
  
  for (int imap = 0; imap < _params.maps_n; imap++) {

    Params::map_t &mparams = _params._maps[imap];

    // create action for this entry

    MapMenuItem *item = new MapMenuItem(nullptr, this);
    QAction *act = new QAction;
    item->setMapParams(&mparams);
    item->setMapIndex(imap);
    item->setOverlay(_gd.overlays[imap]);
    item->setAction(act);
    act->setText(mparams.map_code);
    act->setStatusTip(tr("Turn map layer on/off"));
    act->setCheckable(true);
    act->setChecked(mparams.on_at_startup);
    connect(act, &QAction::toggled,
            item, &MapMenuItem::toggled);
    
    // add item for map selection
    
    _mapMenuItems.push_back(item);
    
    // add to maps menu

    _mapsMenu->addAction(act);

  } // imap
  
}

////////////////////////////////////////////////////////////
// enable maps

void GuiManager::_setMapsEnabled(bool enable)
{
  _mapsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> maps enabled? " << (_mapsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate products menu

void GuiManager::_populateProductsMenu()
{

  // products enabled

  _productsEnabled = _params.symprods_enabled_at_startup;
  _productsEnabledAct = new QAction(tr("Products enabled"), this);
  _productsEnabledAct->setStatusTip(tr("Enable / disable all products"));
  _productsEnabledAct->setCheckable(true);
  _productsEnabledAct->setChecked(_productsEnabled);
  connect(_productsEnabledAct, &QAction::toggled,
	  this, &GuiManager::_setProductsEnabled);

  _productsMenu->addAction(_productsEnabledAct);
  _productsMenu->addSeparator();
  
  // loop through wind entries
  
  for (int iprod = 0; iprod < _params.symprod_prod_info_n; iprod++) {
    
    Params::symprod_prod_info_t &prodParams = _params._symprod_prod_info[iprod];
    
    // create action for this entry

    ProdMenuItem *item = new ProdMenuItem;
    const Product *product = _gd.prod_mgr->getProduct(iprod);
    QAction *act = new QAction;
    item->setProdParams(&prodParams);
    item->setProduct(product);
    item->setProdIndex(iprod);
    item->setAction(act);
    act->setText(prodParams.menu_label);
    act->setStatusTip(tr("Turn product on/off"));
    act->setCheckable(true);
    act->setChecked(prodParams.on_by_default);
    connect(act, &QAction::toggled,
            item, &ProdMenuItem::toggled);
    
    // add item for wind selection
    
    _productMenuItems.push_back(item);

    // add to products menu

    _productsMenu->addAction(act);

  } // iprod
  
}

////////////////////////////////////////////////////////////
// enable products

void GuiManager::_setProductsEnabled(bool enable)
{
  _productsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> products enabled? " << (_productsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate winds menu

void GuiManager::_populateWindsMenu()
{

  // winds enabled

  _windsEnabled = _params.winds_on_at_startup;
  _windsEnabledAct = new QAction(tr("Winds enabled"), this);
  _windsEnabledAct->setStatusTip(tr("Enable / disable all winds"));
  _windsEnabledAct->setCheckable(true);
  _windsEnabledAct->setChecked(_windsEnabled);
  connect(_windsEnabledAct, &QAction::toggled,
	  this, &GuiManager::_setWindsEnabled);

  _windsMenu->addAction(_windsEnabledAct);
  _windsMenu->addSeparator();
  
  // loop through wind entries
  
  for (int iwind = 0; iwind < _params.winds_n; iwind++) {

    Params::wind_t &wparams = _params._winds[iwind];

    // create action for this entry

    WindMenuItem *item = new WindMenuItem;
    QAction *act = new QAction;
    item->setWindParams(&wparams);
    item->setWindIndex(iwind);
    item->setWindData(&_gd.layers.wind[iwind]);
    item->setAction(act);
    act->setText(wparams.button_label);
    act->setStatusTip(tr("Turn wind layer on/off"));
    act->setCheckable(true);
    act->setChecked(wparams.on_at_startup);
    connect(act, &QAction::toggled,
            item, &WindMenuItem::toggled);
    
    // add item for wind selection
    
    _windMenuItems.push_back(item);

    // add to winds menu

    _windsMenu->addAction(act);

  } // iwind
  
}

////////////////////////////////////////////////////////////
// enable winds

void GuiManager::_setWindsEnabled(bool enable)
{
  _windsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> winds enabled? " << (_windsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate zooms menu
// this is exclusive - only 1 item selected at a time

void GuiManager::_populateZoomsMenu()
{

  _zoomsActionGroup = new QActionGroup(_zoomsMenu);
  _zoomsActionGroup->setExclusive(true);
  
  _zoomOutAct = new QAction(tr("Zoom out"), this);
  _zoomOutAct->setStatusTip(tr("Unzoom all the way"));
  _zoomOutAct->setEnabled(false);
  connect(_zoomOutAct, &QAction::triggered, this, &GuiManager::_zoomOut);
  _zoomsMenu->addAction(_zoomOutAct);
  
  _zoomBackAct = new QAction(tr("Zoom back"), this);
  _zoomBackAct->setStatusTip(tr("Unzoom to previous view"));
  _zoomBackAct->setEnabled(false);
  connect(_zoomBackAct, &QAction::triggered, this, &GuiManager::_zoomBack);
  _zoomsMenu->addAction(_zoomBackAct);
  
  // loop through zoom entries
  
  for (int izoom = 0; izoom < _params.zoom_levels_n; izoom++) {
    
    Params::zoom_level_t &zparams = _params._zoom_levels[izoom];
    
    // create item for this entry
    
    ZoomMenuItem *item = new ZoomMenuItem(nullptr, this);
    item->setZoomParams(&zparams);
    item->setZoomIndex(izoom);

    // create action for the item
    
    QAction *act = new QAction;
    item->setAction(act);
    act->setText(zparams.label);
    act->setStatusTip(tr("Select predefined zoom"));
    act->setCheckable(true);
    if (izoom == _gd.h_win.zoom_level) {
      act->setChecked(true);
    }
    
    // add to exclusive group
    
    _zoomsActionGroup->addAction(act);

    // connect
    
    connect(act, &QAction::toggled,
            item, &ZoomMenuItem::toggled);
    
    // add item for zoom selection
    
    _zoomMenuItems.push_back(item);

    // add to zooms menu
    
    _zoomsMenu->addAction(act);
    
  } // izoom
  
}

/////////////////////////////////////////////////////////////
// populate overlays menu

void GuiManager::_populateOverlaysMenu()
{

  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_fixedRingsAct);
  _overlaysMenu->addAction(_dataRingsAct);
  _overlaysMenu->addSeparator();
  _overlaysMenu->addAction(_showVertAct);

}

/////////////////////////////////////////////////////////////
// create the vlevel panel
// buttons will be filled in by createVlevelRadioButtons()

// void GuiManager::_createVlevelFrame()
// {
  
//   _vlevelFrame = new QFrame(_main);
//   QHBoxLayout *frameLayout = new QHBoxLayout;
//   _vlevelFrame->setLayout(frameLayout);
  
//   _vlevelPanel = new QGroupBox("Z", _vlevelFrame);
//   frameLayout->addWidget(_vlevelPanel);
//   _vlevelVBoxLayout = new QVBoxLayout;
//   _vlevelPanel->setLayout(_vlevelVBoxLayout);
//   _vlevelPanel->setAlignment(Qt::AlignHCenter);
  
//   _vlevelRButtons = new vector<QRadioButton *>();
  
  
// }

/////////////////////////////////////////////////////////////////////
// create radio buttons
// this requires that _vlevelManager is up to date with vlevel info

// void GuiManager::_createVlevelRadioButtons() 
// {

//   // clear previous
  
//   _clearVlevelRadioButtons();
  
//   // fonts
  
//   QLabel dummy;
//   QFont font = dummy.font();
//   QFont fontm2 = dummy.font();
//   int fsize = _params.label_font_size;
//   int fsizem2 = _params.label_font_size - 2;
//   font.setPixelSize(fsize);
//   fontm2.setPixelSize(fsizem2);
  
//   // radar and site name
  
//   char buf[256];
//   _vlevelRButtons = new vector<QRadioButton *>();
  
//  for (int ielev = 0; ielev < (int) _vlevelManager.getNLevels(); ielev++) {
    
//     std::snprintf(buf, 256, "%.2f", _vlevelManager.getLevel(ielev));
//     QRadioButton *radio1 = new QRadioButton(buf); 
//     radio1->setFont(fontm2); 
    
//     if (ielev == _vlevelManager.getIndexInGui()) {
//       radio1->setChecked(true);
//     }
    
//     _vlevelRButtons->push_back(radio1);
//     _vlevelVBoxLayout->addWidget(radio1);
    
//     // connect slot for vlevel change
                              
//     connect(radio1, SIGNAL(toggled(bool)), this, SLOT(_changeVlevel(bool)));
    
//   }

//   QString title("Z");
//   if (_vlevelManager.getUnits().size() > 0) {
//     title += "(";
//     title += _vlevelManager.getUnits();
//     title += ")";
//   }
//   _vlevelPanel->setTitle(title);

// }

///////////////////////////////////////////////////////////////////
// create vlevel panel of radio buttons

// void GuiManager::_clearVlevelRadioButtons() 
// {
  
//   QLayoutItem* child;
//   if (_vlevelVBoxLayout != NULL) {
//     while (_vlevelVBoxLayout->count() !=0) {
//       child = _vlevelVBoxLayout->takeAt(0);
//       if (child->widget() !=0) {
//         delete child->widget();
//       }
//       delete child;
//     }
//   }
  
// }

/////////////////////////////////////////////////////////////
// change vlevel

// void GuiManager::_changeVlevel(bool value) {
  
//   if (_params.debug) {
//     cerr << "From GuiManager: the vlevel was changed ";
//     cerr << endl;
//   }
  
//   if (!value) {
//     return;
//   }
  
//   for (size_t vlevelIndex = 0; vlevelIndex < _vlevelRButtons->size();
//        vlevelIndex++) {
//     if (_vlevelRButtons->at(vlevelIndex)->isChecked()) {
//       _vlevelManager.setIndexInGui(vlevelIndex);
//       if (_params.debug) {
//         cerr << "vlevelRButton " << vlevelIndex << " is checked" << endl;
//         cerr << "  moving to vlevel index, value: "
//              << vlevelIndex << ", " << _vlevelManager.getLevel() << endl;
//       }
//       // _horiz->setStartOfVlevel(true);
//       // _vert->setStartOfVlevel(true);
//       _moveUpDown();

//       _vlevelHasChanged = true;
//       _gd.redraw_horiz = true;
      
//       // reloadBoundaries();
//       return;
//     }
//   }  // ii
  
// }

///////////////////////////////////////////////////////////////
// change vlevel
// only set the vlevelIndex in one place;
// here, just move the radio button up or down one step
// when the radio button is changed, a signal is emitted and
// the slot that receives the signal will increase the vlevelIndex
// value = +1 move forward
// value = -1 move backward in vlevels

// void GuiManager::_changeVlevelRadioButton(int increment)

// {
  
//   if (_params.debug) {
//     cerr << "-->> changing vlevel index by increment: " << increment << endl;
//   }
  
//   if (increment != 0) {
//     _vlevelManager.changeIndexInGui(increment);
//     _vlevelRButtons->at(_vlevelManager.getIndexInGui())->setChecked(true);
//     _gd.redraw_horiz = true;
//   }
  
// }

///////////////////////////////////////
// set input file list for archive mode

// void GuiManager::setArchiveFileList(const vector<string> &list,
//                                      bool fromCommandLine /* = true */)
// {

//   if (fromCommandLine && list.size() > 0) {
//     // determine start and end time from file list
//     DateTime startTime, endTime;
//     NcfRadxFile::getTimeFromPath(list[0], startTime);
//     NcfRadxFile::getTimeFromPath(list[list.size()-1], endTime);
//     // round to nearest five minutes
//     time_t startTimeSecs = startTime.utime();
//     startTimeSecs =  (startTimeSecs / 300) * 300;
//     time_t endTimeSecs = endTime.utime();
//     endTimeSecs =  (endTimeSecs / 300) * 300 + 300;
//     _timeControl->setArchiveStartTime(startTimeSecs);
//     _timeControl->setArchiveEndTime(endTimeSecs);
//     _timeControl->setArchiveScanIndex(0);
//   }

//   _archiveFileList = list;
//   setArchiveRetrievalPending();

//   if (_timeControl->getArchiveScanIndex() < 0) {
//     _timeControl->setArchiveScanIndex(0);
//   } else if (_timeControl->getArchiveScanIndex() > (int) _archiveFileList.size() - 1) {
//     _timeControl->setArchiveScanIndex(_archiveFileList.size() - 1);
//   }

//   _timeControl->setTimeSliderMinimum(0);
//   if (_archiveFileList.size() <= 1) {
//     _timeControl->setTimeSliderMaximum(1);
//   } else {
//     _timeControl->setTimeSliderMaximum(_archiveFileList.size() - 1);
//   }
//   _timeControl->setTimeSliderPosition(_timeControl->getArchiveScanIndex());

//   // check if the paths include a day dir

//   _archiveFilesHaveDayDir = false;
//   if (list.size() > 0) {
//     RadxPath path0(list[0]);
//     RadxPath parentPath(path0.getDirectory());
//     string parentDir = parentPath.getFile();
//     int year, month, day;
//     if (sscanf(parentDir.c_str(), "%4d%2d%2d", &year, &month, &day) == 3) {
//       _archiveFilesHaveDayDir = true;
//     }
//   }

//   if (_archiveFilesHaveDayDir) {
//     _timeControl->setArchiveEnabled(true);
//   } else {
//     _timeControl->setArchiveEnabled(true);
//   }

//   cerr << "xxxxxxxxxxxxxxxxxxxxxx" << endl;
//   _timeControl->setGuiFromArchiveStartTime();
//   _timeControl->setGuiFromArchiveEndTime();

// }
  
// ///////////////////////////////////////////////
// // get archive file list by searching for files
// // returns 0 on success, -1 on failure

// int GuiManager::loadArchiveFileList()

// {
//   return 0;

// }

/////////////////////////////
// apply new, edited  data in archive mode
// the volume has been updated 

// void GuiManager::_applyDataEdits()
// {

//   if (_params.debug) {
//     std::ofstream outfile("/tmp/voldebug_GuiManager_applyDataEdits.txt");
//     _vol.printWithFieldData(outfile);  
//     outfile << "_vol = " << &_vol << endl;
//   }

//   _plotArchiveData();
// }

/////////////////////////////
// plot data in archive mode

// void GuiManager::_plotArchiveData()

// {

//   if(_params.debug) {
//     cerr << "Plotting archive data" << endl;
//     cerr << "  volume start time: " << _plotStartTime.asString() << endl;
//   }

// handle the rays

// const SweepManager::GuiSweep &gsweep = _sweepManager.getSelectedSweep();
// for (size_t ii = gsweep.radx->getStartRayIndex();
//      ii <= gsweep.radx->getEndRayIndex(); ii++) {
//   RadxRay *ray = rays[ii];
//   _handleRay(_platform, ray);
//   if (ii == 0) {
//     _updateStatusPanel(ray);
//   }
// }
  
// }

//////////////////////////////////////////////////
// set up read

// void GuiManager::_setupVolRead(RadxFile &file)
// {

//   if (_params.debug >= Params::DEBUG_VERBOSE) {
//     file.setDebug(true);
//   }
//   if (_params.debug >= Params::DEBUG_EXTRA) {
//     file.setDebug(true);
//     file.setVerbose(true);
//   }

//   for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
//     const DisplayField *field = _fields[ifield];
//     file.addReadField(field->getName());
//   }

// }

//////////////////////////////////////////////////////////////
// handle an incoming ray

// void GuiManager::_handleRay(RadxPlatform &platform, RadxRay *ray)
  
// {

//   if (ray->getNGates() < 1) {
//     return;
//   }

//   // do we need to reconfigure the HORIZ?

//   _nGates = ray->getNGates();
//   double maxRange = ray->getStartRangeKm() + _nGates * ray->getGateSpacingKm();
//   _maxRangeKm = maxRange;
//   _horiz->configureRange(_maxRangeKm);
//   _vert->configureRange(_maxRangeKm);

//   // create 2D field data vector

//   vector< vector<double> > fieldData;
//   fieldData.resize(_fields.size());
  
//   // fill data vector

//   for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

//     vector<double> &data = fieldData[ifield];
//     data.resize(_nGates);
//     RadxField *rfld = ray->getField(_fields[ifield]->getName());

//     // at this point, we know the data values for the field AND the color map                                                                        
//     bool haveColorMap = _fields[ifield]->haveColorMap();
//     Radx::fl32 min = FLT_MAX;;
//     Radx::fl32 max = FLT_MIN;

//     if (rfld == NULL) {
//       // fill with missing
//       for (int igate = 0; igate < _nGates; igate++) {
//         data[igate] = -9999.0;
//       }
//     } else {
//       rfld->convertToFl32();
//       const Radx::fl32 *fdata = rfld->getDataFl32();
//       const Radx::fl32 missingVal = rfld->getMissingFl32();
//       // we can only look at the data available, so only go to nGates
//       for (int igate = 0; igate < _nGates; igate++, fdata++) {  // was _nGates
//         Radx::fl32 val = *fdata;
//         if (fabs(val - missingVal) < 0.0001) {
//           data[igate] = -9999.0;
//         } else {
//           data[igate] = val;
//           if (!haveColorMap) {
//             // keep track of min and max data values
// 	    // just display something.  The color scale can be edited as needed, later.
// 	    bool newMinOrMax = false;
//             if (val < min) {
//               min = *fdata;
// 	      newMinOrMax = true;
// 	    }
//             if (val > max) {
// 	      max = *fdata;
// 	      newMinOrMax = true;
// 	    }
// 	    if ((newMinOrMax) && (_params.debug >= Params::DEBUG_VERBOSE)) { 
// 	      printf("field index %zu, gate %d \t", ifield, igate);
// 	      printf("new min, max of data %g, %g\t", min,  max);
// 	      printf("missing value %g\t", missingVal);
// 	      printf("current value %g\n", val);
// 	    }
//           }
//         } // end else not missing value
//       } // end for each gate
      
//       if (!haveColorMap) {                              
//         _fields[ifield]->setColorMapRange(min, max);
//         _fields[ifield]->changeColorMap(); // just change bounds on existing map
//       } // end do not have color map

//     } // end else vector not NULL
//   } // end for each field

//   // Store the ray location (which also sets _startAz and _endAz), then
//   // draw beam on the HORIZ or VERT, as appropriate

//   if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
//       ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI) {

//     _vertMode = true;

//     // If this is the first VERT beam we've encountered, automatically open
//     // the VERT window.  After this, opening and closing the window will be
//     // left to the user.

//     if (!_vertWindowDisplayed) {
//       _vertWindow->show();
//       _vertWindow->resize();
//       _vertWindowDisplayed = true;
//     }

//     // Add the beam to the display

//     _vert->addBeam(ray, fieldData, _fields);
//     _vertWindow->setAzimuth(ray->getAzimuthDeg());
//     _vertWindow->setElevation(ray->getElevationDeg());
    
//   } else {

//     _vertMode = false;

//     // check for elevation surveillance sweep mode
//     // in this case, set azimuth to rotation if georef is available

//     if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
//       ray->setAnglesForElevSurveillance();
//     }

//     // Store the ray location using the azimuth angle and the HORIZ location
//     // table

//     double az = ray->getAzimuthDeg();
//     _storeRayLoc(ray, az, platform.getRadarBeamWidthDegH());

//     // Save the angle information for the next iteration

//     _prevAz = az;
//     _prevEl = -9999.0;

//     // Add the beam to the display

//     _horiz->addBeam(ray, _startAz, _endAz, fieldData, _fields);

//   }
  
// }

///////////////////////////////////////////////////////////
// store ray location

// void GuiManager::_storeRayLoc(const RadxRay *ray, 
//                                const double az,
//                                const double beam_width)
// {

// #ifdef NOTNOW
  
//   LOG(DEBUG_VERBOSE) << "az = " << az << " beam_width = " << beam_width;

//   // Determine the extent of this ray

//   if (_params.horiz_override_rendering_beam_width) {
//     double half_angle = _params.horiz_rendering_beam_width / 2.0;
//     _startAz = az - half_angle - 0.1;
//     _endAz = az + half_angle + 0.1;
//   } else if (ray->getIsIndexed()) {
//     double half_angle = ray->getAngleResDeg() / 2.0;
//     _startAz = az - half_angle - 0.1;
//     _endAz = az + half_angle + 0.1;
//   } else {
//     double beam_width_min = beam_width;
//     if (beam_width_min < 0) 
//       beam_width_min = 10.0;

//     double max_half_angle = beam_width_min / 2.0;
//     double prev_offset = max_half_angle;
//     if (_prevAz > 0.0) { // >= 0.0) {
//       double az_diff = az - _prevAz;
//       if (az_diff < 0.0)
// 	az_diff += 360.0;
//       double half_az_diff = az_diff / 2.0;
	
//       if (prev_offset > half_az_diff)
// 	prev_offset = half_az_diff;
//     }
//     _startAz = az - prev_offset - 0.1;
//     _endAz = az + max_half_angle + 0.1;
//   }
    
//   // store
//   // HERE !!! fix up negative values here or in clearRayOverlap??
//   if (_startAz < 0) _startAz += 360.0;
//   if (_endAz < 0) _endAz += 360.0;
//   if (_startAz >= 360) _startAz -= 360.0;
//   if (_endAz >= 360) _endAz -= 360.0;
    
//   LOG(DEBUG_VERBOSE) << " startAz = " << _startAz << " endAz = " << _endAz;

//   // compute start and end indices, using modulus to keep with array bounds

//   int startIndex = ((int) (_startAz * RayLoc::RAY_LOC_RES)) % RayLoc::RAY_LOC_N;
//   int endIndex = ((int) (_endAz * RayLoc::RAY_LOC_RES + 1)) % RayLoc::RAY_LOC_N;

//   // Clear out any rays in the locations list that are overlapped by the
//   // new ray
    
//   if (startIndex > endIndex) {

//     // area crosses the 360; 0 boundary; must break into two sections

//     // first from start index to 360
    
//     _clearRayOverlap(startIndex, RayLoc::RAY_LOC_N - 1);

//     for (int ii = startIndex; ii < RayLoc::RAY_LOC_N; ii++) { // RayLoc::RAY_LOC_N; ii++) {
//       _rayLoc[ii].ray = ray;
//       _rayLoc[ii].active = true;
//       _rayLoc[ii].startIndex = startIndex;
//       _rayLoc[ii].endIndex = RayLoc::RAY_LOC_N - 1; // RayLoc::RAY_LOC_N;
//     }

//     // then from 0 to end index
    
//     _clearRayOverlap(0, endIndex);

//     // Set the locations associated with this ray
    
//     for (int ii = 0; ii <= endIndex; ii++) {
//       _rayLoc[ii].ray = ray;
//       _rayLoc[ii].active = true;
//       _rayLoc[ii].startIndex = 0;
//       _rayLoc[ii].endIndex = endIndex;
//     }

//   } else { // if (startIndex > endIndex) 

//     _clearRayOverlap(startIndex, endIndex);
    
//     // Set the locations associated with this ray

//     for (int ii = startIndex; ii <= endIndex; ii++) {
//       _rayLoc[ii].ray = ray;
//       _rayLoc[ii].active = true;
//       _rayLoc[ii].startIndex = startIndex;
//       _rayLoc[ii].endIndex = endIndex;
//     }

//   } // if (startIndex > endIndex)

// #endif

// }

 
///////////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

// void GuiManager::_clearRayOverlap(const int start_index, const int end_index)
// {

// #ifdef NOTNOW
  
//   LOG(DEBUG_VERBOSE) << "enter" << " start_index=" << start_index <<
//     " end_index = " << end_index;

//   if ((start_index < 0) || (start_index > RayLoc::RAY_LOC_N)) {
//     cout << "ERROR: _clearRayOverlap start_index out of bounds " << start_index << endl;
//     return;
//   }
//   if ((end_index < 0) || (end_index > RayLoc::RAY_LOC_N)) {
//     cout << "ERROR: _clearRayOverlap end_index out of bounds " << end_index << endl;
//     return;
//   }

//   // Loop through the ray locations, clearing out old information

//   int i = start_index;
  
//   while (i <= end_index) {

//     RayLoc &loc = _rayLoc[i];
    
//     // If this location isn't active, we can skip it

//     if (!loc.active) {
//       // LOG(DEBUG_VERBOSE) << "loc NOT active";
//       ++i;
//       continue;
//     }
    
//     int loc_start_index = loc.startIndex;
//     int loc_end_index = loc.endIndex;

//     if ((loc_start_index < 0) || (loc_start_index > RayLoc::RAY_LOC_N)) {
//       cout << "ERROR: _clearRayOverlap loc_start_index out of bounds " << loc_start_index << endl;
//       ++i;
//       continue;
//     }
//     if ((loc_end_index < 0) || (loc_end_index > RayLoc::RAY_LOC_N)) {
//       cout << "ERROR: _clearRayOverlap loc_end_index out of bounds " << loc_end_index << endl;
//       ++i;
//       continue;
//     }

//     if (loc_end_index < i) {
//       cout << " OH NO! We are HERE" << endl;
//       ++i;
//       continue;
//     }
//     // If we get here, this location is active.  We now have 4 possible
//     // situations:

//     if (loc.startIndex < start_index && loc.endIndex <= end_index) {

//       // The overlap area covers the end of the current beam.  Reduce the
//       // current beam down to just cover the area before the overlap area.
//       LOG(DEBUG_VERBOSE) << "Case 1a:";
//       LOG(DEBUG_VERBOSE) << " i = " << i;
//       LOG(DEBUG_VERBOSE) << "clearing from start_index=" << start_index <<
// 	  " to loc_end_index=" << loc_end_index;
      
//       for (int j = start_index; j <= loc_end_index; ++j) {

// 	_rayLoc[j].ray = NULL;
// 	_rayLoc[j].active = false;

//       }

//       // Update the end indices for the remaining locations in the current
//       // beam
//       LOG(DEBUG_VERBOSE) << "Case 1b:";
//       LOG(DEBUG_VERBOSE) << "setting endIndex to " << start_index - 1 << " from loc_start_index=" << loc_start_index <<
// 	  " to start_index=" << start_index;
      
//       for (int j = loc_start_index; j < start_index; ++j)
// 	_rayLoc[j].endIndex = start_index - 1;

//     } else if (loc.startIndex < start_index && loc.endIndex > end_index) {
      
//       // The current beam is bigger than the overlap area.  This should never
//       // happen, so go ahead and just clear out the locations for the current
//       // beam.
//       LOG(DEBUG_VERBOSE) << "Case 2:";
//       LOG(DEBUG_VERBOSE) << " i = " << i;
//       LOG(DEBUG_VERBOSE) << "clearing from loc_start_index=" << loc_start_index <<
// 	  " to loc_end_index=" << loc_end_index;
      
//       for (int j = loc_start_index; j <= loc_end_index; ++j) {
//         _rayLoc[j].clear();
//       }

//     } else if (loc.endIndex > end_index) {
      
//       // The overlap area covers the beginning of the current beam.  Reduce the
//       // current beam down to just cover the area after the overlap area.

// 	LOG(DEBUG_VERBOSE) << "Case 3a:";
// 	LOG(DEBUG_VERBOSE) << " i = " << i;
// 	LOG(DEBUG_VERBOSE) << "clearing from loc_start_index=" << loc_start_index <<
// 	  " to end_index=" << end_index;

//       for (int j = loc_start_index; j <= end_index; ++j) {
// 	_rayLoc[j].ray = NULL;
// 	_rayLoc[j].active = false;
//       }

//       // Update the start indices for the remaining locations in the current
//       // beam

//       LOG(DEBUG_VERBOSE) << "Case 3b:";
//       LOG(DEBUG_VERBOSE) << "setting startIndex to " << end_index + 1 << " from end_index=" << end_index <<
// 	  " to loc_end_index=" << loc_end_index;
      
//       for (int j = end_index + 1; j <= loc_end_index; ++j) {
// 	_rayLoc[j].startIndex = end_index + 1;
//       }

//     } else {
      
//       // The current beam is completely covered by the overlap area.  Clear
//       // out all of the locations for the current beam.
//       LOG(DEBUG_VERBOSE) << "Case 4:";
//       LOG(DEBUG_VERBOSE) << " i = " << i;
//       LOG(DEBUG_VERBOSE) << "clearing from loc_start_index=" << loc_start_index <<
// 	  " to loc_end_index=" << loc_end_index;
      
//       for (int j = loc_start_index; j <= loc_end_index; ++j) {
//         _rayLoc[j].clear();
//       }

//     }
    
//     i = loc_end_index + 1;

//   } /* endwhile - i */
  
//   LOG(DEBUG_VERBOSE) << "exit ";

// #endif
  
// }

////////////////////////////////////////////
// freeze / unfreeze

// void GuiManager::_freeze()
// {
//   if (_frozen) {
//     _frozen = false;
//     _freezeAct->setText("Freeze");
//     _freezeAct->setStatusTip(tr("Click to freeze display, or hit ESC"));
//   } else {
//     _frozen = true;
//     _freezeAct->setText("Unfreeze");
//     _freezeAct->setStatusTip(tr("Click to unfreeze display, or hit ESC"));
//   }
// }

////////////////////////////////////////////
// unzoom to previous zoom

void GuiManager::_zoomBack()
{
  _horiz->zoomBackView();
  if (_horiz->getSavedZooms().size() == 0) {
    _zoomBackAct->setEnabled(false);
    _zoomOutAct->setEnabled(false);
  }
  // _gd.redraw_horiz = true;
}

////////////////////////////////////////////
// unzoom all the way out

void GuiManager::_zoomOut()
{
  _horiz->zoomOutView();
  _horiz->clearSavedZooms();
  _zoomBackAct->setEnabled(false);
  _zoomOutAct->setEnabled(false);
  // _gd.redraw_horiz = true;
}

////////////////////////////////////////////
// refresh

void GuiManager::_refresh()
{
  if (_horiz) {
    _horiz->update();
  }
}

////////////////////////////////////////////////////////
// handle params file

void GuiManager::_openFile() {
}

void GuiManager::_saveFile() {
}


#ifdef NOTYET
// TODO: need to add the background changed, etc. 
void GuiManager::colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
                                          QColor gridColor,
                                          QColor emphasisColor,
                                          QColor annotationColor,
                                          QColor backgroundColor) {

  LOG(DEBUG_VERBOSE) << "enter";

  // connect the new color map with the field                                                       
  // find the fieldName in the list of FieldDisplays                                                
  bool found = false;
  vector<DisplayField *>::iterator it;
  int fieldId = -1;

  it = _fields.begin();
  while ( it != _fields.end() && !found ) {
    DisplayField *field = *it;

    string name = field->getName();
    if (name.compare(fieldName) == 0) {
      found = true;
      field->replaceColorMap(newColorMap);
    }
    fieldId++;
    it++;
  }
  if (!found) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << "ERROR - field not found; no color map change";
    // TODO: present error message box                                                              
  } else {
    // look up the fieldId from the fieldName                                                       
    // change the field variable                                                                    
    _horiz->backgroundColor(backgroundColor);
    _horiz->gridRingsColor(gridColor);
    _changeField(fieldId, false);
  }
  LOG(DEBUG_VERBOSE) << "exit";
}

void GuiManager::setVolume() { // const RadxVol &radarDataVolume) {

  LOG(DEBUG_VERBOSE) << "enter";

  // _applyDataEdits(); // radarDataVolume);

  LOG(DEBUG_VERBOSE) << "exit";



}
#endif

#ifdef NOTNOW

///////////////////////////////////////////////////
// respond to a change in click location on the HORIZ

void GuiManager::_horizLocationClicked(double xkm, double ykm, 
                                       const RadxRay *closestRay)

{

  cerr << "1111111111 horizLocationClicked, xkm, km: " << xkm << ", " << ykm << endl;
  
  // find the relevant ray
  // ignore closest ray sent in
  
  double azDeg = 0.0;
  if (xkm != 0 || ykm != 0) {
    azDeg = atan2(xkm, ykm) * RAD_TO_DEG;
    if (azDeg < 0) {
      azDeg += 360.0;
    }
  }
  if (_params.debug) {
    cerr << "    azDeg = " << azDeg << endl;
  }
  
  int rayIndex = ((int) (azDeg * RayLoc::RAY_LOC_RES)) % RayLoc::RAY_LOC_N;
  if (_params.debug) {
    cerr << "    rayIndex = " << rayIndex << endl;
  }
  
  const RadxRay *ray = _rayLoc[rayIndex].ray;
  if (ray == NULL) {
    // no ray data yet
    if (_params.debug) {
      cerr << "    No ray data yet..." << endl;
      cerr << "      active = " << _rayLoc[rayIndex].active << endl;
      cerr << "      startIndex = " << _rayLoc[rayIndex].startIndex << endl;
      cerr << "      endIndex = " << _rayLoc[rayIndex].endIndex << endl;
    }
    return;
  }

  _locationClicked(xkm, ykm, ray);

}

///////////////////////////////////////////////////
// respond to a change in click location on the VERT

void GuiManager::_vertLocationClicked(double xkm, double ykm, 
                                      const RadxRay *closestRay)
  
{

  _locationClicked(xkm, ykm, closestRay);

}

////////////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void GuiManager::_locationClicked(double xkm, double ykm,
                                  const RadxRay *ray)
  
{

  if (_params.debug) {
    cerr << "*** Entering GuiManager::_locationClicked()" << endl;
  }
  
  double rangeKm = sqrt(xkm * xkm + ykm * ykm);
  int gateNum = (int) 
    ((rangeKm - ray->getStartRangeKm()) / ray->getGateSpacingKm() + 0.5);

  if (gateNum < 0 || gateNum >= (int) ray->getNGates()) {
    //user clicked outside of ray
    return;
  }

  if (_params.debug) {
    cerr << "Clicked on location: xkm, ykm: " << xkm << ", " << ykm << endl;
    cerr << "  rangeKm, gateNum: " << rangeKm << ", " << gateNum << endl;
    cerr << "  az, el from ray: "
         << ray->getAzimuthDeg() << ", "
         << ray->getElevationDeg() << endl;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      ray->print(cerr);
    }
  }

  DateTime rayTime(ray->getTimeSecs());
  char text[256];
  snprintf(text, 256, "%.4d/%.2d/%.2d",
           rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateClicked->setText(text);

  snprintf(text, 256, "%.2d:%.2d:%.2d.%.3d",
           rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
           ((int) (ray->getNanoSecs() / 1000000)));
  _timeClicked->setText(text);
  
  _setText(text, sizeof(text), "%6.2f", ray->getElevationDeg());
  _elevClicked->setText(text);
  
  _setText(text, sizeof(text), "%6.2f", ray->getAzimuthDeg());
  _azClicked->setText(text);
  
  _setText(text, sizeof(text), "%d", gateNum);
  _gateNumClicked->setText(text);
  
  _setText(text, sizeof(text), "%6.2f", rangeKm);
  _rangeClicked->setText(text);

  if (_radarAltKm > -9990) {
    double gateHtKm = _beamHt.computeHtKm(ray->getElevationDeg(), rangeKm);
    _setText(text, sizeof(text), "%6.2f (km)", gateHtKm);
    _altitudeClicked->setText(text);
  }

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setSelectValue(-9999.0);
    _fields[ii]->setDialogText("----");
  }

  vector<RadxField *> rflds = ray->getFields();
  for (size_t ifield = 0; ifield < rflds.size(); ifield++) {
    const RadxField *field = rflds[ifield];
  
    const string fieldName = field->getName();
    if (fieldName.size() == 0) {
      continue;
    }
    Radx::fl32 *data = (Radx::fl32 *) field->getData();
    double val = data[gateNum];
    const string fieldUnits = field->getUnits();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Field name, selected name: "
	   << fieldName << ", "
	   << _selectedName << endl;
    }
    if (fieldName == _selectedName) {
      char text[128];
      if (fabs(val) < 10000) {
        snprintf(text, 128, "%g %s", val, fieldUnits.c_str());
      } else {
        snprintf(text, 128, "%g %s", -9999.0, fieldUnits.c_str());
      }
      _valueLabel->setText(text);
    }
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Field name, units, val: "
	   << field->getName() << ", "
	   << field->getUnits() << ", "
	   << val << endl;
    }
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      if (_fields[ii]->getName() == fieldName) {
	_fields[ii]->setSelectValue(val);
        char text[128];
        if (fabs(val) > 10000) {
          snprintf(text, 128, "----");
        } else if (fabs(val) > 10) {
          snprintf(text, 128, "%.2f", val);
        } else {
          snprintf(text, 128, "%.3f", val);
        }
        _fields[ii]->setDialogText(text);
      }
    }

  } // ifield

  // update the status panel
  
  _updateStatusPanel(ray);

  // write the click location to FMQ

  _writeClickPointXml2Fmq(ray, rangeKm, gateNum);
  
}

#endif
  
//////////////////////////////////////////////
// get size of table widget

QSize GuiManager::_getTableWidgetSize(QTableWidget *t,
                                      bool includeVertHeader,
                                      bool includeHorizHeader)
{
  int w = 4;
  if (includeVertHeader) {
    w += t->verticalHeader()->width();
  }
  for (int i = 0; i < t->columnCount(); i++) {
    w += t->columnWidth(i);
  }
  int h = 4;
  if (includeHorizHeader) {
    h += t->horizontalHeader()->height();
  }
  for (int i = 0; i < t->rowCount(); i++) {
    h += t->rowHeight(i);
  }
  return QSize(w, h);
}

//////////////////////////////////////////////
// create the field menu

void GuiManager::_createFieldMenu()
{

  // top-level
  
  _fieldMenu = new QDialog(this);
  _fieldMenu->setWindowTitle("Select active field");
  QPoint pos(0,0);
  _fieldMenu->move(pos);
  // _fieldMenu->show();

  // layout
  
  QBoxLayout *fieldMenuLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, _fieldMenu);
  fieldMenuLayout->setSpacing(0);
  
  // panel
  
  _fieldMenuPanel = new QFrame(_fieldMenu);
  fieldMenuLayout->addWidget(_fieldMenuPanel, Qt::AlignCenter);
  _fieldMenuLayout = new QVBoxLayout;
  _fieldMenuPanel->setLayout(_fieldMenuLayout);

  // table
  
  _fieldTable = new QTableWidget(_fieldMenu);
  _fieldMenuLayout->addWidget(_fieldTable);

  int ncols = _params.num_field_menu_cols;
  int nfields = _params.fields_n;
  int nrows = (int) ((double) (nfields - 1) / ncols) + 1;
  
  _fieldTable->setRowCount(nrows);
  _fieldTable->setColumnCount(ncols);
  
  for (int irow = 0; irow < nrows; irow++) {
    // is this row active?
    for (int icol = 0; icol < ncols; icol++) {
      int fieldNum = irow * ncols + icol;
      FieldTableItem *item = new FieldTableItem(this, "");
      item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      if (fieldNum < _params.fields_n) {
        if (strlen(_params._fields[fieldNum].group_name) > 0) {
          item->setText(_params._fields[fieldNum].button_label);
          item->setFieldIndex(fieldNum);
          if (strlen(_params._fields[fieldNum].button_label) == 0) {
            // gray out
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setBackground(Qt::lightGray);
            item->setForeground(Qt::gray);
          }
        }
      }
      item->setFieldParams(&_params._fields[fieldNum]);
      _fieldTable->setItem(irow, icol, item);
    } // icol
  } // irow

  _fieldTable->verticalHeader()->setVisible(false);
  _fieldTable->horizontalHeader()->setVisible(false);

  // set the column widths, for the strings they contain

  _fieldTable->resizeColumnsToContents();
  _fieldTable->resizeRowsToContents();
  _fieldTable->adjustSize();
  _fieldTable->setAlternatingRowColors(true);
  _fieldTable->setSelectionMode(QAbstractItemView::SingleSelection);
  
  // initialize

  _fieldTable->setCurrentCell(0, 0);

  // set the size
  
  _fieldTable->setMaximumSize(_getTableWidgetSize(_fieldTable, false, false));
  _fieldTable->setMinimumSize(_fieldTable->maximumSize());

  connect(_fieldTable, SIGNAL(cellClicked(const int, const int)),
          this, SLOT(_fieldTableCellClicked(const int, const int)));
  
}

/////////////////////////////////////
// show the field menu

void GuiManager::_showFieldMenu()
{

  if (_fieldMenu) {
    if (_fieldMenu->isVisible()) {
      _fieldMenu->setVisible(false);
    } else {
      _fieldMenu->setVisible(true);
      _fieldMenu->raise();
      _placeFieldMenu();
    }
  }

}

/////////////////////////////////////
// place the field menu

void GuiManager::_placeFieldMenu()
{
  if (!_fieldMenu) {
    return;
  }
  
  if (_fieldMenuPlaced) {
    return;
  }
  
  int margin = (frameGeometry().width() - geometry().width()) / 2;
  int titleBarHt = frameGeometry().height() - geometry().height() - margin;
  QPoint pos;
  pos.setX(x() + (frameGeometry().width()));
  pos.setY(y() + titleBarHt);
  _fieldMenu->move(pos);

  _fieldMenuPlaced = true;

}

void GuiManager::_fieldTableCellClicked(int row, int col)
{

  if (_fieldTableRow == _fieldTable->currentRow() &&
      _fieldTableCol == _fieldTable->currentColumn()) {
    // no change
    _fieldHasChanged = false;
    return;
  }

  _prevFieldTableCol = _fieldTableCol;
  _prevFieldTableRow = _fieldTableRow;
  
  _fieldTableCol = col;
  _fieldTableRow = row;
  
  const FieldTableItem *item =
    (const FieldTableItem *) _fieldTable->item(row, col);

  _prevFieldNum = _fieldNum;
  _fieldNum = item->getFieldIndex();
  
  _gd.prev_field = _gd.h_win.page;
  _setField(_fieldNum);
  _gd.mread[_fieldNum]->h_data_valid = 0;

  if (_params.debug) {
    const Params::field_t *fparams = item->getFieldParams();
    cerr << "Changing to field: " << fparams->button_label << endl;
    cerr << "              url: " << fparams->url << endl;
    cerr << "         fieldNum: " << _fieldNum << endl;
    cerr << "      field_label: " << _gd.mread[_fieldNum]->field_label << endl;
    cerr << "      button_name: " << _gd.mread[_fieldNum]->button_name << endl;
    cerr << "      legend_name: " << _gd.mread[_fieldNum]->legend_name << endl;
  }

  // _gd.redraw_horiz = true;
  _gd.field_has_changed = true;
  _gd.selected_field = _fieldNum;
  _gd.h_win.page = _fieldNum;

  _fieldHasChanged = true;
  
}

/*************************************************************************
 * Set proper flags which switching fields 
 */

void GuiManager::_setField(int value)
{

  static int last_page = 0;
  static int last_value = 0;
  static int cur_value = 0;
  
  if(value < 0) {
    int tmp = last_page;
    last_page = _gd.h_win.page;
    _gd.h_win.page = tmp;
    _gd.v_win.page = tmp;
    tmp = cur_value;
    cur_value = last_value;
    value = last_value;
    last_value = tmp;
  } else {
    last_page = _gd.h_win.page;
    last_value = cur_value;
    cur_value = value;
    _gd.h_win.page = _gd.field_index[value];
    _gd.v_win.page = _gd.field_index[value];
  }
  
  for(int i=0; i < _gd.num_datafields; i++) {
    if(_gd.mread[i]->auto_render == 0) _gd.h_win.redraw_flag[i] = 1;
  }
  
  if(_gd.mread[_gd.h_win.page]->auto_render && 
     _gd.h_win.page_pdev[_gd.h_win.page] != 0 &&
     _gd.h_win.redraw_flag[_gd.h_win.page] == 0) {

#ifdef NOTNOW
    save_h_movie_frame(_gd.movie.cur_frame,
                       _gd.h_win.page_pdev[_gd.h_win.page],
                       _gd.h_win.page);
#endif
    
  }
  
  for(int i=0; i < Constants::MAX_FRAMES; i++) {
    _gd.movie.frame[i].redraw_horiz = 1;
  }
  
  if(_gd.movie.movie_on ) {
    // reset_data_valid_flags(1,0);
  }
  
}


/////////////////////////////////////
// show the time controller dialog

void GuiManager::_showTimeControl()
{

  if (_timeControl) {
    if (_timeControl->isVisible()) {
      _timeControl->setVisible(false);
    } else {
      _timeControl->setVisible(true);
      _timeControl->raise();
      _placeTimeControl();
    }
  }

}

/////////////////////////////////////
// place the time controller dialog

void GuiManager::_placeTimeControl()
{

  if (!_timeControl) {
    return;
  }

  if (_timeControl->x() == 0 && _timeControl->y() == 0) {
    // not yet sized and placed
    return;
  }
  
  if (_timeControlPlaced) {
    return;
  }
  
  int margin = (frameGeometry().width() - geometry().width()) / 2;
  int titleBarHt = frameGeometry().height() - geometry().height() - margin;
  
  QPoint pos;
  pos.setX(x() + margin);
  pos.setY(y() + frameGeometry().height() + titleBarHt);
  _timeControl->move(pos);
  _timeControlPlaced = true;

  _timeControl->resize(width(), _timeControl->height());
  
}

////////////////////////////////////////////////////////
// BoundaryEditor circle (radius) slider has changed value

void GuiManager::_circleRadiusSliderValueChanged(int value)
{
  //returns true if existing circle was resized with this new radius
  // if (BoundaryPointEditor::Instance()->setCircleRadius(value)){
  //   _horiz->update();
  // }
}

// BoundaryEditor brush (size) slider has changed value

void GuiManager::_brushRadiusSliderValueChanged(int value)
{
  // BoundaryPointEditor::Instance()->setBrushRadius(value);
}

// set the directory (_boundaryDir) into which boundary
// files will be read/written for current radar file (_openFilePath)

void GuiManager::setBoundaryDir()
{
  // if (!_openFilePath.empty()) {
  //   _boundaryDir =
  //     BoundaryPointEditor::Instance()->getBoundaryDirFromRadarFilePath
  //     (BoundaryPointEditor::Instance()->getRootBoundaryDir(), _openFilePath);
  // } else {
  //   _boundaryDir = BoundaryPointEditor::Instance()->getRootBoundaryDir();
  // }
}

/////////////////////////////////////
// clear display widgets

void GuiManager::_clear()
{
  if (_horiz) {
    _horiz->clear();
  }
  if (_vert) {
    _vert->clear();
  }
}

/////////////////////////////////////
// set archive mode

void GuiManager::setArchiveMode(bool state)
{

  // _setSweepPanelVisibility();
  
  if (_horiz) {
    _horiz->setArchiveMode(state);
  }
  if (_vert) {
    _vert->setArchiveMode(state);
  }

  if (state) {
    if (!_archiveMode) {
      _archiveMode = true;
      _activateArchiveRendering();
      // loadArchiveFileList();
    }
  } else {
    if (_archiveMode) {
      _archiveMode = false;
      _activateRealtimeRendering();
    }
  }

}

/////////////////////////////////////
// activate realtime rendering

void GuiManager::_activateRealtimeRendering()
{
  // _nGates = 1000;
  // _maxRangeKm = 1000;
  _clear();
  if (_horiz) {
    _horiz->activateRealtimeRendering();
  }
  if (_vert) {
    _vert->activateRealtimeRendering();
  }
}

/////////////////////////////////////
// activate archive rendering

void GuiManager::_activateArchiveRendering()
{
  _clear();
  if (_horiz) {
    _horiz->activateArchiveRendering();
  }
  if (_vert) {
    _vert->activateArchiveRendering();
  }
}

/////////////////////////////////////////////////////
// creating image files in realtime mode

void GuiManager::_createRealtimeImageFiles()
{

  // int interval = _params.images_schedule_interval_secs;
  // int delay = _params.images_schedule_delay_secs;
  // time_t latestRayTime = _readerRayTime.utime();
  
  // initialize the schedule if required

  if (_imagesScheduledTime.utime() == 0) {
    // if (latestRayTime == 0) {
    //   // no data yet
    //   return;
    // }
    // time_t nextUtime = ((latestRayTime / interval) + 1) * interval;
    // _imagesScheduledTime.set(nextUtime + delay);
    // if (_params.debug) {
    //   cerr << "Next scheduled time for images: " 
    //        << _imagesScheduledTime.asString() << endl;
    // }
  }
  
  // if (_readerRayTime > _imagesScheduledTime) {
    
  //   // create images

  //   _timeControl->setArchiveEndTime(_imagesScheduledTime - delay);
  //   _timeControl->setArchiveStartTime
  //     (_timeControl->getArchiveEndTime() - _imagesScanIntervalSecs);
  //   _createImageFilesAllSweeps();

  //   // set next scheduled time
    
  //   time_t nextUtime = ((latestRayTime / interval) + 1) * interval;
  //   _imagesScheduledTime.set(nextUtime + delay);
  //   if (_params.debug) {
  //     cerr << "Next scheduled time for images: "
  //          << _imagesScheduledTime.asString() << endl;
  //   }

  // }

}

/////////////////////////////////////////////////////
// creating image files in archive mode

void GuiManager::_createArchiveImageFiles()
{

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_THEN_EXIT) {
    
    if (_archiveFileList.size() > 0) {

      // using input file list to drive image generation

      while (_archiveFileList.size() > 0) {
        _createImageFilesAllLevels();
        _archiveFileList.erase(_archiveFileList.begin(), 
                               _archiveFileList.begin() + 1);
      }
      
    } else {
      
      // using archive time to drive image generation
      
      while (_timeControl->getStartTime() <= _imagesEndTime) {
        _createImageFilesAllLevels();
        // _timeControl->setStartTime(_timeControl->getStartTime() +
        //                            _imagesScanIntervalSecs);
      }
      
    }
    
  } else if (_params.images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    
    for (DateTime stime = _imagesStartTime;
         stime <= _imagesEndTime;
         stime += _params.images_schedule_interval_secs) {
      
      // _timeControl->setStartTime(stime);
      // _timeControl->setEndTime(_timeControl->getStartTime() +
      //                          _imagesScanIntervalSecs);
      
      _createImageFilesAllLevels();
      
    } // stime
    
  } // if (_params.images_creation_mode ...

}

/////////////////////////////////////////////////////
// creating one image per field, for each level

void GuiManager::_createImageFilesAllLevels()
{
  
  // if (_params.images_set_sweep_index_list) {
    
  //   for (int ii = 0; ii < _params.images_sweep_index_list_n; ii++) {
  //     int index = _params._images_sweep_index_list[ii];
  //     if (index >= 0 && index < (int) _vol.getNSweeps()) {
  //       // _sweepManager.setFileIndex(index);
  //       _createImageFiles();
  //     }
  //   }
    
  // } else {
    
  //   for (size_t index = 0; index < _vol.getNSweeps(); index++) {
  //     // _sweepManager.setFileIndex(index);
  //     _createImageFiles();
  //   }
    
  // }

}

/////////////////////////////////////////////////////
// creating one image per field

void GuiManager::_createImageFiles()
{

  if (_params.debug) {
    cerr << "GuiManager::_createImageFiles()" << endl;
  }

  PMU_auto_register("createImageFiles");

  // plot the data

  // _horiz->setStartOfSweep(true);
  // _vert->setStartOfSweep(true);
  // _handleArchiveData();

  // set times from plots

  if (_vertMode) {
    _plotStartTime = _vert->getPlotStartTime();
    _plotEndTime = _vert->getPlotEndTime();
  } else {
    _plotStartTime = _horiz->getPlotStartTime();
    _plotEndTime = _horiz->getPlotEndTime();
  }

#ifdef NOTYET
  
  // save current field

  int fieldNum = _fieldNum;
  
  // loop through fields

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    
    // select field
    
    _changeField(ii, false);
    
    // create plot
    
    if (_params.debug) {
      cerr << "Creating image for field: " << getSelectedFieldLabel() << endl;
    }
    
    // save image for plot
    
    _saveImageToFile(false);

  }

  // change field back

  _changeField(fieldNum, false);

#endif
  
  if (_params.debug) {
    cerr << "Done creating image files" << endl;
  }

}

string GuiManager::_getOutputPath(bool interactive, string &outputDir, string fileExt)
{
  // set times from plots
  if (_vertMode) {
    _plotStartTime = _vert->getPlotStartTime();
    _plotEndTime = _vert->getPlotEndTime();
  } else {
    _plotStartTime = _horiz->getPlotStartTime();
    _plotEndTime = _horiz->getPlotEndTime();
  }

  // compute output dir
  outputDir = _params.images_output_dir;
  char dayStr[1024];
  if (_params.images_write_to_day_dir)
  {
    snprintf(dayStr, 1024, "%.4d%.2d%.2d", _plotStartTime.getYear(), _plotStartTime.getMonth(), _plotStartTime.getDay());
    outputDir += PATH_DELIM;
    outputDir += dayStr;
  }

  // make sure output dir exists

  if (ta_makedir_recurse(outputDir.c_str())) {
    string errmsg("Cannot create output dir: " + outputDir);
    cerr << "ERROR - GuiManager::_saveImageToFile()" << endl;
    cerr << "  " << errmsg << endl;
    if (interactive) {
      QMessageBox::critical(this, "Error", errmsg.c_str());
    }
    return(NULL);
  }

  // compute file name

  string fileName;

  // category

  if (strlen(_params.images_file_name_category) > 0) {
    fileName += _params.images_file_name_category;
  }

  // platform

  if (strlen(_params.images_file_name_platform) > 0) {
    fileName += _params.images_file_name_delimiter;
    fileName += _params.images_file_name_platform;
  }

  // time

  if (_params.images_include_time_part_in_file_name) {
    fileName += _params.images_file_name_delimiter;
    char timeStr[1024];
    if (_params.images_include_seconds_in_time_part) {
      snprintf(timeStr, 1024, "%.4d%.2d%.2d%.2d%.2d%.2d",
               _plotStartTime.getYear(),
               _plotStartTime.getMonth(),
               _plotStartTime.getDay(),
               _plotStartTime.getHour(),
               _plotStartTime.getMin(),
               _plotStartTime.getSec());
    } else {
      snprintf(timeStr, 1024, "%.4d%.2d%.2d%.2d%.2d",
               _plotStartTime.getYear(),
               _plotStartTime.getMonth(),
               _plotStartTime.getDay(),
               _plotStartTime.getHour(),
               _plotStartTime.getMin());
    }
    fileName += timeStr;
  }

  // field label

  if (_params.images_include_field_label_in_file_name) {
    fileName += _params.images_file_name_delimiter;
    fileName += getSelectedFieldLabel();
  }
    
  // scan Id

  // if (_params.images_include_scan_id_in_file_name) {
  //   fileName += _params.images_file_name_delimiter;
  //   fileName += std::to_string(_vol.getScanId());
  // }

  // scan type

  // if (_params.images_include_scan_type_in_file_name) {
  //   fileName += _params.images_file_name_delimiter;
  //   string scanType = "SUR";
  //   if (_vol.getSweeps().size() > 0) {
  //     Radx::SweepMode_t predomSweepMode = _vol.getPredomSweepMode();
  //     scanType = Radx::sweepModeToShortStr(predomSweepMode);
  //   }
  //   fileName += scanType;
  // }

  // extension

  fileName += ".";
  fileName += fileExt;

  // compute output path

  string outputPath(outputDir);
  outputPath += PATH_DELIM;
  outputPath += fileName;

  return(outputPath);
}

/////////////////////////////////////////////////////
// save image to file
// If interactive is true, use dialog boxes to indicate errors or report
// where the image was saved.

void GuiManager::_saveImageToFile(bool interactive)
{

  cerr << "SSSSSSSSSSSSSSSSSSSSSSSSSSSSS" << endl;
  return;
  
  // create image
  QPixmap pixmap;
  if (_vertMode)
    pixmap = _vert->grab();
  else
    pixmap = _horiz->grab();
  QImage image = pixmap.toImage();

  string outputDir;
  string outputPath = _getOutputPath(interactive, outputDir, _params.images_file_name_extension);

  // write the file
  if (!image.save(outputPath.c_str())) {
    string errmsg("Cannot save image to file: " + outputPath);
    cerr << "ERROR - GuiManager::_saveImageToFile()" << endl;
    cerr << "  " << errmsg << endl;
    if (interactive) {
      QMessageBox::critical(this, "Error", errmsg.c_str());
    }
    return;
  }

  if (interactive) {
    string infoMsg("Saved image to file " + outputPath);
    QMessageBox::information(this, "Image Saved", infoMsg.c_str());
  }

  if (_params.debug) {
    cerr << "==>> saved image to file: " << outputPath << endl;
  }

  // write latest data info

  if (_params.images_write_latest_data_info) {
    
    DsLdataInfo ldataInfo(_params.images_output_dir);
    
    string relPath;
    RadxPath::stripDir(_params.images_output_dir, outputPath, relPath);
    
    if(_params.debug) {
      ldataInfo.setDebug();
    }
    ldataInfo.setLatestTime(_plotStartTime.utime());
    ldataInfo.setWriter("Lucid");
    ldataInfo.setDataFileExt(_params.images_file_name_extension);
    ldataInfo.setDataType(_params.images_file_name_extension);
    ldataInfo.setRelDataPath(relPath);
    
    if(ldataInfo.write(_plotStartTime.utime())) {
      cerr << "ERROR - GuiManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.images_write_latest_data_info)

}



void GuiManager::ShowContextMenu(const QPoint &pos) {
  _horiz->ShowContextMenu(pos /*, &_vol*/);
}


#ifdef NOTNOW
// Creates the boundary editor dialog and associated event slots
void GuiManager::_createBoundaryEditorDialog()
{
  _boundaryEditorDialog = new QDialog(this);
  _boundaryEditorDialog->setMaximumHeight(368);
  _boundaryEditorDialog->setWindowTitle("Boundary Editor");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignRight(Qt::AlignRight);

  _boundaryEditorDialogLayout = new QGridLayout(_boundaryEditorDialog);
  _boundaryEditorDialogLayout->setVerticalSpacing(4);

  int row = 0;
  _boundaryEditorInfoLabel = new QLabel("Boundary Editor allows you to select\nan area of your radar image", _boundaryEditorDialog);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorInfoLabel, row, 0, 1, 2, alignCenter);
  
  _boundaryEditorDialogLayout->addWidget(new QLabel(" ", _boundaryEditorDialog), ++row, 0, 1, 2, alignCenter);

  QLabel *toolsCaption = new QLabel("Editor Tools:", _boundaryEditorDialog);
  _boundaryEditorDialogLayout->addWidget(toolsCaption, ++row, 0, 1, 2, alignCenter);

  _boundaryEditorPolygonBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorPolygonBtn->setMaximumWidth(130);
  _boundaryEditorPolygonBtn->setText(" Polygon");
  _boundaryEditorPolygonBtn->setIcon(QIcon("images/polygon.png"));
  _boundaryEditorPolygonBtn->setCheckable(TRUE);
  _boundaryEditorPolygonBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorPolygonBtn, ++row, 0);
  connect(_boundaryEditorPolygonBtn, &QPushButton::clicked,
          this, &GuiManager::polygonBtnBoundaryEditorClick);

  _boundaryEditorCircleBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorCircleBtn->setMaximumWidth(130);
  _boundaryEditorCircleBtn->setText(" Circle  ");
  _boundaryEditorCircleBtn->setIcon(QIcon("images/circle.png"));
  _boundaryEditorCircleBtn->setCheckable(TRUE);
  _boundaryEditorCircleBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorCircleBtn, ++row, 0);
  connect(_boundaryEditorCircleBtn, &QPushButton::clicked,
          this, &GuiManager::circleBtnBoundaryEditorClick());

  _circleRadiusSlider = new QSlider(Qt::Horizontal);
  _circleRadiusSlider->setFocusPolicy(Qt::StrongFocus);
  _circleRadiusSlider->setTracking(true);
  _circleRadiusSlider->setSingleStep(1);
  _circleRadiusSlider->setPageStep(0);
  _circleRadiusSlider->setFixedWidth(100);
  _circleRadiusSlider->setToolTip("Set the circle radius");
  _circleRadiusSlider->setMaximumWidth(180);
  _circleRadiusSlider->setValue(50);
  _circleRadiusSlider->setMinimum(8);
  _circleRadiusSlider->setMaximum(200);
  _boundaryEditorDialogLayout->addWidget(_circleRadiusSlider, row, 1);
  connect(_circleRadiusSlider, SIGNAL(valueChanged(int)),
          this, &GuiManager::_circleRadiusSliderValueChanged(int));

  _boundaryEditorBrushBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorBrushBtn->setMaximumWidth(130);
  _boundaryEditorBrushBtn->setText(" Brush ");
  _boundaryEditorBrushBtn->setIcon(QIcon("images/brush.png"));
  _boundaryEditorBrushBtn->setCheckable(TRUE);
  _boundaryEditorBrushBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorBrushBtn, ++row, 0);
  connect(_boundaryEditorBrushBtn, &QPushButton::clicked,
          this, &GuiManager::brushBtnBoundaryEditorClick());

  _brushRadiusSlider = new QSlider(Qt::Horizontal);
  _brushRadiusSlider->setFocusPolicy(Qt::StrongFocus);
  _brushRadiusSlider->setTracking(true);
  _brushRadiusSlider->setSingleStep(1);
  _brushRadiusSlider->setPageStep(0);
  _brushRadiusSlider->setFixedWidth(100);
  _brushRadiusSlider->setToolTip("Set the smart brush radius");
  _brushRadiusSlider->setMaximumWidth(180);
  _brushRadiusSlider->setValue(18);
  _brushRadiusSlider->setMinimum(12);
  _brushRadiusSlider->setMaximum(75);
  _boundaryEditorDialogLayout->addWidget(_brushRadiusSlider, row, 1);
  connect(_brushRadiusSlider, SIGNAL(valueChanged(int)),
          this, &GuiManager::_brushRadiusSliderValueChanged(int));

  _boundaryEditorBrushBtn->setChecked(TRUE);
  _boundaryEditorDialogLayout->addWidget(new QLabel(" ", _boundaryEditorDialog),
                                         ++row, 0, 1, 2, alignCenter);
  
  _boundaryEditorList = new QListWidget(_boundaryEditorDialog);
  QListWidgetItem *newItem5 = new QListWidgetItem;
  newItem5->setText("Boundary5 <none>");
  _boundaryEditorList->insertItem(0, newItem5);
  QListWidgetItem *newItem4 = new QListWidgetItem;
  newItem4->setText("Boundary4 <none>");
  _boundaryEditorList->insertItem(0, newItem4);
  QListWidgetItem *newItem3 = new QListWidgetItem;
  newItem3->setText("Boundary3 <none>");
  _boundaryEditorList->insertItem(0, newItem3);
  QListWidgetItem *newItem2 = new QListWidgetItem;
  newItem2->setText("Boundary2 <none>");
  _boundaryEditorList->insertItem(0, newItem2);
  QListWidgetItem *newItem1 = new QListWidgetItem;
  newItem1->setText("Boundary1");
  _boundaryEditorList->insertItem(0, newItem1);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorList, ++row, 0, 1, 2);
  connect(_boundaryEditorList, SIGNAL(itemClicked(QListWidgetItem*)),
          this, &GuiManager::onBoundaryEditorListItemClicked(QListWidgetItem*));

  // horizontal layout contains the "Clear", "Help", and "Save" buttons
  QHBoxLayout *hLayout = new QHBoxLayout;
  _boundaryEditorDialogLayout->addLayout(hLayout, ++row, 0, 1, 2);
  
  _boundaryEditorClearBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorClearBtn->setText("Clear");
  hLayout->addWidget(_boundaryEditorClearBtn);
  connect(_boundaryEditorClearBtn, &QPushButton::clicked,
          this, &GuiManager::clearBoundaryEditorClick());

  _boundaryEditorHelpBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorHelpBtn->setText("Help");
  hLayout->addWidget(_boundaryEditorHelpBtn);
  connect(_boundaryEditorHelpBtn, &QPushButton::clicked,
          this, &GuiManager::helpBoundaryEditorClick());
  
  _boundaryEditorSaveBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorSaveBtn->setText("Save");
  hLayout->addWidget(_boundaryEditorSaveBtn);

}
#endif

/////////////////////////////////////////////////////////
// check for zoom change

bool GuiManager::_checkForZoomChange()
{
  if (_zoomXy != _prevZoomXy) {
    _prevZoomXy = _zoomXy;
    return true;
  }
  return false;
}

/////////////////////////////////////////////////////////
// handle first time event

void GuiManager::_handleFirstTimerEvent()
{

  _horiz->resize(_horizFrame->width(), _horizFrame->height());
  _horiz->updatePixelScales();

}

/////////////////////////////////////////////////////////
// read click point from FMQ - i.e. from another app

void GuiManager::_readClickPoint()
{

#ifdef NOTNOW
  bool gotNew = false;
  
  if (_readClickPointFmq(gotNew) == 0) {
    if (gotNew) {
      if (_params.debug) {
        cerr << "====>> gotNewClickInfo" << endl;
      }
      if (_horiz) {
        _horiz->setClickPoint(_clickPointFmq.getAzimuth(),
                              _clickPointFmq.getElevation(),
                              _clickPointFmq.getRangeKm());
      }
    }
  }
#endif
  
}

  
/**********************************************************************
 * HANDLE_CLIENT_EVENT: 
 */

void GuiManager::_handleClientEvent()
{

#ifdef NOTNOW
  
  time_t clock;
  
  if(_gd.debug1) {
    fprintf(stderr,
            "Found Client Event: %d, Args: %s\n",
            _gd.coord_expt->client_event,
            _gd.coord_expt->client_args);
  }
  
  switch(_gd.coord_expt->client_event) {
    case NEW_MDV_AVAIL:
      remote_new_mdv_avail(_gd.coord_expt->client_args);
      break;
      
    case NEW_SPDB_AVAIL:
      remote_new_spdb_avail(_gd.coord_expt->client_args);
      break;

    case RELOAD_DATA:
      // invalidate_all_data();
      // set_redraw_flags(1,1);
      break;

    case SET_FRAME_NUM:
      int	frame;
      if((sscanf(_gd.coord_expt->client_args,"%d",&frame)) == 1) {
        /* Sanity Check */
        if(frame <= 0 ) frame = 1;
        if(frame > _gd.movie.num_frames ) frame = _gd.movie.num_frames ;

        _gd.movie.cur_frame = frame - 1;
      } else {
        fprintf(stderr,"Invalid SET_FRAME_NUM: Args: %s\n",_gd.coord_expt->client_args);
      }
      break;

    case SET_NUM_FRAMES:
      int	nframes;
      if((sscanf(_gd.coord_expt->client_args,"%d",&nframes)) == 1) {
        _setEndFrame(nframes);
      } else {
        fprintf(stderr,"Invalid SET_NUM_FRAMES: Args: %s\n",_gd.coord_expt->client_args);
      }
      break;

    case SET_REALTIME:
      _gd.movie.mode = REALTIME_MODE;
      _gd.movie.cur_frame = _gd.movie.num_frames -1;
      _gd.movie.end_frame = _gd.movie.num_frames -1;
      clock = time(0);
      _gd.movie.start_time = clock - (time_t) ((_gd.movie.num_frames -1) *
                                              _gd.movie.time_interval_mins * 60.0);
      _gd.movie.start_time -= (_gd.movie.start_time % _gd.movie.round_to_seconds);
      _gd.coord_expt->runtime_mode = RUNMODE_REALTIME;	
      _gd.coord_expt->time_seq_num++;

      // reset_time_points();
      // invalidate_all_data();
      // set_redraw_flags(1,1);

      // Set forecast and past time choosers to "now"
      // xv_set(_gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
      // xv_set(_gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);
      // Set movie mode widget to REALTIME 
      // xv_set(_gd.movie_pu->movie_type_st,PANEL_VALUE,0,NULL);

      break;

    case SET_TIME:
      UTIMstruct ts;
      time_t interest_time;

      if((sscanf(_gd.coord_expt->client_args,"%ld %ld %ld %ld %ld %ld",
                 &ts.year,&ts.month,&ts.day, &ts.hour,&ts.min,&ts.sec)) == 6) {
		
        interest_time = UTIMdate_to_unix(&ts);

        _setDisplayTime(interest_time);
        // invalidate_all_data();
        // set_redraw_flags(1,1);
      } else {
        fprintf(stderr,"Invalid SET_TIME Args: %s\n",_gd.coord_expt->client_args);
      }
      break;

    default: {}

  }

  // Reset  the command
  _gd.coord_expt->client_event = NO_MESSAGE;

#endif
  
}


/**********************************************************************
 * CHECK_FOR_EXPIRED_DATA: Check all data and determine if 
 * the data's expiration time has been exceeded - Mark it invalid if so.
 */

void GuiManager::_checkForExpiredData(time_t tm)
{
  int i;

  // Mark all data past the expiration data as invalid
  /* look thru primary data fields */
  for (i=0; i < _gd.num_datafields; i++) {
    /* if data has expired or field should be updated */
    if (_gd.mread[i]->h_mhdr.time_expire < tm ) {
      //        if(_gd.debug1) fprintf(stderr,"Field: %s expired at %s\n",
      //		 _gd.mread[i]->button_name,
      //		 asctime(gmtime(&((time_t) _gd.mread[i]->h_mhdr.time_expire))));
      _gd.mread[i]->h_data_valid = 0;
      _gd.mread[i]->v_data_valid = 0;
    }
  }

  /* Look through wind field data too */
  for (i=0; i < _gd.layers.num_wind_sets; i++ ) {
    if (_gd.layers.wind[i].active) {
      if (_gd.layers.wind[i].wind_u->h_mhdr.time_expire < tm) {
        _gd.layers.wind[i].wind_u->h_data_valid = 0;
        _gd.layers.wind[i].wind_u->v_data_valid = 0;
      }
      if (_gd.layers.wind[i].wind_v->h_mhdr.time_expire < tm) {
        _gd.layers.wind[i].wind_v->h_data_valid = 0;
        _gd.layers.wind[i].wind_v->v_data_valid = 0;
      }
      if (_gd.layers.wind[i].wind_w != NULL && _gd.layers.wind[i].wind_w->h_mhdr.time_expire < tm) {
        _gd.layers.wind[i].wind_w->h_data_valid = 0;
        _gd.layers.wind[i].wind_w->v_data_valid = 0;
      }
    }
  }
}

/**********************************************************************
 * _CHECKFORDATAUPDATES: Check all data and determine if 
 * new data has arrived - Mark it invalid if so.
 */

void GuiManager::_checkForDataUpdates(time_t tm)
{
  DmapAccess dmap;
  int i,j;
  char *start_ptr, *end_ptr;
  char dir_buf[MAX_PATH_LEN];

  if( strlen(_params.datamap_host) < 2 || dmap.reqAllInfo(_params.datamap_host) != 0) {

    // Force a reload of the data
    // reset_data_valid_flags(1,1);
    if (_gd.prod_mgr) {
      _gd.prod_mgr->reset_product_valid_flags();
      _gd.prod_mgr->reset_times_valid_flags();
    }

  } else {   // Got valid  data back from the Data Mapper

    int nsets = dmap.getNInfo();


    if(_gd.debug1) fprintf(stderr,"Found %d Datamapper info sets\n",nsets);

    /* look thru all data fields */

    for (i=0; i < _gd.num_datafields; i++) {

      // pull out dir from URL

      char tmpUrl[MAX_PATH_LEN];
      strncpy(tmpUrl, _gd.mread[i]->url.c_str(), MAX_PATH_LEN - 1);
      end_ptr = strrchr(tmpUrl, '&');
      if(end_ptr == NULL) continue;  // broken URL.
      
      start_ptr =  strrchr(tmpUrl, ':');
      if(start_ptr == NULL) {
	start_ptr =  tmpUrl; // Must be a local file/dir based URL
      } else {
	start_ptr++;  // Move up one character
      }

      strncpy(dir_buf,start_ptr,(size_t) (end_ptr - start_ptr + 1));
      dir_buf[(size_t) (end_ptr - start_ptr)] = '\0'; // Null terminate
	
      // Look through the data mapper info
      for(j = 0;  j < nsets; j++) {
	const DMAP_info_t &info = dmap.getInfo(j);

	// See if any data matches
	if(strstr(info.dir,dir_buf) != NULL) {
          // Note unix_time is signed (time_t)  and info.end_time is unsigned int
          if (_gd.mread[i]->h_date.utime() < (time_t) info.end_time) {
            _gd.mread[i]->h_data_valid = 0;
            _gd.mread[i]->v_data_valid = 0;
          }
	}
      }
    }
    
    for (i=0; i < _gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (_gd.layers.wind[i].active) {
	// pull out dir from URL

        char tmpUrl[MAX_PATH_LEN];
        strncpy(tmpUrl, _gd.mread[i]->url.c_str(), MAX_PATH_LEN - 1);
        
        char tmp2[MAX_PATH_LEN];
        strncpy(tmp2, _gd.layers.wind[i].wind_u->url.c_str(), MAX_PATH_LEN - 1);
	end_ptr = strrchr(tmp2,'&');
	if(end_ptr == NULL) continue;  // broken URL.
        
	start_ptr =  strrchr(tmp2,':');
	if(start_ptr == NULL) {
          start_ptr = tmpUrl; // Must be a local file/dir based URL
	} else {
          start_ptr++;  // Move up one character
	}

	strncpy(dir_buf,start_ptr,(size_t) (end_ptr - start_ptr + 1));
	dir_buf[(size_t) (end_ptr - start_ptr)] = '\0'; // Null terminate
	
	// Look through the data mapper info
	for(j = 0;  j < nsets; j++) {
          const DMAP_info_t &info = dmap.getInfo(j);
          // See if any data matches
          if(strstr(info.dir,dir_buf) != NULL) {
            // Check if that data is more current
            if (_gd.layers.wind[i].wind_u->h_date.utime() < (time_t) info.end_time) {
              _gd.layers.wind[i].wind_u->h_data_valid = 0;
              _gd.layers.wind[i].wind_u->v_data_valid = 0;
              _gd.layers.wind[i].wind_v->h_data_valid = 0;
              _gd.layers.wind[i].wind_v->v_data_valid = 0;
              if (_gd.layers.wind[i].wind_w != NULL) {
		_gd.layers.wind[i].wind_w->h_data_valid = 0;
		_gd.layers.wind[i].wind_w->v_data_valid = 0;
              }
            }
          }  // If a match
	} // For all data mapper info
      }   // If wind layer is active
    }     // For all wind layers

    _gd.prod_mgr->check_product_validity(tm, dmap);
  } // If data mapper info is availible
}

////////////////////////////////////////////////////////////////// 
// _checkWhatNeedsRendering:

void GuiManager::_checkWhatNeedsRendering(int frame_index)
{
  int i;
  
  // If data used to draw plan view is invalid
  // Indicate plan view image needs rerendering
  
  if (_gd.mread[_gd.h_win.page]->h_data_valid == 0 ||
      _gd.prod_mgr->num_products_invalid() > 0) {
    _gd.movie.frame[frame_index].redraw_horiz = 1;
    _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
  }

  for ( i=0; i < _gd.layers.num_wind_sets; i++ ) {
    // Look through wind field data too
    if (_gd.layers.wind[i].active) {
      if (_gd.layers.wind[i].wind_u->h_data_valid == 0) {
        _gd.movie.frame[frame_index].redraw_horiz = 1;
        _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
      if (_gd.layers.wind[i].wind_v->h_data_valid == 0) {
        _gd.movie.frame[frame_index].redraw_horiz = 1;
        _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
      if (_gd.layers.wind[i].wind_w != NULL &&
          _gd.layers.wind[i].wind_w->h_data_valid == 0) {
        _gd.movie.frame[frame_index].redraw_horiz = 1;
        _gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
    }
  }

  // Check overlay contours if active

  for(i= 0; i < Constants::NUM_CONT_LAYERS; i++) {
    if(_gd.layers.cont[i].active) {
      if(_gd.mread[_gd.layers.cont[i].field]->h_data_valid == 0) {
	_gd.movie.frame[frame_index].redraw_horiz = 1;
	_gd.h_win.redraw_flag[_gd.h_win.page] = 1;
      }
    }
  } 

  // If data used to draw cross section is invalid
  // Indicate pcross section image needs rerendering
  
  if (_gd.v_win.active ) {
    if(_gd.mread[_gd.v_win.page]->v_data_valid == 0)  {
      _gd.movie.frame[frame_index].redraw_vert = 1;
      _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
    }

    for ( i=0; i < _gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (_gd.layers.wind[i].active) {
	if (_gd.layers.wind[i].wind_u->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
	if (_gd.layers.wind[i].wind_v->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
	if (_gd.layers.wind[i].wind_w != NULL &&
            _gd.layers.wind[i].wind_w->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
      }
    }

    /* Check overlay contours if active */
    for(i= 0; i < Constants::NUM_CONT_LAYERS; i++) {
      if(_gd.layers.cont[i].active) {
	if(_gd.mread[_gd.layers.cont[i].field]->v_data_valid == 0) {
          _gd.movie.frame[frame_index].redraw_vert = 1;
          _gd.v_win.redraw_flag[_gd.v_win.page] = 1;
	}
      }
    } 
  }
}

///////////////////////////////////////////
// set text for GUI panels

void GuiManager::_setText(char *text,
                          size_t maxTextLen,
                          const char *format,
                          int val)
{
  if (abs(val) < 9999) {
    snprintf(text, maxTextLen, format, val);
  } else {
    snprintf(text, maxTextLen, format, -9999);
  }
}

void GuiManager::_setText(char *text,
                          size_t maxTextLen,
                          const char *format,
                          double val)
{
  if (fabs(val) < 9999) {
    snprintf(text, maxTextLen, format, val);
  } else {
    snprintf(text, maxTextLen, format, -9999.0);
  }
}

/////////////////////////////
// show data at click point

void GuiManager::_showClick()
{
#ifdef NOTNOW
  if (_clickReportDialog) {
    if (_clickReportDialog->isVisible()) {
      _clickReportDialog->setVisible(false);
    } else {
      if (_clickReportDialog->x() == 0 &&
          _clickReportDialog->y() == 0) {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y());
        _clickReportDialog->move(pos);
      }
      _clickReportDialog->setVisible(true);
      _clickReportDialog->raise();
    }
  }
#endif
}

/////////////////////////////////////////////////////
// howto help

void GuiManager::_howto()
{
  string text;
  text += "HOWTO HINTS FOR Lucid\n";
  text += "========================\n";
  text += "\n";
  text += "Use NUMBER keys to display RAW fields\n";
  text += "Use ALT-NUMBER keys to display FILTERED fields\n";
  text += "Hit '.' to toggle between the two latest fields\n";
  text += "\n";
  text += "You can also use the arrow keys to select different fields\n";
  text += "\n";
  text += "Click in main window to get field data at a point.\n";
  QMessageBox::about(this, tr("Howto dialog"), tr(text.c_str()));
}

void GuiManager::_about()
{

  //QMessageBox::about(this, tr("About Menu"),
  //tr("Lucid is an integrating Cartesian display for weather data."));

  string text;
  
  text += "Lucid is an integrating Cartesian display for weather data.\n\n";
  text += "Get help with Lucid ...  \n ";
  text += "\nReport an issue https://github.com/NCAR/lrose-core/issues \n ";
  text += "\nLucid version ... \n ";  
  text += "\nCopyright UCAR (c) 1990 - 2019  ";
  text += "\nUniversity Corporation for Atmospheric Research (UCAR)  ";  
  text += "\nNational Center for Atmospheric Research (NCAR)   ";  
  text += "\nBoulder, Colorado, USA ";  
  text += "\n\nBSD licence applies - redistribution and use in source and binary";  
  text += " forms, with or without modification, are permitted provided that";  
  text += " the following conditions are met: ";  
  text += "\n1) If the software is modified to produce derivative works,";  
  text += " such modified software should be clearly marked, so as not";  
  text += " to confuse it with the version available from UCAR. ";  
  text += "\n2) Redistributions of source code must retain the above copyright";  
  text += " notice, this list of conditions and the following disclaimer.";  
  text += "\n3) Redistributions in binary form must reproduce the above copyright";  
  text += " notice, this list of conditions and the following disclaimer in the";  
  text += " documentation and/or other materials provided with the distribution.";  
  text += "\n4) Neither the name of UCAR nor the names of its contributors,";  
  text += "if any, may be used to endorse or promote products derived from";  
  text += " this software without specific prior written permission.";  
  text += "\n\nDISCLAIMER: THIS SOFTWARE IS PROVIDED \"AS IS\" AND WITHOUT ANY EXPRESS ";  
  text += " OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED";  
  text += " WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.";  

  QMessageBox::about(this, tr("About Menu"), tr(text.c_str()));
}

//////////////////////////////////////////////////////////////////////////////
// Legacy - to be remove later
//////////////////////////////////////////////////////////////////////////////


/************************************************************************
 * TIMER_FUNC: This routine acts as the main branch point for most of the
 * threads involved in the display. This function gets called through
 * XView's notifier mechanism, which is built on sigalarm().
 * This is parameterized, and usually runs on a 5-100 msec interval.
 * First, any pending IO is handled.
 * Current time tickers, and the display's shmem communications are updated
 * Checks for out-of date images and expired Data are initiated periodically
 * Animation and Rendering are handled.
 * Finally Background Rendering is scheduled
 *
 */

void GuiManager::_autoCreateFunc()
{

    // handle image creation

  if (_params.images_auto_create) {

    // if we are just creating files in archive mode
    // and then exiting, do that now
    
    if ((_params.images_creation_mode ==
         Params::CREATE_IMAGES_THEN_EXIT) ||
        (_params.images_creation_mode ==
         Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE)) {
      _createArchiveImageFiles();
      close();
      return;
    }
    
    // if we are creating files in realtime mode, do that now
    
    if (_params.images_creation_mode ==
        Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
      // _handleRealtimeData();
      // _createRealtimeImageFiles();
      return;
    }
    
  } // if (_params.images_auto_create)

}


/*************************************************************************
 * SET_DISPLAY_TIME
 */

void GuiManager::_setDisplayTime(time_t utime)
{
  int i,j,found;
  time_t    last_time;
  time_t    target_time;

  movie_frame_t    tmp_frame[Constants::MAX_FRAMES];    /* info about each frame */

  last_time = _gd.movie.start_time;
     
  // Round to the nearest even interval 
  utime -= (utime % _gd.movie.round_to_seconds);

  // Already set to this time
  if(utime == _gd.movie.start_time) return;

  /* if starting time moves more than 2 volume intervals, assume we want archive mode */
  if(abs(last_time - utime) > (2 * _gd.movie.time_interval_mins * 60)) {
    _gd.movie.mode = ARCHIVE_MODE;
    _gd.coord_expt->runtime_mode = RUNMODE_ARCHIVE;
    _gd.coord_expt->time_seq_num++;
    // xv_set(_gd.movie_pu->movie_type_st,PANEL_VALUE,ARCHIVE_MODE,NULL);

  } else {
    if(_gd.movie.mode == REALTIME_MODE) {
      _gd.coord_expt->runtime_mode = RUNMODE_REALTIME;
      _gd.coord_expt->time_seq_num++;

      // Not Sensible to change start times in real time mode - Ignore;
      _updateMoviePopup();
      return ;
    }
  }

  _gd.movie.start_time = utime;

  // Record the time we're currently on
  target_time = _gd.movie.frame[_gd.movie.cur_frame].time_start;

  // Make a temporary copy
  // Zero out global array
  for (int ii = 0; ii < Constants::MAX_FRAMES; ii++) {
    tmp_frame[ii] = _gd.movie.frame[ii];
    _gd.movie.frame[ii].init();
  }
  // memcpy(tmp_frame,_gd.movie.frame,sizeof(movie_frame_t) * Constants::MAX_FRAMES);
  
  
  // Fill in time points on global array
  // reset_time_points();

  // Search for frames already rendered for this interval and use them
  for(i=0 ; i < _gd.movie.num_frames; i++) {
    found = 0;
    for(j=0; j < Constants::MAX_FRAMES && !found; j++) {
      if(_gd.movie.frame[i].time_start == tmp_frame[j].time_start) {
        found = 1;
        _gd.movie.frame[i] = tmp_frame[j];
        // Render a new time selector for this frame
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
        tmp_frame[j].init();
      }
	 
    }
  }

  // Reuse pixmaps in unused frames
  for(i=0 ; i < _gd.movie.num_frames; i++) {
    if(_gd.movie.frame[i].h_pdev) continue; // Alreday is accounted for.

    found = 0;
#ifdef NOTYET
    for(j=0; j < Constants::MAX_FRAMES && !found; j++) {
      if(tmp_frame[j].h_xid) {
        found = 1;
        _gd.movie.frame[i].h_xid = tmp_frame[j].h_xid;
        _gd.movie.frame[i].v_xid = tmp_frame[j].v_xid;
        _gd.movie.frame[i].redraw_horiz = 1;
        _gd.movie.frame[i].redraw_vert = 1;
        memset(&tmp_frame[j],0,sizeof(movie_frame_t));
      }
    }
#endif
  }

  _gd.movie.cur_frame = 0;
  for(i=0; i < _gd.movie.num_frames; i++) {
    if(target_time >= _gd.movie.frame[i].time_start &&
       target_time <= _gd.movie.frame[i].time_end) {

      _gd.movie.cur_frame = i;
    }
  }

  // Reset gridded and product data validity flags
  // invalidate_all_data();

  _updateMoviePopup();
     
}

/*************************************************************************
 *  SET_END_FRAME
 */
void GuiManager::_setEndFrame(int num_frames)
  
{
  int i,j;
  time_t    target_time;
  movie_frame_t    tmp_frame[Constants::MAX_FRAMES];    /* info about each frame */
  int old_frames;

  _gd.movie.end_frame = num_frames -1;
  
  // Sanity check
  if(_gd.movie.end_frame < 0) _gd.movie.end_frame = 0;
  if(_gd.movie.end_frame >= Constants::MAX_FRAMES) _gd.movie.end_frame = Constants::MAX_FRAMES -1;
  old_frames = _gd.movie.num_frames;
  _gd.movie.num_frames = _gd.movie.end_frame +1;
  
  if(_gd.movie.num_frames > 1) {
    // xv_set(_gd.movie_pu->movie_frame_sl,XV_SHOW,TRUE,NULL);
  } else {
    // xv_set(_gd.movie_pu->movie_frame_sl,XV_SHOW,FALSE,NULL);
  }

  target_time = _gd.movie.frame[_gd.movie.cur_frame].time_start;

  if(_gd.movie.mode == REALTIME_MODE) {
    _gd.movie.cur_frame = _gd.movie.end_frame;
    // Make a temporary copy
    // Zero out global array
    for (int ii = 0; ii < Constants::MAX_FRAMES; ii++) {
      tmp_frame[ii] = _gd.movie.frame[ii];
      _gd.movie.frame[ii].init();
    }
    
    // Start point changes
    _gd.movie.start_time -= (time_t) ((_gd.movie.time_interval_mins * 60.0) *
                                      (_gd.movie.num_frames - old_frames));
    
    // reset_time_points();
    
    if(_gd.movie.num_frames > old_frames) {
      // copy original frames
      for(i = _gd.movie.num_frames - old_frames, j = 0; j < old_frames; i++, j++) {
        _gd.movie.frame[i] = tmp_frame[j];
        
        // Render time selector in reused frame
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
      
      // Mark new frames for allocation & redrawing
      j = _gd.movie.num_frames - old_frames;
      for(i = 0; i < j; i++) {
        _gd.movie.frame[i].redraw_horiz = 1;
        _gd.movie.frame[i].redraw_vert = 1;
      }
    } else {
      for(i = 0, j = old_frames - _gd.movie.num_frames ; j < old_frames; i++, j++) {
        _gd.movie.frame[i] = tmp_frame[j];
        // Render time selector in reused frame
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
      // Copy unused frames too so they get de-allocated
      for(j = 0, i = _gd.movie.num_frames; j < old_frames - _gd.movie.num_frames; i++, j++) {
        _gd.movie.frame[i] = tmp_frame[j];
      }
    }
  } else {
    _gd.movie.cur_frame = 0;
    // Start point remains the same
    // reset_time_points();

    if(_gd.movie.num_frames > old_frames) {
      for(i = _gd.movie.num_frames -1; i < old_frames; i++) {
        _gd.movie.frame[i].redraw_horiz = 1;
        _gd.movie.frame[i].redraw_vert = 1;
      }
      // Render time selector in reused frames
      for(i= 0; i < old_frames; i++) {
#ifdef NOTYET
       draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
    } else {
      // Render time selector in reused frames
      for(i= 0; i < _gd.movie.num_frames; i++) {
#ifdef NOTYET
        draw_hwin_bot_margin(_gd.movie.frame[i].h_xid,_gd.h_win.page,
                             _gd.movie.frame[i].time_start,
                             _gd.movie.frame[i].time_end);
#endif
      }
    }
  }
     
  for(i=0; i < _gd.movie.num_frames; i++) {
    if(target_time >= _gd.movie.frame[i].time_start &&
       target_time <= _gd.movie.frame[i].time_end) {

      _gd.movie.cur_frame = i;
    }
  }
  // Reset gridded and product data validity flags
  // invalidate_all_data();
     
  _updateMoviePopup();

  // adjust_pixmap_allocation();
     
  return;
}

/*************************************************************************
 * UPDATE_MOVIE_PU: Update critical displayed values on the movie popup
 */

void GuiManager::_updateMoviePopup()
{
  int        index;
  char    text[64];
  char    fmt_text[128];
  struct tm tms;

  if(_gd.movie.cur_frame < 0) {
    index =  _gd.movie.num_frames - 1;
  } else {
    index = _gd.movie.cur_frame;
  }
 
  /* Update the Current Frame Begin Time text */
  snprintf(fmt_text,128,"Frame %d: %%H:%%M %%m/%%d/%%Y",index+1);
  if(_params.use_local_timestamps) {
    strftime (text,64,fmt_text,localtime_r(&_gd.movie.frame[index].time_mid,&tms));
  } else {
    strftime (text,64,fmt_text,gmtime_r(&_gd.movie.frame[index].time_mid,&tms));
  }
  // xv_set(_gd.movie_pu->fr_begin_msg,PANEL_LABEL_STRING,text,NULL);
  
  // Update the movie time start text
  if(_params.use_local_timestamps) {
    strftime (text, 64, _params.moviestart_time_format,
              localtime_r(&_gd.movie.start_time,&tms));
  } else {
    strftime (text, 64, _params.moviestart_time_format,
              gmtime_r(&_gd.movie.start_time,&tms));
  }
  // xv_set(_gd.movie_pu->start_time_tx,PANEL_VALUE,text,NULL);

  // xv_set(_gd.movie_pu->movie_type_st,PANEL_VALUE,_gd.movie.mode,NULL);

  if(_gd.debug1) printf("Time Start: %ld, End: %ld\n",_gd.movie.frame[index].time_start,
                       _gd.movie.frame[index].time_end);
   
  /* update the time_interval  text */
  switch(_gd.movie.climo_mode) {
    case Params::CLIMO_REGULAR_INTERVAL:
      snprintf(text,64,"%.2f",_gd.movie.time_interval_mins);
      // xv_set(_gd.movie_pu->time_interval_tx,PANEL_VALUE,text,NULL);
      // xv_set(_gd.movie_pu->min_msg,PANEL_LABEL_STRING,"min",NULL);
      break;

    case Params::CLIMO_DAILY_INTERVAL:
      // xv_set(_gd.movie_pu->time_interval_tx,PANEL_VALUE,"1",NULL);
      // xv_set(_gd.movie_pu->min_msg,PANEL_LABEL_STRING,"day",NULL);
      break;

    case Params::CLIMO_YEARLY_INTERVAL:
      // xv_set(_gd.movie_pu->time_interval_tx,PANEL_VALUE,"1",NULL);
      // xv_set(_gd.movie_pu->min_msg,PANEL_LABEL_STRING,"yr",NULL);
      break;

  }

  /* update the forecast period text */
  snprintf(text,64,"%.2f",_gd.movie.forecast_interval);
  // xv_set(_gd.movie_pu->fcast_per_tx,PANEL_VALUE,text,NULL);

  // xv_set(_gd.movie_pu->movie_frame_sl,
  //        PANEL_MIN_VALUE,_gd.movie.start_frame + 1,
  //        PANEL_MAX_VALUE,_gd.movie.end_frame + 1,
  //        PANEL_VALUE,_gd.movie.cur_frame +1,
  //        NULL);

  /* update the start/end frames text */
  snprintf(text,64,"%d",_gd.movie.end_frame +1);
  // xv_set(_gd.movie_pu->end_frame_tx,PANEL_VALUE,text,NULL);

  if (_gd.prod_mgr) {
    _gd.prod_mgr->reset_times_valid_flags();
  }

#ifdef NOTYET
  if(_gd.time_plot)
  {
    _gd.time_plot->Set_times((time_t) _gd.epoch_start,
                            (time_t) _gd.epoch_end,
                            (time_t) _gd.movie.frame[_gd.movie.cur_frame].time_start,
                            (time_t) _gd.movie.frame[_gd.movie.cur_frame].time_end,
                            (time_t)((_gd.movie.time_interval_mins * 60.0) + 0.5),
                            _gd.movie.num_frames);
    _gd.time_plot->Draw(); 
  }
#endif
    

}


#ifdef JUNK

void GuiManager::_ciddTimerFunc(QTimerEvent *event)
{

  MdvReader *mr;
  int index,flag = 0;
  int msec_diff = 0;
  int msec_delay = 0;
  long tm = 0;

  QPixmap *h_pdev = 0;
  QPixmap *v_pdev = 0;

  struct timezone cur_tz;

  // if(_gd.io_info.outstanding_request) {
  //   check_for_io();
  // }

  gettimeofday(&cur_tm,&cur_tz);

  /* Update the current time ticker if necessary */
  if(cur_tm.tv_sec > last_tick) {
    if(!_params.run_once_and_exit) {
      char buf[128];
      snprintf(buf,128,"Idle %d secs, Req: %d, Mode: %d, Type: %d",
               (int) (cur_tm.tv_sec - _gd.last_event_time),
               _gd.io_info.outstanding_request,
               _gd.io_info.mode,
               _gd.io_info.request_type);
      if(_gd.debug || _gd.debug1 || _gd.debug2) {
        PMU_force_register(buf);
      } else {
        PMU_auto_register(buf);
      }

    }
    update_ticker(cur_tm.tv_sec);
    last_tick = cur_tm.tv_sec;
    if(_gd.last_event_time < (last_tick - _params.idle_reset_seconds)) {
      reset_display();
      _gd.last_event_time = last_tick;
    }
  }

  msec_delay = _gd.movie.display_time_msec;

  /**** Get present frame index *****/
  if (_gd.movie.cur_frame < 0) {
    index = _gd.movie.num_frames - 1;
  } else {
    index = _gd.movie.cur_frame;
  }
  // If no images or IO are pending - Check for remote commands
  if(_gd.image_needs_saved == 0 &&
     _gd.movie.frame[index].redraw_horiz == 0 &&
     _gd.io_info.outstanding_request == 0) { 

    if(_gd.v_win.active == 0 || _gd.movie.frame[index].redraw_vert == 0) {
      if(_gd.remote_ui != NULL) ingest_remote_commands();
    }
  }

  // Update the times in the Coordinate SHMEM
  _gd.coord_expt->epoch_start = _gd.epoch_start;
  _gd.coord_expt->epoch_end = _gd.epoch_end; 
  _gd.coord_expt->time_min = _gd.movie.frame[index].time_start;
  _gd.coord_expt->time_max = _gd.movie.frame[index].time_end;
  if(_gd.movie.movie_on) { 
    _gd.coord_expt->time_cent = _gd.coord_expt->epoch_end;
  } else {
    _gd.coord_expt->time_cent = _gd.coord_expt->time_min +
      (_gd.coord_expt->time_max - _gd.coord_expt->time_min) / 2;
  }
  _gd.coord_expt->time_current_field = _gd.mread[_gd.h_win.page]->h_mhdr.time_centroid;

  if (_gd.movie.movie_on ) {
    flag = 1;        /* set OK state */
    if (_gd.movie.frame[index].redraw_horiz != 0) flag = 0;
    if (_gd.movie.frame[index].redraw_vert != 0 && _gd.v_win.active) flag = 0;

    msec_diff =
      ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) +
      ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);

    if (flag && msec_diff > _gd.movie.display_time_msec) {
      /* Advance Movie frame */
      _gd.movie.cur_frame += _gd.movie.sweep_dir;    

      /* reset to beginning of the loop if needed */
      if (_gd.movie.cur_frame > _gd.movie.end_frame) {
	if(_gd.series_save_active) { // End of the Series Save - Turn off
          char cmd[4096];
          _gd.series_save_active = 0;
          _gd.movie.movie_on = 0;
          if(_params.series_convert_script !=NULL) {
            STRncopy(cmd,_params.series_convert_script,4096);
            for(int ii= _gd.movie.start_frame; ii <= _gd.movie.end_frame; ii++) {
              STRconcat(cmd," ",4096);
              STRconcat(cmd,_gd.movie.frame[ii].fname,4096);
            }
            printf("Running: %s\n",cmd);
          }
          set_busy_state(1);
          safe_system(cmd,_params.complex_command_timeout_secs);

          if(_gd.v_win.active) {
            if(_params.series_convert_script !=NULL) {
              STRncopy(cmd,_params.series_convert_script,4096);
              for(int ii= _gd.movie.start_frame; ii <= _gd.movie.end_frame; ii++) {
                STRconcat(cmd," ",4096);
                STRconcat(cmd,_gd.movie.frame[ii].vfname,4096);
              }
              printf("Running: %s\n",cmd);
            }
            set_busy_state(1);
            safe_system(cmd,_params.complex_command_timeout_secs);
          }

          set_busy_state(0);
          _gd.movie.cur_frame = _gd.movie.end_frame;
	} else {
          if(_gd.movie.sweep_on) {
            _gd.movie.sweep_dir = -1;
            _gd.movie.cur_frame = _gd.movie.end_frame -1;
          } else {
            _gd.movie.cur_frame = _gd.movie.start_frame -1;
          }
	}
      }

      if(_gd.movie.cur_frame < _gd.movie.start_frame) { 
        _gd.movie.sweep_dir = 1;
        if(_gd.movie.sweep_on) {
          _gd.movie.cur_frame = _gd.movie.start_frame+1;
        }
      }
	
      if (_gd.movie.cur_frame == _gd.movie.end_frame) {
        msec_delay = _gd.movie.delay;
      }

      /**** recalc current frame index *****/
      if (_gd.movie.cur_frame < 0) {
        index =  _gd.movie.num_frames - 1;
      } else {
        index = _gd.movie.cur_frame;
      }
    }
  } // if (_gd.movie.movie_on)

  /* Set up convienient pointer to main met record */
  mr = _gd.mread[_gd.h_win.page];

  /* Decide which Pixmaps to use for rendering */
  if (_gd.movie.movie_on ) {
    /* set to the movie frame Pixmaps */
    h_pdev = _gd.movie.frame[index].h_pdev;
    if (h_pdev == 0) {
      if(mr->auto_render) {    
        h_pdev = _gd.h_win.page_pdev[_gd.h_win.page];
      } else {
        h_pdev = _gd.h_win.tmp_pdev;
      }
    }

    v_pdev = _gd.movie.frame[index].v_pdev;
    if (v_pdev == 0) {
      if(mr->auto_render) {
        v_pdev = _gd.v_win.page_pdev[_gd.v_win.page];
      } else {
        v_pdev = _gd.v_win.tmp_pdev;
      }
    }
	
  } else {
    /* set to the field Pixmaps */
    if(mr->auto_render) {
      h_pdev = _gd.h_win.page_pdev[_gd.h_win.page];
    } else {
      h_pdev = _gd.h_win.tmp_pdev;
    }

    if(_gd.mread[_gd.v_win.page]->auto_render) {
      v_pdev = _gd.v_win.page_pdev[_gd.v_win.page];
    } else {
      v_pdev = _gd.v_win.tmp_pdev;
    }
  }

  /******* Handle Real Time Updating  ********/
  switch (_gd.movie.mode) {
    case REALTIME_MODE :
      if (time_for_a_new_frame()) {
	rotate_movie_frames(); 
	/* Vectors must be removed from the (currently) last frame if the wind_mode > 0 */
	if(_gd.layers.wind_mode && _gd.layers.wind_vectors)  {
          _gd.movie.frame[_gd.movie.cur_frame].redraw_horiz = 1;
	}

	// All product data must be reloaded - Set all to invalid
	if (_gd.prod_mgr) {
          _gd.prod_mgr->reset_product_valid_flags();
          _gd.prod_mgr->reset_times_valid_flags();
	}

	/* Move movie loop to the last frame when aging off old movie frames */
	_gd.movie.cur_frame = _gd.movie.end_frame;
	goto return_point;
      }

      tm = time(0);
      /* CHECK FOR NEW DATA */
      if ( tm >= update_due ) {

	/* Check only on the last frame - Because its the only "live/realtime" one */
	if (_gd.movie.cur_frame == _gd.movie.num_frames -1) {
          update_due = tm + update_interv;

          _checkForExpiredData(tm);  // Look for old data

          _checkForDataUpdates(tm);  // Look for data that's newly updated

          _checkWhatNeedsRendering(index);

          // Auto click to get ancillary displays to update too.
          _gd.coord_expt->click_type = CIDD_OTHER_CLICK;
          _gd.coord_expt->pointer_seq_num++;
	}
      }

      break;

    case ARCHIVE_MODE :
      break;

    default:
      fprintf(stderr,
              "Invalid movie mode %d in timer_func\n",
              _gd.movie.mode);
      break;
  } 


  /***** Handle Field changes *****/

  if (_gd.h_win.page != _gd.h_win.prev_page) {
    // cerr << "FFFFFFFFFFF _gd.h_win.page, _gd.h_win.prev_page: " <<  _gd.h_win.page << ", " << _gd.h_win.prev_page << endl;
    if (_gd.movie.movie_on ) {
      set_redraw_flags(1,0);
    } else {
      if (_gd.h_win.redraw_flag[_gd.h_win.page] == 0) {
        _gd.h_copy_flag = 1;
        _gd.h_win.prev_page = _gd.h_win.page;
        // cerr << "GGGGGGGGGGGGGG _gd.h_win.page, _gd.h_win.prev_page: " <<  _gd.h_win.page << ", " << _gd.h_win.prev_page << endl;
        set_redraw_flags(0,1);
        _vlevelManager.setFromMdvx();
        _createVlevelRadioButtons();
        _horiz->update();
        if (!_guiSizeInitialized) {
          resize(width() + 1, height() + 1);
          resize(width() + 1, height() + 1);
          _guiSizeInitialized = true;
        }
      }
    }
  }
  if (_gd.v_win.page != _gd.v_win.prev_page ) {
    if (_gd.movie.movie_on ) {
      set_redraw_flags(0,1);
    } else {
      if (_gd.v_win.redraw_flag[_gd.v_win.page] == 0) {
        _gd.v_copy_flag = 1;
        _gd.v_win.prev_page = _gd.v_win.page;
      }
    }
  }

  /******** Handle Frame changes ********/
  if (_gd.movie.last_frame != _gd.movie.cur_frame && _gd.movie.cur_frame >= 0) {

    // reset_data_valid_flags(1,1);

    if(_params.symprod_short_requests) {
      // All product data must be reloaded - Set all to invalid
      _gd.prod_mgr->reset_product_valid_flags();
    }

    /* Move the indicators */
    // xv_set(_gd.movie_pu->movie_frame_sl,
    //        PANEL_VALUE, _gd.movie.cur_frame + 1,
    //        NULL);
	
    if(_gd.debug2) {
      printf("Moved movie frame, index : %d\n",index);
    }

    _gd.coord_expt->epoch_start = _gd.epoch_start;
    _gd.coord_expt->epoch_end = _gd.epoch_end;

    if(_gd.movie.movie_on) {
      _gd.coord_expt->time_cent = _gd.coord_expt->epoch_end;
    } else {
      _gd.coord_expt->time_min = _gd.movie.frame[index].time_start;
      _gd.coord_expt->time_max = _gd.movie.frame[index].time_end;
      _gd.coord_expt->time_cent = _gd.coord_expt->time_min +
	(_gd.coord_expt->time_max - _gd.coord_expt->time_min) / 2;
      _gd.coord_expt->click_type = CIDD_OTHER_CLICK;
      _gd.coord_expt->pointer_seq_num++;
    }
    _gd.coord_expt->time_current_field = _gd.mread[_gd.h_win.page]->h_mhdr.time_centroid;

    /* Change Labels on Frame Begin, End messages */
    update_frame_time_msg(index);
		
    if (_gd.movie.frame[index].redraw_horiz == 0) {
      /* Get Frame */
      retrieve_h_movie_frame(index,h_pdev);
      _gd.h_copy_flag = 1;
    }

    if (_gd.v_win.active && _gd.movie.frame[index].redraw_vert == 0) {
      retrieve_v_movie_frame(index,v_pdev);
      _gd.v_copy_flag = 1;
    }

    _gd.movie.last_frame = _gd.movie.cur_frame;
  }


  /* Draw Selected field - Vertical  for this movie frame */
  if (_gd.v_win.active) {
    if (_gd.movie.frame[index].redraw_vert) {
      if (gather_vwin_data(_gd.v_win.page,_gd.movie.frame[index].time_start,
                           _gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
        if (_gd.v_win.redraw_flag[_gd.v_win.page]) {
          render_v_movie_frame(index,v_pdev);
          save_v_movie_frame(index,v_pdev);
        } 
        _gd.movie.frame[index].redraw_vert = 0;
        _gd.v_win.redraw_flag[_gd.v_win.page] = 0;
        _gd.v_copy_flag = 1;
      }
    }
  }

  // generate vert section images as required

  if (_params.image_generate_vsection) {

    if (_params.image_debug) {
      cerr << "============>> generating specified vsection images" << endl;
    }

    for (int ii = 0; ii < _params.image_vsection_spec_n; ii++) {

      Params::image_vsection_spec_t vsect = _params._image_vsection_spec[ii];
      if (_params.image_debug) {
        cerr << "=================>> generating vsection ii: " << ii << endl;
        cerr << "    vsection_label: " << vsect.vsection_label << endl;
        cerr << "    n_waypts: " << vsect.n_waypts << endl;
        cerr << "    waypt_locs: " << vsect.waypt_locs << endl;
      }
      cerr << "=====================================" << endl;
      
      // get waypts

      string waypts_locs(vsect.waypt_locs);
      vector<string> toks;
      TaStr::tokenize(waypts_locs, "(", toks);

      int npts_found = 0;
      for (size_t jj = 0; jj < toks.size(); jj++) {
        double xx, yy;
        if (sscanf(toks[jj].c_str(), "%lg, %lg", &xx, &yy) == 2) {
          _gd.h_win.route.x_world[jj] = xx;
          _gd.h_win.route.y_world[jj] = yy;
          npts_found++;
        }
      }

      if (npts_found == vsect.n_waypts && npts_found > 1) {
        _gd.h_win.route.total_length = 0.0;
        _gd.h_win.route.num_segments = npts_found - 1;
        for (int iseg = 0; iseg < _gd.h_win.route.num_segments; iseg++) {
          _gd.h_win.route.seg_length[iseg] =
            disp_proj_dist(_gd.h_win.route.x_world[iseg],_gd.h_win.route.y_world[iseg],
                           _gd.h_win.route.x_world[iseg+1],_gd.h_win.route.y_world[iseg+1]);
          _gd.h_win.route.total_length += _gd.h_win.route.seg_length[iseg];
        }
      }
      
      if (gather_vwin_data(_gd.v_win.page,_gd.movie.frame[index].time_start,
                           _gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
        _gd.series_save_active = 1;
        render_v_movie_frame(index,v_pdev);
        save_v_movie_frame(index,v_pdev);
      }

    } // ii

  } // if (_params.image_generate_vsection)

  /* Draw Selected field - Horizontal for this movie frame */
#ifdef NOTNOW
  if (_gd.movie.frame[index].redraw_horiz) {
#endif
    /* Draw Frame */
  
    if (gather_hwin_data(_gd.h_win.page,
                         _gd.movie.frame[index].time_start,
                         _gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
      if (_gd.h_win.redraw_flag[_gd.h_win.page]) {
        // render_h_movie_frame(index,h_pdev);
        _horiz->setFrameForRendering(_gd.h_win.page, index);
#ifdef NOTNOW
        save_h_movie_frame(index, h_pdev, _gd.h_win.page);
#endif
      } 

      /* make sure the horiz window's slider has the correct label */
      //set_height_label();

      _gd.movie.frame[index].redraw_horiz = 0;
      _gd.h_win.redraw_flag[_gd.h_win.page] = 0;
      _gd.h_copy_flag = 1;
    }
#ifdef NOTNOW
  }
#endif
	

  /* Selected Field or movie frame has changed
   * Copy background image onto visible canvas
   */
  
  if (_gd.h_copy_flag || (_gd.h_win.cur_cache_im != _gd.h_win.last_cache_im)) {

    if (_gd.debug2) {
      fprintf(stderr,
              "\nCopying Horiz grid image - "
              "field %d, index %d pdev: %p to pdev: %p\n",
              _gd.h_win.page,index,(void *) h_pdev, (void *)_gd.h_win.can_pdev[_gd.h_win.cur_cache_im]);

      if(_gd.h_win.cur_cache_im == _gd.h_win.last_cache_im) {
#ifdef NOTYET
        XCopyArea(_gd.dpy,h_pdev,
                  _gd.h_win.can_pdev[_gd.h_win.cur_cache_im],
                  _gd.def_gc,    0,0,
                  _gd.h_win.can_dim.width,
                  _gd.h_win.can_dim.height,
                  _gd.h_win.can_dim.x_pos,
                  _gd.h_win.can_dim.y_pos);
#endif
      } else {
        _gd.h_win.last_cache_im = _gd.h_win.cur_cache_im; 
      }
      _gd.h_win.prev_page = _gd.h_win.page;
      _gd.h_copy_flag = 0;

      if (_gd.zoom_in_progress == 1) redraw_zoom_box();
      if (_gd.pan_in_progress) redraw_pan_line();
      if (_gd.route_in_progress) redraw_route_line(&_gd.h_win);

      if(!_params.run_once_and_exit) PMU_auto_register("Rendering Products (OK)");

      if (_gd.debug2) {
        fprintf(stderr,
                "\nTimer: Displaying Horiz final image - "
                "field %d, index %d pdev: %p to pdev: %p\n",
                _gd.h_win.page,index,
                (void *) _gd.h_win.can_pdev[_gd.h_win.cur_cache_im],
                (void *) _gd.h_win.vis_pdev);

        /* Now copy last stage pixmap to visible pixmap */
#ifdef NOTYET
        XCopyArea(_gd.dpy,_gd.h_win.can_pdev[_gd.h_win.cur_cache_im],
                  _gd.h_win.vis_pdev,
                  _gd.def_gc,    0,0,
                  _gd.h_win.can_dim.width,
                  _gd.h_win.can_dim.height,
                  _gd.h_win.can_dim.x_pos,
                  _gd.h_win.can_dim.y_pos);
#endif

        if (_gd.zoom_in_progress == 1) redraw_zoom_box();
        if (_gd.pan_in_progress) redraw_pan_line();
        if (_gd.route_in_progress) redraw_route_line(&_gd.h_win);

        // Render a time indicator plot in the movie popup
        // if(_gd.time_plot)
        // {
        //   _gd.time_plot->Set_times((time_t) _gd.epoch_start,
        //                           (time_t) _gd.epoch_end,
        //                           (time_t) _gd.movie.frame[_gd.movie.cur_frame].time_start,
        //                           (time_t) _gd.movie.frame[_gd.movie.cur_frame].time_end,
        //                           (time_t)((_gd.movie.time_interval_mins * 60.0) + 0.5),
        //                           _gd.movie.num_frames); 

        //   if(_gd.movie.active) _gd.time_plot->Draw();
        // }
    
        /* keep track of how much time will elapse showing the current image */
        gettimeofday(&last_frame_tm,&cur_tz);
	
      }

      if(_gd.v_win.cur_cache_im != _gd.v_win.last_cache_im) {
        _gd.v_copy_flag = 1;
        _gd.v_win.last_cache_im = _gd.v_win.cur_cache_im;
      }

      if (_gd.v_win.active && _gd.v_copy_flag) {
        if (_gd.debug2) fprintf(stderr,"\nCopying Vert grid image - field %d, index %d pdev: %p to pdev: %p\n",
                               _gd.v_win.page,index,(void *) v_pdev,(void *) _gd.v_win.can_pdev[_gd.v_win.cur_cache_im]);
#ifdef NOTYET
        XCopyArea(_gd.dpy,v_pdev,
                  _gd.v_win.can_pdev[_gd.v_win.cur_cache_im],
                  _gd.def_gc,    0,0,
                  _gd.v_win.can_dim.width,
                  _gd.v_win.can_dim.height,
                  _gd.v_win.can_dim.x_pos,
                  _gd.v_win.can_dim.y_pos);
#endif
        _gd.v_win.prev_page  = _gd.v_win.page;
        _gd.v_copy_flag = 0;

        if (_gd.debug2) fprintf(stderr,"\nDisplaying Vert final image - field %d, index %d pdev: %p to pdev: %p\n",
                               _gd.v_win.page,index,(void *) _gd.v_win.can_pdev[_gd.v_win.cur_cache_im],(void *) _gd.v_win.vis_pdev);

        /* Now copy last stage pixmap to visible pixmap */
#ifdef NOTYET
        XCopyArea(_gd.dpy,_gd.v_win.can_pdev[_gd.v_win.cur_cache_im],
                  _gd.v_win.vis_pdev,
                  _gd.def_gc,    0,0,
                  _gd.v_win.can_dim.width,
                  _gd.v_win.can_dim.height,
                  _gd.v_win.can_dim.x_pos,
                  _gd.v_win.can_dim.y_pos);
#endif

      }

      // If remote driven image(s) needs saved - do it.
      if(_gd.image_needs_saved ) {
        int ready = 1;

        // Check to make sure the images are done
        if((_gd.save_im_win & PLAN_VIEW) && _gd.h_win.redraw_flag[_gd.h_win.page] != 0) ready = 0;
        if((_gd.save_im_win & XSECT_VIEW) && _gd.v_win.redraw_flag[_gd.v_win.page] != 0) ready = 0;
        if(ready) {
          dump_cidd_image(_gd.save_im_win,0,0,_gd.h_win.page);
          _gd.image_needs_saved = 0;
        }
      }


      /***** Handle redrawing background images *****/

      msec_diff = ((cur_tm.tv_sec - last_dcheck_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_dcheck_tm.tv_usec) / 1000);

      if (msec_diff < 0 ||  (msec_diff > redraw_interv  && _gd.movie.movie_on == 0)) {
        _horiz->setRenderInvalidImages(index, _vert);

        /* keep track of how much time will elapse since the last check */
        gettimeofday(&last_dcheck_tm,&cur_tz);
	
      }
      
    return_point:

      if (_gd.movie.movie_on == 0) {
        if (_gd.zoom_in_progress == 1) redraw_zoom_box();
        if (_gd.pan_in_progress) redraw_pan_line();
        if (_gd.route_in_progress) redraw_route_line(&_gd.h_win);

        if (_gd.zoom_in_progress == 1) redraw_zoom_box();
        if (_gd.pan_in_progress) redraw_pan_line();
        if (_gd.route_in_progress) redraw_route_line(&_gd.h_win);

      } else {
        gettimeofday(&cur_tm,&cur_tz);
        msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
        msec_delay = msec_delay - msec_diff;
        if(msec_delay <= 0 || msec_delay > 10000) msec_delay = 100;
      }

      if (client_seq_num != _gd.coord_expt->client_seq_num) {
        render_click_marks();
        client_seq_num = _gd.coord_expt->client_seq_num;
      }
      
    }

  }

}

#endif

