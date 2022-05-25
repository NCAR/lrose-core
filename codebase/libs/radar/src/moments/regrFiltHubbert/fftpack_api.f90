!=======================================================================
! FFTPack
!-----------------------------------------------------------------------
! FFTPack aims to provide an easily usable package of functions using
! FFTPACK library (Fortran 77).
!
! Created by
!     Keurfon Luu <keurfon.luu@mines-paristech.fr>
!     MINES ParisTech - Centre de Géosciences
!     PSL - Research University
!
! Last updated
!     2016-12-20 15:22
!
! Functions
!-----------------------------------------------------------------------
!   C
!   --
!   -   conv              -   czt
!
!   F
!   --
!   -   fft               -   fftshift          -   filter
!   -   freq2ind
!
!   H
!   --
!   -   hilbert
!
!   I
!   --
!   -   ifft              -   ifftshift         -   ind2freq
!
!   X
!   --
!   -   xcorr
!=======================================================================

module fftpack

  implicit none

!=======================================================================
! Polymorphic functions and subroutines
!=======================================================================

  !---------------------------------------------------------------------
  ! Function conv
  !---------------------------------------------------------------------
  interface conv
    module procedure conv_c8, conv_r8, conv_c4, conv_r4
  end interface conv
  public :: conv
  private :: conv_c8, conv_r8, conv_c4, conv_r4

  !---------------------------------------------------------------------
  ! Function czt
  !---------------------------------------------------------------------
  interface czt
    module procedure czt_c8, czt_r8, czt_c4, czt_r4
  end interface czt
  public :: czt
  private :: czt_c8, czt_r8, czt_c4, czt_r4

  !---------------------------------------------------------------------
  ! Function fft
  !---------------------------------------------------------------------
  interface fft
    module procedure fft_c8, fft_r8, fft_c4, fft_r4
  end interface fft
  public :: fft
  private :: fft_c8, fft_r8, fft_c4, fft_r4

  !---------------------------------------------------------------------
  ! Function fftshift
  !---------------------------------------------------------------------
  interface fftshift
    module procedure fftshift_c8, fftshift_r8, fftshift_c4, fftshift_r4
  end interface fftshift
  public :: fftshift
  private :: fftshift_c8, fftshift_r8, fftshift_c4, fftshift_r4

  !---------------------------------------------------------------------
  ! Function filter
  !---------------------------------------------------------------------
  interface filter
    module procedure filter_r8, filter_r4
  end interface filter
  public :: filter
  private :: filter_r8, filter_r4

  !---------------------------------------------------------------------
  ! Function freq2ind
  !---------------------------------------------------------------------
  interface freq2ind
    module procedure freq2ind_r8, freq2ind_r4
  end interface freq2ind
  public :: freq2ind
  private :: freq2ind_r8, freq2ind_r4

  !---------------------------------------------------------------------
  ! Function hilbert
  !---------------------------------------------------------------------
  interface hilbert
    module procedure hilbert_r8, hilbert_r4
  end interface hilbert
  public :: hilbert
  private :: hilbert_r8, hilbert_r4

  !---------------------------------------------------------------------
  ! Function ifft
  !---------------------------------------------------------------------
  interface ifft
    module procedure ifft_c8, ifft_c4
  end interface ifft
  public :: ifft
  private :: ifft_c8, ifft_c4

  !---------------------------------------------------------------------
  ! Function ifftshift
  !---------------------------------------------------------------------
  interface ifftshift
    module procedure ifftshift_c8, ifftshift_r8, ifftshift_c4, ifftshift_r4
  end interface ifftshift
  public :: ifftshift
  private :: ifftshift_c8, ifftshift_r8, ifftshift_c4, ifftshift_r4

  !---------------------------------------------------------------------
  ! Function ind2freq
  !---------------------------------------------------------------------
  interface ind2freq
    module procedure ind2freq_r8, ind2freq_r4
  end interface ind2freq
  public :: ind2freq
  private :: ind2freq_r8, ind2freq_r4

  !---------------------------------------------------------------------
  ! Function xcorr
  !---------------------------------------------------------------------
  interface xcorr
    module procedure xcorr_c8, xcorr_r8, xcorr_c4, xcorr_r4
  end interface xcorr
  public :: xcorr
  private :: xcorr_c8, xcorr_r8, xcorr_c4, xcorr_r4

