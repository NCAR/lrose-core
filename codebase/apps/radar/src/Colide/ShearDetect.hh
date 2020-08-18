/**
 * @file ShearDetect.hh
 * @brief shear detection at a point is the computed from 3 lookup tables
 * @class ShearDetect
 * @brief shear detection at a point is the computed from 3 lookup tables
 *
 * 3 tables created based on ParmShear values, and a neighborhood
 * offset template is used to check out dbz values for appropriate ranges
 * near each point.
 */
# ifndef    SHEAR_DETECT_H
# define    SHEAR_DETECT_H

#include "ShearDetectOffsets.hh"
#include <Mdv/MdvxProj.hh>

class ShearDetect
{
 public:
    // default constructor
    ShearDetect();

    // default destructor
    virtual ~ShearDetect();

    // initialize.
  void init(const Parms &p, const MdvxProj &proj, int nx, double missing,
	    double test_circle_diameter);

    // return true if the input indices and secondary data are ok.
    bool secondary_ok(int x, int y, int nx, int ny,
		      const Grid2d &secondary,
		      const double *secondaryRange) const;

    // return true if the input indices are ok.
    //   Same test, but no secondary data.
    inline bool ok(int x, int y, int nx, int ny) const
    {
      return !__neighbor->outOfRange(x, y, nx, ny);
    }

    bool best_direction(const Grid2d &input, const int x,
			const int y, const int nx, const int ny, 
			const ShearDetectOffsets &o, 
			double shearMin, double &dir) const;

  bool valueAtDirection(const Grid2d &input, double direction, int x, int y,
			int nx, int ny, const ShearDetectOffsets &o,
			double cutAngle,
			bool isConvergent,
			double &det) const;

    // // return true if a good fit found and det/dir are set
    // bool best_fit(const Grid2d &input, const int x,
    // 		  const int y, const int nx, const int ny, 
    // 		  const ShearDetectOffsets &o, double &det,
    // 		  double &dir) const;

 private:
    Parms __p;         // parameter storage.
  MdvxProj _proj;
  double _test_circle_diameter;
    bool __is_secondary;
    Grid2dOffset *__neighbor;  // the secondary neighborhood offset template.
    int __neighbor_max_off; // the max offset in that neighborhood.
    

    // return absolute diff between left and right sides
  bool _shear_detect(const Grid2d &input, const int x, const int y,
		     const ShearDetectOffset &l, double &v) const;

  // return fuzzy measure of shear.
  bool _fuzzy_shear_detect(int x, int y, int ny, 
			   const Grid2d &input,
			   const ShearDetectOffset &s,
			   double cutAngle,
			   bool isConvergent,
			   double &v) const;

  bool _direction_good(int x, int y, int ny, const ShearDetectOffset &s,
		       double mr, double ml, double cutAngle,
		       bool isConvergent) const;
  double _shear_detection_value (vector<double> &pixup,
				 vector<double> &pixlow, 
				 double missing, double mean) const;
  void _accumulate(vector<double> &pix, double missing,
		   double mean, bool above_mean,
		   double &v, double &cnt) const;
    void _compute_neighbor(int nx, double missing);
};


# endif     /* CLDSHEAR_DETECT_H */
