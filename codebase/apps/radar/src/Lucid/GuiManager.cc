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
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>
#include <QActionGroup>
#include <QApplication>
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
#include <QStatusBar>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QTimeZone>
#include <QColor>
#include <QDir>
#include <QDoubleSpinBox>
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
#include <QFileInfo>
#include <QRegularExpression>
#include <QStringList>

#include <fstream>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/TaStr.hh>
#include <radar/RadarComplex.hh>
#include <toolsa/DateTime.hh>
#include <Ncxx/H5x.hh>

#include "GuiManager.hh"
#include "ImageGenerationManager.hh"
#include "MovieManager.hh"
#include "ProductMgr.hh"
#include "FieldTableItem.hh"
#include "HorizView.hh"
#include "MapMenuItem.hh"
#include "MdvReader.hh"
#include "ProdMenuItem.hh"
#include "TimeControl.hh"
#include "VlevelSelector.hh"
#include "VertView.hh"
#include "VertManager.hh"
#include "WindMenuItem.hh"
#include "WindManager.hh"
#include "ZoomMenuItem.hh"

// GlobalData.hh must be included AFTER qt includes because of #define None in X11/X.h

#include "GlobalData.hh"

using namespace std;
using namespace H5x;

namespace {

// Place numeric controls directly in the context menu so related contour and
// color-scale values can be inspected and edited together.

QDoubleSpinBox *addFieldNumberEditor(QMenu *menu, const QString &label,
                                     double value,
                                     int decimals = 6,
                                     double minimum = -1.0e12,
                                     double maximum = 1.0e12)
{
  QWidget *editor = new QWidget(menu);
  QHBoxLayout *layout = new QHBoxLayout(editor);
  layout->setContentsMargins(8, 2, 8, 2);

  QLabel *nameLabel = new QLabel(label, editor);
  nameLabel->setMinimumWidth(65);
  layout->addWidget(nameLabel);

  QDoubleSpinBox *spinBox = new QDoubleSpinBox(editor);
  spinBox->setRange(minimum, maximum);
  spinBox->setDecimals(decimals);
  spinBox->setValue(value);
  spinBox->setKeyboardTracking(false);
  layout->addWidget(spinBox);

  QWidgetAction *action = new QWidgetAction(menu);
  action->setDefaultWidget(editor);
  menu->addAction(action);
  return spinBox;
}

QString safeFileNameComponent(QString value)
{
  value = value.trimmed();
  value.replace(QRegularExpression("[^A-Za-z0-9._-]+"), "_");
  value.remove(QRegularExpression("^_+|_+$"));
  return value.isEmpty() ? QString("field") : value;
}

} // namespace

GuiManager* GuiManager::m_pInstance = NULL;

GuiManager* GuiManager::Instance()
{
  return m_pInstance;
}

// Constructor

