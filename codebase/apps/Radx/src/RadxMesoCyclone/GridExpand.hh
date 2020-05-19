/**
 * @file GridExpand.hh
 * @class GridExpand
 */

# ifndef    GRID_EXPAND_H
# define    GRID_EXPAND_H

#include <euclid/Grid2d.hh>
#include <Mdv/MdvxProj.hh>
#include <vector>

class GridExpand 
{

public:

  /**
   * @param[in] nptX
   */
  GridExpand(int nptX);

  /**
   * Destructor
   */
  virtual ~GridExpand (void);

  /**
   * update using input grid
   *
   * @param[in] g   Grid with data to use
   */
  void update(const Grid2d &g, const MdvxProj &proj, Grid2d &e);
	      
protected:
private:

  int _nptX;

};

#endif
