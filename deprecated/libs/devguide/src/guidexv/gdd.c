/*
 * This file is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify this file without charge, but are not authorized to
 * license or distribute it to anyone else except as part of a product
 * or program developed by the user.
 * 
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 * 
 * This file is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 * 
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS FILE
 * OR ANY PART THEREOF.
 * 
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even
 * if Sun has been advised of the possibility of such damages.
 * 
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#ifndef lint
static char	sccsid[] = "@(#)gdd.c	2.38 91/10/15 Copyright 1991 Sun Microsystems";
#endif

#include <stdio.h>
#include <sys/param.h>
#ifdef SVR4
#include <sys/dirent.h>
#else
#include <sys/dir.h>
#endif
#include <sys/stat.h>
#include <xview/xview.h>
#include <xview/panel.h>
#include <xview/canvas.h>
#include <xview/dragdrop.h>
#include <xview/xv_xrect.h>
#include <xview/svrimage.h>
#include <devguide/gdd.h>

/* Internal Functions */
static void	drop_site_append();
static void	fill_drop_info();
static void	free_drop_info();
static void	begin_item_transfer();
static void	stop_dragdrop();
static void	drop_site_on();
static void	init_dnd_code_table();
static void	set_drag_data();

/* 
 * Temporary buffer size for managing INCR (incremental) transfers thru 
 * the selection service 
 */
#define XFER_BUFSIZ	5000

/*
 * Flag used to keep state during selection transfers.
 */
#define TARGET_RESPONSE         0x1
#define HOST_RESPONSE           0x2
#define TRANSPORT_RESPONSE      0x4
#define FILE_NAME_RESPONSE      0x8
#define DATA_LABEL_RESPONSE     0x10
#define BYTES_REQUESTED         0x20
#define BYTES_XFERRED           0x40

#define RESPONSE_LEVEL_1        0x1f
#define RESPONSE_LEVEL_2        0x7f


/*
 * Structure used to store the data as it is being accumulated.
 */
typedef struct
{
	char	*data;
	int	alloc_size;
	int	bytes_used;
} CHAR_BUF;


/* 
 * Context structure for describing the current drag/drop action 
 */
typedef struct
{
	Atom            *target_list;
	int             num_targets;
	Atom            *transport_list;
	int             num_transports;
	char            *data_label;
	int             object_count;
	char            *source_host;
	char            *source_name;
	char            *source_filename;
	Atom            chosen_target;
	Atom            chosen_transport;
	int             transfer_state;
	int             processing_stream;
	int             state_mask;
	int             stop_hit;
	int             objects_transferred;
	char		*tmpfile_name;
	FILE		*fd;
} GDD_CONTEXT;


/*
 * Structure to store information for each drop_site/drop_target.
 */
typedef struct Dnd_drop_site
{
	Xv_opaque		item;
	Event			*event;
	int			is_drop_site;
	int			is_move;
	struct Dnd_drop_site	*next;
	Xv_opaque		drop_site;
	Rect			*rectlist;
	void 			(*drop_func)();
	void 			(*drag_func)();
} GDD_DROP_SITE;

#ifdef SVR4
static char			Hostname[MAXNAMELEN];
#else
static char			Hostname[MAXHOSTNAMELEN];
#endif
static GDD_CONTEXT		*D;
GDD_DROP_SITE			*Drop_sites;
GDD_DROP_SITE			*Current_drop_site;
GDD_DROP_INFO			*Drop_info;
static Selection_requestor	Sel;
static Xv_Server		My_server;
static Xv_opaque		Base_frame;
static char			*Dnd_codes[8];
static char 			xfer_data_buf[XFER_BUFSIZ];

static int			debug_on = 0;
#define DEBUG_FLAG 		debug_on

#define DP(a)	{if( DEBUG_FLAG ) printf a ; }


static Atom text_atom;
static Atom current_type;
static Atom incr_atom;
static Atom targets_atom;
static Atom length_atom;
static Atom host_atom;
static Atom file_name_atom;
static Atom atm_file_name_atom;
static Atom delete_atom;
static Atom selection_end_atom;
static Atom dragdrop_done_atom;
static Atom data_label_atom;
static Atom name_atom;
static Atom alternate_transport_atom;
static Atom available_types_atom;
static Atom enumeration_count_atom;
static Atom enumeration_item_atom;
static Atom null_atom;


/*
 * Initialize the drag and drop code table.
 */
static void
init_dnd_code_table()
{
	Dnd_codes[0] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Completed");
	Dnd_codes[1] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Error");
	Dnd_codes[2] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Illegal Target");
	Dnd_codes[3] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Timed out");
	Dnd_codes[4] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Unable to obtain selection");
	Dnd_codes[5] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Dropped on root window");
	Dnd_codes[6] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Aborted");
	Dnd_codes[7] = (char *) xv_dgettext("libguidexv",
					 "Dragdrop: Unknown return code");
}

/*
 * Determine if a certain atom is in a list of atoms.
 */
static int
#ifdef __STDC__
atom_in_list(Atom atom, Atom *atom_list, int length)
#else
atom_in_list(atom, atom_list, length)
	Atom	atom;
	Atom	*atom_list;
	int	length;
#endif
{
	int	i;

	if (!atom_list)
		return(False);

	for (i = 0; i < length; i++)
		if (atom_list[i] == atom)
			return(True);

	return(False);
}

/* 
 * Clears the context block in preparation for a new drag/drop transaction. 
 */
static void
#ifdef __STDC__
clear_context(GDD_CONTEXT *dnd_ptr, int all_state)
#else
clear_context(dnd_ptr, all_state)
	GDD_CONTEXT	*dnd_ptr;
	int 		all_state;
#endif
{
	if (dnd_ptr->target_list)
		free(dnd_ptr->target_list);
	dnd_ptr->target_list = NULL;

	dnd_ptr->num_targets = 0;

	if (all_state)
	{
		dnd_ptr->objects_transferred = 0;
		dnd_ptr->object_count = 0;
	}

	if (dnd_ptr->transport_list)
		free(dnd_ptr->transport_list);
	dnd_ptr->transport_list = NULL;

	dnd_ptr->num_transports = 0;
	if (dnd_ptr->data_label)
		free(dnd_ptr->data_label);
	dnd_ptr->data_label = NULL;

	if (dnd_ptr->source_host)
		free(dnd_ptr->source_host);
	dnd_ptr->source_host = NULL;

	if (dnd_ptr->source_name)
		free(dnd_ptr->source_name);
	dnd_ptr->source_name = NULL;

	if (dnd_ptr->source_filename)
		free(dnd_ptr->source_filename);
	dnd_ptr->source_filename = NULL;

	dnd_ptr->chosen_target     = 0;
	dnd_ptr->chosen_transport  = 0;

	dnd_ptr->state_mask        = 0;
	dnd_ptr->stop_hit          = 0;
	dnd_ptr->processing_stream = 0;
	if (dnd_ptr->tmpfile_name)
		free(dnd_ptr->tmpfile_name);
	dnd_ptr->tmpfile_name      = NULL;
	dnd_ptr->fd 		   = 0;
}

