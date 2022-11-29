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
 * @file RadxPersistentClutterVirtualMethods.hh
 * @brief Methods, to be included in other include files
 */
#ifdef MAIN

/**
 * Initialization step.
 *
 * @param[in] t   First processing time
 * @param[in] vol  First volume of data at the processing time
 *
 * These are actions to take only on the first data 
 */
virtual void initFirstTime(const time_t &t, const RadxVol &vol) = 0;

/**
 * Completion step (good)
 *
 * @param[in] t   Last processing time
 * @param[in] vol  Last volume of data at the processing time
 *
 * These are actions to take only on the last data, after it is processed
 */
virtual void finishLastTimeGood(const time_t &t, RadxVol &vol) = 0;

/**
 * Completion step (bad)
 *
 * Actions to take when the 'good' state was never achieved
 */
virtual void finishBad(void) = 0;

/**
 * Complete the processing of a volume of data.
 *
 * @param[in] t   volume time
 * @param[in] vol  volume of data at t
 *
 * @return true for success
 *
 * These are the steps taken after each ray has been processed
 */
virtual bool processFinishVolume(const time_t &t, RadxVol &vol) = 0;

/**
 * Pre-process a ray.
 *
 * @param[in] ray   The data
 *
 * @return true if successful
 *
 * These are actions taken prior to normal ray processing
 */
virtual bool preProcessRay(const RadxRay &ray) = 0;

/**
 * Normal ray processing
 * @param[in] r   The ray data
 * @param[in,out]  Pointer to the storage object, updated
 * 
 * @return true for success
 */
virtual bool processRay(const RayData &r,
			RayClutterInfo *i) const = 0;

/**
 * Modify a ray prior to output
 *
 * @param[in] h  Pointer to storage object (state)
 * @param[in] r  The data
 * @param[in,out]  ray  The object that is updated
 *
 * @return true for success
 */
virtual bool setRayForOutput(const RayClutterInfo *h, const RayData &r,
			     RadxRay &ray) = 0;

/**
 * @return non-const pointer to the storage object state for the input location,
 *         NULL for no match.
 * @param[in] az   Ray azimuth
 * @param[in] elev  Ray elevation
 */
virtual RayClutterInfo *matchingClutterInfo(const double az,
					    const double elev) = 0;
/**
 * @return const pointer to the storage object state for the input location,
 *         NULL for no match.
 * @param[in] az   Ray azimuth
 * @param[in] elev  Ray elevation
 */
virtual const
RayClutterInfo * matchingClutterInfoConst(const double az,
					  const double elev) const = 0;


#else


/**
 * Initialization step.
 *
 * @param[in] t   First processing time
 * @param[in] vol  First volume of data at the processing time
 *
 * These are actions to take only on the first data 
 */
virtual void initFirstTime(const time_t &t, const RadxVol &vol);

/**
 * Completion step (good)
 *
 * @param[in] t   Last processing time
 * @param[in] vol  Last volume of data at the processing time
 *
 * These are actions to take only on the last data, after it is processed
 */
virtual void finishLastTimeGood(const time_t &t, RadxVol &vol);

/**
 * Completion step (bad)
 *
 * Actions to take when the 'good' state was never achieved
 */
virtual void finishBad(void);

/**
 * Complete the processing of a volume of data.
 *
 * @param[in] t   volume time
 * @param[in] vol  volume of data at t
 *
 * @return true for success
 *
 * These are the steps taken after each ray has been processed
 */
virtual bool processFinishVolume(const time_t &t, RadxVol &vol);

/**
 * Pre-process a ray.
 *
 * @param[in] ray   The data
 *
 * @return true if successful
 *
 * These are actions taken prior to normal ray processing
 */
virtual bool preProcessRay(const RadxRay &ray);

/**
 * Normal ray processing
 * @param[in] r   The ray data
 * @param[in,out]  Pointer to the storage object, updated
 * 
 * @return true for success
 */
virtual bool processRay(const RayData &r, RayClutterInfo *i) const;

/**
 * Modify a ray prior to output
 *
 * @param[in] h  Pointer to storage object (state)
 * @param[in] r  The data
 * @param[in,out]  ray  The object that is updated
 *
 * @return true for success
 */
virtual bool setRayForOutput(const RayClutterInfo *h, const RayData &r,
			     RadxRay &ray);

/**
 * @return non-const pointer to the storage object state for the input location,
 *         NULL for no match.
 * @param[in] az   Ray azimuth
 * @param[in] elev  Ray elevation
 */
virtual RayClutterInfo *matchingClutterInfo(const double az,
					    const double elev);
/**
 * @return const pointer to the storage object state for the input location,
 *         NULL for no match.
 * @param[in] az   Ray azimuth
 * @param[in] elev  Ray elevation
 */
virtual const
RayClutterInfo * matchingClutterInfoConst(const double az,
					  const double elev) const;
#endif
