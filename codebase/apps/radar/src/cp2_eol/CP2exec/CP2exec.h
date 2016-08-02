/// @page cp2exec-overview The CP2Exec Program
///
/// CP2Exec is the primary CP2 radar data acquisition program.
/// It manages operation of the Piraq III digital receivers,
/// collecting pulse data from each card, packaging these into a
/// CP2 defined network structure, and broadcasting them to the 
/// network. Configuration is managed by CP2Exec.ini. CP2Exec also
/// controls the PCI Timer card, which is used to generate basic 
/// system timing.
///
/// The CP2Exec main program provides the user interface 
/// functionality. In order for the
/// user interface to remain responsive during data collection and processing,
/// these activities are implmented and executed in a separate thread.
/// A timer in the main (user interface) thread periodically queries the processing
/// thread for status information and displays this on the user interface. 
/// The interface thread also sets a stop flag in the processing thread
/// when the user interface has been closed, signalling the thread to 
/// shutdown gracefully.
///
/// The processing thread manages the Piraq receivers and the PCI timer.
/// The general program flow is as follows:
/// - Initialize the network subsystem.
/// - Initialize and stop the PCI timer. It is configured to generate a
///   PRF pulse that will synchronize each of the timer cards. It also 
///   generates two timing signals for controling the H/V switch, and another
///   pulse for firing the X band transmitter.
/// - For each Piraq card:
///   - Reset the card.
///   - Load the dsp program into the card.
///   - Start the dsp program running on the card. From now on, the 
///     dsp program will sample data every time it receives a synchronization
///     pulse from the PCI timer. At this pooint the timer is stopped, and 
///     so all Piraq cads are poised and ready to read data at the same time,
///     when they receive the first synchronization signal.
/// - Start the PCI timer, which will generate the PRF pulses and start the 
///   sampling on the Piraq cards.
/// - During sampling, each Piraq card is placing I/Q pulse data into 
///   circular buffers in the host memory. The Piraq processing thread 
///   polls each Piraq circular buffer in succession, pulling all 
///   available pulse data out of the buffer when it is available.
///   The pulse data is broadcast on the network as soon as it has
///   been copied from the circular buffer. A 1 ms sleep is performed
///   before the polling of each succesive Piraq circular buffer. As inocuous
///   as it looks in the source code, this sleep is what prevents the Piraq
///   polling loop from using 100% CPU. In fact, CP2Exec uses a very small 
///   amont of CPU, in spite of transferring about 25 MB/s from the Piraq
///   circular buffers to the network interface.
///
/// <h3>PCI Timer Bpulse Definitions</h3>
/// - bpulse(0) - Gate0 pulse to the piraq cards (t0)
/// - bpulse(1) - H/V switch strobe (t0 - 22uS)
/// - bpulse(2) - H/V switch select (t0 - 22uS) (alternates each PRF)
/// - bpulse(3) - PRF pulse for Xband (t0 - ? uS)
/// - bpulse(4) - PRF pulse to CP2 timing generator (t0 - 24uS)
///
/// <h3>Main Classes</h3>
/// - CP2Exec: The main thread, for user interface activies.
/// - CP2ExecThread: The receiver processing thread. It confgures and
///   controls the Piraqs and PCI tmer.
/// - CP2PIRAQ: Derived from PIRAQ, this class models a PIRAQ card.
/// - PciTimer: this class models the PCI timer card.
/// - SimAngles: Successive calls to the SimAngles::nextAngle() will return
///   simluated antenna angles, which are useful for system testing when
///   there is not an actual antenna providing angles to the Piraqs.
/// - CP2Pulse: Used to encapsulate pulse data for network transmission. A
///   CP2PulseHeader is held within this class, and carries ancillary 
///   data, such as antenna angles, pulse numbers and so on. The serialized bytes
///   from a CP2Pulse can be sent as a network datagram. A consumer then reads these
///   bytes back into a CP2Pulse in order to reconstitute the pulse as a CP2Pulse.
///

#ifndef CP2EXECH_
#define CP2EXECH_


#include "ui_CP2Exec.h"
#include "CP2PIRAQ.h"
#include "CP2ExecThread.h"
#include <QPalette>
#include <QDateTime>

/// CP2Exec is the main Qt dialog that
/// manages the CP2 Piraq data acquisition
/// subsystem. It also presents status
/// information about the piraqs and
/// data throughput.
///
/// In order for the GUI and the piraq
/// data activity to run concurrently,
/// the CP2ExecThread is used to manage
/// the piraqs in a separate thread. 
/// CP2Exec calls status functions
/// in CP2ExecThread to query the state
/// of that thread. 
class CP2Exec: public QDialog, public Ui::CP2Exec 
{
	Q_OBJECT
public:
	CP2Exec(QDialog* parent);
	virtual ~CP2Exec();

public slots:

protected:
	/// The piraq management thread
	CP2ExecThread* _pThread;
	/// The file containing the dsp object code
	std::string _dspObjFile;
	/// The configuration file
	std::string _configFile;
	// The builtin timer will be used to display statistics.
	void timerEvent(QTimerEvent*);
	/// The update interval (s) for the status and 
	/// statistics reporting
	int _statsUpdateInterval;
	/// The timer will start out at a fast rate, but
	/// once the status is RUNNING, then it will update
	/// less frequently.
	bool _timerReset;
	/// The id of the stats timer.
	int _timerId;
	/// EOF for piraq 0 flag tracks the state of the EOF widget,
	/// so that we don't keep setting it if it is 
	/// already set.
	bool _eofLed0;
	/// EOF for piraq 1 flag tracks the state of the EOF widget,
	/// so that we don't keep setting it if it is 
	/// already set.
	bool _eofLed1;
	/// EOF for piraq 2 flag tracks the state of the EOF widget,
	/// so that we don't keep setting it if it is 
	/// already set.
	bool _eofLed2;
	/// Palette for making the leds green
	QPalette _greenPalette;
	/// Platette for making the leds red
	QPalette _redPalette;
	/// Preserve the time tht we started, so that we can display an uptime
	QDateTime _startTime;

};

#endif