/*
 * Copy str from character 0 to end to strbuf.
 */
static void
#ifdef __STDC__
strcpyupto(char *strbuf, char *str, char *end)
#else
strcpyupto(strbuf, str, end)
	char *strbuf;
	char *str;
	char *end;
#endif
{
	while (str != end)
		*strbuf++ = *str++;

	*strbuf = '\0';
}

/*
 * Parse the input string into dir and file.
 * The resulting file and dir is stored in the file and dir variable.
 * Note: dir and file have to be pre-allocated memory.
 */
static int
#ifdef __STDC__
parse_path(char *somedir, char *somefile, char *dir, char *file)
#else
parse_path(somedir, somefile, dir, file)
	char	*somedir;
	char	*somefile;
	char	*dir;
	char	*file;
#endif
{
	char 	*file_index;
	char	path[MAXPATHLEN];

	if (somedir && *somedir)
	{
		strcpy(path, somedir);
		strcat(path, "/");
		strcat(path, somefile);
	} else
		strcpy(path, somefile);

	if (!path || !*path)
		return FALSE;

	file_index = strrchr(path, '/');

	if (file_index)
	{
		strcpy(file, file_index+1);
		strcpyupto(dir, path, file_index);
	}
	else
	{
		strcpy(file, path);
		strcpy(dir, "./");
	}
	
	return TRUE;
}


/* 
 * Huge monolithic routine that processes all the replies 
 * to the questions that I ask about the current drag/drop 
 * selection.  These requests are made in groups, and the 
 * routine contains a state machine whose current state is stored 
 * in the D block.  This routine updates the 
 * state machine as replies, or failed replies come in.  
 * Changes in state may require issuing new requests for data, 
 * which are processed by this same routine.  
 */
static void
#ifdef __STDC__
reply_proc (Selection_requestor sel_req, Atom target, Atom type,
	    Xv_opaque replyBuf, unsigned long len, int format )
#else
reply_proc ( sel_req, target, type, replyBuf, len, format )
	Selection_requestor  	sel_req;
	Atom   			target;
	Atom   			type;
	Xv_opaque   		replyBuf;
	unsigned long  		len;
	int    			format;
