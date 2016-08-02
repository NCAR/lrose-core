function do_all_plots(dirname,varargin)
% DO_ALL_PLOTS: do all plots for data in a given dir
%
% usage do_all_plots(dirname,'param1',value1,...)
%
%  inputs: 
%    dirname - string: name of dir to find files
%  Optional Params:
%    do_save - (def: 0) can be 1 or 0: 1 saves the figure as jpeg, 
%              0 does not

% init options
do_save = 0;  %no save (jpeg)
jpeg_level = 90; %quality number from 1 to 100

% parse command line options
paramparse(varargin);

% do the plots

rvplot(fullfile(dirname, 'P1_bias.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P1_bias.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P1_bias.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'P1_sdev.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P1_sdev.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P1_sdev.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'P2_bias.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P2_bias.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P2_bias.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'P2_sdev.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P2_sdev.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'P2_sdev.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'V1_bias.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V1_bias.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V1_bias.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'V1_sdev.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V1_sdev.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V1_sdev.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'V2_bias.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V2_bias.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V2_bias.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'V2_sdev.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V2_sdev.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'V2_sdev.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'W1_bias.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W1_bias.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W1_bias.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'W1_sdev.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W1_sdev.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W1_sdev.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'W2_bias.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W2_bias.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W2_bias.w2_4.0.dat'),'do_save',do_save);

rvplot(fullfile(dirname, 'W2_sdev.w2_2.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W2_sdev.w2_3.0.dat'),'do_save',do_save);
rvplot(fullfile(dirname, 'W2_sdev.w2_4.0.dat'),'do_save',do_save);






