#ifndef MOMENTTHREADH_
#define MOMENTTHREADH_

#include <qthread.h>
#include <qmutex.h>
#include <qwaitcondition.h>
#include <vector>

// CP2 timeseries network transfer protocol.
#include "CP2Net.h"
using namespace CP2Net;

// Clases used in the moments computatons:
#include "MomentsEngine.hh"
#include "MomentsMgr.hh"
#include "Params.hh"
#include "Pulse.hh"
#include "Params.hh"

/// Pulses are delivered via processPulse(). MomentThread will
/// delete the pulses when it is finished with them.
/// New beams are retrieved using getNewBeam(). The caller
/// has responsibility for deleting the beam when finished
/// with it.
class MomentThread :
	public QThread
{
public:
	MomentThread(Params params);
	virtual ~MomentThread(void);
	virtual void run();
	void processPulse(CP2FullPulse* pHPulse, CP2FullPulse* pVPulse);
	Beam* getNewBeam();

protected:
	/// Pointers to pulses are queued here. If the moments calculation
	/// does not receive coplaner and cross oulses at the same time,
	/// then the second pointer will be null. For a moments scheme which
	/// receives coplaner and cross pulses at the same time step,
	/// the second pulse will will be the cross pulse.
	/// pulses are queued via processPulse(), and removed in the run() loop.
	std::vector<std::pair<CP2FullPulse*, CP2FullPulse*> > _pulseQueue;

	/// Beam results are queued here. Beams are queued by the run() loop,
	/// and removed by getNewBeam().
	std::vector<Beam*> _beamQueue;
	
	/// This mutex synchronizes access to the _pulseQueue.
	QMutex _pulseQueueMutex;
	
	/// This mutex synchronizes access to the _beamQueue.
	QMutex _beamQueueMutex;
	
	/// This is used to signal the thread that there is new data in _pulseQueue.
	QWaitCondition _queueWait;

	/// The compute engine for the moments
	MomentsEngine* _momentsEngine;

	/// The parameters for the moments compute engine.
	Params _params;


};

#endif

