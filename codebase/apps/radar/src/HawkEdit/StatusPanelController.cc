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
// StatusPanel.cc
//
// Status Panel object
//
// Brenda Javornik, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2023
//
///////////////////////////////////////////////////////////////
//
// StatusPanel manages the metadata displayed on the left-hand side
// of the Polar display
//
///////////////////////////////////////////////////////////////

#include "StatusPanelController.hh"
#include "DisplayField.hh"
#include "FieldListView.hh"
#include "Params.hh"
#include "Reader.hh"
#include "AllocCheck.hh"

#include <string>
#include <cmath>
#include <iostream>
#include <algorithm>    // std::find
#include <unordered_map>
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

#include <cstdlib>
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
//using namespace H5x;

// Constructor


StatusPanelController::StatusPanelController(StatusPanelView *view)
{
  _view = view;
}

// destructor

StatusPanelController::~StatusPanelController()
{
  cerr << "StatusPanel destructor called " << endl;
}

void StatusPanelController::setDisplay(
    bool show_fixed_angle,
    bool show_volume_number,
    bool show_sweep_number,
    bool show_n_samples,
    bool show_n_gates,
    bool show_gate_length,
    bool show_pulse_width,
    bool show_prf_mode,
    bool show_prf,
    bool show_nyquist,
    bool show_max_range,
    bool show_unambiguous_range,
    bool show_measured_power_h,
    bool show_measured_power_v,
    bool show_scan_name,
    bool show_scan_mode,
    bool show_polarization_mode,
    bool show_latitude,
    bool show_longitude,
    bool show_altitude,
    bool show_altitude_rate,
    bool show_speed,
    bool show_heading,
    bool show_track,
    bool show_sun_elevation,
    bool show_sun_azimuth) {

    _show_fixed_angle = show_fixed_angle;
    _show_volume_number = show_volume_number;
    _show_sweep_number = false; //show_sweep_number;
    _show_n_samples = false; // show_n_samples;
    _show_n_gates = false; // show_n_gates;
    _show_gate_length = false; // show_gate_length;
    _show_pulse_width = false; // show_pulse_width;
    _show_prf_mode = false; // show_prf_mode;
    _show_prf = false; // show_prf;
    _show_nyquist = false; // show_nyquist;
    _show_max_range = false; // show_max_range;
    _show_unambiguous_range = false; // show_unambiguous_range;
    _show_measured_power_h = false; // show_measured_power_h;
    _show_measured_power_v = false; // show_measured_power_v;
    _show_scan_name = false; // show_scan_name;
    _show_scan_mode = false; // show_scan_mode;
    _show_polarization_mode = false; // show_polarization_mode;
    _show_latitude = false; // show_latitude;
    _show_longitude = false; // show_longitude;
    _show_altitude = false; // show_altitude;
    _show_altitude_rate = false; // show_altitude_rate;
    _show_speed = false; // show_speed;
    _show_heading = false; // show_heading;
    _show_track = false; // show_track;
    _show_sun_elevation = false; // show_sun_elevation;
    _show_sun_azimuth = false; // show_sun_azimuth;
}


void StatusPanelController::createStatusPanel() {

//  _elevVal = _createStatusVal("Elev", "-99.99", row++, fsize2);
//  _azVal = _createStatusVal("Az", "-999.99", row++, fsize2);

  if (_show_fixed_angle) {
    _view->create(StatusPanelView::FixedAngleKey);
  }
  
  if (_show_volume_number) {
    _view->create(StatusPanelView::VolumeNumberKey);
  }
  
  if (_show_sweep_number) {
    _view->create(StatusPanelView::SweepNumberKey);
  }
  
  if (_show_n_samples) {
    _view->create(StatusPanelView::NSamplesKey);
  }

  if (_show_n_gates) {
    _view->create(StatusPanelView::NGatesKey);
  }
  
  if (_show_gate_length) {
    _view->create(StatusPanelView::GateSpacingKey);
  }
  
  
  if (_show_pulse_width) {
    _view->create(StatusPanelView::PulseWidthKey);
  }

  if (_show_prf_mode) {
    _view->create(StatusPanelView::PrfModeKey);
  }

  if (_show_prf) {
    _view->create(StatusPanelView::PrfKey);
  }

  if (_show_nyquist) {
    _view->create(StatusPanelView::NyquistKey);
  }

  if (_show_max_range) {
    _view->create(StatusPanelView::MaxRangeKey);
  }

  if (_show_unambiguous_range) {
    _view->create(StatusPanelView::UnambiguousRangeKey);
  }

  if (_show_measured_power_h) {
    _view->create(StatusPanelView::PowerHKey);
  }

  if (_show_measured_power_v) {
    _view->create(StatusPanelView::PowerVKey);
  }

  if (_show_scan_name) {
    _view->create(StatusPanelView::ScanNameKey);
  }

  if (_show_scan_mode) {
    _view->create(StatusPanelView::SweepModeKey);
  }

  if (_show_polarization_mode) {
    _view->create(StatusPanelView::PolarizationModeKey);
  }

  if (_show_latitude) {
    _view->create(StatusPanelView::LatitudeKey);
  }

  if (_show_longitude) {
    _view->create(StatusPanelView::LongitudeKey);
  }

  if (_show_altitude) {
    if (_altitudeInFeet) {
      _view->create(StatusPanelView::AltitudeInFeetKey);
    } else {
      _view->create(StatusPanelView::AltitudeInKmKey);
    }
  }

  if (_show_altitude_rate) {
    if (_altitudeInFeet) {
      _view->create(StatusPanelView::AltitudeRateFtsKey);
    } else {
      _view->create(StatusPanelView::AltitudeRateMsKey);
    }
  }

  if (_show_speed) {
    _view->create(StatusPanelView::SpeedKey);
  }

  if (_show_heading) {
    _view->create(StatusPanelView::HeadingKey);
  }

  if (_show_track) {
    _view->create(StatusPanelView::TrackKey);
  }

  if (_show_sun_elevation) {
    _view->create(StatusPanelView::SunElevationKey);
  }

  if (_show_sun_azimuth) {
    _view->create(StatusPanelView::SunAzimuthKey);
  }
  
  _view->show();

}


