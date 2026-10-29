#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)es_mem.c 20.25 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

/*
 * Entity stream implementation for one block of virtual memory.
 */

#include <string.h>
#include <sys/types.h>
#include <xview/pkg.h>
#include <xview/attrol.h>
#include <xview_private/primal.h>
#include <xview_private/txt_18impl.h>
#include <xview_private/es.h>
#ifdef SVR4 
#include <stdlib.h> 
#endif /* SVR4 */

typedef struct es_mem_text {
    Es_status       status;
    CHAR           *value;
    u_int           length;
    u_int           position;
    u_int           max_length;
    u_int           initial_max_length;
    Xv_opaque       client_data;
}               Es_mem_text;
typedef Es_mem_text *Es_mem_data;
#define	ABS_TO_REP(esh)	(Es_mem_data)esh->data

/* extern CHAR    *STRNCPY(); */

Pkg_private Es_handle es_mem_create();
static Es_status es_mem_commit();
static Es_handle es_mem_destroy();
static Es_index es_mem_get_length();
static Es_index es_mem_get_position();
static Es_index es_mem_set_position();
static Es_index es_mem_read();
static Es_index es_mem_replace();
static int      es_mem_set();

static struct es_ops es_mem_ops = {
    es_mem_commit,
    es_mem_destroy,
    es_mem_get,
    es_mem_get_length,
    es_mem_get_position,
    es_mem_set_position,
    es_mem_read,
    es_mem_replace,
    es_mem_set
};

Pkg_private     Es_handle
es_mem_create(max, init)
    u_int           max;
    CHAR           *init;
{
    Es_handle       esh = NEW(Es_object);
    Es_mem_data     private = NEW(Es_mem_text);

    if (esh == ES_NULL) {
	return (ES_NULL);
    }
    if (private == 0) {
	free((char *) esh);
	return (ES_NULL);
    }
    private->initial_max_length = max;
    if (max == ES_INFINITY) {
	max = 10000;
    }
    private->value = MALLOC(max + 1);
    if (private->value == 0) {
	free((char *) private);
	free((char *) esh);
	return (ES_NULL);
    }
    (void) STRNCPY(private->value, init, (int) max);
    private->value[max] = '\0';
    private->length = STRLEN(private->value);
    private->position = private->length;
    private->max_length = max - 1;

    esh->ops = &es_mem_ops;
    esh->data = (caddr_t) private;
    return (esh);
}

/* ARGSUSED */
static          Es_status
es_mem_commit(esh)
    Es_handle       esh;
{
    return (ES_SUCCESS);
}

static          Es_handle
es_mem_destroy(esh)
    Es_handle       esh;
{
    register Es_mem_data private = ABS_TO_REP(esh);

    free((char *) esh);
    free(private->value);
    free((char *) private);
    return (ES_NULL);
}

/* ARGSUSED */
caddr_t
#ifdef ANSI_FUNC_PROTO
es_mem_get(Es_handle esh, Es_attribute attribute, ...)
#else
es_mem_get(esh, attribute, va_alist)
    Es_handle       esh;
    Es_attribute    attribute;
va_dcl
#endif
{
    register Es_mem_data private = ABS_TO_REP(esh);
#ifndef lint
    va_list         args;
#endif

    switch (attribute) {
      case ES_CLIENT_DATA:
	return ((caddr_t) (private->client_data));
      case ES_NAME:
	return (0);
      case ES_STATUS:
	return ((caddr_t) (private->status));
      case ES_SIZE_OF_ENTITY:
	return ((caddr_t) sizeof(CHAR));
      case ES_TYPE:
	return ((caddr_t) ES_TYPE_MEMORY);
      default:
	return (0);
    }
}

static int
es_mem_set(esh, attrs)
    Es_handle       esh;
    Attr_avlist     attrs;
{
    register Es_mem_data private = ABS_TO_REP(esh);
    Es_status       status_dummy = ES_SUCCESS;
    register Es_status *status = &status_dummy;

    for (; *attrs && (*status == ES_SUCCESS); attrs = attr_next(attrs)) {
	switch ((Es_attribute) * attrs) {
	  case ES_CLIENT_DATA:
	    private->client_data = attrs[1];
	    break;
	  case ES_STATUS:
	    private->status = (Es_status) attrs[1];
	    break;
	  case ES_STATUS_PTR:
	    status = (Es_status *) attrs[1];
	    *status = status_dummy;
	    break;
	  default:
	    *status = ES_INVALID_ATTRIBUTE;
	    break;
	}
    }
    return ((*status == ES_SUCCESS));
}

