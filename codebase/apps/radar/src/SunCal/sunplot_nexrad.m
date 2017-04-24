%==================================================
% plot sun calibration results for NEXRAD
%

function x = sunplot_nexrad(dataDir)

   fprintf('Input datadir is: %s\n', dataDir);

     plot_var(dataDir, 'dbm', 'Power averaged (dBm)', 'color_axis', [-80 -63]);
     plot_var(dataDir, 'dbmH', 'Power H channel (dBm)', 'color_axis', [-80 -63]);
     plot_var(dataDir, 'dbmV', 'Power V channel (dBm)', 'color_axis', [-80 -63]);
     plot_var(dataDir, 'corrHV', 'Correlation H-V', 'color_axis', [0.0 0.05]);
     plot_var(dataDir, 'phaseHV', 'Arg H-V', 'color_axis', [-180 180]);
     plot_var(dataDir, 'SS', 'SS (dB)','color_axis',[-1.0 1.5]);
     plot_var(dataDir, 'zdr', 'zdr (dB)','color_axis',[-1.0 1.0]);

end

%=================================================
% plot a specified variable
%

function plot_var(dataDir, varName, label, col_axis, data_range)

  % color_axis = [];
     color_axis = data_range;
     
     fileName = strcat(dataDir, '/', varName, '.txt');
     fprintf('Input field fileName is: %s\n', fileName);
     fprintf('Data range is: %s\n', data_range);
     %varData = load(fileName);
     
     %x_grid = num2cell(load(strcat(dataDir, '/', 'x_grid_info')));
     %y_grid = num2cell(load(strcat(dataDir, '/', 'y_grid_info')));

     fid = fopen(fileName,'rt');
     
     [x_grid,count] = fscanf(fid,'%f',3);
     if count ~= 3 
       error('bang');
     end;
     Nx = x_grid(3);
     min_x = x_grid(1);
     del_x = x_grid(2);
     
     [y_grid,count] = fscanf(fid,'%f',3);
     if count ~= 3 
       error('bang');
     end;
     min_y = y_grid(1);
     del_y = y_grid(2);
     Ny = y_grid(3);
     
     [varData,count] = fscanf(fid,'%f',Nx * Ny);
     if count ~= Nx * Ny
       error('bang');
     end;
     
     varData = reshape(varData,Nx,Ny).';
     
     [extraData,count] = fscanf(fid,'%f');
     if count == 2
       x_centroid = extraData(1);
       y_centroid = extraData(2);
       isCentroid = 1;
     else
       isCentroid = 0;
     end;

     %fprintf('x_grid: %g %g %d %g\n', x_grid{:});
     %fprintf('y_grid: %g %g %d %g\n', y_grid{:});
     
     %xlin=linspace(-5, 5, 101);
     %ylin=linspace(-2, 2, 41);
     %xlin=linspace(x_grid{1:3});
     %ylin=linspace(y_grid{1:3});
     xlin=linspacedel(x_grid(1),x_grid(2),x_grid(3));
     ylin=linspacedel(y_grid(1),y_grid(2),y_grid(3));
     %[X,Y]=meshgrid(xlin,ylin);

     figure;
     %set(gcf,'units','inches','position',[x_ll, y_ll, x_w, y_h]);
     %hold on;
     %contour(X, Y, varData);
     
     % [cs,h] = contourf(xlin, ylin, varData, 15);
     if isempty(color_axis)
       [cs,h] = contourf(xlin, ylin, varData,15);
     else
       [cs,h] = contourf(xlin, ylin, varData, linspace(color_axis(1),...
                                                       color_axis(2),15));
     end
     
     colorbar;
     %%clabel(cs,h);

     %% surfmat(xlin, ylin, varData);

     axis square;
     xlim([(x_centroid - 1.5) (x_centroid + 1.5)]);
     ylim([(y_centroid - 1.5) (y_centroid + 1.5)]);
     if ~isempty(color_axis)
       caxis(color_axis);
     end;
     
     line(xlim,y_centroid*[1 1],'color','cyan');
     line(x_centroid*[1 1],ylim,'color','cyan');% [.5 .9 1], 'linewidth',2,'linestyle',{'-',':','-.'}  help plot
     

%%   plot 1 deg and 2 deg circles

     circle_pos = [(x_centroid-0.5) (y_centroid-0.5) 1 1];
     rectangle('Position',circle_pos,'Curvature',[1 1],'EdgeColor','white','LineWidth',1.5);

     circle_pos = [(x_centroid-1) (y_centroid-1) 2 2];
     rectangle('Position',circle_pos,'Curvature',[1 1],'EdgeColor','white','LineWidth',1.5);

     %colormap(nex_cmap); 
     colormap(nex_cmap(20)); 
     colorbar;
     
     % title(label);

     [pathstr, dirname, ext] = fileparts(dataDir);
     my_title = strcat(label, '---', dirname);
     my_title1 = strrep(my_title, '_', ' ');
     my_title2 = strrep(my_title1, '---', '   ');
     
     title(my_title2);
     xlabel('Azim (deg) from theoretical sun center');
     ylabel('Elev (deg) from theoretical sun center');

     % h = gca;
     % axes(getcolorbar);
     % ylabel('dBm');
     % axes(h);
     

     set(gcf,'Renderer','zbuffer','PaperPositionMode','auto');
     print(gcf,'-dpng',[fullfile(dataDir,varName) '.png']);
     %hold off;

end

%======================================================
% color bar

function x = nex_cmap(n);

if nargin==1 & isempty(n)
  n = size(get(gcf,'Colormap'),1);
end;

cmap = [ 0         0    0.5000; ...
         0         0    1.0000; ...
         0    0.9000    1.0000; ...
         0    1.0000    0.7000; ...
         0    0.8000         0; ...
         0    0.5000         0; ...
    0.6000    0.6000    0.6000; ...
    0.5000    0.5000         0; ...
    0.7500    0.7500         0; ...
    1.0000    1.0000    0.3000; ...
    1.0000    0.8500         0; ...
    1.0000    0.5000         0; ...
    1.0000         0         0];

if nargin < 1
  n = size(cmap,1);
end;

x = interp1(linspace(0,1,size(cmap,1)),cmap(:,1),linspace(0,1,n)','linear');
x(:,2) = interp1(linspace(0,1,size(cmap,1)),cmap(:,2),linspace(0,1,n)','linear');
x(:,3) = interp1(linspace(0,1,size(cmap,1)),cmap(:,3),linspace(0,1,n)','linear');

x = min(x,1);
x = max(x,0);

end

%======================================================
%LINSPACE Linearly spaced vector.
function y = linspacedel(d1, del, n)
%
%   LINSPACE(x1, del, N) generates N points between x1 and (N-1)*del.
%
%   See also LOGSPACE, :., linspace
%   Copyright 1984-2001 The MathWorks, Inc. 
%   $Revision: 1.1 $  $Date: 2002/08/20 19:10:58 $

y = [d1:del:((n-1)*del+d1)];

end


