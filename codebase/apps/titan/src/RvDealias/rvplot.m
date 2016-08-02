function d = rvplot(filename,varargin)
% RVPLOT - reads and plots data from RvDealias
%
% usage  d = rvplot(filename,'param1',value1,...)
%
%  inputs: 
%    filename - string: name of file to read
%  Optional Params:
%    do_save - (def: 0) can be 1 or 0: 1 saves the figure as png
%              0 does not
%    save_name - (def: same as filename but extention is changed to .jpg)
%              string containing name of jpg file to save - only used
%              if do_save is 1
%
%  Outputs:
%     d - a structure containing the data as read by this function
%
%  This function read the data file, throwing values into variables
%  once done, it plots the data. 
%  The first non-comment(#) is assumed to be the title
%      second                                    xlabel
%      third                                     ylabel
%      fourth                                    colorbar ylabel
%      5th                                       x-axis vector
%      6th                                       y-axis vector
%      7th                                       matrix to surfmat
%

% init options
do_save = 0;  %no save (png)

[p,f,e] = fileparts(filename);
save_name = fullfile(p,[f '.png']); % init to filename but with .png

% parse command line options
paramparse(varargin);

% open file
fid = fopen(filename);

%
% init for loop
%

% list of modes
modes = {'toplabel','Xlabel','Ylabel','Caxis','Ncolors','Xticks','Yticks','values'};
string_list = {'toplabel','Xlabel','Ylabel'};

%modes = {'toplabel','Xlabel','Ylabel','Clabel','Caxis','Ncolors','Xticks','Yticks','values'};
%string_list = {'toplabel','Xlabel','Ylabel','Clabel'};

% current mode
mode = 1;
% comments at top of file
first_comments = {};
% flag to mark that we are at the beginning of the file
onfirst = 1;

% start loop
while ~feof(fid)
  % mark current file position
  file_offset = ftell(fid);
  
  % read and deblank the next line
  str = leftdeblank(fgetl(fid));
  str = deblank(str);
  
  if str(1)~='#'
    % if not comment:
    % no longer in the first comment section
    onfirst = 0;
    
    switch modes{mode}
     case string_list
      % create variables with the name of the mode that
      % contains the last line read as the value.
      eval(sprintf('%s = str;',modes{mode}));
     otherwise
      % otherwise we are reading numbers:
      % get the number of columns by converting the string
      % we read into a numeric vector (using eval)
      ncols = length(eval(sprintf('[%s]',str)));
      % rewind to before we read this line
      fseek(fid,file_offset,'bof');
      % now read floats until we bang up against some non-float
      data = fscanf(fid,'%f');
      % reshape to the correct size - assuming that all rows in this block
      % have same number of cols as the first row
      data = reshape(data,ncols,[])';
      % assign the data to the variable with name the same as the current mode
      eval(sprintf('%s = data;',modes{mode}));
    end;
    % increment the mode we are running in
    mode = mode + 1;
  elseif onfirst
    % if we are still in first comment block, save the comment
    first_comments{end+1} = str(2:end);
  end;
end;

% close the file
fclose(fid);

% get rid of last comment, since it is a header for the next line
first_comments = first_comments(1:end-1)';

% 'tar' all the data into the struct to get output
d = varstruct({modes{:} 'first_comments'});      

% new figure
figure(1); 
clf

% fontsize
fsize = 7;

%axes
ax = axes;
set(ax,'Position',[.17 .17 .775 .66],'FontSize',fsize);

% create plot from data
surfmat(Xticks,Yticks,values);

% set color scale limits
caxis(Caxis);

%colormap(jet(Ncolors));
%colormap([1-cool(5);1-hot(5)])
colormap([1-cool(5);1-hot(4)]);
set(getcolorbar,'FontSize',fsize);
refreshcolorbar;

%set colorbar's ylabel
%set(get(getcolorbar,'YLabel'),'String',Clabel);

% set plot's x/y labels and title
xlabel(Xlabel,'FontSize',fsize);
ylabel(Ylabel,'FontSize',fsize);
title(toplabel,'FontSize',fsize);

% set window size
% set(1,'units','pixels','Position',[0 0 400 300]);
set(1,'units','pixels','Position',[0 0 250 175]);
% get(1);

set(gcf,'Renderer','zbuffer','PaperPositionMode','auto')

% if do_save - then save the figure as a png file
if do_save
  print(gcf,'-dpng',save_name);
end;

% pause(2);

% close(1);





