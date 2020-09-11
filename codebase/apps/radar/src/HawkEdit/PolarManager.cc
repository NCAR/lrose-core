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
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"
#include "BoundaryPointEditor.hh"

#include <string>
#include <cmath>
#include <iostream>
#include <H5Cpp.h>
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
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>

using namespace std;
using namespace H5;

PolarManager* PolarManager::m_pInstance = NULL;

PolarManager* PolarManager::Instance()
{
   return m_pInstance;
}

// Constructor

/*
PolarManager::PolarManager(const Params &params,
                           Reader *reader,
                           const vector<DisplayField *> &fields,
                           bool haveFilteredFields) :
        DisplayManager(params, reader, fields, haveFilteredFields), _sweepManager(params), _rhiWindowDisplayed(false)
{
*/

PolarManager::PolarManager(const Params &params,
                           Reader *reader,
			   DisplayFieldController *displayFieldController,
                           bool haveFilteredFields) :
  DisplayManager(params, reader, displayFieldController, haveFilteredFields), _sweepManager(params), _rhiWindowDisplayed(false)
{

	m_pInstance = this;

  // initialize

  _firstTime = true;

  // setWindowIcon(QIcon("HawkEyePolarIcon.icns"));
  
  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  _ppiRays = NULL;
  _rhiMode = false;

  _nGates = 1000;
  _maxRangeKm = _params.max_range_km;
  
  _archiveRetrievalPending = false;
  
  _ppiFrame = NULL;
  _ppi = NULL;

  _rhiWindow = NULL;
  _rhi = NULL;

  _sweepVBoxLayout = NULL;
  _sweepPanel = NULL;

  _archiveStartTimeEdit = NULL;
  _archiveEndTimeEdit = NULL;

  _selectedTimeLabel = NULL;
  
  _back1 = NULL;
  _fwd1 = NULL;
  _backPeriod = NULL;
  _fwdPeriod = NULL;

  _timeControl = NULL;
  _timeControlPlaced = false;
  _timeLayout = NULL;
  _timeSlider = NULL;

  _setArchiveMode(_params.begin_in_archive_mode);
  _archiveStartTime.set(_params.archive_start_time);
  _archiveEndTime = _archiveStartTime + _params.archive_time_span_secs;
  _archiveScanIndex = 0;

  _imagesArchiveStartTime.set(_params.images_archive_start_time);
  _imagesArchiveEndTime.set(_params.images_archive_end_time);
  _imagesScanIntervalSecs = _params.images_scan_interval_secs;

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
  
    if ((_archiveMode) && (_urlOK)) {
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
  char keychar = e->text().toLatin1().data()[0];
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
  size_t nFields = _displayFieldController->getNFields();
  for (size_t ifield = 0; ifield < nFields; ifield++) {
    
    const DisplayField *field = _displayFieldController->getField(ifield);

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
    _ppi->setStartOfSweep(true);
    _rhi->setStartOfSweep(true);
    _goBack1();

  } else if (key == Qt::Key_Right) {

    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _ppi->setStartOfSweep(true);
    _rhi->setStartOfSweep(true);
    _goFwd1();
    
  } else if (key == Qt::Key_Up) {

    if (_sweepManager.getGuiIndex() > 0) {

      if (_params.debug) {
        cerr << "Clicked up arrow, go up a sweep" << endl;
      }
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);
      _changeSweepRadioButton(-1);

    }

  } else if (key == Qt::Key_Down) {

    if (_sweepManager.getGuiIndex() < (int) _sweepManager.getNSweeps() - 1) {

      if (_params.debug) {
        cerr << "Clicked down arrow, go down a sweep" << endl;
      }
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);
      _changeSweepRadioButton(+1);

    }

  }

}

void PolarManager::_moveUpDown() 
{
  this->setCursor(Qt::WaitCursor);
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
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
  QHBoxLayout *mainLayout = new QHBoxLayout;
  _main->setLayout(mainLayout);
  mainLayout->setSpacing(5);
  mainLayout->setContentsMargins(3,3,3,3);
  setCentralWidget(_main);

  // ppi window

  _ppiFrame = new QFrame(_main);
  _ppiFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the PPI

  _ppi = new PpiWidget(_ppiFrame, *this, _params, _platform, _displayFieldController, _haveFilteredFields);

  connect(this, SIGNAL(frameResized(const int, const int)),
	  _ppi, SLOT(resize(const int, const int)));
  
  // Create the RHI window

  _rhiWindow = new RhiWindow(this, _params, _platform,
                             _displayFieldController, _haveFilteredFields);
  _rhiWindow->setRadarName(_params.radar_name);

  // set pointer to the rhiWidget

  _rhi = _rhiWindow->getWidget();
  
  // connect slots for location

  connect(_ppi, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_ppiLocationClicked(double, double, const RadxRay*)));
  connect(_rhi, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_rhiLocationClicked(double, double, const RadxRay*)));

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);
  // customContextMenuRequested(e->pos());
  connect(_ppi, SIGNAL(customContextMenuRequested(const QPoint &)),
	  this, SLOT(ShowContextMenu(const QPoint &)));

  // create status panel

  _createStatusPanel();

  // create fields panel
  
  _createFieldPanel();

  // add widgets

  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_fieldPanel);
  mainLayout->addWidget(_ppiFrame);

  // sweep panel

   _createSweepPanel();
   mainLayout->addWidget(_sweepPanel);

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
  _createBoundaryEditorDialog();

  if (_archiveMode) {
    _showTimeControl();
  }
   _setSweepPanelVisibility();

}

//////////////////////////////
// add/remove  sweep panel (archive mode only)

