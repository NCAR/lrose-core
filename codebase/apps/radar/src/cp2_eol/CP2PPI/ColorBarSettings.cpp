#include "ColorBarSettings.h"

//////////////////////////////////////////////////////////////////////////////
ColorBarSettings::ColorBarSettings(double min, 
								   double max, 
								   std::string currentMap,
								   std::vector<std::string> mapNames, 
								   QWidget* parent):
QDialog(parent)
{
	setupUi(this);	

	// set the spin box min and max ranges to accept virtually everything
	_minSpin->setMinimum(-1.0e10);
	_minSpin->setMaximum( 1.0e10);
	_maxSpin->setMinimum(-1.0e10);
	_maxSpin->setMaximum( 1.0e10);

	// set the spin box values
	_minSpin->setValue(min);
	_maxSpin->setValue(max);

	// Put the map names in the combo box
	int currentIndex = 0;
	for (int i = 0; i < mapNames.size(); i++) {
		_mapComboBox->insertItem(0, mapNames[i].c_str());
		if (currentMap == mapNames[i])
			currentIndex = i;
	}
	_mapComboBox->setCurrentIndex(mapNames.size()-currentIndex-1);
}

//////////////////////////////////////////////////////////////////////////////
ColorBarSettings::~ColorBarSettings()
{
}
//////////////////////////////////////////////////////////////////////////////
double
ColorBarSettings::getMinimum()
{
	return _minSpin->value();
}
//////////////////////////////////////////////////////////////////////////////
double
ColorBarSettings::getMaximum()
{
	return _maxSpin->value();
}
//////////////////////////////////////////////////////////////////////////////
std::string
ColorBarSettings::getMapName()
{
	QString selectedName = _mapComboBox->currentText();
	return selectedName.toStdString();
}
//////////////////////////////////////////////////////////////////////////////
