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
#include "Ascope.hh"
#include <ScopePlot.h>
#include <Knob.h>

#include <QMessageBox>
#include <QButtonGroup>
#include <QLabel>
#include <QTimer>
#include <QSpinBox>
#include <QLCDNumber>
#include <QSlider>
#include <QLayout>
#include <QTabWidget>
#include <QWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QFrame>
#include <QPushButton>
#include <QPalette>
#include <QDateTime>
#include <QFileDialog>
#include <QProgressBar>
#include <string>
#include <algorithm>

#include <iostream>
#include <time.h>

#include <qwt_wheel.h>

//////////////////////////////////////////////////////////////////////
AScope::AScope(double refreshRateHz, std::string saveDir, QWidget* parent ) :
    QWidget(parent),
    _refreshIntervalHz(refreshRateHz),
    _IQplot(TRUE),
    _fftwData(0),
    _blockSize(64),
    _paused(false),
    _zeroMoment(0.0),
    _channel(0),
    _gateChoice(0),
    _combosInitialized(false),
    _alongBeam(false),
    _nextIQ(0),
    _gates(0),
    _capture(true),
    _saveDir(saveDir),
    _sampleRateHz(10.0e6)
{
    // Set up our form
    setupUi(this);

    // Let's be reasonable with the refresh rate.
    if (refreshRateHz < 1.0) {
    	refreshRateHz = 1.0;
    }

    // initialize running statistics
    for (int i = 0; i < 3; i++) {
        //		_pulseCount[i]	= 0;
        //		_prevPulseCount[i] = 0;
        _errorCount[i] = 0;
        _lastPulseNum[i] = 0;
    }

	// create a button group for the channels
	_chanButtonGroup = new QButtonGroup;

    // connect the controls
    connect(_autoScale,       SIGNAL(released()),           this, SLOT(autoScaleSlot()));
    connect(_gainKnob,        SIGNAL(valueChanged(double)), this, SLOT(gainChangeSlot(double)));
    connect(_up,              SIGNAL(released()),           this, SLOT(upSlot()));
    connect(_dn,              SIGNAL(released()),           this, SLOT(dnSlot()));
    connect(_saveImage,       SIGNAL(released()),           this, SLOT(saveImageSlot()));
    connect(_pauseButton,     SIGNAL(toggled(bool)),        this, SLOT(pauseSlot(bool)));
    connect(_windowButton,    SIGNAL(toggled(bool)),        this, SLOT(windowSlot(bool)));
    connect(_gateNumber,      SIGNAL(activated(int)),       this, SLOT(gateChoiceSlot(int)));
    connect(_alongBeamCheck,  SIGNAL(toggled(bool)),        this, SLOT(alongBeamSlot(bool)));
    connect(_blockSizeCombo,  SIGNAL(activated(int)),       this, SLOT(blockSizeSlot(int)));
    connect(_chanButtonGroup, SIGNAL(buttonReleased(int)),  this, SLOT(channelSlot(int)));

    connect(_xGrid, SIGNAL(toggled(bool)), _scopePlot, SLOT(enableXgrid(bool)));
    connect(_yGrid, SIGNAL(toggled(bool)), _scopePlot, SLOT(enableYgrid(bool)));

    // set the checkbox selections
    _pauseButton->setChecked(false);
    _xGrid->setChecked(true);
    _yGrid->setChecked(true);

    // initialize the book keeping for the plots.
    // This also sets up the radio buttons
    // in the plot type tab widget
    initPlots();

    _gainKnob->setRange(-7, 7);
    _gainKnob->setTitle("Gain");

    // set the minor ticks
    _gainKnob->setScaleMaxMajor(5);
    _gainKnob->setScaleMaxMinor(5);

    // initialize the activity bar
    _activityBar->setRange(0, 100);
    _activityBar->setValue(0);

    _xyGraphRange = 1;
    _xyGraphCenter = 0.0;
    _knobGain = 0.0;
    _knobOffset = 0.0;
    _specGraphRange = 120.0;
    _specGraphCenter = -40.0;

    // set up the palettes
    _greenPalette = this->palette();
    _greenPalette.setColor(this->backgroundRole(), QColor("green"));
    _redPalette = _greenPalette;
    _redPalette.setColor(this->backgroundRole(), QColor("red"));

    // The initial plot type will be I and Q timeseries
    plotTypeSlot(TS_IANDQ_PLOT);


    // start the statistics timer
    int interval = (int)(1000/_refreshIntervalHz);
    if (interval < 1) {
    	// require at least 1 ms pauses.
    	interval = 1;
    }
    startTimer(interval);

    // let the data sources get themselves ready
    sleep(1);

}
//////////////////////////////////////////////////////////////////////
AScope::~AScope() {
}

