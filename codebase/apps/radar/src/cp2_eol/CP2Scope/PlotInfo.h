#ifndef PLOTINFOINC_
#define PLOTINFOINC_

#include <string>

class PlotInfo {
public:
	PlotInfo();
	PlotInfo(int id, int displayType, std::string shortName, std::string longName,
		double gainMin, double gainMax, double gainCurrent, 
		double offsetMin, double offsetMax, double offsetCurrent);
	virtual ~PlotInfo();

	int getId();
	int getDisplayType();

	void setGain(double min, double max, double current);
	void setOffset(double min, double Max, double current);

	double getGainMin();
	double getGainMax();
	double getGainCurrent();

	double getOffsetMin();
	double getOffsetMax();
	double getOffsetCurrent();

	std::string getShortName();
	std::string getLongName();

protected:
	int _id;
	int _displayType;
	std::string _shortName;
	std::string _longName;
	double _gainMin;
	double _gainMax;
	double _gainCurrent;
	double _offsetMin;
	double _offsetMax;
	double _offsetCurrent;
};
#endif
