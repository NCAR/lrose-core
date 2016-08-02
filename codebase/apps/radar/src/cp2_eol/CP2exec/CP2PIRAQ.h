#ifndef CP2PIRAQINC_
#define CP2PIRAQINC_


#include "Piraq.h"
#include "piraqComm.h"
#include "CP2Net.h"
#include "CP2UdpSocket.h"
#include "SimAngles.h"
#include "CP2Config.h"
#include <fstream>

#define C       2.99792458E8

/* the bits of the status register */
#define STAT0_SW_RESET    0x0001
#define STAT0_TRESET      0x0002
#define STAT0_TMODE       0x0004
#define STAT0_DELAY_SIGN  0x0008
#define STAT0_EVEN_TRIG   0x0010
#define STAT0_ODD_TRIG    0x0020
#define STAT0_EVEN_TP     0x0040
#define STAT0_ODD_TP      0x0080
#define STAT0_PCLED       0x0100
#define STAT0_GLEN0       0x0200
#define STAT0_GLEN1       0x0400
#define STAT0_GLEN2       0x0800
#define STAT0_GLEN3       0x1000

#define STAT1_PLL_CLOCK   0x0001
#define STAT1_PLL_LE      0x0002
#define STAT1_PLL_DATA    0x0004
#define STAT1_WATCHDOG    0x0008
#define STAT1_PCI_INT     0x0010
#define STAT1_EXTTRIGEN   0x0020
#define STAT1_FIRST_TRIG  0x0040
#define STAT1_PHASELOCK   0x0080
#define STAT1_SPARE0      0x0100
#define STAT1_SPARE1      0x0200
#define STAT1_SPARE2      0x0400
#define STAT1_SPARE3      0x0800

/* macro's for writing to the status register */
#define STATUS0(card,a)        (card->GetControl()->SetValue_StatusRegister0(a)) ///< *card->status0 = (a)
#define STATUSRD0(card,a)      (card->GetControl()->GetBit_StatusRegister0(a)) ///< (*card->status0 & (a))
#define STATSET0(card,a)       (card->GetControl()->SetBit_StatusRegister0(a))  ///< *card->status0 |= (a)
#define STATCLR0(card,a)       (card->GetControl()->UnSetBit_StatusRegister0(a)) ///< *card->status0 &= ~(a)
#define STATTOG0(card,a)       (if(STATUSRD0(card,a)) { STATCLR0(card,a);}else{STATSET0(card,a);}) ///< *card->status0 ^= (a)
#define STATUS1(card,a)        (card->GetControl()->SetValue_StatusRegister1(a)) ///< *card->status0 = (a)
#define STATUSRD1(card,a)      (card->GetControl()->GetBit_StatusRegister1(a)) ///< (*card->status0 & (a))
#define STATSET1(card,a)       (card->GetControl()->SetBit_StatusRegister1(a))  ///< *card->status0 |= (a)
#define STATCLR1(card,a)       (card->GetControl()->UnSetBit_StatusRegister1(a)) ///< *card->status0 &= ~(a)
#define STATTOG1(card,a)       if(STATUSRD0(card,a)) { STATCLR1(card,a);}else{STATSET1(card,a);} ///< *card->status0 ^= (a)
#define STATPLL(card,a)        STATUS1(card,STATUSRD1(card,0xFFF8) |  (a))

using namespace CP2Net;

///
///\brief
///Representation of a Piraq card as it is utilized for CP2.
///
///Write detailed description for CP2PIRAQ here.
///
///\remarks
/// This code was pulled, kicking and screaming, out of the morass
/// of legacy radar processor code that came from the DOW radars, and 
/// who knows what else.
///
///\see
///Separate items with the '|' character.
///
class CP2PIRAQ: public PIRAQ {

public:

	/// Constructor
	CP2PIRAQ( 
		CP2UdpSocket* pPulseSocket, ///< The socket that pulse data will be broadcast on
		std::string configOrg,		///< The organization name, used to identify the configuration
		std::string configApp,		///< The application name, used to identify the configuration
		char* dspObjFnamefloat,
		unsigned int pulsesPerPciXfer, ///< The number of pulses to include in each PCI transfer
		unsigned int pmacDpramBusAddr, ///< The physical PCI address of the PMAC DPRAM 
		int boardnum,				   ///< The piraq board number; used to stagger the PCI transfers.
		RCVRTYPE rcvrType,			   ///< Identifies the card as Sban, Xh or Xv.
		bool doSimAngles,			   ///< If true, simulate angles rather than read from the PMAC
		SimAngles simAngles,		   ///< The angle simulation engine.
		int system_clock		       ///< The system clock frequency, which timing parameter counts are based on.
		);

	/// Destructor
	~CP2PIRAQ();

