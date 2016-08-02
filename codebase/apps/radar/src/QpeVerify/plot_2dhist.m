function plot_2dhist(file,cols,Title,Xlabel,Ylabel,output,varargin)
%
% usage data = plot_2dhist(file,cols,Title,Xlabel,Ylabel,output,'param',value,...)
%
%     cols is either vector, e.g. [2 3] OR string to be eval'ed by 
%          [f,c] = ndhist(eval(cols),'n',64);, e.g. 
%          '[(data(:,5)-data(:6)), data(:,7)]'
%
%  params:
%    fitline - draw fitline - can be 0,1: default 1: 1 to draw fit line
%    correlation - compute corr stats - can be 0,1: def 1: 1 to calc and print
%    placement - if fitline = 1, then placement dictates where stats will be
%                printed.  can be 'tl','tr','bl','br'. default 'tl'
%    yeqxline - draw y=x line, can be 1,0: def 1: 1 to draw y=x line
%

% >> addpath /home/dixon/cvs/apps/matlab/src/utils
% >> addpath /home/dixon/cvs/apps/radar/src/RadarMdvCompare
% # cd /home/dixon/cvs/apps/matlab/src/utils
% # mex ndhist_mex.c

%%%%%%%%%%%%%% creating PNG files
%% set 1 global default - stops _ from subscripting
%set(0,'DefaultTextInterpreter','none')

%% Changes attributes for all current plots
%set(gcfs,'Units','inches','Position',[1.8000 6.1889 6.3333 3.6000]);
%set(gcfs,'Renderer','zbuffer','PaperPositionMode','auto')

%% actually do the plot, gcf returns the current plot handle
%% replace with the integer plot number to plot any specific 
%% plot without having to click on it to make it active.
%print(gcf,'-dpng','plot.png'); end;


%%%%%%%%%% new
correlation = 1;
fitline = 0;
placement = 'tl';
yeqxline = 1;

paramparse(varargin);
%%%%%%%%%%

%% data = load(file);
data = dlmread(file, '', 0, 0);

%% data

%inds = data(:,cols(1))~=0 & data(:,cols(2))~=0;
%inds = data(:,cols(18))>10;
%data = data(inds,:);
%inds = data(:,cols(1))<-1 | data(:,cols(1))>1;
%data = data(inds,:);
%inds = data(:,cols(2))<-1 | data(:,cols(2))>1;
%data = data(inds,:);
%inds = data(:,cols(1))<50 & data(:,cols(2))<50;
%data = data(inds,:);
%inds = data(:,20)>30;
%data = data(inds,:);
%inds = data(:,cols(1))>.5 & data(:,cols(2))>.5 & ...
%       data(:,cols(1))<8 & data(:,cols(2))<8;
%data = data(inds,:);

if isnumeric(cols)
  [f,c] = ndhist(data(:,cols),'n',64);
else
  [f,c] = ndhist(eval(cols),'n',64);
end;

% figure; 
clf;
surfmat(c{1},c{2},log10(f.'/sum(f(:))))
% surfmat(log10(f.'/sum(f(:))))

ticklabelfun('10.^x','y','fmt','%.3g','ax',getcolorbar);
title(Title)
xlabel(Xlabel)
ylabel(Ylabel)

h = gca;
axes(getcolorbar);
xlabel('Percent');
axes(h);

%%%%%%%%%%% new
if correlation
  data1 = data(:,cols(1));
  data2 = data(:,cols(2));
  
  find_ind = find(~isnan(data1) & ~isnan(data2) & ...
		  ~isinf(data1) & ~isinf(data2));
  data1= data1(find_ind);
  data2 = data2(find_ind);
  
  p = perpfit(data1,data2,1);
  %p = polyfit(data1,data2,1);
  corr_mat = corrcoef([data1,data2]);
  if(length(corr_mat)<2)
    corr = 1;
  else
    corr = corr_mat(1,2);
  end
  stand_dev = std(polyval(p,data1)-data2);
  set(gcf,'Units','normalized');
  switch placement(1)
   case 't'
    pos(2) = .98;
    va = 'top';
   otherwise
    pos(2) = .02;
    va = 'bottom';
  end;
  switch placement(2)
   case 'r'
    pos(1) = .98;
    ha = 'right';
   otherwise
    pos(1) = .02;
    ha = 'left';
  end;
    
  text(0,0,strvcat(['\rho = ' num2str(corr)], ...
		   ['\sigma = ' num2str(stand_dev)], ...
		   ['number of points = ' num2str(length(data1))]), ...
       'VerticalAlignment',va,'horizontalalignment',ha,...
       'Units','Normalized', ...
       'Position',pos,...
       'backgroundcolor','white');  
  if fitline
    hold on
    fitline_XLim = xlim;
    plot3(fitline_XLim,polyval(p,fitline_XLim),[1 1],'linewidth',2,'color','w');
  end;
  if yeqxline 
    hold on
    lims = [xlim;ylim];
    lims = [min(lims([1 3])) max(lims([2 4]))];
    plot3(lims,lims,[2 2],'linewidth',2,'color',[1 0 1]);
  end;
    
    
end;

%%%%%%%%%%%

  

set(gcf,'Renderer','zbuffer','PaperPositionMode','auto');
if exist('output','var') & ~isempty(output)
  %print(gcf,'-djpeg75',output);
  print(gcf,'-dpng',output);
end;
