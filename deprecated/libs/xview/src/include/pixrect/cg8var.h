/* @(#)cg8var.h	1.14 of 8/22/90 SMI */

/*
 * Copyright 1988 by Sun Microsystems, Inc.
 */

#ifndef cg8var_DEFINED
#define	cg8var_DEFINED

/* <sundev/p4reg.h> and <pixrect/memvar.h> included */
#include <pixrect/cg4var.h>
#ifndef SVR4
#include <sundev/cg8reg.h>
#include <sunwindow/cms.h>
#else
#include <sys/cms.h>
#endif /* SVR4 */

/* FBIOSATTR device specific array indices, copied from cg4var.h */
#define	FB_ATTR_CG8_SETOWNER_CMD	0	/* 1 indicates PID is valid */
#define	FB_ATTR_CG8_SETOWNER_PID	1	/* new owner of device */


#define	CG8_NFBS	3

#define	CG8_PRIMARY		0x01	/* Mark the PRIMARY Pixrect	*/
#define	CG8_OVERLAY_CMAP	0x02	/* Overlay CMAP to be changed	*/
#define	CG8_24BIT_CMAP		0x04	/* 24 Bit CMAP to be changed 	*/
#define	CG8_KERNEL_UPDATE	0x08	/* kernel vs. user ioctl	*/
					/* 0x10 & 0x20 are dbl buf in cg9 */
#define	CG8_SLEEPING		0x40	/* Denote if wake_up is necessary */
#define	CG8_COLOR_OVERLAY	0x80	/* view overlay & enable as 2 bits */
#define	CG8_UPDATE_PENDING	0x100

struct cg8_data
{
	struct	mprp_data	mprp;		/* memory pixrect simulator */
	int			flags;		/* misc. flags */
	int			planes;		/* current group and mask */
	int			fd;		/* file descriptor */
	short			active;		/* active fb no. */
	struct	cg4fb		fb[CG8_NFBS];	/* frame buffer info */
	int			windowfd;	/* if 8-bit indexed pw */
	struct	colormapseg	cms;		/* if 8-bit indexed pr */
	int			real_windowfd;	/* if 8-bit indexed pw */
};

#define	cg8_d(pr)	((struct cg8_data *)((pr)->pr_data))

#define CG8_PR_TO_MEM(src, mem)						\
    if (src && src->pr_ops != &mem_ops)					\
    {									\
	mem.pr_ops = &mem_ops;						\
	mem.pr_size = src->pr_size;					\
	mem.pr_depth = src->pr_depth;					\
	mem.pr_data = (char *) &cg8_d(src)->mprp;			\
	src = &mem;							\
    }

#define CG8_PR_TO_MEM32(pr, mem32_pr, mem32_pr_data)			\
    if (pr && pr->pr_ops != &mem_ops)					\
    {									\
	mem32_pr.pr_ops = &mem32_ops;					\
	mem32_pr.pr_size = pr->pr_size;					\
	mem32_pr.pr_depth = pr->pr_depth;				\
	mem32_pr_data.mprp = cg8_d(pr)->mprp;				\
	mem32_pr_data.plane_group = cg8_d(pr)->fb[cg8_d(pr)->active].group;\
	mem32_pr_data.fd = cg8_d(pr)->fd;				\
	mem32_pr_data.windowfd = cg8_d(pr)->windowfd;			\
	mem32_pr_data.cms = cg8_d(pr)->cms;				\
	mem32_pr.pr_data = (char *) &mem32_pr_data;			\
	pr = &mem32_pr;							\
    }

extern struct pixrectops cg8_ops;

int		cg8_putcolormap();
int		cg8_putattributes();
int		cg8_ioctl();

#ifndef KERNEL

Pixrect		*cg8_make();
int		cg8_destroy();
Pixrect		*cg8_region();
int		cg8_getcolormap();
int		cg8_getattributes();
int		cg8_vector();
int		cg8_get();
int		cg8_put();
int		cg8_rop();

#endif /*	!KERNEL */

#endif /* cg8var_DEFINED */
