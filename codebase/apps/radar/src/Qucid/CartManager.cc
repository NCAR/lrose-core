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
#include "WindWrapper.hh"
#include "MapWrapper.hh"
#include "HorizWidget.hh"
#include "VertWidget.hh"
#include "VertWindow.hh"

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

  _firstTime = true;
  _urlOK = true;

  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  _vertMode = false;

  _nGates = 1000;
  _maxRangeKm = 1.0;
  
  _archiveMode = false;
  _archiveRetrievalPending = false;
  
  _horizFrame = NULL;
  _horiz = NULL;

  _vertWindow = NULL;
  _vert = NULL;

  _sweepVBoxLayout = NULL;
  _sweepPanel = NULL;

  _archiveStartTimeEdit = NULL;
  _archiveEndTimeEdit = NULL;

  _selectedTimeLabel = NULL;
  
  _back1 = NULL;
  _fwd1 = NULL;
  _backPeriod = NULL;
  _fwdPeriod = NULL;

  _fieldMenu = NULL;
  _fieldTable = NULL;
  _fieldMenuPlaced = false;
  _fieldMenuPanel = NULL;
  _fieldMenuLayout = NULL;
  _fieldTableCurrentColumn = -1;
  _fieldTableCurrentRow = -1;

  _timeControl = NULL;
  _timeLayout = NULL;
  _timeSlider = NULL;
  _timeControlPlaced = false;

  _setArchiveMode(_params.begin_in_archive_mode);
  _archiveStartTime.set(_params.archive_start_time);
  _archiveEndTime = _archiveStartTime + _params.archive_time_span_secs;
  _archiveScanIndex = 0;

  _imagesArchiveStartTime.set(_params.images_archive_start_time);
  _imagesArchiveEndTime.set(_params.images_archive_end_time);
  _imagesScanIntervalSecs = _params.images_scan_interval_secs;

  // set up ray locators

  // _rayLoc.resize(RayLoc::RAY_LOC_N);

  // set up windows

  _setupWindows();

  // set initial field to 0

  _changeField(0, false);

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

  // field change?

  if (_fieldTable != NULL) {

    if (_fieldTableCurrentRow != _fieldTable->currentRow() ||
        _fieldTableCurrentColumn != _fieldTable->currentColumn()) {
      
      const FieldTableItem *item =
        (const FieldTableItem *) _fieldTable->item(_fieldTable->currentRow(),
                                             _fieldTable->currentColumn());
      
      if (item->text().toStdString().size() == 0) {
        _fieldTable->setCurrentCell(_fieldTableCurrentRow,
                                    _fieldTableCurrentColumn);
      } else {
        _fieldTableCurrentColumn = _fieldTable->currentColumn();
        _fieldTableCurrentRow = _fieldTable->currentRow();
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          const Params::field_t *fparams = item->getFieldParams();
          cerr << "Changing to field: " << fparams->button_label << endl;
          cerr << "              url: " << fparams->url << endl;
        }
      }
      
    }
    
  }

  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.

  if (_timerEventCount == 0) {

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
  
    if ((_archiveMode) && (_urlOK)) {
      _archiveRetrievalPending = true;
    }

  } // if (_firstTimerEvent)

  _timerEventCount++;

  // read Sprite update of click point info from FMQ
  
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

  // handle event
  
  if (event->timerId() == _mainTimerId) {
    
    if (_archiveMode) {
      if (_archiveRetrievalPending) {
        _handleArchiveData(/*event*/);
        _archiveRetrievalPending = false;
      }
    } else {
      _handleRealtimeData(event);
    }

  }

  // get the time control into the right place
  // we need to let the windows fully draw

  if (_timerEventCount > 10) {
    _placeTimeControl();
  }

  // check for image creation
  
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
    
  }

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
    _goBack1();

  } else if (key == Qt::Key_Right) {

    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _horiz->setStartOfSweep(true);
    _vert->setStartOfSweep(true);
    _goFwd1();
    
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
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
}

//////////////////////////////////////////////////
// Set radar name in title bar

void CartManager::_setTitleBar(const string &radarName)
{
  string windowTitle = "HAWK_EYE -- " + radarName;
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

  _horiz = new HorizWidget(_horizFrame, *this, _params, _platform, _fields, _haveFilteredFields);

  connect(this, SIGNAL(frameResized(const int, const int)),
	  _horiz, SLOT(resize(const int, const int)));
  
  // Create the VERT window

  _vertWindow = new VertWindow(this, _params, _platform,
                             _fields, _haveFilteredFields);
  _vertWindow->setRadarName(_params.radar_name);

  // set pointer to the vertWidget

  _vert = _vertWindow->getWidget();
  
  // connect slots for location

  connect(_horiz, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_horizLocationClicked(double, double, const RadxRay*)));
  connect(_vert, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_vertLocationClicked(double, double, const RadxRay*)));

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);
  // customContextMenuRequested(e->pos());
  connect(_horiz, SIGNAL(customContextMenuRequested(const QPoint &)),
	  this, SLOT(ShowContextMenu(const QPoint &)));

  // create status panel

  _createStatusPanel();

  // create fields panel
  
  _createFieldPanel();

  // add widgets

  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_fieldPanel);
  mainLayout->addWidget(_horizFrame);

  // sweep panel

   _createSweepPanel();
   mainLayout->addWidget(_sweepPanel);

  // field menu

  _createFieldMenu();

  // time panel

  _createTimeControl();

  // fill out menu bar

  _createActions();
  _createMenus();

  // title bar

  _setTitleBar(_params.radar_name);
  setMinimumSize(400, 300);
  resize(_params.main_window_width, _params.main_window_height);
  
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
  _setSweepPanelVisibility();

}

//////////////////////////////
// add/remove  sweep panel (archive mode only)

void CartManager::_setSweepPanelVisibility()
{
  if (_sweepPanel != NULL) {
    if (_archiveMode) {
      _sweepPanel->setVisible(true);
    } else {
      _sweepPanel->setVisible(false);
    }
  }
}

//////////////////////////////
// create actions for menus

