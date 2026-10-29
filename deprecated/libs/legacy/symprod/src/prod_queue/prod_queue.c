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
/*********************************************************************
 * prod_queue.c
 *
 * Routines to access a product queue maintained in shared memory.
 * The ProdQueue module of the Symprod class library contains a class
 * which uses these routines.
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * April 1997
 *
 *********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>

#include <toolsa/os_config.h>

#include <symprod/prod_queue.h>
#include <symprod/symprod.h>
#include <toolsa/globals.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

/*
 * Local defines
 */

/*
 * Local typedefs
 */

/*
 * Global variables
 */

static int Last_prod_instance = 0;

/*
 * file scope prototypes
 */

static char *create_shmem_and_sem(int key, int size, int *sem_id);
static void free_buffer_data(pq_handle_t *handle,
			     int buffer_offset);
static void free_buffer_space(pq_handle_t *handle);
static void free_slot(pq_handle_t *handle, int slot_num);
static char *get_shmem_and_sem(int key, int *sem_id);
static void set_sem_override(int sem_id,
			     int override_secs);

/*****************************************************
 * CONSTRUCTORS
 *****************************************************/

/*****************************************************
 * PQ_create_creator()
 *
 * Constructor to be used by the creator of a product queue shared
 * memory area.  This routine creates and initializes the shared
 * memory areas and stores needed information in the handle.
 *
 * Returns a pointer to the product queue handle created, or NULL
 * on error.  This handle is used by the rest of the routines in
 * the product queue module to identify this product queue.
 */

pq_handle_t *PQ_create_creator(int status_shmem_key,
			       int buffer_shmem_key,
			       int buffer_shmem_size,
			       int num_prods,
			       pq_prod_info_t *prod_info)
{
  int prod;
  int slot;
  int slot_offset;

  int stat_header_size;
  int stat_shmem_size;
  int total_slots;
  
  pq_handle_t *handle = (pq_handle_t *)umalloc(sizeof(pq_handle_t));
  
  int begin_slot;
  
  /*
   * Calculate the total number of slots so we can initialize the
   * shared memory.  Note that we subtract one from the number of
   * products when calculating the header size because pq_stat_t
   * contains a single pq_prod_t structure.
   */

  total_slots = 0;
  
  for (prod = 0; prod < num_prods; prod++)
    total_slots += prod_info[prod].num_slots;
  
  stat_header_size = sizeof(pq_stat_t) + (num_prods - 1) * sizeof(pq_prod_t);
  stat_shmem_size = stat_header_size + total_slots * sizeof(pq_slot_t);
  
  /*
   * Create the shared memory areas.
   */

  if ((handle->status_ptr =
       (pq_stat_t *)create_shmem_and_sem(status_shmem_key,
					 stat_shmem_size,
					 &handle->status_sem_id)) == NULL)
  {
    ufree(handle);
    return(NULL);
  }
  
  if ((handle->buffer_ptr =
       create_shmem_and_sem(buffer_shmem_key,
			    buffer_shmem_size,
			    &handle->buffer_sem_id)) == NULL)
  {
    ufree(handle);
    return(NULL);
  }
  
  /*
   * Initialize the handle information.
   */

  handle->status_area_key = status_shmem_key;
  handle->buffer_area_key = buffer_shmem_key;
  
  handle->creator_flag = TRUE;
  
  handle->semaphores_set = FALSE;
  
  handle->slot_ptr = (pq_slot_t *)((char *)handle->status_ptr +
				   stat_header_size);
  
  /*******************************************
   * Initialize the shared memory information.
   */

  /*
   * Initialize the information in the status area
   */

  handle->status_ptr->server_update_flag = FALSE;
  handle->status_ptr->display_update_flag = FALSE;
  handle->status_ptr->total_slots = total_slots;
  handle->status_ptr->slot_offset = stat_header_size;
  handle->status_ptr->buf_size = buffer_shmem_size;
  handle->status_ptr->num_prods = num_prods;
  handle->status_ptr->use_display_data_time = FALSE;
  handle->status_ptr->display_time = -1;
  handle->status_ptr->display_data_time = -1;
  handle->status_ptr->data_time = -2;
  handle->status_ptr->map_flag = TRUE;
  handle->status_ptr->begin_insert = 0;
  handle->status_ptr->end_insert = buffer_shmem_size;
  handle->status_ptr->begin_append = buffer_shmem_size;
  
  begin_slot = 0;
  
  for (prod = 0; prod < num_prods; prod++)
  {
    handle->status_ptr->prod_info[prod].type = prod_info[prod].type;
    STRcopy(handle->status_ptr->prod_info[prod].label,
	    prod_info[prod].label, PQ_LABEL_LEN);
    handle->status_ptr->prod_info[prod].num_slots = prod_info[prod].num_slots;
    handle->status_ptr->prod_info[prod].begin_slot = begin_slot;
    handle->status_ptr->prod_info[prod].latest_slot_offset = -1;
    handle->status_ptr->prod_info[prod].oldest_slot_offset = -1;
    handle->status_ptr->prod_info[prod].display = 1;
    
    /*
     * Initialize the product index in the slot table while we're
     * here.  The rest of the slot table is initialized below.
     */

    for (slot_offset = 0;
	 slot_offset < prod_info[prod].num_slots;
	 slot_offset++)
      handle->slot_ptr[begin_slot + slot_offset].prod_index = prod;
    
    begin_slot += prod_info[prod].num_slots;
  } /* endfor - prod */
  
  /*
   * Initialize the rest of the slot table.  The prod_index value
   * is initialized above.
   */

  for (slot = 0; slot < total_slots; slot++)
  {
    handle->slot_ptr[slot].active = 0;
    handle->slot_ptr[slot].expired = 0;
    handle->slot_ptr[slot].display = 0;
    handle->slot_ptr[slot].generate_time = -1;
    handle->slot_ptr[slot].received_time = -1;
    handle->slot_ptr[slot].start_time = -1;
    handle->slot_ptr[slot].expire_time = -1;
    handle->slot_ptr[slot].len = 0;
    handle->slot_ptr[slot].offset = 0;
    handle->slot_ptr[slot].subtype = 0;
  } /* endfor - slot */
  
  return(handle);
}


