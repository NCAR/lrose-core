/**
 * @file PolarCircularFilter.hh
 * @brief Actions inside regions that are km by km
 * @class PolarCircularFilter
 * @brief Actions inside regions that are km by km
 *
 * All static methods. This should go into GridAlgs probably
 */
#ifndef PolarCircularFilter_h
#define PolarCircularFilter_h
class Grid2d;
class MdvxProj;
class PolarCircularTemplate;

class PolarCircularFilter
{
public:
  /**
   * Apply a smoothing filter to the input grid using the template
   * @param[in,out] a  The grid to smooth
   * @param[in] pt  The template
   */
  static void smooth(Grid2d &a, const PolarCircularTemplate &pt);

  /**
   * Apply a smoothing filter to the input grid using the template
   * SMoothing is applied only at points where at least one data value
   * within the smoothing window has value >= thresh. At all other points
   * set the output to the uninteresting value
   *
   * @param[in,out] a  The grid to smooth
   * @param[in] pt  The template
   * @param[in] thresh
   * @param[in] uninterestingValue
   */
  static void smoothWithThresh(Grid2d &a,  const PolarCircularTemplate &pt,
			       double thresh, double uninterestingValue);

  /**
   * Apply a max expand (dilate) filter to the input grid using the template
   * @param[in,out] a  The grid to dilate
   * @param[in] pt  The template
   */
  static void dilate(Grid2d &a, const PolarCircularTemplate &pt);

  /**
   * Apply a filter to the input grid using the template.
   * At each point set output to the percent of points in the template region 
   * with value < min
   * @param[in,out] a  The grid to read/write
   * @param[in] pt  The template
   * @param[in] min  The minimum threshold value
   */
  static void percentLessThan(Grid2d &a, const PolarCircularTemplate &pt, double min);

  /**
   * Apply a filter to the input grid using the template
   * 
   * At each point set output to fabs(npos-nneg)/(npos+nneg)
   * where npos is number of points > min and nneg is number of points < -min
   *
   * @param[in,out] a  The grid read/write
   * @param[in] pt  The template
   * @param[in] thresh  The threshold value
   */
  static void largePosNeg(Grid2d &a, const PolarCircularTemplate &pt, double thresh);

  /**
   * Apply a filter to the input grid using the template
   * 
   * At each point set output to median of values in the template
   *
   * @param[in,out] a  The grid read/write
   * @param[in] pt  The template
   * @param[in] binMin histogram spec
   * @param[in] binMax histogram spec
   * @param[in] binDelta histogram spec
   */
  static void median(Grid2d &a, const PolarCircularTemplate &pt, double binMin,
		     double binMax, double binDelta);

private:

};

#endif
