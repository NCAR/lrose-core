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

/*******************************************************************
 * didss/ds_message.h
 *
 * Message definition for DIDSS.
 ******************************************************************/

#ifndef DsMessage_h
#define DsMessage_h

#ifdef __cplusplus
extern "C" {
#endif

#include <dataport/port_types.h>

/*
 * DsMessage
 *
 * A DsMessage is a simple, self-describing message format for
 * the DIDSS subsystem. It may be applied to any message, whatever
 * the actual transport mechanism.
 *
 * A DsMessage has 3 sections, a header struct which contains the
 * number of parts, an array of part structs which indicate the
 * data types, lengths and offsets, and the data parts themselves.
 *
 * DsMessage format:
 *
 *   DsMsgHdr_t
 *   nParts * DsMsgPart_t
 *   nParts * data
 */

/*
 * missing data values
 */

#define DS_MISSING_UCHAR 0
#define DS_MISSING_INT -999
#define DS_MISSING_FLOAT -999.0

/*
 * header struct
 *
 * type, subType, mode, flags and error are defined and used for the
 * specific message type. These values may be used in whatever manner
 * the author chooses.
 *
 * The version numbers and serial number may be optionally defined and used
 * by a message type. Their use is defined by the author of the type.
 *
 * category is defined and used by certain classes of message.
 * The intention is that the category definition will apply uniformly
 * to all subclasses.
 */

typedef struct {
  
  si32 type;
  si32 subType;
  si32 mode;
  si32 flags;
  si32 majorVersion;
  si32 minorVersion;
  si32 serialNum;
  si32 category;
  si32 error;
  si32 nParts;
  si32 spare[6];
  
} DsMsgHdr_t;

/*
 * part struct
 *
 * dataType is defined by the author of the message type.
 * length refers to the length of the part data in bytes.
 * offset refers to the offset of the part data from the start
 * of the message.
 */

typedef struct {

  si32 dataType;
  si32 offset;
  si32 len;
  si32 spare[3];

} DsMsgPart_t;

/*
 * prototypes
 */

/*******************
 * BE_to_DsMsgHdr()
 *
 * Convert BE to DsMsgHdr_t
 */

extern void BE_to_DsMsgHdr(DsMsgHdr_t *hdr);

/*******************
 * BE_from_DsMsgHdr()
 *
 * Convert DsMsgHdr_t to BE
 */

extern void BE_from_DsMsgHdr(DsMsgHdr_t *hdr);

/*******************
 * BE_to_DsMsgPart()
 *
 * Convert BE to DsMsgPart_t
 */

extern void BE_to_DsMsgPart(DsMsgPart_t *part);

/***************************
 * BE_from_DsMsgPart()
 *
 * Convert DsMsgPart_t to BE
 */

extern void BE_from_DsMsgPart(DsMsgPart_t *part);

#ifdef __cplusplus
}
#endif

#endif