/*****************************************************
 * PQ_check_create_user()
 *
 * Constructor for user access to a product queue shared
 * memory area.  This routine checks to see if the shared
 * memory areas exist and if they do it calls
 * PQ_create_user(). This routine will not block. Use this
 * constructor if one wishes the client to avoid blocking
 * while waiting for the PQ to be created. Call this
 * periodically within the application until it returns
 * a non-NULL handle.
 *
 * Returns a pointer to the product queue handle created
 * or NULL, if no PQ exists currently.
 * The handle is used by the rest of the routines in
 * the product queue module to identify this product queue.
 */

pq_handle_t *PQ_check_create_user(int status_shmem_key,
			    int buffer_shmem_key)
{
  if (ushm_exists(status_shmem_key) &&
      ushm_exists(buffer_shmem_key))
    return(PQ_create_user(status_shmem_key,
			  buffer_shmem_key));
  else
    return((pq_handle_t *)NULL);
}


/*****************************************************
 * PQ_create_user()
 *
 * Constructor to be used by the user of a product queue shared
 * memory area.  This routine accesses the shared memory areas
 * and stores needed information in the handle.  Because this
 * routine is accessing shared memory rather than creating it,
 * it blocks until the shared memory is created.
 *
 * Returns a pointer to the product queue handle created, or NULL
 * on error.  This handle is used by the rest of the routines in
 * the product queue module to identify this product queue.
 */

pq_handle_t *PQ_create_user(int status_shmem_key,
			    int buffer_shmem_key)
{
  pq_handle_t *handle = (pq_handle_t *)umalloc(sizeof(pq_handle_t));
  
  /*
   * Access the shared memory areas.
   */

  if ((handle->status_ptr =
       (pq_stat_t *)get_shmem_and_sem(status_shmem_key,
				      &handle->status_sem_id)) == NULL)
  {
    ufree(handle);
    return(NULL);
  }
  
  if ((handle->buffer_ptr =
       get_shmem_and_sem(buffer_shmem_key,
			 &handle->buffer_sem_id)) == NULL)
  {
    ufree(handle);
    return(NULL);
  }
  
  /*
   * Initialize the handle information.
   */

  handle->status_area_key = status_shmem_key;
  handle->buffer_area_key = buffer_shmem_key;
  
  handle->creator_flag = FALSE;
  
  handle->semaphores_set = FALSE;

  handle->slot_ptr = (pq_slot_t *)((char *)handle->status_ptr +
				   handle->status_ptr->slot_offset);
  
  return(handle);
}


/*****************************************************
 * DESTRUCTOR
 *****************************************************/

/*****************************************************
 * PQ_destroy()
 *
 * Destructor to be used by both the creator and the user of a
 * product queue shared memory area.  This routine frees all
 * memory used by the handle and destroys the shared memory, if
 * appropriate.
 */

void PQ_destroy(pq_handle_t *handle)
{
  /*
   * Make sure the semaphores are cleared in case we don't delete
   * them.
   */

  PQ_clear_semaphores(handle);
  
  /*
   * Detach from the shared memory segment
   */

  ushm_detach((void *)handle->status_ptr);
  ushm_detach((void *)handle->buffer_ptr);
  
  /*
   * Remove the shared memory and semaphores, if appropriate.
   */

  if (ushm_nattach(handle->status_area_key) <= 0)
  {
    usem_remove(handle->status_area_key);
    ushm_remove(handle->status_area_key);
  }
  
  if (ushm_nattach(handle->buffer_area_key) <= 0)
  {
    usem_remove(handle->buffer_area_key);
    ushm_remove(handle->buffer_area_key);
  }
  
  /*
   * Free the space used by the handle.
   */

  ufree(handle);
  
  return;
}


/*****************************************************
 * MEMBER FUNCTIONS
 *****************************************************/

/*****************************************************
 * PQ_add_product()
 *
 * Add a product to the product queue in shared memory.
 *
 * Returns the instance key for the product instance.  This key is
 * used to uniquely identify the created instance of the product.
 *
 * Sets the products_freed flag if any products were freed
 * in adding this product to the queue.  This signals the
 * caller that he might have to update his current product
 * list, if he's keeping one.
 */

