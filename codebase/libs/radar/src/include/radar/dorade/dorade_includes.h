/* 	$Id: dorade_includes.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */


# ifndef DORADEINCLUDES_H
# define DORADEINCLUDES_H

# include <radar/dorade/dd_defines.h>
# include <radar/dorade/dd_math.h>
# include <radar/dorade/Comment.h>
# include <radar/dorade/viraq.h>
# include <radar/dorade/super_SWIB.h>
# include <radar/dorade/Volume.h>
# include <radar/dorade/RadarDesc.h>
# include <radar/dorade/Correction.h>
# include <radar/dorade/Parameter.h>
# include <radar/dorade/CellVector.h>
# include <radar/dorade/CellSpacingFP.h>
# include <radar/dorade/Sweep.h>
# include <radar/dorade/Platform.h>
# include <radar/dorade/Ray.h>
# include <radar/dorade/Pdata.h>
# include <radar/dorade/Qdata.h>
# include <radar/dorade/Xtra_stuff.h>
# include <radar/dorade/FieldRadar.h>

# ifdef just_for_documentation

# include "/zeb/src/ingest/radar/Ethernet.h"

typedef struct s_ENHeader
{
        struct ether_header en_hdr;     /* The ethernet header          */
        unsigned short en_type;         /* FIRST or CONTINUE            */
        unsigned long en_seq;           /* Sequence number              */
        unsigned short en_fperrad;      /* Frames per radial            */
        unsigned short en_number;       /* Number of this frame in rad  */
        unsigned short en_res[4];       /* Reserved junk                */
/*
 * For CP2, the following two fields refer to byte numbers rather than
 * gate numbers.
 */
        unsigned short en_g_first;      /* First gate                   */
        unsigned short en_g_last;       /* Last gate                    */
} ENHeader;

# endif

# endif /* DORADEINCLUDES_H */




