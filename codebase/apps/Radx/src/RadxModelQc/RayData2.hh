/**
 * @file RayData2.hh 
 * @brief Container for the processing of one sweep
 * @class RayData1
 * @brief Container for the processing of one sweep
 */

#ifndef RAY_DATA_2_H
#define RAY_DATA_2_H
#include "RayLoopSweepData.hh"
#include "RayData.hh"
#include <rapmath/MathData.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RayxData.hh>
#include <vector>
#include <map>
#include <string>

class MathUserData;
class RayData;
class CircularLookupHandler;

//------------------------------------------------------------------
class RayData2 : public MathData
{
public:

  RayData2(void);

  /**
   * Set up for index'th sweep in the RadxVol
   *
   * @param[in] The volume object
   * @param[in] index  Ray index
   */
  RayData2(const RayData &r, int index,
	   const CircularLookupHandler *lookup);
  /**
   * Destructor
   */
  virtual ~RayData2(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
protected:
private:


  const std::vector<RadxRay *> *_rays; /**< Pointer to each ray in vol*/
  int _i0; /**< Index to first ray in sweep */
  int _i1; /**< Index to last ray in sweep */
  
  /**
   * Derived values as filters are processed
   */
  std::vector<RayLoopSweepData> _data;

  /**
   * Each filter modifies to produce pointers to its inputs
   */
  std::vector<RayLoopSweepData *> _inps;

  /**
   * Each filter modifies to produce pointer to its output
   */
  RayLoopSweepData *_outputSweep;

  const CircularLookupHandler *_lookup;        /**< Lookup thing for 2dvar */

  RayLoopSweepData *_refToData(const std::string &name,
			       bool suppressWarn=false);
  RayLoopSweepData *_exampleData(const std::string &name);
  void _updateSweep(void);
  void _updateSweepOneDataset(int k);
  bool _needToSynch(const std::string &userKey) const;
  RayLoopSweepData *_match(const std::string &n);
  const RayLoopSweepData *_matchConst(const std::string &n) const;
  bool _processVariance2d(std::vector<ProcessingNode *> &args);
  bool _newLoopData(const std::string &name, RayLoopSweepData &ret,
		    bool warn) const;
  bool _anyLoopData(RayLoopSweepData &ret) const;

};

#endif
