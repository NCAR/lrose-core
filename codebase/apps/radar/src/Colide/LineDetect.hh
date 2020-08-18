/**
 * @file LineDetect.hh
 * @brief line detection done using 3 lookup tables
 * @class LineDetect
 * @brief line detection done using 3 lookup tables
 *
 * Lookup tables created from ParmLine as well as a neighborhood offset
 * template to check data values near each point for appropriateness in
 * computing thinline.
 */
# ifndef    LineDetect_H
# define    LineDetect_H

#include "Parms.hh"
#include <vector>
class LineDetectOffsets;
class LineDetectOffset;
class Grid2dOffset;
class Grid2d;

class LineDetect
{
public:
  LineDetect();
  ~LineDetect();
    
  // init for given p and nx.
  void init(const Parms &p, const int nx, const double missing,
	    bool allow_missing_side);

  // return  true if input data is ok near the inputs for computing
  // line detection.
  bool ok(const int x, const int y, const int nx, const int ny,
	  const Grid2d &input) const;

  bool best_fit(const Grid2d &input, const double missing,
		const int x, const int y, const int nx, const int ny, 
		const LineDetectOffsets &o, double &det) const;//, double &dir) 

  bool best_direction(const Grid2d &input, const double missing,
		const int x, const int y, const int nx, const int ny, 
		const LineDetectOffsets &o, double &dir) const;
  bool valueAtDirection(const Grid2d &input, double direction,
			const double missing,
			const int x, const int y, const int nx, const int ny, 
			const LineDetectOffsets &o, double &det) const;
private:

  Parms _p;
  Grid2dOffset *_neighbor;      // neighborhood template offsets
  int _max_neighbor_offset;  // max offset in neighborhood.
  bool _allow_missing_side;
    
  // return line detection interest pixel value using template offset l
  // at the data inputs.
  double _line_detect(const LineDetectOffset &l, const Grid2d &input,
		      const int x, const int y,  bool &left_missing,
		      bool &right_missing) const;

  // fuzzy line detection value.
  double _fuzzy_line_detect(const LineDetectOffset &l, const Grid2d &input,
			    const int x, const int y, const double missing,
			    const bool left_missing,
			    const bool right_missing) const;
  void _compute_fuzzy(void);
  double _mean_side(const double offset, const Grid2dOffset &o,
		    const Grid2d &input, const int x, const int y,
		    bool &is_missing) const;
  void _map_center(const std::vector<double> &cpix, const double missing,
		   int &count, double &mean, double &meansq,
		   double &fuzzyv) const;
  void _map_side(const std::vector<double> &spix, const double missing, 
		 const double mean_center, const bool is_missing,
		 int &count, double &fuzzyv) const;
};

# endif     /* CLDLINE_DETECT_H */
