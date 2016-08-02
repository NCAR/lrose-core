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
/////////////////////////////////////////////////////////////
// Antenna.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2003
//
///////////////////////////////////////////////////////////////

#include "Antenna.hh"
#include <iostream>
#include <iomanip>
#include <math.h>

using namespace std;


//////////////
// Constructor

Antenna::Antenna(const string &prog_name,
		 const Params &params) :
  _progName(prog_name),
  _params(params)
  
{
  _firstBeam = true;
  _mode = MODE_UNKNOWN;
  _elDirn = ANG_CHANGE_UNKNOWN;
  _azDirn = ANG_CHANGE_UNKNOWN;
  _tiltDeltaAz = 0.0;
  _volMinEl = 180.0;
  _volMaxEl = -180.0;
  _nBeamsVol = 0;
}

/////////////
// destructor

Antenna::~Antenna()

{
  
}

////////////////////////////////////////////////////////////////
// add a beam
//
// Returns true if this beam is the end-of-vol,
// false otherwise

bool Antenna::addBeam(const Beam *beam)
  
{
  
  bool endOfVol = false;

  double el = beam->elev;
  double az = beam->az;

  if (_firstBeam) {
    _accumDeltaAz = 0.0;
    _accumDeltaEl = 0.0;
    _tiltDeltaAz = 0.0;
    _prevEl = el;
    _volMinEl = el;
    _volMaxEl = el;
    _prevAz = az;
    _prevPrf = beam->prf;
    _firstBeam = false;
    return false;
  }

  _nBeamsVol++;
  
  // set elev stats
  
  if (el < _prevEl && el < _volMinEl) {
    _volMinEl = el;
  }
  if (el > _prevEl && el > _volMaxEl) {
    _volMaxEl = el;
  }
    
  if (_params.set_end_of_vol_from_elev_change) {
    if (_nBeamsVol >= _params.min_beams_per_ppi_vol) {
      double deltaEl = 0;
      if (_params.vol_starts_at_bottom) {
        deltaEl = _volMaxEl - el;
      } else {
        deltaEl = el - _volMinEl;
      }
      if (deltaEl > _params.elev_change_for_end_of_vol) {
        endOfVol = true;
        if (_params.debug) {
          cerr << "====>> Setting end_of_vol on ELEV change <<=====" << endl;
          cerr << "  volMinEl: " << _volMinEl << endl;
          cerr << "  volMaxEl: " << _volMaxEl << endl;
          cerr << "  el: " << el << endl;
          cerr << "  deltaEl: " << deltaEl << endl;
          cerr << "    exceeds: "
               << _params.elev_change_for_end_of_vol << endl;
        }
      }
    }
  }

  // has prf changed?
  
  if (_params.set_end_of_vol_on_prf_change) {
    if (!endOfVol && fabs(beam->prf - _prevPrf) > 1.0) {
      endOfVol = true;
      if (_params.debug) {
	cerr << "=====>> Setting end_of_vol on PRF change <<=======" << endl;
	cerr << "        Prev PRF: " << _prevPrf << endl;
	cerr << "        New  PRF: " << beam->prf << endl;
      }
    }
  }

  // compute deltas

  angle_delta_t delta;
  delta.el = el - _prevEl;
  delta.az = az - _prevAz;
  if (delta.az < -180) {
    delta.az += 360.0;
  } else if (delta.az > 180) {
    delta.az -= 360.0;
  }
  
  // save for next time
  
  _prevEl = el;
  _prevAz = az;
  _prevPrf = beam->prf;

  // return early if setting end of vol from elevation change
  
  if (_params.set_end_of_vol_from_elev_change) {
    if (endOfVol) {
      _volMinEl = 180.0;
      _volMaxEl = -180.0;
      _nBeamsVol = 0;
      cerr << endl;
      return true;
    } else {
      return false;
    }
  }

  // store deltas
  
  _deltas.push_back(delta);

  // accumulate the changes in elevation and azimuth

  _accumDeltaEl += delta.el;
  _accumDeltaAz += delta.az;

  // return now if queue has not yet reached full size

  int queueSize = _deltas.size();
  if (queueSize <= _params.nbeams_history) {
    return false;
  }

  // drop off last element, decrementing accumulators
  
  const angle_delta_t &first = _deltas[0];
  _accumDeltaEl -= first.el;
  _accumDeltaAz -= first.az;
  _deltas.pop_front();

  if (_params.debug_auto_detection) {
    cerr << "el, az, deltaEl, deltaAz: "
	 << setw(12) << el
	 << setw(12) << az
	 << setw(12) << _accumDeltaEl
	 << setw(12) << _accumDeltaAz << endl;
  }

  // check mode
  
  bool isPpi = false;
  bool isRhi = false;
  bool isSlewing = false;
  bool isStopped = false;
  
  angle_change_t azChange = ANG_CHANGE_NONE;
  angle_change_t elChange = ANG_CHANGE_NONE;
  
  if (fabs(_accumDeltaEl) <= _params.el_accuracy &&
      fabs(_accumDeltaAz) <= _params.az_accuracy) {
    isStopped = true;
  }
  
  if (fabs(_accumDeltaEl) > _params.el_accuracy &&
      fabs(_accumDeltaAz) > _params.az_accuracy) {
    isSlewing = true;
  }
  
  if (fabs(_accumDeltaEl) > _params.el_accuracy) {
    if (_accumDeltaEl > 0) {
      elChange = ANG_INCR;
    } else {
      elChange = ANG_DECR;
    }
  }
  
  if (fabs(_accumDeltaAz) > _params.az_accuracy) {
    if (_accumDeltaAz > 0) {
      azChange = ANG_INCR;
    } else {
      azChange = ANG_DECR;
    }
  }

  if (fabs(_accumDeltaEl) <= _params.el_accuracy) {
    if (fabs(_accumDeltaAz) >= _params.min_az_change_ppi) {
      isPpi = true;
    }
  }

  if (fabs(_accumDeltaAz) <= _params.az_accuracy) {
    if (fabs(_accumDeltaEl) >= _params.min_el_change_rhi) {
      isRhi = true;
    }
  }
  
  if (isStopped) {
    
    if (_mode != MODE_STOPPED) {
      // if (_params.debug_auto_detection) {
      // cerr << "======>> STOPPED <<======" << endl;
      // }
    }
    
  } else if (isSlewing) {
    
    _handleSlewing(elChange, azChange, endOfVol);

  } else if (isPpi) {

    _handlePpi(delta.az, endOfVol);
    
  } else if (isRhi) {

    _handleRhi(endOfVol);
    
  }

  if (_tiltDeltaAz >= _params.max_az_change_per_tilt) {
    endOfVol = true;
    if (_params.debug_auto_detection) {
      cerr << "========>> az change in tilt exceeded <<============" << endl;
    }
  }
  
  if (endOfVol) {
    if (_params.debug_auto_detection) {
      cerr << ">>>>>> !!!!!!! END-OF-VOL !!!!!!!!!! <<<<<<<" << endl;
    }
    _tiltDeltaAz = 0.0;
    _volMinEl = 180.0;
    _volMaxEl = -180.0;
    _nBeamsVol = 0;
  }

  return endOfVol;
  
}