contains

!=======================================================================
! conv
!-----------------------------------------------------------------------
! conv computes the convolution of two vectors.
!
! Syntax
!-----------------------------------------------------------------------
! w = conv(x, y)
!
! Description
!-----------------------------------------------------------------------
! w = conv(x, y) returns the convolution of vectors x and y. If x and y
! are polynomial coefficients, convolving them is equivalent to
! multiplying the two polynomials.
!=======================================================================

  function conv_c8(x, y)
    complex(kind = 8), dimension(:), allocatable :: conv_c8
    complex(kind = 8), dimension(:), intent(in) :: x, y
    integer(kind = 4) :: n, nf

    n = size(x) + size(y) - 1
    nf = 2 ** ( ceiling( log( real(n) ) / log(2.) ) )
    conv_c8 = ifft(fft(x, nf) * fft(y, nf))
    conv_c8 = conv_c8(:n)
    return
  end function conv_c8

  function conv_r8(x, y)
    real(kind = 8), dimension(:), allocatable :: conv_r8
    real(kind = 8), dimension(:), intent(in) :: x, y
    integer(kind = 4) :: n, nf

    n = size(x) + size(y) - 1
    nf = 2 ** ( ceiling( log( real(n) ) / log(2.) ) )
    conv_r8 = ifft(fft(x, nf) * fft(y, nf))
    conv_r8 = conv_r8(:n)
    return
  end function conv_r8

  function conv_c4(x, y)
    complex(kind = 4), dimension(:), allocatable :: conv_c4
    complex(kind = 4), dimension(:), intent(in) :: x, y
    integer(kind = 4) :: n, nf

    n = size(x) + size(y) - 1
    nf = 2 ** ( ceiling( log( real(n) ) / log(2.) ) )
    conv_c4 = ifft(fft(x, n) * fft(y, n))
    conv_c4 = conv_c4(:n)
    return
  end function conv_c4

  function conv_r4(x, y)
    real(kind = 4), dimension(:), allocatable :: conv_r4
    real(kind = 4), dimension(:), intent(in) :: x, y
    integer(kind = 4) :: n, nf

    n = size(x) + size(y) - 1
    nf = 2 ** ( ceiling( log( real(n) ) / log(2.) ) )
    conv_r4 = ifft(fft(x, n) * fft(y, n))
    conv_r4 = conv_r4(:n)
    return
  end function conv_r4

