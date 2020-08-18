/**
 * @file EllipHandler.hh
 * @brief line detection done using 3 lookup tables
 * @class EllipHandler
 * @brief line detection done using 3 lookup tables
 *
 * Lookup tables created from Parms as well as a neighborhood offset
 * template to check data values near each point for appropriateness in
 * computing thinline.
 */
# ifndef    EllipHandler_H
# define    EllipHandler_H

#include "Parms.hh"
#include "Window.hh"
#include <vector>
#include <toolsa/TaThreadDoubleQue.hh>
class Elliptical;
class Grid2d;

class EllipHandler
{
public:
  EllipHandler(int nx, int ny, int nthread, const Parms &parms);
  ~EllipHandler();
    
  void processConf(const Grid2d *input, const Grid2d *dir, Grid2d *output);
  void processDir(const Grid2d *input, Grid2d *output);
  void process(const Grid2d *input, const Grid2d *dir,
	       Grid2d *output);

private:

  int _nx, _ny, _nthread;
  Window _window;
  Parms _parms;
  double _angle_res;
  double _smooth_thresh;
  double _smooth_background;
  bool _is_min_variance;
};

# endif     /* CLDLINE_DETECT_H */