#endif
{
	char		*atom_name;
	char		*target_name;
	int		*err_ptr = (int *) replyBuf;
	char		*char_buf = (char *) replyBuf;
	int		old_length;
	int		end_enum_context = -1;
	int		print_file();
	GDD_DROP_INFO	*di; 

	/* 
	 * Try to turn the type and target atoms into 
	 * some useful text for debugging. To use this feature, set the
	 * debug_on variable to 1 and recompile the package.
	 */
	if (debug_on)
	{
		if (type > 0)
			atom_name = XGetAtomName((Display *)
						 xv_get(My_server,
							XV_DISPLAY), type);
		else
			atom_name = "[None]";
		if (target > 0)
			target_name = XGetAtomName((Display *)
						   xv_get(My_server,
							  XV_DISPLAY),
						   target);
		else
			target_name = "[None]";

		DP(("reply_proc: type = %s/%d, target %s/%d\n",
		    atom_name, type, target_name, target));
		DP(("                      len = %d, format = %d, buf = %d, state %x\n",
		    len, format, err_ptr, D->state_mask));
	}

	/* 
	 * Simply processing the return from the termination 
	 * request.  No action necessary.
	 */
	if ((target == selection_end_atom) ||
	    (target == enumeration_item_atom))
		return;

	if ((len == SEL_ERROR) && ((*err_ptr) == SEL_BAD_CONVERSION))
	{
		/* 
		 * A conversion of some type failed.  Mark
		 * the state variable 
		 */
		if (target == targets_atom)
			D->state_mask |= TARGET_RESPONSE;
		else if (target == host_atom)
			D->state_mask |= HOST_RESPONSE;
		else if (target == alternate_transport_atom)
			D->state_mask |= TRANSPORT_RESPONSE;
		else if (target == file_name_atom)
			D->state_mask |= FILE_NAME_RESPONSE;
		else if (target == data_label_atom)
			D->state_mask |= DATA_LABEL_RESPONSE;
	}
	else if (len == SEL_ERROR)
	{
		/* 
		 * Some internal error happened as a result of 
		 * an earlier posted request.  Tell the user. The messages
		 * are very curt because the Drag and Drop package
		 * and the selection service package do not provide
		 * a detailed description of the error.
		 */

		switch (*err_ptr)
		{
		case SEL_BAD_PROPERTY :
		        xv_set(Base_frame, FRAME_LEFT_FOOTER,
			       xv_dgettext("libguidexv",
					"Dragdrop: Bad property"), NULL);
          		break;
      		case SEL_BAD_TIME:
          		xv_set(Base_frame, FRAME_LEFT_FOOTER,
			       xv_dgettext("libguidexv",
					"Dragdrop: Bad time"), NULL);
          		break;
      		case SEL_BAD_WIN_ID:
          		xv_set(Base_frame, FRAME_LEFT_FOOTER,
			       xv_dgettext("libguidexv",
					"Dragdrop: Bad window id"), NULL);
          		break;
      		case SEL_TIMEDOUT:
          		xv_set(Base_frame, FRAME_LEFT_FOOTER,
			       xv_dgettext("libguidexv",
					"Dragdrop: Timed out"), NULL);
          		break;
      		case SEL_PROPERTY_DELETED:
          		xv_set(Base_frame, FRAME_LEFT_FOOTER,
			       xv_dgettext("libguidexv",
					"Dragdrop: Property deleted"), NULL);
          		break;
      		case SEL_BAD_PROPERTY_EVENT:
          		xv_set(Base_frame, FRAME_LEFT_FOOTER,
			       xv_dgettext("libguidexv",
					"Dragdrop: Bad property event"), NULL);
          		break;
		}
		stop_dragdrop();
		return;
	}
	else if (type == incr_atom)
		D->processing_stream = TRUE;
    	else if ((target == XA_STRING) ||
		 (target == text_atom))
	{
		/* 
		 * Data stream coming thru. 
		 */
		if (len && !D->stop_hit)
		{
			/*
			 * The length is non-zero, so data, and 
			 * not the end of transmission. Create a name
			 * for the temporary file.
			 */
			if (!D->tmpfile_name)
			{
				D->tmpfile_name =
					strdup(tempnam("/tmp", "guide"));
				D->fd = fopen(D->tmpfile_name, "w");
			}

			fwrite(char_buf, 1, len, D->fd);

			if (!D->processing_stream)
			{

				fclose(D->fd);

				/*
				 * We tell the source that we are all done
				 * and call the applications callback
				 * function for the given drop_site/target.
				 */
 
				DP(("should print object number %d of %d, data_label: %s, tmpfile: %s\n", 
				    D->objects_transferred + 1, 
				    D->object_count, 
				    D->data_label,
				    D->tmpfile_name));
				
				fill_drop_info(D, &di, char_buf, len);
				Current_drop_site->drop_func(Current_drop_site->item, Current_drop_site->event, di);
				free_drop_info(di);

				D->state_mask |= BYTES_XFERRED;
			}
		}
	    	else if (D->processing_stream)
		{
			/* 
			 * The length was 0, so we have the 
			 * end of a data transmission. Fill the drop_info
			 * data structure and call the application's
			 * callback function for the given drop_site/target.
			 */
			fclose(D->fd);
 
			DP(("should print object number %d of %d, data_label: %s\n", 
			    D->objects_transferred, 
			    D->object_count, 
			    D->data_label));

			fill_drop_info(D, &di, char_buf, len);
			Current_drop_site->drop_func(Current_drop_site->item,
						     Current_drop_site->event,  di);
			free_drop_info(di);

			D->state_mask |= BYTES_XFERRED;

		}
		else
		{
 
			stop_dragdrop();
			return;
		}
	}
	else if (target == targets_atom)
	{
		if (len)
		{
			if (D->target_list &&
			    !D->processing_stream)
			{
				free(D->target_list);
				D->target_list = NULL;
			}

			if (!D->target_list)
			{
				D->target_list = (Atom *)
					malloc(len * sizeof(Atom));
				memcpy((char *) D->target_list, char_buf,
				       len * sizeof(Atom));
				if (!D->processing_stream)
				{
					D->state_mask |=
						TARGET_RESPONSE;
				}
			}
			else
			{
				D->target_list = (Atom *)
					realloc(D->target_list,
						D->num_targets * sizeof(Atom)
						+ len * sizeof(Atom));
				memcpy((char *) &D->target_list[D->num_targets
								- 1],
				       char_buf, len * sizeof(Atom));
			}
		}
		else
		{
			D->state_mask |= TARGET_RESPONSE;
			D->processing_stream = FALSE;
		}

		D->num_targets += len;
	}
	else if (target == alternate_transport_atom)
	{
		if (len)
		{
			if (D->transport_list &&
			    !D->processing_stream)
			{
				free(D->transport_list);
				D->transport_list = NULL;
			}

			if (!D->transport_list)
			{
				D->transport_list = (Atom *)
					malloc(len * sizeof(Atom));
				memcpy((char *) D->transport_list, char_buf,
				       len * sizeof(Atom));
				if (!D->processing_stream)
				{
					D->state_mask |=
						TRANSPORT_RESPONSE;
				}
			}
			else
			{
				D->transport_list = (Atom *)
					realloc(D->transport_list,
						D->num_transports * sizeof(Atom) +
						len * sizeof(Atom));
				memcpy((char *) &D->
				       transport_list[D->num_transports - 1],
				       char_buf, len * sizeof(Atom));
			}
		}
		else
		{
			D->state_mask |= TRANSPORT_RESPONSE;
			D->processing_stream = FALSE;
		}

		D->num_transports += len;
	}
	else if (target == name_atom)
	{
		if (len)
		{
			if (D->source_name && !D->processing_stream)
			{
				free(D->source_name);
				D->source_name = NULL;
			}

			if (!D->source_name)
			{
				D->source_name = (char *)
					malloc(len + 1);
				memcpy(D->source_name, char_buf, len);
				D->source_name[len] = '\0';
				if (!D->processing_stream)
					D->state_mask |= HOST_RESPONSE;
			}
			else
			{
				old_length = strlen(D->source_name);
				D->source_name = (char *) realloc(D->source_name, old_length + len + 1);
				memcpy(&D->source_name[old_length], char_buf, len);
				D->source_name[old_length + len] = '\0';
			}
		}
		else
		{
			D->state_mask |= HOST_RESPONSE;
			D->processing_stream = FALSE;
		}
	}
	else if (target == host_atom)
	{
		if (len)
		{
			if (D->source_host && !D->processing_stream)
			{
				free(D->source_host);
				D->source_host = NULL;
			}

			if (!D->source_host)
			{
				D->source_host = (char *)
					malloc(len + 1);
				memcpy(D->source_host, char_buf, len);
				D->source_host[len] = '\0';
				if (!D->processing_stream)
					D->state_mask |= HOST_RESPONSE;
			}
			else
			{
				old_length = strlen(D->source_host);
				D->source_host = (char *) realloc(D->source_host, old_length + len + 1);
				memcpy(&D->source_host[old_length], char_buf, len);
				D->source_host[old_length + len] = '\0';
			}
		}
		else
		{
			D->state_mask |= HOST_RESPONSE;
			D->processing_stream = FALSE;
		}
	}
	else if (target == file_name_atom)
	{
		if (len)
		{
			if (D->source_filename && !D->processing_stream)
			{
				free(D->source_filename);
				D->source_filename = NULL;
			}

			if (!D->source_filename)
			{
				D->source_filename = (char *)
					malloc(len + 1);
				memcpy(D->source_filename, char_buf, len);
				D->source_filename[len] = '\0';
				if (!D->processing_stream)
					D->state_mask |= FILE_NAME_RESPONSE;
			}
			else
			{
				old_length = strlen(D->source_filename);
				D->source_filename = (char *) realloc(D->source_filename, old_length + len + 1);
				memcpy(&D->source_filename[old_length], char_buf, len);
				D->source_filename[old_length + len] = '\0';
			}
		}
		else
		{
			D->state_mask |= FILE_NAME_RESPONSE;
			D->processing_stream = FALSE;
		}
	}
	else if (target == data_label_atom)
	{
		if (len)
		{
			if (D->data_label && !D->processing_stream)
			{
				free(D->data_label);
				D->data_label = NULL;
			}

			if (!D->data_label)
			{
				D->data_label = (char *)
					malloc(len + 1);
				memcpy(D->data_label, char_buf, len);
				D->data_label[len] = '\0';
				if (!D->processing_stream)
					D->state_mask |= DATA_LABEL_RESPONSE;
			}
			else
			{
				old_length = strlen(D->data_label);
				D->data_label = (char *) realloc(D->data_label, old_length + len + 1);
				memcpy(&D->data_label[old_length], char_buf, len);
				D->data_label[old_length + len] = '\0';
			}
		}
		else
		{
			D->state_mask |= DATA_LABEL_RESPONSE; 
			D->processing_stream = FALSE;
		}
	}
	else
		return;

	if (D->state_mask == RESPONSE_LEVEL_1)
	{
		DP(("first batch of replies processed, asking for second\n"));

                if ((!atom_in_list(XA_STRING, D->target_list, D->num_targets)) &&
                    (!atom_in_list(atm_file_name_atom, D->transport_list, D->num_targets)) &&
                    (!atom_in_list(text_atom, D->target_list, D->num_targets)))

		{
			xv_set(Base_frame, FRAME_LEFT_FOOTER, xv_dgettext("libguidexv","Dragdrop: Unknown type of data"), NULL);
			stop_dragdrop();
			return;
		}

		if (!atom_in_list(XA_STRING, D->target_list, D->num_targets))
			D->chosen_target = text_atom;
		else
			D->chosen_target = XA_STRING;
	
		/* 
		 * Determine what sort of data we have coming,
		 * get the host name to go with the file name.
		 */
	
		if (atom_in_list(atm_file_name_atom,
				 D->transport_list, D->num_transports) && 
		    D->source_filename)
		{
			if (!strcoll(Hostname, D->source_host))
			{
				/* 
				 * A valid filename has been recieved. Fill
				 * the drop_info_structure and call the
				 * application's callback function for the
				 * given drop_site/target.
				 */
				fill_drop_info(D, &di, char_buf, len);
				Current_drop_site->drop_func(Current_drop_site->item, Current_drop_site->event, di);
				free_drop_info(di);

				DP(("should print object number %d of %d, host: %s, name: %s\n",
				    D->objects_transferred, 
				    D->object_count, 
				    D->source_host, 
				    D->source_filename));

				D->objects_transferred++;
				if (D->objects_transferred < D->object_count)
				{
					clear_context(D, FALSE);
					begin_item_transfer();
					return;
				}
				else
				{
					stop_dragdrop();
					return;
				}
			}
		}
		/* 
		 * Data stream comming in. 
		 */
		DP(("asking for data stream, target %d (%s)\n",
		    D->chosen_target,
		    xv_get(My_server, SERVER_ATOM_NAME,
			   D->chosen_target)));

		xv_set(Sel,
		       SEL_REPLY_PROC, reply_proc,
		       SEL_TYPES, 
		       enumeration_item_atom,
		       D->chosen_target, 
		       enumeration_item_atom,
		       0,

		       SEL_TYPE_INDEX, 0,
		       SEL_PROP_TYPE, XA_INTEGER,
		       SEL_PROP_DATA, &D->objects_transferred,
		       SEL_PROP_FORMAT, 32,
		       SEL_PROP_LENGTH, 1,
		       SEL_TYPE_INDEX, 2,
		       SEL_PROP_TYPE, XA_INTEGER,
		       SEL_PROP_DATA, &end_enum_context,
		       SEL_PROP_FORMAT, 32,
		       SEL_PROP_LENGTH, 1,

		       NULL);
	
		sel_post_req(Sel);

		D->state_mask |= BYTES_REQUESTED;

	}
	else if (D->state_mask == RESPONSE_LEVEL_2)
	{

		D->objects_transferred++;
		if (D->objects_transferred < D->object_count)
		{
			clear_context(D, FALSE);
			begin_item_transfer();
		}
		else
		{
			stop_dragdrop();
			return;
		}
	}

}

