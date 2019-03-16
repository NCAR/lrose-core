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
// AScopeManager.cc
//
// AScopeManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// AScopeManager manages BSCAN rendering - vert pointing etc
//
///////////////////////////////////////////////////////////////

#include "AScopeManager.hh"
#include "AScopeWidget.hh"
#include "ColorMap.hh"
#include "Params.hh"
#include "TsReader.hh"
#include "AllocCheck.hh"
#include <radar/RadarComplex.hh>
#include <toolsa/file_io.h>

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

#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <Radx/RadxFile.hh>
#include <dsserver/DsLdataInfo.hh>

using namespace std;
bool AScopeManager::_firstTimerEvent = true;

// Constructor

AScopeManager::AScopeManager(const Params &params,
                             TsReader *reader) :
        _params(params),
        _reader(reader),
        _plotStart(true)
        
{
  
  _ascope = NULL;

  _prevAltKm = -9999.0;
  _altRateMps = 0.0;
  
  // initialize geometry
  
  _realtimeModeButton = NULL;
  _archiveModeButton = NULL;

  _timeSpanSecs = _params.ascope_time_span_secs;
  _archiveMode = _params.begin_in_archive_mode;
  _archiveRetrievalPending = false;
  _archiveTimeBox = NULL;
  _archiveStartTimeEdit = NULL;
  _archiveEndTimeEcho = NULL;

  _archiveStartTime.set(_params.archive_start_time);

  _dwellSpecsBox = NULL;
  _dwellAutoBox = NULL;
  _dwellAutoVal = NULL;
  _dwellSpecifiedEdit = NULL;
  _dwellSpecifiedFrame = NULL;
  _dwellStatsComboBox = NULL;

  // set up windows
  
  _setupWindows();

  // set initial field to 0
  
  // _changeField(0, false);

}

// destructor

AScopeManager::~AScopeManager()

{

  if (_ascope) {
    delete _ascope;
  }

}

//////////////////////////////////////////////////
// Run

int AScopeManager::run(QApplication &app)
{

  if (_params.debug) {
    cerr << "Running in BSCAN mode" << endl;
  }

  // make window visible

  show();
  
  // set timer running
  
  _dataTimerId = startTimer(5);
  
  return app.exec();

}

////////////////////////////////////////////////////////////////////////
// Set the label in the title bar.

void AScopeManager::_setTitleBar(const string &radarName)
{
  string windowTitle = "HAWK_EYE BSCAN -- " + radarName;
  setWindowTitle(tr(windowTitle.c_str()));
}

//////////////////////////////////////////////////
// set up windows and widgets
  
void AScopeManager::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // bscan - main window

  _ascopeFrame = new QFrame(_main);
  _ascopeFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the BSCAN

  _ascope = new AScopeWidget(_ascopeFrame, *this, _params);
  connect(this, SIGNAL(frameResized(const int, const int)),
	  _ascope, SLOT(resize(const int, const int)));
  
  // connect slots for location change
  
  connect(_ascope, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_ascopeLocationClicked(double, double, const RadxRay*)));
  
  // create status panel
  
  // _createStatusPanel();

  // main window layout
  
  QHBoxLayout *mainLayout = new QHBoxLayout(_main);
  mainLayout->setMargin(3);
  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_ascopeFrame);

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

}