GuiManager::GuiManager() :
        _params(Params::Instance()),
        _gd(GlobalData::Instance()),
        _automaticSavedVertWindowVisible(false)
{

  _timerEventCount = 0;
  _archiveMode = true;
  _mapsEnabled = _params.maps_enabled_at_startup;
  
  m_pInstance = this;

  // initialize

  _main = NULL;
  _resizeTimer = NULL;
  _resized = false;
  
  _horizFrame = NULL;
  _horiz = NULL;

  _clickReportDialog = NULL;
  _clickReportDialogLayout = NULL;
  _clickDate = NULL;
  _clickTime = NULL;
  _clickLatitude = NULL;
  _clickLongitude = NULL;
  _clickHeightName = NULL;
  _clickHeight = NULL;
  _clickXName = NULL;
  _clickX = NULL;
  _clickYName = NULL;
  _clickY = NULL;
  _clickFieldsTable = NULL;
  _clickReportPlaced = false;

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
  _horizontalRequestHasChanged = false;
  
  _fieldTableCol = 0;
  _fieldTableRow = 0;
  
  _prevFieldTableCol = -1;
  _prevFieldTableRow = -1;

  _vlevelManager.requestLevel(_params.start_ht_km);
  _vlevelHasChanged = true;
  
  _overlaysHaveChanged = true;
  // MovieManager owns the immutable cache lifecycle. These callbacks retain
  // request and widget operations in the display controller.
  MovieManager::Callbacks movieCallbacks;
  movieCallbacks.clearFrames =
    [this](int frameCount, bool includeVertical) {
      if (_horiz) {
        _horiz->clearMovieFrames(frameCount);
      }
      if (_vert) {
        _vert->clearMovieFrames(includeVertical ? frameCount : 0);
      }
    };
  movieCallbacks.resetProducts = [this]() {
    if (_gd.prod_mgr) {
      _gd.prod_mgr->reset_product_valid_flags();
    }
  };
  movieCallbacks.beginFrameRequest = [this](bool includeVertical) {
    _terrainHorizontalSettled = true;
    _terrainHorizontalRequestPending = false;
    _terrainVerticalSettled = true;
    _horizontalRequestHasChanged = true;
    if (_horiz) {
      _horiz->stopMoviePlayback();
    }
    if (includeVertical && _vert) {
      _vert->stopMoviePlayback();
      return _requestVerticalSection();
    }
    return -1;
  };
  movieCallbacks.captureFrame =
    [this](int frameIndex, bool includeVertical, QString &error) {
      if (!_horiz || !_horiz->captureMovieFrame(frameIndex)) {
        error = tr("Could not capture the horizontal view");
        return false;
      }
      if (includeVertical &&
          (!_vert || !_vert->captureMovieFrame(frameIndex))) {
        error = tr("Could not capture the vertical-section view");
        return false;
      }
      return true;
    };
  movieCallbacks.showFrame =
    [this](int frameIndex, bool includeVertical) {
      if (!_horiz || !_horiz->hasMovieFrame(frameIndex) ||
          (includeVertical &&
           (!_vert || !_vert->hasMovieFrame(frameIndex)))) {
        return false;
      }
      if (!_horiz->showMovieFrame(frameIndex)) {
        return false;
      }
      return !includeVertical || _vert->showMovieFrame(frameIndex);
    };
  movieCallbacks.hasFrame =
    [this](int frameIndex, bool vertical) {
      return vertical ? (_vert && _vert->hasMovieFrame(frameIndex)) :
                        (_horiz && _horiz->hasMovieFrame(frameIndex));
    };
  movieCallbacks.frameImage =
    [this](int frameIndex, bool vertical) {
      return vertical ? _vert->movieFrameImage(frameIndex) :
                        _horiz->movieFrameImage(frameIndex);
    };
  movieCallbacks.stopPlayback = [this]() {
    if (_horiz) {
      _horiz->stopMoviePlayback();
    }
    if (_vert) {
      _vert->stopMoviePlayback();
    }
  };
  movieCallbacks.frameRendered = [this](int frameIndex) {
    if (_timeControl) {
      _timeControl->movieFrameRendered(frameIndex);
    }
  };
  movieCallbacks.frameCaptureFailed =
    [this](int frameIndex, const QString &error) {
      if (_timeControl) {
        _timeControl->movieFrameCaptureFailed(frameIndex, error);
      }
    };
  movieCallbacks.pendingDependencies = [this]() {
    QStringList pending;
    if (_gd.prod.products_on && _gd.prod_mgr &&
        !_gd.prod_mgr->activeProductsReady()) {
      pending << tr("symbolic products");
    }
    if (_gd.wind_mgr && !_gd.wind_mgr->activeWindsReady()) {
      pending << tr("horizontal winds");
    }
    if (_movieManager && _movieManager->includesVertical() &&
        _gd.wind_mgr && !_gd.wind_mgr->activeVerticalWindsSettled()) {
      pending << tr("vertical winds");
    }
    if (!_terrainHorizontalSettled) {
      pending << tr("horizontal terrain");
    }
    if (_movieManager && _movieManager->includesVertical() &&
        !_terrainVerticalSettled) {
      pending << tr("vertical terrain");
    }
    return pending;
  };
  movieCallbacks.showProgress = [this](const QString &message) {
    statusBar()->showMessage(message);
  };
  _movieManager = new MovieManager(
    _params, std::move(movieCallbacks), this);

  _terrainHorizontalRequestGeneration = -1;
  _terrainHorizontalSettled = true;
  _terrainHorizontalRequestPending = false;
  _terrainVerticalRequestGeneration = -1;
  _terrainVerticalSettled = true;

  _automaticDisplayStateSaved = false;
  _automaticSavedField = -1;
  _automaticSavedZoom = 0;
  _automaticSavedHeight = 0.0;
  _automaticSavedFrameCount = 1;
  _automaticSavedFrameIntervalSecs = 1.0;
  _automaticSavedFrameIndex = 0;
  _automaticSavedRealtime = false;

  // ImageGenerationManager owns all unattended lifecycle state. These
  // callbacks are the narrow boundary for operations which must touch the
  // live display and its asynchronous MDV readers.
  ImageGenerationManager::Callbacks imageCallbacks;
  imageCallbacks.batchStarted = [this](const DateTime &imageTime) {
    _timeControl->configureAutomaticImageTime(imageTime);
  };
  imageCallbacks.saveDisplayState = [this]() {
    _saveAutomaticDisplayState();
  };
  imageCallbacks.restoreDisplayState = [this]() {
    _restoreAutomaticDisplayState();
  };
  imageCallbacks.jobRequested = [this]() {
    _startAutomaticImageJob();
  };
  imageCallbacks.cancelTimedOutRequest =
    [this](const AutomaticImageGenerator::Job &job,
           int expectedGeneration) {
      _cancelTimedOutImageRequest(
        job.fieldIndex,
        job.type == AutomaticImageGenerator::JobType::Vertical,
        expectedGeneration);
    };
  imageCallbacks.captureCurrentJob =
    [this](ImageGenerationManager::Capture &capture, QString &error) {
      return _captureAutomaticImage(
        capture.image, capture.dataTime, capture.world,
        capture.writeGeoXml, error);
    };
  imageCallbacks.runFinished = [](bool success) {
    QTimer::singleShot(0, qApp, [success]() {
      QCoreApplication::exit(success ? EXIT_SUCCESS : EXIT_FAILURE);
    });
  };
  _imageGenerationManager = new ImageGenerationManager(
    _params, _gd.num_datafields, std::move(imageCallbacks), this);
  
  _timeControl = NULL;
  _timeControlPlaced = false;

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

  // Stop the manager-owned timers and release callbacks into this object
  // before tearing down any views or data managers they may reference.
  delete _imageGenerationManager;
  _imageGenerationManager = nullptr;
  delete _movieManager;
  _movieManager = nullptr;

  if (_gd.prod_mgr) {
    _gd.prod_mgr->setProductsChangedCallback(std::function<void()>());
    _gd.prod_mgr->setStatusCallback(
      std::function<void(const string &, bool)>());
  }

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
  const bool spatialChangesAllowed =
    !_params.create_images_automatically && !isMovieActive();
  _zoomBackAct->setEnabled(spatialChangesAllowed && val);
  _zoomOutAct->setEnabled(spatialChangesAllowed && val);
  if (!spatialChangesAllowed) {
    for (size_t ii = 0; ii < _zoomMenuItems.size(); ii++) {
      _zoomMenuItems[ii]->getAction()->setEnabled(false);
    }
  } else if (val) {
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
  if (isMovieActive()) {
    showMovieInteractionBlocked();
    return;
  }
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

////////////////////////////////////////////////////////
// initialize storage and request state for a new movie build

void GuiManager::prepareMovieBuild(int frameCount)
{
  const bool includeVertical = _vertWindow && _vert &&
    _vertWindow->isVisible() && _vsectionRoute.size() >= 2;
  _terrainHorizontalSettled = true;
  _terrainHorizontalRequestPending = false;
  _terrainVerticalSettled = true;
  _movieManager->prepareBuild(
    frameCount, includeVertical,
    QString::fromStdString(getSelectedFieldLabel()));
}

void GuiManager::beginMovieFrameBuild(int frameIndex)
{
  _movieManager->beginFrameBuild(
    frameIndex, _timeControl ? _timeControl->getNFramesMovie() : 0);
}

void GuiManager::cancelMovieBuild()
{
  _movieManager->cancelBuild();
  _terrainVerticalSettled = true;
}

bool GuiManager::showMovieFrame(int frameIndex)
{
  return _movieManager->showFrame(frameIndex);
}

bool GuiManager::exportMovieLoop(QString &message)
{
  if (!_timeControl) {
    message = tr("The movie time controller is not available.");
    return false;
  }
  return _movieManager->exportLoop(
    _timeControl->getNFramesMovie(), _timeControl->getStartTime(), message);
}

void GuiManager::stopMoviePlayback()
{
  _movieManager->stopPlayback();
}

void GuiManager::restoreLiveDisplayAfterMovie()
{
  stopMoviePlayback();
  if (_vertWindow && _vertWindow->isVisible() && _vsectionRoute.size() >= 2) {
    // Supersede any build request so the live section corresponds to the
    // exact frame at which playback was stopped.
    _requestVerticalSection();
  }
}

void GuiManager::stopMovieForDisplayChange()
{
  if (_timeControl && _timeControl->isMovieActive()) {
    _timeControl->stopMovie();
  }
}

bool GuiManager::isMovieBuilding() const
{
  return _timeControl && _timeControl->isMovieBuilding();
}

bool GuiManager::isMoviePlaying() const
{
  return _timeControl && _timeControl->isMoviePlaying();
}

bool GuiManager::isMovieActive() const
{
  return _timeControl && _timeControl->isMovieActive();
}

int GuiManager::getTimelineFrameCount() const
{
  return _timeControl ? _timeControl->getFrameCount() : 0;
}

int GuiManager::getTimelineFrameIndex() const
{
  return _timeControl ? _timeControl->getFrameIndex() : -1;
}

const TimelineFrame *GuiManager::getTimelineFrame(int frameIndex) const
{
  return _timeControl ? _timeControl->getFrame(frameIndex) : nullptr;
}

void GuiManager::movieStateChanged()
{
  const bool building = isMovieBuilding();
  const bool active = isMovieActive();
  const bool interactive = !_params.create_images_automatically;

  // A movie build must capture one stable display configuration. Playback is
  // less restrictive: field and level choices may update the hidden live
  // state and become visible after Stop, but spatial transforms would make
  // the immutable cached frames incompatible with that state. Automated image
  // instances remain permanently non-interactive even as movie state changes.
  if (_fieldTable) {
    _fieldTable->setEnabled(interactive && !building);
  }
  if (_vlevelSelector) {
    _vlevelSelector->setEnabled(interactive && !building);
  }
  if (_mapsMenu) {
    _mapsMenu->setEnabled(interactive && !building);
  }
  if (_windsMenu) {
    _windsMenu->setEnabled(interactive && !building);
  }
  if (_productsMenu) {
    _productsMenu->setEnabled(interactive && !building);
  }
  if (_overlaysMenu) {
    _overlaysMenu->setEnabled(interactive && !building);
  }
  if (_reloadAct) {
    _reloadAct->setEnabled(interactive && !building);
  }
  if (_clearAct) {
    _clearAct->setEnabled(interactive && !building);
  }
  if (_zoomsMenu) {
    _zoomsMenu->setEnabled(interactive && !active);
  }
  if (_showValueAct) {
    _showValueAct->setEnabled(interactive && !active);
  }
  if (_showClickAct) {
    _showClickAct->setEnabled(interactive && !active);
  }
  if (_horiz) {
    _horiz->setMovieInteractionActive(active);
  }
  if (_vertWindow) {
    _vertWindow->setMovieInteractionActive(active);
  }
  if (_showVertAct) {
    // A cached vertical loop may be hidden and reopened. If no vertical cache
    // was selected at Start, opening a live section during horizontal-only
    // playback would imply synchronization which does not exist.
    _showVertAct->setEnabled(
      interactive && (!active || _movieManager->includesVertical()));
  }

  enableUnzoomButtons(_horiz && !_horiz->getCustomZooms().empty());
}

void GuiManager::showMovieInteractionBlocked()
{
  statusBar()->showMessage(tr("Stop the movie to change the view"), 3000);
}

/////////////////////////////////////////////////////////////
// set visibility and request state for terrain in both field views

void GuiManager::setTerrainEnabled(bool enabled)
{
  MdvReader *terrain = _gd.layers.earth.terr;
  const bool active = enabled && terrain != nullptr;
  _params.terrain_active = active ? pTRUE : pFALSE;
  _gd.layers.earth.terrain_active = active ? 1 : 0;

  if (!active) {
    // A hidden optional layer must not hold a movie or automatic image open.
    // Supersede its asynchronous request so a late completion cannot restore
    // terrain after the user has switched it off.
    if (terrain != nullptr && _terrainVerticalRequestGeneration >= 0) {
      terrain->cancelVertRequest(_terrainVerticalRequestGeneration);
    }
    if (terrain != nullptr && _terrainHorizontalRequestGeneration >= 0) {
      terrain->cancelHorizRequest(_terrainHorizontalRequestGeneration);
    }
    _terrainHorizontalRequestGeneration = -1;
    _terrainHorizontalRequestPending = false;
    _terrainHorizontalSettled = true;
    _terrainVerticalRequestGeneration = -1;
    _terrainVerticalSettled = true;
    if (_horiz != nullptr) {
      _horiz->rerenderGrid();
    }
    if (_vert != nullptr) {
      _vert->rerenderTerrain();
    }
    _checkMovieFrameReady();
    _checkAutomaticImageReady();
    return;
  }

  // Either view may have changed while terrain was hidden. Reissue each
  // applicable request rather than assuming the cached geometry still fits.
  _requestHorizontalTerrain();
  _requestVerticalTerrain();
  if (_horiz != nullptr) {
    _horiz->rerenderGrid();
  }
  if (_vert != nullptr) {
    _vert->rerenderTerrain();
  }
}

void GuiManager::_checkMovieFrameReady()
{
  if (!_movieManager || _movieManager->buildFrame() < 0) {
    return;
  }
  const bool productsReady = !_gd.prod.products_on || !_gd.prod_mgr ||
    _gd.prod_mgr->activeProductsReady();
  const bool horizontalWindsReady = !_gd.wind_mgr ||
    _gd.wind_mgr->activeWindsReady();
  const bool verticalWindsReady =
    !_movieManager->includesVertical() || !_gd.wind_mgr ||
    _gd.wind_mgr->activeVerticalWindsSettled();
  const bool verticalTerrainReady =
    !_movieManager->includesVertical() || _terrainVerticalSettled;
  _movieManager->checkFrameReady(
    productsReady && horizontalWindsReady && verticalWindsReady &&
    _terrainHorizontalSettled && verticalTerrainReady);
}

//////////////////////////////////////////////////
// Set radar name in title bar

void GuiManager::setTitleBarStr(const string &titleStr)
{
  setWindowTitle(tr(titleStr.c_str()));
}

//////////////////////////////////////////////////
// selected field metadata

const string &GuiManager::getSelectedFieldLabel() const
{
  static const string empty;
  return _fieldNum >= 0 && _fieldNum < _gd.num_datafields &&
    _gd.mread[_fieldNum] ? _gd.mread[_fieldNum]->legend_name : empty;
}

const string &GuiManager::getSelectedFieldName() const
{
  static const string empty;
  return _fieldNum >= 0 && _fieldNum < _gd.num_datafields &&
    _gd.mread[_fieldNum] ? _gd.mread[_fieldNum]->field_label : empty;
}

const string &GuiManager::getSelectedFieldUnits() const
{
  static const string empty;
  return _fieldNum >= 0 && _fieldNum < _gd.num_datafields &&
    _gd.mread[_fieldNum] ? _gd.mread[_fieldNum]->field_units : empty;
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

  // Wait until the initial show/resize events have settled. Automatic output
  // captures the configured canvas geometry, so starting during the resize
  // debounce could associate a completed read with an obsolete transform.
  if (_params.create_images_automatically &&
      !_imageGenerationManager->isStarted() &&
      _timerEventCount > 10 && !_resizeTimer->isActive()) {
    _imageGenerationManager->start();
  }

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

  // U and V complete independently. Publish a new wind overlay only after
  // the manager has revalidated the pair for the current display request.
  if (_gd.wind_mgr && _gd.wind_mgr->poll()) {
    if (_horiz) {
      _horiz->rerenderWinds();
    }
    _checkMovieFrameReady();
    _checkAutomaticImageReady();
  }
  if (_gd.wind_mgr && _gd.wind_mgr->pollVertical()) {
    if (_vert) {
      _vert->rerenderWinds();
    }
    _checkMovieFrameReady();
    _checkAutomaticImageReady();
  }
  
}

///////////////////////////////////////////////
// override resize event
// we use a timer to debounce the resize event

void GuiManager::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  // Cached movie composites have the canvas dimensions and projection that
  // were active at build time. A resize necessarily returns to live mode so
  // the display can rebuild all layers for the new geometry.
  if (_timeControl && _timeControl->isMovieActive()) {
    _timeControl->stopMovie();
  }
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

  // Automatic image instances are display engines rather than interactive
  // viewers. Consume main-window shortcuts here because disabling the child
  // controls alone does not prevent field, time, or level keyboard actions.
  if (_params.create_images_automatically) {
    e->accept();
    return;
  }

  // get key pressed
  
  // Qt::KeyboardModifiers mods = e->modifiers();
  char keychar = e->text().toLatin1().data()[0];
  int key = e->key();

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Clicked char: " << keychar << ":" << (int) keychar << endl;
    cerr << "         key: " << hex << key << dec << endl;
  }

  // Do not let keyboard shortcuts alter the field or level halfway through a
  // build; every captured frame must use the same display configuration.
  if (isMovieBuilding() &&
      (keychar == '.' || key == Qt::Key_Up || key == Qt::Key_Down)) {
    showMovieInteractionBlocked();
    e->accept();
    return;
  }
  
  // for '.', swap with previous field
  
  if (keychar == '.') {
    if (_prevFieldNum >= 0) {
      _fieldTable->setCurrentCell(_prevFieldTableRow, _prevFieldTableCol);
      emit _fieldTable->cellClicked(_prevFieldTableRow, _prevFieldTableCol);
    }
    return;
  }
  
  // trap ESC - reset some items

  if (keychar == 27) {
    // zoom out
    _zoomOut();
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

    // Keep vlevel navigation and its boundary checks in the selector so the
    // behavior is identical whether the selector or main window has focus.
    _vlevelSelector->keyPressEvent(e);
    
  } else if (key == Qt::Key_Down) {

    _vlevelSelector->keyPressEvent(e);
    
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
  if (_gd.prod_mgr) {
    // SPDB workers publish data on the GUI thread. Regenerate only the
    // transparent product layer when a product snapshot is replaced.
    _gd.prod_mgr->setProductsChangedCallback([this]() {
      if (_horiz) {
        _horiz->rerenderProducts();
      }
      if (_timeControl) {
        _timeControl->refreshDataAvailability();
      }
      _checkMovieFrameReady();
      _checkAutomaticImageReady();
    });
    _gd.prod_mgr->setStatusCallback(
      [this](const string &message, bool isError) {
        // Successful requests use a brief progress message. Errors remain
        // visible longer while the previous valid product stays displayed.
        statusBar()->showMessage(QString::fromStdString(message),
                                 isError ? 15000 : 3000);
      });
  }

  connect(_horiz, &HorizView::verticalRouteCompleted,
          this, &GuiManager::_verticalRouteCompleted);
  
  // Create the VERT window

  _vertWindow = new VertManager(this);
  connect(_vertWindow, &VertManager::singleSegmentVsectionChanged,
          _horiz, &HorizView::setSingleSegmentVsection);
  
  // set pointer to the vertWidget
  
  _vert = _vertWindow->getView();
  // Each view owns the projection-specific sampling while GuiManager owns
  // the one shared dialog and its visibility policy.
  const auto displayClickReport = [this](const ClickReport &report) {
    updateClickReport(report);
    showClickDialog();
  };
  connect(_horiz, &HorizView::clickReportAvailable,
          this, displayClickReport);
  connect(_vert, &VertView::clickReportAvailable,
          this, displayClickReport);

  // Right-button gestures select vertical sections, so HorizView emits this
  // request only for the explicit Shift-right-click help gesture.
  connect(_horiz, &HorizView::customContextMenuRequested,
	  this, &GuiManager::_showInteractionHelp);

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
  _timeControl->refreshDataAvailability();
  
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

  QPoint pos(_params.horiz_window_x_pos, _params.horiz_window_y_pos);
  move(pos);
  show();

  // Resolve automatic placement only after the main window has its final
  // frame geometry. Each configured coordinate can override its fallback
  // independently.

  int vertX = _params.vsection_window_x_pos;
  int vertY = _params.vsection_window_y_pos;
  if (vertX == -1) {
    // Leave enough space for both window-manager borders to remain visible.
    constexpr int vertWindowGap = 5;
    vertX = frameGeometry().right() + 1 + vertWindowGap;
  }
  if (vertY == -1) {
    vertY = frameGeometry().top();
  }
  _vertWindow->move(vertX, vertY);

  // set up click report dialog

  _createClickReportDialog();

  // set up status bar

  statusBar()->setStyleSheet(
          "QStatusBar {"
          "  background-color: palette(alternate-base);"
          "  border-top: 2px solid palette(mid);"
          "  padding: 4px 6px;"
          "}"
                             );

  _configureAutomaticImageUi();
  
}

/////////////////////////////////////////////////////////////
// reserve an automatic-image instance for unattended rendering

void GuiManager::_configureAutomaticImageUi()
{
  if (!_params.create_images_automatically) {
    return;
  }

  // The generator still drives these objects programmatically. Disable their
  // user input at the widget and action boundary so an unattended run cannot
  // have its field, time, geometry, or overlays changed midway through a job.
  _fieldMenu->setDisabled(true);
  _timeControl->setDisabled(true);
  _vlevelSelector->setDisabled(true);
  _horiz->setDisabled(true);
  _vertWindow->setDisabled(true);

  // Image dimensions are part of the unattended product definition. Preserve
  // the configured startup geometry while still leaving window Close and the
  // File/Exit action available for operational shutdown.
  setFixedSize(size());
  _vertWindow->setFixedSize(_vertWindow->size());

  _showFieldMenuAct->setEnabled(false);
  _showTimeControlAct->setEnabled(false);
  _saveImageAct->setEnabled(false);
  _reloadAct->setEnabled(false);
  _clearAct->setEnabled(false);
  _zoomBackAct->setEnabled(false);
  _zoomOutAct->setEnabled(false);
  _mapsMenu->setEnabled(false);
  _productsMenu->setEnabled(false);
  _windsMenu->setEnabled(false);
  _zoomsMenu->setEnabled(false);
  _overlaysMenu->setEnabled(false);
  _showMenu->setEnabled(false);

  _clickReportDialog->hide();
  statusBar()->showMessage(
    tr("Automatic image generation: interactive controls are disabled"));
}

///////////////////////////////////////////////////////
// create the click report dialog

void GuiManager::_createClickReportDialog()
{
  _clickReportDialog = new QDialog(this);
  _clickReportDialog->setWindowTitle("Click report");
  _clickReportDialog->setMinimumSize(210, 280);

  _clickReportDialogLayout = new QGridLayout(_clickReportDialog);
  _clickReportDialogLayout->setVerticalSpacing(5);

  // Keep the metadata names at their natural width and give remaining space
  // to the values.  Allowing both columns to stretch creates a large empty gap
  // between them and makes the dialog appear wider than its content requires.
  _clickReportDialogLayout->setColumnStretch(0, 0);
  _clickReportDialogLayout->setColumnStretch(1, 1);

  int row = 0;
  QLabel *locationHeader = new QLabel("CLICK POINT DATA", _clickReportDialog);
  _clickReportDialogLayout->addWidget(locationHeader, row++, 0, 1, 2,
                                      Qt::AlignCenter);

  _clickDate = _addClickReportRow("Date", row++);
  _clickTime = _addClickReportRow("Time", row++);
  _clickLatitude = _addClickReportRow("Latitude", row++);
  _clickLongitude = _addClickReportRow("Longitude", row++);
  _clickHeight = _addClickReportRow("Height", row++, &_clickHeightName);
  _clickX = _addClickReportRow("X", row++, &_clickXName);
  _clickY = _addClickReportRow("Y", row++, &_clickYName);

  QLabel *fieldsHeader = new QLabel("FIELD VALUES", _clickReportDialog);
  _clickReportDialogLayout->addWidget(fieldsHeader, row++, 0, 1, 2,
                                      Qt::AlignCenter);

  // Field availability follows the user's display history, so use a table
  // that can be rebuilt for each click instead of fixed rows.
  _clickFieldsTable = new QTableWidget(_clickReportDialog);
  _clickFieldsTable->setColumnCount(3);
  _clickFieldsTable->setHorizontalHeaderLabels
    (QStringList() << "Name" << "Value" << "Units");
  _clickFieldsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  _clickFieldsTable->setSelectionMode(QAbstractItemView::NoSelection);
  _clickFieldsTable->setMinimumWidth(0);
  _clickFieldsTable->setSizeAdjustPolicy
    (QAbstractScrollArea::AdjustToContents);
  _clickFieldsTable->verticalHeader()->setVisible(false);
  _clickFieldsTable->horizontalHeader()->setSectionResizeMode
    (QHeaderView::ResizeToContents);
  _clickFieldsTable->horizontalHeader()->setStretchLastSection(false);
  _clickReportDialogLayout->addWidget(_clickFieldsTable, row, 0, 1, 2);

  // Closing the dialog directly must leave the checkable Show action in the
  // same state as the window it controls.
  connect(_clickReportDialog, &QDialog::finished, this, [this]() {
    _showClickAct->setChecked(false);
  });

  _clickReportDialog->hide();
  _clickReportDialog->resize(230, 340);
}

///////////////////////////////////////////////////////
// add one metadata row to the click report dialog

QLabel *GuiManager::_addClickReportRow(const string &name, int row,
                                       QLabel **nameLabelOut)
{
  QLabel *nameLabel = new QLabel(name.c_str(), _clickReportDialog);
  if (nameLabelOut != nullptr) {
    *nameLabelOut = nameLabel;
  }
  nameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _clickReportDialogLayout->addWidget(nameLabel, row, 0, Qt::AlignLeft);

  QLabel *valueLabel = new QLabel("----", _clickReportDialog);
  valueLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _clickReportDialogLayout->addWidget(valueLabel, row, 1, Qt::AlignLeft);
  return valueLabel;
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
  
  // show time control window
  _showTimeControlAct = new QAction(tr("Movie"), this);
  _showTimeControlAct->setStatusTip(tr("Show time control window"));
  connect(_showTimeControlAct, &QAction::triggered,
          this, &GuiManager::_showTimeControl);
  
  // reload data
  
  _reloadAct = new QAction(tr("Reload"), this);
  _reloadAct->setStatusTip(tr("Reload data"));
  connect(_reloadAct, &QAction::triggered,
          this, &GuiManager::_reloadData);

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

  // unzoom
  
  _zoomOutAct = new QAction(tr("Unzoom"), this);
  _zoomOutAct->setStatusTip(tr("Unzoom all the way"));
  _zoomOutAct->setEnabled(false);
  
  _zoomBackAct = new QAction(tr("Zoom back"), this);
  _zoomBackAct->setStatusTip(tr("Unzoom to previous view"));
  _zoomBackAct->setEnabled(false);

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
  
  // show values in data window

  _showValueAct = new QAction(tr("Show values"), this);
  _showValueAct->setStatusTip(tr("Show values in data window"));
  _showValueAct->setCheckable(true);
  _showValueAct->setChecked(false);
  connect(_showValueAct, &QAction::triggered, this, &GuiManager::_showValue);

  // show user click in dialog

  _showClickAct = new QAction(tr("Show click"), this);
  _showClickAct->setStatusTip(tr("Show click value dialog"));
  _showClickAct->setCheckable(true);
  _showClickAct->setChecked(false);
  connect(_showClickAct, &QAction::triggered, this, &GuiManager::showClickDialog);

  // show VERT window

  _showVertAct = new QAction(tr("Show VERT Window"), this);
  _showVertAct->setStatusTip(tr("Show the VERT Window"));
  _showVertAct->setCheckable(true);
  _showVertAct->setChecked(false);
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
  _saveImageAct->setStatusTip(
    tr("Save the horizontal and visible vertical views to the image output directory"));
  connect(_saveImageAct, &QAction::triggered,
          this, &GuiManager::_saveViewImages);

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
  menuBar()->addAction(_zoomOutAct);
  
  // time selector
  
  menuBar()->addAction(_showTimeControlAct);

  // add overlay menu
  
  _overlaysMenu = menuBar()->addMenu(tr("Overlays"));
  _populateOverlaysMenu();

  // add show menu
  
  _showMenu = menuBar()->addMenu(tr("Show"));
  _populateShowMenu();

  // misc actions
  
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
  mapDisplayChanged();
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> maps enabled? " << (_mapsEnabled?"Y":"N") << endl;
  }
}

