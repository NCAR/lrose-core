#include <QNetworkInterface>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QMessageBox>
#include <QIcon>
#include <QPixmap>

#include "CP2Moments.h"
#include "CP2Net.h"
#include "CP2Version.h"
using namespace CP2Lib;

#ifdef WIN32
#include <Windows.h>  // just to get Sleep()
#else
#define Sleep(x) usleep(x*1000)
#endif

#include <iostream>

/* XPM */

static char *icon[] = {
	/* width height ncolors chars_per_pixel */
	"32 32 12 1",
	/* colors */
	"` c #CBFFFF",
	"a c #AB0000",
	"c c #326565",
	"e c red",
	"g c green",
	"h c #770000",
	"i c #99CCCC",
	"j c #00AB00",
	"l c #007700",
	"m c #005500",
	"q c #DC0000",
	"v c black",
	/* pixels */
	"```````````````````````````````i",
	"`iiiiiiiiiiiiiiiiiiiiiiiiiiiiiic",
	"`iiiiiiiiiiiiiiiiiiiiiiiiiiiiiic",
	"`iiiiiiiiiiiiiiiiiiiviiiiiiiiiic",
	"`iiiiiiiiiiiiiiiiiivlviiiiiiiiic",
	"`iiiiiiiiiiiiiiiiivllmviiiiiiiic",
	"`iiiiiiiiiiiiiiiiiivlmmviiiiiiic",
	"`iiiiiiiiiiiiiiiiiiivmmmviiiiiic",
	"`iiivgvvvgvvvvvvvvvvvvmmmviiiiic",
	"`iiiiggggggggjjjjllllmmmmvviiiic",
	"`iiiiivgvvvggjjjjllllmmmmvvviiic",
	"`iiiiiiggggggjjjjllllmmmmvviiiic",
	"`iiiiiiivgvvvjvvvvvvvvmmmviiiiic",
	"`iiiiiiiiiiiiiiiiiiivmmmviiiiiic",
	"`iiiiiiiiiiviiiiiiivlmmviiiiiiic",
	"`iiiiiiiiivqviiiiivllmviiiiiiiic",
	"`iiiiiiiivqqqviiiiivlviiiiiiiiic",
	"`iiiiiiivqqqviiiiiiiviiiiiiiiiic",
	"`iiiiiiveqqviiiiiiiiiiiiiiiiiiic",
	"`iiiiiveeqvvvvvvvvvvvvhvvvvviiic",
	"`iiiiveeeqqqqaaaahhhhhhhhhviiiic",
	"`iiiveeeeqqqqaaaahhhhvvvhviiiiic",
	"`iiiiveeeqqqqaaaahhhhhhhhiiiiiic",
	"`iiiiiveeqvvvvvvvvhvvvhviiiiiiic",
	"`iiiiiiveqqviiiiiiiiiiiiiiiiiiic",
	"`iiiiiiivqqqviiiiiiiiiiiiiiiiiic",
	"`iiiiiiiivqqqviiiiiiiiiiiiiiiiic",
	"`iiiiiiiiivqviiiiiiiiiiiiiiiiiic",
	"`iiiiiiiiiiviiiiiiiiiiiiiiiiiiic",
	"`iiiiiiiiiiiiiiiiiiiiiiiiiiiiiic",
	"`iiiiiiiiiiiiiiiiiiiiiiiiiiiiiic",
	"iccccccccccccccccccccccccccccccc"
};

/////////////////////////////////////////////////////////////////////////////

