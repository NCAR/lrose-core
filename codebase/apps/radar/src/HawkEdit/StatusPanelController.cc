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
    _show_sweep_number = show_sweep_number;
    _show_n_samples = show_n_samples;
    _show_n_gates = show_n_gates;
    _show_gate_length = show_gate_length;
    _show_pulse_width = show_pulse_width;
    _show_prf_mode = show_prf_mode;
    _show_prf = show_prf;
    _show_nyquist = show_nyquist;
    _show_max_range = show_max_range;
    _show_unambiguous_range = show_unambiguous_range;
    _show_measured_power_h = show_measured_power_h;
    _show_measured_power_v = show_measured_power_v;
    _show_scan_name = show_scan_name;
    _show_scan_mode = show_scan_mode;
    _show_polarization_mode = show_polarization_mode;
    _show_latitude = show_latitude;
    _show_longitude = show_longitude;
    _show_altitude = show_altitude;
    _show_altitude_rate = show_altitude_rate;
    _show_speed = show_speed;
    _show_heading = show_heading;
    _show_track = show_track;
    _show_sun_elevation = show_sun_elevation;
    _show_sun_azimuth = show_sun_azimuth;
}


void StatusPanelController::createStatusPanel() {

//  _elevVal = _createStatusVal("Elev", "-99.99", row++, fsize2);
//  _azVal = _createStatusVal("Az", "-999.99", row++, fsize2);

  if (_show_fixed_angle) {
    _view->createFixedAngleDeg();
  }
    //_fixedAngVal = _createStatusVal("Fixed ang", "-99.99", row++, fsize2); // TO VIEW???
  //} //else {
   // _fixedAngVal = NULL;
  //}
  
  if (_show_volume_number) {
    _view->createVolumeNumber();
  }
  //  _view->setVolumeNumber(0);
    //_volNumVal = _createStatusVal("Volume", "0", row++, fsize);
  //} //else {
   // _volNumVal = NULL;
  //}
  /*
  if (_show_sweep_number) {
    _sweepNumVal = _createStatusVal("Sweep", "0", row++, fsize);
  } else {
    _sweepNumVal = NULL;
  }

  if (_show_n_samples) {
    _nSamplesVal = _createStatusVal("N samp", "0", row++, fsize);
  } else {
    _nSamplesVal = NULL;
  }

  if (_show_n_gates) {
    _nGatesVal = _createStatusVal("N gates", "0", row++, fsize);
  } else {
    _nGatesVal = NULL;
  }

  if (_show_gate_length) {
    _gateSpacingVal = _createStatusVal("Gate len", "0", row++, fsize);
  } else {
    _gateSpacingVal = NULL;
  }
  
  if (_show_pulse_width) {
    _pulseWidthVal = _createStatusVal("Pulse width", "-9999", row++, fsize);
  } else {
    _pulseWidthVal = NULL;
  }

  if (_show_prf_mode) {
    _prfModeVal = _createStatusVal("PRF mode", "Fixed", row++, fsize);
  } else {
    _prfModeVal = NULL;
  }

  if (_show_prf) {
    _prfVal = _createStatusVal("PRF", "-9999", row++, fsize);
  } else {
    _prfVal = NULL;
  }

  if (_show_nyquist) {
    _nyquistVal = _createStatusVal("Nyquist", "-9999", row++, fsize);
  } else {
    _nyquistVal = NULL;
  }

  if (_show_max_range) {
    _maxRangeVal = _createStatusVal("Max range", "-9999", row++, fsize);
  } else {
    _maxRangeVal = NULL;
  }

  if (_show_unambiguous_range) {
    _unambigRangeVal = _createStatusVal("U-A range", "-9999", row++, fsize);
  } else {
    _unambigRangeVal = NULL;
  }

  if (_show_measured_power_h) {
    _powerHVal = _createStatusVal("Power H", "-9999", row++, fsize);
  } else {
    _powerHVal = NULL;
  }

  if (_show_measured_power_v) {
    _powerVVal = _createStatusVal("Power V", "-9999", row++, fsize);
  } else {
    _powerVVal = NULL;
  }

  if (_show_scan_name) {
    _scanNameVal = _createStatusVal("Scan name", "unknown", row++, fsize);
  } else {
    _scanNameVal = NULL;
  }

  if (_show_scan_mode) {
    _sweepModeVal = _createStatusVal("Scan mode", "SUR", row++, fsize);
  } else {
    _sweepModeVal = NULL;
  }

  if (_show_polarization_mode) {
    _polModeVal = _createStatusVal("Pol mode", "Single", row++, fsize);
  } else {
    _polModeVal = NULL;
  }

  if (_show_latitude) {
    _latVal = _createStatusVal("Lat", "-99.999", row++, fsize);
  } else {
    _latVal = NULL;
  }

  if (_show_longitude) {
    _lonVal = _createStatusVal("Lon", "-999.999", row++, fsize);
  } else {
    _lonVal = NULL;
  }

  if (_show_altitude) {
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

  if (_show_altitude_rate) {
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

  if (_show_speed) {
    _speedVal = _createStatusVal("Speed(m/s)", "-999.99", row++, fsize);
  } else {
    _speedVal = NULL;
  }

  if (_show_heading) {
    _headingVal = _createStatusVal("Heading(deg)", "-999.99", row++, fsize);
  } else {
    _headingVal = NULL;
  }

  if (_show_track) {
    _trackVal = _createStatusVal("Track(deg)", "-999.99", row++, fsize);
  } else {
    _trackVal = NULL;
  }

  if (_show_sun_elevation) {
    _sunElVal = _createStatusVal("Sun el (deg)", "-999.999", row++, fsize);
  } else {
    _sunElVal = NULL;
  }

  if (_show_sun_azimuth) {
    _sunAzVal = _createStatusVal("Sun az (deg)", "-999.999", row++, fsize);
  } else {
    _sunAzVal = NULL;
  }
*/

}