void PolarManager::_setSweepPanelVisibility()
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

  // show boundary editor dialog
  _showBoundaryEditorAct = new QAction(tr("Boundary Editor"), this);
  _showBoundaryEditorAct->setStatusTip(tr("Show boundary editor dialog"));
  connect(_showBoundaryEditorAct, SIGNAL(triggered()), this, SLOT(_showBoundaryEditor()));
  
  // set time controller settings
  _timeControllerAct = new QAction(tr("Time-Config"), this);
  _timeControllerAct->setStatusTip(tr("Show time control window"));
  connect(_timeControllerAct, SIGNAL(triggered()), this,
          SLOT(_showTimeControl()));

  // show time control window
  _showTimeControlAct = new QAction(tr("Show time control window"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, SIGNAL(triggered()), _timeControl,
          SLOT(show()));

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
  connect(_clearAct, SIGNAL(triggered()), _ppi, SLOT(clear()));
  connect(_clearAct, SIGNAL(triggered()), _rhi, SLOT(clear()));

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
  _fileMenu->addAction(_openFileAct);
  _fileMenu->addAction(_saveFileAct);
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  _timeMenu = menuBar()->addMenu(tr("&Time-control"));
  _timeMenu->addAction(_showTimeControlAct);
  _timeMenu->addSeparator();
  _timeMenu->addAction(_realtimeAct);

  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _overlaysMenu->addAction(_ringsAct);
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_azLinesAct);
  _overlaysMenu->addSeparator();
  _overlaysMenu->addAction(_showRhiAct);

  menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showClickAct);
  menuBar()->addAction(_showBoundaryEditorAct);
  menuBar()->addAction(_unzoomAct);
  menuBar()->addAction(_clearAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

/////////////////////////////////////////////////////////////
// create the sweep panel
// buttons will be filled in by createSweepRadioButtons()

void PolarManager::_createSweepPanel()
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

void PolarManager::_createSweepRadioButtons() 
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
  
  char buf[256];
  _sweepRButtons = new vector<QRadioButton *>();

  for (int ielev = 0; ielev < (int) _sweepManager.getNSweeps(); ielev++) {
    
    std::snprintf(buf, 256, "%.2f", _sweepManager.getFixedAngleDeg(ielev));
    QRadioButton *radio1 = new QRadioButton(buf); 
    radio1->setFont(fontm2);
    
    if (ielev == _sweepManager.getGuiIndex()) {
      radio1->setChecked(true);
    }
    
    _sweepRButtons->push_back(radio1);
    _sweepVBoxLayout->addWidget(radio1);
    
    // connect slot for sweep change

    connect(radio1, SIGNAL(toggled(bool)), this, SLOT(_changeSweep(bool)));

  }

}

/////////////////////////////////////////////////////////////////////
// create sweep panel of radio buttons

void PolarManager::_clearSweepRadioButtons() 
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

