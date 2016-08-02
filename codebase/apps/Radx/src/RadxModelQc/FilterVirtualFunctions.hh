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
/**
 * @file FilterVirtualFunctions.hh
 * @brief Methods to be included in the Filter classes
 */
#ifdef FILTER_BASE

  /**
   * Debug printing of summary of filter in derived class
   */
  virtual void filter_print(void) const = 0;

  /**
   * @return true if this filter supports beam by beam threading
   */
  virtual bool canThread(void) const = 0;

  /**
   * Apply the filter
   * @param[in] t  Time
   * @param[in] ray0   The previous ray, with all input fields in it, or NULL
   * @param[in] ray   The ray, with all input fields in it.
   * @param[in] ray1   The next ray, with all input fields in it, or NULL
   * @param[in,out] data  The data that results from filtering, augmented by
   *                      this method
   * @return true if filtering was successful
   */
   virtual bool filter(const time_t &t, const RadxRay *ray0,
		       const RadxRay &ray, const RadxRay *ray1,
		       std::vector<RayxData> &data) const = 0;

  /**
   * Apply any filtering that require all rays in the volume
   * @param[in] vol  The volume
   */
  virtual void filterVolume(const RadxVol &vol) = 0;

  /**
   * Apply any state changes to be done after filtering
   */
  virtual void finish(void) = 0;
  

#else

  /**
   * Debug printing of summary of filter in derived class
   */
  virtual void filter_print(void) const;

  /**
   * @return true if this filter supports beam by beam threading
   */
  virtual bool canThread(void) const;

  /**
   * Apply the filter
   * @param[in] t  Time
   * @param[in] ray0   The previous ray, with all input fields in it, or NULL
   * @param[in] ray   The ray, with all input fields in it.
   * @param[in] ray1   The next ray, with all input fields in it, or NULL
   * @param[in,out] data  The data that results from filtering, augmented by
   *                      this method
   * @return true if filtering was successful
   */
   virtual bool filter(const time_t &t, const RadxRay *ray0,
		       const RadxRay &ray, const RadxRay *ray1,
		       std::vector<RayxData> &data) const;


  /**
   * Apply any filtering that require all rays in the volume
   * @param[in] vol  The volume
   */
  virtual void filterVolume(const RadxVol &vol);

  /**
   * Apply any state changes to be done after filtering
   */
  virtual void finish(void);

#endif
