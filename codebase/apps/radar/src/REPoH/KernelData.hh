/**
 * @file KernelData.hh 
 * @brief derived data associated with a kernel, and status
 *
 * @class KernelData
 * @brief derived data associated with a kernel, and status
 */

#ifndef KERNEL_DATA_H
#define KERNEL_DATA_H

#include "DataStatus.hh"
#include <string>

class KernelGrids;
class KernelPoints;
class RepohParams;

class KernelData
{
public:
  /**
   * Contructor
   */
  KernelData(void);

  /**
   * Destructor
   */
  ~KernelData(void);

  /**
   * @return true if this data passed all the tests that are done
   */
  inline bool passedTests(void) const {return  _passedTests;}

  /**
   * @return local value _D0
   */
  inline double getD0(void) const {return _D0;}

  /**
   * @return local value _corr
   */
  inline double getCorr(void) const {return _corr;}
  
  /**
   * @return a string with status information
   */
  std::string sprintStatus(void) const;

  /**
   * @return a string with debug information
   */
  std::string sprintDebug(void) const;

  /**
   * evaluate (set state) for a kernel at the radar
   */
  void evaluate(void);

  /**
   * evaluate (set state) for a kernel not at the radar
   *
   * @param[in] grids  Data grids used in kernel computations
   * @param[in] P      parameters
   * @param[in] pts    the points in the kernel
   * @param[in] enoughPts  True if there are enough points in the kernel
   * @param[in] nFilt   Number of points in kernel that were filtered out
   */
  void evaluate(const KernelGrids &grids, const RepohParams &P,
		const KernelPoints &pts, bool enoughPts, int nFilt);

  /**
   * @return the attenuation, defined as average sdbz - average kdbz
   */
  double attenuation(void) const;

protected:
private:

  bool _passedTests;          /**< True if all tests were successful */
  
  DataStatus _kernelStatus;    /**< Status of kernel filtering based on 
				*    dbz_diff */
  int _kernel_npt_removed;     /**< Number of filtered out kernel points */

  DataStatus _pidStatus;      /**< Status of pid values within the kernel */

  DataStatus _sdbzAveStatus;  /**< Status of sDBZ averaging */
  double _sdbz_ave;           /**< Average SDBZ value in cloud */
  double _sdbz_ave_out;       /**< Average SDBZ value outside cloud */
  DataStatus _sdbzInOutStatus; /**< Status of comparison between average
				*  in cloud and out of cloud values */
  DataStatus _sdbzRangeStatus; /**< Status of the range of SDBZ values */
  double _sdbz_range;         /**< Max - min SDBZ values in cloud */

  double _kdbz_ave;           /**< Average KDBZ value */
  double _ZDR_ave;            /**< Average ZDR value */
  double _z_ave;              /**< Average linear SDBZ value */

  DataStatus _D0Status;       /**< Status of D0 computaton */
  double _D0;                 /**< D0 */

  DataStatus _corrStatus;     /**< Status of correlation */
  double _corr;               /**< correlation */

};  

#endif
