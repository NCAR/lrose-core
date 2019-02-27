// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>
/**
 * @file  MultiThreshItem.hh
 * @brief  
 * @class  MultiThreshItem
 * @brief  
 *
 */

# ifndef    MultiThreshItem_H
# define    MultiThreshItem_H

#include <rapformats/FieldThresh2.hh>
#include <rapformats/MultiThresh.hh>
#include <vector>

class TileInfo;

//----------------------------------------------------------------
class MultiThreshItem 
{
public:

  /**
   * Empty constructor
   */
  inline MultiThreshItem(void) :
  _multiThresh(), _genHour(-1), _genMin(-1), _genSec(-1), _leadTime(0),
  _tileIndex(0)
  {
  }

  /**
   * Constructor with member values to use passed in
   *
   * @param[in] fieldthresh Fields/thresholds
   * @param[in] obsTime  time of data used to generate thresholds/bias
   * @param[in] genHour  Forecast generation time
   * @param[in] genMin  Forecast generation time
   * @param[in] genSec  Forecast generation time
   * @param[in] leadTime  Forecast lead time
   * @param[in] tileIndex  Tile index 
   * @param[in] motherTile  True if came from  mother tile
   * @param[in] bias  bias value
   * @param[in] obsValue  Observations data value
   * @param[in] fcstValue  Forecast data value
   */
  inline MultiThreshItem(std::vector<FieldThresh2> fieldthresh, time_t obsTime,
			 int genHour, int genMin, int genSec,
			 int leadTime, int tileIndex, bool motherTile,
			 double bias=-999.99, double obsValue=-99.99,
			 double fcstValue=-99.99) :
    _multiThresh(fieldthresh, bias, obsTime, obsValue, fcstValue, motherTile),
    _genHour(genHour),
    _genMin(genMin),
    _genSec(genSec),
    _leadTime(leadTime),
    _tileIndex(tileIndex)
  {
  }

  /**
   * Constructor with MultitThresh and member values passed in
   *
   * @param[in] mthresh  MultiThresh object to use
   * @param[in] genHour  Forecast generation time
   * @param[in] genMin  Forecast generation time
   * @param[in] genSec  Forecast generation time
   * @param[in] leadTime  Forecast lead time
   * @param[in] tileIndex  Tile index
   */
  inline MultiThreshItem(const MultiThresh &mthresh,
			 int genHour, int genMin, int genSec,
			 int leadTime, int tileIndex) :
    _multiThresh(mthresh), _genHour(genHour),
    _genMin(genMin), _genSec(genSec), _leadTime(leadTime),
    _tileIndex(tileIndex)
  {
  }

  /**
   * Destructor 
   */
  inline ~MultiThreshItem(void) {}

  /**
   * @return true if the is an empty object
   */
  inline bool empty(void) const
  {
    return _genHour == -1 && _genMin == -1 && _genSec == -1;
  }

  /**
   * Get the FieldThresh object associated with a field name
   * @param[in] fieldName
   * @param[out] item  Returned item
   * @return true if fieldName found and item set
   */
  inline bool get(const std::string &fieldName, FieldThresh &item) const
  {
    return _multiThresh.get(fieldName, item);
  }
  
  /**
   * Debug print
   * @param[in] info  Information about tiling
   */
  void print(const TileInfo &info) const;

  /**
   * Debug log message to DEBUG
   */
  void logDebug(void) const;

  MultiThresh _multiThresh;  /**< thresholds information */
  int _genHour;              /**< Forecast gen time */
  int _genMin;               /**< Forecast gen time */
  int _genSec;               /**< Forecast gen time */
  int _leadTime;             /**< Forecast lead time */
  int _tileIndex;            /**< Index into tiles */

protected:
private:  

};

# endif     // MultiThreshItem_HH