/////////////////////////////////////////////////////////////
// rebuild cached map graphics without invalidating field data

void GuiManager::mapDisplayChanged()
{
  if (_horiz != nullptr) {
    _horiz->rerenderMaps();
  }
}

/////////////////////////////////////////////////////////////
// populate products menu

void GuiManager::_populateProductsMenu()
{

  // products enabled

  _productsEnabled = _params.symprods_enabled_at_startup;
  _gd.prod.products_on = _productsEnabled;
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
    connect(act, &QAction::toggled, this, [this](bool) {
      if (_horiz) {
        _horiz->rerenderProducts();
      }
      if (_timeControl) {
        _timeControl->refreshDataAvailability();
      }
    });
    
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
  _gd.prod.products_on = enable;
  if (_horiz) {
    _horiz->rerenderProducts();
  }
  if (enable && _gd.prod_mgr) {
    _gd.prod_mgr->refreshActiveProducts();
  }
  if (_timeControl) {
    _timeControl->refreshDataAvailability();
  }
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

    QAction *act = new QAction;
    act->setText(wparams.button_label);
    act->setStatusTip(tr("Turn wind layer on/off"));
    act->setCheckable(true);
    act->setChecked(wparams.on_at_startup);
    connect(act, &QAction::toggled, this,
            [this, iwind](bool checked) {
      _params._winds[iwind].on_at_startup =
        static_cast<tdrp_bool_t>(checked);
      if (_gd.wind_mgr) {
        _gd.wind_mgr->setLayerEnabled(iwind, checked);
        _gd.wind_mgr->requestData(_timeControl->getSelectedTime().utime(),
                                  _vlevelManager.getRequestedLevel(),
                                  _fieldNum);
        _requestVerticalWinds();
      }
      if (_horiz) {
        _horiz->rerenderWinds();
      }
      if (_vert) {
        _vert->rerenderWinds();
      }
      _checkAutomaticImageReady();
    });

    // add to winds menu

    _windsMenu->addAction(act);

  } // iwind
  
}

