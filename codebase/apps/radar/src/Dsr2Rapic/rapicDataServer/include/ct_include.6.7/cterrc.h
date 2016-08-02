/*
 *      OFFICIAL NOTIFICATION: the following CONFIDENTIAL and PROPRIETARY 
 * 	property legend shall not be removed from this source code module 
 * 	for any reason.
 *
 *	This program is the CONFIDENTIAL and PROPRIETARY property 
 *	of FairCom(R) Corporation. Any unauthorized use, reproduction or
 *	transfer of this computer program is strictly prohibited.
 *
 *      Copyright (c) 1984 - 1997 FairCom Corporation.
 *	This is an unpublished work, and is subject to limited distribution and
 *	restricted disclosure only. ALL RIGHTS RESERVED.
 *
 *			RESTRICTED RIGHTS LEGEND
 *	Use, duplication, or disclosure by the Government is subject to
 *	restrictions set forth in subparagraph (c)(1)(ii) of the Rights in
 * 	Technical Data and Computer Software clause at DFARS 252.227-7013.
 *	FairCom Corporation, 4006 West Broadway, Columbia, MO 65203.
 *
 *	c-tree PLUS(tm)	Version 6.7
 *			Release A2
 *			August 1, 1997
 */
#ifndef ctERRCH
#define ctERRCH

		/* USER ERROR CODES */
#ifdef 	NO_ERROR
#undef  NO_ERROR
#endif
#define NO_ERROR	0

#define	KDUP_ERR	2	/* Key value already exists */
#define	KMAT_ERR	3	/* Could not delete since pntr's don't match */
#define	KDEL_ERR	4	/* Could not find key to delete */
#define	KBLD_ERR	5	/* Cannot call delete w/o verification with  */
				/* duplicate keys */
#define BJMP_ERR	6	/* c-tree(...) jump table error */
#define TUSR_ERR	7	/* Terminate user */

#define FCNF_COD	-8	/* sysiocod value when FNOP_ERR caused by
				   conflicting open requests (server) */
#define FDEV_COD	-9	/* sysiocod value when FNOP_ERR, DCRAT_ERR or
				   KCRAT_ERR caused by device access error */

#define SPAC_ERR	10	/* INTREE parameters require too much space */
#define SPRM_ERR	11	/* Bad INTREE parameters */
#define FNOP_ERR	12	/* Could not open file: not there or locked */
#define	FUNK_ERR	13	/* Unknown file type */
#define FCRP_ERR	14	/* File corrupt at open */
#define FCMP_ERR	15	/* File has been compacted */
#define KCRAT_ERR	16	/* Could not create index file */
#define DCRAT_ERR	17	/* Could not create data file */
#define KOPN_ERR	18	/* Tried to create existing index file */
#define DOPN_ERR	19	/* Tried to create existing data file */
#define KMIN_ERR	20	/* Key length too large for node size */
#define DREC_ERR	21	/* Record length too small */
#define FNUM_ERR	22	/* File number out of range */
#define KMEM_ERR	23	/* Illegal index member info */
#define FCLS_ERR	24	/* Could not close file */
#define KLNK_ERR	25	/* Bad link in deleted node list. REBUILD */
#define FACS_ERR	26	/* File number not active */
#define LBOF_ERR	27	/* drn before beginning of data records */
#define ZDRN_ERR	28	/* Zero drn in ADDKEY */
#define ZREC_ERR	29	/* Zero drn in data file routine */
#define LEOF_ERR	30	/* drn exceeds logical end of file */
#define DELFLG_ERR	31	/* Flag not set on record in delete chain */
#define	DDRN_ERR	32	/* Attempt to delete record twice in a row */
#define DNUL_ERR	33	/* Attempt to use NULL ptr in read/write */
#define PRDS_ERR	34	/* Predecessor repeat attempts exhausted */
#define SEEK_ERR	35	/* Seek error:  check sysiocod value  */
#define READ_ERR	36	/* Read error:  check sysiocod error  */
#define WRITE_ERR	37	/* Write error: check sysiocod error */
#define	VRTO_ERR	38	/* Could not convert virtual open to actual */
#define FULL_ERR	39	/* No more records availble */
#define KSIZ_ERR	40	/* Index node size too large */
#define UDLK_ERR	41	/* Could not unlock data record */
#define DLOK_ERR	42	/* Could not obtain data record lock */
#define FVER_ERR	43	/* Version incompatibility */
#define OSRL_ERR	44	/* Data file serial number overflow */
#define KLEN_ERR	45	/* Key length exceeds MAXLEN parameter */
#define	FUSE_ERR	46	/* File number already in use */
#define FINT_ERR	47	/* c-tree has not been initialized */
#define FMOD_ERR	48	/* Operation incompatible with type of file */
#define	FSAV_ERR	49	/* Could not save file */
#define LNOD_ERR	50	/* Could not lock node */
#define UNOD_ERR	51	/* Could not unlock node */
#define KTYP_ERR	52	/* Variable length keys disabled OR invalid key type */
#define FTYP_ERR	53	/* File mode inconsistent with c-tree config  */
#define REDF_ERR	54	/* Attempt to write a read only file */
#define DLTF_ERR	55	/* File deletion failed */
#define DLTP_ERR	56	/* File must be opened exclusive for delete */
#define DADV_ERR	57	/* Proper lock is not held (CHECKLOCK/READ) */
#define KLOD_ERR	58	/* LOADKEY called with incorrect key number. 
				   You cannot continue */