void PolarManager::_changeSweep(bool value) {

  LOG(DEBUG) << "From PolarManager: the sweep was changed ";

  if (!value) {
    return;
  }

  for (size_t ii = 0; ii < _sweepRButtons->size(); ii++) {
    if (_sweepRButtons->at(ii)->isChecked()) {
      LOG(DEBUG) << "sweepRButton " << ii << " is checked; moving to sweep index " << ii;
      _sweepManager.setGuiIndex(ii);
      _ppi->setStartOfSweep(true);
      _rhi->setStartOfSweep(true);
      _moveUpDown();
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

void PolarManager::_changeSweepRadioButton(int increment)

{
  
  if (_params.debug) {
    cerr << "-->> changing sweep index by increment: " << increment << endl;
  }
  
  if (increment != 0) {
    _sweepManager.changeSelectedIndex(increment);
    _sweepRButtons->at(_sweepManager.getGuiIndex())->setChecked(true);
  }

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
// set input file list for archive mode

void PolarManager::setArchiveFileList(const vector<string> &list,
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

int PolarManager::loadArchiveFileList()

{
  RadxTimeList timeList;
  timeList.setDir(_params.archive_data_url);
  timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  timeList.compile();
  _urlOK = true;

  if (timeList.getPathList().size() < 1) {
    cerr << "ERROR - PolarManager::loadArchiveFileList()" << endl;
    cerr << "  Cannot load file list for url: " 
         << _params.archive_data_url << endl;
    cerr << "  Start time: " << _archiveStartTime.getStr() << endl;
    cerr << "  End time: " << _archiveEndTime.getStr() << endl;
    _urlOK = false;
    return -1;

    /*
    if (_archiveFileList.size() > 0) {
      cerr << "Trying previously opened file ..." << endl;
      // try to read from previous Open file chooser
      try {
        _getArchiveData();
      } catch (FileIException ex) {
        cerr << "   failed. " << endl;
        return -1;
      }
    } else {
      return -1;
    }
    */
  }

  setArchiveFileList(timeList.getPathList(), false);
  
  return 0;

}

///////////////////////////////////////
// handle data in archive mode

void PolarManager::_handleArchiveData(QTimerEvent * event)

{
  
  if (_params.debug) {
    cerr << "handling archive data ..." << endl;
  }

  _ppi->setArchiveMode(true);
  _ppi->setStartOfSweep(true);

  _rhi->setArchiveMode(true);
  _rhi->setStartOfSweep(true);

  // set cursor to wait cursor

  this->setCursor(Qt::WaitCursor);
  _timeControl->setCursor(Qt::WaitCursor);
  
  // get data
  try {
    _getArchiveData();
  } catch (FileIException ex) {
    this->setCursor(Qt::ArrowCursor);
    _timeControl->setCursor(Qt::ArrowCursor);
    return;
  }

  // set up sweep GUI

  _clearSweepRadioButtons();
  _createSweepRadioButtons();
  
  if (_vol.checkIsRhi()) {
    _rhiMode = true;
  } else {
    _rhiMode = false;
  }

  // plot the data
  
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
  _timeControl->setCursor(Qt::ArrowCursor);

  _activateArchiveRendering();

  if (_firstTime) {
    _firstTime = false;
  }
  
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
  
  if (_archiveScanIndex >= 0 &&
      _archiveScanIndex < (int) _archiveFileList.size()) {
    
    string inputPath = _archiveFileList[_archiveScanIndex];
    
    if(_params.debug) {
      cerr << "  reading data file path: " << inputPath << endl;
      cerr << "  archive file index: " << _archiveScanIndex << endl;
    }
    
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

  // compute the fixed angles from the rays
  // so that we reflect reality
  
  _vol.computeFixedAnglesFromRays();

  // load the sweep manager
  
  _sweepManager.set(_vol);

  if (_params.debug) {
    cerr << "----------------------------------------------------" << endl;
    cerr << "perform archive retrieval" << endl;
    cerr << "  read file: " << _vol.getPathInUse() << endl;
    cerr << "  nSweeps: " << _vol.getNSweeps() << endl;
    cerr << "  guiIndex, fixedAngle: " 
         << _sweepManager.getGuiIndex() << ", "
         << _sweepManager.getSelectedAngle() << endl;
    cerr << "----------------------------------------------------" << endl;
  }
  
   _platform = _vol.getPlatform();
   
  return 0;

}

double PolarManager::getSelectedSweepAngle() {
  double angle = _sweepManager.getSelectedAngle();
  return angle;
}

size_t PolarManager::getSelectedFieldIndex() {
  return _displayFieldController->getSelectedFieldNum();
}

/////////////////////////////
// apply new, edited  data in archive mode
// the volume has been updated 

void PolarManager::_applyDataEdits()
{

  if (0) { // _params.debug) {
    std::ofstream outfile("/tmp/voldebug_PolarManager_applyDataEdits.txt");
    _vol.printWithFieldData(outfile);  
    outfile << "_vol = " << &_vol << endl;
  }

  _plotArchiveData();
}


/////////////////////////////////////
// activate archive rendering

void PolarManager::_addNewFields(QStringList  newFieldNames)
{
  LOG(DEBUG) << "enter";
  LOG(DEBUG) << "all fields in _vol ... ";
  vector<RadxField *> allFields = _vol.getFields();
  vector<RadxField *>::iterator it;
  for (it = allFields.begin(); it != allFields.end(); it++) {
    RadxField *radxField = *it;
    LOG(DEBUG) << radxField->getName();
  }


  // TODO: 
  // inheret the color map, units, etc. from the similar field
  // Do this in the ScriptEditor, when the new field is added to RadxVol

  // make new DisplayFields for PpiWidget
  // -----

  vector<DisplayField *> newFields;

  LOG(DEBUG) << "newFieldNames ...";
  //  newFieldNames.split(',');
  for (int i=0; i < newFieldNames.size(); ++i) {
    string name = newFieldNames.at(i).toLocal8Bit().constData();

    LOG(DEBUG) << name;
 
    ColorMap map(-20.0, 20.0, "default");

     // new DisplayField(pfld.label, pfld.raw_name, pfld.units,
      //		       pfld.shortcut, map, ifield, false);
    int buttonRow = _displayFieldController->getNFields() + 1;
    DisplayField *field =
      new DisplayField(name, name, "m/s",
		       "-1", map, buttonRow, false);
    //if (noColorMap)
    field->setNoColorMap();

    newFields.push_back(field);
    _displayFieldController->addField(field);

    // filtered field                                                                                    
    
    /*
      if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
      new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut,
      map, ifield, true);
      newFields.push_back(filt);
      }
    */
    // -----------
  }   
  
  if (_ppi) {
    _ppi->addNewFields(newFields);
  }
  
  if (_rhi) {
    _rhi->addNewFields(newFields);
  }
  LOG(DEBUG) << "exit";
}

// like handleRay
// here's what's available ...
//string fieldName, ColorMap newColorMap,
// QColor gridColor,
//	QColor emphasisColor,
//	QColor annotationColor,
//	QColor backgroundColor) {
//
  //

// We need to resize the arrays that are retained and look up the field Index by field name,
// because we are only redrawing the new fields and these stores have a field index
// dependence:  DisplayFieldModel::_fields, FieldRenderers::_fieldRenderers, Beams::_brushes and buttonRow
// Beams are dynamic and we will just create and delete them with the full dimension
// add new fields to existing ray structures
// NOTE: preconditions ... displayFieldController must contain new Fields
//void PolarManager::_handleRayUpdate(RadxPlatform &platform, RadxRay *ray, vector<string> &newFieldNames)
//{
/*
void PolarManager::_updateField(RadxPlatform &platform, RadxRay *ray, vector<string> &newFieldNames)
{

  // not sure this is needed ???
  LOG(DEBUG) << "enter";
  if (_ppi) {
    _ppi->colorMapChanged(fieldId);
  }
  
  if (_rhi) {
    _rhi->colorMapChanged(fieldId);
  }
  //
  //----


  // create 2D field data vector
  size_t nNewFields = newFieldNames.size();
  vector< vector<double> > fieldData;
  fieldData.resize(nNewFields);
  LOG(DEBUG) << " there are " << nNewFields << " new Fields";
  LOG(DEBUG) << " ray azimuth = " << ray->getAzimuthDeg();
  size_t ifield = 0; 

  //for (int ifield=0; ifield < newFieldNames.size(); ++ifield) {
  string fieldName = newFieldNames.at(ifield); // .toLocal8Bit().constData();
  // vector<double> &data = fieldData[ifield];
  //  data.resize(_nGates);
    RadxField *rfld = ray->getField(fieldName);

    // at this point, we know the data values for the field AND the color map                                                                        
    ColorMap *fieldColorMap = _displayFieldController->getColorMap(fieldName); 
    bool haveColorMap = fieldColorMap != NULL;

    if (rfld == NULL) {
      // fill with missing
      for (int igate = 0; igate < _nGates; igate++) {
        data[igate] = -9999.0;
      }
    } else {
      rfld->convertToFl32();
      const Radx::fl32 *fdata = rfld->getDataFl32();
      // print first 15 data values
      //LOG(DEBUG) << "ray->nGates = " << ray->getNGates();
      //LOG(DEBUG) << "first 30 gates ...";
      //for (int ii = 0; ii< 15; ii++)
      //LOG(DEBUG) << fdata[ii];
      // end print first 15 data values
      const Radx::fl32 missingVal = rfld->getMissingFl32();
      // we can only look at the data available, so only go to nGates
      for (int igate = 0; igate < _nGates; igate++, fdata++) {  // was _nGates
        Radx::fl32 val = *fdata;
        if (fabs(val - missingVal) < 0.0001) {
          data[igate] = -9999.0;
        } else {
          data[igate] = val;
        
        } // end else not missing value
      } // end for each gate

    } // end else vector not NULL
  } // end for each field

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the PPI or RHI, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {

    _rhiMode = true;

    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      _rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }

    // Add the beam to the display
    LOG(DEBUG) << "RHI not being updated";
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
    // ray contains data for ALL fields; fieldData contains only data for the new beams
    // nFields = total number of fields (old + new)
    size_t nFields = _displayFieldController->getNFields();
    vector<string> newFieldNames;
    //newFieldNames.push(fieldName);
    _ppi->updateBeamII(ray, _startAz, _endAz, fieldData, nFields, newFieldNames);

  }


//--


  LOG(DEBUG) << "exit";
}
*/
   

/////////////////////////////
// add results of scripts to display; new, edited  data in archive mode
// the volume has been updated 

// The new field names have been added to the RadxVol,
// pull them out and create DisplayFields for them???
// 
void PolarManager::_volumeDataChanged(QStringList newFieldNames)
{
  LOG(DEBUG) << "enter";
  // the in memory RadxVol has changed; 
  // new fields have been added or deleted

  /* NOT THIS ...
  //  load the sweep manager
    _sweepManager.set(_vol);
  */

  //TODO: need to get a list of new Fields, somehow ...
  // could be as a list of DisplayFields? 

  //vector<DisplayField *> newFields;
  

  _addNewFields(newFieldNames);
  //if (newFieldNames.size() > 0)
  //   _updateFieldPanel(newFieldNames[0].toStdString());
  _fieldPanel->update();

  // _applyDataEdits();
  //_activateArchiveRendering();
  // _plotArchiveData();
  // TODO: create this ... from plotArchiveData()
  _updateArchiveData(newFieldNames); 
  _activateArchiveRendering();

  LOG(DEBUG) << "exit"; 
}

/*
RadxVol PolarManager::getDataVolume() {
  return _vol;
}
*/
/////////////////////////////
// plot data in archive mode

void PolarManager::_plotArchiveData()

{

  if(_params.debug) {
    cerr << "Plotting archive data" << endl;
    cerr << "  volume start time: " << _plotStartTime.asString() << endl;
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

  // clear the canvas

  _clear();

  // handle the rays

  const SweepManager::GuiSweep &gsweep = _sweepManager.getSelectedSweep();
  for (size_t ii = gsweep.radx->getStartRayIndex();
       ii <= gsweep.radx->getEndRayIndex(); ii++) {
    RadxRay *ray = rays[ii];
    _handleRay(_platform, ray);
    if (ii == 0) {
      _updateStatusPanel(ray);
    }
  }

  
}

void PolarManager::_updateArchiveData(QStringList newFieldNames)
{

  vector<string> newFieldNamesConverted;
  for (int i=0; i < newFieldNames.size(); ++i) {
    string name = newFieldNames.at(i).toLocal8Bit().constData();
    newFieldNamesConverted.push_back(name);
  }
  _updateArchiveData(newFieldNamesConverted);
 
}

void PolarManager::_updateArchiveData(vector<string> &newFieldNamesConverted) 
{

  if(_params.debug) {
    cerr << "Updating archive data" << endl;
    cerr << "  volume start time: " << _plotStartTime.asString() << endl;
  }

  // initialize plotting
  //_initialRay = true;

  // handle the rays
  _vol.loadRaysFromFields();  // this line makes the select field update properly  

  const vector<RadxRay *> &rays = _vol.getRays();
  if (rays.size() < 1) {
    cerr << "ERROR - _updateArchiveData" << endl;
    cerr << "  No rays found" << endl;
    return;
  }

  // TODO: make sure we are getting the newFields from the _vol
  // remember, the reader only reads in the fields specified in the params <======
  LOG(DEBUG) << "===========  HERE are the fields ";
  vector<RadxField *> _vol_fields = rays[0]->getFields();
  vector<RadxField *>::iterator it;
  for (it=_vol_fields.begin(); it!=_vol_fields.end(); ++it) {
    LOG(DEBUG) << (*it)->getName();
  }
  
  // TODO: reload the sweeps into the sweepManager?
  //  I added this ...
  _sweepManager.set(_vol);

  //const vector<RadxSweep *> &sweeps = _vol.getSweeps();
  //if (sweeps.size() < 1) {
  //  cerr << "ERROR - _plotArchiveData" << endl;
  //  cerr << "  No sweeps found" << endl;
  //  return;
  //}

  // clear the canvas
  //_clear();

  /*
  vector<string> newFieldNamesConverted;
  for (int i=0; i < newFieldNames.size(); ++i) {
    string name = newFieldNames.at(i).toLocal8Bit().constData();
    newFieldNamesConverted.push_back(name);
  }
  */

  // handle the rays

  const SweepManager::GuiSweep &gsweep = _sweepManager.getSelectedSweep();
  for (size_t ii = gsweep.radx->getStartRayIndex();
       ii <= gsweep.radx->getEndRayIndex(); ii++) {
    RadxRay *ray = rays[ii];
    _handleRayUpdate(_platform, ray, newFieldNamesConverted);  // TODO; _handleRay? or a modified version of it?  We just need something to call updateBeam, instead of addBeam ...
    //if (ii == 0) {
    //  _updateStatusPanel(ray);
    //}
  }
  
}

void PolarManager::_updateColorMap(string fieldName) 
{

  if(_params.debug) {
    cerr << "Updating color map" << endl;
  }
  /*
  // handle the rays
  _vol.loadRaysFromFields();  // this line makes the select field update properly  

  const vector<RadxRay *> &rays = _vol.getRays();
  if (rays.size() < 1) {
    cerr << "ERROR - _updateArchiveData" << endl;
    cerr << "  No rays found" << endl;
    return;
  }

  // TODO: make sure we are getting the newFields from the _vol
  // remember, the reader only reads in the fields specified in the params <======
  //LOG(DEBUG) << "===========  HERE are the fields ";
  vector<RadxField *> _vol_fields = rays[0]->getFields();
  vector<RadxField *>::iterator it;
  //for (it=_vol_fields.begin(); it!=_vol_fields.end(); ++it) {
  //  LOG(DEBUG) << (*it)->getName();
  //}
  
  // TODO: reload the sweeps into the sweepManager?
  //  I added this ...
  _sweepManager.set(_vol);

  // handle the rays

  const SweepManager::GuiSweep &gsweep = _sweepManager.getSelectedSweep();
  //  for (size_t ii = gsweep.radx->getStartRayIndex();
  //       ii <= gsweep.radx->getEndRayIndex(); ii++) {
  //    RadxRay *ray = rays[ii];
  */
    _handleColorMapChangeOnRay(_platform, fieldName); 
    //}
  
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

  vector<string> fieldNames = _displayFieldController->getFieldNames();
  vector<string>::iterator it;
  for (it = fieldNames.begin(); it != fieldNames.end(); it++) {
    file.addReadField(*it);
  }
  //  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
  //    const DisplayField *field = _fields[ifield];
  //    file.addReadField(field->getName());
  //  }

}

//////////////////////////////////////////////////////////////
// handle an incoming ray

void PolarManager::_handleRay(RadxPlatform &platform, RadxRay *ray)
  
{

  // do we need to reconfigure the PPI?

  _nGates = ray->getNGates();
  double maxRange = ray->getStartRangeKm() + _nGates * ray->getGateSpacingKm();
  
  if ((maxRange - _maxRangeKm) > 0.001) {
    _maxRangeKm = maxRange;
    _ppi->configureRange(_maxRangeKm);
    _rhi->configureRange(_maxRangeKm);
  }

  // create 2D field data vector
  size_t nFields = _displayFieldController->getNFields();
  vector< vector<double> > fieldData;
  fieldData.resize(nFields);
  
  // fill data vector
  vector<string> fieldNames = _displayFieldController->getFieldNames();
  vector<string>::iterator ifieldName;
  size_t ifield = 0;
  for (ifieldName = fieldNames.begin(); ifieldName != fieldNames.end(); ++ifieldName) {
    //for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {
    LOG(DEBUG) << "filling data for field " << *ifieldName;
    vector<double> &data = fieldData[ifield];
    data.resize(_nGates);
    string alias = _displayFieldController->getFieldAlias(*ifieldName);
    RadxField *rfld = ray->getField(alias); // *ifieldName);
    if (rfld == NULL) LOG(DEBUG) <<  "No field found for DisplayField name";

    //    RadxField *rfld = ray->getField(_fields[ifield]->getName());

    // at this point, we know the data values for the field AND the color map                                                                        
    ColorMap *fieldColorMap = _displayFieldController->getColorMap(*ifieldName); 
    bool haveColorMap = fieldColorMap != NULL;
    //    bool haveColorMap = _fields[ifield]->haveColorMap();
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
        
	  //=======
	    //data.push_back(*fdata);  // ==> We know the data value here; determine min and max of values
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
	// just change bounds on existing map        
        _displayFieldController->setColorMapMinMax(*ifieldName, min, max);
      } // end do not have color map

    } // end else vector not NULL
    ifield++;
  } // end for each field

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the PPI or RHI, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {

    _rhiMode = true;

    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      _rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }

    // Add the beam to the display

    _rhi->addBeam(ray, fieldData, nFields); // _fields);
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
    
    _ppi->addCullTrackBeam(ray, _startAz, _endAz, fieldData, nFields);
    //    _ppi->addBeam(ray, _startAz, _endAz, fieldData, nFields);

  }
  
}

// We need to resize the arrays that are retained and look up the field Index by field name,
// because we are only redrawing the new fields and these stores have a field index
// dependence:  DisplayFieldModel::_fields, FieldRenderers::_fieldRenderers, Beams::_brushes and buttonRow
// Beams are dynamic and we will just create and delete them with the full dimension
// add new fields to existing ray structures
// NOTE: preconditions ... displayFieldController must contain new Fields
void PolarManager::_handleRayUpdate(RadxPlatform &platform, RadxRay *ray, vector<string> &newFieldNames)
{

  LOG(DEBUG) << "enter";
  // create 2D field data vector
  size_t nNewFields = newFieldNames.size();
  vector< vector<double> > fieldData;
  fieldData.resize(nNewFields);
  LOG(DEBUG) << " there are " << nNewFields << " new Fields";
  LOG(DEBUG) << " ray azimuth = " << ray->getAzimuthDeg();
  // fill data vector
  //vector<string> fieldNames = displayFieldController->getFieldNames();
  //vector<string>::iterator ifieldName;
  size_t ifield = 0; // TODO: this is off; we are looping over the NEW fields, not ALL fields !!!!
  // TODO: Wait! Instead, should we be looping over all the fields? because we need to
  // send new fieldData and it needs to be the entire 2D matrix of data? or just an update???
  // NO. For update, we can just send a 2D array of the necessary data. 
  //for (ifieldName = newFieldNames.begin(); ifieldName != newFieldNames.end(); ifieldName++) {
  for (int ifield=0; ifield < newFieldNames.size(); ++ifield) {
    string fieldName = newFieldNames.at(ifield); // .toLocal8Bit().constData();
    vector<double> &data = fieldData[ifield];
    data.resize(_nGates);
    RadxField *rfld = ray->getField(fieldName);
    //    RadxField *rfld = ray->getField(_fields[ifield]->getName());

    // at this point, we know the data values for the field AND the color map                                                                        
    ColorMap *fieldColorMap = _displayFieldController->getColorMap(fieldName); 
    bool haveColorMap = fieldColorMap != NULL;
    //    bool haveColorMap = _fields[ifield]->haveColorMap();
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
      // print first 15 data values
      LOG(DEBUG) << "ray->nGates = " << ray->getNGates();
      LOG(DEBUG) << "first 30 gates ...";
      for (int ii = 0; ii< 15; ii++)
	LOG(DEBUG) << fdata[ii];
      // end print first 15 data values
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
	      printf("field index %d, gate %d \t", ifield, igate);
	      printf("new min, max of data %g, %g\t", min,  max);
	      printf("missing value %g\t", missingVal);
	      printf("current value %g\n", val);
	    }
          }
        } // end else not missing value
      } // end for each gate

      if (!haveColorMap) {
	// just change bounds on existing map        
        _displayFieldController->setColorMapMinMax(fieldName, min, max);
      } // end do not have color map

    } // end else vector not NULL
    //    ifield++;
  } // end for each field

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the PPI or RHI, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {

    _rhiMode = true;

    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      _rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }

    // Add the beam to the display
    LOG(DEBUG) << "RHI not being updated";
    /* TODO: update the rhi code ...
    _rhi->addBeam(ray, fieldData, displayFieldController); // _fields);
    _rhiWindow->setAzimuth(ray->getAzimuthDeg());
    _rhiWindow->setElevation(ray->getElevationDeg());
    */
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
    // ray contains data for ALL fields; fieldData contains only data for the new beams
    // nFields = total number of fields (old + new)
    size_t nFields = _displayFieldController->getNFields();
    _ppi->updateBeamII(ray, _startAz, _endAz, fieldData, nFields, newFieldNames);

  }
  LOG(DEBUG) << "exit";
  
}