////////////////////////////////////////////////////////////
// enable winds

void GuiManager::_setWindsEnabled(bool enable)
{
  _windsEnabled = enable;
  _params.winds_on_at_startup = static_cast<tdrp_bool_t>(enable);
  if (_gd.wind_mgr) {
    _gd.wind_mgr->setEnabled(enable);
    _gd.wind_mgr->requestData(_timeControl->getSelectedTime().utime(),
                              _vlevelManager.getRequestedLevel(),
                              _fieldNum);
    _requestVerticalWinds();
  }
  if (_horiz) {
    _horiz->rerenderWinds();
  }
  if (_vert) {
    _vert->rerenderWinds();
  }
  _checkAutomaticImageReady();
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

  // unzoom all of the way
  
  connect(_zoomOutAct, &QAction::triggered, this, &GuiManager::_zoomOut);
  _zoomsMenu->addAction(_zoomOutAct);

  // unzoom by 1 level
  
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

}

/////////////////////////////////////////////////////////////
// populate show menu

void GuiManager::_populateShowMenu()
{

  _showMenu->addAction(_showValueAct);
  _showMenu->addAction(_showClickAct);
  _showMenu->addAction(_showVertAct);

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
        item->setFieldParams(&_params._fields[fieldNum]);
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
      } else {
        item->setFieldIndex(-1);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setBackground(Qt::lightGray);
        item->setForeground(Qt::gray);
      }
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
  _fieldTable->setContextMenuPolicy(Qt::CustomContextMenu);
  
  // initialize

  _fieldTable->setCurrentCell(0, 0);

  // set the size
  
  _fieldTable->setMaximumSize(_getTableWidgetSize(_fieldTable, false, false));
  _fieldTable->setMinimumSize(_fieldTable->maximumSize());

  connect(_fieldTable, SIGNAL(cellClicked(const int, const int)),
          this, SLOT(_fieldTableCellClicked(const int, const int)));
  connect(_fieldTable, &QWidget::customContextMenuRequested,
          this, &GuiManager::_showFieldContextMenu);
  
}

/////////////////////////////////////////////////////
// show the configuration controls for one field

void GuiManager::_showFieldContextMenu(const QPoint &pos)
{
  FieldTableItem *item =
    dynamic_cast<FieldTableItem *>(_fieldTable->itemAt(pos));
  if (item == nullptr || item->getFieldIndex() < 0 ||
      item->getFieldIndex() >= _params.fields_n ||
      !(item->flags() & Qt::ItemIsEnabled)) {
    return;
  }

  const int fieldNum = item->getFieldIndex();
  Params::field_t *field = item->getFieldParams();
  if (field == nullptr) {
    return;
  }

  QMenu menu(_fieldTable);
  menu.addSection(QString::fromUtf8(field->button_label));

  QMenu *renderMenu = menu.addMenu(tr("Render method"));
  QActionGroup *renderGroup = new QActionGroup(renderMenu);
  renderGroup->setExclusive(true);
  QAction *noneAction = renderMenu->addAction(tr("None"));
  QAction *polygonsAction = renderMenu->addAction(tr("Polygons"));
  QAction *filledContoursAction =
    renderMenu->addAction(tr("Filled contours"));
  for (QAction *action : {noneAction, polygonsAction,
                          filledContoursAction}) {
    action->setCheckable(true);
    renderGroup->addAction(action);
  }
  noneAction->setChecked(field->render_mode == Params::NONE);
  polygonsAction->setChecked(field->render_mode == Params::POLYGONS);
  filledContoursAction->setChecked(field->render_mode ==
                                   Params::FILLED_CONTOURS);

  connect(noneAction, &QAction::triggered, this,
          [this, field, fieldNum]() {
            field->render_mode = Params::NONE;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->render_method = NONE;
            }
            _applyFieldRenderingChange(fieldNum);
          });
  connect(polygonsAction, &QAction::triggered, this,
          [this, field, fieldNum]() {
            field->render_mode = Params::POLYGONS;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->render_method = POLYGONS;
            }
            _applyFieldRenderingChange(fieldNum);
          });
  connect(filledContoursAction, &QAction::triggered, this,
          [this, field, fieldNum]() {
            field->render_mode = Params::FILLED_CONTOURS;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->render_method = FILLED_CONTOURS;
            }
            _applyFieldRenderingChange(fieldNum);
          });

  QAction *lineContoursAction =
    menu.addAction(tr("Overlay line contours"));
  lineContoursAction->setCheckable(true);
  lineContoursAction->setChecked(field->overlay_line_contours);
  connect(lineContoursAction, &QAction::toggled, this,
          [this, field, fieldNum](bool enabled) {
            field->overlay_line_contours = enabled ? pTRUE : pFALSE;
            if (_gd.mread[fieldNum] != nullptr) {
              _gd.mread[fieldNum]->overlay_line_contours = enabled;
            }
            _applyFieldRenderingChange(fieldNum);
          });

  QMenu *contourMenu = menu.addMenu(tr("Contour details"));
  QDoubleSpinBox *contourLow =
    addFieldNumberEditor(contourMenu, tr("Low"), field->contour_low, 3);
  QDoubleSpinBox *contourHigh =
    addFieldNumberEditor(contourMenu, tr("High"), field->contour_high, 3);
  QDoubleSpinBox *contourInterval =
    addFieldNumberEditor(contourMenu, tr("Interval"),
                         field->contour_interval, 3, 0.0);
  contourMenu->addSeparator();
  QAction *applyContoursAction = contourMenu->addAction(tr("Apply"));
  connect(applyContoursAction, &QAction::triggered, this,
          [this, field, fieldNum, contourLow, contourHigh,
           contourInterval]() {
            const double low = contourLow->value();
            const double high = contourHigh->value();
            const double interval = contourInterval->value();
            if (high < low || interval <= 0.0) {
              QMessageBox::warning(_fieldMenu, tr("Invalid contours"),
                                   tr("High must not be below low, and interval must be positive."));
              return;
            }

            // Params retain the interactive choices for subsequent menus;
            // MdvReader is the rendering-time copy used by both views.
            field->contour_low = low;
            field->contour_high = high;
            field->contour_interval = interval;
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader != nullptr) {
              reader->cont_low = low;
              reader->cont_high = high;
              reader->cont_interv = interval;
            }
            _applyFieldRenderingChange(fieldNum);
          });

  QAction *columnMaxAction = menu.addAction(tr("Column max mode"));
  columnMaxAction->setCheckable(true);
  columnMaxAction->setChecked(field->composite_mode);
  connect(columnMaxAction, &QAction::toggled, this,
          [this, field, fieldNum](bool enabled) {
            field->composite_mode = enabled ? pTRUE : pFALSE;
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader != nullptr) {
              reader->composite_mode = enabled ? TRUE : FALSE;
            }

            // Column-max is an MDV request option rather than a rendering
            // choice.  Queue a new horizontal read for the active field; the
            // normal timer path waits for an existing read and deliberately
            // leaves the vertical-section request and display unchanged.
            if (fieldNum == _fieldNum) {
              _horizontalRequestHasChanged = true;
            }
          });

  double colorScaleMin = field->contour_low;
  double colorScaleMax = field->contour_high;
  if (_gd.mread[fieldNum] != nullptr) {
    const vector<ColorMap::CmapEntry> &entries =
      _gd.mread[fieldNum]->colorMap.getEntries();
    if (!entries.empty()) {
      colorScaleMin = entries.front().minVal;
      colorScaleMax = entries.back().maxVal;
    }
  }
  QMenu *colorScaleMenu = menu.addMenu(tr("Color scale"));
  QDoubleSpinBox *colorScaleMinEditor =
    addFieldNumberEditor(colorScaleMenu, tr("Minimum"), colorScaleMin, 3);
  QDoubleSpinBox *colorScaleMaxEditor =
    addFieldNumberEditor(colorScaleMenu, tr("Maximum"), colorScaleMax, 3);
  colorScaleMenu->addSeparator();
  QAction *applyColorScaleAction = colorScaleMenu->addAction(tr("Apply"));
  connect(applyColorScaleAction, &QAction::triggered, this,
          [this, fieldNum, colorScaleMinEditor, colorScaleMaxEditor]() {
            const double rangeMin = colorScaleMinEditor->value();
            const double rangeMax = colorScaleMaxEditor->value();
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader == nullptr || rangeMax <= rangeMin ||
                !reader->colorMap.rescaleRange(rangeMin, rangeMax)) {
              QMessageBox::warning(_fieldMenu, tr("Invalid color scale"),
                                   tr("Maximum must be greater than minimum, "
                                      "and the existing color scale must have "
                                      "a valid range."));
              return;
            }

            _applyFieldRenderingChange(fieldNum);
          });
  QAction *resetColorScaleAction = colorScaleMenu->addAction(tr("Reset"));
  MdvReader *colorScaleReader = _gd.mread[fieldNum];
  const bool canResetColorScale =
    colorScaleReader != nullptr &&
    colorScaleReader->colorMap.hasOriginalRange();
  resetColorScaleAction->setEnabled(canResetColorScale);
  connect(resetColorScaleAction, &QAction::triggered, this,
          [this, fieldNum]() {
            MdvReader *reader = _gd.mread[fieldNum];
            if (reader == nullptr || !reader->colorMap.resetRange()) {
              QMessageBox::warning(_fieldMenu, tr("Cannot reset color scale"),
                                   tr("The original color-scale range is not "
                                      "available."));
              return;
            }

            _applyFieldRenderingChange(fieldNum);
          });

  menu.exec(_fieldTable->viewport()->mapToGlobal(pos));
}

