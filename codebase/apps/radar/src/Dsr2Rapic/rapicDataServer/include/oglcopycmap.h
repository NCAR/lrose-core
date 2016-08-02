/*
 * This file contains information used for cloning colors from
 * colors from the default X colormap to a copied colormap of the
 * appropriate depth for GL.  The following types determine how
 * to select the colors from the X colormap.
 */

#define GLXC_ABSOLUTE	0	/* use absolute pixel value */
#define GLXC_NAMED	1	/* look up by name */
#define GLXC_RESOURCE	2	/* lookup resource in parent widget */
    
/*
 * An array must be passed in of the following structure type, describing
 * the colors to be passed in.  The first three fields must be filled
 * in for each color, with the XColor being calculated.  Based on type,
 * value should be as follows:
 *
 *	type		value
 *	GLXC_ABSOLUTE	the pixel number to copy (e.g. 5)
 *	GLXC_NAMED	a color name (e.g. "red")
 *	GLXC_RESOURCE	a pixel resource name (e.g. XmNbackground)
 */

struct glxcColorInfo
{
    int type;		/* One of above types */
    caddr_t value;	/* based on above types */
    Colorindex *result;	/* store result in this variable */
    XColor color;	/* the xcolor definition */
};

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern void CopyGlColormap(Widget, struct glxcColorInfo *, int);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif
