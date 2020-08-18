/**
 * @file ShearDetection.hh
 * @brief line detection done using 3 lookup tables
 * @class ShearDetection
 * @brief line detection done using 3 lookup tables
 *
 * Lookup tables created from Parms as well as a neighborhood offset
 * template to check data values near each point for appropriateness in
 * computing thinline.
 */
# ifndef    ShearDetection_H
# define    ShearDetection_H

#include "Parms.hh"
#include "Window.hh"
#include <vector>
#include <Mdv/MdvxProj.hh>
#include <toolsa/TaThreadDoubleQue.hh>
class ShearDetectOffsets;
class ShearDetect;
class Grid2d;

class ShearDetection
{
public:
  ShearDetection(int nx, int ny, int nthread, const Parms &parms,
		 const MdvxProj &proj);
  ~ShearDetection();
    
  void processDir(const Grid2d *input, const Grid2d *secondary,
		  Grid2d *output);
  void processDet(const Grid2d *input, const Grid2d *secondary,
		  const Grid2d *dir, Grid2d *output);

  /**
   * Compute method needed for threading
   * @param[in] i  Information
   */
  static void compute(void *i);

private:

  /**
   * @class FiltLineThreads
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   * the clone() method.
   */
  class ShearDetectionThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline ShearDetectionThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~ShearDetectionThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(const int index);
  };

  int _nx, _ny, _nthread;
  Window _window;
  Parms _parms;
  MdvxProj _proj;
  double _shearMin;
  double _cutAngle;
  double _testCircleDiameter;
  double _secondaryRange[2];
  bool _isConvergent;

  double _shear_direct_xy(const int x, const int y, const int nx,
			  const int ny, const Grid2d &input,
			  const Grid2d &secondary,
			  const ShearDetect &ld,
			  const ShearDetectOffsets &o) const;
  double _shear_detect_xy(const int x, const int y, const int nx,
			  const int ny, const Grid2d &input,
			  const Grid2d &secondary,
			  const Grid2d &dir,
			  const ShearDetect &ld,
			  const ShearDetectOffsets &o) const;
};

# endif     /* CLDLINE_DETECT_H */