void AScopeManager::_createMenus()
{

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  _configMenu = menuBar()->addMenu(tr("&Config"));
  _configMenu->addAction(_rangeAxisAct);
  _configMenu->addAction(_timeAxisAct);

  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _overlaysMenu->addAction(_rangeGridAct);
  _overlaysMenu->addAction(_timeGridAct);
  _overlaysMenu->addAction(_instHtLineAct);
  _overlaysMenu->addAction(_latlonLegendAct);
  _overlaysMenu->addAction(_speedTrackLegendAct);
  _overlaysMenu->addAction(_distScaleAct);


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

void AScopeManager::_createActions()
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

  _rangeAxisAct = new QAction(tr("Range-Config"), this);
  _rangeAxisAct->setStatusTip(tr("Set configuration for range axis"));
  connect(_rangeAxisAct,
          SIGNAL(triggered()), this,
          SLOT(_showRangeAxisDialog()));

  // set time axis settings

  _timeAxisAct = new QAction(tr("Time-Config"), this);
  _timeAxisAct->setStatusTip(tr("Set configuration for time axis"));
  connect(_timeAxisAct,
          SIGNAL(triggered()),
          this, SLOT(_showTimeAxisDialog()));

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
  connect(_clearAct, SIGNAL(triggered()), _ascope, SLOT(clear()));

  // exit app

  _exitAct = new QAction(tr("E&xit"), this);
  _exitAct->setShortcut(tr("Ctrl+Q"));
  _exitAct->setStatusTip(tr("Exit the application"));
  connect(_exitAct, SIGNAL(triggered()), this, SLOT(close()));

  // show range grid lines

  _rangeGridAct = new QAction(tr("Range Grid"), this);
  _rangeGridAct->setStatusTip(tr("Turn range grid on/off"));
  _rangeGridAct->setCheckable(true);
  connect(_rangeGridAct, SIGNAL(triggered(bool)),
	  _ascope, SLOT(setRangeGridEnabled(bool)));

  // show time grid lines

  _timeGridAct = new QAction(tr("Time Grid"), this);
  _timeGridAct->setStatusTip(tr("Turn time grid on/off"));
  _timeGridAct->setCheckable(true);
  connect(_timeGridAct, SIGNAL(triggered(bool)),
	  _ascope, SLOT(setTimeGridEnabled(bool)));
  
  // show instrument height line in altitude display

  _instHtLineAct = new QAction(tr("Instrument Ht Line"), this);
  _instHtLineAct->setStatusTip(tr("Turn instrument height line on/off"));
  _instHtLineAct->setCheckable(true);
  connect(_instHtLineAct, SIGNAL(triggered(bool)),
	  _ascope, SLOT(setInstHtLineEnabled(bool)));

  // show latlon legend

  _latlonLegendAct = new QAction(tr("Starting lat/lon legend"), this);
  _latlonLegendAct->setStatusTip(tr("Display starting lat/lon as a legend"));
  _latlonLegendAct->setCheckable(true);
  connect(_latlonLegendAct, SIGNAL(triggered(bool)),
	  _ascope, SLOT(setLatlonLegendEnabled(bool)));

  // show dist/track legend

  _speedTrackLegendAct = new QAction(tr("Mean speed/track legend"), this);
  _speedTrackLegendAct->setStatusTip(tr("Display mean speed and track as a legend"));
  _speedTrackLegendAct->setCheckable(true);
  connect(_speedTrackLegendAct, SIGNAL(triggered(bool)),
	  _ascope, SLOT(setSpeedTrackLegendEnabled(bool)));

  // display distance ticks

  _distScaleAct = new QAction(tr("Distance scale"), this);
  _distScaleAct->setStatusTip(tr("Display distance scale in addition to time scale"));
  _distScaleAct->setCheckable(true);
  connect(_distScaleAct, SIGNAL(triggered(bool)),
	  this, SLOT(_setDistScaleEnabled()));

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
  
  _saveImageAct = new QAction(tr("Save-Image"), this);
  _saveImageAct->setStatusTip(tr("Save image to png file"));
  connect(_saveImageAct, SIGNAL(triggered()), this, SLOT(_saveImageToFile()));

}

///////////////////////
// initialize actions

void AScopeManager::_initActions()
{

  if (_params.ascope_draw_x_grid_lines) {
    _rangeGridAct->setChecked(true);
  } else {
    _rangeGridAct->setChecked(false);
  }

  if (_params.ascope_draw_time_grid_lines) {
    _timeGridAct->setChecked(true);
  } else {
    _timeGridAct->setChecked(false);
  }

}

///////////////////////////////////////////////////////
// configure the plot axes

void AScopeManager::_configureAxes()
  
{
  
  _ascope->configureAxes(_params.ascope_min_amplitude,
                         _params.ascope_max_amplitude,
                         _params.ascope_time_span_secs);

}

//////////////////////////////////////////////////////////////
// override timer event to respond to timer
  
void AScopeManager::timerEvent(QTimerEvent *event)
{


  // register with procmap

  PMU_auto_register("timerEvent");
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.
  
  if (_firstTimerEvent) {

    _ascope->resize(_ascopeFrame->width(), _ascopeFrame->height());
    
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

  // handle event

  if (event->timerId() == _dataTimerId) {

    if (_archiveMode) {
      if (_archiveRetrievalPending) {
        _setDwellAutoVal();
        _handleArchiveData();
        _archiveRetrievalPending = false;
      }
    } else {
      _handleRealtimeData();
    }

  }

}


///////////////////////////
// override resize event

void AScopeManager::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent" << endl;
  }
  emit frameResized(_ascopeFrame->width(), _ascopeFrame->height());
}

