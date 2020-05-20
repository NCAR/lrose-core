/**
 * @file FullTemplateExtent.hh
 * @brief 
 * @class FullTemplateExtent
 * @brief 
 */
#ifndef FulLTemplateExtent_h
#define FulLTemplateExtent_h
#include "TemplateLookupMgr.hh"
#include <euclid/Grid2dClump.hh>
#include "Parms.hh"
#include <vector>
#include <string>

class Sweep;
class Grid2d;

class FullTemplateExtent
{
public:
  /**
   * Empty lookup for gates too close to radar
   * @param[in] centerIndex  Index to this gate
   */
  inline FullTemplateExtent(const Parms &p, const TemplateLookupMgr *t, bool doExtend) :
    _parms(p), _t(t), _doExtend(doExtend)  {}

  /**
   * @destructor
   */
  inline virtual ~FullTemplateExtent  (void) {}

  void apply(const Grid2d &data, const Sweep &v, Grid2d &out);

protected:
private:

  Parms _parms;
  const TemplateLookupMgr  *_t;
  bool _doExtend;

  void _addToOutput(int clumpIndex, const clump::Region_t &clump, bool circular, Grid2d &out);
  void _addPointToOutput(int clumpIndex, int x, int y, bool circular, Grid2d &out);
  bool _getIndices(int i, int r, int rj, int aj, bool circular, int nx, int ny,
		   int &ix, int &iy) const;
};

#endif
