%**************************************************************************
%
% Deutscher Wetterdienst, Verfahrensentwicklung in der Radarmeteorologie
%
% ReadGamicIQ.m Module: read GAMIC - IQ - TimeSeries Datasets
%
% 2012-12-24, pt, init
%
%**************************************************************************
function [ data RawPulse ] = ReadGamicIQ( GamicIQ_Filename )
%ReadGamicIQ Reading GAMIC IQ-Dataset
%   Reading GAMIC IQ-Dataset according to GAMIC e3p description

%**************************************************************************
%CLEAN ENTRY
%**************************************************************************

%**************************************************************************
%**************************************************************************
%tic
%**************************************************************************
%**************************************************************************

%**************************************************************************
%GET FILE ID AND OPEN
[fileID, error_message] = fopen(GamicIQ_Filename, 'r', 'ieee-le');
if ~strcmp(error_message,'')
    ferror = -1;
    error('GamicIQ:ReadGamicIQ:fopen','%d: unable to open file: %s ... ERROR CODE: %s',ferror,GamicIQ_Filename,error_message);
end
%**************************************************************************

%**************************************************************************
%PREPARE DEFAULTS
E3P_Defaults.E3_RANGE_RES_METERS = 25;
E3P_Defaults.MAX_IQ_PULSE_BINS = 8000;
E3P_Defaults.MAX_ACF_PULSE_BINS = 8000;
E3P_Defaults.MAX_BURST_BINS = 512;
E3P_Defaults.MAX_DFT_BINS = 256;
E3P_Defaults.MIN_PULSE_IN_RAY = 8;
E3P_Defaults.MAX_PULSE_IN_RAY = 1024;
E3P_Defaults.Enigma3RangeResolution = double(E3P_Defaults.E3_RANGE_RES_METERS)/1000.0;
E3P_Defaults.ALIGNTO = 4096;
%**************************************************************************
%print output to screen
%str = sprintf('E3P DEFAULTS'); disp(str);
%str = sprintf('E3P DEFAULTS - E3_RANGE_RES_METERS   : %d', double(E3P_Defaults.E3_RANGE_RES_METERS)); disp(str);
%str = sprintf('E3P DEFAULTS - MAX_IQ_PULSE_BINS     : %d', double(E3P_Defaults.MAX_IQ_PULSE_BINS)); disp(str);
%str = sprintf('E3P DEFAULTS - MAX_ACF_PULSE_BINS    : %d', double(E3P_Defaults.MAX_ACF_PULSE_BINS)); disp(str);
%str = sprintf('E3P DEFAULTS - MAX_BURST_BINS        : %d', double(E3P_Defaults.MAX_BURST_BINS)); disp(str);
%str = sprintf('E3P DEFAULTS - MAX_DFT_BINS          : %d', double(E3P_Defaults.MAX_DFT_BINS)); disp(str);
%str = sprintf('E3P DEFAULTS - MIN_PULSE_IN_RAY      : %d', double(E3P_Defaults.MIN_PULSE_IN_RAY)); disp(str);
%str = sprintf('E3P DEFAULTS - MAX_PULSE_IN_RAY      : %d', double(E3P_Defaults.MAX_PULSE_IN_RAY)); disp(str);
%str = sprintf('E3P DEFAULTS - Enigma3RangeResolution: %8.6f', double(E3P_Defaults.Enigma3RangeResolution)); disp(str);
%str = sprintf('E3P DEFAULTS - ALIGNTO               : %d', double(E3P_Defaults.ALIGNTO)); disp(str);
%str = sprintf(' '); disp(str);
%**************************************************************************

%**************************************************************************
%GET BINARY FILE SIZE IN BYTES
fseek(fileID,0,'eof');
nbytes=ftell(fileID);
fseek(fileID,0,'bof');
%**************************************************************************
%check file size ... filesize needs to be n*ALIGNTO where n is an integer
tolerance_single = eps('single');
n_ALIGNTOS = floor(nbytes / double(E3P_Defaults.ALIGNTO));
m_ALIGNTOS = (nbytes / double(E3P_Defaults.ALIGNTO)) - n_ALIGNTOS;
if ~(abs(m_ALIGNTOS) <= tolerance_single)
    ferror = -1;
    fclose(fileID);
    error('GamicIQ:ReadGamicIQ:sizecheck','%d: file has wrong size: %s ...',ferror,GamicIQ_Filename);
end
%**************************************************************************

