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
// BscanManager.cc
//
// BscanManager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// BscanManager manages BSCAN rendering - vert pointing etc
//
///////////////////////////////////////////////////////////////

#include "BscanManager.hh"
#include "DisplayField.hh"
#include "BscanWidget.hh"
#include "ColorMap.hh"
#include "Params.hh"
#include "Reader.hh"
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

// Constructor

BscanManager::BscanManager(const Params &params,
                           Reader *reader,
                           const vector<DisplayField *> &fields,
                           bool haveFilteredFields) :
        DisplayManager(params, reader, fields, haveFilteredFields),
        _plotStart(true)
        
{
  
  _bscan = NULL;

  _prevAltKm = -9999.0;
  _altRateMps = 0.0;
  
  // initialize geometry
  
  _setRangeLimitsToDefaults();
  _setAltitudeLimitsToDefaults();
  _setCensorDataBelowSurfaceToDefaults();

  _requestedRangeAxisMode = _params.bscan_range_axis_mode;
  _rangeAxisMode = _requestedRangeAxisMode;
  
  _realtimeModeButton = NULL;
  _archiveModeButton = NULL;

  _timeSpanSecs = _params.bscan_time_span_secs;
  _archiveMode = _params.begin_in_archive_mode;
  _archiveRetrievalPending = false;
  _archiveTimeBox = NULL;
  _archiveStartTimeEdit = NULL;
  _archiveEndTimeEcho = NULL;

  _archiveStartTime.set(_params.archive_start_time);

  _archiveImagesStartTime.set(_params.images_archive_start_time);
  _archiveImagesEndTime.set(_params.images_archive_end_time);

  _dwellSpecsBox = NULL;
  _dwellAutoBox = NULL;
  _dwellAutoVal = NULL;
  _dwellSpecifiedEdit = NULL;
  _dwellSpecifiedFrame = NULL;
  _dwellStatsComboBox = NULL;

  // set up windows
  
  _setupWindows();

  // set initial field to 0
  
  _changeField(0, false);

}

// destructor

BscanManager::~BscanManager()

{

  if (_bscan) {
    delete _bscan;
  }

}

//////////////////////////////////////////////////
// Run

int BscanManager::run(QApplication &app)
{

  if (_params.debug) {
    cerr << "Running in BSCAN mode" << endl;
  }

  // make window visible

  show();
  
  // set timer running
  
  _beamTimerId = startTimer(5);
  
  return app.exec();

}

////////////////////////////////////////////////////////////////////////
// Set the label in the title bar.

void BscanManager::_setTitleBar(const string &radarName)
{
  string windowTitle = "HAWK_EYE BSCAN -- " + radarName;
  setWindowTitle(tr(windowTitle.c_str()));
}

//////////////////////////////////////////////////
// set up windows and widgets
  
void BscanManager::_setupWindows()
{

  // set up windows

  _main = new QFrame(this);
  setCentralWidget(_main);
  
  // bscan - main window

  _bscanFrame = new QFrame(_main);
  _bscanFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // configure the BSCAN

  _bscan = new BscanWidget(_bscanFrame, *this, _params, _fields.size());
  connect(this, SIGNAL(frameResized(const int, const int)),
	  _bscan, SLOT(resize(const int, const int)));
  
  // connect slots for location change
  
  connect(_bscan, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_bscanLocationClicked(double, double, const RadxRay*)));
  
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
  mainLayout->addWidget(_bscanFrame);
  //   mainLayout->addWidget(_colorBar);

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

  _createClickReportDialog();

  // create the range axis settings dialog

  _createRangeAxisDialog();

  // create the time axis settings dialog
  
  _createTimeAxisDialog();

}

void BscanManager::_createMenus()
{

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_saveImageAct);
  _fileMenu->addAction(_exitAct);

  menuBar()->addAction(_rangeAxisAct);
  menuBar()->addAction(_timeAxisAct);

  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _overlaysMenu->addAction(_rangeGridAct);
  _overlaysMenu->addAction(_timeGridAct);
  _overlaysMenu->addAction(_instHtLineAct);
  _overlaysMenu->addAction(_latlonLegendAct);
  _overlaysMenu->addAction(_speedTrackLegendAct);
  _overlaysMenu->addAction(_distScaleAct);

  menuBar()->addAction(_showClickAct);
  
  menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_clearAct);
  menuBar()->addAction(_unzoomAct);
  menuBar()->addAction(_refreshAct);

  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addSeparator();
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}

void BscanManager::_createActions()
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
  connect(_rangeAxisAct, SIGNAL(triggered()), this, SLOT(_showRangeAxisDialog()));

  // set time axis settings

  _timeAxisAct = new QAction(tr("Time-Config"), this);
  _timeAxisAct->setStatusTip(tr("Set configuration for time axis"));
  connect(_timeAxisAct, SIGNAL(triggered()), this, SLOT(_showTimeAxisDialog()));

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
  connect(_clearAct, SIGNAL(triggered()), _bscan, SLOT(clear()));

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
	  _bscan, SLOT(setRangeGridEnabled(bool)));

  // show time grid lines

  _timeGridAct = new QAction(tr("Time Grid"), this);
  _timeGridAct->setStatusTip(tr("Turn time grid on/off"));
  _timeGridAct->setCheckable(true);
  connect(_timeGridAct, SIGNAL(triggered(bool)),
	  _bscan, SLOT(setTimeGridEnabled(bool)));
  
  // show instrument height line in altitude display

  _instHtLineAct = new QAction(tr("Instrument Ht Line"), this);
  _instHtLineAct->setStatusTip(tr("Turn instrument height line on/off"));
  _instHtLineAct->setCheckable(true);
  connect(_instHtLineAct, SIGNAL(triggered(bool)),
	  _bscan, SLOT(setInstHtLineEnabled(bool)));

  // show latlon legend

  _latlonLegendAct = new QAction(tr("Starting lat/lon legend"), this);
  _latlonLegendAct->setStatusTip(tr("Display starting lat/lon as a legend"));
  _latlonLegendAct->setCheckable(true);
  connect(_latlonLegendAct, SIGNAL(triggered(bool)),
	  _bscan, SLOT(setLatlonLegendEnabled(bool)));

  // show dist/track legend

  _speedTrackLegendAct = new QAction(tr("Mean speed/track legend"), this);
  _speedTrackLegendAct->setStatusTip(tr("Display mean speed and track as a legend"));
  _speedTrackLegendAct->setCheckable(true);
  connect(_speedTrackLegendAct, SIGNAL(triggered(bool)),
	  _bscan, SLOT(setSpeedTrackLegendEnabled(bool)));

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

void BscanManager::_initActions()
{

  if (_params.bscan_draw_range_grid_lines) {
    _rangeGridAct->setChecked(true);
  } else {
    _rangeGridAct->setChecked(false);
  }

  if (_params.bscan_draw_time_grid_lines) {
    _timeGridAct->setChecked(true);
  } else {
    _timeGridAct->setChecked(false);
  }

  if (_params.bscan_draw_instrument_height_line) {
    _instHtLineAct->setChecked(true);
  } else {
    _instHtLineAct->setChecked(false);
  }

  if (_params.bscan_plot_starting_latlon_as_legend) {
    _latlonLegendAct->setChecked(true);
  } else {
    _latlonLegendAct->setChecked(false);
  }

  if (_params.bscan_plot_mean_track_and_speed_as_legend) {
    _speedTrackLegendAct->setChecked(true);
  } else {
    _speedTrackLegendAct->setChecked(false);
  }

  if (_params.bscan_add_distance_to_time_axis) {
    _distScaleAct->setChecked(true);
  } else {
    _distScaleAct->setChecked(false);
  }

}

///////////////////////////////////////////////////////
// configure the plot axes

void BscanManager::_configureAxes()
  
{
  
  _bscan->configureAxes(_requestedRangeAxisMode,
                        _minPlotRangeKm,
                        _maxPlotRangeKm,
                        _minPlotAltitudeKm,
                        _maxPlotAltitudeKm,
                        _timeSpanSecs,
                        _archiveMode);

}

///////////////////////////////////////////////////////
// create the range axis settings dialog
//
// This allows the user to control the range axis

