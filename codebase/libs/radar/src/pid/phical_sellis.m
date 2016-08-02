%sigma=12*pi/180;
%c1 = 2.7900e-5; c2 = 1.0; a = 1.0086; b = 0.9543;% M. Hadd eq.
%c1 = 5.7900e-5; c2 = 1.0; a = 1.0086; b = 2.76;% M. Hadd eq.
c1 = 3.3200e-5; c2 = 1.0; a = 1.0000; b = 2.0500;% new stuff 10/01
%c1 = (5.304e-5)*(1-2*sigma*sigma); c2 = 1.0; a = 1.0000; b = (2.011-0.423*sigma+6.27*sigma*sigma);

c = c1/c2;

  hdrlim=0;
  haillimit=10;
  
  dzcor= + 0.0
  zdrcor=-0.091 +0.29
  
  dr=0.15;
  
% Read in Data and add corrections for dz and zdr

  %path='/scr/sci/sellis/projects/pecan/phical/'
%  fid=fopen('filelist','r');
  files=dir('*.mat');
  [nfiles,name_size]=size(files);

for i=1:nfiles;
  load(files(i).name)
  n=length(dz);
  nkdp(nkdp<-100)=0;
  dz=dz+dzcor;
  zdr=zdr+zdrcor;

  theta=el_mean*pi/180;
%Attenuation correction

  A_atm=0.0417/sin(theta)*(1-exp(-0.21*sin(theta)*dist));  % Sea level
%  A_atm=0.0354/sin(theta)*(1-exp(-0.18*sin(theta)*dist)); % Colorado
%  A_atm=A_atm*2;

%   Ah(1)=(phi(1))*0.02;
%   Adp(1)=(phi(1))*0.0038;

% use phidp
%  Ah=(phi)*0.02;
%  Adp=(phi)*0.0038;

% use KDP
  Ah(1)=(nkdp(1))*0.02;
  Adp(1)=(nkdp(1))*0.0038;

for j=2:n
  Ah(j)=Ah(j-1)+nkdp(j)*0.02*dr;
  Adp(j)=Adp(j-1) + nkdp(j)*0.0038*dr;
  dzb=dz(j)+2*(Ah(j) +A_atm(j)); zdrb=zdr(j)+2*Adp(j); % use kdp
%  dzb=dz(j)+(Ah(j) +2*A_atm(j)); zdrb=zdr(j)+Adp(j); % use Phidp
%  dzb=dz(j)+2*(A_atm(j)); zdrb=zdr(j)%  atmospheric only
  dz(j)=dzb; 
  zdr(j)=zdrb;
end

  zdr_lin=10.^(zdr/10);

% Compute linear reflectivity

  for j=1:n;
    z(j) = 10.0^(dz(j)/10.0);
  end

%filter Z and ZDR
%  for j=2:n-1;
%    zsmooth(j)=mean(z(j-1:j+1));
%    zdr_linsmooth(j)=mean(zdr_lin(j-1:j+1));
%    z(j)=zsmooth(j);
%    zdr_lin(j)=zdr_linsmooth(j);
%  end
% Compute HDR

  count=0;
  for j=1:n;
    f(i)=0;
    hdr(i)=0;
  end

  for j=1:n;
    if zdr(j) < 0
      f(j)=27;
    elseif zdr(j)>=0 & zdr(j) <= 1.74
      f(j)=19*zdr(j) + 27;
    elseif zdr(j) > 1.74;
      f(j)=60;
    end
    hdr(j)=dz(j)-f(j);
  end
  
% Check to see if there are any hail signals
  if hdr(j)>hdrlim
     count=count + 1;
  end

% Check to see if there are more than 4 hail signals in a row

  count2=0;
  for j=1:n-3;
    if hdr(j) > hdrlim & hdr(j+1) > hdrlim & hdr(j+2) > hdrlim & ...
       hdr(j+3) > hdrlim; 
      count2=count2+1;
    end
  end

%     Perform analysis only if the number of gates with hail is
%     less than or equal to haillimit, i.e. count <= haillimit.
%     haillimit is set at top of program.  Also, do not perform
%     analysis if there are any sections with more than 4 hail
%     signatures in a row.

  if count <= haillimit %& count2==0 %& max(nkdp)<2.0

%##################################################################
%
%     calculate measured phidp,  phidp = (phi(nr)+phi(nr-1)...+phi(nr-4))/5
%                                         - (phi(1) +phi(2)   ...+phi(5))/5
%
%##################################################################

    phidp(i)=(phi(n)+phi(n-1)+phi(n-2)+phi(n-3)+phi(n-4))/5 - ...
	  (phi(1)+phi(2)+phi(3)+phi(4)+phi(5))/5;
%    phidp(i) = phidp(i)/2.0;

%##################################################################
%
%     calculate measured phi using gates in which zdr is not equal to,
%     or less than zero,
%
%##################################################################

    num=0;
    rhs(i)=0;
    count3(i)=0;

    for j=3:n-2
      %if z(j)>0 & zdr(j) > 0.000 & hdr(j) < 10
	      num=num+1;
         rhs(i)=rhs(i) + (z(j)^a)/(zdr_lin(j)^b)*dr;
      %end
    end
%     test1(i)=mean(zdr);
%     test2(i)=mean(z);
    rhs(i)=rhs(i)*c;
    rhs(i)=rhs(i)*2;

    ray_bias(i)=10*log10((rhs(i))/(phidp(i)));
 end
% maxkdp(i)=max(kdpest);

  clear Ah A_atm Adp dz zdr phi hdr z Bold B kdpest Ah2 dist ...
      Ah3 phi_sm dzb zdrb Ah3new zdr_lin
end  % outer loop for seperate ray segments


%bias=10*log10(mean(rhs)/mean(phidp));
bias=10*log10((mean(rhs)/mean(phidp))^(1/a));
bias_str=num2str(bias);
variance=std(rhs-phidp);
figure;scatter(phidp,rhs,'k','+'); 
xlabel('Estimated PHI, deg','FontSize',14)
ylabel('Measured PHI, deg','FontSize',14)
title(strcat('Mean bias = ',bias_str,', dB'))
%grid on
hold on
x=0:max(max(phidp)); y=x; plot(x,y,'k')

hold off