void PolarManager::_handleColorMapChangeOnRay(RadxPlatform &platform, 
					      string fieldName)
{

  LOG(DEBUG) << "enter";

  /*
  // create  field data vector
  size_t nNewFields = 1;
  vector<double> data;
  //fieldData.resize(nNewFields);
  LOG(DEBUG) << " there are " << nNewFields << " new Fields";
  LOG(DEBUG) << " ray azimuth = " << ray->getAzimuthDeg();
  // fill data vector
  size_t ifield = 0; 
  data.resize(_nGates);
  RadxField *rfld = ray->getField(fieldName);

  // at this point, we know the data values for the field AND the color map                                                                        
  ColorMap *fieldColorMap = _displayFieldController->getColorMap(fieldName); 
  bool haveColorMap = fieldColorMap != NULL;
  if (!haveColorMap) {
    // just change bounds on existing map        
    throw "No color map found"; 
  } // end do not have color map

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
      } // end else not missing value
    } // end for each gate
  } // end else vector not NULL

  // Store the ray location (which also sets _startAz and _endAz), then
  // draw beam on the PPI or RHI, as appropriate

  if (ray->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI ||
      ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {

    _rhiMode = true;

    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      _rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }

    // Add the beam to the display
    LOG(DEBUG) << "RHI not being updated";
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
    // ray contains data for ALL fields; fieldData contains only data for the new beams
    // nFields = total number of fields (old + new)
    */
    size_t nFields = _displayFieldController->getNFields();
    if (_nGates < 0) throw "Error, cannot convert _nGates < 0 to type size_t";
    _ppi->updateBeamColors(nFields, fieldName, (size_t) _nGates); //ray, _startAz, _endAz, data, nFields, fieldName);

    //}
  LOG(DEBUG) << "exit";
  
}