int PQ_add_product(pq_handle_t *handle,
		   int prod_type,
		   int prod_subtype,
		   time_t generate_time,
		   time_t received_time,
		   time_t start_time,
		   time_t expire_time,
		   int prod_len,
		   char *prod_info,
		   int *products_freed)
{
  static char *routine_name = "PQ_add_product";
  
  pq_prod_t *product;
  int slot;
  int shmem_len;
  char *buffer_ptr;
  pq_begin_buffer_info_t *begin_buffer_info;
  pq_end_buffer_info_t *end_buffer_info;
  int instance_key;
  
  /*
   * Initialize returned values.
   */

  *products_freed = FALSE;
  
  /*
   * Find the product entry in the status area.  We don't need the
   * semaphores set for this since we're just searching.
   */

  if ((product = PQ_find_product(handle, prod_type)) == NULL)
  {
    fprintf(stderr,
	    "WARNING: prod_queue::%s\n",
	    routine_name);
    fprintf(stderr,
	    "Product type %d not found in shared memory, product not added\n",
	    prod_type);
    return(-1);
  }
  
  /*
   * Determine the instance key for this product instance.
   */

  instance_key = (Last_prod_instance + 1) % PQ_MAX_INSTANCE_KEY;
  Last_prod_instance = instance_key;
  
  /*
   * Set the semaphores so we can start updating the shared memory
   */

  PQ_set_semaphores(handle);
  
  /*
   * Clear area until we can fit the new product into the product
   * buffer.
   */

  shmem_len = prod_len + sizeof(pq_begin_buffer_info_t) +
    sizeof(pq_end_buffer_info_t);
  
  if (shmem_len > handle->status_ptr->buf_size)
  {
    fprintf(stderr, "ERROR: prod_queue::%s\n", routine_name);
    fprintf(stderr, "Product too big for shared memory\n");
    fprintf(stderr, "Product needs %d bytes, shared memory has %d bytes\n",
	    shmem_len, handle->status_ptr->buf_size);
    fprintf(stderr, "SKIPPING PRODUCT\n\n");
    
    PQ_clear_semaphores(handle);
    
    return(-1);
  }
  
  while (handle->status_ptr->end_insert - handle->status_ptr->begin_insert
	 < shmem_len)
  {
    *products_freed = TRUE;
    free_buffer_space(handle);
  }
  
  /*
   * Find the slot where the product should be added
   */

  if (product->latest_slot_offset < 0)
    slot = product->begin_slot;
  else
  {
    int slot_offset;
    
    slot_offset = product->latest_slot_offset + 1;
    if (slot_offset >= product->num_slots)
      slot_offset = 0;
    
    slot = product->begin_slot + slot_offset;
  }
  
  /*
   * Add the product info to the product buffer
   */

  buffer_ptr = handle->buffer_ptr + handle->status_ptr->begin_insert;
  begin_buffer_info = (pq_begin_buffer_info_t *)buffer_ptr;
  end_buffer_info =
    (pq_end_buffer_info_t *)(buffer_ptr +
			     sizeof(pq_begin_buffer_info_t) + prod_len);
			     
  begin_buffer_info->active = TRUE;
  begin_buffer_info->len = shmem_len;
  begin_buffer_info->slot_num = slot;
  
  end_buffer_info->len = shmem_len;
  
  buffer_ptr += sizeof(pq_begin_buffer_info_t);
  
  memcpy(buffer_ptr, prod_info, prod_len);
  
  /*
   * Update the product slotting information.  If we are reusing
   * a slot, free the slot.
   */

  if (handle->slot_ptr[slot].active)
  {
    *products_freed = TRUE;
    free_slot(handle, slot);
  }
  
  product->latest_slot_offset = slot - product->begin_slot;
  if (product->oldest_slot_offset < 0)
    product->oldest_slot_offset = product->latest_slot_offset;
  
  /*
   * Update the slot information
   */

  handle->slot_ptr[slot].active = 1;
  handle->slot_ptr[slot].expired = 0;
  handle->slot_ptr[slot].display = 1;
  handle->slot_ptr[slot].instance_key = instance_key;
  handle->slot_ptr[slot].generate_time = generate_time;
  handle->slot_ptr[slot].received_time = received_time;
  handle->slot_ptr[slot].start_time = start_time;
  handle->slot_ptr[slot].expire_time = expire_time;
  handle->slot_ptr[slot].len = shmem_len;
  handle->slot_ptr[slot].offset = handle->status_ptr->begin_insert;
  handle->slot_ptr[slot].subtype = prod_subtype;
  
  /*
   * Update the status information
   */

  handle->status_ptr->begin_insert += shmem_len;
  handle->status_ptr->server_update_flag = TRUE;
  
  /*
   * Clear the semaphores
   */

  PQ_clear_semaphores(handle);
  
  return(instance_key);
}


/*****************************************************
 * PQ_delete_product()
 *
 * Deletes a product from the product queue in shared memory.
 */

void PQ_delete_product(pq_handle_t *handle,
		       int prod_type,
		       int instance_key)
{
  static char *routine_name = "PQ_delete_product";
  
  pq_slot_t *slot;
  pq_begin_buffer_info_t *begin_buffer_info;
  
  int slot_prod_type;
  
  /*
   * Find the slot to update.
   */

  if ((slot = PQ_find_product_instance(handle, instance_key)) == NULL)
    return;
  
  /*
   * Make sure this slot contains the correct type of product.
   */

  slot_prod_type = handle->status_ptr->prod_info[slot->prod_index].type;
  
  if (prod_type != slot_prod_type)
  {
    fprintf(stderr,
	    "WARNING: prod_queue::%s\n", routine_name);
    fprintf(stderr,
	    "Given product type %d does not match slot product type %d\n",
	    prod_type, slot_prod_type);
    fprintf(stderr,
	    "Product NOT deleted\n");
    
    return;
  }
  
  /*
   * Delete the slot.
   */

  if (slot->active)
  {
    /*
     * Find the associated buffer area for the data so we can get
     * the slot number.
     */

    begin_buffer_info = (pq_begin_buffer_info_t *)(handle->buffer_ptr +
					       slot->offset);
  
    /*
     * Now free the slot.
     */

    free_slot(handle, begin_buffer_info->slot_num);
  }
  
}


/*****************************************************
 * PQ_check_display_update()
 *
 * Checks to see if the display has updated the product queue.
 *
 * Returns TRUE if the product queue has been updated, FALSE
 * otherwise.
 */

int PQ_check_display_update(pq_handle_t *handle)
{
  return(handle->status_ptr->display_update_flag);
}


/*****************************************************
 * PQ_check_map_flag()
 *
 * Returns the current value of the server map flag.
 */

int PQ_check_map_flag(pq_handle_t *handle)
{
  return(handle->status_ptr->map_flag);
}


/*****************************************************
 * PQ_check_server_update()
 *
 * Checks to see if the server has updated the product queue.
 *
 * Returns TRUE if the product queue has been updated, FALSE
 * otherwise.
 */

int PQ_check_server_update(pq_handle_t *handle)
{
  return(handle->status_ptr->server_update_flag);
}


/*****************************************************
 * PQ_clear_semaphores()
 *
 * Clear the semaphores associated with the product queue shared
 * memory areas.  Only clear them if they were set by us.
 */

void PQ_clear_semaphores(pq_handle_t *handle)
{
  if (handle->semaphores_set)
  {
    usem_clear(handle->status_sem_id, 0);
    usem_clear(handle->buffer_sem_id, 0);
  }
  
  handle->semaphores_set = FALSE;
  
  return;
}


/*****************************************************
 * PQ_data_current()
 *
 * Checks to see if the data requested by the display
 * has been received by the server.
 */

int PQ_data_current(pq_handle_t *handle)
{
  return(PQ_get_display_time(handle) == PQ_get_data_time(handle));
}


/*****************************************************
 * PQ_empty_queue()
 *
 * Clear out all of the data in the product queue.
 */

