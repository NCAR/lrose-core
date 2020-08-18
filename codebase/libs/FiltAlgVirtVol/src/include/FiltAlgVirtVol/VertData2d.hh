/**
 * @file VertData2d.hh 
 * @brief A 2 dimensional grid as special MathUserData 
 * @class VertData2d
 * @brief A 2 dimensional grid as special MathUserData 
 */

#ifndef VertData2d_hh
#define VertData2d_hh

#include <euclid/Grid2d.hh>
#include <rapmath/MathUserData.hh>
#include <string>
#include <vector>

//------------------------------------------------------------------
class VertData2d : public MathUserData
{
public:

  /**
   * Empty constructor
   */
  VertData2d();

  /**
   * Constructor
   * @param[in] g  The grid to store locally
   * @param[in] vlevel the Vertical level to store locally 
   */
  VertData2d(const Grid2d &g, double vlevel);

  /**
   * Destructor
   */
  inline virtual ~VertData2d(void) {}


  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return true if object well formed.
   */
  inline bool ok(void) const {return _ok;}

  /**
   * @return vertical level
   */
  inline double getVlevel(void) const {return _vlevel;}

  /**
   * @return Reference to the local grid
   */

  inline const Grid2d &constGridRef(void) const {return _data;}

  /**
   * Filter by keeping congtiguous clumps that have
   * at least pct percent of the points >= thresh.  Set all other
   * clumps to missing (remove them)
   * @param[in] thresh  The threshold to use in the test
   * @param[in] pct  The minimum percent
   * @rerturn a new VertData2d pointer with filtered data
   * The returned pointer is owned by the caller
   */
  VertData2d *clump(double thresh, double pct) const;

protected:
private:

  bool _ok;         /**< True if object well formed */
  double _vlevel;   /**< The vertical level (degrees) */
  Grid2d _data;     /**< The 2 dimensional grid */
};

#endif
 