#define KLOR_ERR	59	/* LOADKEY called with key out of order 
				   You may skip this key & continue */
#define KFRC_ERR	60	/* Percent out of range */
#define CTNL_ERR	61	/* NULL fcb detected during I/O */
#define LERR_ERR	62	/* File must be opened exclusively */
#define RSER_ERR	63	/* Start file / log file serial number error */
#define RLEN_ERR	64	/* Checkpoint past end of log file */
#define RMEM_ERR	65	/* Not enough memory during tran processing */
#define RCHK_ERR	66	/* Log file entry failed to find checkpoint */
#define RENF_ERR	67	/* Could not rename file */
#define LALC_ERR	68	/* Could not allocate memory for control list */
#define BNOD_ERR	69	/* Node does not belong to index */
#define TEXS_ERR	70	/* Transaction already pending */
#define TNON_ERR	71	/* No active transaction */
#define TSHD_ERR	72	/* No space for shadow buffer */
#define TLOG_ERR	73	/* LOGFIL encountered during shadow only */
#define TRAC_ERR	74	/* Recovery: two active tran for user */
#define TROW_ERR	75	/* Recovery: bad tran owner */
#define TBAD_ERR	76	/* Recovery: bad tran type */
#define TRNM_ERR	77	/* Recovery: file name too long */
#define TABN_ERR	78	/* Transaction abandoned: too many log extents 
				   or dynamic dump wait exhausted */
#define FLOG_ERR	79	/* Could not log file opn/cre/cls/del */
#define BKEY_ERR	80	/* NULL target or bad keyno */
#define ATRN_ERR	81	/* Transaction allocation error */
#define UALC_ERR	82	/* User allocation error */
#define IALC_ERR	83	/* ISAM allocation error */
#define MUSR_ERR	84	/* Maximum users exceeded */
#define LUPD_ERR	85	/* Reduce lock to read lock after update */
#define DEAD_ERR	86	/* Dead lock detected */
#define QIET_ERR	87	/* System not quiet: files in use */
#define LMEM_ERR	88	/* Linked list memory allocation error */
#define TMEM_ERR	89	/* Memory allocation during tran processing */
#define NQUE_ERR	90	/* Could not create queue */
#define QWRT_ERR	91	/* Queue write error */
#define QMRT_ERR	92	/* Queue memory error during write */
#define QRED_ERR	93	/* Queue read error */
#define PNDG_ERR	94	/* Pending error: cannot save or commit tran */
#define STSK_ERR	95	/* Could not start task */
#define LOPN_ERR	96	/* Start-file/log open error */
#define SUSR_ERR	97	/* Bad user handle */
#define BTMD_ERR	98	/* Bad transaction mode */
#define TTYP_ERR	99	/* Transaction type / filmod conflict */

#define	ICUR_ERR	100	/* No current record for isam datno */
#define	INOT_ERR	101	/* Could not find isam keyno request */
#define INOD_ERR	102	/* Could not open ISAM parameter file */
#define IGIN_ERR	103	/* Could not read first 5 parameters in ISAM 
				   parameter file */