////////////////////////////////////////////////////////////////
// override key press event

void AScopeManager::keyPressEvent(QKeyEvent * e)
{
  
  // get key pressed

  Qt::KeyboardModifiers mods = e->modifiers();
  if (mods & Qt::AltModifier) {
    cerr << "!!!!!!!!!!!!" << endl;
  }
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();
  
  if (_params.debug) {
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
  
  // check for short-cut keys to fields

  // for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    
  //   const DisplayField *field = _fields[ifield];

  //   char shortcut = 0;
  //   if (field->getShortcut().size() > 0) {
  //     shortcut = field->getShortcut()[0];
  //   }
    
  //   bool correctField = false;
  //   if (shortcut == keychar) {
  //     if (mods & Qt::AltModifier) {
  //       if (field->getIsFilt()) {
  //         correctField = true;
  //       }
  //     } else {
  //       if (!field->getIsFilt()) {
  //         correctField = true;
  //       }
  //     }
  //   }

  //   if (correctField) {
  //     if (_params.debug) {
  //       cerr << "Short-cut key pressed: " << shortcut << endl;
  //       cerr << "  field label: " << field->getLabel() << endl;
  //       cerr << "  field name: " << field->getName() << endl;
  //     }
  //     QRadioButton *button = (QRadioButton *) _fieldGroup->button(ifield);
  //     button->click();
  //     break;
  //   }

  // }

  // check for back or forward in time

  if (key == Qt::Key_Left) {
    if (_params.debug) {
      cerr << "Clicked left arrow, go back in time" << endl;
    }
    _goBack1();
    _performArchiveRetrieval();
  } else if (key == Qt::Key_Right) {
    if (_params.debug) {
      cerr << "Clicked right arrow, go forward in time" << endl;
    }
    _goFwd1();
    _performArchiveRetrieval();
  }

  // check for increase/decreas in range

  if (key == Qt::Key_Up) {
    if (_params.debug) {
      cerr << "Clicked up arrow, change range by 1 gate" << endl;
    }
    _changeRange(1);
  } else if (key == Qt::Key_Down) {
    if (_params.debug) {
      cerr << "Clicked down arrow, change range by 1 gate" << endl;
    }
    _changeRange(-1);
  }

}

/////////////////////////////
// get data in realtime mode

void AScopeManager::_handleRealtimeData()

{

  // do nothing if freeze is on

  if (_frozen) {
    return;
  }

  // get all available beams
  
  // while (true) {
    
  //   // get the next ray from the reader queue
  //   // responsibility for this ray memory passes to
  //   // this (the master) thread
    
  //   RadxRay *ray = _reader->getNextRay(_platform);
  //   if (ray == NULL) {
  //     return; // no pending rays
  //   }
    
  //   if (_params.debug >= Params::DEBUG_EXTRA) {
  //     cerr << "  Got a ray, time, el, az: "
  //          << DateTime::strm(ray->getTimeSecs()) << ", "
  //          << ray->getElevationDeg() << ", "
  //          << ray->getAzimuthDeg() << endl;
  //   }
    
  //   RadxTime thisRayTime = ray->getRadxTime();
  //   double timeSincePrev = thisRayTime - _readerRayTime;
  //   if ((timeSincePrev > 0) &&
  //       (timeSincePrev < _params.bscan_min_secs_between_reading_beams)) {
  //     // discard
  //     if (_params.debug >= Params::DEBUG_EXTRA) {
  //       cerr << "  Discarding ray, not enough elapsed time" << endl;
  //     }
  //     delete ray;
  //     AllocCheck::inst().addFree();
  //     continue;
  //   } else if (timeSincePrev < 0) {
  //     // gone back in time, so reset times
  //     _imagesScheduledTime.set(RadxTime::ZERO);
  //   }
  //   _readerRayTime = thisRayTime;

  //   // compute altitude rate every 2 secs

  //   if (_prevAltKm > -9990) {
  //     double deltaTime = ray->getRadxTime() - _prevAltTime;
  //     if (deltaTime > 2.0) {
  //       if (ray->getGeoreference()) {
  //         double altKm = ray->getGeoreference()->getAltitudeKmMsl();
  //         double deltaAltKm = altKm - _prevAltKm;
  //         _altRateMps = (deltaAltKm / deltaTime) * 1000.0;
  //         _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
  //       }
  //       _prevAltTime = ray->getRadxTime();
  //     }
  //   } else {
  //     if (ray->getGeoreference()) {
  //       _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
  //     }
  //     _prevAltTime = ray->getRadxTime();
  //   }
    
  //   // update the status panel
    
  //   _updateStatusPanel(ray);

  //   // draw the beam
    
  //   _handleRay(ray);
    
  // } // while (true)

}

/////////////////////////////////////////////
// get data in realtime image generation mode

void AScopeManager::_handleRealtimeDataForImages()

{

  // get all available beams
  
  // while (true) {
    
  //   // get the next ray from the reader queue
  //   // responsibility for this ray memory passes to
  //   // this (the master) thread
    
  //   RadxRay *ray = _reader->getNextRay(_platform);
  //   if (ray == NULL) {
  //     return; // no pending rays
  //   }
    
  //   if (_params.debug >= Params::DEBUG_EXTRA) {
  //     cerr << "  Got a ray, time, el, az: "
  //          << DateTime::strm(ray->getTimeSecs()) << ", "
  //          << ray->getElevationDeg() << ", "
  //          << ray->getAzimuthDeg() << endl;
  //   }
    
  //   RadxTime thisRayTime = ray->getRadxTime();
  //   double timeSincePrev = thisRayTime - _readerRayTime;
  //   if (timeSincePrev < 0) {
  //     // gone back in time, so reset times
  //     _imagesScheduledTime.set(RadxTime::ZERO);
  //   }
  //   _readerRayTime = thisRayTime;
    
  //   // update the status panel
    
  //   _updateStatusPanel(ray);
    
  //   // delete the ray
    
  //   delete ray;
  //   AllocCheck::inst().addFree();
    
  // } // while (true)

}

///////////////////////////////////////
// handle data in archive mode

void AScopeManager::_handleArchiveData()

{

  // set up plot times

  _plotStartTime = _archiveStartTime;
  _plotEndTime = _archiveEndTime;

  // erase plot and set time axis

  _ascope->setPlotStartTime(_plotStartTime, true);
  _ascope->activateArchiveRendering();

  // set cursor to wait cursor

  this->setCursor(Qt::WaitCursor);
  _timeAxisDialog->setCursor(Qt::WaitCursor);

  // get data

  if (_getArchiveData()) {
    this->setCursor(Qt::ArrowCursor);
    _timeAxisDialog->setCursor(Qt::ArrowCursor);
    return;
  }

  // plot the data

  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
  _timeAxisDialog->setCursor(Qt::ArrowCursor);

}

/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure

int AScopeManager::_getArchiveData()

{

  // set up file object for reading
  
  RadxFile file;
  // _vol.clear();
  // _setupVolRead(file);

  // if (_params.debug) {
  //   cerr << "----------------------------------------------------" << endl;
  //   cerr << "perform archive retrieval" << endl;
  //   cerr << "  archive start time: " << _archiveStartTime.asString() << endl;
  //   cerr << "  archive end time: " << _archiveEndTime.asString() << endl;
  //   cerr << "  dwell secs: " << _dwellSecs << endl;
  //   cerr << "  dwell stats method: "
  //        << RadxField::statsMethodToStr(_dwellStatsMethod) << endl;
  //   cerr << "----------------------------------------------------" << endl;
  // }
  
  // if (file.readFromDir(_params.archive_data_url, _vol)) {
  //   string errMsg = "ERROR - Cannot retrieve archive data\n";
  //   errMsg += "AScopeManager::_getArchiveData\n";
  //   errMsg += file.getErrStr() + "\n";
  //   errMsg += "  start time: " + _archiveStartTime.asString() + "\n";
  //   errMsg += "  end time: " + _archiveEndTime.asString() + "\n";
  //   char text[1024];
  //   sprintf(text, "  dwell secs: %g\n", _dwellSecs);
  //   errMsg += text;
  //   cerr << errMsg;
  //   if (!_params.images_auto_create)  {
  //     QErrorMessage errorDialog;
  //     errorDialog.setMinimumSize(400, 250);
  //     errorDialog.showMessage(errMsg.c_str());
  //     errorDialog.exec();
  //   }
  //   return -1;
  // }

  // _platform = _vol.getPlatform();

  return 0;

}

/////////////////////////////
// plot data in archive mode

void AScopeManager::_plotArchiveData()

{

  if(_params.debug) {
    cerr << "======== Plotting archive data =======================" << endl;
    cerr << "======>>   plotStartTime: " << _plotStartTime.asString(3) << endl;
    cerr << "======>>   plotEndTime: " << _plotEndTime.asString(3) << endl;
  }

  // initialize plotting

  // _initialRay = true;

  // // handle the rays
  
  // const vector<RadxRay *> &rays = _vol.getRays();
  // if (rays.size() < 1) {
  //   cerr << "ERROR - _plotArchiveData" << endl;
  //   cerr << "  No rays found" << endl;
  // }
  
  // for (size_t ii = 0; ii < rays.size(); ii++) {
  //   RadxRay *ray = rays[ii];
  //   _handleRay(ray);
  // }

  // // update the status panel
  
  // _updateStatusPanel(rays[0]);

}

//////////////////////////////////////////////////
// set up read

void AScopeManager::_setupVolRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }

  // for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
  //   const DisplayField *field = _fields[ifield];
  //   file.addReadField(field->getName());
  // }

  // _dwellSecs = _dwellSpecifiedSecs;
  // if (_dwellAuto) {
  //   _dwellSecs = _dwellAutoSecs;
  // }
  
  // file.setReadRaysInInterval(_archiveStartTime, _archiveEndTime,
  //                            _dwellSecs, _dwellStatsMethod);

}

