
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include "CP2Scope.h"
#include <ScopePlot.h>
#include <Knob.h>

#include <QMessageBox>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qspinbox.h>	
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qwidget.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qpalette.h>
#include <QDateTime>
#include <QFileDialog>

#include <algorithm>

#ifdef WIN32
#include <winsock.h>
#endif
#include <iostream>
#include <time.h>

#include <qwt_wheel.h>

#include "CP2Version.h"
using namespace CP2Lib;

//////////////////////////////////////////////////////////////////////
CP2Scope::CP2Scope(QDialog* parent):
QDialog(parent),
_pPulseSocket(0),    
_pPulseSocketBuf(0),	
_pProductSocket(0),    
_pProductSocketBuf(0),	
_tsDisplayCount(0),
_productDisplayCount(0),
_statsUpdateInterval(5),
_pulsePlot(TRUE),
_performAutoScale(false),
_dataChannel(0),
_config("NCAR", "CP2Scope")
{
	// Set up our form
	setupUi(parent);

	// get our title from the coniguration
	std::string title = _config.getString("title","CP2Scope");
	title += " ";
	title += CP2Version::revision();
	parent->setWindowTitle(title.c_str());

	// initialize running statistics
	for (int i = 0; i < 3; i++) {
		_pulseCount[i]	= 0;
		_prevPulseCount[i] = 0;
		_errorCount[i] = 0;
		_eof[i] = false;
		_lastPulseNum[i] = 0;
	}

	// The initial plot type will be Sband I and Q
	_pulsePlotType = S_TIMESERIES;

	// connect the controls
	connect(_autoScale, SIGNAL(released()),           this, SLOT(autoScaleSlot()));
	connect(_gainKnob,  SIGNAL(valueChanged(double)), this, SLOT(gainChangeSlot(double)));
	connect(_up,        SIGNAL(released()),           this, SLOT(upSlot()));
	connect(_dn,        SIGNAL(released()),           this, SLOT(dnSlot()));
	connect(_saveImage, SIGNAL(released()),           this, SLOT(saveImageSlot()));

	// intialize the data reception sockets.
	initSockets();	

	// initialize the book keeping for the plots.
	// This also sets up the radio buttons 
	// in the plot type tab widget
	initPlots();

	_gainKnob->setRange(-7, 7);
	_gainKnob->setTitle("Gain");

	// set the minor ticks
	_gainKnob->setScaleMaxMajor(5);
	_gainKnob->setScaleMaxMinor(5);

	_xyGraphRange = 1;
	_xyGraphCenter = 0.0;
	_knobGain = 0.0;
	_knobOffset = 0.0;
	_specGraphRange = 120.0;
	_specGraphCenter = -40.0;

	// set up the palettes
	_greenPalette = _chan0led->palette();
	_greenPalette.setColor(_chan0led->backgroundRole(), QColor("green"));
	_redPalette = _greenPalette;
	_redPalette.setColor(_chan0led->backgroundRole(), QColor("red"));

	// initialize eof leds to green
	_chan0led->setAutoFillBackground(true);
	_chan1led->setAutoFillBackground(true);
	_chan2led->setAutoFillBackground(true);
	_chan0led->setPalette(_greenPalette);
	_chan1led->setPalette(_greenPalette);
	_chan2led->setPalette(_greenPalette);

	// set the intial plot type
	plotTypeSlot(S_TIMESERIES);

	_xFullScale = 1000;
	I.resize(_xFullScale);			//	default timeseries array size full range at 1KHz
	Q.resize(_xFullScale);

	//	display decimation, set to get ~50/sec
	_pulseDecimation	= _config.getInt("pulseDecimation", 50); 
	m_pulseDec->setNum((int)_pulseDecimation);
	_productDecimation	= _config.getInt("productDecimation", 5);
	m_productDec->setNum((int)_productDecimation);

	//	set up fft for power calculations: 
	_fftBlockSize = 256;	//	temp constant for initial proving 
	// allocate the data space for fftw
	_fftwData  = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * _fftBlockSize);
	// create the plan.
	_fftwPlan = fftw_plan_dft_1d(_fftBlockSize, _fftwData, _fftwData,
		FFTW_FORWARD, FFTW_ESTIMATE);
	//	power correction factor applied to (uncorrected) powerSpectrum() output:
	_powerCorrection = 0.0;	//	use for power correction to dBm

	// start the statistics timer
	startTimer(_statsUpdateInterval*1000);

}
//////////////////////////////////////////////////////////////////////
CP2Scope::~CP2Scope() {
	if (_pPulseSocket)
		delete _pPulseSocket;

	if (_pProductSocket)
		delete _pProductSocket;

}

