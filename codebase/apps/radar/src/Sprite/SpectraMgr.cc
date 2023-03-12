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
// SpectraMgr.cc
//
// SpectraMgr object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// SpectraMgr manages BSCAN rendering - vert pointing etc
//
///////////////////////////////////////////////////////////////

#include "SpectraMgr.hh"
#include "SpectraWidget.hh"
#include "ColorMap.hh"
#include "Params.hh"
#include "TsReader.hh"
#include "AllocCheck.hh"
#include <radar/RadarComplex.hh>
#include <toolsa/file_io.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/TaXml.hh>
#include <Radx/RadxFile.hh>
#include <dsserver/DsLdataInfo.hh>

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
#include <QLineEdit>
#include <QCheckBox>
#include <QErrorMessage>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QRect>

using namespace std;
bool SpectraMgr::_firstTimerEvent = true;

// Constructor

SpectraMgr::SpectraMgr(const Params &params,
                       TsReader *tsReader) :
        _params(params),
        _tsReader(tsReader),
        _plotStart(true)
        
{
  
  _spectra = NULL;

  // _prevAltKm = -9999.0;
  // _altRateMps = 0.0;
  
  // initialize geometry
  
  // _realtimeModeButton = NULL;
  // _archiveModeButton = NULL;

  _timeSpanSecs = _params.archive_time_span_secs;
  if (_params.input_mode == Params::ARCHIVE_TIME_MODE ||
      _params.input_mode == Params::FILE_LIST_MODE ||
      _params.input_mode == Params::FOLLOW_DISPLAY_MODE) {
    _archiveMode = true;
  } else {
    _archiveMode = false;
  }
  _archiveRetrievalPending = false;
  // _archiveTimeBox = NULL;
  // _archiveStartTimeEdit = NULL;
  // _archiveEndTimeEcho = NULL;

  _archiveStartTime.set(_params.archive_start_time);
  _archiveEndTime = _archiveStartTime + _timeSpanSecs;
  _goForward = true;

  // set up windows
  
  _setupWindows();

  // initialize the click point FMQ object

  _clickPointFmq.setUrl(_params.click_point_fmq_url);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _clickPointFmq.setVerbose();
  } else if (_params.debug) {
    _clickPointFmq.setDebug();
  }

  // set initial field to 0
  
  // _changeField(0, false);

}

// destructor

SpectraMgr::~SpectraMgr()

{

  if (_spectra) {
    delete _spectra;
  }

}

//////////////////////////////////////////////////
// Run

int SpectraMgr::run(QApplication &app)
{

  // make window visible

  show();
  
  // set timer running
  
  _dataTimerId = startTimer(5);
  
  return app.exec();

}

////////////////////////////////////////////////////////////////////////
// Set the label in the title bar.

void SpectraMgr::_setTitleBar(const string &radarName)
{
  _windowTitle = "SPRITE - Spectral Plot for Radar Iq Time sEries - " + radarName;
  setWindowTitle(tr(_windowTitle.c_str()));
}

//////////////////////////////////////////////////
// set up windows and widgets
  
void SpectraMgr::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // ascope - main window

  _spectraFrame = new QFrame(_main);
  _spectraFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the ASCOPE

  _spectra = new SpectraWidget(_spectraFrame, *this, _params);
  connect(this, SIGNAL(frameResized(const int, const int)),
	  _spectra, SLOT(resize(const int, const int)));
  
  // connect slots for location change
  
  connect(_spectra, SIGNAL(locationClicked(double, int)),
          this, SLOT(_spectraLocationClicked(double, int)));
  
  // create status panel

  _createStatusPanel();

  // main window layout
  
  QHBoxLayout *mainLayout = new QHBoxLayout(_main);
  mainLayout->setMargin(3);
  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_spectraFrame);

  _createActions();

  _createMenus();

  _initActions();

  
  // QString message = tr("A context menu is available by right-clicking");
  // statusBar()->showMessage(message);
  
  _setTitleBar(_params.radar_name);
  setMinimumSize(400, 300);
  resize(_params.main_window_width, _params.main_window_height);

  QPoint pos;
  pos.setX(_params.main_window_start_x);
  pos.setY(_params.main_window_start_y);
  move(pos);

  // set up field click dialog

  // _createClickReportDialog();

  // create the range axis settings dialog

  // _createRangeAxisDialog();

  // create the time axis settings dialog
  
  // _createTimeAxisDialog();
  _spectra->refresh();
 
}

