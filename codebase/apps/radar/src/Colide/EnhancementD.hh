/**
 * @file EnhancementD.hh
 * @brief line enhancement at a point
 * @class EnhancementD
 * @brief line enhancement at a point
 *
 * enhancement at a point is computed from an enhancement table
 * the table is specified by 2 params min+diff and max_diff.
 */

# ifndef    EnhancementD_hh
# define    EnhancementD_hh

/* System include files / Local include files */
#include "EnhancementOffsetD.hh"
#include "Window.hh"
#include <rapmath/FuzzyF.hh>

class ParmEnhance;

/*----------------------------------------------------------------*/
class EnhancementD
{
public:

  //! initialize table with input params.
  EnhancementD(const ParmEnhance &p);
  virtual ~EnhancementD();


  bool best_fit(const Grid2d &g, int x, int y,
		int nx, int ny, const EnhancementOffsetsD &o, double &det,
		double &ang) const;
 private:
  FuzzyF _f;
  double _min_diff;
  bool _allow_missing_side;

  //! return the value based on enhancement offset template and data
  bool _enhancement(const EnhancementOffsetD &o,
		    const Grid2d &input, const int x, const int y,
		    double &v) const;
  //! return the value based on enhancement offset template and data
  bool _fuzzy_enhancement(const EnhancementOffsetD &o,
			  const Grid2d &input,  const int x, const int y,
			  double non_fuzzy_enhancement,
			  double &angle, double &v) const;
  //! return the value based on enhancement offset template and data
  bool _half_center_enhancement(const EnhancementOffsetD &o,
				const Grid2d &g, const int x, const int y,
				double &v) const;


};


# endif
