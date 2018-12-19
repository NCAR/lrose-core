/**
 * @file GridFields.hh
 * @brief Grids for all fields, at one height
 * @class GridFields
 * @brief Grids for all fields, at one height
 */

#ifndef GRID_FIELDS_HH
#define GRID_FIELDS_HH

#include <FiltAlgVirtVol/GriddedData.hh>
#include <toolsa/LogStream.hh>

#include <vector>

//------------------------------------------------------------------
class GridFields
{
public:
  /**
   * No fields
   * @param[in] z  Height index
   */
  GridFields(int z) : _z(z) {}

  ~GridFields(void) {}

  inline const std::vector<GriddedData> &fields(void) const
  {
    return _grid2d;
  }

  bool fieldExists(const std::string &name) const
  {
    for (size_t j=0; j<_grid2d.size(); ++j)
    {
      if (name == _grid2d[j].getName())
      {
	return true;
      }
    }
    return false;
  }

  void addField(const GriddedData &data)
  {
    _grid2d.push_back(data);
  }
  
  GriddedData *refToData(const std::string &name)
  {
    for (size_t j=0; j<_grid2d.size(); ++j)
    {
      if (name == _grid2d[j].getName())
      {
	return &_grid2d[j];
      }
    }
    return NULL;
  }
  
  const GriddedData *refToData(const std::string &name) const
  {
    for (size_t j=0; j<_grid2d.size(); ++j)
    {
      if (name == _grid2d[j].getName())
      {
	return &_grid2d[j];
      }
    }
    return NULL;
  }

  bool sampleMissingValue(const std::string &name, double &missingV) const
  {
    for (size_t i=0; i<_grid2d.size(); ++i)
    {
      if (_grid2d[i].getName() == name)
      {
	missingV = _grid2d[i].getMissing();
	return true;
      }
    }
    return false;
  }

    
  void retrieveSweepData(const std::string &name, fl32 *fo) const
  {
    for (size_t f=0; f<_grid2d.size(); ++f)
    {
      if (_grid2d[f].getName() == name)
      {
	int nxy = _grid2d[f].getNdata();
	for (int i=0; i<nxy; ++i)
	{
	  fo[i+_z*nxy] = _grid2d[f].getValue(i);
	}
	return;
      }
    }
    LOG(WARNING) << "Never found data for " << name;
  }
  
  inline void clear(void) { _grid2d.clear();}

  inline GriddedData &operator[](size_t i) {return _grid2d[i];}
  inline const GriddedData &operator[](size_t i) const {return _grid2d[i];}
  inline size_t size(void) const {return _grid2d.size();}
  
private:

  int _z;                            /**< The height index */
  std::vector<GriddedData> _grid2d;  /**< fields */
};

#endif
