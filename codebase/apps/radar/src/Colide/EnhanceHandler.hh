/**
 * @file EnhanceHandler.hh
 * @brief enhancment filter
 * @class EnhanceHandler
 * @brief enhancment filter
 *
 */
# ifndef    EnhanceHandler_H
# define    EnhanceHandler_H

#include "ParmEnhance.hh"
// #include <vector>
// #include <toolsa/TaThreadDoubleQue.hh>

class Grid2d;

class EnhanceHandler
{
public:
  EnhanceHandler(int len, int width, int numAngles,
		 const FuzzyF &f, int num_thread);
  ~EnhanceHandler();
    
  void processDir(const Grid2d *input, Grid2d *output);
  void process(const Grid2d *input, Grid2d *output);
  // void processConf(const Grid2d *input, const Grid2d *dir, Grid2d *output);

private:

  ParmEnhance _parm;
};

# endif     /* CLDLINE_DETECT_H */
