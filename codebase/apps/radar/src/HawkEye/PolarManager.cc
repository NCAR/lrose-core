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
// HawkEye.cc
//
// HawkEye object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// PolarManager manages polar rendering - PPIs and RHIs
//
///////////////////////////////////////////////////////////////

#include "PolarManager.hh"
#include "DisplayField.hh"
#include "PpiWidget.hh"
#include "RhiWidget.hh"
#include "RhiWindow.hh"
// #include "ColorBar.hh"
#include "Params.hh"
#include "Reader.hh"

#include <string>
#include <cmath>
#include <iostream>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QErrorMessage>

#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <dsserver/DsLdataInfo.hh>
#include <radar/RadarComplex.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxSweep.hh>

using namespace std;

// Constructor

PolarManager::PolarManager(const Params &params,
                           Reader *reader,
                           const vector<DisplayField *> &fields,
                           bool haveFilteredFields) :
        DisplayManager(params, reader, fields, haveFilteredFields),
	_rhiWindowDisplayed(false)
{

  _ppi = NULL;
  _rhi = NULL;
  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  _ppiRays = NULL;
  _rhiMode = false;
  _sweepIndex = 0;

  _firstVol = true;
  _moveToHighSweep = false;
  _keepFixedAngle = false;
  _fixedAngleDeg = -9999.0;

  // initialize geometry
  
  _nGates = 1000;
  _maxRangeKm = _params.max_range_km;

  _realtimeModeButton = NULL;
  _archiveModeButton = NULL;

  _archiveRetrievalPending = false;
  _archiveTimeBox = NULL;
  _archiveStartTimeEdit = NULL;
  _archiveStopTimeEcho = NULL;

  _setArchiveMode(_params.begin_in_archive_mode);
  _archiveStartTime.set(_params.archive_start_time);
  _archiveMarginSecs = _params.archive_search_margin_secs;

  _imagesArchiveStartTime.set(_params.images_archive_start_time);
  _imagesArchiveEndTime.set(_params.images_archive_end_time);

  // set up ray locators

  _ppiRays = new RayLoc[RayLoc::RAY_LOC_N];
  _ppiRayLoc = _ppiRays + RayLoc::RAY_LOC_OFFSET;

  // set up windows

  _setupWindows();

  // set initial field to 0

  _changeField(0, false);

}

// destructor

PolarManager::~PolarManager()

{

  if (_ppi) {
    delete _ppi;
  }

  if (_rhi) {
    delete _rhi;
  }

  if (_ppiRays) {
    delete[] _ppiRays;
  }
  
}

//////////////////////////////////////////////////
// Run

int PolarManager::run(QApplication &app)
{

  if (_params.debug) {
    cerr << "Running in POLAR mode" << endl;
  }

  // make window visible

  show();
  
  // set timer running
  
  _beamTimerId = startTimer(2);
  
  return app.exec();

}

//////////////////////////////////////////////////
// enable the zoom button - called by PolarWidgets

void PolarManager::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

//////////////////////////////////////////////////
// time axis changes

void PolarManager::_cancelTimeControllerChanges()
{
  _refreshTimeControllerDialog();
  _timeControllerDialog->setVisible(false);
}

//////////////////////////////////////////////////////////////
// respond to timer events
  
void PolarManager::timerEvent(QTimerEvent *event)
{

  // register with procmap

  PMU_auto_register("timerEvent");
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.

  if (_firstTimerEvent) {

    _ppi->resize(_ppiFrame->width(), _ppiFrame->height());
    
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
  
    if (_archiveMode) {
      _archiveRetrievalPending = true;
    }

    _firstTimerEvent = false;

  } // if (_firstTimerEvent)

  // handle event
  
  if (event->timerId() == _beamTimerId) {
    
    if (_archiveMode) {
      if (_archiveRetrievalPending) {
        _handleArchiveData(event);
        _archiveRetrievalPending = false;
      }
    } else {
      _handleRealtimeData(event);
    }

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

void PolarManager::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent: " << event << endl;
  }
  emit frameResized(_ppiFrame->width(), _ppiFrame->height());
}

////////////////////////////////////////////////////////////////
void PolarManager::keyPressEvent(QKeyEvent * e)
{

  // get key pressed

  Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toAscii().data()[0];
  int key = e->key();

  if (_params.debug) {
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

  // check for back or forward in time
  // or up/down in sweep number

  bool moveUpDown = false;
  _keepFixedAngle = false;
  
  if (key == Qt::Key_Left) {

    if (_params.debug) {
      cerr << "Clicked left arrow, go back in time" << endl;
    }
    _keepFixedAngle = true;
    _ppi->setStartOfSweep(true);
    _rhi->setStartOfSweep(true);
    _goBack1();
    _setArchiveRetrievalPending();

  } else if (key == Qt::Key_Right) {

    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _keepFixedAngle = true;
    _ppi->setStartOfSweep(true);
    _rhi->setStartOfSweep(true);
    _goFwd1();
    _setArchiveRetrievalPending();
    
  } else if (key == Qt::Key_Up) {

    if (_sweepIndex < (int) _vol.getNSweeps() - 1) {

      _sweepIndex++;
      moveUpDown = true;
      _keepFixedAngle = false;
      _setFixedAngle(_sweepIndex);
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);

    } else {

      if (_params.debug) {
        cerr << "Clicked up arrow, moving forward in time" << endl;
      }
      _moveToHighSweep = false; // start with low sweep of next volume
      _keepFixedAngle = false;
      _setFixedAngle(_sweepIndex);
      _goFwd1();
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);
      _setArchiveRetrievalPending();

    }

  } else if (key == Qt::Key_Down) {

    if (_sweepIndex > 0) {

      _sweepIndex--;
      _keepFixedAngle = false;
      _setFixedAngle(_sweepIndex);
      moveUpDown = true;
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);

    } else {

      if (_params.debug) {
        cerr << "Clicked down arrow, go back in time" << endl;
      }
      _keepFixedAngle = false;
      _moveToHighSweep = true;
      _setFixedAngle(_sweepIndex);
      _goBack1();
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);
      _setArchiveRetrievalPending();

    }

  }

  if (moveUpDown) {
    if (_params.debug) {
      cerr << "Clicked up/down arrow, change to sweep num: " 
           << _sweepIndex << endl;
    }
    this->setCursor(Qt::WaitCursor);
    _timeControllerDialog->setCursor(Qt::WaitCursor);
    _plotArchiveData();
    this->setCursor(Qt::ArrowCursor);
    _timeControllerDialog->setCursor(Qt::ArrowCursor);
  }

}


