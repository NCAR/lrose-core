/**
 * @file RadxAppSweepDataSimple.hh 
 * @brief Container for the processing of one sweep
 * @class RadxAppSweepDataSimple
 * @brief Container for the processing of one sweep
 */

#ifndef RADX_APP_SWEEP_DATA_SIMPLE_H
#define RADX_APP_SWEEP_DATA_SIMPLE_H

#include <radar/RadxAppSweepData.hh>

//------------------------------------------------------------------
class RadxAppSweepDataSimple : public RadxAppSweepData
{
public:

  /**
   * Empty constructor
   */
  inline RadxAppSweepDataSimple(void) : RadxAppSweepData() {}

  /**
   * Set up for index'th sweep in the RadxAppVolume
   *
   * @param[in] r The volume object
   * @param[in] index  Ray index
   * @param[in] lookup Lookup pointer
   */
  inline
  RadxAppSweepDataSimple(const RadxAppVolume &r, int index,
			 const RadxAppCircularLookupHandler *lookup=NULL) :
    RadxAppSweepData(r, index, lookup) {}

  /**
   * Destructor
   */
  inline virtual ~RadxAppSweepDataSimple(void) {}

  inline virtual bool radxappUserLoopFunction(const std::string &keyword,
					      ProcessingNode &p)
  {
    return false;
  }
  
  inline virtual MathUserData *
  radxappUserLoopFunctionToUserData(const UnaryNode &p)
  {
    return NULL;
  }

  inline virtual void
  radxappAppendUnaryOperators(std::vector<FunctionDef> &ops) const
  {
  }
  
protected:
private:

};

#endif
