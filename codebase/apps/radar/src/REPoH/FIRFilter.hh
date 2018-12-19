/**
 * @file FIRFilter.hh
 * @brief  Perform FIR filter on data
 * @class FIRFilter
 * @brief  Perform FIR filter on data
 */

#ifndef FIR_FILTER_H
#define FIR_FILTER_H

#include <string>
#include <vector>

class Grid2d;
class GridAlgs;

class FIRFilter
{
public:
  /**
   * Constructor
   */
  FIRFilter();

  /**
   * Destructor
   */
  ~FIRFilter();
  
  /**
   * Peform FIR filter on input to output
   * @param[in] inp  Input data
   * @param[out] out  Output data
   * @return true for success
   */
  bool filter(const Grid2d &inp, GridAlgs &out) const; 
    
private:

  /**
   * The FIR coefficients
   */
  std::vector<double> _coeff;

};

#endif
