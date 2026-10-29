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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:15:01 $
//   $Id: ProdQueue.cc,v 1.15 2016/03/03 18:15:01 dixon Exp $
//   $Revision: 1.15 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ProdQueue.cc: class controlling the product queue in shared memory.
 *               This queue is used for communicating between the
 *               product selector and the display.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1997
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cassert>
#include <cstdio>
#include <sys/types.h>
#include <sys/time.h>

#include <symprod/ProdQueue.h>
#include <symprod/symprod.h>
#include <toolsa/ShmemSem.hh>
#include <toolsa/globals.h>
using namespace std;


/**********************************************************************
 * Constructors
 */


// Creator of product queue

ProdQueue::ProdQueue(const int status_area_key,
		     const int buffer_area_key,
		     const int buffer_area_size,
		     const int num_prods,
		     pq_prod_info_t *prod_info)
{
  _handle = PQ_create_creator(status_area_key,
			      buffer_area_key,
			      buffer_area_size,
			      num_prods,
			      prod_info);
  
  assert(_handle != NULL);
}


// User of product queue

ProdQueue::ProdQueue(const int status_area_key,
		     const int buffer_area_key)
{
  // Create the product queue handle

  _handle = PQ_create_user(status_area_key,
			   buffer_area_key);
  
  assert(_handle != NULL);
}


/**********************************************************************
 * Destructor
 */

ProdQueue::~ProdQueue(void)
{
  PQ_destroy(_handle);
}


/**********************************************************************
 * addProduct() - Add a product to the product queue in shared memory.
 *
 * Returns the instance key for the product instance.  This key is used
 * to uniquely identify the created instance of the product.
 *
 * Sets the products_freed flag if any products were freed
 * in adding this product to the queue.  This signals the
 * caller that he might have to update his current product
 * list, if he's keeping one.
 */

int ProdQueue::addProduct(int prod_type,
			  int prod_subtype,
			  time_t generate_time,
			  time_t received_time,
			  time_t start_time,
			  time_t expire_time,
			  int prod_len,
			  char *prod_info,
			  int *products_freed)
{
  return(PQ_add_product(_handle,
			prod_type,
			prod_subtype,
			generate_time,
			received_time,
			start_time,
			expire_time,
			prod_len,
			prod_info,
			products_freed));
}


/**********************************************************************
 * deleteProduct() - Deletes a product from the product queue in shared
 *                   memory.
 */

void ProdQueue::deleteProduct(int prod_type,
			      int instance_key)
{
  PQ_delete_product(_handle, prod_type, instance_key);
  
  return;
}


/**********************************************************************
 * checkDisplayUpdate() - Checks to see if the display has updated the
 *                        product queue.
 *
 * Returns TRUE if the product queue has been updated, FALSE
 * otherwise.
 */

int ProdQueue::checkDisplayUpdate(void)
{
  return(PQ_check_display_update(_handle));
}


/**********************************************************************
 * checkMapFlag() - Returns the current value of the server map flag.
 */

int ProdQueue::checkMapFlag(void)
{
  return(PQ_check_map_flag(_handle));
}


/*****************************************************
 * checkServerUpdate() - Checks to see if the server has updated the
 *                       product queue.
 *
 * Returns TRUE if the product queue has been updated, FALSE
 * otherwise.
 */

int ProdQueue::checkServerUpdate(void)
{
  return(PQ_check_server_update(_handle));
}


/**********************************************************************
 * dataCurrent() - Checks to see if the data requested by the display
 *                 has been received by the server.
 */

int ProdQueue::dataCurrent(void)
{
  return(PQ_data_current(_handle));
}


/**********************************************************************
 * emptyQueue() - Clear out all of the data in the product queue.
 */

void ProdQueue::emptyQueue(void)
{
  PQ_empty_queue(_handle);

  return;
}


/**********************************************************************
 * expireProduct() - Look through all of the products in the product
 *                   queue and set the expired flag for the first
 *                   expired product instance that is encountered.
 *                   This can be used by the writer to expire products
 *                   and still know which products are expired and
 *                   which are still valid.
 *
 * Returns the instance key of the product instance that is expired
 * and set the product type in the argument list appropriately.
 * Returns -1 if all active products are valid.
 */

int ProdQueue::expireProduct(int *prod_type)
{
  return(PQ_expire_product(_handle, prod_type));
}


/**********************************************************************
 * expireProduct() - Manually expire the indicated product instance.
 */

void ProdQueue::expireProductInstance(int instance_key)
{
  PQ_expire_product_instance(_handle, instance_key);
  return;
}


/**********************************************************************
 * expireProducts() - Look through all of the products in the product
 *                    queue and set the expired flag for any product
 *                    that is no longer valid.
 */

void ProdQueue::expireProducts(void)
{
  PQ_expire_products(_handle);
  
  return;
}


/**********************************************************************
 * findProduct() - Find a product type in the status area of the
 *                 shared memory.  Returns a pointer directly into
 *                 shared memory, or NULL if the product type isn't
 *                 found.
 */

pq_prod_t *ProdQueue::findProduct(int prod_type)
{
  return(PQ_find_product(_handle, prod_type));
}


/**********************************************************************
 * findProductInstance() - Find the slot for a particular instance of a
 *                         product.  Returns a pointer directly into
 *                         shared meomry, or NULL if the instance isn't
 *                         found.
 */

pq_slot_t *ProdQueue::findProductInstance(int instance_key)
{
  return(PQ_find_product_instance(_handle, instance_key));
}


/**********************************************************************
 * getDataTime() - Get the current product queue data time.
 */

time_t ProdQueue::getDataTime(void)
{
  return(PQ_get_data_time(_handle));
}


/**********************************************************************
 * getDisplayTime() - Get the current display time from the product
 *                    queue.
 */

