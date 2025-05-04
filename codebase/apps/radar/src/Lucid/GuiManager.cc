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

  _imagesStartTime.set(_params.images_archive_start_time);
  _imagesEndTime.set(_params.images_archive_end_time);
  _imagesScanIntervalSecs = _params.images_scan_interval_secs;

  // set up ray locators

  // _rayLoc.resize(RayLoc::RAY_LOC_N);

  // set up windows

  _setupWindows();

  // init for timer
  
  redraw_interv = _params.redraw_interval;
  update_interv = _params.update_interval;
  update_due = 0;
  last_frame_tm = {0,0};
  last_dcheck_tm = {0,0};
  last_tick = 0;
  client_seq_num = 0;

  // startup mode
  
  setArchiveMode(_params.start_mode == Params::MODE_ARCHIVE);
  _archiveStartTime.set(_params.archive_start_time);

  // qt attributes
  
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
// enable the unzoom buttons

void GuiManager::enableUnzoomButtons(bool val) const
{
  _zoomBackAct->setEnabled(val);
  _zoomOutAct->setEnabled(val);
  if (val) {
    // disable zoom menu items
    for (size_t ii = 0; ii < _zoomMenuItems.size(); ii++) {
      _zoomMenuItems[ii]->getAction()->setEnabled(false);
    }
  } else {
    // enable zoom menu items
    for (size_t ii = 0; ii < _zoomMenuItems.size(); ii++) {
      _zoomMenuItems[ii]->getAction()->setEnabled(true);
    }
  }
}

//////////////////////////////////////////////////
// respond to zoom action, set the zoom index

void GuiManager::setZoomIndex(int zoomIndex)
{
  _horiz->setZoomIndex(zoomIndex);
}

// void GuiManager::setXyZoom(double minY, double maxY,
//                            double minX, double maxX)
// {
//   _horiz->setXyZoom(minY, maxY, minX, maxX);
// }

/////////////////////////////////////
// set archive mode

void GuiManager::setArchiveMode(bool state)
{

  _archiveMode = state;
  
}

//////////////////////////////////////////////////
// Set radar name in title bar

void GuiManager::setTitleBarStr(const string &titleStr)
{
  setWindowTitle(tr(titleStr.c_str()));
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
  
  // trap ESC in case needed

  if (keychar == 27) {
    // do something
  }

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

//////////////////////////////
// create actions for menus

void GuiManager::_createActions()
{

  // show field menu
  _showFieldMenuAct = new QAction(tr("Fields"), this);
  _showFieldMenuAct->setStatusTip(tr("Show field menu"));
  connect(_showFieldMenuAct, &QAction::triggered,
          this, &GuiManager::_showFieldMenu);
  
  // show user click in dialog
  _showClickAct = new QAction(tr("Values"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, &QAction::triggered, this, &GuiManager::showClick);

  // show time control window
  _showTimeControlAct = new QAction(tr("Movie"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, &QAction::triggered,
          this, &GuiManager::_showTimeControl);
  
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
  
  // time selector
  
  menuBar()->addAction(_showTimeControlAct);

  // add overlay menu
  
  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _populateOverlaysMenu();

  // misc actions
  
  menuBar()->addAction(_showClickAct);
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
  
  if (_horiz->checkForZoomChange()) {
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

////////////////////////////////////////////
// unzoom to previous zoom

void GuiManager::_zoomBack()
{
  _horiz->zoomBackView();
  if (_horiz->getCustomZooms().size() == 0) {
    enableUnzoomButtons(false);
  }
}

////////////////////////////////////////////
// unzoom all the way out

void GuiManager::_zoomOut()
{
  _horiz->zoomOutView();
  _horiz->clearCustomZooms();
  enableUnzoomButtons(false);
}

/////////////////////////////////////////////////////////
// handle first time event

void GuiManager::_handleFirstTimerEvent()
{
  _horiz->resize(_horizFrame->width(), _horizFrame->height());
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

#ifdef NOTYET
  
  if (_vertMode) {
    _plotStartTime = _vert->getPlotStartTime();
    _plotEndTime = _vert->getPlotEndTime();
  } else {
    _plotStartTime = _horiz->getPlotStartTime();
    _plotEndTime = _horiz->getPlotEndTime();
  }

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
    // _plotStartTime = _vert->getPlotStartTime();
    // _plotEndTime = _vert->getPlotEndTime();
  } else {
    // _plotStartTime = _horiz->getPlotStartTime();
    // _plotEndTime = _horiz->getPlotEndTime();
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
// TODO: check HawkEye for example

void GuiManager::showClick()
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