/////////////////////////////////////////////////////////////////////  
// slots

///////////////////////////////////////////////////////////
// respond to change field request from field button group

// void AScopeManager::_changeField(int fieldId, bool guiMode)

// {

//   _selectedField = _fields[fieldId];
  
//   if (_params.debug) {
//     cerr << "Changing to field id: " << fieldId << endl;
//     _selectedField->print(cerr);
//   }

//   // if we click the already-selected field, go back to previous field

//   if (guiMode) {
//     if (_fieldNum == fieldId && _prevFieldNum >= 0) {
//       QRadioButton *button =
//         (QRadioButton *) _fieldGroup->button(_prevFieldNum);
//       button->click();
//       return;
//     }
//   }

//   _prevFieldNum = _fieldNum;
//   _fieldNum = fieldId;
  
//   _ascope->selectVar(_fieldNum);

//   // _colorBar->setColorMap(&_fields[_fieldNum]->getColorMap());
//   _selectedName = _selectedField->getName();
//   _selectedLabel = _selectedField->getLabel();
//   _selectedUnits = _selectedField->getUnits();
  
//   _selectedLabelWidget->setText(_selectedLabel.c_str());
//   char text[128];
//   if (_selectedField->getSelectValue() > -9990) {
//     sprintf(text, "%g %s", 
//             _selectedField->getSelectValue(),
//             _selectedField->getUnits().c_str());
//   } else {
//     text[0] = '\0';
//   }
//   _valueLabel->setText(text);