//////////////////////////////////////////////////////////////////////
void AScope::initCombos(int channels, int gates) {

	// initialize the fft numerics
	initBlockSizes();

	// initialize the number of gates.
	initGates(gates);

	// initialize the channels
	initChans(channels);
}

//////////////////////////////////////////////////////////////////////
void AScope::initGates(int gates) {
	// populate the gate selection combo box
	for (int g = 0; g < gates; g++) {
        QString l = QString("%1").arg(g);
		_gateNumber->addItem(l, QVariant(g));
	}
}

//////////////////////////////////////////////////////////////////////
void AScope::initChans(int channels) {

    // create the channel seletion radio buttons.

	QVBoxLayout *vbox = new QVBoxLayout;
	_chanBox->setLayout(vbox);

	for (int c = 0; c < channels; c++) {
		// create the button and add to the layout
		QString l = QString("Chan %1").arg(c);
		QRadioButton* r = new QRadioButton(l);
		vbox->addWidget(r);
		// add it to the button group, with the channel
		// number as the id
		_chanButtonGroup->addButton(r, c);
		// select the first button
		if (c == 0) {
			r->setChecked(true);
		    _channel = 0;
		} else {
			r->setChecked(false);
		}
	}
}
//////////////////////////////////////////////////////////////////////
void AScope::initBlockSizes() {

    // configure the block/fft size selection
    /// @todo add logic to insure that smallest fft size is a power of two.
    int fftSize = 8;
    int maxFftSize = 4096;
    for (; fftSize <= maxFftSize; fftSize = fftSize*2) {
        _blockSizeChoices.push_back(fftSize);
        QString l = QString("%1").arg(fftSize);
        _blockSizeCombo->addItem(l, QVariant(fftSize));
    }

    // initialize items that depend on the block size selection
    // (fftw and hamming coefficients)
    _blockSizeCombo->setCurrentIndex(5);
    blockSizeSlot(5);

}

//////////////////////////////////////////////////////////////////////
void AScope::initFFT(int size) {

	// return existing structures, if we have them
	if (_fftwData) {
		fftw_destroy_plan(_fftwPlan);
		fftw_free(_fftwData);
	}

	// allocate space
	_fftwData = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)* size);

	// create the plan.
	_fftwPlan = fftw_plan_dft_1d(size, _fftwData, _fftwData,
			FFTW_FORWARD,
			FFTW_ESTIMATE);

	hammingSetup(size);
}


//////////////////////////////////////////////////////////////////////
void AScope::saveImageSlot() {
    QString f = _saveDir.c_str();

    QFileDialog d( this, tr("Save AScope Image"), f,
            tr("PNG files (*.png);;All files (*.*)"));
    d.setFileMode(QFileDialog::AnyFile);
    d.setViewMode(QFileDialog::Detail);
    d.setAcceptMode(QFileDialog::AcceptSave);
    d.setConfirmOverwrite(true);
    d.setDefaultSuffix("png");
    d.setDirectory(f);

    f = "AScope-";
    f += QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    f += ".png";
    d.selectFile(f);
    if (d.exec()) {
        QStringList saveNames = d.selectedFiles();
        _scopePlot->saveImageToFile(saveNames[0].toStdString());
        f = d.directory().absolutePath();
        _saveDir = f.toStdString();
    }
}
//////////////////////////////////////////////////////////////////////
void AScope::processTimeSeries(
        std::vector<double>& Idata,
        std::vector<double>& Qdata) {

	// if we are not plotting time series, ignore
    if (!_IQplot)
        return;

    switch (_tsPlotType) {
    // power spectrum plot
    case TS_SPECTRUM_PLOT: {
        // compute the power spectrum
        _zeroMoment = powerSpectrum(Idata, Qdata);
        displayData();
        break;
    }
    // I Q in time or I versus Q
    case TS_AMPLITUDE_PLOT:
    	Y.resize(Idata.size());
    	for (unsigned int i = 0; i < Y.size(); i++) {
    		Y[i] = sqrt(Idata[i]*Idata[i] + Qdata[i]*Qdata[i]);
    	}
        _zeroMoment = zeroMomentFromTimeSeries(Idata, Qdata);
        displayData();
    	break;
    case TS_IVSQ_PLOT:
    case TS_IANDQ_PLOT:{
        I.resize(Idata.size());
        Q.resize(Qdata.size());
        I = Idata;
        Q = Qdata;
        _zeroMoment = zeroMomentFromTimeSeries(Idata, Qdata);
        displayData();
        break;
    }
    default:
        // ignore others
        break;
    }
}