//////////////////////////////////////////////////
// Set radar name in title bar

void PolarManager::_setTitleBar(const string &radarName)
{
  string windowTitle = "HAWK_EYE -- " + radarName;
  setWindowTitle(tr(windowTitle.c_str()));
}
  
//////////////////////////////////////////////////
// set up windows and widgets
  
void PolarManager::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // ppi - main window

  _ppiFrame = new QFrame(_main);
  _ppiFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the PPI

  _ppi = new PpiWidget(_ppiFrame, *this, _params, _platform, _fields.size());

  connect(this, SIGNAL(frameResized(const int, const int)),
	  _ppi, SLOT(resize(const int, const int)));
  
  // Create the RHI window

  _rhiWindow = new RhiWindow(this, _params, _platform, _fields);
  _rhiWindow->setRadarName(_params.radar_name);

  // set pointer to the rhiWidget

  _rhi = _rhiWindow->getWidget();
  
  // connect slots for location

  connect(_ppi, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_ppiLocationClicked(double, double, const RadxRay*)));
  connect(_rhi, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_rhiLocationClicked(double, double, const RadxRay*)));

  // create status panel

  _createStatusPanel();

  // create fields panel
  
  _createFieldPanel();

  // color bar to right
  
  // _colorBar = new ColorBar(_params.color_scale_width,
  //                          &_fields[0]->getColorMap(), _main);
  
  // main window layout
  
  QHBoxLayout *mainLayout = new QHBoxLayout(_main);
  mainLayout->setMargin(3);
  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_fieldPanel);
  mainLayout->addWidget(_ppiFrame);
  // mainLayout->addWidget(_colorBar);

  _createActions();
  _createMenus();

  // QString message = tr("A context menu is available by right-clicking");
  // statusBar()->showMessage(message);

  _setTitleBar(_params.radar_name);
  setMinimumSize(400, 300);
  resize(_params.main_window_width, _params.main_window_height);

  QPoint pos;
  pos.setX(_params.main_window_start_x);
  pos.setY(_params.main_window_start_y);
  move(pos);

  // set up field status dialog

  _createClickReportDialog();

  // create the time controller settings dialog
  
  _createTimeControllerDialog();

}

//////////////////////////////
// create actions for menus

void PolarManager::_createActions()
{

  // freeze display

  _freezeAct = new QAction(tr("Freeze"), this);
  _freezeAct->setShortcut(tr("Esc"));
  _freezeAct->setStatusTip(tr("Freeze display"));
  connect(_freezeAct, SIGNAL(triggered()), this, SLOT(_freeze()));
  
  // show user click in dialog

  _showClickAct = new QAction(tr("Show Click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, SIGNAL(triggered()), this, SLOT(_showClick()));

  // set time controller settings

  _timeControllerAct = new QAction(tr("Time-Config"), this);
  _timeControllerAct->setStatusTip(tr("Set configuration for time controller"));
  connect(_timeControllerAct, SIGNAL(triggered()), this,
          SLOT(_showTimeControllerDialog()));

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
  connect(_clearAct, SIGNAL(triggered()), _ppi, SLOT(clear()));
  connect(_clearAct, SIGNAL(triggered()), _rhi, SLOT(clear()));

  // exit app

  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // show range rings

  _ringsAct = new QAction(tr("Range Rings"), this);
  _ringsAct->setStatusTip(tr("Turn range rings on/off"));
  _ringsAct->setCheckable(true);
  _ringsAct->setChecked(_params.ppi_range_rings_on_at_startup);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setRings(bool)));

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params.ppi_grids_on_at_startup);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setGrids(bool)));

  // show azimuth lines

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params.ppi_azimuth_lines_on_at_startup);
  connect(_azLinesAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setAngleLines(bool)));

  // show RHI window

  _showRhiAct = new QAction(tr("Show RHI Window"), this);
  _showRhiAct->setStatusTip(tr("Show the RHI Window"));
  connect(_showRhiAct, SIGNAL(triggered()), _rhiWindow, SLOT(show()));

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

void PolarManager::_createMenus()
{

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addSeparator();
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _overlaysMenu->addAction(_ringsAct);
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_azLinesAct);
  _overlaysMenu->addSeparator();
  _overlaysMenu->addAction(_showRhiAct);

  menuBar()->addAction(_timeControllerAct);
  menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showClickAct);
  menuBar()->addAction(_unzoomAct);
  menuBar()->addAction(_clearAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

/////////////////////////////
// get data in realtime mode

void PolarManager::_handleRealtimeData(QTimerEvent * event)

{

  _ppi->setArchiveMode(false);
  _rhi->setArchiveMode(false);

  // do nothing if freeze is on

  if (_frozen) {
    return;
  }

  if (event->timerId() == _beamTimerId && !_frozen) {

    // get all available beams

    while (true) {

      // get the next ray from the reader queue
      // responsibility for this ray memory passes to
      // this (the master) thread

      RadxRay *ray = _reader->getNextRay(_platform);
      if (ray == NULL) {
        break; // no pending rays
      }

      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "  Got a ray, time, el, az: "
             << DateTime::strm(ray->getTimeSecs()) << ", "
             << ray->getElevationDeg() << ", "
             << ray->getAzimuthDeg() << endl;
      }

      // update the status panel
      
      _updateStatusPanel(ray);
      
      // draw the beam
      
      if (_params.images_creation_mode != 
          Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
        _handleRay(_platform, ray);
      }
    
    } // while

  }
    
}

///////////////////////////////////////
// handle data in archive mode

void PolarManager::_handleArchiveData(QTimerEvent * event)