!=======================================================================
! czt
!-----------------------------------------------------------------------
! czt performs the Chirp Z-transform.
!
! Syntax
!-----------------------------------------------------------------------
! y = czt(x)
! y = czt(x, n, w, a)
!
! Description
!-----------------------------------------------------------------------
! y = czt(x) returns the Chirp Z-transform of vector x. With default
! input arguments, it computes the Discrete Fourier Transform of the
! vector x of any size.
!
! y = czt(x, n, w, a) returns the Chirp Z-transform of vector x. The
! chirp is the defined by w and a, n being the length of the chirp.
!=======================================================================

  function czt_c8(x, n, w, a) result(y)
    complex(kind = 8), dimension(:), allocatable :: y
    complex(kind = 8), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    complex(kind = 8), intent(in), optional :: w
    real(kind = 8), intent(in), optional :: a
    integer(kind = 4) :: i, opt_n, m, nfft
    real(kind = 8) :: opt_a
    complex(kind = 8) :: opt_w
    real(kind = 8), dimension(:), allocatable :: ar
    complex(kind = 8), dimension(:), allocatable :: chirp, Wk2, AWk2, tmp
    real(kind = 8), parameter :: pi = 3.141592653589793238460d0

    m = size(x)
    if (.not. present(n)) then; opt_n = m; &
    else; opt_n = n; end if
    if (.not. present(w)) then; opt_w = exp(dcmplx(0.0d0, -2.0d0 * pi / opt_n)); &
    else; opt_w = w; end if
    if (.not. present(a)) then; opt_a = 1.0d0; &
    else; opt_a = a; end if

    allocate(ar(max(m, opt_n)+m-1))
    ar = [ ( i, i = 1-m, max(m, opt_n)-1 ) ]
    chirp = opt_w ** (0.5d0 * ar ** 2)
    deallocate(ar)

    nfft = 2 ** ( ceiling( log( real(m + opt_n - 1) ) / log(2.) ) )

    allocate(tmp(nfft - m), ar(m))
    tmp = 0.
    ar = [ ( i, i = 0, m-1 ) ]
    Wk2 = [ x * opt_a ** (-ar) * chirp(m:2*m-1), tmp ]
    deallocate(tmp)

    allocate(tmp(nfft - (m+opt_n-1)))
    tmp = 0.
    AWk2 = [ 1. / chirp(:m+opt_n-1), tmp ]
    deallocate(tmp)

    y = ifft(fft(Wk2) * fft(AWk2))
    y = y(m:m+opt_n-1) * chirp(m:m+opt_n-1)
    return
  end function czt_c8

  function czt_r8(x, n, w, a) result(y)
    complex(kind = 8), dimension(:), allocatable :: y
    real(kind = 8), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    complex(kind = 8), intent(in), optional :: w
    real(kind = 8), intent(in), optional :: a
    integer(kind = 4) :: opt_n
    real(kind = 8) :: opt_a
    complex(kind = 8) :: opt_w
    real(kind = 8), parameter :: pi = 3.141592653589793238460d0

    if (.not. present(n)) then; opt_n = size(x); &
    else; opt_n = n; end if
    if (.not. present(w)) then; opt_w = exp(dcmplx(0.0d0, -2.0d0 * pi / opt_n)); &
    else; opt_w = w; end if
    if (.not. present(a)) then; opt_a = 1.0d0; &
    else; opt_a = a; end if

    y = czt_c8(dcmplx(x), opt_n, opt_w, opt_a)
    return
  end function czt_r8

  function czt_c4(x, n, w, a) result(y)
    complex(kind = 4), dimension(:), allocatable :: y
    complex(kind = 4), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    complex(kind = 4), intent(in), optional :: w
    real(kind = 4), intent(in), optional :: a
    integer(kind = 4) :: opt_n
    real(kind = 8) :: opt_a
    complex(kind = 8) :: opt_w
    real(kind = 8), parameter :: pi = 3.141592653589793238460d0

    if (.not. present(n)) then; opt_n = size(x); &
    else; opt_n = n; end if
    if (.not. present(w)) then; opt_w = exp(cmplx(0.0d0, -2.0d0 * pi / opt_n)); &
    else; opt_w = w; end if
    if (.not. present(a)) then; opt_a = 1.0d0; &
    else; opt_a = a; end if

    y = czt_c8(dcmplx(x), opt_n, opt_w, opt_a)
    return
  end function czt_c4

  function czt_r4(x, n, w, a) result(y)
    complex(kind = 4), dimension(:), allocatable :: y
    real(kind = 4), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    complex(kind = 4), intent(in), optional :: w
    real(kind = 4), intent(in), optional :: a
    integer(kind = 4) :: opt_n
    real(kind = 8) :: opt_a
    complex(kind = 8) :: opt_w
    real(kind = 8), parameter :: pi = 3.141592653589793238460d0

    if (.not. present(n)) then; opt_n = size(x); &
    else; opt_n = n; end if
    if (.not. present(w)) then; opt_w = exp(cmplx(0.0d0, -2.0d0 * pi / opt_n)); &
    else; opt_w = w; end if
    if (.not. present(a)) then; opt_a = 1.0d0; &
    else; opt_a = a; end if

    y = czt_c8(dcmplx(x), opt_n, opt_w, opt_a)
    return
  end function czt_r4

