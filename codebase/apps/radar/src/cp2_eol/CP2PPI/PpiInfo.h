#ifndef PLOTINFOINC_
#define PLOTINFOINC_

#include <string>

class PpiInfo {
public:
	PpiInfo();
	PpiInfo(int id, 
		std::string key, 
		std::string shortName, 
		std::string longName,
		std::string colorMapName,
		double scaleMin, 
		double scaleMax, 
		int ppiIndex
		);
	virtual ~PpiInfo();

	int getId();
	int getPpiIndex();
	std::string getKey();
	void setScale(double min, double max);
	void setColorMapName(std::string mapName);
	std::string getColorMapName();
	double getScaleMin();
	double getScaleMax();
	std::string getShortName();
	std::string getLongName();


protected:
	int _id;
	std::string _key;
	std::string _shortName;
	std::string _longName;
	double _scaleMin;
	double _scaleMax;
	int _ppiIndex;
	std::string _colorMapName;
};
#endif