static          Es_index
es_mem_get_length(esh)
    Es_handle       esh;
{
    register Es_mem_data private = ABS_TO_REP(esh);
    return (private->length);
}

static          Es_index
es_mem_get_position(esh)
    Es_handle       esh;
{
    register Es_mem_data private = ABS_TO_REP(esh);
    return (private->position);
}

static          Es_index
es_mem_set_position(esh, pos)
    Es_handle       esh;
    Es_index        pos;
{
    register Es_mem_data private = ABS_TO_REP(esh);

    if (pos > private->length) {
	pos = private->length;
    }
    return (private->position = pos);
}

static          Es_index
es_mem_read(esh, len, bufp, resultp)
    Es_handle       esh;
    u_int           len, *resultp;
    register CHAR  *bufp;
{
    register Es_mem_data private = ABS_TO_REP(esh);

    if (private->length - private->position < len) {
	len = private->length - private->position;
    }
    BCOPY(private->value + private->position, bufp, (int) len);
    *resultp = len;
    return (private->position += len);
}

static          Es_index
es_mem_replace(esh, end, new_len, new, resultp)
    Es_handle       esh;
    int             end, new_len, *resultp;
    CHAR           *new;
{
    int             old_len, delta;
    CHAR           *start, *keep, *restore;
    register Es_mem_data private = ABS_TO_REP(esh);

    *resultp = 0;
    if (new == 0 && new_len != 0) {
	private->status = ES_INVALID_ARGUMENTS;
	return ES_CANNOT_SET;
    }
    if (end > private->length) {
	end = private->length;
    } else if (end < private->position) {
	int             tmp = end;
	end = private->position;
	private->position = tmp;
    }
    old_len = end - private->position;
    delta = new_len - old_len;

    if (delta > 0 && private->length + delta > private->max_length) {
	CHAR           *new_value = (CHAR *) 0;
	if (private->initial_max_length == ES_INFINITY) {
#ifdef OW_I18N
	    new_value = (CHAR *)realloc(private->value,
		((private->max_length + delta + 10000 + 1) * sizeof(CHAR)));
#else
	    new_value = realloc(private->value,
				private->max_length + delta + 10000 + 1);
#endif
	    if (new_value) {
		private->value = new_value;
		private->max_length += delta + 10000;
	    }
	}
	if (!new_value) {
	    private->status = ES_SHORT_WRITE;
	    return ES_CANNOT_SET;
	}
    }
    start = private->value + private->position;
    keep = private->value + end;
    restore = start + new_len;
    if (delta != 0) {
	BCOPY(keep, restore, (int) private->length - end + 1);
    }
    if (new_len > 0) {
	BCOPY(new, start, new_len);
    }
    private->position = end + delta;
    private->length += delta;
    private->value[private->length] = '\0';
    *resultp = new_len;
    return restore - private->value;
}

#ifdef DEBUG
Pkg_private void
es_mem_dump(fd, pdh)
    FILE           *fd;
    Es_mem_data     pdh;
{
#ifdef OW_I18N
    extern CHAR		_xv_null_string_wc[];
#endif

    (void) fprintf(fd, "\n\t\t\t\t\t\tmax length:  %ld", pdh->max_length);
    (void) fprintf(fd, "\n\t\t\t\t\t\tcurrent length:  %ld", pdh->length);
    (void) fprintf(fd, "\n\t\t\t\t\t\tposition:  %ld", pdh->position);
#ifdef OW_I18N
    (void) fprintf(fd, "\n\t\t\t\t\t\tvalue (%lx):  \"%ws\"",
		   pdh->value, (pdh->value ? pdh->value : _xv_null_string_wc));
#else
    (void) fprintf(fd, "\n\t\t\t\t\t\tvalue (%lx):  \"%s\"",
		   pdh->value, (pdh->value ? pdh->value : ""));
#endif /* OW_I18N */
}

#endif /* DEBUG */
