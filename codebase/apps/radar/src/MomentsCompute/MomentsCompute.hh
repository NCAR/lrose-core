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
// MomentsCompute.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2006
//
///////////////////////////////////////////////////////////////

#ifndef MomentsCompute_hh
#define MomentsCompute_hh

#include <string>
#include <vector>
#include <deque>
#include <cstdio>
#include "Params.hh"
#include "Pulse.hh"
#include "Beam.hh"
#include "MomentsMgr.hh"
using namespace std;

////////////////////////
// This class

class MomentsCompute {

public:

	// constructor

	MomentsCompute(Params params);

	// destructor

	~MomentsCompute();

        // process a pulse

	int processPulse(
		float* data, 
		float* crossdata,
		int gates, 
		double prt, 
		double el, 
		double az, 
		long long pulseNum,
		bool horizontal);

	// If a new beam is available, return it.
	// @return A pointer to the available beam. If
	// no beam is avaiable, a null is returned. A beam 
	// may be fetched only once. The client must 
	// delete the Beam when finished with it.
	Beam* getNewBeam();

	// data members

	bool isOK;

  // set debugging

  void setDebug();
  void setDebugVerbose();
  void setDebugExtraVerbose();

  // get params

  const Params &getParams() { return _params; }

protected:

	void _prepareForMoments(Pulse *pulse);

	bool _beamReady(double& beamAz);

	int _computeBeamMoments(Beam *beam);

	void _addPulseToQueue(Pulse *pulse);

	void _addBeamToQueue(Beam *beam);

	// missing data value

	static const double _missingDbl;

	// parameters

	Params _params;

	// pulse queue

	deque<Pulse *> _pulseQueue;
	int _maxPulseQueueSize;
	long long _pulseSeqNum;

	// moments computation management

	MomentsMgr *_momentsMgr;

	double _prevPrfForMoments;

	static const int _maxGates = 4096;
	int _nSamples;

	// beam identification

	int _midIndex1;
	int _midIndex2;
	int _countSinceBeam;

	// beam time and location

	double _time;
	double _az;
	double _el;
	double _prevAz;
	double _prevEl;

	int _nGatesPulse;
	int _nGatesOut;
 
	// The currently computed beam, ready to 
	// to be fetched via getNextBeam(). Once fetched,
	// this will be set to null.
	Beam* _currentBeam;


};

#endif