	/// Signal the Piraq to start taking data and transmitting
	/// to the host. The data flow on the Piraq is driven by the 
	/// A/D fifo getting filled, which will not happen until
	/// the sample clock is running. Thus to coordinate Piraqs,
	/// the start() method is called for all of them, and
	/// then the sample clock is started.
	/// @param firstPulseNum The pulse number to assign to the 
	/// first pulse. Consecutive pulses have incremented
	/// pulse numbers.
	int start(long long firstPulseNum);
	/// Stop the Piraq data sampling.
	void stop();
	/// Poll the Piraq DMA buffer. If data is found,
	/// repackaged it and transmit as datagrams on the network.
	int poll();
	/// @return The configuration that is passed to the Piraq.
	/// @todo Users should not really be querying the Piraq for 
	/// this information, since they configured it in the first
	/// place. Eventually remove this, as it is inappropriate.
	PINFOHEADER info();
	/// @return The number of detected pulse number errors. This
	/// value increments ever time two consecutive pulses do not have
	/// consecutive pulse numbers. It is not the number of dropped
	/// pulses.
	int pnErrors();
	/// @return The estimated sample rate, in pulses per second.
	/// There will be some jitter in this value, since the measurment 
	/// is made in between polls.
	double sampleRate();
	/// Return information from the antennna control system that is 
	/// captured by the piraq.
	/// @param az The azimuth, in countes, is returned here.
	/// @param el The elevation, in counts, is returned here
	/// @param sweep The sweep number, an arbitrary value, is returned here
	/// @param volume The volume number, an arbitrary value, is returned here.
	void antennaInfo(double& az, double& el, 
		unsigned int& sweep, unsigned int& volume);
	/// @return The current eof indicator flag. 
	/// The flag is cleared when this function is called.
	bool eof();
	/// @return True if an error has been detected during Piraq initialization.
	/// Usually this is detected by checking to see if the Piraq error message is
	/// non-null, since that class doesn't seem to have an error flag as such.
	bool error();

protected:
	/// Initialize the piraq card, loading the DSP with a program.
	int init(char* dspObjFname);
	/// output a stream of serial data to the PLL 
	///----------------------
	/// PLL bits definition  
	///----------------------
	///  2   |   1   |   0   
	///------+-------+-------
	/// DATA |  LE   | CLOCK 
	///----------------------
	/// @param data The value to be sent to the pll chip.
	void plldata(int data);
	/// program the phase locked loop
	/// @param ref
	/// @param freq
	/// @param cmpfreq
	void pll(double ref, double freq, double cmpfreq);
	/// Program the registers for one section of a three section timer timer.
	/// @param section The timer section of this timer block (0, 1, 2)
	/// @param mode The mode to set the timer to
	/// @param count The timer count
	/// @iobase The virtual address of the timer block.
	void timerRegisterSet(int section, int mode, int count, unsigned short *iobase);
	///
	/// program the timer for different modes.
	/// The following comments were taken out of some legacy code.
	/// They don't seem to be completely accurate.
	///
	/// mode:           0 = continuous mode, 1 = trigger mode, 2 = sync mode
	/// gate0mode:      0 = regular gate 0, 1 = expanded gate 0
	/// gatecounts:     gate spacing in 10 MHz counts
	/// num bergates:    number of range gates
	/// timecounts:     modes 1 and 2: delay (in 10 MHz counts) after trigger
	/// mode 0: pulse repetition interval in 10 MHz counts
	///
	/// For sync mode (mode 0), delaycounts defines the sync time.  The prt
	/// delay must be reprogrammed after the FIRST signal is detected.  
	///
	/// OTHER RESULTS:
	/// 1)  the timing state machine is reset
	///
	/// this routine leaves the timer in a known state 
	/// the timer is assumed stopped on entry (important!) 
	int timerset();
	/// make sure the timer is stopped and no fifo data is pending 
	void stop_piraq();
	/// The FIFO that is used for data transfering from piraq to host
	CircularBuffer*   _pFifo;
	/// This will be the first packet in the fifo. It appears that
	/// the piraq may read this structure?
	PPACKET* _pConfigPacket;
	/// The last pulse number received. Used to detect
	/// dropped pulses.
	long long _lastPulseNumber; 
	/// Cumulative pulse number errors
	int _PNerrors;
	/// current azimuth
	double _az;
	/// current elevation
	double _el; 
	/// current volume
	uint _volume;
	/// current sweep
	unsigned int _sweep;
	/// the number of bytes per gate
	int _bytespergate;
	/// the number of hits in each block transfer
	/// from the piraq.
	unsigned int _pulsesPerPciXfer;
	/// outgoing socket
	CP2UdpSocket* _pPulseSocket;
	/// The pci bus address for the PMAC dpram; this
	/// is passed to the piraq so that it can read
	/// az and el angles directly from the PMAC.
	unsigned int _pmacDpramAddr;
	/// A packet that can be succesive filled with the
	/// blocked pulse data as it is read out of the PCI
	/// circular buffer.
	CP2Packet _cp2Packet;
	/// The last saved system tick count (milliseconds) 
	int _lastTickCount;
	/// The _eof flag is set when an EOF is discovered in
	/// a pulse. It is cleared when the eof() function is called.
	bool _eof;
	/// count pulses, for diagnostic purposes. this will
	/// be removed in production software
	long long _nPulses;
	/// The prt in seconds.
	double _prt;				
	/// prt2 in who knows.
	int _prt2;   
	/// The Piraq card timing mode.
	/// 0: software free running mode
	/// 1: external trigger mode
	/// 2: external sync free running mode
	int _timing_mode;
	/// The number of gates.
	int _gates;
	/// The transmit pulse width in seconds.
	double _xmit_pulsewidth;	
	/// The recieve pulse width in seconds.
	double _rcvr_pulsewidth;	
	unsigned int _totalHits;
	/// The Piraq board number (0,1, 2).
	int _boardnum;
	/// Tell the Piraq what type of recevier it is connected to.
	/// This afffects the setting of the polarization flag.
	RCVRTYPE _rcvrType;
	/// The calculated sample rate, in pulses per second.
	double _sampleRate;
	/// The number of datagram resend attempts that have been made.
	/// These are done wheh the datagram write returns an indication
	/// that it failed.
	int _resendCount;
	/// Send an opaque datagram.
	/// @param size The size, in bytes.
	/// @param data The data buffer.
	int sendData(int size, void* data);
	/// Set true if angles are to be simulated
	bool _doSimAngles;
	/// Angle generator for simulated angles
	SimAngles _simAngles;
	/// Configuration
	CP2Config _config;
	/// enable/disable debugging
	bool _debug;
	/// debugging output file
	std::ofstream _debugFile;
	/// The system clock in Hz. 
	int _system_clock;


};

#endif