{

  _ppi->setArchiveMode(true);
  _ppi->setStartOfSweep(true);

  _rhi->setArchiveMode(true);
  _rhi->setStartOfSweep(true);

  // set up plot times

  _plotStartTime = _archiveStartTime;
  _plotEndTime = _plotStartTime +
    _params.archive_scan_interval_secs * _params.archive_n_scans;

  // set cursor to wait cursor

  this->setCursor(Qt::WaitCursor);
  _timeControllerDialog->setCursor(Qt::WaitCursor);

  // get data
  
  if (_getArchiveData()) {
    this->setCursor(Qt::ArrowCursor);
    _timeControllerDialog->setCursor(Qt::ArrowCursor);
    return;
  }
  
  _activateArchiveRendering();
  
  if (_vol.checkIsRhi()) {
    _rhiMode = true;
  } else {
    _rhiMode = false;
  }

  // plot the data
  
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
  _timeControllerDialog->setCursor(Qt::ArrowCursor);

}

/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure

int PolarManager::_getArchiveData()

{

  // set up file object for reading
  
  RadxFile file;
  _vol.clear();
  _setupVolRead(file);

  if (_inputFileList.size() > 0) {

    // files were specified on the command line

    string inputPath = _inputFileList[0];
    if (file.readFromPath(inputPath, _vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "PolarManager::_getArchiveData\n";
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

    _archiveStartTime.set(_vol.getStartTimeSecs());
    _setGuiFromStartTime();
    
  } else {

    // times were specified on the command line

    if (file.readFromDir(_params.archive_data_url, _vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "PolarManager::_getArchiveData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  start time: " + _archiveStartTime.asString() + "\n";
      char text[1024];
      sprintf(text, "  margin secs: %d\n", _archiveMarginSecs);
      errMsg += text;
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

  // compute the fixed angles from the rays
  // so that we reflect reality
  
  _vol.computeFixedAngleFromRays();

  // for first retrieval, start with sweepIndex of 0

  if (_firstVol) {
    _sweepIndex = 0;
    _setFixedAngle(_sweepIndex);
  }
  _firstVol = false;
  
  // condition sweep number
  
  if (_keepFixedAngle) {
    _setSweepIndex(_fixedAngleDeg);
  } else if (_moveToHighSweep) {
    _sweepIndex = _vol.getNSweeps() - 1;
    _setFixedAngle(_sweepIndex);
  } else {
    _sweepIndex = 0;
    _setFixedAngle(_sweepIndex);
  }
  
  if (_params.debug) {
    cerr << "----------------------------------------------------" << endl;
    cerr << "perform archive retrieval" << endl;
    cerr << "  read file: " << _vol.getPathInUse() << endl;
    cerr << "  nSweeps: " << _vol.getNSweeps() << endl;
    cerr << "  _sweepIndex, _fixedAngleDeg: " 
         << _sweepIndex << ", " << _fixedAngleDeg << endl;
    cerr << "----------------------------------------------------" << endl;
  }
  
   _platform = _vol.getPlatform();

  return 0;

}

/////////////////////////////////////////
// set the sweep index from fixed angle

void PolarManager::_setSweepIndex(double fixedAngle)
{
  const vector<RadxSweep *> &sweeps = _vol.getSweeps();
  if (sweeps.size() < 1) {
    _sweepIndex = 0;
    return;
  }
  double minDiff = 9999.0;
  int bestIndex = 0;
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    const RadxSweep *sweep = sweeps[ii];
    double diff = 
      fabs(Radx::computeAngleDiff(fixedAngle, sweep->getFixedAngleDeg()));
    if (diff < minDiff) {
      minDiff = diff;
      bestIndex = ii;
    }
  } // ii
  _sweepIndex = bestIndex;
}

////////////////////////////////////////////
// set the fixed angle from the sweep index

void PolarManager::_setFixedAngle(int sweepNum)
{
  const vector<RadxSweep *> &sweeps = _vol.getSweeps();
  if (sweeps.size() < 1) {
    _fixedAngleDeg = -8888.0;
    return;
  }
  if (sweepNum < 0) {
    sweepNum = 0;
  }
  if (sweepNum > (int) sweeps.size() - 1) {
    sweepNum = sweeps.size() - 1;
  }
  const RadxSweep *sweep = sweeps[sweepNum];
  _fixedAngleDeg = sweep->getFixedAngleDeg();
}

/////////////////////////////
// plot data in archive mode

void PolarManager::_plotArchiveData()

{

  if(_params.debug) {
    cerr << "======== Plotting archive data =======================" << endl;
    cerr << "======>>   plotStartTime: " << _plotStartTime.asString() << endl;
    cerr << "======>>   plotEndTime: " << _plotEndTime.asString() << endl;
  }

  // initialize plotting

  _initialRay = true;

  // handle the rays
  
  const vector<RadxRay *> &rays = _vol.getRays();
  if (rays.size() < 1) {
    cerr << "ERROR - _plotArchiveData" << endl;
    cerr << "  No rays found" << endl;
    return;
  }
  
  const vector<RadxSweep *> &sweeps = _vol.getSweeps();
  if (sweeps.size() < 1) {
    cerr << "ERROR - _plotArchiveData" << endl;
    cerr << "  No sweeps found" << endl;
    return;
  }

  if (_sweepIndex > (int) sweeps.size()) {
    _sweepIndex = (int) sweeps.size() - 1;
  }

  // clear the canvas

  _clear();

  // handle the rays

  for (size_t ii = sweeps[_sweepIndex]->getStartRayIndex();
       ii <= sweeps[_sweepIndex]->getEndRayIndex(); ii++) {
    RadxRay *ray = rays[ii];
    _handleRay(_platform, ray);
  }

  // update the status panel
  
  _updateStatusPanel(rays[sweeps[_sweepIndex]->getStartRayIndex()]);
    
}

//////////////////////////////////////////////////
// set up read

void PolarManager::_setupVolRead(RadxFile &file)
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

  file.setReadModeClosest(_archiveStartTime, _archiveMarginSecs);

  // if (_params.max_range_km > 0) {
  //   file.setReadMaxRangeKm(_params.max_range_km);
  // }

}

//////////////////////////////////////////////////////////////
// handle an incoming ray

void PolarManager::_handleRay(RadxPlatform &platform, RadxRay *ray)
  
{

  // do we need to reconfigure the PPI?

  int nGates = ray->getNGates();
  double maxRange = ray->getStartRangeKm() + nGates * ray->getGateSpacingKm();
  
  if ((maxRange - _maxRangeKm) > 0.001) {
    _nGates = nGates;
    _maxRangeKm = maxRange;
    _ppi->configureRange(_maxRangeKm);
    _rhi->configureRange(_maxRangeKm);
  }

  // create 2D field data vector

  vector< vector<double> > fieldData;
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    vector<double> field;
    fieldData.push_back(field);
  }

  // fill data vector

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    vector<double> &data = fieldData[ifield];
    RadxField *rfld = ray->getField(_fields[ifield]->getName());
    if (rfld == NULL) {
      // fill with missing
      for (int igate = 0; igate < _nGates; igate++) {
        data.push_back(-9999);
      }
    } else {
      rfld->convertToFl32();
      const Radx::fl32 *fdata = rfld->getDataFl32();
      const Radx::fl32 missingVal = rfld->getMissingFl32();
      for (int igate = 0; igate < _nGates; igate++, fdata++) {
        Radx::fl32 val = *fdata;
        if (fabs(val - missingVal) < 0.0001) {
          data.push_back(-9999);
        } else {
          data.push_back(*fdata);
        }
      }
    }
  }

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the PPI or RHI, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {

    _rhiMode = true;

    // Store the ray location using the elevation angle and the RHI location
    // table

    // double el = 90.0 - ray->getElevationDeg();
    // if (el < 0.0)
    //   el += 360.0;
    // _storeRayLoc(ray, el, platform.getRadarBeamWidthDegV(), _rhiRayLoc);

    // Save the angle information for the next iteration

    // _prevEl = el;
    // _prevAz = -9999.0;
    
    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      _rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }

    // Add the beam to the display

    _rhi->addBeam(ray, fieldData, _fields);
    _rhiWindow->setAzimuth(ray->getAzimuthDeg());
    _rhiWindow->setElevation(ray->getElevationDeg());
    
  } else {

    _rhiMode = false;

    // Store the ray location using the azimuth angle and the PPI location
    // table

    double az = ray->getAzimuthDeg();
    _storeRayLoc(ray, az, platform.getRadarBeamWidthDegH(), _ppiRayLoc);

    // Save the angle information for the next iteration

    _prevAz = az;
    _prevEl = -9999.0;

    // Add the beam to the display
    
    _ppi->addBeam(ray, _startAz, _endAz, fieldData, _fields);

  }
  
}

