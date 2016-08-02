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
// MomentsEngine.cc
//
// MomentsEngine object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////
//
// MomentsEngine reads raw time-series data, and
// computes the moments.
//
////////////////////////////////////////////////////////////////

#include <time.h>
#include <cerrno>
#include <cmath>
#include <iostream>
#include <iomanip>
#include "MomentsEngine.hh"
using namespace std;

const double MomentsEngine::_missingDbl = -9999.0;

////////////////////////////////////////////////////
// Constructor

MomentsEngine::MomentsEngine(Params params):
_currentBeam(0),
_params(params)
{

	_momentsMgr = NULL;

	_nSamples = _params.moments_params.n_samples;
	_maxPulseQueueSize = 0;

	_midIndex1 = 0;
	_midIndex2 = 0;
	_countSinceBeam = 0;

	_pulseSeqNum = 0;

	_az = 0.0;
	_prevAz = -1.0;
	_el = 0.0;
	_prevEl = -180;
	_time = 0;

	_nGatesPulse = 0;
	_nGatesOut = 0;

	isOK = true;

	// set up moments objects
	// This initializes the FFT package to the set number of samples.

	_momentsMgr = new MomentsMgr(_params, _params.moments_params);

	// compute pulse queue size, set to 2 * maxNsamples

	int maxNsamples = _params.moments_params.n_samples;
	_maxPulseQueueSize = maxNsamples * 2 + 2;
	if (_params.debug >= Params::DEBUG_VERBOSE) {
		cerr << "_maxPulseQueueSize: " << _maxPulseQueueSize << endl;
	}

	return;

}

//////////////////////////////////////////////////////////////////
// destructor

MomentsEngine::~MomentsEngine()

{

	for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
		if (_pulseQueue[ii]->removeClient("MomentsEngine destructor") == 0) {
			delete _pulseQueue[ii];
		}
	} // ii
	_pulseQueue.clear();

        if (_momentsMgr) {
          delete _momentsMgr;
        }

}

// set debugging

void MomentsEngine::setDebug()
{
	_params.debug = Params::DEBUG_NORM;
}
void MomentsEngine::setDebugVerbose()
{
	_params.debug = Params::DEBUG_VERBOSE;
}
void MomentsEngine::setDebugExtraVerbose()
{
	_params.debug = Params::DEBUG_EXTRA_VERBOSE;
}

/////////////////////////////////////////////////
// are we ready for a beam?
//
// Side effects: sets _az, _midIndex1, _midIndex2

bool MomentsEngine::_beamReady(double& beamAz)
{

	_countSinceBeam++;
	_az = 0.0;

	// enough data in the queue?

	int minPulses = _nSamples;
	if (_momentsMgr->getMode() == Params::DUAL_CP2_SBAND) {
		// need one extra pulse because we sometimes need to search
		// backwards for horizontal pulse
		minPulses++;
	}

	if ((int) _pulseQueue.size() < minPulses) {
		return false;
	}

	// check we have constant nGates

	int nGates = _pulseQueue[0]->getNGates();
	for (int i = 1; i < _nSamples; i++) {
		if (_pulseQueue[i]->getNGates() != nGates) {
			return false;
		}
	}

	// check we have constant prt

	double prt = _pulseQueue[0]->getPrt();
	for (int i = 1; i < _nSamples; i++) {
		if (fabs(_pulseQueue[i]->getPrt() - prt) > 0.001) {
			return false;
		}
	}

	// compute the indices at the middle of the beam.
	// index1 is just short of the midpt
	// index2 is just past the midpt

	_midIndex1 = _nSamples / 2;
	_midIndex2 = _midIndex1 - 1;

	// compute azimuths which need to straddle the center of the beam

	double midAz1 = _pulseQueue[_midIndex1]->getAz();
	double midAz2 = _pulseQueue[_midIndex2]->getAz();

	if (_params.moments_params.index_beams_in_azimuth) {

		// compute target azimiuth by rounding the azimuth at the
		// center of the data to the closest suitable az

		_az = ((int) (midAz1 / _params.moments_params.azimuth_resolution + 0.5)) *
			_params.moments_params.azimuth_resolution;

		if (_az >= 360.0) {
			_az -= 360;
		} else if (_az < 0) {
			_az += 360.0;
		}

		// Check if the azimuths at the center of the data straddle
		// the target azimuth

		if (midAz1 <= _az && midAz2 >= _az) {

			// az1 is below and az2 above - clockwise rotation
			beamAz = _az;
			return true;

		} else if (midAz1 >= _az && midAz2 <= _az) {

			// az1 is above and az2 below - counterclockwise rotation
			beamAz = _az;
			return true;

		} else if (_az == 0.0) {

			if (midAz1 > 360.0 - _params.moments_params.azimuth_resolution &&
				midAz2 < _params.moments_params.azimuth_resolution) {

					// az1 is below 0 and az2 above 0 - clockwise rotation
					beamAz = _az;
					return true;

				} else if (midAz2 > 360.0 - _params.moments_params.azimuth_resolution &&
					midAz1 < _params.moments_params.azimuth_resolution) {

						// az1 is above 0 and az2 below 0 - counterclockwise rotation
						beamAz = _az;
						return true;

					}

		} else if (_countSinceBeam > (_nSamples * 16)) {

			// antenna moving very slowly, we have waited long enough
			beamAz = _az;
			return true;

		}

	} else {

		// do not index - only check we have enough data

		if (_countSinceBeam >= _nSamples) {
			_az = midAz1;
			if (_az >= 360.0) {
				_az -= 360;
			} else if (_az < 0) {
				_az += 360.0;
			}
			beamAz = _az;
			return true;
		}

	}

	return false;

}