void SpectraMgr::_createMenus()
{

  _fileMenu = menuBar()->addMenu(tr("&File"));
  // _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);
  
  _configMenu = menuBar()->addMenu(tr("&Config"));
  // _configMenu->addAction(_rangeAxisAct);
  // _configMenu->addAction(_timeAxisAct);

  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _overlaysMenu->addAction(_xGridAct);
  _overlaysMenu->addAction(_yGridAct);
  _overlaysMenu->addAction(_legendsAct);

  _actionsMenu = menuBar()->addMenu(tr("&Actions"));
  _actionsMenu->addAction(_showClickAct);
  _actionsMenu->addAction(_freezeAct);
  _actionsMenu->addAction(_clearAct);
  _actionsMenu->addAction(_unzoomAct);
  _actionsMenu->addAction(_refreshAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addSeparator();
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

void SpectraMgr::_createActions()
{

  // freeze display

  _freezeAct = new QAction(tr("Freeze"), this);
  _freezeAct->setShortcut(tr("Esc"));
  _freezeAct->setStatusTip(tr("Freeze display"));
  connect(_freezeAct, SIGNAL(triggered()), this, SLOT(_freeze()));

  // show user click in dialog

  _showClickAct = new QAction(tr("Show-Click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, SIGNAL(triggered()), this, SLOT(_showClick()));

  // set range axis settings

  // _rangeAxisAct = new QAction(tr("Range-Config"), this);
  // _rangeAxisAct->setStatusTip(tr("Set configuration for range axis"));
  // connect(_rangeAxisAct,
  //         SIGNAL(triggered()), this,
  //         SLOT(_showRangeAxisDialog()));

  // set time axis settings

  // _timeAxisAct = new QAction(tr("Time-Config"), this);
  // _timeAxisAct->setStatusTip(tr("Set configuration for time axis"));
  // connect(_timeAxisAct,
  //         SIGNAL(triggered()),
  //         this, SLOT(_showTimeAxisDialog()));

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
  connect(_clearAct, SIGNAL(triggered()), _spectra, SLOT(clear()));

  // exit app

  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // show X grid lines

  _xGridAct = new QAction(tr("X Grid"), this);
  _xGridAct->setStatusTip(tr("Turn X grid on/off"));
  _xGridAct->setCheckable(true);
  connect(_xGridAct, SIGNAL(triggered(bool)),
          _spectra, SLOT(setXGridEnabled(bool)));

  // show Y grid lines

  _yGridAct = new QAction(tr("Y Grid"), this);
  _yGridAct->setStatusTip(tr("Turn Y grid on/off"));
  _yGridAct->setCheckable(true);
  connect(_yGridAct, SIGNAL(triggered(bool)),
          _spectra, SLOT(setYGridEnabled(bool)));
  
  // write legends

  _legendsAct = new QAction(tr("Legends"), this);
  _legendsAct->setStatusTip(tr("Turn legends on/off"));
  _legendsAct->setCheckable(true);
  connect(_legendsAct, SIGNAL(triggered(bool)),
          _spectra, SLOT(setLegendsEnabled(bool)));
  
  // show instrument height line in altitude display

  // _instHtLineAct = new QAction(tr("Instrument Ht Line"), this);
  // _instHtLineAct->setStatusTip(tr("Turn instrument height line on/off"));
  // _instHtLineAct->setCheckable(true);
  // connect(_instHtLineAct, SIGNAL(triggered(bool)),
  //         _spectra, SLOT(setInstHtLineEnabled(bool)));

  // show latlon legend

  // _latlonLegendAct = new QAction(tr("Starting lat/lon legend"), this);
  // _latlonLegendAct->setStatusTip(tr("Display starting lat/lon as a legend"));
  // _latlonLegendAct->setCheckable(true);
  // connect(_latlonLegendAct, SIGNAL(triggered(bool)),
  //         _spectra, SLOT(setLatlonLegendEnabled(bool)));

  // show dist/track legend

  // _speedTrackLegendAct = new QAction(tr("Mean speed/track legend"), this);
  // _speedTrackLegendAct->setStatusTip(tr("Display mean speed and track as a legend"));
  // _speedTrackLegendAct->setCheckable(true);
  // connect(_speedTrackLegendAct, SIGNAL(triggered(bool)),
  //         _spectra, SLOT(setSpeedTrackLegendEnabled(bool)));

  // display distance ticks

  // _distScaleAct = new QAction(tr("Distance scale"), this);
  // _distScaleAct->setStatusTip(tr("Display distance scale in addition to time scale"));
  // _distScaleAct->setCheckable(true);
  // connect(_distScaleAct, SIGNAL(triggered(bool)),
  //         this, SLOT(_setDistScaleEnabled()));

  // howto / about

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
  
  // _saveImageAct = new QAction(tr("Save-Image"), this);
  // _saveImageAct->setStatusTip(tr("Save image to png file"));
  // connect(_saveImageAct, SIGNAL(triggered()), this, SLOT(_saveImageToFile()));

}

///////////////////////
// initialize actions

void SpectraMgr::_initActions()
{

  // if (_params.iqplot_x_grid_lines_on) {
  _xGridAct->setChecked(false); // initialize to false
  //   _xGridAct->trigger();         // toggle to true
  // } else {
    // _xGridAct->setChecked(true); // initialize to true
    // _xGridAct->trigger();        // toggle to false
  // }
  
  // if (_params.iqplot_y_grid_lines_on) {
  _yGridAct->setChecked(false); // initialize to false
  //   _yGridAct->trigger();         // toggle to true
  // } else {
    // _yGridAct->setChecked(true);  // initialize to true
    // _yGridAct->trigger();         // toggle to false
  // }

  _legendsAct->setChecked(false); // initialize to false

}

///////////////////////////////////////////////////////
// configure the plot axes

void SpectraMgr::_configureAxes()
  
{
  
  _spectra->configureAxes(0.0, 1.0,
                          _params.archive_time_span_secs);

}

//////////////////////////////////////////////////////////////
// override timer event to respond to timer
  
void SpectraMgr::timerEvent(QTimerEvent *event)
{

  // register with procmap

  PMU_auto_register("timerEvent");
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.
  
  if (_firstTimerEvent) {

    _spectra->resize(_spectraFrame->width(), _spectraFrame->height());
    
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

  }

  // read HawkEye click point info from FMQ
  
  bool gotNew = false;
  if (_readClickPointFmq(gotNew) == 0) {
    if (gotNew) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "====>> gotNewClickInfo" << endl;
      }
      if (_params.input_mode == Params::FOLLOW_DISPLAY_MODE) {
        _followDisplay();
      }
    }
  }

  // handle data
  
  if (event->timerId() == _dataTimerId) {
    if (_archiveMode) {
      if (_archiveRetrievalPending) {
        if (_params.input_mode == Params::FOLLOW_DISPLAY_MODE) {
          _followDisplay();
        } else {
          _handleArchiveData();
        }
        _archiveRetrievalPending = false;
      }
    } else {
      _handleRealtimeData();
    }
  }
  
}


///////////////////////////
// override resize event

void SpectraMgr::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent" << endl;
  }
  emit frameResized(_spectraFrame->width(), _spectraFrame->height());
}