#define IFIL_ERR	104	/* Too many files in ISAM parameter file */
#define IUND_ERR	105	/* Could noy undo ISAM update. Rebuild Files */
#define IDRI_ERR	106	/* Could not read data file record in ISAM 
				   parameter file */
#define IDRK_ERR	107	/* Too many keys for data file in ISAM 
				   parameter file */
#define IMKY_ERR	108	/* Incorrect keyno for index member in 
				   parameter file */
#define IKRS_ERR	109	/* Too many key segments defined in ISAM 
				   parameter file */
#define ISRC_ERR	110	/* Could not read segment record in ISAM 
				   parameter file */
#define	IKRI_ERR	111	/* Could not read index file record in ISAM 
				   parameter file */
#define IPND_ERR	112	/* LKISAM(ENABLE) found pending locks */
#define INOL_ERR	113	/* No memory for user lock table */
#define IRED_ERR	114	/* 1st byte of data record equals delete flag 
				   or bad variable length record mark */
#define ISLN_ERR	115	/* Key segments do not match key length */
#define IMOD_ERR	116	/* Bad mode parameter */
#define	IMRI_ERR	117	/* Could not read index member record */
#define SKEY_ERR	118	/* NXTSET called before FRSSET for keyno */
#define SKTY_ERR	119	/* FRSSET called for index with wrong keytyp */

#define RRLN_ERR	120	/* Data record length exceeds rebuild max */
#define KBUF_ERR	121	/* Tried to update data with ctISAMKBUFhdr on */
#define RMOD_ERR	122	/* Attempt to change fixed vs variable len */
#define	RVHD_ERR	123	/* Var length header has bad record mark */
#define INIX_ERR	124	/* # of indices does not match (OPNIFIL) */
#define IINT_ERR	125	/* c-tree already initialized */

#define ABDR_ERR	126	/* Bad directory path get */
#define ARQS_ERR	127	/* Could not send request */
#define ARSP_ERR	128	/* Could not receive answer */
#define NINT_ERR	129	/* c-tree not initialized */
#define AFNM_ERR	130	/* Null file name pointer in OPNFIL */
#define AFLN_ERR	131	/* File name length exceeds msg size */
#define ASPC_ERR	132	/* No room for application message buffer */
#define ASKY_ERR	133	/* Server is not active */
#define ASID_ERR	134	/* Could not get servers message id */
#define AAID_ERR	135	/* Could not allocate application id */
#define AMST_ERR	136	/* Could not get application msg status */
#define AMQZ_ERR	137	/* Could not set message appl msg size */
#define AMRD_ERR	138	/* Could not get rid of application msg */ 
#define ABNM_ERR	139	/* Badly formed file name */
#define VMAX_ERR	140	/* Variable record length too long */
#define AMSG_ERR	141	/* Required message size exceeds maximum */

#define SMXL_ERR	142	/* Application MAXLEN > server's MAXLEN */
#define SHND_ERR	143	/* Communications handler not installed */
#define QMEM_ERR	144	/* Application could not id output queue */
#define SCSF_ERR	145	/* Could not find COMM software */
#define	VDLK_ERR	146	/* Could not update free space info */
#define VDLFLG_ERR	147	/* Space to be reused is not marked deleted */
#define	VLEN_ERR	148	/* WRTVREC cannot fit record at recbyt */
#define	VRLN_ERR	149	/* Varlen less than minimum in ADDVREC */
#define SHUT_ERR	150	/* Server is shutting down */
#define STRN_ERR	151	/* Could not shut down. transactions pending */
#define LEXT_ERR	152	/* Could not extend logfile */
#define	VBSZ_ERR	153	/* Buffer too small */
#define	VRCL_ERR	154	/* Zero length record in REDVREC */
#define SYST_ERR	155	/* Native system failure */
#define NTIM_ERR	156	/* timeout error */
#define	VFLG_ERR	158	/* REDVREC record not marked active */
#define	VPNT_ERR	159	/* Zero recbyt value */
#define ITIM_ERR	160	/* Multi-user interference: index information
				   updated by the time user got to actual data
				   record */