CP2Moments::CP2Moments(QDialog* parent): QDialog(parent),
_pPulseSocket(0),    
_pPulseSocketBuf(0),	
_collator(5000),
_statsUpdateInterval(5),
_processSband(true),
_processXband(true),
_config("NCAR", "CP2Moments"),	
_sGates(0),
_xGates(0),
_sBeamCount(0),
_xBeamCount(0),
_shBiQuad(0),
_svBiQuad(0),
_xhBiQuad(0),
_xvBiQuad(0)
{

	// initialize fftw for thread usage.
	fftw_init_threads();
	fftw_plan_with_nthreads(4);

	// setup our form
	setupUi(parent);
	this->setWindowIcon(QIcon(QPixmap(icon)));

	// get our title from the coniguration
	std::string title = _config.getString("Title","CP2Moments");
	title += " ";
	title += CP2Version::revision();
	parent->setWindowTitle(title.c_str());

	// initialize the statictics and error monitoring.
	for (int i = 0; i < 3; i++) {
		_pulseCount[i]	= 0;
		_prevPulseCount[i] = 0;
		_errorCount[i] = 0;
		_eof[i] = false;
		_lastPulseNum[i] = 0;
		_lastPulseAz[i] = 0;
	}

	// get the processing parameters
	// Do we apply the biquad?
	_doSbandBiQuad = _config.getBool("ProcessingSband/biQuadEnabled", false);
	// get the biquad filter parameters
	getBiQuadCoeffs();

	// create the Sband moments processing thread
	Params Sparams;

	Sparams.system_phidp                 = _config.getDouble("ProcessingSband/system_phidp_deg",        0.0);
	Sparams.correct_for_system_phidp     = _config.getBool("ProcessingSband/correct_phidp",             false);

	Sparams.moments_params.mode                   = Params::DUAL_CP2_SBAND;
	Sparams.moments_params.gate_spacing           = _config.getDouble("ProcessingSband/gateSpacingKm",     0.150);
	Sparams.moments_params.start_range            = Sparams.moments_params.gate_spacing/2.0;
	Sparams.moments_params.n_samples              = _config.getInt("ProcessingSband/pulsesPerBeam",          100);
	Sparams.moments_params.algorithm              = Params::ALG_PP;
	Sparams.moments_params.index_beams_in_azimuth = _config.getBool("ProcessingSband/indexBeamInAz",        true);
	Sparams.moments_params.azimuth_resolution     = _config.getDouble("ProcessingSband/azResolutionDeg",     1.0);
	Sparams.moments_params.apply_clutter_filter   = _config.getBool("ProcessingSband/clutterFilterEnable", false);
	if (Sparams.moments_params.apply_clutter_filter)
		Sparams.moments_params.window = Params::WINDOW_VONHANN;
	else
		Sparams.moments_params.window = Params::WINDOW_NONE;

	Sparams.radar.horiz_beam_width       = _config.getDouble("ProcessingSband/horizBeamWidthDeg",  0.91);
	Sparams.radar.vert_beam_width        = _config.getDouble("ProcessingSband/vertBeamWidthDeg",   0.91);
	Sparams.radar.xmit_rcv_mode          = Params::DP_ALT_HV_CO_ONLY;

	Sparams.hc_receiver.noise_dBm        = _config.getDouble("ProcessingSband/hc_rcvr_noise_dbm",      -77.0);
	Sparams.hc_receiver.gain             = _config.getDouble("ProcessingSband/hc_rcvr_gain_db",         37.0);
	Sparams.hc_receiver.radar_constant   = _config.getDouble("ProcessingSband/hc_rcvr_radar_constant", -68.4);

	Sparams.hx_receiver.noise_dBm        = Sparams.hc_receiver.noise_dBm;
	Sparams.hx_receiver.gain             = Sparams.hc_receiver.gain; 
	Sparams.hx_receiver.radar_constant   = Sparams.hx_receiver.radar_constant;

	Sparams.vc_receiver.noise_dBm        = _config.getDouble("ProcessingSband/vc_rcvr_noise_dbm",      -77.0);
	Sparams.vc_receiver.gain             = _config.getDouble("ProcessingSband/vc_rcvr_gain_db",         37.0);
	Sparams.vc_receiver.radar_constant   = _config.getDouble("ProcessingSband/vc_rcvr_radar_constant", -68.4);

	Sparams.vx_receiver.noise_dBm        = Sparams.vc_receiver.noise_dBm;
	Sparams.vx_receiver.gain             = Sparams.vc_receiver.gain;
	Sparams.vx_receiver.radar_constant   = Sparams.vc_receiver.radar_constant;

	_pSmomentThread = new MomentThread(Sparams);

	_doXbandBiQuad = _config.getBool("ProcessingXband/biQuadEnabled", false);
	// create the Sband moments processing thread
	Params Xparams;

	Xparams.system_phidp                          = _config.getDouble("ProcessingXband/system_phidp_deg", 0.0);
	Xparams.correct_for_system_phidp              = _config.getBool("ProcessingXband/correct_phidp",      false);

	Xparams.moments_params.mode                   = Params::DUAL_CP2_XBAND;
	Xparams.moments_params.gate_spacing           = _config.getDouble("ProcessingXband/gateSpacingKm", 0.150);
	Xparams.moments_params.start_range            = Xparams.moments_params.gate_spacing/2.0;
	Xparams.moments_params.n_samples              = _config.getInt("ProcessingXband/pulsesPerBeam", 100);
	Xparams.moments_params.algorithm              = Params::ALG_PP;
	Xparams.moments_params.index_beams_in_azimuth = _config.getBool("ProcessingXband/indexBeamInAz", true);
	Xparams.moments_params.azimuth_resolution     = _config.getDouble("ProcessingXband/azResolutionDeg", 1.0);
	Xparams.moments_params.apply_clutter_filter   = _config.getBool("ProcessingXband/clutterFilterEnable", false);
	if (Xparams.moments_params.apply_clutter_filter)
		Xparams.moments_params.window = Params::WINDOW_VONHANN;
	else
		Xparams.moments_params.window = Params::WINDOW_NONE;


	Xparams.radar.horiz_beam_width       = _config.getDouble("ProcessingXband/horizBeamWidthDeg", 0.91);
	Xparams.radar.vert_beam_width        = _config.getDouble("ProcessingXband/vertBeamWidthDeg", 0.91);
	Xparams.radar.xmit_rcv_mode          = Params::DP_H_ONLY_FIXED_HV;

	Xparams.hc_receiver.noise_dBm        = _config.getDouble("ProcessingXband/hc_rcvr_noise_dbm",      -77.0);
	Xparams.hc_receiver.gain             = _config.getDouble("ProcessingXband/hc_rcvr_gain_db",         37.0);
	Xparams.hc_receiver.radar_constant   = _config.getDouble("ProcessingXband/hc_rcvr_radar_constant", -68.4);

	Xparams.hx_receiver.noise_dBm        = Xparams.hc_receiver.noise_dBm; 
	Xparams.hx_receiver.gain             = Xparams.hc_receiver.gain; 
	Xparams.hx_receiver.radar_constant   = Xparams.hc_receiver.radar_constant; 

	Xparams.vx_receiver.noise_dBm        = _config.getDouble("ProcessingXband/vx_rcvr_noise_dbm",      -77.0);
	Xparams.vx_receiver.gain             = _config.getDouble("ProcessingXband/vx_rcvr_gain_db",         37.0);
	Xparams.vx_receiver.radar_constant   = _config.getDouble("ProcessingXband/vx_rcvr_radar_constant", -68.4);

	Xparams.vc_receiver.noise_dBm        = Xparams.vx_receiver.noise_dBm; 
	Xparams.vc_receiver.gain             = Xparams.vx_receiver.gain; 
	Xparams.vc_receiver.radar_constant   = Xparams.vc_receiver.radar_constant; 

	_pXmomentThread = new MomentThread(Xparams);

	// At the moment, set the gate spacing to the Sband gate spacing.
	// This really needs to be separated into X and S spacing, and accounted
	// for in CP2PPI.
	_gateSpacing = Sparams.moments_params.gate_spacing;

	// display a few of the processing parameters on the UI.
	_sPulsesPerBeam->setNum(Sparams.moments_params.n_samples);
	_sAzIndexed->setText(Sparams.moments_params.index_beams_in_azimuth ? "On":"Off");
	_sBiQuadCheck->setChecked(_doSbandBiQuad);
	_sClutterFilter->setText(Sparams.moments_params.apply_clutter_filter ? "On":"Off");

	_xPulsesPerBeam->setNum(Xparams.moments_params.n_samples);
	_xAzIndexed->setText(Xparams.moments_params.index_beams_in_azimuth ? "On":"Off");
	_xBiQuadCheck->setChecked(_doXbandBiQuad);
	_xClutterFilter->setText(Xparams.moments_params.apply_clutter_filter ? "On":"Off");

	// start the moments processing threads. They will wait
	// patiently until their processPulse() functions are
	// called with pulses to be processed.
	_pSmomentThread->start();
	_pXmomentThread->start();

	// intialize the data reception socket.
	// set up the ocket notifier and connect it
	// to the data reception slot
	initializeSockets();	

	// update the statistics
	timerEvent(0);

	// set the run state
	startStopSlot(false);

	// connect check boxes
	connect(_sBiQuadCheck, SIGNAL(stateChanged(int)), this, SLOT(sBandBiQuadEnable(int)));
	connect(_xBiQuadCheck, SIGNAL(stateChanged(int)), this, SLOT(xBandBiQuadEnable(int)));

	// start the statistics timer
	startTimer(_statsUpdateInterval*1000);

}
/////////////////////////////////////////////////////////////////////
CP2Moments::~CP2Moments()
{
}
/////////////////////////////////////////////////////////////////////
void
CP2Moments::startStopSlot(bool v)
{
	// When the button is pushed in, we are stopped
	_run = !v;
	// set the button text to the opposite of the
	// current state.
	if (!_run) {
		//		_startStopButton->setText("Start");
		_statusText->setText("Stopped");
	} else {
		//		_startStopButton->setText("Stop");
		_statusText->setText("Running");
	}
}
/////////////////////////////////////////////////////////////////////
void
CP2Moments::timerEvent(QTimerEvent*) 
{
	int rate[3];
	for (int i = 0; i < 3; i++) {
		rate[i] = (_pulseCount[i]-_prevPulseCount[i])/(float)_statsUpdateInterval;
		_prevPulseCount[i] = _pulseCount[i];
	}

	int collatorErrors = _collator.discards();

	_piraq0Errors->setNum(_errorCount[0]);
	_piraq1Errors->setNum(_errorCount[1]);
	_piraq2Errors->setNum(_errorCount[2]);

	_piraq0rate->setNum(rate[0]);
	_piraq1rate->setNum(rate[1]);
	_piraq2rate->setNum(rate[2]);

	_sBeams->setNum(_sBeamCount);
	_xBeams->setNum(_xBeamCount);

	_collatorErrors->setNum(collatorErrors);

}
//////////////////////////////////////////////////////////////////////
void
CP2Moments::initializeSockets()	
{
	// assign the incoming and outgoing port numbers.
	_pulsePort	    = _config.getInt("Network/PulsePort", 3100);
	_productsPort   = _config.getInt("Network/ProductPort", 3200);

	// get the network identifiers
	std::string pulseNetwork   = _config.getString("Network/PulseNetwork", "192.168.1");
	std::string productNetwork = _config.getString("Network/ProductNetwork", "192.168.1");

	// allocate the buffer that will recieve the incoming pulse data
	_pPulseSocketBuf = new char[1000000];

	// create the sockets
	_pPulseSocket   = new CP2UdpSocket(pulseNetwork, _pulsePort, false, 0, CP2MOMENTS_PULSE_RCVBUF);

	if (!_pPulseSocket->ok()) {
		std::string errMsg = _pPulseSocket->errorMsg().c_str();
		errMsg += "Products will not be computed";
		QMessageBox e;
		e.warning(this, "Error", errMsg.c_str(), 
			QMessageBox::Ok, QMessageBox::NoButton);
	}

	_pProductSocket = new CP2UdpSocket(productNetwork, _productsPort, true, CP2MOMENTS_PROD_SNDBUF, 0);
	if (!_pProductSocket->ok()) {
		std::string errMsg = _pProductSocket->errorMsg().c_str();
		errMsg += "Products will not be transmitted";
		QMessageBox e;
		e.warning(this, "Error", errMsg.c_str(), 
			QMessageBox::Ok, QMessageBox::NoButton);
	}

	// set the socket info displays
	_outIpText->setText(_pProductSocket->toString().c_str());
	_outPortText->setNum(_productsPort);

	_inIpText->setText(_pPulseSocket->toString().c_str());
	_inPortText->setNum(_pulsePort);


	// set up the socket notifier for pulse datagrams
	connect(_pPulseSocket, SIGNAL(readyRead()), this, SLOT(newPulseDataSlot()));

	// The max datagram message must be smaller than 64K
	_soMaxMsgSize = 64000;

}
//////////////////////////////////////////////////////////////////////
void 
CP2Moments::newPulseDataSlot()
{
	int	readBufLen = _pPulseSocket->readDatagram(
		(char *)_pPulseSocketBuf, 
		sizeof(short)*1000000);

	if (readBufLen > 0) {
		// put this datagram into a packet
		bool packetBad = _pulsePacket.setPulseData(
			readBufLen, 
			_pPulseSocketBuf);

		// Extract the pulses and process them.
		// Observe paranoia for validating packets and pulses.
		// From here on out, we are divorced from the
		// data transport.
		if (!packetBad) {
			for (int i = 0; i < _pulsePacket.numPulses(); i++) {
				CP2Pulse* pPulse = _pulsePacket.getPulse(i);
				if (pPulse) {
					// scale the I/Q counts to something. we will probably do 
					// this eventually in cp2exec.
					float* pData = pPulse->data;
					int gates = pPulse->header.gates;
					for (int i = 0; i < 2*gates; i++) {
						pData[i]   *= PIRAQ3D_SCALE;
					}
					int chan = pPulse->header.channel;
					if (chan >= 0 && chan < 3) {
						// check error flags, count pulses, etc.
						pulseBookKeeping(pPulse);
						// do all of the heavy lifting for this pulse,
						// but only if processing is enabled.
						if (_run) {
							// apply the biquad filter, if enabled
							applyBiQuad(pPulse);
							// compute moments and send out on network
							processPulse(pPulse);
						} 
					}
				}
			}
		}
	} else {
		// read error. What should we do here?
	}
}
//////////////////////////////////////////////////////////////////////
void
CP2Moments::getBiQuadCoeffs() {

	// these coefficients from an email by Eric Loew, ay 3, 2007.

	_sa11 = _config.getFloat("BiQuad/s_a11", -1.8879310269054523f);
	_sa12 = _config.getFloat("BiQuad/s_a12", 0.89293022305163017f);
	_sb10 = _config.getFloat("BiQuad/s_b10", 1.0f);
	_sb11 = _config.getFloat("BiQuad/s_b11", -1.9998230753949735f);
	_sb12 = _config.getFloat("BiQuad/s_b12", 1.0f);

	_sa21 = _config.getFloat("BiQuad/s_a21", -1.9765168993177902f);
	_sa22 = _config.getFloat("BiQuad/s_a22", 0.97954329118391725f);
	_sb20 = _config.getFloat("BiQuad/s_b20", 1.0f);
	_sb21 = _config.getFloat("BiQuad/s_b21", -1.9990748878268811f);
	_sb22 = _config.getFloat("BiQuad/s_b22", 1.0f);

	_xa11 = _config.getFloat("BiQuad/x_a11", -1.8879310269054523f);
	_xa12 = _config.getFloat("BiQuad/x_a12", 0.89293022305163017f);
	_xb10 = _config.getFloat("BiQuad/x_b10", 1.0f);
	_xb11 = _config.getFloat("BiQuad/x_b11", -1.9998230753949735f);
	_xb12 = _config.getFloat("BiQuad/x_b12", 1.0f);

	_xa21 = _config.getFloat("BiQuad/x_a21", -1.9765168993177902f);
	_xa22 = _config.getFloat("BiQuad/x_a22", 0.97954329118391725f);
	_xb20 = _config.getFloat("BiQuad/x_b20", 1.0f);
	_xb21 = _config.getFloat("BiQuad/x_b21", -1.9998230753949735f);
	_xb22 = _config.getFloat("BiQuad/x_b22", 1.0f);
}
//////////////////////////////////////////////////////////////////////
void
CP2Moments::applyBiQuad(CP2Pulse* pPulse) 
{
	int chan = pPulse->header.channel;
	int gates = pPulse->header.gates;
	// reconfigure biquad filters if necessary.
	// *** Warning - there are returns in the following case statement ***
	switch (chan) {
		case 0:
			// S band H and V pulses
			if (!_doSbandBiQuad)
				return;
			if (gates != _sGates) {
				// number of gates have changed; create new filters
				if (_shBiQuad) {
					delete _shBiQuad;
					delete _svBiQuad;
				}
				_sGates = gates;
				_shBiQuad = new CP2PulseBiQuad(_sGates, _sa11, _sa12, _sb10, _sb11, _sb12, _sa21, _sa22, _sb20, _sb21, _sb22);
				_svBiQuad = new CP2PulseBiQuad(_sGates, _sa11, _sa12, _sb10, _sb11, _sb12, _sa21, _sa22, _sb20, _sb21, _sb22);
			}
			break;
		case 1:
		case 2:
			// X band H and V pulses
			if (!_doXbandBiQuad)
				return;
			if (gates != _xGates) {
				// number of gates have changed; create new filters
				if (_xhBiQuad) {
					delete _shBiQuad;
					delete _svBiQuad;
				}
				_xGates = gates;
				_xhBiQuad = new CP2PulseBiQuad(_xGates, _xa11, _xa12, _xb10, _xb11, _xb12, _xa21, _xa22, _xb20, _xb21, _xb22);
				_xvBiQuad = new CP2PulseBiQuad(_xGates, _xa11, _xa12, _xb10, _xb11, _xb12, _xa21, _xa22, _xb20, _xb21, _xb22);
			}
			break;
		default:
			{ 
				// if we get here, something is realy wrong
				return;
			}
	}

	// apply the biquad filters

	switch (chan) {
			case 0: 
				// Apply the filter. The filtering is done in place.
				if (pPulse->header.horiz) {
					// Sh pulse
					_shBiQuad->tick(*pPulse);
				} else {
					// Sv pulse
					_svBiQuad->tick(*pPulse);
				}
				break;
			case 1: 
				// Xh pulse
				// Apply the filter. The filtering is done in place.
				_xhBiQuad->tick(*pPulse);
				break;
			case 2:
				// Xv pulse
				// Apply the filter. The filtering is done in place.
				_xvBiQuad->tick(*pPulse);
				break;
			default:
				{ 
					// if we get here, something is realy wrong
				}
	}
}

