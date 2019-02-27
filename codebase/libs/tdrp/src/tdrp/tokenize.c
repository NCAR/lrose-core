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

/***************************************************************
 * tokenize.c
 *
 * Module for generating token list. Non-reentrant.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * Nov 1997
 *
 ****************************************************************/

#include <tdrp/tdrp.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>

#define NALLOC_INCR 100
#define N_PAD 25

/*
 * file scope prototypes
 */

static void alloc_toks(token_handle_t *handle, int n_needed);
static void free_toks(token_handle_t *handle);
static void store_current_tok(token_handle_t *handle);
static void store_tok(token_handle_t *handle);

static void alloc_buf(token_handle_t *handle, int n_needed);
static void add_to_buf(token_handle_t *handle, int c);
static void reset_buf(token_handle_t *handle);
static void free_buf(token_handle_t *handle);

static void get_next_line(token_handle_t *handle);
static int get_next_char(token_handle_t *handle);
static void push_char(token_handle_t *handle);

/*******************************
 * tdrpInitTokenize()
 *
 * Initialize the tokenize module.
 */

void tdrpInitTokenize(token_handle_t *handle)

{

  handle->string_in_progress = FALSE;
  handle->reading_from_buf = FALSE;
  handle->override_list = NULL;
  handle->n_override = 0;
  handle->tokens = NULL;
  handle->ntok = 0;
  handle->ntok_alloc = 0;
  handle->buffer = NULL;
  handle->nbuf = 0;
  handle->nbuf_alloc = 0;
  handle->read_buf = NULL;
  handle->read_pos = NULL;
  tdrpTokenListInit(handle);

}

/****************************************
 * tdrpInitFileForTokens()
 *
 * Opens the file for tokenizing.
 *
 * Initializes.
 *
 * Sets the override list.
 * The override list may be NULL. When non-NULL, it is a
 * NULL-terminated list of strings used to override the
 * values in the file.
 *
 * returns 0 on success, -1 on failure
 */

int tdrpInitFileForTokens(token_handle_t *handle,
			  const char *file_path, char **override_list)

{
  
  FILE *parse_file;
  struct stat filestat;
  char *filebuf;

  if (file_path != NULL) {

    /*
     * get file size
     */

    if (stat(file_path, &filestat)) {
      fprintf(stderr, "Cannot stat file for parsing\n");
      perror(file_path);
      return (-1);
    }
    
    /*
     * Open the file for tokenizing
     */
    
    if ((parse_file = fopen(file_path, "r")) == NULL) {
      fprintf(stderr, "Cannot open file for parsing\n");
      perror(file_path);
      return (-1);
    }

    /*
     * allocate buffer, read in file
     */

    filebuf = (char *) tdrpMalloc (filestat.st_size);
    if (fread(filebuf, 1, filestat.st_size, parse_file)
	!= (size_t) filestat.st_size) {
      fprintf(stderr, "Cannot read file for parsing\n");
      perror(file_path);
      fclose(parse_file);
      tdrpFree(filebuf);
      return (-1);
    }
    fclose(parse_file);
    /*
     * pass the buffer to InitBuf()
     */

    tdrpInitBufForTokens(handle, filebuf, filestat.st_size, 1, override_list);

    tdrpFree(filebuf);
    
  } else {
    
    /*
     * initialize for parsing
     */
    
    handle->reading_from_buf = FALSE;
    handle->comment_in_progress = FALSE;
    handle->line_num = 0;
    handle->line_ptr = NULL;
    handle->override_list = override_list;
    
  }
    
  return (0);

}

/****************************************
 * tdrpInitBufForTokens()
 *
 * Sets up the read buffer for tokenizing.
 *
 * Initializes.
 *
 * Sets the override list.
 * The override list may be NULL. When non-NULL, it is a
 * NULL-terminated list of strings used to override the
 * values in the file.
 *
 * returns 0 on success, -1 on failure
 */

void tdrpInitBufForTokens(token_handle_t *handle,
			  const char *inbuf, int buflen,
			  int start_line_num,
			  char **override_list)
     
{

  int i;
  char *ch;

  /*
   * allocate the read buffer, copy in the inbuf, null-terminate
   */

  handle->read_buf = (char *) tdrpRealloc(handle->read_buf, buflen + 2);
  memcpy(handle->read_buf, inbuf, buflen);
  handle->read_buf[buflen] = '\0';
  handle->read_buf[buflen+1] = '\0';

  /*
   * search through the read buffer, setting any nulls to spaces
   * so the parsing will work correctly
   */

  ch = handle->read_buf;
  for (i = 0; i < buflen; i++, ch++) {
    if (*ch == '\0') {
      *ch = ' ';
    }
  }

  /*
   * initialize for parsing
   */

  handle->read_pos = handle->read_buf;
  handle->reading_from_buf = TRUE;
  handle->comment_in_progress = FALSE;
  handle->line_num = start_line_num - 1;
  handle->line_ptr = NULL;
  handle->override_list = override_list;

}

