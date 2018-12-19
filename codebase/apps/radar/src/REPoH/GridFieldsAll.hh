/**
 * @file GridFieldsAll.hh
 * @brief Grids for all fields, all heights
 * @class GridFieldsAll
 * @brief Grids for all fields, all heights
 */

#ifndef GRID_FIELDS_ALL_HH
#define GRID_FIELDS_ALL_HH

#include "GridFields.hh"
#include <vector>

//------------------------------------------------------------------
class GridFieldsAll
{
public:
  GridFieldsAll() {}
  ~GridFieldsAll(void) {}
  void clear() { _data.clear();}

  /**
   * @return copy of the vector of gridded data at a height index
   */
  GridFields get2d(int index) const
  {
    return _data[index];
  }

  bool fieldExists(const std::string &name, int zIndex) const
  {
    return _data[zIndex].fieldExists(name);
  }
  
  void addField(const GriddedData &data, int zIndex)
  {
    _data[zIndex].addField(data);
  }
  
  inline size_t size(void) const {return _data.size();}

  void initialize(int nz)
  {
    _data.clear();
    for (int i=0; i<nz; ++i)
    {
      _data.push_back(GridFields(i));
    }
  }
  
  bool sampleMissingValue(const std::string &name, double &missingV) const
  {
    for (size_t i=0; i<_data.size(); ++i)
    {
      if (_data[i].sampleMissingValue(name, missingV))
      {
	return true;
      }
    }
    return false;
  }

  void retrieveVolumeData(const std::string &name, fl32 *fo) const
  {
    for ( size_t z=0; z<_data.size(); ++z)
    {
      _data[z].retrieveSweepData(name, fo);
    }
  }

private:

  std::vector<GridFields> _data;  /**< Gridded fields at each height */
};

#endif
