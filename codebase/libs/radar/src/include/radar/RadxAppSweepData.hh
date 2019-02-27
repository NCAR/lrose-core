/**
 * @file RadxAppSweepData.hh 
 * @brief Container for the processing of one sweep
 * @class RadxAppSweepData
 * @brief Container for the processing of one sweep
 */

#ifndef RADX_APP_SWEEP_DATA_H
#define RADX_APP_SWEEP_DATA_H

#include <radar/RadxAppSweepLoopData.hh>
#include <rapmath/MathData.hh>
#include <vector>
#include <string>

class RadxAppVolume;
class RadxAppCircularLookupHandler;
class RadxRay;

//------------------------------------------------------------------
class RadxAppSweepData : public MathData
{
public:

  /**
   * Empty constructor
   */
  RadxAppSweepData(void);

  /**
   * Set up for index'th sweep in the RadxAppVolume
   *
   * @param[in] r The volume object
   * @param[in] index  Ray index
   * @param[in] lookup Lookup pointer
   */
  RadxAppSweepData(const RadxAppVolume &r, int index,
		   const RadxAppCircularLookupHandler *lookup=NULL);

  /**
   * Destructor
   */
  virtual ~RadxAppSweepData(void);

  #include <rapmath/MathDataVirtualMethods.hh>
  
  #define RADXAPP_MATH_DATA_BASE
  #include <radar/RadxAppMathDataVirtualMethods.hh>
  #undef RADXAPP_MATH_DATA_BASE
  
protected:
private:

  /**
   * Unary function names
   */
  static const std::string _variance2dStr;

  const std::vector<RadxRay *> *_rays; /**< Pointer to each ray in vol*/
  int _i0; /**< Index to first ray in sweep */
  int _i1; /**< Index to last ray in sweep */
  
  /**
   * Derived values as filters are processed
   */
  std::vector<RadxAppSweepLoopData> _data;

  /**
   * Each filter modifies to produce pointers to its inputs
   */
  std::vector<RadxAppSweepLoopData *> _inps;

  /**
   * Each filter modifies to produce pointer to its output
   */
  RadxAppSweepLoopData *_outputSweep;

  const RadxAppCircularLookupHandler *_lookup;   /**< Lookup thing for 2dvar */

  RadxAppSweepLoopData *_refToData(const std::string &name,
			       bool suppressWarn=false);
  RadxAppSweepLoopData *_exampleData(const std::string &name);
  void _updateSweep(void);
  void _updateSweepOneDataset(int k);
  // bool _needToSynch(const std::string &userKey) const;
  RadxAppSweepLoopData *_match(const std::string &n);
  const RadxAppSweepLoopData *_matchConst(const std::string &n) const;
  bool _processVariance2d(std::vector<ProcessingNode *> &args);
  bool _newLoopData(const std::string &name, RadxAppSweepLoopData &ret,
		    bool warn) const;
  bool _anyLoopData(RadxAppSweepLoopData &ret) const;

};

#endif