#define SINA_ERR	161	/* User appears inactive */
#define SGON_ERR	162	/* Server has gone away */
#define SFRE_ERR	163	/* No more room in server lock table */
#define SFIL_ERR	164	/* File number out of range */
#define SNFB_ERR	165	/* No file control block available */
#define SNMC_ERR	166	/* No more ct file control blocks in server */
#define SRQS_ERR	167	/* Could not read request */
#define SRSP_ERR	168	/* Could not send answer */
#define TCRE_ERR	169	/* Create file already opened (in recovery) */

#define SFUN_ERR	170	/* Bad function number */
#define SMSG_ERR	171	/* Application msg size exceeds server size */
#define SSPC_ERR	172	/* Could not allocate server msg buffer */
#define SSKY_ERR	173	/* Could not identify server */
#define SSID_ERR	174	/* Could not get server message id */
#define SAMS_ERR	175	/* Server could not allocate user msg area */	
#define SMST_ERR	176	/* Could not get server msg status */
#define SMQZ_ERR	177	/* Could not set message server msg size */
#define SINM_ERR	178	/* Unexpected file# assigned to [si] in rcv */ 
#define SOUT_ERR	179	/* Server is at full user capacity */


#define IKRU_ERR	180	/* Could not read symbolic key name */
#define IKMU_ERR	181	/* Could not get mem for key symb name */
#define IKSR_ERR	182	/* No room for sort key. increase MAXFIL */
#define IDRU_ERR	183	/* Could not read file field number values */
#define ISDP_ERR	184	/* Attempt to reallocate set space */
#define ISAL_ERR	185	/* Not enough memory for addt'l sets-batches */
#define ISNM_ERR	186	/* Set number out of range */
#define IRBF_ERR	187	/* Null buffer in rtread.c */
#define ITBF_ERR	188	/* Null target buffer in rtread.c */
#define IJSK_ERR	189	/* Join_to skip */
#define IJER_ERR	190	/* Join_to error */
#define IJNL_ERR	191	/* Join_to null fill */
#define IDSK_ERR	192	/* Detail_for skip */
#define IDER_ERR	193	/* Detail_for error */
#define IDNL_ERR	194	/* Detail_for null fill */
#define IDMU_ERR	195	/* Could not get mem for dat symb name */
#define ITML_ERR	196	/* Exceeded RETRY_LIMIT in RTREAD.C */

#define IMEM_ERR	197	/* Could net get memory for ifil block */
#define BIFL_ERR	198	/* Improper ifil block */
#define NSCH_ERR	199	/* Schema not defined for data file */

