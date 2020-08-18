/**
 * @file VirtVolFuzzy.hh 
 * @brief A Fuzzy function derived from MathUserData
 * @class VirtVolFuzzy
 * @brief A Fuzzy function derived from MathUserData
 */

#ifndef VirtVolFuzzy_hh
#define VirtVolFuzzy_hh

#include <rapmath/MathUserData.hh>
#include <rapmath/FuzzyF.hh>
#include <string>
#include <vector>

//------------------------------------------------------------------
class VirtVolFuzzy : public MathUserData, public FuzzyF
{
public:

  /**
   * Empty constructor
   */
  inline VirtVolFuzzy(void)   {}

  /**
   * Constructor
   * @param[in] pairs  xy pairs
   */
  inline VirtVolFuzzy(const std::vector<std::pair<double,double> > &pairs) :
    MathUserData(), FuzzyF(pairs) {}
  
  /**
   * Destructor
   */
  inline virtual ~VirtVolFuzzy(void) {}


  /**
   * Virtual method, returns a float value or not
   * @param[out] v  The value
   * @return true if value was meaningfully set
   */
  inline virtual bool getFloat(double &v) const
  {
    v = 0;
    return false;
  }


protected:
private:

};

#endif
 
