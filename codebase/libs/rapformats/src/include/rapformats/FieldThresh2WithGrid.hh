/**
 * @file FieldThresh2WithGrid.hh
 * @brief store a field name and two thresholds, and
 *        optionally a grid of thresholds which replaces the individual values
 * @class FieldThresh2WithGrid
 * @brief store a field name and two thresholds, and
 *        optionally a grid of thresholds which replaces the individual values
 */

# ifndef    FieldThresh2WithGrid_hh
# define    FieldThresh2WithGrid_hh

#include <string>
#include <rapformats/FieldThresh2.hh>

class Grid2d;

//----------------------------------------------------------------
class FieldThresh2WithGrid : public FieldThresh2
{
public:

  /**
   * Empty constructor
   */
  inline FieldThresh2WithGrid(void) : FieldThresh2(), _gridded(false),
				      _grid(NULL) { }

  /**
   * Constructor from base class FieldThresh, where _thresh2 is set to
   * the same value as the input threshold, and there is no grid
   *
   * @param[in] f  FieldThresh to use
   */
  inline FieldThresh2WithGrid(const FieldThresh &f) : FieldThresh2(f), 
						      _gridded(false),
						      _grid(NULL) {}

  /**
   * Constructor, Members set from inputs
   *
   * @param[in] field     Field name
   * @param[in] thresh    Threshold
   * @param[in] thresh2   2nd threshold
   * @param[in] gridded   True for gridded thresholds
   * @param[in] grid      Pointer to the gridded thresholds when gridded
   */
  inline FieldThresh2WithGrid(const std::string &field, double thresh,
			      double thresh2, bool gridded,
			      const Grid2d *grid) : 
    FieldThresh2(field, thresh, thresh2), _gridded(gridded), _grid(grid) {}


  /**
   * Constructor, Name set from input, with thresholds set to a bad value,
   * no grid
   *
   * @param[in] field
   */
  inline FieldThresh2WithGrid(const std::string &field) : FieldThresh2(field),
							  _gridded(false),
							  _grid(NULL) {}

  /**
   * Destructor
   */
  inline ~FieldThresh2WithGrid(void) {}

  /**
   * @return threshold, using grid index if gridded. The method does not check
   * the validity of the index when gridded.  If not gridded, returns the first
   * threshold
   *
   * @param[in] gridIndex
   */
  double getThresh(int gridIndex) const;

protected:
private:  

  bool _gridded;        /**< True for a gridded threshold field */
  const Grid2d *_grid;  /**< Pointer to gridded threshold field, when _gridded*/

};

# endif
