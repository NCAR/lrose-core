%==================================================
% plot sun calibration results for NEXRAD
%

function x = sunplot_nexrad(dataDir)

   fprintf('Input datadir is: %s\n', dataDir);

   plot_var(dataDir, 'dbm', 'Power averaged (dBm)', [-80 -63]);
   plot_var(dataDir, 'dbmH', 'Power H channel (dBm)', [-80 -63]);
   plot_var(dataDir, 'dbmV', 'Power V channel (dBm)', [-80 -63]);
   plot_var(dataDir, 'corrHV', 'Correlation H-V', [0.0 0.05]);
   plot_var(dataDir, 'phaseHV', 'Arg H-V', [-180 180]);
   plot_var(dataDir, 'SS', 'SS (dB)', [-1.0 1.5]);
   plot_var(dataDir, 'zdr', 'zdr (dB)', [-1.0 1.0]);

end

%=================================================
% plot a specified variable
%

function plot_var(dataDir, varName, label, data_range)

     % Read in grid info and data

     fileName = strcat(dataDir, '/', varName, '.txt');
     fprintf('Input field fileName is: %s\n', fileName);
     fprintf('Data range is: %s\n', data_range);
     
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
     
     fprintf('x_grid Nx min_x delx: %d %g %d\n', Nx, min_x, del_x);
     fprintf('y_grid Ny min_y dely: %d %g %d\n', Ny, min_y, del_y);

     [varData,count] = fscanf(fid,'%f',Nx * Ny);
     if count ~= Nx * Ny
       error('bang');
     end;
     
     varData = reshape(varData,Nx,Ny).';

     % Read in centroid
     
     [extraData,count] = fscanf(fid,'%f');
     if count == 2
       x_centroid = extraData(1);
       y_centroid = extraData(2);
       isCentroid = 1;
     else
       isCentroid = 0;
     end;

     xlin=linspacedel(x_grid(1),x_grid(2),x_grid(3));
     ylin=linspacedel(y_grid(1),y_grid(2),y_grid(3));

     % draw figure

     figure;
     
     if isempty(data_range)
       [cs,h] = contourf(xlin, ylin, varData,15);
     else
       [cs,h] = contourf(xlin, ylin, varData, linspace(data_range(1),...
                                                       data_range(2),15));
     end

     % axes

     axis square;
     xlim([(x_centroid - 1.5) (x_centroid + 1.5)]);
     ylim([(y_centroid - 1.5) (y_centroid + 1.5)]);
     if ~isempty(data_range)
       caxis(data_range);
     end;
     
     line(xlim,y_centroid*[1 1],'color','cyan');
     line(x_centroid*[1 1],ylim,'color','cyan');

     % plot 1 deg and 2 deg circles

     circle_pos = [(x_centroid-0.5) (y_centroid-0.5) 1 1];
     rectangle('Position',circle_pos,'Curvature',[1 1],'EdgeColor','white','LineWidth',1.5);

     circle_pos = [(x_centroid-1) (y_centroid-1) 2 2];
     rectangle('Position',circle_pos,'Curvature',[1 1],'EdgeColor','white','LineWidth',1.5);

     % colormap

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