void StatusPanelController::setRadarName(string radarName, string siteName) {
  _view->setRadarName(radarName, siteName);
};

// ??? the push should NOT come from the PolarManager;
// the pull of information should come from the view via the controller!!!
// PolarManager emits event
// StatusPanelController listens for the event and calls the _view action
// StatusPanelView emits request for info for row,column???
// StatusPanelView getValue (row, column) of table view.
void StatusPanelController::updateStatusPanel(const RadxRay *ray) {


  //if (_fixedAngVal) {  
  //  _setText(text, "%6.2f", ray->getFixedAngleDeg());
  //  _fixedAngVal->setText(text);
  //}

  if (_show_volume_number)
    _view->setVolumeNumber(ray->getVolumeNumber());

  //if (_show_sweep_number)
  //  _view->setSweepNum(ray->getSweepNumber());

    //ray->getElevationDeg(),
    //ray->getAzimuthDeg(),

  if (_show_fixed_angle) 
    _view->setFixedAngleDeg(ray->getFixedAngleDeg());
  /*
  if (_show_n_samples) 
    _view->setNSamples(ray->getNSamples());
  if (_show_n_gates) 
    _view->setNGates(ray->getNGates());      
  if (_show_gate_length) 
    _view->setGateSpacingKm(ray->getGateSpacingKm());
  if (_show_pulse_width) 
    _view->setPulseWidth(ray->getPulseWidthUsec());
  //if (_show_prf_mode) _view->set
  //if (_show_prf) _view->set
  if (_show_nyquist) 
    _view->setNyquist(ray->getNyquistMps());
    */
    /*
    if (_show_max_range) _view->set
    if (_show_unambiguous_range) _view->set
    if (_show_measured_power_h) _view->set
    if (_show_measured_power_v) _view->set
    if (_show_scan_name) _view->set
    if (_show_scan_mode) _view->set
    if (_show_polarization_mode) _view->set
    if (_show_latitude) _view->set
    if (_show_longitude) _view->set
    if (_show_altitude) _view->set
    if (_show_altitude_rate) _view->set
    if (_show_speed) _view->set
    if (_show_heading) _view->set
    if (_show_track) _view->set
    if (_show_sun_elevation) _view->set
    if (_show_sun_azimuth) _view->set

  _view->updateStatusPanel(

    
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