/////////////////////////////////////////////////
// compute moments for the beam

int MomentsEngine::_computeBeamMoments(Beam *beam)

{

	// compute moments

	beam->computeMoments();

	// check for missing az

	bool azMissing = false;
	if (_prevAz >= 0) {
		double deltaAz = fabs(beam->getAz() - _prevAz);
		if (deltaAz > 180) {
			deltaAz = fabs(deltaAz - 360.0);
		}
		if (deltaAz > _params.moments_params.azimuth_resolution) {
			azMissing = true;
		}
	}

	if (_params.debug) {
		if (azMissing) {
			cerr << "Azimuths missing, prev: " << _prevAz
				<< ", this: " << beam->getAz() << endl;
		}
	}

	_prevAz = beam->getAz();
	_prevEl = beam->getEl();

	return 0;

}

/////////////////////////////////////////////////
// add the pulse to the pulse queue

void MomentsEngine::_addPulseToQueue(Pulse *pulse)

{

	// manage the size of the pulse queue, popping off the back

	if ((int) _pulseQueue.size() > _maxPulseQueueSize) {
		Pulse *oldest = _pulseQueue.back();
		if (oldest->removeClient("Deleting from queue") == 0) {
			delete oldest;
		}
		_pulseQueue.pop_back();
	}

	// push pulse onto front of queue

	pulse->addClient("Adding to queue");
	_pulseQueue.push_front(pulse);

	// print missing pulses if requested

	if ((int) pulse->getSeqNum() != _pulseSeqNum + 1) {
		if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
			cout << "**************** Missing seq num: " << _pulseSeqNum
				<< " to " <<  pulse->getSeqNum() << " **************" << endl;
		}
	}
	_pulseSeqNum = pulse->getSeqNum();

}

/////////////////////////////////////////////////////////////

int 
MomentsEngine::processPulse(
							 float* data,
							 float* crossdata,
							 int gates, 
							 double prt, 
                                                         double time,
							 double el, 
							 double az, 
							 long long pulseNum,
							 bool horizontal)
{

	// process pulses as they arrive

//	double now = (double) time(NULL);
	bool isHoriz = horizontal;

	// Create a new pulse object and save a pointer to it in the
	// _pulseBuffer array.  _pulseBuffer is a FIFO, with elements
	// added at the end and dropped off the beginning. So if we have a
	// full buffer delete the first element before shifting the
	// elements to the left.

	Pulse *pulse = new Pulse(_params, 
		pulseNum, gates, time,
		prt, el, az, isHoriz, data, crossdata);

	// add pulse to queue, managing memory appropriately

	_addPulseToQueue(pulse);

	_nGatesPulse = pulse->getNGates();

	// is a beam ready?
	double beamAz;

	if (_momentsMgr != NULL && _beamReady(beamAz)) {

		_countSinceBeam = 0;

		// create new beam
		Beam* beam = new Beam(_params, 
			_pulseQueue, 
			beamAz, 
			_momentsMgr);

		// compute beam moments

		_computeBeamMoments(beam);

                if (_params.debug >= Params::DEBUG_VERBOSE) {
                  cerr << "--->> computing moments, el, az: "
                       << el << ", " << az << endl;
                }

		// return old beam, if it was never fetched.
		// Note that if a beam was fetched from
		// _currentBeam via getNewBeam(),
		// then _currentBeam was set to null, 
		// and it is up to the owner of
		// MomentsEngine to delete that beam when finished with it.
		if (_currentBeam)
			delete _currentBeam;

		_currentBeam = beam;

	}

	return 0;

}

/////////////////////////////////////////////////////////////

Beam*
MomentsEngine::getNewBeam() {

	Beam* retval = _currentBeam;

	_currentBeam = 0;

	return retval;
}


