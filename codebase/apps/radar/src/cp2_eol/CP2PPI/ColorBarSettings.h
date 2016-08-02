#ifndef COLORBARSETTINGSINC_
#define COLORBARSETTINGSINC_

#include "ui_ColorBarSettings.h"

/// Use Ui::ColorBarSettings to present a dialog
/// for setting the color bar properties
class ColorBarSettings: public QDialog, private Ui::ColorBarSettings 
{
public:
	/// Constructor
	ColorBarSettings(
		double min, 
		double max, 
		std::string currentMap,
		std::vector<std::string> mapNames, 
		QWidget* parent=0);
	/// Destructor
	virtual ~ColorBarSettings();
	/// @returns The minimum value
	double getMinimum();
	/// @returns
	double getMaximum();
	/// @returns The selected color map
	std::string getMapName();

protected:
};

#endif
