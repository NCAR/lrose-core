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

/*****************************************************
 * token_list.c
 *
 * Token list manipulation routines for tdrp.
 *
 * The token list is a list of tdrpToken_t * pointers.
 *
 */

#include <tdrp/tdrp.h>
#include <assert.h>
#include <string.h>

/************************
 * tdrpTokenListInit()
 *
 * Initialize the token list.
 */

void tdrpTokenListInit(token_handle_t *handle)

{
  handle->list_buf = tdrpBufCreate();
}

/*************************
 * tdrpTokenListReset()
 *
 * Reset the token list.
 */

void tdrpTokenListReset(token_handle_t *handle)

{
  int nlist = tdrpNTokenList(handle);
  int i;
  tdrpToken_t *tok = tdrpTokenListPtr(handle);
  for (i = 0; i < nlist; i++, tok++) {
    tdrpFree(tok->tok);
  }
  tdrpBufReset(handle->list_buf);
}

/************************
 *  tdrpTokenListAdd()
 *
 *  Adds entry to token list.
 *
 */

void tdrpTokenListAdd(token_handle_t *handle, const tdrpToken_t *token)

{

  tdrpToken_t tmp_tok = *token;
  tmp_tok.tok = (char*) tdrpMalloc(strlen(token->tok) + 1);
  strcpy(tmp_tok.tok, token->tok);
  tdrpBufAdd(handle->list_buf, &tmp_tok, sizeof(tdrpToken_t));

}

/*********************
 * tdrpNTokenList()
 *
 * return number of tokens in list
 */

int tdrpNTokenList(token_handle_t *handle)

{
  int nlist;
  assert(handle->list_buf != NULL);
  nlist = tdrpBufLen(handle->list_buf) / sizeof(tdrpToken_t);
  return (nlist);
}

/************************
 *  tdrpTokenListPtr()
 *
 *  Returns pointer to list.
 *
 */

tdrpToken_t *tdrpTokenListPtr(token_handle_t *handle)

{
  tdrpToken_t *tokens;
  assert(handle->list_buf != NULL);
  tokens = (tdrpToken_t *) tdrpBufPtr(handle->list_buf);
  return (tokens);
}

/************************
 * tdrpTokenListFree()
 *
 * Free the token list.
 */

void tdrpTokenListFree(token_handle_t *handle)

{
  assert(handle->list_buf != NULL);
  tdrpTokenListReset(handle);
  tdrpBufDelete(handle->list_buf);
}