// }

/////////////////////////////////////////////////////////
// respond to a change in click location on the BSCAN

void AScopeManager::_locationClicked(double xsecs, double ykm,
                                     const RadxRay *closestRay)
  
{
  if (_params.debug) {
    cerr << "====>> location clicked - xsecs, ykm: " << xsecs << ", " << ykm << endl;
  }
  _xSecsClicked = xsecs;
  _yKmClicked = ykm;
  // _rayClicked = closestRay;

  // _locationClicked(_xSecsClicked, _yKmClicked, _rayClicked);

}

//////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

// void AScopeManager::_locationClicked(double xsecs, double ykm, const RadxRay *ray)

// {


//   if (_params.debug) {
//     cerr << "*** Entering AScopeManager::_locationClicked()" << endl;
//   }

//   // check the ray

//   if (ray == NULL) {
//     return;
//   }

//   double range = 0.0, altitude = 0.0;
//   double sinEl = sin(ray->getElevationDeg() * DEG_TO_RAD);

//   if (_ascope->getRangeAxisMode() == Params::RANGE_AXIS_ALTITUDE) {
    
//     altitude = ykm;
//     range = (altitude - _getInstHtKm(ray)) / sinEl;
    
//   } else {
    
//     range = ykm;
//     altitude = _getInstHtKm(ray) + range * sinEl;
    
//   }

//   int gate = (int) ((range - ray->getStartRangeKm()) / ray->getGateSpacingKm());

//   if (gate < 0 || gate >= (int) ray->getNGates())
//   {
//     //user clicked outside of ray
//     return;
//   }
  