////////////////////////////////////////////////////////////////
// override key press event

void SpectraMgr::keyPressEvent(QKeyEvent * e)
{
  
  // get key pressed

  Qt::KeyboardModifiers mods = e->modifiers();
  if (mods & Qt::AltModifier) {
    cerr << "!!!!!!!!!!!!" << endl;
  }
  char keychar = e->text().toLatin1().data()[0];  int key = e->key();
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Clicked char: " << keychar << ":" << (int) keychar << endl;
    cerr << "         key: " << hex << key << dec << endl;
  }

  // for '.', swap with previous field

  if (keychar == '.') {
    // QRadioButton *button = (QRadioButton *) _fieldGroup->button(_prevFieldNum);
    // button->click();
    // return;
  }
  
  // for ESC, freeze / unfreeze

  if (keychar == 27) {
    _freezeAct->trigger();
    return;
  }
  
  // check for back or forward in time

  if (key == Qt::Key_Left) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Clicked left arrow, go back in time" << endl;
    }
    _goBack();
    _performArchiveRetrieval();
  } else if (key == Qt::Key_Right) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _goFwd();
    _performArchiveRetrieval();
  }
  
  // check for increase/decreas in range

  if (key == Qt::Key_Up) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Clicked up arrow, change range by 1 gate" << endl;
    }
    _changeRange(1);
  } else if (key == Qt::Key_Down) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Clicked down arrow, change range by 1 gate" << endl;
    }
    _changeRange(-1);
  }

}

/////////////////////////////
// get data in realtime mode

void SpectraMgr::_handleRealtimeData()

{

  // seek to the end of the data queue

  _tsReader->seekToEndOfQueue();
  
  // do nothing if freeze is on

  if (_frozen) {
    return;
  }

  if (_params.debug) {
    cerr << "SpectraMgr::_handleRealtimeData()" << endl;
  }

  // set cursor to wait cursor
  
  this->setCursor(Qt::WaitCursor);
  
  // read in a beam

  Beam *beam = _tsReader->getNextBeam();
  if (beam == NULL) {
    cerr << "ERROR - end of data in realtime mode" << endl;
    // reset cursor
    this->setCursor(Qt::ArrowCursor);
    return;
  }

  // update status

  _updateStatusPanel(beam);

  // set cursor to wait cursor
  
  this->setCursor(Qt::WaitCursor);

  // plot the data
  
  _spectra->plotBeam(beam);
  this->setCursor(Qt::ArrowCursor);

  // clean up
  
  _manageBeamQueue(beam);

}

///////////////////////////////////////
// handle data in archive mode

void SpectraMgr::_handleArchiveData()

{

  if (_params.debug) {
    cerr << "SpectraMgr::_handleArchiveData()" << endl;
  }

  // set cursor to wait cursor
  
  this->setCursor(Qt::WaitCursor);
  
  // read in a beam

  Beam *beam = NULL;
  if (_goForward) {
    beam = _tsReader->getNextBeam();
  } else {
    beam = _tsReader->getPreviousBeam();
  }
  if (beam == NULL) {
    cerr << "ERROR - end of data in archive mode" << endl;
    // reset cursor
    this->setCursor(Qt::ArrowCursor);
    return;
  }

  // update status

  _updateStatusPanel(beam);

  // compute the moments
  
  if (beam->computeMoments()) {

    cerr << "ERROR - SpectraMgr::_handleArchiveData()" << endl;
    cerr << "  Cannot compute moments" << endl;

  } else {

    // set cursor to wait cursor
    
    this->setCursor(Qt::WaitCursor);
    
    // plot the data
    
    _spectra->plotBeam(beam);
    this->setCursor(Qt::ArrowCursor);

  }

  // clean up
  
  _manageBeamQueue(beam);

}

///////////////////////////////////////
// following display click point

void SpectraMgr::_followDisplay()

{

  if (_params.debug) {
    cerr << "SpectraMgr::_followDisplay()" << endl;
  }
  
  // set cursor to wait cursor
  
  this->setCursor(Qt::WaitCursor);
  
  // read in a beam

  Beam *beam = _tsReader->getBeamFollowDisplay(_clickPointTime,
                                               _clickPointElevation,
                                               _clickPointAzimuth);
  if (beam == NULL) {
    cerr << "ERROR - no data in followDisplay mode" << endl;
    // reset cursor
    this->setCursor(Qt::ArrowCursor);
    return;
  }

  _spectra->setRange(_clickPointRangeKm);
  _clickPointGateNum = _spectra->getSelectedGateNum();
  _clickPointTime = beam->getTime();
  _clickPointElevation = beam->getEl();
  _clickPointAzimuth = beam->getAz();
  _clickPointChanged();

  // update status

  _updateStatusPanel(beam);

  // compute the moments
  
  if (beam->computeMoments()) {

    cerr << "ERROR - SpectraMgr::_followDisplay()" << endl;
    cerr << "  Cannot compute moments" << endl;

  } else {

    // set cursor to wait cursor
    
    this->setCursor(Qt::WaitCursor);
    
    // plot the data
    
    _spectra->plotBeam(beam);
    this->setCursor(Qt::ArrowCursor);

  }

  // clean up
  
  _manageBeamQueue(beam);

}