void CartManager::_createActions()
{

  // show field menu
  _showFieldMenuAct = new QAction(tr("Fields"), this);
  _showFieldMenuAct->setStatusTip(tr("Show field menu"));
  connect(_showFieldMenuAct, SIGNAL(triggered()),
          this, SLOT(_showFieldMenu()));
  
  // freeze display
  _freezeAct = new QAction(tr("Freeze"), this);
  _freezeAct->setShortcut(tr("Esc"));
  _freezeAct->setStatusTip(tr("Freeze display"));
  connect(_freezeAct, SIGNAL(triggered()), this, SLOT(_freeze()));
  
  // show user click in dialog
  _showClickAct = new QAction(tr("Show Click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, SIGNAL(triggered()), this, SLOT(_showClick()));

  // show boundary editor dialog
  // _showBoundaryEditorAct = new QAction(tr("Boundary Editor"), this);
  // _showBoundaryEditorAct->setStatusTip(tr("Show boundary editor dialog"));
  // connect(_showBoundaryEditorAct, SIGNAL(triggered()), this, SLOT(showBoundaryEditor()));
  
  // set time controller settings
  _timeControllerAct = new QAction(tr("Time-Config"), this);
  _timeControllerAct->setStatusTip(tr("Show time control window"));
  connect(_timeControllerAct, SIGNAL(triggered()), this,
          SLOT(_showTimeControl()));

  // show time control window
  _showTimeControlAct = new QAction(tr("Show time control window"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, SIGNAL(triggered()),
          this, SLOT(_showTimeControl()));
  // connect(_showTimeControlAct, SIGNAL(triggered()), _timeControl,
  //         SLOT(show()));
  
  // realtime mode
  _realtimeAct = new QAction(tr("Set realtime mode"), this);
  _realtimeAct->setStatusTip(tr("Turn realtime mode on/off"));
  _realtimeAct->setCheckable(true);
  _realtimeAct->setChecked(!_params.begin_in_archive_mode);
  connect(_realtimeAct, SIGNAL(triggered(bool)),
          this, SLOT(_setRealtime(bool)));
  
  // unzoom display
  _unzoomAct = new QAction(tr("Unzoom"), this);
  _unzoomAct->setStatusTip(tr("Unzoom to original view"));
  _unzoomAct->setEnabled(false);
  connect(_unzoomAct, SIGNAL(triggered()), this, SLOT(_unzoom()));

  // refresh display
  _refreshAct = new QAction(tr("Refresh"), this);
  _refreshAct->setStatusTip(tr("Refresh plot"));
  connect(_refreshAct, SIGNAL(triggered()), this, SLOT(_refresh()));

  // clear display
  _clearAct = new QAction(tr("Clear"), this);
  _clearAct->setStatusTip(tr("Clear data"));
  connect(_clearAct, SIGNAL(triggered()), _horiz, SLOT(clear()));
  connect(_clearAct, SIGNAL(triggered()), _vert, SLOT(clear()));

  // exit app
  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // file chooser for Open
  _openFileAct = new QAction(tr("O&pen"), this);
  _openFileAct->setShortcut(tr("Ctrl+F"));
  _openFileAct->setStatusTip(tr("Open File"));
  connect(_openFileAct, SIGNAL(triggered()), this, SLOT(_openFile()));

  // file chooser for Save
  _saveFileAct = new QAction(tr("S&ave"), this);
  //_saveFileAct->setShortcut(tr("Ctrl+S"));
  _saveFileAct->setStatusTip(tr("Save File"));
  connect(_saveFileAct, SIGNAL(triggered()), this, SLOT(_saveFile()));

  // show range rings

  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(_params.horiz_range_rings_on_at_startup);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  _horiz, SLOT(setRings(bool)));

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.horiz_grids_on_at_startup);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  _horiz, SLOT(setGrids(bool)));

  // show azimuth lines

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params.horiz_azimuth_lines_on_at_startup);
  connect(_azLinesAct, SIGNAL(triggered(bool)),
	  _horiz, SLOT(setAngleLines(bool)));

  // show VERT window

  _showVertAct = new QAction(tr("Show VERT Window"), this);
  _showVertAct->setStatusTip(tr("Show the VERT Window"));
  connect(_showVertAct, SIGNAL(triggered()), _vertWindow, SLOT(show()));

  // howto and about
  
  _howtoAct = new QAction(tr("&Howto"), this);
  _howtoAct->setStatusTip(tr("Show the application's Howto box"));
  connect(_howtoAct, SIGNAL(triggered()), this, SLOT(_howto()));

  _aboutAct = new QAction(tr("&About"), this);
  _aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(_aboutAct, SIGNAL(triggered()), this, SLOT(_about()));

  _aboutQtAct = new QAction(tr("About &Qt"), this);
  _aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  // save image
  
  _saveImageAct = new QAction(tr("Save-Image"), this);
  _saveImageAct->setStatusTip(tr("Save image to png file"));
  connect(_saveImageAct, SIGNAL(triggered()), this, SLOT(_saveImageToFile()));

}

////////////////
// create menus

