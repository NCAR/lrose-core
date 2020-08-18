/**
 * @file RegCombHandler.hh
 * @brief Region builder
 * @class RegCombHandler
 * @brief Region builder
 *
 */
# ifndef    RegCombHandler_H
# define    RegCombHandler_H

#include <vector>

class Grid2d;
class GridAlgs;

class RegCombHandler
{
public:
  RegCombHandler(double extension, double angleDiff, double minHit);
  ~RegCombHandler();
    
  void process(const Grid2d * &reg, const Grid2d * &dir, Grid2d *out);

private:

  double _extension, _angleDiff, _minHit;

  bool _shouldCombine(const GridAlgs &data,
		      const Grid2d &orientation,
		      double r0, double r1) const;
};

# endif     /* CLDLINE_DETECT_H */
