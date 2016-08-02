#include "MomentThread.h"
#include <iostream>

/// A singleton mutex to protect the creation of the MomentsEngine, which
/// calls non-reentrant code in FFTW. 
static QMutex _momentsEngineMutex;


///////////////////////////////////////////////////////////////////////////////////
MomentThread::MomentThread(Params params):
_params(params)
{
}

///////////////////////////////////////////////////////////////////////////////////
MomentThread::~MomentThread(void)
{
}

///////////////////////////////////////////////////////////////////////////////////
void
MomentThread::run()
{
	// create the moment compute engine for S band
	// MomentsEngine() calls FFTW plan creation code, 
	// which is non-reentrant. It should really be doing the 
	// locking tht we are doing here, but since it is 
	// not our code...
	_momentsEngineMutex.lock();
	_momentsEngine = new MomentsEngine(_params);
	_momentsEngineMutex.unlock();

	while (1) {
		_pulseQueueMutex.lock();
		_queueWait.wait(&_pulseQueueMutex);
		while (_pulseQueue.size() > 0) {
			std::pair<CP2FullPulse*, CP2FullPulse*> pp = _pulseQueue[0];
			CP2FullPulse* p1 = pp.first;
			CP2FullPulse* p2 = pp.second;
			// cross polar data, if available, appears in p2. Otherwise
			// p2 is zero. The MomentsEngine detects the presence of
			// cross data in the same way, hence the selection of the following
			// two calls.
			if (p2) {
			_momentsEngine->processPulse(
				p1->data(),
				p2->data(),
				p1->header()->gates,
				p1->header()->prt,
				p1->header()->pulse_num,
				p1->header()->el,
				p1->header()->az,
				p1->header()->pulse_num,
				p1->header()->horiz);
			} else {
			_momentsEngine->processPulse(
				p1->data(),
				0,
				p1->header()->gates,
				p1->header()->prt,
				p1->header()->pulse_num,
				p1->header()->el,
				p1->header()->az,
				p1->header()->pulse_num,
				p1->header()->horiz);
			}
			_pulseQueue.erase(_pulseQueue.begin());
			delete p1;
			if (p2) {
				delete p2;
			}
			Beam* pBeam = _momentsEngine->getNewBeam();
			if (pBeam)
			{
				_beamQueueMutex.lock();
				_beamQueue.push_back(pBeam);
				_beamQueueMutex.unlock();
			}

		}
		_pulseQueueMutex.unlock();
	}
}

///////////////////////////////////////////////////////////////////////////////////
void 
MomentThread::processPulse(CP2FullPulse* pPulse1, CP2FullPulse* pPulse2)
{
	_pulseQueueMutex.lock();
	_pulseQueue.push_back(std::pair<CP2FullPulse*, CP2FullPulse*>(pPulse1, pPulse2));	
	_pulseQueueMutex.unlock();
	_queueWait.wakeAll();
}


///////////////////////////////////////////////////////////////////////////////////
Beam*
MomentThread::getNewBeam()
{	
	Beam* pBeam = 0;
	_beamQueueMutex.lock();
	if (_beamQueue.size() > 0) {
		pBeam = _beamQueue[0];
		_beamQueue.erase(_beamQueue.begin());
	}
	_beamQueueMutex.unlock();
	return pBeam;
}
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////