///////////////////////////////////////
// manage the beam memory

void SpectraMgr::_manageBeamQueue(Beam *beam)

{

  // push the beam onto the queue

  _beamQueue.push_front(beam);

  // if the queue is too big, pop one element and delete it

  if (_beamQueue.size() > 16) {
    Beam *oldest = _beamQueue.back();
    delete oldest;
    _beamQueue.pop_back();
  }

}



/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure

/////////////////////////////////////////////////////////////////////  
// slots

/////////////////////////////////////////////////////////
// respond to a change in click location in the widget

void SpectraMgr::_spectraLocationClicked(double selectedRangeKm,
                                         int selectedGateNum)
  
{
  if (_params.debug) {
    cerr << "====>> location clicked - range, gateNum: " 
         << selectedRangeKm << ", "
         << selectedGateNum << endl;
  }
  _clickPointRangeKm = selectedRangeKm;
  _clickPointGateNum = selectedGateNum;
  _clickPointChanged();

}

//////////////////////////////////////////////////////////////////
// respond to a click point location change

void SpectraMgr::_clickPointChanged()

{

  // click point location has changed
  // send location to other display via FMQ

  _writeClickPointXml2Fmq();

}


////////////////////////////////
// unzoom display

void SpectraMgr::_unzoom()
{
  _spectra->unzoom();
  _unzoomAct->setEnabled(false);
}

////////////////////////////////////////////
// refresh

void SpectraMgr::_refresh()
{
  if (_archiveMode) {
    _performArchiveRetrieval();
  }
}

//////////////////////////////
// freeze display

void SpectraMgr::_freeze()
{
  if (_frozen) {
    _frozen = false;
    _freezeAct->setText("Freeze");
    _freezeAct->setStatusTip(tr("Click to freeze display, or hit ESC"));
    // _initialRay = true;
  } else {
    _frozen = true;
    _freezeAct->setText("Unfreeze");
    _freezeAct->setStatusTip(tr("Click to unfreeze display, or hit ESC"));
  }
}

//////////////////////////////////////////////////
// enable the unzoom button

void SpectraMgr::enableUnzoomButton() const
{
  _unzoomAct->setEnabled(true);
}

////////////////////////////////////////////////////////
// change altitude limits

// void SpectraMgr::_setAltitudeLimits()
// {

//   // limits

//   double minAltitude;
//   if (sscanf(_minAltitudeEdit->text().toLocal8Bit().data(), "%lg", &minAltitude) != 1) {
//     QErrorMessage errMsg(_minAltitudeEdit);
//     string text("Bad entry for min altitude: ");
//     text += _minAltitudeEdit->text().toLocal8Bit().data();
//     errMsg.setModal(true);
//     errMsg.showMessage(text.c_str());
//     errMsg.exec();
//     _resetAltitudeLimitsToDefaults();
//     return;
//   }
//   double maxAltitude;
//   if (sscanf(_maxAltitudeEdit->text().toLocal8Bit().data(), "%lg", &maxAltitude) != 1) {
//     QErrorMessage errMsg(_maxAltitudeEdit);
//     string text("Bad entry for max altitude: ");
//     text += _maxAltitudeEdit->text().toLocal8Bit().data();
//     errMsg.setModal(true);
//     errMsg.showMessage(text.c_str());
//     errMsg.exec();
//     _resetAltitudeLimitsToDefaults();
//     return;
//   }

//   if (minAltitude > maxAltitude) {
//     QErrorMessage errMsg(_maxAltitudeEdit);
//     string text("Bad entry for min/max altitudes: ");
//     text += _minAltitudeEdit->text().toLocal8Bit().data();
//     text += " / ";
//     text += _maxAltitudeEdit->text().toLocal8Bit().data();
//     text += "  Max must exceed min";
//     errMsg.setModal(true);
//     errMsg.showMessage(text.c_str());
//     errMsg.exec();
//     _resetAltitudeLimitsToDefaults();
//     return;
//   }

//   _minPlotAltitudeKm = minAltitude / _altitudeUnitsMult;
//   _maxPlotAltitudeKm = maxAltitude / _altitudeUnitsMult;

//   _refreshRangeAxisDialog();

//   // refresh

//   _configureAxes();

//   // _rangeAxisDialog->setVisible(false);

// }

////////////////////////////////////////////////////////
// change time span

void SpectraMgr::_setTimeSpan()
{

  // double timeSpan;
  // if (sscanf(_timeSpanEdit->text().toLocal8Bit().data(), 
  //            "%lg", &timeSpan) != 1) {
  //   QErrorMessage errMsg(_timeSpanEdit);
  //   string text("Bad entry for time span: ");
  //   text += _timeSpanEdit->text().toLocal8Bit().data();
  //   errMsg.setModal(true);
  //   errMsg.showMessage(text.c_str());
  //   errMsg.exec();
  //   _resetTimeSpanToDefault();
  //   return;
  // }
  
  // _timeSpanSecs = timeSpan;
  // _plotEndTime = _plotStartTime + _timeSpanSecs;
    
  // _setArchiveEndTime();
  // _configureAxes();

}

void SpectraMgr::_resetTimeSpanToDefault()
{
  // _timeSpanSecs = _params.spectra_time_span_secs;
  // char text[1024];
  // sprintf(text, "%g", _timeSpanSecs);
  // if (_timeSpanEdit) {
  //   _timeSpanEdit->setText(text);
  // }
  // _setArchiveEndTime();
  // _configureAxes();
}

////////////////////////////////////////////////////////
// set start time from gui widget

