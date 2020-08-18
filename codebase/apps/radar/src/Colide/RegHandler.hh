/**
 * @file RegHandler.hh
 * @brief Region builder
 * @class RegHandler
 * @brief Region builder
 *
 */
# ifndef    RegHandler_H
# define    RegHandler_H

#include <vector>
#include "ParmRegion.hh"

class Grid2d;

class RegHandler
{
public:
  RegHandler(double min_mean_len, double min_max_len,
	     double min_min_len, double min_area,
	     double min_hot_area);
  ~RegHandler();
    
  bool process(const Grid2d *hot, const Grid2d *region,
	       const Grid2d *full, Grid2d *out);

private:

  ParmRegion _parm;
};

# endif     /* CLDLINE_DETECT_H */