void CartManager::_createMenus()
{

  // file menu
  
  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addSeparator();
  _fileMenu->addAction(_openFileAct);
  _fileMenu->addAction(_saveFileAct);
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  // field menu
  
  menuBar()->addAction(_showFieldMenuAct);
  
  // add maps menu
  
  _mapsMenu = menuBar()->addMenu(tr("&Maps"));
  _populateMapsMenu();

  // add winds menu
  
  _windsMenu = menuBar()->addMenu(tr("&Winds"));
  _populateWindsMenu();

   // time selector
  
  _timeMenu = menuBar()->addMenu(tr("&Time-control"));
  _timeMenu->addAction(_showTimeControlAct);
  _timeMenu->addSeparator();
  _timeMenu->addAction(_realtimeAct);

  // add overlay menu
  
  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _populateOverlaysMenu();

  // misc actions
  
  menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showClickAct);
  // menuBar()->addAction(_showBoundaryEditorAct);
  menuBar()->addAction(_unzoomAct);
  menuBar()->addAction(_clearAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

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

    WindWrapper *wrapper = new WindWrapper;
    QAction *act = new QAction;
    wrapper->setWindParams(&wparams);
    wrapper->setWindIndex(iwind);
    wrapper->setWindData(&gd.layers.wind[iwind]);
    wrapper->setAction(act);
    act->setText(wparams.button_label);
    act->setStatusTip(tr("Turn wind layer on/off"));
    act->setCheckable(true);
    act->setChecked(wparams.on_at_startup);
    connect(act, &QAction::toggled,
            wrapper, &WindWrapper::toggled);
    
    // add wrapper for wind selection
    
    _windWrappers.push_back(wrapper);

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

    MapWrapper *wrapper = new MapWrapper;
    QAction *act = new QAction;
    wrapper->setMapParams(&mparams);
    wrapper->setMapIndex(imap);
    wrapper->setOverlay(gd.over[imap]);
    wrapper->setAction(act);
    act->setText(mparams.map_code);
    act->setStatusTip(tr("Turn map layer on/off"));
    act->setCheckable(true);
    act->setChecked(mparams.on_at_startup);
    connect(act, &QAction::toggled,
            wrapper, &MapWrapper::toggled);
    
    // add wrapper for map selection
    
    _mapWrappers.push_back(wrapper);
    
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

void CartManager::_createSweepPanel()
{
  
  _sweepPanel = new QGroupBox("Sweeps", _main);
  _sweepVBoxLayout = new QVBoxLayout;
  _sweepPanel->setLayout(_sweepVBoxLayout);
  _sweepPanel->setAlignment(Qt::AlignHCenter);

  _sweepRButtons = new vector<QRadioButton *>();


}

/////////////////////////////////////////////////////////////////////
// create radio buttons
// this requires that _sweepManager is up to date with sweep info

void CartManager::_createSweepRadioButtons() 
{

  // fonts
  
  QLabel dummy;
  QFont font = dummy.font();
  QFont fontm2 = dummy.font();
  int fsize = _params.label_font_size;
  int fsizem2 = _params.label_font_size - 2;
  font.setPixelSize(fsize);
  fontm2.setPixelSize(fsizem2);

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

}

/////////////////////////////////////////////////////////////////////
// create sweep panel of radio buttons

void CartManager::_clearSweepRadioButtons() 
{

  QLayoutItem* child;
  if (_sweepVBoxLayout != NULL) {
    while (_sweepVBoxLayout->count() !=0) {
      child = _sweepVBoxLayout->takeAt(0);
      if (child->widget() !=0) {
        delete child->widget();
      }
      delete child;
    }
  }
 
}

///////////////////////////////////////////////////////////////
// change sweep

void CartManager::_changeSweep(bool value) {

  if (_params.debug) {
    cerr << "From CartManager: the sweep was changed ";
    cerr << endl;
  }

  if (!value) {
    return;
  }

  for (size_t sweepIndex = 0; sweepIndex < _sweepRButtons->size(); sweepIndex++) {
    if (_sweepRButtons->at(sweepIndex)->isChecked()) {
      if (_params.debug) {
        cerr << "sweepRButton " << sweepIndex << " is checked" << endl;
        cerr << "  moving to sweep index " << sweepIndex << endl;
      }
      // _sweepManager.setGuiIndex(sweepIndex);
      _horiz->setStartOfSweep(true);
      _vert->setStartOfSweep(true);
      _moveUpDown();

      // refreshBoundaries();
      return;
    }
  } // ii

}

///////////////////////////////////////////////////////////////
// change sweep
// only set the sweepIndex in one place;
// here, just move the radio button up or down one step
// when the radio button is changed, a signal is emitted and
// the slot that receives the signal will increase the sweepIndex
// value = +1 move forward
// value = -1 move backward in sweeps

void CartManager::_changeSweepRadioButton(int increment)

{
  
  if (_params.debug) {
    cerr << "-->> changing sweep index by increment: " << increment << endl;
  }
  
  if (increment != 0) {
    // _sweepManager.changeSelectedIndex(increment);
    // _sweepRButtons->at(_sweepManager.getGuiIndex())->setChecked(true);
  }

}

/////////////////////////////
// get data in realtime mode

void CartManager::_handleRealtimeData(QTimerEvent * event)

{

  _horiz->setArchiveMode(false);
  _vert->setArchiveMode(false);

}

///////////////////////////////////////
// set input file list for archive mode

void CartManager::setArchiveFileList(const vector<string> &list,
                                     bool fromCommandLine /* = true */)
{

  if (fromCommandLine && list.size() > 0) {
    // determine start and end time from file list
    RadxTime startTime, endTime;
    NcfRadxFile::getTimeFromPath(list[0], startTime);
    NcfRadxFile::getTimeFromPath(list[list.size()-1], endTime);
    // round to nearest five minutes
    time_t startTimeSecs = startTime.utime();
    startTimeSecs =  (startTimeSecs / 300) * 300;
    time_t endTimeSecs = endTime.utime();
    endTimeSecs =  (endTimeSecs / 300) * 300 + 300;
    _archiveStartTime.set(startTimeSecs);
    _archiveEndTime.set(endTimeSecs);
    _archiveScanIndex = 0;
  }

  _archiveFileList = list;
  _setArchiveRetrievalPending();

  if (_archiveScanIndex < 0) {
    _archiveScanIndex = 0;
  } else if (_archiveScanIndex > (int) _archiveFileList.size() - 1) {
    _archiveScanIndex = _archiveFileList.size() - 1;
  }

  if (_timeSlider) {
    _timeSlider->setMinimum(0);
    if (_archiveFileList.size() <= 1)
      _timeSlider->setMaximum(1);
    else
      _timeSlider->setMaximum(_archiveFileList.size() - 1);
    _timeSlider->setSliderPosition(_archiveScanIndex);
  }

  // check if the paths include a day dir

  _archiveFilesHaveDayDir = false;
  if (list.size() > 0) {
    RadxPath path0(list[0]);
    RadxPath parentPath(path0.getDirectory());
    string parentDir = parentPath.getFile();
    int year, month, day;
    if (sscanf(parentDir.c_str(), "%4d%2d%2d", &year, &month, &day) == 3) {
      _archiveFilesHaveDayDir = true;
    }
  }

  if (_archiveFilesHaveDayDir) {
    _archiveStartTimeEdit->setEnabled(true);
    _archiveEndTimeEdit->setEnabled(true);
    _backPeriod->setEnabled(true);
    _fwdPeriod->setEnabled(true);
  } else {
    _archiveStartTimeEdit->setEnabled(false);
    _archiveEndTimeEdit->setEnabled(false);
    _backPeriod->setEnabled(false);
    _fwdPeriod->setEnabled(false);
  }

  _setGuiFromArchiveStartTime();
  _setGuiFromArchiveEndTime();

}
  
///////////////////////////////////////////////
// get archive file list by searching for files
// returns 0 on success, -1 on failure

int CartManager::loadArchiveFileList()

{
  return 0;

}

///////////////////////////////////////
// handle data in archive mode

void CartManager::_handleArchiveData(/*QTimerEvent * event*/)

{
  
  if (_params.debug) {
    cerr << "handling archive data ..." << endl;
  }

  _horiz->setArchiveMode(true);
  _horiz->setStartOfSweep(true);

  _vert->setArchiveMode(true);
  _vert->setStartOfSweep(true);

  // set cursor to wait cursor

  this->setCursor(Qt::WaitCursor);
  _timeControl->setCursor(Qt::WaitCursor);
  
  // get data
  try {
    _getArchiveData();
  } catch (FileIException &ex) {
    this->setCursor(Qt::ArrowCursor);
    _timeControl->setCursor(Qt::ArrowCursor);
    return;
  }
  _activateArchiveRendering();

  // set up sweep GUI

  _clearSweepRadioButtons();
  _createSweepRadioButtons();
  
  if (_vol.checkIsRhi()) {
    _vertMode = true;
  } else {
    _vertMode = false;
  }

  // plot the data
  
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
  _timeControl->setCursor(Qt::ArrowCursor);

  if (_firstTime) {
    _firstTime = false;
  }
  
}

/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure

int CartManager::_getArchiveData()

{

  // set up file object for reading
  
  RadxFile file;
  _vol.clear();
  _setupVolRead(file);
  
  if (_archiveScanIndex >= 0 &&
      _archiveScanIndex < (int) _archiveFileList.size()) {
    
    string inputPath = _archiveFileList[_archiveScanIndex];
    
    if(_params.debug) {
      cerr << "  reading data file path: " << inputPath << endl;
      cerr << "  archive file index: " << _archiveScanIndex << endl;
    }
    
    if (file.readFromPath(inputPath, _vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "CartManager::_getArchiveData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + inputPath + "\n";
      cerr << errMsg;
      if (!_params.images_auto_create)  {
        QErrorMessage errorDialog;
        errorDialog.setMinimumSize(400, 250);
        errorDialog.showMessage(errMsg.c_str());
        errorDialog.exec();
      }
      return -1;
    } 

  }

  // set plot times
  
  _plotStartTime = _vol.getStartTimeSecs();
  _plotEndTime = _vol.getEndTimeSecs();

  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           _plotStartTime.getYear(),
           _plotStartTime.getMonth(),
           _plotStartTime.getDay(),
           _plotStartTime.getHour(),
           _plotStartTime.getMin(),
           _plotStartTime.getSec());

  _selectedTimeLabel->setText(text);

  // adjust angles for elevation surveillance if needed

  _vol.setAnglesForElevSurveillance();

  // compute the fixed angles from the rays
  // so that we reflect reality
  
  _vol.computeFixedAnglesFromRays();

  // load the sweep manager
  
  // _sweepManager.set(_vol);

  // if (_params.debug) {
  //   cerr << "----------------------------------------------------" << endl;
  //   cerr << "perform archive retrieval" << endl;
  //   cerr << "  read file: " << _vol.getPathInUse() << endl;
  //   cerr << "  nSweeps: " << _vol.getNSweeps() << endl;
  //   cerr << "  guiIndex, fixedAngle: " 
  //        << _sweepManager.getGuiIndex() << ", "
  //        << _sweepManager.getSelectedAngle() << endl;
  //   cerr << "----------------------------------------------------" << endl;
  // }
  
   _platform = _vol.getPlatform();
   
  return 0;

}


/////////////////////////////
// apply new, edited  data in archive mode
// the volume has been updated 

void CartManager::_applyDataEdits()
{

  if (_params.debug) {
    std::ofstream outfile("/tmp/voldebug_CartManager_applyDataEdits.txt");
    _vol.printWithFieldData(outfile);  
    outfile << "_vol = " << &_vol << endl;
  }

  _plotArchiveData();
}

/////////////////////////////
// plot data in archive mode

void CartManager::_plotArchiveData()

{

  if(_params.debug) {
    cerr << "Plotting archive data" << endl;
    cerr << "  volume start time: " << _plotStartTime.asString() << endl;
  }

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
  
}

//////////////////////////////////////////////////
// set up read

void CartManager::_setupVolRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    const DisplayField *field = _fields[ifield];
    file.addReadField(field->getName());
  }

}

