#include "PciTimer.h"
#include <time.h>

using namespace CP2Lib;

/////////////////////////////////////////////////////
PciTimer::PciTimer(float systemClock,
				   float refFrequency,
				   float phaseFrequency,
				   int timingMode):
_error(false),
_systemClock(systemClock),
_reffreq(refFrequency),
_phasefreq(phaseFrequency),
_timingMode(timingMode)
{
	// initialize bpulses
	for (int i = 0; i < 6; i++) {
		setBpulse(i, 0, 0);
	}

	// initialize the TvicHW32 system
	init_pci(); 

	// find the PCI timer card, based on it's PCI identification.
	int boardnumber = 0;
	_pcicard = find_pci_card(PCITIMER_VENDOR_ID,
		PCITIMER_DEVICE_ID,
		boardnumber);

	if (!_pcicard) {
		_error = true;
		return;
	}
	// Get the PCI phys2 address.
	_reg_base = _pcicard->phys2;

	// Map the card's memory
	_base = (char *)pci_card_membase(_pcicard,256);

	// initialize the PCI registers for proper board operation 
	out32(_reg_base + 0x60, 0xA2A2A2A2);  /* pass through register */

	// Set the _sync flag, based on the timing mode.
	/// @todo  What is the sync flag? We don't seem to get anything out unless
	/// it is set to one. Why?
	_sync.hilo = 1;  

	/// @todo _seqdelay needs to be set to something appropriate. What is it? It 
	/// seems that things don't work unless it is some unknown positive value.
	_seqdelay.hilo = 10;

	// stop the card, to be ready for the start()
	stop();
}

/////////////////////////////////////////////////////
PciTimer::~PciTimer() {
	// stop the timer
	stop();
}

/////////////////////////////////////////////////////
void 
PciTimer::setBpulse(int index, unsigned short width, unsigned short delay) 
{
	_bpulse[index].width.hilo = width;

	// enforce constraints
	if (width == 0) {
		_bpulse[index].delay.hilo = 0;
	} else {
		if (delay < 1) {
			_bpulse[index].delay.hilo = 1;
		} else {
			_bpulse[index].delay.hilo = delay;
		}
	}
}

/////////////////////////////////////////////////////
void
PciTimer::addSeqCounts(int length, 
					  unsigned char pulseMask,
					  int polarization,
					  int phase) 
{
	// Silently ignore request to add too many sequences.
	if (_sequences.size() == MAXSEQUENCE)
		return;

	struct Sequence s;
	s.length.hilo = length;
	s.bpulseMask = pulseMask;
	s.polarization = polarization;
	s.phase = phase;
	_sequences.push_back(s);
}
/////////////////////////////////////////////////////
void
PciTimer::addSeqTime(float tt, 
				 unsigned char pulseMask,
				 int polarization,
				 int phase) 
{
	// Silently ignore request to add too many sequences.
	if (_sequences.size() == MAXSEQUENCE)
		return;

	struct Sequence s;
	int countFreq = (int)_systemClock/8;
	s.length.hilo = (unsigned short)(tt  * countFreq + 0.5);
	s.bpulseMask = pulseMask;
	s.polarization = polarization;
	s.phase = phase;
	_sequences.push_back(s);
}

/////////////////////////////////////////////////////
void 
PciTimer::stop()
{
	commandTimer(TIMER_STOP);
}