void PQ_empty_queue(pq_handle_t *handle)
{
  int product;
  int slot;
  pq_slot_t *slot_info;
  pq_begin_buffer_info_t *begin_buffer_info;
  pq_prod_t *product_info;
  pq_stat_t *status_info;
  
  /*
   * Set the semaphores before beginning.
   */

  PQ_set_semaphores(handle);
  
  /*
   * Clear out each of the slots
   */

  for (slot = 0; slot < handle->status_ptr->total_slots; slot++)
  {
    slot_info = &(handle->slot_ptr[slot]);
    
    if (slot_info->active)
    {
      begin_buffer_info =
	(pq_begin_buffer_info_t *)(handle->buffer_ptr + slot_info->offset);
      
      begin_buffer_info->active = FALSE;
      
      slot_info->active = FALSE;
    }
  } /* endfor - slot */
  
  /*
   * Update the product information
   */

  for (product = 0; product < handle->status_ptr->num_prods; product++)
  {
    product_info = &(handle->status_ptr->prod_info[product]);
    
    product_info->latest_slot_offset = -1;
    product_info->oldest_slot_offset = -1;
  } /* endfor - product */
  
  /*
   * Update the status information
   */

  status_info = handle->status_ptr;
  
  status_info->begin_insert = 0;
  status_info->end_insert = status_info->buf_size;
  status_info->begin_append = status_info->buf_size;
  
  /*
   * Tell the user that the buffer has been changed.
   */

  status_info->server_update_flag = TRUE;
  
  /*
   * Clear the semaphores before returning.
   */

  PQ_clear_semaphores(handle);
  
  return;
}


/*****************************************************
 * PQ_expire_product_instance()
 *
 * Manually expire the indicated product instance.
 */

void PQ_expire_product_instance(pq_handle_t *handle,
				int instance_key)
{
  pq_slot_t *slot;

  /*
   * Find the slot to update.
   */

  if ((slot = PQ_find_product_instance(handle, instance_key)) == NULL)
    return;

  /*
   * Set the semaphores so we can update the shared memory area.
   */

  PQ_set_semaphores(handle);
  
  slot->expired = 1;
  slot->display = 0;
  
  handle->status_ptr->server_update_flag = TRUE;

  /*
   * Clear the semaphores.
   */

  PQ_clear_semaphores(handle);

}


/*****************************************************
 * PQ_expire_product()
 *
 * Look through all of the products in the product queue and set
 * the expired flag for the first expired product instance that is
 * encountered.  This can be used by the writer to expire products
 * and still know which products are expired and which are still
 * valid.
 *
 * Returns the instance key of the product instance that is expired
 * and set the product type in the argument list appropriately.
 * Returns -1 if all active products are valid.
 */

int PQ_expire_product(pq_handle_t *handle,
		      int *prod_type)
{
  int slot;
  time_t current_time = time(NULL);
  int instance_key;

  /*
   * Loop through the products, checking for expirations
   */

  for (slot = 0; slot < handle->status_ptr->total_slots; slot++)
  {
    if (handle->slot_ptr[slot].active &&
	!handle->slot_ptr[slot].expired &&
	handle->slot_ptr[slot].expire_time < current_time)
    {
      /*
       * Save the information to be returned.
       */

      instance_key = handle->slot_ptr[slot].instance_key;
      *prod_type =
	handle->status_ptr->prod_info[handle->slot_ptr[slot].prod_index].type;
      
      /*
       * Set the semaphores so we can update the shared memory area.
       */

      PQ_set_semaphores(handle);
  
      handle->slot_ptr[slot].expired = 1;
      handle->slot_ptr[slot].display = 0;
  
      handle->status_ptr->server_update_flag = TRUE;

      /*
       * Clear the semaphores.
       */

      PQ_clear_semaphores(handle);

      /*
       * Return since we found an expired slot.
       */

      return(instance_key);
    }
    
  } /* endfor - slot */
  
  return(-1);
}


/*****************************************************
 * PQ_expire_products()
 *
 * Look through all of the products in the product queue and set
 * the expired flag for any product that is no longer valid.
 */

void PQ_expire_products(pq_handle_t *handle)
{
  int expired_instance;
  int prod_type;
  
  do
  {
    expired_instance = PQ_expire_product(handle, &prod_type);
  } while (expired_instance >= 0);
  
  return;
}


/*****************************************************
 * PQ_find_product()
 *
 * Find a product type in the status area of the shared memory.
 *
 * Returns a pointer directly into shared memory, or NULL if the
 * product type isn't found.
 */

pq_prod_t *PQ_find_product(pq_handle_t *handle,
			   int prod_type)
{
  int prod;
  
  for (prod = 0; prod < handle->status_ptr->num_prods; prod++)
    if (handle->status_ptr->prod_info[prod].type == prod_type)
      return(&(handle->status_ptr->prod_info[prod]));
  
  return(NULL);
}


/*****************************************************
 * PQ_find_product_instance()
 *
 * Find the slot for a particular instance of a product.
 *
 * Returns a pointer directly into shared memory, or NULL if the
 * instance isn't found.
 */

pq_slot_t *PQ_find_product_instance(pq_handle_t *handle,
				    int instance_key)
{
  int slot;
  
  for (slot = 0; slot < handle->status_ptr->total_slots; slot++)
    if (handle->slot_ptr[slot].instance_key == instance_key &&
	handle->slot_ptr[slot].active)
      return(&(handle->slot_ptr[slot]));
  
  return(NULL);
}


/*****************************************************
 * PQ_get_data_time()
 *
 * Get the current product queue data time.
 */

time_t PQ_get_data_time(pq_handle_t *handle)
{
  return(handle->status_ptr->data_time);
}

/*****************************************************
 * PQ_get_display_time()
 *
 * Get the current display time from the product queue.
 */

time_t PQ_get_display_time(pq_handle_t *handle)
{
  if (handle->status_ptr->use_display_data_time)
    return(handle->status_ptr->display_data_time);
  
  return(handle->status_ptr->display_time);
}


/*****************************************************
 * PQ_get_display_data_time()
 *
 * Get the current display_data_time from the product queue.
 */

time_t PQ_get_display_data_time(pq_handle_t *handle)
{
  return(handle->status_ptr->display_data_time);
}


/*****************************************************
 * PQ_get_display_movie_time()
 *
 * Get the current display_time from the product queue.
 */

time_t PQ_get_display_movie_time(pq_handle_t *handle)
{
  return(handle->status_ptr->display_time);
}


/*****************************************************
 * PQ_get_product_display_info()
 *
 * Gets the current display information for the products in
 * the product queue.
 *
 * Note that this routine returns a pointer to static memory
 * which should NOT be freed by the calling routine.  The size
 * of the returned array is returned in the num_products argument.
 */