//   if (_params.debug) {
//     cerr << "Clicked on location: xsecs, ykm: " << xsecs << ", " << ykm << endl;
//     cerr << "  range start, spacing: " << ray->getStartRangeKm() << ", "
//          << ray->getGateSpacingKm() << endl;
//     cerr << "  range, gate: " << range << ", " << gate << endl;
//     if (_params.debug >= Params::DEBUG_VERBOSE) {
//       ray->print(cerr);
//     }
//   }
  
//   DateTime rayTime(ray->getTimeSecs());
//   char text[256];
//   sprintf(text, "%.4d/%.2d/%.2d",
//           rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
//   _dateClicked->setText(text);

//   sprintf(text, "%.2d:%.2d:%.2d.%.3d",
//           rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
//           ((int) (ray->getNanoSecs() / 1000000)));
//   _timeClicked->setText(text);

//   if (fabs(ray->getElevationDeg()) < 10000) {
//     _setText(text, "%6.2f (deg)", ray->getElevationDeg());
//     _elevClicked->setText(text);
//   }
  
//   if (fabs(ray->getAzimuthDeg()) < 10000) {
//     _setText(text, "%6.2f (deg)", ray->getAzimuthDeg());
//     _azClicked->setText(text);
//   }
    
//   _setText(text, "%d", gate);
//   _gateNumClicked->setText(text);
  
//   _setText(text, "%6.2f (km)", range);
//   _rangeClicked->setText(text);

//   if (_altitudeInFeet) {
//     _setText(text, "%6.2f (kft)", altitude * _altitudeUnitsMult);
//   } else {
//     _setText(text, "%6.2f (km)", altitude * _altitudeUnitsMult);
//   }
//   _altitudeClicked->setText(text);
  
//   for (size_t ii = 0; ii < _fields.size(); ii++) {
//     _fields[ii]->setSelectValue(-9999);
//     _fields[ii]->setDialogText("----");
//   }
  
//   for (size_t ifield = 0; ifield < ray->getNFields(); ifield++) {
//     const RadxField *field = ray->getField(ifield);
//     const string fieldName = field->getName();
//     if (fieldName.size() == 0) {
//       continue;
//     }
//     Radx::fl32 *data = (Radx::fl32 *) field->getData();
//     double val = data[gate];
//     const string fieldUnits = field->getUnits();
//     if (_params.debug >= Params::DEBUG_VERBOSE) {
//       cerr << "Field name, selected name: "
// 	   << fieldName << ", "
// 	   << _selectedName << endl;
//     }
//     if (fieldName == _selectedName) {
//       char text[128];
//       if (fabs(val) < 10000) {
//         sprintf(text, "%g %s", val, fieldUnits.c_str());
//       } else {
//         sprintf(text, "%g %s", -9999.0, fieldUnits.c_str());
//       }
//       _valueLabel->setText(text);
//     }
//     if (_params.debug >= Params::DEBUG_VERBOSE) {
//       cerr << "Field name, units, val: "
// 	   << field->getName() << ", "
// 	   << field->getUnits() << ", "
// 	   << val << endl;
//     }
//     for (size_t ii = 0; ii < _fields.size(); ii++) {
//       if (_fields[ii]->getName() == fieldName) {
// 	_fields[ii]->setSelectValue(val);
//         char text[128];
//         if (fabs(val) > 10000) {
//           sprintf(text, "----");
//         } else if (fabs(val) > 10) {
//           sprintf(text, "%.2f", val);
//         } else {
//           sprintf(text, "%g", val);
//         }
//         _fields[ii]->setDialogText(text);
//       }
//     } // ii

//   } // ifield
  
//   // set altitude rate if possible
  
//   if (ray->getGeoreference()) {
//     _altRateMps = ray->getGeoreference()->getVertVelocity();
//   } else {
//     _altRateMps = -9999.0;
//   }
    
  // update the status panel
  
// _updateStatusPanel(ray);
    
// }

////////////////////////////////
// unzoom display

void AScopeManager::_unzoom()
{
  _ascope->unzoomView();
  _unzoomAct->setEnabled(false);
}

////////////////////////////////////////////
// refresh

void AScopeManager::_refresh()
{
  if (_archiveMode) {
    _performArchiveRetrieval();
  }
}

//////////////////////////////
// freeze display

void AScopeManager::_freeze()
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
// enable the zoom button - called by BscanWidget