/* 
 * Big routine to answer all the queries comming from the destination.
 * It uses data extacted from the drop_info data structure that has
 * been filled by the application before the drag has started.
 */
int
#ifdef __STDC__
convert_proc(Selection_owner seln, Atom *type, Xv_opaque *data,
	     unsigned long *length, int *format)
#else
convert_proc(seln, type, data, length, format)
	Selection_owner	 seln;
	Atom		*type;
	Xv_opaque	*data;
	unsigned long	*length;
	int		*format;
#endif
{
	int		length_buf;
	char		*atom_name;
	static Atom	target_list[12];
	static Atom	types_list[2];
	char		tmpfile [MAXPATHLEN];


	/* 
	 * Try to turn the type and target atoms into 
	 *  some useful text for debugging. 
	 */
	if (debug_on)
	{
		printf("convert_proc conversion called\n");

		if (type != NULL)
			atom_name = XGetAtomName((Display *)xv_get(My_server, XV_DISPLAY), *type);
		else
			atom_name = "[None]";

		printf("convert_proc: being asked to convert %s\n", atom_name);
	}

	/* 
	* Interesting sidelight here.  You cannot simply set the type 
	* in the reply to the type requested.  It must be the actual 
	* type of the data returned.  HOST_NAME, for example would 
	* be returned as type STRING. 
	*/

	if ((*type == selection_end_atom) || (*type == dragdrop_done_atom))
	{
		/* 
		 * Destination has told us it has completed the drag
		 * and drop transaction.  We should respond with a
		 * zero-length NULL reply and inform the application that
		 * the drag has been completed.
		 */

        	Current_drop_site->drag_func(Current_drop_site->item, 
				     Current_drop_site->event, 
				     NULL, 
				     GDD_DRAG_COMPLETED);	

		xv_set(xv_get(Current_drop_site->item, PANEL_DROP_DND), 
			SEL_CONVERT_PROC, NULL, 
			NULL);

		xv_set(Base_frame, 
			FRAME_LEFT_FOOTER, xv_dgettext("libguidexv", "Dragdrop: Completed"), 
			NULL);

		*format = 32;
		*length = 0;
		*data = 0;
		*type = null_atom;
		return(True);
	} 

	else if (*type == delete_atom) 
	{
		/* 
		 * In our case, we chose not to listen to delete 
	   	 * commands on the drag/drop item.  The user can
		 * check for a MOVE event and delete the data if
		 * desired when the drop in completed. 
		 */

		*format = 32;
		*length = 0;
		*data = 0;
		*type = null_atom;
		return(True);
	} 
	else if (*type == length_atom)
	{
		if (Drop_info->length)
		{
			length_buf = Drop_info->length;
			*format = 32;
			*length = 1;
			*type = XA_INTEGER;
			*data = (Xv_opaque) &length_buf;
			return(True);
		}
	} 
	else if (*type == enumeration_count_atom)
	{
		length_buf = 1;
		*format = 32;
		*length = 1;
		*type = XA_INTEGER;
		*data = (Xv_opaque) &length_buf;
		return(True);
	} 
	else if (*type == name_atom)
	{
		*format = 8;
		*length = strlen(Drop_info->app_name);
		*type = XA_STRING;
		*data = (Xv_opaque) Drop_info->app_name;
		return (True);
	}
	else if (*type == host_atom)
	{
		/* 
		 * Return the hostname that the application 
		 * is running on 
		 */
		*format = 8;
		*length = strlen(Hostname);
		*data = (Xv_opaque) Hostname;
		*type = XA_STRING;
		return(True);
	} 
	else if (*type == alternate_transport_atom)
	{
		/* 
		 * This request should return all of the targets 
		 * that can be converted on this selection.  This 
		 * includes the types, as well as the queries that 
		 * can be issued. 
		 */

		if ((Drop_info->filename && *Drop_info->filename))
		{
			*format = 32;
			*length = 1;
			*type = XA_ATOM;
			target_list[0] = atm_file_name_atom;
			*data = (Xv_opaque) target_list;
			return(True);
		}
		else
			return(False);
	} 
	else if (*type == targets_atom)
	{

		/* 
		 * This request should return all of the targets 
		 * that can be converted on this selection.  This 
		 * includes the types, as well as the queries that 
		 * can be issued. 
		 */

		*format = 32;
		*length = 0;
		*type = XA_ATOM;
		target_list[(*length)++] = XA_STRING;
		target_list[(*length)++] = text_atom;
		target_list[(*length)++] = delete_atom;
		target_list[(*length)++] = targets_atom;
		target_list[(*length)++] = host_atom;
		target_list[(*length)++] = length_atom;
		target_list[(*length)++] = selection_end_atom;
		target_list[(*length)++] = dragdrop_done_atom;
		target_list[(*length)++] = available_types_atom;
		target_list[(*length)++] = name_atom;

		if (current_type)
		{
			target_list[*length] = current_type;
			(*length)++;
		}
		if (!(Drop_info->filename && *Drop_info->filename))
		{
			target_list[*length] = alternate_transport_atom;
			(*length)++;
		}
		*data = (Xv_opaque) target_list;
		return(True);
    	} 
	else if (*type == available_types_atom)
	{
		/* 
		 * This target returns all of the data types that 
		 * the holder can convert on the selection. 
		 */
		*format = 32;
		*length = 0;
		*type = XA_ATOM;
		types_list[(*length)++] = XA_STRING;
		if (current_type)
		{
			types_list[(*length)++] = current_type;
		}
		*data = (Xv_opaque) types_list;
		return(True);
    	} 
	else if (*type == file_name_atom)
	{
		if (Drop_info->filename && *Drop_info->filename)
		{
			sprintf (tmpfile, "%s", Drop_info->filename);

			*format = 8;
			*length = strlen(tmpfile);
			*data = (Xv_opaque) tmpfile;
			*type = XA_STRING;
			return(True);
		}
		else
			return(False);
    	} 
	else if (*type == data_label_atom)
	{
		if (Drop_info->data_label && *Drop_info->data_label)
		{
			sprintf (tmpfile, "%s", Drop_info->data_label);

			*format = 8;
			*length = strlen(tmpfile);
			*data = (Xv_opaque) tmpfile;
			*type = XA_STRING;
			return(True);
		}
		else
			return(False);
    	} 
	else if ((*type == XA_STRING) || (*type == text_atom) || 
		 (*type == current_type)) 
    	{

		*format = 8;
		*length = Drop_info->length;
		if (Drop_info->data && *Drop_info->data)
			*data = (Xv_opaque) Drop_info->data;
		else
			*data = (Xv_opaque) Drop_info->filename;
		xfer_data_buf[*length] = '\0';

		if ((*type == text_atom) && current_type)
			*type = current_type;

		return(True);
	}
	else
	{
		/* 
		 * Let the default convert procedure deal with the
		 * request.
		 */

		return(sel_convert_proc(seln, type, data, length, format));
	}
}


