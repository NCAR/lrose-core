%
% Simple MatLab script to facilitate plotting of Validator
% output. Based on the CODAS ADCP system from the University
% of Hawaii, by Eric Firing et al.
%

filename = input('Which file to load (no extension)? ', 's');
eval(['load -ascii ' filename '.dat']);
eval(['Valid = 'filename ';']);
eval(['clear ' filename ';']);
%
% Replace instances of -1000 with NaN.
%
k = size(Valid);
for j=1:18, 
	for l=1:k(1), 
		if (Valid(l,j) < -999.7) 
			Valid(l,j)=NaN; 
		end
	end	
 end
%
% Pick off the cols into seperate MatLab variables.
%
year       = Valid(:,1);
month      = Valid(:,2);
day        = Valid(:,3);
hour       = Valid(:,4);
minute     = Valid(:,5);
second     = Valid(:,6);
t          = Valid(:,7);
pop        = Valid(:,8);
num_non    = Valid(:,9);
num_fail   = Valid(:,10);
num_false  = Valid(:,11);
num_success= Valid(:,12);
pod        = Valid(:,13);
far        = Valid(:,14);
csi        = Valid(:,15);
hss        = Valid(:,16);
pod_no     = Valid(:,17);
bias       = Valid(:,18);

clear Valid;
%
% Convert time to hours.
%

t = t / 3600.0;

%
% Give the start time.
%

start(1)=year(1);    start(2)=month(1); 
start(3)=day(1);     start(4)= hour(1);
start(5)= minute(1); start(6)= second(1);
start
clear start

%
% Do some plotting. User must press return after
% each plot.
%

a='User must press return after each plot.'
clear a;


plot(t,csi,'r+');  axis([min(t) max(t) 0 1]);
xlabel('Time (Hours) since start');
ylabel('CSI');
title('Critical Success Index');
pause

plot(t,hss,'r+');
xlabel('Time (Hours) since start');
ylabel('HSS');
title('Heidke Skill Score');
pause

plot(t,pod,'r+'); axis([min(t) max(t) 0 1]);
xlabel('Time (Hours) since start');
ylabel('POD');
title('Probability of Detection');
pause

plot(t,far,'r+'); axis([min(t) max(t) 0 1]);
xlabel('Time (Hours) since start');
ylabel('FAR');
title('False Alarm Ratio');
pause

plot(t,bias,'r+');
xlabel('Time (Hours) since start');
ylabel('Bias');
title('Forecast Bias');
pause

plot(t,pod_no,'r+'); axis([min(t) max(t) 0 1]);
xlabel('Time (Hours) since start');
ylabel('POD NO');
title('POD NO');
pause

plot(t,num_non,'r+');
xlabel('Time (Hours) since start');
ylabel('Non Events');
title('Non Events');
pause

plot(t,num_false,'r+');
xlabel('Time (Hours) since start');
ylabel('False alarms');
title('False Alarms');
pause

plot(t,num_fail,'r+');
xlabel('Time (Hours) since start');
ylabel('Failures');
title('Failures');
pause

plot(t,num_success,'r+');
xlabel('Time (Hours) since start');
ylabel('Successes');
title('Successes');
pause

plot(t,pop,'r+');
xlabel('Time (Hours) since start');
ylabel('Population');
title('Population');