///////////////////////////////////////////////////////////
// store ray location

void PolarManager::_storeRayLoc(const RadxRay *ray, const double az,
                                const double beam_width, RayLoc *ray_loc)
{
  // Determine the extent of this ray

  if (ray->getIsIndexed())
  {
    double half_angle = ray->getAngleResDeg() / 2.0;
    _startAz = az - half_angle;
    _endAz = az + half_angle;
  }
  else
  {
    double max_half_angle = beam_width / 2.0;
    double prev_offset = max_half_angle;
    if (_prevAz >= 0.0)
    {
      double az_diff = az - _prevAz;
      if (az_diff < 0.0)
	az_diff += 360.0;
      double half_az_diff = az_diff / 2.0;
	
      if (prev_offset > half_az_diff)
	prev_offset = half_az_diff;
    }
      
    _startAz = az - prev_offset;
    _endAz = az + max_half_angle;
  }
    
  // store
    
  int startIndex = (int) (_startAz * RayLoc::RAY_LOC_RES);
  int endIndex = (int) (_endAz * RayLoc::RAY_LOC_RES + 1);

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  _clearRayOverlap(startIndex, endIndex, ray_loc);

  // Set the locations associated with this ray

  for (int ii = startIndex; ii <= endIndex; ii++) {
    ray_loc[ii].ray = ray;
    ray_loc[ii].active = true;
    ray_loc[ii].master = false;
    ray_loc[ii].startIndex = startIndex;
    ray_loc[ii].endIndex = endIndex;
  }

  // indicate which ray is the master
  // i.e. it is responsible for ray memory
    
  int midIndex = (int) (az * RayLoc::RAY_LOC_RES);
  ray_loc[midIndex].master = true;
  ray->addClient();

}

///////////////////////////////////////////////////////////
// clear any locations that are overlapped by the given ray

void PolarManager::_clearRayOverlap(const int start_index, const int end_index,
                                    RayLoc *ray_loc)
{
  // Loop through the ray locations, clearing out old information

  int i = start_index;
  
  while (i <= end_index) {

    RayLoc &loc = ray_loc[i];
    
    // If this location isn't active, we can skip it

    if (!loc.active) {
      ++i;
      continue;
    }
    
    int loc_start_index = loc.startIndex;
    int loc_end_index = loc.endIndex;
      
    // If we get here, this location is active.  We now have 4 possible
    // situations:

    if (loc.startIndex < start_index && loc.endIndex <= end_index) {

      // The overlap area covers the end of the current beam.  Reduce the
      // current beam down to just cover the area before the overlap area.

      for (int j = start_index; j <= loc_end_index; ++j) {
	// If the master is in the overlap area, then it needs to be moved
	// outside of this area

	if (ray_loc[j].master)
	  ray_loc[start_index-1].master = true;
	
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	ray_loc[j].master = false;
      }

      // Update the end indices for the remaining locations in the current
      // beam

      for (int j = loc_start_index; j < start_index; ++j)
	ray_loc[j].endIndex = start_index - 1;

    } else if (loc.startIndex < start_index && loc.endIndex > end_index) {
      
      // The current beam is bigger than the overlap area.  This should never
      // happen, so go ahead and just clear out the locations for the current
      // beam.

      for (int j = loc_start_index; j <= loc_end_index; ++j) {
        ray_loc[j].clear();
      }

    } else if (loc.endIndex > end_index) {
      
      // The overlap area covers the beginning of the current beam.  Reduce the
      // current beam down to just cover the area after the overlap area.

      for (int j = loc_start_index; j <= end_index; ++j) {
	// If the master is in the overlap area, then it needs to be moved
	// outside of this area
	if (ray_loc[j].master)
	  ray_loc[end_index+1].master = true;
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	ray_loc[j].master = false;
      }

      // Update the start indices for the remaining locations in the current
      // beam

      for (int j = end_index + 1; j <= loc_end_index; ++j) {
	ray_loc[j].startIndex = end_index + 1;
      }

    } else {
      
      // The current beam is completely covered by the overlap area.  Clear
      // out all of the locations for the current beam.

      for (int j = loc_start_index; j <= loc_end_index; ++j) {
        ray_loc[j].clear();
      }

    }
    
    i = loc_end_index + 1;

  } /* endwhile - i */
  
}

