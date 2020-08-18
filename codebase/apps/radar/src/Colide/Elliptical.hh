/**
 * @file Elliptical.hh
 * @brief Used for elliptical filtering algorithms
 * @class Elliptical
 * @brief Used for elliptical filtering algorithms
 */
#ifndef ELLIPTICAL_HH
#define ELLIPTICAL_HH

#include "Parms.hh"
#include <euclid/Grid2dOffset.hh>
#include <toolsa/TaThreadDoubleQue.hh>

class Grid2d;

class Elliptical
{
public:

  Elliptical(const Parms &p, double angleRes, double smoothThresh,
	     double smoothBackground, bool isMinVariance,
	     int nx, int ny, int nthread,
	     const Grid2d &g);
  
  virtual ~Elliptical();

  void processConfidence(const Grid2d *input, const Grid2d *orientation,
			 Grid2d *out);
  void processOrientation(const Grid2d *input, Grid2d *output);
  void process(const Grid2d *input, const Grid2d *orientation, Grid2d *output);

#ifdef NOT
  /**
   * process the input data.
   *
   *process the input data, filling in template orientation image also,
   * with confidence, fill in orientation images also.
   */
  void process(const Grid2d &input, Grid2d &out, Grid2d &conf,
	       Grid2d &orientation, Grid2d &orientation_median_filtered);


  /**
   * same, no orientation images.
   */
  void process(const Grid2d &input, Grid2d &out, Grid2d &conf);


  /**
   * same, no confidence image.
   */
  void process(const Grid2d &input, Grid2d &out);

  /**
   * same, orientation images,no confidence image.
   */
  void process(const Grid2d &input, Grid2d &out, Grid2d &orientation,
	       Grid2d &orientation_median_filtered);
#endif

  /**
   * Compute method needed for threading
   * @param[in] i  Information
   */
  static void compute(void *i);

private:  

  /**
   * @class EllipThreads
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   * the clone() method.
   */
  class EllipThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline EllipThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~EllipThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(const int index);
  };

  Parms _parm;
  double _angleRes;
  double _smoothThresh;
  double _smoothBackground;
  bool _isMinVariance;
  vector<Grid2dOffset> _offsets;      //!< one for each angle orientation.
  vector<Grid2dOffset> _conf_offsets; //!< same, confidence calculations.
  Grid2dOffset _neighbor;
  EllipThreads _thread;

  // void _set_offsets(double width, double length,  double angle_res);
  // double _process_xy(int x, int y, Grid2d &drc, const Grid2d &input) const;
  // double _process_xy(int x, int y, const Grid2d &input) const;
  // double _process_xy(int x, int y, Grid2d &drc, Grid2d &conf,
  // 		     const Grid2d &input) const;
  // double _process_xy_with_conf(int x, int y,
  // 			       Grid2d &conf, const Grid2d &input) const;
  double _process_orientation_xy(int x, int y, const Grid2d &input) const;

  double _process_confidence_xy(int x, int y, const Grid2d &input,
				const Grid2d &orientation) const;
  double _process_xy(int x, int y, const Grid2d &input,
		     const Grid2d &orientation) const;
};


#endif     /* ELLIPTICAL_H */
