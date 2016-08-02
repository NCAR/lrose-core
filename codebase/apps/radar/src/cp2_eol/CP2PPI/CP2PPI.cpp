#include "CP2PPI.h"
#include <PPI/PPI.h>
#include "CP2PPI.h"
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
#include <QString>
#include <QFrame>
#include <QPushButton>
#include <QPalette>
#include <QStackedWidget>
#include <QCheckBox>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QImage>
#include <QPixmap>
#include <QDateTime>
#include <QPainter>
#include <QRadialGradient>
#include <QBrush>
#include <QPen>

#include "ColorBarSettings.h"
#include "CP2Version.h"
using namespace CP2Lib;
#include <iostream>
#include <algorithm>

#ifdef WIN32
#include <winsock.h>
#endif
#include <iostream>
#include <time.h>

CP2PPI::CP2PPI(QDialog* parent):
QDialog(parent),
_sGates(0),                 // default value until real one is known
_xGates(0),                 // default value until real one is known
_sGateWidthKm(0.0),         // default value until real one is known
_xGateWidthKm(0.0),         // default value until real one is known
_pSocket(0),    
_pSocketBuf(0),	
_currentSbeamNum(0),
_currentXbeamNum(0),
_pause(false),
_ppiSactive(true),
_productPort(3200),         // default value until real one is known
_config("NCAR", "CP2PPI"),
_backColor("royalblue")
{
	setupUi(parent);

	// get our title from the coniguration
	std::string title = _config.getString("Title","CP2PPI");
	title += " ";
	title += CP2Version::revision();
	parent->setWindowTitle(title.c_str());

	_config.setString("title", "CP2PPI Plan Position Index Display");

	_ppiGateDecimation = _config.getInt("gateDecimate", 1);

	_statsUpdateInterval = _config.getInt("statsUpdateSeconds", 5);

	// intialize the data reception socket.
	// set up the ocket notifier and connect it
	// to the data reception slot
	initSocket();	

	// initialize the color maps, reading them from the configuration,
	// and including the builtins provided by ColorMap.
	initColorMaps();

	// initialize the book keeping for the plots.
	// This will initialize _sProductList and _xProductList.
	// This also sets up the radio buttons 
	// in the product type tab widget. 
	initPlots();
	// count the number product types that initPlots gave us
	_nVarsSband = _sProductList.size();
	_nVarsXband = _xProductList.size();

	// Now intialize the colormaps that go with each product.

	// create the Sband color maps
	std::set<PRODUCT_TYPES>::iterator pSet;
	_mapsSband.resize(_nVarsSband);
	for (pSet=_sProductList.begin(); pSet!=_sProductList.end(); pSet++) 
	{
		// Set up the color map.
		// First get the scales.
		double scaleMin = _ppiInfo[*pSet].getScaleMin();
		double scaleMax = _ppiInfo[*pSet].getScaleMax();
		// now get the name
		std::string mapName = _ppiInfo[*pSet].getColorMapName();
		// create a copy of the map in our collection which has this name
		ColorMap* pMap = new ColorMap(_colorMaps[mapName]);
		// and set its range.
		pMap->setRange(scaleMin, scaleMax);
		// Finally save the pointer to the colormap in this variable's _ppiInfo entry.
		int index = _ppiInfo[*pSet].getPpiIndex();
		_mapsSband[index] = pMap;
	}

	// create the Xband color maps
	_mapsXband.resize(_nVarsXband);
	for (pSet=_xProductList.begin(); pSet!=_xProductList.end(); pSet++) 
	{
		// Set up the color map.
		// First get the scales.
		double scaleMin = _ppiInfo[*pSet].getScaleMin();
		double scaleMax = _ppiInfo[*pSet].getScaleMax();
		// now get the name
		std::string mapName = _ppiInfo[*pSet].getColorMapName();
		// create a copy of the map in our collection which has this name
		ColorMap* pMap = new ColorMap(_colorMaps[mapName]);
		// and set its range.
		pMap->setRange(scaleMin, scaleMax);
		// Finally save the pointer to the colormap in this variable's _ppiInfo entry.
		int index = _ppiInfo[*pSet].getPpiIndex();
		_mapsXband[index] = pMap;
	}

	// connect the control buttons
	ZoomFactor->display(1.0);
	_zoomFactor = _config.getDouble("zoomFactor", 1.2);
	_ringsCheckBox->setCheckState(Qt::Checked);
	_gridsCheckBox->setCheckState(Qt::Unchecked);

	connect(_zoomInButton,  SIGNAL(released()),        this, SLOT(zoomInSlot()));
	connect(_zoomOutButton, SIGNAL(released()),        this, SLOT(zoomOutSlot()));
	connect(_ringsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ringStateChanged(int)));
	connect(_gridsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(gridStateChanged(int)));
	connect(_colorBar,      SIGNAL(released()),        this, SLOT(colorBarReleasedSlot()));
	connect(_colorButton,   SIGNAL(released()),        this, SLOT(backColorButtonReleasedSlot()));
	connect(_ringColorButton,   SIGNAL(released()),    this, SLOT(ringColorButtonReleasedSlot()));
	connect(_saveButton,    SIGNAL(released()),        this, SLOT(saveImageSlot()));
	connect(_resetButton,   SIGNAL(released()),        this, SLOT(resetButtonReleasedSlot()));

	_ppiS->backgroundColor(_backColor);
	_ppiX->backgroundColor(_backColor);

	ppiTypeSlot(PROD_S_DBZ);
	_ppiStack->setCurrentIndex(0);

	// start the statistics timer
	startTimer(_statsUpdateInterval*1000);

}


