;rem This is the infamous config.dsp
;outpath         c:\data\
;outfilename     record.dat
ethernet        0xFFFFFFFF   ; ethernet address
freq            9240e6  ;stalo frequency (Hz)
velsign         1       ;use 1 if stalo < xmit, -1 if stalo > xmit
timingmode      1       ;0 = continuous, 1 = triggered, 2 = sync, 
delay           1       ; (no effect in mode 0) delay to first sample (special case: timingmode 2)
gates           950     ;
hits            10      ; number of pulses per beam
rcvr_pulsewidth 6       ;
xmit_pulsewidth 6       ; 
prt		6000	; 1000 Hz 
tpdelay         8       ; test pulse delay in 6MHz counts (line-up G0)
tpwidth         17      ; (6 X pulsewidth + 4) test pulse delay in 100nS counts
trigger         on
testpulse	off
gate0mode       off     ;high rate sampling of gate 0 (on or off)
phasecorrect    off     ;remove gate0 phase from all gates (for magnetron)
clutterfilter   0       ;clutter filter OFF
clutter_start	3       ;offset to linear test data ramp set by Piraq DSP Executable
clutter_end	10      ;multiplier for ? 
ts_start_gate	-1      ; (<0 disables)
startgate	123     ; 1st valid data gate
afcgain         1e5     ;use positive sign if high voltage = > 60MHz IF
locktime        10.0
debug           off      ;turns on certain debug printouts
dataformat      18      ; PIRAQ_CP2_TIMESERIES: CP2
meters_to_first_gate	100.0	; 1551
gate_spacing_meters	20.0	; 
pcitimermode    0       ; 0 for internal sync, 1 for external sync
fakeangles      1       ; 0 for no, 1 to generate fake angles instead of reading PMAC.


; for 48 MHz operation, the timing reference is based on a 6 MHz clock.
; This will be changed to the "normal" 8 MHz clock when the firmware is
; updated to divide by 6 or 8 -- now it only divides by 8. -- E.L. 12/02