/////////////////////////////////////////////////////
/* open the timer board (once) and configure it with the timer structure */
void 
PciTimer::configure()
{
	// stop the timer.
	stop();

	int			a,r,n;
	TIMERHIMEDLO		ref,freq;

	/* clear the dual port ram on the timer */
	for( int i = 0; i < 0xC0; i++)
		_base[i] = 0; 

	/* fill in the dual port ram on the timer */

	// Load the sequence specifications.
	for(unsigned int i = 0; i < _sequences.size(); i++)
	{
		_base[(0x00 + i)] = _sequences[i].length.byte.lo;
		_base[(0x10 + i)] = _sequences[i].length.byte.hi;
		_base[(0x20 + i)] = _sequences[i].bpulseMask ^ 0x3f;
		_base[(0x30 + i)] = _sequences[i].polarization; 
		_base[(0x40 + i)] = _sequences[i].phase;
	}

	// load the bpulse specifications.
	for(int i = 0; i < 6; i++)
	{ 
		_base[(0xC0 + i * 4 + 0) ] = _bpulse[i].delay.byte.lo;
		_base[(0xC0 + i * 4 + 1) ] = _bpulse[i].delay.byte.hi;
		_base[(0xC0 + i * 4 + 2) ] = _bpulse[i].width.byte.lo;
		_base[(0xC0 + i * 4 + 3) ] = _bpulse[i].width.byte.hi;
	}

	// Set the phase lock loop specs.
	n = (int)(0.5 + _systemClock / _phasefreq);
	a = n & 7;
	n /= 8;
	r = (int) (0.5 + _reffreq / _phasefreq);
	ref.himedlo = r << 2 | 0x100000;     					/* r data with reset bit */
	freq.himedlo = n << 7 | a << 2 | 0x100001;     		/* r data with reset bit */

	_base[(0xD8 + 0) ] = freq.byte.lo;				//freqlo;
	_base[(0xD8 + 1) ] = freq.byte.med;				//freqmed;
	_base[(0xD8 + 2) ] = freq.byte.hi;				//freqhi;
	_base[(0xD8 + 3) ] = ref.byte.lo;				//reflo;
	_base[(0xD8 + 4) ] = ref.byte.med;				//refmed;
	_base[(0xD8 + 5) ] = ref.byte.hi;				//refhi;
	_base[(0xD8 + 6) ] = 0;					//div;
	_base[(0xD8 + 7) ] = 0;					//spare;

	_base[(0xD8 +  8) ] = _sync.byte.lo;		// synclo;
	_base[(0xD8 +  9) ] = _sync.byte.hi;		// synchi;
	_base[(0xD8 + 10) ] = _seqdelay.byte.lo;	// sequence delay lo
	_base[(0xD8 + 11) ] = _seqdelay.byte.hi;	// sequence delay hi
	_base[(0xD8 + 12) ] = (char)(_sequences.size());		// sequence length

	_base[(0xD8 + 13) ] = 0;					// current sequence count
	_base[(0xD8 + 14) ] = _timingMode;	// timing mode

	// tell the timer to reconfigure.
	commandTimer(TIMER_RESET);
}
/////////////////////////////////////////////////////
void 
PciTimer::start()
{
	// configure the timer card
	configure();

	commandTimer(TIMER_START);
}

/////////////////////////////////////////////////////
bool
PciTimer::error()
{
	return _error;
}
/////////////////////////////////////////////////////
/* poll the command response byte and handle timeouts */
int	
PciTimer::commandTimer(unsigned char cmd)
{
	// set request command
	_base[0xFE] = cmd; 
	// set request flag
	_base[0xFF] = TIMER_RQST;	

	time_t	first;
	/* wait until the timer comes ready */
	first = time(NULL) + 3;   /* wait up to 3 seconds */
	while((_base[0xFF] & 1)) 
	{
		Sleep(1);
		if(time(NULL) > first)
		{
			printf("Timeout waiting for PCITIMER to respond to request\n");
			exit(0);
		}
	}
	_base[0xFF] = 0x02; /* @@@ was base[0xFF * 4] */
	return(0);   
}

void
PciTimer::dump() {
	// print the registers from the PCI card.
	printf("PCI Configuration registers as seen from timer_read\n");
	for(int j = 0; j < 8; j++) {
		printf("%02X ",4*4*j);
		for(int i = 0; i < 4; i++) {
			unsigned int val = pci_read_config32(_pcicard,4*(i+4*j));  /* read 32 bit value at requested register */
			printf("%08lX ", val);
		}
		printf("\n");
	}
	printf("\nI/O Base: %8X\n",_reg_base);
	for(int j = 0; j < 8; j++) {
		printf("%02X ",4*4*j);
		for(int i = 0; i < 4; i++)
			printf("%08lX ",in32(_reg_base + 4*(i+4*j)));
		printf("\n");
	}

	printf("\nDPR base: %8X\n", _base);
	for(int j = 0; j < 16; j++) {
		printf("%02X ",j);
		for(int i = 0; i < 16; i++)
			printf("%02lX ",(unsigned char)_base[j*16+i]);
		printf("\n");
	}
}