void BscanManager::_createRangeAxisDialog()
{
  
  _rangeAxisDialog = new QDialog(this);
  _rangeAxisDialog->setWindowTitle("Range axis settings");
  
  QBoxLayout *rangeAxisDialogLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, _rangeAxisDialog);
  
  // range axis scale mode
  
  {

    _rangeAxisModeBox = new QGroupBox(_rangeAxisDialog);
    QVBoxLayout *modeBoxLayout = new QVBoxLayout;
    _rangeAxisModeBox->setLayout(modeBoxLayout);
    _rangeAxisModeBox->setTitle("Set mode for range axis");
    
    _rangeAxisModeUpButton = new QRadioButton(tr("Plot range up"), this);
    _rangeAxisModeUpButton->setStatusTip
      (tr("Plot range from instrument, positive upwards"));
    _rangeAxisModeUpButton->setCheckable(true);
    connect(_rangeAxisModeUpButton, SIGNAL(clicked()),
            this, SLOT(_setRangeAxisRangeUp()));
    modeBoxLayout->addWidget(_rangeAxisModeUpButton);
    
    _rangeAxisModeDownButton = new QRadioButton(tr("Plot range down"), this);
    _rangeAxisModeDownButton->setStatusTip
      (tr("Plot range from instrument, positive downwards"));
    _rangeAxisModeDownButton->setCheckable(true);
    connect(_rangeAxisModeDownButton, SIGNAL(clicked()), 
            this, SLOT(_setRangeAxisRangeDown()));
    modeBoxLayout->addWidget(_rangeAxisModeDownButton);
    
    _rangeAxisModeAltitudeButton = new QRadioButton(tr("Plot altitude"), this);
    _rangeAxisModeAltitudeButton->setStatusTip(tr("Plot altitude on vertical axis"));
    _rangeAxisModeAltitudeButton->setCheckable(true);
    connect(_rangeAxisModeAltitudeButton, SIGNAL(clicked()), 
            this, SLOT(_setRangeAxisAltitude()));
    modeBoxLayout->addWidget(_rangeAxisModeAltitudeButton);
    
    QButtonGroup *modeGroup = new QButtonGroup(this);
    modeGroup->setExclusive(true);
    modeGroup->addButton(_rangeAxisModeUpButton);
    modeGroup->addButton(_rangeAxisModeDownButton);
    modeGroup->addButton(_rangeAxisModeAltitudeButton);
    
    switch (_params.bscan_range_axis_mode) {
      case Params::RANGE_AXIS_DOWN:
        _rangeAxisModeDownButton->setChecked(true);
        break;
      case Params::RANGE_AXIS_ALTITUDE:
        _rangeAxisModeAltitudeButton->setChecked(true);
        break;
      case Params::RANGE_AXIS_UP:
      default:
        _rangeAxisModeUpButton->setChecked(true);
    }
    
    rangeAxisDialogLayout->addWidget(_rangeAxisModeBox, Qt::AlignCenter);
    
  }
  
  // set altitude limits

  {
    
    _rangeAxisAltitudeBox = new QGroupBox(_rangeAxisDialog);
    QVBoxLayout *altitudeLayout = new QVBoxLayout;
    _rangeAxisAltitudeBox->setLayout(altitudeLayout);
    _rangeAxisAltitudeBox->setTitle("Set limits for altitude plotting mode");
    
    _altitudeInFeetBox = new QCheckBox(_rangeAxisAltitudeBox);
    _altitudeInFeetBox->setText("Label altitude in feet?");
    _altitudeInFeetBox->setChecked(_altitudeInFeet);
    altitudeLayout->addWidget(_altitudeInFeetBox);
    connect(_altitudeInFeetBox, SIGNAL(clicked()), this, SLOT(_setAltitudeInFeet()));
    
    char textMin[1024], textMax[1024];
    sprintf(textMin, "%g", _minPlotAltitudeKm);
    sprintf(textMax, "%g", _maxPlotAltitudeKm);
    if (_altitudeInFeet) {
      _minAltitudeEdit =
        _addInputRow(_rangeAxisAltitudeBox, altitudeLayout,
                     "Min altitude (kft)", textMin, 0, &_minAltitudeLabel);
      _maxAltitudeEdit =
        _addInputRow(_rangeAxisAltitudeBox, altitudeLayout,
                     "Max altitude (kft)", textMin, 0, &_maxAltitudeLabel);
    } else {
      _minAltitudeEdit =
        _addInputRow(_rangeAxisAltitudeBox, altitudeLayout,
                     "Min altitude (km)", textMin, 0, &_minAltitudeLabel);
      _maxAltitudeEdit =
        _addInputRow(_rangeAxisAltitudeBox, altitudeLayout,
                     "Max altitude (km)", textMin, 0, &_maxAltitudeLabel);
    }
    connect(_minAltitudeEdit, SIGNAL(returnPressed()), this, SLOT(_setAltitudeLimits()));
    connect(_maxAltitudeEdit, SIGNAL(returnPressed()), this, SLOT(_setAltitudeLimits()));

    QFrame *acceptCancel = new QFrame;
    QHBoxLayout *horiz = new QHBoxLayout;
    acceptCancel->setLayout(horiz);
    
    _altitudeAccept = new QPushButton(acceptCancel);
    _altitudeAccept->setText("Accept");
    horiz->addWidget(_altitudeAccept);
    connect(_altitudeAccept, SIGNAL(clicked()), this, SLOT(_setAltitudeLimits()));
    connect(_altitudeAccept, SIGNAL(clicked()), this, SLOT(_refresh()));
    
    _altitudeCancel = new QPushButton(acceptCancel);
    _altitudeCancel->setText("Cancel");
    horiz->addWidget(_altitudeCancel);
    connect(_altitudeCancel, SIGNAL(clicked()), this, SLOT(_cancelRangeAxisChanges()));

    altitudeLayout->addWidget(acceptCancel);
    
    _rangeAxisSetAltitudeFromZoom = new QPushButton(_rangeAxisAltitudeBox);
    _rangeAxisSetAltitudeFromZoom->setText("Set limits from current zoom");
    altitudeLayout->addWidget(_rangeAxisSetAltitudeFromZoom);
    connect(_rangeAxisSetAltitudeFromZoom, SIGNAL(clicked()), this,
            SLOT(_setAltitudeLimitsFromZoom()));
    
    _rangeAxisResetAltitudeLimits = new QPushButton(_rangeAxisAltitudeBox);
    _rangeAxisResetAltitudeLimits->setText("Reset to defaults");
    altitudeLayout->addWidget(_rangeAxisResetAltitudeLimits);
    connect(_rangeAxisResetAltitudeLimits, SIGNAL(clicked()), this,
            SLOT(_resetAltitudeLimitsToDefaults()));

    rangeAxisDialogLayout->addWidget(_rangeAxisAltitudeBox, Qt::AlignCenter);

  }
  
  // set range limits if appropriate

  {
    
    _rangeAxisRangeBox = new QGroupBox(_rangeAxisDialog);
    QVBoxLayout *rangeLayout = new QVBoxLayout;
    _rangeAxisRangeBox->setLayout(rangeLayout);
    _rangeAxisRangeBox->setTitle("Option to set limits for range");
    
    _specifyRangeLimitsBox = new QCheckBox(_rangeAxisRangeBox);
    _specifyRangeLimitsBox->setText("Specify range limits?");
    _specifyRangeLimitsBox->setChecked(_specifyRangeLimits);
    rangeLayout->addWidget(_specifyRangeLimitsBox);
    connect(_specifyRangeLimitsBox, SIGNAL(clicked()),
            this, SLOT(_setSpecifyRangeLimits()));

    char text[1024];
    sprintf(text, "%g", _minPlotRangeKm);
    QLabel *minRangeLabel;
    _minRangeEdit = _addInputRow(_rangeAxisRangeBox, rangeLayout, 
                                 "Min range (km)", text, 0, &minRangeLabel);
    sprintf(text, "%g", _minPlotRangeKm);
    QLabel *maxRangeLabel;
    _maxRangeEdit = _addInputRow(_rangeAxisRangeBox, rangeLayout,
                                 "Max range (km)", text, 0, &maxRangeLabel);
    
    QFrame *acceptCancel = new QFrame;
    QHBoxLayout *horiz = new QHBoxLayout;
    acceptCancel->setLayout(horiz);

    _rangeAccept = new QPushButton(acceptCancel);
    _rangeAccept->setText("Accept");
    horiz->addWidget(_rangeAccept);
    connect(_rangeAccept, SIGNAL(clicked()), this, SLOT(_setRangeLimits()));
    connect(_rangeAccept, SIGNAL(clicked()), this, SLOT(_refresh()));

    _rangeCancel = new QPushButton(acceptCancel);
    _rangeCancel->setText("Cancel");
    horiz->addWidget(_rangeCancel);
    connect(_rangeCancel, SIGNAL(clicked()), 
            this, SLOT(_cancelRangeAxisChanges()));

    rangeLayout->addWidget(acceptCancel);

    _rangeAxisSetRangeFromZoom = new QPushButton(_rangeAxisRangeBox);
    _rangeAxisSetRangeFromZoom->setText("Set limits from current zoom");
    rangeLayout->addWidget(_rangeAxisSetRangeFromZoom);
    connect(_rangeAxisSetRangeFromZoom, SIGNAL(clicked()),
            this, SLOT(_setRangeLimitsFromZoom()));
    
    _rangeAxisResetRangeLimits = new QPushButton(_rangeAxisRangeBox);
    _rangeAxisResetRangeLimits->setText("Reset to defaults");
    rangeLayout->addWidget(_rangeAxisResetRangeLimits);
    connect(_rangeAxisResetRangeLimits, SIGNAL(clicked()), 
            this, SLOT(_resetRangeLimitsToDefaults()));

    rangeAxisDialogLayout->addWidget(_rangeAxisRangeBox, Qt::AlignCenter);

  }
  
  // censor data below surface

  {
    
    _censorDataBelowSurfaceBox = new QGroupBox(_rangeAxisDialog);
    QVBoxLayout *censorBelowSurfaceLayout = new QVBoxLayout;
    _censorDataBelowSurfaceBox->setLayout(censorBelowSurfaceLayout);
    _censorDataBelowSurfaceBox->setTitle("Option to censor data below surface");
    
    _censorDataToggleBox = new QCheckBox(_censorDataBelowSurfaceBox);
    _censorDataToggleBox->setText("Censor data below surface?");
    _censorDataToggleBox->setChecked(_censorDataBelowSurface);
    censorBelowSurfaceLayout->addWidget(_censorDataToggleBox);
    connect(_censorDataToggleBox, SIGNAL(clicked()),
            this, SLOT(_setCensorDataBelowSurface()));
    
    char text[1024];

    sprintf(text, "%s", _surfaceField.c_str());
    QLabel *surfaceFieldLabel;
    _surfaceFieldEdit =
      _addInputRow(_censorDataBelowSurfaceBox, censorBelowSurfaceLayout,
                   "Surface field name", text, 0, &surfaceFieldLabel);
    
    sprintf(text, "%g", _minRangeToSurfaceKm);
    QLabel *minRangeToSurfaceLabel;
    _minRangeToSurfaceEdit =
      _addInputRow(_censorDataBelowSurfaceBox, censorBelowSurfaceLayout,
                   "Min range to surface (km)", text, 0, &minRangeToSurfaceLabel);

    sprintf(text, "%g", _surfaceRangeMarginKm);
    QLabel *surfaceRangeMarginLabel;
    _surfaceRangeMarginEdit =
      _addInputRow(_censorDataBelowSurfaceBox, censorBelowSurfaceLayout,
                   "Surafce range margin (km)", text, 0, &surfaceRangeMarginLabel);
    
    QFrame *acceptCancel = new QFrame;
    QHBoxLayout *horiz = new QHBoxLayout;
    acceptCancel->setLayout(horiz);
    
    _censorDataBelowSurfaceAccept = new QPushButton(acceptCancel);
    _censorDataBelowSurfaceAccept->setText("Accept");
    horiz->addWidget(_censorDataBelowSurfaceAccept);
    connect(_censorDataBelowSurfaceAccept, SIGNAL(clicked()),
            this, SLOT(_setCensorDataBelowSurface()));
    connect(_censorDataBelowSurfaceAccept, SIGNAL(clicked()),
            this, SLOT(_refresh()));

    _censorDataBelowSurfaceCancel = new QPushButton(acceptCancel);
    _censorDataBelowSurfaceCancel->setText("Cancel");
    horiz->addWidget(_censorDataBelowSurfaceCancel);
    connect(_censorDataBelowSurfaceCancel, SIGNAL(clicked()),
            this, SLOT(_cancelCensorDataBelowSurfaceChanges()));
    
    censorBelowSurfaceLayout->addWidget(acceptCancel);

    _rangeAxisResetCensorDataBelowSurface = 
      new QPushButton(_censorDataBelowSurfaceBox);
    _rangeAxisResetCensorDataBelowSurface->setText("Reset to defaults");
    censorBelowSurfaceLayout->addWidget(_rangeAxisResetCensorDataBelowSurface);
    connect(_rangeAxisResetCensorDataBelowSurface, SIGNAL(clicked()), 
            this, SLOT(_resetCensorDataBelowSurfaceToDefaults()));

    rangeAxisDialogLayout->addWidget(_censorDataBelowSurfaceBox, Qt::AlignCenter);

  }
  
  // done?

  {
    
    _rangeAxisDoneBox = new QGroupBox(_rangeAxisDialog);
    QGridLayout *doneLayout = new QGridLayout;
    _rangeAxisDoneBox->setLayout(doneLayout);
    _rangeAxisDoneBox->setTitle("Done with all changes");
    
    int row = 0;
    QPushButton *done = new QPushButton(_rangeAxisDoneBox);
    done->setText("Done");
    doneLayout->addWidget(done, row++, 0, Qt::AlignCenter);
    connect(done, SIGNAL(clicked()), this, SLOT(_doneWithRangeAxis()));

    rangeAxisDialogLayout->addWidget(_rangeAxisDoneBox, Qt::AlignCenter);

  }

  _refreshRangeAxisDialog();
  
}

