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
/******************************************************************/
/**
 *
 * /file <iwrf_user_infterface.h>
 *
 * Defines for protocol between GUI and syscon (system controller task).
 *
 * CSU-CHILL/NCAR
 * IWRF - INTEGRATED WEATHER RADAR FACILILTY
 *
 * Dave Brunkow  May 2010
 *
 *********************************************************************/

#ifndef _IWRF_USER_INTERFACE_H_
#define _IWRF_USER_INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************/
#include <radar/iwrf_data.h>
#include <radar/iwrf_rsm.h>

/************************************************************************/
/*  User Interface items */
/************************************************************************/
/************************************************************************/

/****************** iwrf_ui specific opcode payloads *********************/

	/* task names structure */
typedef struct iwrf_ui_name_element 
{
	char name[IWRF_MAX_SEGMENT_NAME];
} iwrf_ui_name_element_t;


	/* this struct provides scheduling items for either a Tasklist or an individual scan. */
typedef struct iwrf_ui_schedule_info
{	si64	begin_time_utc;		/**< 0 to deactivate timer or valid begin time to set timer */
	si32	repeat_cycle_secs;	/**< 0 or repeat cycle */
	si32	priority;			/**< 0 or priority setting */
	si64	last_run_time_utc;	/**< last time run (on get schedule operation) */
} iwrf_ui_schedule_info_t;


typedef struct iwrf_ui_tasklist_full
{	si32 num_list_items;
	si32 unused_padding;  /* required so structure is the same size on 64 bit architectures */
	iwrf_ui_name_element_t tasks[IWRF_UI_MAX_TASKS_PER_TASKLIST];
	iwrf_ui_schedule_info_t tasklist_schedule;
	si32 unused[32];
} iwrf_ui_tasklist_full_t;


	/* this struct is used to modify a tasklist in place */
typedef struct iwrf_ui_tasklist_mod
{	char name[IWRF_MAX_SEGMENT_NAME]; /**< used by APPEND \*/
	si32 index1;	/**< used by REMOVE_ONE, EXCHANGE, SET*INDEX op's \*/
	si32 index2;	/**< used by EXCHANGE, GET_CURRENT_INDEX \*/
} iwrf_ui_tasklist_mod_t;

/* The IWRF_UI_TASKLIST_GET_CURRENT_INDEX command returns the tasklist index to the currently
    running scan in index1, and the index of the next scan in index2.  */

typedef enum iwrf_ui_error
{	IWRF_UI_ERROR_DELETE_FAILED = 1,	/**< attempt to delete running scan task */
	IWRF_UI_ERROR_TASKLIST_SIZE,			/**< illegal number of tasks in tasklist */
	IWRF_UI_ERROR_MISSING_TASK_NAME,  		/**< task name required, but missing */
	IWRF_UI_ERROR_TASKLIST_INDEX_RANGE,		/**< task list index <0 or > max range */
	IWRF_UI_ERROR_TASKLIST_FULL,			/**< can not append to tasklist */
	IWRF_UI_ERROR_APPEND_TASK_UNDEFINED,		/**< attempt to append undefined task to tasklist */
	IWRF_UI_ERROR_APPEND_TASK_NONAME,		/**< no task name in APPEND request */
	IWRF_UI_ERROR_CANT_SCHEDULE,			/**< begin time was not 15 seconds into the future */
	IWRF_UI_ERROR_REPEAT_CYCLE_RANGE,		/**< schedule repeat cycle <0 or > 3600 secs  */
	IWRF_UI_ERROR_TASKLIST_ENTRY_UNKNOWN,		/**< tasklist contains unknown task name */
	IWRF_UI_ERROR_TASKLIST_ENTRY_UNDEFINED,		/**< tasklist entry is not fully defined */
	IWRF_UI_ERROR_CANT_SCHEDULE_TASK_NOT_IN_TASKLIST,  /**< attempt to sched a task not in tasklist */
	IWRF_ANTCON_NOT_CONNECTED,			/**< antenna controller not connected */
	IWRF_ANTENA_FAULTED,				/**< antenna controller reports fault */
	IWRF_TXCTRL_NOT_CONNECTED,			/**< txmit controller not connected */
	IWRF_SCAN_STATE_TIMEOUT, 			/**< scan state machine time in state limit reached */
	IWRF_SCAN_SEGMENT_LIST_ERROR,			/**< scan segment fixed angle list is bad */
	IWRF_UI_ERROR_BAD_PACKET_ID,		/**< expected ui_task_operations packet ID not found */
	IWRF_UI_WARN_START_TIME_WARN,		/**< start time is in the past */
	IWRF_UI_DATA_SYSTEM_NOT_CONNECTED,	/**< syscon has no connection to control data system */
} iwrf_ui_error_t;

#define IWRF_UI_ENABLES_MASTER_RECORD_MASK 1
#define IWRF_UI_ENABLES_IMMEDIATE_SCAN_UPDATE_MASK 2
#define IWRF_UI_ENABLES_DONT_SEND_RSM_TO_UI 4


/* The iwrf_ui_global structure is used to pass info relating to all tasks to system controller.
   It is also used by the system controller to return status info and error messages to UI.
*/

typedef struct iwrf_ui_global
{
	char operator_name[IWRF_UI_MAX_OPERATOR_NAME];  /**< set by UI */
	si32 command;	/**< global command, set by UI */
	si32 enables;	/**< bit-mapped enables, set by UI */
} iwrf_ui_global_t;