//////////////////////////////////////////////////////////////////////
void AScope::displayData() {
    double yBottom = _xyGraphCenter - _xyGraphRange;
    double yTop    = _xyGraphCenter + _xyGraphRange;

    QString l = QString("%1").arg(_zeroMoment, 6, 'f', 1);
    _powerDB->setText(l);

    // Time series data display
    PlotInfo* pi = &_tsPlotInfo[_tsPlotType];

    std::string xlabel;
    TS_PLOT_TYPES displayType =
            (TS_PLOT_TYPES) pi->getDisplayType();
    switch (displayType) {
    case TS_AMPLITUDE_PLOT:
        if (pi->autoscale()) {
            autoScale(Y, displayType);
            pi->autoscale(false);
        }
        xlabel = std::string("Time");
        _scopePlot->TimeSeries(Y, yBottom, yTop, 1, xlabel, "Amplitude");
        break;
    case TS_IANDQ_PLOT:
        if (pi->autoscale()) {
            autoScale(I, Q, displayType);
            pi->autoscale(false);
        }
        xlabel = std::string("Time");
        _scopePlot->IandQ(I, Q, yBottom, yTop, 1, xlabel, "I - Q");
        break;
    case TS_IVSQ_PLOT:
        if (pi->autoscale()) {
            autoScale(I, Q, displayType);
            pi->autoscale(false);
        }
        _scopePlot->IvsQ(I, Q, yBottom, yTop, 1, "I", "Q");
        break;
    case TS_SPECTRUM_PLOT:
        if (pi->autoscale()) {
            autoScale(_spectrum, displayType);
            pi->autoscale(false);
        }
        _scopePlot->Spectrum(
        		_spectrum,
        		_specGraphCenter -_specGraphRange/2.0,
        		_specGraphCenter +_specGraphRange/2.0,
        		_sampleRateHz,
        		false,
                "Frequency (Hz)",
                "Power (dB)");
        break;
    }
}

//////////////////////////////////////////////////////////////////////
double AScope::powerSpectrum(
        std::vector<double>& Idata,
        std::vector<double>& Qdata) {

    _spectrum.resize(_blockSize);

    unsigned int n = (Idata.size() <_blockSize) ? Idata.size(): _blockSize;
    for (unsigned int j = 0; j < n; j++) {
        // transfer the data to the fftw input space
        _fftwData[j][0] = Idata[j];
        _fftwData[j][1] = Qdata[j];
    }
    // zero pad if necessary
    for (unsigned int j = n; j < _blockSize; j++) {
        _fftwData[j][0] = 0;
        _fftwData[j][1] = 0;
    }

    // apply the hamming window to the time series
    if (_doHamming) {
        doHamming();
    }

    // caclulate the fft
    fftw_execute(_fftwPlan);

    double zeroMoment = 0.0;

    // reorder and copy the results into _spectrum
    for (unsigned int i = 0; i < _blockSize/2; i++) {
        double pow =
           _fftwData[i][0] * _fftwData[i][0] +
           _fftwData[i][1] * _fftwData[i][1];

        zeroMoment += pow;

        pow /= _blockSize*_blockSize;
        pow = 10.0*log10(pow);
        _spectrum[i+_blockSize/2] = pow;
    }

    for (unsigned int i = _blockSize/2; i < _blockSize; i++) {
        double pow =
           _fftwData[i][0] * _fftwData[i][0] +
           _fftwData[i][1] * _fftwData[i][1];

        zeroMoment += pow;

        pow /= _blockSize*_blockSize;
        pow = 10.0*log10(pow);
        _spectrum[i - _blockSize/2] = pow;
    }

    zeroMoment /= _blockSize*_blockSize;
    zeroMoment = 10.0*log10(zeroMoment);

    return zeroMoment;
}

////////////////////////////////////////////////////////////////////
void AScope::plotTypeSlot(int plotType) {

    // find out the index of the current page
    int pageNum = _typeTab->currentIndex();

    // get the radio button id of the currently selected button
    // on that page.
    int ptype = _tabButtonGroups[pageNum]->checkedId();

    // change to a raw plot type
    TS_PLOT_TYPES tstype = (TS_PLOT_TYPES)ptype;
    plotTypeChange( &_tsPlotInfo[tstype], tstype);
}