//////////////////////////////////////////////////////////////
// handle an incoming ray

void CartManager::_handleRay(RadxPlatform &platform, RadxRay *ray)
  
{

  if (ray->getNGates() < 1) {
    return;
  }

  // do we need to reconfigure the HORIZ?

  _nGates = ray->getNGates();
  double maxRange = ray->getStartRangeKm() + _nGates * ray->getGateSpacingKm();
  _maxRangeKm = maxRange;
  _horiz->configureRange(_maxRangeKm);
  _vert->configureRange(_maxRangeKm);

  // create 2D field data vector

  vector< vector<double> > fieldData;
  fieldData.resize(_fields.size());
  
  // fill data vector

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

    vector<double> &data = fieldData[ifield];
    data.resize(_nGates);
    RadxField *rfld = ray->getField(_fields[ifield]->getName());

    // at this point, we know the data values for the field AND the color map                                                                        
    bool haveColorMap = _fields[ifield]->haveColorMap();
    Radx::fl32 min = FLT_MAX;;
    Radx::fl32 max = FLT_MIN;

    if (rfld == NULL) {
      // fill with missing
      for (int igate = 0; igate < _nGates; igate++) {
        data[igate] = -9999.0;
      }
    } else {
      rfld->convertToFl32();
      const Radx::fl32 *fdata = rfld->getDataFl32();
      const Radx::fl32 missingVal = rfld->getMissingFl32();
      // we can only look at the data available, so only go to nGates
      for (int igate = 0; igate < _nGates; igate++, fdata++) {  // was _nGates
        Radx::fl32 val = *fdata;
        if (fabs(val - missingVal) < 0.0001) {
          data[igate] = -9999.0;
        } else {
          data[igate] = val;
          if (!haveColorMap) {
            // keep track of min and max data values
	    // just display something.  The color scale can be edited as needed, later.
	    bool newMinOrMax = false;
            if (val < min) {
              min = *fdata;
	      newMinOrMax = true;
	    }
            if (val > max) {
	      max = *fdata;
	      newMinOrMax = true;
	    }
	    if ((newMinOrMax) && (_params.debug >= Params::DEBUG_VERBOSE)) { 
	      printf("field index %zu, gate %d \t", ifield, igate);
	      printf("new min, max of data %g, %g\t", min,  max);
	      printf("missing value %g\t", missingVal);
	      printf("current value %g\n", val);
	    }
          }
        } // end else not missing value
      } // end for each gate
      
      if (!haveColorMap) {                              
        _fields[ifield]->setColorMapRange(min, max);
        _fields[ifield]->changeColorMap(); // just change bounds on existing map
      } // end do not have color map

    } // end else vector not NULL
  } // end for each field

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the HORIZ or VERT, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI) {

    _vertMode = true;

    // If this is the first VERT beam we've encountered, automatically open
    // the VERT window.  After this, opening and closing the window will be
    // left to the user.

    if (!_vertWindowDisplayed) {
      _vertWindow->show();
      _vertWindow->resize();
      _vertWindowDisplayed = true;
    }

    // Add the beam to the display

    _vert->addBeam(ray, fieldData, _fields);
    _vertWindow->setAzimuth(ray->getAzimuthDeg());
    _vertWindow->setElevation(ray->getElevationDeg());
    
  } else {

    _vertMode = false;

    // check for elevation surveillance sweep mode
    // in this case, set azimuth to rotation if georef is available

    if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
      ray->setAnglesForElevSurveillance();
    }

    // Store the ray location using the azimuth angle and the HORIZ location
    // table

    double az = ray->getAzimuthDeg();
    _storeRayLoc(ray, az, platform.getRadarBeamWidthDegH());

    // Save the angle information for the next iteration

    _prevAz = az;
    _prevEl = -9999.0;

    // Add the beam to the display

    _horiz->addBeam(ray, _startAz, _endAz, fieldData, _fields);

  }
  
}

///////////////////////////////////////////////////////////
// store ray location

