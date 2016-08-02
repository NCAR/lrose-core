clear VEL_*

rad=pi/180;
noise_floor=-107.75;

ground_speed=sqrt(eastward_velocity.^2 + northward_velocity.^2);

[m n]=size(DBZ);

VEL_corr=VEL;
VEL_corr(DBMVC<noise_floor)=nan;

% tau_suba=3.45;% tilt angle wrt aircraft
% theta_suba=azimuth-1.5; % rotation angle wrt aircraft;
tau_suba=tilt -0.14;% tilt angle wrt aircraft
theta_suba=rotation - .10; % rotation angle wrt aircraft;

%vel_pitch=ground_speed.*sin(pitch2*pi/180);

clear track
for i=1:m
%     if eastward_velocity(i)>0 && northward_velocity(i)<0
%         track(i)=atan(-northward_velocity(i)/eastward_velocity(i));
%         track(i)=track(i)*180/pi+90;
%     elseif eastward_velocity(i)>0 && northward_velocity(i)>0
%         track(i)=atan(eastward_velocity(i)/northward_velocity(i));
%         track(i)=track(i)*180/pi;
%     elseif eastward_velocity(i)<0 && northward_velocity(i)>0
%         track(i)=atan(northward_velocity(i)/-eastward_velocity(i));
%         track(i)=track(i)*180/pi+270;
%     elseif eastward_velocity(i)<0 && northward_velocity(i)<0
%         track(i)=atan(-northward_velocity(i)/-eastward_velocity(i));
%         track(i)=track(i)*180/pi+180;
%     end
    track(i)=-atan2(northward_velocity(i),eastward_velocity(i))*180/pi +90;
end

heading(heading<0)=heading(heading<0)+360;
track(track<0)=track(track<0)+360;

drift=track-heading;
drift(drift<-300)=drift(drift<-300)+360;

% convert to radians
drift_rad=drift*rad;
pitch_rad=pitch*rad;
roll_rad=roll*rad;
theta_suba_rad=theta_suba*rad;
tau_suba_rad=tau_suba*rad;

% Compute y_t following equation 9 Lee et al (1994)
y_subt=-cos(theta_suba_rad+roll_rad).*cos(drift_rad).*cos(tau_suba_rad).*sin(pitch_rad)...
    +sin(drift_rad).*sin(theta_suba_rad+roll_rad).*cos(tau_suba_rad)...
    +cos(pitch_rad).*cos(drift_rad).*sin(tau_suba_rad);

% Compute z following equation 9 Lee et al (1994)
z=cos(pitch_rad).*cos(tau_suba_rad).*cos(theta_suba_rad+roll_rad)+sin(pitch_rad).*sin(tau_suba_rad);

% compute tau_t following equation 11 Lee et al (1994)
tau_subt=asin(y_subt);

% Compute phi following equation 17 Lee et al (1994)
phi=asin(z);

% Compute platform motion based on Eq 27 from Lee et al (1994)
vr_platform=-ground_speed.*sin(tau_subt)-vertical_velocity.*sin(phi);
test=vr_platform;

for i=1:m
for j=1:n
    VEL_corr(i,j)=VEL_corr(i,j)-vr_platform(i);
    while VEL_corr(i,j)<-nyquist_velocity(i)
        VEL_corr(i,j)=VEL_corr(i,j)+2*nyquist_velocity(i);
    end
    while VEL_corr(i,j)>nyquist_velocity(i)
        VEL_corr(i,j)=VEL_corr(i,j)-2*nyquist_velocity(i);
    end

end
end

%stop

num_filter=4;
[m n]=size(VEL);

DBZ_tmp=DBZ;
DBZ_tmp(:,1:20)=0;
[bla ground_index]=max(DBZ_tmp,[],2);clear DBZ_tmp

clear VEL_ground VEL_ground_smooth
for i=1:m
    VEL_ground_ucorr(i)=VEL(i,ground_index(i));
    VEL_ground(i)=VEL_corr(i,ground_index(i));
end

figure;plot(time,VEL_ground)
hold on;plot(time,VEL_ground_ucorr-vertical_velocity,'r')
%axis([0 70 -1 1])

% VEL_ground_smooth=VEL_ground;
% for i=1+num_filter:m-num_filter
%     VEL_ground_smooth(i)=mean(VEL_ground(i-num_filter:i+num_filter));
% end

%hold on;plot(time,VEL_ground_smooth,'r')

% for i=1:m
% for j=1:n
%     vel_display2(i,j)=vel_display(i,j)-VEL_ground_smooth(i);
% end
% end
