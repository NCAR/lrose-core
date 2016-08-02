// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <copyright.h>

/**
 * @file MdvPlaybackData.hh
 * @brief  Specification of one data to be played back, including time values
 * @class MdvPlaybackData
 * @brief Specification of one data to be played back, including time values
 *
 */

# ifndef    MDV_PLAYBACK_DATA_HH
# define    MDV_PLAYBACK_DATA_HH

#include <string>
#include <vector>
#include <Mdv/DsMdvx.hh>
class ParmsMdvPlayback;
class MdvPlayback;

//------------------------------------------------------------------
class MdvPlaybackData
{
public:

  /**
   * Default constructor for flat data
   *
   * @param[in] t    Data time
   * @param[in] index  Index into vector of MdvPlaybackData
   * @param[in] url  Input URL
   * @param[in] useDataTime  True to ignore time_written in playback and 
   *                         instead use data_time
   * @param[in] pth  The strings used to convert input URL's to output URL's
   *
   * @note uses Mdv server to fill in state
   */
  MdvPlaybackData(const time_t &t, const int index, const std::string &url,
		  const bool useDataTime,
		  const std::vector<std::pair<std::string, std::string> > &pth);

  /**
   * Default constructor for forecast data
   *
   * @param[in] gt    Data gen time
   * @param[in] lt    Data lead time
   * @param[in] index  Index into vector of MdvPlaybackData
   * @param[in] url  Input URL
   * @param[in] useDataTime  True to ignore time_written in playback and 
   *                         instead use data_time
   * @param[in] pth  The strings used to convert input URL's to output URL's
   *
   * @note uses Mdv server to fill in state
   */
  MdvPlaybackData(const time_t &gt, const int lt, const int index,
		  const std::string &url, const bool useDataTime,
		  const std::vector<std::pair<std::string, std::string> > &pth);

  /**
   * Destructor
   */
  virtual ~MdvPlaybackData(void);

  /**
   * Set index to input
   * @param[in] index
   */
  inline void setIndex(const int index) { pDataIndex = index;}

  inline int getIndex(void) const {return pDataIndex;}

  /**
   * Set alg to input
   * @param[in] alg
   */
  inline void setAlg(MdvPlayback *alg) { pAlg = alg; }
  inline MdvPlayback *getAlg(void) { return pAlg; }


  /**
   * Set this to be next written
   */
  inline void setNextWritten(void) { pIsNextWrite = true;}

  inline bool isNextWrite(void) { return pIsNextWrite;}

  /**
   * @return true if this object is in range for playback, using playback parms
   *
   * @param[in] p  The playback parms.
   */
  bool inRange(const ParmsMdvPlayback &p) const;

  /**
   * Print this playback data showing written time, gen/lead times,
   * input/output URLS, and optionally the real time
   *
   * @param[in] index  Index value to show
   * @param[in] showRealtime   True to show the real time
   */
  void print(const int index, const bool showRealtime=false) const;

  /**
   * Read the data in from the input url and write it to the output url
   */
  void read(void);
  bool write(void);

  /**
   * @return true if time of one data is less than that of another
   *
   * @param[in] d0  Data to compare
   * @param[in] d1  Data to compare
   *
   *
   * Should return true if the time of d0 is less than the time of d1.
   * If time_written values are unequal, use those, if those are the same,
   * and URLs ARE the same, use the data gen/lead times. If all else fails,
   * return true as it doesn't matter
   */
  static bool lessThan( const MdvPlaybackData &d0, const MdvPlaybackData &d1);

  /**
   * @return the times of all data in the range of times for a URL, flat files
   *
   * @param[in] url  URL
   * @param[in] t0  earliest allowed time
   * @param[in] t1  latest allowed time
   *
   * @note uses DsMdvx
   */
  static std::vector<time_t> obsInRange(const std::string &url,
					const time_t &t0,
					const time_t &t1);

  /**
   * @return gen/lead pairs for all data with gen time in the range of times
   * for a URL, forecasts
   *
   * @param[in] url  URL
   * @param[in] t0  earliest allowed gen time
   * @param[in] t1  latest allowed gen time
   *
   * @note uses DsMdvx
   */
  static std::vector<std::pair<time_t,int> > fcstInRange(const std::string &url,
							 const time_t &t0,
							 const time_t &t1);

  /**
   * @return true if flat data exists in a URL at a time
   * @param[in] url
   * @param[in] t
   */
  static bool obsExists(const std::string &url, const time_t &t);

  /**
   * @return true if forecast data exists in a URL at a time
   * @param[in] url
   * @param[in] gt  Gen time
   * @param[in] lt  lead time
   */
  static bool fcstExists(const std::string &url, const time_t &gt,
			 const int lt);


  time_t pGt;     /**< Gen time of forecast data or data time of flat data */
  bool pHasLt;    /**< True for forecast data */
  int pLt;        /**< Lead seconds if pHasLt=true, ignored otherwise */
  time_t pWt;     /**< Time written */
  std::string pUrl;  /**< Input URL */
  std::string pOutUrl;  /**< Output URL */

  DsMdvx pData;    /**< the data, empty unless pHasData=true */
  bool pHasData;   /**< True if pData has the data */
  bool pHasBadData;/**< True if pData has bad (non-writable) data */
  int pDataIndex;  /**< Index into ordered vector of MdvPlaybackData */
  bool pIsNextWrite; /**< True if this is the next thing to write */
  
  MdvPlayback *pAlg;  /**< Algorithm pointer */

protected:
private:  

  void pFormOutUrl(const std::vector<std::pair<std::string, std::string> > &p);
  void pSetWrittenTime(const time_t &t, const int lt, const bool useDataTime);
  void pSetWrittenTime(const time_t &t, const bool useDataTime);

};

# endif
