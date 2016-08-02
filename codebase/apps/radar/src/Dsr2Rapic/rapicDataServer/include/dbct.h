/*

	dbct.h	-	Header file for c-tree Plus database for 3D-Rapic

*/

#include "ctstdr.h"
#include "ctoptn.h"
#include "ctaerr.h"
#include "cterrc.h"
#include "ctdecl.h"
#include "ctifil.h"

/*
	LOCKING POLICY - 
		The Rapic isam database will only add records.
		ie - NO UPDATES
		HOWEVER - Write locks will be used to allow multiple
			Rapics to add to the database.
			THIS REQUIRES EACH RAPIC TO LOCK THE DATA FILE FIRST,
			ADD THE NEW DATA, THEN LOCK THE ISAM, ADD THE RECORD, UNLOCK THE
			ISAM, THEN UNLOCK THE DATA FILE.
			IF ALL RAPICS USE THIS APPROACH, THE FIRST TO GET THE LOCK
			ON THE DATA FILE WILL LOCK ALL OTHERS OUT UNTIL FINISHED.
			TRANSACTIONS WILL BE ATOMIC.
*/

/*
	CTREE is used in a mulit-user incremental ISAM mode, to allow
	the application to control the database files.
*/

ISEG rp_db_stnseg[4] = {
	{ 2, 2, INTSEG },
	{ 4, 4, INTSEG },
	{ 8, 2, INTSEG },
	{10, 2, INTSEG }
	};

ISEG rp_db_dttmseg[4] = {
	{ 4, 4, INTSEG },
	{ 2, 2, INTSEG },
	{ 8, 2, INTSEG },
	{10, 2, INTSEG }
	};

#define KEYSIZE sizeof(short)+sizeof(time_t)+sizeof(short)+sizeof(rdr_angle)
IIDX rp_db_idx[2] = {
	{	KEYSIZE,				// key length
		12,							// 0-fixed len, 4-ld ch comp, 8 trl ch comp, 12 both comp
		0,							// no dup. keys
		0,							// no null key checking
		32,							// null char
		4,							// no of key segments
		rp_db_stnseg,		// stn/date/time/angle key seg data
		"stn_key"},			// index name
	{ KEYSIZE,        // key length
		12,             // 0-fixed len, 4-ld ch comp, 8 trl ch comp, 12 both comp
		0,              // no dup. keys
		0,              // no null key checking
		32,             // null char
		4,              // no of key segments
		rp_db_dttmseg,  // date/stn/time/angle key seg data
		"dttm_key"},   	// index name
};


IFIL rp_db_ifil = {
	"",								// filled in later
	-1,								// data file no. - let OPEN/CREATE assign it
	sizeof(img_rec),	// data record length
	4096,							// data extension size (incremental growth step)
	SHARED | PERMANENT | FIXED | WRITETHRU,	// default file mode
	2,								// no. of indices
	4096,							// index extension size (incremental growth step)
	SHARED | PERMANENT | FIXED | WRITETHRU,	// default index mode
	rp_db_idx,				// IIDX DATA
	"",								// r-tree data
	""								// r-tree data
										// null: tfilno
	};