////////////////////////////////////////////
// freeze / unfreeze

void PolarManager::_freeze()
{
  if (_frozen) {
    _frozen = false;
    _freezeAct->setText("Freeze");
    _freezeAct->setStatusTip(tr("Click to freeze display, or hit ESC"));
  } else {
    _frozen = true;
    _freezeAct->setText("Unfreeze");
    _freezeAct->setStatusTip(tr("Click to unfreeze display, or hit ESC"));
    _initialRay = true;
  }
}

////////////////////////////////////////////
// unzoom

void PolarManager::_unzoom()
{
  _ppi->unzoomView();
  _unzoomAct->setEnabled(false);
}

////////////////////////////////////////////
// refresh

void PolarManager::_refresh()
{
}

///////////////////////////////////////////////////////////
// respond to change field request from field button group

void PolarManager::_changeField(int fieldId, bool guiMode)

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
  
  _ppi->selectVar(_fieldNum);
  _rhi->selectVar(_fieldNum);

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

}

///////////////////////////////////////////////////
// respond to a change in click location on the PPI

void PolarManager::_ppiLocationClicked(double xkm, double ykm, const RadxRay *closestRay)

{

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
  
  int rayIndex = (int) (azDeg * RayLoc::RAY_LOC_RES);
  if (_params.debug) {
    cerr << "    rayIndex = " << rayIndex << endl;
  }
  
  const RadxRay *ray = _ppiRayLoc[rayIndex].ray;
  if (ray == NULL) {
    // no ray data yet
    if (_params.debug) {
      cerr << "    No ray data yet..." << endl;
      cerr << "      active = " << _ppiRayLoc[rayIndex].active << endl;
      cerr << "      master = " << _ppiRayLoc[rayIndex].master << endl;
      cerr << "      startIndex = " << _ppiRayLoc[rayIndex].startIndex << endl;
      cerr << "      endIndex = " << _ppiRayLoc[rayIndex].endIndex << endl;
    }
    return;
  }

  _locationClicked(xkm, ykm, _ppiRayLoc, ray);

}

///////////////////////////////////////////////////
// respond to a change in click location on the RHI

void PolarManager::_rhiLocationClicked(double xkm, double ykm, const RadxRay *closestRay)
  
{

  _locationClicked(xkm, ykm, _rhi->getRayLoc(), closestRay);

}

////////////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void PolarManager::_locationClicked(double xkm, double ykm,
                                    RayLoc *ray_loc, const RadxRay *ray)
  
{

  if (_params.debug) {
    cerr << "*** Entering PolarManager::_locationClicked()" << endl;
  }
  
  double range = sqrt(xkm * xkm + ykm * ykm);
  int gate = (int) 
    ((range - ray->getStartRangeKm()) / ray->getGateSpacingKm() + 0.5);

  if (gate < 0 || gate >= (int) ray->getNGates())
  {
    //user clicked outside of ray
    return;
  }

  if (_params.debug) {
    cerr << "Clicked on location: xkm, ykm: " << xkm << ", " << ykm << endl;
    cerr << "  range, gate: " << range << ", " << gate << endl;
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
  
  _setText(text, "%d", gate);
  _gateNumClicked->setText(text);
  
  _setText(text, "%6.2f", range);
  _rangeClicked->setText(text);
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setSelectValue(-9999);
    _fields[ii]->setDialogText("----");
  }
  
  for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
    const RadxField *field = ray->getField(ifield);
    const string fieldName = field->getName();
    if (fieldName.size() == 0) {
      continue;
    }
    Radx::fl32 *data = (Radx::fl32 *) field->getData();
    double val = data[gate];
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
    
}

///////////////////////////////////////////////////////
// create the time controller settings dialog
//
// This allows the user to control the time controller