///////////////////////////////////////////////////////
// set the state on the range axis dialog

void BscanManager::_refreshRangeAxisDialog()
{
  
  char text[1024];

  // set altitude limits text

  sprintf(text, "%g", _minPlotAltitudeKm * _altitudeUnitsMult);
  _minAltitudeEdit->setText(text);

  sprintf(text, "%g", _maxPlotAltitudeKm * _altitudeUnitsMult);
  _maxAltitudeEdit->setText(text);

  _altitudeInFeetBox->setChecked(_altitudeInFeet);

  // set range limits text

  sprintf(text, "%g", _minPlotRangeKm);
  _minRangeEdit->setText(text);

  sprintf(text, "%g", _maxPlotRangeKm);
  _maxRangeEdit->setText(text);

  // enable/disable widgets depending on mode

  switch (_requestedRangeAxisMode) {
    case Params::RANGE_AXIS_UP:
    case Params::RANGE_AXIS_DOWN: {
      // _rangeAxisRangeBox->setEnabled(true);
      _rangeAxisAltitudeBox->setEnabled(false);
      break;
    }
    case Params::RANGE_AXIS_ALTITUDE:
    default: {
      // _rangeAxisRangeBox->setEnabled(false);
      _rangeAxisAltitudeBox->setEnabled(true);
      break;
    }
  }

  _specifyRangeLimitsBox->setChecked(_specifyRangeLimits);
  
  if (_specifyRangeLimits) {
    _minRangeEdit->setEnabled(true);
    _maxRangeEdit->setEnabled(true);
  } else {
    _minRangeEdit->setEnabled(false);
    _maxRangeEdit->setEnabled(false);
  }

  // set censor below surface text
  
  _censorDataToggleBox->setChecked(_censorDataBelowSurface);

  sprintf(text, "%s", _surfaceField.c_str());
  _surfaceFieldEdit->setText(text);

  sprintf(text, "%g", _minRangeToSurfaceKm);
  _minRangeToSurfaceEdit->setText(text);

  sprintf(text, "%g", _surfaceRangeMarginKm);
  _surfaceRangeMarginEdit->setText(text);

}

///////////////////////////////////////////////////////
// create the time axis settings dialog
//
// This allows the user to control the time axis

