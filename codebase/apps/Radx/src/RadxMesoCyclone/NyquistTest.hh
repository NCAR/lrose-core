/**
 * @file NyquistTest.hh
 * @brief 
 * @class NyquistTest
 * @brief 
 */
#ifndef NyquistTest_H
#define NyquistTest_H
#include "TemplateLookupMgr.hh"
#include "Parms.hh"
#include <vector>
#include <string>

class Sweep;
class Grid2d;

class NyquistTest
{
public:
  /**
   * Empty lookup for gates too close to radar
   * @param[in] centerIndex  Index to this gate
   */
  inline NyquistTest(const Parms &p, const TemplateLookupMgr *t) :
    _parms(p), _t(t) {}

  /**
   * @destructor
   */
  inline virtual ~NyquistTest  (void) {}

  void apply(const Grid2d &data, const Grid2d &mask,
	     const Sweep &v, Grid2d &out);

protected:
private:

  Parms _parms;
  const TemplateLookupMgr  *_t;

  bool _updateOneRay(int i, const Grid2d &data,
		     const Grid2d &mask,
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