pq_prod_disp_info_t *PQ_get_product_display_info(pq_handle_t *handle,
						 int *num_prod_ret)
{
  static pq_prod_disp_info_t *prod_disp_info = NULL;
  static int first_time = TRUE;
  static int num_products = 0;
  
  int i;
  
  if (first_time)
  {
    num_products = handle->status_ptr->num_prods;
    
    prod_disp_info =
      (pq_prod_disp_info_t *)umalloc(num_products *
				     sizeof(pq_prod_disp_info_t));
    
    for (i = 0; i < num_products; i++)
    {
      prod_disp_info[i].prod_type =
	handle->status_ptr->prod_info[i].type;
      STRcopy(prod_disp_info[i].label,
	      handle->status_ptr->prod_info[i].label,
	      PQ_LABEL_LEN);
    }
    
    first_time = FALSE;
  }
  
  for (i = 0; i < num_products; i++)
    prod_disp_info[i].display_flag =
      handle->status_ptr->prod_info[i].display;
  
  *num_prod_ret = num_products;
  
  return(prod_disp_info);
}


/*****************************************************
 * PQ_print()
 *
 * Print the information in the product queue to the given stream
 * in ASCII format.  If the active_only_flag is set, only active
 * slots are printed.  If the displayed_only flag is set, only the
 * displayed slots are printed.  If both the active_only_flag and
 * the displayed_only_flag are set, only slots that are both active
 * and displayed are printed.  If the data_flag is set, the product data
 * in the active slots is also printed.
 */

void PQ_print(pq_handle_t *handle,
	      FILE *stream,
	      int active_only_flag,
	      int displayed_only_flag,
	      int data_flag)
{
  int clear_sem_flag = FALSE;
  int slot;
  
  /*
   * Set the semaphores if they are not already set.  This is done to
   * prevent any other process from changing the shared memory area
   * while we are printing, which could cause the print to fail.
   */

  if (!handle->semaphores_set)
  {
    PQ_set_semaphores(handle);
    clear_sem_flag = TRUE;
  }
  
  /*
   * Print the information
   */

  PQ_print_status(handle, stream);
  
  for (slot = 0; slot < handle->status_ptr->total_slots; slot++)
    if ((!active_only_flag && !displayed_only_flag) ||
	(active_only_flag && displayed_only_flag &&
	 handle->slot_ptr[slot].active &&
	 handle->slot_ptr[slot].display) ||
	(active_only_flag && handle->slot_ptr[slot].active) ||
	(displayed_only_flag && handle->slot_ptr[slot].display))
      PQ_print_slot(handle, stream, &handle->slot_ptr[slot], data_flag);
  
  /*
   * Clear the semaphores if we are the routine that set them.
   */

  if (clear_sem_flag)
    PQ_clear_semaphores(handle);
  
  return;
}

/*****************************************************
 * PQ_print_buffer_info()
 *
 * Print the given buffer information to the indicated
 * stream in ASCII format.
 */

void PQ_print_buffer_info(pq_handle_t *handle,
			  FILE *stream,
			  pq_begin_buffer_info_t *begin_buffer_info)
{
  int clear_sem_flag = FALSE;
  
  /*
   * Set the semaphores if they are not already set.  This is done to
   * prevent any other process from changing the shared memory area
   * while we are printing, which could cause the print to fail.
   */

  if (!handle->semaphores_set)
  {
    PQ_set_semaphores(handle);
    clear_sem_flag = TRUE;
  }
  
  /*
   * Print the information
   */

  fprintf(stream, "\n");
  fprintf(stream, "active = %d\n", begin_buffer_info->active);
  fprintf(stream, "len = %d\n", begin_buffer_info->len);
  fprintf(stream, "slot_num = %d\n", begin_buffer_info->slot_num);
  fprintf(stream, "\n");
  
  /*
   * Clear the semaphores if we are the routine that set them.
   */

  if (clear_sem_flag)
    PQ_clear_semaphores(handle);
  
  return;
}


/*****************************************************
 * PQ_print_product()
 *
 * Print the given product information to the indicated stream
 * in ASCII format.
 */

void PQ_print_product(pq_handle_t *handle,
		      FILE *stream,
		      pq_prod_t *prod_info)
{
  int clear_sem_flag = FALSE;
  
  /*
   * Set the semaphores if they are not already set.  This is done to
   * prevent any other process from changing the shared memory area
   * while we are printing, which could cause the print to fail.
   */

  if (!handle->semaphores_set)
  {
    PQ_set_semaphores(handle);
    clear_sem_flag = TRUE;
  }
  
  /*
   * Print the information
   */

  fprintf(stream, "\n");
  fprintf(stream, "type = %d\n", prod_info->type);
  fprintf(stream, "label = <%s>\n", prod_info->label);
  fprintf(stream, "num_slots = %d\n", prod_info->num_slots);
  fprintf(stream, "begin_slot = %d\n", prod_info->begin_slot);
  fprintf(stream, "latest_slot_offset = %d\n", prod_info->latest_slot_offset);
  fprintf(stream, "oldest_slot_offset = %d\n", prod_info->oldest_slot_offset);
  fprintf(stream, "display = %d\n", prod_info->display);
  fprintf(stream, "\n");

  /*
   * Clear the semaphores if we are the routine that set them.
   */

  if (clear_sem_flag)
    PQ_clear_semaphores(handle);
  
  return;
}


/*****************************************************
 * PQ_print_slot()
 *
 * Print the given slot information to the indicated stream in
 * ASCII format.  If the slot is currently active and the
 * data_flag is set, also print the product data for this slot
 * to the given stream.  If the data_flag is set, the product
 * data in the active slots is also printed.
 */