///////////////////////////////////////////////////////////
// store ray location

void PolarManager::_storeRayLoc(const RadxRay *ray, const double az,
                                const double beam_width, RayLoc *ray_loc)
{

  // Determine the extent of this ray

  if (_params.ppi_override_rendering_beam_width) {
    double half_angle = _params.ppi_rendering_beam_width / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else if (ray->getIsIndexed()) {
    double half_angle = ray->getAngleResDeg() / 2.0;
    _startAz = az - half_angle - 0.1;
    _endAz = az + half_angle + 0.1;
  } else {
    double max_half_angle = beam_width / 2.0;
    double prev_offset = max_half_angle;
    if (_prevAz >= 0.0) {
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
    
  int startIndex = (int) (_startAz * RayLoc::RAY_LOC_RES);
  int endIndex = (int) (_endAz * RayLoc::RAY_LOC_RES + 1);

  // Clear out any rays in the locations list that are overlapped by the
  // new ray
    
  _clearRayOverlap(startIndex, endIndex, ray_loc);

  // Set the locations associated with this ray

  for (int ii = startIndex; ii <= endIndex; ii++) {
    ray_loc[ii].ray = ray;
    ray_loc[ii].active = true;
    // ray_loc[ii].master = false;
    ray_loc[ii].startIndex = startIndex;
    ray_loc[ii].endIndex = endIndex;
  }

  // indicate which ray is the master
  // i.e. it is responsible for ray memory
    
  // int midIndex = (int) (az * RayLoc::RAY_LOC_RES);
  // ray_loc[midIndex].master = true;

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

	// if (ray_loc[j].master)
	//   ray_loc[start_index-1].master = true;
	
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	// ray_loc[j].master = false;
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
	// if (ray_loc[j].master)
	//   ray_loc[end_index+1].master = true;
	ray_loc[j].ray = NULL;
	ray_loc[j].active = false;
	// ray_loc[j].master = false;
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
  //  if (_params.debug) {
  //  cerr << "Changing to field id: " << fieldId << endl;
  //  _selectedField->print(cerr);
  //}

  // convert fieldId to field name (from the tool tip) because not all fields are displayed,
  // so we cannot rely on the id/index.
  QString fieldNameQt = _fieldButtons.at(fieldId)->toolTip();
  string fieldName = fieldNameQt.toStdString();
  LOG(DEBUG) << "fieldName is " << fieldName;

  size_t newSelectionNum = _displayFieldController->getFieldIndex(fieldName);

  // if we click the already-selected field, go back to previous field

  size_t fieldNum = _displayFieldController->getSelectedFieldNum();
  if (guiMode) {
    if (fieldNum == newSelectionNum && _prevFieldNum >= 0) {
      QRadioButton *button =
        (QRadioButton *) _fieldGroup->button(_prevFieldNum);
      button->click();
      return;
    }
  }

  _prevFieldNum = fieldNum;
  fieldNum = newSelectionNum;

  _ppi->selectVar(fieldNum);
  //_rhi->selectVar(fieldNum);  TODO: reinstate this 

  _displayFieldController->setSelectedField(fieldName);

  // _colorBar->setColorMap(&_fields[_fieldNum]->getColorMap());
  _selectedField = _displayFieldController->getField(fieldNum);

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

// PolarManager::colorMapRedefineReceived(string, ColorMap)
// TODO: need to add the background changed, etc. 
void PolarManager::colorMapRedefineReceived(string fieldName, ColorMap newColorMap,
					    QColor gridColor,
					    QColor emphasisColor,
					    QColor annotationColor,
					    QColor backgroundColor) {

  LOG(DEBUG) << "enter";
  // connect the new color map with the field                                                       
  try {
    //  This should save/perpetuate the color map in the DisplayField object
    _displayFieldController->saveColorMap(fieldName, &newColorMap);
    _updateFieldPanel(fieldName);
    //size_t fieldId = _displayFieldController->getFieldIndex(fieldName);
    //vector<string> fieldNames;
    //fieldNames.push_back(fieldName);
    //_updateArchiveData(fieldNames);  // ?? or  _updateField(fieldId);
    _updateColorMap(fieldName);
  } catch (std::invalid_argument ex) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << ex.what(); // "ERROR - field not found; no color map change";
    // TODO: show error message box
  }
  _ppi->backgroundColor(backgroundColor);
  _ppi->gridRingsColor(gridColor);
  _ppi->colorScaleLegend(); // TODO: may not need this??
  //_ppi->drawOverlays();

  // ??   _ppi->selectVar(fieldId);

  /*
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
    _ppi->backgroundColor(backgroundColor);
    _ppi->gridRingsColor(gridColor);
    _changeField(fieldId, false);
  }
  */
  LOG(DEBUG) << "exit";
}

void PolarManager::setVolume() { // const RadxVol &radarDataVolume) {

  LOG(DEBUG) << "enter";

  _applyDataEdits(); // radarDataVolume);

  LOG(DEBUG) << "exit";



}

// TODO: make this a SLOT to a SIGNAL from  ScriptEditor 

void PolarManager::updateVolume(QStringList newFieldNames) {

  LOG(DEBUG) << "enter";
  _volumeDataChanged(newFieldNames);

  vector<DisplayField *> newFields;


  LOG(DEBUG) << "exit";

}

///////////////////////////////////////////////////
// respond to a change in click location on the PPI

void PolarManager::_ppiLocationClicked(double xkm, double ykm, 
                                       const RadxRay *closestRay)

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
      // cerr << "      master = " << _ppiRayLoc[rayIndex].master << endl;
      cerr << "      startIndex = " << _ppiRayLoc[rayIndex].startIndex << endl;
      cerr << "      endIndex = " << _ppiRayLoc[rayIndex].endIndex << endl;
    }
    return;
  }

  _locationClicked(xkm, ykm, _ppiRayLoc, ray);

}