!=======================================================================
! fft
!-----------------------------------------------------------------------
! fft performs the Fast Fourier Transform.
!
! Syntax
!-----------------------------------------------------------------------
! y = fft(x)
! y = fft(x, n)
!
! Description
!-----------------------------------------------------------------------
! y = fft(x) returns the Discrete Fourier Transform (DFT) of vector x
! using a Fast Fourier Transform algorithm.
!
! y = fft(x, n) returns the n-points DFT of vector x using a Fast
! Fourier Transform algorithm. If n is greater than the length of x, x
! is padded with trailing zeros to length n. If n is less than the
! length of x, x is truncated to the length n.
!=======================================================================

  function fft_c8(x, n) result(y)
    complex(kind = 8), dimension(:), allocatable :: y
    complex(kind = 8), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    integer(kind = 4) :: dim, lensav, lenwrk, ierr
    complex(kind = 8), dimension(:), allocatable :: wsave, work, tmp

    if (present(n)) then
      dim = n
      if (dim .le. size(x)) then
        y = x(:dim)
      elseif (dim .gt. size(x)) then
        y = x
        allocate(tmp(dim - size(x)))
        tmp = 0.
        y = [ x, tmp ]
        deallocate(tmp)
      end if
    else
      dim = size(x)
      y = x
    end if

    ! Initialize FFT
    !================
    lensav = 2*dim + int(log(real(dim, 8))/log(2.0d0)) + 4
    lenwrk = 2*dim
    allocate(wsave(lensav), work(lenwrk))
    call cfft1i(dim, wsave, lensav, ierr)

    ! Forward transformation
    !========================
    call cfft1f(dim, 1, y, dim, wsave, lensav, work, lenwrk, ierr)
    y = y * dim

    return
  end function fft_c8

!  recursive function fft_c8(x, n) result(y)
!    complex(kind = 8), dimension(:), allocatable :: y
!    complex(kind = 8), dimension(:), intent(in) :: x
!    integer(kind = 4), intent(in), optional :: n
!    integer(kind = 4) :: i, dim, dim_min
!    complex(kind = 8) :: t
!    complex(kind = 8), dimension(:), allocatable :: odd, even, fact, tmp
!    real(kind = 8), dimension(:,:), allocatable :: k
!    complex(kind = 8), dimension(:,:), allocatable :: M, ymat, &
!      odd_v, even_v, tmp1, tmp2
!    real(kind = 8), parameter :: pi = 3.141592653589793238460d0
!
!    if (present(n)) then
!      dim = n
!      if (dim .le. size(x)) then
!        y = x(:dim)
!      elseif (dim .gt. size(x)) then
!        allocate(tmp(dim - size(x)))
!        tmp = 0.
!        y = [ x, tmp ]
!      end if
!    else
!      dim = size(x)
!      y = x
!    end if
!
!    if (dim .eq. 1) then
!      return
!    elseif (iand(dim, dim-1) .eq. 0) then
!      odd = y(1:dim:2)
!      odd = fft_c8(odd)
!      even = y(2:dim:2)
!      even = fft_c8(even)
!      do i = 1, dim/2
!        t = exp(dcmplx(0.0d0, -2.0d0 * pi * (i-1) / dim)) * even(i)
!        y(i) = odd(i) + t
!        y(i+dim/2) = odd(i) - t
!      end do
!    else
!      y = czt(x, dim)
!    end if
!    return
!  end function fft_c8

  function fft_r8(x, n) result(y)
    complex(kind = 8), dimension(:), allocatable :: y
    real(kind = 8), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n

    if (present(n)) then
      y = fft_c8(dcmplx(x), n)
    else
      y = fft_c8(dcmplx(x))
    end if
    return
  end function fft_r8

  function fft_c4(x, n) result(y)
    complex(kind = 4), dimension(:), allocatable :: y
    complex(kind = 4), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n

    if (present(n)) then
      y = fft_c8(dcmplx(x), n)
    else
      y = fft_c8(dcmplx(x))
    end if
    return
  end function fft_c4

  function fft_r4(x, n) result(y)
    complex(kind = 4), dimension(:), allocatable :: y
    real(kind = 4), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n

    if (present(n)) then
      y = fft_c8(dcmplx(x), n)
    else
      y = fft_c8(dcmplx(x))
    end if
    return
  end function fft_r4