//////////////////////////////////////////////////////////////////////
void AScope::tabChangeSlot(QWidget* w) {
    // find out the index of the current page
    int pageNum = _typeTab->currentIndex();

    // get the radio button id of the currently selected button
    // on that page.
    int ptype = _tabButtonGroups[pageNum]->checkedId();

    // change to a raw plot type
    TS_PLOT_TYPES plotType = (TS_PLOT_TYPES)ptype;
    plotTypeChange( &_tsPlotInfo[plotType], plotType);
}

////////////////////////////////////////////////////////////////////
void AScope::plotTypeChange(
        PlotInfo* pi,
        TS_PLOT_TYPES newPlotType) {

    // save the gain and offset of the current plot type
    PlotInfo* currentPi;
    currentPi = &_tsPlotInfo[_tsPlotType];
    currentPi->setGain(pi->getGainMin(), pi->getGainMax(), _knobGain);
    currentPi->setOffset(pi->getOffsetMin(), pi->getOffsetMax(), _xyGraphCenter);

    // restore gain and offset for new plot type
    gainChangeSlot(pi->getGainCurrent());
    _xyGraphCenter = pi->getOffsetCurrent();

    // set the knobs for the new plot type
    _gainKnob->setValue(_knobGain);

     _tsPlotType = newPlotType;

}

