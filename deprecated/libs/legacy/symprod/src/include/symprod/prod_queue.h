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
#ifdef __cplusplus
 extern "C" {
#endif


/******************************************************************
 * prod_queue.h: header file for the product queue module of the
 *               symprod library.
 *
 ******************************************************************/

#ifndef prod_queue_h
#define prod_queue_h

#include <stdio.h>
#include <sys/time.h>

/*
 * Number of seconds to wait for getting a semaphore before we decide
 * that the other process has died and we override the semaphore.
 */

#define PQ_OVERRIDE_SECS      30

/*
 * Maximum value for the unique instance key for a product instance.
 */

#define PQ_MAX_INSTANCE_KEY   99999

/*
 * Length of the product label field.
 */

#define PQ_LABEL_LEN          80


/*
 * The product queue consists of two areas of shared memory.  The first
 * area, called the status area, contains status and product information
 * while the second, called the buffer area, contains the actual product
 * data.
 *
 * The status area consists of a status structure - pq_stat_t defined
 * below - followed by total_slots slot structures - pq_slot_t defined
 * below.
 *
 * The buffer area is a large buffer containing product data in the
 * SYMPROD format - described in symprod/symprod.h.  Each product is
 * preceded by a pq_buffer_info_t structure containing information
 * used for processing the data.  The offset given in the slot table is
 * the offset to the buffer informatio data in the buffer area, not the
 * offset to the data itself.  To get to the data itself, add
 * sizeof(pq_buffer_info_t) to this offset.
 *
 * Users of the product queue should look in the status area for
 * "interesting" product information and then use that slot information
 * to find the actual product data.
 */

typedef struct
{
  int type;               /* product type */
  char label[PQ_LABEL_LEN];
                          /* product label */
  int num_slots;          /* number of slots dedicated to this product type */
  int begin_slot;         /* first slot in slot table used by this */
                          /*   product type */
  int latest_slot_offset; /* offset from begin_slot of slot containing the */
                          /*   latest message for this product type. */
                          /*   -1 when there are no messages */
  int oldest_slot_offset; /* offset from begin_slot of slot containing the */
                          /*   oldest message for this product type. */
                          /*   -1 when there are no messages */
  int display;            /* flag indicating whether this product type */
                          /*   should currently be displayed.  If 1, the */
                          /*   product instances will be displayed based on */
                          /*   their individual display flags.  If 0, no */
                          /*   instances of this product type will be */
                          /*   displayed. */
} pq_prod_t;


typedef struct
{
  int server_update_flag; /* flag indicating if the server (prod_sel) has */
                          /*   made any updates to the shared memory areas */
  int display_update_flag;/* flag indicating if the display has made any */
                          /*   updates to the shared memory areas */
  int total_slots;        /* total number of message slots */
  int slot_offset;        /* offset in the status area of the beginning of */
                          /*   of the slot table */
  int buf_size;           /* size of buffer */
  int num_prods;          /* number of product types in queue */
  int use_display_data_time;
                          /* Flag indicating whether to use display_time or */
                          /*   display_data_time when retrieving data. */
  time_t display_time;    /* Unix time the display.  This is set by the */
                          /*   display.  The server should use this time */
                          /*   when requesting data if use_display_data_time */
                          /*   is FALSE.  This is the display used to */
                          /*   request its field data. */
                          /*   -1 for realtime */
  time_t display_data_time;
                          /* Unix time of data currently being displayed by */
                          /*   the display.  The server should use this time */
                          /*   when requesting data if use_display_data_time */
                          /*   is TRUE.  This is the time of the currently */
                          /*   displayed field data retrieved by the display */
                          /*   using display_time. */
                          /*   -1 for realtime */
  time_t data_time;       /* Unix time of products contained in the queue. */
                          /*   This is set by the server (prod_sel). */
                          /*   The display can then use this to see if */
                          /*   the requested data has been received. */
                          /*   -1 for realtime */
  int map_flag;           /* Flag indicating whether the prod_sel window */
                          /*   should be mapped or unmapped.  This is set by */
                          /*   the display.  The initial mapped state of the */
                          /*   prod_sel window is determined by a command */
                          /*   line parameter or a parameter file flag in */
                          /*   the prod_sel parameter file.  After that, */
                          /*   prod_sel will watch this flag and map and */
                          /*   unmap accordingly. */
  int begin_insert;       /* offset to start of insert free region in buffer */
  int end_insert;         /* offset to end of insert free region in buffer */
  int begin_append;       /* offset to start of append free region in buffer */
  pq_prod_t prod_info[1]; /* information about each product type in queue. */
                          /*   (repeats num_prods times) */
} pq_stat_t;


typedef struct
{
  int active;             /* active flag, 1 or 0 */
  int expired;            /* expired flag, 1 or 0 */
  int display;            /* display flag, 1 or 0 */
  int prod_index;         /* index of this product in the product */
                          /*   array in the status structure */
  int instance_key;       /* key which uniquely identifies this instance */
                          /*   of the product */
  time_t generate_time;   /* Unix time at which product was generated */
  time_t received_time;   /* Unix time at which product was received */
  time_t start_time;      /* Unix time at which product begins to be valid */
  time_t expire_time;     /* Unix time at which product expires */
  int len;                /* product info len in bytes */
  int offset;             /* product info offset in buffer */
  int subtype;            /* product subtype - user-defined */
} pq_slot_t;

typedef struct
{
  int active;             /* indicates if this space is currently being */
                          /*   used.  This is needed for when we run out */
                          /*   of slots for a particular product type and */
                          /*   reuse the slot.  We need to also be able to */
                          /*   reuse the data area when our circular buffer */
                          /*   gets to it. */
  int len;                /* length, in bytes, of buffer space.  Includes */
                          /*   the pq_begin_buffer_info_t and */
                          /*   pq_end_buffer_t structures. */
  int slot_num;           /* if the space is active, this is the slot number */
                          /*   of the slot pointing to this data.  This is */
                          /*   needed so we can reclaim the slot, too. */
} pq_begin_buffer_info_t;

typedef struct
{
  int len;                /* length, in bytes, of buffer space.  Includes */
                          /*   the pq_begin_buffer_info_t and */
                          /*   pq_end_buffer_t structures. */
} pq_end_buffer_info_t;


/*
 * The following structure, pq_prod_info_t, is used in the creator
 * Constructor to initialize the slot information.
 */

typedef struct
{
  int type;
  int num_slots;
  char label[PQ_LABEL_LEN];
} pq_prod_info_t;

/*
 * The product queue handle, pq_handle_t, is like the this pointer in
 * the ProdQueue class.  It is used to identify and store information
 * about the product queue.  The information in this handle can be
 * looked at by the user, but should never be changed directly.
 */

typedef struct
{
  int status_area_key;
  int status_sem_id;
  
  int buffer_area_key;
  int buffer_sem_id;
  
  int creator_flag;
  
  int semaphores_set;
  
  pq_stat_t *status_ptr;
  pq_slot_t *slot_ptr;
  char *buffer_ptr;
  
} pq_handle_t;

/*
 * The following structure, pq_prod_disp_info_t, is returned by the
 * PQ_get_product_display_info() routine to return the product
 * display information.
 */

typedef struct
{
  int prod_type;
  char label[PQ_LABEL_LEN];
  int display_flag;
} pq_prod_disp_info_t;


/*****************************************************
 * FUNCTION PROTOTYPES
 *****************************************************/

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
			       pq_prod_info_t *prod_info);

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
                            int buffer_shmem_key);
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
			    int buffer_shmem_key);

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