#define RCRE_ERR	400	/* Resource already enabled */
#define RNON_ERR	401	/* Resources not enabled */
#define RXCL_ERR	402	/* File must be exclusive to enable res*/
#define RZRO_ERR	403	/* Empty resource id */
#define RBUF_ERR	404	/* Output buffer to small */
#define RDUP_ERR	405	/* Resource id already added */
#define RCSE_ERR	406	/* Bad resource search mode */
#define RRED_ERR	407	/* Attempt to get non-resource info */
#define RNOT_ERR	408	/* Resource not found */
#define LKEP_ERR	409	/* Not in use: available */
#define USTP_ERR	410	/* User not active */
#define BSUP_ERR	411	/* Not a superfile */
#define LCIP_ERR	412	/* WRL to WXL commit promote pending(CIL) */
#define SDIR_ERR	413	/* Superfile host not opened */
#define SNST_ERR	414	/* Cannot nest superfiles */
#define SADD_ERR	415	/* Illegal ADDKEY to superfile */
#define SDEL_ERR	416	/* Illegal DELBLD to superfile */
#define SPAG_ERR	417	/* Cache page size error */
#define SNAM_ERR	418	/* Max name inconsistency */
#define SRCV_ERR	419	/* Host superfile does not support recovery */
#define TPND_ERR	420	/* Key update with pending transaction */
#define BTFL_ERR	421	/* Filter not supported yet */
#define BTFN_ERR	422	/* Other functions not sup */
#define BTIC_ERR	423	/* Incomplete */
#define BTAD_ERR	424	/* Add list err */
#define BTIP_ERR	425	/* Batch in progress */
#define BTNO_ERR	426	/* No batch active */
#define BTST_ERR	427	/* Status info already returned */
#define BTMT_ERR	428	/* No more info, batch cancelled */
#define BTBZ_ERR	429	/* Bufsiz too small for record */
#define BTRQ_ERR	430	/* Request is empty or inconsistent */
#define LAGR_ERR	431	/* Aggregate/serialization lock denied */
#define FLEN_ERR	432	/* Fixed length string requires len in DODA */
#define SSCH_ERR	433	/* Segment def inconsistent with schema */
#define DLEN_ERR	434	/* Very long def block not supported */
#define FMEM_ERR	435	/* File def memory error */
#define DNUM_ERR	436	/* Bad def number */
#define DADR_ERR	437	/* defptr NULL during GETDEFBLK */
#define	DZRO_ERR	438	/* Requested def blk is empty */
#define DCNV_ERR	439	/* No conversion routine for Definition Block */
#define DDDM_ERR	440	/* Dynamic dump already in progress */
#define DMEM_ERR	441	/* No memory for dynamic dump file buffer */
#define DAVL_ERR	442	/* One or more files not available for dump */
#define DSIZ_ERR	443	/* File length discrepancy */
#define DCRE_ERR	444	/* Could not create file during dump rcv */
#define SDAT_ERR	445	/* Not enough data to assemble key value */
#define BMOD_ERR	446	/* Bad key segment mode */
#define BOWN_ERR	447	/* Only the file's owner can perform op */
#define DEFP_ERR	448	/* Permission to set file definition denied */
#define DADM_ERR	449	/* ADMIN has opened file. Cannot delete file */
#define LUID_ERR	450	/* Invalid user id */
#define LPWD_ERR	451	/* Invalid password */
#define LSRV_ERR	452	/* Server could not process user/acct info */
#define NSRV_ERR	453	/* No such server */
#define NSUP_ERR	454	/* Service not supported */
#define SGRP_ERR	455	/* User does not belong to group */
#define SACS_ERR	456	/* Group access denied */
#define SPWD_ERR	457	/* File password invalid */
#define SWRT_ERR	458	/* Write permission not granted */
#define SDLT_ERR	459	/* File delete permission denied */
#define SRES_ERR	460	/* Resource not enabled */
#define SPER_ERR	461	/* Bad permission flag */
#define SHDR_ERR	462	/* No directory found in superfile */
#define UQID_ERR	463	/* File id uniqueness error */
#define IISM_ERR	464	/* ISAM level logon not performed */

#define IINI_ERR	465	/* Incremental Index: dnumidx < 1 */
#define IIDT_ERR	466	/* Incremental Index: dfilno not a ISAM file */
#define IINM_ERR	467	/* Incremental Index: aidxnam NULL for 1st */
#define IITR_ERR	468	/* Incremental Index: active tran not allowed */
#define NGIO_ERR	469	/* Negative I/O request */
#define LGST_ERR	470	/* Guest logons disabled */

#define SORT_ERR	370	/* Sort base: errors SORT_ERR + 101 thru 126 
				   see CTSORT.C or CTERRC.H for error listing */

/* Sort errors range from 471 thru 496 (e.g.: SORT_ERR + 101 thru 126).
   These errors are returned from routines found in CTSORT.C.
   Error codes and short descriptions returned by these routines are as 
   follows:

471 = error deleting sortwork file
472 = error creating unique name
473 = error opening first dummy file
474 = too few handles available min 3
475 = error closing dummy file 
476 = error unlinking dummy file
477 = error getting first data area
478 = sinit phase not previously performed-srelease
479 = sreturn phase already started
480 = no records in data buffers
481 = sint phase not previously performed-sreturn
482 = not enough memory
483 = no valid record pointers in merge buffers
484 = error opening sortwork file
485 = error creating sortwork.00x file
486 = no records fit in output buffer
487 = error reading sortwork file
488 = bytes in buf <> merge buf size
489 = error adjusting file pointer
490 = error closing sortwork.00x
491 = error closing sortwork file
492 = error deleting sortwork file
493 = error renaming sortwork.00x
494 = error closing output file
495 = error creating output file
496 = insufficient disk space 
*/