void BscanManager::_createTimeAxisDialog()
{
  
  _timeAxisDialog = new QDialog(this);
  _timeAxisDialog->setWindowTitle("Time axis settings");
  
  QBoxLayout *timeAxisDialogLayout =
    new QBoxLayout(QBoxLayout::TopToBottom, _timeAxisDialog);
  
  {  // set time span

    QGroupBox *timeSpanBox = new QGroupBox(_timeAxisDialog);
    QVBoxLayout *timeSpanLayout = new QVBoxLayout;
    timeSpanBox->setLayout(timeSpanLayout);
    timeSpanBox->setTitle("Set time span for plot");
    
    QFrame *timeSpanEditLabel;
    _timeSpanEdit = _addInputRow(timeSpanBox, timeSpanLayout, 
                                 "Time span for plot (secs)", "",
                                 0, &timeSpanEditLabel);
    _resetTimeSpanToDefault();
    
    QFrame *acceptCancelReset = new QFrame;
    QHBoxLayout *horiz = new QHBoxLayout;
    acceptCancelReset->setLayout(horiz);
    
    QPushButton *acceptButton = new QPushButton(timeSpanBox);
    acceptButton->setText("Accept");
    horiz->addWidget(acceptButton);
    connect(acceptButton, SIGNAL(clicked()), this, SLOT(_setTimeSpan()));
    
    QPushButton *cancelButton = new QPushButton(timeSpanBox);
    cancelButton->setText("Cancel");
    horiz->addWidget(cancelButton);
    connect(cancelButton, SIGNAL(clicked()), this,
            SLOT(_cancelTimeAxisChanges()));
    
    QPushButton *resetButton = new QPushButton(timeSpanBox);
    resetButton->setText("Reset to default");
    horiz->addWidget(resetButton);
    connect(resetButton, SIGNAL(clicked()), this,
            SLOT(_resetTimeSpanToDefault()));

    timeSpanLayout->addWidget(acceptCancelReset);

    // add to main dialog

    timeAxisDialogLayout->addWidget(timeSpanBox, Qt::AlignCenter);

  } // time span
  
  {  // archive / realtime mode
  
    QGroupBox *modeBox = new QGroupBox(_timeAxisDialog);
    QHBoxLayout *modeBoxLayout = new QHBoxLayout;
    modeBox->setLayout(modeBoxLayout);
    modeBox->setTitle("Set data retrieval mode");
    
    _realtimeModeButton = new QRadioButton(tr("Realtime mode"), this);
    _realtimeModeButton->setStatusTip(tr("Run in realtime mode"));
    _realtimeModeButton->setCheckable(true);
    connect(_realtimeModeButton, SIGNAL(clicked()), this,
            SLOT(_setDataRetrievalMode()));
    modeBoxLayout->addWidget(_realtimeModeButton);
    
    _archiveModeButton = new QRadioButton(tr("Archive mode"), this);
    _archiveModeButton->setStatusTip(tr("Run in archive mode"));
    _archiveModeButton->setCheckable(true);
    connect(_archiveModeButton, SIGNAL(clicked()), this,
            SLOT(_setDataRetrievalMode()));
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
    
    timeAxisDialogLayout->addWidget(modeBox, Qt::AlignCenter);
    
  } // archive / realtime mode
  
  {  // set archival time retrieval

    // box for setting start timew

    _archiveTimeBox = new QGroupBox(_timeAxisDialog);
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
    
    _archiveEndTimeEcho = new QLabel(_archiveTimeBox);
    _setArchiveEndTime();
    timeEndLayout->addWidget(_archiveEndTimeEcho);
    
    archiveTimeLayout->addWidget(timeEndFrame);

    // back / forward
    
    QFrame *backFwd = new QFrame;
    QHBoxLayout *layout1 = new QHBoxLayout;
    backFwd->setLayout(layout1);
    
    QPushButton *back5 = new QPushButton(backFwd);
    back5->setText("Back <<");
    layout1->addWidget(back5);
    connect(back5, SIGNAL(clicked()), this, SLOT(_goBack5()));
    
    QPushButton *back1 = new QPushButton(backFwd);
    back1->setText("Back <");
    layout1->addWidget(back1);
    connect(back1, SIGNAL(clicked()), this, SLOT(_goBack1()));
    
    QPushButton *fwd1 = new QPushButton(backFwd);
    fwd1->setText("Fwd >");
    layout1->addWidget(fwd1);
    connect(fwd1, SIGNAL(clicked()), this, SLOT(_goFwd1()));
    
    QPushButton *fwd5 = new QPushButton(backFwd);
    fwd5->setText("Fwd >>");
    layout1->addWidget(fwd5);
    connect(fwd5, SIGNAL(clicked()), this, SLOT(_goFwd5()));
    
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
    connect(goButton, SIGNAL(clicked()), this, 
            SLOT(_performArchiveRetrieval()));
    
    QPushButton *cancelButton = new QPushButton(goCancelReset);
    cancelButton->setText("Cancel");
    layout2->addWidget(cancelButton);
    connect(cancelButton, SIGNAL(clicked()), this,
            SLOT(_cancelTimeAxisChanges()));
    
    QPushButton *resetButton = new QPushButton(goCancelReset);
    resetButton->setText("Reset to default");
    layout2->addWidget(resetButton);
    connect(resetButton, SIGNAL(clicked()), this,
            SLOT(_setArchiveStartTimeToDefault()));

    archiveTimeLayout->addWidget(goCancelReset);

    // add to main dialog

    timeAxisDialogLayout->addWidget(_archiveTimeBox, Qt::AlignCenter);

  }  // set archival time retrieval
  
  {  // Dwell specifications

    _setDwellToDefaults();
    
    _dwellSpecsBox = new QGroupBox(_timeAxisDialog);
    QVBoxLayout *dwellLayout = new QVBoxLayout;
    _dwellSpecsBox->setLayout(dwellLayout);
    _dwellSpecsBox->setTitle("Set dwell specifications");

    // set the dwell automatically from window width and time span

    QFrame *dwellAutoFrame = new QFrame;
    QHBoxLayout *dwellAutoLayout = new QHBoxLayout;
    dwellAutoFrame->setLayout(dwellAutoLayout);
    
    _dwellAutoBox = new QCheckBox(_dwellSpecsBox);
    _dwellAutoBox->setText("Dwell auto (secs)?");
    _dwellAutoBox->setChecked(_dwellAuto);
    dwellAutoLayout->addWidget(_dwellAutoBox);
    connect(_dwellAutoBox, SIGNAL(clicked()), this,
            SLOT(_setDwellAuto()));
    
    _dwellAutoVal = new QLabel();
    dwellAutoLayout->addWidget(_dwellAutoVal);
     _setDwellAutoVal();

    dwellLayout->addWidget(dwellAutoFrame);

    // specify the dwell

    _dwellSpecifiedEdit = _addInputRow(_dwellSpecsBox, dwellLayout,
                                       "Set the dwell (secs)", "", 0,
                                       &_dwellSpecifiedFrame);
    _resetDwellSpecifiedToDefault();
    connect(_dwellSpecifiedEdit, SIGNAL(returnPressed()), this,
            SLOT(_setDwellSpecified()));

    // set the auto selection

    _setDwellAuto();

    // dwell stats method

    QFrame *statsFrame = new QFrame;
    QHBoxLayout *statsLayout = new QHBoxLayout;
    statsFrame->setLayout(statsLayout);
    
    QLabel *statsLabel = new QLabel(statsFrame);
    statsLabel->setText("Dwell stats method");
    statsLayout->addWidget(statsLabel);

    _dwellStatsComboBox = new QComboBox(_dwellSpecsBox);
    _dwellStatsComboBox->addItem("Mean");
    _dwellStatsComboBox->addItem("Median");
    _dwellStatsComboBox->addItem("Maximum");
    _dwellStatsComboBox->addItem("Minimum");
    _dwellStatsComboBox->addItem("Middle");
    statsLayout->addWidget(_dwellStatsComboBox);
    connect(_dwellStatsComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(_setDwellStats()));

    dwellLayout->addWidget(statsFrame);
    _setDwellToDefaults();
    
    // add to main dialog

    timeAxisDialogLayout->addWidget(_dwellSpecsBox, Qt::AlignCenter);

  } // dwell
  
  // done?
  
  {
    
    QGroupBox *doneBox = new QGroupBox(_timeAxisDialog);
    QGridLayout *doneLayout = new QGridLayout;
    doneBox->setLayout(doneLayout);
    doneBox->setTitle("Done with all changes");

    int row = 0;
    QPushButton *done = new QPushButton(doneBox);
    done->setText("Done");
    doneLayout->addWidget(done, row++, 0, Qt::AlignCenter);
    connect(done, SIGNAL(clicked()), this, SLOT(_cancelTimeAxisChanges()));
    
    timeAxisDialogLayout->addWidget(doneBox, Qt::AlignCenter);

  } // done
  
  _refreshTimeAxisDialog();
  
}

///////////////////////////////////////////////////////
// set the state on the time axis dialog

void BscanManager::_refreshTimeAxisDialog()
{
  
  char text[1024];

  // set altitude limits text

  sprintf(text, "%g", _timeSpanSecs);
  _timeSpanEdit->setText(text);

}

//////////////////////////////////////////////////////////////
// override timer event to respond to timer
  
void BscanManager::timerEvent(QTimerEvent *event)
{


  // register with procmap

  PMU_auto_register("timerEvent");
  
  // Handle widget stuff that can't be done at initial setup.  For some reason
  // the widget sizes are off until we get to this point.  There's probably
  // a better way to do this, but I couldn't figure anything out.
  
  if (_firstTimerEvent) {

    _bscan->resize(_bscanFrame->width(), _bscanFrame->height());
    
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

  // check for image creation

  if (_params.images_auto_create) {

    // if we are just creating files in archive mode and then exiting, do that now
    
    if (_params.images_creation_mode == Params::CREATE_IMAGES_THEN_EXIT ||
        _params.images_creation_mode == Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
      _createArchiveImageFiles();
      close();
      return;
    }
    
    // if we are creating files in realtime mode, do that now
    
    if (_params.images_creation_mode == Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
      _handleRealtimeData();
      _createRealtimeImageFiles();
      return;
    }

  }
  
  // handle event

  if (event->timerId() == _beamTimerId) {

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

void BscanManager::resizeEvent(QResizeEvent *event)
{
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "resizeEvent" << endl;
  }
  emit frameResized(_bscanFrame->width(), _bscanFrame->height());
}

////////////////////////////////////////////////////////////////
// override key press event

void BscanManager::keyPressEvent(QKeyEvent * e)
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
	cerr << "  field name: " << field->getName() << endl;
      }
      QRadioButton *button = (QRadioButton *) _fieldGroup->button(ifield);
      button->click();
      break;
    }

  }

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

void BscanManager::_handleRealtimeData()