void StatusPanelController::setRadarName(string radarName, string siteName) {
  _view->setRadarName(radarName, siteName);
};

void StatusPanelController::createDateTime() {
  _view->createDateTime();
}

// ??? the push should NOT come from the PolarManager;
// the pull of information should come from the view via the controller!!!
// PolarManager emits event
// StatusPanelController listens for the event and calls the _view action
// StatusPanelView emits request for info for row,column???
// StatusPanelView getValue (row, column) of table view.
void StatusPanelController::updateStatusPanel(const RadxRay *ray) {

  if (ray == NULL) 
    return;

  DateTime rayTime(ray->getTimeSecs());
  //sprintf(text, "%.4d/%.2d/%.2d",
  //        rayTime.getYear(), rayTime.getMonth(), rayTime.getDay());

  //sprintf(text, "%.2d:%.2d:%.2d.%.3d",
  //        rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
  //        ((int) ray->getNanoSecs() / 1000000));

  _view->setDateTime(rayTime.getYear(), rayTime.getMonth(), rayTime.getDay(),
    rayTime.getHour(), rayTime.getMin(), rayTime.getSec(),
    ray->getNanoSecs());

  _view->updateAzEl(ray->getAzimuthDeg(), ray->getElevationDeg());

  //if (_fixedAngVal) {  
  //  _setText(text, "%6.2f", ray->getFixedAngleDeg());
  //  _fixedAngVal->setText(text);
  //}

  //   _view->set(<key>, <value>) // one for int, one for float, one for string.
  //values.setSet(nDisplayValues);

  // we now have a hash/unordered_map of all the values to display
  // if we can just go through the unordered_map systematically, and
  // know which ray value is associated, then we don't need all the if
  // statements.

  // go through the unordered_map and use a case statement on the Keys?
  // to associate the ray variable?


  const StatusPanelView::Hashy myHashy = _view->getMetaDataMap();
  StatusPanelView::HashyConstIt it;
  for (it=myHashy.begin(); it != myHashy.end(); ++it) {
    int key = it->first;
    switch (key) {
      case StatusPanelView::FixedAngleKey:
        _view->set(StatusPanelView::FixedAngleKey, ray->getFixedAngleDeg());
        break;
      case StatusPanelView::VolumeNumberKey:
        _view->set(StatusPanelView::VolumeNumberKey, ray->getVolumeNumber());
        break;
      default:
        cerr << "not found" << endl;

    }
  }

/*

  HashyIt it;
  for (it=hashy.begin(); it != hashy.end(); ++it) {
    int key = it->first;
    switch (key) {
      case FixedAngleKey:
        //_view->set(StatusPanelView::FixedAngleKey, ray->getFixedAngleDeg());
        cerr << "FixedAngleKey" << endl;
        break;
      case VolumeNumberKey:
        //_view->set(StatusPanelView::VolumeNumberKey, ray->getVolumeNumber());
        cerr << "VolumeNumberKey" << endl;
        break;
      default:
        cerr << "not found" << endl;

    }
  }
*/


  if (_show_volume_number)
    //_view->setValue(index, value) // requires templates in view/Q_OBJECT
    //_values->push_back(format(ray->getVolumeNumber()); // requires the format to be kept by the controller;
      // AND cannot use QString for formatting.  Grrh! 
      // just use separate functions darn it!!!
      // make each row it's own class??? with format info, etc???
    _view->set(StatusPanelView::VolumeNumberKey, ray->getVolumeNumber());

  if (_show_sweep_number)
    _view->set(StatusPanelView::SweepNumberKey, ray->getSweepNumber());
    // _view->setSweepNum(ray->getSweepNumber());

  if (_show_fixed_angle) 
    _view->set(StatusPanelView::FixedAngleKey, ray->getFixedAngleDeg());
  
  if (_show_n_samples) 
    _view->set(StatusPanelView::NSamplesKey, ray->getNSamples());
  if (_show_n_gates) 
    _view->set(StatusPanelView::NGatesKey, ray->getNGates());      
  if (_show_gate_length) 
    _view->set(StatusPanelView::GateSpacingKey, ray->getGateSpacingKm());
  if (_show_pulse_width) 
    _view->set(StatusPanelView::PulseWidthKey, ray->getPulseWidthUsec());
  //if (_show_prf_mode) _view->set(StatusPanelView::
  //if (_show_prf) _view->set(StatusPanelView::
  if (_show_nyquist) 
    _view->set(StatusPanelView::NyquistKey, ray->getNyquistMps());
    
/*

    if (_show_max_range) _view->set(StatusPanelView::("maxRange", value)
    if (_show_unambiguous_range) _view->set(StatusPanelView::
    if (_show_measured_power_h) _view->set(StatusPanelView::
    if (_show_measured_power_v) _view->set(StatusPanelView::
    if (_show_scan_name) _view->set(StatusPanelView::
    if (_show_scan_mode) _view->set(StatusPanelView::
    if (_show_polarization_mode) _view->set(StatusPanelView::
    if (_show_latitude) _view->set(StatusPanelView::
    if (_show_longitude) _view->set(StatusPanelView::
    if (_show_altitude) _view->set(StatusPanelView::
    if (_show_altitude_rate) _view->set(StatusPanelView::
    if (_show_speed) _view->set(StatusPanelView::
    if (_show_heading) _view->set(StatusPanelView::
    if (_show_track) _view->set(StatusPanelView::
    if (_show_sun_elevation) _view->set(StatusPanelView::
    if (_show_sun_azimuth) _view->set(StatusPanelView::

*/
    /*
    ray->getGateSpacingKm(),
    ray->getPulseWidthUsec(),
    ray->getNyquistMps()
    );
    */
}

