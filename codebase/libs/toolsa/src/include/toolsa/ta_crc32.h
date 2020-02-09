/*******************************************************************
 * ta_crc32.h 
 * Copyright (c) 2011-2019 Stephan Brumme. All rights reserved. 
 * Based on code by Stephan Brumme 
 * Slicing-by-16 contributed by Bulat Ziganshin 
 * Tableless bytewise CRC contributed by Hagai Gold 
 * see http://create.stephan-brumme.com/disclaimer.html 
 *
 * Modified by Mike Dixon, EOL, NCAR, Boulder, CO, 80301
 * Feb 2020
 ********************************************************************/

#ifndef ta_crc32_h
#define ta_crc32_h

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
  
/* compute CRC32 */
  
extern uint32_t ta_crc32(const void* data, size_t length);
  
#ifdef __cplusplus
}
#endif

#endif