typedef struct iwrf_ui_rcs_status 
{
	char message[IWRF_UI_MAX_ERROR_MSG]; /**< ascii error message, set by system controller */
	si32 error;			/**< error number, set by system controller */
	si32 misc_status;		/**< status bits, set by system controller */
	ui32 az_status;		/**< az motor status bits, see MS_... in deltatau.h */
	ui32 el_status;		/**< el motor status bits, see MS_... in deltatau.h */
} iwrf_ui_rcs_status_t;


typedef enum iwrf_ui_opcodes
{		/* Task Definition */
	     /*        Opcode           Payload Flow Dir.      Payload  */
	IWRF_UI_UPD_SCAN_SEGMENT = 1,	/**<  UI<>RCS  iwrf_scan_segment_t, */
	IWRF_UI_UPD_TS_PROCESSING,	/**<  UI<>RCS  iwrf_ts_procession_t */
	IWRF_UI_UPD_DELETE_TASK,	/**<  UI<>RCS  iwrf_ui_name_element_t */
	IWRF_UI_UPD_SEGMENT_SCHEDULE,  /**<  UI<>RCS iwrf_ui_schedule_info_t */
	IWRF_UI_GO_IDLE,	/**< UI<>RCS   stop scanning (begin SYS_IDLE segment) */
			/* last opcode allows for alternative scheduling methods */
		/* Tasklist operations */
	IWRF_UI_TASKLIST_UPD_LIST,	    /**<  UI<>RCS    iwrf_ui_tasklist_full */
	IWRF_UI_TASKLIST_GET_UNUSED_LIST,   /**<  UI<-RCS user query, in response, 
				RCS sends iwrf_ui_tasklist_full containing unused task names */
	IWRF_UI_TASKLIST_GET_CURRENT_INDEX, /**< UI<-RCS user query,  in response,
						    RCS sends current and next task indexes in
						    iwrf_ui_tasklist_mod->index1&2 */
	IWRF_UI_TASKLIST_UPD_NEXT_INDEX,    /**<  UI<>RCS  iwrf_ui_tasklist_mod->index1 
					indexed task starts when current task ends */
	IWRF_UI_TASKLIST_SET_INDEX_IMMEDIATE, /**< UI->RCS iwrf_ui_tasklist_mod->index1 
						indexed	task starts immediately */
	IWRF_UI_TASKLIST_UPD_LIST_SCHEDULE, /**<  UI<>RCS  iwrf_schedule_info_t */
	IWRF_UI_TASKLIST_GET_LIST_SCHEDULE, /**<  UI->RCS user query, in response, 
						RCS sends current iwrf_ui_tasklist_full + sched. */
	IWRF_UI_TASKLIST_REMOVE_ALL,	    /**< UI<>RCS   none */
	IWRF_UI_TASKLIST_REMOVE_ONE,	    /**< UI<>RCS   ...tasklist_mod->index1 */
	IWRF_UI_TASKLIST_APPEND,	    /**< UI<>RCS   ...tasklist_mod->name */
	IWRF_UI_TASKLIST_EXCHANGE,	    /**< UI<>RCS   ...tasklist_mod->index1&2 */
			/* last three are to facilitate shared tasklist management */
		/* Misc Global items */
	IWRF_UI_UPD_GLOBAL,			/**< UI->RCS  iwrf_ui_global_t */
	IWRF_UI_UPD_RSM_PACKET,			/**< UI<-RCS  rsm_pkt_t  */
	IWRF_UI_RCS_STATUS,			/**< UI<-RCS  iwrf_ui_rcs_status_t */
	IWRF_UI_SHUTDOWN_CONNECTION,  /** disconnect from server */
        /* RCS requests UI to set Polarization mode */
        IWRF_UI_SET_POL_MODE,                   /**< UI<-RCS  iwrf_scan_segment_t */
        /* UI confirms Polarization mode */
        IWRF_UI_CONFIRM_POL_MODE,               /**< UI->RCS  iwrf_scan_segment_t */
	IWRF_UI_LAST
} iwrf_ui_opcodes_t;


typedef union iwrf_ui_rcs_payloads
{
	iwrf_scan_segment_t scan_segment;
	iwrf_ts_processing_t ts_processing;
	iwrf_ui_schedule_info_t schedule;
	iwrf_ui_name_element_t delete_task;
	iwrf_ui_tasklist_mod_t tasklist_modify;
	iwrf_ui_tasklist_full_t tasklist_full;
	iwrf_ui_global_t global_items;
	iwrf_ui_rcs_status_t rcs_status;
	rsm_pkt_t rsm_pkt;
} iwrf_ui_rcs_payloads_t;


typedef struct iwrf_ui_task_operations
{
	iwrf_packet_info_t packet;		/**< packet id = IWRF_UI_OPERATIONS \*/
	char task_name[IWRF_MAX_SEGMENT_NAME];  /**< req'd for Task Def operations \*/
	char owner_name[IWRF_UI_MAX_OPERATOR_NAME]; /**< originating operator name \*/
	si32 op_code;			/**< see iwrf_ui_opcodes_t */
	ui32 pad1;			/* structure  alignment - */
	iwrf_ui_rcs_payloads_t un;
} iwrf_ui_task_operations_t;

#ifdef __cplusplus
}
#endif

#endif