void CartManager::_storeRayLoc(const RadxRay *ray, 
                               const double az,
                               const double beam_width)
{

#ifdef NOTNOW
  
  LOG(DEBUG_VERBOSE) << "az = " << az << " beam_width = " << beam_width;

  // Determine the extent of this ray

  if (_params.horiz_override_rendering_beam_width) {
    double half_angle = _params.horiz_rendering_beam_width / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else if (ray->getIsIndexed()) {
    double half_angle = ray->getAngleResDeg() / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else {
    double beam_width_min = beam_width;
    if (beam_width_min < 0) 
      beam_width_min = 10.0;

    double max_half_angle = beam_width_min / 2.0;
    double prev_offset = max_half_angle;
    if (_prevAz > 0.0) { // >= 0.0) {
      double az_diff = az - _prevAz;
      if (az_diff < 0.0)
	az_diff += 360.0;
      double half_az_diff = az_diff / 2.0;
	
      if (prev_offset > half_az_diff)
	prev_offset = half_az_diff;
    }
    _startAz = az - prev_offset - 0.1;
    _endAz = az + max_half_angle + 0.1;
  }
    
  // store
  // HERE !!! fix up negative values here or in clearRayOverlap??
  if (_startAz < 0) _startAz += 360.0;
  if (_endAz < 0) _endAz += 360.0;
  if (_startAz >= 360) _startAz -= 360.0;
  if (_endAz >= 360) _endAz -= 360.0;
    
  LOG(DEBUG_VERBOSE) << " startAz = " << _startAz << " endAz = " << _endAz;

  // compute start and end indices, using modulus to keep with array bounds

  int startIndex = ((int) (_startAz * RayLoc::RAY_LOC_RES)) % RayLoc::RAY_LOC_N;
  int endIndex = ((int) (_endAz * RayLoc::RAY_LOC_RES + 1)) % RayLoc::RAY_LOC_N;

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  if (startIndex > endIndex) {

    // area crosses the 360; 0 boundary; must break into two sections

    // first from start index to 360
    
    _clearRayOverlap(startIndex, RayLoc::RAY_LOC_N - 1);

    for (int ii = startIndex; ii < RayLoc::RAY_LOC_N; ii++) { // RayLoc::RAY_LOC_N; ii++) {
      _rayLoc[ii].ray = ray;
      _rayLoc[ii].active = true;
      _rayLoc[ii].startIndex = startIndex;
      _rayLoc[ii].endIndex = RayLoc::RAY_LOC_N - 1; // RayLoc::RAY_LOC_N;
    }

    // then from 0 to end index
    
    _clearRayOverlap(0, endIndex);

    // Set the locations associated with this ray
    
    for (int ii = 0; ii <= endIndex; ii++) {
      _rayLoc[ii].ray = ray;
      _rayLoc[ii].active = true;
      _rayLoc[ii].startIndex = 0;
      _rayLoc[ii].endIndex = endIndex;
    }

  } else { // if (startIndex > endIndex) 

    _clearRayOverlap(startIndex, endIndex);
    
    // Set the locations associated with this ray

    for (int ii = startIndex; ii <= endIndex; ii++) {
      _rayLoc[ii].ray = ray;
      _rayLoc[ii].active = true;
      _rayLoc[ii].startIndex = startIndex;
      _rayLoc[ii].endIndex = endIndex;
    }

  } // if (startIndex > endIndex)

#endif

}

 
///////////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

void CartManager::_clearRayOverlap(const int start_index, const int end_index)
{

#ifdef NOTNOW
  
  LOG(DEBUG_VERBOSE) << "enter" << " start_index=" << start_index <<
    " end_index = " << end_index;

  if ((start_index < 0) || (start_index > RayLoc::RAY_LOC_N)) {
    cout << "ERROR: _clearRayOverlap start_index out of bounds " << start_index << endl;
    return;
  }
  if ((end_index < 0) || (end_index > RayLoc::RAY_LOC_N)) {
    cout << "ERROR: _clearRayOverlap end_index out of bounds " << end_index << endl;
    return;
  }

  // Loop through the ray locations, clearing out old information

  int i = start_index;
  
  while (i <= end_index) {

    RayLoc &loc = _rayLoc[i];
    
    // If this location isn't active, we can skip it

    if (!loc.active) {
      // LOG(DEBUG_VERBOSE) << "loc NOT active";
      ++i;
      continue;
    }
    
    int loc_start_index = loc.startIndex;
    int loc_end_index = loc.endIndex;

    if ((loc_start_index < 0) || (loc_start_index > RayLoc::RAY_LOC_N)) {
      cout << "ERROR: _clearRayOverlap loc_start_index out of bounds " << loc_start_index << endl;
      ++i;
      continue;
    }
    if ((loc_end_index < 0) || (loc_end_index > RayLoc::RAY_LOC_N)) {
      cout << "ERROR: _clearRayOverlap loc_end_index out of bounds " << loc_end_index << endl;
      ++i;
      continue;
    }

    if (loc_end_index < i) {
      cout << " OH NO! We are HERE" << endl;
      ++i;
      continue;
    }
    // If we get here, this location is active.  We now have 4 possible
    // situations:

    if (loc.startIndex < start_index && loc.endIndex <= end_index) {

      // The overlap area covers the end of the current beam.  Reduce the
      // current beam down to just cover the area before the overlap area.
      LOG(DEBUG_VERBOSE) << "Case 1a:";
      LOG(DEBUG_VERBOSE) << " i = " << i;
      LOG(DEBUG_VERBOSE) << "clearing from start_index=" << start_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = start_index; j <= loc_end_index; ++j) {

	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;

      }

      // Update the end indices for the remaining locations in the current
      // beam
      LOG(DEBUG_VERBOSE) << "Case 1b:";
      LOG(DEBUG_VERBOSE) << "setting endIndex to " << start_index - 1 << " from loc_start_index=" << loc_start_index <<
	  " to start_index=" << start_index;
      
      for (int j = loc_start_index; j < start_index; ++j)
	_rayLoc[j].endIndex = start_index - 1;

    } else if (loc.startIndex < start_index && loc.endIndex > end_index) {
      
      // The current beam is bigger than the overlap area.  This should never
      // happen, so go ahead and just clear out the locations for the current
      // beam.
      LOG(DEBUG_VERBOSE) << "Case 2:";
      LOG(DEBUG_VERBOSE) << " i = " << i;
      LOG(DEBUG_VERBOSE) << "clearing from loc_start_index=" << loc_start_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = loc_start_index; j <= loc_end_index; ++j) {
        _rayLoc[j].clear();
      }

    } else if (loc.endIndex > end_index) {
      
      // The overlap area covers the beginning of the current beam.  Reduce the
      // current beam down to just cover the area after the overlap area.

	LOG(DEBUG_VERBOSE) << "Case 3a:";
	LOG(DEBUG_VERBOSE) << " i = " << i;
	LOG(DEBUG_VERBOSE) << "clearing from loc_start_index=" << loc_start_index <<
	  " to end_index=" << end_index;

      for (int j = loc_start_index; j <= end_index; ++j) {
	_rayLoc[j].ray = NULL;
	_rayLoc[j].active = false;
      }

      // Update the start indices for the remaining locations in the current
      // beam

      LOG(DEBUG_VERBOSE) << "Case 3b:";
      LOG(DEBUG_VERBOSE) << "setting startIndex to " << end_index + 1 << " from end_index=" << end_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = end_index + 1; j <= loc_end_index; ++j) {
	_rayLoc[j].startIndex = end_index + 1;
      }

    } else {
      
      // The current beam is completely covered by the overlap area.  Clear
      // out all of the locations for the current beam.
      LOG(DEBUG_VERBOSE) << "Case 4:";
      LOG(DEBUG_VERBOSE) << " i = " << i;
      LOG(DEBUG_VERBOSE) << "clearing from loc_start_index=" << loc_start_index <<
	  " to loc_end_index=" << loc_end_index;
      
      for (int j = loc_start_index; j <= loc_end_index; ++j) {
        _rayLoc[j].clear();
      }

    }
    
    i = loc_end_index + 1;

  } /* endwhile - i */
  
  LOG(DEBUG_VERBOSE) << "exit ";

