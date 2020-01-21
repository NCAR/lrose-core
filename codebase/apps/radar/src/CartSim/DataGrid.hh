// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  DataGrid.hh
 * @brief  Bookkeeping for a data field from MDV
 * @class  DataGrid
 * @brief  Bookkeeping for a data field from MDV
 *
 * The data pointer is stored locally, not owned locally.
 *
 */

# ifndef    DATAGRID_HH
# define    DATAGRID_HH

#include <dataport/port_types.h>
#include <vector>

//----------------------------------------------------------------
class DataGrid
{
public:

  /**
   * Default constructor (empty)
   */
  DataGrid(void);
  
  /**
   * Default constructor
   *
   * @param[in] nx  Grid dimension
   * @param[in] ny  Grid dimension
   * @param[in] nz  Grid dimension
   * @param[in] missing  Missing data value
   * @param[in] data  Pointer to data array
   */
  DataGrid(const int nx, const int ny, const int nz, const fl32 missing,
	   fl32 *data);
  
  /**
   * Destructor
   */
  virtual ~DataGrid(void);

  /**
   * Set the value at a point into the data grid
   *
   * @param[in] x  Point
   * @param[in] y  Point
   * @param[in] z  Point
   * @param[in] v  Value
   * @param[in] motion  True if this is a 'motion' field, which needs to
   *                    be converted from (external) knots into local
   *                    meters per second
   */
  void setValue(int x, int y, int z, fl32 v, bool motion);

  /**
   * Set data to missing at all points
   */
  void clear(void);

  /**
   * @return data pointer 
   */
  inline fl32 *getPtr(void) { return _data;}

protected:
private:  

  int _nx;   /**< Grid dimension */
  int _ny;   /**< Grid dimension */
  int _nz;   /**< Grid dimension */
  fl32 *_data;  /**< Pointer to data */
  fl32 _missing; /**< missing data value */
};

# endif 