void StatusPanelController::setFontSize(int fontSize) {
  _view->setFontSize(fontSize);
}

/*
string StatusPanelController::interpretPrf(Radx::  rayPrtMode, double rayPrtSec, double rayPrtRatio) {
  string text;

    if (_view->_prfVal) {
    if (ray->getPrtMode() == Radx::PRT_MODE_FIXED) {
      if (ray->getPrtSec() <= 0) {
        _setText(text, "%d", -9999);
      } else {
        _setText(text, "%d", (int) ((1.0 / ray->getPrtSec()) * 10.0 + 0.5) / 10);
      }
    } else {
      double prtSec = rayPrtSec;
      if (prtSec <= 0) {
        _setText(text, "%d", -9999);
      } else {
        int iprt = (int) ((1.0 / rayPrtSec) * 10.0 + 0.5) / 10;
        double prtRatio = rayPrtRatio;
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
  return text;
}
*/

string StatusPanelController::interpretSweepMode(Radx::SweepMode_t sweepMode) {

    switch (sweepMode) {
      case Radx::SWEEP_MODE_SECTOR: {
        return "sector"; break;
      }
      case Radx::SWEEP_MODE_COPLANE: {
        return "coplane"; break;
      }
      case Radx::SWEEP_MODE_RHI: {
        return "RHI"; break;
      }
      case Radx::SWEEP_MODE_VERTICAL_POINTING: {
        return "vert"; break;
      }
      case Radx::SWEEP_MODE_IDLE: {
        return "idle"; break;
      }
      case Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE:
      case Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE: {
        return "SUR"; break;
      }
      case Radx::SWEEP_MODE_SUNSCAN: {
        return "sunscan"; break;
      }
      case Radx::SWEEP_MODE_SUNSCAN_RHI: {
        return "sun_rhi"; break;
      }
      case Radx::SWEEP_MODE_POINTING: {
        return "point"; break;
      }
      case Radx::SWEEP_MODE_CALIBRATION: {
        return "cal"; break;
      }
      default: {
        return "unknown";
      }
    }
}

void StatusPanelController::newDataFile() {
  //clear();
}
/*
//////////////////////////////////////////////////

  // create status panel

  _createStatusPanel();

void StatusPanel::selectedSweepChanged(int sweepNumber) {
  LOG(DEBUG) << "enter"; 
  //string fieldName = newFieldName.toStdString();
  //_displayFieldController->setSelectedField(fieldName);
  if (_sweepController->getSelectedNumber() != sweepNumber) {
    _sweepController->setSelectedNumber(sweepNumber);
    _readDataFile();
    // signal polar display to update; which causes rayLocations to update
    selectedFieldChanged(_displayFieldController->getSelectedFieldName());
    emit newSweepData(sweepNumber);
    //processEvents();
    //_setupRayLocation();  // this is done by _getArchiveData
    //_plotArchiveData();
    //refreshBoundaries();
  } else {
    if (sheetView != NULL) {
      spreadSheetControl->displaySweepData(sweepNumber);
    }
  }
  LOG(DEBUG) << "exit";
}

*/