#endif
  
}

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

  _applyDataEdits(); // radarDataVolume);

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
  
  _timeControl = new QDialog(this);
  _timeControl->setWindowTitle("Time controller");
  QPoint pos(0,0);
  _timeControl->move(pos);

  QBoxLayout *timeControlLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, _timeControl);
  timeControlLayout->setSpacing(0);

  // create time panel
  
  _timePanel = new QFrame(_timeControl);
  timeControlLayout->addWidget(_timePanel, Qt::AlignCenter);
  _timeLayout = new QVBoxLayout;
  _timePanel->setLayout(_timeLayout);

  QFrame *timeUpper = new QFrame(_timePanel);
  QHBoxLayout *timeUpperLayout = new QHBoxLayout;
  timeUpperLayout->setSpacing(10);
  timeUpper->setLayout(timeUpperLayout);
  
  QFrame *timeLower = new QFrame(_timePanel);
  QHBoxLayout *timeLowerLayout = new QHBoxLayout;
  timeLowerLayout->setSpacing(10);
  timeLower->setLayout(timeLowerLayout);

  _timeLayout->addWidget(timeUpper);
  _timeLayout->addWidget(timeLower);
  
  // create slider
  
  _timeSlider = new QSlider(Qt::Horizontal);
  _timeSlider->setFocusPolicy(Qt::StrongFocus);
  _timeSlider->setTickPosition(QSlider::TicksBothSides);
  _timeSlider->setTickInterval(1);
  _timeSlider->setTracking(true);
  _timeSlider->setSingleStep(1);
  _timeSlider->setPageStep(0);
  _timeSlider->setFixedWidth(400);
  _timeSlider->setToolTip("Drag to change time selection");
  
  // active time
  
  // _selectedTimeLabel = new QLabel("yyyy/MM/dd hh:mm:ss", _timePanel);
  _selectedTimeLabel = new QPushButton(_timePanel);
  _selectedTimeLabel->setText("yyyy/MM/dd hh:mm:ss");
  QPalette pal = _selectedTimeLabel->palette();
  pal.setColor(QPalette::Active, QPalette::Button, Qt::cyan);
  _selectedTimeLabel->setPalette(pal);
  _selectedTimeLabel->setToolTip("This is the selected data time");

  // time editing

  _archiveStartTimeEdit = new QDateTimeEdit(timeUpper);
  _archiveStartTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  QDate startDate(_archiveStartTime.getYear(), 
                  _archiveStartTime.getMonth(),
                  _archiveStartTime.getDay());
  QTime startTime(_archiveStartTime.getHour(),
                  _archiveStartTime.getMin(),
                  _archiveStartTime.getSec());
  QDateTime startDateTime(startDate, startTime);
  _archiveStartTimeEdit->setDateTime(startDateTime);
  connect(_archiveStartTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), 
          this, SLOT(_setArchiveStartTimeFromGui(const QDateTime &)));
  _archiveStartTimeEdit->setToolTip("Start time of archive period");
  
  _archiveEndTimeEdit = new QDateTimeEdit(timeUpper);
  _archiveEndTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
  QDate endDate(_archiveEndTime.getYear(), 
                 _archiveEndTime.getMonth(),
                 _archiveEndTime.getDay());
  QTime endTime(_archiveEndTime.getHour(),
                 _archiveEndTime.getMin(),
                 _archiveEndTime.getSec());
  QDateTime endDateTime(endDate, endTime);
  _archiveEndTimeEdit->setDateTime(endDateTime);
  connect(_archiveEndTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), 
          this, SLOT(_setArchiveEndTimeFromGui(const QDateTime &)));
  _archiveEndTimeEdit->setToolTip("End time of archive period");
  
  // fwd and back buttons

  _back1 = new QPushButton(timeLower);
  _back1->setText("<");
  connect(_back1, SIGNAL(clicked()), this, SLOT(_goBack1()));
  _back1->setToolTip("Go back by 1 file");
  
  _fwd1 = new QPushButton(timeLower);
  _fwd1->setText(">");
  connect(_fwd1, SIGNAL(clicked()), this, SLOT(_goFwd1()));
  _fwd1->setToolTip("Go forward by 1 file");
    
  _backPeriod = new QPushButton(timeLower);
  _backPeriod->setText("<<");
  connect(_backPeriod, SIGNAL(clicked()), this, SLOT(_goBackPeriod()));
  _backPeriod->setToolTip("Go back by the archive time period");
  
  _fwdPeriod = new QPushButton(timeLower);
  _fwdPeriod->setText(">>");
  connect(_fwdPeriod, SIGNAL(clicked()), this, SLOT(_goFwdPeriod()));
  _fwdPeriod->setToolTip("Go forward by the archive time period");

  // accept cancel buttons

  QPushButton *acceptButton = new QPushButton(timeUpper);
  acceptButton->setText("Accept");
  QPalette acceptPalette = acceptButton->palette();
  acceptPalette.setColor(QPalette::Active, QPalette::Button, Qt::green);
  acceptButton->setPalette(acceptPalette);
  connect(acceptButton, SIGNAL(clicked()), this, SLOT(_acceptGuiTimes()));
  acceptButton->setToolTip("Accept the selected start and end times");

  QPushButton *cancelButton = new QPushButton(timeUpper);
  cancelButton->setText("Cancel");
  QPalette cancelPalette = cancelButton->palette();
  cancelPalette.setColor(QPalette::Active, QPalette::Button, Qt::red);
  cancelButton->setPalette(cancelPalette);
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(_cancelGuiTimes()));
  cancelButton->setToolTip("Cancel the selected start and end times");
    
  // add time widgets to layout
  
  int stretch = 0;
  timeUpperLayout->addWidget(cancelButton, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(_archiveStartTimeEdit, stretch, Qt::AlignRight);
  timeUpperLayout->addWidget(_selectedTimeLabel, stretch, Qt::AlignCenter);
  timeUpperLayout->addWidget(_archiveEndTimeEdit, stretch, Qt::AlignLeft);
  timeUpperLayout->addWidget(acceptButton, stretch, Qt::AlignLeft);
  
  timeLowerLayout->addWidget(_backPeriod, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_back1, stretch, Qt::AlignRight);
  timeLowerLayout->addWidget(_timeSlider, stretch, Qt::AlignCenter);
  timeLowerLayout->addWidget(_fwd1, stretch, Qt::AlignLeft);
  timeLowerLayout->addWidget(_fwdPeriod, stretch, Qt::AlignLeft);

  // connect slots for time slider
  
  connect(_timeSlider, SIGNAL(actionTriggered(int)),
          this, SLOT(_timeSliderActionTriggered(int)));
  
  connect(_timeSlider, SIGNAL(valueChanged(int)),
          this, SLOT(_timeSliderValueChanged(int)));
  
  connect(_timeSlider, SIGNAL(sliderReleased()),
          this, SLOT(_timeSliderReleased()));

  connect(_timeSlider, SIGNAL(sliderPressed()),
          this, SLOT(_timeSliderPressed()));

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

///////////////////////////////////////////////////////////////////////////////
// print time slider actions for debugging

void CartManager::_timeSliderActionTriggered(int action) {
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    switch (action) {
      case QAbstractSlider::SliderNoAction:
        cerr << "SliderNoAction action in _timeSliderActionTriggered" << endl;
        break;
      case QAbstractSlider::SliderSingleStepAdd: 
        cerr << "SliderSingleStepAdd action in _timeSliderActionTriggered" << endl;
        break; 
      case QAbstractSlider::SliderSingleStepSub:	
        cerr << "SliderSingleStepSub action in _timeSliderActionTriggered" << endl;
        break;
      case QAbstractSlider::SliderPageStepAdd:
        cerr << "SliderPageStepAdd action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderPageStepSub:
        cerr << "SliderPageStepSub action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderToMinimum:
        cerr << "SliderToMinimum action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderToMaximum:
        cerr << "SliderToMaximum action in _timeSliderActionTriggered" << endl;
        break;	
      case QAbstractSlider::SliderMove:
        cerr << "SliderMove action in _timeSliderActionTriggered" << endl;
        break;
      default: 
        cerr << "unknown action in _timeSliderActionTriggered" << endl;
    }
    cerr << "timeSliderActionTriggered, value: " << _timeSlider->value() << endl;
  }
} 

void CartManager::_timeSliderValueChanged(int value) 
{
  if (value < 0 || value > (int) _archiveFileList.size() - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  _setGuiFromSelectedTime();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider changed, value: " << value << endl;
  }
}