////////////////////////////////////////////////////////////////
// handle slewing state

void Antenna::_handleSlewing(angle_change_t elChange,
			     angle_change_t azChange,
			     bool &endOfVol)
  
{  

  if (_mode == MODE_SLEWING) {
    return;
  }

  if (_mode == MODE_STOPPED || _mode == MODE_UNKNOWN) {
    if (_params.debug_auto_detection) {
      cerr << "========>> SLEWING <<=======" << endl;
    }
    _mode = MODE_SLEWING;
    return;
  }
  
  if (_mode == MODE_PPI) {
    
    _tiltDeltaAz = 0.0;

    if (_elDirn == ANG_CHANGE_UNKNOWN) {
      
      if (elChange == ANG_INCR) {
	_elDirn = ANG_INCR;
	if (_params.debug_auto_detection) {
	  cerr << "======>> PPI EL INCREASING <<======" << endl;
	}
      } else if (elChange == ANG_DECR) {
	_elDirn = ANG_DECR;
	if (_params.debug_auto_detection) {
	  cerr << "======>> PPI EL DECREASING <<======" << endl;
	}
      }
      return;
      
    } else if (_elDirn == elChange) {
      
      return;
      
    } else {
      
      if (_nBeamsVol < _params.min_beams_per_ppi_vol) {
	if (_params.debug_auto_detection) {
	  cerr << "===>> Cancel PPI end-of-vol - too few beams <<===" << endl;
	}
      } else {
	endOfVol = true;
      }
      _mode = MODE_SLEWING;
      if (_params.debug_auto_detection) {
	cerr << "======>> PPI END OF VOL - SLEWING <<======" << endl;
      }
      return;
      
    }

  } // if (_mode == PPI)
    
  if (_mode == MODE_RHI) {
    
    if (_azDirn == ANG_CHANGE_UNKNOWN) {
      
      if (azChange == ANG_INCR) {
	_azDirn = ANG_INCR;
	if (_params.debug_auto_detection) {
	  cerr << "======>> RHI AZ INCREASING <<======" << endl;
	}
      } else if (azChange == ANG_DECR) {
	_azDirn = ANG_DECR;
	if (_params.debug_auto_detection) {
	  cerr << "======>> RHI AZ DECREASING <<======" << endl;
	}
      }
      return;
      
    } else if (_azDirn == azChange) {
      
      return;
      
    } else {
      
      if (_nBeamsVol < _params.min_beams_per_rhi_vol) {
	if (_params.debug_auto_detection) {
	  cerr << "===>> Cancel RHI end-of-vol - too few beams <<===" << endl;
	}
      } else {
	endOfVol = true;
      }
      _mode = MODE_SLEWING;
      if (_params.debug_auto_detection) {
	cerr << "======>> RHI END OF VOL - SLEWING <<======" << endl;
      }
      return;
      
    }

  } // if (_mode == RHI)
    
}
    