///////////////////////////////////////////////////////////////////////

CP2PPI::~CP2PPI() 
{

	if (_pSocket)
		delete _pSocket;

	if (_pSocketBuf)
		delete [] _pSocketBuf;

	for (int i = 0; i < _mapsSband.size(); i++)
		delete _mapsSband[i];
}

//////////////////////////////////////////////////////////////////////
void
CP2PPI::configureForGates() 
{
	// Note that the following call determines whether PPI will 
	// use preallocated or dynamically allocated beams. If a third
	// parameter is specifiec, it will set the number of preallocated
	// beams.
	// The configure must be called after initPlots(), bcause
	// that is when _nVarsSband and _nVarsXband are determined,
	// so that we have a count of variables.
	int sNbeams   = _config.getInt("Sband/numberOfBeams", 360);
	_ppiS->configure(_nVarsSband, _sGates, sNbeams, _sGateWidthKm*2.0*_sGates, _ppiGateDecimation);
	_ppiS->grids(_gridsCheckBox->isChecked());
	_ppiS->rings(_ringsCheckBox->isChecked());

	int xNbeams   = _config.getInt("Xband/numberOfBeams", 360);
	_ppiX->configure(_nVarsXband, _xGates, xNbeams, _xGateWidthKm*2.0*_xGates, _ppiGateDecimation);
	_ppiX->grids(_gridsCheckBox->isChecked());
	_ppiX->rings(_ringsCheckBox->isChecked());

	// allocate the beam data arrays
	_beamSdata.resize(_nVarsSband);
	for (int i = 0; i < _nVarsSband; i++) {
		_beamSdata[i].resize(_sGates);
	}

	_beamXdata.resize(_nVarsXband);
	for (int i = 0; i < _nVarsXband; i++) {
		_beamXdata[i].resize(_xGates);
	}
}