////////////////////////////////////////////////////////////////////
void AScope::initPlots() {

    _pulsePlots.insert(TS_AMPLITUDE_PLOT);
    _pulsePlots.insert(TS_IANDQ_PLOT);
    _pulsePlots.insert(TS_IVSQ_PLOT);
    _pulsePlots.insert(TS_SPECTRUM_PLOT);

    _tsPlotInfo[TS_AMPLITUDE_PLOT] = PlotInfo(1, TS_AMPLITUDE_PLOT, "I and Q", "Amplitude", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
    _tsPlotInfo[TS_IANDQ_PLOT]     = PlotInfo(2, TS_IANDQ_PLOT, "I and Q", "I and Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
    _tsPlotInfo[TS_IVSQ_PLOT]      = PlotInfo(3, TS_IVSQ_PLOT, "I vs Q", "I vs Q", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);
    _tsPlotInfo[TS_SPECTRUM_PLOT]  = PlotInfo(4, TS_SPECTRUM_PLOT, "Power Spectrum", "Power Spectrum", -5.0, 5.0, 0.0, -5.0, 5.0, 0.0);

    // remove the one tab that was put there by designer
    _typeTab->removeTab(0);

    // add tabs, and save the button group for
    // for each tab. This code is here to support
    // addition of new tabs for grouping display types,
    // such as an I and Q tab, a Products tab, etc.
    // Right now it is only creating an I and Q tab.
    QButtonGroup* pGroup;

    pGroup = addTSTypeTab("I & Q", _pulsePlots);
    _tabButtonGroups.push_back(pGroup);

    connect(_typeTab, SIGNAL(currentChanged(QWidget *)),
            this, SLOT(tabChangeSlot(QWidget*)));
}

//////////////////////////////////////////////////////////////////////
QButtonGroup* AScope::addTSTypeTab(
        std::string tabName,
        std::set<TS_PLOT_TYPES> types) {
    // The page that will be added to the tab widget
    QWidget* pPage = new QWidget;
    // the layout manager for the page, will contain the buttons
    QVBoxLayout* pVbox = new QVBoxLayout;
    // the button group manager, which has nothing to do with rendering
    QButtonGroup* pGroup = new QButtonGroup;

    std::set<TS_PLOT_TYPES>::iterator i;

    for (i = types.begin(); i != types.end(); i++) {
        // create the radio button
        int id = _tsPlotInfo[*i].getDisplayType();
        QRadioButton* pRadio = new QRadioButton;
        const QString label = _tsPlotInfo[*i].getLongName().c_str();
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
void AScope::timerEvent(QTimerEvent*) {
	_capture = true;
}

//////////////////////////////////////////////////////////////////////
void AScope::gainChangeSlot(
        double gain) {

    // keep a local copy of the gain knob value
    _knobGain = gain;

    _specGraphRange = pow(10.0, gain+2.0);

    _xyGraphRange = pow(10.0, -gain);

    _gainKnob->setValue(gain);

}

//////////////////////////////////////////////////////////////////////
void AScope::upSlot() {
    bool spectrum = false;

    if (_IQplot) {
        PlotInfo* pi = &_tsPlotInfo[_tsPlotType];
        if (pi->getDisplayType() == ScopePlot::SPECTRUM) {
            spectrum = true;
        }
    }

    if (!spectrum) {
        _xyGraphCenter -= 0.03*_xyGraphRange;
    } else {
        _specGraphCenter -= 0.03*_specGraphRange;
    }
    displayData();
}

//////////////////////////////////////////////////////////////////////
void AScope::dnSlot() {

    bool spectrum = false;

    if (_IQplot) {
        PlotInfo* pi = &_tsPlotInfo[_tsPlotType];
        if (pi->getDisplayType() == ScopePlot::SPECTRUM) {
            spectrum = true;
        }
    }

    if (!spectrum) {
        _xyGraphCenter += 0.03*_xyGraphRange;
    } else {
        _specGraphCenter += 0.03*_specGraphRange;
    }

    displayData();
}

//////////////////////////////////////////////////////////////////////
void AScope::autoScale(
        std::vector<double>& data,
        AScope::TS_PLOT_TYPES displayType) {

	if (data.size() == 0)
        return;

    // find the min and max
    double min = *std::min_element(data.begin(), data.end());
    double max = *std::max_element(data.begin(), data.end());

    // adjust the gains
    adjustGainOffset(min, max, displayType);
}

//////////////////////////////////////////////////////////////////////
void AScope::autoScale(
        std::vector<double>& data1,
        std::vector<double>& data2,
        AScope::TS_PLOT_TYPES displayType) {

    if (data1.size() == 0 || data2.size() == 0)
        return;

    // find the min and max
    double min1 = *std::min_element(data1.begin(), data1.end());
    double min2 = *std::min_element(data2.begin(), data2.end());
    double min = std::min(min1, min2);

    double max1 = *std::max_element(data1.begin(), data1.end());
    double max2 = *std::max_element(data2.begin(), data2.end());
    double max = std::max(max1, max2);

    // adjust the gains
    adjustGainOffset(min, max, displayType);
}

//////////////////////////////////////////////////////////////////////
void AScope::adjustGainOffset(
        double min,
        double max,
        AScope::TS_PLOT_TYPES displayType) {

    if (displayType == TS_SPECTRUM_PLOT) {
        // currently in spectrum plot mode
        _specGraphCenter = min + (max-min)/2.0;
        _specGraphRange = 3*(max-min);
        _knobGain = -log10(_specGraphRange);
    } else {
        double factor = 0.8;
        _xyGraphCenter = (min+max)/2.0;
        _xyGraphRange = (1/factor)*(max - min)/2.0;
        if (min == max ||
        		std::isnan(min) ||
        		std::isnan(max) ||
        		std::isinf(min) ||
        		std::isinf(max))
        	_xyGraphRange = 1.0;
        //std::cout << "min:"<<min<<"  max:"<<max<<"     _xxGraphRange is " << _xyGraphRange << "\n";
        _knobGain = -log10(_xyGraphRange);
        _gainKnob->setValue(_knobGain);
    }
}

//////////////////////////////////////////////////////////////////////
void
AScope::newTSItemSlot(AScope::TimeSeries pItem) {

	int chanId = pItem.chanId;
	int tsLength = pItem.IQbeams.size();
    _gates = pItem.gates;
    _sampleRateHz = pItem.sampleRateHz;

	if (!_combosInitialized) {
		// initialize the combo selectors
		initCombos(4, _gates);
		_combosInitialized = true;

	}

	if (chanId == _channel && !_paused && _capture) {
        // extract the time series from the DDS sample
        if (_alongBeam) {
        	for (int i = 0; i < _gates; i++)  {
        		_I[_nextIQ] = pItem.i(0, _nextIQ);
        		_Q[_nextIQ] = pItem.q(0, _nextIQ);
        		_nextIQ++;
        	}
        } else {
        	for (int t = 0; t < tsLength; t++) {
        		_I[_nextIQ] = pItem.i(t, _gateChoice);
        		_Q[_nextIQ] = pItem.q(t, _gateChoice);
        		_nextIQ++;
        		if (_nextIQ == _I.size()) {
        			break;
        		}
        	}
        }

		// now see if we have collected enough samples
        if (_nextIQ == _I.size()) {
			// process the time series
			processTimeSeries(_I, _Q);
			_nextIQ = 0;
			_capture = false;
		}
	}

	// return the DDS item
	emit returnTSItem(pItem);

	// bump the activity bar
	_activityBar->setValue((_activityBar->value()+1) % 100);
}

//////////////////////////////////////////////////////////////////////
void AScope::autoScaleSlot() {
    PlotInfo* pi;

    pi = &_tsPlotInfo[_tsPlotType];

    pi->autoscale(true);
}

//////////////////////////////////////////////////////////////////////
void AScope::pauseSlot(
        bool p) {
    _paused = p;
}

//////////////////////////////////////////////////////////////////////
void AScope::channelSlot(int c) {
    _channel = c;
}

//////////////////////////////////////////////////////////////////////
void AScope::gateChoiceSlot(int index) {
    _gateChoice = index;
}

//////////////////////////////////////////////////////////////////////
void AScope::blockSizeSlot(int index) {
    unsigned int size = _blockSizeChoices[index];

	// Reconfigure fftw if the size has changed
	if (size != _blockSize) {
		initFFT(size);
		// save the size
		_blockSize = size;
	}

	// If not in alongBeam mode, reconfigure _I and _Q capture
	if (!_alongBeam) {
		_I.resize(_blockSize);
		_Q.resize(_blockSize);
		_nextIQ = 0;
	}
}

////////////////////////////////////////////////////////////////////////
double AScope::zeroMomentFromTimeSeries(
		std::vector<double>& I,
		std::vector<double>& Q) {
    double p = 0;
    int n = I.size();

    for (unsigned int i = 0; i < I.size(); i++) {
        p += I[i]*I[i] + Q[i]*Q[i];
    }

    p /= n;
    p = 10.0*log10(p);
    return p;
}

////////////////////////////////////////////////////////////////////////
void
AScope::doHamming() {

  for (unsigned int i = 0; i < _blockSize; i++) {
    _fftwData[i][0] *= _hammingCoefs[i];
    _fftwData[i][1] *= _hammingCoefs[i];
  }
}

////////////////////////////////////////////////////////////////////////
void
AScope::hammingSetup(int size) {

  _hammingCoefs.resize(size);

  for (int i = 0; i < size; i++) {
    _hammingCoefs[i] = 0.54 - 0.46*(cos(2.0*M_PI*i/(size-1)));
  }

}

////////////////////////////////////////////////////////////////////////
void
AScope::windowSlot(bool flag) {
	_doHamming = flag;
}

////////////////////////////////////////////////////////////////////////
void
AScope::alongBeamSlot(bool flag) {
	_alongBeam = flag;

	// If changing into alongBeam mode, set _I and _Q
	// to the number of gates. Otherwise, set them to the
	// blocksize.
	if (_alongBeam) {
		_I.resize(_gates);
		_Q.resize(_gates);
		_nextIQ = 0;
		_gateNumber->setEnabled(false);
	} else {
		_I.resize(_blockSize);
		_Q.resize(_blockSize);
		_gateNumber->setEnabled(true);
	}
	_nextIQ = 0;
}

////////////////////////////////////////////////////////////////////////
AScope::TimeSeries::TimeSeries():
dataType(VOIDDATA)
{
}

////////////////////////////////////////////////////////////////////////
AScope::TimeSeries::TimeSeries(TsDataTypeEnum type):
dataType(type)
{
	sampleRateHz = 10.0e6;
}

////////////////////////////////////////////////////////////////////////
double AScope::TimeSeries::i(int pulse, int gate) const {
    switch (dataType) {
        case FLOATDATA:
            return(static_cast<float*>(IQbeams[pulse])[2 * gate]);
        case SHORTDATA:
            return(static_cast<short*>(IQbeams[pulse])[2 * gate]);
        default:
            std::cerr << "Attempt to extract data from " <<
                "AScope::TimeSeries with data type unset!" << std::endl;
            abort();
    }
}

////////////////////////////////////////////////////////////////////////
double AScope::TimeSeries::q(int pulse, int gate) const {
    switch (dataType) {
      case FLOATDATA:
        return(static_cast<float*>(IQbeams[pulse])[2 * gate + 1]);
      case SHORTDATA:
        return(static_cast<short*>(IQbeams[pulse])[2 * gate + 1]);
      default:
        std::cerr << "Attempt to extract data from " <<
        "AScope::TimeSeries with data type unset!" << std::endl;
        abort();
    }
}

//////////////////////////////////////////////////////////////////////
QFrame* AScope::userFrame() {
	return _userFrame;
}


