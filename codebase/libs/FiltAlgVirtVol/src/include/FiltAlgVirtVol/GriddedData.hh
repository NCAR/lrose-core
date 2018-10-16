/**
 * @file GriddedData.hh
 * @brief One named data field, 2d
 * @class GriddedData
 * @brief One named data field, 2d
 */

#ifndef GRIDDED_DATA_HH
#define GRIDDED_DATA_HH

#include <euclid/Grid2d.hh>
#include <rapmath/MathLoopData.hh>
#include <ctime>
#include <vector>

//------------------------------------------------------------------
class GriddedData : public Grid2d, public MathLoopData
{
public:

  /**
   * Constructor
   */
  GriddedData(void);

  /**
   * Constructor
   * @param[in] g  Grid2d to copy in
   */
  GriddedData(const Grid2d &g);

  /**
   * Destructor
   */
  virtual ~GriddedData(void);


  #include <rapmath/MathLoopDataVirtualMethods.hh>

protected:
private:

};

#endif