void SpectraMgr::_setStartTimeFromGui(const QDateTime &datetime1)
{
  // QDateTime datetime = _archiveStartTimeEdit->dateTime();
  // QDate date = datetime.date();
  // QTime time = datetime.time();
  // _archiveStartTime.set(date.year(), date.month(), date.day(),
  //                       time.hour(), time.minute(), time.second());
  // _setArchiveEndTime();
}

////////////////////////////////////////////////////////
// set gui widget from start time

void SpectraMgr::_setGuiFromStartTime()
{
  // QDate date(_archiveStartTime.getYear(),
  //            _archiveStartTime.getMonth(), _archiveStartTime.getDay());
  // QTime time(_archiveStartTime.getHour(),
  //            _archiveStartTime.getMin(), _archiveStartTime.getSec());
  // QDateTime datetime(date, time);
  // _archiveStartTimeEdit->setDateTime(datetime);
}

////////////////////////////////////////////////////////
// set start time to defaults

void SpectraMgr::_setArchiveStartTimeToDefault()

{

  // _archiveStartTime.set(_params.archive_start_time);
  // if (!_archiveStartTime.isValid()) {
  //   _archiveStartTime.set(RadxTime::NOW);
  // }
  // _setGuiFromStartTime();
  // _setArchiveEndTime();

}

////////////////////////////////////////////////////////
// set start time

void SpectraMgr::_setArchiveStartTime(const RadxTime &rtime)

{

  // if (rtime.utime() == 0) {
  //   return;
  // }

  // _archiveStartTime = rtime;
  // if (!_archiveStartTime.isValid()) {
  //   _archiveStartTime.set(RadxTime::NOW);
  // }
  // _setGuiFromStartTime();
  // _setArchiveEndTime();

}

////////////////////////////////////////////////////////
// set end time from start time and span

void SpectraMgr::_setArchiveEndTime()

{

  // _archiveEndTime = _archiveStartTime + _timeSpanSecs;
  // if (_archiveEndTimeEcho) {
  //   _archiveEndTimeEcho->setText(_archiveEndTime.asString(3).c_str());
  // }

}


////////////////////////////////////////////////////////
// change modes

void SpectraMgr::_setDataRetrievalMode()
{
  // if (!_archiveTimeBox) {
  //   return;
  // }
  // if (_realtimeModeButton && _realtimeModeButton->isChecked()) {
  //   if (_archiveMode) {
  //     _archiveMode = false;
  //     _archiveTimeBox->setEnabled(false);
  //     _spectra->activateRealtimeRendering();
  //   }
  // } else {
  //   if (!_archiveMode) {
  //     _archiveMode = true;
  //     _archiveTimeBox->setEnabled(true);
  //     if (_plotStartTime.utime() != 0) {
  //       _setArchiveStartTime(_plotStartTime - _timeSpanSecs);
  //       _setGuiFromStartTime();
  //     }
  //     _spectra->activateArchiveRendering();
  //   }
  // }
  // _configureAxes();
}

////////////////////////////////////////////////////////
// change time or azimuth

void SpectraMgr::_goBack()
{
  if (_params.input_mode == Params::FOLLOW_DISPLAY_MODE) {
    _clickPointAzimuth -= _params.click_point_delta_azimuth_deg;
    if (_clickPointAzimuth < 0) {
      _clickPointAzimuth += 360.0;
    }
  } else {
    _goForward = false;
    _archiveStartTime -= 1 * _timeSpanSecs;
    _setGuiFromStartTime();
  }
}

void SpectraMgr::_goFwd()
{
  if (_params.input_mode == Params::FOLLOW_DISPLAY_MODE) {
    _clickPointAzimuth += _params.click_point_delta_azimuth_deg;
    if (_clickPointAzimuth >= 360.0) {
      _clickPointAzimuth -= 360.0;
    }
  } else {
    _goForward = true;
    _archiveStartTime += 1 * _timeSpanSecs;
    _setGuiFromStartTime();
  }
}

void SpectraMgr::_changeRange(int deltaGates)
{
  _spectra->changeRange(deltaGates);
  _clickPointRangeKm = _spectra->getSelectedRangeKm();
  _clickPointGateNum = _spectra->getSelectedGateNum();
  _clickPointChanged();
}

////////////////////////////////////////////////////////
// perform archive retrieval

void SpectraMgr::_performArchiveRetrieval()
{
  _archiveRetrievalPending = true;
}

////////////////////////////////////////////////////////
// Open file

void SpectraMgr::_openFile()
{
}

/////////////////////////////////////////////////////////////////////  
// slots

/////////////////////////////
// show data at click point

void SpectraMgr::_showClick()
{
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
}

/////////////////////////////////////////////////////
// howto help

