/** @page cp2ppi-overview The CP2PPI program

CP2PPI provides a traditional real-time PPI display of CP2
beam data. Both of thes products are read from the network.
CP2PPI is configured via CP2PPI.ini.
**/

#ifndef CP2PPIH
#define CP2PPIH
#ifdef WIN32
#include <winsock2.h>		//	no redefinition errors if before Qt includes?
#endif
#include <QUdpSocket> 
#include <QEvent>
#include <QButtonGroup>
#include <QColor>

#include <deque>
#include <set>
#include <map>
#include <string>

#include "CP2UdpSocket.h"

// request this much space for the socket receive buffer
#define CP2PPI_RCVBUF 100000000

// The UI class created from the designer .ui specification
#include "ui_CP2PPI.h"

// Components from the QtToolbox
#include <PPI/PPI.h>
#include <ColorBar/ColorMap.h>
#include <ColorBar/ColorBar.h>

// CP2 timeseries network transfer protocol.
#include "CP2Net.h"
using namespace CP2Net;

// ProductInfo knows the characteristics of a particular 
// PPI product display
#include "PpiInfo.h"

// Configuration maintenance
#include "CP2Config.h"

// The color bar setting dialog
#include "ColorBarSettings.h"

#define PIRAQ3D_SCALE	1.0/(unsigned int)pow(2,31)	