time_t ProdQueue::getDisplayTime(void)
{
  return(PQ_get_display_time(_handle));
}


/**********************************************************************
 * getDisplayDataTime() - Get the current display_data_time from the
 *                        product queue.
 */

time_t ProdQueue::getDisplayDataTime(void)
{
  return(PQ_get_display_data_time(_handle));
}


/**********************************************************************
 * getDisplayMovieTime() - Get the current display movie time from the
 *                         product queue.
 */

time_t ProdQueue::getDisplayMovieTime(void)
{
  return(PQ_get_display_movie_time(_handle));
}


/**********************************************************************
 * getProductDisplayInfo() - Gets the current display information for
 *                           the products in the product queue.
 *
 * Note that this routine returns a pointer to static memory
 * which should NOT be freed by the calling routine.  The size
 * of the returned array is returned in the num_products argument.
 */

pq_prod_disp_info_t *ProdQueue::getProductDisplayInfo(int *num_prod_ret)
{
  return(PQ_get_product_display_info(_handle,
				     num_prod_ret));
}


/**********************************************************************
 * print() - Print the product queue contents to the indicated stream
 *           in ASCII format.  If the active_only_flag is set, only
 *           active slots are printed.  If the displayed_only flag is
 *           set, only the displayed slots are printed.  If both the
 *           active_only_flag and the displayed_only_flag are set, only
 *           slots that are both active and displayed are printed.  If
 *           the data_flag is set, the product data in the active slots
 *           is also printed.
 */

void ProdQueue::print(FILE *stream,
		      int active_only_flag,
		      int displayed_only_flag,
		      int data_flag)
{
  PQ_print(_handle, stream, active_only_flag, displayed_only_flag, data_flag);
  return;
}


/**********************************************************************
 * printProduct() - Print the given product information to the
 *                  indicated stream in ASCII format.
 */

void ProdQueue::printProduct(FILE *stream, pq_prod_t *prod_info)
{
  PQ_print_product(_handle, stream, prod_info);
  return;
}


/**********************************************************************
 * printSlot() - Print the given slot information to the indicated
 *               stream in ASCII format.  If the slot is currently
 *               active and data_flag is set, also print the product
 *               data for this slot to the given stream.
 */

void ProdQueue::printSlot(FILE *stream, pq_slot_t *slot_info,
			  int data_flag)
{
  PQ_print_slot(_handle, stream, slot_info, data_flag);
  return;
}


/**********************************************************************
 * printStatus() - Print the status information in shared memory to
 *                 the indicated stream in ASCII format.
 */

void ProdQueue::printStatus(FILE *stream)
{
  PQ_print_status(_handle, stream);
  return;
}


/**********************************************************************
 * setDataTime() - Sets the product queue data time.  This should be
 *                 done after updating the information in the queue and
 *                 the time given should be the requested time being
 *                 satisfied by the server (i.e. when the request is
 *                 satisfied, data_time should equal display_time or
 *                 display_data_time).
 */

void ProdQueue::setDataTime(time_t data_time)
{
  PQ_set_data_time(_handle, data_time);
  return;
}
 

/**********************************************************************
 * setDisplayUpdate() - Sets the display update flag to the given value.
 */

void ProdQueue::setDisplayUpdate(int value)
{
  PQ_set_display_update(_handle, value);
  return;
}


/**********************************************************************
 * setMapFlag() - Set the value of the server map flag.  Also sets the
 *                display update flag.  This will be used by the server
 *                when it first comes up to store it's initial state,
 *                but will just be used by the display after that.
 */

void ProdQueue::setMapFlag(int flag_value)
{
  PQ_set_map_flag(_handle, flag_value);
  return;
}


/**********************************************************************
 * setInstanceDisplay() - Set the display flag for the indicated
 *                        instance of the product of the given type to
 *                        the given state.
 */

void ProdQueue::setInstanceDisplay(int prod_type,
				   int instance_key,
				   int state)
{
  PQ_set_instance_display(_handle, prod_type, instance_key, state);
  return;
}


/**********************************************************************
 * setProductDisplay() - Set the display flag for all products of the
 *                       given type to the given state.
 */

void ProdQueue::setProductDisplay(int prod_type,
				  int state)
{
  PQ_set_product_display(_handle, prod_type, state);
  return;
}


/**********************************************************************
 * setUseDisplayDataTime() - Set the flag indicating whether we are using
 *                           the display_time or the display_data_time
 *                           when retrieving product data.
 *
 * Set state to FALSE to use display_time, to TRUE to use
 * display_data_time.
 */

void ProdQueue::setUseDisplayDataTime(int state)
{
  PQ_set_use_display_data_time(_handle, state);
  return;
}


/*****************************************************
 * updateDisplayTime() - Updates the display time in the product
 *                       queue.  This routine is used by the
 *                       display attached to the product queue to
 *                       indicate changes in archive time displays.
 *                       Set the display time to -1 for realtime
 *                       display.
 *
 * This routine sets and clears all necessary semaphores
 * and flags for the communication with the server.
 */

void ProdQueue::updateDisplayTime(time_t display_time)
{
  PQ_update_display_time(_handle, display_time);
  return;
}


/*****************************************************
 * updateDisplayDataTime() - Updates the display data time in the
 *                           product queue.  This routine is used
 *                           by the display attached to the product
 *                           queue to indicate changes in archive
 *                           data time displays.
 *                           Set the display data time to -1 for realtime
 *                           display.
 *
 * This routine sets and clears all necessary semaphores
 * and flags for the communication with the server.
 */

void ProdQueue::updateDisplayDataTime(time_t display_data_time)
{
  PQ_update_display_data_time(_handle, display_data_time);
  return;
}