void AScopeManager::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

////////////////////////////////////////////////////////
// change altitude limits

// void AScopeManager::_setAltitudeLimits()
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

void AScopeManager::_setTimeSpan()
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
  // _setDwellAutoVal();
  // _configureAxes();

}

void AScopeManager::_resetTimeSpanToDefault()
{
  _timeSpanSecs = _params.ascope_time_span_secs;
  char text[1024];
  sprintf(text, "%g", _timeSpanSecs);
  if (_timeSpanEdit) {
    _timeSpanEdit->setText(text);
  }
  _setArchiveEndTime();
  _setDwellAutoVal();
  _configureAxes();
}

////////////////////////////////////////////////////////
// set start time from gui widget

void AScopeManager::_setStartTimeFromGui(const QDateTime &datetime1)
{
  QDateTime datetime = _archiveStartTimeEdit->dateTime();
  QDate date = datetime.date();
  QTime time = datetime.time();
  _archiveStartTime.set(date.year(), date.month(), date.day(),
                        time.hour(), time.minute(), time.second());
  _setArchiveEndTime();
}

////////////////////////////////////////////////////////
// set gui widget from start time

void AScopeManager::_setGuiFromStartTime()
{
  QDate date(_archiveStartTime.getYear(),
             _archiveStartTime.getMonth(), _archiveStartTime.getDay());
  QTime time(_archiveStartTime.getHour(),
             _archiveStartTime.getMin(), _archiveStartTime.getSec());
  QDateTime datetime(date, time);
  _archiveStartTimeEdit->setDateTime(datetime);
}

////////////////////////////////////////////////////////
// set start time to defaults

void AScopeManager::_setArchiveStartTimeToDefault()

{

  _archiveStartTime.set(_params.archive_start_time);
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromStartTime();
  _setArchiveEndTime();

}

////////////////////////////////////////////////////////
// set start time

void AScopeManager::_setArchiveStartTime(const RadxTime &rtime)

{

  if (rtime.utime() == 0) {
    return;
  }

  _archiveStartTime = rtime;
  if (!_archiveStartTime.isValid()) {
    _archiveStartTime.set(RadxTime::NOW);
  }
  _setGuiFromStartTime();
  _setArchiveEndTime();

}

////////////////////////////////////////////////////////
// set end time from start time and span

void AScopeManager::_setArchiveEndTime()

{

  _archiveEndTime = _archiveStartTime + _timeSpanSecs;
  if (_archiveEndTimeEcho) {
    _archiveEndTimeEcho->setText(_archiveEndTime.asString(3).c_str());
  }

}


////////////////////////////////////////////////////////
// change modes

void AScopeManager::_setDataRetrievalMode()
{
  if (!_archiveTimeBox || !_dwellSpecsBox) {
    return;
  }
  if (_realtimeModeButton && _realtimeModeButton->isChecked()) {
    if (_archiveMode) {
      _archiveMode = false;
      _archiveTimeBox->setEnabled(false);
      _dwellSpecsBox->setEnabled(false);
      _ascope->activateRealtimeRendering();
    }
  } else {
    if (!_archiveMode) {
      _archiveMode = true;
      _archiveTimeBox->setEnabled(true);
      _dwellSpecsBox->setEnabled(true);
      if (_plotStartTime.utime() != 0) {
        _setArchiveStartTime(_plotStartTime - _timeSpanSecs);
        _setGuiFromStartTime();
      }
      _ascope->activateArchiveRendering();
    }
  }
  _configureAxes();
}

////////////////////////////////////////////////////////
// change start time