{

  // do nothing if freeze is on

  if (_frozen) {
    return;
  }

  // get all available beams
  
  while (true) {
    
    // get the next ray from the reader queue
    // responsibility for this ray memory passes to
    // this (the master) thread
    
    RadxRay *ray = _reader->getNextRay(_platform);
    if (ray == NULL) {
      return; // no pending rays
    }
    
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "  Got a ray, time, el, az: "
           << DateTime::strm(ray->getTimeSecs()) << ", "
           << ray->getElevationDeg() << ", "
           << ray->getAzimuthDeg() << endl;
    }
    
    RadxTime thisRayTime = ray->getRadxTime();
    double timeSincePrev = thisRayTime - _readerRayTime;
    if (timeSincePrev > 0 &&
        timeSincePrev < (_params.bscan_min_secs_between_reading_beams)) {
      // discard
      if (_params.debug >= Params::DEBUG_EXTRA) {
        cerr << "  Discarding ray, not enough elapsed time" << endl;
      }
      delete ray;
      continue;
    } else if (timeSincePrev < 0) {
      // gone back in time, so reset times
      _imagesScheduledTime.set(RadxTime::ZERO);
    }
    _readerRayTime = thisRayTime;

    // compute altitude rate every 2 secs

    if (_prevAltKm > -9990) {
      double deltaTime = ray->getRadxTime() - _prevAltTime;
      if (deltaTime > 2.0) {
        if (ray->getGeoreference()) {
          double altKm = ray->getGeoreference()->getAltitudeKmMsl();
          double deltaAltKm = altKm - _prevAltKm;
          _altRateMps = (deltaAltKm / deltaTime) * 1000.0;
          _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
        }
        _prevAltTime = ray->getRadxTime();
      }
    } else {
      if (ray->getGeoreference()) {
        _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      }
      _prevAltTime = ray->getRadxTime();
    }
    
    // update the status panel
    
    _updateStatusPanel(ray);

    // draw the beam
    
    if (_params.images_creation_mode != Params::CREATE_IMAGES_ON_REALTIME_SCHEDULE) {
      _handleRay(ray);
    }
    
  } // while (true)

}

///////////////////////////////////////
// handle data in archive mode

void BscanManager::_handleArchiveData()

{

  // set up plot times

  _plotStartTime = _archiveStartTime;
  _plotEndTime = _archiveEndTime;

  // erase plot and set time axis

  _bscan->setPlotStartTime(_plotStartTime, true);
  _bscan->activateArchiveRendering();

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

int BscanManager::_getArchiveData()

{

  // set up file object for reading
  
  RadxFile file;
  _vol.clear();
  _setupVolRead(file);

  if (_params.debug) {
    cerr << "----------------------------------------------------" << endl;
    cerr << "perform archive retrieval" << endl;
    cerr << "  archive start time: " << _archiveStartTime.asString() << endl;
    cerr << "  archive end time: " << _archiveEndTime.asString() << endl;
    cerr << "  dwell secs: " << _dwellSecs << endl;
    cerr << "  dwell stats method: "
         << RadxField::statsMethodToStr(_dwellStatsMethod) << endl;
    cerr << "----------------------------------------------------" << endl;
  }
  
  if (file.readFromDir(_params.archive_data_url, _vol)) {
    string errMsg = "ERROR - Cannot retrieve archive data\n";
    errMsg += "BscanManager::_getArchiveData\n";
    errMsg += file.getErrStr() + "\n";
    errMsg += "  start time: " + _archiveStartTime.asString() + "\n";
    errMsg += "  end time: " + _archiveEndTime.asString() + "\n";
    char text[1024];
    sprintf(text, "  dwell secs: %g\n", _dwellSecs);
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

  return 0;

}

/////////////////////////////
// plot data in archive mode

void BscanManager::_plotArchiveData()

{

  if(_params.debug) {
    cerr << "======== Plotting archive data =======================" << endl;
    cerr << "======>>   plotStartTime: " << _plotStartTime.asString(3) << endl;
    cerr << "======>>   plotEndTime: " << _plotEndTime.asString(3) << endl;
  }

  // initialize plotting

  _initialRay = true;

  // handle the rays
  
  const vector<RadxRay *> &rays = _vol.getRays();
  if (rays.size() < 1) {
    cerr << "ERROR - _plotArchiveData" << endl;
    cerr << "  No rays found" << endl;
  }
  
  for (size_t ii = 0; ii < rays.size(); ii++) {
    RadxRay *ray = rays[ii];
    _handleRay(ray);
  }

  // update the status panel
  
  _updateStatusPanel(rays[0]);
    
}

//////////////////////////////////////////////////
// set up read

void BscanManager::_setupVolRead(RadxFile &file)
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

  _dwellSecs = _dwellSpecifiedSecs;
  if (_dwellAuto) {
    _dwellSecs = _dwellAutoSecs;
  }
  
  file.setReadRaysInInterval(_archiveStartTime, _archiveEndTime,
                             _dwellSecs, _dwellStatsMethod);

}

//////////////////////////////////////////////////////////////
// handle an incoming ray

void BscanManager::_handleRay(const RadxRay *ray)
  
{

  // do we need to reconfigure the BSCAN?

  int nGates = ray->getNGates();
  double minRange = ray->getStartRangeKm() - ray->getGateSpacingKm() / 2.0;
  double maxRange = minRange + nGates * ray->getGateSpacingKm();
  
  if (fabs(_rayMinRangeKm - minRange) > 0.001 ||
      fabs(_rayMaxRangeKm - maxRange) > 0.001) {
    _rayMinRangeKm = minRange;
    _rayMaxRangeKm = maxRange;
    if (_bscan->getRangeAxisMode() != Params::RANGE_AXIS_ALTITUDE && !_specifyRangeLimits) {
      _minPlotRangeKm = _rayMinRangeKm;
      _maxPlotRangeKm = _rayMaxRangeKm;
      _configureAxes();
    }
  }

  // get time

  RadxTime rayTime = ray->getRadxTime();

  // add to ray vector
  
  _rays.push_back(ray);

  // in realtime mode, set up initial plot time window
  
  if (!_archiveMode) {
    if (_plotStart || (rayTime < _plotStartTime)) {
      if (_params.bscan_truncate_start_time) {
        _plotStartTime.set(ray->getTimeSecs());
      } else {
        _plotStartTime.set(ray->getTimeSecs(), ray->getNanoSecs() / 1.0e9);
      }
      _plotEndTime = _plotStartTime + _timeSpanSecs;
      _bscan->setPlotStartTime(_plotStartTime);
      _bscan->clear();
      _plotStart = false;
    }
  }

  // first ray in series, or after unfreezing display?
    
  if (_initialRay) {
    _prevRayTime.set(ray->getTimeSecs(), ray->getNanoSecs() / 1.0e9);
    _initialRay = false;
  }

  _addRay(ray);

  // in realtime mode, do we need to move time domain?

  if (!_archiveMode && (rayTime > _plotEndTime)) {

    if(_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "======== Moving time domain =======================" << endl;
      cerr << "======>> rayTime: " << rayTime.asString(3) << endl;
      cerr << "======>> plotStartTime: " << _plotStartTime.asString(3) << endl;
      cerr << "======>> plotEndTime: " << _plotEndTime.asString(3) << endl;
    }

    // move domain
    
    double secsSaved = _params.bscan_realtime_fraction_saved * _timeSpanSecs;
    double secsDeleted = floor(_timeSpanSecs - secsSaved);
    
    _plotStartTime += secsDeleted;
    _plotEndTime += secsDeleted;
    
    // reset the start time on the widget, which will release unwanted beams

    _bscan->resetPlotStartTime(_plotStartTime);
    
  }

}

////////////////////////////////////////////////////////////////

void BscanManager::_addRay(const RadxRay *ray)

{

  RadxTime rayTime(ray->getTimeSecs(), ray->getNanoSecs() / 1.0e9);
  double halfDwellTime = (rayTime - _prevRayTime) / 2.0;
  RadxTime rayStartTime(rayTime - halfDwellTime);
  RadxTime rayEndTime(rayTime + halfDwellTime);
  _prevRayTime = rayTime;

  // create 2D field data vector

  vector< vector<double> > fieldData;
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    vector<double> field;
    fieldData.push_back(field);
  }

  // do we censor?

  bool doCensoring = false;
  double censorMinRange = 0.0;
  double censorMaxRange = 9999.0;
  
  // get surface range if required

  if (_censorDataBelowSurface) {
    double el = ray->getElevationDeg();
    // are we pointing at the ground?
    if (el < -45 || el > 225) {
      censorMaxRange = _getCensorRange(ray);
      doCensoring = true;
    }
  }
  
  if (_specifyRangeLimits) {
    if (censorMaxRange > _maxPlotRangeKm) {
      censorMaxRange = _maxPlotRangeKm;
    }
    censorMinRange = _minPlotRangeKm;
    doCensoring = true;
  }
  
  // fill data vector
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

    vector<double> &data = fieldData[ifield];

    RadxField *rfld = (RadxField *) ray->getField(_fields[ifield]->getName());
    if (rfld == NULL) {
      // fill with missing
      for (size_t igate = 0; igate < ray->getNGates(); igate++) {
        data.push_back(-9999);
      }
    } else {
      rfld->convertToFl32();
      const Radx::fl32 *fdata = rfld->getDataFl32();
      const Radx::fl32 missingVal = rfld->getMissingFl32();
      double range = ray->getStartRangeKm();
      double drange = ray->getGateSpacingKm();
      for (size_t igate = 0; igate < ray->getNGates(); igate++, fdata++, range += drange) {
        Radx::fl32 val = *fdata;
        if (doCensoring &&
            (range < censorMinRange || range > censorMaxRange)) {
          data.push_back(-9999);
        } else if (fabs(val - missingVal) < 0.0001) {
          data.push_back(-9999);
        } else {
          data.push_back(*fdata);
        }
      }
    }

  } // ifield

  // Add the beam to the display
  
  _bscan->addBeam(ray, _getInstHtKm(ray),
                  _plotStartTime, rayStartTime, rayEndTime,
                  fieldData, _fields);

}
  