//////////////////////////////////////////////////////////////////////
void 
CP2Scope::newProductSlot()
{
	CP2Packet packet;
	int	readBufLen = _pProductSocket->readDatagram(
		&_pProductSocketBuf[0], 
		_pProductSocketBuf.size());

	if (readBufLen > 0) {
		// put this datagram into a packet
		bool packetBad = packet.setProductData(readBufLen, &_pProductSocketBuf[0]);

		// Extract the products and process them.
		// From here on out, we are divorced from the
		// data transport.
		if (!packetBad) {
			for (int i = 0; i < packet.numProducts(); i++) {
				CP2Product* pProduct = packet.getProduct(i);
				// do all of the heavy lifting for this pulse
				processProduct(pProduct);
			} 
		} else {
			// packet error. What to do?
			int x = 0;
		}
	} else {
		// read error. What to do?
#ifdef WIN32
		int e = WSAGetLastError();
#endif
	}

}

//////////////////////////////////////////////////////////////////////
void 
CP2Scope::newPulseSlot()
{
	CP2Packet packet;
	int	readBufLen = _pPulseSocket->readDatagram(
		&_pPulseSocketBuf[0], 
		_pPulseSocketBuf.size());

	if (readBufLen > 0) {
		// put this datagram into a packet
		bool packetBad = packet.setPulseData(readBufLen, &_pPulseSocketBuf[0]);

		// Extract the pulses and process them.
		// Observe paranoia for validating packets and pulses.
		// From here on out, we are divorced from the
		// data transport.
		if (!packetBad) {
			for (int i = 0; i < packet.numPulses(); i++) {
				CP2Pulse* pPulse = packet.getPulse(i);
				if (pPulse) {
					int chan = pPulse->header.channel;
					if (chan >= 0 && chan < 3) {
						// do all of the heavy lifting for this pulse
						processPulse(pPulse);
						// sanity check on channel
						_pulseCount[chan]++;
						if (pPulse->header.status & PIRAQ_FIFO_EOF) {
							switch (chan) 
							{
							case 0:
								if (!_eof[0]) {
									_eof[0] = true;
									_chan0led->setPalette(_redPalette);
								}
								break;
							case 1:
								if (!_eof[1]) {
									_eof[1] = true;
									_chan1led->setPalette(_redPalette);
								}
								break;
							case 2:
								if (!_eof[2]) {
									_eof[2] = true;
									_chan2led->setPalette(_redPalette);
								}
								break;
							}
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
CP2Scope::saveImageSlot() {
	QString f = _config.getString("imageSaveDirectory", "c:/").c_str();

	QFileDialog d(this, tr("Save CP2Scope Image"),
		f, tr("PNG files (*.png);;All files (*.*)"));
	d.setFileMode(QFileDialog::AnyFile);
	d.setViewMode(QFileDialog::Detail);
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setConfirmOverwrite(true);
	d.setDefaultSuffix("png");
	d.setDirectory(f);

	f = "CP2Scope-";
	f += QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
	f += ".png";
	d.selectFile(f);
	if (d.exec()) {
		QStringList saveNames = d.selectedFiles();
		_scopePlot->saveImageToFile(saveNames[0].toStdString());
		f = d.directory().absolutePath();
		_config.setString("imageSaveDirectory", f.toStdString());
	}
}
//////////////////////////////////////////////////////////////////////
void
CP2Scope::processPulse(CP2Pulse* pPulse) 
{
	if (!_pulsePlot)
		return;

	int chan = pPulse->header.channel;

	// look for pulse number errors
	if (_lastPulseNum[chan]) {
		if (_lastPulseNum[chan]+1 != pPulse->header.pulse_num) {
			_errorCount[chan]++;
		}
	}
	_lastPulseNum[chan] = pPulse->header.pulse_num;

	float* data = &(pPulse->data[0]);

	PlotInfo* pi = &_pulsePlotInfo[_pulsePlotType];
	switch (pi->getDisplayType()) 
	{
	case ScopePlot::SPECTRUM:
		{
			if (pPulse->header.channel != _dataChannel)
				break;
			_tsDisplayCount++;
			if	(_tsDisplayCount >= _pulseDecimation)	{	
				_spectrum.resize(_fftBlockSize);	//	probably belongs somewhere else
				for( int j = 0; j < _fftBlockSize; j++)	{
					// transfer the data to the fftw input space
					_fftwData[j][0] = pPulse->data[2*j]*PIRAQ3D_SCALE;
					_fftwData[j][1] = pPulse->data[2*j+1]*PIRAQ3D_SCALE;
				}
				double zeroMoment = powerSpectrum();
				// correct unscaled power data using knob setting: 
				for(int j = 0; j < _fftBlockSize; j++)	{
					_spectrum[j] += _powerCorrection;
				}
				displayData();	
				_tsDisplayCount = 0; 
			}
			break;
		}
	case TIMESERIES:
	case IVSQ:
		{
			if (pPulse->header.channel != _dataChannel)
				break;
			_tsDisplayCount++;
			if	(_tsDisplayCount >= _pulseDecimation)	{	
				I.resize(pPulse->header.gates);
				Q.resize(pPulse->header.gates);
				for (int i = 0; i < 2*pPulse->header.gates; i+=2) {	
					I[i/2] = pPulse->data[i]*PIRAQ3D_SCALE;
					Q[i/2] = pPulse->data[i+1]*PIRAQ3D_SCALE;
				}
				displayData();	
				_tsDisplayCount = 0; 
			}
		}
	default:
		// ignore others
		break;
	}
}

//////////////////////////////////////////////////////////////////////
void
CP2Scope::processProduct(CP2Product* pProduct) 
{
	// if we are displaying a raw plot, just ignore
	if (_pulsePlot) 
		return;

	PRODUCT_TYPES prodType = pProduct->header.prodType;

	if (prodType == _productPlotType) {
		_productDisplayCount++;
		if	(_productDisplayCount < _productDecimation)	{
			return;
		}
		_productDisplayCount = 0;
		// this is the product that we are currntly displaying
		// extract the data and display it.
		_ProductData.resize(pProduct->header.gates);
		for (int i = 0; i < pProduct->header.gates; i++) {
			_ProductData[i] = pProduct->data[i];
		}
		displayData();
	}
}

//////////////////////////////////////////////////////////////////////
void
CP2Scope::displayData() 
{
	double yBottom = _xyGraphCenter - _xyGraphRange;
	double yTop =    _xyGraphCenter + _xyGraphRange;
	if (_pulsePlot) {
		PlotInfo* pi = &_pulsePlotInfo[_pulsePlotType];

		switch (pi->getDisplayType()) 
		{
		case ScopePlot::TIMESERIES:
			if (_performAutoScale)
				autoScale(I, Q);
			_scopePlot->TimeSeries(I, Q, yBottom, yTop, 1, "Gate", "I - Q (V)");
			break;
		case ScopePlot::IVSQ:
			if (_performAutoScale)
				autoScale(I, Q);
			_scopePlot->IvsQ(I, Q, yBottom, yTop, 1, "I (V)", "Q (V)"); 
			break;
		case ScopePlot::SPECTRUM:
			_scopePlot->Spectrum(_spectrum, 
				_specGraphCenter-_specGraphRange/2.0, 
				_specGraphCenter+_specGraphRange/2.0, 
				1000000, 
				false, 
				"Frequency (Hz)", 
				"Power (dB)");	
			break;
		}

	} else {
		if (_performAutoScale)
			autoScale(_ProductData);
		// send in the product id, which ScopePlot::Product() uses
		// to decide if axis rescaling is needed.
		PlotInfo* pi = &_prodPlotInfo[_productPlotType];
		_scopePlot->Product(_ProductData, 
			pi->getId(), 
			yBottom, 
			yTop, 
			_ProductData.size(),
			"Gate",
			pi->getLongName());
	}
}

//////////////////////////////////////////////////////////////////////
void
CP2Scope::initSockets()	
{
	// assign the incoming data ports
	_pulsePort	 = _config.getInt("Network/pulsePort",   3100);
	_productPort = _config.getInt("Network/productPort", 3200);

	// get the interface addresses
	std::string pulseNetwork   = _config.getString("Network/pulseNetwork", "192.168.1");
	std::string productNetwork = _config.getString("Network/productNetwork", "192.168.1");

	int result;

	// creat the sockets
	_pPulseSocket   = new CP2UdpSocket(pulseNetwork, _pulsePort,   false, 0, 10000000);
	if (!_pPulseSocket->ok()) {
		QMessageBox e;
		e.warning(this, "Error opening pulse port",_pPulseSocket->errorMsg().c_str(), 
			QMessageBox::Ok, QMessageBox::NoButton);
	}
	_pProductSocket = new CP2UdpSocket(productNetwork, _productPort, false, 0, 10000000);
	if (!_pProductSocket->ok()) {
		QMessageBox e;
		e.warning(this, "Error opening product port",_pProductSocket->errorMsg().c_str(), 
			QMessageBox::Ok, QMessageBox::NoButton);
	}

	// allocate the socket read buffers
	_pPulseSocketBuf.resize(_pPulseSocket->rcvBufferSize());
	_pProductSocketBuf.resize(_pProductSocket->rcvBufferSize());

	m_pulseIP->setText(_pPulseSocket->toString().c_str());
	m_productIP->setText(_pProductSocket->toString().c_str());

	// connect the incoming data signal to the read slots
	if (!connect(_pPulseSocket, SIGNAL(readyRead()), 
		     this, SLOT(newPulseSlot())))
	  std::cout << "CP2Scope::initSockets connect(pPulseSocket) failed"
		    << std::endl;
	if (!connect(_pProductSocket, SIGNAL(readyRead()), 
		    this, SLOT(newProductSlot())))
	  std::cout << "CP2Scope::initSockets connect(pProductSocket) failed"
		    << std::endl;
}

//////////////////////////////////////////////////////////////////////
double
CP2Scope::powerSpectrum()
{
	// apply the hamming window to the time series
	//  if (_doHamming)
	//    doHamming();

	// caclulate the fft
	fftw_execute(_fftwPlan);

	double zeroMoment = 0.0;
	int n = _fftBlockSize;

	// reorder and copy the results into _spectrum
	for (int i = 0 ; i < n/2; i++) {
		double pow = 
			_fftwData[i][0] * _fftwData[i][0] +
			_fftwData[i][1] * _fftwData[i][1];

		zeroMoment += pow;

		pow /= n*n;
		pow = 10.0*log10(pow);
		_spectrum[i+n/2] = pow;
	}

	for (int i = n/2 ; i < n; i++) {
		double pow =      
			_fftwData[i][0] * _fftwData[i][0] +
			_fftwData[i][1] * _fftwData[i][1];

		zeroMoment += pow;

		pow /= n*n;
		pow = 10.0*log10(pow);
		_spectrum[i - n/2] = pow;
	}

	zeroMoment /= n*n;
	zeroMoment = 10.0*log10(zeroMoment);

	return zeroMoment;
}
////////////////////////////////////////////////////////////////////
void
CP2Scope::plotTypeSlot(int plotType) {
	// reset the product display counts
	_tsDisplayCount = 0;
	_productDisplayCount = 0;

	// find out the index of the current page
	int pageNum = _typeTab->currentIndex();

	// get the radio button id of the currently selected button
	// on that page.
	int ptype = _tabButtonGroups[pageNum]->checkedId();

	if (pageNum == 0) {
		// change to a raw plot type
		PLOTTYPE plotType = (PLOTTYPE)ptype;
		plotTypeChange(&_pulsePlotInfo[plotType], plotType, (PRODUCT_TYPES)0 , true);
	} else {
		// change to a product plot type
		PRODUCT_TYPES productType = (PRODUCT_TYPES)ptype;
		plotTypeChange(&_prodPlotInfo[productType], (PLOTTYPE)0, productType , false);
	}
}
//////////////////////////////////////////////////////////////////////
void
CP2Scope::tabChangeSlot(QWidget* w) 
{
	// find out the index of the current page
	int pageNum = _typeTab->currentIndex();

	// get the radio button id of the currently selected button
	// on that page.
	int ptype = _tabButtonGroups[pageNum]->checkedId();

	if (pageNum == 0) {
		// change to a raw plot type
		PLOTTYPE plotType = (PLOTTYPE)ptype;
		plotTypeChange(&_pulsePlotInfo[plotType], plotType, (PRODUCT_TYPES)0 , true);
	} else {
		// change to a product plot type
		PRODUCT_TYPES productType = (PRODUCT_TYPES)ptype;
		plotTypeChange(&_prodPlotInfo[productType], (PLOTTYPE)0, productType , false);
	}
}

////////////////////////////////////////////////////////////////////
void
CP2Scope::plotTypeChange(PlotInfo* pi, 
						 PLOTTYPE newPlotType, 
						 PRODUCT_TYPES newProductType, 
						 bool pulsePlot)
{

	// save the gain and offset of the current plot type
	PlotInfo* currentPi;
	if (_pulsePlot) {
		currentPi = &_pulsePlotInfo[_pulsePlotType];
	} else {
		currentPi = &_prodPlotInfo[_productPlotType];
	}
	currentPi->setGain(pi->getGainMin(), pi->getGainMax(), _knobGain);
	currentPi->setOffset(pi->getOffsetMin(), pi->getOffsetMax(), _xyGraphCenter);

	// restore gain and offset for new plot type
	gainChangeSlot(pi->getGainCurrent());
	_xyGraphCenter = pi->getOffsetCurrent();


	// set the knobs for the new plot type
	_gainKnob->setValue(_knobGain);

	// change the plot type
	if (pulsePlot) {
		_pulsePlotType = newPlotType;
	} else {
		_productPlotType = newProductType;
	}

	_pulsePlot = pulsePlot;

	// select the piraq discriminator based
	// on the plot type.
	if (_pulsePlot) {
		// change data channel if necessary
		switch(_pulsePlotType) 
		{
		case S_TIMESERIES:	// S time series
		case S_IQ:			// S IQ
		case S_SPECTRUM:		// S spectrum 
			_dataChannel = 0;
			break;
		case XH_TIMESERIES:	// Xh time series
		case XH_IQ:			// Xh IQ
		case XH_SPECTRUM:	// Xh spectrum 
			_dataChannel = 1;
			break;
		case XV_TIMESERIES:	// Xv time series
		case XV_IQ:			// Xv IQ
		case XV_SPECTRUM:	// Xv spectrum 
			_dataChannel = 2;
			break;
		default:
			break;
		}
	}

}

////////////////////////////////////////////////////////////////////
void
CP2Scope::initPlots()
{

	_pulsePlots.insert(S_TIMESERIES);
	_pulsePlots.insert(XH_TIMESERIES);
	_pulsePlots.insert(XV_TIMESERIES);

	_pulsePlots.insert(S_IQ);
	_pulsePlots.insert(XH_IQ);
	_pulsePlots.insert(XV_IQ);

	_pulsePlots.insert(S_SPECTRUM);
	_pulsePlots.insert(XH_SPECTRUM);
	_pulsePlots.insert(XV_SPECTRUM);

	_sMomentsPlots.insert(PROD_S_DBMHC);
	_sMomentsPlots.insert(PROD_S_DBMVC);
	_sMomentsPlots.insert(PROD_S_DBZ);
	_sMomentsPlots.insert(PROD_S_WIDTH);
	_sMomentsPlots.insert(PROD_S_VEL);
	_sMomentsPlots.insert(PROD_S_SNR);
	_sMomentsPlots.insert(PROD_S_RHOHV);
	_sMomentsPlots.insert(PROD_S_PHIDP);
	_sMomentsPlots.insert(PROD_S_ZDR);

	_xMomentsPlots.insert(PROD_X_DBMHC);
	_xMomentsPlots.insert(PROD_X_DBMVX);
	_xMomentsPlots.insert(PROD_X_DBZ);
	_xMomentsPlots.insert(PROD_X_SNR);
	_xMomentsPlots.insert(PROD_X_LDR);

	_pulsePlotInfo[S_TIMESERIES]  = PlotInfo( S_TIMESERIES, TIMESERIES, "I and Q", "S:  I and Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[XH_TIMESERIES] = PlotInfo(XH_TIMESERIES, TIMESERIES, "I and Q", "Xh: I and Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[XV_TIMESERIES] = PlotInfo(XV_TIMESERIES, TIMESERIES, "I and Q", "Xv: I and Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[S_IQ]          = PlotInfo(         S_IQ,       IVSQ, "I vs Q", "S:  I vs Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[XH_IQ]         = PlotInfo(        XH_IQ,       IVSQ, "I vs Q", "Xh: I vs Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[XV_IQ]         = PlotInfo(        XV_IQ,       IVSQ, "I vs Q", "Xv: I vs Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[S_SPECTRUM]    = PlotInfo(   S_SPECTRUM,   SPECTRUM, "Power Spectrum", "S:  Power Spectrum", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[XH_SPECTRUM]   = PlotInfo(  XH_SPECTRUM,   SPECTRUM, "Power Spectrum", "Xh: Power Spectrum", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_pulsePlotInfo[XV_SPECTRUM]   = PlotInfo(  XV_SPECTRUM,   SPECTRUM, "Power Spectrum", "Xv: Power Spectrum", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);

	_prodPlotInfo[PROD_S_DBMHC]       = PlotInfo(      PROD_S_DBMHC,    PRODUCT, "H Dbm", "Sh: Dbm", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_DBMVC]       = PlotInfo(      PROD_S_DBMVC,    PRODUCT, "V Dbm", "Sv: Dbm", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_DBZ]         = PlotInfo(      PROD_S_DBZ,      PRODUCT, "Dbz",   "S : Dbz", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_WIDTH]       = PlotInfo(      PROD_S_WIDTH,    PRODUCT, "Width", "S:  Width", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_VEL]         = PlotInfo(      PROD_S_VEL,      PRODUCT, "Velocity", "S:  Velocity", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_SNR]         = PlotInfo(      PROD_S_SNR,      PRODUCT, "SNR", "S:  SNR", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_RHOHV]       = PlotInfo(      PROD_S_RHOHV,    PRODUCT, "Rhohv", "S:  Rhohv", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_PHIDP]       = PlotInfo(      PROD_S_PHIDP,    PRODUCT, "Phidp", "S:  Phidp", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_S_ZDR]         = PlotInfo(      PROD_S_ZDR,      PRODUCT, "Zdr", "S:  Zdr", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);

	_prodPlotInfo[PROD_X_DBMHC]       = PlotInfo(      PROD_X_DBMHC,    PRODUCT, "H Dbm", "Xh: Dbm", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_X_DBMVX]       = PlotInfo(      PROD_X_DBMVX,    PRODUCT, "V Cross Dbm", "Xv: Dbm", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_X_DBZ]         = PlotInfo(      PROD_X_DBZ,      PRODUCT, "Dbz",  "X: Dbz", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_X_SNR]         = PlotInfo(      PROD_X_SNR,      PRODUCT, "SNR", "Xh: SNR", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_X_WIDTH]       = PlotInfo(      PROD_X_WIDTH,    PRODUCT, "Width", "Xh: Width", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_X_VEL]         = PlotInfo(      PROD_X_VEL,      PRODUCT, "Velocity", "Xh: Velocity", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
	_prodPlotInfo[PROD_X_LDR]         = PlotInfo(      PROD_X_LDR,      PRODUCT, "LDR", "Xhv:LDR", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);

	// remove the one tab that was put there by designer
	_typeTab->removeTab(0);

	// add tabs, and save the button group for
	// for each tab.
	QButtonGroup* pGroup;

	pGroup = addPlotTypeTab("Raw", _pulsePlots);
	_tabButtonGroups.push_back(pGroup);

	pGroup = addProductTypeTab("S", _sMomentsPlots);
	_tabButtonGroups.push_back(pGroup);

	pGroup = addProductTypeTab("X", _xMomentsPlots);
	_tabButtonGroups.push_back(pGroup);

	connect(_typeTab, SIGNAL(currentChanged(QWidget *)), 
		this, SLOT(tabChangeSlot(QWidget*)));
}
//////////////////////////////////////////////////////////////////////
QButtonGroup*
CP2Scope::addPlotTypeTab(std::string tabName, std::set<PLOTTYPE> types)
{
	// The page that will be added to the tab widget
	QWidget* pPage = new QWidget;
	// the layout manager for the page, will contain the buttons
	QVBoxLayout* pVbox = new QVBoxLayout;
	// the button group manager, which has nothing to do with rendering
	QButtonGroup* pGroup = new QButtonGroup;

	std::set<PLOTTYPE>::iterator i;

	for (i = types.begin(); i != types.end(); i++) 
	{
		// create the radio button
		int id = _pulsePlotInfo[*i].getId();
		QRadioButton* pRadio = new QRadioButton;
		const QString label = _pulsePlotInfo[*i].getLongName().c_str();
		pRadio->setText(label);

		// put the button in the button group
		pGroup->addButton(pRadio, id);
		// assign the button to the layout manager
		pVbox->addWidget(pRadio);

		// set the first radio button of the group
		// to be selected.
		if (i == types.begin()) {
			pRadio->setChecked(true);
		}
	}
	// associate the layout manager with the page
	pPage->setLayout(pVbox);

	// put the page on the tab
	_typeTab->insertTab(-1, pPage, tabName.c_str());

	// connect the button released signal to our plot type change slot.
	connect(pGroup, SIGNAL(buttonReleased(int)), this, SLOT(plotTypeSlot(int)));

	return pGroup;
}
//////////////////////////////////////////////////////////////////////
QButtonGroup*
CP2Scope::addProductTypeTab(std::string tabName, std::set<PRODUCT_TYPES> types)
{
	// The page that will be added to the tab widget
	QWidget* pPage = new QWidget;
	// the layout manager for the page, will contain the buttons
	QVBoxLayout* pVbox = new QVBoxLayout;
	// the button group manager, which has nothing to do with rendering
	QButtonGroup* pGroup = new QButtonGroup;

	std::set<PRODUCT_TYPES>::iterator i;

	for (i = types.begin(); i != types.end(); i++) 
	{
		// create the radio button
		int id = _prodPlotInfo[*i].getId();
		QRadioButton* pRadio = new QRadioButton;
		const QString label = _prodPlotInfo[*i].getLongName().c_str();
		pRadio->setText(label);

		// put the button in the button group
		pGroup->addButton(pRadio, id);
		// assign the button to the layout manager
		pVbox->addWidget(pRadio);

		// set the first radio button of the group
		// to be selected.
		if (i == types.begin()) {
			pRadio->setChecked(true);
		}
	}
	// associate the layout manager with the page
	pPage->setLayout(pVbox);

	// put the page on the tab
	_typeTab->insertTab(-1, pPage, tabName.c_str());

	// connect the button released signal to our plot type change slot.
	connect(pGroup, SIGNAL(buttonReleased(int)), this, SLOT(plotTypeSlot(int)));

	return pGroup;
}
//////////////////////////////////////////////////////////////////////
void
CP2Scope::timerEvent(QTimerEvent*) 
{

	int rate[3];
	for (int i = 0; i < 3; i++) 
	{
		rate[i] = (_pulseCount[i] - _prevPulseCount[i])/(double)_statsUpdateInterval;
		_prevPulseCount[i] = _pulseCount[i];
	}
	_chan0pulseCount->setNum(_pulseCount[0]/1000);
	_chan0pulseRate->setNum(rate[0]);
	_chan0errors->setNum(_errorCount[0]);
	_chan1pulseCount->setNum(_pulseCount[1]/1000);
	_chan1pulseRate->setNum(rate[1]);
	_chan1errors->setNum(_errorCount[1]);
	_chan2pulseCount->setNum(_pulseCount[2]/1000);
	_chan2pulseRate->setNum(rate[2]);
	_chan2errors->setNum(_errorCount[2]);
	  
//	std::cout << "Packet errors = " <<
//	  _errorCount[0] << " " << _errorCount[1] << " " <<
//	  _errorCount[2] << std::endl;

}

//////////////////////////////////////////////////////////////////////
void CP2Scope::gainChangeSlot(double gain)	{	

	// keep a local copy of the gain knob value
	_knobGain = gain;

	_powerCorrection = gain; 

	_xyGraphRange = pow(10.0,-gain);

	_gainKnob->setValue(gain);

}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
void 
CP2Scope::upSlot()	{
	bool spectrum = false;

	if (_pulsePlot) {
		PlotInfo* pi = &_pulsePlotInfo[_pulsePlotType];
		if (pi->getDisplayType() == ScopePlot::SPECTRUM) {
			spectrum = true;
		} 
	}

	if (!spectrum) {
		_xyGraphCenter -= 0.03*_xyGraphRange;
	} else {
		_specGraphCenter -= 0.03*_specGraphRange;
	}
}
//////////////////////////////////////////////////////////////////////
void 
CP2Scope::dnSlot()	{

	bool spectrum = false;

	if (_pulsePlot) {
		PlotInfo* pi = &_pulsePlotInfo[_pulsePlotType];
		if (pi->getDisplayType() == ScopePlot::SPECTRUM) {
			spectrum = true;
		} 
	}

	if (!spectrum) {
		_xyGraphCenter += 0.03*_xyGraphRange;
	} else {
		_specGraphCenter += 0.03*_specGraphRange;
	}
}
//////////////////////////////////////////////////////////////////////
void 
CP2Scope::autoScale(std::vector<double>& data)
{
	if (data.size() == 0) {
		_performAutoScale = false;
		return;
	}
	double min = data[0];
	double max = data[0];
	for (int i = 0; i < data.size(); i++) {
		if (data[i] > max)
			max = data[i]; 
		else
			if (data[i] < min)
				min = data[i];
	}
	adjustGainOffset(min, max);
	_performAutoScale = false;
}
//////////////////////////////////////////////////////////////////////
void 
CP2Scope::autoScale(std::vector<double>& data1, std::vector<double>& data2)
{
	if (data1.size() == 0 || data2.size() == 0) {
		_performAutoScale = false;
		return;
	}

	double min = 1.0e10;
	double max = -1.0e10;
	for (int i = 0; i < data1.size(); i++) {
		if (data1[i] > max)
			max = data1[i]; 
		else
			if (data1[i] < min)
				min = data1[i];
	}

	for  (int i = 0; i < data2.size(); i++) {
		if (data2[i] > max)
			max = data2[i]; 
		else
			if (data2[i] < min)
				min = data2[i];
	}
	adjustGainOffset(min, max);
	_performAutoScale = false;
}
//////////////////////////////////////////////////////////////////////
void 
CP2Scope::adjustGainOffset(double min, double max)
{
	double factor = 0.8;
	_xyGraphCenter = (min+max)/2.0;
	_xyGraphRange = (1/factor)*(max - min)/2.0;

	_knobGain = -log10(_xyGraphRange);

	_gainKnob->setValue(_knobGain);
}
//////////////////////////////////////////////////////////////////////
void 
CP2Scope::autoScaleSlot()
{
	_performAutoScale = true;
}