/********************************
 * tdrpTokenize()
 *
 * Add tokens in file to token
 * array.
 */

int tdrpTokenize(token_handle_t *handle)

{

  int string_divide_found = FALSE;
  int c;
  
  /*
   * loop getting characters for parsing
   */

  while ((c = get_next_char(handle)) != -1) {
    /*
     * for character strings ...
     */
    
    if (handle->string_in_progress != FALSE) {

      /*
       * we are in a string
       */
      
      if (!string_divide_found) {

	/*
	 * check for escaped quotes
	 */

	if (c == '\\') {

	  if ((c = get_next_char(handle)) == '"') {
	    add_to_buf(handle, '"');
	  } else {
	    add_to_buf(handle, '\\');
	    add_to_buf(handle, c);
	  }

	} else if (c == '"') {

	  /* check for end of string or divider */
	  
	  string_divide_found = TRUE;

	} else {

	  add_to_buf(handle, c);

	}
	
      } else { /* if (!string_divide_found) */

	/*
	 * we are in a string, after a quote which indicates a
	 * string divider
	 */

	if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {

	  /* do nothing - between parts of string */

	} else if (c == '"') {

	  /* start of next part of string - cancel divider */

	  string_divide_found = FALSE;
	  /* add_to_buf(' '); */

	} else {

	  /*
	   * some other character - store string token and
	   * push the latest char back onto the stack for
	   * later processing
	   */

	  store_current_tok(handle);
	  push_char(handle);
	  handle->string_in_progress = FALSE;

	}

      } /* if (!string_divide_found) */

    } else { /* if (handle->string_in_progress != FALSE) */

      /*
       * no string in progress
       */

      if (c == '"') {

	store_current_tok(handle);
	handle->string_in_progress = TRUE;
	string_divide_found = FALSE;

      } else if (tdrpReservedChar(c)) {

	/* reserved character */
	store_current_tok(handle);
	add_to_buf(handle, c);
	store_current_tok(handle);

      } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {

	/* white space */
	store_current_tok(handle);

      } else {

	add_to_buf(handle, c);

      }
      
    } /* if (handle->string_in_progress != FALSE) */

  } /* while */

  /*
   * pad out the token list with semi-colons so that we do not need
   * to check for end-of-list conditions while parsing
   */

  {
    int i;
    for (i = 0; i < N_PAD; i++) {
      reset_buf(handle);
      add_to_buf(handle, ';');
      store_current_tok(handle);
    }
  }

  if (handle->string_in_progress != FALSE) {
    fprintf(stderr, "ERROR - string in progress at end of file\n");
    fprintf(stderr, "  Check for missing quote\n");
    return -1;
  }

  return 0;

}

/*******************************
 * tdrpNtok()
 *
 * Returns the number of tokens
 */

int tdrpNtok(token_handle_t *handle)

{
  return (handle->ntok);
}

/*******************************
 * tdrpTokens()
 *
 * Returns pointer to the token array
 */

tdrpToken_t *tdrpTokens(token_handle_t *handle)

{
  return (handle->tokens);
}

/*******************************
 * tdrpFreeTokenize()
 *
 * Free up the tokenize module
 */

void tdrpFreeTokenize(token_handle_t *handle)

{
  free_toks(handle);
  free_buf(handle);
  tdrpTokenListFree(handle);
}

/*******************************
 * tdrpReservedChar()
 *
 * Returns TRUE if char is reserved, FALSE otherwise
 */

