; IHOP version 

;m170 points to the AZ position source
;m270 points to the EL position source
;Az sources D:$0028 commanded   d:$002B  actual
;EL sources D:$0064 commanded   d:$0067  actual

;p49=1234 for using commanded angles as position source
;otherwise actual angles are used
;This is DANGEROUS!!!

;p48=1234 turn ON wierd high wind dealer with.

#define imhere p47      ;accuracy in degrees for start of scan.
#define imstopped p46   ;counts/ms for slow moving antenna

; p variable usage map


; rtdisplay uses p10->p30
; rtdisplay runs as plc 10

; send_ang uses p300 as a loop counter
; send_ang runs as plc 0

; master uses p302 as a loop counter
; master runs as plc20
; master uses p50 as the command input 

; stow uses p51 and p52 as the stow location
; stow runs as prog 200

; p53 is used to set the real time clock in the PMAC
; p54 contains the modulo trigger in min past the hour
; p55 contains decimal minutes past the modulo trigger
; p56 >0 enables waiting for sync trigger

; sun.pmc used p51 for sunaz and p52 for sunel
; sun.pmc uses prog 210 and 211 do &1b210r and &2b211r
; set sunflag=0 for normal =1 for az disp and 2 for el disp
#define rtsyncm  p54               ;modulo minutes past the hour for clock sync
#define rtsync   p55               ;minutes past the modulo for clock sync
#define syncon   p56               ;enables pausing for sync
#define clockrate p57              ;;counts/sec of PMAC clock M0
#define debug p58						 
#define azacmax p100               ;az max acceleration deg/sec^2 from I variables
#define aziaz   p101               ;initial azimuth 
#define azioff  p102               ;startup offset    
#define azits   p103               ;startup TS
#define azita   p104               ;startup TA
#define azk     p105               ;slope of acceleration ramp    
#define azl     p106               ;azimuth left
#define azmid   p107               ;sweep mid point  
#define azmidl  p108	           ;Left of midpoint
#define azmidr  p109		   ;Right of midpoint
#define aznow   p110		   ; Az right now deg.
#define azpnt   p111
#define azpntl  p112
#define azpntr  p113
#define azr     p114               ;azimuth right
#define azstep  p115		   ;Az step size RHI
#define azta    p116               ;az TA
#define aztam   p117
#define aztlm   p118
#define aztm    p119
#define aztoff  p120               ;azimuth turnaround offset  
#define azts    p121               ;az TS
#define azts0   p122               ;az TS0       
#define azts03  p123               ;azTS0^3
#define aztt    p124               ;azimuth turnaround time  PPI
#define azttt   p125		   ;Az transition time
#define azv     p126               ;az velocity deg/sec
#define azvmax  p127               ;az max velocity
#define azxmv   p128               ;x distance to acheive maximum velocity
#define azxts   p129               ;ts for maximum performance
#define done    p130               ;done with volume? >0=yes
#define elacmax p131               ;elevation max acceleration deg/sec^2 from I variables
#define elbot   p132               ;elevation bottom
#define eliel   p133		   ;RHI starting elevation
#define elioff  p134
#define elita   p135               ;elevation initial TA
#define elits   p136               ;elevation initial TS
#define elk     p137               ;elevation k
#define elmid   P138		   ;RHI midpoint
#define elnow   p139               ;elevation right now  deg.
#define elpnt   p140               ;elpoint immediately
#define elpntl  p141               ;elpoint right limit triggered
#define elpntr  p142               ;elpoint left limit triggered
#define elstep  p143               ;elevation step 
#define elta    p144               ;el TA  
#define eltam   p145               ;TA to max velocity 
#define eltm    p146               ;elevation TM
#define eltlm   p147               ;TL to max velocity
#define eltoff  p148		   ;Elevation turnaround offset RHI
#define eltop   p149               ;elevation top
#define elts    p150               ;el TS
#define eltt    p151               ;elevation turnaround time
#define elttt   p152               ;elevation transition time

#define elts0   p153        ;TS for max velocity
#define elts03  p154        ;elts0^3
#define elxmv   p155        ;x to accelerate to max velocity
#define elv     p156
#define elvmax  p157        ;max velocity deg/sec
#define elxts   p158        ;TS for max velocity
#define fixed_ang p159
#define last_ang p160
#define pfixang p161        ;fixed angle pointer
#define temp1   p162        ;temporary variable 
#define temp2   p163        ;temporary variable

#define eltamx  p164        ;el ta for max perf
#define eltsmx  p165        ;el ts for max perf
#define sunexa   p166
#define sunexe   p167
#define sunflag  p168      ;=0 for normal =1 for az expand =2 el expand
#define tap      p169       ; for turnaround calculations
;the following are control parameters
#define nazr    p170
#define nazl    p171
#define nazv    p172
#define nelbot  p173
#define neltop  p174
#define nelv    p175
#define nazstp  p176
#define swpcnt  p177        ;sweep count
#define volcnt  p178        ;volume count
#define scan_type p179
#define volpnt  p180        ;pointer to which source volume to execute
#define running p181        ;scan running flag 1=yes 0=no
#define scanid  p182
#define run1    p183
#define clock0  p185
#define clock   p186
#define repeet  p187        ;sequence run command for sequence repeats
#define pig     p188
#define plc30wait p189	;KG added 20080219
#define fatab   200         ;2nd fixed angle table this is the final
                            ;destination for fixed angles


; the following is the volume definition template These are offsets.
#define vid     0               ;volume ID #
#define vtype   1               ;volume type 1=ppi 2=rhi 3=sur 4=vert 5= syncronization
#define vmodulo 2		;sychronization modulo min past the hour
#define vfirst  2              ; 1=right or bot; 2=left or top
#define vazr    3               ;az right (deg)
#define vazl    4               ;az left (deg)
#define velbot  5               ;elevation bottom (deg)
#define veltop  6               ;elevation top (deg)
#define vrate   7               ;scan rate (deg/sec)
#define vmode   8               ;dual pol=1, single pol=2
#define vprf    9               ;Hz
#define vsamp   10              ;Integration samples (Hits/Beam)
#define vdel    11              ;starting delay (ms)
#define vazoff  12              ; az offset (deg)
#define veloff  13              ; elevation offset (deg)
#define vnext   14              ;pointer to next to run volume frame
#define vtab    15              ;pointer fixed angle table,end with -99


; "P" locations of the volume and sequence headers
; This table must also exist in frmMain Module1.bas
;uses p400 to p639
#define Ppi1   400
#define Ppi2   416
#define Ppi3   432
#define Ppi4   448

#define Rhi1   464
#define Rhi2   480
#define Rhi3   496

#define Sur1   512
#define Sur2   528
#define Sur3   544

#define Seq1   560
#define Seq2   570
#define Seq3   580
#define Seq4   590
#define Seq5   600
#define Seq6   610
#define seq7   620
#define seq10  630
;next available=640


;Fixed angel lists 
;This table must also exist in frmMain Module1.bas
;uses p640 to p969
#define Afl1	640
#define Afl2	673
#define Afl3	706
#define Afl4	739
#define Afl5	772
#define Afl6	805
#define AFl7	838
#define AFl8	871
#define AFl9	904
#define Afl10	937
;next available = 970

;#define vsync1=970             ;dummy sychronization volume
;#define vsync2=973             ;dummy sychronization volume
;#define vsync3=976             ;dummy sychronization volume
;#define vsync4=979             ;dummy sychronization volume
;#define vsync5=982             ;dummy sychronization volume
;#define vsync6=985             ;dummy sychronization volume
;#define vsync7=988             ;dummy sychronization volume

;next available 993





