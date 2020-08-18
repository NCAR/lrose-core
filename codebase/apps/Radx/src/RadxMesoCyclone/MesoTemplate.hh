/**
 * @file MesoTemplate.hh
 * @brief 
 * @class MesoTemplate
 * @brief 
 */
#ifndef MESOTEMPLATE_H
#define MESOTEMPLATE_H
#include "TemplateLookupMgr.hh"
#include <rapmath/FuzzyF.hh>
#include <vector>
#include <string>

class Sweep;
class Grid2d;

class MesoTemplate
{
public:
  /**
   * Constructor
   * @param[in] t  Pointer to the template to use
   * @param[in] minPctGood    Minimum percentage of points within the template
   *                          with valid data to do the computation
   * @param[in] minDiff       Minimum absolute difference in averages from the two template
   *                          sides to to the computation
   * @param[in] minPctLarge   Minimum percentage of points at least 50% as large as average
   *                          to do the computation
   * @param[in] ff            Pointer to fuzzy function for computation, applied to absolute differene
   *                          of averages
   */
  inline MesoTemplate(const TemplateLookupMgr *t,
		      double minPctGood, double minDiff,
		      double minPctLarge, FuzzyF *ff) :
    _t(t), _minPctGood(minPctGood), _minDiff(minDiff),
    _minPctLarge(minPctLarge),
    _ff(ff)  {}

  /**
   * @destructor
   */
  inline virtual ~MesoTemplate  (void) {}

  /**
   * Do the computations at each point
   * @param[in] data  Data to use
   * @param[in] v Sweep from which to get information
   * @param[out] out  Results grid
   */
  void apply(const Grid2d &data, const Sweep &v, Grid2d &out);

protected:
private:

  /**
   * Pointer to the template to use
   */
  const TemplateLookupMgr  *_t; 

  /**
   * Minimum percentage of points within the template  with valid data to do the computation
   */
  double _minPctGood;

  /**
   * Minimum absolute difference in averages from the two template sides to to the computation
   */
  double _minDiff;

  /**
   * Minimum percentage of points at least 50% as large as average  to do the computation
   */
  double _minPctLarge;

  /**
   * Pointer to fuzzy function for computation, applied to absolute differene  of averages
   */
  const FuzzyF *_ff;

  bool _updateOneRay(int i, const Grid2d &data,
			 bool circular, Grid2d &out);
  bool _updateGate(int i, int r,
		   const Grid2d &data,
		   bool circular, Grid2d &out);
  bool _addLookupToData(int i, int r, int rj, int aj,
			const Grid2d &data,
			bool circular,
			std::vector<double> &vdata,
			double &count) const;
  double _process(const std::vector<double> &data1,
		  const std::vector<double> &data2,
		  double count1, double count2);


};

#endif