void PolarManager::_createTimeControllerDialog()
{
  
  _timeControllerDialog = new QDialog(this);
  _timeControllerDialog->setWindowTitle("Time controller settings");
  
  QBoxLayout *timeControllerDialogLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, _timeControllerDialog);
  
  {  // archive / realtime mode
  
    QGroupBox *modeBox = new QGroupBox(_timeControllerDialog);
    QHBoxLayout *modeBoxLayout = new QHBoxLayout;
    modeBox->setLayout(modeBoxLayout);
    modeBox->setTitle("Set data retrieval mode");
    
    _realtimeModeButton = new QRadioButton(tr("Realtime mode"), this);
    _realtimeModeButton->setStatusTip(tr("Run in realtime mode"));
    _realtimeModeButton->setCheckable(true);
    connect(_realtimeModeButton, SIGNAL(clicked()), this, SLOT(_setDataRetrievalMode()));
    modeBoxLayout->addWidget(_realtimeModeButton);
    
    _archiveModeButton = new QRadioButton(tr("Archive mode"), this);
    _archiveModeButton->setStatusTip(tr("Run in archive mode"));
    _archiveModeButton->setCheckable(true);
    connect(_archiveModeButton, SIGNAL(clicked()), this, SLOT(_setDataRetrievalMode()));
    modeBoxLayout->addWidget(_archiveModeButton);
    
    QButtonGroup *modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);
    modeGroup->addButton(_realtimeModeButton);
    modeGroup->addButton(_archiveModeButton);
    
    if (_archiveMode) {
      _archiveModeButton->setChecked(true);
    } else {
      _realtimeModeButton->setChecked(true);
    }
    _setDataRetrievalMode();
    
    timeControllerDialogLayout->addWidget(modeBox, Qt::AlignCenter);
    
  } // archive / realtime mode
  
  {  // set nscans and scan interval

    QGroupBox *archiveScanIntervalBox = new QGroupBox(_timeControllerDialog);
    QVBoxLayout *archiveScanIntervalLayout = new QVBoxLayout;
    archiveScanIntervalBox->setLayout(archiveScanIntervalLayout);
    archiveScanIntervalBox->setTitle("Set nscans and interval for archive mode");
    
    QFrame *nArchiveScansEditLabel;
    _nArchiveScansEdit = 
      _addInputRow(archiveScanIntervalBox, archiveScanIntervalLayout,
                   "N scans in archive mode", "",
                   0, &nArchiveScansEditLabel);

    QFrame *archiveScanIntervalEditLabel;
    _archiveScanIntervalEdit = 
      _addInputRow(archiveScanIntervalBox, archiveScanIntervalLayout,
                   "Scan interval in archive (secs)", "",
                   0, &archiveScanIntervalEditLabel);
    
    _resetArchiveScanConfigToDefault();
    
    QFrame *acceptCancelReset = new QFrame;
    QHBoxLayout *horiz = new QHBoxLayout;
    acceptCancelReset->setLayout(horiz);
    
    QPushButton *acceptButton = new QPushButton(archiveScanIntervalBox);
    acceptButton->setText("Accept");
    horiz->addWidget(acceptButton);
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(_setArchiveScanConfig()));
    
    QPushButton *cancelButton = new QPushButton(archiveScanIntervalBox);
    cancelButton->setText("Cancel");
    horiz->addWidget(cancelButton);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(_cancelTimeControllerChanges()));
    
    QPushButton *resetButton = new QPushButton(archiveScanIntervalBox);
    resetButton->setText("Reset to default");
    horiz->addWidget(resetButton);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(_resetArchiveScanConfigToDefault()));

    archiveScanIntervalLayout->addWidget(acceptCancelReset);

    // add to main dialog

    timeControllerDialogLayout->addWidget(archiveScanIntervalBox, Qt::AlignCenter);

  } // scan interval for plot
  
  {  // set archival time retrieval

    // box for setting start timew

    _archiveTimeBox = new QGroupBox(_timeControllerDialog);
    QVBoxLayout *archiveTimeLayout = new QVBoxLayout;
    _archiveTimeBox->setLayout(archiveTimeLayout);
    _archiveTimeBox->setTitle("Set times for archive mode");

    // start time edit

    QFrame *timeStartFrame = new QFrame;
    QHBoxLayout *timeStartLayout = new QHBoxLayout;
    timeStartFrame->setLayout(timeStartLayout);
    
    QLabel *timeStartLabel = new QLabel(timeStartFrame);
    timeStartLabel->setText("Start time (UTC)");
    timeStartLayout->addWidget(timeStartLabel);
    
    _archiveStartTimeEdit = new QDateTimeEdit(_archiveTimeBox);
    _archiveStartTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    _setArchiveStartTimeToDefault();
    timeStartLayout->addWidget(_archiveStartTimeEdit);
    connect(_archiveStartTimeEdit, SIGNAL(dateTimeChanged(const QDateTime &)), 
            this, SLOT(_setStartTimeFromGui(const QDateTime &)));
    
    archiveTimeLayout->addWidget(timeStartFrame);

    // end time echo

    QFrame *timeEndFrame = new QFrame;
    QHBoxLayout *timeEndLayout = new QHBoxLayout;
    timeEndFrame->setLayout(timeEndLayout);
    
    QLabel *timeEndLabel = new QLabel(timeEndFrame);
    timeEndLabel->setText("End time (UTC)");
    timeEndLayout->addWidget(timeEndLabel);
    
    _archiveStopTimeEcho = new QLabel(_archiveTimeBox);
    _computeArchiveStopTime();
    timeEndLayout->addWidget(_archiveStopTimeEcho);
    
    archiveTimeLayout->addWidget(timeEndFrame);

    // back / forward
    
    QFrame *backFwd = new QFrame;
    QHBoxLayout *layout1 = new QHBoxLayout;
    backFwd->setLayout(layout1);
    
    QPushButton *backNScans = new QPushButton(backFwd);
    backNScans->setText("<< NScans");
    layout1->addWidget(backNScans);
    connect(backNScans, SIGNAL(clicked()), this, SLOT(_goBackNScans()));
    
    QPushButton *back1 = new QPushButton(backFwd);
    back1->setText("< 1");
    layout1->addWidget(back1);
    connect(back1, SIGNAL(clicked()), this, SLOT(_goBack1()));
    
    QPushButton *fwd1 = new QPushButton(backFwd);
    fwd1->setText("> 1");
    layout1->addWidget(fwd1);
    connect(fwd1, SIGNAL(clicked()), this, SLOT(_goFwd1()));
    
    QPushButton *fwdNScans = new QPushButton(backFwd);
    fwdNScans->setText(">> NScans");
    layout1->addWidget(fwdNScans);
    connect(fwdNScans, SIGNAL(clicked()), this, SLOT(_goFwdNScans()));
    
    archiveTimeLayout->addWidget(backFwd);

    // accept / cancel / reset
    
    QFrame *goCancelReset = new QFrame;
    QHBoxLayout *layout2 = new QHBoxLayout;
    goCancelReset->setLayout(layout2);
    
    QPushButton *goButton = new QPushButton(goCancelReset);
    goButton->setText("Go");
    QPalette goPalette = goButton->palette();
    QColor goColor(0, 210, 0); // green
    goPalette.setColor(QPalette::Active, QPalette::Button, goColor);
    // pal.setColor( QPalette::Inactive, QPalette::Button, color );
    goButton->setPalette(goPalette);
    layout2->addWidget(goButton);
    connect(goButton, SIGNAL(clicked()), this, SLOT(_setArchiveRetrievalPending()));
    
    QPushButton *cancelButton = new QPushButton(goCancelReset);
    cancelButton->setText("Cancel");
    layout2->addWidget(cancelButton);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(_cancelTimeControllerChanges()));
    
    QPushButton *resetButton = new QPushButton(goCancelReset);
    resetButton->setText("Reset to default");
    layout2->addWidget(resetButton);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(_setArchiveStartTimeToDefault()));

    archiveTimeLayout->addWidget(goCancelReset);

    // add to main dialog

    timeControllerDialogLayout->addWidget(_archiveTimeBox, Qt::AlignCenter);

  }  // set archival time retrieval
  
  // done?
  
  {
    
    QGroupBox *doneBox = new QGroupBox(_timeControllerDialog);
    QGridLayout *doneLayout = new QGridLayout;
    doneBox->setLayout(doneLayout);
    doneBox->setTitle("Done with all changes");

    int row = 0;
    QPushButton *done = new QPushButton(doneBox);
    done->setText("Done");
    doneLayout->addWidget(done, row++, 0, Qt::AlignCenter);
    connect(done, SIGNAL(clicked()), this, SLOT(_cancelTimeControllerChanges()));
    
    timeControllerDialogLayout->addWidget(doneBox, Qt::AlignCenter);

  } // done
  
  _refreshTimeControllerDialog();
  
}

