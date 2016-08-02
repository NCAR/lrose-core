#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)stat_imcb.c 1.15 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1990 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#ifdef OW_I18N
/*
 * "stat_imcb.c" XIM Status callback interface routines.  Actual
 * routine is beside the libxvol/frame code.
 */

#include <xview_private/i18n_impl.h>
#include <xview_private/portable.h>
#include <X11/Xlib.h>

#include <xview/xview.h>
#include <xview/frame.h>


Xv_private void
_xv_status_start(ic, client_data, cb_data)
	XIC				*ic;
	XPointer			 client_data;
	XIMStatusDrawCallbackStruct	*cb_data;
{
	/*
	 * Frame code actually initiatializes the status region when
	 * WIN_USE_IM is TRUE.
	 */
}

#define CB_STRING	cb_data->data.text->string
#define	CB_LENGTH	cb_data->data.text->length

Xv_private void
_xv_status_draw(ic, client_data, cb_data)
	XIC				*ic;
	XPointer			 client_data;
	XIMStatusDrawCallbackStruct	*cb_data;
{
	Frame		frame;

	frame = (Frame)client_data;

	switch (cb_data->type)
	{
	   case XIMTextType: {
		static unsigned short	ws_status_len = 0;
                static wchar_t	       *ws_status_str = NULL;
 
		/* 
		 * XIMText does not guarantee a null terminated string,
		 * so need to insure this.
		 */
		if (ws_status_len < CB_LENGTH) {
		   if (ws_status_str != NULL)
			  xv_free(ws_status_str);
		   ws_status_str = xv_alloc_n(wchar_t, CB_LENGTH+1);
		   ws_status_len = CB_LENGTH;
		}
                if (cb_data->data.text->encoding_is_wchar) 
                    wsncpy(ws_status_str, (wchar_t *)CB_STRING.wide_char, CB_LENGTH);

                else
		    mbstowcs(ws_status_str, (char *)CB_STRING.multi_byte, CB_LENGTH);	
		ws_status_str[CB_LENGTH] = '\0';

		/*
		 * FIX_ME: Length is being sent in terms of multibyte
		 * characters, but feedback length is being sent in terms of
		 * wide characters. For now, just checking initial feedback 
		 * value to determine feedback.
		 * 
		 * If feedback is XIMTertiary then stipple the feedback
		 * so that the IM status region is inactive.  
		 */
		if (cb_data->data.text->feedback != NULL && 
		    cb_data->data.text->feedback[0] == XIMTertiary) 
		    xv_set(frame,
                           FRAME_LEFT_IMSTATUS_WCS, ws_status_str,
			   FRAME_INACTIVE_IMSTATUS, TRUE,
			   NULL);
		else
		    xv_set(frame,
                           FRAME_LEFT_IMSTATUS_WCS, ws_status_str,
			   FRAME_INACTIVE_IMSTATUS, FALSE,
			   NULL);

		break;
	   }

	   case BitmapType:
		/*
		 * Currently not implemented by IM server.
		 */
		break;

	   default:
		/*
		 * Any other types are not supported.
		 */
		break;
	}
}


Xv_private void
_xv_status_done(ic, client_data, cb_data)
	XIC				*ic;
	XPointer			 client_data;
	XIMStatusDrawCallbackStruct	*cb_data;
{
#ifdef notdef

	Frame		frame;
	static wchar_t	*wcs_null = {'\0'};

	frame = (Frame)client_data;

	/* Clear the status region */
	xv_set(frame,
	       FRAME_LEFT_IMSTATUS_WCS,	wcs_null,
	       NULL);
#endif /* notdef */
}
#endif /* OW_I18N */