void PQ_print_slot(pq_handle_t *handle,
		   FILE *stream,
		   pq_slot_t *slot_info,
		   int data_flag)
{
  int clear_sem_flag = FALSE;
  
  /*
   * Set the semaphores if they are not already set.  This is done to
   * prevent any other process from changing the shared memory area
   * while we are printing, which could cause the print to fail.
   */

  if (!handle->semaphores_set)
  {
    PQ_set_semaphores(handle);
    clear_sem_flag = TRUE;
  }
  
  /*
   * Print the information
   */

  fprintf(stream, "\n");
  fprintf(stream, "active = %d\n", slot_info->active);
  fprintf(stream, "expired = %d\n", slot_info->expired);
  fprintf(stream, "display = %d\n", slot_info->display);
  fprintf(stream, "prod_index = %d\n", slot_info->prod_index);
  fprintf(stream, "instance_key = %d\n", slot_info->instance_key);
  fprintf(stream, "generate_time = %s\n",
	  utimstr(slot_info->generate_time));
  fprintf(stream, "received_time = %s\n",
	  utimstr(slot_info->received_time));
  fprintf(stream, "start_time = %s\n",
	  utimstr(slot_info->start_time));
  fprintf(stream, "expire_time = %s\n",
	  utimstr(slot_info->expire_time));
  fprintf(stream, "len = %d\n", slot_info->len);
  fprintf(stream, "offset = %d\n", slot_info->offset);
  fprintf(stream, "subtype = %d\n", slot_info->subtype);
  
  fprintf(stream, "\n");
  
  /*
   * Print the data if the slot is active
   */

  if (data_flag && slot_info->active)
  {
    pq_begin_buffer_info_t *begin_buffer_info =
      (pq_begin_buffer_info_t *)(handle->buffer_ptr + slot_info->offset);
    char *prod_info =
      (char *)begin_buffer_info + sizeof(pq_begin_buffer_info_t);
    
    fprintf(stream, "Beginning buffer info:\n");
    
    PQ_print_buffer_info(handle, stream, begin_buffer_info);
    
    fprintf(stream, "Slot data:\n\n");
    
    SYMPRODprintProductBuffer(stream, prod_info);
    
  } /* endif - slot_info->active */
  
  /*
   * Clear the semaphores if we are the routine that set them.
   */

  if (clear_sem_flag)
    PQ_clear_semaphores(handle);
  
  return;
}

/*****************************************************
 * PQ_print_status()
 *
 * Print the status information in shared memory to the indicated
 * stream in ASCII format.
 */

void PQ_print_status(pq_handle_t *handle,
		     FILE *stream)
{
  int clear_sem_flag = FALSE;
  int prod;
  
  /*
   * Set the semaphores if they are not already set.  This is done to
   * prevent any other process from changing the shared memory area
   * while we are printing, which could cause the print to fail.
   */

  if (!handle->semaphores_set)
  {
    PQ_set_semaphores(handle);
    clear_sem_flag = TRUE;
  }
  
  /*
   * Print the information
   */

  fprintf(stream, "\n");
  fprintf(stream, "Status information:\n");
  fprintf(stream, "\n");
  fprintf(stream, "server_update_flag = %d\n",
	  handle->status_ptr->server_update_flag);
  fprintf(stream, "display_update_flag = %d\n",
	  handle->status_ptr->display_update_flag);
  fprintf(stream, "total_slots = %d\n", handle->status_ptr->total_slots);
  fprintf(stream, "slot_offset = %d\n", handle->status_ptr->slot_offset);
  fprintf(stream, "buf_size = %d\n", handle->status_ptr->buf_size);
  fprintf(stream, "num_prods = %d\n", handle->status_ptr->num_prods);
  if (handle->status_ptr->display_time > 0)
    fprintf(stream, "display_time = %s\n",
	    utimstr(handle->status_ptr->display_time));
  else
    fprintf(stream, "display_time = %ld\n", handle->status_ptr->display_time);
  if (handle->status_ptr->display_data_time > 0)
    fprintf(stream, "display_data_time = %s\n",
	    utimstr(handle->status_ptr->display_data_time));
  else
    fprintf(stream, "display_data_time = %ld\n",
	    handle->status_ptr->display_data_time);
  if (handle->status_ptr->data_time > 0)
    fprintf(stream, "data_time = %s\n",
	    utimstr(handle->status_ptr->data_time));
  else
    fprintf(stream, "data_time = %ld\n", handle->status_ptr->data_time);
  fprintf(stream, "map_flag = %d\n", handle->status_ptr->map_flag);
  fprintf(stream, "begin_insert = %d\n", handle->status_ptr->begin_insert);
  fprintf(stream, "end_insert = %d\n", handle->status_ptr->end_insert);
  fprintf(stream, "begin_append = %d\n", handle->status_ptr->begin_append);
  fprintf(stream, "\n");
  
  for (prod = 0; prod < handle->status_ptr->num_prods; prod++)
  {
    fprintf(stream, "Product #%d:\n", prod);
    fprintf(stream, "\n");
    PQ_print_product(handle, stream, &handle->status_ptr->prod_info[prod]);
  } /* endfor - prod */
      
  fprintf(stream, "\n");
  
  /*
   * Clear the semaphores if we are the routine that set them.
   */

  if (clear_sem_flag)
    PQ_clear_semaphores(handle);
  
  return;
}


/*****************************************************
 * PQ_set_data_time()
 *
 * Set the product queue data time.  This should be done
 * after updating the information in the queue and the
 * time given should be the requested time being satisfied
 * by the server (i.e. when the request is satisfied,
 * data_time should equal display_time or display_data_time,
 * depending on the value of use_display_data_time).
 */

void PQ_set_data_time(pq_handle_t *handle,
		      time_t data_time)
{
  /*
   * Just set the data_time value.  No need to use the
   * semaphores since this action should be autonomous.
   */

  handle->status_ptr->data_time = data_time;
  
  return;
}

/*****************************************************
 * PQ_set_display_update()
 *
 * Sets the display update flag to the given value.  Does not
 * set or clear the shared memory semaphores since this is an
 * autonomous action.
 */

void PQ_set_display_update(pq_handle_t *handle, int value)
{
  handle->status_ptr->display_update_flag = value;
  
  return;
}


/*****************************************************
 * PQ_set_instance_display()
 *
 * Set the display flag for the indicated instance of the product
 * of the given type to the given state.
 */