/////////////////////////////////////////////////////////////////////  
// slots

///////////////////////////////////////////////////////////
// respond to change field request from field button group

void BscanManager::_changeField(int fieldId, bool guiMode)

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
  
  _bscan->selectVar(_fieldNum);

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

/////////////////////////////////////////////////////////
// respond to a change in click location on the BSCAN

void BscanManager::_bscanLocationClicked(double xsecs, double ykm,
                                         const RadxRay *closestRay)
  
{
  if (_params.debug) {
    cerr << "====>> location clicked - xsecs, ykm: " << xsecs << ", " << ykm << endl;
  }
  _xSecsClicked = xsecs;
  _yKmClicked = ykm;
  _rayClicked = closestRay;

  _locationClicked(_xSecsClicked, _yKmClicked, _rayClicked);

}

//////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void BscanManager::_locationClicked(double xsecs, double ykm, const RadxRay *ray)

{

  // check the ray

  if (ray == NULL) {
    return;
  }

  double range = 0.0, altitude = 0.0;
  double sinEl = sin(ray->getElevationDeg() * DEG_TO_RAD);

  if (_bscan->getRangeAxisMode() == Params::RANGE_AXIS_ALTITUDE) {
    
    altitude = ykm;
    range = (altitude - _getInstHtKm(ray)) / sinEl;
    
  } else {
    
    range = ykm;
    altitude = _getInstHtKm(ray) + range * sinEl;
    
  }

  int gate = (int) ((range - ray->getStartRangeKm()) / ray->getGateSpacingKm());

  if (gate < 0 || gate >= (int) ray->getNGates())
  {
    //user clicked outside of ray
    return;
  }
  
  if (_params.debug) {
    cerr << "Clicked on location: xsecs, ykm: " << xsecs << ", " << ykm << endl;
    cerr << "  range start, spacing: " << ray->getStartRangeKm() << ", "
         << ray->getGateSpacingKm() << endl;
    cerr << "  range, gate: " << range << ", " << gate << endl;
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

  if (fabs(ray->getElevationDeg()) < 10000) {
    _setText(text, "%6.2f (deg)", ray->getElevationDeg());
    _elevClicked->setText(text);
  }
  
  if (fabs(ray->getAzimuthDeg()) < 10000) {
    _setText(text, "%6.2f (deg)", ray->getAzimuthDeg());
    _azClicked->setText(text);
  }
    
  _setText(text, "%d", gate);
  _gateNumClicked->setText(text);
  
  _setText(text, "%6.2f (km)", range);
  _rangeClicked->setText(text);

  if (_altitudeInFeet) {
    _setText(text, "%6.2f (kft)", altitude * _altitudeUnitsMult);
  } else {
    _setText(text, "%6.2f (km)", altitude * _altitudeUnitsMult);
  }
  _altitudeClicked->setText(text);
  
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
          sprintf(text, "%g", val);
        }
        _fields[ii]->setDialogText(text);
      }
    } // ii

  } // ifield
  
  // set altitude rate if possible
  
  if (ray->getGeoreference()) {
    _altRateMps = ray->getGeoreference()->getVertVelocity();
  } else {
    _altRateMps = -9999.0;
  }
    
  // update the status panel
  
  _updateStatusPanel(ray);
    
}

/////////////////////////////////////
// show the range axis dialog

void BscanManager::_showRangeAxisDialog()
{
  if (_rangeAxisDialog) {
    if (_rangeAxisDialog->isVisible()) {
      _rangeAxisDialog->setVisible(false);
    } else {
      _refreshRangeAxisDialog();
      _rangeAxisDialog->setVisible(true);
      _rangeAxisDialog->raise();
      if (_rangeAxisDialog->x() == 0 &&
          _rangeAxisDialog->y() == 0) {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y() + (height() - _rangeAxisDialog->height()));
        _rangeAxisDialog->move(pos);
      }
    }
  }
}

/////////////////////////////////////
// show the time axis dialog

void BscanManager::_showTimeAxisDialog()
{

  // compute the auto dwell value

  _setDwellAutoVal();

  if (_timeAxisDialog) {
    if (_timeAxisDialog->isVisible()) {
      _timeAxisDialog->setVisible(false);
    } else {
      if (!_archiveMode) {
        _setArchiveStartTime(_plotStartTime);
      }
      _refreshTimeAxisDialog();
      _timeAxisDialog->setVisible(true);
      _timeAxisDialog->raise();
      if (_timeAxisDialog->x() == 0 &&
          _timeAxisDialog->y() == 0) {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y() + (height() - _timeAxisDialog->height()));
        _timeAxisDialog->move(pos);
      }
    }
  }
}

////////////////////////////////
// unzoom display

void BscanManager::_unzoom()
{
  _bscan->unzoomView();
  _unzoomAct->setEnabled(false);
}

////////////////////////////////////////////
// refresh

void BscanManager::_refresh()
{
  if (_archiveMode) {
    _performArchiveRetrieval();
  }
}

//////////////////////////////
// freeze display

void BscanManager::_freeze()
{
  if (_frozen) {
    _frozen = false;
    _freezeAct->setText("Freeze");
    _freezeAct->setStatusTip(tr("Click to freeze display, or hit ESC"));
    _initialRay = true;
  } else {
    _frozen = true;
    _freezeAct->setText("Unfreeze");
    _freezeAct->setStatusTip(tr("Click to unfreeze display, or hit ESC"));
  }
}

//////////////////////////////////////////////////
// enable the zoom button - called by BscanWidget

void BscanManager::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

//////////////////////////////////////////////////
// change range axis mode

void BscanManager::_setRangeAxisRangeUp()
{
  _requestedRangeAxisMode = Params::RANGE_AXIS_UP;
  _refreshRangeAxisDialog();
  _configureAxes();
}

void BscanManager::_setRangeAxisRangeDown()
{
  _requestedRangeAxisMode = Params::RANGE_AXIS_DOWN;
  _refreshRangeAxisDialog();
  _configureAxes();
}

void BscanManager::_setRangeAxisAltitude()
{
  _requestedRangeAxisMode = Params::RANGE_AXIS_ALTITUDE;
  _refreshRangeAxisDialog();
  _configureAxes();
}

void BscanManager::_cancelRangeAxisChanges()
{
  _rangeAxisDialog->setVisible(false);
  _refreshRangeAxisDialog();
}

void BscanManager::_doneWithRangeAxis()
{
  _rangeAxisDialog->setVisible(false);
}

//////////////////////////////////////////////////
// time axis changes

void BscanManager::_cancelTimeAxisChanges()
{
  _refreshTimeAxisDialog();
  _timeAxisDialog->setVisible(false);
}

////////////////////////////////////////////////////////
// change altitude limits

void BscanManager::_setAltitudeLimits()
{

  // limits

  double minAltitude;
  if (sscanf(_minAltitudeEdit->text().toLocal8Bit().data(), "%lg", &minAltitude) != 1) {
    QErrorMessage errMsg(_minAltitudeEdit);
    string text("Bad entry for min altitude: ");
    text += _minAltitudeEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetAltitudeLimitsToDefaults();
    return;
  }
  double maxAltitude;
  if (sscanf(_maxAltitudeEdit->text().toLocal8Bit().data(), "%lg", &maxAltitude) != 1) {
    QErrorMessage errMsg(_maxAltitudeEdit);
    string text("Bad entry for max altitude: ");
    text += _maxAltitudeEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetAltitudeLimitsToDefaults();
    return;
  }

  if (minAltitude > maxAltitude) {
    QErrorMessage errMsg(_maxAltitudeEdit);
    string text("Bad entry for min/max altitudes: ");
    text += _minAltitudeEdit->text().toLocal8Bit().data();
    text += " / ";
    text += _maxAltitudeEdit->text().toLocal8Bit().data();
    text += "  Max must exceed min";
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetAltitudeLimitsToDefaults();
    return;
  }

  _minPlotAltitudeKm = minAltitude / _altitudeUnitsMult;
  _maxPlotAltitudeKm = maxAltitude / _altitudeUnitsMult;

  _refreshRangeAxisDialog();

  // refresh

  _configureAxes();

  // _rangeAxisDialog->setVisible(false);

}

