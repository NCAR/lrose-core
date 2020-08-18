/**
 * @file ShapePolygons.hh 
 * @brief Create, store and output GenPoly objects to go with clumps in
 *        gridded data
 * @class ShapePolygons
 * @brief Create, store and output GenPoly objects to go with clumps in
 *        gridded data
 */

#ifndef ShapePolygons_hh
#define ShapePolygons_hh

#include <euclid/Grid2d.hh>
#include <rapformats/GenPoly.hh>
#include <rapmath/MathUserData.hh>
#include <string>
#include <vector>

class MdvxProj;

//------------------------------------------------------------------
class ShapePolygons : public MathUserData
{
public:

  /**
   * Empty constructor
   */
  ShapePolygons();

  /**
   * Create and store the GenPoly objects
   * @param[in]  t  Time of the grid
   * @param[in]  expireSeconds  valid number of seconds
   * @param[in]  proj  The projection of the grid
   * @param[in]  g  The grid
   * @param[in] isDiamondShape  True to create an enclosing diamond shape,
   *                           false to follow the boundary
   */
  ShapePolygons(const time_t &t, int expireSeconds,
		const MdvxProj &proj, const Grid2d &g,
		bool isDiamondShape);

  /**
   * Create and store the GenPoly objects
   * @param[in]  t  Time of the grid
   * @param[in]  expireSeconds  valid number of seconds
   * @param[in]  proj  The projection of the grid
   * @param[in] shapeSizeKm  Fixed size for shapes
   * @param[in]  g  The grid
   */
  ShapePolygons(const time_t &t, int expireSeconds,
		const MdvxProj &proj,
		double shapeSizeKm, const Grid2d &g);

  /**
   * Write the GenPoly objects to a url
   * @param[in]  t  Time of the grid
   * @param[in]  expireSeconds  valid number of seconds
   * @param[in] url The SPDB url
   */
  void output(const time_t &t, int expireSeconds,
	      const std::string &url);

  /**
   * Destructor
   */
  inline virtual ~ShapePolygons(void) {}


  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return true if object well formed.
   */
  inline bool ok(void) const {return _ok;}

protected:
private:

  std::vector<GenPoly> _shapes;  /**< The genpoly shapes */
  bool _ok;         /**< True if object well formed */

  void _createDiamonds(const time_t &t, int expireSeconds,
		       const MdvxProj &proj, const Grid2d &g,
		       bool fixed, double fixedSizeKm) ;
  void _createWrappedShapes(const time_t &t, int expireSeconds,
			    const MdvxProj &proj, const Grid2d &g);
};

#endif
 