void SpectraMgr::_howto()
{
  string text;
  text += "HOWTO HINTS FOR SPRITE\n";
  text += "======================\n";
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

void SpectraMgr::_about()
{
  string text;
  
  text += "Sprite is the LROSE app for display of time series data\n\n";
  text += "Get help with Sprite ...  \n ";
  text += "\nReport an issue https://github.com/NCAR/lrose-core/issues \n ";
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

//////////////////////////////////////////////
// create the status panel

void SpectraMgr::_createStatusPanel()
{
 
  // Qt::Alignment alignLeft(Qt::AlignLeft);
  // Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignTop(Qt::AlignTop);

  // status panel - rows of label value pairs
  
  _statusPanel = new QGroupBox(_main);
  _statusLayout = new QGridLayout(_statusPanel);
  _statusLayout->setVerticalSpacing(5);

  int row = 0;
  
  // fonts
  
  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font6 = dummy.font();
  int fsize = _params.main_label_font_size;
  int fsize2 = _params.main_label_font_size + 2;
  int fsize6 = _params.main_label_font_size + 6;
  font.setPixelSize(fsize);
  font2.setPixelSize(fsize2);
  font6.setPixelSize(fsize6);

  // radar and site name
  
  _radarName = new QLabel(_statusPanel);
  string rname(_params.radar_name);
  _radarName->setText(rname.c_str());
  _radarName->setFont(font6);
  _statusLayout->addWidget(_radarName, row, 0, 1, 4, alignCenter);
  row++;

  // date and time

  _dateVal = new QLabel("9999/99/99", _statusPanel);
  _dateVal->setFont(font2);
  _statusLayout->addWidget(_dateVal, row, 0, 1, 2, alignCenter);
  row++;

  _timeVal = new QLabel("99:99:99.999", _statusPanel);
  _timeVal->setFont(font2);
  _statusLayout->addWidget(_timeVal, row, 0, 1, 2, alignCenter);
  row++;


  // other labels.  Note that we set the minimum size of the column
  // containing the right hand labels in timerEvent() to prevent the
  // wiggling we were seeing in certain circumstances.  For this to work,
  // the default values for these fields must represent the maximum digits
  // posible for each field.

  _elevVal = _createStatusVal("Elev", "-99.99", row++, fsize2);
  _azVal = _createStatusVal("Az", "-999.99", row++, fsize2);

  if (_params.show_status_in_gui.fixed_angle) {
    _fixedAngVal = _createStatusVal("FixedAng", "-99.99", row++, fsize2);
  } else {
    _fixedAngVal = NULL;
  }
  
  if (_params.show_status_in_gui.volume_number) {
    _volNumVal = _createStatusVal("Volume", "0", row++, fsize);
  } else {
    _volNumVal = NULL;
  }
  
  if (_params.show_status_in_gui.sweep_number) {
    _sweepNumVal = _createStatusVal("Sweep", "0", row++, fsize);
  } else {
    _sweepNumVal = NULL;
  }

  if (_params.show_status_in_gui.n_samples) {
    _nSamplesVal = _createStatusVal("NSamp", "0", row++, fsize);
  } else {
    _nSamplesVal = NULL;
  }

  if (_params.show_status_in_gui.n_gates) {
    _nGatesVal = _createStatusVal("NGates", "0", row++, fsize);
  } else {
    _nGatesVal = NULL;
  }

  if (_params.show_status_in_gui.gate_length) {
    _gateSpacingVal = _createStatusVal("GateLen(m)", "0", row++, fsize);
  } else {
    _gateSpacingVal = NULL;
  }
  
  if (_params.show_status_in_gui.pulse_width) {
    _pulseWidthVal = _createStatusVal("PulseLen", "-9999", row++, fsize);
  } else {
    _pulseWidthVal = NULL;
  }

  if (_params.show_status_in_gui.prf_mode) {
    _prfModeVal = _createStatusVal("PRF-mode", "Fixed", row++, fsize);
  } else {
    _prfModeVal = NULL;
  }

  if (_params.show_status_in_gui.prf) {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
  } else {
    _prfVal = NULL;
  }

  if (_params.show_status_in_gui.nyquist) {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
  } else {
    _nyquistVal = NULL;
  }

  if (_params.show_status_in_gui.max_range) {
    _maxRangeVal = _createStatusVal("MaxRng(km)", "-9999", row++, fsize);
  } else {
    _maxRangeVal = NULL;
  }

  if (_params.show_status_in_gui.unambiguous_range) {
    _unambigRangeVal = _createStatusVal("UARng(km)", "-9999", row++, fsize);
  } else {
    _unambigRangeVal = NULL;
  }

  if (_params.show_status_in_gui.measured_power_h) {
    _powerHVal = _createStatusVal("XmitPwrH", "-9999", row++, fsize);
  } else {
    _powerHVal = NULL;
  }

  if (_params.show_status_in_gui.measured_power_v) {
    _powerVVal = _createStatusVal("XmitPwrVV", "-9999", row++, fsize);
  } else {
    _powerVVal = NULL;
  }

  if (_params.show_status_in_gui.scan_name) {
    _scanNameVal = _createStatusVal("ScanName", "unknown", row++, fsize);
  } else {
    _scanNameVal = NULL;
  }

  if (_params.show_status_in_gui.scan_mode) {
    _sweepModeVal = _createStatusVal("ScanMode", "sur", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_params.show_status_in_gui.polarization_mode) {
    _polModeVal = _createStatusVal("PolMode", "single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_params.show_status_in_gui.latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_params.show_status_in_gui.longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_params.show_status_in_gui.altitude) {
    _altVal = _createStatusVal("Alt(m)", "-999.999",
                               row++, fsize, &_altLabel);
  } else {
    _altVal = NULL;
  }

  if (_params.show_status_in_gui.sun_elevation) {
    _sunElVal = _createStatusVal("SunEl(deg)", "-999.999", row++, fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_params.show_status_in_gui.sun_azimuth) {
    _sunAzVal = _createStatusVal("SunAz(deg)", "-999.999", row++, fsize);
  } else {
    _sunAzVal = NULL;
  }

  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

}

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *SpectraMgr::_createStatusVal(const string &leftLabel,
                                     const string &rightLabel,
                                     int row, 
                                     int fontSize,
                                     QLabel **label)
  
{

  QLabel *left = new QLabel(_statusPanel);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  _statusLayout->addWidget(left, row, 0, alignRight);
  if (label != NULL) {
    *label = left;
  }

  QLabel *right = new QLabel(_statusPanel);
  right->setText(rightLabel.c_str());
  // Qt::Alignment alignCenter(Qt::AlignCenter);
  _statusLayout->addWidget(right, row, 1, alignRight);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  _valsRight.push_back(right);

  return right;
}

//////////////////////////////////////////////
// update the status panel

void SpectraMgr::_updateStatusPanel(const Beam *beam)
{

  // set time etc

  char text[1024];
  
  QString prev_radar_name = _radarName->text();
  
  string rname(beam->getInfo().get_radar_name());
  if (_params.override_radar_name) rname = _params.radar_name;
  _radarName->setText(rname.c_str());

  if (prev_radar_name != _radarName->text()) {
    _setTitleBar(rname);
  }
  
  DateTime beamTime(beam->getTimeSecs());
  sprintf(text, "%.4d/%.2d/%.2d",
          beamTime.getYear(), beamTime.getMonth(), beamTime.getDay());
  _dateVal->setText(text);

  int nanoSecs = beam->getNanoSecs();
  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          beamTime.getHour(), beamTime.getMin(), beamTime.getSec(),
          (nanoSecs / 1000000));
  _timeVal->setText(text);
  
  if (_volNumVal) {
    _setText(text, "%d", beam->getVolumeNumber());
    _volNumVal->setText(text);
  }
  
  if (_sweepNumVal) {
    _setText(text, "%d", beam->getSweepNumber());
    _sweepNumVal->setText(text);
  }
  
  if (_fixedAngVal) {  
    _setText(text, "%6.2f", beam->getTargetAngle());
    _fixedAngVal->setText(text);
  }
  
  if (_elevVal) {
    if (fabs(beam->getEl()) < 1000) {
      _setText(text, "%6.2f", beam->getEl());
      _elevVal->setText(text);
    }
  }

  if (_azVal) {
    if (fabs(beam->getAz()) < 1000) {
      _setText(text, "%6.2f", beam->getAz());
      _azVal->setText(text);
    }
  }
  
  if (_nSamplesVal) {
    _setText(text, "%d", (int) beam->getNSamples());
    _nSamplesVal->setText(text);
  }
  
  if (_nGatesVal) {
    _setText(text, "%d", (int) beam->getNGates());
    _nGatesVal->setText(text);
  }
  
  if (_gateSpacingVal) {
    _setText(text, "%.1f", beam->getGateSpacingKm() * 1000.0);
    _gateSpacingVal->setText(text);
  }
  
  if (_pulseWidthVal) {
    _setText(text, "%.2f", beam->getPulseWidth());
    _pulseWidthVal->setText(text);
  }

  if (_prfVal) {
    if (!beam->getIsStagPrt()) {
      if (beam->getPrt() <= 0) {
        _setText(text, "%d", -9999);
      } else {
        _setText(text, "%d", (int) ((1.0 / beam->getPrt()) * 10.0 + 0.5) / 10);
      }
    } else {
      double prtSec = beam->getPrt();
      if (prtSec <= 0) {
        _setText(text, "%d", -9999);
      } else {
        int iprt = (int) ((1.0 / beam->getPrt()) * 10.0 + 0.5) / 10;
        snprintf(text, 1024, "%d(%d/%d)", iprt, beam->getStagM(), beam->getStagN());
      }
    }
    _prfVal->setText(text);
  }

  if (_nyquistVal) {
    if (fabs(beam->getNyquist()) < 1000) {
      _setText(text, "%.1f", beam->getNyquist());
      _nyquistVal->setText(text);
    }
  }

  if (_maxRangeVal) {
    double maxRangeData =
      beam->getStartRangeKm() + beam->getGateSpacingKm() * beam->getNGates();
    _setText(text, "%.1f", maxRangeData);
    _maxRangeVal->setText(text);
  }

  if (_unambigRangeVal) {
    if (fabs(beam->getUnambigRange()) < 100000) {
      _setText(text, "%.1f", beam->getUnambigRange());
      _unambigRangeVal->setText(text);
    }
  }
  
  if (_powerHVal) {
    if (beam->getMeasXmitPowerDbmH() > -9990) {
      _setText(text, "%.1f", beam->getMeasXmitPowerDbmH());
      _powerHVal->setText(text);
    }
  }
   
  if (_powerVVal) {
    if (beam->getMeasXmitPowerDbmV() > -9990) {
      _setText(text, "%.1f", beam->getMeasXmitPowerDbmV());
      _powerVVal->setText(text);
    }
  }

  if (_scanNameVal) {
    string scanName = beam->getInfo().get_scan_segment_name();
    size_t len = scanName.size();
    if (len > 8) {
      scanName = scanName.substr(0, 8);
    }
    _scanNameVal->setText(scanName.c_str());
  }

  if (_sweepModeVal) {
    switch (beam->getScanMode()) {
      case IWRF_SCAN_MODE_SECTOR: {
        _sweepModeVal->setText("sec"); break;
      }
      case IWRF_SCAN_MODE_COPLANE: {
        _sweepModeVal->setText("coplane"); break;
      }
      case IWRF_SCAN_MODE_EL_SUR_360: {
        case IWRF_SCAN_MODE_RHI: {
          _sweepModeVal->setText("rhi"); break;
        }
        case IWRF_SCAN_MODE_VERTICAL_POINTING: {
          _sweepModeVal->setText("vert"); break;
        }
        case IWRF_SCAN_MODE_IDLE: {
          _sweepModeVal->setText("idle"); break;
        }
        case IWRF_SCAN_MODE_AZ_SUR_360:
          _sweepModeVal->setText("sur"); break;
      }
      case IWRF_SCAN_MODE_SUNSCAN: {
        _sweepModeVal->setText("sun"); break;
      }
      case IWRF_SCAN_MODE_POINTING: {
        _sweepModeVal->setText("point"); break;
      }
      default: {
        _sweepModeVal->setText("---");
      }
    }
  }

  if (_polModeVal) {
    switch (beam->getInfo().get_proc_xmit_rcv_mode()) {
      case IWRF_SINGLE_POL:
      default:
        _polModeVal->setText("single");
        break;
      case IWRF_ALT_HV_CO_ONLY:
      case IWRF_ALT_HV_CO_CROSS:
      case IWRF_ALT_HV_FIXED_HV:
        _polModeVal->setText("alt");
        break;
      case IWRF_SIM_HV_FIXED_HV:
      case IWRF_SIM_HV_SWITCHED_HV:
        _polModeVal->setText("sim");
        break;
      case IWRF_H_ONLY_FIXED_HV:
        _polModeVal->setText("xmit_h_dual");
        break;
      case IWRF_V_ONLY_FIXED_HV:
        _polModeVal->setText("xmit_v_dual");
        break;
    }
  }
   
  if (_prfModeVal) {
    if (beam->getIsStagPrt()) {
      _prfModeVal->setText("staggered");
    } else {
      _prfModeVal->setText("fixed");
    }
  }

  if (_params.override_radar_location) {
    _radarLat = _params.radar_latitude_deg;
    _radarLon = _params.radar_longitude_deg;
    _radarAltM = _params.radar_altitude_meters;
  } else {
    _radarLat = beam->getInfo().get_radar_latitude_deg();
    _radarLon = beam->getInfo().get_radar_longitude_deg();
    _radarAltM = beam->getInfo().get_radar_altitude_m();
  }
  if (beam->getInfo().isPlatformGeorefActive()) {
    iwrf_platform_georef_t georef = beam->getInfo().getPlatformGeoref();
    _radarLat = georef.latitude;
    _radarLon = georef.longitude;
    _radarAltM = georef.altitude_msl_km * 1000.0;
  }

  _sunPosn.setLocation(_radarLat, _radarLon, _radarAltM);

  if (_latVal) {
    _setText(text, "%.3f", _radarLat);
    _latVal->setText(text);
  }
     
  if (_lonVal) {
    _setText(text, "%.3f", _radarLon);
    _lonVal->setText(text);
  }
    
  if (_altVal) {
    _setText(text, "%.0f", _radarAltM);
    _altVal->setText(text);
  }
  
  double sunEl, sunAz;
  _sunPosn.computePosn(beam->getTime().getTimeAsDouble(), sunEl, sunAz);
  _setText(text, "%.3f", sunEl);
  if (_sunElVal) {
    _sunElVal->setText(text);
  }
  _setText(text, "%.3f", sunAz);
  if (_sunAzVal) {
    _sunAzVal->setText(text);
  }

}

///////////////////////////////////////////
// set text for GUI panels

void SpectraMgr::_setText(char *text,
                          const char *format,
                          int val)
{
  if (abs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999);
  }
}

void SpectraMgr::_setText(char *text,
                          const char *format,
                          double val)
{
  if (fabs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999.0);
  }
}

/////////////////////////////////////////////////
// read click point data from FMQ
// Returns 0 on success, -1 on failure

int SpectraMgr::_readClickPointFmq(bool &gotNew)
  
{
  
  // read in a new message
  
  if (_clickPointFmq.read(gotNew)) {
    cerr << "ERROR -  SpectraMgr::_readClickPointFmq" << endl;
    cerr << "  Cannot read click point info from FMQ" << endl;
    cerr << "  Fmq: " << _params.click_point_fmq_url << endl;
    return -1;
  }
  
  if (!gotNew) {
    // no data
    return 0;
  }

  // set the members

  _clickPointTimeSecs = _clickPointFmq.getDataTimeSecs();
  _clickPointNanoSecs = _clickPointFmq.getDataNanoSecs();
  _clickPointTime.set(_clickPointTimeSecs, (double) _clickPointNanoSecs * 1.0e-9);
  _clickPointElevation = _clickPointFmq.getElevation();
  _clickPointAzimuth = _clickPointFmq.getAzimuth();
  _clickPointRangeKm = _clickPointFmq.getRangeKm();
  _clickPointGateNum = _clickPointFmq.getGateNum();

  if (_params.debug) {
    cerr << "=========== latest click point XML ==================" << endl;
    cerr << "_clickPointTime: " << _clickPointTime.asString(6) << endl;
    cerr << "_clickPointElevation: " << _clickPointElevation << endl;
    cerr << "_clickPointAzimuth: " << _clickPointAzimuth << endl;
    cerr << "_clickPointRangeKm: " << _clickPointRangeKm << endl;
    cerr << "_clickPointGateNum: " << _clickPointGateNum << endl;
    cerr << "=====================================================" << endl;
  }
  
  return 0;

}

/////////////////////////////////////////////////////////////////
// write click point data, in XML format, to FMQ

int SpectraMgr::_writeClickPointXml2Fmq()

{
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "DisplayManager::_writeClickPointXml2Fmq() called\n");
  }

  if (_clickPointFmq.write(_clickPointTimeSecs,
                           _clickPointNanoSecs,
                           _clickPointAzimuth,
                           _clickPointElevation,
                           _clickPointRangeKm,
                           _clickPointGateNum)) {
    cerr << "ERROR - DisplayManager::_writeClickPointXml2Fmq()" << endl;
    return -1;
  }

  return 0;

}

