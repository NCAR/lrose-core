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
// PolarManager.cc
//
// Polar Manager object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2014
//
///////////////////////////////////////////////////////////////
//
// PolarManager manages polar rendering - PPIs and RHIs
// Manages top level Menus
// Connects actions to Controllers
// Transfers actions from Menus to top level Controllers
// FieldPanelController
// SweepController
// PpiWidget/Controller
// SpreadsheetController
// ScriptController
//
///////////////////////////////////////////////////////////////

#include "PolarManager.hh"
#include "DisplayField.hh"
#include "FieldListView.hh"
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
#include <algorithm>    // std::find
#include <Ncxx/H5x.hh>
#include <QActionGroup>
#include <QApplication>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QDir>
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
#include <QStringListModel>
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

#include <toolsa/toolsa_macros.h>
#include <toolsa/Path.hh>

#include "CloseEventFilter.hh"
using namespace std;
using namespace H5x;


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

PolarManager::PolarManager(DisplayFieldController *displayFieldController,
                           bool haveFilteredFields) :
// DisplayManager(params, reader, displayFieldController, haveFilteredFields), 
        //_sweepManager(params),
        _rhiWindowDisplayed(false),
        QMainWindow(NULL),
        //_params->params),
        //_reader(reader),
        _initialRay(true),
        _displayFieldController(displayFieldController),
        _haveFilteredFields(haveFilteredFields)
{

	m_pInstance = this;

  // initialize

  // from DisplayManager ...
  //_beamTimerId = 0;
  // _frozen = false;
  //  _displayFieldController->setSelectedField(0);
  _prevFieldNum = -1;

  _radarLat = -9999.0;
  _radarLon = -9999.0;
  _radarAltKm = -9999.0;

  _altitudeInFeet = false;
  // end from DisplayManager    

  _firstTime = true;

  // setWindowIcon(QIcon("HawkEyePolarIcon.icns"));
  
  _prevAz = -9999.0;
  _prevEl = -9999.0;
  _startAz = -9999.0;
  _endAz = -9999.0;
  //_ppiRays = NULL;
  _rhiMode = false;

  _nGates = 1000;
  _maxRangeKm = 1.0;
  
  //_archiveRetrievalPending = false;
  
  _ppiFrame = NULL;
  _ppi = NULL;

  _rhiWindow = NULL;
  _rhi = NULL;

  //_sweepVBoxLayout = NULL;
  _sweepPanel = NULL;

  //_archiveStartTimeEdit = NULL;
  //_archiveEndTimeEdit = NULL;

  //_selectedTimeLabel = NULL;
  
  //_back1 = NULL;
  //_fwd1 = NULL;
  //_backPeriod = NULL;
  //_fwdPeriod = NULL;

  _timeNavController = NULL;
  _timeNavView = NULL;
  _timeControlPlaced = false;
  //_timeLayout = NULL;
  //_timeSlider = NULL;

  ParamFile *_params = ParamFile::Instance();

  _setArchiveMode(_params->begin_in_archive_mode);

  if (_timeNavController != NULL) {
    //_timeNavController->setArchiveStartTime(_params->archive_start_time);
    //_timeNavController->setArchiveEndTime(_archiveStartTime + _params->archive_time_span_secs);
    //_timeNavController->setArchiveScanIndex(0);
  }

  _imagesArchiveStartTime.set(_params->images_archive_start_time);
  _imagesArchiveEndTime.set(_params->images_archive_end_time);
  _imagesScanIntervalSecs = _params->images_scan_interval_secs;

  // set up ray locators

// move to RayLocModel
  //_ppiRays = new RayLoc[RayLoc::RAY_LOC_N];
  //_ppiRayLoc = _ppiRays + RayLoc::RAY_LOC_OFFSET;
  _rayLocationController = new RayLocationController();

  sheetView = NULL;
  scriptEditorView = NULL;
  boundaryPointEditorControl = NULL;
  boundaryPointEditorView = NULL;
  boundaryView = NULL;

  // set up windows

  _setupWindows();

  // install event filter to catch when the PolarManager is closed
  CloseEventFilter *closeFilter = new CloseEventFilter(this);
  installEventFilter(closeFilter);

  // set initial field to 0

  //_changeField(0, false);

  //connect(this, SIGNAL(newDataFile()), _displayFieldController, SLOT(dataFileChanged()));  
  //connect(this, SIGNAL(fieldSelected(string)), _displayFieldController, SLOT(fieldSelected(string))); 
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

  //if (_ppiRays) {
  //  delete[] _ppiRays;
  //}
  // TODO: delete all controllers
  if (_timeNavController) delete _timeNavController;
  if (_timeNavView) delete _timeNavView;
}

//////////////////////////////////////////////////
// Run

int PolarManager::run(QApplication &app)
{

  LOG(DEBUG) << "Running in POLAR mode";  

  // make window visible

  show();

  //if (_archiveFileList.size() == 0) {
  //  //getFileAndFields();
  //  _openFile();
  //}
 

  // set timer running
  
  //_beamTimerId = startTimer(2000);
  
  return app.exec();

}
/*
void PolarManager::setParamsFile() {
  _params = ParamFile::Instance();
}
*/
//////////////////////////////////////////////////
// enable the zoom button - called by PolarWidgets

void PolarManager::enableZoomButton() const
{
  _unzoomAct->setEnabled(true);
}

///////////////////////////////////////////////
// override resize event

void PolarManager::resizeEvent(QResizeEvent *event)
{
  LOG(DEBUG) << "resizeEvent: _ppiFrame width = " << 
  _ppiFrame->width() << 
  ", height = " << _ppiFrame->height(); //  << event;
  emit frameResized(_ppiFrame->width(), _ppiFrame->height());
}

////////////////////////////////////////////////////////////////
void PolarManager::keyPressEvent(QKeyEvent * e)
{

  // get key pressed

  Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();

  
    LOG(DEBUG) << "Clicked char: " << keychar << ":" << (int) keychar;
    //LOG(DEBUG) << "         key: " << hex << key << dec;
  
  
  // for '.', swap with previous field

  if (keychar == '.') {
    QRadioButton *button = (QRadioButton *) _fieldGroup->button(_prevFieldNum);
    button->click();
    return;
  }
  
  /* for ESC, freeze / unfreeze

  if (keychar == 27) {
    _freezeAct->trigger();
    return;
  }
  */

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

    	LOG(DEBUG) << "Short-cut key pressed: " << shortcut;
    	LOG(DEBUG) << "  field label: " << field->getLabel();
    	LOG(DEBUG) << "   field name: " << field->getName();
      
      QRadioButton *button = (QRadioButton *) _fieldGroup->button(ifield);
      button->click();
      break;
    }

  }

  // check for up/down in sweeps

  if (key == Qt::Key_Left) {

    LOG(DEBUG) << "Clicked left arrow, go back in time";

    //_ppi->setStartOfSweep(true);
    //_rhi->setStartOfSweep(true);
    //_goBack1();

  } else if (key == Qt::Key_Right) {

    LOG(DEBUG) << "Clicked right arrow, go forward in time";
    
    //_ppi->setStartOfSweep(true);
    //_rhi->setStartOfSweep(true);
    //_goFwd1();
    
  } else if (key == Qt::Key_Up) {

    //if (_sweepManager.getGuiIndex() > 0) {

      LOG(DEBUG) << "Clicked up arrow, go up a sweep";
      //_ppi->setStartOfSweep(true);
      //_rhi->setStartOfSweep(true);
      _changeSweepRadioButton(-1);

    //}

  } else if (key == Qt::Key_Down) {

    //if (_sweepManager.getGuiIndex() < (int) _sweepManager.getNSweeps() - 1) {

      LOG(DEBUG) << "Clicked down arrow, go down a sweep";
      
      //_ppi->setStartOfSweep(true);
      //_rhi->setStartOfSweep(true);
      _changeSweepRadioButton(+1);

    //}

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
  string windowTitle = "HAWK_EDIT -- " + radarName;
  setWindowTitle(tr(windowTitle.c_str()));
}
  
//////////////////////////////////////////////////
// set up windows and widgets 
// make signal and slot connections between controllers
  
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
  _ppiFrame->setLineWidth(3);
  _ppiFrame->setMidLineWidth(3);  
  _ppiFrame->setFrameStyle(QFrame::Box);
  // configure the PPI

 // _ppi = new PpiWidget(_ppiFrame, this, _platform, _displayFieldController, _haveFilteredFields);

  _ppi = new PolarWidget(_ppiFrame, this, _platform, 
    _displayFieldController, _haveFilteredFields,
    _rayLocationController);
  _ppiFrame->setMinimumSize(400,400);
  _ppi->setMinimumSize(400,400);


  connect(this, SIGNAL(frameResized(const int, const int)),
	  _ppi, SLOT(resize(const int, const int)));
  
  // Create the RHI window

  //_rhiWindow = new RhiWindow(this, _params-> _platform,
  //                           _displayFieldController, _haveFilteredFields);
  //_rhiWindow->setRadarName(_params->radar_name);

  // set pointer to the rhiWidget

  //_rhi = _rhiWindow->getWidget();
  
  // connect slots for location

  connect(_ppi, SIGNAL(locationClicked(double, double, const RadxRay*)),
          this, SLOT(_ppiLocationClicked(double, double, const RadxRay*)));
  //connect(_rhi, SIGNAL(locationClicked(double, double, const RadxRay*)),
  //        this, SLOT(_rhiLocationClicked(double, double, const RadxRay*)));

  // add a right-click context menu to the image
  setContextMenuPolicy(Qt::CustomContextMenu);
  // customContextMenuRequested(e->pos());
  connect(_ppi, SIGNAL(customContextMenuRequested(const QPoint &)),
	  this, SLOT(ShowContextMenu(const QPoint &)));

  // create status panel

  _createStatusPanel();

  // create fields panel
  
  //_createFieldPanel();
  _fieldPanel = new DisplayFieldView(); // _displayFieldController);
  //_displayFieldController->setView(_fieldPanel);
  _fieldPanel->createFieldPanel(_main);
 //TODO: can only connect QObjects with signals and slots...
  //connect(_fieldPanel, SIGNAL(selectedFieldChanged(QString)),
  //  this, SLOT(changeToField(QString)));

  connect(_fieldPanel, SIGNAL(selectedFieldChanged(QString)),
          this, SLOT(selectedFieldChanged(QString)));

  connect(_fieldPanel, SIGNAL(ShowParameterColorDialog(QString)),
    this, SLOT(ShowParameterColorDialog(QString)));  // call DisplayFieldController::deleteFieldFromVolume(DisplayField *field)
 // connect(_fieldPanel, SIGNAL(setFieldToMissing(QString)),
 //   this, SLOT(setFieldToMissing(QString)));


  //connect(this, SIGNAL(addField(QString)), 
  //  _displayFieldController, SLOT(addField(QString)));
  //connect(this, SIGNAL(addField(QString)), 
  //  _ppi, SLOT(addField(QString)));

  // add widgets

  mainLayout->addWidget(_statusPanel);
  mainLayout->addWidget(_fieldPanel); // <=== here 
  mainLayout->addWidget(_ppiFrame);
  _ppiFrame->show();
  _ppi->show();

  // sweep panel

   //_createSweepPanel();
  _sweepPanel = new SweepView(_main);
  _sweepController = new SweepController();
  _sweepController->setView(_sweepPanel);
   mainLayout->addWidget(_sweepPanel);

  connect(_sweepPanel, SIGNAL(selectedSweepChanged(double)),
          this, SLOT(selectedSweepChanged(double)));
  connect(this, SIGNAL(newDataFile()), this, SLOT(dataFileChanged()));
  //connect(this, SIGNAL(sweepSelected()), _sweepController, SLOT(sweepSelected()));

  // fill out menu bar

  _createActions();
  _createMenus();

  // title bar

  _params = ParamFile::Instance();

  _setTitleBar(_params->radar_name);
  setMinimumSize(400, 300);
  resize(1100,635); // _params->main_window_width, _params->main_window_height);
  
  // set location on screen

  QPoint pos;
  pos.setX(_params->main_window_start_x);
  pos.setY(_params->main_window_start_y);
  move(pos);
  
  // set up field status dialog

  _createClickReportDialog();

  //_createBoundaryEditorDialog();

 
   _setSweepPanelVisibility();

  // time panel

  _createTimeControl();
  _showTimeControl();
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

  _params = ParamFile::Instance();

  // freeze display
  //_freezeAct = new QAction(tr("Freeze"), this);
  //_freezeAct->setShortcut(tr("Esc"));
  //_freezeAct->setStatusTip(tr("Freeze display"));
  //connect(_freezeAct, SIGNAL(triggered()), this, SLOT(_freeze()));
  
  // show user click in dialog
  _showClickAct = new QAction(tr("Show Click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  connect(_showClickAct, SIGNAL(triggered()), this, SLOT(_showClick()));

  // show boundary editor dialog
  _showBoundaryEditorAct = new QAction(tr("Boundary Editor"), this);
  _showBoundaryEditorAct->setStatusTip(tr("Show boundary editor dialog"));
  connect(_showBoundaryEditorAct, SIGNAL(triggered()), this, SLOT(showBoundaryEditor()));
  
  // set time controller settings
  //_timeControllerAct = new QAction(tr("Time-Config"), this);
  //_timeControllerAct->setStatusTip(tr("Show time control window"));
  //connect(_timeControllerAct, SIGNAL(triggered()), this,
  //        SLOT(_showTimeControl()));

  // show time control window
  _showTimeControlAct = new QAction(tr("Show/hide time navigation"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, SIGNAL(triggered()), this,
          SLOT(_showTimeControl()));

  /* realtime mode
  _realtimeAct = new QAction(tr("Set realtime mode"), this);
  _realtimeAct->setStatusTip(tr("Turn realtime mode on/off"));
  _realtimeAct->setCheckable(true);
  _realtimeAct->setChecked(!_params->begin_in_archive_mode);
  connect(_realtimeAct, SIGNAL(triggered(bool)),
	  this, SLOT(_setRealtime(bool)));
  */

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
  _ringsAct->setChecked(_params->ppi_range_rings_on_at_startup);
  connect(_ringsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setRings(bool)));

  // show grids

  _gridsAct = new QAction(tr("Grids"), this);
  _gridsAct->setStatusTip(tr("Turn range grids on/off"));
  _gridsAct->setCheckable(true);
  _gridsAct->setChecked(_params->ppi_grids_on_at_startup);
  connect(_gridsAct, SIGNAL(triggered(bool)),
	  _ppi, SLOT(setGrids(bool)));

  // show azimuth lines

  _azLinesAct = new QAction(tr("Az Lines"), this);
  _azLinesAct->setStatusTip(tr("Turn range azLines on/off"));
  _azLinesAct->setCheckable(true);
  _azLinesAct->setChecked(_params->ppi_azimuth_lines_on_at_startup);
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

  // Solo Editing
  
  _examineAct = new QAction(tr("Examine SpreadSheet"), this);
  _examineAct->setStatusTip(tr("Solo Examine using a spreadsheet"));
  connect(_examineAct, SIGNAL(triggered()), this, SLOT(_examineSpreadSheetSetup()));

  _editAct = new QAction(tr("Edit Script"), this);
  _editAct->setStatusTip(tr("Solo Edit using (Java)Script"));
  connect(_editAct, SIGNAL(triggered()), this, SLOT(_scriptEditorSetup()));  

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

  _timeMenu = menuBar()->addMenu(tr("&Time"));
  _timeMenu->addAction(_showTimeControlAct);
  _timeMenu->addSeparator();
  //_timeMenu->addAction(_realtimeAct);

  _overlaysMenu = menuBar()->addMenu(tr("&Overlays"));
  _overlaysMenu->addAction(_ringsAct);
  _overlaysMenu->addAction(_gridsAct);
  _overlaysMenu->addAction(_azLinesAct);
  _overlaysMenu->addSeparator();
  _overlaysMenu->addAction(_showRhiAct);

  //menuBar()->addAction(_freezeAct);
  menuBar()->addAction(_showClickAct);

  _boundaryMenu = menuBar()->addMenu(tr("&Boundary"));
  _boundaryMenu->addAction(_showBoundaryEditorAct);
  
  menuBar()->addAction(_unzoomAct);
  menuBar()->addAction(_clearAct);

  _editMenu = menuBar()->addMenu(tr("&Edit"));
  _editMenu->addSeparator();
  _editMenu->addAction(_examineAct);
  _editMenu->addAction(_editAct);


  _helpMenu = menuBar()->addMenu(tr("&Help"));
  _helpMenu->addAction(_howtoAct);
  _helpMenu->addAction(_aboutAct);
  _helpMenu->addAction(_aboutQtAct);

}


///////////////////////////////////////////////////////////////
/* change sweep

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
      //_rhi->setStartOfSweep(true);
      _moveUpDown();
      return;
    }
  } // ii

}
*/

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
  LOG(DEBUG) << "-->> changing sweep index by increment: " << increment;
  
  if (increment != 0) {
    _sweepController->changeSelectedIndex(increment);
   // _sweepRButtons->at(_sweepManager.getGuiIndex())->setChecked(true);
  }

}