/* 
 * Stop the drag and drop operation.  Converts _SUN_SELECTION_END 
 * on the current selection, signaling the end, and then 
 * puts the rest of the application back to normal. 
 */
static void
stop_dragdrop()

{
	int	length, format;

	DP(("stop_dragdrop called\n"));
	/*
	 * This code can be used to support a move operation.
	  if (Current_drop_site->is_move)
	  {
	  xv_set(Sel, SEL_TYPE, delete_atom, NULL);
	  (void)xv_get(Sel, SEL_DATA, &length, &format);
	  }
	  */

	/* Signal termination of transaction */
	xv_set(Sel,
	       SEL_REPLY_PROC, reply_proc,
	       SEL_TYPES, 
	       selection_end_atom, 
	       0,
	       NULL);

	sel_post_req(Sel);

	/* reactivate the drop site */

	drop_site_on();
 	
	clear_context(D, TRUE);
}

/*
 * Activate and de-activate the drop sites/targets.
 */
static void
drop_site_on()
{

	GDD_DROP_SITE 	*ds;

	for (ds = Drop_sites; ds; ds = ds->next)
	{
		if (ds->is_drop_site)
			xv_set(ds->drop_site,
			       DROP_SITE_DELETE_REGION_PTR, NULL,
			       DROP_SITE_REGION_PTR, ds->rectlist,
			       NULL);
		else
			xv_set(ds->drop_site, PANEL_BUSY, FALSE, NULL);
	}
}


/*
 * Set up the atoms that we will support, assign the convert proc,
 * and post the initial selection request.
 */
static void
begin_item_transfer()
{
	int	end_enum_context = -1;

	xv_set(Sel,
	       SEL_REPLY_PROC, reply_proc,
	       SEL_TYPES, 
	       enumeration_item_atom,
	       targets_atom,
	       host_atom,
	       name_atom,
	       alternate_transport_atom, 
	       file_name_atom,
	       data_label_atom,    
	       enumeration_item_atom,
	       0,

	       SEL_TYPE_INDEX, 0,
	       SEL_PROP_TYPE, XA_INTEGER,
	       SEL_PROP_DATA, &D->objects_transferred,
	       SEL_PROP_FORMAT, 32,
	       SEL_PROP_LENGTH, 1,
	       SEL_TYPE_INDEX, 6,
	       SEL_PROP_TYPE, XA_INTEGER,
	       SEL_PROP_DATA, &end_enum_context,
	       SEL_PROP_FORMAT, 32,
	       SEL_PROP_LENGTH, 1,

	       NULL);

	sel_post_req(Sel); 
}


/* 
 * Initial routine to kick of the transfer process.
 */
static void
#ifdef __STDC__
load_from_dragdrop(Xv_Server server, Event *event)
#else
load_from_dragdrop(server, event)
	Xv_Server	server;
	Event		*event;
