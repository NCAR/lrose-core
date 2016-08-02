%==================================================
% plot sun calibration results
%

function sunplot(dirName)

     currentDir = pwd;
     dataDir = fullfile(currentDir, dirName);
     fprintf('datadir: %s\n', dataDir);

%     plot_var(dataDir, 'dbm', 'Power averaged (dBm)','color_axis',[-85 -63]);

    plot_var(dataDir, 'dbmHc', 'Power H co-pol (dBm)','color_axis',[-85 -63]);
    plot_var(dataDir, 'dbmHx', 'Power H x-pol (dBm)','color_axis',[-85 -63]);
    plot_var(dataDir, 'dbmVc', 'Power V co-pol (dBm)','color_axis',[-85 -63]);
    plot_var(dataDir, 'dbmVx', 'Power V x-pol (dBm)','color_axis',[-85 -63]);

     plot_var(dataDir, 'corr00H', 'Correlation H-V, H flag = 1');
     plot_var(dataDir, 'corr00V', 'Correlation H-V, V flag = 0');

     plot_var(dataDir, 'arg00H', 'Arg H-V, H flag = 1');
     plot_var(dataDir, 'arg00V', 'Arg H-V, H flag = 0');

     plot_var(dataDir, 'ratioDbmVcHc', 'Power ratio, Vc/Hc (dB)','color_axis',[-2.5 2.5]);
     plot_var(dataDir, 'ratioDbmVxHx', 'Power ratio, Vx/Hx (dB)','color_axis',[-2.5 2.5]);

     plot_var(dataDir, 'ratioDbmHcHx', 'Power ratio, Hc/Hx (dB)','color_axis',[-0.5 0.5]);
     plot_var(dataDir, 'ratioDbmVcVx', 'Power ratio, Vc/Vx (dB)','color_axis',[-0.5 0.5]);

     plot_var(dataDir, 'ratioDbmVcHx', 'Power ratio, Vc/Hx (dB)','color_axis',[-2.5 2.5]);
     plot_var(dataDir, 'ratioDbmVxHc', 'Power ratio, Vx/Hc (dB)','color_axis',[-2.5 2.5]);

     plot_var(dataDir, 'S1S2', 'S1S2 (dB)');
      plot_var(dataDir, 'S1S2', 'S1S2 (dB)','color_axis',[-2.5 0.5]);

end

%=================================================
% plot a particular variable
%

function plot_var(dataDir, varName, label, varargin)

     color_axis = [];
     
     paramparse(varargin);


     fileName = strcat(dataDir, '/', varName, '.txt');
     %varData = load(fileName);
     
     %x_grid = num2cell(load(strcat(dataDir, '/', 'x_grid_info')));
     %y_grid = num2cell(load(strcat(dataDir, '/', 'y_grid_info')));

     fid = fopen(fileName,'rt');
     if (fid < 0)
       return;
     end;
     fprintf('variable file name: %s\n', fileName);
     fprintf('fid: %d\n', fid);
     
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
     
     %colormap(nex_cmap); 
     colormap(nex_cmap(20)); 
     colorbar;
     
     % title(label);

     [pathstr, dirname, ext, versn] = fileparts(dataDir);
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