#define NLOG_ERR		498 /* Old log file found during log create */
#define FIDD_ERR		499 /* Mismatch between recv log & file id */
#define SQLINIT_ERR		500 /* Server could not init SQL engine */
#define SQLCONNECT_ERR		501 /* Could not init SQL for a user */
#define SQL_REQUEST_ERROR	502 /* Could not access SQL master info */
#define SQL_INVALID_CONTINUE	503 /* Could not continue SQL request */
#define NSQL_ERR		504 /* Server does not support SQL */
#define USQL_ERR		505 /* User profile does not enable SQL */
#define SRFL_ERR		506 /* Could open save-restore file */
#define SRIO_ERR		507 /* Could not process save-restore file */
#define SRIN_ERR		508 /* Save restore inconsistency */
#define DSRV_ERR		509 /* Duplicate server */
#define RFCK_ERR		510 /* Active chkpnt at start of roll-forward */
#define ILOP_ERR		511 /* Index nodes form illegal loop: rebuild */
#define DLOP_ERR		512 /* Data file loop detected */
#define SBLF_ERR		513 /* FPUTFGET does not support CTSBLDX () */
#define CQUE_ERR		514 /* Queue has been closed */
#define OIFL_ERR		515 /* Cannot convert old IFIL structure */
#define GNUL_ERR		516 /* ctNOGLOBALS not allocated */
#define GNOT_ERR		517 /* 'regid' is not registered */
#define GEXS_ERR		518 /* 'regid' is already registered */
#define IEOF_ERR		519 /* index logical EOF error */
#define HTRN_ERR		520 /* Attempt to update index with 
				       inconsistent tran# */
				    
/*				521 - 526 reserved for BANYAN env */
#ifdef VINES
#define BMAL_ERR		521 /* Could not allocate memory for the 
				       streettalk login message buffer */
#define STID_ERR		522 /* Userid in INTISAM does not match 
				       current login id */
#endif /* ifdef VINES */

#define BIDX_ERR		527 /* index must be rebuilt:see CTSTATUS.FCS */
#define SLEN_ERR		528 /* key segment length error		  */
#define CHKP_ERR		529 /* system checkpoints terminated	  */
#define LMTC_ERR		530 /* client does not match server	  */
#define BREP_ERR		531 /* index reorg entry error		  */
#define ASAV_ERR		532 /* TRANSAV called with AUTOSAVE on	  */
#define MTRN_ERR		533 /* file header high-water-mark overflow*/
#define OTRN_ERR		534 /* transaction # overflow		  */
#define REGC_ERR		535 /* ctree not registered. Call REGCTREE*/
#define AREG_ERR		536 /* only automatic REGCTREEs allowed   */

#define PIOT_ERR		538 /* client-side bad function array type*/
#define BFIL_COD	       -539 /* sysiocod when file does not appear
				       to contain any valid information	  */

#define PNUL_ERR		540 /* null parameter			  */
#define LWRT_ERR		541 /* transaction log cannot be written  */
#define MCRE_ERR		542 /* could not create mirror file	  */
#define MOPN_ERR		543 /* could not open mirror file	  */
#define MCLS_ERR		544 /* could not close mirror file	  */
#define MDLT_ERR		545 /* could not delete mirror file	  */
#define MWRT_ERR		546 /* could not write to mirror file	  */
#define MSAV_ERR		547 /* could not save mirror file	  */
#define MRED_ERR		548 /* could not read from mirror	  */
#define MHDR_ERR		549 /* mismatch between mirror headers	  */
#define MSKP_ERR		550 /* attempt to open primary w/o mirror:*/
				    /* or'ing in a file mode of MIRROR_SKP*/
				    /* permits a primary to be opened w/o */
				    /* error				  */
#define MNOT_ERR		551 /* file already opened without mirror */

#define PREA_ERR		555 /* could not read primary, switching  */
#define PWRT_ERR		556 /* could not write primary, switching */
#define CWRT_ERR		557 /* could not write mirror,suspend mir */
#define PSAV_ERR		558 /* could not save primary, switching  */
#define CSAV_ERR		559 /* could not save mirror, suspend mir */

#define SMON_ERR		560 /* only one of each monitor at a time */
#define DDMP_BEG		561 /* SYSMON: dynamic dump begins	  */
#define DDMP_END		562 /* SYSMON: dynamic dump ends	  */
#define DDMP_ERR		563 /* SYSMON: dynamic dump ends (errors) */