///////////////////////////////////////////////////
// respond to a change in click location on the RHI

void PolarManager::_rhiLocationClicked(double xkm, double ykm, 
                                       const RadxRay *closestRay)
  
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

  //**** testing ****
  //  QToolTip::showText(this->mapToGlobal(QPoint(xkm, ykm)), "cindy");
  //QToolTip::showText(mapToGlobal(QPoint(xkm, ykm)), "cindy");
  //QToolTip::showText(mapToGlobal(QPoint(xkm, ykm)), "louise");
  //QToolTip::showText(QPoint(xkm, ykm), "jay");
  //int xp = _ppi->_zoomWorld.getIxPixel(xkm);
  //int yp = _ppi->_zoomWorld.getIyPixel(ykm);
  //QToolTip::showText(_ppi->mapToGlobal(QPoint(xp, yp)), "louigi");

  //_ppi->smartBrush(xkm, ykm);
  //qImage->convertToFormat(QImage::Format_RGB32);
  //qImage->invertPixels()
  // ****** end testing *****

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

  _displayFieldController->setForLocationClicked(-9999.0, "----");  
  //for (size_t ii = 0; ii < nFields; ii++) {
  //  _fields[ii]->setSelectValue(-9999.0);
  //  _fields[ii]->setDialogText("----");
  //}
  
  vector<RadxField *> flds = ray->getFields();
  for (size_t ifield = 0; ifield < flds.size(); ifield++) {
    const RadxField *field = flds[ifield];

    //  for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
    //const RadxField *field = ray->getField(ifield);
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
    //for (size_t ii = 0; ii < _fields.size(); ii++) {
    //  if (_fields[ii]->getName() == fieldName) {
    //_fields[ii]->setSelectValue(val);
        char text[128];
        if (fabs(val) > 10000) {
          sprintf(text, "----");
        } else if (fabs(val) > 10) {
          sprintf(text, "%.2f", val);
        } else {
          sprintf(text, "%.3f", val);
        }
	//  _fields[ii]->setDialogText(text);
	//  }
	//}
    _displayFieldController->setForLocationClicked(fieldName, val, text);

  } // ifield

  // update the status panel
  
  _updateStatusPanel(ray);
    
}