int tdrpReservedChar(int c) {

  if (c == '{' || c == '}' || c == '[' ||
      c == ']' || c == '=' || c == ',' ||
      c == ';') {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

/*******************************
 * tdrpReservedStr()
 *
 * Returns TRUE if string is reserved, FALSE otherwise
 */

int tdrpReservedStr(char *str) {

  if (!strcmp(str, "{") ||
      !strcmp(str, "}") ||
      !strcmp(str, "[") ||
      !strcmp(str, "]") ||
      !strcmp(str, "=") ||
      !strcmp(str, ",") ||
      !strcmp(str, ";")) {
    return (TRUE);
  } else {
    return (FALSE);
  }
}

/********************************
 * alloc_toks()
 *
 * Allocate the memory for tokens
 */

static void alloc_toks(token_handle_t *handle, int n_needed)

{

  if (n_needed > handle->ntok_alloc) {
    
    if (handle->tokens == NULL) {
      handle->tokens = (tdrpToken_t *)
	tdrpMalloc((handle->ntok_alloc + NALLOC_INCR) * sizeof(tdrpToken_t));
    } else {
      handle->tokens = (tdrpToken_t *)
	tdrpRealloc(handle->tokens,
		    (handle->ntok_alloc + NALLOC_INCR) * sizeof(tdrpToken_t));
    }

    handle->ntok_alloc += NALLOC_INCR;

  }
					 
}

/*********************************************************
 * store_current_tok
 *
 * Checks whether to store the token.
 * For normal tokens, must be non-zero in length to store.
 * For strings, 0 length is OK.
 */

static void store_current_tok(token_handle_t *handle)

{
  if (handle->string_in_progress != FALSE || 
      handle->nbuf > 0) {
    store_tok(handle);
  } 
}

/*********************************************************
 * store_tok
 *
 * Takes the string from the buffer and stores it in a new
 * token. Clears out the buffer.
 */

static void store_tok(token_handle_t *handle)

{

  /*
   * check allocation
   */
  
  alloc_toks(handle, handle->ntok + 1);
  
  /*
   * store
   */
  
  handle->tokens[handle->ntok].used = FALSE;
  handle->tokens[handle->ntok].line_num = handle->line_num;
  if (handle->string_in_progress != FALSE) {
    handle->tokens[handle->ntok].is_string = TRUE;
  } else {
    handle->tokens[handle->ntok].is_string = FALSE;
  }
  handle->tokens[handle->ntok].tok = (char *) tdrpMalloc(handle->nbuf + 1);
  memcpy(handle->tokens[handle->ntok].tok, handle->buffer, handle->nbuf);
  handle->tokens[handle->ntok].tok[handle->nbuf] = '\0';
  handle->ntok++;
  
  /*
   * clear buffer
   */

  reset_buf(handle);

}

/********************************
 * free_toks()
 *
 * Free the memory for tokens
 */

static void free_toks(token_handle_t *handle)

{

  int i;

  for (i = 0; i < handle->ntok; i++) {
    tdrpFree(handle->tokens[i].tok);
  }
  tdrpFree(handle->tokens);
  handle->tokens = NULL;
  handle->ntok_alloc = 0;
  handle->ntok = 0;
  
}

/*************************************
 * alloc_buf()
 *
 * Allocate the memory for char buffer
 */

static void alloc_buf(token_handle_t *handle, int n_needed)

{

  if (n_needed > handle->nbuf_alloc) {
    
    if (handle->buffer == NULL) {
      handle->buffer = (char *)
	tdrpMalloc((handle->nbuf_alloc + NALLOC_INCR) * sizeof(char));
    } else {
      handle->buffer = (char *)
	tdrpRealloc(handle->buffer,
		    (handle->nbuf_alloc + NALLOC_INCR) * sizeof(char));
    }

    handle->nbuf_alloc += NALLOC_INCR;

  }
					 
}

/*************************************
 * add_to_buf()
 *
 * Add a character to the token buffer
 */

static void add_to_buf(token_handle_t *handle, int c)

{

  alloc_buf(handle, handle->nbuf + 1);
  handle->buffer[handle->nbuf] = c;
  handle->nbuf++;

}

/*************************************
 * reset_buf()
 *
 * Reset the buffer to start.
 */

static void reset_buf(token_handle_t *handle)

{

  handle->nbuf = 0;

}

/*************************************
 * free_buf()
 *
 * Free the memory for char buffer
 */

static void free_buf(token_handle_t *handle)

{

  tdrpFree(handle->buffer);
  handle->buffer = NULL;
  handle->nbuf_alloc = 0;
  handle->nbuf = 0;
  tdrpFree(handle->read_buf);
  handle->read_buf = NULL;

}

/****************************************
 * get_next_char()
 *
 * Gets the next char.
 *
 * returns char on success, -1 on failure
 */

static int get_next_char(token_handle_t *handle)
     
{

  while (handle->line_ptr == NULL ||
	 handle->char_pos >= (int) strlen(handle->line_ptr)) {

    /*
     * get new line
     */

    get_next_line(handle);
    if (handle->line_ptr == NULL) {
      return (-1);
    }

    handle->char_pos = 0;
    
  } /* while */

  return (handle->line_ptr[handle->char_pos++]);

}

/****************************************
 * push_prev_char()
 *
 * Pushes char back onto line.
 *
 */

static void push_char(token_handle_t *handle)
     
{
  handle->char_pos--;
}

/**********************************************************
 * get_next_line()
 *
 * Gets the next line, stripping comments and substituting
 * environment variables.
 *
 * returns line on success, NULL on failure.
 */

static void get_next_line(token_handle_t *handle)
     
{
  
  char *token;
  char *endl;
  char *start_comment = NULL, *end_comment = NULL;
  int len;
  int done;
  int n_comment;
  int override_len;

  if (handle->reading_from_buf) {
    
    /*
     * read next line from file
     */

    endl = strchr(handle->read_pos, '\n');

    /*
     * check for non-empty last line
     */
    
    if ((endl == NULL) && (strlen(handle->read_pos) > 0)) {
      endl = handle->read_pos + strlen(handle->read_pos);
    }

    if (endl == NULL) {

      /* done */
      handle->reading_from_buf = FALSE;
      /* add a ';' token to be safe */
      strcpy(handle->line, ";\n");
      handle->line_num++;

    } else {
	
      /* line read */
      len = endl - handle->read_pos;
      if (len > (TDRP_LINE_MAX - 1)) {
	handle->line_ptr = NULL;
	fprintf(stderr, "Line length is %d exceeded limit %d\n", len, TDRP_LINE_MAX);
	return;
      }
      tdrpStrNcopy(handle->line, handle->read_pos, len + 1);
      handle->line_num++;
      handle->read_pos += (len + 1);

    }
    
  } else {
    
    /*
     * read next override line
     */
    
    if (handle->override_list == NULL ||
	handle->override_list[handle->n_override] == NULL) {
      handle->line_ptr = NULL;
      return;
    }
    handle->n_override++;
    strncpy(handle->line, handle->override_list[handle->n_override-1],
	    TDRP_LINE_MAX - 2);
    handle->line[TDRP_LINE_MAX-3]='\0';
    override_len = strlen(handle->line);
    handle->line[override_len] = '\n';
    handle->line[override_len+1] = '\0';
    handle->line_num = -(handle->n_override);
    
  } /* if (handle->reading_from_buf) */

  /*
   * strip new-line
   */

  len = strlen(handle->line);
  if (len > 0) {
    if (handle->line[len - 1] == '\n') {
      handle->line[len - 1] = '\0';
    }
  }

  /*
   * blank out lines starting with "#"
   */
  
  if (handle->line[0] == '#') {
    handle->line[0] = '\0';
  }
  
  /*
   * clear beyond "//"
   */
  
  if ((token = strstr(handle->line, "//")) != NULL) {
    char *qq = handle->line;
    int nquotes = 0;
    /* make sure we are not in a string */
    while (qq < token) {
      if (*qq == '"') {
	nquotes++;
      }
      qq++;
    }
    if ((nquotes % 2) == 0) {
      /* even number of quotes */
      *token = '\0';
    }
  }
  
  /*
   * now get rid of C-style comments
   */
  
  done = FALSE;
  if (handle->comment_in_progress) {
    start_comment = handle->line;
  }
  
  while (!done) {
    
    if (!handle->comment_in_progress) {
      
      /*
       * no handle->comment_in_progress
       */
      
      start_comment = strstr(handle->line, "/*");
      
      if (start_comment == NULL) {
	done = TRUE;
	continue;
      } else {
	handle->comment_in_progress = TRUE;
      }
      
    } else {
      
      /*
       * comment_in_progress
       */
      
      end_comment = strstr(handle->line, "*/");
      
      if (end_comment == NULL) {
	done = TRUE;
	continue;
      } else {
	n_comment = (end_comment - start_comment + 2);
	memset((void *) start_comment, ' ', n_comment);
	handle->comment_in_progress = FALSE;
      }
      
    }
    
  } /* while */
  
  /*
   * if comment in progress, blank out from start of comment
   * to end of line
   */
  
  if (handle->comment_in_progress) {
    *start_comment = '\0';
  }

  handle->line_ptr = handle->line;
  return;

}

/****************
 * tdrpLineInfo()
 * 
 * Assembles parse line information for error reporting.
 *
 * If the line number is >= 0, it loads a string with the
 * label and line number.
 *
 * If the line number < 0, it assumes that the token
 * came from command line overrides, and assembles that info.
 *
 * Returns a string composed from the line number info.
 *
 */

char *tdrpLineInfo(token_handle_t *handle, const tdrpToken_t *token)
     
{
  
  if (token->line_num >= 0) {
    sprintf(handle->line_num_info, "Line num %d", token->line_num);
  } else {
    sprintf(handle->line_num_info,
	    "ERROR in command line override #%d",
	    abs(token->line_num));
  }
  
  return (handle->line_num_info);

}