void AScopeManager::_goBack1()
{
  _archiveStartTime -= 1 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void AScopeManager::_goBack5()
{
  _archiveStartTime -= 5 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void AScopeManager::_goFwd1()
{
  _archiveStartTime += 1 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void AScopeManager::_goFwd5()
{
  _archiveStartTime += 5 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void AScopeManager::_changeRange(int deltaGates)
{
  if (!_ascope->getPointClicked()) {
    return;
  }
  // if (_requestedRangeAxisMode == Params::RANGE_AXIS_DOWN) {
  //   deltaGates *= -1;
  // }
  // _yKmClicked += deltaGates * _rayClicked->getGateSpacingKm();
  // _locationClicked(_xSecsClicked, _yKmClicked, _rayClicked);
  // _ascope->setMouseClickPoint(_xSecsClicked, _yKmClicked);
}

////////////////////////////////////////////////////////
// perform archive retrieval

void AScopeManager::_performArchiveRetrieval()
{
  _archiveRetrievalPending = true;
}

////////////////////////////////////////////////////////
// set dwell to defaults

void AScopeManager::_setDwellToDefaults()

{

  // _dwellAuto = _params.bscan_archive_dwell_auto;
  // _dwellSpecifiedSecs = _params.bscan_archive_dwell_secs;

  // int index = 0;
  // switch (_params.bscan_dwell_stats) {
  //   case Params::DWELL_STATS_MEAN:
  //     _dwellStatsMethod = RadxField::STATS_METHOD_MEAN;
  //     index = 0;
  //     break;
  //   case Params::DWELL_STATS_MEDIAN:
  //     _dwellStatsMethod = RadxField::STATS_METHOD_MEDIAN;
  //     index = 1;
  //     break;
  //   case Params::DWELL_STATS_MAXIMUM:
  //     _dwellStatsMethod = RadxField::STATS_METHOD_MAXIMUM;
  //     index = 2;
  //     break;
  //   case Params::DWELL_STATS_MINIMUM:
  //     _dwellStatsMethod = RadxField::STATS_METHOD_MINIMUM;
  //     index = 3;
  //     break;
  //   case Params::DWELL_STATS_MIDDLE:
  //   default:
  //     _dwellStatsMethod = RadxField::STATS_METHOD_MIDDLE;
  //     index = 4;
  //     break;
  // }

  // if (_dwellStatsComboBox) {
  //   _dwellStatsComboBox->setCurrentIndex(index);
  // }

}

//////////////////////////////////////
// set the specified dwell

void AScopeManager::_setDwellSpecified()

{

  double dwellSecs;
  bool error = false;
  if (sscanf(_dwellSpecifiedEdit->text().toLocal8Bit().data(),
             "%lg", &dwellSecs) != 1) {
    error = true;
  }
  if (dwellSecs <= 0) {
    error = true;
  }
  if (error) {
    QErrorMessage errMsg(_dwellSpecifiedEdit);
    string text("Bad entry for dwell: ");
    text += _dwellSpecifiedEdit->text().toLocal8Bit().data();
    text += " Must be >= 0";
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetDwellSpecifiedToDefault();
    return;
  }
  
  _dwellSpecifiedSecs = dwellSecs;

}

void AScopeManager::_resetDwellSpecifiedToDefault()

{
  // _dwellSpecifiedSecs = _params.bscan_archive_dwell_secs;
  // char text[1024];
  // sprintf(text, "%g", _dwellSpecifiedSecs);
  // _dwellSpecifiedEdit->setText(text);
}


//////////////////////////////////////
// set the dwell from auto selection

void AScopeManager::_setDwellAuto()

{
  // if (_dwellAutoBox) {
  //   _dwellAuto = _dwellAutoBox->isChecked();
  // } else {
  //   _dwellAuto = _params.bscan_archive_dwell_auto;
  // }
  // if (_dwellSpecifiedFrame) {
  //   _dwellSpecifiedFrame->setEnabled(!_dwellAuto);
  // }
}

void AScopeManager::_setDwellAutoVal()

{
  // if (_dwellAutoVal) {
  //   _configureAxes();
  //   const WorldPlot &world = _ascope->getFullWorld();
  //   double nPixelsPlot = world.getPlotWidth();
  //   _dwellAutoSecs = _timeSpanSecs / nPixelsPlot;
  //   char text[128];
  //   sprintf(text, "%lg", _dwellAutoSecs);
  //   _dwellAutoVal->setText(text);
  // }
}

///////////////////////////////////////////
// set dwell stats from combo box selection

void AScopeManager::_setDwellStats()
{
}

/////////////////////////////////////////////////////////////////////  
// slots

/////////////////////////////
// show data at click point

void AScopeManager::_showClick()
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

void AScopeManager::_howto()
{
  string text;
  text += "HOWTO HINTS FOR HAWK-EYE in BSCAN mode\n";
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

void AScopeManager::_about()
{
  string text;
  
  text += "HawkEye is an LROSE application for engineering and research display of radar data. \n\n";
  text += "Get help with HawkEye ...  \n ";
  text += "\nReport an issue https://github.com/NCAR/lrose-core/issues \n ";
  text += "\nHawkEye Version ... \n ";  
  text += "\nCopyright UCAR (c) 1990 - 2018  ";  
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

