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
// CartManager manages cartesian rendering - plan view
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
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Ncxx/H5x.hh>

#include "CartManager.hh"
#include "DisplayField.hh"
#include "FieldTableItem.hh"
#include "HorizWidget.hh"
#include "MapMenuItem.hh"
#include "ProdMenuItem.hh"
#include "TimeControl.hh"
#include "VertWidget.hh"
#include "VertWindow.hh"
#include "WindMenuItem.hh"
#include "ZoomMenuItem.hh"

// cidd.h must be included AFTER qt includes because of #define None in X11/X.h

#include "cidd.h"

using namespace std;
using namespace H5x;

CartManager* CartManager::m_pInstance = NULL;

CartManager* CartManager::Instance()
{
  return m_pInstance;
}

// Constructor

CartManager::CartManager(const Params &params,
                         const vector<DisplayField *> &fields,
                         bool haveFilteredFields) :
        DisplayManager(params, fields, haveFilteredFields),
        // _sweepManager(params),
        _vertWindowDisplayed(false)
{

  m_pInstance = this;

  // initialize

  // _firstTime = true;
  // _urlOK = true;

  // _prevAz = -9999.0;
  // _prevEl = -9999.0;
  // _startAz = -9999.0;
  // _endAz = -9999.0;
  // _vertMode = false;

  // _nGates = 1000;
  // _maxRangeKm = 1.0;
  
  _horizFrame = NULL;
  _horiz = NULL;

  _vertWindow = NULL;
  _vert = NULL;

  // _sweepVBoxLayout = NULL;
  // _sweepPanel = NULL;

  _fieldMenu = NULL;
  _fieldTable = NULL;
  _fieldMenuPlaced = false;
  _fieldMenuPanel = NULL;
  _fieldMenuLayout = NULL;
  _fieldTableCurrentColumn = -1;
  _fieldTableCurrentRow = -1;

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

  _changeField(0, false);

  // init for timer
  
  redraw_interv = _params.redraw_interval;
  update_interv = _params.update_interval;
  update_due = 0;
  last_frame_tm = {0,0};
  last_dcheck_tm = {0,0};
  last_tick = 0;
  client_seq_num = 0;
  
}

// destructor

CartManager::~CartManager()

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

int CartManager::run(QApplication &app)
{

  // make window visible

  show();
  
  // set timer running
  
  _mainTimerId = startTimer(2);
  
  return app.exec();

}

//////////////////////////////////////////////////
// enable the zoom button - called by PolarWidgets

void CartManager::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void CartManager::timerEvent(QTimerEvent *event)
{

  // register with procmap

  PMU_auto_register("timerEvent");

  // check ID
  
  if (event->timerId() != _mainTimerId) {
    return;
  }
  
  // field change?

  _checkForFieldChange();
  
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
  
  // handle legacy cidd timer event

  _ciddTimerFunc(event);
  
  // if (_archiveMode) {
  //   if (_archiveRetrievalPending) {
  //     _handleArchiveData(/*event*/);
  //     _archiveRetrievalPending = false;
  //   }
  // } else {
  //   _handleRealtimeData(event);
  // }
  
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
      _handleRealtimeData(event);
      _createRealtimeImageFiles();
      return;
    }
    
  } // if (_params.images_auto_create)
  
}

///////////////////////////////////////////////
// override resize event

void CartManager::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent: " << event << endl;
  }
  emit frameResized(_horizFrame->width(), _horizFrame->height());
}

////////////////////////////////////////////////////////////////
void CartManager::keyPressEvent(QKeyEvent * e)
{

  // get key pressed
  
  Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Clicked char: " << keychar << ":" << (int) keychar << endl;
    cerr << "         key: " << hex << key << dec << endl;
  }
  
  // for '.', swap with previous field
  
  if (keychar == '.') {
    QRadioButton *button = (QRadioButton *) _fieldGroup->button(_prevFieldNum);
    button->click();
    return;
  }
  
  // for ESC, freeze / unfreeze

  if (keychar == 27) {
    _freezeAct->trigger();
    return;
  }
  
  // check for short-cut keys to fields

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
    const DisplayField *field = _fields[ifield];

    char shortcut = 0;
    if (field->getShortcut().size() > 0) {
      shortcut = field->getShortcut()[0];
    }
    
    bool correctField = false;
    if (shortcut == keychar) {
      if (mods & Qt::AltModifier) {
        if (field->getIsFilt()) {
          correctField = true;
        }
      } else {
        if (!field->getIsFilt()) {
          correctField = true;
        }
      }
    }

    if (correctField) {
      if (_params.debug) {
	cerr << "Short-cut key pressed: " << shortcut << endl;
	cerr << "  field label: " << field->getLabel() << endl;
	cerr << "   field name: " << field->getName() << endl;
      }
      QRadioButton *button = (QRadioButton *) _fieldGroup->button(ifield);
      button->click();
      break;
    }

  }

  // check for up/down in sweeps

  if (key == Qt::Key_Left) {

    if (_params.debug) {
      cerr << "Clicked left arrow, go back in time" << endl;
    }
    _horiz->setStartOfSweep(true);
    _vert->setStartOfSweep(true);
    _timeControl->_goBack1();
    
  } else if (key == Qt::Key_Right) {

    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _horiz->setStartOfSweep(true);
    _vert->setStartOfSweep(true);
    _timeControl->_goFwd1();
    
  } else if (key == Qt::Key_Up) {

    // if (_sweepManager.getGuiIndex() > 0) {

    //   if (_params.debug) {
    //     cerr << "Clicked up arrow, go up a sweep" << endl;
    //   }
    //   _horiz->setStartOfSweep(true);
    //   _vert->setStartOfSweep(true);
    //   _changeSweepRadioButton(-1);

    // }

  } else if (key == Qt::Key_Down) {

    // if (_sweepManager.getGuiIndex() < (int) _sweepManager.getNSweeps() - 1) {

    //   if (_params.debug) {
    //     cerr << "Clicked down arrow, go down a sweep" << endl;
    //   }
    //   _horiz->setStartOfSweep(true);
    //   _vert->setStartOfSweep(true);
    //   _changeSweepRadioButton(+1);

    // }

  }

}

void CartManager::_moveUpDown() 
{
  this->setCursor(Qt::WaitCursor);
  // _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
}

//////////////////////////////////////////////////
// Set radar name in title bar

void CartManager::_setTitleBar(const string &radarName)
{
  string windowTitle = _params.horiz_frame_label;
  setWindowTitle(tr(windowTitle.c_str()));
}
  
//////////////////////////////////////////////////
// set up windows and widgets
  
void CartManager::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  QHBoxLayout *mainLayout = new QHBoxLayout;
  _main->setLayout(mainLayout);
  mainLayout->setSpacing(5);
  mainLayout->setContentsMargins(3,3,3,3);
  setCentralWidget(_main);

  // horiz window

  _horizFrame = new QFrame(_main);
  _horizFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the HORIZ

  _horiz = new HorizWidget(_horizFrame, *this, _params,
                           _platform, _fields, _haveFilteredFields);

  connect(this, &CartManager::frameResized, _horiz, &HorizWidget::resize);
  
  // Create the VERT window

  _vertWindow = new VertWindow(this, _params, _platform,
                               _fields, _haveFilteredFields);
  _vertWindow->setRadarName(_params.radar_name);

  // set pointer to the vertWidget

  _vert = _vertWindow->getWidget();
  
  // connect slots for location
  
  connect(_horiz, &HorizWidget::locationClicked,
          this, &CartManager::_horizLocationClicked);
  connect(_vert, &VertWidget::locationClicked,
          this, &CartManager::_vertLocationClicked);

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);
  // customContextMenuRequested(e->pos());
  connect(_horiz, &CartWidget::customContextMenuRequested,
	  this, &CartManager::ShowContextMenu);

  // create status panel

  _createStatusPanel();

  // create fields panel
  
  _createFieldPanel();

  // add widgets

  // mainLayout->addWidget(_statusPanel);
  // mainLayout->addWidget(_fieldPanel);
  mainLayout->addWidget(_horizFrame);

  // sweep panel

  // _createSweepPanel();
  // mainLayout->addWidget(_sweepPanel);

  // field menu

  _createFieldMenu();

  // time panel

  _createTimeControl();

  // fill out menu bar

  _createActions();
  _createMenus();

  // title bar

  _setTitleBar(_params.radar_name);
  setMinimumSize(400, 400);
  // resize(_params.main_window_width, _params.main_window_height);
  resize(_params.horiz_default_width, _params.horiz_default_height);
  
  // set location on screen

  QPoint pos;
  pos.setX(_params.main_window_start_x);
  pos.setY(_params.main_window_start_y);
  move(pos);
  
  // set up field status dialog
  _createClickReportDialog();

  // createBoundaryEditorDialog();

  if (_archiveMode) {
    _showTimeControl();
  }
  // _setSweepPanelVisibility();

}

//////////////////////////////////////////////////
// add/remove  sweep panel (archive mode only)

// void CartManager::_setSweepPanelVisibility()
// {
//   if (_sweepPanel != NULL) {
//     if (_archiveMode) {
//       _sweepPanel->setVisible(true);
//     } else {
//       _sweepPanel->setVisible(false);
//     }
//   }
// }