/////////////////////////////////////////////////////////////
// redraw retained data after an interactive rendering edit

void GuiManager::_applyFieldRenderingChange(int fieldNum)
{
  if (fieldNum < 0 || fieldNum >= _gd.num_datafields ||
      _gd.mread[fieldNum] == nullptr) {
    return;
  }

  // These settings change only presentation. Reuse retained horizontal and
  // vertical data so menu edits do not initiate MDV requests or disturb the
  // current movie time and selected level.
  if (_horiz != nullptr) {
    _horiz->rerenderGrid();
  }
  if (_vert != nullptr) {
    _vert->displayImage(fieldNum);
  }
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
// get (x, y) of top right corner
// for placing child windows

QPoint GuiManager::getTopRightPos()
{
  
  int margin = (frameGeometry().width() - geometry().width()) / 2;
  int titleBarHt = frameGeometry().height() - geometry().height() - margin;
  QPoint pos(x() + (frameGeometry().width()), y() + titleBarHt);

  return pos;

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
  
  _fieldMenu->move(getTopRightPos());

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
  if (_fieldNum < 0 || _fieldNum >= _params.fields_n) {
    _fieldHasChanged = false;
    return;
  }
  
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

  _fieldHasChanged = true;

  // A completed route describes display geometry rather than a particular
  // field.  Reuse it so the vertical view follows the horizontal field
  // selection without requiring the user to draw the section again.

  if (_vsectionRoute.size() >= 2) {
    _requestVerticalSection();
  }
  
}

//////////////////////////////////////////////////////////////
// check if we need new H data, and read accordingly
// then trigger rendering if appropriate
  
void GuiManager::_checkAndReadH(MdvReader *mr)
{

  // check for state change, except field
  
  bool stateHasChanged = _checkForStateChange();

  // clear field data if non-field state has changed
  
  if (stateHasChanged) {
    _horiz->clearFieldDataQueue();
  }
  
  // check for field change
  
  if (_fieldHasChanged) {
    stateHasChanged = true;
    _fieldHasChanged = false;
    _timeControl->refreshDataAvailability();
  }

  // Request options such as column-max require new horizontal data even
  // though the selected field, level, time, and display geometry are stable.
  if (_horizontalRequestHasChanged) {
    stateHasChanged = true;
    _horizontalRequestHasChanged = false;
  }
  
  // get new data if needed - data is retrieved in a thread
  
  const int frameIndex = _timeControl->getFrameIndex();
  const TimelineFrame *frame = _timeControl->getFrame(frameIndex);

  if (stateHasChanged) {
    const int generation = mr->requestHorizPlane(
      _timeControl->getSelectedTime().utime(),
      _vlevelManager.getRequestedLevel(),
      _fieldNum,
      true);
    if (_gd.wind_mgr) {
      _gd.wind_mgr->requestData(_timeControl->getSelectedTime().utime(),
                                _vlevelManager.getRequestedLevel(),
                                _fieldNum);
    }
    if (_imageGenerationManager->isActive() &&
        _imageGenerationManager->currentJobIsHorizontal() &&
        _imageGenerationManager->hasCurrentJob() &&
        _fieldNum == _imageGenerationManager->currentJob().fieldIndex &&
        mr == _gd.mread[_fieldNum]) {
      _imageGenerationManager->setExpectedGeneration(generation);
    }
    if (_gd.prod_mgr && frame) {
      _gd.prod_mgr->requestData(frame->start, frame->end,
                                _timeControl->getSelectedTime().utime(),
                                _vlevelManager.getRequestedLevel(),
                                *_horiz->getZoomWorld());
    }
  } else if (_gd.prod_mgr &&
             _gd.prod_mgr->num_products_invalid() > 0) {
    _gd.prod_mgr->refreshActiveProducts();
  }
  
  // check for new data
  
  if (mr->isNewH()) {
    if (!mr->isValidH()) {
      cerr << "ERROR - GuiManager::timerEvent" << endl;
      cerr << "  mr->requestHorizPlane" << endl;
      if (frame) {
        cerr << "  time_start: " << DateTime::strm(frame->start) << endl;
        cerr << "  time_end: " << DateTime::strm(frame->end) << endl;
      }
      cerr << "  page: " << _fieldNum << endl;
    }
    // The main header determines whether its vertical coordinate can be
    // compared meaningfully with MSL terrain. Start that optional request
    // before rendering so a retained terrain plane can be used immediately.
    _requestHorizontalTerrain();
    _horiz->triggerGridRendering(_fieldNum, frameIndex);
    if (_imageGenerationManager->isActive() &&
        _imageGenerationManager->currentJobIsHorizontal() &&
        _imageGenerationManager->hasCurrentJob() &&
        _fieldNum == _imageGenerationManager->currentJob().fieldIndex &&
        mr == _gd.mread[_fieldNum] && mr->isValidH() &&
        _imageGenerationManager->acceptDataCompletion(
          mr->getHorizCompletedGeneration())) {
      _checkAutomaticImageReady();
    }
    if (_movieManager->buildFrame() == frameIndex) {
      _movieManager->markGridReady(frameIndex);
      _checkMovieFrameReady();
    }
    if (_fieldNum < _gd.num_datafields) {
      _vlevelManager.set(*_gd.mread[_fieldNum]);
    }
    _vert->setSelectedHorizHeight(_vlevelManager.getSelectedLevel());
    _vlevelSelector->update();
    _timeControl->refreshDataAvailability();
  }

}

///////////////////////////////////////////
// check for state changes

bool GuiManager::_checkForStateChange()

{

  bool stateHasChanged = false;

  // zoom change?
  
  if (_horiz->checkForZoomChange()) {
    stateHasChanged = true;
  }

  // vlevel change?
  
  if (_vlevelHasChanged) {
    _vlevelHasChanged = false;
    _vert->setSelectedHorizHeight(_vlevelManager.getSelectedLevel());
    stateHasChanged = true;
  }

  // time change

  if (_timeControl->timeHasChanged()) {
    stateHasChanged = true;
  }

  // resize?

  if (_resized) {
    _resized = false;
    stateHasChanged = true;
  }

  // overlays?

  if (_overlaysHaveChanged) {
    _overlaysHaveChanged = false;
    stateHasChanged = true;
  }

  return stateHasChanged;

}

////////////////////////////////////////////
// unzoom to previous zoom

void GuiManager::_zoomBack()
{
  if (isMovieActive()) {
    showMovieInteractionBlocked();
    return;
  }
  _horiz->zoomBackView();
  if (_horiz->getCustomZooms().size() == 0) {
    enableUnzoomButtons(false);
  }
}

////////////////////////////////////////////
// unzoom all the way out

void GuiManager::_zoomOut()
{
  if (isMovieActive()) {
    showMovieInteractionBlocked();
    return;
  }
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
// reload the data currently represented by the display

void GuiManager::_reloadData()
{
  // Use the normal request path so a reload waits for any active horizontal
  // read and preserves the displayed image until replacement data arrive.
  _horizontalRequestHasChanged = true;
  if (_fieldNum >= 0 && _fieldNum < _gd.num_datafields &&
      _gd.mread[_fieldNum]) {
    _gd.mread[_fieldNum]->invalidateTimeList();
  }
  if (_gd.prod_mgr) {
    _gd.prod_mgr->reset_product_valid_flags();
    _gd.prod_mgr->refreshActiveProducts();
  }
  if (_gd.wind_mgr) {
    _gd.wind_mgr->invalidate();
  }

  // A visible vertical section is part of the current display state.  Reuse
  // its retained route; request generations prevent an older in-flight result
  // from replacing this explicit reload when it eventually completes.
  if (_vertWindow != nullptr && _vertWindow->isVisible() &&
      _vsectionRoute.size() >= 2) {
    _requestVerticalSection();
  }
}

////////////////////////////////////////////////////////
// handle params file

void GuiManager::_openFile() {
}

void GuiManager::_saveFile() {
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

/////////////////////////////////////////////////////////////
// preserve the live display while a realtime catalog batch takes control

void GuiManager::_saveAutomaticDisplayState()
{
  if (_automaticDisplayStateSaved) {
    return;
  }
  _automaticSavedField = _fieldNum;
  _automaticSavedZoom = _horiz ? _horiz->getZoomIndex() : 0;
  _automaticSavedHeight = _vlevelManager.getRequestedLevel();
  _automaticSavedStartTime = _timeControl->getStartTime();
  _automaticSavedFrameCount = _timeControl->getNFramesMovie();
  _automaticSavedFrameIntervalSecs = _timeControl->getFrameIntervalSecs();
  _automaticSavedFrameIndex = _timeControl->getFrameIndex();
  _automaticSavedRealtime = _timeControl->isRealtime();
  _automaticSavedVertWindowVisible =
    _vertWindow != nullptr && _vertWindow->isVisible();
  _automaticSavedVsectionRoute = _vsectionRoute;
  _automaticDisplayStateSaved = true;
}

/////////////////////////////////////////////////////////////
// return control to the accepted display between realtime batches

void GuiManager::_restoreAutomaticDisplayState()
{
  if (!_automaticDisplayStateSaved) {
    return;
  }
  _timeControl->restoreAfterAutomaticImages(
    _automaticSavedStartTime, _automaticSavedFrameCount,
    _automaticSavedFrameIntervalSecs, _automaticSavedFrameIndex,
    _automaticSavedRealtime);
  _vlevelManager.requestLevel(_automaticSavedHeight);
  _vlevelHasChanged = true;
  setZoomIndex(_automaticSavedZoom);
  if (_automaticSavedField >= 0 &&
      _automaticSavedField < _gd.num_datafields) {
    _selectAutomaticImageField(_automaticSavedField);
  }
  _vsectionRoute = _automaticSavedVsectionRoute;
  if (_vsectionRoute.size() >= 2 && _automaticSavedVertWindowVisible) {
    _requestVerticalSection();
  }
  _automaticDisplayStateSaved = false;
}

/////////////////////////////////////////////////////////////
// apply the current generator job to the live display

void GuiManager::_startAutomaticImageJob()
{
  if (!_imageGenerationManager->isActive() ||
      !_imageGenerationManager->hasCurrentJob()) {
    return;
  }
  const AutomaticImageGenerator::Job &job =
    _imageGenerationManager->currentJob();
  const int fieldIndex = job.fieldIndex;
  if (fieldIndex < 0 || _gd.mread[fieldIndex] == nullptr) {
    _imageGenerationManager->failCurrentJob(
      tr("Field %1 has no MDV reader").arg(fieldIndex));
    return;
  }

  if (_imageGenerationManager->currentJobIsVertical()) {
    _startAutomaticVerticalJob();
  } else {
    _startAutomaticHorizontalJob();
  }
}

/////////////////////////////////////////////////////////////
// apply one horizontal height/zoom/field job

void GuiManager::_startAutomaticHorizontalJob()
{
  const AutomaticImageGenerator::Job &job =
    _imageGenerationManager->currentJob();
  const int fieldIndex = job.fieldIndex;

  // Apply both outer dimensions before selecting the field. Their normal
  // invalidation paths then let the timer issue one request containing the
  // complete height, geometry, and field state for this job.
  _vlevelManager.requestLevel(job.heightKm);
  _vlevelHasChanged = true;
  setZoomIndex(job.zoomIndex);
  _selectAutomaticImageField(fieldIndex);

  const QString fieldName = QString::fromUtf8(
    _params._fields[fieldIndex].field_name);
  statusBar()->showMessage(
    tr("Creating automatic image %1 of %2: %3 at %4 km, zoom %5")
      .arg(_imageGenerationManager->currentJobNumber())
      .arg(_imageGenerationManager->jobCount())
      .arg(fieldName).arg(job.heightKm).arg(job.zoomName));
  if (_params.images_debug != Params::IMAGE_DEBUG_OFF) {
    cerr << "Automatic image: requesting field " << fieldName.toStdString()
         << " at " << job.heightKm << " km, zoom "
         << job.zoomName.toStdString() << " ("
         << _imageGenerationManager->currentJobNumber() << " of "
         << _imageGenerationManager->jobCount() << ")" << endl;
  }
}

/////////////////////////////////////////////////////////////
// request one configured route for the selected field

void GuiManager::_startAutomaticVerticalJob()
{
  const AutomaticImageGenerator::Job &job =
    _imageGenerationManager->currentJob();
  VsectionRoute route;
  QString error;
  if (!_buildAutomaticVsectionRoute(route, error)) {
    _imageGenerationManager->failCurrentJob(error);
    return;
  }

  _selectAutomaticImageField(job.fieldIndex, false);
  _vsectionRoute = route;
  const int generation = _requestVerticalSection();
  if (generation < 0) {
    _imageGenerationManager->failCurrentJob(
      tr("Could not start vertical-section request"));
    return;
  }
  _imageGenerationManager->setExpectedGeneration(generation);

  const QString fieldName = QString::fromUtf8(
    _params._fields[job.fieldIndex].field_name);
  statusBar()->showMessage(
    tr("Creating automatic image %1 of %2: %3, vertical section %4")
      .arg(_imageGenerationManager->currentJobNumber())
      .arg(_imageGenerationManager->jobCount())
      .arg(fieldName)
      .arg(_imageGenerationManager->currentRoute().name));
  if (_params.images_debug != Params::IMAGE_DEBUG_OFF) {
    cerr << "Automatic image: requesting vertical field "
         << fieldName.toStdString() << ", route "
         << _imageGenerationManager->currentRoute().name.toStdString()
         << " (" << _imageGenerationManager->currentJobNumber() << " of "
         << _imageGenerationManager->jobCount() << ")" << endl;
  }
}

/////////////////////////////////////////////////////////////
// update field state without synthesizing a table interaction

void GuiManager::_selectAutomaticImageField(int fieldIndex,
                                            bool requestHorizontal)
{
  _prevFieldNum = _fieldNum;
  _fieldNum = fieldIndex;
  _fieldHasChanged = requestHorizontal;

  // Keep the visible selector consistent for diagnostics without routing the
  // programmatic change back through its user-click handler.
  const int ncols = std::max(1, _params.num_field_menu_cols);
  _fieldTableRow = fieldIndex / ncols;
  _fieldTableCol = fieldIndex % ncols;
  if (_fieldTable) {
    _fieldTable->setCurrentCell(_fieldTableRow, _fieldTableCol);
  }
}

/////////////////////////////////////////////////////////////
// save only after the grid and the configured Product policy are ready

void GuiManager::_checkAutomaticImageReady()
{
  if (!_imageGenerationManager->isActive() ||
      !_imageGenerationManager->hasCurrentJob()) {
    return;
  }

  const bool productsRelevant =
    !_imageGenerationManager->currentJobIsVertical() &&
    _gd.prod.products_on && _gd.prod_mgr;
  const bool productsReady = !productsRelevant ||
    (_params.images_require_all_symprods ?
       _gd.prod_mgr->activeProductsReady() :
       _gd.prod_mgr->activeProductRequestsSettled());
  const bool windsRelevant = _gd.wind_mgr && _gd.wind_mgr->isEnabled();
  const bool windsReady = !windsRelevant ||
    (_imageGenerationManager->currentJobIsVertical() ?
       _gd.wind_mgr->activeVerticalWindsSettled() :
       _gd.wind_mgr->activeWindsReady());
  const bool terrainReady =
    _imageGenerationManager->currentJobIsVertical() ?
      _terrainVerticalSettled : _terrainHorizontalSettled;
  const int unavailableProducts =
    productsRelevant && !_params.images_require_all_symprods ?
      _gd.prod_mgr->numActiveProductsUnavailable() : 0;
  _imageGenerationManager->checkReady(
    productsReady && windsReady && terrainReady, unavailableProducts);
}

/////////////////////////////////////////////////////////////
// cancel the exact request generation selected by a timed-out job

void GuiManager::_cancelTimedOutImageRequest(int fieldIndex,
                                             bool vertical,
                                             int expectedGeneration)
{
  if (fieldIndex < 0 || fieldIndex >= _gd.num_datafields ||
      !_gd.mread[fieldIndex]) {
    return;
  }
  if (vertical) {
    const int generation = expectedGeneration >= 0 ? expectedGeneration :
      _gd.mread[fieldIndex]->getVertGeneration();
    _gd.mread[fieldIndex]->cancelVertRequest(generation);
  } else {
    const int generation = expectedGeneration >= 0 ? expectedGeneration :
      _gd.mread[fieldIndex]->getHorizGeneration();
    _gd.mread[fieldIndex]->cancelHorizRequest(generation);
  }
}

/////////////////////////////////////////////////////////////
// capture the current job after the manager declares its inputs ready

bool GuiManager::_captureAutomaticImage(QImage &image,
                                        DateTime &dataTime,
                                        const WorldPlot *&world,
                                        bool &writeGeoXml,
                                        QString &error)
{
  image = QImage();
  world = nullptr;
  writeGeoXml = false;
  error.clear();
  if (!_imageGenerationManager->hasCurrentJob()) {
    error = tr("No automatic image job is active");
    return false;
  }

  const int fieldIndex =
    _imageGenerationManager->currentJob().fieldIndex;
  MdvReader *reader = _gd.mread[fieldIndex];
  if (_imageGenerationManager->currentJobIsVertical()) {
    if (!_vert || !_vertWindow || !reader || !reader->isValidV() ||
        reader->getVertGeneration() !=
          _imageGenerationManager->expectedGeneration()) {
      error = tr("The completed vertical data do not match the requested field");
      return false;
    }

    // A hidden automatic window still needs finalized layouts before QWidget
    // can synchronously render the configured section at its intended size.
    _vertWindow->ensurePolished();
    if (_vertWindow->layout()) {
      _vertWindow->layout()->activate();
    }
    if (_vertWindow->centralWidget() &&
        _vertWindow->centralWidget()->layout()) {
      _vertWindow->centralWidget()->layout()->activate();
    }
    if (_vert->width() < 1 || _vert->height() < 1) {
      error = tr("The vertical view has no drawable area");
      return false;
    }
    image = _vert->captureImageWithoutInteractions();
    if (image.isNull()) {
      error = tr("Could not capture the vertical-section view");
      return false;
    }
    dataTime = DateTime(reader->v_date);
    return true;
  }

  if (!_horiz || !reader || !reader->isValidH() ||
      reader->getHorizCompletedGeneration() !=
        _imageGenerationManager->expectedGeneration()) {
    error = tr("The completed horizontal data do not match the requested field");
    return false;
  }
  image = _horiz->captureImageWithoutInteractions();
  if (image.isNull()) {
    error = tr("Could not capture the horizontal view");
    return false;
  }
  dataTime = DateTime(reader->h_date);
  world = _horiz->getZoomWorld();
  writeGeoXml = true;
  return true;
}

/////////////////////////////////////////////////////////////
// convert one configured route into the display and MDV coordinate forms

bool GuiManager::_buildAutomaticVsectionRoute(VsectionRoute &route,
                                               QString &error) const
{
  route.clear();
  error.clear();
  if (!_imageGenerationManager->currentJobIsVertical()) {
    error = tr("No automatic vertical-section job is active");
    return false;
  }

  for (const QPointF &point :
       _imageGenerationManager->currentRoute().points) {
    double x = point.x();
    double y = point.y();
    double lat = 0.0;
    double lon = 0.0;
    if (_params.images_vsection_waypts_in_latlon) {
      lat = point.x();
      lon = point.y();
      _gd.proj.latlon2xy(lat, lon, x, y);
    } else {
      _gd.proj.xy2latlon(x, y, lat, lon);
    }
    route.append(x, y, lat, lon);
  }
  if (route.size() < 2 || route.totalLengthKm() <= 0.0) {
    error = tr("Vertical section %1 has zero usable length")
      .arg(_imageGenerationManager->currentRoute().name);
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////
// save current view snapshots to the configured image directory

void GuiManager::_saveViewImages()
{
  if (!_horiz) {
    QMessageBox::critical(this, tr("Cannot save images"),
                          tr("The horizontal view is not available."));
    return;
  }

  QString outputDirPath = QString::fromUtf8(_params.images_output_dir);
  outputDirPath.replace("$(HOME)", QDir::homePath());
  if (outputDirPath.startsWith("$HOME")) {
    outputDirPath.replace(0, 5, QDir::homePath());
  }
  if (outputDirPath.trimmed().isEmpty()) {
    QMessageBox::critical(this, tr("Cannot save images"),
                          tr("images_output_dir is empty."));
    return;
  }
  outputDirPath = QDir::cleanPath(outputDirPath);
  QDir outputDir;
  if (!outputDir.mkpath(outputDirPath)) {
    QMessageBox::critical(
      this, tr("Cannot save images"),
      tr("Cannot create the output directory:\n%1").arg(outputDirPath));
    return;
  }
  outputDir.setPath(outputDirPath);

  const time_t selectedTime = _timeControl ?
    _timeControl->getSelectedTime().utime() : time(nullptr);
  const QString timePart = QDateTime::fromSecsSinceEpoch(
    selectedTime, QTimeZone(QTimeZone::UTC))
    .toString("yyyyMMdd_HHmmss'_UTC'");
  const QString fieldPart = safeFileNameComponent(
    QString::fromStdString(getSelectedFieldLabel()));
  const QString commonPart = fieldPart + "_" + timePart;
  const bool saveVertical = _vertWindow && _vert &&
                            _vertWindow->isVisible();

  // Choose one suffix for the complete capture so horizontal and vertical
  // files remain an obvious pair and an earlier snapshot is never replaced.
  QString suffix;
  QString horizPath;
  QString vertPath;
  for (int sequence = 1; ; ++sequence) {
    suffix = sequence == 1 ? QString() :
      QString("_%1").arg(sequence, 2, 10, QChar('0'));
    horizPath = outputDir.filePath(
      "lucid_horiz_" + commonPart + suffix + ".png");
    vertPath = outputDir.filePath(
      "lucid_vert_" + commonPart + suffix + ".png");
    if (!QFileInfo::exists(horizPath) &&
        (!saveVertical || !QFileInfo::exists(vertPath))) {
      break;
    }
  }

  QStringList savedPaths;
  QStringList failedPaths;
  if (_horiz->captureImageWithoutInteractions().save(horizPath, "PNG")) {
    savedPaths.push_back(horizPath);
  } else {
    failedPaths.push_back(horizPath);
  }
  if (saveVertical) {
    if (_vert->captureImageWithoutInteractions().save(vertPath, "PNG")) {
      savedPaths.push_back(vertPath);
    } else {
      failedPaths.push_back(vertPath);
    }
  }

  if (!failedPaths.isEmpty()) {
    QMessageBox::critical(
      this, tr("Image save failed"),
      tr("Could not save:\n%1").arg(failedPaths.join("\n")));
  } else {
    QMessageBox::information(
      this, tr("Images saved"),
      tr("Saved:\n%1").arg(savedPaths.join("\n")));
  }
}

void GuiManager::_showInteractionHelp(const QPoint &pos)
{
  QMenu menu(_horiz);
  QWidgetAction *helpAction = new QWidgetAction(&menu);
  QLabel *helpText = new QLabel(&menu);
  helpText->setTextFormat(Qt::RichText);
  helpText->setTextInteractionFlags(Qt::TextSelectableByMouse);
  helpText->setWordWrap(true);
  helpText->setMinimumWidth(390);
  helpText->setContentsMargins(12, 10, 12, 10);

  QString vsectionHelp;
  if (_params.single_segment_vsection) {
    vsectionHelp = tr(
      "<b>Vertical section</b><br>"
      "Right-drag and release to select one segment. "
      "On macOS, Control-drag is the right-button alternative.");
  } else {
    vsectionHelp = tr(
      "<b>Vertical section</b><br>"
      "Right-drag to start, then right-click to add segments. "
      "Double-click to finish; Backspace removes the last point and Escape cancels. "
      "On macOS, Control-click is the right-button alternative.");
  }

  // Keep the popup as one passive item: it documents canvas and selector
  // gestures without suggesting that the inherited HawkEye actions still
  // perform operations in Lucid.
  helpText->setText(tr(
    "<b>Plan-view controls</b><br><br>"
    "<b>Zoom</b><br>Left-drag a rectangle over the plot.<br><br>"
    "<b>Pan</b><br>Middle-button drag, or Option-left-drag.<br><br>"
    "%1<br><br>"
    "<b>Height selector</b><br>"
    "Click or drag vertically in the selector to the right of the plot to choose a level."
  ).arg(vsectionHelp));

  helpAction->setDefaultWidget(helpText);
  menu.addAction(helpAction);
  menu.exec(_horiz->mapToGlobal(pos));
}


/**********************************************************************
 * HANDLE_CLIENT_EVENT: 
 */

void GuiManager::_handleClientEvent()
{
  // Remote CIDD client commands are not implemented by the Qt application.
}

/////////////////////////////
// update data at the latest click point

void GuiManager::updateClickReport(const ClickReport &report)
{
  if (!_clickReportDialog) {
    return;
  }

  DateTime dataTime(report.dataTime);
  char text[256];
  snprintf(text, sizeof(text), "%.4d/%.2d/%.2d",
           dataTime.getYear(), dataTime.getMonth(), dataTime.getDay());
  _clickDate->setText(text);
  snprintf(text, sizeof(text), "%.2d:%.2d:%.2d UTC",
           dataTime.getHour(), dataTime.getMin(), dataTime.getSec());
  _clickTime->setText(text);

  snprintf(text, sizeof(text), "%.5f deg", report.latitude);
  _clickLatitude->setText(text);
  snprintf(text, sizeof(text), "%.5f deg", report.longitude);
  _clickLongitude->setText(text);

  _clickHeightName->setText(report.heightLabel.c_str());
  _clickHeightName->setVisible(report.showHeight);
  _clickHeight->setVisible(report.showHeight);
  snprintf(text, sizeof(text), "%g %s", report.height,
           report.heightUnits.c_str());
  _clickHeight->setText(text);

  _clickXName->setText(report.xLabel.c_str());
  _clickXName->setVisible(report.showX);
  _clickX->setVisible(report.showX);
  snprintf(text, sizeof(text), "%g %s", report.x, report.xUnits.c_str());
  _clickX->setText(text);

  _clickYName->setText(report.yLabel.c_str());
  _clickYName->setVisible(report.showY);
  _clickY->setVisible(report.showY);
  snprintf(text, sizeof(text), "%g %s", report.y, report.yUnits.c_str());
  _clickY->setText(text);

  _clickFieldsTable->setRowCount(report.fields.size());
  for (size_t ii = 0; ii < report.fields.size(); ii++) {
    const ClickReportField &field = report.fields[ii];
    QTableWidgetItem *nameItem =
      new QTableWidgetItem(field.name.c_str());
    snprintf(text, sizeof(text), "%g", field.value);
    QTableWidgetItem *valueItem = new QTableWidgetItem(text);
    valueItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QTableWidgetItem *unitsItem =
      new QTableWidgetItem(field.units.c_str());
    _clickFieldsTable->setItem(ii, 0, nameItem);
    _clickFieldsTable->setItem(ii, 1, valueItem);
    _clickFieldsTable->setItem(ii, 2, unitsItem);
  }
  _clickFieldsTable->resizeColumnsToContents();
  _clickFieldsTable->resizeRowsToContents();
}

/////////////////////////////
// show data at click point

void GuiManager::showClickDialog()
{

  if (!_clickReportDialog) {
    return;
  }

  // The checkable action is the source of truth.  Click updates may call this
  // repeatedly, so visibility must not be toggled on each new report.
  if (_showClickAct->isChecked()) {
    if (!_clickReportPlaced) {
      QPoint pos;
      pos.setX(x() + width() + 5);
      pos.setY(y());
      _clickReportDialog->move(pos);
      _clickReportPlaced = true;
    }
    _clickReportDialog->show();
    _clickReportDialog->raise();
  } else {
    _clickReportDialog->hide();
  }
}

/////////////////////////////
// show value in data plot

void GuiManager::_showValue()

{
  const bool enabled = _showValueAct->isChecked();
  _horiz->setShowValue(enabled);
  _vert->setShowValue(enabled);
}

/////////////////////////////////////////////////////////////
// retain a completed vertical-section route

void GuiManager::_verticalRouteCompleted(const VsectionRoute &route)
{
  _vsectionRoute = route;

  char message[256];
  snprintf(message, sizeof(message),
           "Vertical section route: %zu points, %.1f km",
           route.size(), route.totalLengthKm());
  // statusBar()->showMessage(message, 5000);

  if (_params.debug) {
    cerr << "Vertical section route completed" << endl;
    cerr << "  points: " << route.size() << endl;
    cerr << "  total length km: " << route.totalLengthKm() << endl;
    const vector<VsectionRoute::Point> &points = route.points();
    for (size_t ii = 0; ii < points.size(); ++ii) {
      cerr << "  waypoint " << ii
           << ": lat, lon = " << points[ii].lat << ", " << points[ii].lon
           << "; x, y = " << points[ii].x << ", " << points[ii].y
           << "; distance km = " << points[ii].cumulativeDistanceKm
           << endl;
    }
  }

  _requestVerticalSection();
}

/////////////////////////////////////////////////////////////
// request the retained route for the currently selected field

int GuiManager::_requestVerticalSection()
{
  if (_vsectionRoute.size() < 2 ||
      _fieldNum < 0 || _fieldNum >= _gd.num_datafields) {
    return -1;
  }

  WayPts wayPts;
  for (const VsectionRoute::Point &point : _vsectionRoute.points()) {
    wayPts.addPoint(point.lat, point.lon);
  }

  MdvReader *reader = _gd.mread[_fieldNum];
  connect(reader, &MdvReader::verticalReadComplete,
          this, &GuiManager::_verticalReadComplete,
          Qt::UniqueConnection);
  // statusBar()->showMessage("Requesting vertical section");
  const int generation = reader->requestVertSection(
    _timeControl->getSelectedTime(), _fieldNum, wayPts);
  _requestVerticalWinds();
  _requestVerticalTerrain();
  return generation;
}

/////////////////////////////////////////////////////////////
// request horizontal U/V components along the retained section route

int GuiManager::_requestVerticalWinds()
{
  if (_gd.wind_mgr == nullptr || _vsectionRoute.size() < 2) {
    return -1;
  }

  WayPts wayPts;
  for (const VsectionRoute::Point &point : _vsectionRoute.points()) {
    wayPts.addPoint(point.lat, point.lon);
  }
  return _gd.wind_mgr->requestVerticalData(
    _timeControl->getSelectedTime(), _fieldNum, wayPts);
}

/////////////////////////////////////////////////////////////
// determine whether the selected plan view has a terrain height coordinate

bool GuiManager::_horizontalTerrainApplies() const
{
  if (!_gd.layers.earth.terrain_active ||
      _gd.layers.earth.terr == nullptr ||
      _fieldNum < 0 || _fieldNum >= _gd.num_datafields) {
    return false;
  }

  const MdvReader *field = _gd.mread[_fieldNum];
  return field != nullptr && field->isValidH() &&
    field->ds_fhdr.nz > 1 && field->composite_mode == FALSE &&
    field->ds_fhdr.vlevel_type == Mdvx::VERT_TYPE_Z &&
    field->proj.getProjType() != Mdvx::PROJ_POLAR_RADAR;
}

/////////////////////////////////////////////////////////////
// request the horizontal terrain plane for the current plan view

int GuiManager::_requestHorizontalTerrain()
{
  MdvReader *terrain = _gd.layers.earth.terr;
  _terrainHorizontalSettled = true;
  if (!_horizontalTerrainApplies() || terrain == nullptr) {
    _terrainHorizontalRequestGeneration = -1;
    _terrainHorizontalRequestPending = false;
    return -1;
  }

  connect(terrain, &MdvReader::horizontalReadComplete,
          this, &GuiManager::_terrainHorizontalReadComplete,
          Qt::UniqueConnection);

  // Do not supersede a blocking library call: MdvReader cannot interrupt it,
  // and bumping its generation would discard the result without starting the
  // replacement. Queue the latest display state for that completion instead.
  if (terrain->getReadBusyH()) {
    _terrainHorizontalRequestPending = true;
    _terrainHorizontalSettled = false;
    return _terrainHorizontalRequestGeneration;
  }

  _terrainHorizontalRequestPending = false;
  _terrainHorizontalSettled = false;
  _terrainHorizontalRequestGeneration = terrain->requestHorizPlane(
    _timeControl->getSelectedTime(), 0.0, _fieldNum);

  // An unchanged static request completes synchronously from retained data
  // and therefore does not pass through readDoneH or emit a completion signal.
  if (terrain->isNewH()) {
    _terrainHorizontalSettled = true;
  }
  return _terrainHorizontalRequestGeneration;
}

/////////////////////////////////////////////////////////////
// request the static elevation field along the retained section route

int GuiManager::_requestVerticalTerrain()
{
  _terrainVerticalRequestGeneration = -1;
  _terrainVerticalSettled = true;
  MdvReader *terrain = _gd.layers.earth.terr;
  if (!_gd.layers.earth.terrain_active || terrain == nullptr ||
      _vsectionRoute.size() < 2) {
    return -1;
  }

  WayPts wayPts;
  for (const VsectionRoute::Point &point : _vsectionRoute.points()) {
    wayPts.addPoint(point.lat, point.lon);
  }
  connect(terrain, &MdvReader::verticalReadComplete,
          this, &GuiManager::_terrainVerticalReadComplete,
          Qt::UniqueConnection);
  _terrainVerticalRequestGeneration = terrain->requestVertSection(
    _timeControl->getSelectedTime(), _fieldNum, wayPts);
  _terrainVerticalSettled = _terrainVerticalRequestGeneration < 0;
  return _terrainVerticalRequestGeneration;
}

/////////////////////////////////////////////////////////////
// display a completed vertical-section read

void GuiManager::_verticalReadComplete(MdvReader *reader)
{
  // Vertical reads run independently for each field.  Ignore a result from a
  // field which was superseded while its request was in flight; the selected
  // field has its own request pending (or has already completed).

  if (_fieldNum < 0 || _fieldNum >= _gd.num_datafields ||
      reader != _gd.mread[_fieldNum]) {
    return;
  }

  // if (reader == nullptr || !reader->isValidV()) {
  //   statusBar()->showMessage("Vertical section data are not available", 5000);
  //   return;
  // }

  _vert->setSection(reader);
  _vertWindow->setSectionInfo(_params._fields[_fieldNum].field_name,
                              reader->v_date,
                              reader->getVertTotalLengthKm());

  // Automatic vertical output is tied to the exact request generation. A
  // failed completion advances immediately; waiting for the timeout would
  // only delay an unattended batch after the MDV layer has already answered.
  if (_imageGenerationManager->isActive() &&
      _imageGenerationManager->currentJobIsVertical() &&
      reader == _gd.mread[
        _imageGenerationManager->currentJob().fieldIndex] &&
      reader->getVertGeneration() ==
        _imageGenerationManager->expectedGeneration()) {
    if (!reader->isValidV()) {
      _imageGenerationManager->failCurrentJob(
        tr("Vertical-section data are not available for %1")
          .arg(_params._fields[_fieldNum].field_name));
    } else if (_imageGenerationManager->acceptDataCompletion(
                 reader->getVertGeneration())) {
      _checkAutomaticImageReady();
    }
    return;
  }

  if (isMovieBuilding() &&
      _movieManager->markVerticalReady(reader->getVertGeneration())) {
    _checkMovieFrameReady();
    return;
  }

  // Field changes during cached playback update the hidden live section. Do
  // not raise the window or replace the frame currently owned by the movie.
  if (isMovieActive()) {
    return;
  }

  _showVertAct->setChecked(true);
  _vertWindow->show();
  _vertWindow->raise();
  _vertWindow->activateWindow();
  // statusBar()->showMessage("Vertical section ready", 5000);
}

/////////////////////////////////////////////////////////////
// publish a completed horizontal terrain plane

void GuiManager::_terrainHorizontalReadComplete(MdvReader *reader,
                                                int generation,
                                                bool success)
{
  if (reader == nullptr || reader != _gd.layers.earth.terr) {
    return;
  }

  if (_terrainHorizontalRequestPending) {
    // A field, level, time, or zoom changed while the previous static read was
    // blocked. Its data are safe to install, but request the newest display
    // state before declaring terrain ready for capture.
    _terrainHorizontalRequestPending = false;
    if (!success) {
      reader->cancelHorizRequest(generation);
    }
    _requestHorizontalTerrain();
    if (_horiz != nullptr) {
      _horiz->rerenderGrid();
    }
    _checkMovieFrameReady();
    _checkAutomaticImageReady();
    return;
  }

  if (generation != _terrainHorizontalRequestGeneration) {
    return;
  }

  _terrainHorizontalSettled = true;
  if (!success) {
    // Terrain is optional. Stop MdvReader's normal indefinite retry policy so
    // a missing static file does not prevent movies or unattended images.
    reader->cancelHorizRequest(generation);
    _terrainHorizontalRequestGeneration = -1;
  }
  if (_horiz != nullptr) {
    _horiz->rerenderGrid();
  }
  _checkMovieFrameReady();
  _checkAutomaticImageReady();
}

/////////////////////////////////////////////////////////////
// publish a completed terrain profile without disturbing the field section

void GuiManager::_terrainVerticalReadComplete(MdvReader *reader)
{
  if (reader == nullptr || reader != _gd.layers.earth.terr ||
      reader->getVertGeneration() != _terrainVerticalRequestGeneration) {
    return;
  }

  // Terrain is optional display context. Completion, including a failed MDV
  // read, releases movie and automatic-image readiness; VertView simply omits
  // the profile when the reader is invalid.
  _terrainVerticalSettled = true;
  if (_vert) {
    _vert->rerenderTerrain();
  }
  _checkMovieFrameReady();
  _checkAutomaticImageReady();
}

/////////////////////////////
// vert window has closed

void GuiManager::vertWindowClosed(QCloseEvent *event)

{
  _showVertAct->setChecked(false);
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
  text += "\n";
  if (_params.single_segment_vsection) {
    text += "Right-drag and release to select a vertical section.\n";
  } else {
    text += "Right-drag to start a vertical-section route.\n";
    text += "Right-click to add segments and double-click to finish.\n";
    text += "Use Backspace to remove a point or Escape to cancel.\n";
  }
  text += "Use Shift-right-click for plan-view interaction help.\n";
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
