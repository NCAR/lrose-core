/**
 * @file TileRange.hh
 * @brief Information about one tile, a struct-like class
 * @class TileRange
 * @brief Information about one tile, a struct-like class
 */

# ifndef    TileRange_hh
# define    TileRange_hh

#include <string>
#include <cstdio>

//----------------------------------------------------------------
class TileRange
{
public:
  /**
   * Empty
   */
  inline TileRange(void) : _x0(0), _y0(0), _nx(0), _ny(0), _index(-1),
			   _ok(false) { }


  /**
   * Construct from args
   */
  inline TileRange(int x0, int y0, int nx, int ny, int index) :
    _x0(x0), _y0(y0), _nx(nx), _ny(ny), _index(index), _ok(true) {}


  /**
   * Destructor
   */
  inline virtual ~TileRange(void) {}

  /**
   * @return true if the range is all in bounds, Y
   * @param[in] gridNptY  Number of gridpoints in Y
   */
  inline bool inBoundsY(int gridNptY) const
  {
    return _y0 >= 0 && (_y0 + _ny < gridNptY);
  }

  /**
   * @return true if the input y value is inside the range y0 to y0+ny-1
   * @param[in] y
   */
  inline bool inRangeY(int y) const
  {
    return y >= _y0 && y < _y0 + _ny;
  }

  /**
   * @return true if the input x value is inside the range x0 to x0+nx-1
   * @param[in] x
   */
  inline bool inRangeX(int x) const
  {
    return x >= _x0 && x < _x0 + _nx;
  }

  /**
   * @return tile index
   */
  inline int getTileIndex(void) const {return _index;}

  /**
   * @return x0
   */
  inline int getX0(void) const {return _x0;}

  /**
   * @return y0
   */
  inline int getY0(void) const {return _y0;}

  /**
   * @return nx
   */
  inline int getNx(void) const {return _nx;}

  /**
   * @return ny
   */
  inline int getNy(void) const {return _ny;}

  /**
   * @return true if local tile is 'above' (larger y0) than input
   * @param[in] t
   */
  inline bool isAbove(const TileRange &t) const
  {
    return _y0 > t._y0;
  }
    
  /**
   * @return true if object is valid
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * @return string debug print
   */
  inline std::string sprint(void) const
  {
    char buf[1000];
    sprintf(buf, "(x0,y0):(%d,%d)  (nx,ny):(%d,%d) tile:%d",
	    _x0, _y0, _nx, _ny, _index);
    std::string s = buf;
    return s;
  }

protected:
private:  

  int _x0;  /**< Lower left grid index for a tile */
  int _y0;  /**< Lower left grid index for a tile */
  int _nx;  /**< Number of grid points in a tile */
  int _ny;  /**< Number of grid points in a tile */
  int _index; /**< Index for this tile */
  bool _ok;  /**< True if values are good */
};


# endif