/* *** At the end of automatic recovery, the following conditions     *** */
/* *** were detected which require cleanup before continuing. The     *** */
/* *** specifics are reported on in CTSTATUS.FCS:		      *** */

#define RCL1_ERR		570 /* incomplete compression		  */
#define RCL2_ERR		571 /* index rebuild required		  */
#define RCL3_ERR		572 /* incomplete compression & index re- */
				    /* build required			  */
#define RCL4_ERR		573 /* primary\mirror out-of-sync. Copy	  */
				    /* good file over bad.		  */
#define RCL5_ERR		574 /* incomplete compression & primary\  */
				    /* mirror out-of-sync		  */
#define RCL6_ERR		575 /* index rebuild required & primary\  */
				    /* mirror out-of-sync		  */
#define RCL7_ERR		576 /* incomplete compression & index re- */
				    /* build required & primary\mirror	  */
				    /* out-of-sync			  */

#define NCON_ERR		590 /* could not find ISAM context ID	  */
#define OCON_ERR		591 /* old context ID. Call CLSICON()	  */
#define ECON_ERR		592 /* context ID exists		  */

#define CLEN_ERR		595 /* varlen too small in PUTCRES	  */
#define CMIS_ERR		596 /* missing information 		  */
#define CINI_ERR		597 /* could not initialize expression	  */
#define CVAL_ERR		598 /* could not evalutate condtional exp */

#define CTHD_ERR		600 /* no more client threads		  */
#define VRFY_ERR		601 /* ctVERIFY detected problems with idx*/
#define CMEM_ERR		602 /* no memory for system lock table	  */

#define HNUL_ERR		610 /* CTHIST target==NULL		     */
#define HLOG_ERR		611 /* CTHIST could not access log	     */
#define HSTR_ERR		612 /* CTHIST must be called with ctHISTfirst*/
#define HONE_ERR		613 /* CTHIST can only access data or index  */
#define HMAP_ERR		614 /* no valid ISAM map from index to data  */
#define HIDX_ERR		615 /* cannot get index info from data filno */
#define HACT_ERR		616 /* CTHIST cannot be called during a tran */
#define HNOT_ERR		617 /* did not find target		     */
#define HENT_ERR		618 /* log scan terminated: EOF or bad entry */
#define HZRO_ERR		619 /* CTHIST on data file: recbyt==0	     */
#define HSIZ_ERR		620 /* bufsiz too small			     */
#define HTYP_ERR		621 /* transaction type not expected	     */
#define HMID_ERR		622 /* must reset CTHIST first		     */
#define HMEM_ERR		623 /* not enough memory for CTHIST	     */
#define HNET_ERR		624 /* net change only applies to specific
				       match of key or record position	     */
#define HMTC_ERR		625 /* must specify exactly one matching
				       criteria (user & node may be combined)*/ 
#define HUND_ERR		626 /* encountered an UNDTRAN going forward:
				       must completely restart this CTHIST
				       sequence				     */
#define HUNK_ERR		627 /* unknown type of request		     */
#define HFIL_ERR		628 /* must specify filno		     */
#define HTFL_ERR		629 /* could not initialize internal file ID */
#define HUNX_ERR		630 /* unexpected length in log entry	     */

#define NPLN_ERR		633 /* null plen (pointer to size)	  */
#define NLEN_ERR		634 /* negative length specified	  */
#define TSYC_ERR		635 /* could not create thread sync object*/
#define TSYF_ERR		636 /* thread sync object 'get' failed	  */
#define TSYR_ERR		637 /* thread sync object 'rel' failed	  */
#define TQUE_ERR		638 /* queue message truncated to fit	  */
#define TZRO_ERR		639 /* semaphore must be init with count>0*/
#define TINT_ERR		640 /* semaphore already initialized	  */
#define TSYX_ERR		641 /* thread sync object 'cls' failed	  */

/*				649    ... reserved ...			  */
#define DUPJ_ERR		650 /* duplicate keys rejected and logged */
#define DUPX_ERR		651 /* could not process dup key log	  */
#define MAPL_ERR		653 /* attempt to exceed mapped lock limit*/
#define TLNG_ERR		654 /* record length too long for log size*/

#endif /* ~ctERRCH */
/* end of cterrc.h */