!=======================================================================
! fftshift
!-----------------------------------------------------------------------
! fftshift shifts zero-frequency component to center of spectrum.
!
! Syntax
!-----------------------------------------------------------------------
! y = fftshift(x)
!
! Description
!-----------------------------------------------------------------------
! y = fftshift(x) returns the symmetric vector y by moving the zero
! frequency component of the vector x to the center.
!=======================================================================

  function fftshift_c8(x)
    complex(kind = 8), dimension(:), allocatable :: fftshift_c8
    complex(kind = 8), dimension(:), intent(in) :: x

    fftshift_c8 = cshift(x, shift = -floor(0.5d0*size(x)))
    return
  end function fftshift_c8

  function fftshift_r8(x)
    real(kind = 8), dimension(:), allocatable :: fftshift_r8
    real(kind = 8), dimension(:), intent(in) :: x

    fftshift_r8 = cshift(x, shift = -floor(0.5d0*size(x)))
    return
  end function fftshift_r8

  function fftshift_c4(x)
    complex(kind = 4), dimension(:), allocatable :: fftshift_c4
    complex(kind = 4), dimension(:), intent(in) :: x

    fftshift_c4 = cshift(x, shift = -floor(0.5d0*size(x)))
    return
  end function fftshift_c4

  function fftshift_r4(x)
    real(kind = 4), dimension(:), allocatable :: fftshift_r4
    real(kind = 4), dimension(:), intent(in) :: x

    fftshift_r4 = cshift(x, shift = -floor(0.5d0*size(x)))
    return
  end function fftshift_r4

!=======================================================================
! filter
!-----------------------------------------------------------------------
! filter applies a frequency filter to the input signal.
!
! Syntax
!-----------------------------------------------------------------------
! y = filter(x, fs, method, fc)
! y = filter(x, fs, method, fc, [options = ])
!
! Description
!-----------------------------------------------------------------------
! y = filter(x, fs, method, fc) returns the filtered signal in the
! frequency band fc, given the sampling rate fs (Hz) and the method.
!
! Inputs
!-----------------------------------------------------------------------
! x                   Signal (real array)
! fs                  Sampling rate in Hz (real)
! method              Frequency filter method (integer)
!                       -   1   Low pass
!                       -   2   High pass
!                       -   3   Band pass
! fc                  Cutoff frequency range (real array)
!
! Options
!-----------------------------------------------------------------------
! taper = 2           Taper method (integer)
!                       -   1   Linear
!                       -   2   Sin
! taper_width = 0     Taper width (real)
!=======================================================================

  function filter_r8(x, fs, method, fc, taper, taper_width)

    ! Input arguments
    !=================
    real(kind = 8), dimension(:), allocatable :: filter_r8
    real(kind = 8), dimension(:), intent(in) :: x
    real(kind = 8), intent(in) :: fs
    integer(kind = 4), intent(in) :: method
    real(kind = 8), dimension(:), intent(in) :: fc
    integer(kind = 4), intent(in), optional :: taper
    real(kind = 8), intent(in), optional :: taper_width

    ! Local variables
    !=================
    integer(kind = 4) :: opt_taper, i, nf, ind1, ind2, indtap
    real(kind = 8) :: opt_taper_width, fc1, fc2, tmp, step, ratio
    real(kind = 8), dimension(:), allocatable :: filt
    complex(kind = 8), dimension(:), allocatable :: x_ft

    ! Optional arguments default values
    !===================================
    opt_taper = 2
    opt_taper_width = 0
    if (present(taper)) opt_taper = taper
    if (present(taper_width)) opt_taper_width = taper_width

    ! Set cutoff frequency
    !======================
    select case(method)

      ! Low pass
      case(1)
        fc1 = 0
        fc2 = fc(1)

      ! High pass
      case(2)
        fc1 = fc(1)
        fc2 = 0.5d0 * fs

      ! Band pass
      case(3)
        fc1 = fc(1)
        fc2 = fc(2)

    end select

    ! Switch cutoff frequencies if fc1 > fc2
    !========================================
    if ( fc1 .gt. fc2 ) then
      tmp = fc1
      fc1 = fc2
      fc2 = tmp
    end if

    ! Compute FFT padding to the next power of 2
    !============================================
    nf = 2 ** ( ceiling( log( real(size(x)) ) / log(2.) ) )
    x_ft = fft(x, nf)

    ! Put 1 inside the filter and 0 outside
    !=======================================
    allocate(filt(nf/2+1))
    filt = 0.0d0
    ind1 = freq2ind(fc1, fs, nf)
    ind2 = freq2ind(fc2, fs, nf)
    filt(ind1:ind2) = 1.0d0

    ! Taper to avoid ringing
    !========================
    if ( opt_taper_width .ne. 0.0d0 ) then
      indtap = freq2ind(opt_taper_width, fs, nf)

      select case(taper)

        ! Linear
        case(1)
          step = 1.0d0 / indtap
          do i = 1, indtap
            if ( ind1 - indtap + i .ge. 1 ) then
              filt(ind1-indtap+i) = step * (i-1)
            end if
            if ( ind2 + i .le. 0.5d0*nf ) then
              filt(ind2+i) = 1.0d0 - step * (i-1)
            end if
          end do

        ! Sin
        case(2)
          step = 90.0d0 / indtap
          ratio = 3.141592653589793238460d0 / 180.0d0
          do i = 1, indtap
            if ( ind1 - indtap + i .ge. 1 ) then
              filt(ind1-indtap+i) = sin( step * (i-1) * ratio )**2
            end if
            if ( ind2 + i .le. 0.5d0*nf ) then
              filt(ind2+i) = sin( ( 90.0d0 - step * (i-1) ) * ratio )**2
            end if
          end do

      end select
    end if

    filt = [ filt, filt(size(filt)-1:2:-1) ]

    ! Convolve x with the filter
    !============================
    filter_r8 = real(ifft( x_ft * filt ))
    filter_r8 = filter_r8(:size(x))

    return
  end function filter_r8

  function filter_r4(x, fs, method, fc, taper, taper_width)
    real(kind = 4), dimension(:), allocatable :: filter_r4
    real(kind = 4), dimension(:), intent(in) :: x
    real(kind = 4), intent(in) :: fs
    integer(kind = 4), intent(in) :: method
    real(kind = 4), dimension(:), intent(in) :: fc
    integer(kind = 4), intent(in), optional :: taper
    real(kind = 4), intent(in), optional :: taper_width
    integer(kind = 4) :: opt_taper
    real(kind = 8) :: opt_taper_width

    opt_taper = 2
    opt_taper_width = 0
    if (present(taper)) opt_taper = taper
    if (present(taper_width)) opt_taper_width = taper_width

    filter_r4 = filter_r8(dble(x), dble(fs), method, dble(fc), opt_taper, &
                          dble(opt_taper_width))
    return
  end function filter_r4