//////////////////////////////
// create actions for menus

void CartManager::_createActions()
{

  // show field menu
  _showFieldMenuAct = new QAction(tr("Fields"), this);
  _showFieldMenuAct->setStatusTip(tr("Show field menu"));
  connect(_showFieldMenuAct, &QAction::triggered,
          this, &CartManager::_showFieldMenu);
  
  // freeze display
  _freezeAct = new QAction(tr("Freeze"), this);
  _freezeAct->setShortcut(tr("Esc"));
  _freezeAct->setStatusTip(tr("Freeze display"));
  connect(_freezeAct, &QAction::triggered, this, &CartManager::_freeze);
  
  // show user click in dialog
  _showClickAct = new QAction(tr("Values"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, &QAction::triggered, this, &CartManager::_showClick);

  // show boundary editor dialog
  // _showBoundaryEditorAct = new QAction(tr("Boundary Editor"), this);
  // _showBoundaryEditorAct->setStatusTip(tr("Show boundary editor dialog"));
  // connect(_showBoundaryEditorAct, &QAction::triggered, this, &CartManager::showBoundaryEditor);
  
  // show time control window
  _showTimeControlAct = new QAction(tr("Movie"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, &QAction::triggered,
          this, &CartManager::_showTimeControl);
  
  // unzoom display
  
  _unzoomAct = new QAction(tr("Back"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to previous view"));
  _unzoomAct->setEnabled(false);
  connect(_unzoomAct, &QAction::triggered, this, &CartManager::_unzoom);

  // reload data
  
  _reloadAct = new QAction(tr("Reload"), this);
  _reloadAct->setStatusTip(tr("Reload data"));
  connect(_reloadAct, &QAction::triggered, this, &CartManager::_refresh);

  // clear display
  _clearAct = new QAction(tr("Clear"), this);
  _clearAct->setStatusTip(tr("Clear data"));
  connect(_clearAct, &QAction::triggered, _horiz, &HorizWidget::clear);
  connect(_clearAct, &QAction::triggered, _vert, &VertWidget::clear);

  // exit app
  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, &QAction::triggered, this, &CartManager::close);

  // file chooser for Open
  _openFileAct = new QAction(tr("O&pen"), this);
  _openFileAct->setShortcut(tr("Ctrl+F"));
  _openFileAct->setStatusTip(tr("Open File"));
  connect(_openFileAct, &QAction::triggered, this, &CartManager::_openFile);

  // file chooser for Save
  _saveFileAct = new QAction(tr("S&ave"), this);
  //_saveFileAct->setShortcut(tr("Ctrl+S"));
  _saveFileAct->setStatusTip(tr("Save File"));
  connect(_saveFileAct, &QAction::triggered, this, &CartManager::_saveFile);

  // show range rings

  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(_params.horiz_range_rings_on_at_startup);
  connect(_ringsAct, &QAction::triggered, _horiz, &HorizWidget::setRings);

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.horiz_grids_on_at_startup);
  connect(_gridsAct, &QAction::triggered, _horiz, &HorizWidget::setGrids);

  // show azimuth lines
  
  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params.horiz_azimuth_lines_on_at_startup);
  connect(_azLinesAct, &QAction::triggered, _horiz, &HorizWidget::setAngleLines);

  // show VERT window

  _showVertAct = new QAction(tr("Show VERT Window"), this);
  _showVertAct->setStatusTip(tr("Show the VERT Window"));
  connect(_showVertAct, &QAction::triggered, _vertWindow, &VertWidget::show);

  // howto and about
  
  _howtoAct = new QAction(tr("Howto"), this);
  _howtoAct->setStatusTip(tr("Show the application's Howto box"));
  connect(_howtoAct, &QAction::triggered, this, &CartManager::_howto);

  _aboutAct = new QAction(tr("About"), this);
  _aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAct, &QAction::triggered, this, &CartManager::_about);

  _aboutQtAct = new QAction(tr("About &Qt"), this);
  _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(_aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

  // save image
  
  _saveImageAct = new QAction(tr("Save-Image"), this);
  _saveImageAct->setStatusTip(tr("Save image to png file"));
  connect(_saveImageAct, &QAction::triggered, this, &CartManager::_saveImageToFile);

}

////////////////
// create menus

void CartManager::_createMenus()
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
  menuBar()->addAction(_unzoomAct);
  
  // time selector
  
  menuBar()->addAction(_showTimeControlAct);

  // add overlay menu
  
  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _populateOverlaysMenu();

  // misc actions
  
  menuBar()->addAction(_freezeAct);
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

void CartManager::_populateMapsMenu()
{

  // maps enabled

  _mapsEnabled = _params.maps_enabled_at_startup;
  _mapsEnabledAct = new QAction(tr("Maps enabled"), this);
  _mapsEnabledAct->setStatusTip(tr("Enable / disable all maps"));
  _mapsEnabledAct->setCheckable(true);
  _mapsEnabledAct->setChecked(_mapsEnabled);
  connect(_mapsEnabledAct, &QAction::toggled,
	  this, &CartManager::_setMapsEnabled);

  _mapsMenu->addAction(_mapsEnabledAct);
  _mapsMenu->addSeparator();
  
  // loop through map entries
  
  for (int imap = 0; imap < _params.maps_n; imap++) {

    Params::map_t &mparams = _params._maps[imap];

    // create action for this entry

    MapMenuItem *item = new MapMenuItem;
    QAction *act = new QAction;
    item->setMapParams(&mparams);
    item->setMapIndex(imap);
    item->setOverlay(gd.over[imap]);
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

void CartManager::_setMapsEnabled(bool enable)
{
  _mapsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> maps enabled? " << (_mapsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate products menu

void CartManager::_populateProductsMenu()
{

  // products enabled

  _productsEnabled = _params.symprods_enabled_at_startup;
  _productsEnabledAct = new QAction(tr("Products enabled"), this);
  _productsEnabledAct->setStatusTip(tr("Enable / disable all products"));
  _productsEnabledAct->setCheckable(true);
  _productsEnabledAct->setChecked(_productsEnabled);
  connect(_productsEnabledAct, &QAction::toggled,
	  this, &CartManager::_setProductsEnabled);

  _productsMenu->addAction(_productsEnabledAct);
  _productsMenu->addSeparator();
  
  // loop through wind entries
  
  for (int iprod = 0; iprod < _params.symprod_prod_info_n; iprod++) {
    
    Params::symprod_prod_info_t &prodParams = _params._symprod_prod_info[iprod];
    
    // create action for this entry

    ProdMenuItem *item = new ProdMenuItem;
    const Product *product = gd.prod_mgr->getProduct(iprod);
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

void CartManager::_setProductsEnabled(bool enable)
{
  _productsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> products enabled? " << (_productsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate winds menu

void CartManager::_populateWindsMenu()
{

  // winds enabled

  _windsEnabled = _params.winds_enabled_at_startup;
  _windsEnabledAct = new QAction(tr("Winds enabled"), this);
  _windsEnabledAct->setStatusTip(tr("Enable / disable all winds"));
  _windsEnabledAct->setCheckable(true);
  _windsEnabledAct->setChecked(_windsEnabled);
  connect(_windsEnabledAct, &QAction::toggled,
	  this, &CartManager::_setWindsEnabled);

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
    item->setWindData(&gd.layers.wind[iwind]);
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

void CartManager::_setWindsEnabled(bool enable)
{
  _windsEnabled = enable;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> winds enabled? " << (_windsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// populate zooms menu
// this is exclusive - only 1 item selected at a time

void CartManager::_populateZoomsMenu()
{

  _zoomsActionGroup = new QActionGroup(_zoomsMenu);
  _zoomsActionGroup->setExclusive(true);
  
  // loop through zoom entries
  
  for (int izoom = 0; izoom < _params.zoom_levels_n; izoom++) {
    
    Params::zoom_level_t &zparams = _params._zoom_levels[izoom];
    
    // create item for this entry
    
    ZoomMenuItem *item = new ZoomMenuItem;
    item->setZoomParams(&zparams);
    item->setZoomIndex(izoom);

    // create action for the item
    
    QAction *act = new QAction;
    item->setAction(act);
    act->setText(zparams.label);
    act->setStatusTip(tr("Select predefined zoom"));
    act->setCheckable(true);
    if (izoom == _params.start_zoom_level - 1) {
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

void CartManager::_populateOverlaysMenu()
{

  _overlaysMenu->addAction(_ringsAct);
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_azLinesAct);
  _overlaysMenu->addSeparator();
  _overlaysMenu->addAction(_showVertAct);

}

/////////////////////////////////////////////////////////////
// create the sweep panel
// buttons will be filled in by createSweepRadioButtons()

// void CartManager::_createSweepPanel()
// {
  
//   _sweepPanel = new QGroupBox("Sweeps", _main);
//   _sweepVBoxLayout = new QVBoxLayout;
//   _sweepPanel->setLayout(_sweepVBoxLayout);
//   _sweepPanel->setAlignment(Qt::AlignHCenter);

//   _sweepRButtons = new vector<QRadioButton *>();


// }

/////////////////////////////////////////////////////////////////////
// create radio buttons
// this requires that _sweepManager is up to date with sweep info

// void CartManager::_createSweepRadioButtons() 
// {

//   // fonts
  
//   QLabel dummy;
//   QFont font = dummy.font();
//   QFont fontm2 = dummy.font();
//   int fsize = _params.label_font_size;
//   int fsizem2 = _params.label_font_size - 2;
//   font.setPixelSize(fsize);
//   fontm2.setPixelSize(fsizem2);

// radar and site name
  
// char buf[256];
// _sweepRButtons = new vector<QRadioButton *>();

// for (int ielev = 0; ielev < (int) _sweepManager.getNSweeps(); ielev++) {

//   std::snprintf(buf, 256, "%.2f", _sweepManager.getFixedAngleDeg(ielev));
//   QRadioButton *radio1 = new QRadioButton(buf); 
//   radio1->setFont(fontm2);
    
//   if (ielev == _sweepManager.getGuiIndex()) {
//     radio1->setChecked(true);
//   }
    
//   _sweepRButtons->push_back(radio1);
//   _sweepVBoxLayout->addWidget(radio1);
    
//   // connect slot for sweep change

//   connect(radio1, SIGNAL(toggled(bool)), this, SLOT(_changeSweep(bool)));

// }

// // }

// /////////////////////////////////////////////////////////////////////
// // create sweep panel of radio buttons

// void CartManager::_clearSweepRadioButtons() 
// {

//   QLayoutItem* child;
//   if (_sweepVBoxLayout != NULL) {
//     while (_sweepVBoxLayout->count() !=0) {
//       child = _sweepVBoxLayout->takeAt(0);
//       if (child->widget() !=0) {
//         delete child->widget();
//       }
//       delete child;
//     }
//   }
 
// }

///////////////////////////////////////////////////////////////
// change sweep

// void CartManager::_changeSweep(bool value) {

//   if (_params.debug) {
//     cerr << "From CartManager: the sweep was changed ";
//     cerr << endl;
//   }

//   if (!value) {
//     return;
//   }

//   for (size_t sweepIndex = 0; sweepIndex < _sweepRButtons->size();
//        sweepIndex++) {
//     if (_sweepRButtons->at(sweepIndex)->isChecked()) {
//       if (_params.debug) {
//         cerr << "sweepRButton " << sweepIndex << " is checked" << endl;
//         cerr << "  moving to sweep index " << sweepIndex << endl;
//       }
//       // _sweepManager.setGuiIndex(sweepIndex);
//       _horiz->setStartOfSweep(true);
//       _vert->setStartOfSweep(true);
//       _moveUpDown();

//       // reloadBoundaries();
//       return;
//     }
//   } // ii

// }

///////////////////////////////////////////////////////////////
// change sweep
// only set the sweepIndex in one place;
// here, just move the radio button up or down one step
// when the radio button is changed, a signal is emitted and
// the slot that receives the signal will increase the sweepIndex
// value = +1 move forward
// value = -1 move backward in sweeps

// void CartManager::_changeSweepRadioButton(int increment)

// {
  
//   if (_params.debug) {
//     cerr << "-->> changing sweep index by increment: " << increment << endl;
//   }
  
//   if (increment != 0) {
//     // _sweepManager.changeSelectedIndex(increment);
//     // _sweepRButtons->at(_sweepManager.getGuiIndex())->setChecked(true);
//   }

// }

/////////////////////////////
// get data in realtime mode

void CartManager::_handleRealtimeData(QTimerEvent * event)

{

  _horiz->setArchiveMode(false);
  _vert->setArchiveMode(false);

}

///////////////////////////////////////
// set input file list for archive mode

// void CartManager::setArchiveFileList(const vector<string> &list,
//                                      bool fromCommandLine /* = true */)
// {

//   if (fromCommandLine && list.size() > 0) {
//     // determine start and end time from file list
//     RadxTime startTime, endTime;
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

// int CartManager::loadArchiveFileList()

// {
//   return 0;

// }

//////////////////////////////////////
// handle data in archive mode

void CartManager::_handleArchiveData(/*QTimerEvent * event*/)

{
  
  if (_params.debug) {
    cerr << "handling archive data ..." << endl;
  }

  //   _horiz->setArchiveMode(true);
  //   _horiz->setStartOfSweep(true);

  //   _vert->setArchiveMode(true);
  //   _vert->setStartOfSweep(true);

  //   // set cursor to wait cursor

  //   this->setCursor(Qt::WaitCursor);
  //   _timeControl->setCursor(Qt::WaitCursor);
  
  //   // get data
  //   try {
  //     _getArchiveData();
  //   } catch (FileIException &ex) {
  //     this->setCursor(Qt::ArrowCursor);
  //     _timeControl->setCursor(Qt::ArrowCursor);
  //     return;
  //   }
  //   _activateArchiveRendering();

  //   // set up sweep GUI

  //   _clearSweepRadioButtons();
  //   _createSweepRadioButtons();
  
  //   if (_vol.checkIsRhi()) {
  //     _vertMode = true;
  //   } else {
  //     _vertMode = false;
  //   }

  //   // plot the data
  
  //   _plotArchiveData();
  //   this->setCursor(Qt::ArrowCursor);
  //   _timeControl->setCursor(Qt::ArrowCursor);

  // if (_firstTime) {
  //   _firstTime = false;
  // }
  
}

/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure

// int CartManager::_getArchiveData()

// {

//   // set up file object for reading
  
//   RadxFile file;
//   _vol.clear();
//   _setupVolRead(file);
  
//   if (_timeControl->getArchiveScanIndex() >= 0 &&
//       _timeControl->getArchiveScanIndex() < (int) _archiveFileList.size()) {
    
//     string inputPath = _archiveFileList[_timeControl->getArchiveScanIndex()];
    
//     if(_params.debug) {
//       cerr << "  reading data file path: " << inputPath << endl;
//       cerr << "  archive file index: "
//            << _timeControl->getArchiveScanIndex() << endl;
//     }
    
//     if (file.readFromPath(inputPath, _vol)) {
//       string errMsg = "ERROR - Cannot retrieve archive data\n";
//       errMsg += "CartManager::_getArchiveData\n";
//       errMsg += file.getErrStr() + "\n";
//       errMsg += "  path: " + inputPath + "\n";
//       cerr << errMsg;
//       if (!_params.images_auto_create)  {
//         QErrorMessage errorDialog;
//         errorDialog.setMinimumSize(400, 250);
//         errorDialog.showMessage(errMsg.c_str());
//         errorDialog.exec();
//       }
//       return -1;
//     } 

//   }

//   // set plot times
  
//   _plotStartTime = _vol.getStartTimeSecs();
//   _plotEndTime = _vol.getEndTimeSecs();

//   char text[128];
//   snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
//            _plotStartTime.getYear(),
//            _plotStartTime.getMonth(),
//            _plotStartTime.getDay(),
//            _plotStartTime.getHour(),
//            _plotStartTime.getMin(),
//            _plotStartTime.getSec());

//   _timeControl->setSelectedTimeLabel(text);
  
//   // adjust angles for elevation surveillance if needed

//   _vol.setAnglesForElevSurveillance();

//   // compute the fixed angles from the rays
//   // so that we reflect reality
  
//   _vol.computeFixedAnglesFromRays();

//   // load the sweep manager
  
//   // _sweepManager.set(_vol);

//    _platform = _vol.getPlatform();
   
//   return 0;

// }


/////////////////////////////
// apply new, edited  data in archive mode
// the volume has been updated 

// void CartManager::_applyDataEdits()
// {

//   if (_params.debug) {
//     std::ofstream outfile("/tmp/voldebug_CartManager_applyDataEdits.txt");
//     _vol.printWithFieldData(outfile);  
//     outfile << "_vol = " << &_vol << endl;
//   }

//   _plotArchiveData();
// }

/////////////////////////////
// plot data in archive mode

// void CartManager::_plotArchiveData()

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

// void CartManager::_setupVolRead(RadxFile &file)
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

// void CartManager::_handleRay(RadxPlatform &platform, RadxRay *ray)
  
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

// void CartManager::_storeRayLoc(const RadxRay *ray, 
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

// void CartManager::_clearRayOverlap(const int start_index, const int end_index)
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

void CartManager::_freeze()
{
  if (_frozen) {
    _frozen = false;
    _freezeAct->setText("Freeze");
    _freezeAct->setStatusTip(tr("Click to freeze display, or hit ESC"));
  } else {
    _frozen = true;
    _freezeAct->setText("Unfreeze");
    _freezeAct->setStatusTip(tr("Click to unfreeze display, or hit ESC"));
  }
}

////////////////////////////////////////////
// unzoom

void CartManager::_unzoom()
{
  _horiz->unzoomView();
  _unzoomAct->setEnabled(false);
}

////////////////////////////////////////////
// refresh

void CartManager::_refresh()
{
}

///////////////////////////////////////////////////////////
// respond to change field request from field button group

void CartManager::_changeField(int fieldId, bool guiMode)

{
  _selectedField = _fields[fieldId];
  
  if (_params.debug) {
    cerr << "Changing to field id: " << fieldId << endl;
    _selectedField->print(cerr);
  }

  // if we click the already-selected field, go back to previous field

  if (guiMode) {
    if (_fieldNum == fieldId && _prevFieldNum >= 0) {
      QRadioButton *button =
        (QRadioButton *) _fieldGroup->button(_prevFieldNum);
      button->click();
      return;
    }
  }

  _prevFieldNum = _fieldNum;
  _fieldNum = fieldId;
  
  _horiz->selectVar(_fieldNum);
  _vert->selectVar(_fieldNum);

  // _colorBar->setColorMap(&_fields[_fieldNum]->getColorMap());

  _selectedName = _selectedField->getName();
  _selectedLabel = _selectedField->getLabel();
  _selectedUnits = _selectedField->getUnits();
  
  _selectedLabelWidget->setText(_selectedLabel.c_str());
  char text[128];
  if (_selectedField->getSelectValue() > -9990) {
    sprintf(text, "%g %s", 
            _selectedField->getSelectValue(),
            _selectedField->getUnits().c_str());
  } else {
    text[0] = '\0';
  }
  _valueLabel->setText(text);

  // refreshBoundaries();
}

// TODO: need to add the background changed, etc. 
void CartManager::colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
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

void CartManager::setVolume() { // const RadxVol &radarDataVolume) {

  LOG(DEBUG_VERBOSE) << "enter";

  // _applyDataEdits(); // radarDataVolume);

  LOG(DEBUG_VERBOSE) << "exit";



}

///////////////////////////////////////////////////
// respond to a change in click location on the HORIZ

void CartManager::_horizLocationClicked(double xkm, double ykm, 
                                        const RadxRay *closestRay)

{

  cerr << "1111111111 horizLocationClicked, xkm, km: " << xkm << ", " << ykm << endl;
  
#ifdef NOTNOW
  
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

#endif
  
}

///////////////////////////////////////////////////
// respond to a change in click location on the VERT

void CartManager::_vertLocationClicked(double xkm, double ykm, 
                                       const RadxRay *closestRay)
  
{

  _locationClicked(xkm, ykm, closestRay);

}

////////////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void CartManager::_locationClicked(double xkm, double ykm,
                                   const RadxRay *ray)
  
{

  if (_params.debug) {
    cerr << "*** Entering CartManager::_locationClicked()" << endl;
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
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateClicked->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) (ray->getNanoSecs() / 1000000)));
  _timeClicked->setText(text);
  
  _setText(text, "%6.2f", ray->getElevationDeg());
  _elevClicked->setText(text);
  
  _setText(text, "%6.2f", ray->getAzimuthDeg());
  _azClicked->setText(text);
  
  _setText(text, "%d", gateNum);
  _gateNumClicked->setText(text);
  
  _setText(text, "%6.2f", rangeKm);
  _rangeClicked->setText(text);

  if (_radarAltKm > -9990) {
    double gateHtKm = _beamHt.computeHtKm(ray->getElevationDeg(), rangeKm);
    _setText(text, "%6.2f (km)", gateHtKm);
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
        sprintf(text, "%g %s", val, fieldUnits.c_str());
      } else {
        sprintf(text, "%g %s", -9999.0, fieldUnits.c_str());
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
          sprintf(text, "----");
        } else if (fabs(val) > 10) {
          sprintf(text, "%.2f", val);
        } else {
          sprintf(text, "%.3f", val);
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

//////////////////////////////////////////////
// get size of table widget

QSize CartManager::_getTableWidgetSize(QTableWidget *t,
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

void CartManager::_createFieldMenu()
{

  // top-level
  
  _fieldMenu = new QDialog(this);
  _fieldMenu->setWindowTitle("Select active field");
  QPoint pos(0,0);
  _fieldMenu->move(pos);

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
      FieldTableItem *item = new FieldTableItem("");
      if (fieldNum < _params.fields_n) {
        if (strlen(_params._fields[fieldNum].group_name) > 0) {
          item->setText(_params._fields[fieldNum].button_label);
          item->setFieldIndex(fieldNum);
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
  
  // connect(_fieldTable, SIGNAL(cellClicked(const int, const int)),
  //         _fieldTable, SLOT(setCurrentCell(const int, const int)));
  
}

/////////////////////////////////////
// show the field menu

void CartManager::_showFieldMenu()
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

void CartManager::_placeFieldMenu()
{
  if (_fieldMenu) {
    if (!_fieldMenuPlaced) {
      int topFrameWidth = _fieldMenu->geometry().y() - _fieldMenu->y();
      int topFrameHeight =
        _fieldMenu->frameGeometry().height() - _fieldMenu->height();
      QPoint pos;
      pos.setX(x() + (frameGeometry().width()));
      pos.setY(y());
      _fieldMenu->move(pos);
      if (topFrameWidth != 0 || topFrameHeight != 0) {
        _fieldMenuPlaced = true;
      }
    }
  }
}

//////////////////////////////////////////////
// create the time panel

void CartManager::_createTimeControl()
{
  
  _timeControl = new TimeControl(this, _params);
  _timeControl->setStartTime(_archiveStartTime);

}

/////////////////////////////////////
// show the time controller dialog

void CartManager::_showTimeControl()
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

void CartManager::_placeTimeControl()
{
  if (_timeControl) {
    if (!_timeControlPlaced) {
      int topFrameWidth = _timeControl->geometry().y() - _timeControl->y();
      int topFrameHeight =
        _timeControl->frameGeometry().height() - _timeControl->height();
      QPoint pos;
      pos.setX(x() + (frameGeometry().width() / 2) -
               (_timeControl->frameGeometry().width()/2));
      pos.setY(y() + frameGeometry().height());
      _timeControl->move(pos);
      if (topFrameWidth != 0 || topFrameHeight != 0) {
        _timeControlPlaced = true;
      }
    }
  }
}

// BoundaryEditor circle (radius) slider has changed value

void CartManager::_circleRadiusSliderValueChanged(int value)
{
  //returns true if existing circle was resized with this new radius
  // if (BoundaryPointEditor::Instance()->setCircleRadius(value)){
  //   _horiz->update();
  // }
}

// BoundaryEditor brush (size) slider has changed value

void CartManager::_brushRadiusSliderValueChanged(int value)
{
  // BoundaryPointEditor::Instance()->setBrushRadius(value);
}

// set the directory (_boundaryDir) into which boundary
// files will be read/written for current radar file (_openFilePath)

void CartManager::setBoundaryDir()
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

void CartManager::_clear()
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

void CartManager::setArchiveMode(bool state)
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

void CartManager::_activateRealtimeRendering()
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

void CartManager::_activateArchiveRendering()
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

void CartManager::_createRealtimeImageFiles()
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

void CartManager::_createArchiveImageFiles()
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
        _timeControl->setStartTime(_timeControl->getStartTime() +
                                   _imagesScanIntervalSecs);
      }
      
    }
    
  } else if (_params.images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    
    for (RadxTime stime = _imagesStartTime;
         stime <= _imagesEndTime;
         stime += _params.images_schedule_interval_secs) {
      
      _timeControl->setStartTime(stime);
      _timeControl->setEndTime(_timeControl->getStartTime() +
                               _imagesScanIntervalSecs);
      
      _createImageFilesAllLevels();
      
    } // stime
    
  } // if (_params.images_creation_mode ...

}

/////////////////////////////////////////////////////
// creating one image per field, for each level

void CartManager::_createImageFilesAllLevels()
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

void CartManager::_createImageFiles()
{

  if (_params.debug) {
    cerr << "CartManager::_createImageFiles()" << endl;
  }

  PMU_auto_register("createImageFiles");

  // plot the data

  _horiz->setStartOfSweep(true);
  _vert->setStartOfSweep(true);
  _handleArchiveData();

  // set times from plots

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

  if (_params.debug) {
    cerr << "Done creating image files" << endl;
  }

}

string CartManager::_getOutputPath(bool interactive, string &outputDir, string fileExt)
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
    sprintf(dayStr, "%.4d%.2d%.2d", _plotStartTime.getYear(), _plotStartTime.getMonth(), _plotStartTime.getDay());
    outputDir += PATH_DELIM;
    outputDir += dayStr;
  }

  // make sure output dir exists

  if (ta_makedir_recurse(outputDir.c_str())) {
    string errmsg("Cannot create output dir: " + outputDir);
    cerr << "ERROR - CartManager::_saveImageToFile()" << endl;
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
      sprintf(timeStr, "%.4d%.2d%.2d%.2d%.2d%.2d",
              _plotStartTime.getYear(),
              _plotStartTime.getMonth(),
              _plotStartTime.getDay(),
              _plotStartTime.getHour(),
              _plotStartTime.getMin(),
              _plotStartTime.getSec());
    } else {
      sprintf(timeStr, "%.4d%.2d%.2d%.2d%.2d",
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

void CartManager::_saveImageToFile(bool interactive)
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
    cerr << "ERROR - CartManager::_saveImageToFile()" << endl;
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
    ldataInfo.setWriter("Qucid");
    ldataInfo.setDataFileExt(_params.images_file_name_extension);
    ldataInfo.setDataType(_params.images_file_name_extension);
    ldataInfo.setRelDataPath(relPath);
    
    if(ldataInfo.write(_plotStartTime.utime())) {
      cerr << "ERROR - CartManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.images_write_latest_data_info)

}



void CartManager::ShowContextMenu(const QPoint &pos) {
  _horiz->ShowContextMenu(pos /*, &_vol*/);
}


/////////////////////////////////////////////////////
// howto help

void CartManager::_howto()
{
  string text;
  text += "HOWTO HINTS FOR HAWK-EYE in POLAR mode\n";
  text += "======================================\n";
  text += "\n";
  text += "To go forward  in time, click in data window, hit Right Arrow\n";
  text += "To go backward in time, click in data window, hit Left  Arrow\n";
  text += "\n";
  text += "To change fields, click on field buttons\n";
  text += "  Once active, you can use the arrow keys to change the field selection\n";
  text += "  Hit '.' to toggle between the two latest fields\n";
  text += "\n";
  text += "Hot-keys for fields:\n";
  text += "  Use NUMBER or LETTER keys to display RAW fields\n";
  text += "  Use ALT-NUMBER and ALT-LETTER keys to display FILTERED fields\n";
  text += "\n";
  text += "To see field data at a point:\n";
  text += "  Click in main window\n";
  QMessageBox::about(this, tr("Howto dialog"), tr(text.c_str()));
}

#ifdef NOTNOW
// Creates the boundary editor dialog and associated event slots
void CartManager::_createBoundaryEditorDialog()
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
          this, &CartManager::polygonBtnBoundaryEditorClick);

  _boundaryEditorCircleBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorCircleBtn->setMaximumWidth(130);
  _boundaryEditorCircleBtn->setText(" Circle  ");
  _boundaryEditorCircleBtn->setIcon(QIcon("images/circle.png"));
  _boundaryEditorCircleBtn->setCheckable(TRUE);
  _boundaryEditorCircleBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorCircleBtn, ++row, 0);
  connect(_boundaryEditorCircleBtn, &QPushButton::clicked,
          this, &CartManager::circleBtnBoundaryEditorClick());

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
          this, &CartManager::_circleRadiusSliderValueChanged(int));

  _boundaryEditorBrushBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorBrushBtn->setMaximumWidth(130);
  _boundaryEditorBrushBtn->setText(" Brush ");
  _boundaryEditorBrushBtn->setIcon(QIcon("images/brush.png"));
  _boundaryEditorBrushBtn->setCheckable(TRUE);
  _boundaryEditorBrushBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorBrushBtn, ++row, 0);
  connect(_boundaryEditorBrushBtn, &QPushButton::clicked,
          this, &CartManager::brushBtnBoundaryEditorClick());

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
          this, &CartManager::_brushRadiusSliderValueChanged(int));

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
          this, &CartManager::onBoundaryEditorListItemClicked(QListWidgetItem*));

  // horizontal layout contains the "Clear", "Help", and "Save" buttons
  QHBoxLayout *hLayout = new QHBoxLayout;
  _boundaryEditorDialogLayout->addLayout(hLayout, ++row, 0, 1, 2);
  
  _boundaryEditorClearBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorClearBtn->setText("Clear");
  hLayout->addWidget(_boundaryEditorClearBtn);
  connect(_boundaryEditorClearBtn, &QPushButton::clicked,
          this, &CartManager::clearBoundaryEditorClick());

  _boundaryEditorHelpBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorHelpBtn->setText("Help");
  hLayout->addWidget(_boundaryEditorHelpBtn);
  connect(_boundaryEditorHelpBtn, &QPushButton::clicked,
          this, &CartManager::helpBoundaryEditorClick());
  
  _boundaryEditorSaveBtn = new QPushButton(_boundaryEditorDialog);
  _boundaryEditorSaveBtn->setText("Save");
  hLayout->addWidget(_boundaryEditorSaveBtn);

}

#endif

/////////////////////////////////////////////////////////
// check for field change

void CartManager::_checkForFieldChange()
{

  if (_fieldTable == NULL) {
    return;
  }

  if (_fieldTableCurrentRow == _fieldTable->currentRow() &&
      _fieldTableCurrentColumn == _fieldTable->currentColumn()) {
    // no change
    return;
  }
  
  const FieldTableItem *item =
    (const FieldTableItem *) _fieldTable->item(_fieldTable->currentRow(),
                                               _fieldTable->currentColumn());
  
  if (item->text().toStdString().size() == 0) {
    _fieldTable->setCurrentCell(_fieldTableCurrentRow,
                                _fieldTableCurrentColumn);
  } else {
    _fieldTableCurrentColumn = _fieldTable->currentColumn();
    _fieldTableCurrentRow = _fieldTable->currentRow();
    int fieldNum = item->getFieldIndex();
    set_field(fieldNum);
    gd.h_win.page = fieldNum;
    gd.v_win.page = fieldNum;
    if (_params.debug) {
      const Params::field_t *fparams = item->getFieldParams();
      cerr << "Changing to field: " << fparams->button_label << endl;
      cerr << "              url: " << fparams->url << endl;
      cerr << "         fieldNum: " << fieldNum << endl;
      cerr << "      field_label: " << gd.mrec[fieldNum]->field_label << endl;
      cerr << "      button_name: " << gd.mrec[fieldNum]->button_name << endl;
      cerr << "      legend_name: " << gd.mrec[fieldNum]->legend_name << endl;
    }
  }
  
}

/////////////////////////////////////////////////////////
// handle first time event

void CartManager::_handleFirstTimerEvent()
{

  _horiz->resize(_horizFrame->width(), _horizFrame->height());
  
  // Set the size of the second column to the size of the largest
  // label.  This should keep the column from wiggling as the values change.
  // The default values for the labels must be their maximum size for this
  // to work.  This is ugly, but it works.
  
  int maxWidth = 0;
  for (size_t ii = 0; ii < _valsRight.size(); ii++) {
    if (maxWidth < _valsRight[ii]->width()) {
      maxWidth = _valsRight[ii]->width();
    }
  }
  _statusLayout->setColumnMinimumWidth(1, maxWidth);
  
}

/////////////////////////////////////////////////////////
// read click point from FMQ - i.e. from another app

void CartManager::_readClickPoint()
{

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
  
}

  
/**********************************************************************
 * HANDLE_CLIENT_EVENT: 
 */

void CartManager::_handleClientEvent()
{

  time_t clock;
  
  if(gd.debug1) fprintf(stderr,"Found Client Event: %d, Args: %s\n",
			gd.coord_expt->client_event,gd.coord_expt->client_args);
  
  switch(gd.coord_expt->client_event) {
    case NEW_MDV_AVAIL:
      remote_new_mdv_avail(gd.coord_expt->client_args);
      break;
      
    case NEW_SPDB_AVAIL:
      remote_new_spdb_avail(gd.coord_expt->client_args);
      break;

    case RELOAD_DATA:
      invalidate_all_data();
      set_redraw_flags(1,1);
      break;

    case SET_FRAME_NUM:
      int	frame;
      if((sscanf(gd.coord_expt->client_args,"%d",&frame)) == 1) {
        /* Sanity Check */
        if(frame <= 0 ) frame = 1;
        if(frame > gd.movie.num_frames ) frame = gd.movie.num_frames ;

        gd.movie.cur_frame = frame - 1;
      } else {
        fprintf(stderr,"Invalid SET_FRAME_NUM: Args: %s\n",gd.coord_expt->client_args);
      }
      break;

    case SET_NUM_FRAMES:
      int	nframes;
      if((sscanf(gd.coord_expt->client_args,"%d",&nframes)) == 1) {
        set_end_frame(nframes);
      } else {
        fprintf(stderr,"Invalid SET_NUM_FRAMES: Args: %s\n",gd.coord_expt->client_args);
      }
      break;

    case SET_REALTIME:
      gd.movie.mode = REALTIME_MODE;
      gd.movie.cur_frame = gd.movie.num_frames -1;
      gd.movie.end_frame = gd.movie.num_frames -1;
      clock = time(0);
      gd.movie.start_time = clock - (time_t) ((gd.movie.num_frames -1) *
                                              gd.movie.time_interval_mins * 60.0);
      gd.movie.start_time -= (gd.movie.start_time % gd.movie.round_to_seconds);
      gd.coord_expt->runtime_mode = RUNMODE_REALTIME;	
      gd.coord_expt->time_seq_num++;

      reset_time_points();
      invalidate_all_data();
      set_redraw_flags(1,1);

      // Set forecast and past time choosers to "now"
      // xv_set(gd.fcast_pu->fcast_st,PANEL_VALUE,0,NULL);
      // xv_set(gd.past_pu->past_hr_st,PANEL_VALUE,0,NULL);
      // Set movie mode widget to REALTIME 
      // xv_set(gd.movie_pu->movie_type_st,PANEL_VALUE,0,NULL);

      break;

    case SET_TIME:
      UTIMstruct ts;
      time_t interest_time;

      if((sscanf(gd.coord_expt->client_args,"%ld %ld %ld %ld %ld %ld",
                 &ts.year,&ts.month,&ts.day, &ts.hour,&ts.min,&ts.sec)) == 6) {
		
        interest_time = UTIMdate_to_unix(&ts);

        set_display_time(interest_time);
        invalidate_all_data();
        set_redraw_flags(1,1);
      } else {
        fprintf(stderr,"Invalid SET_TIME Args: %s\n",gd.coord_expt->client_args);
      }
      break;

    default: {}

  }

  // Reset  the command
  gd.coord_expt->client_event = NO_MESSAGE;
}


/**********************************************************************
 * CHECK_FOR_EXPIRED_DATA: Check all data and determine if 
 * the data's expiration time has been exceeded - Mark it invalid if so.
 */

void CartManager::_checkForExpiredData(time_t tm)
{
  int i;

  // Mark all data past the expiration data as invalid
  /* look thru primary data fields */
  for (i=0; i < gd.num_datafields; i++) {
    /* if data has expired or field should be updated */
    if (gd.mrec[i]->h_mhdr.time_expire < tm ) {
      //        if(gd.debug1) fprintf(stderr,"Field: %s expired at %s\n",
      //		 gd.mrec[i]->button_name,
      //		 asctime(gmtime(&((time_t) gd.mrec[i]->h_mhdr.time_expire))));
      gd.mrec[i]->h_data_valid = 0;
      gd.mrec[i]->v_data_valid = 0;
    }
  }

  /* Look through wind field data too */
  for (i=0; i < gd.layers.num_wind_sets; i++ ) {
    if (gd.layers.wind[i].active) {
      if (gd.layers.wind[i].wind_u->h_mhdr.time_expire < tm) {
        gd.layers.wind[i].wind_u->h_data_valid = 0;
        gd.layers.wind[i].wind_u->v_data_valid = 0;
      }
      if (gd.layers.wind[i].wind_v->h_mhdr.time_expire < tm) {
        gd.layers.wind[i].wind_v->h_data_valid = 0;
        gd.layers.wind[i].wind_v->v_data_valid = 0;
      }
      if (gd.layers.wind[i].wind_w != NULL && gd.layers.wind[i].wind_w->h_mhdr.time_expire < tm) {
        gd.layers.wind[i].wind_w->h_data_valid = 0;
        gd.layers.wind[i].wind_w->v_data_valid = 0;
      }
    }
  }
}

/**********************************************************************
 * _CHECKFORDATAUPDATES: Check all data and determine if 
 * new data has arrived - Mark it invalid if so.
 */

void CartManager::_checkForDataUpdates(time_t tm)
{
  DmapAccess dmap;
  int i,j;
  char *start_ptr, *end_ptr;
  char dir_buf[MAX_PATH_LEN];

  if( strlen(_params.datamap_host) < 2 || dmap.reqAllInfo(_params.datamap_host) != 0) {

    // Force a reload of the data
    reset_data_valid_flags(1,1);
    if (gd.prod_mgr) {
      gd.prod_mgr->reset_product_valid_flags();
      gd.prod_mgr->reset_times_valid_flags();
    }

  } else {   // Got valid  data back from the Data Mapper

    int nsets = dmap.getNInfo();


    if(gd.debug1) fprintf(stderr,"Found %d Datamapper info sets\n",nsets);

    /* look thru all data fields */
    for (i=0; i < gd.num_datafields; i++) {
      // pull out dir from URL

      end_ptr = strrchr(gd.mrec[i]->url,'&');
      if(end_ptr == NULL) continue;  // broken URL.

      start_ptr =  strrchr(gd.mrec[i]->url,':');
      if(start_ptr == NULL) {
	start_ptr =  gd.mrec[i]->url; // Must be a local file/dir based URL
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
          if (gd.mrec[i]->h_date.unix_time < (int) info.end_time) {
            gd.mrec[i]->h_data_valid = 0;
            gd.mrec[i]->v_data_valid = 0;
          }
	}
      }
    }

    for (i=0; i < gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (gd.layers.wind[i].active) {
	// pull out dir from URL

	end_ptr = strrchr(gd.layers.wind[i].wind_u->url,'&');
	if(end_ptr == NULL) continue;  // broken URL.

	start_ptr =  strrchr(gd.layers.wind[i].wind_u->url,':');
	if(start_ptr == NULL) {
          start_ptr =  gd.mrec[i]->url; // Must be a local file/dir based URL
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
            if (gd.layers.wind[i].wind_u->h_date.unix_time < (int) info.end_time) {
              gd.layers.wind[i].wind_u->h_data_valid = 0;
              gd.layers.wind[i].wind_u->v_data_valid = 0;
              gd.layers.wind[i].wind_v->h_data_valid = 0;
              gd.layers.wind[i].wind_v->v_data_valid = 0;
              if (gd.layers.wind[i].wind_w != NULL) {
		gd.layers.wind[i].wind_w->h_data_valid = 0;
		gd.layers.wind[i].wind_w->v_data_valid = 0;
              }
            }
          }  // If a match
	} // For all data mapper info
      }   // If wind layer is active
    }     // For all wind layers

    gd.prod_mgr->check_product_validity(tm, dmap);
  } // If data mapper info is availible
}

////////////////////////////////////////////////////////////////// 
// _checkWhatNeedsRendering:

void CartManager::_checkWhatNeedsRendering(int frame_index)
{
  int i;
  
  // If data used to draw plan view is invalid
  // Indicate plan view image needs rerendering
  
  if (gd.mrec[gd.h_win.page]->h_data_valid == 0 ||
      gd.prod_mgr->num_products_invalid() > 0) {
    gd.movie.frame[frame_index].redraw_horiz = 1;
    gd.h_win.redraw[gd.h_win.page] = 1;
  }

  for ( i=0; i < gd.layers.num_wind_sets; i++ ) {
    // Look through wind field data too
    if (gd.layers.wind[i].active) {
      if (gd.layers.wind[i].wind_u->h_data_valid == 0) {
        gd.movie.frame[frame_index].redraw_horiz = 1;
        gd.h_win.redraw[gd.h_win.page] = 1;
      }
      if (gd.layers.wind[i].wind_v->h_data_valid == 0) {
        gd.movie.frame[frame_index].redraw_horiz = 1;
        gd.h_win.redraw[gd.h_win.page] = 1;
      }
      if (gd.layers.wind[i].wind_w != NULL &&
          gd.layers.wind[i].wind_w->h_data_valid == 0) {
        gd.movie.frame[frame_index].redraw_horiz = 1;
        gd.h_win.redraw[gd.h_win.page] = 1;
      }
    }
  }

  // Check overlay contours if active

  for(i= 0; i < NUM_CONT_LAYERS; i++) {
    if(gd.layers.cont[i].active) {
      if(gd.mrec[gd.layers.cont[i].field]->h_data_valid == 0) {
	gd.movie.frame[frame_index].redraw_horiz = 1;
	gd.h_win.redraw[gd.h_win.page] = 1;
      }
    }
  } 

  // If data used to draw cross section is invalid
  // Indicate pcross section image needs rerendering
  
  if (gd.v_win.active ) {
    if(gd.mrec[gd.v_win.page]->v_data_valid == 0)  {
      gd.movie.frame[frame_index].redraw_vert = 1;
      gd.v_win.redraw[gd.v_win.page] = 1;
    }

    for ( i=0; i < gd.layers.num_wind_sets; i++ ) {
      /* Look through wind field data too */
      if (gd.layers.wind[i].active) {
	if (gd.layers.wind[i].wind_u->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
	if (gd.layers.wind[i].wind_v->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
	if (gd.layers.wind[i].wind_w != NULL &&
            gd.layers.wind[i].wind_w->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
      }
    }

    /* Check overlay contours if active */
    for(i= 0; i < NUM_CONT_LAYERS; i++) {
      if(gd.layers.cont[i].active) {
	if(gd.mrec[gd.layers.cont[i].field]->v_data_valid == 0) {
          gd.movie.frame[frame_index].redraw_vert = 1;
          gd.v_win.redraw[gd.v_win.page] = 1;
	}
      }
    } 
  }
}

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

void CartManager::_ciddTimerFunc(QTimerEvent *event)
{

  met_record_t *mr;
  int index,flag = 0;
  int msec_diff = 0;
  int msec_delay = 0;
  long tm = 0;

  Pixmap h_xid = 0;
  Pixmap v_xid = 0;

  struct timezone cur_tz;

  if(gd.io_info.outstanding_request) {
    check_for_io();
  }

  if(gd.coord_expt->client_event != NO_MESSAGE) {
    _handleClientEvent();
  }

  gettimeofday(&cur_tm,&cur_tz);

  /* Update the current time ticker if necessary */
  if(cur_tm.tv_sec > last_tick) {
    if(!_params.run_once_and_exit) {
      char buf[128];
      sprintf(buf,"Idle %d secs, Req: %d, Mode: %d, Type: %d",
              (int) (cur_tm.tv_sec - gd.last_event_time),
              gd.io_info.outstanding_request,
              gd.io_info.mode,
              gd.io_info.request_type);
      if(gd.debug || gd.debug1 || gd.debug2) {
        PMU_force_register(buf);
      } else {
        PMU_auto_register(buf);
      }

    }
    update_ticker(cur_tm.tv_sec);
    last_tick = cur_tm.tv_sec;
    if(gd.last_event_time < (last_tick - _params.idle_reset_seconds)) {
      reset_display();
      gd.last_event_time = last_tick;
    }
  }

  msec_delay = gd.movie.display_time_msec;

  /**** Get present frame index *****/
  if (gd.movie.cur_frame < 0) {
    index = gd.movie.num_frames - 1;
  } else {
    index = gd.movie.cur_frame;
  }
  // If no images or IO are pending - Check for remote commands
  if(gd.image_needs_saved == 0 &&
     gd.movie.frame[index].redraw_horiz == 0 &&
     gd.io_info.outstanding_request == 0) { 

    if(gd.v_win.active == 0 || gd.movie.frame[index].redraw_vert == 0) {
      if(gd.remote_ui != NULL) ingest_remote_commands();
    }
  }

  // Update the times in the Coordinate SHMEM
  gd.coord_expt->epoch_start = gd.epoch_start;
  gd.coord_expt->epoch_end = gd.epoch_end; 
  gd.coord_expt->time_min = gd.movie.frame[index].time_start;
  gd.coord_expt->time_max = gd.movie.frame[index].time_end;
  if(gd.movie.movie_on) { 
    gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
  } else {
    gd.coord_expt->time_cent = gd.coord_expt->time_min +
      (gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
  }
  gd.coord_expt->time_current_field = gd.mrec[gd.h_win.page]->h_mhdr.time_centroid;

  if (gd.movie.movie_on ) {
    flag = 1;        /* set OK state */
    if (gd.movie.frame[index].redraw_horiz != 0) flag = 0;
    if (gd.movie.frame[index].redraw_vert != 0 && gd.v_win.active) flag = 0;

    msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
    if (flag && msec_diff > gd.movie.display_time_msec) {
      /* Advance Movie frame */
      gd.movie.cur_frame += gd.movie.sweep_dir;    

      /* reset to beginning of the loop if needed */
      if (gd.movie.cur_frame > gd.movie.end_frame) {
	if(gd.series_save_active) { // End of the Series Save - Turn off
          char cmd[4096];
          gd.series_save_active = 0;
          gd.movie.movie_on = 0;
          if(_params.series_convert_script !=NULL) {
            STRncopy(cmd,_params.series_convert_script,4096);
            for(int ii= gd.movie.start_frame; ii <= gd.movie.end_frame; ii++) {
              STRconcat(cmd," ",4096);
              STRconcat(cmd,gd.movie.frame[ii].fname,4096);
            }
            printf("Running: %s\n",cmd);
          }
          set_busy_state(1);
          safe_system(cmd,_params.complex_command_timeout_secs);

          if(gd.v_win.active) {
            if(_params.series_convert_script !=NULL) {
              STRncopy(cmd,_params.series_convert_script,4096);
              for(int ii= gd.movie.start_frame; ii <= gd.movie.end_frame; ii++) {
                STRconcat(cmd," ",4096);
                STRconcat(cmd,gd.movie.frame[ii].vfname,4096);
              }
              printf("Running: %s\n",cmd);
            }
            set_busy_state(1);
            safe_system(cmd,_params.complex_command_timeout_secs);
          }

          set_busy_state(0);
          gd.movie.cur_frame = gd.movie.end_frame;
	} else {
          if(gd.movie.sweep_on) {
            gd.movie.sweep_dir = -1;
            gd.movie.cur_frame = gd.movie.end_frame -1;
          } else {
            gd.movie.cur_frame = gd.movie.start_frame -1;
          }
	}
      }

      if(gd.movie.cur_frame < gd.movie.start_frame) { 
        gd.movie.sweep_dir = 1;
        if(gd.movie.sweep_on) {
          gd.movie.cur_frame = gd.movie.start_frame+1;
        }
      }
	
      if (gd.movie.cur_frame == gd.movie.end_frame) {
        msec_delay = gd.movie.delay;
      }

      /**** recalc current frame index *****/
      if (gd.movie.cur_frame < 0) {
        index =  gd.movie.num_frames - 1;
      } else {
        index = gd.movie.cur_frame;
      }
    }
  }
	
  /* Set up convienient pointer to main met record */
  mr = gd.mrec[gd.h_win.page];

  /* Decide which Pixmaps to use for rendering */
  if (gd.movie.movie_on ) {
    /* set to the movie frame Pixmaps */
    h_xid = gd.movie.frame[index].h_xid;
    if (h_xid == 0) {
      if(mr->auto_render) {    
        h_xid = gd.h_win.page_xid[gd.h_win.page];
      } else {
        h_xid = gd.h_win.tmp_xid;
      }
    }

    v_xid = gd.movie.frame[index].v_xid;
    if (v_xid == 0) {
      if(mr->auto_render) {
        v_xid = gd.v_win.page_xid[gd.v_win.page];
      } else {
        v_xid = gd.v_win.tmp_xid;
      }
    }
	
  } else {
    /* set to the field Pixmaps */
    if(mr->auto_render) {
      h_xid = gd.h_win.page_xid[gd.h_win.page];
    } else {
      h_xid = gd.h_win.tmp_xid;
    }

    if(gd.mrec[gd.v_win.page]->auto_render) {
      v_xid = gd.v_win.page_xid[gd.v_win.page];
    } else {
      v_xid = gd.v_win.tmp_xid;
    }
  }


  /******* Handle Real Time Updating  ********/
  switch (gd.movie.mode) {
    case REALTIME_MODE :
      if (time_for_a_new_frame()) {
	rotate_movie_frames(); 
	/* Vectors must be removed from the (currently) last frame if the wind_mode > 0 */
	if(gd.layers.wind_mode && gd.layers.wind_vectors)  {
          gd.movie.frame[gd.movie.cur_frame].redraw_horiz = 1;
	}

	// All product data must be reloaded - Set all to invalid
	if (gd.prod_mgr) {
          gd.prod_mgr->reset_product_valid_flags();
          gd.prod_mgr->reset_times_valid_flags();
	}

	/* Move movie loop to the last frame when aging off old movie frames */
	gd.movie.cur_frame = gd.movie.end_frame;
	goto return_point;
      }

      tm = time(0);
      /* CHECK FOR NEW DATA */
      if ( tm >= update_due ) {

	/* Check only on the last frame - Because its the only "live/realtime" one */
	if (gd.movie.cur_frame == gd.movie.num_frames -1) {
          update_due = tm + update_interv;

          _checkForExpiredData(tm);  // Look for old data

          _checkForDataUpdates(tm);  // Look for data that's newly updated

          _checkWhatNeedsRendering(index);

          // Auto click to get ancillary displays to update too.
          gd.coord_expt->click_type = CIDD_OTHER_CLICK;
          gd.coord_expt->pointer_seq_num++;
	}
      }

      break;

    case ARCHIVE_MODE :
      break;

    default:
      fprintf(stderr,
              "Invalid movie mode %d in timer_func\n",
              gd.movie.mode);
      break;
  } 


  /***** Handle Field changes *****/
  if (gd.h_win.page != gd.h_win.last_page) {
    if (gd.movie.movie_on ) {
      set_redraw_flags(1,0);
    } else {
      if (gd.h_win.redraw[gd.h_win.page] == 0) {
        gd.h_copy_flag = 1;
        gd.h_win.last_page = gd.h_win.page;
      }
    }
  }
  if (gd.v_win.page != gd.v_win.last_page ) {
    if (gd.movie.movie_on ) {
      set_redraw_flags(0,1);
    } else {
      if (gd.v_win.redraw[gd.v_win.page] == 0) {
        gd.v_copy_flag = 1;
        gd.v_win.last_page = gd.v_win.page;
      }
    }
  }

  /******** Handle Frame changes ********/
  if (gd.movie.last_frame != gd.movie.cur_frame && gd.movie.cur_frame >= 0) {

    reset_data_valid_flags(1,1);

    if(_params.symprod_short_requests) {
      // All product data must be reloaded - Set all to invalid
      gd.prod_mgr->reset_product_valid_flags();
    }

    /* Move the indicators */
    // xv_set(gd.movie_pu->movie_frame_sl,
    //        PANEL_VALUE, gd.movie.cur_frame + 1,
    //        NULL);
	
    if(gd.debug2) {
      printf("Moved movie frame, index : %d\n",index);
    }

    gd.coord_expt->epoch_start = gd.epoch_start;
    gd.coord_expt->epoch_end = gd.epoch_end;

    if(gd.movie.movie_on) {
      gd.coord_expt->time_cent = gd.coord_expt->epoch_end;
    } else {
      gd.coord_expt->time_min = gd.movie.frame[index].time_start;
      gd.coord_expt->time_max = gd.movie.frame[index].time_end;
      gd.coord_expt->time_cent = gd.coord_expt->time_min +
	(gd.coord_expt->time_max - gd.coord_expt->time_min) / 2;
      gd.coord_expt->click_type = CIDD_OTHER_CLICK;
      gd.coord_expt->pointer_seq_num++;
    }
    gd.coord_expt->time_current_field = gd.mrec[gd.h_win.page]->h_mhdr.time_centroid;

    /* Change Labels on Frame Begin, End messages */
    update_frame_time_msg(index);
		
    if (gd.movie.frame[index].redraw_horiz == 0) {
      /* Get Frame */
      retrieve_h_movie_frame(index,h_xid);
      gd.h_copy_flag = 1;
    }

    if (gd.v_win.active && gd.movie.frame[index].redraw_vert == 0) {
      retrieve_v_movie_frame(index,v_xid);
      gd.v_copy_flag = 1;
    }

    gd.movie.last_frame = gd.movie.cur_frame;
  }


  /* Draw Selected field - Vertical  for this movie frame */
  if (gd.v_win.active) {
    if (gd.movie.frame[index].redraw_vert) {
      if (gather_vwin_data(gd.v_win.page,gd.movie.frame[index].time_start,
                           gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
        if (gd.v_win.redraw[gd.v_win.page]) {
          render_v_movie_frame(index,v_xid);
          save_v_movie_frame(index,v_xid);
        } 
        gd.movie.frame[index].redraw_vert = 0;
        gd.v_win.redraw[gd.v_win.page] = 0;
        gd.v_copy_flag = 1;
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
          gd.h_win.route.x_world[jj] = xx;
          gd.h_win.route.y_world[jj] = yy;
          npts_found++;
        }
      }

      if (npts_found == vsect.n_waypts && npts_found > 1) {
        gd.h_win.route.total_length = 0.0;
        gd.h_win.route.num_segments = npts_found - 1;
        for (int iseg = 0; iseg < gd.h_win.route.num_segments; iseg++) {
          gd.h_win.route.seg_length[iseg] =
            disp_proj_dist(gd.h_win.route.x_world[iseg],gd.h_win.route.y_world[iseg],
                           gd.h_win.route.x_world[iseg+1],gd.h_win.route.y_world[iseg+1]);
          gd.h_win.route.total_length += gd.h_win.route.seg_length[iseg];
        }
      }
      
      if (gather_vwin_data(gd.v_win.page,gd.movie.frame[index].time_start,
                           gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
        gd.series_save_active = 1;
        render_v_movie_frame(index,v_xid);
        save_v_movie_frame(index,v_xid);
      }

    } // ii

  } // if (_params.image_generate_vsection)

  /* Draw Selected field - Horizontal for this movie frame */
  if (gd.movie.frame[index].redraw_horiz) {
    /* Draw Frame */ 
    if (gather_hwin_data(gd.h_win.page, gd.movie.frame[index].time_start,
                         gd.movie.frame[index].time_end) == CIDD_SUCCESS) {
      if (gd.h_win.redraw[gd.h_win.page]) {
        render_h_movie_frame(index,h_xid);
        save_h_movie_frame(index,h_xid,gd.h_win.page);
      } 

      /* make sure the horiz window's slider has the correct label */
      set_height_label();

      gd.movie.frame[index].redraw_horiz = 0;
      gd.h_win.redraw[gd.h_win.page] = 0;
      gd.h_copy_flag = 1;
    }
  }
	

  /* Selected Field or movie frame has changed
   * Copy background image onto visible canvas
   */
  
  if (gd.h_copy_flag || (gd.h_win.cur_cache_im != gd.h_win.last_cache_im)) {

    if (gd.debug2) {
      fprintf(stderr,
              "\nCopying Horiz grid image - "
              "field %d, index %d xid: %ld to xid: %ld\n",
              gd.h_win.page,index,h_xid,gd.h_win.can_xid[gd.h_win.cur_cache_im]);

      if(gd.h_win.cur_cache_im == gd.h_win.last_cache_im) {
        XCopyArea(gd.dpy,h_xid,
                  gd.h_win.can_xid[gd.h_win.cur_cache_im],
                  gd.def_gc,    0,0,
                  gd.h_win.can_dim.width,
                  gd.h_win.can_dim.height,
                  gd.h_win.can_dim.x_pos,
                  gd.h_win.can_dim.y_pos);
      } else {
        gd.h_win.last_cache_im = gd.h_win.cur_cache_im; 
      }
      gd.h_win.last_page = gd.h_win.page;
      gd.h_copy_flag = 0;

      if (gd.zoom_in_progress == 1) redraw_zoom_box();
      if (gd.pan_in_progress) redraw_pan_line();
      if (gd.route_in_progress) redraw_route_line(&gd.h_win);

      if(!_params.run_once_and_exit) PMU_auto_register("Rendering Products (OK)");

      if (gd.debug2) {
        fprintf(stderr,
                "\nTimer: Displaying Horiz final image - "
                "field %d, index %d xid: %ld to xid: %ld\n",
                gd.h_win.page,index,
                gd.h_win.can_xid[gd.h_win.cur_cache_im],
                gd.h_win.vis_xid);

        /* Now copy last stage pixmap to visible pixmap */
        XCopyArea(gd.dpy,gd.h_win.can_xid[gd.h_win.cur_cache_im],
                  gd.h_win.vis_xid,
                  gd.def_gc,    0,0,
                  gd.h_win.can_dim.width,
                  gd.h_win.can_dim.height,
                  gd.h_win.can_dim.x_pos,
                  gd.h_win.can_dim.y_pos);

        if (gd.zoom_in_progress == 1) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.route_in_progress) redraw_route_line(&gd.h_win);

        // Render a time indicator plot in the movie popup
        if(gd.time_plot)
        {
          gd.time_plot->Set_times((time_t) gd.epoch_start,
                                  (time_t) gd.epoch_end,
                                  (time_t) gd.movie.frame[gd.movie.cur_frame].time_start,
                                  (time_t) gd.movie.frame[gd.movie.cur_frame].time_end,
                                  (time_t)((gd.movie.time_interval_mins * 60.0) + 0.5),
                                  gd.movie.num_frames); 

          if(gd.movie.active) gd.time_plot->Draw();
        }
    
        /* keep track of how much time will elapse showing the current image */
        gettimeofday(&last_frame_tm,&cur_tz);
	
      }

      if(gd.v_win.cur_cache_im != gd.v_win.last_cache_im) {
        gd.v_copy_flag = 1;
        gd.v_win.last_cache_im = gd.v_win.cur_cache_im;
      }

      if (gd.v_win.active && gd.v_copy_flag) {
        if (gd.debug2) fprintf(stderr,"\nCopying Vert grid image - field %d, index %d xid: %ld to xid: %ld\n",
                               gd.v_win.page,index,v_xid,gd.v_win.can_xid[gd.v_win.cur_cache_im]);

        XCopyArea(gd.dpy,v_xid,
                  gd.v_win.can_xid[gd.v_win.cur_cache_im],
                  gd.def_gc,    0,0,
                  gd.v_win.can_dim.width,
                  gd.v_win.can_dim.height,
                  gd.v_win.can_dim.x_pos,
                  gd.v_win.can_dim.y_pos);
        gd.v_win.last_page  = gd.v_win.page;
        gd.v_copy_flag = 0;

        if (gd.debug2) fprintf(stderr,"\nDisplaying Vert final image - field %d, index %d xid: %ld to xid: %ld\n",
                               gd.v_win.page,index,gd.v_win.can_xid[gd.v_win.cur_cache_im],gd.v_win.vis_xid);

        /* Now copy last stage pixmap to visible pixmap */
        XCopyArea(gd.dpy,gd.v_win.can_xid[gd.v_win.cur_cache_im],
                  gd.v_win.vis_xid,
                  gd.def_gc,    0,0,
                  gd.v_win.can_dim.width,
                  gd.v_win.can_dim.height,
                  gd.v_win.can_dim.x_pos,
                  gd.v_win.can_dim.y_pos);

      }

      // If remote driven image(s) needs saved - do it.
      if(gd.image_needs_saved ) {
        int ready = 1;

        // Check to make sure the images are done
        if((gd.save_im_win & PLAN_VIEW) && gd.h_win.redraw[gd.h_win.page] != 0) ready = 0;
        if((gd.save_im_win & XSECT_VIEW) && gd.v_win.redraw[gd.v_win.page] != 0) ready = 0;
        if(ready) {
          dump_cidd_image(gd.save_im_win,0,0,gd.h_win.page);
          gd.image_needs_saved = 0;
        }
      }


      /***** Handle redrawing background images *****/
      msec_diff = ((cur_tm.tv_sec - last_dcheck_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_dcheck_tm.tv_usec) / 1000);

      if (msec_diff < 0 ||  (msec_diff > redraw_interv  && gd.movie.movie_on == 0)) {
        check_for_invalid_images(index);

        /* keep track of how much time will elapse since the last check */
        gettimeofday(&last_dcheck_tm,&cur_tz);
	
      }
      
    return_point:

      if (gd.movie.movie_on == 0) {
        if (gd.zoom_in_progress == 1) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.route_in_progress) redraw_route_line(&gd.h_win);

        if (gd.zoom_in_progress == 1) redraw_zoom_box();
        if (gd.pan_in_progress) redraw_pan_line();
        if (gd.route_in_progress) redraw_route_line(&gd.h_win);

      } else {
        gettimeofday(&cur_tm,&cur_tz);
        msec_diff = ((cur_tm.tv_sec - last_frame_tm.tv_sec) * 1000) + ((cur_tm.tv_usec - last_frame_tm.tv_usec) / 1000);
        msec_delay = msec_delay - msec_diff;
        if(msec_delay <= 0 || msec_delay > 10000) msec_delay = 100;
      }

      if (client_seq_num != gd.coord_expt->client_seq_num) {
        render_click_marks();
        client_seq_num = gd.coord_expt->client_seq_num;
      }
      
    }

  }

}