%**************************************************************************
%estimate number of pulses: estimate the worst case: n_ALIGNTOS represents
%the actual number of pulses (because each single pulse will be padded to
%fit exactly into n * ALIGNTO (where n is an integer). So n=1 represents
%the worst case. Unfortunatly we do not know the exact size of a pulse and,
%in addition to that, the pulse size may change within the file. The 
%resulting array may be very large. So we try to loop over the file and
%count the pulses inside the file. This should be a relat. fast solution.
position1 = 0;
pulses_in_file = 0;
while position1 < nbytes
    try %multiple reads and fseek ... better use try&catch
        actpos  = ftell(fileID);
        command = fread(fileID, 1, '*uint32'); %0 "12"
        if (command ~= 12) % this would be an error
            error('GamicIQ:ReadGamicIQ:pulseinfile:pulsehead','Pulsehead check failed');
        end
        length  = fread(fileID, 1, '*uint32') + 2;   %1 "lenght without padding"
        padsize = (E3P_Defaults.ALIGNTO - mod(length*4,E3P_Defaults.ALIGNTO));
        fseek(fileID,actpos+length*4+padsize,'bof'); %move to end of pulse
        pulses_in_file = pulses_in_file + 1;         %count the pulse
        position1 = ftell(fileID);                   %loop check
    catch err %catch the error and re-emit
        error('GamicIQ:ReadGamicIQ:pulseinfile','Failed with id: %s and message: %s for: %s.',err.identifier, err.message, GamicIQ_Filename);
    end
end
fseek(fileID,0,'bof'); % rewind %
%**************************************************************************

%**************************************************************************
%prepare pulse structure container (dstruct)
RawPulse = struct(    ...
              'pulse_loop_counter',{},    ...
                  'pulses_in_file',{},    ...
                         'command',{},    ...
                          'length',{},    ...
                        'acqModeA',{},    ...
                        'acqModeB',{},    ...
                        'numBurst',{},    ...
                     'offsetBurst',{},    ...
                        'numHorIQ',{},    ...
                     'offsetHorIQ',{},    ...
                        'numVerIQ',{},    ...
                     'offsetVerIQ',{},    ...
                       'numDualIQ',{},    ...
                    'offsetDualIQ',{},    ...
             'reserve_num_offsetA',{},    ...
             'reserve_num_offsetB',{},    ...
                        'reserve1',{},    ...
                          'aziTag',{},    ...
                          'eleTag',{},    ...
                       'aziTagDeg',{},    ...
                       'eleTagDeg',{},    ...
                        'aziSpeed',{},    ...
                        'eleSpeed',{},    ...
                   'angleSyncFlag',{},    ...
             'angleSyncPulseCount',{},    ...
                    'pulseCounter',{},    ...
                         'highPrf',{},    ...
                          'lowPrf',{},    ...
                            'prf3',{},    ...
                            'prf4',{},    ...
                    'prfIndicator',{},    ...
               'rangeResolutionIQ',{},    ...
          'numberOfValidRangeBins',{},    ...
                      'timestampS',{},    ...
                     'timestampUS',{},    ...
                  'meanBurstPower',{},    ...
                      'burstPower',{},    ...
                   'meanBurstFreq',{},    ...
                      'burstPhase',{},    ...
                  'lastBurstPhase',{},    ...
                  'IFSamplingFreq',{},    ...
    'converterPhaseDiffHorizontal',{},    ...
      'converterPhaseDiffVertical',{},    ...
    'converterPowerDiffHorizontal',{},    ...
      'converterPowerDiffVertical',{},    ...
                        'reserve2',{},    ...
                       'afcOffset',{},    ...
                         'afcMode',{},    ...
                      'errorFlags',{},    ...
                  'adcTemperature',{},    ...
                   'ifdPowerFlags',{},    ...
                        'reserve3',{},    ...
                        'headerID',{},    ...
               'adcHLMaxAmplitude',{},    ...
               'adcHHMaxAmplitude',{},    ...
               'adcVLMaxAmplitude',{},    ...
               'adcVHMaxAmplitude',{},    ...
                'adcOverflowFlags',{},    ...
              'pciDMATransferRate',{},    ...
            'pciDMAFIFOFillDegree',{},    ...
                        'reserve4',{},    ...
           'errorConditionCounter',{},    ...
                  'errorCondition',{},    ...
                        'reserve5',{},    ...
              'dspFirmwareVersion',{},    ...
          'fpgaPciFirmwareVersion',{},    ...
          'fpgaIfdFirmwareVersion',{},    ...
                       'reserve6a',{},    ...
                        'reserve6',{},    ...
                        'checksum',{},    ...
                 'radarWavelength',{},    ...
             'horizontalBeamwidth',{},    ...
               'verticalBeamwidth',{},    ...
                         'pwIndex',{},    ...
                     'noisePowerH',{},    ...
                     'noisePowerV',{},    ...
                           'dbz0H',{},    ...
                           'dbz0V',{},    ...
                           'dbz0C',{},    ...
                       'zdrOffset',{},    ...
                       'ldrOffset',{},    ...
                     'phidpOffset',{},    ...
                        'sweepUID',{},    ...
                        'reserve7',{},    ...
                              'Ih',{},    ...
                              'Qh',{},    ...
                              'Iv',{},    ...
                              'Qv',{},    ...
                          'Ih_PAD',{},    ...
                          'Qh_PAD',{},    ...
                          'Iv_PAD',{},    ...
                          'Qv_PAD',{},    ...
                       'position1',{});
%**************************************************************************
%create pulse container
RawPulse(pulses_in_file).pulse_loop_counter = pulses_in_file;
%**************************************************************************

%**************************************************************************
%init loop
position1 = 0;
pulse_loop_counter = 1;
%**************************************************************************
%loop over the file
while position1 < nbytes
    %**********************************************************************
    try %multiple reads ... better use try&catch
        %******************************************************************
        RawPulse(pulse_loop_counter).pulse_loop_counter = pulse_loop_counter; %starting at 1
        RawPulse(pulse_loop_counter).pulses_in_file = pulses_in_file;         %done at pulses_in_file
        %******************************************************************
        RawPulse(pulse_loop_counter).command = fread(fileID, 1, '*uint32'); %0
        if (RawPulse(pulse_loop_counter).command ~= 12)
            error('GamicIQ:ReadGamicIQ:readpulses:pulsehead','Pulsehead check failed');
        end
        %******************************************************************
        RawPulse(pulse_loop_counter).length = fread(fileID, 1, '*uint32');       %1
        %******************************************************************
        RawPulse(pulse_loop_counter).acqModeA = fread(fileID, 1, '*uint32');     %2
        RawPulse(pulse_loop_counter).acqModeB = fread(fileID, 1, '*uint32');     %3
        %******************************************************************
        RawPulse(pulse_loop_counter).numBurst = fread(fileID, 1, '*uint32');     %4
        RawPulse(pulse_loop_counter).offsetBurst = fread(fileID, 1, '*uint32');  %5
        RawPulse(pulse_loop_counter).numHorIQ = fread(fileID, 1, '*uint32');     %6
        RawPulse(pulse_loop_counter).offsetHorIQ = fread(fileID, 1, '*uint32');  %7
        RawPulse(pulse_loop_counter).numVerIQ = fread(fileID, 1, '*uint32');     %8
        RawPulse(pulse_loop_counter).offsetVerIQ = fread(fileID, 1, '*uint32');  %9
        RawPulse(pulse_loop_counter).numDualIQ = fread(fileID, 1, '*uint32');    %10
        RawPulse(pulse_loop_counter).offsetDualIQ = fread(fileID, 1, '*uint32'); %11
        RawPulse(pulse_loop_counter).reserve_num_offsetA = fread(fileID, 56, '*uint32'); %12 l_56 %67  unused offset
        RawPulse(pulse_loop_counter).reserve_num_offsetB = fread(fileID, 64, '*uint32'); %68 l_64 %131 unused offset
        RawPulse(pulse_loop_counter).reserve1 = fread(fileID, 9, '*uint32');             %132 l_9 %140      reserve
        %******************************************************************
        RawPulse(pulse_loop_counter).aziTag = fread(fileID, 1, '*uint32');       %141
        RawPulse(pulse_loop_counter).eleTag = fread(fileID, 1, '*uint32');       %142
        RawPulse(pulse_loop_counter).aziTagDeg = double(RawPulse(pulse_loop_counter).aziTag) * 359.9945068359375 / 65535.0; %compute
        RawPulse(pulse_loop_counter).eleTagDeg = double(RawPulse(pulse_loop_counter).eleTag) * 179.9945068359375 / 32767.0; %compute
        RawPulse(pulse_loop_counter).aziSpeed = fread(fileID, 1, '*float32');    %143
        RawPulse(pulse_loop_counter).eleSpeed = fread(fileID, 1, '*float32');    %144
        %******************************************************************
        RawPulse(pulse_loop_counter).angleSyncFlag = fread(fileID, 1, '*uint32');       %145
        RawPulse(pulse_loop_counter).angleSyncPulseCount = fread(fileID, 1, '*uint32'); %146
        RawPulse(pulse_loop_counter).pulseCounter = fread(fileID, 1, '*uint32');        %147
        %******************************************************************
        RawPulse(pulse_loop_counter).highPrf = fread(fileID, 1, '*uint32');             %148
        RawPulse(pulse_loop_counter).lowPrf = fread(fileID, 1, '*uint32');              %149
        RawPulse(pulse_loop_counter).prf3 = fread(fileID, 1, '*uint32');                %150
        RawPulse(pulse_loop_counter).prf4 = fread(fileID, 1, '*uint32');                %151
        RawPulse(pulse_loop_counter).prfIndicator = fread(fileID, 1, '*uint32');        %152
        %******************************************************************
        RawPulse(pulse_loop_counter).rangeResolutionIQ = fread(fileID, 1, '*float32');     %153
        RawPulse(pulse_loop_counter).numberOfValidRangeBins = fread(fileID, 1, '*uint32'); %154
        %******************************************************************
        RawPulse(pulse_loop_counter).timestampS = fread(fileID, 1, '*uint32');          %155
        RawPulse(pulse_loop_counter).timestampUS = fread(fileID, 1, '*uint32');         %156
        %******************************************************************
        RawPulse(pulse_loop_counter).meanBurstPower = fread(fileID, 1, '*float32');     %157
        RawPulse(pulse_loop_counter).burstPower = fread(fileID, 1, '*float32');         %158
        RawPulse(pulse_loop_counter).meanBurstFreq = fread(fileID, 1, '*float32');      %159
        RawPulse(pulse_loop_counter).burstPhase = fread(fileID, 1, '*float32');         %160
        RawPulse(pulse_loop_counter).lastBurstPhase = fread(fileID, 1, '*float32');     %161
        %******************************************************************
        RawPulse(pulse_loop_counter).IFSamplingFreq = fread(fileID, 1, '*float32');     %162
        %******************************************************************
        RawPulse(pulse_loop_counter).converterPhaseDiffHorizontal = fread(fileID, 1, '*float32');  %163
        RawPulse(pulse_loop_counter).converterPhaseDiffVertical = fread(fileID, 1, '*float32');    %164
        RawPulse(pulse_loop_counter).converterPowerDiffHorizontal = fread(fileID, 1, '*float32');  %165
        RawPulse(pulse_loop_counter).converterPowerDiffVertical = fread(fileID, 1, '*float32');    %166
        RawPulse(pulse_loop_counter).reserve2 = fread(fileID, 2, '*uint32');              %167 l_2 %168      reserve
        %******************************************************************
        RawPulse(pulse_loop_counter).afcOffset = fread(fileID, 1, '*float32');             %169
        RawPulse(pulse_loop_counter).afcMode = fread(fileID, 1, '*uint32');                %170
        RawPulse(pulse_loop_counter).errorFlags = fread(fileID, 1, '*uint32');             %171
        %******************************************************************
        RawPulse(pulse_loop_counter).adcTemperature = fread(fileID, 8, '*int8');  %172 l_8 %179
        %******************************************************************
        RawPulse(pulse_loop_counter).ifdPowerFlags = fread(fileID, 1, '*uint32');          %173
        RawPulse(pulse_loop_counter).reserve3 = fread(fileID, 4, '*uint32');      %174 l_4 %177   reserve
        %******************************************************************
        RawPulse(pulse_loop_counter).headerID = fread(fileID, 1, '*uint32');               %178
        %******************************************************************
        RawPulse(pulse_loop_counter).adcHLMaxAmplitude = fread(fileID, 1, '*float32');     %179
        RawPulse(pulse_loop_counter).adcHHMaxAmplitude = fread(fileID, 1, '*float32');     %180
        RawPulse(pulse_loop_counter).adcVLMaxAmplitude = fread(fileID, 1, '*float32');     %181
        RawPulse(pulse_loop_counter).adcVHMaxAmplitude = fread(fileID, 1, '*float32');     %182
        RawPulse(pulse_loop_counter).adcOverflowFlags = fread(fileID, 1, '*uint32');       %183
        %******************************************************************
        RawPulse(pulse_loop_counter).pciDMATransferRate = fread(fileID, 1, '*float32');    %184
        RawPulse(pulse_loop_counter).pciDMAFIFOFillDegree = fread(fileID, 1, '*float32');  %185
        RawPulse(pulse_loop_counter).reserve4 = fread(fileID, 3, '*float32');     %186 l_3 %188   reserve
        %******************************************************************
        RawPulse(pulse_loop_counter).errorConditionCounter = fread(fileID, 1, '*uint32');  %189
        RawPulse(pulse_loop_counter).errorCondition = fread(fileID, 32, '*uint32');        %190 l_32 %221
        RawPulse(pulse_loop_counter).reserve5 = fread(fileID, 7, '*float32');     %222 l_7 %228   reserve
        %******************************************************************
        RawPulse(pulse_loop_counter).dspFirmwareVersion = fread(fileID, 1, '*uint32');     %229
        RawPulse(pulse_loop_counter).fpgaPciFirmwareVersion = fread(fileID, 1, '*uint32'); %230
        RawPulse(pulse_loop_counter).fpgaIfdFirmwareVersion = fread(fileID, 1, '*uint32'); %231
        RawPulse(pulse_loop_counter).reserve6a = fread(fileID, 1, '*uint32');              %232 reserve
        RawPulse(pulse_loop_counter).reserve6 = fread(fileID, 5, '*float32');     %233 l_5 %237 reserve
        %******************************************************************
        RawPulse(pulse_loop_counter).checksum = fread(fileID, 1, '*uint32');               %238
        %******************************************************************
        RawPulse(pulse_loop_counter).radarWavelength = fread(fileID, 1, '*float32');       %239
        RawPulse(pulse_loop_counter).horizontalBeamwidth = fread(fileID, 1, '*float32');   %240
        RawPulse(pulse_loop_counter).verticalBeamwidth = fread(fileID, 1, '*float32');     %241
        %******************************************************************
        RawPulse(pulse_loop_counter).pwIndex = fread(fileID, 1, '*uint32');                %242
        %******************************************************************
        RawPulse(pulse_loop_counter).noisePowerH = fread(fileID, 1, '*float32');           %243
        RawPulse(pulse_loop_counter).noisePowerV = fread(fileID, 1, '*float32');           %244
        RawPulse(pulse_loop_counter).dbz0H = fread(fileID, 1, '*float32');                 %245
        RawPulse(pulse_loop_counter).dbz0V = fread(fileID, 1, '*float32');                 %246
        RawPulse(pulse_loop_counter).dbz0C = fread(fileID, 1, '*float32');                 %247
        RawPulse(pulse_loop_counter).zdrOffset = fread(fileID, 1, '*float32');             %248
        RawPulse(pulse_loop_counter).ldrOffset = fread(fileID, 1, '*float32');             %249
        RawPulse(pulse_loop_counter).phidpOffset = fread(fileID, 1, '*float32');           %250
        %******************************************************************
        RawPulse(pulse_loop_counter).sweepUID = fread(fileID, 1, '*uint32');               %251
        RawPulse(pulse_loop_counter).reserve7 = fread(fileID, 3, '*uint32');      %252 l_3 %254   reserve
        %******************************************************************
        %******************************************************************
        IQDATA = fread(fileID, 4*RawPulse(pulse_loop_counter).numDualIQ, '*float32'); %IhQhIvQv DUAL
        index_Ih = (((0:RawPulse(pulse_loop_counter).numDualIQ-1)*4)+1);
        index_Qh = (((0:RawPulse(pulse_loop_counter).numDualIQ-1)*4)+2);
        index_Iv = (((0:RawPulse(pulse_loop_counter).numDualIQ-1)*4)+3);
        index_Qv = (((0:RawPulse(pulse_loop_counter).numDualIQ-1)*4)+4);
        RawPulse(pulse_loop_counter).Ih = IQDATA(index_Ih);
        RawPulse(pulse_loop_counter).Qh = IQDATA(index_Qh);
        RawPulse(pulse_loop_counter).Iv = IQDATA(index_Iv);
        RawPulse(pulse_loop_counter).Qv = IQDATA(index_Qv);
        %******************************************************************
        %******************************************************************
        position = ftell(fileID);                                                   %need multiples of 4096
        words2read = E3P_Defaults.ALIGNTO - mod( position, E3P_Defaults.ALIGNTO );  %calculating
        floats2read = words2read / 4 / 4 * 4;                                       %calculating
        %******************************************************************
        %******************************************************************
        IQDATA_PAD = fread(fileID, floats2read, '*float32');   %pad data -- ?? extra IhQhIvQv ??
        index_Ih_PAD = (((0:(floats2read/4.0)-1)*4)+1);
        index_Qh_PAD = (((0:(floats2read/4.0)-1)*4)+2);
        index_Iv_PAD = (((0:(floats2read/4.0)-1)*4)+3);
        index_Qv_PAD = (((0:(floats2read/4.0)-1)*4)+4);
        RawPulse(pulse_loop_counter).Ih_PAD = IQDATA_PAD(index_Ih_PAD);
        RawPulse(pulse_loop_counter).Qh_PAD = IQDATA_PAD(index_Qh_PAD);
        RawPulse(pulse_loop_counter).Iv_PAD = IQDATA_PAD(index_Iv_PAD);
        RawPulse(pulse_loop_counter).Qv_PAD = IQDATA_PAD(index_Qv_PAD);
        %******************************************************************
        %******************************************************************
        position1 = ftell(fileID); %check
        RawPulse(pulse_loop_counter).position1 = position1;
        %******************************************************************
        %******************************************************************
        
        %******************************************************************
        %******************************************************************
        %str = sprintf('NR: %i, MX: %i, ID: %i, AZ: %8.6f, EL: %8.6f, ASF: %i, ASPC: %i, PC: %i',    ...
        %    RawPulse(pulse_loop_counter).pulse_loop_counter,    ...
        %    RawPulse(pulse_loop_counter).pulses_in_file,    ...
        %    RawPulse(pulse_loop_counter).command,    ...
        %    RawPulse(pulse_loop_counter).aziTagDeg,    ...
        %    RawPulse(pulse_loop_counter).eleTagDeg,    ...
        %    RawPulse(pulse_loop_counter).angleSyncFlag,    ...
        %    RawPulse(pulse_loop_counter).angleSyncPulseCount,    ...
        %    RawPulse(pulse_loop_counter).pulseCounter); 
        %disp(str); %full structure: RawPulse(pulse_loop_counter)
        %******************************************************************
        %******************************************************************
        
        %******************************************************************
        %******************************************************************
        pulse_loop_counter = pulse_loop_counter + 1;%increment loop counter
        %******************************************************************
        %******************************************************************
        
    catch err %catch the error and re-emit
        error('GamicIQ:ReadGamicIQ:readpulses','Failed with id: %s and message: %s for: %s.',err.identifier, err.message, GamicIQ_Filename);
    end
end
%**************************************************************************
%**************************************************************************

%**************************************************************************
%"DATA STRUCTURE"
data.UserFunction='ReadGamicIQ';
data.UserFunctionVersion='1.00';
data.GamicIQ_Filename=GamicIQ_Filename;
data.nbytes=nbytes;
data.pulses_in_file=pulses_in_file;
%**************************************************************************

%**************************************************************************
%CLOSE FILE
ferror = fclose(fileID);
if ferror ~= 0
    error('GamicIQ:ReadGamicIQ:fclose','%d: unable to close file: %s', ferror, GamicIQ_Filename);
end
%**************************************************************************

%**************************************************************************
%**************************************************************************
str = sprintf('Function         : %s', data.UserFunction); disp(str);
str = sprintf('Function Version : %s', data.UserFunctionVersion); disp(str);
str = sprintf('Parsing File     : %s', data.GamicIQ_Filename); disp(str);
str = sprintf('File length      : %i', data.nbytes); disp(str);
str = sprintf('Pulses in File   : %i', data.pulses_in_file); disp(str);
%**************************************************************************
%**************************************************************************

%**************************************************************************
%**************************************************************************
%toc
%**************************************************************************
%**************************************************************************

%return; %here we may switch off plotting

%**************************************************************************
%**************************************************************************
%tic
%**************************************************************************
%**************************************************************************


%**************************************************************************
%**************************************************************************
str = sprintf('\nGamicIQPulsePlot : '); disp(str);
%**************************************************************************
%**************************************************************************

%**************************************************************************
%**************************************************************************
%plot
opengl hardware;
scrsz = get(0,'ScreenSize');
%**************************************************************************
X1 = (1:pulses_in_file); XLim = [0, pulses_in_file];
Y1(pulses_in_file)=0; Y2(pulses_in_file)=0; 
Y3(pulses_in_file)=0; Y4(pulses_in_file)=0;
for ujk=1:pulses_in_file
    Y1(ujk) = RawPulse(ujk).aziTagDeg; Y2(ujk) = RawPulse(ujk).eleTagDeg;
    Y3(ujk) = RawPulse(ujk).aziSpeed; Y4(ujk) = RawPulse(ujk).eleSpeed;
end
%**************************************************************************
antenna_movement = figure('Name','Position and Speed of the Antenna',    ...
                          'NumberTitle','off','Renderer','OpenGL',    ...
                          'Position',[1 scrsz(4)/6 scrsz(3)*(2/3) scrsz(4)*(2/3)]);
figure(antenna_movement);
annotation(antenna_movement,'textbox',[0 0.99 1 0.01],    ...
    'String',{'Position and Speed of the Antenna'},'LineStyle','none',    ...
    'HorizontalAlignment','center','FontSize',10,'FitBoxToText','off');
subplot(2,2,1), plot(X1,Y1,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Azimut'), xlabel('Pulsecount'),    ...
                ylabel('Azimut'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,2), plot(X1,Y2,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Elevation'), xlabel('Pulsecount'),    ...
                ylabel('Elevation'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,3), plot(X1,Y3,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Speed in Azimut'), xlabel('Pulsecount'),    ...
                ylabel('Speed in Azimut'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,4), plot(X1,Y4,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Speed in Elevation'), xlabel('Pulsecount'),    ...
                ylabel('Speed in Elevation'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
saveas(antenna_movement,'Position_and_Speed_of_the_Antenna.png','png');
delete(antenna_movement);
clear X1 XLim Y1 Y2 Y3 Y4 antenna_movement scrsz ujk;
str = sprintf('Created and Saved: %s', 'Position_and_Speed_of_the_Antenna.png'); disp(str); %print
%**************************************************************************
%**************************************************************************

%**************************************************************************
%**************************************************************************
%plot
opengl hardware;
scrsz = get(0,'ScreenSize');
%**************************************************************************
X1 = (1:pulses_in_file); XLim = [0, pulses_in_file];
Y1(pulses_in_file)=0; Y2(pulses_in_file)=0; 
Y3(pulses_in_file)=0; Y4(pulses_in_file)=0;
for ujk=1:pulses_in_file
    Y1(ujk) = RawPulse(ujk).burstPower; Y2(ujk) = RawPulse(ujk).burstPhase;
    Y3(ujk) = RawPulse(ujk).meanBurstFreq*1.0d-7; Y4(ujk) = RawPulse(ujk).burstPhase;
end
[n,xout] = hist(Y4,50);
%**************************************************************************
BurstProperties = figure('Name','Burst Properties',    ...
                          'NumberTitle','off','Renderer','OpenGL',    ...
                          'Position',[1 scrsz(4)/6 scrsz(3)*(2/3) scrsz(4)*(2/3)]);
figure(BurstProperties);
annotation(BurstProperties,'textbox',[0 0.99 1 0.01],    ...
    'String',{'Burst Properties'},'LineStyle','none',    ...
    'HorizontalAlignment','center','FontSize',10,'FitBoxToText','off');
subplot(2,2,1), plot(X1,Y1,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Burst Power'), xlabel('Pulsecount'),    ...
                ylabel('Burst Power'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,2), plot(X1,Y2,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Burst Phase'), xlabel('Pulsecount'),    ...
                ylabel('Burst Phase'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,3), plot(X1,Y3,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Mean Burst Frequency'), xlabel('Pulsecount'),    ...
                ylabel('Mean Burst Frequency'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,4), bar(xout,n,'FaceColor','b','EdgeColor','w','LineWidth',1.0),    ...
                title('Histogram of Burst Phase'), xlabel('Burst Phase'),    ...
                ylabel('Frequency'), grid on, set(gca,'XLim',[-pi, pi],'LineWidth',1.5);
saveas(BurstProperties,'Burst_Properties.png','png');
delete(BurstProperties);
clear X1 XLim Y1 Y2 Y3 Y4 BurstProperties scrsz ujk xout n;
str = sprintf('Created and Saved: %s', 'Burst_Properties.png'); disp(str); %print
%**************************************************************************
%**************************************************************************

%**************************************************************************
%**************************************************************************
%plot
opengl hardware;
scrsz = get(0,'ScreenSize');
%**************************************************************************
X1 = (1:pulses_in_file); XLim = [0, pulses_in_file];
Y1(pulses_in_file)=0; Y2(pulses_in_file)=0; 
Y3(pulses_in_file)=0; Y4(pulses_in_file)=0;
for ujk=1:pulses_in_file
    Y1(ujk) = RawPulse(ujk).converterPowerDiffHorizontal; Y2(ujk) = RawPulse(ujk).converterPowerDiffVertical;
    Y3(ujk) = RawPulse(ujk).converterPhaseDiffHorizontal; Y4(ujk) = RawPulse(ujk).converterPhaseDiffVertical;
end
%**************************************************************************
ConverterProperties = figure('Name','Converter Properties',    ...
                          'NumberTitle','off','Renderer','OpenGL',    ...
                          'Position',[1 scrsz(4)/6 scrsz(3)*(2/3) scrsz(4)*(2/3)]);
figure(ConverterProperties);
annotation(ConverterProperties,'textbox',[0 0.99 1 0.01],    ...
    'String',{'Converter Properties'},'LineStyle','none',    ...
    'HorizontalAlignment','center','FontSize',10,'FitBoxToText','off');
subplot(2,2,1), plot(X1,Y1,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Converter Power Diff Horizontal'), xlabel('Pulsecount'),    ...
                ylabel('Converter Power Diff Horizontal'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,2), plot(X1,Y2,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Converter Power Diff Vertical'), xlabel('Pulsecount'),    ...
                ylabel('Converter Power Diff Vertical'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,3), plot(X1,Y3,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Converter Phase Diff Horizontal'), xlabel('Pulsecount'),    ...
                ylabel('Converter Phase Diff Horizontal'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,4), plot(X1,Y4,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. Converter Phase Diff Vertical'), xlabel('Pulsecount'),    ...
                ylabel('Converter Phase Diff Vertical'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
saveas(ConverterProperties,'Converter_Properties.png','png');
delete(ConverterProperties);
clear X1 XLim Y1 Y2 Y3 Y4 ConverterProperties scrsz ujk;
str = sprintf('Created and Saved: %s', 'Converter_Properties.png'); disp(str); %print
%**************************************************************************
%**************************************************************************

%**************************************************************************
%**************************************************************************
%plot
opengl hardware;
scrsz = get(0,'ScreenSize');
%**************************************************************************
X1 = (1:pulses_in_file); XLim = [0, pulses_in_file];
Y1(pulses_in_file)=0; Y2(pulses_in_file)=0; 
Y3(pulses_in_file)=0; Y4(pulses_in_file)=0;
for ujk=1:pulses_in_file
    Y1(ujk) = RawPulse(ujk).adcHLMaxAmplitude; Y2(ujk) = RawPulse(ujk).adcHHMaxAmplitude;
    Y3(ujk) = RawPulse(ujk).adcVLMaxAmplitude; Y4(ujk) = RawPulse(ujk).adcVHMaxAmplitude;
end
%**************************************************************************
ADCProperties = figure('Name','ADC Amplitude Properties',    ...
                          'NumberTitle','off','Renderer','OpenGL',    ...
                          'Position',[1 scrsz(4)/6 scrsz(3)*(2/3) scrsz(4)*(2/3)]);
figure(ADCProperties);
annotation(ADCProperties,'textbox',[0 0.99 1 0.01],    ...
    'String',{'ADC Amplitude Properties'},'LineStyle','none',    ...
    'HorizontalAlignment','center','FontSize',10,'FitBoxToText','off');
subplot(2,2,1), plot(X1,Y1,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC HL Max Amplitude'), xlabel('Pulsecount'),    ...
                ylabel('ADC HL Max Amplitude'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,2), plot(X1,Y2,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC HH Max Amplitude'), xlabel('Pulsecount'),    ...
                ylabel('ADC HH Max Amplitude'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,3), plot(X1,Y3,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC VL Max Amplitude'), xlabel('Pulsecount'),    ...
                ylabel('ADC VL Max Amplitude'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(2,2,4), plot(X1,Y4,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC VH Max Amplitude'), xlabel('Pulsecount'),    ...
                ylabel('ADC VH Max Amplitude'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
saveas(ADCProperties,'ADC_Amplitude_Properties.png','png');
delete(ADCProperties);
clear X1 XLim Y1 Y2 Y3 Y4 ADCProperties scrsz ujk;
str = sprintf('Created and Saved: %s', 'ADC_Amplitude_Properties.png'); disp(str); %print
%**************************************************************************
%**************************************************************************

%**************************************************************************
%**************************************************************************
%plot
opengl hardware;
scrsz = get(0,'ScreenSize');
%**************************************************************************
X1 = (1:pulses_in_file); XLim = [0, pulses_in_file];
Y1(pulses_in_file)=0; Y2(pulses_in_file)=0;
Y3(pulses_in_file)=0; Y4(pulses_in_file)=0;
Y5(pulses_in_file)=0; Y6(pulses_in_file)=0;
for ujk=1:pulses_in_file
    Y1(ujk) = RawPulse(ujk).adcTemperature(1); Y2(ujk) = RawPulse(ujk).adcTemperature(2);
    Y3(ujk) = RawPulse(ujk).adcTemperature(3); Y4(ujk) = RawPulse(ujk).adcTemperature(4);
    Y5(ujk) = RawPulse(ujk).adcTemperature(5); Y6(ujk) = RawPulse(ujk).adcTemperature(6);
end
%**************************************************************************
ADCProperties = figure('Name','ADC Temperature Properties',    ...
                          'NumberTitle','off','Renderer','OpenGL',    ...
                          'Position',[1 scrsz(4)/6 scrsz(3)*(2/3) scrsz(4)*(2/3)]);
figure(ADCProperties);
annotation(ADCProperties,'textbox',[0 0.99 1 0.01],    ...
    'String',{'ADC Temperature Properties'},'LineStyle','none',    ...
    'HorizontalAlignment','center','FontSize',10,'FitBoxToText','off');
subplot(3,2,1), plot(X1,Y1,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC HL Temperature'), xlabel('Pulsecount'),    ...
                ylabel('ADC HL Temperature'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(3,2,2), plot(X1,Y2,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC HH Temperature'), xlabel('Pulsecount'),    ...
                ylabel('ADC HH Temperature'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(3,2,3), plot(X1,Y3,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC VL Temperature'), xlabel('Pulsecount'),    ...
                ylabel('ADC VL Temperature'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(3,2,4), plot(X1,Y4,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC VH Temperature'), xlabel('Pulsecount'),    ...
                ylabel('ADC VH Temperature'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(3,2,5), plot(X1,Y5,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC BURST Temperature'), xlabel('Pulsecount'),    ...
                ylabel('ADC BURST Temperature'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
subplot(3,2,6), plot(X1,Y6,'Color','blue','LineWidth',1.5),    ...
                title('Pulsecount vs. ADC MB Temperature'), xlabel('Pulsecount'),    ...
                ylabel('ADC MB Temperature'), grid on, set(gca,'XLim',XLim,'LineWidth',1.5);
saveas(ADCProperties,'ADC_Temperature_Properties.png','png');
delete(ADCProperties);
clear X1 XLim Y1 Y2 Y3 Y4 ADCProperties scrsz ujk;
str = sprintf('Created and Saved: %s', 'ADC_Temperature_Properties.png'); disp(str); %print
%**************************************************************************
%**************************************************************************

%**************************************************************************
%CLEAN EXIT
clear E3P_Defaults GamicIQ_Filename IQDATA IQDATA_PAD ans    ...
      actpos command error_message ferror fileID floats2read    ...
      index_Ih index_Ih_PAD index_Iv index_Iv_PAD index_Qh index_Qh_PAD    ...
      index_Qv index_Qv_PAD length m_ALIGNTOS n_ALIGNTOS nbytes padsize    ... 
      position position1 pulse_loop_counter pulses_in_file str    ...
      tolerance_single words2read;
%**************************************************************************

%**************************************************************************
%DONE % whos
%**************************************************************************

%**************************************************************************
%**************************************************************************
%toc
%**************************************************************************
%**************************************************************************

end