!=======================================================================
! freq2ind
!-----------------------------------------------------------------------
! freq2ind converts a frequency (Hz) into its corresponding sample index
! given a sampling rate fs (Hz) of the signal and the signal's total
! number of samples nf.
!
! Syntax
!-----------------------------------------------------------------------
! i = freq2ind(f, fs, nf)
!
! Description
!-----------------------------------------------------------------------
! i = freq2ind(f, fs, nf) returns the index of the frequency f given the
! sampling rate fs and the total number of samples nf.
!
! Example
!-----------------------------------------------------------------------
! ind = freq2ind(250., 500., 1200)
!     600
!=======================================================================

  integer(kind = 4) function freq2ind_r8(f, fs, nf)
    real(kind = 8), intent(in) :: f, fs
    integer(kind = 4), intent(in) :: nf
    real(kind = 8) :: f_Ny, df

    f_Ny = 0.5d0 * fs
    df = f_Ny / ( floor(0.5d0*nf) - 1 )
    freq2ind_r8 = floor(f/df) + 1
    return
  end function freq2ind_r8

  integer(kind = 4) function freq2ind_r4(f, fs, nf)
    real(kind = 4), intent(in) :: f, fs
    integer(kind = 4), intent(in) :: nf
    real(kind = 8) :: f_Ny, df

    f_Ny = 0.5d0 * fs
    df = f_Ny / ( floor(0.5d0*nf) - 1 )
    freq2ind_r4 = floor(f/df) + 1
    return
  end function freq2ind_r4