#endif
{
	int		length, format;
	int		*count_ptr;
	GDD_DROP_SITE	*ds;

	if (debug_on)
	{
		printf("load_from_dragdrop: called\n");
	}


	/* deactivate the drop sites/targets */
	for (ds = Drop_sites; ds; ds = ds->next)
	{
		if (ds->is_drop_site)
			xv_set(ds->drop_site,
				DROP_SITE_DELETE_REGION_PTR, NULL,
				NULL);
		else
			xv_set(ds->drop_site, PANEL_BUSY, TRUE, NULL);
	}

	/* clear the left footer for new response status */

	xv_set(Base_frame, FRAME_LEFT_FOOTER, "", NULL);

	clear_context(D, TRUE);

	xv_set(Sel, SEL_TYPE, enumeration_count_atom, NULL);

	count_ptr = (int *) xv_get(Sel, SEL_DATA, &length, &format);

	if ((length <= 0) || !count_ptr)
	{
		/* 
		 * Not supporting enumeration count is not necessarily 
		 * fatal.  We just assume 1. 
		 */
		D->object_count = 1;
	}
	else
	{
		D->object_count = *count_ptr;
	}

	if (debug_on)
		printf("holder had %d items\n", D->object_count);

	begin_item_transfer();

	if (debug_on)
	{
		printf("load_from_dragdrop: after sel_post_req\n");
	}

}

/* 
 * Event proc used to track Drag and Drop events for the application.
 */
Notify_value
#ifdef __STDC__
gdd_load_event_proc(Xv_opaque window, Event *event,
		    Notify_arg arg, Notify_event_type type)
#else
gdd_load_event_proc(window, event, arg, type)
	Xv_opaque               window;
	Event                   *event;
	Notify_arg              arg;
	Notify_event_type       type;
#endif
{
	GDD_DROP_SITE	*ds;

       	Xv_Server       server = XV_SERVER_FROM_WINDOW(event_window(event));

	/* 
	 * Make sure the package has been initialized.
	 */
	if (!Base_frame)
	{
		printf("gdd: gdd_init_dragdrop(frame_handle) has not been called, the drag and drop package has not been initialized.\n");

		return NOTIFY_IGNORED;
	}

	switch (event_action(event))
	{
	case ACTION_DRAG_COPY:
	case ACTION_DRAG_MOVE:
		
		Current_drop_site = NULL;

		/* Determine if this is a drop site or a drop target. If
		 * it's a drop target, then just pass the event through
		 * and let the drop target notify proc handle the event.
		 * If it is a drop site, then figure out which one it is
		 * and make it the current drop site.
		 */
		for (ds = Drop_sites; ds; ds = ds->next)
		{
			if (!ds->is_drop_site && 
			    rect_includespoint((Rect *) xv_get(ds->drop_site,
							       XV_RECT), event->ie_locx, event->ie_locy))
			{
				/* Item is a drop target */
				return (notify_next_event_func(window, 
							       (Notify_event) event, arg, type));
			}

			/*
			 * We can use the macro dnd_site_id() to access
			 * the site id of the drop site that was
			 * dropped on.
			 */
			if (xv_get(ds->drop_site,
				   DROP_SITE_ID) == dnd_site_id(event))
			{
				Current_drop_site = ds;
				Current_drop_site->item = window;
				Current_drop_site->event = event;
				if (event_action(event) ==  
				    ACTION_DRAG_MOVE)
					ds->is_move = TRUE;
				else
					ds->is_move = FALSE;
				break;
			}
		}

		if (!Sel)
			Sel = xv_create(window, SELECTION_REQUESTOR, NULL);

                /* 
		 * To acknowledge the drop and to associate the
		 * rank of the source's selection to our
		 * requestor selection object, we call
		 * dnd_decode_drop().
		 */

        	if (dnd_decode_drop(Sel, event) != XV_ERROR) 
		{
			if (Current_drop_site)
                		load_from_dragdrop(server, event);
        	} 
		else
		{
			if (debug_on)
            			printf ("drop error\n");
		}

		return NOTIFY_DONE;
	}

	return (notify_next_event_func(window, (Notify_event) event, arg, type));
}

/*
 * Notify procedure for drop targets to deal with drag and drop operations.
 */
int
#ifdef __STDC__
gdd_drop_target_notify_proc(Panel_item item, unsigned int value, Event *event)
#else
gdd_drop_target_notify_proc(item, value, event)
	Panel_item	item;
	unsigned int	value;
	Event		*event;
#endif
{

	GDD_DROP_SITE	*ds;
       	Xv_Server       server;
    	Drag_drop	dnd = xv_get(item, PANEL_DROP_DND);

	if (!Base_frame)
	{
		printf("gdd: gdd_init_dragdrop(frame_handle) has not been called, the drag and drop package has not been initialized.\n");

		return XV_ERROR;
	}

	Sel = (Selection_requestor) xv_get(item, PANEL_DROP_SEL_REQ);

	switch (event_action(event))
	{
	case ACTION_DRAG_COPY:
	case ACTION_DRAG_MOVE:

		Current_drop_site = NULL;

		for (ds = Drop_sites; ds; ds = ds->next)
		{
			if (!ds->is_drop_site && (item == ds->drop_site) &&
			    ds->drop_func)
			{
				Current_drop_site = ds;
				Current_drop_site->item = item; 
				Current_drop_site->event = event; 
				if (event_action(event) ==  
				    ACTION_DRAG_MOVE)
					ds->is_move = TRUE;
				else
					ds->is_move = FALSE;
				break;

			}
		}

		if (Current_drop_site)
		{
			server = XV_SERVER_FROM_WINDOW(event_window(event));
               		load_from_dragdrop(server, event);
		}

		break;

	case LOC_DRAG:
		if (value == XV_OK)
		{
			Current_drop_site = NULL;

			for (ds = Drop_sites; ds; ds = ds->next)
			{
				if (!ds->is_drop_site && item == ds->drop_site)
				{
					Current_drop_site = ds;
					Current_drop_site->item = item; 
					Current_drop_site->event = event; 
					break;

				}
			}

			if (Current_drop_site && Current_drop_site->drag_func)
			{
				Drop_info = (GDD_DROP_INFO *) calloc(1, sizeof(GDD_DROP_INFO));
				Current_drop_site->drag_func(item, event, Drop_info, GDD_DRAG_STARTED);	
				set_drag_data(&Drop_info);

				xv_set(dnd, SEL_CONVERT_PROC, convert_proc, NULL);
			}
		}
		else
          		xv_set(Base_frame, 
			       FRAME_LEFT_FOOTER, Dnd_codes[MIN(value, 7)], 
			       NULL);

		break;
	default:
		break;
	}

	return XV_ERROR;
}

/*
 * Initialize the dragdrop package. Set up a frame handle to post 
 * messages to.
 */
void
#ifdef __STDC__
gdd_init_dragdrop(Xv_opaque frame)
#else
gdd_init_dragdrop(frame)
	Xv_opaque frame;

