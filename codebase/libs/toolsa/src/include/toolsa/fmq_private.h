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
/******************************************************************
 * fmq_private.h
 *
 * Private include for FMQ
 * Installed in the includes so that DsFmq may access the prototypes
 * This header should not be accessed by anyone other than fmq and DsFmq!
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * April 1997
 *
 */

#ifndef FMQ_PRIVATE_H
#define FMQ_PRIVATE_H

#ifdef __cplusplus
 extern "C" {
#endif


#include <dataport/bigend.h>
   /* #include <toolsa/umisc.h> */
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/fmq.h>

/*
 * private function prototypes
 */

extern void fmq_add_stat_checksum(fmq_stat_t *stat);
extern void fmq_add_slot_checksum(fmq_slot_t *slot);

extern void fmq_alloc_entry(FMQ_handle_t *handle,
			    int msg_len);

extern int fmq_alloc_slots(FMQ_handle_t *handle, int nslots);

extern void fmq_be_from_slot(fmq_slot_t *slot);

extern void fmq_be_from_stat(fmq_stat_t *stat);

extern void fmq_be_to_slot(fmq_slot_t *slot);

extern void fmq_be_to_stat(fmq_stat_t *stat);

extern int fmq_check (FMQ_handle_t *handle);

extern int fmq_check_and_clear (FMQ_handle_t *handle);

extern int fmq_check_and_recover (FMQ_handle_t *handle);

extern int fmq_check_file_sizes(FMQ_handle_t *handle);

extern int fmq_check_init(FMQ_handle_t *handle);

extern int fmq_check_init_no_error_message(FMQ_handle_t *handle);

extern int fmq_compute_stat_checksum(const fmq_stat_t *stat);
extern int fmq_compute_slot_checksum(const fmq_slot_t *slot);

extern int fmq_check_stat_checksum(const fmq_stat_t *stat);
extern int fmq_check_slot_checksum(const fmq_slot_t *slot);

extern int fmq_close(FMQ_handle_t *handle);

extern int fmq_clear(FMQ_handle_t *handle);

extern void fmq_debug_perror(FMQ_handle_t *handle,
			     char *label);
     
extern int fmq_exist (FMQ_handle_t *handle);

extern int fmq_fraction_used(FMQ_handle_t *handle,
			     double *slot_fraction_p,
			     double *buffer_fraction_p);

extern int fmq_find_slot_for_id(FMQ_handle_t *handle,
				int search_id, int *slot_p);
     
extern void fmq_free_entry(FMQ_handle_t *handle);

extern void fmq_free_handle(FMQ_handle_t *handle);

extern void fmq_free_slots(FMQ_handle_t *handle);

extern void fmq_init_buf(FMQ_handle_t *handle);

extern int fmq_init_handle(FMQ_handle_t *handle,
			   const char *fmq_path,
			   int debug,
			   const char *prog_name);

extern int fmq_lock_rdwr(FMQ_handle_t *handle);
     
extern int fmq_lock_rdonly(FMQ_handle_t *handle);
     
extern int fmq_next_id(int id);

extern int fmq_next_slot(FMQ_handle_t *handle, int slot_num);

extern int fmq_open_blocking(FMQ_handle_t *handle, int msecs_sleep);

extern int fmq_open_blocking_rdwr(FMQ_handle_t *handle, int msecs_sleep);

extern int fmq_open_create(FMQ_handle_t *handle,
			   int nslots, int buf_size);

extern int fmq_open(FMQ_handle_t *handle, char *mode);

extern int fmq_open_rdonly(FMQ_handle_t *handle);

extern int fmq_open_rdwr(FMQ_handle_t *handle,
			 int nslots, int buf_size);

extern int fmq_open_rdwr_nocreate(FMQ_handle_t *handle);

extern int fmq_prev_id(int id);

extern int fmq_prev_slot(FMQ_handle_t *handle, int slot_num);

extern int fmq_print_debug(char *fmq_path,
			   char *prog_name,
			   FILE *out);
     
extern void fmq_print_slots(FMQ_handle_t *handle, FILE *out);
extern void fmq_print_active_slots(FMQ_handle_t *handle, FILE *out);

extern void fmq_print_slot(int slot_num, fmq_slot_t *slot, FILE *out);

extern void fmq_pretty_print_slot(int slot_num, fmq_slot_t *slot, FILE *out);

extern void fmq_print_debug_error(FMQ_handle_t *handle,
				  char *routine,
				  char *format,
				  ...);

extern void fmq_print_error(FMQ_handle_t *handle,
			    char *routine,
			    char *format,
			    ...);

extern void fmq_print_debug_msg(FMQ_handle_t *handle,
				char *routine,
				char *format,
				...);
     
extern int fmq_print_slot_read(FMQ_handle_t *handle, FILE *out);

extern int fmq_print_slot_written(FMQ_handle_t *handle, FILE *out);

extern int fmq_print_stat(FMQ_handle_t *handle, FILE *out);

extern int fmq_read(FMQ_handle_t *handle, int *msg_read, int type);

extern int fmq_read_blocking(FMQ_handle_t *handle,
			     int msecs_sleep, int type);

extern int fmq_read_msg_for_slot(FMQ_handle_t *handle, int slot_num);

extern int fmq_read_slot(FMQ_handle_t *handle, int slot_num);

extern int fmq_read_slots(FMQ_handle_t *handle);

extern int fmq_read_stat(FMQ_handle_t *handle);

extern int fmq_load_read_msg (FMQ_handle_t *handle,
			      int msg_type,
			      int msg_subtype,
			      int msg_id,
			      time_t msg_time,
			      void *msg,
			      int stored_len,
			      int compressed,
			      int uncompressed_len);
     
extern int fmq_read_with_retry(int fd, const void *mess, size_t len);

extern int fmq_seek_end(FMQ_handle_t *handle);

extern int fmq_seek_last(FMQ_handle_t *handle);

extern int fmq_seek_start(FMQ_handle_t *handle);

extern int fmq_seek_back(FMQ_handle_t *handle);

extern int fmq_seek_to_id (FMQ_handle_t *handle, int id);

extern void fmq_set_compress(FMQ_handle_t *handle);

extern void fmq_set_compression_method(FMQ_handle_t *handle,
				       ta_compression_method_t method);

extern void fmq_set_server(FMQ_handle_t *handle);

extern void fmq_set_blocking_write(FMQ_handle_t *handle);

extern void fmq_set_heartbeat(FMQ_handle_t *handle,
			      TA_heartbeat_t heartbeat_func);

extern int fmq_slot_in_active_region(FMQ_handle_t *handle,
				     int slot_num);

extern int fmq_unlock(FMQ_handle_t *handle);

extern int fmq_write(FMQ_handle_t *handle, void *msg, int msg_len,
		     int msg_type, int msg_subtype);

extern int fmq_write_precompressed(FMQ_handle_t *handle,
				   void *msg, int msg_len,
				   int msg_type, int msg_subtype,
				   int uncompressed_len);

extern int fmq_write_stat(FMQ_handle_t *handle);

extern int fmq_write_slot(FMQ_handle_t *handle, int slot_num);

extern int fmq_write_with_retry(int fd, const void *mess, size_t len);

#ifdef __cplusplus
}
#endif

#endif