////////////////////////////////////////////////////////////////
// handle ppi state

void Antenna::_handlePpi(double delta_az, bool &endOfVol)
  
{  
  
  if (_mode == MODE_PPI) {
    _tiltDeltaAz += delta_az;
    return;
  }
  
  if (_mode == MODE_RHI) {
    if (_nBeamsVol < _params.min_beams_per_rhi_vol) {
      if (_params.debug_auto_detection) {
	cerr << "===>> Cancel RHI end-of-vol - too few beams <<===" << endl;
      }
    } else {
      endOfVol = true;
      if (_params.debug_auto_detection) {
	cerr << "===========>> RHI - END OF VOL <<==========" << endl;
      }
    }
  }
  
  if (_params.debug_auto_detection) {
    cerr << "=========>> starting PPI <<=============" << endl;
  }
  _tiltDeltaAz = 0.0;
  _mode = MODE_PPI;
  _elDirn = ANG_CHANGE_UNKNOWN;
  return;

}

////////////////////////////////////////////////////////////////
// handle rhi state

void Antenna::_handleRhi(bool &endOfVol)
  
{  
  
  if (_mode == MODE_RHI) {
    return;
  }

  if (_mode == MODE_PPI) {
    if (_nBeamsVol < _params.min_beams_per_ppi_vol) {
      if (_params.debug_auto_detection) {
	cerr << "===>> Cancel PPI end-of-vol - too few beams <<===" << endl;
      }
    } else {
      endOfVol = true;
      if (_params.debug_auto_detection) {
	cerr << "===========>> PPI - END OF VOL <<==========" << endl;
      }
    }
  }

  if (_params.debug_auto_detection) {
    cerr << "=========>> start RHI <<=============" << endl;
  }
  _mode = MODE_RHI;
  _azDirn = ANG_CHANGE_UNKNOWN;
  return;

}