//////////////////////////////////////////////////////////////////////
void
CP2Moments::processPulse(CP2Pulse* pPulse) 
{

	// beam will point to computed moments when they are ready,
	// or null if not ready
	Beam* pBeam = 0;

	int chan = pPulse->header.channel;
	if (chan == 0) 
	{	
		// S band pulses: are successive coplaner H and V pulses
		/// @todo Verify that the h/v flags are correctly making it all 
		/// of the way through the moments processing 
		CP2FullPulse* pFullPulse = new CP2FullPulse(pPulse);
		if (_processSband) {
			_pSmomentThread->processPulse(pFullPulse, 0);
		} else {
			delete pFullPulse;
		}

		// ask for a completed beam. The return value will be
		// 0 if nothing is ready yet.
		pBeam = _pSmomentThread->getNewBeam();
		if (pBeam) {
			// we got a complete beam, send it out on the network.
			sBeamOut(pBeam);
			// and return the storage.
			delete pBeam;
			// bump the counter
			_sBeamCount++;
		}
	} else {
		// X band will have H coplanar pulse on channel 1
		// and V cross planar pulses on channel 2. We need
		// to buffer up the data from the pulses and send it to 
		// the collator to match beam numbers, since matching
		// pulses (i.e. identical pulse numbers) are required
		// for the moments calculations.

		// create a CP2FullPuse, which is a class that will
		// hold the IQ data.
		CP2FullPulse* pFullPulse = new CP2FullPulse(pPulse);
		// send the pulse to the collator. The collator finds matching 
		// pulses. If orphan pulses are detected, they are deleted
		// by the collator. Otherwise, matching pulses returned from
		// the collator can be deleted here.
		_collator.addPulse(pFullPulse, chan-1);

		// now see if we have some matching pulses
		CP2FullPulse* pHPulse;
		CP2FullPulse* pVPulse;
		if (_collator.gotMatch(&pHPulse, &pVPulse)) {
			// a matching pair was found. Send them to the X band
			// moments compute engine.
			if (_processXband) {
				_pXmomentThread->processPulse(pHPulse, pVPulse);
			} else {

				// not processing x band, so delete them.
				delete pHPulse;
				delete pVPulse;
			}
			// ask for a completed beam. The return value will be
			// 0 if nothing is ready yet.
			pBeam = _pXmomentThread->getNewBeam();
		} else {
			pBeam = 0;
		}
		if (pBeam) {
			// we have X products, send on the network
			xBeamOut(pBeam);
			// return the storage
			delete pBeam;
			// bump the counter
			_xBeamCount++;
		}
	}
}
////////////////////////////////////////////////////////////////////
void 
CP2Moments::sendProduct(CP2ProductHeader& header, 
						std::vector<double>& data,
						CP2Packet& packet,
						bool forceSend)
{
	int incr = data.size()*sizeof(data[0]) + sizeof(header);
	// if this packet will get too large by adding new data, 
	// go ahead and send it
	if (packet.packetSize()+incr > _soMaxMsgSize) {
		int bytesSent = 0;
		while (bytesSent != packet.packetSize()) {
			bytesSent = _pProductSocket->writeDatagram(
				(const char*)packet.packetData(),
				packet.packetSize());
			if(bytesSent != packet.packetSize())
				Sleep(1);
		}
		packet.clear();
	}

	packet.addProduct(header, data.size(), &data[0]);
	if (forceSend) {
		int bytesSent = 0;
		while (bytesSent != packet.packetSize()) {
			bytesSent = _pProductSocket->writeDatagram(
				(const char*)packet.packetData(),
				packet.packetSize());
			if(bytesSent != packet.packetSize())
				Sleep(1);
		}
		packet.clear();
	}
}
////////////////////////////////////////////////////////////////////
void 
CP2Moments::sBeamOut(Beam* pBeam)
{
	int gates = pBeam->getNGatesOut();

	_sProductPacket.clear();
	_sProductData.resize(gates);

	CP2ProductHeader header;
	header.beamNum	   = pBeam->getSeqNum();
	header.gates	   = gates;
	header.az		   = pBeam->getAz();
	header.el		   = pBeam->getEl();
	header.gateWidthKm = _gateSpacing;

	const Fields* fields = pBeam->getFields();

	header.prodType = PROD_S_DBMHC;	///< S-band dBm horizontal co-planar
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].dbmhc;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_DBMVC;	///< S-band dBm vertical co-planar
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].dbmvc;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_DBZ;	///< S-band dBz
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].dbz;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_RHOHV;	///< S-band rhohv
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].rhohv;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_PHIDP;	///< S-band phidp
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].phidp;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_ZDR;	///< S-band zdr
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].zdr;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_WIDTH;	///< S-band spectral width
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].width;  }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_VEL;		///< S-band velocity
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].vel;    }
	sendProduct(header, _sProductData, _sProductPacket);

	header.prodType = PROD_S_SNR;		///< S-band SNR
	for (int i = 0; i < gates; i++) { _sProductData[i] = fields[i].snr;    }
	sendProduct(header, _sProductData, _sProductPacket, true);

}
/////////////////////////////////////////////////////////////////////
void 
CP2Moments::xBeamOut(Beam* pBeam)
{
	int gates = pBeam->getNGatesOut();

	_xProductPacket.clear();
	_xProductData.resize(gates);

	CP2ProductHeader header;
	header.beamNum     = pBeam->getSeqNum();
	header.gates       = gates;
	header.az          = pBeam->getAz();
	header.el          = pBeam->getEl();
	header.gateWidthKm = _gateSpacing;

	const Fields* fields = pBeam->getFields();

	header.prodType = PROD_X_DBMHC;
	for (int i = 0; i < gates; i++) { _xProductData[i] = fields[i].dbmhc;}
	sendProduct(header, _xProductData, _xProductPacket);

	header.prodType = PROD_X_DBMVX;
	for (int i = 0; i < gates; i++) { _xProductData[i] = fields[i].dbmvx;}
	sendProduct(header, _xProductData, _xProductPacket);

	header.prodType =  PROD_X_DBZ;	   ///< X-band dBz
	for (int i = 0; i < gates; i++) { _xProductData[i] = fields[i].dbz;  } 
	sendProduct(header, _xProductData, _xProductPacket);

	header.prodType = PROD_X_SNR;		///< X-band SNR
	for (int i = 0; i < gates; i++) { _xProductData[i] = fields[i].snr;    }
	sendProduct(header, _xProductData, _xProductPacket);

	header.prodType = PROD_X_LDR;		///< X-band LDR
	for (int i = 0; i < gates; i++) { _xProductData[i] = fields[i].ldrh;   }
	sendProduct(header, _xProductData, _xProductPacket, true);

}
/////////////////////////////////////////////////////////////////////
void 
CP2Moments::pulseBookKeeping(CP2Pulse* pPulse) 
{
	// look for pulse number errors
	int chan = pPulse->header.channel;

	// check for consecutive pulse numbers
	if (_lastPulseNum[chan]) {
		if (_lastPulseNum[chan]+1 != pPulse->header.pulse_num) {
			_errorCount[chan]++;
			std::cout << "Out of order pulse - Channel=" << chan <<
			" lastnum=" << _lastPulseNum[chan] << 
			"(lastaz=" << _lastPulseAz[chan] << ")" << 
			" received=" << pPulse->header.pulse_num << 
			"(az=" << pPulse->header.az << ")" << 
			" diff=" << pPulse->header.pulse_num - _lastPulseNum[chan] << 
			std::endl;
		}
	}
	_lastPulseNum[chan] = pPulse->header.pulse_num;
	_lastPulseAz[chan] = pPulse->header.az;

	// count the pulses
	_pulseCount[chan]++;

	// look for eofs.
	if (pPulse->header.status & PIRAQ_FIFO_EOF) {
		switch (chan) 
		{
		case 0:
			if (!_eof[0]) {
				_eof[0] = true;
				//_chan0led->setBackgroundColor(QColor("red"));
			}
			break;
		case 1:
			if (!_eof[1]) {
				_eof[1] = true;
				//_chan1led->setBackgroundColor(QColor("red"));
			}
			break;
		case 2:
			if (!_eof[2]) {
				_eof[2] = true;
				//_chan2led->setBackgroundColor(QColor("red"));
			}
			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////
void
CP2Moments::sBandBiQuadEnable(int state) {
	_doSbandBiQuad = (state == Qt::Checked);
}

/////////////////////////////////////////////////////////////////////
void 
CP2Moments::xBandBiQuadEnable(int state) {
	_doXbandBiQuad = (state == Qt::Checked);
}