void PQ_set_instance_display(pq_handle_t *handle,
			     int prod_type,
			     int instance_key,
			     int state)
{
  static char *routine_name = "PQ_set_instance_display";
  
  pq_slot_t *slot;
  int slot_prod_type;
  
  /*
   * Find the product instance
   */

  if ((slot = PQ_find_product_instance(handle, instance_key))
      == (pq_slot_t *)NULL)
  {
    fprintf(stderr,
	    "WARNING: prod_queue::%s\n", routine_name);
    fprintf(stderr,
	    "Product instance %d not found in active queue\n",
	    instance_key);
    fprintf(stderr,
	    "Product display state NOT changed\n");
    
    return;
  }
  
  /*
   * Make sure this slot contains the correct type of product.
   */

  slot_prod_type = handle->status_ptr->prod_info[slot->prod_index].type;
  
  if (prod_type != slot_prod_type)
  {
    fprintf(stderr,
	    "WARNING: prod_queue::%s\n", routine_name);
    fprintf(stderr,
	    "Given product type %d does not match slot product type %d\n",
	    prod_type, slot_prod_type);
    fprintf(stderr,
	    "Product display state NOT changed\n");
    
    return;
  }
  
  /*
   * Update the display state for the product instance.
   */

  if (slot->display != state)
  {
    PQ_set_semaphores(handle);
  
    slot->display = state;

    handle->status_ptr->server_update_flag = TRUE;
      
    PQ_clear_semaphores(handle);
  }

  return;
}


/*****************************************************
 * PQ_set_map_flag()
 *
 * Set the value of the server map flag.  Also sets the
 * display update flag.  This will be used by the server
 * when it first comes up to store it's initial state,
 * but will just be used by the display after that.
 */

void PQ_set_map_flag(pq_handle_t *handle,
		     int flag_value)
{
  /*
   * Just set the map_flag value.  No need to use the
   * semaphores since this action should be autonomous.
   */

  handle->status_ptr->map_flag = flag_value;
  handle->status_ptr->display_update_flag = TRUE;
  
  return;
}

/*****************************************************
 * PQ_set_product_display()
 *
 * Set the display flags for all of the products of the given type
 * to the given state.
 */

void PQ_set_product_display(pq_handle_t *handle,
			    int prod_type,
			    int state)
{
  pq_prod_t *prod;

  if ((prod = PQ_find_product(handle, prod_type)) == NULL)
    return;

  if (prod->display != state)
  {
    /*
     * Atomic operation, don't need to set semaphores
     */

    prod->display = state;
    
    handle->status_ptr->server_update_flag = TRUE;
    handle->status_ptr->display_update_flag = TRUE;
  }
  
  return;
}


/*****************************************************
 * PQ_set_use_display_data_time()
 *
 * Set the flag indicating whether we are using the display_time
 * or the display_data_time when retrieving product data.
 *
 * Set state to FALSE to use display_time, to TRUE to use
 * display_data_time.
 */

void PQ_set_use_display_data_time(pq_handle_t *handle,
				  int state)
{
  handle->status_ptr->use_display_data_time = state;
  handle->status_ptr->display_update_flag = TRUE;
  
  return;
}


/*****************************************************
 * PQ_set_semaphores()
 *
 * Set the semaphores associated with the product queue shared
 * memory areas.
 */

void PQ_set_semaphores(pq_handle_t *handle)
{
  if (!handle->semaphores_set)
  {
    set_sem_override(handle->status_sem_id,
		     PQ_OVERRIDE_SECS);
    set_sem_override(handle->buffer_sem_id,
		     PQ_OVERRIDE_SECS);
  }
  
  handle->semaphores_set = TRUE;
  
  return;
}


/*****************************************************
 * PQ_set_server_update()
 *
 * Sets the server update flag to the given value.  Does not
 * set or clear the shared memory semaphores since this is an
 * autonomous action.
 */

void PQ_set_server_update(pq_handle_t *handle, int value)
{
  handle->status_ptr->server_update_flag = value;
  
  return;
}


/*****************************************************
 * PQ_update_display_time()
 *
 * Updates the display time in the product queue.  This
 * routine is used by the display attached to the product
 * queue to indicate that the user has selected a new
 * display time.
 * Set the display time to -1 for realtime display.
 */

void PQ_update_display_time(pq_handle_t *handle,
			    time_t display_time)
{
  /*
   * Since both of these operations are autonomous, I'm
   * not setting the semaphores here.
   */

  if (handle->status_ptr->display_time != display_time)
  {
    handle->status_ptr->display_time = display_time;
  
    handle->status_ptr->display_update_flag = TRUE;
  }
  
  return;
}


/*****************************************************
 * PQ_update_display_data_time()
 *
 * Updates the display data time in the product queue.  This
 * routine is used by the display attached to the product
 * queue to indicate that the currently displayed data time
 * has changed.  
 * Set the display data time to -1 for realtime display.
 */

void PQ_update_display_data_time(pq_handle_t *handle,
				 time_t display_data_time)
{
  /*
   * Since both of these operations are autonomous, I'm
   * not setting the semaphores here.
   */

  if (handle->status_ptr->display_data_time != display_data_time)
  {
    handle->status_ptr->display_data_time = display_data_time;
  
    handle->status_ptr->display_update_flag = TRUE;
  }
  
  return;
}


/*************************************************
 * STATIC functions
 *************************************************/

/*****************************************************
 * create_shmem_and_sem()
 *
 * Create the shared memory segment and semaphore identified by the
 * given key.
 */

static char *create_shmem_and_sem(int key, int size, int *sem_id)
{
  char *buffer;

  /*
   * Create the shared memory segment.
   */

  if ((buffer = (char *)ushm_create(key, size, 0666)) == NULL)
    return(NULL);
  
  /*
   * Clear the shared memory
   */

  memset((void *)buffer, (int)0, size);
  
  /*
   * Create the semaphore.
   */

  if ((*sem_id = usem_create(key, 1, 0666)) < 0)
  {
    /* ushm_remove(key); */ /* tmp - see dixon */
    return(NULL);
  }
  
  return(buffer);
}


/*****************************************************
 * free_buffer_space()
 *
 * Free the oldest space in the buffer area.  Note that this routine
 * assumes that the semaphores for the shared memory areas have
 * already been set by the calling routine.
 */

static void free_buffer_space(pq_handle_t *handle)
{
  pq_begin_buffer_info_t *begin_buffer_info;
  
  fprintf(stderr,
	  "---> Entering free_buffer_space\n");
  
  /*
   * If we are at the end of the product buffer and there still isn't
   * enough space, start deleting stuff at the beginning of the
   * buffer.
   */

  if (handle->status_ptr->end_insert ==
      handle->status_ptr->buf_size)
  {
    handle->status_ptr->begin_append = handle->status_ptr->begin_insert;
    handle->status_ptr->end_insert = 0;
    handle->status_ptr->begin_insert = 0;
    
    return;
  }
  
  /*
   * Free the slot associated with the data
   */

  begin_buffer_info = (pq_begin_buffer_info_t *)(handle->buffer_ptr +
					     handle->status_ptr->end_insert);
  
  if (begin_buffer_info->active)
    free_slot(handle, begin_buffer_info->slot_num);
  else
    free_buffer_data(handle, handle->status_ptr->end_insert);
  
  return;
}


