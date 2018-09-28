/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

# include "dorade_headers.h"
# include "solo_editor_structs.h"
# include <ui.h>
# include <ui_error.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;

/*
 * se_assert_bad_flags
 * se_bad_flags_logic
 * se_clear_bad_flags
 * se_copy_bad_flags
 * se_set_bad_flags
 * 
 */

void se_do_clear_bad_flags_array();

/* c------------------------------------------------------------------------ */

int se_assert_bad_flags(arg, cmds)	/* #assert-bad-flags# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *dst_name;

    int nc, nd, bad, fn;
    int ii, nn, mark;
    short *ss, *zz;
    unsigned short *bnd, *flag;


    dst_name = (cmdq++)->uc_text;
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    flag = seds->bad_flag_mask;
    /*
     * find the field
     */
    if((fn = dd_find_field(dgi, dst_name)) < 0) {
	/* field not found
	 */
	uii_printf("Field to be asserted: %s not found\n", dst_name);
	seds->punt = YES;
	return(-1);
    }
    bad = dds->parm[fn]->bad_data;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;

    /*
     * loop through the data
     */

    for(; ss < zz; ss++,bnd++,flag++) {
	if(*bnd && *flag)
	      *ss = bad;
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

int se_flagged_add(arg, cmds)	/* #flagged-add# */
  int arg;
  struct ui_command *cmds;	
{
    /* #flagged-add#
     * #flagged-multiply#
     */
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name, *where;
    float f_const;

    int nc, bad, fn;
    int gg, ii, jj, kk, nn, scaled_const, mark;
    short *ss, *zz;
    unsigned short *bnd, *flag;

    f_const = (cmdq++)->uc_v.us_v_float;
    name = (cmdq++)->uc_text;

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
    /*
     * find the field
     */
    if((fn = dd_find_field(dgi, name)) < 0) {
	/* field not found
	 */
	uii_printf("Flagged add field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    scaled_const = DD_SCALE(f_const, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    bad = dds->parm[fn]->bad_data;

    /*
     * loop through the data
     */

    if(strstr(cmds->uc_text, "d-m")) { /* multiply */
	for(; ss < zz; ss++,bnd++,flag++) {
	    if(*bnd && *ss != bad && *flag) {
		*ss *= f_const;
	    }
	}
    }
    else {			/* add */
	for(; ss < zz; ss++,bnd++,flag++) {
	    if(*bnd && *ss != bad && *flag) {
		*ss += scaled_const;
	    }
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_bad_flags_logic(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     * #and-bad-flags#
     * #or-bad-flags#
     * #xor-bad-flags#
     */
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name, *where;
    float f_thr1=0, f_thr2=0;

    int nc, nchar, bad, fn;
    int gg, ii, jj, kk, nn, scaled_thr1, scaled_thr2, mark;
    short *ss, *zz, *thr=NULL;
    unsigned short *bnd, *flag;


    name = (cmdq++)->uc_text;
    nchar = strlen(name);
    where = (cmdq++)->uc_text;
    f_thr1 = (cmdq++)->uc_v.us_v_float;
    if(cmdq->uc_ctype != UTT_END)
	  f_thr2 = cmdq->uc_v.us_v_float;

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
    /*
     * find the thr field
     */
    if((fn = dd_find_field(dgi, name)) < 0) {
	/* thr field not found
	 */
	uii_printf("Bad flag logic field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    thr = (short *)dds->qdat_ptrs[fn];
# else
    thr = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = thr +nc;
    scaled_thr1 = DD_SCALE(f_thr1, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    scaled_thr2 = DD_SCALE(f_thr2, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    bad = dds->parm[fn]->bad_data;

    /*
     * loop through the data
     */

    if(strncmp(where, "below", 3) == 0) {
	if(strncmp(cmds->uc_text, "and", 3) == 0) {
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr == bad) {
		    *flag = 0;
		}
		else {
		    *flag &= *thr < scaled_thr1;
		}
	    }
	}
	else if(strncmp(cmds->uc_text, "xor", 3) == 0) {
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		      *flag ^= *thr < scaled_thr1;
	    }
	}
	else {			/* or */
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		      *flag |= *thr < scaled_thr1;
	    }
	}
    }
    else if(strncmp(where, "above", 3) == 0) {
	if(strncmp(cmds->uc_text, "and", 3) == 0) {
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr == bad) {
		    *flag = 0;
		}
		else {
		    *flag &= *thr > scaled_thr1;
		}
	    }
	}
	else if(strncmp(cmds->uc_text, "xor", 3) == 0) {
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		      *flag ^= *thr > scaled_thr1;
	    }
	}
	else {			/* or */
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad)
		      *flag |= *thr > scaled_thr1;
	    }
	}
    }
    else {			/* between */
	if(strncmp(cmds->uc_text, "and", 3) == 0) {
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr == bad) {
		    *flag = 0;
		}
		else {
		    *flag &= *thr >= scaled_thr1 && *thr <= scaled_thr2;
		}
	    }
	}
	else if(strncmp(cmds->uc_text, "xor", 3) == 0) {
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad) {
		    *flag ^= *thr >= scaled_thr1 && *thr <= scaled_thr2;
		}
	    }
	}
	else {			/* or */
	    for(; thr < zz; thr++,bnd++,flag++) {
		if(!(*bnd))
		      continue;
		if(*thr != bad) {
		    *flag |= *thr >= scaled_thr1 && *thr <= scaled_thr2;
		}
	    }
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_clear_bad_flags(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
  /* #clear-bad-flags#
   * #complement-bad-flags#
   */
  int nn;
  unsigned short *flag;
  struct solo_edit_stuff *seds, *return_sed_stuff();


    seds = return_sed_stuff();
    if(seds->finish_up)
	  return(1);

    if(strncmp(cmds->uc_text, "complement-bad-flags", 3) == 0) {
      flag = seds->bad_flag_mask_array;
      nn = seds->max_gates;
      for(; nn--; flag++) {
	*flag = *flag ? 0 : 1;
      }
    }
    else {
      se_do_clear_bad_flags_array(0);
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */

int se_copy_bad_flags(arg, cmds)	/* #copy-bad-flags# */
  int arg;
  struct ui_command *cmds;	
{
    /* creates a bad flag mask corresponding to the bad flagged
     * gates in the test field
     */
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name, *where;

    int nc, nd, nchar, bad, thr_bad, fn, fgg;
    int gg, ii, jj, kk, nn, scaled_thr1, scaled_thr2, mark;
    short *anchor, *ss, *zz, *thr=NULL;
    unsigned short *bnd, *flag;


    name = (cmdq++)->uc_text;
    nchar = strlen(name);

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    fgg = seds->first_good_gate;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
    /*
     * find the thr field
     */
    if((fn = dd_find_field(dgi, name)) < 0) {
	/* thr field not found
	 */
	uii_printf("Copy bad flags field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    thr = (short *)dds->qdat_ptrs[fn];
# else
    thr = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    thr_bad = dds->parm[fn]->bad_data;
    zz = thr +nc;
    bad = dds->parm[fn]->bad_data;

    /*
     * loop through the data
     */

    for(; thr < zz; thr++,bnd++,flag++) {
	if(!(*bnd))
	      continue;
	*flag = *thr == bad ? 1 : 0;
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

void se_do_clear_bad_flags_array(nn)
  int nn;
{
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();
    memset(seds->bad_flag_mask_array, 0, (nn > 0 ? nn : seds->max_gates) *
	   sizeof(*seds->bad_flag_mask_array));
}
/* c------------------------------------------------------------------------ */

int se_set_bad_flags(arg, cmds)	/* #set-bad-flags# */
  int arg;
  struct ui_command *cmds;	
{
    int below;
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;
    char *name, *dst_name, *where;
    float f_thr1, f_thr2;

    int nc, nd, nchar, bad, thr_bad, fn, fgg;
    int gg, ii, jj, kk, nn, scaled_thr1, scaled_thr2, mark, fthr;
    short *anchor, *ss, *zz, *thr=NULL;
    unsigned short *bnd, *flag;


    name = (cmdq++)->uc_text;
    nchar = strlen(name);
    where = (cmdq++)->uc_text;
    f_thr1 = (cmdq++)->uc_v.us_v_float;
    if(cmdq->uc_ctype != UTT_END)
	  f_thr2 = cmdq->uc_v.us_v_float;

    seds = return_sed_stuff();
    if(seds->finish_up) {
	return(1);
    }
    seds->modified = YES;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;
    nc = dgi->clip_gate+1;
    fgg = seds->first_good_gate;
    bnd = seds->boundary_mask;
    seds->bad_flag_mask = flag = seds->bad_flag_mask_array;
    /*
     * find the thr field
     */
    if((fn = dd_find_field(dgi, name)) < 0) {
	/* thr field not found
	 */
	uii_printf("Set bad flags field: %s not found\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    thr = (short *)dds->qdat_ptrs[fn];
# else
    thr = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    thr_bad = dds->parm[fn]->bad_data;
    zz = thr +nc;
    scaled_thr1 = DD_SCALE(f_thr1, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    scaled_thr2 = DD_SCALE(f_thr2, dds->parm[fn]->parameter_scale
			  , dds->parm[fn]->parameter_bias);
    bad = dds->parm[fn]->bad_data;
    se_do_clear_bad_flags_array(nc);

    /*
     * loop through the data
     */

    if(strncmp(where, "below", 3) == 0) {
	for(; thr < zz; thr++,bnd++,flag++) {
	    if(!(*bnd) || *thr == bad)
		  continue;
	    if(*thr < scaled_thr1) {
		*flag = 1;
	    }
	}
    }
    else if(strncmp(where, "above", 3) == 0) {
	for(; thr < zz; thr++,bnd++,flag++) {
	    if(!(*bnd) || *thr == bad)
		  continue;
	    if(*thr > scaled_thr1) {
		*flag = 1;
	    }
	}
    }
    else {			/* between */
	for(; thr < zz; thr++,bnd++,flag++) {
	    if(!(*bnd) || *thr == bad)
		  continue;
	    if(*thr >= scaled_thr1 && *thr <= scaled_thr2) {
		*flag = 1;
	    }
	}
    }
    return(fn);
}  
/* c------------------------------------------------------------------------ */