#endif
{
	static int	initialized = FALSE;

	if (!frame || (xv_get(frame, FRAME_LEFT_FOOTER) == XV_ERROR))
	{
		fprintf(stderr, "Invalid Base Frame handle passed to gdd_init_dragdrop().\n");
	}
	else
		Base_frame = frame;

	/*
	 * Initialize atom variables and error messages. Only do
	 * this once.
	 */
	if (initialized)
		return;

	My_server = XV_SERVER_FROM_WINDOW(Base_frame);
#ifdef SVR4	
	gethostname(Hostname, MAXNAMELEN);
#else
	gethostname(Hostname, MAXHOSTNAMELEN);
#endif

	text_atom = xv_get(My_server, SERVER_ATOM, "TEXT");
	incr_atom = xv_get(My_server, SERVER_ATOM, "INCR");
	targets_atom = xv_get(My_server, SERVER_ATOM, "TARGETS");
	length_atom = xv_get(My_server, SERVER_ATOM, "LENGTH");
	host_atom = xv_get(My_server, SERVER_ATOM, "_SUN_FILE_HOST_NAME");
	name_atom = xv_get(My_server, SERVER_ATOM, "NAME");
	file_name_atom = xv_get(My_server, SERVER_ATOM, "FILE_NAME");
	atm_file_name_atom = xv_get(My_server, SERVER_ATOM,
				    "_SUN_ATM_FILE_NAME");
	delete_atom = xv_get(My_server, SERVER_ATOM, "DELETE");
	selection_end_atom = xv_get(My_server, SERVER_ATOM, "_SUN_SELECTION_END");
	dragdrop_done_atom = xv_get(My_server, SERVER_ATOM, "_SUN_DRAGDROP_DONE");
	data_label_atom = xv_get(My_server, SERVER_ATOM, "_SUN_DATA_LABEL");
	alternate_transport_atom = xv_get(My_server, SERVER_ATOM, "_SUN_ALTERNATE_TRANSPORT_METHODS");
	available_types_atom = xv_get(My_server, SERVER_ATOM, "_SUN_AVAILABLE_TYPES");

	enumeration_count_atom = xv_get(My_server, SERVER_ATOM, "_SUN_ENUMERATION_COUNT");
	enumeration_item_atom = xv_get(My_server, SERVER_ATOM, "_SUN_ENUMERATION_ITEM");
	null_atom = xv_get(My_server, SERVER_ATOM, "NULL");
	current_type = '\0';
	
	D = (GDD_CONTEXT *) malloc(sizeof(GDD_CONTEXT));
	memset((char *) D, 0, sizeof(GDD_CONTEXT));
	
	init_dnd_code_table();

	initialized = TRUE;
}

/*
 * Register a drop site with the drag and drop target. Basically just
 * adds a new data structure to a linked list.
 */
void
#ifdef __STDC__
gdd_register_drop_site(Xv_opaque drop_site, void (*f)())
#else
gdd_register_drop_site(drop_site, f)
	Xv_opaque	drop_site;
	void 		(*f)();
#endif
{
	GDD_DROP_SITE	*ds;

	ds = (GDD_DROP_SITE *) calloc(1, sizeof(GDD_DROP_SITE));
	
	ds->drop_site = drop_site;
	ds->is_drop_site = TRUE;
	ds->drop_func = f;

	drop_site_append(&Drop_sites, ds);

}

/*
 * Register a drop target with the drag and drop target. Basically,
 * just adds a new data structure to a linked list. 
 */
void
#ifdef __STDC__
gdd_register_drop_target(Xv_opaque target_item, void (*drop_func)(),
			 void (*drag_func)())
#else
gdd_register_drop_target(target_item, drop_func, drag_func)
	Xv_opaque	target_item;
	void 		(*drop_func)();
	void 		(*drag_func)();
#endif
{
	GDD_DROP_SITE	*ds;

	ds = (GDD_DROP_SITE *) calloc(1, sizeof(GDD_DROP_SITE));

	ds->drop_site = target_item;
	ds->is_drop_site = FALSE;
	ds->drop_func = drop_func;
	if (drag_func)
	{
		ds->drag_func = drag_func;
	}

	drop_site_append(&Drop_sites, ds);

}

/* Unregister a drop site or a drop target from the drag and drop 
 * package.
 */
void
#ifdef __STDC__
gdd_unregister_drop_site(Xv_opaque drop_site)
#else
gdd_unregister_drop_site(drop_site)
	Xv_opaque	drop_site;
#endif
{

	GDD_DROP_SITE 	*ds;
	GDD_DROP_SITE 	*previous;
    	Drag_drop	dnd; 

	for (ds = Drop_sites, previous = Drop_sites; ds; ds = ds->next)
	{
		if (ds->drop_site == drop_site)
		{
			if (ds == Drop_sites)
			{
				Drop_sites = ds->next;
			}
			else
			{
				previous->next = ds->next;
			}
			
			/*
			 * If drop target, make sure that it does not
			 * have a convert proc on it.
			 */
			if (!ds->is_drop_site)
			{
 				dnd = xv_get(drop_site, PANEL_DROP_DND);
				if (dnd)
					xv_set(dnd,
						SEL_CONVERT_PROC, NULL,
						NULL);
 			}

			free(ds);
			break;
		}
		else
			previous = ds;
	}
}

/*
 * Activate a drop site. Accepts a pointer to a list of Rect structures
 * that indicate which regions within a pane are valid drop regions.
 * The list needs to be terminated with a Rect whose width or height
 * are set to zero. This function only works for previously registered
 * drop sites.
 * If NULL is passed in in the rectlist field, the region will become
 * a non-active region within the window.
 */
void
#ifdef __STDC__
gdd_activate_drop_site(Xv_opaque drop_site, Rect rectlist[])
#else
gdd_activate_drop_site(drop_site, rectlist)
	Xv_opaque	drop_site;
	Rect		rectlist[];
#endif
{
	int		i, count;
	GDD_DROP_SITE	*ds;
	Rect		*rp;

	for (ds = Drop_sites; ds; ds = ds->next)
	{
		if (ds->drop_site == drop_site)
			break;
	}

	if (!ds || !ds->drop_site)
	{
		fprintf(stderr, "gdd_activate_drop_site: Invalid drop site\n");
		return;
	}


	if (rectlist)
	{
		for (count = 0, rp = rectlist; !rect_isnull(rp); rp++, count++)
			;

		if (!ds->rectlist)
			ds->rectlist = (Rect *) malloc((count + 1) *
						       sizeof(Rect));
		else if (sizeof(ds->rectlist) != ((count +1) * sizeof(Rect)))
		{
			free(ds->rectlist);
			ds->rectlist = (Rect *) malloc((count + 1) *
						       sizeof(Rect));
		}

		for (i = 0, rp = rectlist; !rect_isnull(rp); rp++, i++)
			ds->rectlist[i] = *rp;
		ds->rectlist[i] = *rp; /* include last item (Null item) */


		if (ds->rectlist)
		{
			xv_set(ds->drop_site,
			       DROP_SITE_REGION_PTR, ds->rectlist,
			       NULL);
		}
	}
	else
	{
		free(ds->rectlist);
		xv_set(ds->drop_site, DROP_SITE_DELETE_REGION_PTR, NULL, NULL);
	}
}

