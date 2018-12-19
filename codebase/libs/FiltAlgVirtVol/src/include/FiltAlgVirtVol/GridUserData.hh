/**
 * @file GridUserData.hh
 * @brief GridUserData data.  A 2d grid treated as MathUserData
 * @class GridUserData
 * @brief GridUserData data.  A 2d grid treated as MathUserData
 */

#ifndef GRID_USER_DATA_HH
#define GRID_USER_DATA_HH

// #include <FiltAlgVirtVol/FiltAlgParms.hh>
// #include <FiltAlgVirtVol/GriddedData.hh>
// #include <rapmath/SpecialUserData.hh>
// #include <rapmath/MathData.hh>
// #include <Mdv/MdvxProj.hh>
// #include <Mdv/MdvxRadar.hh>
// #include <vector>

// class VolumeMdv;
// class Algorithm;

#include <euclid/Grid2d.hh>
#include <rapmath/MathUserData.hh>

//------------------------------------------------------------------
class GridUserData : public MathUserData, public Grid2d
{
public:

  /**
   * Empty constructor
   */
  GridUserData(void);

  /**
   * Constructor
   * @param[in] grid  The Grid2d to use
   * @param[in] name  The Name to give it
   */
  GridUserData(const Grid2d &grid, const std::string &name);

  /**
   * Destructor
   */
  virtual ~GridUserData(void);


  #include <rapmath/MathUserDataVirtualMethods.hh>


protected:
private:
};

#endif