/*****************************************************
 * free_slot()
 *
 * Free up the given slot so it can be re-used.  This routine
 * assumes that any semaphores are set appropriately before the
 * routine is called.
 */

static void free_slot(pq_handle_t *handle, int slot_num)
{
  static char *routine_name = "free_slot";
  
  int prod;
  pq_prod_t *product;
  int buffer_offset = handle->slot_ptr[slot_num].offset;
  
  int i;
  
  prod = handle->slot_ptr[slot_num].prod_index;
  product = &handle->status_ptr->prod_info[prod];
  
  /*
   * Reset all of the offsets for the product.
   */

  if (slot_num < product->begin_slot ||
      slot_num > product->begin_slot + product->num_slots - 1)
  {
    /*
     * The slot number is outside of the range of slots for this
     * product.
     */

    fprintf(stderr,
	    "ERROR: prod_queue::%s\n", routine_name);
    fprintf(stderr,
	    "Shared memory is corrupted.\n");
    fprintf(stderr,
	    "Trying to delete slot %d, begin_slot = %d, end_slot = %d\n",
	    slot_num, product->begin_slot,
	    product->begin_slot + product->num_slots - 1);

    return;
  }
  else if (slot_num == product->begin_slot + product->latest_slot_offset)
  {
    handle->slot_ptr[slot_num].active = 0;
    
    /*
     * Deleting the last slot in the list
     */
	
    if (product->latest_slot_offset == product->oldest_slot_offset)
    {
      product->latest_slot_offset = -1;
      product->oldest_slot_offset = -1;
    }
    else
    {
      product->latest_slot_offset--;
      if (product->latest_slot_offset < 0)
	product->latest_slot_offset = product->num_slots - 1;
    }
  }
  else if (slot_num == product->begin_slot + product->oldest_slot_offset)
  {
    handle->slot_ptr[slot_num].active = 0;
    
    /*
     * Deleting the first slot in the list
     */

    product->oldest_slot_offset++;
    if (product->oldest_slot_offset >=
	product->num_slots)
      product->oldest_slot_offset = 0;
  }
  else
  {
    /*
     * Deleting a slot in the middle.  Shift the list around it.
     */

    if (product->latest_slot_offset > product->oldest_slot_offset ||
	slot_num < product->begin_slot + product->latest_slot_offset)
    {
      /*
       * Either there is no wrap-around in list over end of circular
       * queue or the queue wraps around and the slot to be deleted is
       * in the beginning part of the queue.
       */

      int i;
      
      for (i = slot_num;
	   i < product->begin_slot + product->latest_slot_offset;
	   i++)
      {
	pq_begin_buffer_info_t *begin_buffer_info = 
	  (pq_begin_buffer_info_t *)(handle->buffer_ptr +
				     handle->slot_ptr[i + 1].offset);
	
	handle->slot_ptr[i] = handle->slot_ptr[i+1];
	begin_buffer_info->slot_num = i;
      }
      
      handle->slot_ptr[product->begin_slot +
		       product->latest_slot_offset].active = 0;
      
      product->latest_slot_offset--;
    }
    else
    {
      /*
       * The list wraps around in circular queue and the slot to be
       * deleted is in the ending part of the queue.
       */

      for (i = slot_num - 1;
	   i >= product->begin_slot + product->oldest_slot_offset; i--)
      {
	pq_begin_buffer_info_t *begin_buffer_info =
	  (pq_begin_buffer_info_t *)(handle->buffer_ptr +
				     handle->slot_ptr[i].offset);
	
	handle->slot_ptr[i + 1] = handle->slot_ptr[i];
	begin_buffer_info->slot_num = i + 1;
      }
      
      handle->slot_ptr[product->begin_slot +
		      product->oldest_slot_offset].active = 0;
      
      product->oldest_slot_offset++;
    }
    
    }

  /*
   * Free the buffer data.
   */

  free_buffer_data(handle, buffer_offset);
  
  return;
}


/*****************************************************
 * free_buffer_data()
 *
 * Free up the given shared memory buffer data.  Also updates the
 * insert area for the buffer, if appropriate.
 */

static void free_buffer_data(pq_handle_t *handle,
			     int buffer_offset)
{
  pq_begin_buffer_info_t *begin_buffer_info =
    (pq_begin_buffer_info_t *)(handle->buffer_ptr + buffer_offset);
  
  /*
   * Set the buffer as inactive.
   */

  begin_buffer_info->active = 0;

  /*
   * Update the buffer insert area, if appropriate.
   */

  if (buffer_offset == handle->status_ptr->end_insert)
  {
    handle->status_ptr->end_insert += begin_buffer_info->len;
      
    if (handle->status_ptr->end_insert >=
	handle->status_ptr->begin_append)
    {
      handle->status_ptr->end_insert = handle->status_ptr->buf_size;
      handle->status_ptr->begin_append = handle->status_ptr->buf_size;
    }
  }
  
  return;
}


/*****************************************************
 * get_shmem_and_sem()
 *
 * Access the shared memory segment and semaphore identified by the
 * given key.  Blocks until both are created by another process.
 */

static char *get_shmem_and_sem(int key, int *sem_id)
{
  char *buffer;

  /*
   * Attach to the shared memory segment.
   */

   if ((buffer = (char *)ushm_get(key, 0)) == NULL)
    return(NULL);
  
  /*
   * Attach to the semaphore.
   */

  if ((*sem_id = usem_get(key, 1)) < 0)
    return(NULL);
    
  return(buffer);
}


/*****************************************************
 * set_sem_override()
 *
 * Waits until the semaphore is cleared or until the given number
 * of seconds has passed and then sets the semaphore.
 */

static void set_sem_override(int sem_id,
			     int override_secs)
{
  int i;
  struct timeval tv = { 1,0 };
  
  for (i = 0; i < override_secs; i++)
  {
    if (usem_check(sem_id, 0) == 0)
      break;
    
    select(0,0,0,0,&tv);
  }
  
  usem_set(sem_id, 0);

  return;
}