void PQ_destroy(pq_handle_t *handle);

/*****************************************************
 * SERVER-SIDE MEMBER FUNCTIONS
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
		   int *products_freed);

/*****************************************************
 * PQ_delete_product()
 *
 * Deletes a product from the product queue in shared memory.
 */

void PQ_delete_product(pq_handle_t *handle,
		       int prod_type,
		       int instance_key);

/*****************************************************
 * PQ_check_display_update()
 *
 * Checks to see if the display has updated the product queue.
 *
 * Returns TRUE if the product queue has been updated, FALSE
 * otherwise.
 */

int PQ_check_display_update(pq_handle_t *handle);

/*****************************************************
 * PQ_empty_queue()
 *
 * Clear out all of the data in the product queue.
 */

void PQ_empty_queue(pq_handle_t *handle);

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
		      int *prod_type);

/*****************************************************
 * PQ_expire_product_instance()
 *
 * Manually expire the indicated product instance.
 */

void PQ_expire_product_instance(pq_handle_t *handle,
				int instance_key);

/*****************************************************
 * PQ_expire_products()
 *
 * Look through all of the products in the product queue and set
 * the expired flag for any product that is no longer valid.
 */

void PQ_expire_products(pq_handle_t *handle);

/*****************************************************
 * PQ_find_product()
 *
 * Find a product type in the status area of the shared memory.
 *
 * Returns a pointer directly into shared memory, or NULL if the
 * product type isn't found.
 */

pq_prod_t *PQ_find_product(pq_handle_t *handle,
			   int prod_type);

/*****************************************************
 * PQ_find_product_instance()
 *
 * Find the slot for a particular instance of a product.
 *
 * Returns a pointer directly into shared memory, or NULL if the
 * instance isn't found.
 */

pq_slot_t *PQ_find_product_instance(pq_handle_t *handle,
				    int instance_key);

/*****************************************************
 * PQ_get_data_time()
 *
 * Get the current product queue data time.
 */

time_t PQ_get_data_time(pq_handle_t *handle);

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
	      int data_flag);