///////////////////////////////////////////////////////
// set the state on the time controller dialog

void PolarManager::_refreshTimeControllerDialog()
{
  
  char text[1024];

  // set altitude limits text

  sprintf(text, "%g", _archiveScanIntervalSecs);
  _archiveScanIntervalEdit->setText(text);

}

/////////////////////////////////////
// show the time controller dialog

void PolarManager::_showTimeControllerDialog()
{

  if (_timeControllerDialog) {
    if (_timeControllerDialog->isVisible()) {
      _timeControllerDialog->setVisible(false);
    } else {
      if (!_archiveMode) {
        _setArchiveStartTime(_plotStartTime);
      }
      _refreshTimeControllerDialog();
      _timeControllerDialog->setVisible(true);
      _timeControllerDialog->raise();
      if (_timeControllerDialog->x() == 0 &&
          _timeControllerDialog->y() == 0) {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y() + (height() - _timeControllerDialog->height()));
        _timeControllerDialog->move(pos);
      }
    }
  }
}

////////////////////////////////////////////////////////
// change modes for retrieving the data

void PolarManager::_setDataRetrievalMode()
{
  if (!_archiveTimeBox) {
    return;
  }
  if (_realtimeModeButton && _realtimeModeButton->isChecked()) {
    if (_archiveMode) {
      _setArchiveMode(false);
      _activateRealtimeRendering();
    }
  } else {
    if (!_archiveMode) {
      _setArchiveMode(true);
      _activateArchiveRendering();
      if (_plotStartTime.utime() != 0) {
        _setArchiveStartTime(_plotStartTime - _archiveScanIntervalSecs * _nArchiveScans);
        _setGuiFromStartTime();
      }
    }
  }
}


////////////////////////////////////////////////////////
// set archive scan configuration from GUI

void PolarManager::_setArchiveScanConfig()
{

  double archiveScanInterval = 0;
  if (sscanf(_archiveScanIntervalEdit->text().toLocal8Bit().data(),
             "%lg", &archiveScanInterval) != 1) {
    QErrorMessage errMsg(_archiveScanIntervalEdit);
    string text("Bad entry for scan interval: ");
    text += _archiveScanIntervalEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetArchiveScanConfigToDefault();
    return;
  }
  _archiveScanIntervalSecs = archiveScanInterval;
  
  int nArchiveScans = 0;
  if (sscanf(_nArchiveScansEdit->text().toLocal8Bit().data(),
             "%d", &nArchiveScans) != 1) {
    QErrorMessage errMsg(_nArchiveScansEdit);
    string text("Bad entry for n scans: ");
    text += _nArchiveScansEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetArchiveScanConfigToDefault();
    return;
  }
  _nArchiveScans = nArchiveScans;

  _computeArchiveStopTime();
  
  if (_archiveMode) {
    _setArchiveRetrievalPending();
  }

}

////////////////////////////////////////////////////////
// reset to default archive scan config

void PolarManager::_resetArchiveScanConfigToDefault()
{

  char text[1024];

  _archiveScanIntervalSecs = _params.archive_scan_interval_secs;
  sprintf(text, "%g", _archiveScanIntervalSecs);
  if (_archiveScanIntervalEdit) {
    _archiveScanIntervalEdit->setText(text);
  }

  _nArchiveScans = _params.archive_n_scans;
  sprintf(text, "%d", _nArchiveScans);
  if (_nArchiveScansEdit) {
    _nArchiveScansEdit->setText(text);
  }

  _computeArchiveStopTime();

}

////////////////////////////////////////////////////////
// set start time from gui widget

void PolarManager::_setStartTimeFromGui(const QDateTime &datetime1)
{
  QDateTime datetime = _archiveStartTimeEdit->dateTime();
  QDate date = datetime.date();
  QTime time = datetime.time();
  _archiveStartTime.set(date.year(), date.month(), date.day(),
                        time.hour(), time.minute(), time.second());
  _computeArchiveStopTime();
}

////////////////////////////////////////////////////////
// set gui widget from start time

void PolarManager::_setGuiFromStartTime()
{
  QDate date(_archiveStartTime.getYear(), 
             _archiveStartTime.getMonth(),
             _archiveStartTime.getDay());
  QTime time(_archiveStartTime.getHour(),
             _archiveStartTime.getMin(),
             _archiveStartTime.getSec());
  QDateTime datetime(date, time);
  _archiveStartTimeEdit->setDateTime(datetime);
}

////////////////////////////////////////////////////////
// set start time to defaults

void PolarManager::_setArchiveStartTimeToDefault()

{

  _archiveStartTime.set(_params.archive_start_time);
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromStartTime();
  _computeArchiveStopTime();

}

////////////////////////////////////////////////////////
// set start time

void PolarManager::_setArchiveStartTime(const RadxTime &rtime)

{

  if (rtime.utime() == 0) {
    return;
  }

  _archiveStartTime = rtime;
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromStartTime();
  _computeArchiveStopTime();

}

////////////////////////////////////////////////////////
// set end time from start time, nscans and interval

void PolarManager::_computeArchiveStopTime()

{

  _archiveStopTime =
    _archiveStartTime + _archiveScanIntervalSecs * _nArchiveScans;
  if (_archiveStopTimeEcho) {
    _archiveStopTimeEcho->setText(_archiveStopTime.asString(0).c_str());
  }

}

/////////////////////////////////////////////////////
// creating image files in realtime mode

void PolarManager::_createRealtimeImageFiles()
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

    _archiveStopTime = _imagesScheduledTime - delay;
    _archiveStartTime = _archiveStopTime - _archiveScanIntervalSecs;
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