void CartManager::_timeSliderReleased() 
{
  int value = _timeSlider->value();
  if (value < 0 || value > (int) _archiveFileList.size() - 1) {
    return;
  }
  // get path for this value
  string path = _archiveFileList[value];
  // get time for this path
  RadxTime pathTime;
  NcfRadxFile::getTimeFromPath(path, pathTime);
  // set selected time
  _selectedTime = pathTime;
  _setGuiFromSelectedTime();
  // request data
  if (_archiveScanIndex != value) {
    _archiveScanIndex = value;
    _setArchiveRetrievalPending();
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
}

void CartManager::_timeSliderPressed() 
{
  int value = _timeSlider->value();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
}

// sets the directory (_boundaryDir) into which boundary files will be read/written for current radar file (_openFilePath)
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

////////////////////////////////////////////////////
// create the file chooser dialog
//
// This allows the user to choose an archive file to open

void CartManager::_openFile()
{
  // seed with files for the day currently in view, generate like this: *yyyymmdd*
  string pattern = _archiveStartTime.getDateStrPlain();
  QString finalPattern = "Cfradial (*.nc);; All Files (*.*);; All files (*";
  finalPattern.append(pattern.c_str());
  finalPattern.append("*)");
  
  QString inputPath = QDir::currentPath();
  // get the path of the current file, if available
  if (_archiveFileList.size() > 0) {
    QDir temp(_archiveFileList[0].c_str());
    inputPath = temp.absolutePath();
  }
  
  //since we are opening a new radar file, close any boundaries currently being displayed
  // BoundaryPointEditor::Instance()->clear();
  // if (_boundaryEditorDialog) {
  //   clearBoundaryEditorClick();
  //   _boundaryEditorDialog->setVisible(false);
  // }

  if (_horiz){
    _horiz->showOpeningFileMsg(true);
  }
  
  QString filePath =
    QFileDialog::getOpenFileName(this,
                                 "Open Document",
                                 inputPath, finalPattern);
  //QDir::currentPath(),
  //"All files (*.*)");

   //wait 10ms so the QFileDialog has time to close before proceeding...
  QTimer::singleShot(10, [=]()
  {
    if( !filePath.isNull() )
    {
      QByteArray qb = filePath.toUtf8();
      const char *openFilePath = qb.constData();
      _openFilePath = openFilePath;

      cout << "_openFilePath=" << _openFilePath << endl;

      //use _openFilePath to determine the new directory
      // into which boundaries will be read/written
      setBoundaryDir();

      // trying this ... to get the data from the file selected
      _setArchiveRetrievalPending();
      vector<string> list;
      list.push_back(openFilePath);
      setArchiveFileList(list, false);


      try {
        _getArchiveData();
      } catch (FileIException &ex) {
        _horiz->showOpeningFileMsg(false);
        this->setCursor(Qt::ArrowCursor);
        // _timeControl->setCursor(Qt::ArrowCursor);
        return;
      }
    }

    // now update the time controller window
    QDateTime epoch(QDate(1970, 1, 1), QTime(0, 0, 0));
    _setArchiveStartTimeFromGui(epoch);
    QDateTime now = QDateTime::currentDateTime();
    _setArchiveEndTimeFromGui(now);

    _archiveStartTime = _guiStartTime;
    _archiveEndTime = _guiEndTime;
    QFileInfo fileInfo(filePath);
    string absolutePath = fileInfo.absolutePath().toStdString();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "changing to path " << absolutePath << endl;
    }
    //  loadArchiveFileList(dir.absolutePath());
    
    RadxTimeList timeList;
    timeList.setDir(absolutePath);
    timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
    if (timeList.compile()) {
      cerr << "ERROR - CartManager::openFile()" << endl;
      cerr << "  " << timeList.getErrStr() << endl;
    }

    vector<string> pathList = timeList.getPathList();
    if (pathList.size() <= 0) {
      cerr << "ERROR - CartManager::openFile()" << endl;
      cerr << "  pathList is empty" << endl;
      cerr << "  " << timeList.getErrStr() << endl;
    } else {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "pathList is NOT empty" << endl;
        for(vector<string>::const_iterator i = pathList.begin(); i != pathList.end(); ++i) {
         cerr << *i << endl;
        }
        cerr << endl;
      }

      setArchiveFileList(pathList, false);

      // now fetch the first time and last time from the directory
      // and set these values in the time controller display

      RadxTime firstTime;
      RadxTime lastTime;
      timeList.getFirstAndLastTime(firstTime, lastTime);
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "first time " << firstTime << endl;
        cerr << "last time " << lastTime << endl;
      }
      // convert RadxTime to QDateTime
      _archiveStartTime = firstTime;
      _archiveEndTime = lastTime;
      _setGuiFromArchiveStartTime();
      _setGuiFromArchiveEndTime();

      _horiz->showOpeningFileMsg(false);
    } // end else pathList is not empty
  });
}



////////////////////////////////////////////////////
// create the file chooser dialog
//
// This allows the user to choose the name of a
// file in which to save Volume data

void CartManager::_saveFile()
{
  
  QString finalPattern = "All files (*.nc)";

  QString inputPath = QDir::currentPath();
  // get the path of the current file, if available 
  if (_archiveFileList.size() > 0) {
    QDir temp(_archiveFileList[0].c_str());
    inputPath = temp.absolutePath();
  } 

  QString filename =  QFileDialog::getSaveFileName(
          this,
          "Save Radar Volume",
          inputPath, finalPattern);
 
  if( !filename.isNull() )
  {
    QByteArray qb = filename.toUtf8();
    const char *name = qb.constData();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      LOG(DEBUG_VERBOSE) << "selected file path : " << name;
    }

    // TODO: hold it! the save message should
    // go to the Model (Data) level because
    // we'll be using Radx utilities.
    // 
    RadxFile outFile;
    try {
      LOG(DEBUG_VERBOSE) << "writing to file " << name;
      outFile.writeToPath(_vol, name);
    } catch (FileIException &ex) {
      this->setCursor(Qt::ArrowCursor);
      return;
    }
  }
}

void CartManager::_createFileChooserDialog()
{
  _refreshFileChooserDialog();
}

void CartManager::_refreshFileChooserDialog()
{

}

void CartManager::_showFileChooserDialog()
{

}

////////////////////////////////////////////////////////
// set times from gui widgets

void CartManager::_setArchiveStartTimeFromGui(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiStartTime.set(date.year(), date.month(), date.day(),
                    time.hour(), time.minute(), time.second());
}

void CartManager::_setArchiveEndTimeFromGui(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiEndTime.set(date.year(), date.month(), date.day(),
                  time.hour(), time.minute(), time.second());
}

void CartManager::_acceptGuiTimes()
{
  _archiveStartTime = _guiStartTime;
  _archiveEndTime = _guiEndTime;
  loadArchiveFileList();
}

void CartManager::_cancelGuiTimes()
{
  _setGuiFromArchiveStartTime();
  _setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void CartManager::_setGuiFromArchiveStartTime()
{
  if (!_archiveStartTimeEdit) {
    return;
  }
  QDate date(_archiveStartTime.getYear(), 
             _archiveStartTime.getMonth(),
             _archiveStartTime.getDay());
  QTime time(_archiveStartTime.getHour(),
             _archiveStartTime.getMin(),
             _archiveStartTime.getSec());
  QDateTime datetime(date, time);
  _archiveStartTimeEdit->setDateTime(datetime);
  _guiStartTime = _archiveStartTime;
}

////////////////////////////////////////////////////////
// set gui widget from archive end time

void CartManager::_setGuiFromArchiveEndTime()
{
  if (!_archiveEndTimeEdit) {
    return;
  }
  QDate date(_archiveEndTime.getYear(), 
             _archiveEndTime.getMonth(),
             _archiveEndTime.getDay());
  QTime time(_archiveEndTime.getHour(),
             _archiveEndTime.getMin(),
             _archiveEndTime.getSec());
  QDateTime datetime(date, time);
  _archiveEndTimeEdit->setDateTime(datetime);
  _guiEndTime = _archiveEndTime;
}

////////////////////////////////////////////////////////
// set gui selected time label

void CartManager::_setGuiFromSelectedTime()
{
  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           _selectedTime.getYear(),
           _selectedTime.getMonth(),
           _selectedTime.getDay(),
           _selectedTime.getHour(),
           _selectedTime.getMin(),
           _selectedTime.getSec());
  _selectedTimeLabel->setText(text);
}

////////////////////////////////////////////////////////
// set archive start time

void CartManager::_setArchiveStartTime(const RadxTime &rtime)

{
  _archiveStartTime = rtime;
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromArchiveStartTime();
}

////////////////////////////////////////////////////////
// set archive end time

void CartManager::_setArchiveEndTime(const RadxTime &rtime)