/*
 * Utility function to add data structures to the link list.
 */
static void
#ifdef __STDC__
drop_site_append(GDD_DROP_SITE **drop_site, GDD_DROP_SITE *ds)
#else
drop_site_append(drop_site, ds)
	GDD_DROP_SITE	**drop_site, *ds;
#endif
{
	
	if (!(*drop_site))
	{
		*drop_site = (GDD_DROP_SITE *) calloc(1,
						      sizeof(GDD_DROP_SITE));
		*drop_site = ds;
		(*drop_site)->next = NULL;
	}
	else
	{
		ds->next = *drop_site;
		*drop_site = ds;
	}
}

/*
 * Ready a drop info structure to pass back to the application after
 * a drop has been processed.
 */
static void
#ifdef __STDC__
fill_drop_info(GDD_CONTEXT *dnd_ptr, GDD_DROP_INFO **di_ptr,
	       char *char_buf, int len)
#else
fill_drop_info(dnd_ptr, di_ptr, char_buf, len)
	GDD_CONTEXT	*dnd_ptr;
	GDD_DROP_INFO 	**di_ptr;
	char		*char_buf;
	int		len;
#endif
{
	char	dir[MAXPATHLEN];
	char	file[MAXPATHLEN];

	*di_ptr = (GDD_DROP_INFO *) calloc(1, sizeof(GDD_DROP_INFO));

	if (dnd_ptr->tmpfile_name &&
	    (strcmp(dnd_ptr->tmpfile_name, "") != 0 ))
	{
		if (dnd_ptr->data_label)
		{
			parse_path(NULL, dnd_ptr->tmpfile_name, dir, file);
			sprintf(file,"%s/%s", dir, dnd_ptr->data_label);
			rename(dnd_ptr->tmpfile_name, file);
			strcpy(dnd_ptr->tmpfile_name, file);
		}
	}	

	if (dnd_ptr->data_label)
		(*di_ptr)->data_label = strdup(dnd_ptr->data_label);
	if (dnd_ptr->source_host)
		(*di_ptr)->source_host =  strdup(dnd_ptr->source_host);
	if (dnd_ptr->source_name)
		(*di_ptr)->app_name =  strdup(dnd_ptr->source_name);
	if (dnd_ptr->source_filename)
		(*di_ptr)->filename =  strdup(dnd_ptr->source_filename);
	if (dnd_ptr->tmpfile_name)
		(*di_ptr)->tmpfile =  strdup(dnd_ptr->tmpfile_name);
	if(char_buf && len > 0)
	{
		(*di_ptr)->data = strdup(char_buf);
		(*di_ptr)->length = len;
	}

}

/*
 * Free up the drop info structure.
 */
static void
#ifdef __STDC__
free_drop_info(GDD_DROP_INFO *di_ptr)
#else
free_drop_info(di_ptr)
	GDD_DROP_INFO	*di_ptr;
#endif
{
	if (!di_ptr)
		return;

	if (di_ptr->data_label)
		free(di_ptr->data_label);
	if (di_ptr->source_host)
		free(di_ptr->source_host);
	if (di_ptr->app_name)
		free(di_ptr->app_name);
	if (di_ptr->filename)
		free(di_ptr->filename);
	if (di_ptr->data)
		free(di_ptr->data);
	if (di_ptr->tmpfile)
	{
		unlink(di_ptr->tmpfile);
		free(di_ptr->tmpfile);
	}
	
	free(di_ptr);
}

/*
 * Set up any drag data that needs to be set but wasn't by the application.
 */
static void
#ifdef __STDC__
set_drag_data(GDD_DROP_INFO **di)
#else
set_drag_data(di)
	GDD_DROP_INFO	**di;
#endif
{
	if (((*di)->data && *(*di)->data) && !(*di)->length)
		(*di)->length = strlen((*di)->data);
}


/*
 * Utility function to be used for experimentation with different
 * kind of sources dropping on the application.
 */
void
#ifdef __STDC__
gdd_print_drop_info(GDD_DROP_INFO *di)
#else
gdd_print_drop_info(di)
	GDD_DROP_INFO	*di;
#endif
{
	
	printf("\tdata_label: %s\n", di->data_label ? di->data_label : "");
	printf("\tsource_host: %s\n", di->source_host ? di->source_host : "");
	printf("\tapp_name: %s\n", di->app_name ? di->app_name : "");
	printf("\tfilename: %s\n", di->filename ? di->filename : "");
	printf("\ttmpfile: %s\n", di->tmpfile ? di->tmpfile : "");
	printf("\tlength: %d\n", di->length);
	printf("\tdata: %s\n", di->data ? di->data : "");
	printf("\n");
}

/*
 * NOTE: This routine is no longer supported. It has been replaced with
 * the new Drag and Drop package.
 *
 * Get the name of the file being dragged and dropped.  Sets the file name
 * and returns 0 if successful, otherwise returns -1.
 */
int
#ifdef __STDC__
gdd_get_drag_name(Xv_window window, char *name)
#else
gdd_get_drag_name(window, name)
	Xv_window       window;			/* event window */
	char           *name;			/* MAXPATHLEN length buffer */
#endif
{
	printf("gdd_get_drag_name is no longer a supported function\n");
	return -1;
}

/*
 * NOTE: This routine is no longer supported. It has been replaced with
 * the new Drag and Drop package.
 *
 * Send the name of a file being dragged and dropped.  Returns 0 if 
 * successful, otherwise -1.
 */
int
#ifdef __STDC__
gdd_set_drag_name(Xv_window source_win, Xv_window dest_win,
		  int x, int y, char *name)
#else
gdd_set_drag_name(source_win, dest_win, x, y, name)
	Xv_window	source_win;		/* source window */
	Xv_window	dest_win;		/* destination window */
	int		x;			/* drop x */
	int		y;			/* drop y */
	char	       *name;			/* file name */
#endif
{
	printf("gdd_set_drag_name is no longer a supported function\n");
	return -1;
}

/*
 * NOTE: This routine is no longer supported. It has been replaced with
 * the new Drag and Drop package.
 *
 * Drag a glyph around the screen, send an ACTION_DRAG_LOAD message to
 * the window under the pointer when dropped.  Returns XID of winow dropped
 * on, -1 for error.
 */
int
#ifdef __STDC__
gdd_drag_file(Xv_window src_win, Server_image drag_image, char *filename)
#else
gdd_drag_file(src_win, drag_image, filename)
	Xv_window	src_win;
	Server_image	drag_image;
	char		*filename;
#endif
{
	printf("gdd_set_drag_name is no longer a supported function\n");
	return -1;
}
