#ifndef lint
#ifdef sccs
static char     sccsid[] = "@(#) quark.c 50.11 93/06/28";
#endif
#endif

/*
 *	(c) Copyright 1989 Sun Microsystems, Inc. Sun design patents 
 *	pending in the U.S. and foreign countries. See LEGAL NOTICE 
 *	file for terms of the license.
 */

#include  <X11/X.h>
#include  <X11/Xlib.h>
#include  <X11/Xresource.h>
#include  <xview/xview.h>
#include  <xview/pkg.h>

static Xv_opaque resource_type_conv();

/* 
 *    Utilities to deal with quark lists and such.
 */
Xv_private Xv_opaque 
db_name_from_qlist(qlist) 
    XrmQuarkList    qlist;
{   
    register int    i;  

    if (qlist == NULL)
	return(XV_ZERO);
    
    for (i = 0; qlist[i] != NULLQUARK; i++) 
                ;
    if (i != 0) 
        return((Xv_opaque)XrmQuarkToString(qlist[i - 1]));
    else 
        return((Xv_opaque)NULL);
}

Xv_private XrmQuarkList
db_qlist_from_name(char *name, XrmQuarkList parent_quarks)
{
    register int    i;
    int		    num_quarks = 0;
    XrmQuarkList    quarks;

    if (name == NULL) 
	return(NULL);

    if (parent_quarks != NULL) {
        for ( ;parent_quarks[num_quarks] != NULLQUARK; num_quarks++)
			;
        quarks = (XrmQuarkList) xv_calloc(num_quarks + 2, sizeof(XrmQuark));
        for (i = 0; i < num_quarks; i++)
	    quarks[i] = parent_quarks[i];
    } else {
	quarks = (XrmQuarkList) xv_calloc(2, sizeof(XrmQuark));
	i = 0;
    }

    quarks[i++] = XrmStringToQuark(name);
    quarks[i] = NULLQUARK;

    return(quarks);
}

Xv_private Xv_opaque
db_get_data(db, instance_qlist, attr_name, attr, default_value)
    XrmDatabase		db;
    XrmQuarkList	instance_qlist;
    char		*attr_name;
    Attr_attribute	attr;
    Xv_opaque		default_value;
{
    Xv_opaque           result;
    XrmRepresentation   quark_type;
    XrmValue            value;
    XrmQuark            *qlist;
    Attr_base_cardinality	type;
    register int	i = 0;
    int			num_quarks = 0;

    if (instance_qlist)  {
	/*
	 * Figure out how many quarks in list
	 */
        for (num_quarks = 0; instance_qlist[num_quarks] != NULLQUARK; 
		num_quarks++);

	/*
	 * Alloc quark array
	 * The additional two quarks - for attr_name and NULLQUARK
	 */
	qlist = (XrmQuark *) xv_calloc(num_quarks+2, sizeof(XrmQuark));

	/*
	 * Copy quark array
	 */
        for (i = 0; instance_qlist[i] != NULLQUARK; i++)
	    qlist[i] = instance_qlist[i];
    }
    else  {
	/*
	 * If no instance_qlist, alloc quarks for attr_name, NULLQUARK
	 */
	qlist = (XrmQuark *) xv_calloc(2, sizeof(XrmQuark));
    }

    qlist[i++] = XrmStringToQuark(attr_name);
    qlist[i] = NULLQUARK;

    /*
     * Get type of attribute
     */
    type = ATTR_WHICH_TYPE(attr);

    if (XrmQGetResource(db, qlist, qlist, &quark_type, &value) == True)
	result = 
	    (Xv_opaque)resource_type_conv(value.addr, type, default_value);
    else 
	result = default_value;
    
    free((char *)qlist);

    return(result);
}
    

static Xv_opaque
resource_type_conv(str, xv_type, def_val)
        char			*str;
        Attr_base_cardinality	xv_type;
        Xv_opaque		def_val;
{
    Xv_opaque           to_val;

    switch (xv_type) {
        case ATTR_LONG:
          db_cvt_string_to_long(str, &to_val);
          return (to_val);
 
        case ATTR_X:
        case ATTR_Y:
        case ATTR_INT:
          db_cvt_string_to_int(str, &to_val);
          return (to_val);
 
        case ATTR_BOOLEAN:
          db_cvt_string_to_bool(str, &to_val);
          return (to_val);
 
        case ATTR_CHAR:
          db_cvt_string_to_char(str, &to_val);
          return (to_val);
 
        case ATTR_STRING:
          to_val = (Xv_opaque) str;
          return (to_val);
 
#ifdef OW_I18N
        case ATTR_WSTRING:
           db_cvt_string_to_wcs(str, &to_val);
           return (to_val);
#endif
        default:
          return (def_val);
    }
}