//////////////////////////////////////////////
// create the time panel

void PolarManager::_createTimeControl()
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

void PolarManager::_showTimeControl()
{

  if (_timeControl) {
    if (_timeControl->isVisible()) {
      _timeControl->setVisible(false);
    } else {
      if (!_timeControlPlaced) {
        _timeControl->setVisible(true);
        QPoint pos;
        pos.setX(x() + 
                 (frameGeometry().width() / 2) -
                 (_timeControl->width() / 2));
        pos.setY(y() + frameGeometry().height());
        _timeControl->move(pos);
      }
      _timeControl->setVisible(true);
      _timeControl->raise();
    }
  }
}

/////////////////////////////////////
// place the time controller dialog

void PolarManager::_placeTimeControl()
{

  if (_timeControl) {
    if (!_timeControlPlaced) {
      int topFrameWidth = _timeControl->geometry().y() - _timeControl->y();
      QPoint pos;
      pos.setX(x() + 
               (frameGeometry().width() / 2) -
               (_timeControl->width() / 2));
      pos.setY(y() + frameGeometry().height() + topFrameWidth);
      _timeControl->move(pos);
      _timeControlPlaced = true;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// print time slider actions for debugging

void PolarManager::_timeSliderActionTriggered(int action) {
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

void PolarManager::_timeSliderValueChanged(int value) 
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

void PolarManager::_timeSliderReleased() 
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

void PolarManager::_timeSliderPressed() 
{
  int value = _timeSlider->value();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Time slider released, value: " << value << endl;
  }
}

////////////////////////////////////////////////////
// create the file chooser dialog
//
// This allows the user to choose an archive file to open

void PolarManager::_openFile()
{
  // seed with files for the day currently in view
  // generate like this: *yyyymmdd*
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

  QString filename =  QFileDialog::getOpenFileName(
          this,
          "Open Document",
          inputPath, finalPattern);  //QDir::currentPath(),
  //"All files (*.*)");
 
  if( !filename.isNull() )
  {
    QByteArray qb = filename.toUtf8();
    const char *name = qb.constData();
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "selected file path : " << name << endl;
    }

    // trying this ... to get the data from the file selected
    _setArchiveRetrievalPending();
    vector<string> list;
    list.push_back(name);
    setArchiveFileList(list, false);

    try {
      _getArchiveData();
    } catch (FileIException ex) { 
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
  QFileInfo fileInfo(filename);
  string absolutePath = fileInfo.absolutePath().toStdString();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "changing to path " << absolutePath << endl;
  }
//  loadArchiveFileList(dir.absolutePath());

  RadxTimeList timeList;
  timeList.setDir(absolutePath);
  timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  if (timeList.compile()) {
    cerr << "ERROR - PolarManager::openFile()" << endl;
    cerr << "  " << timeList.getErrStr() << endl;
  }

  vector<string> pathList = timeList.getPathList();
  if (pathList.size() <= 0) {
    cerr << "ERROR - PolarManager::openFile()" << endl;
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
  } // end else pathList is not empty
}

////////////////////////////////////////////////////
// create the file chooser dialog
//
// This allows the user to choose the name of a
// file in which to save Volume data

void PolarManager::_saveFile()
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
      LOG(DEBUG) << "selected file path : " << name;
    }

    // TODO: hold it! the save message should
    // go to the Model (Data) level because
    // we'll be using Radx utilities.
    // 
    RadxFile outFile;
    try {
      LOG(DEBUG) << "writing to file " << name;
      outFile.writeToPath(_vol, name);
    } catch (FileIException ex) {
      this->setCursor(Qt::ArrowCursor);
      return;
    }
  }
}

void PolarManager::_createFileChooserDialog()
{
  _refreshFileChooserDialog();
}

void PolarManager::_refreshFileChooserDialog()
{

}

void PolarManager::_showFileChooserDialog()
{

}

////////////////////////////////////////////////////////
// set times from gui widgets

void PolarManager::_setArchiveStartTimeFromGui(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiStartTime.set(date.year(), date.month(), date.day(),
                    time.hour(), time.minute(), time.second());
}

void PolarManager::_setArchiveEndTimeFromGui(const QDateTime &qdt)
{
  QDate date = qdt.date();
  QTime time = qdt.time();
  _guiEndTime.set(date.year(), date.month(), date.day(),
                  time.hour(), time.minute(), time.second());
}

void PolarManager::_acceptGuiTimes()
{
  _archiveStartTime = _guiStartTime;
  _archiveEndTime = _guiEndTime;
  loadArchiveFileList();
}