!=======================================================================
! hilbert
!-----------------------------------------------------------------------
! hilbert computes the discrete-time analytic signal using Hilbert
! transform.
!
! Syntax
!-----------------------------------------------------------------------
! y = hilbert(x)
! y = hilbert(x, n)
!
! Description
!-----------------------------------------------------------------------
! y = hilbert(x) returns the complex analytic signal of a real data
! signal.
!
! y = hilbert(x, n) returns the n points analytic signal.
!=======================================================================

  function hilbert_r8(x, n)
    complex(kind = 8), dimension(:), allocatable :: hilbert_r8
    real(kind = 8), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    integer(kind = 4) :: i, dim, nh
    real(kind = 8), dimension(:), allocatable :: H

    dim = size(x)
    if (present(n)) dim = n

    allocate(H(dim))
    H = 0.0d0
    H(1) = 1.0d0
    if (mod(dim, 2) .eq. 0) then
      nh = floor(0.5d0*dim)
      H(nh+1) = 1.0d0
      do i = 2, nh
        H(i) = 2.0d0
      end do
    else
      nh = ceiling(0.5d0*dim)
      do i = 2, nh
        H(i) = 2.0d0
      end do
    end if
    hilbert_r8 = ifft(fft(x, dim) * H)
    return
  end function hilbert_r8

  function hilbert_r4(x, n) result(y)
    real(kind = 4), dimension(:), allocatable :: y
    real(kind = 4), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n

    if (present(n)) then
      y = hilbert_r8(dble(x), n)
    else
      y = hilbert_r8(dble(x))
    end if
    return
  end function hilbert_r4

!=======================================================================
! ifft
!-----------------------------------------------------------------------
! ifft performs the Inverse Fast Fourier Transform.
!
! Syntax
!-----------------------------------------------------------------------
! y = ifft(x)
! y = ifft(x, n)
!
! Description
!-----------------------------------------------------------------------
! y = ifft(x) returns the Inverse Discrete Fourier Transform of vector x
! using the Fast Fourier Transform algorithm.
!
! y = ifft(x, n) returns the n points Inverse Discrete Fourier Transform
! of vector x.
!=======================================================================

  function ifft_c8(x, n) result(y)
    complex(kind = 8), dimension(:), allocatable :: y
    complex(kind = 8), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n
    integer(kind = 4) :: dim, lensav, lenwrk, ierr
    complex(kind = 8), dimension(:), allocatable :: wsave, work, tmp

    if (present(n)) then
      dim = n
      if (dim .le. size(x)) then
        y = x(:dim)
      elseif (dim .gt. size(x)) then
        y = x
        allocate(tmp(dim - size(x)))
        tmp = 0.
        y = [ x, tmp ]
        deallocate(tmp)
      end if
    else
      dim = size(x)
      y = x
    end if

    ! Initialize FFT
    !================
    lensav = 2*dim + int(log(real(dim, 8))/log(2.0d0)) + 4
    lenwrk = 2*dim
    allocate(wsave(lensav), work(lenwrk))
    call cfft1i(dim, wsave, lensav, ierr)

    ! Backward transformation
    !=========================
    call cfft1b(dim, 1, y, dim, wsave, lensav, work, lenwrk, ierr)
    y = y / dim

    return
  end function ifft_c8

!  function ifft_c8(x, n) result(y)
!    complex(kind = 8), dimension(:), allocatable :: y
!    complex(kind = 8), dimension(:), intent(in) :: x
!    integer(kind = 4), intent(in), optional :: n
!    integer(kind = 4) :: dim
!
!    dim = size(x)
!    if (present(n)) dim = n
!
!    y = fft(conjg(x), dim)
!    y = conjg(y) / size(y)
!    return
!  end function ifft_c8

  function ifft_c4(x, n) result(y)
    complex(kind = 4), dimension(:), allocatable :: y
    complex(kind = 4), dimension(:), intent(in) :: x
    integer(kind = 4), intent(in), optional :: n

    if (present(n)) then
      y = ifft_c8(dcmplx(x), n)
    else
      y = ifft_c8(dcmplx(x))
    end if
    return
  end function ifft_c4

!=======================================================================
! ifftshift
!-----------------------------------------------------------------------
! ifftshift shifts zero-frequency component to beginning of spectrum.
!
! Syntax
!-----------------------------------------------------------------------
! y = ifftshift(x)
!
! Description
!-----------------------------------------------------------------------
! y = ifftshift(x) returns the asymmetric vector y by moving the zero
! frequency component of the vector x to the beginning.
!=======================================================================

  function ifftshift_c8(x)
    complex(kind = 8), dimension(:), allocatable :: ifftshift_c8
    complex(kind = 8), dimension(:), intent(in) :: x

    ifftshift_c8 = cshift(x, shift = -ceiling(0.5d0*size(x)))
    return
  end function ifftshift_c8

  function ifftshift_r8(x)
    real(kind = 8), dimension(:), allocatable :: ifftshift_r8
    real(kind = 8), dimension(:), intent(in) :: x

    ifftshift_r8 = cshift(x, shift = -ceiling(0.5d0*size(x)))
    return
  end function ifftshift_r8

  function ifftshift_c4(x)
    complex(kind = 4), dimension(:), allocatable :: ifftshift_c4
    complex(kind = 4), dimension(:), intent(in) :: x

    ifftshift_c4 = cshift(x, shift = -ceiling(0.5d0*size(x)))
    return
  end function ifftshift_c4

  function ifftshift_r4(x)
    real(kind = 4), dimension(:), allocatable :: ifftshift_r4
    real(kind = 4), dimension(:), intent(in) :: x

    ifftshift_r4 = cshift(x, shift = -ceiling(0.5d0*size(x)))
    return
  end function ifftshift_r4