void PolarManager::deleteFieldFromVolume(QString fieldName) {
  _displayFieldController->deleteFieldFromVolume(fieldName.toStdString());
}

void PolarManager::setFieldToMissing(QString fieldName) {
  _displayFieldController->setFieldToMissing(fieldName.toStdString());
}

///////////////////////////////////////
// set input file list for archive mode

void PolarManager::setArchiveFileList(const vector<string> &list,
                                      bool fromCommandLine // = true 
)
// TODO: don't think fromCommandLine is used???
{

  if (list.size() <= 0) {
    errorMessage("Error", "Empty list of archive files");
    return;
  } 

/* done in TimeNavMVC classes
    // determine start and end time from file list
    RadxTime startTime, endTime;
    NcfRadxFile::getTimeFromPath(list[0], startTime);
    NcfRadxFile::getTimeFromPath(list[list.size()-1], endTime);
    // round to nearest five minutes
    time_t startTimeSecs = startTime.utime();
    startTimeSecs =  (startTimeSecs / 300) * 300;
    time_t endTimeSecs = endTime.utime();
    endTimeSecs =  (endTimeSecs / 300) * 300 + 300;
    //_archiveStartTime.set(startTimeSecs);
    //_archiveEndTime.set(endTimeSecs);
    //_archiveScanIndex = 0;
*/

  //_archiveFileList = list;
  //_setArchiveRetrievalPending();

  if (_timeNavController) {
    // move up two levels in the directory to find the top level
    // maybe like this ...
    //  .../toplevel/yyyymmdd/chosenfile
    QString fullUrl(QString(list.at(0).c_str()));
    QDir theDir(fullUrl);
    if (theDir.cd("../../")) {
      QString thePath = theDir.absolutePath();
      QFileInfo fileInfo(fullUrl);
      QString theFile = fileInfo.fileName();
      _timeNavController->fetchArchiveFiles(thePath.toStdString(),
        theFile.toStdString());
      //_timeNavController->fetchArchiveFiles
    }
  }

/*
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
*/
}
  
///////////////////////////////////////////////
// get archive file list by searching for files
// returns 0 on success, -1 on failure

int PolarManager::loadArchiveFileList()

{
  /*
  RadxTimeList timeList;
  timeList.setDir(_params->archive_data_url);
  timeList.setModeInterval(_archiveStartTime, _archiveEndTime);
  timeList.compile();
  _urlOK = true;

  if (timeList.getPathList().size() < 1) {
    cerr << "ERROR - PolarManager::loadArchiveFileList()" << endl;
    cerr << "  Cannot load file list for url: " 
         << _params->archive_data_url << endl;
    cerr << "  Start time: " << _archiveStartTime.getStr() << endl;
    cerr << "  End time: " << _archiveEndTime.getStr() << endl;
    _urlOK = false;
    return -1;

  }

  setArchiveFileList(timeList.getPathList(), false);
  */
  return 0;

}


///////////////////////////////////////
// handle data in archive mode

void PolarManager::_handleArchiveData()
{
    LOG(DEBUG) << "enter";

  _ppi->setArchiveMode(true);
  //_ppi->setStartOfSweep(true);

  //_rhi->setArchiveMode(true);
  //_rhi->setStartOfSweep(true);

  // set cursor to wait cursor

  this->setCursor(Qt::WaitCursor);
  //_timeNavController->setCursor(Qt::WaitCursor);
  
  // get data
  try {
    _getArchiveData();
    _setupRayLocation();
  } catch (FileIException &ex) {
    this->setCursor(Qt::ArrowCursor);
    //_timeNavController->setCursor(Qt::ArrowCursor);
    return;
  }
  
  //if (_vol.checkIsRhi()) {
  //  _rhiMode = true;
  //} else {
    _rhiMode = false;
  //}

  // plot the data
  
  _plotArchiveData();
  this->setCursor(Qt::ArrowCursor);
  //_timeNavController->setCursor(Qt::ArrowCursor);

  _activateArchiveRendering();

  if (_firstTime) {
    _firstTime = false;
  }
  
}

/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure

vector<string> *PolarManager::getFieldsArchiveData(string fileName)
{

  LOG(DEBUG) << "enter";

  // set up file object for reading
  
  RadxFile file;
  RadxVol vol;

  vol.clear();
  //_setupVolRead(file);
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params->debug >= Params::DEBUG_EXTRA) {
    file.setDebug(true);
    file.setVerbose(true);
  }
  file.setReadMetadataOnly(true);
  
  //if (_archiveScanIndex >= 0 &&
  //    _archiveScanIndex < (int) _archiveFileList.size()) {
    
    string inputPath = fileName; // _archiveFileList[_archiveScanIndex];
    
  
      LOG(DEBUG) << "  reading data file path: " << inputPath;
      //cerr << "  archive file index: " << _archiveScanIndex << endl;
    
    
    if (file.readFromPath(inputPath, vol)) {
      string errMsg = "ERROR - Cannot retrieve archive data\n";
      errMsg += "PolarManager::_getFieldsArchiveData\n";
      errMsg += file.getErrStr() + "\n";
      errMsg += "  path: " + inputPath + "\n";
      cerr << errMsg;
      if (!_params->images_auto_create)  {
        QErrorMessage errorDialog;
        errorDialog.setMinimumSize(400, 250);
        errorDialog.showMessage(errMsg.c_str());
        errorDialog.exec();
      }

    } 
    vol.loadFieldsFromRays();
    const vector<RadxField *> fields = vol.getFields();
    vector<string> *allFieldNames = new vector<string>;
    for (vector<RadxField *>::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      RadxField *field = *iter;
      cout << field->getName() << endl;
      allFieldNames->push_back(field->getName());
    }

    LOG(DEBUG) << "exit";
    return allFieldNames;
  //}
}


vector<string> *PolarManager::userSelectFieldsForReading(string fileName) {

  vector<string> *availableFields = getFieldsArchiveData(fileName);
  /*
    QStringListModel model; //  = new QStringListModel();
    QStringList list;
    list << "a" << "b" << "c";
    model.setStringList(list);
    QListView theList;
    theList.setModel(&model);
    theList.show();
  */

  FieldListView *listview = new FieldListView(); // this);
  listview->setList(availableFields);

  QGroupBox *viewBox = new QGroupBox(tr("Select fields to import"));
  QDialogButtonBox *buttonBox = new QDialogButtonBox;
  QPushButton *saveButton = buttonBox->addButton(QDialogButtonBox::Apply);
  QPushButton *closeButton = buttonBox->addButton(QDialogButtonBox::Cancel);


  connect(saveButton, SIGNAL(clicked(bool)), listview, SLOT(fieldsSelected(bool)));
  connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(closeFieldListDialog(bool)));
  connect(listview, SIGNAL(sendSelectedFieldsForImport(vector<string> *)),
    this, SLOT(fieldsSelected(vector<string> *)));
        //lve->show();
  QVBoxLayout* viewLayout = new QVBoxLayout;
  viewLayout->addWidget(listview);
  viewBox->setLayout(viewLayout);
  viewBox->setMinimumHeight(200);

  QHBoxLayout* horizontalLayout = new QHBoxLayout;
  horizontalLayout->addWidget(buttonBox);

  QVBoxLayout* mainLayout = new QVBoxLayout;
  mainLayout->addWidget(viewBox);
  mainLayout->addLayout(horizontalLayout);

  fieldListDialog = new QDialog(this);
  fieldListDialog->setModal(true);
  fieldListDialog->setLayout(mainLayout);

  fieldListDialog->exec();  // this halts the app until fields are selected

  return availableFields;

}


// TODO: I don't think this is used
void PolarManager::getFileAndFields() {
          QString inputPath = "/"; // QDir::currentPath();
        // get the path of the current file, if available 
        //if (_archiveFileList.size() > 0) {
        //  QDir temp(_archiveFileList[0].c_str());
        //  inputPath = temp.absolutePath();
        //} 

        QString filename =  QFileDialog::getOpenFileName(
                NULL,
                "Open Document",
                inputPath, "All files (*);; Cfradial (*.nc)");  //QDir::currentPath(),
        if (filename.isNull() || filename.isEmpty()) {
          return;
        }
        vector<string> fileList;
        fileList.push_back(filename.toStdString());

        // override archive data url from input file
        //string url = _getArchiveUrl(fileList[0]);
        //TDRP_str_replace(&_params->archive_data_url, url.c_str());

        // choose which fields to import

        vector<string> *allFieldNames = userSelectFieldsForReading(fileList[0]);
        for (size_t ii = 0; ii < allFieldNames->size(); ii++) {
          cerr << "ii, allFieldNames[ii]: "
               << ii << ", " << allFieldNames + ii << endl;
        }

        //setArchiveFileList(fileList, false);

}
/////////////////////////////
// get data in archive mode
// returns 0 on success, -1 on failure
// precondition: assumes setArchiveFileList is called before

int PolarManager::_getArchiveData()

{
/* moved to DataModel::readData()
  // set up file object for reading
  
  RadxFile file;
  _vol.clear();
  _setupVolRead(file);
  */
  bool debug_verbose = false;
  bool debug_extra = false;
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    debug_verbose = true;
  }
  if (_params->debug >= Params::DEBUG_EXTRA) {
    debug_verbose = true;
    debug_extra = true;
  }

  DataModel *dataModel = DataModel::Instance();

  // be sure to call setArchiveFileList before we get here! TODO: TimeNav now manages this;
  // may be able to remove this restriction
  //if (_archiveScanIndex >= 0 &&
  //    _archiveScanIndex < (int) _archiveFileList.size()) {
    
    try {
      string inputPath = _timeNavController->getSelectedArchiveFile();
      vector<string> fieldNames = _displayFieldController->getFieldNames();
      dataModel->readData(inputPath, fieldNames,
        debug_verbose, debug_extra);
      emit newDataFile();
    } catch (const string &errMsg) {
      if (!_params->images_auto_create)  {
        QErrorMessage errorDialog;
        errorDialog.setMinimumSize(400, 250);
        errorDialog.showMessage(errMsg.c_str());
        errorDialog.exec();
      }
    }
  //}

  // set plot times
  
  _plotStartTime = dataModel->getStartTimeSecs();
  _plotEndTime = dataModel->getEndTimeSecs();

  char text[128];
  snprintf(text, 128, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
           _plotStartTime.getYear(),
           _plotStartTime.getMonth(),
           _plotStartTime.getDay(),
           _plotStartTime.getHour(),
           _plotStartTime.getMin(),
           _plotStartTime.getSec());

    LOG(DEBUG) << "----------------------------------------------------";
    LOG(DEBUG) << "perform archive retrieval";
    LOG(DEBUG) << "  read file: " << dataModel->getPathInUse();
    LOG(DEBUG) << "  nSweeps: " << dataModel->getNSweeps();
   // LOG(DEBUG) << "  guiIndex, fixedAngle: " 
   //      << _sweepManager.getGuiIndex() << ", "
   //      << _sweepManager.getSelectedAngle();
    LOG(DEBUG) << "----------------------------------------------------";
  
  
  _platform = dataModel->getPlatform();

  LOG(DEBUG) << "exit";
  return 0;
}

double PolarManager::getSelectedSweepAngle() {
  double angle = _sweepController->getSelectedAngle();
  return angle;
}

size_t PolarManager::getSelectedFieldIndex() {
  size_t selectedFieldIndex = 0;
  try {
    selectedFieldIndex = _displayFieldController->getSelectedFieldNum();
  } catch (std::range_error &ex) {
      LOG(ERROR) << ex.what();
      QMessageBox::warning(NULL, "Error selectingField (_getArchiveData):", ex.what());
      return 0;
  }
  return selectedFieldIndex;
}




/////////////////////////////
// apply new, edited  data in archive mode
// the volume has been updated 



void PolarManager::_applyDataEdits()
{
  LOG(DEBUG) << "enter";
  if (0) { // _params->debug) {
    std::ofstream outfile("/tmp/voldebug_PolarManager_applyDataEdits.txt");
    _vol.printWithFieldData(outfile);  
    outfile << "_vol = " << &_vol << endl;
  }
  _setupRayLocation();
  _plotArchiveData();
  LOG(DEBUG) << "exit";
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
  
  //if (_rhi) {
  //  _rhi->addNewFields(newFields);
  //}
  LOG(DEBUG) << "exit";
}


