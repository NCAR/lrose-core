/**
 * @file OldDataHandler.hh
 * @brief OldDataHandler data
 * @class OldDataHandler
 * @brief OldDataHandler data
 */

#ifndef OLD_DATA_HANDLER_HH
#define OLD_DATA_HANDLER_HH

#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <FiltAlgVirtVol/GriddedData.hh>
#include <rapmath/FuzzyF.hh>
#include <toolsa/LogStream.hh>
#include <vector>

//------------------------------------------------------------------
class OldDataAtTime
{
public:
  inline OldDataAtTime(const time_t &t) : _time(t) {}
  inline ~OldDataAtTime(void) {}
  void initialize(const std::string &url, const time_t &t,
		  const std::string &fieldName,  const VirtVolParms &parms);
  inline bool hasGrid(int zIndex) const {return (int)_dataAtHeight.size() > zIndex;}
  inline bool grid2d(int i, Grid2d &g) const
  {
    if (hasGrid(i))
    {
      g = _dataAtHeight[i];
      return true;
    }
    else
    {
      LOG(ERROR) << "tried to access non-existant grid height = " << i;
      return false;
    }
  }

  inline time_t getTime(void) const {return _time;}

private:
  time_t _time;
  std::vector<GriddedData> _dataAtHeight;  // one per height
};

//------------------------------------------------------------------
class OldDataField
{
public:
  inline OldDataField(const std::string &fieldName) : _fieldName(fieldName) {}
  inline ~OldDataField(void) {}
  void initialize(const std::string &url, const std::string &fieldName,
		  const std::vector<time_t> &times, const VirtVolParms &Parms);
  inline size_t size(void) const {return _data.size();}
  inline const OldDataAtTime& operator[](size_t i) const {return _data[i];}

  inline std::string fieldName(void) const {return _fieldName;}

private:
  std::string _fieldName;
  std::vector <OldDataAtTime> _data;
};

//------------------------------------------------------------------
class OldDataAtTimeHeight
{
public:
  OldDataAtTimeHeight(const OldDataAtTime &o, int zIndex);
  inline ~OldDataAtTimeHeight(void) {}
  inline time_t getTime(void) const {return _time;}
  inline bool getData(Grid2d &g) const
  {
    if (_hasData)
    {
      g = _dataAtHeight;
      return true;
    }
    else
    {
      return false;
    }
  }
  bool hasData(void) const {return _hasData;}

private:
  bool _hasData;
  GriddedData _dataAtHeight;
  time_t _time;
};

//------------------------------------------------------------------
/**
 * One grid per old time (at a height for a paritcular field)
 */
class OldDataSweepField
{
public:
  OldDataSweepField(const OldDataField &o, int zIndex);
  inline ~OldDataSweepField(void) {}
  inline bool fieldMatch(const std::string &name) const
  {
    return name == _fieldName;
  }
  bool constructWeightedFields(const time_t &t, const FuzzyF &dtToWt,
			       std::vector<Grid2d> &inps,
			       std::vector<double> &weights) const;
  
  bool constructAgeWithMax(const time_t &t, Grid2d &ageGrid) const;
  
private:
  int _zIndex;
  std::string _fieldName;
  bool _vectorIsSet;
  std::vector <OldDataAtTimeHeight> _data;
};

//-------------------------------------------------------------------
class OldData
{
public:
  inline OldData(void) {}
  inline ~OldData(void) {}
  
  inline size_t size(void) const {return _fields.size();}
  inline const OldDataField& operator[](size_t i) const {return _fields[i];}

  void addField(const time_t &triggerTime, const std::string &fieldName,
		const std::string &url,	const int maxSecondsBack,
		const VirtVolParms &parms);

private:

  std::vector<OldDataField> _fields;
};


//-------------------------------------------------------------------
class OldSweepData
{
public:
  inline OldSweepData(void) {}
  OldSweepData(const OldData &o, int zIndex);
  inline ~OldSweepData(void) {}
  inline size_t size(void) const {return _data.size();}
  inline const OldDataSweepField & operator[](size_t i) const {return _data[i];}

private:

  // one per field with old data
  std::vector<OldDataSweepField> _data;
};

#endif