!=======================================================================
! ind2freq
!-----------------------------------------------------------------------
! ind2freq converts a sample index into its corresponding frequency
! given the sampling rate fs (Hz) of a signal and the signal's total
! number of samples nf.
!
! Syntax
!-----------------------------------------------------------------------
! f = ind2freq(i, fs, nf)
!
! Description
!-----------------------------------------------------------------------
! f = ind2freq(i, fs, nf) returns the frequency of the index i given the
! sampling rate fs and the total number of samples nf.
!
! Example
!-----------------------------------------------------------------------
! f = ind2freq(600, 500., 1200)
!     250.
!=======================================================================

  real(kind = 8) function ind2freq_r8(i, fs, nf)
    integer(kind = 4), intent(in) :: i
    real(kind = 8), intent(in) :: fs
    integer(kind = 4), intent(in) :: nf
    real(kind = 8) :: f_Ny, df

    f_Ny = 0.5d0 * fs
    df = f_Ny / ( floor(0.5d0*nf) - 1 )
    ind2freq_r8 = df * ( i - 1 )
    return
  end function ind2freq_r8

  real(kind = 4) function ind2freq_r4(i, fs, nf)
    integer(kind = 4), intent(in) :: i
    real(kind = 4), intent(in) :: fs
    integer(kind = 4), intent(in) :: nf
    real(kind = 8) :: f_Ny, df

    f_Ny = 0.5d0 * fs
    df = f_Ny / ( floor(0.5d0*nf) - 1 )
    ind2freq_r4 = df * ( i - 1 )
    return
  end function ind2freq_r4

!=======================================================================
! xcorr
!-----------------------------------------------------------------------
! xcorr computes the cross-correlation.
!
! Syntax
!-----------------------------------------------------------------------
! r = xcorr(x, y)
!
! Description
!-----------------------------------------------------------------------
! r = xcorr(x, y) returns the cross-correlation of two discrete-time
! sequences x and y.
!=======================================================================

  function xcorr_c8(x, y)
    complex(kind = 8), dimension(:), allocatable :: xcorr_c8
    complex(kind = 8), dimension(:), intent(in) :: x, y
    integer(kind = 4) :: n1, n2, n, nf

    n1 = size(x)
    n2 = size(y)
    n = max(n1, n2)
    nf = 2 ** ( ceiling( log( real(2*n-1) ) / log(2.) ) )
    xcorr_c8 = ifft(fft(x, nf) * conjg(fft(y, nf)))
    xcorr_c8 = [ xcorr_c8(nf-n+2:), xcorr_c8(:n) ]
    return
  end function xcorr_c8

  function xcorr_r8(x, y)
    real(kind = 8), dimension(:), allocatable :: xcorr_r8
    real(kind = 8), dimension(:), intent(in) :: x, y

    xcorr_r8 = xcorr_c8(dcmplx(x), dcmplx(y))
    return
  end function xcorr_r8

  function xcorr_c4(x, y)
    complex(kind = 4), dimension(:), allocatable :: xcorr_c4
    complex(kind = 4), dimension(:), intent(in) :: x, y

    xcorr_c4 = xcorr_c8(dcmplx(x), dcmplx(y))
    return
  end function xcorr_c4

  function xcorr_r4(x, y)
    real(kind = 4), dimension(:), allocatable :: xcorr_r4
    real(kind = 4), dimension(:), intent(in) :: x, y

    xcorr_r4 = xcorr_c8(dcmplx(x), dcmplx(y))
    return
  end function xcorr_r4

end module fftpack