void PolarManager::_createArchiveImageFiles()
{

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_THEN_EXIT) {
    
    if (_inputFileList.size() > 0) {

      // using input file list to drive image generation

      while (_inputFileList.size() > 0) {
        _createImageFilesAllSweeps();
        _inputFileList.erase(_inputFileList.begin(), 
                             _inputFileList.begin() + 1);
      }

    } else {
      
      // using archive time to drive image generation

      while (_archiveStartTime <= _imagesArchiveEndTime) {
        _createImageFilesAllSweeps();
        _archiveStartTime += _params.archive_scan_interval_secs;
      }

    }

  } else if (_params.images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {

    for (RadxTime stime = _imagesArchiveStartTime;
         stime <= _imagesArchiveEndTime;
         stime += _params.images_schedule_interval_secs) {
      
      _archiveStartTime = stime;
      _archiveStopTime = _archiveStartTime + _archiveScanIntervalSecs;

      _createImageFilesAllSweeps();
      
    } // stime

  } // if (_params.images_creation_mode ...

}

/////////////////////////////////////////////////////
// creating one image per field, for each sweep

void PolarManager::_createImageFilesAllSweeps()
{

  for (size_t ii = 0; ii < _vol.getNSweeps(); ii++) {
    _sweepIndex = ii;
    _createImageFiles();
  }

}

/////////////////////////////////////////////////////
// creating one image per field

void PolarManager::_createImageFiles()
{

  if (_params.debug) {
    cerr << "PolarManager::_createImageFiles()" << endl;
  }

  PMU_auto_register("createImageFiles");

  // plot the data

  _ppi->setStartOfSweep(true);
  _rhi->setStartOfSweep(true);
  _plotArchiveData();

  // set times from plots

  if (_rhiMode) {
    _plotStartTime = _rhi->getPlotStartTime();
    _plotEndTime = _rhi->getPlotEndTime();
  } else {
    _plotStartTime = _ppi->getPlotStartTime();
    _plotEndTime = _ppi->getPlotEndTime();
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

/////////////////////////////////////////////////////
// howto help

void PolarManager::_howto()
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

////////////////////////////////////////////////////////
// change start time

void PolarManager::_goBack1()
{
  _archiveStartTime -= 1 * _archiveScanIntervalSecs;
  _setGuiFromStartTime();
}

void PolarManager::_goBackNScans()
{
  _archiveStartTime -= _nArchiveScans * _archiveScanIntervalSecs;
  _setGuiFromStartTime();
}

void PolarManager::_goFwd1()
{
  _archiveStartTime += 1 * _archiveScanIntervalSecs;
  _setGuiFromStartTime();
}

void PolarManager::_goFwdNScans()
{
  _archiveStartTime += _nArchiveScans * _archiveScanIntervalSecs;
  _setGuiFromStartTime();
}

////////////////////////////////////////////////////////
// set for pending archive retrieval

void PolarManager::_setArchiveRetrievalPending()
{
  _archiveRetrievalPending = true;
  // if files were specified on command line, clear them
  _inputFileList.clear();
}

/////////////////////////////////////
// clear display widgets

void PolarManager::_clear()
{
  if (_ppi) {
    _ppi->clear();
  }
  if (_rhi) {
    _rhi->clear();
  }
}

/////////////////////////////////////
// set archive mode

void PolarManager::_setArchiveMode(bool state)
{
  _archiveMode = state;
  if (_archiveTimeBox) {
    _archiveTimeBox->setEnabled(state);
  }
  if (_ppi) {
    _ppi->setArchiveMode(state);
  }
  if (_rhi) {
    _rhi->setArchiveMode(state);
  }
}

/////////////////////////////////////
// activate realtime rendering

void PolarManager::_activateRealtimeRendering()
{
  if (_ppi) {
    _ppi->activateRealtimeRendering();
  }
  if (_rhi) {
    _rhi->activateRealtimeRendering();
  }
}

/////////////////////////////////////
// activate archive rendering

void PolarManager::_activateArchiveRendering()
{
  if (_ppi) {
    _ppi->activateArchiveRendering();
  }
  if (_rhi) {
    _rhi->activateArchiveRendering();
  }
}

/////////////////////////////////////////////////////
// save image to file
// If interactive is true, use dialog boxes to indicate errors or report
// where the image was saved.

void PolarManager::_saveImageToFile(bool interactive)
{

  // set times from plots

  if (_rhiMode) {
    _plotStartTime = _rhi->getPlotStartTime();
    _plotEndTime = _rhi->getPlotEndTime();
  } else {
    _plotStartTime = _ppi->getPlotStartTime();
    _plotEndTime = _ppi->getPlotEndTime();
  }

  // create image
  
  QPixmap pixmap;
  if (_rhiMode) {
    pixmap = QPixmap::grabWidget(_rhi);
  } else {
    pixmap = QPixmap::grabWidget(_ppi);
  }
  QImage image = pixmap.toImage();
  
  // compute output dir
  
  string outputDir(_params.images_output_dir);
  char dayStr[1024];
  if (_params.images_write_to_day_dir) {
    sprintf(dayStr, "%.4d%.2d%.2d",
            _plotStartTime.getYear(),
            _plotStartTime.getMonth(),
            _plotStartTime.getDay());
    outputDir += PATH_DELIM;
    outputDir += dayStr;
  }
  
  // make sure output dir exists

  if (ta_makedir_recurse(outputDir.c_str())) {
    string errmsg("Cannot create output dir: " + outputDir);
    cerr << "ERROR - PolarManager::_saveImageToFile()" << endl;
    cerr << "  " << errmsg << endl;
    if (interactive) {
        QMessageBox::critical(this, "Error", errmsg.c_str());
    }
    return;
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

  // extension

  fileName += ".";
  fileName += _params.images_file_name_extension;

  // compute output path

  string outputPath(outputDir);
  outputPath += PATH_DELIM;
  outputPath += fileName;

  // write the file
  
  if (!image.save(outputPath.c_str())) {
    string errmsg("Cannot save image to file: " + outputPath);
    cerr << "ERROR - PolarManager::_saveImageToFile()" << endl;
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
    Path::stripDir(_params.images_output_dir, outputPath, relPath);
    
    if(_params.debug) {
      ldataInfo.setDebug();
    }
    ldataInfo.setLatestTime(_plotStartTime.utime());
    ldataInfo.setWriter("HawkEye");
    ldataInfo.setDataFileExt(_params.images_file_name_extension);
    ldataInfo.setDataType(_params.images_file_name_extension);
    ldataInfo.setRelDataPath(relPath);
    
    if(ldataInfo.write(_plotStartTime.utime())) {
      cerr << "ERROR - PolarManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.images_write_latest_data_info)

}

