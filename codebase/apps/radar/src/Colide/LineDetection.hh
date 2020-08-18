/**
 * @file LineDetection.hh
 * @brief line detection done using 3 lookup tables
 * @class LineDetection
 * @brief line detection done using 3 lookup tables
 *
 * Lookup tables created from Parms as well as a neighborhood offset
 * template to check data values near each point for appropriateness in
 * computing thinline.
 */
# ifndef    LineDetection_H
# define    LineDetection_H

#include "Parms.hh"
#include "Window.hh"
#include <vector>
#include <toolsa/TaThreadDoubleQue.hh>
class LineDetectOffsets;
class LineDetect;
// class LineDetectionOffset;
// class Grid2dOffset;
// class Grid2d;
class Grid2d;

class LineDetection
{
public:
  LineDetection(int nx, int ny, int nthread, const Parms &parms);
  ~LineDetection();
    
  void processDir(const Grid2d *input, Grid2d *output);
  void processDet(const Grid2d *input, const Grid2d *dir,
		  Grid2d *output);

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
  class LineDetectionThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline LineDetectionThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~LineDetectionThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(const int index);
  };

  int _nx, _ny, _nthread;
  Window _window;
  Parms _parms;
  bool _allow_missing_side;
  bool _flag_where_flagged;
  double _flag_replacement;

  double _line_direct_xy(const int x, const int y, const int nx,
			 const int ny, const Grid2d &input,
			 const LineDetect &ld,
			 const LineDetectOffsets &o) const;
  double _line_detect_xy(const int x, const int y, const int nx,
			 const int ny, const Grid2d &input,
			 const Grid2d &dir,
			 const LineDetect &ld,
			 const LineDetectOffsets &o) const;
};

# endif     /* CLDLINE_DETECT_H */
