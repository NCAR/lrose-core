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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:25:36 $
 *   $Id: ProdQueue.h,v 1.15 2016/03/03 19:25:36 dixon Exp $
 *   $Revision: 1.15 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * ProdQueue.h : header file for the ProdQueue class.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 1997
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef ProdQueue_H
#define ProdQueue_H

/*
 **************************** includes **********************************
 */

#include <stdio.h>
#include <sys/time.h>

#include <symprod/prod_queue.h>
#include <toolsa/globals.h>
#include <toolsa/ShmemSem.hh>

/*
 ******************************* defines ********************************
 */

/*
 ******************************* structures *****************************
 */


/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class ProdQueue
{
 public:
  
  // Constructors.
  // There will be two users of a product queue - the creator and the
  // user.  Generally, the creator is the process that writes to the
  // queue and the user is the process that reads from the queue.

  // This first Constructor is used by the process that creates the
  // shared memory areas.

  ProdQueue(const int status_area_key,
	    const int buffer_area_key,
	    const int buffer_area_size,
	    const int num_prods,
	    pq_prod_info_t *prod_info);
  
  // This second Constructor is used by the process that just uses the
  // shared memory area (the reader).  In this case, the constructor
  // will block until the shared memory areas and semaphores are created
  // by the creating process.

  ProdQueue(const int status_area_key,
	    const int buffer_area_key);
  
  // Destructor

  ~ProdQueue(void);
      
  // Add a product to the product queue in shared memory.  Returns the
  // instance key for the product instance.  This key is used to uniquely
  // identify the created instance of the product.
  //
  // Sets the products_freed flag if any products were freed
  // in adding this product to the queue.  This signals the
  // caller that he might have to update his current product
  // list, if he's keeping one.

  int addProduct(int prod_type,
		 int prod_subtype,
		 time_t generate_time,
		 time_t received_time,
		 time_t start_time,
		 time_t expire_time,
		 int prod_len,
		 char *prod_info,
                 int *products_freed);
  
  // Deletes a product from the product queue in shared memory.

  void deleteProduct(int prod_type,
                     int instance_key);

  // Checks to see if the display has updated the product queue.
  // Returns TRUE if the product queue has been updated, FALSE
  // otherwise.

  int checkDisplayUpdate(void);
  
  // Returns the current value of the server map flag.

  int checkMapFlag(void);

  // Checks to see if the server has updated the product queue.
  // Returns TRUE if the product queue has been updated, FALSE
  // otherwise.

  int checkServerUpdate(void);

  // Checks to see if the data requested by the display has been
  // received by the server.

  int dataCurrent(void);

  // Clear out all of the data in the product queue.

  void emptyQueue(void);
  
  // Look through all of the products in the product queue and set
  // the expired flag for the first expired product instance that is
  // encountered.  This can be used by the writer to expire products
  // and still know which products are expired and which are still valid.
  //
  // Returns the instance key of the product instance that is expired
  // and set the product type in the argument list appropriately.
  // Returns -1 if all active products are valid.

  int expireProduct(int *prod_type);
  
  // Manually expire the indicated product instance.

  void expireProductInstance(int instance_key);

  // Look through all of the products in the product queue and set
  // the expired flag for any product that is no longer valid.

  void expireProducts(void);
  
  // Find a product type in the status area of the shared memory.
  // Returns a pointer directly into shared memory, or NULL if the
  // product type isn't found.

  pq_prod_t *findProduct(int prod_type);
  
  // Find the slot for a particular instance of a product.  Returns
  // a pointer directly into shared meomry, or NULL if the instance
  // isn't found.

  pq_slot_t *findProductInstance(int instance_key);

  // Get the current product queue data time.

  time_t getDataTime(void);

  // Get the current display time from the product queue.

  time_t getDisplayTime(void);

  // Get the current display_data_time from the product queue.

  time_t getDisplayDataTime(void);

  // Get the current display movie time from the product queue.

  time_t getDisplayMovieTime(void);

  // Gets the current display information for the products in the
  // product queue.
  //
  // Note that this routine returns a pointer to static memory
  // which should NOT be freed by the calling routine.  The size
  // of the returned array is returned in the num_products argument.

  pq_prod_disp_info_t *getProductDisplayInfo(int *num_prod_ret);

  // Set the product queue data time.  This should be done after
  // updating the information in the queue and the time given should
  // be the requested time being satisfied by the server (i.e. when
  // the request is satisfied, data_time should equal display_time).

  void setDataTime(time_t data_time);

  // Sets the display update flag to the given value.

  void setDisplayUpdate(int value);

  // Set the value of the server map flag.  Also sets the display
  // update flag.  This will be used by the server when it first
  // comes up to store it's initial state, but will just be used by
  // the display after that.

  void setMapFlag(int flag_value);

  // Set the display flag for the indicated instance of the product of
  // the given type to the given state.

  void setInstanceDisplay(int prod_type,
			  int instance_key,
			  int state);

  // Set the display flag for all products of the given type to the
  // given state.

  void setProductDisplay(int prod_type,
                         int state);

  // Set the flag indicating whether we are using the display_time or
  // the display_data_time when retrieving product data.
  //
  // Set state to FALSE to use display_time, to TRUE to use
  // display_data_time.

  void setUseDisplayDataTime(int state);
  
  // Updates the display time in the product queue.  This
  // routine is used by the display attached to the product
  // queue to indicate changes in archive time displays.
  // Set the display time to -1 for realtime display.
  //
  // This routine sets and clears all necessary semaphores
  // and flags for the communication with the server.

  void updateDisplayTime(time_t display_time);

  // Updates the display data time in the product queue.  This
  // routine is used by the display attached to the product
  // queue to indicate changes in archive time displays.
  // Set the display time to -1 for realtime display.
  //
  // This routine sets and clears all necessary semaphores
  // and flags for the communication with the server.

  void updateDisplayDataTime(time_t display_time);


  ///////////////////////////////////////////////////////
  // PRINT MEMBER FUNCTIONS
  ///////////////////////////////////////////////////////

  // Print the product queue contents to the indicated stream in
  // ASCII format.  If the active_only_flag is set, only active
  // slots are printed.  If the displayed_only flag is set, only
  // the displayed slots are printed.  If both the active_only_flag
  // and the displayed_only_flag are set, only slots that are both
  // active and displayed are printed.  If the data_flag is set, the
  // product data in the active slots is also printed.

  void print(FILE *stream,
             int active_only_flag = FALSE,
             int displayed_only_flag = FALSE,
             int data_flag = TRUE);

  // Print the given product information to the indicated stream in
  // ASCII format.

  void printProduct(FILE *stream, pq_prod_t *prod_info);

  // Print the product structure for the product with the given index

  void printProductByIndex(FILE *stream, int prod_index)
  {
    printProduct(stream, &_handle->status_ptr->prod_info[prod_index]);
    return;
  }
  
  // Print the indicated slot for the indicated product

  void printProductSlot(FILE *stream, 
			int prod_index,
			int slot_index,
			int print_data = TRUE)
  {
    int slot_num = getSlot(prod_index, slot_index);
    
    printSlot(stream, &_handle->slot_ptr[slot_num], print_data);
  }
  
  // Print the given slot information to the indicated stream in ASCII
  // format.  If the slot is currently active and data_flag is set,
  // also print the product data for this slot to the given stream.

  void printSlot(FILE *stream, pq_slot_t *slot_info,
                 int data_flag = TRUE);

  // Print the status information in shared memory to the indicated
  // stream in ASCII format.

  void printStatus(FILE *stream);

  ///////////////////////////////////////////////////////
  // ACCESS MEMBER FUNCTIONS
  ///////////////////////////////////////////////////////

  int getNumProducts(void)
  {
    return(_handle->status_ptr->num_prods);
  }
  
  int getNumSlots(int prod_index)
  {
    return(_handle->status_ptr->prod_info[prod_index].num_slots);
  }

  int getSlot(int prod_index, int slot_index)
  {
    return(_handle->status_ptr->prod_info[prod_index].begin_slot + slot_index);
  }
  
  int checkSlotActive(int prod_index,
		      int slot_index)
  {
    int slot_num = getSlot(prod_index, slot_index);
    return(_handle->slot_ptr[slot_num].active);
  }
  
 private:

  pq_handle_t *_handle;

  const char *const _className(void)
  {
    return("ProdQueue");
  }

};


#endif
