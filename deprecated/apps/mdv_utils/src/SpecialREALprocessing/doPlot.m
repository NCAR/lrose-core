
load RootMeanSquaredDeltaBeta.dat
plot (RootMeanSquaredDeltaBeta(:,1)/3600.0, RootMeanSquaredDeltaBeta(:,2))
axis([16.5 17.5 0.1 0.25]);
title('RMS Delta Beta vs. Time');
xlabel('Decimal hour of day (UTC)');

pause

load ascii/AutoCorr_20040507_163228.dat; auto = AutoCorr_20040507_163228;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
axis([0 500 -0.2 1.2]);
title('Autocorrelation at 163228 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

pause

load ascii/AutoCorr_20040507_164016.dat; auto = AutoCorr_20040507_164016;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
axis([0 500 -0.2 1.2]);
title('Autocorrelation at 164016 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

pause

load ascii/AutoCorr_20040507_165021.dat; auto = AutoCorr_20040507_165021;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
axis([0 500 -0.2 1.2]);
title('Autocorrelation at 165021 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

pause

load ascii/AutoCorr_20040507_170026.dat; auto = AutoCorr_20040507_170026;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
axis([0 500 -0.2 1.2]);
title('Autocorrelation at 170026 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

pause

load ascii/AutoCorr_20040507_171004.dat; auto = AutoCorr_20040507_171004;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
axis([0 500 -0.2 1.2]);
title('Autocorrelation at 171004 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

pause

load ascii/AutoCorr_20040507_172010.dat; auto = AutoCorr_20040507_172010;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
title('Autocorrelation at 172010 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

pause

load ascii/AutoCorr_20040507_172418.dat; auto = AutoCorr_20040507_172418;
plot(auto(:,1), auto(:,2),'r', auto(:,3), auto(:,4),'g');
axis([0 500 -0.2 1.2]);
title('Autocorrelation at 172418 : X Red, Y green, distances in meters.');
xlabel('Distance in meters');