void PolarManager::_cancelGuiTimes()
{
  _setGuiFromArchiveStartTime();
  _setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// set gui widget from archive start time

void PolarManager::_setGuiFromArchiveStartTime()
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

void PolarManager::_setGuiFromArchiveEndTime()
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

void PolarManager::_setGuiFromSelectedTime()
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

void PolarManager::_setArchiveStartTime(const RadxTime &rtime)

{
  _archiveStartTime = rtime;
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromArchiveStartTime();
}

////////////////////////////////////////////////////////
// set archive end time

void PolarManager::_setArchiveEndTime(const RadxTime &rtime)

{
  _archiveEndTime = rtime;
  if (!_archiveEndTime.isValid()) {
    _archiveEndTime.set(RadxTime::NOW);
  }
  _setGuiFromArchiveEndTime();
}

////////////////////////////////////////////////////////
// change start time

void PolarManager::_goBack1()
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

void PolarManager::_goBackPeriod()
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

void PolarManager::_goFwd1()
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

void PolarManager::_goFwdPeriod()
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

void PolarManager::_setArchiveRetrievalPending()
{
  _archiveRetrievalPending = true;
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
  _setSweepPanelVisibility();

  if (_ppi) {
    _ppi->setArchiveMode(state);
  }
  if (_rhi) {
    _rhi->setArchiveMode(state);
  }
}

////////////////////////////////////////////////////////
// set modes for retrieving the data

void PolarManager::_setRealtime(bool enabled)
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

void PolarManager::_activateRealtimeRendering()
{
  _nGates = 1000;
  _maxRangeKm = _params.max_range_km;
  _clear();
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
  _clear();
  if (_ppi) {
    //_fieldRendererController->performRendering(0);
    _ppi->activateArchiveRendering();
  }
  if (_rhi) {
    _rhi->activateArchiveRendering();
    //_fieldRendererController->performRendering(0); // activateArchiveRendering();
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

void PolarManager::_createArchiveImageFiles()
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

void PolarManager::_createImageFilesAllSweeps()
{
  
  if (_params.images_set_sweep_index_list) {
    
    for (int ii = 0; ii < _params.images_sweep_index_list_n; ii++) {
      int index = _params._images_sweep_index_list[ii];
      if (index >= 0 && index < (int) _vol.getNSweeps()) {
        _sweepManager.setFileIndex(index);
        _createImageFiles();
      }
    }
    
  } else {
    
    for (size_t index = 0; index < _vol.getNSweeps(); index++) {
      _sweepManager.setFileIndex(index);
      _createImageFiles();
    }
    
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

  int fieldNum = _displayFieldController->getSelectedFieldNum(); // _fieldNum;
  
  // loop through fields
  size_t nFields = _displayFieldController->getNFields();
  for (size_t ii = 0; ii < nFields; ii++) {
    
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

string PolarManager::_getOutputPath(bool interactive, string &outputDir, string fileExt)
{
	  // set times from plots
	  if (_rhiMode) {
	    _plotStartTime = _rhi->getPlotStartTime();
	    _plotEndTime = _rhi->getPlotEndTime();
	  } else {
	    _plotStartTime = _ppi->getPlotStartTime();
	    _plotEndTime = _ppi->getPlotEndTime();
	  }

	  // compute output dir

	  outputDir = _params.images_output_dir;
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

void PolarManager::_saveImageToFile(bool interactive)
{
	  // create image
  QPixmap pixmap;
  if (_rhiMode)
    pixmap = QPixmap::grabWidget(_rhi);
  else
    pixmap = QPixmap::grabWidget(_ppi);
  QImage image = pixmap.toImage();

  string outputDir;
  string outputPath = _getOutputPath(interactive, outputDir, _params.images_file_name_extension);

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
      cerr << "ERROR - PolarManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.images_write_latest_data_info)

}



void PolarManager::ShowContextMenu(const QPoint &pos) {
  _ppi->ShowContextMenu(pos, &_vol);
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

void PolarManager::_createBoundaryEditorDialog()
{
	_boundaryEditorDialog = new QDialog(this);
//	_boundaryEditorDialog->setMaximumHeight(200);
	_boundaryEditorDialog->setMaximumHeight(220);
	_boundaryEditorDialog->setWindowTitle("Boundary Editor");

	Qt::Alignment alignCenter(Qt::AlignCenter);
	Qt::Alignment alignRight(Qt::AlignRight);

	_boundaryEditorDialogLayout = new QGridLayout(_boundaryEditorDialog);
	_boundaryEditorDialogLayout->setVerticalSpacing(4);

	int row = 0;
	QLabel *mainHeader = new QLabel("Click points in main window to draw\na polygon boundary and click near the first\npoint to close the polygon. Once closed,\nhold Shift key to insert/delete points.", _boundaryEditorDialog);
	_boundaryEditorDialogLayout->addWidget(mainHeader, row, 0, 1, 2, alignCenter);

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

	_boundaryEditorDialogLayout->addWidget(_boundaryEditorList, 1, 0, 1, 2);

	_boundaryEditorClearBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorClearBtn->setText("Clear");
	_boundaryEditorDialogLayout->addWidget(_boundaryEditorClearBtn, 2, 0);
    connect(_boundaryEditorClearBtn, SIGNAL(clicked()), this, SLOT(_clearBoundaryEditorClick()));

    _boundaryEditorSaveBtn = new QPushButton(_boundaryEditorDialog);
	_boundaryEditorSaveBtn->setText("Save");
	_boundaryEditorDialogLayout->addWidget(_boundaryEditorSaveBtn, 2, 1);
    connect(_boundaryEditorSaveBtn, SIGNAL(clicked()), this, SLOT(_saveBoundaryEditorClick()));

    connect(_boundaryEditorList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onBoundaryEditorListItemClicked(QListWidgetItem*)));
}

void PolarManager::onBoundaryEditorListItemClicked(QListWidgetItem* item)
{
	string fileExt = item->text().toUtf8().constData();
	bool found = (fileExt.find("<none>") != string::npos);
	if (!found)
	{
		cout << "clicked on item " << fileExt << endl;
		string outputDir;
		string path = _getOutputPath(false, outputDir, fileExt);
		BoundaryPointEditor::Instance()->load(path);
		_ppi->update();   //forces repaint which clears existing polygon
	}
}

void PolarManager::_clearBoundaryEditorClick()
{
	BoundaryPointEditor::Instance()->clear();
	_ppi->update();   //forces repaint which clears existing polygon
}

void PolarManager::_saveBoundaryEditorClick()
{
	cout << "PolarManager, _saveBoundaryEditorClick" << endl;

	string text = "Boundary" + to_string(_boundaryEditorList->currentRow()+1);
//	QString qtext = text.c_str();
	_boundaryEditorList->currentItem()->setText(text.c_str());

	string outputDir;
	string fileExt = _boundaryEditorList->currentItem()->text().toUtf8().constData();
	string path = _getOutputPath(false, outputDir, fileExt);
	BoundaryPointEditor::Instance()->save(path);
}

/////////////////////////////
// show boundary editor
void PolarManager::_showBoundaryEditor()
{
  if (_boundaryEditorDialog)
  {
    if (_boundaryEditorDialog->isVisible())
    {
    	_clearBoundaryEditorClick();
    	_boundaryEditorDialog->setVisible(false);
    }
    else
    {
      if (_boundaryEditorDialog->x() == 0 && _boundaryEditorDialog->y() == 0)
      {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y());
        _boundaryEditorDialog->move(pos);
      }
      _boundaryEditorDialog->setVisible(true);
      BoundaryPointEditor::Instance()->clear();

      _boundaryEditorDialog->raise();

      //rename any items that have corresponding file on disk
      for (int i=4; i > 0; i--)
      {
		string outputDir;
		string fileExt = "Boundary" + to_string(i+1);
		string path = _getOutputPath(false, outputDir, fileExt);
		ifstream infile(path);
		if (infile.good())
			_boundaryEditorList->item(i)->setText(fileExt.c_str());
      }

      //load the first boundary in list (if exists)
	  _boundaryEditorList->setCurrentRow(0);
	  onBoundaryEditorListItemClicked(_boundaryEditorList->currentItem());
    }
  }
}