//////////////////////////////////////////////////////////////////////
void 
CP2PPI::newDataSlot()
{
	int	readBufLen = _pSocket->readDatagram((char *)_pSocketBuf, sizeof(short)*1000000);

	if (readBufLen > 0) {
		// put this datagram into a packet
		bool packetBad = packet.setProductData(readBufLen, _pSocketBuf);

		// Extract the products and process them.
		// From here on out, we are divorced from the
		// data transport.
		if (!packetBad) {
			for (int i = 0; i < packet.numProducts(); i++) {
				CP2Product* pProduct = packet.getProduct(i);
				// do all of the heavy lifting for this pulse
				processProduct(pProduct);
			} 
		}
	} else {
		// read error. What should we do here?
	}
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::processProduct(CP2Product* pProduct) 
{

	// fill _productSdata and _productXdata
	// from the incoming product. Once we
	// have collected all products fro either S band
	// or X band, display the beam.
	PRODUCT_TYPES prodType = pProduct->header.prodType;
	long long beamNum      = pProduct->header.beamNum;
	double az              = pProduct->header.az;
	int gates              = pProduct->header.gates;
	double gateWidthKm     = pProduct->header.gateWidthKm;

	double el = pProduct->header.el;

	if (_sProductList.find(prodType) != _sProductList.end()) {
		// product is one we want for S band
		// check for a configuration change
		if (gates       != _sGates || gateWidthKm != _sGateWidthKm) {
			_sGates = gates;
			_sGateWidthKm = gateWidthKm;
			configureForGates();
		}
		if (beamNum != _currentSbeamNum) {
			// beam number has changed; start fresh
			_currentSbeamNum = beamNum;
			_currentSproducts.clear();
		}
		_currentSproducts.insert(prodType);
		int index = _ppiInfo[prodType].getPpiIndex();
		for (int i = 0; i < _sGates; i++) {
			_beamSdata[index][i] = pProduct->data[i];
		}
		if (_sProductList.size() == _currentSproducts.size()) {
			// all products have been collected, display the beam
			displaySbeam(az, el);
			// reset the beam number tracking so that we 
			// will start a new colection of products
			_currentSbeamNum = 0;
		}
	} else {
		if (_xProductList.find(prodType) != _xProductList.end()) {
			// product is one that we want for X band
			// check for a configuration change
			if (gates != _xGates || gateWidthKm != _xGateWidthKm) {
				_xGates = gates;
				_xGateWidthKm = gateWidthKm;
				configureForGates();
			}
			if (beamNum != _currentXbeamNum) {
				// beam number has changed; start fresh
				_currentXbeamNum = beamNum;
				_currentXproducts.clear();
			}
			_currentXproducts.insert(prodType);
			int index = _ppiInfo[prodType].getPpiIndex();
			for (int i = 0; i < _xGates; i++) {
				_beamXdata[index][i] = pProduct->data[i];
			}
			if (_xProductList.size() == _currentXproducts.size()) {
				// all products have been collected, display the beam
				displayXbeam(az, el);
				// reset the beam number tracking so that we 
				// will start a new colection of products
				_currentXbeamNum = 0;
			}
		} else {
			// unwanted product
			// std::cout << "U product " << prodType << "   " << beamNum << "\n";			
		}
	}
}

//////////////////////////////////////////////////////////////////////
void
CP2PPI::initColorMaps()	
{

	// get the builtin maps. Note that we are expecting there to
	// be one named default.
	std::vector<std::string> mapNames = ColorMap::builtinMaps();
	for (int i = 0; i < mapNames.size(); i++) {
		_colorMaps[mapNames[i]] = ColorMap(0.0, 1.0, mapNames[i]);
	}

	// get the names of all existing colormaps saved in the configuration.
	std::vector<std::string> names = _config.childGroups("ColorMaps");

	std::vector<std::vector<int> > emptyMap;
	// save colormaps for each of these choices.
	for (int i = 0; i < names.size(); i++) {
		std::vector<std::vector<int> > colors;
		std::string mapKey("ColorMaps/");
		mapKey += names[i];
		colors = _config.getArray(mapKey, "RGB", emptyMap);
		// create the map. Use a bogus range, which will be reset 
		// when the map is used by a variable.
		// also note that this can allow ua to overide any of
		// the builtin maps.
		if (colors.size() != 0) {
			_colorMaps[names[i]] = ColorMap(0.0, 1.0, colors);
		}
	}

}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::initSocket()	
{
	// get the interface specification
	std::string interfaceNetwork;
	interfaceNetwork = _config.getString("Network/ProductNetwork", "192.168.1");
	_productPort = _config.getInt("Network/ProductPort", 3200);

	// create the incoming product socket
	_pSocket = new CP2UdpSocket(interfaceNetwork, _productPort, false, 0, 10000000);

	if (!_pSocket->ok()) {
		QMessageBox e;
		e.warning(this, "Error",_pSocket->errorMsg().c_str(), 
			QMessageBox::Ok, QMessageBox::NoButton);
		return;
	}

	// display the socket specifics
	QString ip(_pSocket->toString().c_str());
	_textIP->setText(ip);
	_textPort->setNum(_productPort);

	_pSocketBuf = new char[1000000];

	connect(_pSocket, SIGNAL(readyRead()), this, SLOT(newDataSlot()));
}
////////////////////////////////////////////////////////////////////
void
CP2PPI::ppiTypeSlot(int newPpiType)
{
	if (newPpiType == _ppiSType) {
		return;
	}

	_ppiSType  = (PRODUCT_TYPES)newPpiType;
	int index = _ppiInfo[_ppiSType].getPpiIndex();
	if (_sProductList.find(_ppiSType)!=_sProductList.end()){
		_ppiS->selectVar(index);
		_colorBar->configure(*_mapsSband[index]);	
		ZoomFactor->display(_ppiS->getZoom());
	} else {
		_ppiX->selectVar(index);
		_colorBar->configure(*_mapsXband[index]);
		ZoomFactor->display(_ppiX->getZoom());
	}
}
////////////////////////////////////////////////////////////////////
void
CP2PPI::initPlots()
{

	// set the initial plot type
	_ppiSType = PROD_S_DBMHC;

	_sProductList.insert(PROD_S_DBMHC);
	_sProductList.insert(PROD_S_DBMVC);
	_sProductList.insert(PROD_S_DBZ);
	_sProductList.insert(PROD_S_WIDTH);
	_sProductList.insert(PROD_S_VEL);
	_sProductList.insert(PROD_S_SNR);
	_sProductList.insert(PROD_S_RHOHV);
	_sProductList.insert(PROD_S_PHIDP);
	_sProductList.insert(PROD_S_ZDR);

	_xProductList.insert(PROD_X_DBMHC);
	_xProductList.insert(PROD_X_DBMVX);
	_xProductList.insert(PROD_X_DBZ);
	_xProductList.insert(PROD_X_SNR);
	_xProductList.insert(PROD_X_LDR);

	int ppiVarIndex = 0;
	setPpiInfo(PROD_S_DBMHC, "S_DBMHC", "H Dbm",    "Sh: Dbm",     -70.0,   0.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_DBMVC, "S_DBMVC", "V Dbm",    "Sv: Dbm",     -70.0,   0.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_DBZ,   "S_DBZ",   "Dbz",      "S: Dbz",      -70.0,   0.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_WIDTH, "S_WIDTH", "Width",    "S:  Width",     0.0,   5.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_VEL,   "S_VEL",   "Velocity", "S:  Velocity",-20.0,  20.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_SNR,   "S_SNR",   "SNR",      "S:  SNR",     -40.0,  10.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_RHOHV, "S_RHOHV", "Rhohv",    "S:  Rhohv",     0.0,   1.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_PHIDP, "S_PHIDP", "Phidp",    "S:  Phidp",  -180.0, 180.0,   ppiVarIndex++);
	setPpiInfo(PROD_S_ZDR,   "S_ZDR",   "Zdr",      "S:  Zdr",     -70.0,   0.0,   ppiVarIndex++);
	// restart the X band ppi indices at 0
	ppiVarIndex = 0;
	setPpiInfo(PROD_X_DBMHC,"X_DBMHC",  "H Dbm", "Xh: Dbm",       -70.0,   0.0,   ppiVarIndex++);
	setPpiInfo(PROD_X_DBMVX,"X_DBMVX",  "V DbM", "Xv: Dbm",       -70.0,   0.0,   ppiVarIndex++);
	setPpiInfo(PROD_X_DBZ,  "X_DBZ",    "Dbz",   "X: Dbz",        -70.0,   0.0,   ppiVarIndex++);
	setPpiInfo(  PROD_X_SNR,"X_SNR",    "SNR",   "Xh: SNR",       -40.0,   0.0,   ppiVarIndex++);
	setPpiInfo(  PROD_X_LDR,"X_LDR",    "LDR",   "Xhv:LDR",         0.0,   1.0,   ppiVarIndex++);

	_typeTab->removeTab(0);
	// add tabs, and save the button group for
	// for each tab.
	QButtonGroup* pGroup;

	pGroup = addPlotTypeTab("S", _sProductList);
	_tabButtonGroups.push_back(pGroup);

	pGroup = addPlotTypeTab("X", _xProductList);
	_tabButtonGroups.push_back(pGroup);

	connect(_typeTab, SIGNAL(currentChanged(QWidget *)), 
		this, SLOT(tabChangeSlot(QWidget*)));
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::tabChangeSlot(QWidget* w) 
{
	// find out the index of the current page
	int pageNum = _typeTab->currentIndex();

	// get the radio button id of the currently selected button
	// on that page.
	int plotType = _tabButtonGroups[pageNum]->checkedId();

	// change the plot type
	ppiTypeSlot(plotType);

	if (pageNum == 0) {
		_ppiStack->setCurrentIndex(0);
		_ppiSactive = true;
	} else {
		_ppiStack->setCurrentIndex(1);
		_ppiSactive = false;
	}
}

//////////////////////////////////////////////////////////////////////
QButtonGroup*
CP2PPI::addPlotTypeTab(std::string tabName, std::set<PRODUCT_TYPES> types)
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
		int id = _ppiInfo[*i].getId();
		QRadioButton* pRadio = new QRadioButton;
		const QString label = _ppiInfo[*i].getLongName().c_str();
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
	connect(pGroup, SIGNAL(buttonReleased(int)), this, SLOT(ppiTypeSlot(int)));

	return pGroup;
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::timerEvent(QTimerEvent*) 
{

}


///////////////////////////////////////////////////////////////////////

void CP2PPI::zoomInSlot()
{
	if (_ppiSactive) {
		_ppiS->setZoom(_ppiS->getZoom()*_zoomFactor);
		ZoomFactor->display(_ppiS->getZoom());
	} else {
		_ppiX->setZoom(_ppiX->getZoom()*_zoomFactor);
		ZoomFactor->display(_ppiX->getZoom());
	}
}

///////////////////////////////////////////////////////////////////////

void CP2PPI::zoomOutSlot()
{
	if (_ppiSactive) {
		_ppiS->setZoom(_ppiS->getZoom()/_zoomFactor);
		ZoomFactor->display(_ppiS->getZoom());
	} else {
		_ppiX->setZoom(_ppiX->getZoom()/_zoomFactor);
		ZoomFactor->display(_ppiX->getZoom());
	}
}


///////////////////////////////////////////////////////////////////////
void
CP2PPI::pauseSlot(bool flag)
{
	_pause = flag;
}

///////////////////////////////////////////////////////////////////////
void
CP2PPI::saveImageSlot()
{
	// get the image form the currently displayed ppi
	QImage* ppiImage;
	if (_ppiSactive) {
		ppiImage = _ppiS->getImage();
	} else {
		ppiImage = _ppiX->getImage();
	}
	// get the color bar image
	QImage* colorBarImage = _colorBar->getImage();

	// we need a painter with a good font
	QFont font(_config.getString("Image/font", "Helvetica").c_str(),
		_config.getInt("Image/pointSize", 14));
	QFontMetrics fm(font);

	// create a composite pixmap with three elements:
	// a label box across the top, containing annotation
	// the ppi, lower left
	// the colorbar, lower right
	int textHeight = fm.height();
	int hlabel = 2.5 * textHeight;;
	int wppi  = ppiImage->width();
	int hppi = ppiImage->height();
	int wcolorbar = colorBarImage->width();
	int hcolorbar = colorBarImage->height();

	QString textColor = _config.getString("Image/textColor", "blue").c_str();
	// the total height will be max(ppi, colorbar) + label
	int h = hppi;
	if (h < hcolorbar)
		h = hcolorbar;
	h += hlabel;

	// the total width will be ppi + colorbar
	int w = wppi + wcolorbar;

	// create the pixmap and painter for it
	QPixmap pm(w, h);
	QPainter painter(&pm);
	painter.setFont(font);
	
	// gradient fill 
	QString colorOne = _config.getString("Image/captionColorOne", "wheat").c_str();
	QString colorTwo = _config.getString("Image/captionColorTwo", "plum").c_str();
	QRadialGradient grad(QPointF(w/10, 0), w/40);
	grad.setColorAt(0,   QColor(colorOne));
	grad.setColorAt(1, QColor(colorTwo));
	grad.setSpread(QGradient::ReflectSpread);
	QBrush gradBrush(grad);
	painter.fillRect(0, 0, w, h, gradBrush);

	// copy in the ppi
	painter.fillRect(0, hlabel, wppi, hppi, _backColor);
	painter.drawImage(0, hlabel, *ppiImage);

	// add the color bar on the right
	painter.drawImage(wppi, hlabel, *colorBarImage);

	// add text to caption
	QRect textRect(0, 0, w-wcolorbar, hlabel);
	painter.setPen(QColor(textColor));
	painter.drawText(textRect, Qt::AlignHCenter | Qt::AlignVCenter, 
		makeCaption(), &textRect);

	// get the directory that it will be saved in.
	QString f = _config.getString("Image/saveDirectory", "c:/").c_str();
	
	// configure the file save dialog
	QFileDialog d(this, tr("Save CP2PPI Image"),
		f, tr("PNG files (*.png);;All files (*.*)"));
	d.setFileMode(QFileDialog::AnyFile);
	d.setViewMode(QFileDialog::Detail);
	d.setAcceptMode(QFileDialog::AcceptSave);
	d.setConfirmOverwrite(true);
	d.setDefaultSuffix("png");
	d.setDirectory(f);
	f = "CP2PPI-";
	f += QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
	f += ".png";
	d.selectFile(f);

	// popup the file save dialog
	if (d.exec()) {
		// if success, save the image to the specified loaction.
		QStringList saveNames = d.selectedFiles();
		pm.save(saveNames[0], "PNG", 100);
		// and preserve the (possibly) new directory location
		f = d.directory().absolutePath();
		_config.setString("Image/saveDirectory", f.toStdString());
	}

	// return the images
	delete ppiImage;
	delete colorBarImage;

}
//////////////////////////////////////////////////////////////////////
QString
CP2PPI::makeCaption() 
{
	QString caption = _config.getString("Image/caption", "CP2PPI").c_str();
	caption += "\n";
	caption += _config.getString("Image/subCaption", "").c_str();
	caption += "          ";
	// get the currently selected product type
	PRODUCT_TYPES plotType = currentProductType();
	// add the product title
	caption += _ppiInfo[plotType].getLongName().c_str();
	caption += "          ";
	caption += QDateTime::currentDateTime().toUTC().toString("dd-MMM-yyyy hh:mm:ss");
	caption += " UTC";

	return caption;
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::displaySbeam(double az, double el)
{
	if (!_pause) {
		int gates = _beamSdata[0].size();
		// convert meterological angle to cartessian angle
		double cartAz = 450 - az;
		if (cartAz < 0) 
			cartAz += 360.0;
		else
			if (cartAz > 360)
				cartAz -= 360.0;
		_ppiS->addBeam(cartAz - 0.5, cartAz + 0.5, gates, _beamSdata, 1, _mapsSband);
		if (_ppiSactive) {
			_azLCD->display(az);
			_elLCD->display(el);
		}
	}
}
//////////////////////////////////////////////////////////////////////
void 
CP2PPI::displayXbeam(double az, double el)
{
	if (!_pause) {
		int gates = _beamXdata[0].size();
		// convert meterological angle to cartessian angle
		double cartAz = 450 - az;
		if (cartAz < 0) 
			cartAz += 360.0;
		else
			if (cartAz > 360)
				cartAz -= 360.0;
		_ppiX->addBeam(cartAz - 0.5, cartAz + 0.5, gates, _beamXdata, 1, _mapsXband);
		if (!_ppiSactive) {
			_azLCD->display(az);
			_elLCD->display(el);
		}
	}
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::colorBarReleasedSlot()
{
	// get the currently selected plot type
	PRODUCT_TYPES plotType = currentProductType();

	// get the current settings
	double min = _ppiInfo[plotType].getScaleMin();
	double max = _ppiInfo[plotType].getScaleMax();
	std::string currentName = _ppiInfo[plotType].getColorMapName();

	// create the color bar settings dialog
	std::vector<std::string> mapNames;
	for (std::map<std::string, ColorMap>::iterator i = _colorMaps.begin();
		i != _colorMaps.end(); i++) {
			mapNames.push_back(i->first);
	}
	_colorBarSettings = new ColorBarSettings(min, max, currentName, mapNames, this);

	// connect the finished slot so that the dialog status 
	// can be captuyred when the dialog closes
	connect(_colorBarSettings, SIGNAL(finished(int)), 
		this, SLOT(colorBarSettingsFinishedSlot(int)));

	// and show it
	_colorBarSettings->show();

}
//////////////////////////////////////////////////////////////////////////////
void 
CP2PPI::colorBarSettingsFinishedSlot(int result)
{
	// see if the OK button was hit
	if (result == QDialog::Accepted) {
		// get the scale values from the settings dialog
		double scaleMin = _colorBarSettings->getMinimum();
		double scaleMax = _colorBarSettings->getMaximum();

		// if the user inverted the values, swap them
		if (scaleMin > scaleMax) 
		{
			double temp = scaleMax;
			scaleMax = scaleMin;
			scaleMin = temp;
		}

		// get the map name
		std::string newMapName = _colorBarSettings->getMapName();

		// find out what product is currently displayed
		// (it might not be the one selected when the
		// dialog was acitvated, but that's okay)
		PRODUCT_TYPES plotType = currentProductType();
		// and reconfigure the color bar
		int index = _ppiInfo[plotType].getPpiIndex();
		// save the new map name
		_ppiInfo[plotType].setColorMapName(newMapName);

		// configure the color bar with the new map and ranges.
		if (_sProductList.find(_ppiSType)!=_sProductList.end())
		{
			// get rid of the existing map
			delete _mapsSband[index];
			// create a new map
			ColorMap* newMap = new ColorMap(_colorMaps[newMapName]);
			_mapsSband[index] = newMap;
			// set range on the new color map
			_mapsSband[index]->setRange(scaleMin, scaleMax);
			// configure the color bar with it
			_colorBar->configure(*_mapsSband[index]);
		} else {
			// get rid of the existing map
			delete _mapsXband[index];
			// create a new map
			ColorMap* newMap = new ColorMap(_colorMaps[newMapName]);
			_mapsXband[index] = newMap;
			// set range on the color map
			_mapsXband[index]->setRange(scaleMin, scaleMax);
			// configure the color bar with it
			_colorBar->configure(*_mapsXband[index]);
		}
		// assign the new scale values to the current product
		_ppiInfo[plotType].setScale(scaleMin, scaleMax);
		// save the new values in the configuration
		// create the configuration keys
		std::string key = _ppiInfo[plotType].getKey();
		std::string minKey = key + "/min";
		std::string maxKey = key + "/max";
		std::string mapKey = key + "/colorMap";

		// set the configuration values
		_config.setDouble(minKey, scaleMin);
		_config.setDouble(maxKey, scaleMax);
		_config.setString(mapKey, newMapName);
	}
}
//////////////////////////////////////////////////////////////////////
PRODUCT_TYPES
CP2PPI::currentProductType()
{
	// find out the index of the current page
	int pageNum = _typeTab->currentIndex();

	// get the radio button id of the currently selected button
	// on that page.
	PRODUCT_TYPES plotType = (PRODUCT_TYPES)_tabButtonGroups[pageNum]->checkedId();

	return plotType;
}
//////////////////////////////////////////////////////////////////////
void 
CP2PPI::setPpiInfo(PRODUCT_TYPES t, 
				   std::string key,             
				   std::string shortName,       
				   std::string longName,        
				   double defaultScaleMin,      
				   double defaultScaleMax,      
				   int ppiVarIndex)
{
	// create the configuration keys
	std::string minKey = key;
	minKey += "/min";

	std::string maxKey = key;
	maxKey += "/max";

	std::string mapKey = key;
	mapKey += "/colorMap";

	// get the configuration values
	double min = _config.getDouble(minKey, defaultScaleMin);
	double max = _config.getDouble(maxKey, defaultScaleMax);
	std::string mapName = _config.getString(mapKey, "default");
	if (_colorMaps.find(mapName) == _colorMaps.end()) {
		mapName = "default";
	}

	// set the ppi configuration
	_ppiInfo[t] = PpiInfo(t, key, shortName, longName, mapName, min, max, ppiVarIndex);

	_config.sync();

}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::ringStateChanged(int state) {
	_ppiS->rings(state == Qt::Checked? true : false);
	_ppiX->rings(state == Qt::Checked? true : false);
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::gridStateChanged(int state) {
	_ppiS->grids(state == Qt::Checked? true : false);
	_ppiX->grids(state == Qt::Checked? true : false);
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::backColorButtonReleasedSlot() {
	_backColor = QColorDialog::getColor("blue");

	_ppiS->backgroundColor(_backColor);
	_ppiX->backgroundColor(_backColor);
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::ringColorButtonReleasedSlot() {
	_ringsGridColor = QColorDialog::getColor("black");

	_ppiS->gridRingsColor(_ringsGridColor);
	_ppiX->gridRingsColor(_ringsGridColor);
}
//////////////////////////////////////////////////////////////////////
void
CP2PPI::resetButtonReleasedSlot() {
	if (_ppiSactive) {
		_ppiS->setZoom(1.0);
		ZoomFactor->display(_ppiS->getZoom());
	} else {
		_ppiX->setZoom(1.0);
		ZoomFactor->display(_ppiX->getZoom());
	}
}
