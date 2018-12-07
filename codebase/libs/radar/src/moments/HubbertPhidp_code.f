c  THE ORIGINAL CHILL DATA HAD THE V TS AS THE LEADING SAMPLE
C  AND THE ORIGINAL CALCULATION IS BASED ON THAT. SPOL DATA HAS THE
C  H LEADING THE V AND THUS THIS NEEDS TO BE ACCOUNTED FOR BELOW

c--------

        call xcor0(vi,vq,ui,uq,nn,a,b) !SPOL  -Vel + PHIDP
        call xcor1(ui,uq,vi,vq,nn,c,d) !SPOL  -Vel - PHIDP

c-------- these are auto correlations lag1 so no change needed for SPOL

        call xcor1(ui,uq,ui,uq,nn,e,f)
        call xcor1(vi,vq,vi,vq,nn,g,h)

        r2hhhh=(e**2+f**2)**.5
        r2hhhh=r2hhhh/varu
        r2vvvv=(g**2+h**2)**.5
        r2vvvv=r2vvvv/varv


        rouv=((a**2+b**2)**.5+(c**2+d**2)**.5)/2
        rouv=rouv/((varv*varu)**.5)
        r1hhvv=rouv
        ro11=(r2hhhh+ r2vvvv)/2
        rouv=rouv/(ro11**.25)
        r0hhvv=rouv

c

C        phhvv= atan2(b,a)*rad2deg   ! -Vel + PHIDP
C        pvvhh= atan2(d,c)*rad2deg   ! -VEL + -PHIDP

C
C       PUT IN PHASE OFFSET ON THE TWO VVHH LAG1 COVARIANCES SO THAT
C       PHIDP STARTS AT ABOUT -70
C

C       THE TWO LAG 1 VVHH ARE

        tmpc = cmplx(a,b)*cexp(-j*vvhh_offset)
        tmpc1 = cmplx(c,d)*cexp(j*vvhh_offset)

        tmpc2= tmpc*conjg(tmpc1)     !2PHIDP
        pdp = atan2(aimag(tmpc2),real(tmpc2))/2


c       SUBTRACT PHIDP FROM THE TWO LAG1S

c

        tmpc=tmpc*conjg(csqrt(tmpc2))
        tmpc1=tmpc1*(csqrt(tmpc2))

c  AT THIS POINT tmpc and tmpc1 both have only the velocity and it should be negative velocity 12/5/2018
c      NOW GET DOPPLER VELOCITY
C


        dop=atan2(aimag(tmpc),real(tmpc))
        dop1=atan2(aimag(tmpc1),real(tmpc1))
        dop=-(dop+dop1)/2
        pdp=pdp*180/pi
        phidp = pdp

        lambda=.1067 ! meters
        const=lambda/(4.*pi*Ts)
        co_dop_dg=dop*rad2deg
        dop=const*dop

c_______________________________________________________________

        subroutine xcor0(x,y,x1,y1,n,a,b)

        real x(1025),y(1025),x1(1025),y1(1025)

        sumr=0.0
        sumi=0.0

        do 16 i=1,n
           sumr=sumr+x(i)*x1(i)+y(i)*y1(i)
           sumi=sumi+y(i)*x1(i)-x(i)*y1(i)
16      continue

        a=sumr/n
        b=sumi/n

        return

        end

c--------------------------------------------------

        subroutine xcor1(x,y,x1,y1,n,a,b)

        real x(n),y(n),x1(n),y1(n)

        sumr=0.0
        sumi=0.0

        do 16 i=1,n-1
           sumr=sumr+x(i+1)*x1(i)+y(i+1)*y1(i)
           sumi=sumi+y(i+1)*x1(i)-x(i+1)*y1(i)
16      continue

        a=sumr/(n)
        b=sumi/(n)

        return

        end