{
  _archiveEndTime = rtime;
  if (!_archiveEndTime.isValid()) {
    _archiveEndTime.set(RadxTime::NOW);
  }
  _setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// change start time

void CartManager::_goBack1()
{
  if (_archiveScanIndex > 0) {
    _archiveScanIndex -= 1;
    _setArchiveRetrievalPending();
  } else {
    if (_params.debug) {
      cerr << "At start of data, cannot go back" << endl;
    }
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void CartManager::_goBackPeriod()
{

  int archiveSpanSecs = _archiveEndTime - _archiveStartTime;
  _archiveStartTime -= archiveSpanSecs;
  _archiveEndTime -= archiveSpanSecs;
  loadArchiveFileList();
  if (_archiveScanIndex > (int) _archiveFileList.size() - 1) {
    _archiveScanIndex = (int) _archiveFileList.size() - 1;
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);

}

void CartManager::_goFwd1()
{
  if (_archiveScanIndex < (int) _archiveFileList.size() - 1) {
    _archiveScanIndex += 1;
    _setArchiveRetrievalPending();
  } else {
    if (_params.debug) {
      cerr << "At end of data, cannot go forward" << endl;
    }
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);
}

void CartManager::_goFwdPeriod()
{

  int archiveSpanSecs = _archiveEndTime - _archiveStartTime;
  _archiveStartTime += archiveSpanSecs;
  _archiveEndTime += archiveSpanSecs;
  loadArchiveFileList();
  if (_archiveScanIndex > (int) _archiveFileList.size() - 1) {
    _archiveScanIndex = (int) _archiveFileList.size() - 1;
  }
  _timeSlider->setSliderPosition(_archiveScanIndex);

}

////////////////////////////////////////////////////////
// set for pending archive retrieval

void CartManager::_setArchiveRetrievalPending()
{
  _archiveRetrievalPending = true;
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

void CartManager::_setArchiveMode(bool state)
{
  _archiveMode = state;
  _setSweepPanelVisibility();

  if (_horiz) {
    _horiz->setArchiveMode(state);
  }
  if (_vert) {
    _vert->setArchiveMode(state);
  }
}

////////////////////////////////////////////////////////
// set modes for retrieving the data

void CartManager::_setRealtime(bool enabled)
{
  if (enabled) {
    if (_archiveMode) {
      _setArchiveMode(false);
      _activateRealtimeRendering();
    }
  } else {
    if (!_archiveMode) {
      _setArchiveMode(true);
      _activateArchiveRendering();
      loadArchiveFileList();
    }
  }
}


/////////////////////////////////////
// activate realtime rendering

void CartManager::_activateRealtimeRendering()
{
  _nGates = 1000;
  _maxRangeKm = 1000;
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

  int interval = _params.images_schedule_interval_secs;
  int delay = _params.images_schedule_delay_secs;
  time_t latestRayTime = _readerRayTime.utime();
  
  // initialize the schedule if required

  if (_imagesScheduledTime.utime() == 0) {
    if (latestRayTime == 0) {
      // no data yet
      return;
    }
    time_t nextUtime = ((latestRayTime / interval) + 1) * interval;
    _imagesScheduledTime.set(nextUtime + delay);
    if (_params.debug) {
      cerr << "Next scheduled time for images: " 
           << _imagesScheduledTime.asString() << endl;
    }
  }
  
  if (_readerRayTime > _imagesScheduledTime) {

    // create images

    _archiveEndTime = _imagesScheduledTime - delay;
    _archiveStartTime = _archiveEndTime - _imagesScanIntervalSecs;
    _createImageFilesAllSweeps();

    // set next scheduled time
    
    time_t nextUtime = ((latestRayTime / interval) + 1) * interval;
    _imagesScheduledTime.set(nextUtime + delay);
    if (_params.debug) {
      cerr << "Next scheduled time for images: "
           << _imagesScheduledTime.asString() << endl;
    }

  }

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
        _createImageFilesAllSweeps();
        _archiveFileList.erase(_archiveFileList.begin(), 
                               _archiveFileList.begin() + 1);
      }
      
    } else {
      
      // using archive time to drive image generation
      
      while (_archiveStartTime <= _imagesArchiveEndTime) {
        _createImageFilesAllSweeps();
        _archiveStartTime += _imagesScanIntervalSecs;
      }
      
    }
    
  } else if (_params.images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    
    for (RadxTime stime = _imagesArchiveStartTime;
         stime <= _imagesArchiveEndTime;
         stime += _params.images_schedule_interval_secs) {
      
      _archiveStartTime = stime;
      _archiveEndTime = _archiveStartTime + _imagesScanIntervalSecs;
      
      _createImageFilesAllSweeps();
      
    } // stime
    
  } // if (_params.images_creation_mode ...

}

/////////////////////////////////////////////////////
// creating one image per field, for each sweep

void CartManager::_createImageFilesAllSweeps()
{
  
  if (_params.images_set_sweep_index_list) {
    
    for (int ii = 0; ii < _params.images_sweep_index_list_n; ii++) {
      int index = _params._images_sweep_index_list[ii];
      if (index >= 0 && index < (int) _vol.getNSweeps()) {
        // _sweepManager.setFileIndex(index);
        _createImageFiles();
      }
    }
    
  } else {
    
    for (size_t index = 0; index < _vol.getNSweeps(); index++) {
      // _sweepManager.setFileIndex(index);
      _createImageFiles();
    }
    
  }

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

    if (_params.images_include_scan_id_in_file_name) {
      fileName += _params.images_file_name_delimiter;
      fileName += std::to_string(_vol.getScanId());
    }

    // scan type

    if (_params.images_include_scan_type_in_file_name) {
      fileName += _params.images_file_name_delimiter;
      string scanType = "SUR";
      if (_vol.getSweeps().size() > 0) {
        Radx::SweepMode_t predomSweepMode = _vol.getPredomSweepMode();
        scanType = Radx::sweepModeToShortStr(predomSweepMode);
      }
      fileName += scanType;
    }

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
    ldataInfo.setWriter("HawkEye");
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
  _horiz->ShowContextMenu(pos, &_vol);
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
void CartManager::createBoundaryEditorDialog()
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
  connect(_boundaryEditorPolygonBtn, SIGNAL(clicked()), this, SLOT(polygonBtnBoundaryEditorClick()));

	_boundaryEditorCircleBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorCircleBtn->setMaximumWidth(130);
	_boundaryEditorCircleBtn->setText(" Circle  ");
	_boundaryEditorCircleBtn->setIcon(QIcon("images/circle.png"));
	_boundaryEditorCircleBtn->setCheckable(TRUE);
	_boundaryEditorCircleBtn->setFocusPolicy(Qt::NoFocus);
	_boundaryEditorDialogLayout->addWidget(_boundaryEditorCircleBtn, ++row, 0);
  connect(_boundaryEditorCircleBtn, SIGNAL(clicked()), this, SLOT(circleBtnBoundaryEditorClick()));

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
  connect(_circleRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(_circleRadiusSliderValueChanged(int)));

	_boundaryEditorBrushBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorBrushBtn->setMaximumWidth(130);
	_boundaryEditorBrushBtn->setText(" Brush ");
	_boundaryEditorBrushBtn->setIcon(QIcon("images/brush.png"));
	_boundaryEditorBrushBtn->setCheckable(TRUE);
	_boundaryEditorBrushBtn->setFocusPolicy(Qt::NoFocus);
	_boundaryEditorDialogLayout->addWidget(_boundaryEditorBrushBtn, ++row, 0);
  connect(_boundaryEditorBrushBtn, SIGNAL(clicked()), this, SLOT(brushBtnBoundaryEditorClick()));

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
  connect(_brushRadiusSlider, SIGNAL(valueChanged(int)), this, SLOT(_brushRadiusSliderValueChanged(int)));

  _boundaryEditorBrushBtn->setChecked(TRUE);
	_boundaryEditorDialogLayout->addWidget(new QLabel(" ", _boundaryEditorDialog), ++row, 0, 1, 2, alignCenter);

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
  connect(_boundaryEditorList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onBoundaryEditorListItemClicked(QListWidgetItem*)));

	// horizontal layout contains the "Clear", "Help", and "Save" buttons
	QHBoxLayout *hLayout = new QHBoxLayout;
	_boundaryEditorDialogLayout->addLayout(hLayout, ++row, 0, 1, 2);

	_boundaryEditorClearBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorClearBtn->setText("Clear");
	hLayout->addWidget(_boundaryEditorClearBtn);
  connect(_boundaryEditorClearBtn, SIGNAL(clicked()), this, SLOT(clearBoundaryEditorClick()));

	_boundaryEditorHelpBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorHelpBtn->setText("Help");
	hLayout->addWidget(_boundaryEditorHelpBtn);
  connect(_boundaryEditorHelpBtn, SIGNAL(clicked()), this, SLOT(helpBoundaryEditorClick()));

  _boundaryEditorSaveBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorSaveBtn->setText("Save");
	hLayout->addWidget(_boundaryEditorSaveBtn);
}
#endif

