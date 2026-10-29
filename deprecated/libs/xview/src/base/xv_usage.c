#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#)xv_usage.c 1.18 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include <stdio.h>
#include <xview_private/i18n_impl.h>

/*
 * Print usage message to stderr
 */
void
xv_usage(name)
    char           *name;
{
    /*
     * Long string has to split because the entry for the long string
     * in the .po file (for i18n/l10n) is too long for vi. It's OK for
     * emacs/textedit
     */
    (void) fprintf(stderr, 
    XV_MSG("usage of %s generic window arguments:\n\
FLAG\t(LONG FLAG)\t\tARGS\t\tNOTES\n\
-Ww\t(-width)\t\tcolumns\n\
-Wh\t(-height)\t\tlines\n\
-Ws\t(-size)\t\t\tx y\n\
-Wp\t(-position)\t\tx y\n\
\t(-geometry)\t\"[WxH][{+|-}X{+|-}Y]\"\t(X geometry)\n"),
		   name ? name : "");

    (void) fprintf(stderr, 
    XV_MSG("-WP\t(-icon_position)\tx y\n\
-Wl\t(-label)\t\t\"string\"\n\
\t(-title)\t\t\"string\"\t(Same as -label)\n\
-Wi\t(-iconic)\t\t(Application will come up closed)\n\
+Wi\t(+iconic)\t\t(Application will come up open)\n\
-Wt\t(-font)\t\t\tfontname\n\
-fn\t\t\t\tfontname\t\n\
-Wx\t(-scale)\t\tsmall | medium | large | extra_large\n"));

    (void) fprintf(stderr, 
    XV_MSG("-Wf\t(-foreground_color)\tred green blue\t0-255 (no color-full color)\n\
-fg\t(-foreground)\t\tcolorname\t(X Color specification)\n\
-Wb\t(-background_color)\tred green blue\t0-255 (no color-full color)\n\
-bg\t(-background)\t\tcolorname\t(X Color specification)\n\
-rv\t(-reverse)\t\t(Foreground and background colors will be reversed)\n\
+rv\t(+reverse)\t\t(Foreground and background colors will not be reversed)\n"));

    (void) fprintf(stderr, 
    XV_MSG("-WI\t(-icon_image)\t\tfilename\n\
-WL\t(-icon_label)\t\t\"string\"\n\
-WT\t(-icon_font)\t\tfilename\n\
-Wr\t(-display)\t\t\"server_name:screen\"\n\
-visual\t\t\t\tStaticGray | GrayScale | StaticColor |\n\
\t\t\t\tPseudoColor | TrueColor | DirectColor\n\
-depth\t\t\t\tdepth\n\
-Wdr\t(-disable_retained)\t\n\
-Wdxio\t(-disable_xio_error_handler)\t\n"));

    (void) fprintf(stderr, 
    XV_MSG("-Wfsdb\t(-fullscreendebug)\t\n\
-Wfsdbs\t(-fullscreendebugserver)\t\n\
-Wfsdbp\t(-fullscreendebugptr)\t\n\
-Wfsdbk\t(-fullscreendebugkbd)\t\n\
-Wdpgs\t(-disable_pass_grab_select)\t\n"));

    (void) fprintf(stderr, 
    XV_MSG("-WS\t(-defeateventsecurity)\t\n\
-sync\t(-synchronous)\t\t\t\t(Force a synchronous connection)\n\
+sync\t(+synchronous)\t\t\t\t(Make an asynchronous connection)\n\
-Wd\t(-default)\t\tresource value\t(Set the X resource to value)\n\
-xrm\t\t\t\tresource:value\t(Set the X resource to value)\n"));

    (void) fprintf(stderr, 
    XV_MSG("-name\t\t\t\tstring\t(Set application instance name to string)\n\
-lc_basiclocale\t\t\tlocale\t(Set basic locale of application to locale)\n\
-lc_displaylang\t\t\tlocale\t(Set display language of application to locale)\n\
-lc_inputlang\t\t\tlocale\t(Set input language of application to locale)\n\
-lc_numeric\t\t\tlocale\t(Set numeric format of application to locale)\n\
-lc_timeformat\t\t\tlocale\t(Set time format of application to locale)\n"));

    (void) fprintf(stderr,
    XV_MSG("-preedit_style\t\t\tonTheSpot | overTheSpot | rootWindow | none\n\
\t\t\t\t(Set input method preedit style)\n\
-status_style\t\t\tclientDisplays | imDisplaysInClient | none\n\
\t\t\t\t(Set input method status style)\n\
-WH\t(-help)\t\n"));

    exit(97);
}
