#ifndef CP2EXECTHREADH_
#define CP2EXECTHREADH_

#include <string>
#include <qthread.h>
#include <QUdpSocket>

#include "CP2PIRAQ.h"
#include "CP2Net.h"
#include "CP2UdpSocket.h"
#include "CP2Config.h"
#include "PciTimer.h"
#include "SimAngles.h"

using namespace CP2Lib;

class CP2ExecThread: public QThread {

public:
	/// The current state of CP2Exec
	enum STATUS {
		STARTUP,   ///< Starting
		PIRAQINIT, ///< Initializing hardware
		RUNNING    ///< Piraqs running and data being transmitted to the network
	};
	/// Contructor
	CP2ExecThread(
		std::string dspObjfile ///< The path to the dsp object file
		);
	/// Destructor
	virtual ~CP2ExecThread();
	/// Run the CP2ExecThread
	void run();
	/// stop the CP2ExecThread
	/// @todo This currently does not work correctly. We want to be able to stop and
	/// restart the piraqs without restarting the whole CP2exec application; this
	/// function is supposed to allow us to do that. But the piraqs don't restart 
	/// properly. It may relate to the one time timer initialization.
	void stop();
	/// @returns The status of the CP2execThread
	STATUS status();
	/// Get the pulse number errors for each piraq.
	void pnErrors(int& errors1, int& errors2, int& errors3);
	/// Get the total number of pulses processed by each piraq.
	void pulses(int& pulses1, int& pulses2, int& pulses);
	/// Get the current pulse throughput rate for each piraq.
	void rates(double& rate1, double& rate2, double& rate3);
	/// Get the eof flag for each piraq. Calling this cause the eof flags to be cleared.
	void eof(bool eof[3]);
	/// Get the current antenna information for each piraq.
	void antennaInfo(
		double* az,				///< Azimuth, in degrees magnetic.
		double* el,				///< Elevation, in degress above horizontal.
		unsigned int* sweep,	///< Sweep number
		unsigned int* volume	///< Volume number
		);	

protected:
	/// Initialize the pulse output socket
	void initSocket();
	/// Get the simulated angles information from the configuration.
	/// Will also set the _doSimAngles flag.
	SimAngles* createSimAngles();
	/// Collect error messages for all three piraqs.
	/// @returns Collect error mesages from the piraqs. If
	/// none, will be an empty string.
	std::string getPiraqErrors();
	/// The piraq dsp's will read the antenna pointing information
	// directly across the pci bus from the PMAC. They need the 
	/// PCI physical address of the PMAC dual ported ram.
	/// @return The pci physical address of the PMAC dual ported ram.
	unsigned int findPMACdpram();
	/// Configure the PCI timer to generate the PRF and
	/// H/V switch control signals.
	void configurePciTimer(PciTimer& pciTimer);
	/// The configuration for CP2Exec
	CP2Config _config;
	/// The S band piraq
	CP2PIRAQ* _piraq0;
	/// The Xh piraq
	CP2PIRAQ* _piraq1;
	/// The Xv piraq
	CP2PIRAQ* _piraq2;
	/// The dsp object code file name
	std::string _dspObjFile;
	/// The configuration file name
	std::string _configFile;
	/// The number of pulses per PCI transfer. 
	/// This sized so that each PCI transfer is less than 64KB, 
	///which is the size of the burst FIFO on the piraq which
	/// feeds the PCI transfer. 
	unsigned int _pulsesPerPciXfer;
	/// The output socket
	CP2UdpSocket* _pPulseSocket;
	/// The destination datagram port.
	int _pulsePort;
	/// The destination network.
	QHostAddress _hostAddr;
	/// The current status
	STATUS _status;
	/// Set true to ask the thread to stop polling the piraqs. 
	/// Currently does not work correctly (see todo above)
	bool _stop;
	/// The cumulative number of pulses from piraq1
	int _pulses1;
	/// The cumulative number of pulses from piraq2
	int _pulses2;
	/// The cumulative number of pulses from piraq3
	int _pulses3;
	// will be set true if an EOF is detected by the
	// CP2PIRAQ since the last time that CP2PIRAQ was
	// queried. (when queried, CP2PIRAQ returns the
	// current state, and clears it's internal flag)
	bool _eofFlags[3];
	/// Set true if angles are to be simulated
	bool _doSimAngles;
	SimAngles* _simAngles;

};



#endif