////////////////////////////////////////////////////
// set the altitude to be displayed in feet

void BscanManager::_setAltitudeInFeet()
{

  // units

  if (_altitudeInFeetBox->isChecked()) {
    _altitudeInFeet = true;
    _altitudeUnitsMult = 1.0 / 0.3048;
    _altitudeUnits = "kft";
    _altLabel->setText("Alt(kft)");
    _altRateLabel->setText("AltRate(ft/s)");
    _minAltitudeLabel->setText("Min altitude (kft)");
    _maxAltitudeLabel->setText("Max altitude (kft)");
  } else {
    _altitudeInFeet = false;
    _altitudeUnitsMult = 1.0;
    _altitudeUnits = "km";
    _altLabel->setText("Alt(km)");
    _altRateLabel->setText("AltRate(m/s)");
    _minAltitudeLabel->setText("Min altitude (km)");
    _maxAltitudeLabel->setText("Max altitude (km)");
  }
  _bscan->setAltitudeInFeet(_altitudeInFeet);

  _refreshRangeAxisDialog();

}

////////////////////////////////////////////////////
// set the altitude limits from the current zoom

void BscanManager::_setAltitudeLimitsFromZoom()
{
  if (_requestedRangeAxisMode == Params::RANGE_AXIS_ALTITUDE) {
    _minPlotAltitudeKm = _bscan->getZoomWorld().getYMinWorld();
    _maxPlotAltitudeKm = _bscan->getZoomWorld().getYMaxWorld();
  }
  _refreshRangeAxisDialog();
  _configureAxes();
}

////////////////////////////////////////////////////
// set the range limits from the current zoom

void BscanManager::_setRangeLimitsFromZoom()
{
  if (_requestedRangeAxisMode == Params::RANGE_AXIS_UP) {
    _minPlotRangeKm = _bscan->getZoomWorld().getYMinWorld();
    _maxPlotRangeKm = _bscan->getZoomWorld().getYMaxWorld();
    _specifyRangeLimits = true;
  } else {
    _minPlotRangeKm = _bscan->getZoomWorld().getYMaxWorld();
    _maxPlotRangeKm = _bscan->getZoomWorld().getYMinWorld();
    _specifyRangeLimits = true;
  }
  _refreshRangeAxisDialog();
  _configureAxes();
                          
}

///////////////////////////////////////////////////////
// set altitude limits to defaults

void BscanManager::_setAltitudeLimitsToDefaults()
{
  _altitudeInFeet = _params.bscan_altitude_in_feet;
  if (_altitudeInFeet) {
    _altitudeUnits = "kft";
    _altitudeUnitsMult = 1.0 / 0.3048;
  } else {
    _altitudeUnits = "km";
    _altitudeUnitsMult = 1.0;
  }
  _minPlotAltitudeKm = _params.bscan_min_altitude_km;
  _maxPlotAltitudeKm = _params.bscan_max_altitude_km;
}

void BscanManager::_resetAltitudeLimitsToDefaults()
{
  _setAltitudeLimitsToDefaults();
  _altitudeInFeetBox->setChecked(_altitudeInFeet);
  _setAltitudeInFeet();
  _refreshRangeAxisDialog();
  _configureAxes();
}

///////////////////////////////////////////////////////
// set censor below surface to defaults

void BscanManager::_setCensorDataBelowSurfaceToDefaults()
{
  _censorDataBelowSurface = _params.bscan_censor_data_below_surface;
  _surfaceField = _params.bscan_surface_field;
  _minRangeToSurfaceKm = _params.bscan_min_range_to_surface_km;
  _surfaceRangeMarginKm = _params.bscan_surface_range_margin_km;
}

void BscanManager::_resetCensorDataBelowSurfaceToDefaults()
{
  _setCensorDataBelowSurfaceToDefaults();
  _refreshRangeAxisDialog();
}

void BscanManager::_cancelCensorDataBelowSurfaceChanges()
{
  _refreshRangeAxisDialog();
}

////////////////////////////////////////////////////////
// set censor below surface

void BscanManager::_setCensorDataBelowSurface()
{

  _censorDataBelowSurface = _censorDataToggleBox->isChecked();
    
  _surfaceField = _surfaceFieldEdit->text().toLocal8Bit().data();

  double minRange;
  if (sscanf(_minRangeToSurfaceEdit->text().toLocal8Bit().data(), 
             "%lg", &minRange) != 1) {
    QErrorMessage errMsg(_minRangeToSurfaceEdit);
    string text("Bad entry for min range: ");
    text += _minRangeToSurfaceEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _setCensorDataBelowSurfaceToDefaults();
    return;
  }
  _minRangeToSurfaceKm = minRange;

  double rangeMargin;
  if (sscanf(_surfaceRangeMarginEdit->text().toLocal8Bit().data(),
             "%lg", &rangeMargin) != 1) {
    QErrorMessage errMsg(_surfaceRangeMarginEdit);
    string text("Bad entry for range margin: ");
    text += _surfaceRangeMarginEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _setCensorDataBelowSurfaceToDefaults();
    return;
  }
  _surfaceRangeMarginKm = rangeMargin;

  _surfaceFieldEdit->setEnabled(_censorDataBelowSurface);
  _minRangeToSurfaceEdit->setEnabled(_censorDataBelowSurface);
  _surfaceRangeMarginEdit->setEnabled(_censorDataBelowSurface);

}

////////////////////////////////////////////////////////
// change range limits

void BscanManager::_setRangeLimits()
{
  double minRange;
  if (sscanf(_minRangeEdit->text().toLocal8Bit().data(),
             "%lg", &minRange) != 1) {
    QErrorMessage errMsg(_minRangeEdit);
    string text("Bad entry for min range: ");
    text += _minRangeEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetRangeLimitsToDefaults();
    return;
  }
  double maxRange;
  if (sscanf(_maxRangeEdit->text().toLocal8Bit().data(),
             "%lg", &maxRange) != 1) {
    QErrorMessage errMsg(_maxRangeEdit);
    string text("Bad entry for max range: ");
    text += _maxRangeEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetRangeLimitsToDefaults();
    return;
  }
  _minPlotRangeKm = minRange;
  _maxPlotRangeKm = maxRange;
  // _rangeAxisDialog->setVisible(false);
  _configureAxes();
}

void BscanManager::_setSpecifyRangeLimits()
{
  if (_specifyRangeLimitsBox->isChecked()) {
    _specifyRangeLimits = true;
  } else {
    _specifyRangeLimits = false;
  }
  _refreshRangeAxisDialog();
  _configureAxes();
}

void BscanManager::_resetRangeLimitsToDefaults()
{
  _setRangeLimitsToDefaults();
  _refreshRangeAxisDialog();
  
  // _rangeAxisDialog->setVisible(false);
  _configureAxes();
}

void BscanManager::_setRangeLimitsToDefaults()
{
  _specifyRangeLimits = _params.bscan_specify_range_limits;
  _rayMinRangeKm = _params.bscan_min_range_km;
  _rayMaxRangeKm = _params.bscan_max_range_km;
  _minPlotRangeKm = _rayMinRangeKm;
  _maxPlotRangeKm = _rayMaxRangeKm;
}

////////////////////////////////////////////////////////
// change range limits

void BscanManager::_setDistScaleEnabled()
{
  bool enabled = _distScaleAct->isChecked();
  _bscan->setDistScaleEnabled(enabled);
  _configureAxes();
}

////////////////////////////////////////////////////////
// change time span

void BscanManager::_setTimeSpan()
{

  double timeSpan;
  if (sscanf(_timeSpanEdit->text().toLocal8Bit().data(), 
             "%lg", &timeSpan) != 1) {
    QErrorMessage errMsg(_timeSpanEdit);
    string text("Bad entry for time span: ");
    text += _timeSpanEdit->text().toLocal8Bit().data();
    errMsg.setModal(true);
    errMsg.showMessage(text.c_str());
    errMsg.exec();
    _resetTimeSpanToDefault();
    return;
  }
  
  _timeSpanSecs = timeSpan;
  _plotEndTime = _plotStartTime + _timeSpanSecs;
    
  _setArchiveEndTime();
  _setDwellAutoVal();
  _configureAxes();

  // if (_archiveMode) {
  //   _performArchiveRetrieval();
  // }

}