class CP2PPI : public QDialog, public Ui::CP2PPI {
	Q_OBJECT
public:
	/// Constructor
	CP2PPI(QDialog* parent = 0);
	/// Destructor
	~CP2PPI();

public slots:
	/// Called when data is available on the data socket.
	void newDataSlot();
	/// Called when the product type is changed.
	virtual void ppiTypeSlot(int ppiType);
	/// Called when the user changes between the tabs.
	void tabChangeSlot(QWidget* w);
	/// pause the display  update
	void pauseSlot(bool flag);	
	/// To zoom in one level
    void zoomInSlot();
	/// To zoom out one level
    void zoomOutSlot();
	/// Activated when a mouse click is released for the color bar.
	void colorBarReleasedSlot();
	/// Activated when the ColorBarSettings dialog is finished.
	/// @param result The dialog result code
	void colorBarSettingsFinishedSlot(int result);
	/// Called when the Color button has been pushed to activate background color choosing.
	void backColorButtonReleasedSlot();
	/// Called when the Color button has been pushed to activate ring/grid color choosing.
	void ringColorButtonReleasedSlot();
	/// Called then the ring choice has changed state.
	void ringStateChanged(int state);
	/// Called then the grid choice has changed state.
	void gridStateChanged(int state);
	/// Save the PPI image.
	void saveImageSlot();
	/// reset the zoom
	void resetButtonReleasedSlot();

protected:
	/// The configuration for CP2PPI
	CP2Config _config;
	/// intialize the socket that the product data 
	/// is received on
	void initSocket(); 
	/// initialize all of the book keeping structures
	/// for the various plots.
	void initPlots();
	/// Initialize the color maps.
	void initColorMaps();
	/// Process the products as they come in. 
	/// Sband and X band products with identical 
	/// beam numbers are collected until a full
	/// set of products are received for a given display.
	/// The full set of S or X products	is then sent to the display.
	/// If a complete set is not collected for a given
	/// beam number, it will not be displayed.
	void processProduct(CP2Product* pProduct);
	/// Configure the ppi displays and the beam data vectors for
	/// for the number of gates, number of beams and number of variables.
	/// Call this function when any of these change in order to reconfigure
	/// the display.
	void configureForGates();
	/// Configure the PpiInfo entry for a product, 
	/// getting values from the configuration.
	void setPpiInfo(PRODUCT_TYPES t, ///< The product type
		std::string key,             ///< The key to use in the configuration
		std::string shortName,       ///< Short name
		std::string longName,        ///< Long name
		double defaultScaleMin,      ///< Colorbar minimum
		double defaultScaleMax,      ///< Colorbar maximum
		int ppiVarIndex              ///< Variable index on the selection tab
		);    
	/// add a tab to the plot type selection tab widget.
	/// Radio buttons are created for all of specified
	/// plty types, and grouped into one button group.
	/// _ppiInfo provides the label information for
	/// the radio buttons.
	/// @param tabName The title for the tab.
	/// @param types A set of the desired PRODUCT_TYPES types 
	/// @return The button group that the inserted buttons
	/// belong to.
	QButtonGroup* addPlotTypeTab(std::string tabName, std::set<PRODUCT_TYPES> types);
	/// Add a beam of S band products to the ppi
	void displaySbeam(double az, double el);
	/// Add a beam of X band products to the ppi
	void displayXbeam(double az, double el);
	/// @returns The plot index for the plot selected on
	/// the current tab page.
	PRODUCT_TYPES currentProductType();
	/// @returns A caption for the saved image
	QString makeCaption();
	/// The incoming product socket.
	CP2UdpSocket*   _pSocket;
	/// The buffer that the product data will be read into
	/// from the socket.
	char*   _pSocketBuf;
	/// Incoming produt port number.
	int	_productPort;
	// how often to update the statistics (in seconds)
	int _statsUpdateInterval;
	/// the currently selected display type
	PRODUCT_TYPES _ppiSType;
	// The builtin timer will be used to calculate beam statistics.
	void timerEvent(QTimerEvent*);
	/// For each PRODUCT_TYPES, there will be an entry in this map.
	std::map<PRODUCT_TYPES, PpiInfo> _ppiInfo;
	/// This set contains PRODUCT_TYPESs identifiers for all desired 
	/// S band moments plots. It is used to filter S products from
	/// the incoming data stream.
	std::set<PRODUCT_TYPES> _sProductList;
	/// This set contains PRODUCT_TYPESs identifiers for all desired 
	/// X band moments plots. It is used to filter X products from
	/// the incoming data stream.
	std::set<PRODUCT_TYPES> _xProductList;
	/// This set contains the list of all received S band products 
	/// that are on the desired list, and have the same beam id.
	/// When the length reaches the same size as _sProductList, 
	/// then we have all products for a given S band beam.
	std::set<PRODUCT_TYPES> _currentSproducts;
	/// This set contains the list of all received X band products 
	/// that are on the desired list, and have the same beam id.
	/// When the length reaches the same size as _xProductList, 
	/// then we have all products for a given X band beam.
	std::set<PRODUCT_TYPES> _currentXproducts;
	/// The current beam number for S band products, used
	/// for collating S band products as they are received.
	long long _currentSbeamNum;
	/// The current beam number for X band products, used
	/// for collating X band products as they are received.
	long long _currentXbeamNum;
	/// save the button group for each tab,
	/// so that we can find the selected button
	/// and change the plot type when tabs are switched.
	std::vector<QButtonGroup*> _tabButtonGroups;
    /// true to pause the display. Data will still be coming in,
	/// but not sent to the display.
	bool _pause;
	/// The number of S band gates
	int _sGates;
	/// The number of X band gates
	int _xGates;
	/// S band gate width in km
	double _sGateWidthKm;
	/// X band gate width in km
	double _xGateWidthKm;
	/// Set true when the Sband ppi is active,
	/// false when Xband is active.
	bool _ppiSactive;
	/// The number of Sband variables
	int _nVarsSband;
	/// Color maps for each Sband variable
	std::vector<ColorMap*> _mapsSband;
	/// Will hold the beam values for all S band variables in one beam
	std::vector<std::vector<double> > _beamSdata;
	/// The number of X band variables
	int _nVarsXband;
	/// Color maps for each X band variable
	std::vector<ColorMap*> _mapsXband;
	/// Will hold the beamvalues for all X band vbariables in one beam.
	std::vector<std::vector<double> > _beamXdata;
	/// A CP2Packet will be assembled here from the datagram.
	CP2Packet packet;
	/// The dialog that will collect colorbar settings
	ColorBarSettings* _colorBarSettings;
	/// The available ColorMaps. They are individually identified by a name.
	std::map<std::string, ColorMap> _colorMaps;
	/// The zoom multiplication factor applied on each zoom request
	double _zoomFactor;
	/// Decimate the display of gates by this factor. Used to improve
	/// graphics performance
	int _ppiGateDecimation;
	/// The background color
	QColor _backColor;
	/// The rings/grid color
	QColor _ringsGridColor;
  };

#endif

