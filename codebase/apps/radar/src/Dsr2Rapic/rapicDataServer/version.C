#include <string.h>
#include <stdio.h>

/*
	version.no

	List of additions/bugs/fixes for 3D-Rapic


Version	Date	Description
-------	----	-----------
1.00	7/9/93	First release
	DisplaysPPI/RHI/PPIRHI/3DTops
	Comms	SockX28/X28/Socket
		SockX28 only used 
		Blocking comms. connect.
		Single comms line only
	DB	Leapfrog db implemented. Requires archive
		admin. No db to browse select mechanism
		Db browser fully functional.
	Bugs	Process size grows slowly. Will run for
		many days. Suggest restarting every week
		Load DEFAULT Layout - FocusWin bug 
		if seq stopped will probably crash
1.01	8/9/93	Fixed			FocusWin bug
		Additions	PPIRHI grid switch using 'g','G'
	9/9/93	Additions	rpcomm.ini file to set initial comms connect
	10/9/93 Addition	PrimVolComplete flag, incomplete volumes
			indicated by background of Tops
1.02	27/9/93	Diagnostics Database locking turned off.
1.03		interpolated ppi bug fixed, maxcells could overrun radial

1.04	25/11/93 16level dBZ defaults set to match tx
1.05	15/12/93 Named layouts implemented
		Different height palette
		Merged working/archive database access
1.06		Record locking fixed in db merge
		Merged db file descr fixed.
1.07		Problem switching dbs when max sz reached fixed.
		DB reset(close/open) if unable to write img. 
		Force DB Merge available in DB menus
1.09		Administrative error, I forgot to use version 1.08!!!
		Database copy facilities added.
		Source/Dest db selector added.
		Double click to load seq from db in browser.
1.10		Problem introduced in 1.09 with comms auto-reconnect fixed
1.11		Problem selecting both Src/Dest DB at once fixed.
		m (or M) key to toggle map overlay/underlay 
		Display links (toggled by l (for link) key) operate on full 
		3D point of view parameters, ie tops will follow PPIRHI 
		display etc.
		Environment variable RPDBSIZE added to set max size of
		realtime DBs in Megabytes.
		e.g. setenv RPDBSIZE 30	- sets the Max DB size to 30MB
1.12		IRIX5.2 version
1.13		Multiple concurrent comms. Multi thread comms. 
1.14		Multiple RPSrv Splits available
1.15		Comms requests, fixed <1000m ppi problem
1.16		VIL & CAPPI, MaxdBZ widget
1.17		Various comms fixes, maxrng on ppi indicator	
1.18		Velocity display added. PPI only.
1.19		Vel & VIL color palettes added
1.20		Bugs fixed. Pal Updates didn't resstore Win
		ppi/rhi close mismanaged links
		Still db create problem & splitter crash
1.21		db bug fixed
		Vel RHI done.
		Memory leak with data splitter.
1.22		More data splitter fixes.
1.23		Fixed read data file function.
1.24		Malaysia functionality substantially completed. 
Probably due for a new major revision number now
2.00		8bit single buffer mode color maps
2.02		Splitter operation improved, more reliable
2.03		Various fixes, Splitter query implemented, client asks
		for product, splitter only send matching scan
2.04		Dial-Backup fallback support for MMS
2.05		Try shutdown(fd,2) before socket close to avoid CLOSE_WAIT sockets
2.06		Multi-station merge from database browse and load
2.07		RHI color pallette bug fixed (could use 6lvl pal for 16lvl data)
2.08		cmap files support overlay color definition
2.09		New Rapic ctl panel with cursor data
2.11		Drag Seq and Always on Top option in RapicCtlPanel, 
		New LatLongHt uses float for Ht, upsets old .dcf files
		Sel Stn and SelDB Name uif destroyed on select
2.14		DB Browser could freeze whole user IF on NextDay call.
2.15		Font and line size setting via overlay dialog box
		Scan data text on image windows. Same size as map text, nearly white
2.16		Client/Server code overhauled. 
		txcomplete=0 option in device= string, demand in progress 
		scans from splitter
		use SO_KEEPALIVE on Listen mode sockets, disable on Connect
2.17		A number of changes made to increase comms reliabailty.
		Decreased heap fragmentation.
		AIFS IBM AIX port merged in
2.18 18/7/96	Splitter call-out mode implemented, simple splitter scan filters
		VILDBZLIMIT environment variable added. value limits max dBZ value used in VIL calculations
		e.g. setenv VILDBZLIMIT 55.0	- sets the Max dBZ to 55.0dBZ
2.19		RPDBREADONLY option added for "slaved" display.
		Slaved display will often load a sequence and receive live 
		data from a 3drapic but not store to a db
		RPSINGLETHREADCOMMS environment variable to run all comms in single thread
		The slaved operation wiould typically use the rpdb_files file to point to the
		NFS mounted path of the master db. 
		The rpdb_path should contain the string pathname=/pathtorpdbdirectory
		/pathtorpdbdirectory will contain the file rpdb_files, which determines the files
		to use as the main database.
2.20 26/8/96	rpdb_path EXTENSIONS
		pathname=/pname/ [READONLY] [RPDBSIZE=ss]
		text file can have multiple pathname= entries
		READONLY parameter added if db should not be written
		RPDBSIZE=nn to set the switching size for database
		first pathname entry used for initial db load. New scans written to all dbs (except READONLY)
		use # to comment out lines in rpdb_path
		RPDBREADONLY env var removed
2.21 18/9/96	TITAN port merged in
		Use of overlay planes(where available) for az ind, etc
		Geographically synchronised cursor: 'c' to toggle
2.22

2.24		
2.25		Added HEIGHTMAXIMUM=nn to default.cmap (nn in metres) Defines max height for height palette
2.26		USELOCKS set to OFF. NOT Multi-User DB safe. No-one using Multi-User DB as far as I know.
2.27 13/11/96	Added expect_ack to txdevice (splitter). If set it will expect
		rx device to acknowledge receipt of scan with scan details
		If not correct,  txdevice will disconnect,  reconnect,  retransmit				
2.28 20/11/96	txdevice queue length, if > maxqsize keep only one of each product
2.29 12/12/96	Fixed 3DPPIRHI product color palette occasionally wrong
3.02		Rapidapp (Builder Xcessory) user interface
		Additional txdevice functionality to support ASA TAAATS requirements
3.03		txdevice ackfail_tmout parameter added to rpcomm.ini		
3.04 4/4/97	
3.05 16/4/97	FIXED - Blank scans were being ignored, (not shown to scan clients)
		FIXED - New UIF Latest mode didn't work
3.06 17/4/97	send RPREFTIME and RPTXSTNSET if enabled
3.07 18/4/97	txdevice filterscan rejected all if no filters defined
3.08 23/4/97	3.07 bug on auto reconnect fixed
		***TEST USE OF SOCKET close with linger mode and timeout defined
		***instead of using shutdown (mode=0)
3.09 28/4/97	gethostbyname/addr still failing,  try single lock for both
		playaifc sets output rate, then restores if necessary
3.10 29/4/97	tx scan queue control flag setting fixed
3.11 30/4/97	recurrent ack mismatch failure removes tx scan after 3 failures
3.12 5/5/97	Distinguish btwn ack fail types,  mismatch,  nack & tmout
3.13 9/5/97	Ack processing fixed.
3.14 16/5/97	New merged database creation bug fixed.
3.15 20/5/97	Comm Mng init,  create all rx/tx devices before enbling
3.16 26/5/97	rpdb_path parameters added: DELALLOLD,  DBPURGEDEPTH=nn
3.17 30/5/97	Display config: saves ShowLatest mode
3.18 17/7/97	Database repair function fixed.
3.19 7/8/97	Rainfall accumulation product added		
3.20 12/9/97	Auto db merge if rec count mismatch removed. Usually due to duplicates
3.21 23/9/97	
3.22 25/9/97	New spinlock class from Ray's code
		rpcomm.ini.reload, rpcomm.ini.add mechanism,  can contain new config filename
		rplogflags support
		DB repair,  use file's basename for isam record filename
3.23 11/10/97	Added "rapicquitflag" file to close rapic function
3.24 20/10/97	Added dumpsched flag file. Dumps to dumpsched.txt
3.25 12/11/97	Adding GMS satellite functionality
3.26 12/11/97
3.27 25/11/97	Added GMS sat auto resampling
3.28 3/12/97	satdatamng created if satmanager.start flag file present
		Looks for flag files newgms.flag newasdagms.flag newcdatagms.flag
3.31 11/12/97	reading sat files detects type from filename, "gms" "asd" "cdata"
		Use only raw data from all files (at suitable offset)
		and read lines to fetch nav/cal data
3.33 16/12/97	Satellite manager working,  loading newgms.flag files
		and with windows specifying products.
3.34 17/12/97	mmap/unmap implemented
3.38 24/12/97	Stacks of gms display chnages
3.39 6/3/98	added Hayes modem dial support
		added initstring= option to rxdevice,  primarily for Hayes
		e.g.
		rxdevice=Hayes /dev/ttym2 \
		initstring=AT&F0&H3&R2&I2S0=0V1E0&B1S15=64
3.40 11/3/98	Started fixing GMS stuff for MMS
3.41 19/3/98	Problem with startline vs firstline vert offset fixed
		Sequence resampling fixed
		GMS file purging fixed
3.42 19/3/98	Fixed a GMS frame deletion bug
3.43 20/3/98	Added screen select option for dual head, 
		Fixed gross error in lat/long to line/elem nav
3.44 26/3/98	Adding satdatamanaged auto subset resamples
3.45 31/3/98	Added 32/64/160 level ASCII data support
3.47 7/4/98	rxdevice/commmng fixes (for aeolus1 Gambier problem) 
3.48 7/4/98	Added dumpconn flag file. Dumps to dumpconn.txt
3.49 12/5/98	default MergeMode is OFF, 
		Manual GIF/JPEG generation,  'g' key for GIF,  'j' for jpeg
		Auto GIF/JPEG generation. 'G' to toggle GIF,  'J' for JPEG
		In rpcomm.ini, radarstatusfile=filename to enable logging of status changes
3.51 5/6/98	Lots of GMS additions,  histogram win,  histogram eq,  highlight editor
		Grey scale option
3.52 11/6/98	Changed resample flag approach,  uses pointer to resample subset params
3.53 18/6/98	SVISSR munmap changed to close file,  allows external deletion
3.54 19/6/98	Further menu items suppressed in sat display mode
		Nav Sat projection fixed.
3.55 24/6/98	GMS File Viewer added
3.56 9/7/98	Full res. GMS vis,  change GMS proj maintains point of view
		Chg Channel,  applies current img only by default,  change ch all seq menu option
		Reset projection menu option
3.58 16/7/98	GMS,  bad files (CRC errors) weren't being purged
		Allow loading of bad CRC files. MAY BE RISKY,  NEED TO TEST. 
		svissr_constants.h #DEFINE SVISSR_ALLOW_BAD_CRC_DATA true
3.59 22/7/98	Blend function functional. 
		DON'T ALLOW BAD CRC GMS DATA,  IT WAS VERY DANGEROUS
3.60 27/7/98	Fixed GMS hist_eq color map mem leak
3.61 7/8/98	annotation support
3.62 18/8/98	Merge scale map added,  expects rapic.map to contain ASCII LONG LAT [text]
3.63 10/9/98	Further GMS annotation fixes,  incl IMAGETITLE color in default.cmap
		Some fixes for Rapic/Titan compile
3.64 29/9/98	circf namespace clash worked around, uses arcf instead,  there must be a better way
3.66 29/10/98	Major changes to satellite frame sequencing.
3.67 5/11/98	GMS line offset adjustment to prevent images jumping around
		Re-use all gms subset buffers to avoid massive fragmentation.
3.68 13/11/98	Radar TIMESTAMP: field support added, overrides TIME: DATE: 
3.69 18/11/98	GMS - NavSat projection fixed. Wasn't picking up ref grid in some cases
		It was also incorrectly modifying subsetparams in subsets before they were resampled
		Added "noglctext" flag file. If present,  GLC is suppressed. 
		Temp workaround for Irix6.5 crash on GLC
3.70 20/11/98	Annotation text - implemented per annot text size/rot
		Still a problem with menu panes on windows opened from work proc
		Added a opengmswin flag file. If present, a GMS win will be opened on startup
3.71 30/11/98	Fixed a couple of GMS sequencing problems
		Fixed handling of resample when file has been deleted
3.72 29/1/99	Fixed Y2k directory date listing problem
3.73 23/2/99	Fixed RPQUERY: scan type resolution, can function as PCRapic (V8.00l+)
		server.
3.74 30/3/99	16 level data interpreted as 6 level problem fixed.
3.75 7/4/99	Problem with > 16level doppler data level tables fixed
		Fixed: Manual GIF/JPEG image could crash if images/ directory not present
3.76 21/4/99	Added auto JPEG generation for GMS display
3.77 23/4/99	Added save/load gms screen layout
				Added GMS WIndow user defined size dialog
3.78 5/5/99	Increased rdrscan lock timeout from 5 to 10secs
		See if it helps problem in NSWRFC,  3drapic crashing
3.79 14/5/99	Fixed the ASCII .mp1 type map properties
3.80 18/5/99	Further ASCII mapfixes, added circle primitive to .mp1 files
		long lat @CIRC radius_km format
		e.g. 150.874 -34.264 @CIRC 50.0
3.81 27/6/99	Added debugging code to rdr_scan - will log error if deleting with Usercount != 0
3.83 31/6/99	Further debugging code to rdr_scan - will log error if deleting with Usercount != 0
3.84 31/6/99	Clean compile. 
		Fixed txdevice filter where stn = any,  but other filter specified
		This used to fail decoding,  and no filter resulted.
3.85 11/6/99	DB allow open of read-only databases. 
		Source will be opened readonly,  dest must be read-write
3.86 16/7/99	fixed ASCII maps in 3dtops window
3.87 7/9/99	Added titan output support
		Added volfilepath= and volfilestn= options to rpdb_path
		If volfilestn=-1, all radars will be written
3.88 27/10/99	Nexrad O/P support for multiple radars
    		rpcomm.ini rxdevice disabled_on_startup option added
3.89 11/11/99	Fixed problem querying for data,  string was not terminated with <CR> and queries didn't work	
3.90 12/11/99	Added rpdb_path volfilepath line option - filename_use_stnid - to specify stnids used in file names
3.92 17/11/99	Fixed - Serious bug in rxdevice, would crash on incomplete volumes - bad sprintf params
3.93 24/11/99	Fixed problem with Refl palette labelling. 
		Added option to Seq Spd/Mem uif etc to allow 5 minute update sequences.
3.94 29/11/99	Fixed another rxdevice bug,  would crash if no scans rx'd in a volume 
3.95 4/12/99	Fixed - MindBZ setting correctly retrieved for < 1dBZ
     8/12/99	Added XlatDevice - config via rdrxlat=src_stnid dest_stnid in rpcomm.ini
		Allows a scan (compppi only) to be translated from ths src_stnid to dest_stnid
		e.g. Used to translate Letterbox scan to Kurnell origin,  for Air Services
3.96 14/12/99	Added ignoreFutureData flag. Set by toggle menu in DIsplay menu
		or will be always be set if the file IgnoreFutureData.flag exists
3.97 23/12/99	Added Titan auto-start facility. Uses titan.stn, simply add a * after desired stn
3.98 15/1/00	Added replay mode support. Uses replay.ini to configure
3.99 15/1/00	Added Delete All Sequence option. Useful for remote replay.
4.00 17/1/00	Added NexRad tx pertilt delay,  defaults to 5 secs
4.01 14/3/00	Added reverttolatest mode,  if seq in realtime and seq is stopped,  will revert to lastest mode after 15minutes
		Fixed getnextradial multi-thread bug.
		rdrseq - linked display mode,  will only set pooint of view for same stn windows
4.02 23/3/00	Fixed rxdevice::newline bug accessing lastscan()-> crashed if lastscan was NULL
4.03 24/3/00	Trying different approach to angle decode in RLE_6L_radl, long shot to stop qldradar crashes
4.04 27/3/00	Changes to rxdevice header handling - Rdr in WF on semi-perm
4.05 28/3/00	Added fbtocommserver option to rxdevice. THis rxdevice will typically
		connect to primary 3drapic comms server, and keep sched and all other
		(non fbtocommserver) semi-perm rxdevices held off unless primary connection fails
4.06 5/4/00	Added delete of NexRadMgr to CloseVRC
4.07 5/4/00	Added dbpath/switchdb.flag polling. If flag exists,  del it and switch dbs
		For use in archiving. Allows new db per day, week etc. using cron trigger
4.08 19/4/00	possible rdr_scan_linebuff overflow bug fixed
4.09 25/5/00	Full recompile fixed problem reading older version *.dcf files
4.10 13/6/00	Fixed RHI cursor data readout error
		Extended use of ignoreFutureData: if defined, DBMng will igonore scans outside of a 6 hour date/time window
		Database loading progress indicator and Cancel option
		Seq Speed Depth panel Depth and Mem scales now reflect actual values 
		Database sequence loading through scan manager added
		    - still supports explicit scan clients as well
		rxdevice: scansettimeout=nn
		    If this is set the rxdevice will force a reconnect if a 
		    Scan set hasn't been received in nn seconds 
4.11 20/7/00	Incorporated a number of fixes to prevent comms restart crashing
		    particularly in txdevice.C. Also rdrxlat.C
		Added rpcomm.ini rxdevice&txdevice option - debug=0, 1, 2, 3 (debug levels)
		Fixed problem with rain_accum spinlock timeouts
		Added further detail to palette titles
4.12 2/8/00	Added group concept to communications.
		Typically for use with fallback comms. See rpcomm.ini.ref
		Added flag file create_dos_image_link (default is now OFF) to switch on dos dir path links
4.13 18/8/00	Added velocity filtering option, 
		 - enabled by flag file "filter_Vel_nn" where nn is stnid
4.14 31/8/00	Changed filtered scan creation to be on-demand not by global flag file
		Potential filtered scan user need to "ask" for them through the unfiltered
		 scan's rdr_scan::getFilteredScan call
4.15 4/9/00	Environment variable FILTER_MIN_NEIGHBOR added to set min neighbors for median filter
		e.g. setenv FILTER_MIN_NEIGHBOR 2
		Added more exhaustive testing of binary radials before decode
		**CPol** bin radial length count incorrectly includes length count bytes
		Added no_gif_flag_files flag file option
		Added no_gif_latest_link flag file option
4.16 31/10/00	Improved VIL display, stores calculated VIL scans for quicker response
		Added scanProduct linked list to rdr_scan - for filtered scans,  VIL,  CAPPI
4.17 15/12/00	Interactive RHI will centre on mouse cursor position
		RHI range grid resolution changes with zoom
4.18 18/1/01	Old style RHI centre RIGHT MOUSE ONLY
		New RHI centred on cursor with MIDDLE AND RIGHT mouse buttons
4.19 5/2/02	Added radar coverage support. Text coverage file in the coverage directory
4.20 8/2/01	Database error rebuilding from data file with /IMAGEEND: lines fixed
4.21 22/3/01	Fixed groupid and fbgroup function in comms
		Removed Nexrad client exit() calls
4.22 4/4/01	Support name rpdb.ini as well as rpdb_path for consistency
		rpdb.ini allows multiple rapic format file write entries
		volfilepath=/pathname/ [volfilestn=nn] [volfilesuffix=.suffix] 
			[filename_use_stnid] [create_latest_data_info] [volumeonly]
			[permissions=oooo] where oooo is octal permissions
			[owner=uid] [group=gid]
		Added rpdb.ini option "no_rapic_db" to disable rapic database
		Typically used for comms server,  converter type utilities
		Added rpdb.ini option "filter_bad_date" to prevent scans out of time window
		being added to window.
		Default window is 2 hours
		Added rpdb.ini option "bad_date_window" to set bad date window
		    bad_date_window=secs
		Added rpdb.ini option loop_delay=nn to set db polling period
		    loop_delay=secs
		Fixed all spinlock new calls - hsould pass wait time
		in 0.01secs i.e. (200) would set a 2 sec timeout
4.23 14/3/02    OpenGL implementation - still missing 3dtops & vil
                New map feature support - embedded directives for color, line thickness
                fontid, font size, font scale, font rotation, text grouping, lat/long order
		min/max render ranges
		FontIDS Stroked
		1-Times, 2 Courier, 3 Arial, 4 Techno, 5 Techno1, 6 Crystal
		FontIDS Bitmap
		7 Arbat, 8 Arial, 9 Brushtype, 10 Chicago, 11 Courier, 12 Cricket, 
		13 Crystal, 14 Fixedsys, 15 Gals, 16 Greek, 17 Impact, 18 Proun, 
		19 Techno, 20 Times
4.24 28/3/02    Improved Merged PPI panning, lat/long based.
4.27 6/5/02     3DTops ("t" key) & vil (<CTL-V> key) working. 
                Can use key switch or open window menu to get it
                JPEG image dump works
		GIF doesn't. Probably never will, 3drapic now uses RGB render, not color index
		RGB render not so suited to GIF, will look into using PNG for lossless image format
		Merged display improved - coverage diagrams overlaying pretty well
		Environment var RP_NO_FASTUPDATE can be defined to allow radar data render
		when pan/zoom merged image. Needs fairly fast graphics, e.g. NVidia PC
		First Titan rendering available Real time only. <Ctl> t to toggle
		<Ctl> r may be used to suppress radar data to make titan more clearly visible
		<Alt> t will toggle btwn 35dBZ/45dBZ Titan
4.28 17/5/02    Fixed 3dtops&vil product generation to update as further scans are received
                Fixed RHI centred on cursor, using right AND middle mouse buttons
4.29 14/6/02    Fixed potential velocity filter crash.
                Support uncorrected refl display - use z key to select
		<CTL> p to enable polygons per second display
		<CTL> f to enable frames per second display
4.30 26/6/02    OpenGL capabilities config via environment variables
                RP_OGL_USE_AUXBUFFS - by default no aux buffers are requested, set this if
		  overlay planes available, e.g. NVidia Quadro
		RP_OGL_NO_STENCILS - define if stencils not supported in OpenGL **
		RP_OGL_NO_ALPHA - define if alpha planes not supported in OpenGL **
		RP_OGL_NO_DBLBUFF - define if dbl buffering not supported in OpenGL **
		  ** try these options if following error occurs when running 3drapic
		  Error: GLwMDrawingArea: requested visual not supported
		  e.g. in tcsh, setenv RP_OGL_NO_ALPHA
		  unsetenv RP_OGL_NO_ALPHA to turn env variable off again
		  e.g. on some laptops running Mesa OGL, RP_OGL_NO_ALPHA is reqd.
4.31 3/7/02     Added signals to quit 3drapic - SIGTERM, SIGABRT, SIGQUIT
                Bug fix - Merged PPI didn't display radar data on startup, 
		lat/long pan on Merged PPI window startup fixed
		no_img_flag_files file existence will stop Auto-Render *.flag files being created
		no_img_latest_link existence will stop Auto-Render "latest" files being created
		Auto-JPEG Render fixed. Added code to ensure older images aren't linked to "latest"
4.34 25/7/02    CAPPI product - use 'c' key to activate, 'p' to return to PPI. 
                  Must define a CAPPI ht before a scan will be visible.
4.35 25/7/02    Added suppress Radar (<Ctl> r) timeout, currently hard wired as 10 minutes
4.36 25/7/02    New default rapicdb filename style yymmddhhmm_stnid, allows crono dir order
                New style names can be easily identified by presence of "_" in name
                rpdb.ini "oldstyle_rapicdb_names" will force old style names 
		Titan mode indicator in window title, "T35" or "T45"
		Clear indication if radar data suppressed.
4.37 14/8/02    Added text annotations to Titan overlay
4.38 19/9/02    DB Browser shows scan types with times
4.39 23/9/02    /RXTIME: error in printed year, was struct tm type yr since 1900
                apply bkwds compatible fix to print actual year but still handle years < 1900 properly
4.40 25/9/02    Fix txdevice listen socket error, listenfd & port not set
4.41 30/9/02    Fix bug with nested FBGroups, didn't disable past 1st level.
4.42 8/10/02    Overlay of all Titan on merged image
4.43 11/10/02   Try to detect and remove "embedded" MSSG: in radials. addchar_parsed
                **Need to use rxdevice_check_radls_for_mssg global declaration in rpcomm.ini to enable**
4.44 11/10/02   Log any embedded MSSG: detected in radials
4.45 11/10/02   Don't Log any embedded MSSG: detected in radials, just in case this may upset syslog & crash system (probably unfounded)
4.46 23/10/02   Merged PPI fixed depth buffer problem, improved lat/long grid
4.47 8/11/02    Fixed rxdevice vol scan bug where incomplete vols would get mixed into one scan set
                Fixed tops product where az angle_res != 1.0deg, i.e. for Gunn_Pt @1.5deg
4.48 13/11/02   Fixed get_radl_angl where az angle_res != 1deg. Previously caused problems with cursor readout & RHI
4.49 13/11/02   Fixed problem with failure to display "current" titan cell
4.50 13/11/02   Added titanSettings.ini (and titanSettings.ini.reload) support. See sample titanSettings.ini file for syntax
4.51 14/11/02   Numerous PPIRHI enhancements. Allow Refl RHI for Tops/VIL/Accum products
                Fixed RHI lat/long/height/rng cursor readout
4.52 25/11/02   Changed <W> key - Will write rapic file as appropriate for display in window, 
                Files will be written to rpdb_path location, filenames specify date/time & type etc.
4.53 28/11/02   Fixed cappi scan generation for az res != 1.0deg
5.00 20/02/03   SGI port alpha version
                Additional OGL Visual env options, use glxinfo to check what system supports
                RP_OGL_USE_AUXBUFFS - by default no aux buffers are requested, set this if
		  overlay planes available, e.g. NVidia Quadro
		RP_OGL_NO_STENCILS - define if stencils not supported in OpenGL **
		RP_OGL_NO_ALPHA - define if alpha planes not supported in OpenGL **
		RP_OGL_NO_DBLBUFF - define if dbl buffering not supported in OpenGL **
		RP_OGL_RGB_SIZE n - number of bits for r,g&b planes, defaults is 8 
		RP_OGL_ALPHA_SIZE n - number of alpha plane bits, defaults is 8
		RP_OGL_STENCIL_SIZE n - number ofd stencil planes bits, defaults is 1
		RP_OGL_DEPTH_SIZE n - depth bugger bits, default is 24
5.02 5/5/03     Added rng res reduction option for nexrad generation to allow WDSS operation with 250m res data
5.03 8/5/03     Added compscanonly  & filename_use_stndatepath options for volfilepath entries, to support visAD client
5.04 6/8/03     Extensive GUI work, added titan properties editor
5.05 18/8/03    Most of titan properties dialog implemented, still need to do min VIL
5.06 8/9/03     Implemented redundant rapicTitanClient support
5.07 17/9/03    Stn select dialog - stns sorted alphabetically
                Implementing drawTrack-drawTitan-drawWDSS class heirarchy
5.08 16/10/03   First implementation of WDSS properties, WDSS Data Table
                Double buffer DSTitan object to avoid titan render blocking on locks
5.09 5/11/03    Added fast duplicate scan filtering in scanmng using scan key map with 3hr purging
5.10 4/12/03    Added separate refl_rngres_reduce_factor & vel_rngres_reduce_factor in nexrad.ini
5.11 9/12/03    Added solid background for WDSS & Titan text when mouse button 1 down
                WDSS Data table time series added
5.12 17/12/03   Added WDSS Meso icons - meso & tvs
5.13 30/1/04    Added useScanRegistry and useScanUserList flag files for runtime rdrscan tracking
5.14 30/1/04    Added rp_zr_coeff.ini support to initialise the ZR values
                  Also checks for presence of rp_zr_coeff.ini.new every minute and loads new ZR values if present
5.15 3/2/04     Added writeDetailedScanReport flag file for hourly scan usage report to file "rp_scan_report.txt"
5.16 16/02/04   Added Fileld map support  - use the *Filled *NoFilled directives in ASCII map files. 
                Causes one-off delay when map loaded
		Added color ogl palettes to windows. Use <CTL><ALT>P to toggle palette on/off
		<CTL><ALT><SHIFT>P to toggle palette vert/horiz
5.17 20/02/2004 Fixed bug with tops array de-allocation/re-allocation
5.18 24/02/2004 Palette improvements - live value indicator
5.19 02/03/2004 Added facility=local[0-7] option as 3rd line to rplog.ini (or rplogflags)
5.20 02/03/2004 Log occurences of embedded MSSG: strings in rapic data
5.21 04/03/2004 Fixed bug loading PPI/RHI window pair
5.22 09/03/2004 Numerous GUI fixes
5.23 10/03/2004 More GUI fixes. Added PPI/VXSect/3D - centre 3DPPI/RHI on PPI cursor pos if 
                   Right/Middle Mouse held down
5.24 11/03/2004 Multiple doppler display fixes - including interp mode
5.25 12/03/2004 Fixed issue with ppirhi/3d labels - window name in constructor
5.26 19/03/2004 Major changes in VXSect (i.e. not RHI) Uses full radial resolution - 
                ***BETA -needs extensive testing - 
5.27 22/03/04   Can use <Alt>X to toggle substep mode or 
		noVxSectSubSteps flag file to initially set vxsect mode to old one value per radial mode
5.28 22/03/04   Fixes to cursor readout in VXSect modes
5.29 23/03/04   Added Nominal VXSect grids
5.30 8/04/04    Implemented texture mapping classes drawOGlLLTexture and a textured tile class
                Implemented ausdem40 class - Australian terrain - 
		    - use <Ctl><Alt>m to toggle through Terrain/Terrain3D/No Terrain modes
		    - with <Ctl><Shift>z for 3d view in PPIs
5.31 21/4/04    Implemented layer manager GUI & hot keys (<Ctl>1-9) and layer support in ASCII lat/long maps
5.32 23/4/04    Added raw RPFILTER option to rpcomm.ini - using *RPFILTER=.... will force the filter string to be used literally
                   Usually the RPFILTER is parsed then re-constructed
		Added new GUI menus to PPI/RHI window including layer mngr
5.33 18/5/04    Adding rainfields manager, servers etc. infrastructure
                Fixed Dual VXSect
5.34 24/05/04   Added RAPIC_MAX_SEQ_MEM	environment to allow setting of max_seq_mem value (in MBs)
5.35 24/05/04   Fixed titan interval mode for use in archive mode. Seems to work now, limited testing only so far.
5.36 18/06/14   Adding rainfields client infrastructure
5.37 22/06/04   Modified titan display - <Ctl><Shift>t - toggles drawFcstOnly - i.e. will not draw fwd cell locations from 
                current render time, only draw forecast tracks
5.38 28/0604    Fixed a couple of bugs which would cause cdrom demo 
                to fail when run from the readonly file system
5.39 8/07/04    Further rainfields work
                Fixed ref point ghosting - bug introduced with new palette
		Added ForceSingleBuffer flag file to force single buffer display for debugging
		Fixed flat shaded radial endpoint bug - sometimes drew extra cell
		Improved memdump.data ttal process size
5.40 23/7/04    Fixed bug rebuilding database if index missing - from V5.38
5.41 26/07/04   Added radial velocity display lockout mechanism
                  radlVelLocks(contains stns to lock 1 name per line or All)
                  radlVelLockAll(will lock out radlVel display for all radars)
5.42 6/08/04    Fixed recently introduced (> V5.30) bug decoding rapic data - datatype
5.43 11/08/04   Honor comment char (#) in radlVelLocks
5.44 26/08/04   Changes to rapicTitanClient to handle slow or unavailable titan 
                servers. Also new - writes rapicTitanClient.status. 
		refer rapicTitanClient.ini.ref 
5.45 30/08/04   Added smart polling to titan client based on last scan time and min interval
                Fixed cursor readout where radl rng was used instead of tangent plane range
5.46 31/08/04   Further enhancements to titan server polling.
5.47 31/08/04   Added drawSetting.suppressDopplerSymbols to wdssSettings.ini
                See runtime/wdssSettings.ini.ref for more details
5.48 31/08/04   Fixed wdss ProbOfSeverehail decoding from XML
5.49 31/08/04   Suspect jpeg file write crashes if no disk space
                Check for space before write and abort if inadequate
		Improved exit behaviour - was crashing when titan client stuck.
5.50 1/9/04     Suppress decode binary radial error messages - can be excessive to console
                Added quit call to spinlock class to force all get_lock waits to 
		fail immediately - helps to close app quickly
		Save terrain/allow3dview setting with display layout
5.51 1/9/04     Fixed VIL palette scaling
5.52 9/9/04     Further rainfields implementation - not there yet
5.53 10/9/04    Fixed exit crash with rainfields/soap server
                Started work on compressed storage for rainfields thresholded
		array using rapic ASCII RLE encoding of array 
5.54 13/9/04    Testing rainfields RLE encoding
                Fix scansettimeout - prevent spurious timeouts
5.55 21/9/04    Further progress rainfields implementation - now functional
                To enable rainfields, the rainfieldsClient.ini must be 
		configured - see rainfieldsClient.ini.ref
		The rainfields_status text file identifies available
		rainfields radars. 
		To display these select the radar in a PPI window then 
		choose rainfields display with the 'R' key
		i.e. <Shift> R - a sequence corresponding to the current seq 
		will be loaded from the server.
5.56 22/9/04    Further rainfields refinement/bug fixes
5.57 23/9/04    Still fixing rainfields sequence context switch, 
                e.g. load db seq, return to real time. 
		Fixed rfimg purge crash
5.58 24/9/04    Enhanced key stroke support 
                Left Arrow - Step Seq Bkwd
		Right Arrow - Step Seq Fwd
		Home - Oldest in seq
		End - Latest in Seq
		Up Arrow - Next Scan Up (in volumetric PPI)
		Down Arrow - Next Scan Down (in volumetric PPI)
5.59 6/10/04    Further refinements rainfields
                Hide doppler vel products in wdss table if drawSetting.suppressDopplerSymbols in wdssSettings.ini
		Titan 35/40/45dBZ thresholds supported - toggle display with <Alt> t - no GUI control yet
		Requires drawSetting.thresh1=35.0 etc in titanSettings.ini, and appropriate entries 
		in rapicTitanClient.ini
		
                Major release version for RNDSUP Milestone 3 handover to CCSB
5.59.1 7/10/04  Fixed bug - core dumps with PPI/RHI when stn not selected and no rapic scan in image
       8/10/04  Problem found where the WDSS cell id re-use limit changed on servers to 500
                Fix made to wdss implementation - new default is 500, actual value configurable
		in rapicWDSSClient.ini via e.g. max_cell_id=1000 at end of URL entry line
		Fix made to WDSS initial load - handle cell id re-use correctly when data loaded 
		in reverse as per 3drapic startup.
       12/10/04	Fixed bug - core dumps when PPI/RHI/3D opened
		Fixed bug - WDSS Data table not closing on 3drapic close - can stop 3drapic shutdown 
		and subsequent restart.
		Fixed WdssDataFormClass.C nullpointer dereference vulnerability.
5.59.2 13/10/04 Fixed bug drawing merged PPI in PPI/RHI window		
       14/10/04 Removed libqt from compile - not rerqd, tidies up licencing dependencies
                Removed GIF write code - not used - avoid any licencing issues
       15/10/04	Fixed potential null pointer de-reference in Titan DsMessage.cc, can cause core dump 
                (only seen once)
5.60   20/10/04 Fixed rainfields texture blending - rainfield areas are now solid
5.61   28/10/04 Added radar fault status tracking - writes to radar_fault.status
5.62   3/11/04  Found and fixed potential null pointer de-reference in rainfall accum.
5.63   4/11/04  Found and fixed null pointer vulnerability in displ.C, rainfields related
5.64   14/11/04 Further bug fixes, fixed ascii map filename bug
5.65   16/12/04 Initial support for two volumes per radar time period. Use <Alt> v to toggle
5.66   11/2/05  Added further memdump.data points, dump for each dispay state (dcf) reload
                Also added MemoryStatusDumpPeriod env variabe to set memdump period (seconds)
		Also SIGHUP (Ctl C) will cause memdump update
		Removed SIGTSTP (keyboard stop, i.e. Ctl Z) from caught signals
                In expbuff_freelist::GetNode - Handle lock fail scenario, add null dereference blocks.
5.68   11/03/05 Fixed bug where sockets connected and failed on read/write immediately.
                Caused increased CPU usage in some circumstances
5.69   29/03/05 Added useLastScanDList flag file option, also toggle via <Alt>D
                Does not currently apply to merged PPIs
5.70   6/04/05  New wdss event driven mode now working with RH9 libraries
                Added filtering of MSSG string in rapic data DBZ header lines.
                Delay deletion of rapicquit flag until shutdown complete and
                check for existence of rapicquit flag on startup and delete if true. Do the same
                for the Rapic data server.
                Added flag for nexrad.ini to disable production of future Nexrad Level2 data.
                Added the capability to write new Rapic files modified from input complete scansets.
5.71  1/5/05    Implemented ip name to address caching class
     10/5/05    Parse rainfields url hostname to dot notation ipaddress string, avoids name lookups
     16/06/05   Fixed rdrscan CountryCode/CountryID behaviour, fix product scan 
                stn id behaviour in Malaysia
		Added IMAGETITLESOLIDBG option in default/print.cmap - ensure image
		title always readable, but hides data uder title
5.72 22/06/05   Numerous fixes to rapic rainaccum - add support for display to 
                select accum product using <Alt>a to step through prods
		defined in accum manager gui
5.72.1 24/8/05  Added flightTrack support
                Fixed bug in rxdevice where TxComplete=0 behaviour always defined
		  THis broke connections where mutliple radars were being received (bug addedd 10/6/2005)
5.72.2 29/8/05  Fixed bug in velocity palette - not rendering extreme values correctly
                Improve cursor readout to show undefined values as Undef or N/A 
		Completed CAPPI Gui functionality to enable toggle btwn kft/km
		Also fixed some redraw issues introduced with display list scan redraws
		Flight track support of new format - also added flighttrack refresh hook in main loop
		Fixed bug with cursor readout on CAPPI displays - height was interpreted as elev
		Serialised WDSS createindex calls  - to check possible wdss thread issue at startup 
       8/9/05   Fixed merged Titan
                <Alt>t usage reverted to incrementing titan threshold
		<Clt><Alt> t for Titan Properties GUI
		<Ctl>V for VIL, (<Ctl>v opens new PPI/VxSect Window)
		titan.status and wdss.status include easy to read per URL summaries
		If Titan data is >60days old, print red T instead of huge days
		Fixed Titan Props GUI Fwd time
       9/9/05   Fixed Titan forecast render
                Fixed WDSS Table tab labels
		Toggling wdsss/titan will close/open associated props & table widgets
		Fixed WDSS Fcst only render mode
		Fixed WDSS graph update on seq step
		Workaround wdss table widget close - hide widget instead
      16/9/05   Fixed Comms status suppress behavious, add suppress options to alert dialogs
                Fixed wdss table flashing when cursor moved and no wdss cells 
		Added rapicDataServer recent data cache function 
		  Config through serveRecentDataMins=n option in rpdb.ini (&rpcomm.ini)
		Added 3drapic rpcomm.ini option sendrecentdatamins=n to allow
		  3drapic clients to request recent data from rapicDataServer
		Ignore # lines in flightTracks.ini
5.72.3 20/9/05  3DRapic & rapicDataServer implementation of getRecentData functioning
                In txdevice check for no tx scan before sending Status Acknowledge
5.72.4 28/9/05  Fixed up some issues with pushMatrix/popMatrix and pushAttrib/popAttrib
                  These were causing some problems viewing overlay planes on newer machines
		Implemented 3drapic loadfrom functionality to recent data loading based on latest 
		time in initial database load
       30/9/05  Fixed issue with dlists & push/popattribs for vectors, still a problem with text dlists
                  Currently default to not using dlists for text. 
		  The flag file enableTextDlists may be used to enable text dlists 
		  (reread every 10 secs)
		  Need to use with the enableGLMatrixCheck & enableGLErrorCheck to ensure no gl errors
		  Fixed a problem with map text scaling without dlists
5.72.5 6/10/05  Fixed a problem with 3DPPIRHI window ogl state. 
                Also fixed problem saving/restoring 3dPPIRHI window pair from dcf file.
		  Dcf files written by Versions before 5.72.5 with 3DPPIRHI windows do not
		  have correct window info 
		  New window layouts and dcf files to be created by closing any 3DPPIRHI windows
		  and opening new ones
5.72.6 17/10/05 titan "supports_interval" mode supported. Requires compatible Nowcast Server 
                  version to function correctly.
		To utilise this feature, add the option "supports_interval" to each URL which supports
		  this feature, or use the "supports_interval" option on its own line to enable
		  it for all subsequent URLs
		The file name titan.ini is now supported as well as rapicTitanClient.ini (too big!)
		The file name wdss.ini is now supported as well as rapicWDSSClient.ini (too big!)
5.72.7 21/10/05 Change Titan client to always use latest seq time to specify load date/time
                  Previously used loadlatest, which could future data if server somehow had it
		Added rapic data rx monitoring stats, use debuglevel=1 or 2 in rxdevice to enable
		  monitoring includes rx delay of scan and bytes/sec stats
5.72.8 24/10/05 Fixed bug initialising defaultMaxCellID for wdss tracks - 
                  - was previously generating bad track numbers in WDSS when ids re-used
		PPIRHI window jpeg dump will now produce 2 jpegs, one for PPI, one for RHI
       26/10/05 Fixed Titan Show All Fwd Data
                Fixed Write All Seq Jpegs (<Alt> J)
		Fixed WDSS Properties GUI to be more consistent with Titan Props
       28/10/05 Added option to rainfieldsClient.ini in server= entry - uses_rainrate 
                  Old rainfields servers rainfall prods < 1hr rainrate(mm/hr), >1hr rainaccum(mm)
                  New rainfields servers all prods are rainaccum(mm)
		  Need to use uses_rainrate option in server= entry for OLD servers ONLY
		  New servers are rainaccum(mm) for everything
5.72.9 4/11/05  Fix rainfields polling - previous versions would fail where products other 
                than 10mins on server
		Initial implementation of rainfields product selection, using <Alt>R to 
		step through products on an accumulation interval approach. 
		Up to 5 products, then return to first.
5.72.10 9/11/05  Further ehancements to rainfields display
                Added showBaseProdAsRainRate option in rainfieldsClient.ini
		  default is to show base rainfall product as base period accumulation(mm)
		  If showBaseProdAsRainRate is defined, base product will be scaled and
		  shown in mm/hr
	11/11/05 Added all map reload via <Ctl><Alt><Shift>M - allows map changes to be 
	           picked up without restarting 3DRapic
		 Fixed possible core dump with corrupted wdss cell data
	16/11/05 Added code to generate rainfields accums
	         Fix to rxdevice to record sratrtRxTime when header added to new scan
		 Fixed bug in NexRadStnHndlr redundant unlinking rdr_scan_nodes
		 before delete, results in bad links
	18/11/05 Fixed null pointer dereference if no accum mng, and accum prod selected
	         Fixed bad pointer dereference in wdss gui update, potential stale 
		 nearestCell pointer
        22/11/05 Fixed bug in NexRadStnHdlr - unlinking rdr_scan_nodes from list done twice
	23/11/05 New map strategy - more layer based, will try to load map layer for stn
	           and fall back to global map for layer if stn layer not defined
5.72.11	23/11/05 New map strategy working - 
                   default to not using text dlists - 
		   can turn on with enableTextDlists flag file
5.72.12 7/12/05  Numerous fixes to underlay/overlay. 
                   Render underlays before data, overlays after
		   Still issues with 3D Terrain blending, 
		   Some maps still visible "through" world on side views
		 Added options for rpmap.ini - useDefaultMapsByMap & useDefaultMapsByLayer
		   If ByMap, no global maps layers used if any layer found for a radar
		   If ByLayer, globals layers will be used whenever that layer is not found
		   for a radar
5.72.14 16/12/05 Default to no Group Text, still causes attrib stack overflow in some cases
                   Most map rendering issues now resolved.
		 Support per layer overlay fallback to global maps, can override with 
		   useDefaultMapsByMap in rpmap.ini - see rpmap.ini.ref
		 Crash bugs fixed, with Accum mode with no accum mgr, 
		   & wdss stale pointer crash
5.72.14 20/12/05 Fixes to CAPPI dialog box, corrected ht when cappi gui re-opened
                 Fix potential crash bug in wdss latestStormCells
5.72.15 24/01/06 Added PNG image file support. <Alt> j toggles PNG/JPEG mode in window
                 Option outputJPEG in display.ini will force default output back to JPEG
5.72.16 24/0106  Write replay time out to file when in replay mode
                 Enable adding entry e.g.
		 writeClockTimeFile=replayclocktime.out 
		 to replay.ini This may include path, but the path must exist
5.72.17 31/1/06  Implement absolute velocity palette - still default to scan nyquist vel pal
                 To enable abs vel pal insert the following into default.cmap
		 UseAbsVelPalette
		 AbsVelPalNyquist=70.0
		 Holding <Ctl> with left mouse will temporarily revert to scan nyquist vel pal 
5.72.18 7/2/06   Fixed scan nyquist palette range indication
                 Allow negative nyquist values in scans to switch twds/away
5.72.19 9/2/06   Added some WDSS data valid checks to try to avoid bad wdss data crashes
                 Enhanced highlighting of abs vel palette range 
5.72.20 13/2/06  Added Cursor relative radial velocity - uses <Ctl>Right Mouse
                 Will track cursor movement while in this mode
5.72.21 17/2/06  A bunch of GUI fixes from WREQ.
                 Added Storm Rel Winds gui, not yet wired in.
		 Reflectivity palette shows current minumum refl setting
5.72.22 2/3/06   Added Window Group support - 
                 Each window by default will display in all groups unless a group # is 
		 specified in which case it will display when its group is selected
		 or if the All Groups option (<Alt>0 is selected.
		 The Radar Window's Window Group GUI menu must be used to assign a window
		 to a group
		 The Main Window's Display: Windows Group: menu may be used to select
		 which window group to display or the key stroke <Alt>1-9 in a radar 
		 window will select a group 
		 <Alt>0 will show all windows.
		 Groups with no windows will not be selected.
		 All window group settings are saved with the Save Layout option
5.73.2 10/3/06     Start new cvs_head version - branch start at 5.72.20 then 5.72.23
                 Adding Radl Vel Tools display modes transient/fixed
		 Also make Cell mode keep last defined values, previous reset to zero
5.73.3 5/4/06    Changed default rxdevice null radial filtering - disable filtering
                 Previous filtering of null radials may be enable by 
		   allow_nullradl_filtering in rpcomm.ini
5.73.4 2/5/06    Fixed crash bug in wdss render of closest cell - possible dangling pointer
                 Tighter tests to ensure closestcell map matched render time
5.73.5 9/5/06    Fix ptr not initialised crash bug opening Radl Vel tools gui
5.73.6 10/5/06   Serialise rainfields server call to loadRFImg - suspect
                   non thread-safe behaviour. - CANCEL THIS, NO PROBLEM WAS PROVED
		 May enable serialisation with useGlobalRFLoadImgLock in
		 rainfields.ini
		 Frame around textures shaded for raised appearance
5.73.7 12/5/06   rpmap.ini - dem texture would fail if demTexturePath= 
                 didn't have trailing /. Fixed, will add / if not present
5.73.8 16/5/06   Rainfields real-time polling fixes, bug introduced in smart
                 polling
5.74.1 19/5/06   First implementation using vertex array radial rendering
                 Default is old style radials - 
		 <Ctl><Alt>V toggle use vertex arrays
		 <Ctl><Alt>i toggle vert array indices
		 <Ctl><Alt>I toggle VBOs
5.74.2 24/5/06   Added -D_PTHREAD and --enablethreads=posix options to
                 compiles and linking
		 Fixed VertexArray elevation rotation, apply AFTER az rotation
		 Added Since9am product detection (9am string in TAG) - 
		 print in Title and scan data
		 Implementation of vertex array rendering for PPIs in testing
		 Tests show multi-radial packed arrays fastest for low cell 
		 count scans
		 Non-packed indexed VAs fastest for hi cell count (>60k) scans 
                 Added drawSetting.drawFcstOnly & 
                   drawSetting.drawFcstOnlyLocked to wdssSettings.ini and
		   titanSettings.ini - training school reqt
		 Option disableVertexArrays to display.ini, use if problem
		 occurs with vertex array rendering
		 Added option demTextureRes=n to allow downsampling of 
		 dem textures
		 Added option demTextureSwap to enable byte swapping of dem
		 textures - seems to be reqd for textures from USGS 
		 Added option TERRAINHTINCR=10.0 to default.dcf
		 Allows terrain range > default 0-2550m where reqd.
		 Implemented out-of-boundingbox filtering of map text
		   as new global map text was badly hurting performance
5.74.4 18/7/06   Increased checking for closed windows in rdrseq
                 Previously could crash if win closed before initial 
		   seq load completed
5.74.5 30/8/06   Fixed Vertex Array Packed array rngres bug
                 Fixed integrated accum product define gui stn select
5.74.6 20/9/06   Implemented obsData rendering - requires latest.axf in pwd
                 Currently using wget_latest_axf script to poll & fetch
		 Also requires fileReaders.ini entry-refer fileReaders.ini.ref
		 <Ctl>o toggles obsData display
5.74.7 2/10/06   Added maxDisplayAge=90 option to obsDataSettings.ini
                 Don't render obs > maxDisplayAge older/newer
		 Integrated urlReqObj implementation - see fileReader.ini.ref
		 for url config format
5.74.8 17/10/06  First implementation of lightning from axf file urls
                 See fileReaders.ini.ref for config - <Ctl> l to toggle
		 This version is realtime only, i.e. doesn't load history
		 Implemented internal URL reader - 
		   external url fetch no longer required
		 Uses url= type syntax in fileReaders.ini to specify url
5.74.9 20/10/06  Ltning history load from lightning directory implemented
                 See fileReaders.ini.ref for config - 
		 Uses specialised ltningurl= entry to get index.txt and 
		 relevent files from ltning directory
		 loadTimePeriod=120   sets depth of history to load in minutes
		 Added stdWinSize and winScaleMinLimit to display.ini to
		 allow adjustment of text and symbol scaling
5.74.10 28/11/06 Using Rainfields V2 API
                 Added scaleFactorLimit to obsDataSettings.ini - applies
		   to both obs and lighning
5.74.11 8/12/06  Improved synchronised cursor - includes synchronised right 
                 mouse button cross section location synchronisation
5.74.12 14/12/06 Added maxVolID=n option to display.ini to allow <Alt> v
                 selection of more than 2 Volume ids
		 Added useCG option - starting to include CG programs
		 e.g. for unsigned byte color mapped textures
5.74.13 2/2/07   Added Cg fragment program for interpolated radar image
                 rendering - interpolates intensity and uses color palette
		 colors - previously interpolating rgb colors
		 Can turn Cg off using useCG=false in display.ini
		 Added feature to select radar for PPI/RHI display using
		 <Ctl> Right Mouse in merged PPI - applies to nearest 
		 PPI/RHI window. Prefers to display nearest volumetric
		 radar (<300km) if none will select nearest CompPPI
	8/2/07   Changes to rdr_scan to remove non thread safe traversal via 
                 member variable "thisscan"
5.74.14 28/2/07  Implemented wdss meso and tvs cell annotations 
5.74.15 21/3/07  Fixed RadlVelTools wdss cell vel scaling
                 Shifted obsData render temp top/left, DP bot/left, 
		 rain10min top/right rainsince9am bot/right
5.74.16 22/3/07  Fixed problem with RadlVelTool using nearest cell, wasn't selecting
                 cell correctly
5.74.17 4/5/07   Implemented wdss meso and tvs, and titan cell table list 
                   (not data table or time series graphs yet)
                 If new window opened from an existing win, the new win will 
		   have same radar selected
                 Added <Alt>PgUp/Dn to switch all wins with same radar selected
		 Added <Ctl>PgUp/Dn to switch all wins in same group
		 Added Cell Table Mode indication
5.74.18 23/5/07  Use <Ctl><Alt><Shift>R to toggle replay paused mode
                 Implemented dynamic s_radl buffer size to handle > 1024 bins
		 Implemented Obs & Lightning state indication in title bar
5.74.19 28/5/07  Added suppressTVS option to wdssSettings.ini
5.74.20 28/6/07  Added suppressDownburst - also show/DopplerSynbols/TVS/ etc
                 Can use <Ctl><Alt>w to reload wdss settings (also opens props)
		 supressTVS suppresses all TVS symbols and text
		 suppressDownburst suppresses all downburst symbols and text
		 Fixed wds table mode selection buttons
		 Fixed CAPPI undefined range (cone of silence) display
		 Added RHI "coverage" background
5.74.21 5/7/07   Changed last img in seq display to show scans from prev img
                 if none i latest for that radar yet
5.74.22 17/7/07  Changed rainfiels max_seq_load_time implementation to enforce
                 the limit more effectively. 
5.74.23 10/10/07 Change NEXRAD RRR_MAX to RRR_PWR  SD
5.74.24 16/11/07 Fixed RHI crash bug
                 Fixes to VxSect - much improved rendering
                 Disable ObsAXF freelist - seems to cause crash occasionally
5.74.26 18/2/08  Changes for CP2 - 
                 Handle unknown no of scans: 
		 use PASS: n of 0 when scan no unknown
		 Introduced END SCAN SET stnname [vollabel] at end of scan set
		 rdrscan & ogldisplrdrppi for RHI display
		 Changed rp_isam isam offsets to unsigned long should
		 allow 4GB databases instead of 2GB limit
		 
*/	         
static char const cvsid[] = "$Id: version.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";

int	VersionNo = 574;
int	VersionBranch = 26;
char    VersionString[32] = "";

char* versionStr()
{
  if (!strlen(VersionString))
    {
      if (!VersionBranch)
	sprintf(VersionString,"V%4.2f", VersionNo/100.);
      else
	sprintf(VersionString,"V%4.2f.%1d", VersionNo/100., VersionBranch);
    }
  return VersionString;
}
