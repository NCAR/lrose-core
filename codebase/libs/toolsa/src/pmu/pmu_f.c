/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/********************************************************************
 *
 * Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
 * February 1998
 *
 ********************************************************************/

#ifdef __cplusplus                                                      
extern "C" {
#endif                                                                      

#if defined(F_UNDERSCORE2)
  #define pmu_f_auto_init pmu_f_auto_init__
  #define pmu_f_auto_register pmu_f_auto_register__
  #define pmu_f_force_register pmu_f_force_register__
  #define pmu_f_auto_unregister pmu_f_auto_unregister__
  #define pmu_f_set_status pmu_f_set_status__
#elif defined(F_UNDERSCORE)
  #define pmu_f_auto_init pmu_f_auto_init_
  #define pmu_f_auto_register pmu_f_auto_register_
  #define pmu_f_force_register pmu_f_force_register_
  #define pmu_f_auto_unregister pmu_f_auto_unregister_
  #define pmu_f_set_status pmu_f_set_status_
#endif

/******************************************************************************                  
 *  pmu_f.c FORTRAN Wrappers for functions to register with process mapper.
 *  Terri Betancourt  June 1997                                                     
 */                      

#include <dataport/port_types.h>
#include <toolsa/pmu.h>

/******************************************
 * PMU_auto_init()
 *
 * Sets up statics for auto regsitration
 */

void pmu_f_auto_init(
   char *prog_name,
   char *instance,
   int  *reg_interval )
{
   PMU_auto_init( prog_name, instance, *reg_interval );
}

/******************************************
 * PMU_auto_register()
 *
 * Automatically registers if Reg_interval secs have expired
 * since the previous registration.
 *
 * This routine may be called frequently - registration will
 * only occur at the specified Reg_interval.
 */

void pmu_f_auto_register( char *status_str )
{
   PMU_auto_register( status_str );
}

/******************************************
 * PMU_force_register()
 *
 * Forced registration.
 *
 * This routine should only be called from places in the code which do
 * not run frequently. Call PMU_auto_register() from most places
 * in the code.
 */

void pmu_f_force_register( char *status_str )
{
   PMU_force_register( status_str );
}

/******************************************
 * PMU_auto_unregister()
 *
 * Automatically unregisters - remembers process name
 * and instance
 */

void pmu_f_auto_unregister()
{
   PMU_auto_unregister();
}

/******************************************
 * PMU_set_status()
 *
 * Set status, force register.
 *
 */

void pmu_f_set_status( si32 *status, char *status_str )
{
   PMU_set_status( *status, status_str );
}

#ifdef __cplusplus
}
#endif