/*****************************************************
 * PQ_print_buffer_info()
 *
 * Print the given buffer information to the indicated
 * stream in ASCII format.
 */

void PQ_print_buffer_info(pq_handle_t *handle,
			  FILE *stream,
			  pq_begin_buffer_info_t *buffer_info);

/*****************************************************
 * PQ_print_product()
 *
 * Print the given product information to the indicated stream
 * in ASCII format.
 */

void PQ_print_product(pq_handle_t *handle,
		      FILE *stream,
		      pq_prod_t *prod_info);

/*****************************************************
 * PQ_print_slot()
 *
 * Print the given slot information to the indicated stream in
 * ASCII format.  If the slot is currently active and the
 * data_flag is set, also print the product data for this slot
 * to the given stream.
 */

void PQ_print_slot(pq_handle_t *handle,
		   FILE *stream,
		   pq_slot_t *slot_info,
		   int data_flag);

/*****************************************************
 * PQ_print_status()
 *
 * Print the status information in shared memory to the indicated
 * stream in ASCII format.
 */

void PQ_print_status(pq_handle_t *handle,
		     FILE *stream);

/*****************************************************
 * PQ_set_instance_display()
 *
 * Set the display flag for the indicated instance of the product
 * of the given type to the given state.
 */

void PQ_set_instance_display(pq_handle_t *handle,
			     int prod_type,
			     int instance_key,
			     int state);

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
		      time_t data_time);

/*****************************************************
 * PQ_set_product_display()
 *
 * Set the display flags for all of the products of the given type
 * to the given state.
 */

void PQ_set_product_display(pq_handle_t *handle,
			    int prod_type,
			    int state);

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
				  int state);

/*****************************************************
 * DISPLAY-SIDE MEMBER FUNCTIONS
 *****************************************************/

/*****************************************************
 * PQ_check_server_update()
 *
 * Checks to see if the server has updated the product queue.
 *
 * Returns TRUE if the product queue has been updated, FALSE
 * otherwise.
 */

int PQ_check_server_update(pq_handle_t *handle);

/*****************************************************
 * PQ_data_current()
 *
 * Checks to see if the data requested by the display
 * has been received by the server.
 */

int PQ_data_current(pq_handle_t *handle);

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
			    time_t display_time);

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
				 time_t display_data_time);

/*****************************************************
 * MEMBER FUNCTIONS USED BY BOTH SIDES
 *****************************************************/

/*****************************************************
 * PQ_clear_semaphores()
 *
 * Clear the semaphores associated with the product queue shared
 * memory areas.  Only clear them if they were set by us.
 */

void PQ_clear_semaphores(pq_handle_t *handle);

/*****************************************************
 * PQ_check_map_flag()
 *
 * Returns the current value of the server map flag.
 */

int PQ_check_map_flag(pq_handle_t *handle);

/*****************************************************
 * PQ_get_display_time()
 *
 * Get the current display time from the product queue.
 */

time_t PQ_get_display_time(pq_handle_t *handle);

/*****************************************************
 * PQ_get_display_data_time()
 *
 * Get the current display_data_time from the product queue.
 */

time_t PQ_get_display_data_time(pq_handle_t *handle);

/*****************************************************
 * PQ_get_display_movie_time()
 *
 * Get the current display_time from the product queue.
 */

time_t PQ_get_display_movie_time(pq_handle_t *handle);

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
						 int *num_prod_ret);

/*****************************************************
 * PQ_set_display_update()
 *
 * Sets the display update flag to the given value.  Does not
 * set or clear the shared memory semaphores since this is an
 * autonomous action.  This is used with a value of TRUE by
 * the display to indicate to the server that a change has
 * been made and with a value of FALSE by the server to
 * indicate to itself that the change has been processed.
 */

void PQ_set_display_update(pq_handle_t *handle, int value);

/*****************************************************
 * PQ_set_map_flag()
 *
 * Set the value of the server map flag.  Also sets the
 * display update flag.  This will be used by the server
 * when it first comes up to store it's initial state,
 * but will just be used by the display after that.
 */

void PQ_set_map_flag(pq_handle_t *handle,
		     int flag_value);

/*****************************************************
 * PQ_set_semaphores()
 *
 * Set the semaphores associated with the product queue shared
 * memory areas.
 */

void PQ_set_semaphores(pq_handle_t *handle);

/*****************************************************
 * PQ_set_server_update()
 *
 * Sets the server update flag to the given value.  Does not
 * set or clear the shared memory semaphores since this is an
 * autonomous action.  This is used with a value of TRUE by
 * the server to indicate to the display that a change has
 * been made and with a value of FALSE by the display to
 * indicate to itself that the change has been processed.
 */

void PQ_set_server_update(pq_handle_t *handle, int value);


#endif

#ifdef __cplusplus
}
#endif
