#include "PpiInfo.h"

////////////////////////////////////////////////////////
PpiInfo::PpiInfo():
_key(""),
_id(0),
_shortName(""),
_longName(""),
_scaleMin(1),
_scaleMax(10)
{
}

////////////////////////////////////////////////////////
PpiInfo::PpiInfo(int id, 
				 std::string key, 
				 std::string shortName, 
				 std::string longName,
				 std::string colorMapName,
				 double scaleMin, 
				 double scaleMax,
				 int ppiIndex):
_key(key),
_id(id),
_shortName(shortName),
_longName(longName),
_colorMapName(colorMapName),
_scaleMin(scaleMin),
_scaleMax(scaleMax),
_ppiIndex(ppiIndex)
{
}

////////////////////////////////////////////////////////

PpiInfo::~PpiInfo()
{
}

////////////////////////////////////////////////////////
void
PpiInfo::setScale(double min, double max)
{
	_scaleMin = min;
	_scaleMax = max;
}

////////////////////////////////////////////////////////
double 
PpiInfo::getScaleMin()
{
	return _scaleMin;
}

////////////////////////////////////////////////////////
double 
PpiInfo::getScaleMax()
{
	return _scaleMax;
}

////////////////////////////////////////////////////////
std::string 
PpiInfo::getShortName()
{
	return _shortName;
}

////////////////////////////////////////////////////////
std::string 
PpiInfo::getLongName()
{
	return _longName;
}
////////////////////////////////////////////////////////
int 
PpiInfo::getId()
{
	return _id;
}

////////////////////////////////////////////////////////
void 
PpiInfo::setColorMapName(std::string mapName)
{
	_colorMapName = mapName;
}

////////////////////////////////////////////////////////
std::string 
PpiInfo::getColorMapName()
{
	return _colorMapName;
}

////////////////////////////////////////////////////////
std::string 
PpiInfo::getKey()
{
	return _key;
}

////////////////////////////////////////////////////////
int 
PpiInfo::getPpiIndex()
{
	return _ppiIndex;
}