void BscanManager::_resetTimeSpanToDefault()
{
  _timeSpanSecs = _params.bscan_time_span_secs;
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

void BscanManager::_setStartTimeFromGui(const QDateTime &datetime1)
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

void BscanManager::_setGuiFromStartTime()
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

void BscanManager::_setArchiveStartTimeToDefault()

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

void BscanManager::_setArchiveStartTime(const RadxTime &rtime)

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

void BscanManager::_setArchiveEndTime()

{

  _archiveEndTime = _archiveStartTime + _timeSpanSecs;
  if (_archiveEndTimeEcho) {
    _archiveEndTimeEcho->setText(_archiveEndTime.asString(3).c_str());
  }

}


////////////////////////////////////////////////////////
// change modes

void BscanManager::_setDataRetrievalMode()
{
  if (!_archiveTimeBox || !_dwellSpecsBox) {
    return;
  }
  if (_realtimeModeButton && _realtimeModeButton->isChecked()) {
    if (_archiveMode) {
      _archiveMode = false;
      _archiveTimeBox->setEnabled(false);
      _dwellSpecsBox->setEnabled(false);
      _bscan->activateRealtimeRendering();
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
      _bscan->activateArchiveRendering();
    }
  }
  _configureAxes();
}

////////////////////////////////////////////////////////
// change start time

void BscanManager::_goBack1()
{
  _archiveStartTime -= 1 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void BscanManager::_goBack5()
{
  _archiveStartTime -= 5 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void BscanManager::_goFwd1()
{
  _archiveStartTime += 1 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void BscanManager::_goFwd5()
{
  _archiveStartTime += 5 * _timeSpanSecs;
  _setGuiFromStartTime();
}

void BscanManager::_changeRange(int deltaGates)
{
  if (!_bscan->getPointClicked()) {
    return;
  }
  if (_requestedRangeAxisMode == Params::RANGE_AXIS_DOWN) {
    deltaGates *= -1;
  }
  _yKmClicked += deltaGates * _rayClicked->getGateSpacingKm();
  _locationClicked(_xSecsClicked, _yKmClicked, _rayClicked);
  _bscan->setMouseClickPoint(_xSecsClicked, _yKmClicked);
}

////////////////////////////////////////////////////////
// perform archive retrieval

void BscanManager::_performArchiveRetrieval()
{
  _archiveRetrievalPending = true;
}

////////////////////////////////////////////////////////
// set dwell to defaults

void BscanManager::_setDwellToDefaults()

{

  _dwellAuto = _params.bscan_archive_dwell_auto;
  _dwellSpecifiedSecs = _params.bscan_archive_dwell_secs;

  int index = 0;
  switch (_params.bscan_dwell_stats) {
    case Params::DWELL_STATS_MEAN:
      _dwellStatsMethod = RadxField::STATS_METHOD_MEAN;
      index = 0;
      break;
    case Params::DWELL_STATS_MEDIAN:
      _dwellStatsMethod = RadxField::STATS_METHOD_MEDIAN;
      index = 1;
      break;
    case Params::DWELL_STATS_MAXIMUM:
      _dwellStatsMethod = RadxField::STATS_METHOD_MAXIMUM;
      index = 2;
      break;
    case Params::DWELL_STATS_MINIMUM:
      _dwellStatsMethod = RadxField::STATS_METHOD_MINIMUM;
      index = 3;
      break;
    case Params::DWELL_STATS_MIDDLE:
    default:
      _dwellStatsMethod = RadxField::STATS_METHOD_MIDDLE;
      index = 4;
      break;
  }

  if (_dwellStatsComboBox) {
    _dwellStatsComboBox->setCurrentIndex(index);
  }

}

//////////////////////////////////////
// set the specified dwell

void BscanManager::_setDwellSpecified()

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

void BscanManager::_resetDwellSpecifiedToDefault()

{
  _dwellSpecifiedSecs = _params.bscan_archive_dwell_secs;
  char text[1024];
  sprintf(text, "%g", _dwellSpecifiedSecs);
  _dwellSpecifiedEdit->setText(text);
}


//////////////////////////////////////
// set the dwell from auto selection

void BscanManager::_setDwellAuto()

{
  if (_dwellAutoBox) {
    _dwellAuto = _dwellAutoBox->isChecked();
  } else {
    _dwellAuto = _params.bscan_archive_dwell_auto;
  }
  if (_dwellSpecifiedFrame) {
    _dwellSpecifiedFrame->setEnabled(!_dwellAuto);
  }
}

void BscanManager::_setDwellAutoVal()

{
  if (_dwellAutoVal) {
    _configureAxes();
    const WorldPlot &world = _bscan->getFullWorld();
    double nPixelsPlot = world.getPlotWidth();
    _dwellAutoSecs = _timeSpanSecs / nPixelsPlot;
    char text[128];
    sprintf(text, "%lg", _dwellAutoSecs);
    _dwellAutoVal->setText(text);
  }
}

///////////////////////////////////////////
// set dwell stats from combo box selection

void BscanManager::_setDwellStats()
{
  if (_dwellStatsComboBox) {
    int index = _dwellStatsComboBox->currentIndex();
    switch (index) {
      case 0:
        _dwellStatsMethod = RadxField::STATS_METHOD_MEAN;
        break;
      case 1:
        _dwellStatsMethod = RadxField::STATS_METHOD_MEDIAN;
        break;
      case 2:
        _dwellStatsMethod = RadxField::STATS_METHOD_MAXIMUM;
        break;
      case 3:
        _dwellStatsMethod = RadxField::STATS_METHOD_MINIMUM;
        break;
      case 4:
      default:
        _dwellStatsMethod = RadxField::STATS_METHOD_MIDDLE;
        break;
    }
  }
}

///////////////////////////////////////////
// get range at which to censor the data

double BscanManager::_getCensorRange(const RadxRay *ray)

{

  double censorRange = 9999.0;

  // get surface field

  const RadxField *cfld = ray->getField(_params.bscan_surface_field);
  if (cfld == NULL) {
    return censorRange;
  }

  // find range to max

  double maxVal = -1.0e99;
  double range = cfld->getStartRangeKm();
  double drange = cfld->getGateSpacingKm();
  Radx::fl32 missing = cfld->getMissingFl32();
  const Radx::fl32 *vals = cfld->getDataFl32();
  for (size_t ii = 0; ii < cfld->getNPoints(); ii++, range += drange) {
    if (range < _minRangeToSurfaceKm) {
      continue;
    }
    Radx::fl32 val = vals[ii];
    if (val != missing) {
      if (val > maxVal) {
        maxVal = val;
        censorRange = range;
      }
    }
  }

  // no max found - all data missing

  if (censorRange > 9990) {
    return censorRange;
  }

  // increment range by margin

  censorRange += _surfaceRangeMarginKm;

  // now check field values below the surface

  int igate = (int) ((censorRange - cfld->getStartRangeKm()) / 
                     cfld->getGateSpacingKm() + 0.5);
  for (size_t ii = igate; ii < cfld->getNPoints(); ii++) {
    Radx::fl32 val = vals[ii];
    if (val != missing) {
      if (val > _params.bscan_max_field_val_below_surface) {
        // found large field value below the surface
        // so surface location is not reliable
        // no censoring
        return 9999.0;
      }
    }
  }

  return censorRange;

}

/////////////////////////////////////////////////////
// save image to file
// If interactive is true, use dialog boxes to indicate errors or report
// where the image was saved.

void BscanManager::_saveImageToFile(bool interactive)
{

  // create image
  
  QPixmap pixmap = QPixmap::grabWidget(_bscan);
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
    cerr << "ERROR - BscanManager::_saveImageToFile()" << endl;
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
    cerr << "ERROR - BscanManager::_saveImageToFile()" << endl;
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
      cerr << "ERROR - BscanManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params.images_write_latest_data_info)

}

/////////////////////////////////////////////////////
// creating image files in realtime mode

void BscanManager::_createRealtimeImageFiles()
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
    _archiveStartTime = _archiveEndTime - _timeSpanSecs;
    _createImageFiles();

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

void BscanManager::_createArchiveImageFiles()
{

  if (_params.images_creation_mode ==
      Params::CREATE_IMAGES_THEN_EXIT) {

    _createImageFiles();

  } else if (_params.images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {

    for (time_t stime = _archiveImagesStartTime.utime();
         stime <= _archiveImagesEndTime.utime();
         stime += _params.images_schedule_interval_secs) {
      
      _archiveStartTime.set(stime);
      _archiveEndTime = _archiveStartTime + _timeSpanSecs;

      _createImageFiles();
      
    } // stime

  } // if (_params.images_creation_mode ...

}

/////////////////////////////////////////////////////
// creating one image per field

void BscanManager::_createImageFiles()
{

  if (_params.debug) {
    cerr << "BscanManager::_createImageFiles()" << endl;
  }

  PMU_auto_register("createImageFiles");

  // retrieve data

  _setDwellAutoVal();
  _handleArchiveData();

  // save current field

  int fieldNum = _fieldNum;
  
  // loop through fields

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    
    // select field
    
    _changeField(ii, false);
    _bscan->update();
    
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

void BscanManager::_howto()
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