void PolarManager::_addNewFields(vector<string> *newFieldNames)
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
  for (int i=0; i < newFieldNames->size(); ++i) {
    string name = newFieldNames->at(i); // .toLocal8Bit().constData();

    LOG(DEBUG) << name;
 
    ColorMap map(-20.0, 20.0, "default");

     // new DisplayField(pfld.label, pfld.raw_name, pfld.units,
      //           pfld.shortcut, map, ifield, false);
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
  
  //if (_rhi) {
  //  _rhi->addNewFields(newFields);
  //}
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


void PolarManager::selectedFieldChanged(QString newFieldName) {
  selectedFieldChanged(newFieldName.toStdString());
}

void PolarManager::selectedFieldChanged(string fieldName) {
  _displayFieldController->setSelectedField(fieldName);
  _plotArchiveData();
  refreshBoundaries();
}


void PolarManager::selectedSweepChanged(double angle) {
  //string fieldName = newFieldName.toStdString();
  //_displayFieldController->setSelectedField(fieldName);
  _sweepController->setAngle(angle);
  _setupRayLocation();
  _plotArchiveData();
  refreshBoundaries();
}

/*
RadxVol PolarManager::getDataVolume() {
  return _vol;
}
*/
/////////////////////////////
// plot the selected field and sweep
// call when the field is changed, or the sweep is changed
void PolarManager::_plotArchiveData()

{

    LOG(DEBUG) << "enter";
    LOG(DEBUG) << "  volume start time: " << _plotStartTime.asString();
  
  // initialize plotting

  _initialRay = true;

  // clear the canvas

  _clear();


  string currentFieldName = _displayFieldController->getSelectedFieldName();
  double currentSweepAngle = _sweepController->getSelectedAngle();

  ColorMap *colorMap = _displayFieldController->getColorMap(currentFieldName);
  
  string backgroundColorName = _displayFieldController->getBackgroundColor();
  QColor backgroundColor(backgroundColorName.c_str());

  _ppi->displayImage(currentFieldName, currentSweepAngle,
    *colorMap, backgroundColor); 

  LOG(DEBUG) << "exit";
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

    LOG(DEBUG) << "Updating archive data";
    LOG(DEBUG) << "  volume start time: " << _plotStartTime.asString();
  

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
  //_sweepManager.set(_vol);

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
/*
  const SweepManager::GuiSweep &gsweep = _sweepManager.getSelectedSweep();
  for (size_t ii = gsweep.radx->getStartRayIndex();
       ii <= gsweep.radx->getEndRayIndex(); ii++) {
    RadxRay *ray = rays[ii];
    _handleRayUpdate(_platform, ray, newFieldNamesConverted);  // TODO; _handleRay? or a modified version of it?  We just need something to call updateBeam, instead of addBeam ...
    //if (ii == 0) {
    //  _updateStatusPanel(ray);
    //}
  }
 */ 
}

void PolarManager::_updateColorMap(string fieldName) 
{

  
  //  LOG(DEBUG) << "Updating color map";
  
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
  //  _handleColorMapChangeOnRay(_platform, fieldName); 
    //}
  
}


//////////////////////////////////////////////////
// set up read

/* moved to DataModel
void PolarManager::_setupVolRead(RadxFile &file)
{

  if (_params->debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params->debug >= Params::DEBUG_EXTRA) {
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
*/
//////////////////////////////////////////////////////////////
// handle an incoming ray

void PolarManager::_handleRay(RadxPlatform &platform, RadxRay *ray)
  
{

  // do we need to reconfigure the PPI?

  _nGates = ray->getNGates();

  double maxRange = ray->getStartRangeKm() + _nGates * ray->getGateSpacingKm();
  if (!_params->set_max_range && (maxRange > _maxRangeKm)) {
    _maxRangeKm = maxRange;
    _ppi->configureRange(_maxRangeKm);
    //_rhi->configureRange(_maxRangeKm);
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
	    if ((newMinOrMax) && (_params->debug >= Params::DEBUG_VERBOSE)) { 
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
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI) {

    //_rhiMode = true;

    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    //if (!_rhiWindowDisplayed) {
    //  _rhiWindow->show();
      //_rhiWindow->resize();
    //  _rhiWindowDisplayed = true;
    //}

    // Add the beam to the display

    //_rhi->addBeam(ray, fieldData, nFields); // _fields);
    //_rhiWindow->setAzimuth(ray->getAzimuthDeg());
    //_rhiWindow->setElevation(ray->getElevationDeg());
    
  } else {

    _rhiMode = false;
    
    // check for elevation surveillance sweep mode
    // in this case, set azimuth to rotation if georef is available
    
    if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
      ray->setAnglesForElevSurveillance();
    }

    // Store the ray location using the azimuth angle and the PPI location
    // table

    double az = ray->getAzimuthDeg();
    //_storeRayLoc(ray, az, platform.getRadarBeamWidthDegH(), _ppiRayLoc);

    // Save the angle information for the next iteration

    _prevAz = az;
    _prevEl = -9999.0;

    // Add the beam to the display
    
    //_ppi->addCullTrackBeam(ray, _startAz, _endAz, fieldData, nFields);
    //    _ppi->addBeam(ray, _startAz, _endAz, fieldData, nFields);

  }
  
}

// call when new data file is read; or when switching to new sweep?
// or when new parameter file is read
void PolarManager::_setupRayLocation() {
  cerr << "setupRayLocations: enter " << endl;
  float ppi_rendering_beam_width = _platform.getRadarBeamWidthDegH();
  if (_params->ppi_override_rendering_beam_width) {
    ppi_rendering_beam_width = _params->ppi_rendering_beam_width;
  }
  int currentSweepNumber = _sweepController->getSelectedNumber();
  _rayLocationController->sortRaysIntoRayLocations(ppi_rendering_beam_width,
    currentSweepNumber);

  _setMaxRangeKm();
    cerr << "setupRayLocations: exit " << endl;
}

// find and set the max range for the Ppi display
void PolarManager::_setMaxRangeKm() {
   double maxRangeKm = _rayLocationController->getMaxRangeKm();

  // configure the max range for display
  _ppi->configureRange(maxRangeKm);

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
  /// size_t ifield = 0; // TODO: this is off; we are looping over the NEW fields, not ALL fields !!!!
  // TODO: Wait! Instead, should we be looping over all the fields? because we need to
  // send new fieldData and it needs to be the entire 2D matrix of data? or just an update???
  // NO. For update, we can just send a 2D array of the necessary data. 
  //for (ifieldName = newFieldNames.begin(); ifieldName != newFieldNames.end(); ifieldName++) {
  for (size_t ifield=0; ifield < newFieldNames.size(); ++ifield) {
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
      	    if ((newMinOrMax) && (_params->debug >= Params::DEBUG_VERBOSE)) { 
      	      printf("field index %d, gate %d \t", (int) ifield, igate);
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
      ray->getSweepMode() == Radx::SWEEP_MODE_SUNSCAN_RHI) {

    //_rhiMode = true;

    // If this is the first RHI beam we've encountered, automatically open
    // the RHI window.  After this, opening and closing the window will be
    // left to the user.

    /*
    if (!_rhiWindowDisplayed) {
      _rhiWindow->show();
      //_rhiWindow->resize();
      _rhiWindowDisplayed = true;
    }
    */
    // Add the beam to the display
    LOG(DEBUG) << "RHI not being updated";
    // TODO: update the rhi code ...
    //_rhi->addBeam(ray, fieldData, displayFieldController); // _fields);
    //_rhiWindow->setAzimuth(ray->getAzimuthDeg());
    //_rhiWindow->setElevation(ray->getElevationDeg());
    
  } else {

    _rhiMode = false;

    // check for elevation surveillance sweep mode
    // in this case, set azimuth to rotation if georef is available
    
    
    // TODO: fix for airborne data
    //if (ray->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    //  ray->setAnglesForElevSurveillance();
    //}
    

    // Store the ray location using the azimuth angle and the PPI location
    // table

    double az = ray->getAzimuthDeg();
    LOG(DEBUG) << "az = " << az;
    //_storeRayLoc(ray, az, platform.getRadarBeamWidthDegH(), _ppiRayLoc);

    // Save the angle information for the next iteration

    _prevAz = az;
    _prevEl = -9999.0;

    // Add the beam to the display
    // ray contains data for ALL fields; fieldData contains only data for the new beams
    // nFields = total number of fields (old + new)
    size_t nFields = _displayFieldController->getNFields();
    //_ppi->cleanBeams(nFields);
    //_ppi->updateBeamII(ray, _startAz, _endAz, fieldData, nFields, newFieldNames);

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
    try {
      size_t nFields = _displayFieldController->getNFields();
      if (_nGates < 0) throw "Error, cannot convert _nGates < 0 to type size_t";
      //_ppi->updateBeamColors(nFields, fieldName, (size_t) _nGates); //ray, _startAz, _endAz, data, nFields, fieldName);
    } catch (std::range_error &ex) {
      LOG(ERROR) << fieldName;
      LOG(ERROR) << ex.what();
      QMessageBox::warning(NULL, "Error changing color map", ex.what());
    } catch(std::exception &ex) {
      LOG(ERROR) << fieldName;
      LOG(ERROR) << ex.what();
      QMessageBox::warning(NULL, "Error changing color map", ex.what());
    }
    //}
  LOG(DEBUG) << "exit";
  
}


void PolarManager::dataFileChanged() {

  _sweepController->clearSweepRadioButtons();
  _sweepController->createSweepRadioButtons();

}

///////////////////////////////////////////////////////////
/* store ray location

void PolarManager::_storeRayLoc(const RadxRay *ray, const double az,
                                const double beam_width, RayLoc *ray_loc)
{

  // Determine the extent of this ray

  if (_params->ppi_override_rendering_beam_width) {
    double half_angle = _params->ppi_rendering_beam_width / 2.0;
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

  } // endwhile - i 
  
}
*/

////////////////////////////////////////////
// freeze / unfreeze

/*
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
*/

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

//void PolarManager::_changeField(string fieldName, bool guiMode)
void PolarManager::changeToField(QString newFieldName)
{  
  LOG(DEBUG) << "enter";

  // convert fieldId to field name (from the tool tip) because not all fields are displayed,
  // so we cannot rely on the id/index.
  //QString fieldNameQt = _fieldButtons.at(fieldId)->toolTip();
  //string fieldName = fieldNameQt.toStdString();


  //size_t newSelectionNum = _displayFieldController->getFieldIndex(fieldName);
/*
  // removing this: if we click the already-selected field, go back to previous field
  // 
  try {
    size_t ;
      LOG(DEBUG) << "fieldName is " << fieldName;

    if (guiMode) {
      if (fieldNum == newSelectionNum && _prevFieldNum >= 0) {
        QRadioButton *button =
          (QRadioButton *) _fieldGroup->button(_prevFieldNum);
        button->click();
        return;
      }
    }
*/   
  
  //_prevFieldNum = fieldNum;
  //fieldNum = newSelectionNum;
  string fieldName = newFieldName.toStdString();
  //_displayFieldController->setSelectedField(fieldName);
  // I don't think this does anything ... _ppi->selectVar(fieldName);
  _plotArchiveData();  // either one of these, not both?
  //_rhi->selectVar(fieldNum);  TODO: reinstate this 
// TODO: update the statusPanel
//  _statusPanel->???

  //_selectedName = _selectedField->getName();
  //_selectedLabel = _selectedField->getLabel();
  //_selectedUnits = _selectedField->getUnits();
  
  //_selectedLabelWidget->setText(_selectedLabel.c_str());
  //char text[128];
  //if (_selectedField->getSelectValue() > -9990) {
  //  sprintf(text, "%g %s", 
  //          _selectedField->getSelectValue(),
  //          _selectedField->getUnits().c_str());
  //} else {
  //  text[0] = '\0';
  //}
  //_valueLabel->setText(text);
  //} catch (std::range_error &ex) {
  //    LOG(ERROR) << ex.what();
  //    QMessageBox::warning(NULL, "Error changing field (_changeField):", ex.what());
  //}
  LOG(DEBUG) << "exit";
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
    _displayFieldController->updateFieldPanel(fieldName, _fieldPanel);
  } catch (std::invalid_argument &ex) {
    LOG(ERROR) << fieldName;
    LOG(ERROR) << ex.what(); // "ERROR - field not found; no color map change";
    QMessageBox::warning(NULL, "Error changing color map", ex.what());
  } 
  //_ppi->backgroundColor(backgroundColor);
  //_ppi->gridRingsColor(gridColor);

  selectedFieldChanged(fieldName);

  LOG(DEBUG) << "exit";
}

void PolarManager::setVolume() { // const RadxVol &radarDataVolume) {

  LOG(DEBUG) << "enter";

  _applyDataEdits(); // radarDataVolume);

  LOG(DEBUG) << "exit";



}

// this is a SLOT to a SIGNAL from  ScriptEditor 

void PolarManager::updateVolume(QStringList newFieldNames) {

  LOG(DEBUG) << "enter";
  _volumeDataChanged(newFieldNames);
  _unSavedEdits = true;
  vector<DisplayField *> newFields;


  LOG(DEBUG) << "exit";

}

void PolarManager::spreadsheetDataChanged() {
  _unSavedEdits = true;
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
  LOG(DEBUG) << "    azDeg = " << azDeg;
  
  double range = 0.0;
  if (xkm != 0 || ykm != 0) {
    range = sqrt(xkm * xkm + ykm * ykm);
  }
  LOG(DEBUG) << "    range = " << range;
  
  //int rayIndex = (int) (azDeg * RayLoc::RAY_LOC_RES);

  //  LOG(DEBUG) << "    rayIndex = " << rayIndex;
  
  const RadxRay *ray = _rayLocationController->getClosestRay(azDeg);
  //const RadxRay *ray = _ppiRayLoc[rayIndex].ray;
  if (ray == NULL) {
    // no ray data yet

      LOG(DEBUG) << "    No ray data yet...";
      errorMessage("Error", "No ray data found at " + to_string(azDeg));
      //LOG(DEBUG) << "      active = " << _ppiRayLoc[rayIndex].active;
      // cerr << "      master = " << _ppiRayLoc[rayIndex].master << endl;
      //LOG(DEBUG) << "      startIndex = " << _ppiRayLoc[rayIndex].startIndex;
      //LOG(DEBUG) << "      endIndex = " << _ppiRayLoc[rayIndex].endIndex;
    
    return;
  }

  // notify the status display area to update
  _locationClicked(xkm, ykm, ray);

  // first right of refusal is the boundary editor, if a polygon is not complete
  // TODO:  moved to BPE
    //BoundaryPointEditor *editor = boundaryPointEditorControl;
    //int _worldReleaseX = mouseReleaseX;
    //int _worldReleaseY = mouseReleaseY;
  bool isUseful = false;
  if (boundaryPointEditorControl != NULL) {
    isUseful = boundaryPointEditorControl->evaluatePoint((int) xkm, (int) ykm);
  }

  /*
      if (editor->getCurrentTool() == BoundaryToolType::polygon) {
        if (!editor->isAClosedPolygon()) {
          editor->addPoint(_worldReleaseX, _worldReleaseY);
        } else { //polygon finished, user may want to insert/delete a point
          editor->checkToAddOrDelPoint(_worldReleaseX,
                                       _worldReleaseY);
        }
      } else if (editor->getCurrentTool() == BoundaryToolType::circle) {
        if (editor->isAClosedPolygon()) {
          editor->checkToAddOrDelPoint(_worldReleaseX,
                                       _worldReleaseY);
        } else {
          editor->makeCircle(_worldReleaseX,
                             _worldReleaseY,
                             editor->getCircleRadius());
        }
      }  
  */

  // second choice is the spreadsheet editor
  if (!isUseful) {
    // update the spreadsheet if it is active
    if (sheetView != NULL) {
      _examineSpreadSheetSetup(azDeg, range);
    }
  }

}

///////////////////////////////////////////////////
// respond to a change in click location on the RHI

void PolarManager::_rhiLocationClicked(double xkm, double ykm, 
                                       const RadxRay *closestRay)
  
{

//  _locationClicked(xkm, ykm, _rhi->getRayLoc(), closestRay);

}

////////////////////////////////////////////////////////////////////////
// respond to a change in click location on one of the windows

void PolarManager::_locationClicked(double xkm, double ykm,
                                    // RayLoc *ray_loc, 
                                    const RadxRay *ray)
  
{


    LOG(DEBUG) << "*** Entering PolarManager::_locationClicked()";
  
  
  double range = sqrt(xkm * xkm + ykm * ykm);
  int gate = (int) 
    ((range - ray->getStartRangeKm()) / ray->getGateSpacingKm() + 0.5);

  if (gate < 0 || gate >= (int) ray->getNGates())
  {
    //user clicked outside of ray
    return;
  }

  if (_params->debug) {
    LOG(DEBUG) << "Clicked on location: xkm, ykm: " << xkm << ", " << ykm;
    LOG(DEBUG) << "  range, gate: " << range << ", " << gate;
    LOG(DEBUG) << "  az, el from ray: "
         << ray->getAzimuthDeg() << ", "
         << ray->getElevationDeg();
    //if (_params->debug >= Params::DEBUG_VERBOSE) {
    //  ray->print(cerr);
    //}
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

      LOG(DEBUG) << "Field name, selected name: "
	   << fieldName << ", "
	   << _selectedName;
    
    if (fieldName == _selectedName) {
      char text[128];
      if (fabs(val) < 10000) {
        sprintf(text, "%g %s", val, fieldUnits.c_str());
      } else {
        sprintf(text, "%g %s", -9999.0, fieldUnits.c_str());
      }
      _valueLabel->setText(text);
    }

      LOG(DEBUG) << "Field name, units, val: "
	   << field->getName() << ", "
	   << field->getUnits() << ", "
	   << val;
    
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
// create the time navigation controller and view

void PolarManager::_createTimeControl()
{
  _timeNavView = new TimeNavView(this);
  _timeNavController = new TimeNavController(_timeNavView);
  /*
  // connect slots for time slider

  connect(_timeNavView, SIGNAL(endTimeChanged(string)),
          this, SLOT(endTimeChanged(string)));
  
  connect(_timeNavView, SIGNAL(startTimeChanged(string)),
          this, SLOT(startTimeChanged(string)));
  
  connect(_timeNavView, SIGNAL(newTimeSelected(int)),
          this, SLOT(newTimeSelected(int)));

//  connect(_timeNavView, SIGNAL(sliderPressed()),
//          this, SLOT(_timeSliderPressed()));
*/
}



void PolarManager::_showTimeControl()
{
  if (_timeNavView) {
    _timeNavView->showTimeControl();
  }
}
/*
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
*/

////////////////////////////////////////////////////
// create the file chooser dialog
//
// This allows the user to choose an archive file to open

void PolarManager::_openFile()
{
  LOG(DEBUG) << "enter";
  // seed with files for the day currently in view
  // generate like this: *yyyymmdd*
  //string pattern = _archiveStartTime.getDateStrPlain();
  QString finalPattern = "All Files (*);; Cfradial (*.nc);; All files (*)";
  //finalPattern.append(pattern.c_str());
  //finalPattern.append("*)");

  QString inputPath = "/"; // QDir::currentPath();
  // get the path of the current file, if available 
  string currentFile = _timeNavController->getSelectedArchiveFile();
  if (!currentFile.empty()) {
  //if (_archiveFileList.size() > 0) {
    QDir temp(currentFile.c_str());
    inputPath = temp.absolutePath();
  } 

  QString filename =  QFileDialog::getOpenFileName(
          this,
          "Open Data File",
          inputPath, finalPattern);  //QDir::currentPath(),
  //"All files (*.*)");
 
//------ begin 1/4/2020
  if (filename.isNull() || filename.isEmpty()) {
    return;
  }

  vector<string> fileList;
  fileList.push_back(filename.toStdString());

  setArchiveFileList(fileList, false);

  QMessageBox::information(this, "Status", "retrieving field names ...");
  // choose which fields to import
  vector<string> *selectedFields = userSelectFieldsForReading(fileList[0]);   

  QByteArray qb = filename.toUtf8();
  const char *name = qb.constData();

  LOG(DEBUG) << "selected file path : " << name;

  //since we are opening a new radar file, close any boundaries currently being displayed

  if (boundaryPointEditorControl!= NULL) {
    boundaryPointEditorControl->clear();
    boundaryPointEditorView->setVisible(false);
  }

  // update the time navigation mechanism
  //updateTimeNavigation(fileList, false);

  LOG(DEBUG) << "exit";
}

void PolarManager::_readDataFile(vector<string> *selectedFields) {

  LOG(DEBUG) << "enter";
  if (selectedFields->size() <= 0) {
    QMessageBox::information(this, "Status", "No fields selected for import."); 
  } else {
    QMessageBox::information(this, "Status", "reading data ...");

    //_displayFieldController->clearAllFields();
    _updateDisplayFields(selectedFields);
  //_setupDisplayFields(allFieldNames);
     // trying this ... to get the data from the file selected
    //_setArchiveRetrievalPending();

    try {
      _getArchiveData();
      _setupRayLocation();

    } catch (FileIException &ex) { 
      this->setCursor(Qt::ArrowCursor);
      // _timeControl->setCursor(Qt::ArrowCursor);
      return;
    }

    LOG(DEBUG) << "exit";
  }
}
/*
void PolarManager::_reconcileDisplayFields() {

  
  DisplayField *field;
  _displayFieldController->deleteFieldFromDisplay(field);
}
*/
void PolarManager::fieldsSelected(vector<string> *selectedFields) {

// TODO:
//  delete availableFields;
  LOG(DEBUG) << "enter";

  if (selectedFields->size() > 0) {
    //QStringList qselectedFields;
    LOG(DEBUG) << selectedFields->size() << " selected";
    for (vector<string>::iterator it=selectedFields->begin(); it != selectedFields->end(); ++it) {
      LOG(DEBUG) << *it;
      //qselectedFields.push_back(QString::fromStdString(*it));
      //_displayFieldController->addField(*it);
      //emit addField(*it);
    }
    _addNewFields(selectedFields);
    // give the selected fields to the volume read ...
    _readDataFile(selectedFields);
    
  }  
  // close the modal dialog box for field selection
  closeFieldListDialog(true);
  //fieldListDialog->close();
  LOG(DEBUG) << "exit";
}

void PolarManager::closeFieldListDialog(bool clicked) {
  fieldListDialog->close();
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
  string currentFile = _timeNavController->getSelectedArchiveFile();
  if (!currentFile.empty()) {  
    QDir temp(currentFile.c_str());
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

      LOG(DEBUG) << "selected file path : " << name;


    // TODO: hold it! the save message should
    // go to the Model (Data) level because
    // we'll be using Radx utilities.
    
    try {
      LOG(DEBUG) << "writing to file " << name;
      DataModel *dataModel = DataModel::Instance();
      dataModel->writeData(name);
      _unSavedEdits = false;
    } catch (FileIException &ex) {
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

/*
moved to TimeNavMVC classes
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

      LOG(DEBUG) << "At start of data, cannot go back";
    
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

      LOG(DEBUG) << "At end of data, cannot go forward";
    
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
*/

////////////////////////////////////////////////////////
// set for pending archive retrieval

//void PolarManager::_setArchiveRetrievalPending()
//{
  //_archiveRetrievalPending = true;
//}

/////////////////////////////////////
// clear display widgets

void PolarManager::_clear()
{
  //if (_ppi) {
  //  _ppi->clear();
  //}
  //if (_rhi) {
  //  _rhi->clear();
  //}
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

}

////////////////////////////////////////////////////////
// set modes for retrieving the data


/////////////////////////////////////
// activate archive rendering

void PolarManager::_activateArchiveRendering()
{
  _clear();
  //_displayFieldController->renderFields();
  //if (_ppi) {
    //_fieldRendererController->performRendering(0);
  //  _ppi->activateArchiveRendering();
  //}
  //if (_rhi) {
  //  _rhi->activateArchiveRendering();
    //_fieldRendererController->performRendering(0); // activateArchiveRendering();
  //}
}

/////////////////////////////////////////////////////
// creating image files in archive mode

void PolarManager::_createArchiveImageFiles()
{
  /*
  if (_params->images_creation_mode ==
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
    
  } else if (_params->images_creation_mode ==
             Params::CREATE_IMAGES_ON_ARCHIVE_SCHEDULE) {
    
    for (RadxTime stime = _imagesArchiveStartTime;
         stime <= _imagesArchiveEndTime;
         stime += _params->images_schedule_interval_secs) {
      
      _archiveStartTime = stime;
      _archiveEndTime = _archiveStartTime + _imagesScanIntervalSecs;
      
      _createImageFilesAllSweeps();
      
    } // stime
    
  } // if (_params->images_creation_mode ...
  */
}

/////////////////////////////////////////////////////
// creating one image per field, for each sweep

void PolarManager::_createImageFilesAllSweeps()
{
  /*
  if (_params->images_set_sweep_index_list) {
    
    for (int ii = 0; ii < _params->images_sweep_index_list_n; ii++) {
      int index = _params->_images_sweep_index_list[ii];
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
*/
}

/////////////////////////////////////////////////////
// creating one image per field

void PolarManager::_createImageFiles()
{


    LOG(DEBUG) << "PolarManager::_createImageFiles()";
  /*

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
    

      LOG(DEBUG) << "Creating image for field: " << getSelectedFieldLabel();
    
    
    // save image for plot
    
    _saveImageToFile(false);

  }
  
  // change field back

  _changeField(fieldNum, false);


    LOG(DEBUG) << "Done creating image files";
*/  

}

string PolarManager::_getOutputPath(bool interactive, string &outputDir, string fileExt)
{
  /*
	  // set times from plots
	  if (_rhiMode) {
	    _plotStartTime = _rhi->getPlotStartTime();
	    _plotEndTime = _rhi->getPlotEndTime();
	  } else {
	    _plotStartTime = _ppi->getPlotStartTime();
	    _plotEndTime = _ppi->getPlotEndTime();
	  }

	  // compute output dir

	  outputDir = _params->images_output_dir;
	  char dayStr[1024];
	  if (_params->images_write_to_day_dir) {
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

	  if (strlen(_params->images_file_name_category) > 0) {
	    fileName += _params->images_file_name_category;
	  }

	  // platform

	  if (strlen(_params->images_file_name_platform) > 0) {
	    fileName += _params->images_file_name_delimiter;
	    fileName += _params->images_file_name_platform;
	  }

	  // time

	  if (_params->images_include_time_part_in_file_name) {
	    fileName += _params->images_file_name_delimiter;
	    char timeStr[1024];
	    if (_params->images_include_seconds_in_time_part) {
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

	  if (_params->images_include_field_label_in_file_name) {
	    fileName += _params->images_file_name_delimiter;
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
    */
  return "not implemented";
}

/////////////////////////////////////////////////////
// save image to file
// If interactive is true, use dialog boxes to indicate errors or report
// where the image was saved.

void PolarManager::_saveImageToFile(bool interactive)
{
  /*
	  // create image
  QPixmap pixmap;
  if (_rhiMode)
    pixmap = QPixmap::grabWidget(_rhi);
  else
    pixmap = QPixmap::grabWidget(_ppi);
  QImage image = pixmap.toImage();

  string outputDir;
  string outputPath = _getOutputPath(interactive, outputDir, _params->images_file_name_extension);

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


    LOG(DEBUG) << "==>> saved image to file: " << outputPath;
  

  // write latest data info

  if (_params->images_write_latest_data_info) {
    
    DsLdataInfo ldataInfo(_params->images_output_dir);
    
    string relPath;
    RadxPath::stripDir(_params->images_output_dir, outputPath, relPath);
    
    if(_params->debug) {
      ldataInfo.setDebug();
    }
    ldataInfo.setLatestTime(_plotStartTime.utime());
    ldataInfo.setWriter("HawkEye");
    ldataInfo.setDataFileExt(_params->images_file_name_extension);
    ldataInfo.setDataType(_params->images_file_name_extension);
    ldataInfo.setRelDataPath(relPath);
    
    if(ldataInfo.write(_plotStartTime.utime())) {
      cerr << "ERROR - PolarManager::_saveImageToFile()" << endl;
      cerr << "  Cannot write _latest_data_info to dir: " << outputDir << endl;
      return;
    }
    
  } // if (_params->images_write_latest_data_info)
*/
}



void PolarManager::ShowContextMenu(const QPoint &pos) {
  //_ppi->ShowContextMenu(pos, &_vol);
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

//void PolarManager::boundaryColorChanged(QColor newColor) {
//  boundaryPointEditorControl->setBoundaryColor(newColor.toStdString());
//}

void PolarManager::boundaryCircleRadiusChanged(int value) 
{
  if (boundaryPointEditorControl != NULL) {
    bool resizeExistingCircle = boundaryPointEditorControl->setCircleRadius(value);
    if (resizeExistingCircle) { //resize existing circle
      // TODO: somehow get the worldPlot and painter from ppiWidget
      _ppi->showSelectedField(); // refresh/replot->BoundaryPointEditor::Instance()->draw(_zoomWorld, painter); 
      //boundaryPointEditorControl->draw(worldPlot, painter);
      //WorldPlot zoomWorld = _ppi->getZoomWorld();
      //QPainter painter = _ppi->getPainter();
    }
  }
}

void PolarManager::boundaryBrushRadiusChanged(int value) 
{
  if (boundaryPointEditorControl != NULL) {
    //bool resizeExistingCircle = 
    boundaryPointEditorControl->setBrushRadius(value);
    //if (resizeExistingCircle) { //resize existing circle
      // TODO: somehow get the worldPlot and painter from ppiWidget
      _ppi->showSelectedField(); // refresh/replot->BoundaryPointEditor::Instance()->draw(_zoomWorld, painter); 
      //boundaryPointEditorControl->draw(worldPlot, painter);
      //WorldPlot zoomWorld = _ppi->getZoomWorld();
      //QPainter painter = _ppi->getPainter();
    //}
  }
}

void PolarManager::drawBoundary(WorldPlot &zoomWorld, QPainter &painter) {
  if (boundaryPointEditorControl != NULL) {
    boundaryPointEditorControl->drawBoundary(zoomWorld, painter);
  }
}

// I'm not sure this is used???
void PolarManager::mouseMoveEvent(int worldX, int worldY)
{  
  /*
  if (boundaryPointEditorControl != NULL) {

    bool redraw = boundaryPointEditorControl->evaluatePoint(worldX, worldY);
    if (redraw) { 
      _ppi->showSelectedField(); 
    }
  }
  */
}

bool PolarManager::evaluateCursor(bool isShiftKeyDown) {
  bool changeCursor = false;
  if (boundaryPointEditorControl != NULL) {
    changeCursor = boundaryPointEditorControl->evaluateCursor(isShiftKeyDown);
  }
  return changeCursor;
}

bool PolarManager::evaluateRange(double xRange) {
    //doUpdate = BoundaryPointEditor::Instance()->updateScale(xRange);
  if (boundaryPointEditorControl != NULL) {
    return boundaryPointEditorControl->updateScale(xRange);
  }
}

bool PolarManager::isOverBoundaryPoint(double worldX, double worldY) {
  if (boundaryPointEditorControl != NULL) {
    return boundaryPointEditorControl->isOverBoundaryPoint(worldX, worldY);
  } else {
    return false;
  }
}

bool PolarManager::moveBoundaryPoint(double worldPressX, double worldPressY,
  double worldReleaseX, double worldReleaseY) {
  bool redraw = false;
  if (boundaryPointEditorControl != NULL) {
    redraw = boundaryPointEditorControl->moveBoundaryPoint(worldPressX, worldPressY,
      worldReleaseX, worldReleaseY);
  }
  return redraw;
}

void PolarManager::addDeleteBoundaryPoint(double mouseReleaseX, double mouseReleaseY,
  bool isShiftKeyDown)
{    
    // If boundary editor active, then interpret boundary mouse release event
  if (boundaryPointEditorControl != NULL) {
    boundaryPointEditorControl->addDeleteBoundaryPoint(mouseReleaseX, mouseReleaseY,
      isShiftKeyDown); 
  }
} 

/* TODO: grab new code from HawkEye ... 
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
*/

void PolarManager::_clearBoundaryEditorClick()
{
	//BoundaryPointEditor::Instance()->clear();
	_ppi->update();   //forces repaint which clears existing polygon
}


void PolarManager::saveBoundaryEvent(int boundaryIndex)
{
	LOG(DEBUG) << "enter";

  // get selected field name
  string currentFieldName = _displayFieldController->getSelectedFieldName();
  int currentSweepIndex = _sweepController->getSelectedNumber();
  string currentFile = _timeNavController->getSelectedArchiveFile();
  if (!currentFile.empty()) {

    string radarFilePath = currentFile;
  
    try {
      boundaryPointEditorControl->save(boundaryIndex, currentFieldName, 
        currentSweepIndex, radarFilePath);
    } catch (std::runtime_error &ex) {
      errorMessage("Save Boundary Error", ex.what());
    }
  } else {
    errorMessage("Save Boundary Error",
      "cannot save boundary, because no open archive file");
  }
  LOG(DEBUG) << "exit";
}


void PolarManager::loadBoundaryEvent(int boundaryIndex)
{
  LOG(DEBUG) << "enter";
  //if (boundaryIndex > 0) {
    //  saved boundary
    // get selected field name
  string currentFieldName = _displayFieldController->getSelectedFieldName();
  int currentSweepIndex = _sweepController->getSelectedNumber();
  string currentFile = _timeNavController->getSelectedArchiveFile();

  if (!currentFile.empty()) {

  string radarFilePath = currentFile;

    bool successful = boundaryPointEditorControl->load(boundaryIndex, 
      currentFieldName, currentSweepIndex, radarFilePath);
    if (!successful) {
      string msg = "could not load boundary for " + currentFieldName;
      errorMessage("Load Boundary Error", msg.c_str());
    }
  } else {
    errorMessage("Load Boundary Error",
     "cannot load boundary, because no open archive file");
  }

  // repaint which clears the existing boundary
  _ppi->update(); // showSelectedField();   

  LOG(DEBUG) << "exit";
}

void PolarManager::refreshBoundaries()
{
  LOG(DEBUG) << "enter";
  if (boundaryPointEditorControl != NULL) {

    // get selected field name
    string currentFieldName = _displayFieldController->getSelectedFieldName();
    int currentSweepIndex = _sweepController->getSelectedNumber();

    string currentFile = _timeNavController->getSelectedArchiveFile();

    if (!currentFile.empty()) {

      string radarFilePath = currentFile;    
    
      boundaryPointEditorControl->refreshBoundaries(
        radarFilePath, 
        currentFieldName,
        currentSweepIndex);
    }
  }

  LOG(DEBUG) << "exit";
}

/////////////////////////////
// show boundary editor
void PolarManager::showBoundaryEditor()
{
  LOG(DEBUG) << "enter";
 
  // create the view

  if (boundaryPointEditorView == NULL) {
    boundaryPointEditorView = new BoundaryPointEditorView(this);

    // install event filter to catch when the boundary point enditor is closed
    CloseEventFilter *closeFilter = new CloseEventFilter(boundaryPointEditorView);
    boundaryPointEditorView->installEventFilter(closeFilter);

    boundaryView = new BoundaryView();
    // create the model in the controller
    
    // create the controller
    boundaryPointEditorControl = 
      new BoundaryPointEditor(boundaryPointEditorView, boundaryView);

    // connect some signals and slots in order to retrieve information
    // and send changes back to display
                                         
    connect(boundaryPointEditorView, SIGNAL(boundaryPointEditorClosed()), this, SLOT(boundaryEditorClosed()));

    connect(boundaryPointEditorView, SIGNAL(boundaryCircleRadiusChanged(int)),
      this, SLOT(boundaryCircleRadiusChanged(int)));  
    connect(boundaryPointEditorView, SIGNAL(boundaryBrushRadiusChanged(int)),
      this, SLOT(boundaryBrushRadiusChanged(int))); 

    connect(boundaryPointEditorControl, SIGNAL(clearBoundaryClicked()),
      this, SLOT(_clearBoundaryEditorClick())); 

    connect(boundaryPointEditorView, SIGNAL(refreshBoundariesEvent()),
      this, SLOT(refreshBoundaries()));  

    connect(boundaryPointEditorView, SIGNAL(saveBoundary(int)), 
      this, SLOT(saveBoundaryEvent(int)));
    connect(boundaryPointEditorView, SIGNAL(loadBoundary(int)), 
      this, SLOT(loadBoundaryEvent(int)));

// done in BoundaryPointEditor ...
  //connect(boundaryPointEditorView, SIGNAL(userClickedPolygonButton()),
  //  boundaryPointEditorControl, SLOT(userClickedPolygonButton()));
  //connect(boundaryPointEditorView, SIGNAL(userClickedCircleButton()),
  //  boundaryPointEditorControl, SLOT(userClickedCircleButton()));
  //connect(boundaryPointEditorView, SIGNAL(userClickedBrushButton()),
  //  boundaryPointEditorControl, SLOT(userClickedBrushButton()));  
    
    //boundaryPointEditorView->init();
    //boundaryPointEditorView->show();
  } else {

    //string currentFieldName = _displayFieldController->getSelectedFieldName();
    //sheetView->highlightClickedData(currentFieldName, azimuth, (float) range);
  }
  
  //BoundaryPointEditor::Instance()->setManager(this);
  boundaryPointEditorControl->showBoundaryEditor();
// ----



  /*
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
  */
  LOG(DEBUG) << "exit";
}



// from DisplayManager ...

//////////////////////////////////////////////
// create the status panel

void PolarManager::_createStatusPanel()
{
 
  Qt::Alignment alignLeft(Qt::AlignLeft);
  Qt::Alignment alignRight(Qt::AlignRight);
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignTop(Qt::AlignTop);

  // status panel - rows of label value pairs
  
  _statusPanel = new QGroupBox(_main);
  _statusLayout = new QGridLayout(_statusPanel);
  _statusLayout->setVerticalSpacing(5);

  int row = 0;
  
  ParamFile *_params = ParamFile::Instance();
  // fonts
  
  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font6 = dummy.font();
  int fsize = _params->label_font_size;
  int fsize2 = _params->label_font_size + 2;
  int fsize6 = _params->label_font_size + 6;
  font.setPixelSize(fsize);
  font2.setPixelSize(fsize2);
  font6.setPixelSize(fsize6);

  // radar and site name
  
  _radarName = new QLabel(_statusPanel);
  string rname(_params->radar_name);
  if (_params->display_site_name) {
    rname += ":";
    rname += _params->site_name;
  }
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

  if (_params->show_status_in_gui.fixed_angle) {
    _fixedAngVal = _createStatusVal("Fixed ang", "-99.99", row++, fsize2);
  } else {
    _fixedAngVal = NULL;
  }
  
  if (_params->show_status_in_gui.volume_number) {
    _volNumVal = _createStatusVal("Volume", "0", row++, fsize);
  } else {
    _volNumVal = NULL;
  }
  
  if (_params->show_status_in_gui.sweep_number) {
    _sweepNumVal = _createStatusVal("Sweep", "0", row++, fsize);
  } else {
    _sweepNumVal = NULL;
  }

  if (_params->show_status_in_gui.n_samples) {
    _nSamplesVal = _createStatusVal("N samp", "0", row++, fsize);
  } else {
    _nSamplesVal = NULL;
  }

  if (_params->show_status_in_gui.n_gates) {
    _nGatesVal = _createStatusVal("N gates", "0", row++, fsize);
  } else {
    _nGatesVal = NULL;
  }

  if (_params->show_status_in_gui.gate_length) {
    _gateSpacingVal = _createStatusVal("Gate len", "0", row++, fsize);
  } else {
    _gateSpacingVal = NULL;
  }
  
  if (_params->show_status_in_gui.pulse_width) {
    _pulseWidthVal = _createStatusVal("Pulse width", "-9999", row++, fsize);
  } else {
    _pulseWidthVal = NULL;
  }

  if (_params->show_status_in_gui.prf_mode) {
    _prfModeVal = _createStatusVal("PRF mode", "Fixed", row++, fsize);
  } else {
    _prfModeVal = NULL;
  }

  if (_params->show_status_in_gui.prf) {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
  } else {
    _prfVal = NULL;
  }

  if (_params->show_status_in_gui.nyquist) {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
  } else {
    _nyquistVal = NULL;
  }

  if (_params->show_status_in_gui.max_range) {
    _maxRangeVal = _createStatusVal("Max range", "-9999", row++, fsize);
  } else {
    _maxRangeVal = NULL;
  }

  if (_params->show_status_in_gui.unambiguous_range) {
    _unambigRangeVal = _createStatusVal("U-A range", "-9999", row++, fsize);
  } else {
    _unambigRangeVal = NULL;
  }

  if (_params->show_status_in_gui.measured_power_h) {
    _powerHVal = _createStatusVal("Power H", "-9999", row++, fsize);
  } else {
    _powerHVal = NULL;
  }

  if (_params->show_status_in_gui.measured_power_v) {
    _powerVVal = _createStatusVal("Power V", "-9999", row++, fsize);
  } else {
    _powerVVal = NULL;
  }

  if (_params->show_status_in_gui.scan_name) {
    _scanNameVal = _createStatusVal("Scan name", "unknown", row++, fsize);
  } else {
    _scanNameVal = NULL;
  }

  if (_params->show_status_in_gui.scan_mode) {
    _sweepModeVal = _createStatusVal("Scan mode", "SUR", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_params->show_status_in_gui.polarization_mode) {
    _polModeVal = _createStatusVal("Pol mode", "Single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_params->show_status_in_gui.latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_params->show_status_in_gui.longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_params->show_status_in_gui.altitude) {
    if (_altitudeInFeet) {
      _altVal = _createStatusVal("Alt(kft)", "-999.999",
                                 row++, fsize, &_altLabel);
    } else {
      _altVal = _createStatusVal("Alt(km)", "-999.999",
                                 row++, fsize, &_altLabel);
    }
  } else {
    _altVal = NULL;
  }

  if (_params->show_status_in_gui.altitude_rate) {
    if (_altitudeInFeet) {
      _altRateVal = _createStatusVal("AltRate(ft/s)", "-999.999",
                                     row++, fsize, &_altRateLabel);
    } else {
      _altRateVal = _createStatusVal("AltRate(m/s)", "-999.999",
                                     row++, fsize, &_altRateLabel);
    }
  } else {
    _altRateVal = NULL;
  }

  if (_params->show_status_in_gui.speed) {
    _speedVal = _createStatusVal("Speed(m/s)", "-999.99", row++, fsize);
  } else {
    _speedVal = NULL;
  }

  if (_params->show_status_in_gui.heading) {
    _headingVal = _createStatusVal("Heading(deg)", "-999.99", row++, fsize);
  } else {
    _headingVal = NULL;
  }

  if (_params->show_status_in_gui.track) {
    _trackVal = _createStatusVal("Track(deg)", "-999.99", row++, fsize);
  } else {
    _trackVal = NULL;
  }

  if (_params->show_status_in_gui.sun_elevation) {
    _sunElVal = _createStatusVal("Sun el (deg)", "-999.999", row++, fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_params->show_status_in_gui.sun_azimuth) {
    _sunAzVal = _createStatusVal("Sun az (deg)", "-999.999", row++, fsize);
  } else {
    _sunAzVal = NULL;
  }

  QLabel *spacerRow = new QLabel("", _statusPanel);
  _statusLayout->addWidget(spacerRow, row, 0);
  _statusLayout->setRowStretch(row, 1);
  row++;

}

/* moved to DisplayFieldView
//////////////////////////////////////////////
// create the field panel

void PolarManager::_createFieldPanel()
{
  
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  int fsize = _params->label_font_size;
  int fsize2 = _params->label_font_size + 2;
  int fsize4 = _params->label_font_size + 4;
  int fsize6 = _params->label_font_size + 6;

  _fieldPanel = new QGroupBox(_main);
  _fieldGroup = new QButtonGroup;
  _fieldsLayout = new QGridLayout(_fieldPanel);
  _fieldsLayout->setVerticalSpacing(5);

  int row = 0;
  int nCols = 3;
  if (_haveFilteredFields) {
    nCols = 4;
  }

  _displayFieldController->setSelectedField(0);
  _selectedField = _displayFieldController->getSelectedField(); //_fields[0];
  _selectedLabel = _selectedField->getLabel(); //_fields[0]->getLabel();
  _selectedName = _selectedField->getName(); // _fields[0]->getName();
  //  _selectedField = _fields[0];
  //_selectedLabel = _fields[0]->getLabel();
  //_selectedName = _fields[0]->getName();
  _selectedLabelWidget = new QLabel(_selectedLabel.c_str(), _fieldPanel);
  QFont font6 = _selectedLabelWidget->font();
  font6.setPixelSize(fsize6);
  _selectedLabelWidget->setFont(font6);
  _fieldsLayout->addWidget(_selectedLabelWidget, row, 0, 1, nCols, alignCenter);

  row++;


  QFont font4 = _selectedLabelWidget->font();
  font4.setPixelSize(fsize4);
  QFont font2 = _selectedLabelWidget->font();
  font2.setPixelSize(fsize2);
  QFont font = _selectedLabelWidget->font();
  font.setPixelSize(fsize);
  

  QLabel dummy;
  QFont font = dummy.font();
  QFont font2 = dummy.font();
  QFont font4 = dummy.font();

  _valueLabel = new QLabel("", _fieldPanel);
  _valueLabel->setFont(font);
  _fieldsLayout->addWidget(_valueLabel, row, 0, 1, nCols, alignCenter);
  row++;

  QLabel *fieldHeader = new QLabel("FIELD LIST", _fieldPanel);
  fieldHeader->setFont(font);
  _fieldsLayout->addWidget(fieldHeader, row, 0, 1, nCols, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _fieldPanel);
  nameHeader->setFont(font);
  _fieldsLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *keyHeader = new QLabel("HotKey", _fieldPanel);
  keyHeader->setFont(font);
  _fieldsLayout->addWidget(keyHeader, row, 1, alignCenter);
  if (_haveFilteredFields) {
    QLabel *rawHeader = new QLabel("Raw", _fieldPanel);
    rawHeader->setFont(font);
    _fieldsLayout->addWidget(rawHeader, row, 2, alignCenter);
    QLabel *filtHeader = new QLabel("Filt", _fieldPanel);
    filtHeader->setFont(font);
    _fieldsLayout->addWidget(filtHeader, row, 3, alignCenter);
  }
  row++;
  _rowOffset = row;

  // add fields, one row at a time
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present
  size_t nFields = _displayFieldController->getNFields();
  for (size_t ifield = 0; ifield < nFields; ifield++) {

    // get raw field - always present
    
    const DisplayField *rawField = _displayFieldController->getField(ifield); // _fields[ifield];
    int buttonRow = rawField->getButtonRow();
    
    // get filt field - may not be present
    const DisplayField *filtField = _displayFieldController->getFiltered(ifield, buttonRow);

    QLabel *label = new QLabel(_fieldPanel);
    label->setFont(font);
    label->setText(rawField->getLabel().c_str());
    QLabel *key = new QLabel(_fieldPanel);
    key->setFont(font);
    if (rawField->getShortcut().size() > 0) {
      char text[4];
      text[0] = rawField->getShortcut()[0];
      text[1] = '\0';
      key->setText(text);
      char text2[128];
      sprintf(text2, "Hit %s for %s, ALT-%s for filtered",
        text, rawField->getName().c_str(), text);
      label->setToolTip(text2);
      key->setToolTip(text2);
    }

    QRadioButton *rawButton = new QRadioButton(_fieldPanel);
    rawButton->setToolTip(rawField->getName().c_str());
    if (ifield == 0) {
      rawButton->click();
    }
    _fieldsLayout->addWidget(label, row, 0, alignCenter);
    _fieldsLayout->addWidget(key, row, 1, alignCenter);
    _fieldsLayout->addWidget(rawButton, row, 2, alignCenter);
    _fieldGroup->addButton(rawButton, ifield);
    // connect slot for field change
    connect(rawButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));

    _fieldButtons.push_back(rawButton);
    if (filtField != NULL) {
      QRadioButton *filtButton = new QRadioButton(_fieldPanel);
      filtButton->setToolTip(filtField->getName().c_str());
      _fieldsLayout->addWidget(filtButton, row, 3, alignCenter);
      _fieldGroup->addButton(filtButton, ifield + 1);
      _fieldButtons.push_back(filtButton);
      // connect slot for field change
      connect(filtButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
    }

    _displayFieldController->setVisible(ifield);

    if (filtField != NULL) {
      ifield++;
    }
    _fieldsLayout->setRowStretch(row, 1);

    row++;
  }

  //QLabel *spacerRow = new QLabel("", _fieldPanel);
  //_fieldsLayout->addWidget(spacerRow, row, 0);
  //_fieldsLayout->setRowStretch(row, 1);
  //row++;

  // HERE <<=== 
  //  _lastButtonRowFixed = row; // Q: is this just the size of _fieldButtons?
  // connect slot for field change
  
  //connect(_fieldGroup, SIGNAL(buttonClicked(int)),
  //        this, SLOT(_changeField(int)));

}

//////////////////////////////////////////////
// update the field panel

void PolarManager::_updateFieldPanel(string newFieldName)
{
  LOG(DEBUG) << "enter";
  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  int fsize = _params->label_font_size;
  int fsize2 = _params->label_font_size + 2;
  int fsize4 = _params->label_font_size + 4;
  int fsize6 = _params->label_font_size + 6;

  int row; //  = _rowOffset;
  int nCols = 3;
  if (_haveFilteredFields) {
    nCols = 4;
  }
  //QFont font = _selectedLabelWidget->font();
  QLabel dummy;
  QFont font = dummy.font();
  font.setPixelSize(fsize);



  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present

  size_t nFields = _displayFieldController->getNFields(); // 0 - based index
  size_t ifield = _displayFieldController->getFieldIndex(newFieldName);
  //if (ifield < lastButtonRowFixed - 1) {
    // field already in panel
  //  return;
  //}

    // get raw field - always present
    
  DisplayField *rawField = _displayFieldController->getField(ifield); //_fields[ifield];
  if (rawField->isHidden()) {
    int lastButtonRowFixed = _fieldButtons.size(); // 1 - based index

    row = lastButtonRowFixed + _rowOffset;
    rawField->setButtonRow(row);
    
    // get filt field - may not be present
    const DisplayField *filtField = _displayFieldController->getFiltered(ifield, -1);

//----
//    _spolDivColorMapLabel = new ClickableLabel();
//ColorMapTemplates.cc:    // connect(cmapLabel, &ClickableLabel::clicked, this, &ParameterColorDialog::pickColorPalette);
//ColorMapTemplates.cc:    connect(_defaultColorMapLabel,   &ClickableLabel::clicked, this, &ColorMapTemplates::defaultClicked);

//---

    //QLabel *label = new QLabel(_fieldPanel);
    ClickableLabel *label = new ClickableLabel(_fieldPanel);
    connect(label, SIGNAL(ClickableLabel::clicked), this, SLOT(contextMenuParameterColors));
    label->setFont(font);
    label->setText(rawField->getLabel().c_str());
    QLabel *key = new QLabel(_fieldPanel);
    key->setFont(font);
    if (rawField->getShortcut().size() > 0) {
      char text[4];
      text[0] = rawField->getShortcut()[0];
      text[1] = '\0';
      key->setText(text);
      char text2[128];
      sprintf(text2, "Hit %s for %s, ALT-%s for filtered",
        text, rawField->getName().c_str(), text);
      label->setToolTip(text2);
      key->setToolTip(text2);
    }

    QRadioButton *rawButton = new QRadioButton(_fieldPanel);
    rawButton->setToolTip(rawField->getName().c_str());
    //if (ifield == 0) {
      rawButton->click();
    //}
    //row = buttonRow + 4;
    _fieldsLayout->addWidget(label, row, 0, alignCenter);
    _fieldsLayout->addWidget(key, row, 1, alignCenter);
    _fieldsLayout->addWidget(rawButton, row, 2, alignCenter);
    _fieldGroup->addButton(rawButton, ifield);
    // connect slot for field change
    connect(rawButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));

    _fieldButtons.push_back(rawButton);
    if (filtField != NULL) {
      QRadioButton *filtButton = new QRadioButton(_fieldPanel);
      filtButton->setToolTip(filtField->getName().c_str());
      _fieldsLayout->addWidget(filtButton, row, 3, alignCenter);
      _fieldGroup->addButton(filtButton, ifield + 1);
      _fieldButtons.push_back(filtButton);
      // connect slot for field change
      connect(filtButton, SIGNAL(toggled(bool)), this, SLOT(_changeFieldVariable(bool)));
    }

    //if (filtField != NULL) {
    //  ifield++;
    //}

    //QLabel *spacerRow = new QLabel("", _fieldPanel);
    //_fieldsLayout->addWidget(spacerRow, row, 0);
    _fieldsLayout->setRowStretch(row, 1);
    
    rawField->setStateVisible();
    _displayFieldController->setSelectedField(ifield);

  }

  // connect slot for field change
  
  //connect(_fieldGroup, SIGNAL(buttonClicked(int)),
  //        this, SLOT(_changeField(int)));
  LOG(DEBUG) << "exit";
}
*/

void PolarManager::ShowParameterColorDialog(QString fieldName)
{
  
  LOG(DEBUG) << "enter";

  //DisplayField selectedField;                                                                             

  // const DisplayField &field = _manager.getSelectedField();
  // const ColorMap &colorMapForSelectedField = field.getColorMap();
  ParameterColorView *parameterColorView = new ParameterColorView(this);
  // vector<DisplayField *> displayFields = displayFieldController->getDisplayFields(); // TODO: I guess, implement this as a signal and a slot? // getDisplayFields();
  DisplayField *selectedField = _displayFieldController->getSelectedField();
  string emphasis_color = "white";
  string annotation_color = "white";

  DisplayFieldModel *displayFieldModel = _displayFieldController->getModel();

  FieldColorController *fieldColorController = new FieldColorController(parameterColorView, displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
                                                                         
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
      this, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!

  if (boundaryPointEditorControl != NULL) {
    connect(fieldColorController, SIGNAL(boundaryColorSet(QColor)),
      boundaryPointEditorControl, SLOT(boundaryColorChanged(QColor)));
    string boundaryColor = boundaryPointEditorControl->getBoundaryColor();
    fieldColorController->updateBoundaryColor(boundaryColor);
  }
  fieldColorController->startUp(); 
 
  LOG(DEBUG) << "exit ";
  
}

/*
// needs PolarManager, DisplayFieldModel access, or DisplayFieldController access
void PolarManager::contextMenuParameterColors()
{
  
  LOG(DEBUG) << "enter";

  // TODO: parent should be PolarManager ...
  ParameterColorView *parameterColorView = new ParameterColorView(this);
  // vector<DisplayField *> displayFields = displayFieldController->getDisplayFields(); // TODO: I guess, implement this as a signal and a slot? // getDisplayFields();
  DisplayField *selectedField = _displayFieldController->getSelectedField();
  string emphasis_color = "white";
  string annotation_color = "white";

  DisplayFieldModel *displayFieldModel = _displayFieldController->getModel();

  FieldColorController *fieldColorController = new FieldColorController(parameterColorView,
    displayFieldModel);
  // connect some signals and slots in order to retrieve information
  // and send changes back to display
              
  // TODO: move to PolarManager ...                                                           
  //  connect(parameterColorView, SIGNAL(retrieveInfo), &_manager, SLOT(InfoRetrieved()));
  connect(fieldColorController, SIGNAL(colorMapRedefineSent(string, ColorMap, QColor, QColor, QColor, QColor)),
      this, SLOT(colorMapRedefineReceived(string, ColorMap, QColor, QColor, QColor, QColor))); // THIS IS NOT CALLED!!
  //  PolarManager::colorMapRedefineReceived(string, ColorMap)
  //connect(fieldColorController, SIGNAL(colorMapRedefined(string)),
  //    this, SLOT(changeToDisplayField(string))); // THIS IS NOT CALLED!!

  // TODO: combine with replot
  //connect(fieldColorController, SIGNAL(backgroundColorSet(QColor)),
  //    this, SLOT(backgroundColor(QColor)));
  //

  fieldColorController->startUp(); 

  //connect(parameterColorView, SIGNAL(needFieldNames()), this, SLOT(getFieldNames()));
  //connect(this, SIGNAL(fieldNamesSupplied(vector<string>)), 
  //  parameterColorView, SLOT(fieldNamesSupplied(vector<string>));
  // TODO: move this call to the controller?                                                                
    // parameterColorView.exec();

  //  if(parameterColorController.Changes()) {
    // TODO: what are changes?  new displayField(s)?                                                        
  //}
 
  LOG(DEBUG) << "exit ";
  
}
*/

/* Moved to DisplayFieldView
void PolarManager::_changeFieldVariable(bool value) {

  LOG(DEBUG) << " field variable changed ";

  if (value) {
    for (size_t i = 0; i < _fieldButtons.size(); i++) {
      if (_fieldButtons.at(i)->isChecked()) {
        LOG(DEBUG) << "_fieldButton " << i
        << "out of " << _fieldButtons.size() 
        << " is checked";
        QString fieldNameQ = _fieldButtons.at(i)->text();
        LOG(DEBUG) << "fieldname is " << fieldNameQ.toStdString();
        _changeField(i, true);
      }
    }
  }

}
*/

/*
void PolarManager::colorMapRedefineReceived(string fieldName, ColorMap newColorMap) {

  LOG(DEBUG) << "enter"; 
  
  // connect the new color map with the field
  // find the fieldName in the list of FieldDisplays
  // This should save/perpetuate the color map in the DisplayField object
  _displayFieldController->saveColorMap(fieldName, &newColorMap);
  size_t fieldIndex = _displayFieldController->getFieldIndex(fieldName);
  _changeField(fieldIndex, true); 
*/
  /*
  bool found = false;
  vector<DisplayField *>::iterator it;
  int fieldId = 0;

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
    _changeField(fieldId, true); 
  }
  */
/*
  LOG(DEBUG) << "exit";
}
*/


///////////////////////////////////////////////////////
// create the click report dialog
//
// This shows the field values at the latest click point

void PolarManager::_createClickReportDialog()
{
  
  _clickReportDialog = new QDialog(this);
  _clickReportDialog->setMinimumSize(100, 100);
  _clickReportDialog->setWindowTitle("Field values");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  Qt::Alignment alignRight(Qt::AlignRight);
  
  _clickReportDialogLayout = new QGridLayout(_clickReportDialog);
  _clickReportDialogLayout->setVerticalSpacing(5);

  int row = 0;
  QLabel *mainHeader = new QLabel("CLICK POINT DATA", _clickReportDialog);
  _clickReportDialogLayout->addWidget(mainHeader, row, 0, 1, 3, alignCenter);
  row++;

  // _clickReportDialogLayout->addWidget(left, row, 0, alignRight);

  _dateClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Date", "9999/99/99", row++);
  _timeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Time", "99:99:99.999", row++);
  _elevClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                              "Elevation", "-9999", row++);
  _azClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                            "Azimuth", "-9999", row++);
  _gateNumClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                                 "Gate num", "-9999", row++);
  _rangeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                               "Range", "-9999", row++);
  _altitudeClicked = _addLabelRow(_clickReportDialog, _clickReportDialogLayout,
                                  "Altitude", "-9999", row++);

  QLabel *valHeader = new QLabel("FIELD VALUES", _clickReportDialog);
  _clickReportDialogLayout->addWidget(valHeader, row, 0, 1, 3, alignCenter);
  row++;

  QLabel *nameHeader = new QLabel("Name", _clickReportDialog);
  _clickReportDialogLayout->addWidget(nameHeader, row, 0, alignCenter);
  QLabel *rawHeader = new QLabel("Raw", _clickReportDialog);
  _clickReportDialogLayout->addWidget(rawHeader, row, 1, alignCenter);
  if (_haveFilteredFields) {
    QLabel *filtHeader = new QLabel("Filt", _clickReportDialog);
    _clickReportDialogLayout->addWidget(filtHeader, row, 2, alignCenter);
  }
  row++;

  // add fields, one row at a time
  // a row can have 1 or 2 buttons, depending on whether the
  // filtered field is present
  size_t nFields = _displayFieldController->getNFields();
  for (size_t ifield = 0; ifield < nFields; ifield++) {

    // get raw field - always present
    
    DisplayField *rawField = _displayFieldController->getField(ifield); // _fields[ifield];
    int buttonRow = rawField->getButtonRow();
    
    // get filt field - may not be present
    DisplayField *filtField = _displayFieldController->getFiltered(ifield, buttonRow);

    QLabel *label = new QLabel(_clickReportDialog);
    label->setText(rawField->getLabel().c_str());
    _clickReportDialogLayout->addWidget(label, row, 0, alignCenter);
    
    rawField->createDialog(_clickReportDialog, "-------------");
    _clickReportDialogLayout->addWidget(rawField->getDialog(), row, 1, alignCenter);

    if (filtField) {
      filtField->createDialog(_clickReportDialog, "-------------");
      _clickReportDialogLayout->addWidget(filtField->getDialog(), row, 2, alignCenter);
    }
      
    if (filtField != NULL) {
      ifield++;
    }

    row++;
  }
}

//////////////////////////////////////////////
// make a new label with right justification

QLabel *PolarManager::_newLabelRight(const string &text)
{
  QLabel *label = new QLabel;
  label->setText("-----");
  label->setAlignment(Qt::AlignRight);
  return label;
}

//////////////////////////////////////////////////
// create a row in the status panel

QLabel *PolarManager::_createStatusVal(const string &leftLabel,
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
  Qt::Alignment alignCenter(Qt::AlignCenter);
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

//////////////////////////////////////////////////
// create a label row in a dialog

QLabel *PolarManager::_addLabelRow(QWidget *parent,
                                     QGridLayout *layout,
                                     const string &leftLabel,
                                     const string &rightLabel,
                                     int row, 
                                     int fontSize)
  
{

  QLabel *left = new QLabel(parent);
  left->setText(leftLabel.c_str());
  Qt::Alignment alignRight(Qt::AlignRight);
  layout->addWidget(left, row, 0, alignRight);
  
  QLabel *right = new QLabel(parent);
  right->setText(rightLabel.c_str());
  Qt::Alignment alignCenter(Qt::AlignCenter);
  layout->addWidget(right, row, 1, 1, 2, alignCenter);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  return right;
}

/* only used by BScanManager
//////////////////////////////////////////////////
// create a user input row in a widget

QLineEdit *PolarManager::_addInputRow(QWidget *parent,
                                        QVBoxLayout *layout,
                                        const string &leftLabel,
                                        const string &rightContent,
                                        int fontSize,
                                        QLabel **label)
  
{
  
  QFrame *frame = new QFrame(parent);
  QHBoxLayout *horiz = new QHBoxLayout;
  frame->setLayout(horiz);
    
  QLabel *left = new QLabel(frame);
  left->setText(leftLabel.c_str());
  horiz->addWidget(left);
  
  QLineEdit *right = new QLineEdit(frame);
  right->setText(rightContent.c_str());
  horiz->addWidget(right);

  layout->addWidget(frame);
  
  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  if (label != NULL) {
    *label = left;
  }
  return right;

}
*/
/* only used by BScanManager
//////////////////////////////////////////////////
// create a user input row in a widget

QLineEdit *PolarManager::_addInputRow(QWidget *parent,
                                        QVBoxLayout *layout,
                                        const string &leftLabel,
                                        const string &rightContent,
                                        int fontSize,
                                        QFrame **framePtr)
  
{
  
  QFrame *frame = new QFrame(parent);
  QHBoxLayout *horiz = new QHBoxLayout;
  frame->setLayout(horiz);
    
  QLabel *left = new QLabel(frame);
  left->setText(leftLabel.c_str());
  horiz->addWidget(left);
  
  QLineEdit *right = new QLineEdit(frame);
  right->setText(rightContent.c_str());
  horiz->addWidget(right);

  layout->addWidget(frame);

  if (fontSize > 0) {
    QFont font = left->font();
    font.setPixelSize(fontSize);
    left->setFont(font);
    right->setFont(font);
  }

  if (framePtr != NULL) {
    *framePtr = frame;
  }
  return right;

}
*/

//////////////////////////////////////////////
// update the status panel

void PolarManager::_updateStatusPanel(const RadxRay *ray)
{

  // set time etc

  char text[1024];
  
  QString prev_radar_name = _radarName->text();
  
  string rname(_platform.getInstrumentName());
  if (_params->override_radar_name) rname = _params->radar_name;
  if (_params->display_site_name) {
    rname += ":";
    if (_params->override_site_name) {
      rname += _params->site_name;
    } else {
      rname += _platform.getSiteName();
    }
  }
  _radarName->setText(rname.c_str());

  if (prev_radar_name != _radarName->text()) {
    _setTitleBar(rname);
  }
  
  DateTime rayTime(ray->getTimeSecs());
  sprintf(text, "%.4d/%.2d/%.2d",
          rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());
  _dateVal->setText(text);

  sprintf(text, "%.2d:%.2d:%.2d.%.3d",
          rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
          ((int) ray->getNanoSecs() / 1000000));
  _timeVal->setText(text);
  
  if (_volNumVal) {
    _setText(text, "%d", ray->getVolumeNumber());
    _volNumVal->setText(text);
  }
  
  if (_sweepNumVal) {
    _setText(text, "%d", ray->getSweepNumber());
    _sweepNumVal->setText(text);
  }
  
  if (_fixedAngVal) {  
    _setText(text, "%6.2f", ray->getFixedAngleDeg());
    _fixedAngVal->setText(text);
  }
  
  if (_elevVal) {
    if (fabs(ray->getElevationDeg()) < 1000) {
      _setText(text, "%6.2f", ray->getElevationDeg());
      _elevVal->setText(text);
    }
  }

  if (_azVal) {
    if (fabs(ray->getAzimuthDeg()) < 1000) {
      _setText(text, "%6.2f", ray->getAzimuthDeg());
      _azVal->setText(text);
    }
  }
  
  if (_nSamplesVal) {
    _setText(text, "%d", (int) ray->getNSamples());
    _nSamplesVal->setText(text);
  }
  
  if (_nGatesVal) {
    _setText(text, "%d", (int) ray->getNGates());
    _nGatesVal->setText(text);
  }
  
  if (_gateSpacingVal) {
    _setText(text, "%.4f", ray->getGateSpacingKm());
    _gateSpacingVal->setText(text);
  }
  
  if (_pulseWidthVal) {
    _setText(text, "%.2f", ray->getPulseWidthUsec());
    _pulseWidthVal->setText(text);
  }

  if (_prfVal) {
    if (ray->getPrtMode() == Radx::PRT_MODE_FIXED) {
      if (ray->getPrtSec() <= 0) {
        _setText(text, "%d", -9999);
      } else {
        _setText(text, "%d", (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10);
      }
    } else {
      double prtSec = ray->getPrtSec();
      if (prtSec <= 0) {
        _setText(text, "%d", -9999);
      } else {
        int iprt = (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10;
        double prtRatio = ray->getPrtRatio();
        if (prtRatio > 0.6 && prtRatio < 0.7) {
          _setText(text, "%d(2/3)", iprt);
        } else if (prtRatio < 0.775) {
          _setText(text, "%d(3/4)", iprt);
        } else if (prtRatio < 0.825) {
          _setText(text, "%d(4/5)", iprt);
        } else {
          _setText(text, "%d", iprt);
        }
      }
    }
    _prfVal->setText(text);
  }

  if (_nyquistVal) {
    if (fabs(ray->getNyquistMps()) < 1000) {
      _setText(text, "%.1f", ray->getNyquistMps());
      _nyquistVal->setText(text);
    }
  }

  if (_maxRangeVal) {
    double maxRangeData = ray->getStartRangeKm() +
      ray->getNGates() * ray->getGateSpacingKm();
    _setText(text, "%.1f", maxRangeData);
    _maxRangeVal->setText(text);
  }

  if (_unambigRangeVal) {
    if (fabs(ray->getUnambigRangeKm()) < 100000) {
      _setText(text, "%.1f", ray->getUnambigRangeKm());
      _unambigRangeVal->setText(text);
    }
  }
  
  if (_powerHVal) {
    if (ray->getMeasXmitPowerDbmH() > -9990) {
      _setText(text, "%.1f", ray->getMeasXmitPowerDbmH());
      _powerHVal->setText(text);
    }
  }
   
  if (_powerVVal) {
    if (ray->getMeasXmitPowerDbmV() > -9990) {
      _setText(text, "%.1f", ray->getMeasXmitPowerDbmV());
      _powerVVal->setText(text);
    }
  }

  if (_scanNameVal) {
    string scanName = ray->getScanName();
    size_t len = scanName.size();
    if (len > 8) {
      scanName = scanName.substr(0, 8);
    }
    _scanNameVal->setText(scanName.c_str());
  }

  if (_sweepModeVal) {
    switch (ray->getSweepMode()) {
      case Radx::SWEEP_MODE_SECTOR: {
        _sweepModeVal->setText("sector"); break;
      }
      case Radx::SWEEP_MODE_COPLANE: {
        _sweepModeVal->setText("coplane"); break;
      }
      case Radx::SWEEP_MODE_RHI: {
        _sweepModeVal->setText("RHI"); break;
      }
      case Radx::SWEEP_MODE_VERTICAL_POINTING: {
        _sweepModeVal->setText("vert"); break;
      }
      case Radx::SWEEP_MODE_IDLE: {
        _sweepModeVal->setText("idle"); break;
      }
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE: {
        _sweepModeVal->setText("SUR"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN: {
        _sweepModeVal->setText("sunscan"); break;
      }
      case Radx::SWEEP_MODE_SUNSCAN_RHI: {
        _sweepModeVal->setText("sun_rhi"); break;
      }
      case Radx::SWEEP_MODE_POINTING: {
        _sweepModeVal->setText("point"); break;
      }
      case Radx::SWEEP_MODE_CALIBRATION: {
        _sweepModeVal->setText("cal"); break;
      }
      default: {
        _sweepModeVal->setText("unknown");
      }
    }
  }

  if (_polModeVal) {
    _polModeVal->setText(Radx::polarizationModeToStr
                         (ray->getPolarizationMode()).c_str());
  }
   
  if (_prfModeVal) {
    _prfModeVal->setText(Radx::prtModeToStr
                         (ray->getPrtMode()).c_str());
  }

  if (fabs(_radarLat - _platform.getLatitudeDeg()) > 0.0001 ||
      fabs(_radarLon - _platform.getLongitudeDeg()) > 0.0001 ||
      fabs(_radarAltKm - _platform.getAltitudeKm()) > 0.0001) {
    _radarLat = _platform.getLatitudeDeg();
    _radarLon = _platform.getLongitudeDeg();
    _radarAltKm = _platform.getAltitudeKm();
    _sunPosn.setLocation(_radarLat, _radarLon, _radarAltKm * 1000.0);
  }

  if (ray->getGeoreference() != NULL) {

    if (_latVal) {
      _radarLat = ray->getGeoreference()->getLatitude();
      _setText(text, "%.3f", _radarLat);
      _latVal->setText(text);
    }
     
    if (_lonVal) {
      _radarLon = ray->getGeoreference()->getLongitude();
      _setText(text, "%.3f", _radarLon);
      _lonVal->setText(text);
    }

    if (_altVal) {
      _radarAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      if (_altitudeInFeet) {
        _setText(text, "%.3f", _radarAltKm / 0.3048);
      } else {
        _setText(text, "%.3f", _radarAltKm);
      }
      _altVal->setText(text);
    }

    // compute altitude rate every 2 secs
    
    if (_prevAltKm > -9990) {
      double deltaTime = ray->getRadxTime() - _prevAltTime;
      if (deltaTime > 2.0) {
        double altKm = ray->getGeoreference()->getAltitudeKmMsl();
        double deltaAltKm = altKm - _prevAltKm;
        _altRateMps = (deltaAltKm / deltaTime) * 1000.0;
        _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
        _prevAltTime = ray->getRadxTime();
      }
    } else {
      _prevAltKm = ray->getGeoreference()->getAltitudeKmMsl();
      _prevAltTime = ray->getRadxTime();
    }
    
    if (_altitudeInFeet) {
      _setText(text, "%.1f", _altRateMps / 0.3048);
    } else {
      _setText(text, "%.1f", _altRateMps);
    }
    if (_altRateVal) {
      _altRateVal->setText(text);
    }

    if (_speedVal) {
      double ewVel = ray->getGeoreference()->getEwVelocity();
      double nsVel = ray->getGeoreference()->getNsVelocity();
      double speed = sqrt(ewVel * ewVel + nsVel * nsVel);
      _setText(text, "%.2f", speed);
      _speedVal->setText(text);
    }
     
    if (_headingVal) {
      double heading = ray->getGeoreference()->getHeading();
      if (heading >= 0 && heading <= 360.0) {
        _setText(text, "%.2f", heading);
        _headingVal->setText(text);
      }
    }
     
    if (_trackVal) {
      double track = ray->getGeoreference()->getTrack();
      if (track >= 0 && track <= 360.0) {
        _setText(text, "%.2f", track);
        _trackVal->setText(text);
      }
    }
     
  } else {
    
    _setText(text, "%.3f", _platform.getLatitudeDeg());
    if (_latVal) {
      _latVal->setText(text);
    }
    
    _setText(text, "%.3f", _platform.getLongitudeDeg());
    if (_lonVal) {
      _lonVal->setText(text);
    }
    
    if (_altitudeInFeet) {
      _setText(text, "%.3f", _platform.getAltitudeKm() / 0.3048);
    } else {
      _setText(text, "%.3f", _platform.getAltitudeKm());
    }
    if (_altVal) {
      _altVal->setText(text);
    }
    if (_altRateVal) {
      _altRateVal->setText("0.0");
    }

  }
  
  double sunEl, sunAz;
  _sunPosn.computePosn(ray->getTimeDouble(), sunEl, sunAz);
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

void PolarManager::_setText(char *text,
                              const char *format,
                              int val)
{
  if (abs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999);
  }
}

void PolarManager::_setText(char *text,
                              const char *format,
                              double val)
{
  if (fabs(val) < 9999) {
    sprintf(text, format, val);
  } else {
    sprintf(text, format, -9999.0);
  }
}

////////////////////////////////////////////////////////////////

double PolarManager::_getInstHtKm(const RadxRay *ray)

{
  double instHtKm = _platform.getAltitudeKm();
  if (ray->getGeoreference() != NULL) {
    instHtKm = ray->getGeoreference()->getAltitudeKmMsl();
  }
  return instHtKm;
}

/////////////////////////////////////////////////////////////////////  
// slots

/////////////////////////////
// show data at click point
void PolarManager::_showClick()
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

/*
  //////////////////////////////////////////////////
// set up field objects, with their color maps
// use same map for raw and unfiltered fields
// returns 0 on success, -1 on failure
HERE ==> call this with the _params->fields at initialization 
or move to HawkEye.cc  YES! Do this in HawkEye, the code is there, just commented.
Then, on updateDisplayFields, check if the field is already there, before adding it.
int PolarManager::_setupDisplayFields(
  string colorMapDir, //  = _params->color_scale_dir,
  vector<string> label,
  vector<string> raw_name,
  vector<string> filtered_name,
  vector<string> units,
  vector<string> color_map_path,
  vector<string> shortcut,
  // include in vector: _params->fields_n,
  //const Params::field_t &pfld = _params->_fields,
  bool debug = _params->debug,  
  )
{

  // check for color map location
  
  string colorMapDir = _params->color_scale_dir;
  Path mapDir(_params->color_scale_dir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params->color_scale_dir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - HawkEye" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params->color_scale_dir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      return -1;
    }
    if (_params->debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

  // we interleave unfiltered fields and filtered fields

  for (int ifield = 0; ifield < _params->fields_n; ifield++) {

    const Params::field_t &pfld = _params->_fields[ifield];

    // check we have a valid label
    
    if (strlen(pfld.label) == 0) {
      cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Empty field label, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }
    
    // check we have a raw field name
    
    if (strlen(pfld.raw_name) == 0) {
      cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
      cerr << "  Empty raw field name, ifield: " << ifield << endl;
      cerr << "  Ignoring" << endl;
      continue;
    }

    // create color map
    
    string colorMapPath = colorMapDir;
    colorMapPath += PATH_DELIM;
    colorMapPath += pfld.color_map;
    ColorMap map;
    map.setName(pfld.label);
    map.setUnits(pfld.units);
    // TODO: the logic here is a little weird ... the label and units have been set, but are we throwing them away?

    bool noColorMap = false;

    if (map.readMap(colorMapPath)) {
        cerr << "WARNING - HawkEye::_setupDisplayFields()" << endl;
        cerr << "  Cannot read in color map file: " << colorMapPath << endl;
        cerr << "  Looking for default color map for field " << pfld.label << endl; 

        try {
          // check here for smart color scale; look up by field name/label and
          // see if the name is a usual parameter for a known color map
          SoloDefaultColorWrapper sd = SoloDefaultColorWrapper::getInstance();
          ColorMap colorMap = sd.ColorMapForUsualParm.at(pfld.label);
          cerr << "  found default color map for " <<  pfld.label  << endl;
          // if (_params->debug) colorMap.print(cout); // LOG(DEBUG_VERBOSE)); // cout);
          map = colorMap;
          // HERE: What is missing from the ColorMap object??? 
        } catch (std::out_of_range ex) {
          cerr << "WARNING - did not find default color map for field; using rainbow colors" << endl;
    // Just set the colormap to a generic color map
    // use range to indicate it needs update; update when we have access to the actual data values
          map = ColorMap(0.0, 1.0);
    noColorMap = true; 
          // return -1
        }
    }

    // unfiltered field

    DisplayField *field =
      new DisplayField(pfld.label, pfld.raw_name, pfld.units, 
                       pfld.shortcut, map, ifield, false);
    if (noColorMap)
      field->setNoColorMap();

    _displayFields.push_back(field);

    // filtered field

    if (strlen(pfld.filtered_name) > 0) {
      string filtLabel = string(pfld.label) + "-filt";
      DisplayField *filt =
        new DisplayField(filtLabel, pfld.filtered_name, pfld.units, pfld.shortcut, 
                         map, ifield, true);
      _displayFields.push_back(filt);
    }

  } // ifield

  if (_displayFields.size() < 1) {
    cerr << "ERROR - HawkEye::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }

  return 0;

}
*/

//TODO: change model for displayFieldController
//_displayFieldController->setModel(new DisplayFieldModel(...))
  
// reset or sync the displayFields with those in the list
// used for a read of new data file
// TODO: shouldn't this go to DisplayFieldController?? just send the color map directory?
int PolarManager::_updateDisplayFields(vector<string> *fieldNames) {

  _displayFieldController->reconcileFields(fieldNames, _fieldPanel);

  // TODO: not sure where this needs to happen ...
  // check for color map location
  
  string colorMapDir = _params->color_scale_dir;
  Path mapDir(_params->color_scale_dir);
  if (!mapDir.dirExists()) {
    colorMapDir = Path::getPathRelToExec(_params->color_scale_dir);
    mapDir.setPath(colorMapDir);
    if (!mapDir.dirExists()) {
      cerr << "ERROR - DisplayManager" << endl;
      cerr << "  Cannot find color scale directory" << endl;
      cerr << "  Primary is: " << _params->color_scale_dir << endl;
      cerr << "  Secondary is relative to binary: " << colorMapDir << endl;
      return -1;
    }
    if (_params->debug) {
      cerr << "NOTE - using color scales relative to executable location" << endl;
      cerr << "  Exec path: " << Path::getExecPath() << endl;
      cerr << "  Color scale dir:: " << colorMapDir << endl;
    }
  }

/*
  //vector<DisplayField *> displayFields;

  // There is the fieldPanel, which is a view of the DisplayFields, for selection
  // then, there is the displayFieldController which manages the fields and all
  // their attributes.  
  // sometimes, we need to add a field, remove a field, and then sync the fields as in reset.

  //for (int ifield = 0; ifield < _params->fields_n; ifield++) {
  int ifield = (int) _displayFieldController->getNFields() + 1;
  for (vector<string>::iterator it = fieldNames->begin(); it != fieldNames->end(); ++it) {
    string fieldName = *it;

//HERE TODO:
//distingquish between add and update on fieldName;
//then set last field or first field as selected? or do something to render image

    if (!_displayFieldController->contains(fieldName)) {

      ColorMap map;
      map = ColorMap(0.0, 1.0);
      bool noColorMap = true; 
      // unfiltered field
      string shortcut = to_string(ifield);
      DisplayField *field =
        new DisplayField(fieldName, fieldName, "m/s", 
                         shortcut, map, ifield, false);
      if (noColorMap)
        field->setNoColorMap();

      //displayFields.push_back(field);
      _displayFieldController->addField(field);
      //_updateFieldPanel(fieldName);
      // TODO: causes a EXC_BAD_ACCESS if outside the loop
      // somehow this is called when setting up the menus???
      //_fieldPanel->updateFieldPanel(fieldName, fieldName, fieldName);
      ifield += 1;
    }
    _fieldPanel->updateFieldPanel(fieldName, fieldName, fieldName);


  } // ifield
*/

  if (fieldNames->size() < 1) {
    cerr << "ERROR - PolarManager::_setupDisplayFields()" << endl;
    cerr << "  No fields found" << endl;
    return -1;
  }


  return 0;

}

void PolarManager::_scriptEditorSetup() {
  LOG(DEBUG) << "enter";
  try {
    EditRunScript();
  } catch (const string& ex) {
    errorMessage("EditRunScript Error", ex);
  }  
  LOG(DEBUG) << "exit";  
}


void PolarManager::EditRunScript() {
  LOG(DEBUG) << "enter";
  if (scriptEditorView == NULL) {
    // create the view

    scriptEditorView = new ScriptEditorView(this);

    // create the model
    ScriptEditorModel *model = new ScriptEditorModel(); // _vol);
    
    // create the controller
    //ScriptEditorController *
    scriptEditorControl = new ScriptEditorController(scriptEditorView, model);

    // connect some signals and slots in order to retrieve information
    // and send changes back to display
                                                                           
    connect(scriptEditorControl, SIGNAL(scriptChangedVolume(QStringList)),
        this, SLOT(updateVolume(QStringList)));
    connect(scriptEditorView, SIGNAL(scriptEditorClosed()), this, SLOT(scriptEditorClosed()));
    //connect scriptEditorView, SIGNAL(runForEachRayScript(), 
    //  this, SLOT(runForEachRayScript()));
    
    scriptEditorView->init();
  }
  scriptEditorView->show();
  // scriptEditorView->layout()->setSizeConstraint(QLayout::SetFixedSize);
  LOG(DEBUG) << "exit";
}

void PolarManager::runForEachRayScript(QString script, bool useBoundary, bool useAllSweeps) {
  vector<Point> boundaryPoints;
  if (boundaryPointEditorControl != NULL) {
    boundaryPoints = boundaryPointEditorControl->getWorldPoints();
  }
  if (useAllSweeps) {
    scriptEditorControl->runForEachRayScript(script, useBoundary, boundaryPoints);
  } else {
    // send the current sweep to the script editor controller
    int currentSweepIndex = _sweepController->getSelectedNumber();
    currentSweepIndex -= 1; // since GUI is 1-based and Volume sweep 
    // index is a vector and zero-based 
    scriptEditorControl->runForEachRayScript(script, currentSweepIndex,
     useBoundary, boundaryPoints);
  }
}

void PolarManager::_examineSpreadSheetSetup(double  closestAz, double range)
{
  LOG(DEBUG) << "enter";

  // get click location in world coords
  // by using the location stored in class variables
  //double x_km = _worldPressX;
  //double y_km = _worldPressY;

  // get azimuth closest to click point
  //double  closestAz =  30.0; // _getClosestAz(x_km, y_km);
  // TODO: make sure the point is in the valid area
  //if (closestRay == NULL) {
    // report error
  //  QMessageBox::information(this, QString::fromStdString(""), QString::fromStdString("No ray found at location clicked"));
    // TODO: move to this ...  errorMessage("", "No ray found at location clicked");
  //} else {
  try {
    double elevation = getSelectedSweepAngle();
    size_t fieldIdx = getSelectedFieldIndex();
    LOG(DEBUG) << "elevation=" << elevation << ", fieldIdx=" << fieldIdx;
    ExamineEdit(closestAz, elevation, fieldIdx, range);
  } catch (const string& ex) {
    errorMessage("ExamineEdit Error", ex);
  }
  LOG(DEBUG) << "exit";
}


void PolarManager::ExamineEdit(double azimuth, double elevation, size_t fieldIndex,
  double range) {   

  // get an version of the ray that we can edit
  // we'll need the az, and sweep number to get a list from
  // the volume

  LOG(DEBUG) << "azimuth=" << azimuth << ", elevation=" << elevation << ", fieldIndex=" << fieldIndex;
  // TODO: replace with ...
  const RadxRay *closestRayToEdit = _rayLocationController->getClosestRay(azimuth);

  LOG(DEBUG) << "Found closest ray: pointer = " << closestRayToEdit;
  //closestRayToEdit->print(cout); 


  // create the view
  //SpreadSheetView *sheetView;
  if (sheetView == NULL) {
    sheetView = new SpreadSheetView(this, closestRayToEdit->getAzimuthDeg(),
      _sweepController->getSelectedAngle());

    // install event filter to catch when the spreadsheet is closed
    CloseEventFilter *closeFilter = new CloseEventFilter(sheetView);
    sheetView->installEventFilter(closeFilter);


    // create the model

    // SpreadSheetModel *model = new SpreadSheetModel(closestRayCopy);
    SpreadSheetModel *model = new SpreadSheetModel(const_cast<RadxRay *> (closestRayToEdit)); // , _vol);
    //SpreadSheetModel *model = new SpreadSheetModel(closestRay, _vol);
    
    // create the controller
    spreadSheetControl = new SpreadSheetController(sheetView, model);

    // finish the other connections ..
    //sheetView->addController(sheetController);
    // model->setController(sheetController);

    // connect some signals and slots in order to retrieve information
    // and send changes back to display
                                                                           
    connect(sheetView, SIGNAL(replotRequested()),
        this, SLOT(setVolume()));
    connect(sheetView, SIGNAL(spreadSheetClosed()), this, SLOT(spreadSheetClosed()));
    connect(sheetView, SIGNAL(setDataMissing(string, float)), 
      this, SLOT(setDataMissing(string, float)));    
//HERE ==> do i update the controller, which will update the model, then the view?
//Yes, always go through the controller, never directly to the view!!
//    spreadSheetControl->newElevation(elevation);

    connect(sheetView, SIGNAL(dataChanged()), 
      this, SLOT(spreadsheetDataChanged()));

    sheetView->init();
    sheetView->show();
  } else {
    string currentFieldName = _displayFieldController->getSelectedFieldName();
    //spreadSheetControl->switchRay(closestRayToEdit->getAzimuthDeg(), elevation);
    float azimuth = closestRayToEdit->getAzimuthDeg();
    float elevation = _sweepController->getSelectedAngle();
    spreadSheetControl->moveToLocation(currentFieldName, elevation,
      azimuth, range);
    //spreadSheetControl->changeAzEl(closestRayToEdit->getAzimuthDeg(), elevation);   
    // should be called withing Controller ... 

    //spreadSheetControl->highlightClickedData(currentFieldName, azimuth, (float) range);
  }
  
}

void PolarManager::setDataMissing(string fieldName, float missingValue) {
  spreadSheetControl->setDataMissing(fieldName, missingValue);
  _applyDataEdits();
}

void PolarManager::spreadSheetClosed() {
  //delete sheetView;  this is handled by the close event
  sheetView = NULL;
}

void PolarManager::scriptEditorClosed() {
  //delete View;  this is handled by the close event
  scriptEditorView = NULL;
}

void PolarManager::boundaryEditorClosed() {
  //delete View;  this is handled by the close event
  boundaryPointEditorView = NULL;
  delete boundaryPointEditorControl;
  boundaryPointEditorControl = NULL;
  delete boundaryView;
  boundaryView = NULL;
}

void PolarManager::closeEvent(QEvent *event)
{
    if (_unSavedEdits) {
        string msg = "Unsaved changes to the data. \n";
        msg.append("Use File->Save before closing to avoid this message. \n");
        msg.append("Do you want to save these changes?");

        QMessageBox::StandardButton reply =
            QMessageBox::warning(this, "QMessageBox::warning()",
                          QString::fromStdString(msg),
                          QMessageBox::Save | QMessageBox::Discard);
  
        //  QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
        if (reply == QMessageBox::Save) {
            LOG(DEBUG) << "Save";
            _saveFile();
            _unSavedEdits = false;
        }

    }  
    emit close();
    //QMainWindow::closeEvent(event);
}

/////////////////////////////////////////////////////
// howto help
/*
void PolarManager::_howto()
{
  string text;
  text += "HOWTO HINTS FOR HAWK-EYE\n";
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
*/

void PolarManager::_about()
{
  //QMessageBox::about(this, tr("About Menu"),
         //tr("HawkEye is an engineering display for beam-by-beam radar data. "));
  string text;
  
  text += "HawkEdit is an LROSE application for engineering and research display of radar data. \n\n";
  text += "Get help with HawkEdit ...  \n ";
  text += "\nReport an issue https://github.com/NCAR/lrose-core/issues \n ";
  text += "\nHawkEdit Version ... \n ";  
  text += "\nCopyright UCAR (c) 2019 - 2021  ";  
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


void PolarManager::errorMessage(string title, string message) {
  QMessageBox::information(this, QString::fromStdString(title), QString::fromStdString(message));
}

