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
 * @file TileLatLon.hh
 * @brief Mapping from tile index to lat/lon
 * @class TileLatLon
 * @brief Mapping from tile index to lat/lon
 */

# ifndef    TILE_LAT_LON_H
# define    TILE_LAT_LON_H

#include <map>
#include <string>

//----------------------------------------------------------------
class TileLatLon
{
public:

  /**
   * Default constructor, leaves things empty
   */
  TileLatLon(void);
  
  /**
   * constructor that parses XML
   * @param[in] xml  String to parse
   */
  TileLatLon(const std::string &xml);
  
  /**
   *  Destructor
   */
  virtual ~TileLatLon(void);

  /**
   * Set ok
   */
  inline void setOk(void) {_ok = true;}

  /**
   * @param[in] t
   */
  bool operator==(const TileLatLon &t) const;
  void printDiffs(const TileLatLon &t) const;
  
  /**
   * Add lat/lon for a tile index
   * @param[in] index  The index
   * @param[in] lat
   * @param[in] lon
   */
  void add(int index, double lat, double lon);

  /**
   * @return XML for current tiles
   */
  std::string getXml(void) const;

  /**
   * @return string to go with a tile index, or a blank string if not in
   * state
   * @param[in] tileIndex
   */
  std::string debugString(int tileIndex) const;

  /**
   * @return true if values are set
   */
  inline bool isOk(void) const {return _ok;}

  /**
   * @return number of lat/lons in state
   */
  inline size_t size(void) const {return _latlon.size();}

  /**
   * Value of XML tag
   */ 
  static const std::string _tag;

protected:
private:  

  bool _ok;  /**< True if values are set  */
  /**
   * Mapping from tile index to lat/lon pair
   */
  std::map<int, std::pair<double,double> > _latlon;
};

# endif 
