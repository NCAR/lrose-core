# 0 "Nc3Values.cc"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "Nc3Values.cc"
# 42 "Nc3Values.cc"
# 1 "/usr/include/c++/11/iostream" 1 3
# 36 "/usr/include/c++/11/iostream" 3
       
# 37 "/usr/include/c++/11/iostream" 3

# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 1 3


# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 4 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 2 3
# 2439 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 3

# 2439 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 3
namespace std
{
  typedef long unsigned int size_t;
  typedef long int ptrdiff_t;


  typedef decltype(nullptr) nullptr_t;

}
# 2461 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 3
namespace std
{
  inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))) { }
}
namespace __gnu_cxx
{
  inline namespace __cxx11 __attribute__((__abi_tag__ ("cxx11"))) { }
}
# 2747 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 3
# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/os_defines.h" 1 3
# 39 "/usr/include/c++/11/x86_64-redhat-linux/bits/os_defines.h" 3
# 1 "/usr/include/features.h" 1 3 4
# 392 "/usr/include/features.h" 3 4
# 1 "/usr/include/features-time64.h" 1 3 4
# 20 "/usr/include/features-time64.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 21 "/usr/include/features-time64.h" 2 3 4
# 1 "/usr/include/bits/timesize.h" 1 3 4
# 22 "/usr/include/features-time64.h" 2 3 4
# 393 "/usr/include/features.h" 2 3 4
# 490 "/usr/include/features.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 551 "/usr/include/sys/cdefs.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 552 "/usr/include/sys/cdefs.h" 2 3 4
# 1 "/usr/include/bits/long-double.h" 1 3 4
# 553 "/usr/include/sys/cdefs.h" 2 3 4
# 491 "/usr/include/features.h" 2 3 4
# 514 "/usr/include/features.h" 3 4
# 1 "/usr/include/gnu/stubs.h" 1 3 4
# 10 "/usr/include/gnu/stubs.h" 3 4
# 1 "/usr/include/gnu/stubs-64.h" 1 3 4
# 11 "/usr/include/gnu/stubs.h" 2 3 4
# 515 "/usr/include/features.h" 2 3 4
# 40 "/usr/include/c++/11/x86_64-redhat-linux/bits/os_defines.h" 2 3
# 2748 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 2 3


# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/cpu_defines.h" 1 3
# 2751 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 2 3
# 2938 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 3
# 1 "/usr/include/c++/11/pstl/pstl_config.h" 1 3
# 2939 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++config.h" 2 3
# 39 "/usr/include/c++/11/iostream" 2 3
# 1 "/usr/include/c++/11/ostream" 1 3
# 36 "/usr/include/c++/11/ostream" 3
       
# 37 "/usr/include/c++/11/ostream" 3

# 1 "/usr/include/c++/11/ios" 1 3
# 36 "/usr/include/c++/11/ios" 3
       
# 37 "/usr/include/c++/11/ios" 3

# 1 "/usr/include/c++/11/iosfwd" 1 3
# 36 "/usr/include/c++/11/iosfwd" 3
       
# 37 "/usr/include/c++/11/iosfwd" 3


# 1 "/usr/include/c++/11/bits/stringfwd.h" 1 3
# 37 "/usr/include/c++/11/bits/stringfwd.h" 3
       
# 38 "/usr/include/c++/11/bits/stringfwd.h" 3


# 1 "/usr/include/c++/11/bits/memoryfwd.h" 1 3
# 46 "/usr/include/c++/11/bits/memoryfwd.h" 3
       
# 47 "/usr/include/c++/11/bits/memoryfwd.h" 3



namespace std __attribute__ ((__visibility__ ("default")))
{

# 63 "/usr/include/c++/11/bits/memoryfwd.h" 3
  template<typename>
    class allocator;

  template<>
    class allocator<void>;



  template<typename, typename>
    struct uses_allocator;

  template<typename>
    struct allocator_traits;





}
# 41 "/usr/include/c++/11/bits/stringfwd.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{








  template<class _CharT>
    struct char_traits;

  template<> struct char_traits<char>;


  template<> struct char_traits<wchar_t>;







  template<> struct char_traits<char16_t>;
  template<> struct char_traits<char32_t>;


namespace __cxx11 {

  template<typename _CharT, typename _Traits = char_traits<_CharT>,
           typename _Alloc = allocator<_CharT> >
    class basic_string;

}


  typedef basic_string<char> string;



  typedef basic_string<wchar_t> wstring;
# 93 "/usr/include/c++/11/bits/stringfwd.h" 3
  typedef basic_string<char16_t> u16string;


  typedef basic_string<char32_t> u32string;





}
# 40 "/usr/include/c++/11/iosfwd" 2 3
# 1 "/usr/include/c++/11/bits/postypes.h" 1 3
# 38 "/usr/include/c++/11/bits/postypes.h" 3
       
# 39 "/usr/include/c++/11/bits/postypes.h" 3

# 1 "/usr/include/c++/11/cwchar" 1 3
# 39 "/usr/include/c++/11/cwchar" 3
       
# 40 "/usr/include/c++/11/cwchar" 3




# 1 "/usr/include/wchar.h" 1 3 4
# 27 "/usr/include/wchar.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 28 "/usr/include/wchar.h" 2 3 4


# 1 "/usr/include/bits/floatn.h" 1 3 4
# 74 "/usr/include/bits/floatn.h" 3 4
typedef _Complex float __cfloat128 __attribute__ ((__mode__ (__TC__)));
# 86 "/usr/include/bits/floatn.h" 3 4
typedef __float128 _Float128;
# 119 "/usr/include/bits/floatn.h" 3 4
# 1 "/usr/include/bits/floatn-common.h" 1 3 4
# 24 "/usr/include/bits/floatn-common.h" 3 4
# 1 "/usr/include/bits/long-double.h" 1 3 4
# 25 "/usr/include/bits/floatn-common.h" 2 3 4
# 214 "/usr/include/bits/floatn-common.h" 3 4
typedef float _Float32;
# 251 "/usr/include/bits/floatn-common.h" 3 4
typedef double _Float64;
# 268 "/usr/include/bits/floatn-common.h" 3 4
typedef double _Float32x;
# 285 "/usr/include/bits/floatn-common.h" 3 4
typedef long double _Float64x;
# 120 "/usr/include/bits/floatn.h" 2 3 4
# 31 "/usr/include/wchar.h" 2 3 4




# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 209 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 3 4
typedef long unsigned int size_t;
# 36 "/usr/include/wchar.h" 2 3 4


# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stdarg.h" 1 3 4
# 40 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 39 "/usr/include/wchar.h" 2 3 4

# 1 "/usr/include/bits/wchar.h" 1 3 4
# 41 "/usr/include/wchar.h" 2 3 4
# 1 "/usr/include/bits/types/wint_t.h" 1 3 4
# 20 "/usr/include/bits/types/wint_t.h" 3 4
typedef unsigned int wint_t;
# 42 "/usr/include/wchar.h" 2 3 4
# 1 "/usr/include/bits/types/mbstate_t.h" 1 3 4



# 1 "/usr/include/bits/types/__mbstate_t.h" 1 3 4
# 13 "/usr/include/bits/types/__mbstate_t.h" 3 4
typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 5 "/usr/include/bits/types/mbstate_t.h" 2 3 4

typedef __mbstate_t mbstate_t;
# 43 "/usr/include/wchar.h" 2 3 4
# 1 "/usr/include/bits/types/__FILE.h" 1 3 4



struct _IO_FILE;
typedef struct _IO_FILE __FILE;
# 44 "/usr/include/wchar.h" 2 3 4


# 1 "/usr/include/bits/types/FILE.h" 1 3 4



struct _IO_FILE;


typedef struct _IO_FILE FILE;
# 47 "/usr/include/wchar.h" 2 3 4


# 1 "/usr/include/bits/types/locale_t.h" 1 3 4
# 22 "/usr/include/bits/types/locale_t.h" 3 4
# 1 "/usr/include/bits/types/__locale_t.h" 1 3 4
# 28 "/usr/include/bits/types/__locale_t.h" 3 4
struct __locale_struct
{

  struct __locale_data *__locales[13];


  const unsigned short int *__ctype_b;
  const int *__ctype_tolower;
  const int *__ctype_toupper;


  const char *__names[13];
};

typedef struct __locale_struct *__locale_t;
# 23 "/usr/include/bits/types/locale_t.h" 2 3 4

typedef __locale_t locale_t;
# 50 "/usr/include/wchar.h" 2 3 4
# 79 "/usr/include/wchar.h" 3 4
extern "C" {



struct tm;



extern wchar_t *wcscpy (wchar_t *__restrict __dest,
   const wchar_t *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern wchar_t *wcsncpy (wchar_t *__restrict __dest,
    const wchar_t *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern wchar_t *wcscat (wchar_t *__restrict __dest,
   const wchar_t *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern wchar_t *wcsncat (wchar_t *__restrict __dest,
    const wchar_t *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int wcscmp (const wchar_t *__s1, const wchar_t *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int wcsncmp (const wchar_t *__s1, const wchar_t *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));



extern int wcscasecmp (const wchar_t *__s1, const wchar_t *__s2) noexcept (true);


extern int wcsncasecmp (const wchar_t *__s1, const wchar_t *__s2,
   size_t __n) noexcept (true);



extern int wcscasecmp_l (const wchar_t *__s1, const wchar_t *__s2,
    locale_t __loc) noexcept (true);

extern int wcsncasecmp_l (const wchar_t *__s1, const wchar_t *__s2,
     size_t __n, locale_t __loc) noexcept (true);




extern int wcscoll (const wchar_t *__s1, const wchar_t *__s2) noexcept (true);



extern size_t wcsxfrm (wchar_t *__restrict __s1,
         const wchar_t *__restrict __s2, size_t __n) noexcept (true);







extern int wcscoll_l (const wchar_t *__s1, const wchar_t *__s2,
        locale_t __loc) noexcept (true);




extern size_t wcsxfrm_l (wchar_t *__s1, const wchar_t *__s2,
    size_t __n, locale_t __loc) noexcept (true);


extern wchar_t *wcsdup (const wchar_t *__s) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (__builtin_free, 1)));




extern "C++" wchar_t *wcschr (wchar_t *__wcs, wchar_t __wc)
     noexcept (true) __asm ("wcschr") __attribute__ ((__pure__));
extern "C++" const wchar_t *wcschr (const wchar_t *__wcs, wchar_t __wc)
     noexcept (true) __asm ("wcschr") __attribute__ ((__pure__));






extern "C++" wchar_t *wcsrchr (wchar_t *__wcs, wchar_t __wc)
     noexcept (true) __asm ("wcsrchr") __attribute__ ((__pure__));
extern "C++" const wchar_t *wcsrchr (const wchar_t *__wcs, wchar_t __wc)
     noexcept (true) __asm ("wcsrchr") __attribute__ ((__pure__));
# 182 "/usr/include/wchar.h" 3 4
extern wchar_t *wcschrnul (const wchar_t *__s, wchar_t __wc)
     noexcept (true) __attribute__ ((__pure__));




extern size_t wcscspn (const wchar_t *__wcs, const wchar_t *__reject)
     noexcept (true) __attribute__ ((__pure__));


extern size_t wcsspn (const wchar_t *__wcs, const wchar_t *__accept)
     noexcept (true) __attribute__ ((__pure__));


extern "C++" wchar_t *wcspbrk (wchar_t *__wcs, const wchar_t *__accept)
     noexcept (true) __asm ("wcspbrk") __attribute__ ((__pure__));
extern "C++" const wchar_t *wcspbrk (const wchar_t *__wcs,
         const wchar_t *__accept)
     noexcept (true) __asm ("wcspbrk") __attribute__ ((__pure__));






extern "C++" wchar_t *wcsstr (wchar_t *__haystack, const wchar_t *__needle)
     noexcept (true) __asm ("wcsstr") __attribute__ ((__pure__));
extern "C++" const wchar_t *wcsstr (const wchar_t *__haystack,
        const wchar_t *__needle)
     noexcept (true) __asm ("wcsstr") __attribute__ ((__pure__));






extern wchar_t *wcstok (wchar_t *__restrict __s,
   const wchar_t *__restrict __delim,
   wchar_t **__restrict __ptr) noexcept (true);


extern size_t wcslen (const wchar_t *__s) noexcept (true) __attribute__ ((__pure__));




extern "C++" wchar_t *wcswcs (wchar_t *__haystack, const wchar_t *__needle)
     noexcept (true) __asm ("wcswcs") __attribute__ ((__pure__));
extern "C++" const wchar_t *wcswcs (const wchar_t *__haystack,
        const wchar_t *__needle)
     noexcept (true) __asm ("wcswcs") __attribute__ ((__pure__));
# 241 "/usr/include/wchar.h" 3 4
extern size_t wcsnlen (const wchar_t *__s, size_t __maxlen)
     noexcept (true) __attribute__ ((__pure__));





extern "C++" wchar_t *wmemchr (wchar_t *__s, wchar_t __c, size_t __n)
     noexcept (true) __asm ("wmemchr") __attribute__ ((__pure__));
extern "C++" const wchar_t *wmemchr (const wchar_t *__s, wchar_t __c,
         size_t __n)
     noexcept (true) __asm ("wmemchr") __attribute__ ((__pure__));






extern int wmemcmp (const wchar_t *__s1, const wchar_t *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__));


extern wchar_t *wmemcpy (wchar_t *__restrict __s1,
    const wchar_t *__restrict __s2, size_t __n) noexcept (true);



extern wchar_t *wmemmove (wchar_t *__s1, const wchar_t *__s2, size_t __n)
     noexcept (true);


extern wchar_t *wmemset (wchar_t *__s, wchar_t __c, size_t __n) noexcept (true);




extern wchar_t *wmempcpy (wchar_t *__restrict __s1,
     const wchar_t *__restrict __s2, size_t __n)
     noexcept (true);





extern wint_t btowc (int __c) noexcept (true);



extern int wctob (wint_t __c) noexcept (true);



extern int mbsinit (const mbstate_t *__ps) noexcept (true) __attribute__ ((__pure__));



extern size_t mbrtowc (wchar_t *__restrict __pwc,
         const char *__restrict __s, size_t __n,
         mbstate_t *__restrict __p) noexcept (true);


extern size_t wcrtomb (char *__restrict __s, wchar_t __wc,
         mbstate_t *__restrict __ps) noexcept (true);


extern size_t __mbrlen (const char *__restrict __s, size_t __n,
   mbstate_t *__restrict __ps) noexcept (true);
extern size_t mbrlen (const char *__restrict __s, size_t __n,
        mbstate_t *__restrict __ps) noexcept (true);
# 338 "/usr/include/wchar.h" 3 4
extern size_t mbsrtowcs (wchar_t *__restrict __dst,
    const char **__restrict __src, size_t __len,
    mbstate_t *__restrict __ps) noexcept (true);



extern size_t wcsrtombs (char *__restrict __dst,
    const wchar_t **__restrict __src, size_t __len,
    mbstate_t *__restrict __ps) noexcept (true);





extern size_t mbsnrtowcs (wchar_t *__restrict __dst,
     const char **__restrict __src, size_t __nmc,
     size_t __len, mbstate_t *__restrict __ps) noexcept (true);



extern size_t wcsnrtombs (char *__restrict __dst,
     const wchar_t **__restrict __src,
     size_t __nwc, size_t __len,
     mbstate_t *__restrict __ps) noexcept (true);






extern int wcwidth (wchar_t __c) noexcept (true);



extern int wcswidth (const wchar_t *__s, size_t __n) noexcept (true);





extern double wcstod (const wchar_t *__restrict __nptr,
        wchar_t **__restrict __endptr) noexcept (true);



extern float wcstof (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr) noexcept (true);
extern long double wcstold (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr) noexcept (true);
# 397 "/usr/include/wchar.h" 3 4
extern _Float32 wcstof32 (const wchar_t *__restrict __nptr,
     wchar_t **__restrict __endptr) noexcept (true);



extern _Float64 wcstof64 (const wchar_t *__restrict __nptr,
     wchar_t **__restrict __endptr) noexcept (true);



extern _Float128 wcstof128 (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr) noexcept (true);



extern _Float32x wcstof32x (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr) noexcept (true);



extern _Float64x wcstof64x (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr) noexcept (true);
# 429 "/usr/include/wchar.h" 3 4
extern long int wcstol (const wchar_t *__restrict __nptr,
   wchar_t **__restrict __endptr, int __base) noexcept (true);



extern unsigned long int wcstoul (const wchar_t *__restrict __nptr,
      wchar_t **__restrict __endptr, int __base)
     noexcept (true);




__extension__
extern long long int wcstoll (const wchar_t *__restrict __nptr,
         wchar_t **__restrict __endptr, int __base)
     noexcept (true);



__extension__
extern unsigned long long int wcstoull (const wchar_t *__restrict __nptr,
     wchar_t **__restrict __endptr,
     int __base) noexcept (true);





__extension__
extern long long int wcstoq (const wchar_t *__restrict __nptr,
        wchar_t **__restrict __endptr, int __base)
     noexcept (true);



__extension__
extern unsigned long long int wcstouq (const wchar_t *__restrict __nptr,
           wchar_t **__restrict __endptr,
           int __base) noexcept (true);






extern long int wcstol_l (const wchar_t *__restrict __nptr,
     wchar_t **__restrict __endptr, int __base,
     locale_t __loc) noexcept (true);

extern unsigned long int wcstoul_l (const wchar_t *__restrict __nptr,
        wchar_t **__restrict __endptr,
        int __base, locale_t __loc) noexcept (true);

__extension__
extern long long int wcstoll_l (const wchar_t *__restrict __nptr,
    wchar_t **__restrict __endptr,
    int __base, locale_t __loc) noexcept (true);

__extension__
extern unsigned long long int wcstoull_l (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr,
       int __base, locale_t __loc)
     noexcept (true);

extern double wcstod_l (const wchar_t *__restrict __nptr,
   wchar_t **__restrict __endptr, locale_t __loc)
     noexcept (true);

extern float wcstof_l (const wchar_t *__restrict __nptr,
         wchar_t **__restrict __endptr, locale_t __loc)
     noexcept (true);

extern long double wcstold_l (const wchar_t *__restrict __nptr,
         wchar_t **__restrict __endptr,
         locale_t __loc) noexcept (true);
# 512 "/usr/include/wchar.h" 3 4
extern _Float32 wcstof32_l (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr,
       locale_t __loc) noexcept (true);



extern _Float64 wcstof64_l (const wchar_t *__restrict __nptr,
       wchar_t **__restrict __endptr,
       locale_t __loc) noexcept (true);



extern _Float128 wcstof128_l (const wchar_t *__restrict __nptr,
         wchar_t **__restrict __endptr,
         locale_t __loc) noexcept (true);



extern _Float32x wcstof32x_l (const wchar_t *__restrict __nptr,
         wchar_t **__restrict __endptr,
         locale_t __loc) noexcept (true);



extern _Float64x wcstof64x_l (const wchar_t *__restrict __nptr,
         wchar_t **__restrict __endptr,
         locale_t __loc) noexcept (true);
# 552 "/usr/include/wchar.h" 3 4
extern wchar_t *wcpcpy (wchar_t *__restrict __dest,
   const wchar_t *__restrict __src) noexcept (true);



extern wchar_t *wcpncpy (wchar_t *__restrict __dest,
    const wchar_t *__restrict __src, size_t __n)
     noexcept (true);
# 581 "/usr/include/wchar.h" 3 4
extern __FILE *open_wmemstream (wchar_t **__bufloc, size_t *__sizeloc) noexcept (true)
  __attribute__ ((__malloc__)) ;





extern int fwide (__FILE *__fp, int __mode) noexcept (true);






extern int fwprintf (__FILE *__restrict __stream,
       const wchar_t *__restrict __format, ...)
                                                           ;




extern int wprintf (const wchar_t *__restrict __format, ...)
                                                           ;

extern int swprintf (wchar_t *__restrict __s, size_t __n,
       const wchar_t *__restrict __format, ...)
     noexcept (true) ;





extern int vfwprintf (__FILE *__restrict __s,
        const wchar_t *__restrict __format,
        __gnuc_va_list __arg)
                                                           ;




extern int vwprintf (const wchar_t *__restrict __format,
       __gnuc_va_list __arg)
                                                           ;


extern int vswprintf (wchar_t *__restrict __s, size_t __n,
        const wchar_t *__restrict __format,
        __gnuc_va_list __arg)
     noexcept (true) ;






extern int fwscanf (__FILE *__restrict __stream,
      const wchar_t *__restrict __format, ...)
                                                          ;




extern int wscanf (const wchar_t *__restrict __format, ...)
                                                          ;

extern int swscanf (const wchar_t *__restrict __s,
      const wchar_t *__restrict __format, ...)
     noexcept (true) ;
# 657 "/usr/include/wchar.h" 3 4
extern int fwscanf (__FILE *__restrict __stream, const wchar_t *__restrict __format, ...) __asm__ ("" "__isoc99_fwscanf")


                                                          ;
extern int wscanf (const wchar_t *__restrict __format, ...) __asm__ ("" "__isoc99_wscanf")

                                                          ;
extern int swscanf (const wchar_t *__restrict __s, const wchar_t *__restrict __format, ...) noexcept (true) __asm__ ("" "__isoc99_swscanf")


                                                          ;
# 688 "/usr/include/wchar.h" 3 4
extern int vfwscanf (__FILE *__restrict __s,
       const wchar_t *__restrict __format,
       __gnuc_va_list __arg)
                                                          ;




extern int vwscanf (const wchar_t *__restrict __format,
      __gnuc_va_list __arg)
                                                          ;

extern int vswscanf (const wchar_t *__restrict __s,
       const wchar_t *__restrict __format,
       __gnuc_va_list __arg)
     noexcept (true) ;







extern int vfwscanf (__FILE *__restrict __s, const wchar_t *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfwscanf")


                                                          ;
extern int vwscanf (const wchar_t *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vwscanf")

                                                          ;
extern int vswscanf (const wchar_t *__restrict __s, const wchar_t *__restrict __format, __gnuc_va_list __arg) noexcept (true) __asm__ ("" "__isoc99_vswscanf")


                                                          ;
# 744 "/usr/include/wchar.h" 3 4
extern wint_t fgetwc (__FILE *__stream);
extern wint_t getwc (__FILE *__stream);





extern wint_t getwchar (void);






extern wint_t fputwc (wchar_t __wc, __FILE *__stream);
extern wint_t putwc (wchar_t __wc, __FILE *__stream);





extern wint_t putwchar (wchar_t __wc);







extern wchar_t *fgetws (wchar_t *__restrict __ws, int __n,
   __FILE *__restrict __stream);





extern int fputws (const wchar_t *__restrict __ws,
     __FILE *__restrict __stream);






extern wint_t ungetwc (wint_t __wc, __FILE *__stream);
# 799 "/usr/include/wchar.h" 3 4
extern wint_t getwc_unlocked (__FILE *__stream);
extern wint_t getwchar_unlocked (void);







extern wint_t fgetwc_unlocked (__FILE *__stream);







extern wint_t fputwc_unlocked (wchar_t __wc, __FILE *__stream);
# 825 "/usr/include/wchar.h" 3 4
extern wint_t putwc_unlocked (wchar_t __wc, __FILE *__stream);
extern wint_t putwchar_unlocked (wchar_t __wc);
# 835 "/usr/include/wchar.h" 3 4
extern wchar_t *fgetws_unlocked (wchar_t *__restrict __ws, int __n,
     __FILE *__restrict __stream);







extern int fputws_unlocked (const wchar_t *__restrict __ws,
       __FILE *__restrict __stream);






extern size_t wcsftime (wchar_t *__restrict __s, size_t __maxsize,
   const wchar_t *__restrict __format,
   const struct tm *__restrict __tp) noexcept (true);




extern size_t wcsftime_l (wchar_t *__restrict __s, size_t __maxsize,
     const wchar_t *__restrict __format,
     const struct tm *__restrict __tp,
     locale_t __loc) noexcept (true);
# 882 "/usr/include/wchar.h" 3 4
}
# 45 "/usr/include/c++/11/cwchar" 2 3
# 62 "/usr/include/c++/11/cwchar" 3
namespace std
{
  using ::mbstate_t;
}
# 135 "/usr/include/c++/11/cwchar" 3
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{


  using ::wint_t;

  using ::btowc;
  using ::fgetwc;
  using ::fgetws;
  using ::fputwc;
  using ::fputws;
  using ::fwide;
  using ::fwprintf;
  using ::fwscanf;
  using ::getwc;
  using ::getwchar;
  using ::mbrlen;
  using ::mbrtowc;
  using ::mbsinit;
  using ::mbsrtowcs;
  using ::putwc;
  using ::putwchar;

  using ::swprintf;

  using ::swscanf;
  using ::ungetwc;
  using ::vfwprintf;

  using ::vfwscanf;


  using ::vswprintf;


  using ::vswscanf;

  using ::vwprintf;

  using ::vwscanf;

  using ::wcrtomb;
  using ::wcscat;
  using ::wcscmp;
  using ::wcscoll;
  using ::wcscpy;
  using ::wcscspn;
  using ::wcsftime;
  using ::wcslen;
  using ::wcsncat;
  using ::wcsncmp;
  using ::wcsncpy;
  using ::wcsrtombs;
  using ::wcsspn;
  using ::wcstod;

  using ::wcstof;

  using ::wcstok;
  using ::wcstol;
  using ::wcstoul;
  using ::wcsxfrm;
  using ::wctob;
  using ::wmemcmp;
  using ::wmemcpy;
  using ::wmemmove;
  using ::wmemset;
  using ::wprintf;
  using ::wscanf;
  using ::wcschr;
  using ::wcspbrk;
  using ::wcsrchr;
  using ::wcsstr;
  using ::wmemchr;
# 234 "/usr/include/c++/11/cwchar" 3

}
}







namespace __gnu_cxx
{





  using ::wcstold;
# 260 "/usr/include/c++/11/cwchar" 3
  using ::wcstoll;
  using ::wcstoull;

}

namespace std
{
  using ::__gnu_cxx::wcstold;
  using ::__gnu_cxx::wcstoll;
  using ::__gnu_cxx::wcstoull;
}
# 280 "/usr/include/c++/11/cwchar" 3
namespace std
{

  using std::wcstof;


  using std::vfwscanf;


  using std::vswscanf;


  using std::vwscanf;



  using std::wcstold;
  using std::wcstoll;
  using std::wcstoull;

}
# 41 "/usr/include/c++/11/bits/postypes.h" 2 3
# 68 "/usr/include/c++/11/bits/postypes.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{

# 88 "/usr/include/c++/11/bits/postypes.h" 3
  typedef long streamoff;
# 98 "/usr/include/c++/11/bits/postypes.h" 3
  typedef ptrdiff_t streamsize;
# 111 "/usr/include/c++/11/bits/postypes.h" 3
  template<typename _StateT>
    class fpos
    {
    private:
      streamoff _M_off;
      _StateT _M_state;

    public:




      fpos()
      : _M_off(0), _M_state() { }
# 133 "/usr/include/c++/11/bits/postypes.h" 3
      fpos(streamoff __off)
      : _M_off(__off), _M_state() { }


      fpos(const fpos&) = default;
      fpos& operator=(const fpos&) = default;
      ~fpos() = default;



      operator streamoff() const { return _M_off; }


      void
      state(_StateT __st)
      { _M_state = __st; }


      _StateT
      state() const
      { return _M_state; }





      fpos&
      operator+=(streamoff __off)
      {
 _M_off += __off;
 return *this;
      }





      fpos&
      operator-=(streamoff __off)
      {
 _M_off -= __off;
 return *this;
      }







      fpos
      operator+(streamoff __off) const
      {
 fpos __pos(*this);
 __pos += __off;
 return __pos;
      }







      fpos
      operator-(streamoff __off) const
      {
 fpos __pos(*this);
 __pos -= __off;
 return __pos;
      }






      streamoff
      operator-(const fpos& __other) const
      { return _M_off - __other._M_off; }
    };






  template<typename _StateT>
    inline bool
    operator==(const fpos<_StateT>& __lhs, const fpos<_StateT>& __rhs)
    { return streamoff(__lhs) == streamoff(__rhs); }

  template<typename _StateT>
    inline bool
    operator!=(const fpos<_StateT>& __lhs, const fpos<_StateT>& __rhs)
    { return streamoff(__lhs) != streamoff(__rhs); }





  typedef fpos<mbstate_t> streampos;

  typedef fpos<mbstate_t> wstreampos;
# 245 "/usr/include/c++/11/bits/postypes.h" 3
  typedef fpos<mbstate_t> u16streampos;

  typedef fpos<mbstate_t> u32streampos;



}
# 41 "/usr/include/c++/11/iosfwd" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{

# 74 "/usr/include/c++/11/iosfwd" 3
  class ios_base;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_ios;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_streambuf;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_istream;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_ostream;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_iostream;


namespace __cxx11 {

  template<typename _CharT, typename _Traits = char_traits<_CharT>,
     typename _Alloc = allocator<_CharT> >
    class basic_stringbuf;

  template<typename _CharT, typename _Traits = char_traits<_CharT>,
    typename _Alloc = allocator<_CharT> >
    class basic_istringstream;

  template<typename _CharT, typename _Traits = char_traits<_CharT>,
    typename _Alloc = allocator<_CharT> >
    class basic_ostringstream;

  template<typename _CharT, typename _Traits = char_traits<_CharT>,
    typename _Alloc = allocator<_CharT> >
    class basic_stringstream;

}

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_filebuf;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_ifstream;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_ofstream;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class basic_fstream;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class istreambuf_iterator;

  template<typename _CharT, typename _Traits = char_traits<_CharT> >
    class ostreambuf_iterator;



  typedef basic_ios<char> ios;


  typedef basic_streambuf<char> streambuf;


  typedef basic_istream<char> istream;


  typedef basic_ostream<char> ostream;


  typedef basic_iostream<char> iostream;


  typedef basic_stringbuf<char> stringbuf;


  typedef basic_istringstream<char> istringstream;


  typedef basic_ostringstream<char> ostringstream;


  typedef basic_stringstream<char> stringstream;


  typedef basic_filebuf<char> filebuf;


  typedef basic_ifstream<char> ifstream;


  typedef basic_ofstream<char> ofstream;


  typedef basic_fstream<char> fstream;



  typedef basic_ios<wchar_t> wios;


  typedef basic_streambuf<wchar_t> wstreambuf;


  typedef basic_istream<wchar_t> wistream;


  typedef basic_ostream<wchar_t> wostream;


  typedef basic_iostream<wchar_t> wiostream;


  typedef basic_stringbuf<wchar_t> wstringbuf;


  typedef basic_istringstream<wchar_t> wistringstream;


  typedef basic_ostringstream<wchar_t> wostringstream;


  typedef basic_stringstream<wchar_t> wstringstream;


  typedef basic_filebuf<wchar_t> wfilebuf;


  typedef basic_ifstream<wchar_t> wifstream;


  typedef basic_ofstream<wchar_t> wofstream;


  typedef basic_fstream<wchar_t> wfstream;
# 230 "/usr/include/c++/11/iosfwd" 3

}
# 39 "/usr/include/c++/11/ios" 2 3
# 1 "/usr/include/c++/11/exception" 1 3
# 33 "/usr/include/c++/11/exception" 3
       
# 34 "/usr/include/c++/11/exception" 3

#pragma GCC visibility push(default)


# 1 "/usr/include/c++/11/bits/exception.h" 1 3
# 34 "/usr/include/c++/11/bits/exception.h" 3
       
# 35 "/usr/include/c++/11/bits/exception.h" 3

#pragma GCC visibility push(default)



extern "C++" {

namespace std
{
# 61 "/usr/include/c++/11/bits/exception.h" 3
  class exception
  {
  public:
    exception() noexcept { }
    virtual ~exception() noexcept;

    exception(const exception&) = default;
    exception& operator=(const exception&) = default;
    exception(exception&&) = default;
    exception& operator=(exception&&) = default;




    virtual const char*
    what() const noexcept;
  };



}

}

#pragma GCC visibility pop
# 39 "/usr/include/c++/11/exception" 2 3

extern "C++" {

namespace std
{
# 53 "/usr/include/c++/11/exception" 3
  class bad_exception : public exception
  {
  public:
    bad_exception() noexcept { }



    virtual ~bad_exception() noexcept;


    virtual const char*
    what() const noexcept;
  };


  typedef void (*terminate_handler) ();


  typedef void (*unexpected_handler) ();


  terminate_handler set_terminate(terminate_handler) noexcept;



  terminate_handler get_terminate() noexcept;




  void terminate() noexcept __attribute__ ((__noreturn__));


  unexpected_handler set_unexpected(unexpected_handler) noexcept;



  unexpected_handler get_unexpected() noexcept;




  void unexpected() __attribute__ ((__noreturn__));
# 108 "/usr/include/c++/11/exception" 3
  __attribute__ ((__deprecated__ ("use '" "std::uncaught_exceptions()" "' instead")))
  bool uncaught_exception() noexcept __attribute__ ((__pure__));







  int uncaught_exceptions() noexcept __attribute__ ((__pure__));



}

namespace __gnu_cxx
{

# 143 "/usr/include/c++/11/exception" 3
  void __verbose_terminate_handler();


}

}

#pragma GCC visibility pop


# 1 "/usr/include/c++/11/bits/exception_ptr.h" 1 3
# 34 "/usr/include/c++/11/bits/exception_ptr.h" 3
#pragma GCC visibility push(default)


# 1 "/usr/include/c++/11/bits/exception_defines.h" 1 3
# 38 "/usr/include/c++/11/bits/exception_ptr.h" 2 3
# 1 "/usr/include/c++/11/bits/cxxabi_init_exception.h" 1 3
# 34 "/usr/include/c++/11/bits/cxxabi_init_exception.h" 3
       
# 35 "/usr/include/c++/11/bits/cxxabi_init_exception.h" 3

#pragma GCC visibility push(default)

# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 143 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 3 4
typedef long int ptrdiff_t;
# 415 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 3 4
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
# 426 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 3 4
} max_align_t;






  typedef decltype(nullptr) nullptr_t;
# 39 "/usr/include/c++/11/bits/cxxabi_init_exception.h" 2 3
# 50 "/usr/include/c++/11/bits/cxxabi_init_exception.h" 3
namespace std
{
  class type_info;
}

namespace __cxxabiv1
{
  struct __cxa_refcounted_exception;

  extern "C"
    {

      void*
      __cxa_allocate_exception(size_t) noexcept;

      void
      __cxa_free_exception(void*) noexcept;


      __cxa_refcounted_exception*
      __cxa_init_primary_exception(void *object, std::type_info *tinfo,
                void ( *dest) (void *)) noexcept;

    }
}



#pragma GCC visibility pop
# 39 "/usr/include/c++/11/bits/exception_ptr.h" 2 3
# 1 "/usr/include/c++/11/typeinfo" 1 3
# 32 "/usr/include/c++/11/typeinfo" 3
       
# 33 "/usr/include/c++/11/typeinfo" 3



# 1 "/usr/include/c++/11/bits/hash_bytes.h" 1 3
# 33 "/usr/include/c++/11/bits/hash_bytes.h" 3
       
# 34 "/usr/include/c++/11/bits/hash_bytes.h" 3



namespace std
{







  size_t
  _Hash_bytes(const void* __ptr, size_t __len, size_t __seed);





  size_t
  _Fnv_hash_bytes(const void* __ptr, size_t __len, size_t __seed);


}
# 37 "/usr/include/c++/11/typeinfo" 2 3


#pragma GCC visibility push(default)

extern "C++" {

namespace __cxxabiv1
{
  class __class_type_info;
}
# 80 "/usr/include/c++/11/typeinfo" 3
namespace std
{






  class type_info
  {
  public:




    virtual ~type_info();



    const char* name() const noexcept
    { return __name[0] == '*' ? __name + 1 : __name; }
# 115 "/usr/include/c++/11/typeinfo" 3
    bool before(const type_info& __arg) const noexcept
    { return (__name[0] == '*' && __arg.__name[0] == '*')
 ? __name < __arg.__name
 : __builtin_strcmp (__name, __arg.__name) < 0; }

    bool operator==(const type_info& __arg) const noexcept
    {
      return ((__name == __arg.__name)
       || (__name[0] != '*' &&
    __builtin_strcmp (__name, __arg.__name) == 0));
    }
# 138 "/usr/include/c++/11/typeinfo" 3
    bool operator!=(const type_info& __arg) const noexcept
    { return !operator==(__arg); }



    size_t hash_code() const noexcept
    {

      return _Hash_bytes(name(), __builtin_strlen(name()),
    static_cast<size_t>(0xc70f6907UL));



    }



    virtual bool __is_pointer_p() const;


    virtual bool __is_function_p() const;







    virtual bool __do_catch(const type_info *__thr_type, void **__thr_obj,
       unsigned __outer) const;


    virtual bool __do_upcast(const __cxxabiv1::__class_type_info *__target,
        void **__obj_ptr) const;

  protected:
    const char *__name;

    explicit type_info(const char *__n): __name(__n) { }

  private:

    type_info& operator=(const type_info&);
    type_info(const type_info&);
  };







  class bad_cast : public exception
  {
  public:
    bad_cast() noexcept { }



    virtual ~bad_cast() noexcept;


    virtual const char* what() const noexcept;
  };





  class bad_typeid : public exception
  {
  public:
    bad_typeid () noexcept { }



    virtual ~bad_typeid() noexcept;


    virtual const char* what() const noexcept;
  };
}

}

#pragma GCC visibility pop
# 40 "/usr/include/c++/11/bits/exception_ptr.h" 2 3
# 1 "/usr/include/c++/11/new" 1 3
# 38 "/usr/include/c++/11/new" 3
       
# 39 "/usr/include/c++/11/new" 3




#pragma GCC visibility push(default)

extern "C++" {

namespace std
{






  class bad_alloc : public exception
  {
  public:
    bad_alloc() throw() { }


    bad_alloc(const bad_alloc&) = default;
    bad_alloc& operator=(const bad_alloc&) = default;




    virtual ~bad_alloc() throw();


    virtual const char* what() const throw();
  };


  class bad_array_new_length : public bad_alloc
  {
  public:
    bad_array_new_length() throw() { }



    virtual ~bad_array_new_length() throw();


    virtual const char* what() const throw();
  };



  enum class align_val_t: size_t {};


  struct nothrow_t
  {

    explicit nothrow_t() = default;

  };

  extern const nothrow_t nothrow;



  typedef void (*new_handler)();



  new_handler set_new_handler(new_handler) throw();



  new_handler get_new_handler() noexcept;

}
# 126 "/usr/include/c++/11/new" 3
[[__nodiscard__]] void* operator new(std::size_t)
  __attribute__((__externally_visible__));
[[__nodiscard__]] void* operator new[](std::size_t)
  __attribute__((__externally_visible__));
void operator delete(void*) noexcept
  __attribute__((__externally_visible__));
void operator delete[](void*) noexcept
  __attribute__((__externally_visible__));

void operator delete(void*, std::size_t) noexcept
  __attribute__((__externally_visible__));
void operator delete[](void*, std::size_t) noexcept
  __attribute__((__externally_visible__));

[[__nodiscard__]] void* operator new(std::size_t, const std::nothrow_t&) noexcept
  __attribute__((__externally_visible__, __alloc_size__ (1), __malloc__));
[[__nodiscard__]] void* operator new[](std::size_t, const std::nothrow_t&) noexcept
  __attribute__((__externally_visible__, __alloc_size__ (1), __malloc__));
void operator delete(void*, const std::nothrow_t&) noexcept
  __attribute__((__externally_visible__));
void operator delete[](void*, const std::nothrow_t&) noexcept
  __attribute__((__externally_visible__));

[[__nodiscard__]] void* operator new(std::size_t, std::align_val_t)
  __attribute__((__externally_visible__, __alloc_size__ (1), __malloc__));
[[__nodiscard__]] void* operator new(std::size_t, std::align_val_t, const std::nothrow_t&)
  noexcept __attribute__((__externally_visible__, __alloc_size__ (1), __malloc__));
void operator delete(void*, std::align_val_t)
  noexcept __attribute__((__externally_visible__));
void operator delete(void*, std::align_val_t, const std::nothrow_t&)
  noexcept __attribute__((__externally_visible__));
[[__nodiscard__]] void* operator new[](std::size_t, std::align_val_t)
  __attribute__((__externally_visible__, __alloc_size__ (1), __malloc__));
[[__nodiscard__]] void* operator new[](std::size_t, std::align_val_t, const std::nothrow_t&)
  noexcept __attribute__((__externally_visible__, __alloc_size__ (1), __malloc__));
void operator delete[](void*, std::align_val_t)
  noexcept __attribute__((__externally_visible__));
void operator delete[](void*, std::align_val_t, const std::nothrow_t&)
  noexcept __attribute__((__externally_visible__));

void operator delete(void*, std::size_t, std::align_val_t)
  noexcept __attribute__((__externally_visible__));
void operator delete[](void*, std::size_t, std::align_val_t)
  noexcept __attribute__((__externally_visible__));




[[__nodiscard__]] inline void* operator new(std::size_t, void* __p) noexcept
{ return __p; }
[[__nodiscard__]] inline void* operator new[](std::size_t, void* __p) noexcept
{ return __p; }


inline void operator delete (void*, void*) noexcept { }
inline void operator delete[](void*, void*) noexcept { }

}



namespace std
{


  template<typename _Tp>
    [[nodiscard]] constexpr _Tp*
    launder(_Tp* __p) noexcept
    { return __builtin_launder(__p); }




  template<typename _Ret, typename... _Args , bool _NE>
    void launder(_Ret (*)(_Args...) noexcept (_NE)) = delete;
  template<typename _Ret, typename... _Args , bool _NE>
    void launder(_Ret (*)(_Args......) noexcept (_NE)) = delete;

  void launder(void*) = delete;
  void launder(const void*) = delete;
  void launder(volatile void*) = delete;
  void launder(const volatile void*) = delete;
}
# 230 "/usr/include/c++/11/new" 3
#pragma GCC visibility pop
# 41 "/usr/include/c++/11/bits/exception_ptr.h" 2 3


# 1 "/usr/include/c++/11/bits/move.h" 1 3
# 38 "/usr/include/c++/11/bits/move.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{







  template<typename _Tp>
    inline constexpr _Tp*
    __addressof(_Tp& __r) noexcept
    { return __builtin_addressof(__r); }




}

# 1 "/usr/include/c++/11/type_traits" 1 3
# 32 "/usr/include/c++/11/type_traits" 3
       
# 33 "/usr/include/c++/11/type_traits" 3







namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename... _Elements>
    class tuple;

  template<typename _Tp>
    class reference_wrapper;
# 64 "/usr/include/c++/11/type_traits" 3
  template<typename _Tp, _Tp __v>
    struct integral_constant
    {
      static constexpr _Tp value = __v;
      typedef _Tp value_type;
      typedef integral_constant<_Tp, __v> type;
      constexpr operator value_type() const noexcept { return value; }




      constexpr value_type operator()() const noexcept { return value; }

    };

  template<typename _Tp, _Tp __v>
    constexpr _Tp integral_constant<_Tp, __v>::value;


  using true_type = integral_constant<bool, true>;


  using false_type = integral_constant<bool, false>;



  template<bool __v>
    using __bool_constant = integral_constant<bool, __v>;






  template<bool __v>
    using bool_constant = integral_constant<bool, __v>;




  template<bool, typename, typename>
    struct conditional;


  template <typename _Type>
    struct __type_identity
    { using type = _Type; };

  template<typename _Tp>
    using __type_identity_t = typename __type_identity<_Tp>::type;

  template<typename...>
    struct __or_;

  template<>
    struct __or_<>
    : public false_type
    { };

  template<typename _B1>
    struct __or_<_B1>
    : public _B1
    { };

  template<typename _B1, typename _B2>
    struct __or_<_B1, _B2>
    : public conditional<_B1::value, _B1, _B2>::type
    { };

  template<typename _B1, typename _B2, typename _B3, typename... _Bn>
    struct __or_<_B1, _B2, _B3, _Bn...>
    : public conditional<_B1::value, _B1, __or_<_B2, _B3, _Bn...>>::type
    { };

  template<typename...>
    struct __and_;

  template<>
    struct __and_<>
    : public true_type
    { };

  template<typename _B1>
    struct __and_<_B1>
    : public _B1
    { };

  template<typename _B1, typename _B2>
    struct __and_<_B1, _B2>
    : public conditional<_B1::value, _B2, _B1>::type
    { };

  template<typename _B1, typename _B2, typename _B3, typename... _Bn>
    struct __and_<_B1, _B2, _B3, _Bn...>
    : public conditional<_B1::value, __and_<_B2, _B3, _Bn...>, _B1>::type
    { };

  template<typename _Pp>
    struct __not_
    : public __bool_constant<!bool(_Pp::value)>
    { };





  template<typename... _Bn>
    inline constexpr bool __or_v = __or_<_Bn...>::value;
  template<typename... _Bn>
    inline constexpr bool __and_v = __and_<_Bn...>::value;




  template<typename... _Bn>
    struct conjunction
    : __and_<_Bn...>
    { };

  template<typename... _Bn>
    struct disjunction
    : __or_<_Bn...>
    { };

  template<typename _Pp>
    struct negation
    : __not_<_Pp>
    { };




  template<typename... _Bn>
    inline constexpr bool conjunction_v = conjunction<_Bn...>::value;

  template<typename... _Bn>
    inline constexpr bool disjunction_v = disjunction<_Bn...>::value;

  template<typename _Pp>
    inline constexpr bool negation_v = negation<_Pp>::value;





  template<typename>
    struct is_reference;
  template<typename>
    struct is_function;
  template<typename>
    struct is_void;
  template<typename>
    struct remove_cv;
  template<typename>
    struct is_const;


  template<typename>
    struct __is_array_unknown_bounds;




  template <typename _Tp, size_t = sizeof(_Tp)>
    constexpr true_type __is_complete_or_unbounded(__type_identity<_Tp>)
    { return {}; }

  template <typename _TypeIdentity,
      typename _NestedType = typename _TypeIdentity::type>
    constexpr typename __or_<
      is_reference<_NestedType>,
      is_function<_NestedType>,
      is_void<_NestedType>,
      __is_array_unknown_bounds<_NestedType>
    >::type __is_complete_or_unbounded(_TypeIdentity)
    { return {}; }






  template<typename _Tp>
    struct __success_type
    { typedef _Tp type; };

  struct __failure_type
  { };


  template<typename _Tp>
    using __remove_cv_t = typename remove_cv<_Tp>::type;



  template<typename>
    struct __is_void_helper
    : public false_type { };

  template<>
    struct __is_void_helper<void>
    : public true_type { };



  template<typename _Tp>
    struct is_void
    : public __is_void_helper<__remove_cv_t<_Tp>>::type
    { };


  template<typename>
    struct __is_integral_helper
    : public false_type { };

  template<>
    struct __is_integral_helper<bool>
    : public true_type { };

  template<>
    struct __is_integral_helper<char>
    : public true_type { };

  template<>
    struct __is_integral_helper<signed char>
    : public true_type { };

  template<>
    struct __is_integral_helper<unsigned char>
    : public true_type { };





  template<>
    struct __is_integral_helper<wchar_t>
    : public true_type { };
# 310 "/usr/include/c++/11/type_traits" 3
  template<>
    struct __is_integral_helper<char16_t>
    : public true_type { };

  template<>
    struct __is_integral_helper<char32_t>
    : public true_type { };

  template<>
    struct __is_integral_helper<short>
    : public true_type { };

  template<>
    struct __is_integral_helper<unsigned short>
    : public true_type { };

  template<>
    struct __is_integral_helper<int>
    : public true_type { };

  template<>
    struct __is_integral_helper<unsigned int>
    : public true_type { };

  template<>
    struct __is_integral_helper<long>
    : public true_type { };

  template<>
    struct __is_integral_helper<unsigned long>
    : public true_type { };

  template<>
    struct __is_integral_helper<long long>
    : public true_type { };

  template<>
    struct __is_integral_helper<unsigned long long>
    : public true_type { };
# 391 "/usr/include/c++/11/type_traits" 3
  template<typename _Tp>
    struct is_integral
    : public __is_integral_helper<__remove_cv_t<_Tp>>::type
    { };


  template<typename>
    struct __is_floating_point_helper
    : public false_type { };

  template<>
    struct __is_floating_point_helper<float>
    : public true_type { };

  template<>
    struct __is_floating_point_helper<double>
    : public true_type { };

  template<>
    struct __is_floating_point_helper<long double>
    : public true_type { };
# 421 "/usr/include/c++/11/type_traits" 3
  template<typename _Tp>
    struct is_floating_point
    : public __is_floating_point_helper<__remove_cv_t<_Tp>>::type
    { };


  template<typename>
    struct is_array
    : public false_type { };

  template<typename _Tp, std::size_t _Size>
    struct is_array<_Tp[_Size]>
    : public true_type { };

  template<typename _Tp>
    struct is_array<_Tp[]>
    : public true_type { };

  template<typename>
    struct __is_pointer_helper
    : public false_type { };

  template<typename _Tp>
    struct __is_pointer_helper<_Tp*>
    : public true_type { };


  template<typename _Tp>
    struct is_pointer
    : public __is_pointer_helper<__remove_cv_t<_Tp>>::type
    { };


  template<typename>
    struct is_lvalue_reference
    : public false_type { };

  template<typename _Tp>
    struct is_lvalue_reference<_Tp&>
    : public true_type { };


  template<typename>
    struct is_rvalue_reference
    : public false_type { };

  template<typename _Tp>
    struct is_rvalue_reference<_Tp&&>
    : public true_type { };

  template<typename>
    struct __is_member_object_pointer_helper
    : public false_type { };

  template<typename _Tp, typename _Cp>
    struct __is_member_object_pointer_helper<_Tp _Cp::*>
    : public __not_<is_function<_Tp>>::type { };


  template<typename _Tp>
    struct is_member_object_pointer
    : public __is_member_object_pointer_helper<__remove_cv_t<_Tp>>::type
    { };

  template<typename>
    struct __is_member_function_pointer_helper
    : public false_type { };

  template<typename _Tp, typename _Cp>
    struct __is_member_function_pointer_helper<_Tp _Cp::*>
    : public is_function<_Tp>::type { };


  template<typename _Tp>
    struct is_member_function_pointer
    : public __is_member_function_pointer_helper<__remove_cv_t<_Tp>>::type
    { };


  template<typename _Tp>
    struct is_enum
    : public integral_constant<bool, __is_enum(_Tp)>
    { };


  template<typename _Tp>
    struct is_union
    : public integral_constant<bool, __is_union(_Tp)>
    { };


  template<typename _Tp>
    struct is_class
    : public integral_constant<bool, __is_class(_Tp)>
    { };


  template<typename _Tp>
    struct is_function
    : public __bool_constant<!is_const<const _Tp>::value> { };

  template<typename _Tp>
    struct is_function<_Tp&>
    : public false_type { };

  template<typename _Tp>
    struct is_function<_Tp&&>
    : public false_type { };



  template<typename>
    struct __is_null_pointer_helper
    : public false_type { };

  template<>
    struct __is_null_pointer_helper<std::nullptr_t>
    : public true_type { };


  template<typename _Tp>
    struct is_null_pointer
    : public __is_null_pointer_helper<__remove_cv_t<_Tp>>::type
    { };



  template<typename _Tp>
    struct __is_nullptr_t
    : public is_null_pointer<_Tp>
    { } __attribute__ ((__deprecated__ ("use '" "std::is_null_pointer" "' instead")));




  template<typename _Tp>
    struct is_reference
    : public __or_<is_lvalue_reference<_Tp>,
                   is_rvalue_reference<_Tp>>::type
    { };


  template<typename _Tp>
    struct is_arithmetic
    : public __or_<is_integral<_Tp>, is_floating_point<_Tp>>::type
    { };


  template<typename _Tp>
    struct is_fundamental
    : public __or_<is_arithmetic<_Tp>, is_void<_Tp>,
     is_null_pointer<_Tp>>::type
    { };


  template<typename _Tp>
    struct is_object
    : public __not_<__or_<is_function<_Tp>, is_reference<_Tp>,
                          is_void<_Tp>>>::type
    { };

  template<typename>
    struct is_member_pointer;


  template<typename _Tp>
    struct is_scalar
    : public __or_<is_arithmetic<_Tp>, is_enum<_Tp>, is_pointer<_Tp>,
                   is_member_pointer<_Tp>, is_null_pointer<_Tp>>::type
    { };


  template<typename _Tp>
    struct is_compound
    : public __not_<is_fundamental<_Tp>>::type { };


  template<typename _Tp>
    struct __is_member_pointer_helper
    : public false_type { };

  template<typename _Tp, typename _Cp>
    struct __is_member_pointer_helper<_Tp _Cp::*>
    : public true_type { };



  template<typename _Tp>
    struct is_member_pointer
    : public __is_member_pointer_helper<__remove_cv_t<_Tp>>::type
    { };

  template<typename, typename>
    struct is_same;


  template<typename _Tp, typename... _Types>
    using __is_one_of = __or_<is_same<_Tp, _Types>...>;


  template<typename _Tp>
    using __is_signed_integer = __is_one_of<__remove_cv_t<_Tp>,
   signed char, signed short, signed int, signed long,
   signed long long
# 637 "/usr/include/c++/11/type_traits" 3
   >;


  template<typename _Tp>
    using __is_unsigned_integer = __is_one_of<__remove_cv_t<_Tp>,
   unsigned char, unsigned short, unsigned int, unsigned long,
   unsigned long long
# 656 "/usr/include/c++/11/type_traits" 3
   >;


  template<typename _Tp>
    using __is_standard_integer
      = __or_<__is_signed_integer<_Tp>, __is_unsigned_integer<_Tp>>;


  template<typename...> using __void_t = void;



  template<typename _Tp, typename = void>
    struct __is_referenceable
    : public false_type
    { };

  template<typename _Tp>
    struct __is_referenceable<_Tp, __void_t<_Tp&>>
    : public true_type
    { };





  template<typename>
    struct is_const
    : public false_type { };

  template<typename _Tp>
    struct is_const<_Tp const>
    : public true_type { };


  template<typename>
    struct is_volatile
    : public false_type { };

  template<typename _Tp>
    struct is_volatile<_Tp volatile>
    : public true_type { };


  template<typename _Tp>
    struct is_trivial
    : public integral_constant<bool, __is_trivial(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_trivially_copyable
    : public integral_constant<bool, __is_trivially_copyable(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_standard_layout
    : public integral_constant<bool, __is_standard_layout(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };





  template<typename _Tp>
    struct
   
    is_pod
    : public integral_constant<bool, __is_pod(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };




  template<typename _Tp>
    struct
    [[__deprecated__]]
    is_literal_type
    : public integral_constant<bool, __is_literal_type(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_empty
    : public integral_constant<bool, __is_empty(_Tp)>
    { };


  template<typename _Tp>
    struct is_polymorphic
    : public integral_constant<bool, __is_polymorphic(_Tp)>
    { };





  template<typename _Tp>
    struct is_final
    : public integral_constant<bool, __is_final(_Tp)>
    { };



  template<typename _Tp>
    struct is_abstract
    : public integral_constant<bool, __is_abstract(_Tp)>
    { };


  template<typename _Tp,
    bool = is_arithmetic<_Tp>::value>
    struct __is_signed_helper
    : public false_type { };

  template<typename _Tp>
    struct __is_signed_helper<_Tp, true>
    : public integral_constant<bool, _Tp(-1) < _Tp(0)>
    { };



  template<typename _Tp>
    struct is_signed
    : public __is_signed_helper<_Tp>::type
    { };


  template<typename _Tp>
    struct is_unsigned
    : public __and_<is_arithmetic<_Tp>, __not_<is_signed<_Tp>>>
    { };


  template<typename _Tp, typename _Up = _Tp&&>
    _Up
    __declval(int);

  template<typename _Tp>
    _Tp
    __declval(long);


  template<typename _Tp>
    auto declval() noexcept -> decltype(__declval<_Tp>(0));

  template<typename, unsigned = 0>
    struct extent;

  template<typename>
    struct remove_all_extents;


  template<typename _Tp>
    struct __is_array_known_bounds
    : public integral_constant<bool, (extent<_Tp>::value > 0)>
    { };

  template<typename _Tp>
    struct __is_array_unknown_bounds
    : public __and_<is_array<_Tp>, __not_<extent<_Tp>>>
    { };
# 842 "/usr/include/c++/11/type_traits" 3
  struct __do_is_destructible_impl
  {
    template<typename _Tp, typename = decltype(declval<_Tp&>().~_Tp())>
      static true_type __test(int);

    template<typename>
      static false_type __test(...);
  };

  template<typename _Tp>
    struct __is_destructible_impl
    : public __do_is_destructible_impl
    {
      typedef decltype(__test<_Tp>(0)) type;
    };

  template<typename _Tp,
           bool = __or_<is_void<_Tp>,
                        __is_array_unknown_bounds<_Tp>,
                        is_function<_Tp>>::value,
           bool = __or_<is_reference<_Tp>, is_scalar<_Tp>>::value>
    struct __is_destructible_safe;

  template<typename _Tp>
    struct __is_destructible_safe<_Tp, false, false>
    : public __is_destructible_impl<typename
               remove_all_extents<_Tp>::type>::type
    { };

  template<typename _Tp>
    struct __is_destructible_safe<_Tp, true, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_destructible_safe<_Tp, false, true>
    : public true_type { };



  template<typename _Tp>
    struct is_destructible
    : public __is_destructible_safe<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };







  struct __do_is_nt_destructible_impl
  {
    template<typename _Tp>
      static __bool_constant<noexcept(declval<_Tp&>().~_Tp())>
      __test(int);

    template<typename>
      static false_type __test(...);
  };

  template<typename _Tp>
    struct __is_nt_destructible_impl
    : public __do_is_nt_destructible_impl
    {
      typedef decltype(__test<_Tp>(0)) type;
    };

  template<typename _Tp,
           bool = __or_<is_void<_Tp>,
                        __is_array_unknown_bounds<_Tp>,
                        is_function<_Tp>>::value,
           bool = __or_<is_reference<_Tp>, is_scalar<_Tp>>::value>
    struct __is_nt_destructible_safe;

  template<typename _Tp>
    struct __is_nt_destructible_safe<_Tp, false, false>
    : public __is_nt_destructible_impl<typename
               remove_all_extents<_Tp>::type>::type
    { };

  template<typename _Tp>
    struct __is_nt_destructible_safe<_Tp, true, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_nt_destructible_safe<_Tp, false, true>
    : public true_type { };



  template<typename _Tp>
    struct is_nothrow_destructible
    : public __is_nt_destructible_safe<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, typename... _Args>
    struct __is_constructible_impl
    : public __bool_constant<__is_constructible(_Tp, _Args...)>
    { };



  template<typename _Tp, typename... _Args>
    struct is_constructible
      : public __is_constructible_impl<_Tp, _Args...>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_default_constructible
    : public __is_constructible_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_copy_constructible_impl;

  template<typename _Tp>
    struct __is_copy_constructible_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_copy_constructible_impl<_Tp, true>
    : public __is_constructible_impl<_Tp, const _Tp&>
    { };



  template<typename _Tp>
    struct is_copy_constructible
    : public __is_copy_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_move_constructible_impl;

  template<typename _Tp>
    struct __is_move_constructible_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_move_constructible_impl<_Tp, true>
    : public __is_constructible_impl<_Tp, _Tp&&>
    { };



  template<typename _Tp>
    struct is_move_constructible
    : public __is_move_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, typename... _Args>
    using __is_nothrow_constructible_impl
      = __bool_constant<__is_nothrow_constructible(_Tp, _Args...)>;



  template<typename _Tp, typename... _Args>
    struct is_nothrow_constructible
    : public __is_nothrow_constructible_impl<_Tp, _Args...>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_nothrow_default_constructible
    : public __bool_constant<__is_nothrow_constructible(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_nothrow_copy_constructible_impl;

  template<typename _Tp>
    struct __is_nothrow_copy_constructible_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_nothrow_copy_constructible_impl<_Tp, true>
    : public __is_nothrow_constructible_impl<_Tp, const _Tp&>
    { };



  template<typename _Tp>
    struct is_nothrow_copy_constructible
    : public __is_nothrow_copy_constructible_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_nothrow_move_constructible_impl;

  template<typename _Tp>
    struct __is_nothrow_move_constructible_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_nothrow_move_constructible_impl<_Tp, true>
    : public __is_nothrow_constructible_impl<_Tp, _Tp&&>
    { };



  template<typename _Tp>
    struct is_nothrow_move_constructible
    : public __is_nothrow_move_constructible_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, typename _Up>
    struct is_assignable
    : public __bool_constant<__is_assignable(_Tp, _Up)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_copy_assignable_impl;

  template<typename _Tp>
    struct __is_copy_assignable_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_copy_assignable_impl<_Tp, true>
    : public __bool_constant<__is_assignable(_Tp&, const _Tp&)>
    { };


  template<typename _Tp>
    struct is_copy_assignable
    : public __is_copy_assignable_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_move_assignable_impl;

  template<typename _Tp>
    struct __is_move_assignable_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_move_assignable_impl<_Tp, true>
    : public __bool_constant<__is_assignable(_Tp&, _Tp&&)>
    { };


  template<typename _Tp>
    struct is_move_assignable
    : public __is_move_assignable_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, typename _Up>
    using __is_nothrow_assignable_impl
      = __bool_constant<__is_nothrow_assignable(_Tp, _Up)>;


  template<typename _Tp, typename _Up>
    struct is_nothrow_assignable
    : public __is_nothrow_assignable_impl<_Tp, _Up>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_nt_copy_assignable_impl;

  template<typename _Tp>
    struct __is_nt_copy_assignable_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_nt_copy_assignable_impl<_Tp, true>
    : public __is_nothrow_assignable_impl<_Tp&, const _Tp&>
    { };


  template<typename _Tp>
    struct is_nothrow_copy_assignable
    : public __is_nt_copy_assignable_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_nt_move_assignable_impl;

  template<typename _Tp>
    struct __is_nt_move_assignable_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_nt_move_assignable_impl<_Tp, true>
    : public __is_nothrow_assignable_impl<_Tp&, _Tp&&>
    { };


  template<typename _Tp>
    struct is_nothrow_move_assignable
    : public __is_nt_move_assignable_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, typename... _Args>
    struct is_trivially_constructible
    : public __bool_constant<__is_trivially_constructible(_Tp, _Args...)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_trivially_default_constructible
    : public __bool_constant<__is_trivially_constructible(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  struct __do_is_implicitly_default_constructible_impl
  {
    template <typename _Tp>
    static void __helper(const _Tp&);

    template <typename _Tp>
    static true_type __test(const _Tp&,
                            decltype(__helper<const _Tp&>({}))* = 0);

    static false_type __test(...);
  };

  template<typename _Tp>
    struct __is_implicitly_default_constructible_impl
    : public __do_is_implicitly_default_constructible_impl
    {
      typedef decltype(__test(declval<_Tp>())) type;
    };

  template<typename _Tp>
    struct __is_implicitly_default_constructible_safe
    : public __is_implicitly_default_constructible_impl<_Tp>::type
    { };

  template <typename _Tp>
    struct __is_implicitly_default_constructible
    : public __and_<__is_constructible_impl<_Tp>,
      __is_implicitly_default_constructible_safe<_Tp>>
    { };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_trivially_copy_constructible_impl;

  template<typename _Tp>
    struct __is_trivially_copy_constructible_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_trivially_copy_constructible_impl<_Tp, true>
    : public __and_<__is_copy_constructible_impl<_Tp>,
      integral_constant<bool,
   __is_trivially_constructible(_Tp, const _Tp&)>>
    { };


  template<typename _Tp>
    struct is_trivially_copy_constructible
    : public __is_trivially_copy_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_trivially_move_constructible_impl;

  template<typename _Tp>
    struct __is_trivially_move_constructible_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_trivially_move_constructible_impl<_Tp, true>
    : public __and_<__is_move_constructible_impl<_Tp>,
      integral_constant<bool,
   __is_trivially_constructible(_Tp, _Tp&&)>>
    { };


  template<typename _Tp>
    struct is_trivially_move_constructible
    : public __is_trivially_move_constructible_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, typename _Up>
    struct is_trivially_assignable
    : public __bool_constant<__is_trivially_assignable(_Tp, _Up)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_trivially_copy_assignable_impl;

  template<typename _Tp>
    struct __is_trivially_copy_assignable_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_trivially_copy_assignable_impl<_Tp, true>
    : public __bool_constant<__is_trivially_assignable(_Tp&, const _Tp&)>
    { };


  template<typename _Tp>
    struct is_trivially_copy_assignable
    : public __is_trivially_copy_assignable_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __is_trivially_move_assignable_impl;

  template<typename _Tp>
    struct __is_trivially_move_assignable_impl<_Tp, false>
    : public false_type { };

  template<typename _Tp>
    struct __is_trivially_move_assignable_impl<_Tp, true>
    : public __bool_constant<__is_trivially_assignable(_Tp&, _Tp&&)>
    { };


  template<typename _Tp>
    struct is_trivially_move_assignable
    : public __is_trivially_move_assignable_impl<_Tp>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_trivially_destructible
    : public __and_<__is_destructible_safe<_Tp>,
      __bool_constant<__has_trivial_destructor(_Tp)>>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };



  template<typename _Tp>
    struct has_virtual_destructor
    : public integral_constant<bool, __has_virtual_destructor(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };





  template<typename _Tp>
    struct alignment_of
    : public integral_constant<std::size_t, alignof(_Tp)>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename>
    struct rank
    : public integral_constant<std::size_t, 0> { };

  template<typename _Tp, std::size_t _Size>
    struct rank<_Tp[_Size]>
    : public integral_constant<std::size_t, 1 + rank<_Tp>::value> { };

  template<typename _Tp>
    struct rank<_Tp[]>
    : public integral_constant<std::size_t, 1 + rank<_Tp>::value> { };


  template<typename, unsigned _Uint>
    struct extent
    : public integral_constant<std::size_t, 0> { };

  template<typename _Tp, unsigned _Uint, std::size_t _Size>
    struct extent<_Tp[_Size], _Uint>
    : public integral_constant<std::size_t,
          _Uint == 0 ? _Size : extent<_Tp,
          _Uint - 1>::value>
    { };

  template<typename _Tp, unsigned _Uint>
    struct extent<_Tp[], _Uint>
    : public integral_constant<std::size_t,
          _Uint == 0 ? 0 : extent<_Tp,
             _Uint - 1>::value>
    { };





  template<typename _Tp, typename _Up>
    struct is_same

    : public integral_constant<bool, __is_same(_Tp, _Up)>



    { };
# 1420 "/usr/include/c++/11/type_traits" 3
  template<typename _Base, typename _Derived>
    struct is_base_of
    : public integral_constant<bool, __is_base_of(_Base, _Derived)>
    { };

  template<typename _From, typename _To,
           bool = __or_<is_void<_From>, is_function<_To>,
                        is_array<_To>>::value>
    struct __is_convertible_helper
    {
      typedef typename is_void<_To>::type type;
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
  template<typename _From, typename _To>
    class __is_convertible_helper<_From, _To, false>
    {
      template<typename _To1>
 static void __test_aux(_To1) noexcept;

      template<typename _From1, typename _To1,
        typename = decltype(__test_aux<_To1>(std::declval<_From1>()))>
 static true_type
 __test(int);

      template<typename, typename>
 static false_type
 __test(...);

    public:
      typedef decltype(__test<_From, _To>(0)) type;
    };
#pragma GCC diagnostic pop


  template<typename _From, typename _To>
    struct is_convertible
    : public __is_convertible_helper<_From, _To>::type
    { };


  template<typename _ToElementType, typename _FromElementType>
    using __is_array_convertible
      = is_convertible<_FromElementType(*)[], _ToElementType(*)[]>;

  template<typename _From, typename _To,
           bool = __or_<is_void<_From>, is_function<_To>,
                        is_array<_To>>::value>
    struct __is_nt_convertible_helper
    : is_void<_To>
    { };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
  template<typename _From, typename _To>
    class __is_nt_convertible_helper<_From, _To, false>
    {
      template<typename _To1>
 static void __test_aux(_To1) noexcept;

      template<typename _From1, typename _To1>
 static
 __bool_constant<noexcept(__test_aux<_To1>(std::declval<_From1>()))>
 __test(int);

      template<typename, typename>
 static false_type
 __test(...);

    public:
      using type = decltype(__test<_From, _To>(0));
    };
#pragma GCC diagnostic pop
# 1512 "/usr/include/c++/11/type_traits" 3
  template<typename _Tp>
    struct remove_const
    { typedef _Tp type; };

  template<typename _Tp>
    struct remove_const<_Tp const>
    { typedef _Tp type; };


  template<typename _Tp>
    struct remove_volatile
    { typedef _Tp type; };

  template<typename _Tp>
    struct remove_volatile<_Tp volatile>
    { typedef _Tp type; };


  template<typename _Tp>
    struct remove_cv
    { using type = _Tp; };

  template<typename _Tp>
    struct remove_cv<const _Tp>
    { using type = _Tp; };

  template<typename _Tp>
    struct remove_cv<volatile _Tp>
    { using type = _Tp; };

  template<typename _Tp>
    struct remove_cv<const volatile _Tp>
    { using type = _Tp; };


  template<typename _Tp>
    struct add_const
    { typedef _Tp const type; };


  template<typename _Tp>
    struct add_volatile
    { typedef _Tp volatile type; };


  template<typename _Tp>
    struct add_cv
    {
      typedef typename
      add_const<typename add_volatile<_Tp>::type>::type type;
    };






  template<typename _Tp>
    using remove_const_t = typename remove_const<_Tp>::type;


  template<typename _Tp>
    using remove_volatile_t = typename remove_volatile<_Tp>::type;


  template<typename _Tp>
    using remove_cv_t = typename remove_cv<_Tp>::type;


  template<typename _Tp>
    using add_const_t = typename add_const<_Tp>::type;


  template<typename _Tp>
    using add_volatile_t = typename add_volatile<_Tp>::type;


  template<typename _Tp>
    using add_cv_t = typename add_cv<_Tp>::type;





  template<typename _Tp>
    struct remove_reference
    { typedef _Tp type; };

  template<typename _Tp>
    struct remove_reference<_Tp&>
    { typedef _Tp type; };

  template<typename _Tp>
    struct remove_reference<_Tp&&>
    { typedef _Tp type; };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __add_lvalue_reference_helper
    { typedef _Tp type; };

  template<typename _Tp>
    struct __add_lvalue_reference_helper<_Tp, true>
    { typedef _Tp& type; };


  template<typename _Tp>
    struct add_lvalue_reference
    : public __add_lvalue_reference_helper<_Tp>
    { };

  template<typename _Tp, bool = __is_referenceable<_Tp>::value>
    struct __add_rvalue_reference_helper
    { typedef _Tp type; };

  template<typename _Tp>
    struct __add_rvalue_reference_helper<_Tp, true>
    { typedef _Tp&& type; };


  template<typename _Tp>
    struct add_rvalue_reference
    : public __add_rvalue_reference_helper<_Tp>
    { };



  template<typename _Tp>
    using remove_reference_t = typename remove_reference<_Tp>::type;


  template<typename _Tp>
    using add_lvalue_reference_t = typename add_lvalue_reference<_Tp>::type;


  template<typename _Tp>
    using add_rvalue_reference_t = typename add_rvalue_reference<_Tp>::type;







  template<typename _Unqualified, bool _IsConst, bool _IsVol>
    struct __cv_selector;

  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, false, false>
    { typedef _Unqualified __type; };

  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, false, true>
    { typedef volatile _Unqualified __type; };

  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, true, false>
    { typedef const _Unqualified __type; };

  template<typename _Unqualified>
    struct __cv_selector<_Unqualified, true, true>
    { typedef const volatile _Unqualified __type; };

  template<typename _Qualified, typename _Unqualified,
    bool _IsConst = is_const<_Qualified>::value,
    bool _IsVol = is_volatile<_Qualified>::value>
    class __match_cv_qualifiers
    {
      typedef __cv_selector<_Unqualified, _IsConst, _IsVol> __match;

    public:
      typedef typename __match::__type __type;
    };


  template<typename _Tp>
    struct __make_unsigned
    { typedef _Tp __type; };

  template<>
    struct __make_unsigned<char>
    { typedef unsigned char __type; };

  template<>
    struct __make_unsigned<signed char>
    { typedef unsigned char __type; };

  template<>
    struct __make_unsigned<short>
    { typedef unsigned short __type; };

  template<>
    struct __make_unsigned<int>
    { typedef unsigned int __type; };

  template<>
    struct __make_unsigned<long>
    { typedef unsigned long __type; };

  template<>
    struct __make_unsigned<long long>
    { typedef unsigned long long __type; };
# 1736 "/usr/include/c++/11/type_traits" 3
  template<typename _Tp,
    bool _IsInt = is_integral<_Tp>::value,
    bool _IsEnum = is_enum<_Tp>::value>
    class __make_unsigned_selector;

  template<typename _Tp>
    class __make_unsigned_selector<_Tp, true, false>
    {
      using __unsigned_type
 = typename __make_unsigned<__remove_cv_t<_Tp>>::__type;

    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __unsigned_type>::__type;
    };

  class __make_unsigned_selector_base
  {
  protected:
    template<typename...> struct _List { };

    template<typename _Tp, typename... _Up>
      struct _List<_Tp, _Up...> : _List<_Up...>
      { static constexpr size_t __size = sizeof(_Tp); };

    template<size_t _Sz, typename _Tp, bool = (_Sz <= _Tp::__size)>
      struct __select;

    template<size_t _Sz, typename _Uint, typename... _UInts>
      struct __select<_Sz, _List<_Uint, _UInts...>, true>
      { using __type = _Uint; };

    template<size_t _Sz, typename _Uint, typename... _UInts>
      struct __select<_Sz, _List<_Uint, _UInts...>, false>
      : __select<_Sz, _List<_UInts...>>
      { };
  };


  template<typename _Tp>
    class __make_unsigned_selector<_Tp, false, true>
    : __make_unsigned_selector_base
    {

      using _UInts = _List<unsigned char, unsigned short, unsigned int,
      unsigned long, unsigned long long>;

      using __unsigned_type = typename __select<sizeof(_Tp), _UInts>::__type;

    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __unsigned_type>::__type;
    };






  template<>
    struct __make_unsigned<wchar_t>
    {
      using __type
 = typename __make_unsigned_selector<wchar_t, false, true>::__type;
    };
# 1812 "/usr/include/c++/11/type_traits" 3
  template<>
    struct __make_unsigned<char16_t>
    {
      using __type
 = typename __make_unsigned_selector<char16_t, false, true>::__type;
    };

  template<>
    struct __make_unsigned<char32_t>
    {
      using __type
 = typename __make_unsigned_selector<char32_t, false, true>::__type;
    };






  template<typename _Tp>
    struct make_unsigned
    { typedef typename __make_unsigned_selector<_Tp>::__type type; };


  template<>
    struct make_unsigned<bool>;




  template<typename _Tp>
    struct __make_signed
    { typedef _Tp __type; };

  template<>
    struct __make_signed<char>
    { typedef signed char __type; };

  template<>
    struct __make_signed<unsigned char>
    { typedef signed char __type; };

  template<>
    struct __make_signed<unsigned short>
    { typedef signed short __type; };

  template<>
    struct __make_signed<unsigned int>
    { typedef signed int __type; };

  template<>
    struct __make_signed<unsigned long>
    { typedef signed long __type; };

  template<>
    struct __make_signed<unsigned long long>
    { typedef signed long long __type; };
# 1892 "/usr/include/c++/11/type_traits" 3
  template<typename _Tp,
    bool _IsInt = is_integral<_Tp>::value,
    bool _IsEnum = is_enum<_Tp>::value>
    class __make_signed_selector;

  template<typename _Tp>
    class __make_signed_selector<_Tp, true, false>
    {
      using __signed_type
 = typename __make_signed<__remove_cv_t<_Tp>>::__type;

    public:
      using __type
 = typename __match_cv_qualifiers<_Tp, __signed_type>::__type;
    };


  template<typename _Tp>
    class __make_signed_selector<_Tp, false, true>
    {
      typedef typename __make_unsigned_selector<_Tp>::__type __unsigned_type;

    public:
      typedef typename __make_signed_selector<__unsigned_type>::__type __type;
    };






  template<>
    struct __make_signed<wchar_t>
    {
      using __type
 = typename __make_signed_selector<wchar_t, false, true>::__type;
    };
# 1940 "/usr/include/c++/11/type_traits" 3
  template<>
    struct __make_signed<char16_t>
    {
      using __type
 = typename __make_signed_selector<char16_t, false, true>::__type;
    };

  template<>
    struct __make_signed<char32_t>
    {
      using __type
 = typename __make_signed_selector<char32_t, false, true>::__type;
    };






  template<typename _Tp>
    struct make_signed
    { typedef typename __make_signed_selector<_Tp>::__type type; };


  template<>
    struct make_signed<bool>;



  template<typename _Tp>
    using make_signed_t = typename make_signed<_Tp>::type;


  template<typename _Tp>
    using make_unsigned_t = typename make_unsigned<_Tp>::type;





  template<typename _Tp>
    struct remove_extent
    { typedef _Tp type; };

  template<typename _Tp, std::size_t _Size>
    struct remove_extent<_Tp[_Size]>
    { typedef _Tp type; };

  template<typename _Tp>
    struct remove_extent<_Tp[]>
    { typedef _Tp type; };


  template<typename _Tp>
    struct remove_all_extents
    { typedef _Tp type; };

  template<typename _Tp, std::size_t _Size>
    struct remove_all_extents<_Tp[_Size]>
    { typedef typename remove_all_extents<_Tp>::type type; };

  template<typename _Tp>
    struct remove_all_extents<_Tp[]>
    { typedef typename remove_all_extents<_Tp>::type type; };



  template<typename _Tp>
    using remove_extent_t = typename remove_extent<_Tp>::type;


  template<typename _Tp>
    using remove_all_extents_t = typename remove_all_extents<_Tp>::type;




  template<typename _Tp, typename>
    struct __remove_pointer_helper
    { typedef _Tp type; };

  template<typename _Tp, typename _Up>
    struct __remove_pointer_helper<_Tp, _Up*>
    { typedef _Up type; };


  template<typename _Tp>
    struct remove_pointer
    : public __remove_pointer_helper<_Tp, __remove_cv_t<_Tp>>
    { };

  template<typename _Tp, bool = __or_<__is_referenceable<_Tp>,
          is_void<_Tp>>::value>
    struct __add_pointer_helper
    { typedef _Tp type; };

  template<typename _Tp>
    struct __add_pointer_helper<_Tp, true>
    { typedef typename remove_reference<_Tp>::type* type; };


  template<typename _Tp>
    struct add_pointer
    : public __add_pointer_helper<_Tp>
    { };



  template<typename _Tp>
    using remove_pointer_t = typename remove_pointer<_Tp>::type;


  template<typename _Tp>
    using add_pointer_t = typename add_pointer<_Tp>::type;


  template<std::size_t _Len>
    struct __aligned_storage_msa
    {
      union __type
      {
 unsigned char __data[_Len];
 struct __attribute__((__aligned__)) { } __align;
      };
    };
# 2076 "/usr/include/c++/11/type_traits" 3
  template<std::size_t _Len, std::size_t _Align =
    __alignof__(typename __aligned_storage_msa<_Len>::__type)>
    struct aligned_storage
    {
      union type
      {
 unsigned char __data[_Len];
 struct __attribute__((__aligned__((_Align)))) { } __align;
      };
    };

  template <typename... _Types>
    struct __strictest_alignment
    {
      static const size_t _S_alignment = 0;
      static const size_t _S_size = 0;
    };

  template <typename _Tp, typename... _Types>
    struct __strictest_alignment<_Tp, _Types...>
    {
      static const size_t _S_alignment =
        alignof(_Tp) > __strictest_alignment<_Types...>::_S_alignment
 ? alignof(_Tp) : __strictest_alignment<_Types...>::_S_alignment;
      static const size_t _S_size =
        sizeof(_Tp) > __strictest_alignment<_Types...>::_S_size
 ? sizeof(_Tp) : __strictest_alignment<_Types...>::_S_size;
    };
# 2115 "/usr/include/c++/11/type_traits" 3
  template <size_t _Len, typename... _Types>
    struct aligned_union
    {
    private:
      static_assert(sizeof...(_Types) != 0, "At least one type is required");

      using __strictest = __strictest_alignment<_Types...>;
      static const size_t _S_len = _Len > __strictest::_S_size
 ? _Len : __strictest::_S_size;
    public:

      static const size_t alignment_value = __strictest::_S_alignment;

      typedef typename aligned_storage<_S_len, alignment_value>::type type;
    };

  template <size_t _Len, typename... _Types>
    const size_t aligned_union<_Len, _Types...>::alignment_value;





  template<typename _Up,
    bool _IsArray = is_array<_Up>::value,
    bool _IsFunction = is_function<_Up>::value>
    struct __decay_selector;


  template<typename _Up>
    struct __decay_selector<_Up, false, false>
    { typedef __remove_cv_t<_Up> __type; };

  template<typename _Up>
    struct __decay_selector<_Up, true, false>
    { typedef typename remove_extent<_Up>::type* __type; };

  template<typename _Up>
    struct __decay_selector<_Up, false, true>
    { typedef typename add_pointer<_Up>::type __type; };



  template<typename _Tp>
    class decay
    {
      typedef typename remove_reference<_Tp>::type __remove_type;

    public:
      typedef typename __decay_selector<__remove_type>::__type type;
    };




  template<typename _Tp>
    struct __strip_reference_wrapper
    {
      typedef _Tp __type;
    };

  template<typename _Tp>
    struct __strip_reference_wrapper<reference_wrapper<_Tp> >
    {
      typedef _Tp& __type;
    };


  template<typename _Tp>
    using __decay_t = typename decay<_Tp>::type;

  template<typename _Tp>
    using __decay_and_strip = __strip_reference_wrapper<__decay_t<_Tp>>;




  template<bool, typename _Tp = void>
    struct enable_if
    { };


  template<typename _Tp>
    struct enable_if<true, _Tp>
    { typedef _Tp type; };




  template<bool _Cond, typename _Tp = void>
    using __enable_if_t = typename enable_if<_Cond, _Tp>::type;


  template<typename... _Cond>
    using _Require = __enable_if_t<__and_<_Cond...>::value>;


  template<typename _Tp>
    using __remove_cvref_t
     = typename remove_cv<typename remove_reference<_Tp>::type>::type;




  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    struct conditional
    { typedef _Iftrue type; };


  template<typename _Iftrue, typename _Iffalse>
    struct conditional<false, _Iftrue, _Iffalse>
    { typedef _Iffalse type; };


  template<typename... _Tp>
    struct common_type;




  struct __do_common_type_impl
  {
    template<typename _Tp, typename _Up>
      using __cond_t
 = decltype(true ? std::declval<_Tp>() : std::declval<_Up>());



    template<typename _Tp, typename _Up>
      static __success_type<__decay_t<__cond_t<_Tp, _Up>>>
      _S_test(int);
# 2255 "/usr/include/c++/11/type_traits" 3
    template<typename, typename>
      static __failure_type
      _S_test_2(...);

    template<typename _Tp, typename _Up>
      static decltype(_S_test_2<_Tp, _Up>(0))
      _S_test(...);
  };


  template<>
    struct common_type<>
    { };


  template<typename _Tp0>
    struct common_type<_Tp0>
    : public common_type<_Tp0, _Tp0>
    { };


  template<typename _Tp1, typename _Tp2,
    typename _Dp1 = __decay_t<_Tp1>, typename _Dp2 = __decay_t<_Tp2>>
    struct __common_type_impl
    {


      using type = common_type<_Dp1, _Dp2>;
    };

  template<typename _Tp1, typename _Tp2>
    struct __common_type_impl<_Tp1, _Tp2, _Tp1, _Tp2>
    : private __do_common_type_impl
    {


      using type = decltype(_S_test<_Tp1, _Tp2>(0));
    };


  template<typename _Tp1, typename _Tp2>
    struct common_type<_Tp1, _Tp2>
    : public __common_type_impl<_Tp1, _Tp2>::type
    { };

  template<typename...>
    struct __common_type_pack
    { };

  template<typename, typename, typename = void>
    struct __common_type_fold;


  template<typename _Tp1, typename _Tp2, typename... _Rp>
    struct common_type<_Tp1, _Tp2, _Rp...>
    : public __common_type_fold<common_type<_Tp1, _Tp2>,
    __common_type_pack<_Rp...>>
    { };




  template<typename _CTp, typename... _Rp>
    struct __common_type_fold<_CTp, __common_type_pack<_Rp...>,
         __void_t<typename _CTp::type>>
    : public common_type<typename _CTp::type, _Rp...>
    { };


  template<typename _CTp, typename _Rp>
    struct __common_type_fold<_CTp, _Rp, void>
    { };

  template<typename _Tp, bool = is_enum<_Tp>::value>
    struct __underlying_type_impl
    {
      using type = __underlying_type(_Tp);
    };

  template<typename _Tp>
    struct __underlying_type_impl<_Tp, false>
    { };



  template<typename _Tp>
    struct underlying_type
    : public __underlying_type_impl<_Tp>
    { };


  template<typename _Tp>
    struct __declval_protector
    {
      static const bool __stop = false;
    };






  template<typename _Tp>
    auto declval() noexcept -> decltype(__declval<_Tp>(0))
    {
      static_assert(__declval_protector<_Tp>::__stop,
      "declval() must not be used!");
      return __declval<_Tp>(0);
    }


  template<typename _Signature>
    struct result_of;






  struct __invoke_memfun_ref { };
  struct __invoke_memfun_deref { };
  struct __invoke_memobj_ref { };
  struct __invoke_memobj_deref { };
  struct __invoke_other { };


  template<typename _Tp, typename _Tag>
    struct __result_of_success : __success_type<_Tp>
    { using __invoke_type = _Tag; };


  struct __result_of_memfun_ref_impl
  {
    template<typename _Fp, typename _Tp1, typename... _Args>
      static __result_of_success<decltype(
      (std::declval<_Tp1>().*std::declval<_Fp>())(std::declval<_Args>()...)
      ), __invoke_memfun_ref> _S_test(int);

    template<typename...>
      static __failure_type _S_test(...);
  };

  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_memfun_ref
    : private __result_of_memfun_ref_impl
    {
      typedef decltype(_S_test<_MemPtr, _Arg, _Args...>(0)) type;
    };


  struct __result_of_memfun_deref_impl
  {
    template<typename _Fp, typename _Tp1, typename... _Args>
      static __result_of_success<decltype(
      ((*std::declval<_Tp1>()).*std::declval<_Fp>())(std::declval<_Args>()...)
      ), __invoke_memfun_deref> _S_test(int);

    template<typename...>
      static __failure_type _S_test(...);
  };

  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_memfun_deref
    : private __result_of_memfun_deref_impl
    {
      typedef decltype(_S_test<_MemPtr, _Arg, _Args...>(0)) type;
    };


  struct __result_of_memobj_ref_impl
  {
    template<typename _Fp, typename _Tp1>
      static __result_of_success<decltype(
      std::declval<_Tp1>().*std::declval<_Fp>()
      ), __invoke_memobj_ref> _S_test(int);

    template<typename, typename>
      static __failure_type _S_test(...);
  };

  template<typename _MemPtr, typename _Arg>
    struct __result_of_memobj_ref
    : private __result_of_memobj_ref_impl
    {
      typedef decltype(_S_test<_MemPtr, _Arg>(0)) type;
    };


  struct __result_of_memobj_deref_impl
  {
    template<typename _Fp, typename _Tp1>
      static __result_of_success<decltype(
      (*std::declval<_Tp1>()).*std::declval<_Fp>()
      ), __invoke_memobj_deref> _S_test(int);

    template<typename, typename>
      static __failure_type _S_test(...);
  };

  template<typename _MemPtr, typename _Arg>
    struct __result_of_memobj_deref
    : private __result_of_memobj_deref_impl
    {
      typedef decltype(_S_test<_MemPtr, _Arg>(0)) type;
    };

  template<typename _MemPtr, typename _Arg>
    struct __result_of_memobj;

  template<typename _Res, typename _Class, typename _Arg>
    struct __result_of_memobj<_Res _Class::*, _Arg>
    {
      typedef __remove_cvref_t<_Arg> _Argval;
      typedef _Res _Class::* _MemPtr;
      typedef typename conditional<__or_<is_same<_Argval, _Class>,
        is_base_of<_Class, _Argval>>::value,
        __result_of_memobj_ref<_MemPtr, _Arg>,
        __result_of_memobj_deref<_MemPtr, _Arg>
      >::type::type type;
    };

  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_memfun;

  template<typename _Res, typename _Class, typename _Arg, typename... _Args>
    struct __result_of_memfun<_Res _Class::*, _Arg, _Args...>
    {
      typedef typename remove_reference<_Arg>::type _Argval;
      typedef _Res _Class::* _MemPtr;
      typedef typename conditional<is_base_of<_Class, _Argval>::value,
        __result_of_memfun_ref<_MemPtr, _Arg, _Args...>,
        __result_of_memfun_deref<_MemPtr, _Arg, _Args...>
      >::type::type type;
    };






  template<typename _Tp, typename _Up = __remove_cvref_t<_Tp>>
    struct __inv_unwrap
    {
      using type = _Tp;
    };

  template<typename _Tp, typename _Up>
    struct __inv_unwrap<_Tp, reference_wrapper<_Up>>
    {
      using type = _Up&;
    };

  template<bool, bool, typename _Functor, typename... _ArgTypes>
    struct __result_of_impl
    {
      typedef __failure_type type;
    };

  template<typename _MemPtr, typename _Arg>
    struct __result_of_impl<true, false, _MemPtr, _Arg>
    : public __result_of_memobj<__decay_t<_MemPtr>,
    typename __inv_unwrap<_Arg>::type>
    { };

  template<typename _MemPtr, typename _Arg, typename... _Args>
    struct __result_of_impl<false, true, _MemPtr, _Arg, _Args...>
    : public __result_of_memfun<__decay_t<_MemPtr>,
    typename __inv_unwrap<_Arg>::type, _Args...>
    { };


  struct __result_of_other_impl
  {
    template<typename _Fn, typename... _Args>
      static __result_of_success<decltype(
      std::declval<_Fn>()(std::declval<_Args>()...)
      ), __invoke_other> _S_test(int);

    template<typename...>
      static __failure_type _S_test(...);
  };

  template<typename _Functor, typename... _ArgTypes>
    struct __result_of_impl<false, false, _Functor, _ArgTypes...>
    : private __result_of_other_impl
    {
      typedef decltype(_S_test<_Functor, _ArgTypes...>(0)) type;
    };


  template<typename _Functor, typename... _ArgTypes>
    struct __invoke_result
    : public __result_of_impl<
        is_member_object_pointer<
          typename remove_reference<_Functor>::type
        >::value,
        is_member_function_pointer<
          typename remove_reference<_Functor>::type
        >::value,
 _Functor, _ArgTypes...
      >::type
    { };


  template<typename _Functor, typename... _ArgTypes>
    struct result_of<_Functor(_ArgTypes...)>
    : public __invoke_result<_Functor, _ArgTypes...>
    { };



  template<size_t _Len, size_t _Align =
     __alignof__(typename __aligned_storage_msa<_Len>::__type)>
    using aligned_storage_t = typename aligned_storage<_Len, _Align>::type;

  template <size_t _Len, typename... _Types>
    using aligned_union_t = typename aligned_union<_Len, _Types...>::type;


  template<typename _Tp>
    using decay_t = typename decay<_Tp>::type;


  template<bool _Cond, typename _Tp = void>
    using enable_if_t = typename enable_if<_Cond, _Tp>::type;


  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    using conditional_t = typename conditional<_Cond, _Iftrue, _Iffalse>::type;


  template<typename... _Tp>
    using common_type_t = typename common_type<_Tp...>::type;


  template<typename _Tp>
    using underlying_type_t = typename underlying_type<_Tp>::type;


  template<typename _Tp>
    using result_of_t = typename result_of<_Tp>::type;





  template<typename...> using void_t = void;





  template<typename _Default, typename _AlwaysVoid,
    template<typename...> class _Op, typename... _Args>
    struct __detector
    {
      using value_t = false_type;
      using type = _Default;
    };


  template<typename _Default, template<typename...> class _Op,
     typename... _Args>
    struct __detector<_Default, __void_t<_Op<_Args...>>, _Op, _Args...>
    {
      using value_t = true_type;
      using type = _Op<_Args...>;
    };


  template<typename _Default, template<typename...> class _Op,
    typename... _Args>
    using __detected_or = __detector<_Default, void, _Op, _Args...>;


  template<typename _Default, template<typename...> class _Op,
    typename... _Args>
    using __detected_or_t
      = typename __detected_or<_Default, _Op, _Args...>::type;
# 2649 "/usr/include/c++/11/type_traits" 3
  template <typename _Tp>
    struct __is_swappable;

  template <typename _Tp>
    struct __is_nothrow_swappable;

  template<typename>
    struct __is_tuple_like_impl : false_type
    { };

  template<typename... _Tps>
    struct __is_tuple_like_impl<tuple<_Tps...>> : true_type
    { };


  template<typename _Tp>
    struct __is_tuple_like
    : public __is_tuple_like_impl<__remove_cvref_t<_Tp>>::type
    { };


  template<typename _Tp>
   
    inline
    _Require<__not_<__is_tuple_like<_Tp>>,
      is_move_constructible<_Tp>,
      is_move_assignable<_Tp>>
    swap(_Tp&, _Tp&)
    noexcept(__and_<is_nothrow_move_constructible<_Tp>,
             is_nothrow_move_assignable<_Tp>>::value);

  template<typename _Tp, size_t _Nm>
   
    inline
    __enable_if_t<__is_swappable<_Tp>::value>
    swap(_Tp (&__a)[_Nm], _Tp (&__b)[_Nm])
    noexcept(__is_nothrow_swappable<_Tp>::value);


  namespace __swappable_details {
    using std::swap;

    struct __do_is_swappable_impl
    {
      template<typename _Tp, typename
               = decltype(swap(std::declval<_Tp&>(), std::declval<_Tp&>()))>
        static true_type __test(int);

      template<typename>
        static false_type __test(...);
    };

    struct __do_is_nothrow_swappable_impl
    {
      template<typename _Tp>
        static __bool_constant<
          noexcept(swap(std::declval<_Tp&>(), std::declval<_Tp&>()))
        > __test(int);

      template<typename>
        static false_type __test(...);
    };

  }

  template<typename _Tp>
    struct __is_swappable_impl
    : public __swappable_details::__do_is_swappable_impl
    {
      typedef decltype(__test<_Tp>(0)) type;
    };

  template<typename _Tp>
    struct __is_nothrow_swappable_impl
    : public __swappable_details::__do_is_nothrow_swappable_impl
    {
      typedef decltype(__test<_Tp>(0)) type;
    };

  template<typename _Tp>
    struct __is_swappable
    : public __is_swappable_impl<_Tp>::type
    { };

  template<typename _Tp>
    struct __is_nothrow_swappable
    : public __is_nothrow_swappable_impl<_Tp>::type
    { };







  template<typename _Tp>
    struct is_swappable
    : public __is_swappable_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    struct is_nothrow_swappable
    : public __is_nothrow_swappable_impl<_Tp>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };



  template<typename _Tp>
    inline constexpr bool is_swappable_v =
      is_swappable<_Tp>::value;


  template<typename _Tp>
    inline constexpr bool is_nothrow_swappable_v =
      is_nothrow_swappable<_Tp>::value;



  namespace __swappable_with_details {
    using std::swap;

    struct __do_is_swappable_with_impl
    {
      template<typename _Tp, typename _Up, typename
               = decltype(swap(std::declval<_Tp>(), std::declval<_Up>())),
               typename
               = decltype(swap(std::declval<_Up>(), std::declval<_Tp>()))>
        static true_type __test(int);

      template<typename, typename>
        static false_type __test(...);
    };

    struct __do_is_nothrow_swappable_with_impl
    {
      template<typename _Tp, typename _Up>
        static __bool_constant<
          noexcept(swap(std::declval<_Tp>(), std::declval<_Up>()))
          &&
          noexcept(swap(std::declval<_Up>(), std::declval<_Tp>()))
        > __test(int);

      template<typename, typename>
        static false_type __test(...);
    };

  }

  template<typename _Tp, typename _Up>
    struct __is_swappable_with_impl
    : public __swappable_with_details::__do_is_swappable_with_impl
    {
      typedef decltype(__test<_Tp, _Up>(0)) type;
    };


  template<typename _Tp>
    struct __is_swappable_with_impl<_Tp&, _Tp&>
    : public __swappable_details::__do_is_swappable_impl
    {
      typedef decltype(__test<_Tp&>(0)) type;
    };

  template<typename _Tp, typename _Up>
    struct __is_nothrow_swappable_with_impl
    : public __swappable_with_details::__do_is_nothrow_swappable_with_impl
    {
      typedef decltype(__test<_Tp, _Up>(0)) type;
    };


  template<typename _Tp>
    struct __is_nothrow_swappable_with_impl<_Tp&, _Tp&>
    : public __swappable_details::__do_is_nothrow_swappable_impl
    {
      typedef decltype(__test<_Tp&>(0)) type;
    };



  template<typename _Tp, typename _Up>
    struct is_swappable_with
    : public __is_swappable_with_impl<_Tp, _Up>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "first template argument must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Up>{}),
 "second template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp, typename _Up>
    struct is_nothrow_swappable_with
    : public __is_nothrow_swappable_with_impl<_Tp, _Up>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "first template argument must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Up>{}),
 "second template argument must be a complete class or an unbounded array");
    };



  template<typename _Tp, typename _Up>
    inline constexpr bool is_swappable_with_v =
      is_swappable_with<_Tp, _Up>::value;


  template<typename _Tp, typename _Up>
    inline constexpr bool is_nothrow_swappable_with_v =
      is_nothrow_swappable_with<_Tp, _Up>::value;
# 2876 "/usr/include/c++/11/type_traits" 3
  template<typename _Result, typename _Ret,
    bool = is_void<_Ret>::value, typename = void>
    struct __is_invocable_impl
    : false_type
    {
      using __nothrow_type = false_type;
    };


  template<typename _Result, typename _Ret>
    struct __is_invocable_impl<_Result, _Ret,
                                true,
          __void_t<typename _Result::type>>
    : true_type
    {
      using __nothrow_type = true_type;
    };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"

  template<typename _Result, typename _Ret>
    struct __is_invocable_impl<_Result, _Ret,
                                false,
          __void_t<typename _Result::type>>
    {
    private:



      static typename _Result::type _S_get() noexcept;

      template<typename _Tp>
 static void _S_conv(_Tp) noexcept;


      template<typename _Tp, bool _Check_Noex = false,
        typename = decltype(_S_conv<_Tp>(_S_get())),
        bool _Noex = noexcept(_S_conv<_Tp>(_S_get()))>
 static __bool_constant<_Check_Noex ? _Noex : true>
 _S_test(int);

      template<typename _Tp, bool = false>
 static false_type
 _S_test(...);

    public:

      using type = decltype(_S_test<_Ret>(1));


      using __nothrow_type = decltype(_S_test<_Ret, true>(1));
    };
#pragma GCC diagnostic pop

  template<typename _Fn, typename... _ArgTypes>
    struct __is_invocable
    : __is_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, void>::type
    { };

  template<typename _Fn, typename _Tp, typename... _Args>
    constexpr bool __call_is_nt(__invoke_memfun_ref)
    {
      using _Up = typename __inv_unwrap<_Tp>::type;
      return noexcept((std::declval<_Up>().*std::declval<_Fn>())(
     std::declval<_Args>()...));
    }

  template<typename _Fn, typename _Tp, typename... _Args>
    constexpr bool __call_is_nt(__invoke_memfun_deref)
    {
      return noexcept(((*std::declval<_Tp>()).*std::declval<_Fn>())(
     std::declval<_Args>()...));
    }

  template<typename _Fn, typename _Tp>
    constexpr bool __call_is_nt(__invoke_memobj_ref)
    {
      using _Up = typename __inv_unwrap<_Tp>::type;
      return noexcept(std::declval<_Up>().*std::declval<_Fn>());
    }

  template<typename _Fn, typename _Tp>
    constexpr bool __call_is_nt(__invoke_memobj_deref)
    {
      return noexcept((*std::declval<_Tp>()).*std::declval<_Fn>());
    }

  template<typename _Fn, typename... _Args>
    constexpr bool __call_is_nt(__invoke_other)
    {
      return noexcept(std::declval<_Fn>()(std::declval<_Args>()...));
    }

  template<typename _Result, typename _Fn, typename... _Args>
    struct __call_is_nothrow
    : __bool_constant<
 std::__call_is_nt<_Fn, _Args...>(typename _Result::__invoke_type{})
      >
    { };

  template<typename _Fn, typename... _Args>
    using __call_is_nothrow_
      = __call_is_nothrow<__invoke_result<_Fn, _Args...>, _Fn, _Args...>;


  template<typename _Fn, typename... _Args>
    struct __is_nothrow_invocable
    : __and_<__is_invocable<_Fn, _Args...>,
             __call_is_nothrow_<_Fn, _Args...>>::type
    { };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
  struct __nonesuchbase {};
  struct __nonesuch : private __nonesuchbase {
    ~__nonesuch() = delete;
    __nonesuch(__nonesuch const&) = delete;
    void operator=(__nonesuch const&) = delete;
  };
#pragma GCC diagnostic pop






  template<typename _Functor, typename... _ArgTypes>
    struct invoke_result
    : public __invoke_result<_Functor, _ArgTypes...>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Functor>{}),
 "_Functor must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
    };


  template<typename _Fn, typename... _Args>
    using invoke_result_t = typename invoke_result<_Fn, _Args...>::type;


  template<typename _Fn, typename... _ArgTypes>
    struct is_invocable
    : __is_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, void>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
    };


  template<typename _Ret, typename _Fn, typename... _ArgTypes>
    struct is_invocable_r
    : __is_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, _Ret>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Ret>{}),
 "_Ret must be a complete class or an unbounded array");
    };


  template<typename _Fn, typename... _ArgTypes>
    struct is_nothrow_invocable
    : __and_<__is_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, void>,
      __call_is_nothrow_<_Fn, _ArgTypes...>>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
    };


  template<typename _Result, typename _Ret>
    using __is_nt_invocable_impl
      = typename __is_invocable_impl<_Result, _Ret>::__nothrow_type;



  template<typename _Ret, typename _Fn, typename... _ArgTypes>
    struct is_nothrow_invocable_r
    : __and_<__is_nt_invocable_impl<__invoke_result<_Fn, _ArgTypes...>, _Ret>,
             __call_is_nothrow_<_Fn, _ArgTypes...>>::type
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Fn>{}),
 "_Fn must be a complete class or an unbounded array");
      static_assert((std::__is_complete_or_unbounded(
 __type_identity<_ArgTypes>{}) && ...),
 "each argument type must be a complete class or an unbounded array");
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Ret>{}),
 "_Ret must be a complete class or an unbounded array");
    };
# 3094 "/usr/include/c++/11/type_traits" 3
template <typename _Tp>
  inline constexpr bool is_void_v = is_void<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_null_pointer_v = is_null_pointer<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_integral_v = is_integral<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_floating_point_v = is_floating_point<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_array_v = is_array<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_pointer_v = is_pointer<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_lvalue_reference_v =
    is_lvalue_reference<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_rvalue_reference_v =
    is_rvalue_reference<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_member_object_pointer_v =
    is_member_object_pointer<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_member_function_pointer_v =
    is_member_function_pointer<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_enum_v = is_enum<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_union_v = is_union<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_class_v = is_class<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_function_v = is_function<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_reference_v = is_reference<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_arithmetic_v = is_arithmetic<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_fundamental_v = is_fundamental<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_object_v = is_object<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_scalar_v = is_scalar<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_compound_v = is_compound<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_member_pointer_v = is_member_pointer<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_const_v = is_const<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_volatile_v = is_volatile<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_trivial_v = is_trivial<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_copyable_v =
    is_trivially_copyable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_standard_layout_v = is_standard_layout<_Tp>::value;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
template <typename _Tp>
 
  inline constexpr bool is_pod_v = is_pod<_Tp>::value;
template <typename _Tp>
  [[__deprecated__]]
  inline constexpr bool is_literal_type_v = is_literal_type<_Tp>::value;
#pragma GCC diagnostic pop
 template <typename _Tp>
  inline constexpr bool is_empty_v = is_empty<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_polymorphic_v = is_polymorphic<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_abstract_v = is_abstract<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_final_v = is_final<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_signed_v = is_signed<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_unsigned_v = is_unsigned<_Tp>::value;
template <typename _Tp, typename... _Args>
  inline constexpr bool is_constructible_v =
    is_constructible<_Tp, _Args...>::value;
template <typename _Tp>
  inline constexpr bool is_default_constructible_v =
    is_default_constructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_copy_constructible_v =
    is_copy_constructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_move_constructible_v =
    is_move_constructible<_Tp>::value;
template <typename _Tp, typename _Up>
  inline constexpr bool is_assignable_v = is_assignable<_Tp, _Up>::value;
template <typename _Tp>
  inline constexpr bool is_copy_assignable_v = is_copy_assignable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_move_assignable_v = is_move_assignable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_destructible_v = is_destructible<_Tp>::value;
template <typename _Tp, typename... _Args>
  inline constexpr bool is_trivially_constructible_v =
    is_trivially_constructible<_Tp, _Args...>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_default_constructible_v =
    is_trivially_default_constructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_copy_constructible_v =
    is_trivially_copy_constructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_move_constructible_v =
    is_trivially_move_constructible<_Tp>::value;
template <typename _Tp, typename _Up>
  inline constexpr bool is_trivially_assignable_v =
    is_trivially_assignable<_Tp, _Up>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_copy_assignable_v =
    is_trivially_copy_assignable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_move_assignable_v =
    is_trivially_move_assignable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_trivially_destructible_v =
    is_trivially_destructible<_Tp>::value;
template <typename _Tp, typename... _Args>
  inline constexpr bool is_nothrow_constructible_v =
    is_nothrow_constructible<_Tp, _Args...>::value;
template <typename _Tp>
  inline constexpr bool is_nothrow_default_constructible_v =
    is_nothrow_default_constructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_nothrow_copy_constructible_v =
    is_nothrow_copy_constructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_nothrow_move_constructible_v =
    is_nothrow_move_constructible<_Tp>::value;
template <typename _Tp, typename _Up>
  inline constexpr bool is_nothrow_assignable_v =
    is_nothrow_assignable<_Tp, _Up>::value;
template <typename _Tp>
  inline constexpr bool is_nothrow_copy_assignable_v =
    is_nothrow_copy_assignable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_nothrow_move_assignable_v =
    is_nothrow_move_assignable<_Tp>::value;
template <typename _Tp>
  inline constexpr bool is_nothrow_destructible_v =
    is_nothrow_destructible<_Tp>::value;
template <typename _Tp>
  inline constexpr bool has_virtual_destructor_v =
    has_virtual_destructor<_Tp>::value;
template <typename _Tp>
  inline constexpr size_t alignment_of_v = alignment_of<_Tp>::value;
template <typename _Tp>
  inline constexpr size_t rank_v = rank<_Tp>::value;
template <typename _Tp, unsigned _Idx = 0>
  inline constexpr size_t extent_v = extent<_Tp, _Idx>::value;

template <typename _Tp, typename _Up>
  inline constexpr bool is_same_v = __is_same(_Tp, _Up);




template <typename _Base, typename _Derived>
  inline constexpr bool is_base_of_v = is_base_of<_Base, _Derived>::value;
template <typename _From, typename _To>
  inline constexpr bool is_convertible_v = is_convertible<_From, _To>::value;
template<typename _Fn, typename... _Args>
  inline constexpr bool is_invocable_v = is_invocable<_Fn, _Args...>::value;
template<typename _Fn, typename... _Args>
  inline constexpr bool is_nothrow_invocable_v
    = is_nothrow_invocable<_Fn, _Args...>::value;
template<typename _Ret, typename _Fn, typename... _Args>
  inline constexpr bool is_invocable_r_v
    = is_invocable_r<_Ret, _Fn, _Args...>::value;
template<typename _Ret, typename _Fn, typename... _Args>
  inline constexpr bool is_nothrow_invocable_r_v
    = is_nothrow_invocable_r<_Ret, _Fn, _Args...>::value;





  template<typename _Tp>
    struct has_unique_object_representations
    : bool_constant<__has_unique_object_representations(
      remove_cv_t<remove_all_extents_t<_Tp>>
      )>
    {
      static_assert(std::__is_complete_or_unbounded(__type_identity<_Tp>{}),
 "template argument must be a complete class or an unbounded array");
    };


  template<typename _Tp>
    inline constexpr bool has_unique_object_representations_v
      = has_unique_object_representations<_Tp>::value;





  template<typename _Tp>
    struct is_aggregate
    : bool_constant<__is_aggregate(remove_cv_t<_Tp>)>
    { };


  template<typename _Tp>
    inline constexpr bool is_aggregate_v = is_aggregate<_Tp>::value;
# 3598 "/usr/include/c++/11/type_traits" 3

}
# 58 "/usr/include/c++/11/bits/move.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{

# 74 "/usr/include/c++/11/bits/move.h" 3
  template<typename _Tp>
    [[__nodiscard__]]
    constexpr _Tp&&
    forward(typename std::remove_reference<_Tp>::type& __t) noexcept
    { return static_cast<_Tp&&>(__t); }







  template<typename _Tp>
    [[__nodiscard__]]
    constexpr _Tp&&
    forward(typename std::remove_reference<_Tp>::type&& __t) noexcept
    {
      static_assert(!std::is_lvalue_reference<_Tp>::value,
   "std::forward must not be used to convert an rvalue to an lvalue");
      return static_cast<_Tp&&>(__t);
    }






  template<typename _Tp>
    [[__nodiscard__]]
    constexpr typename std::remove_reference<_Tp>::type&&
    move(_Tp&& __t) noexcept
    { return static_cast<typename std::remove_reference<_Tp>::type&&>(__t); }


  template<typename _Tp>
    struct __move_if_noexcept_cond
    : public __and_<__not_<is_nothrow_move_constructible<_Tp>>,
                    is_copy_constructible<_Tp>>::type { };
# 121 "/usr/include/c++/11/bits/move.h" 3
  template<typename _Tp>
    [[__nodiscard__]]
    constexpr typename
    conditional<__move_if_noexcept_cond<_Tp>::value, const _Tp&, _Tp&&>::type
    move_if_noexcept(_Tp& __x) noexcept
    { return std::move(__x); }
# 142 "/usr/include/c++/11/bits/move.h" 3
  template<typename _Tp>
    [[__nodiscard__]]
    inline constexpr _Tp*
    addressof(_Tp& __r) noexcept
    { return std::__addressof(__r); }



  template<typename _Tp>
    const _Tp* addressof(const _Tp&&) = delete;


  template <typename _Tp, typename _Up = _Tp>
   
    inline _Tp
    __exchange(_Tp& __obj, _Up&& __new_val)
    {
      _Tp __old_val = std::move(__obj);
      __obj = std::forward<_Up>(__new_val);
      return __old_val;
    }
# 186 "/usr/include/c++/11/bits/move.h" 3
  template<typename _Tp>
   
    inline

    typename enable_if<__and_<__not_<__is_tuple_like<_Tp>>,
         is_move_constructible<_Tp>,
         is_move_assignable<_Tp>>::value>::type



    swap(_Tp& __a, _Tp& __b)
    noexcept(__and_<is_nothrow_move_constructible<_Tp>, is_nothrow_move_assignable<_Tp>>::value)

    {




      _Tp __tmp = std::move(__a);
      __a = std::move(__b);
      __b = std::move(__tmp);
    }




  template<typename _Tp, size_t _Nm>
   
    inline

    typename enable_if<__is_swappable<_Tp>::value>::type



    swap(_Tp (&__a)[_Nm], _Tp (&__b)[_Nm])
    noexcept(__is_nothrow_swappable<_Tp>::value)
    {
      for (size_t __n = 0; __n < _Nm; ++__n)
 swap(__a[__n], __b[__n]);
    }



}
# 44 "/usr/include/c++/11/bits/exception_ptr.h" 2 3
# 52 "/usr/include/c++/11/bits/exception_ptr.h" 3
extern "C++" {

namespace std
{
  class type_info;






  namespace __exception_ptr
  {
    class exception_ptr;
  }

  using __exception_ptr::exception_ptr;





  exception_ptr current_exception() noexcept;

  template<typename _Ex>
  exception_ptr make_exception_ptr(_Ex) noexcept;


  void rethrow_exception(exception_ptr) __attribute__ ((__noreturn__));

  namespace __exception_ptr
  {
    using std::rethrow_exception;





    class exception_ptr
    {
      void* _M_exception_object;

      explicit exception_ptr(void* __e) noexcept;

      void _M_addref() noexcept;
      void _M_release() noexcept;

      void *_M_get() const noexcept __attribute__ ((__pure__));

      friend exception_ptr std::current_exception() noexcept;
      friend void std::rethrow_exception(exception_ptr);
      template<typename _Ex>
      friend exception_ptr std::make_exception_ptr(_Ex) noexcept;

    public:
      exception_ptr() noexcept;

      exception_ptr(const exception_ptr&) noexcept;


      exception_ptr(nullptr_t) noexcept
      : _M_exception_object(nullptr)
      { }

      exception_ptr(exception_ptr&& __o) noexcept
      : _M_exception_object(__o._M_exception_object)
      { __o._M_exception_object = nullptr; }
# 128 "/usr/include/c++/11/bits/exception_ptr.h" 3
      exception_ptr&
      operator=(const exception_ptr&) noexcept;


      exception_ptr&
      operator=(exception_ptr&& __o) noexcept
      {
        exception_ptr(static_cast<exception_ptr&&>(__o)).swap(*this);
        return *this;
      }


      ~exception_ptr() noexcept;

      void
      swap(exception_ptr&) noexcept;
# 155 "/usr/include/c++/11/bits/exception_ptr.h" 3
      explicit operator bool() const noexcept
      { return _M_exception_object; }







      friend bool
      operator==(const exception_ptr& __x, const exception_ptr& __y)
      noexcept
      { return __x._M_exception_object == __y._M_exception_object; }

      friend bool
      operator!=(const exception_ptr& __x, const exception_ptr& __y)
      noexcept
      { return __x._M_exception_object != __y._M_exception_object; }


      const class std::type_info*
      __cxa_exception_type() const noexcept
 __attribute__ ((__pure__));
    };

   
    inline
    exception_ptr::exception_ptr() noexcept
    : _M_exception_object(0)
    { }

   
    inline
    exception_ptr::exception_ptr(const exception_ptr& __other)
    noexcept
    : _M_exception_object(__other._M_exception_object)
    {
      if (_M_exception_object)
 _M_addref();
    }

   
    inline
    exception_ptr::~exception_ptr() noexcept
    {
      if (_M_exception_object)
 _M_release();
    }

   
    inline exception_ptr&
    exception_ptr::operator=(const exception_ptr& __other) noexcept
    {
      exception_ptr(__other).swap(*this);
      return *this;
    }

   
    inline void
    exception_ptr::swap(exception_ptr &__other) noexcept
    {
      void *__tmp = _M_exception_object;
      _M_exception_object = __other._M_exception_object;
      __other._M_exception_object = __tmp;
    }


    inline void
    swap(exception_ptr& __lhs, exception_ptr& __rhs)
    { __lhs.swap(__rhs); }


    template<typename _Ex>
      inline void
      __dest_thunk(void* __x)
      { static_cast<_Ex*>(__x)->~_Ex(); }


  }


  template<typename _Ex>
    exception_ptr
    make_exception_ptr(_Ex __ex) noexcept
    {


      using _Ex2 = typename remove_reference<_Ex>::type;
      void* __e = __cxxabiv1::__cxa_allocate_exception(sizeof(_Ex));
      (void) __cxxabiv1::__cxa_init_primary_exception(
   __e, const_cast<std::type_info*>(&typeid(_Ex)),
   __exception_ptr::__dest_thunk<_Ex2>);
      try
 {
   ::new (__e) _Ex2(std::forward<_Ex>(__ex));
          return exception_ptr(__e);
 }
      catch(...)
 {
   __cxxabiv1::__cxa_free_exception(__e);
   return current_exception();
 }
# 269 "/usr/include/c++/11/bits/exception_ptr.h" 3
    }




}

}

#pragma GCC visibility pop
# 154 "/usr/include/c++/11/exception" 2 3
# 1 "/usr/include/c++/11/bits/nested_exception.h" 1 3
# 33 "/usr/include/c++/11/bits/nested_exception.h" 3
#pragma GCC visibility push(default)
# 42 "/usr/include/c++/11/bits/nested_exception.h" 3
extern "C++" {

namespace std
{






  class nested_exception
  {
    exception_ptr _M_ptr;

  public:
    nested_exception() noexcept : _M_ptr(current_exception()) { }

    nested_exception(const nested_exception&) noexcept = default;

    nested_exception& operator=(const nested_exception&) noexcept = default;

    virtual ~nested_exception() noexcept;

    [[noreturn]]
    void
    rethrow_nested() const
    {
      if (_M_ptr)
 rethrow_exception(_M_ptr);
      std::terminate();
    }

    exception_ptr
    nested_ptr() const noexcept
    { return _M_ptr; }
  };



  template<typename _Except>
    struct _Nested_exception : public _Except, public nested_exception
    {
      explicit _Nested_exception(const _Except& __ex)
      : _Except(__ex)
      { }

      explicit _Nested_exception(_Except&& __ex)
      : _Except(static_cast<_Except&&>(__ex))
      { }
    };




  template<typename _Tp>
    [[noreturn]]
    inline void
    __throw_with_nested_impl(_Tp&& __t, true_type)
    {
      using _Up = typename remove_reference<_Tp>::type;
      throw _Nested_exception<_Up>{std::forward<_Tp>(__t)};
    }

  template<typename _Tp>
    [[noreturn]]
    inline void
    __throw_with_nested_impl(_Tp&& __t, false_type)
    { throw std::forward<_Tp>(__t); }





  template<typename _Tp>
    [[noreturn]]
    inline void
    throw_with_nested(_Tp&& __t)
    {
      using _Up = typename decay<_Tp>::type;
      using _CopyConstructible
 = __and_<is_copy_constructible<_Up>, is_move_constructible<_Up>>;
      static_assert(_CopyConstructible::value,
   "throw_with_nested argument must be CopyConstructible");
      using __nest = __and_<is_class<_Up>, __bool_constant<!__is_final(_Up)>,
       __not_<is_base_of<nested_exception, _Up>>>;
      std::__throw_with_nested_impl(std::forward<_Tp>(__t), __nest{});
    }




  template<typename _Tp>
    using __rethrow_if_nested_cond = typename enable_if<
      __and_<is_polymorphic<_Tp>,
      __or_<__not_<is_base_of<nested_exception, _Tp>>,
     is_convertible<_Tp*, nested_exception*>>>::value
    >::type;


  template<typename _Ex>
    inline __rethrow_if_nested_cond<_Ex>
    __rethrow_if_nested_impl(const _Ex* __ptr)
    {
      if (auto __ne_ptr = dynamic_cast<const nested_exception*>(__ptr))
 __ne_ptr->rethrow_nested();
    }


  inline void
  __rethrow_if_nested_impl(const void*)
  { }




  template<typename _Ex>
    inline void
    rethrow_if_nested(const _Ex& __ex)
    { std::__rethrow_if_nested_impl(std::__addressof(__ex)); }


}

}



#pragma GCC visibility pop
# 155 "/usr/include/c++/11/exception" 2 3
# 40 "/usr/include/c++/11/ios" 2 3
# 1 "/usr/include/c++/11/bits/char_traits.h" 1 3
# 37 "/usr/include/c++/11/bits/char_traits.h" 3
       
# 38 "/usr/include/c++/11/bits/char_traits.h" 3

# 1 "/usr/include/c++/11/bits/stl_algobase.h" 1 3
# 60 "/usr/include/c++/11/bits/stl_algobase.h" 3
# 1 "/usr/include/c++/11/bits/functexcept.h" 1 3
# 42 "/usr/include/c++/11/bits/functexcept.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{



  void
  __throw_bad_exception(void) __attribute__((__noreturn__));


  void
  __throw_bad_alloc(void) __attribute__((__noreturn__));

  void
  __throw_bad_array_new_length(void) __attribute__((__noreturn__));


  void
  __throw_bad_cast(void) __attribute__((__noreturn__));

  void
  __throw_bad_typeid(void) __attribute__((__noreturn__));


  void
  __throw_logic_error(const char*) __attribute__((__noreturn__));

  void
  __throw_domain_error(const char*) __attribute__((__noreturn__));

  void
  __throw_invalid_argument(const char*) __attribute__((__noreturn__));

  void
  __throw_length_error(const char*) __attribute__((__noreturn__));

  void
  __throw_out_of_range(const char*) __attribute__((__noreturn__));

  void
  __throw_out_of_range_fmt(const char*, ...) __attribute__((__noreturn__))
    __attribute__((__format__(__gnu_printf__, 1, 2)));

  void
  __throw_runtime_error(const char*) __attribute__((__noreturn__));

  void
  __throw_range_error(const char*) __attribute__((__noreturn__));

  void
  __throw_overflow_error(const char*) __attribute__((__noreturn__));

  void
  __throw_underflow_error(const char*) __attribute__((__noreturn__));


  void
  __throw_ios_failure(const char*) __attribute__((__noreturn__));

  void
  __throw_ios_failure(const char*, int) __attribute__((__noreturn__));


  void
  __throw_system_error(int) __attribute__((__noreturn__));


  void
  __throw_future_error(int) __attribute__((__noreturn__));


  void
  __throw_bad_function_call() __attribute__((__noreturn__));


}
# 61 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/bits/cpp_type_traits.h" 1 3
# 35 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
       
# 36 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
# 67 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
extern "C++" {

namespace std __attribute__ ((__visibility__ ("default")))
{


  struct __true_type { };
  struct __false_type { };

  template<bool>
    struct __truth_type
    { typedef __false_type __type; };

  template<>
    struct __truth_type<true>
    { typedef __true_type __type; };



  template<class _Sp, class _Tp>
    struct __traitor
    {
      enum { __value = bool(_Sp::__value) || bool(_Tp::__value) };
      typedef typename __truth_type<__value>::__type __type;
    };


  template<typename, typename>
    struct __are_same
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };

  template<typename _Tp>
    struct __are_same<_Tp, _Tp>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };


  template<typename _Tp>
    struct __is_void
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };

  template<>
    struct __is_void<void>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };




  template<typename _Tp>
    struct __is_integer
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };





  template<>
    struct __is_integer<bool>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<signed char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<unsigned char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };


  template<>
    struct __is_integer<wchar_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
# 184 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
  template<>
    struct __is_integer<char16_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<char32_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };


  template<>
    struct __is_integer<short>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<unsigned short>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<int>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<unsigned int>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<unsigned long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<long long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_integer<unsigned long long>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
# 287 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
  template<typename _Tp>
    struct __is_floating
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };


  template<>
    struct __is_floating<float>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_floating<double>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_floating<long double>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };




  template<typename _Tp>
    struct __is_pointer
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };

  template<typename _Tp>
    struct __is_pointer<_Tp*>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };




  template<typename _Tp>
    struct __is_arithmetic
    : public __traitor<__is_integer<_Tp>, __is_floating<_Tp> >
    { };




  template<typename _Tp>
    struct __is_scalar
    : public __traitor<__is_arithmetic<_Tp>, __is_pointer<_Tp> >
    { };




  template<typename _Tp>
    struct __is_char
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };

  template<>
    struct __is_char<char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };


  template<>
    struct __is_char<wchar_t>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };


  template<typename _Tp>
    struct __is_byte
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };

  template<>
    struct __is_byte<char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_byte<signed char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<>
    struct __is_byte<unsigned char>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };


  enum class byte : unsigned char;

  template<>
    struct __is_byte<byte>
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };
# 423 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
  template<typename> struct iterator_traits;


  template<typename _Tp>
    struct __is_nonvolatile_trivially_copyable
    {
      enum { __value = __is_trivially_copyable(_Tp) };
    };




  template<typename _Tp>
    struct __is_nonvolatile_trivially_copyable<volatile _Tp>
    {
      enum { __value = 0 };
    };


  template<typename _OutputIter, typename _InputIter>
    struct __memcpyable
    {
      enum { __value = 0 };
    };

  template<typename _Tp>
    struct __memcpyable<_Tp*, _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };

  template<typename _Tp>
    struct __memcpyable<_Tp*, const _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };






  template<typename _Iter1, typename _Iter2>
    struct __memcmpable
    {
      enum { __value = 0 };
    };


  template<typename _Tp>
    struct __memcmpable<_Tp*, _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };

  template<typename _Tp>
    struct __memcmpable<const _Tp*, _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };

  template<typename _Tp>
    struct __memcmpable<_Tp*, const _Tp*>
    : __is_nonvolatile_trivially_copyable<_Tp>
    { };







  template<typename _Tp, bool _TreatAsBytes =



 __is_byte<_Tp>::__value

    >
    struct __is_memcmp_ordered
    {
      static const bool __value = _Tp(-1) > _Tp(1);
    };

  template<typename _Tp>
    struct __is_memcmp_ordered<_Tp, false>
    {
      static const bool __value = false;
    };


  template<typename _Tp, typename _Up, bool = sizeof(_Tp) == sizeof(_Up)>
    struct __is_memcmp_ordered_with
    {
      static const bool __value = __is_memcmp_ordered<_Tp>::__value
 && __is_memcmp_ordered<_Up>::__value;
    };

  template<typename _Tp, typename _Up>
    struct __is_memcmp_ordered_with<_Tp, _Up, false>
    {
      static const bool __value = false;
    };
# 532 "/usr/include/c++/11/bits/cpp_type_traits.h" 3
  template<>
    struct __is_memcmp_ordered_with<std::byte, std::byte, true>
    { static constexpr bool __value = true; };

  template<typename _Tp, bool _SameSize>
    struct __is_memcmp_ordered_with<_Tp, std::byte, _SameSize>
    { static constexpr bool __value = false; };

  template<typename _Up, bool _SameSize>
    struct __is_memcmp_ordered_with<std::byte, _Up, _SameSize>
    { static constexpr bool __value = false; };





  template<typename _Tp>
    struct __is_move_iterator
    {
      enum { __value = 0 };
      typedef __false_type __type;
    };



  template<typename _Iterator>
   
    inline _Iterator
    __miter_base(_Iterator __it)
    { return __it; }


}
}
# 62 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/ext/type_traits.h" 1 3
# 32 "/usr/include/c++/11/ext/type_traits.h" 3
       
# 33 "/usr/include/c++/11/ext/type_traits.h" 3




extern "C++" {

namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{



  template<bool, typename>
    struct __enable_if
    { };

  template<typename _Tp>
    struct __enable_if<true, _Tp>
    { typedef _Tp __type; };



  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    struct __conditional_type
    { typedef _Iftrue __type; };

  template<typename _Iftrue, typename _Iffalse>
    struct __conditional_type<false, _Iftrue, _Iffalse>
    { typedef _Iffalse __type; };



  template<typename _Tp>
    struct __add_unsigned
    {
    private:
      typedef __enable_if<std::__is_integer<_Tp>::__value, _Tp> __if_type;

    public:
      typedef typename __if_type::__type __type;
    };

  template<>
    struct __add_unsigned<char>
    { typedef unsigned char __type; };

  template<>
    struct __add_unsigned<signed char>
    { typedef unsigned char __type; };

  template<>
    struct __add_unsigned<short>
    { typedef unsigned short __type; };

  template<>
    struct __add_unsigned<int>
    { typedef unsigned int __type; };

  template<>
    struct __add_unsigned<long>
    { typedef unsigned long __type; };

  template<>
    struct __add_unsigned<long long>
    { typedef unsigned long long __type; };


  template<>
    struct __add_unsigned<bool>;

  template<>
    struct __add_unsigned<wchar_t>;



  template<typename _Tp>
    struct __remove_unsigned
    {
    private:
      typedef __enable_if<std::__is_integer<_Tp>::__value, _Tp> __if_type;

    public:
      typedef typename __if_type::__type __type;
    };

  template<>
    struct __remove_unsigned<char>
    { typedef signed char __type; };

  template<>
    struct __remove_unsigned<unsigned char>
    { typedef signed char __type; };

  template<>
    struct __remove_unsigned<unsigned short>
    { typedef short __type; };

  template<>
    struct __remove_unsigned<unsigned int>
    { typedef int __type; };

  template<>
    struct __remove_unsigned<unsigned long>
    { typedef long __type; };

  template<>
    struct __remove_unsigned<unsigned long long>
    { typedef long long __type; };


  template<>
    struct __remove_unsigned<bool>;

  template<>
    struct __remove_unsigned<wchar_t>;



  template<typename _Type>
    inline bool
    __is_null_pointer(_Type* __ptr)
    { return __ptr == 0; }

  template<typename _Type>
    inline bool
    __is_null_pointer(_Type)
    { return false; }


  inline bool
  __is_null_pointer(std::nullptr_t)
  { return true; }




  template<typename _Tp, bool = std::__is_integer<_Tp>::__value>
    struct __promote
    { typedef double __type; };




  template<typename _Tp>
    struct __promote<_Tp, false>
    { };

  template<>
    struct __promote<long double>
    { typedef long double __type; };

  template<>
    struct __promote<double>
    { typedef double __type; };

  template<>
    struct __promote<float>
    { typedef float __type; };


  template<typename... _Tp>
    using __promoted_t = decltype((typename __promote<_Tp>::__type(0) + ...));


  template<typename _Tp, typename _Up,
           typename _Tp2 = typename __promote<_Tp>::__type,
           typename _Up2 = typename __promote<_Up>::__type>
    struct __promote_2
    {
      typedef __typeof__(_Tp2() + _Up2()) __type;
    };

  template<typename _Tp, typename _Up, typename _Vp,
           typename _Tp2 = typename __promote<_Tp>::__type,
           typename _Up2 = typename __promote<_Up>::__type,
           typename _Vp2 = typename __promote<_Vp>::__type>
    struct __promote_3
    {
      typedef __typeof__(_Tp2() + _Up2() + _Vp2()) __type;
    };

  template<typename _Tp, typename _Up, typename _Vp, typename _Wp,
           typename _Tp2 = typename __promote<_Tp>::__type,
           typename _Up2 = typename __promote<_Up>::__type,
           typename _Vp2 = typename __promote<_Vp>::__type,
           typename _Wp2 = typename __promote<_Wp>::__type>
    struct __promote_4
    {
      typedef __typeof__(_Tp2() + _Up2() + _Vp2() + _Wp2()) __type;
    };


}
}
# 63 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/ext/numeric_traits.h" 1 3
# 32 "/usr/include/c++/11/ext/numeric_traits.h" 3
       
# 33 "/usr/include/c++/11/ext/numeric_traits.h" 3




namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

# 50 "/usr/include/c++/11/ext/numeric_traits.h" 3
  template<typename _Tp>
    struct __is_integer_nonstrict
    : public std::__is_integer<_Tp>
    {
      using std::__is_integer<_Tp>::__value;


      enum { __width = __value ? sizeof(_Tp) * 8 : 0 };
    };

  template<typename _Value>
    struct __numeric_traits_integer
    {

      static_assert(__is_integer_nonstrict<_Value>::__value,
      "invalid specialization");




      static const bool __is_signed = (_Value)(-1) < 0;
      static const int __digits
 = __is_integer_nonstrict<_Value>::__width - __is_signed;


      static const _Value __max = __is_signed
 ? (((((_Value)1 << (__digits - 1)) - 1) << 1) + 1)
 : ~(_Value)0;
      static const _Value __min = __is_signed ? -__max - 1 : (_Value)0;
    };

  template<typename _Value>
    const _Value __numeric_traits_integer<_Value>::__min;

  template<typename _Value>
    const _Value __numeric_traits_integer<_Value>::__max;

  template<typename _Value>
    const bool __numeric_traits_integer<_Value>::__is_signed;

  template<typename _Value>
    const int __numeric_traits_integer<_Value>::__digits;
# 128 "/usr/include/c++/11/ext/numeric_traits.h" 3
  template<> struct __is_integer_nonstrict<__int128> { enum { __value = 1 }; typedef std::__true_type __type; enum { __width = 128 }; }; template<> struct __is_integer_nonstrict<unsigned __int128> { enum { __value = 1 }; typedef std::__true_type __type; enum { __width = 128 }; };






  template<typename _Tp>
    using __int_traits = __numeric_traits_integer<_Tp>;
# 155 "/usr/include/c++/11/ext/numeric_traits.h" 3
  template<typename _Value>
    struct __numeric_traits_floating
    {

      static const int __max_digits10 = (2 + (std::__are_same<_Value, float>::__value ? 24 : std::__are_same<_Value, double>::__value ? 53 : 64) * 643L / 2136);


      static const bool __is_signed = true;
      static const int __digits10 = (std::__are_same<_Value, float>::__value ? 6 : std::__are_same<_Value, double>::__value ? 15 : 18);
      static const int __max_exponent10 = (std::__are_same<_Value, float>::__value ? 38 : std::__are_same<_Value, double>::__value ? 308 : 4932);
    };

  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__max_digits10;

  template<typename _Value>
    const bool __numeric_traits_floating<_Value>::__is_signed;

  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__digits10;

  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__max_exponent10;






  template<typename _Value>
    struct __numeric_traits
    : public __numeric_traits_integer<_Value>
    { };

  template<>
    struct __numeric_traits<float>
    : public __numeric_traits_floating<float>
    { };

  template<>
    struct __numeric_traits<double>
    : public __numeric_traits_floating<double>
    { };

  template<>
    struct __numeric_traits<long double>
    : public __numeric_traits_floating<long double>
    { };
# 236 "/usr/include/c++/11/ext/numeric_traits.h" 3

}
# 64 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/bits/stl_pair.h" 1 3
# 69 "/usr/include/c++/11/bits/stl_pair.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{

# 80 "/usr/include/c++/11/bits/stl_pair.h" 3
  struct piecewise_construct_t { explicit piecewise_construct_t() = default; };


  inline constexpr piecewise_construct_t piecewise_construct =
    piecewise_construct_t();




  template<typename...>
    class tuple;

  template<size_t...>
    struct _Index_tuple;






  template <bool, typename _T1, typename _T2>
    struct _PCC
    {
      template <typename _U1, typename _U2>
      static constexpr bool _ConstructiblePair()
      {
 return __and_<is_constructible<_T1, const _U1&>,
        is_constructible<_T2, const _U2&>>::value;
      }

      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyConvertiblePair()
      {
 return __and_<is_convertible<const _U1&, _T1>,
        is_convertible<const _U2&, _T2>>::value;
      }

      template <typename _U1, typename _U2>
      static constexpr bool _MoveConstructiblePair()
      {
 return __and_<is_constructible<_T1, _U1&&>,
        is_constructible<_T2, _U2&&>>::value;
      }

      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyMoveConvertiblePair()
      {
 return __and_<is_convertible<_U1&&, _T1>,
        is_convertible<_U2&&, _T2>>::value;
      }

      template <bool __implicit, typename _U1, typename _U2>
      static constexpr bool _CopyMovePair()
      {
 using __do_converts = __and_<is_convertible<const _U1&, _T1>,
      is_convertible<_U2&&, _T2>>;
 using __converts = typename conditional<__implicit,
           __do_converts,
           __not_<__do_converts>>::type;
 return __and_<is_constructible<_T1, const _U1&>,
        is_constructible<_T2, _U2&&>,
        __converts
        >::value;
      }

      template <bool __implicit, typename _U1, typename _U2>
      static constexpr bool _MoveCopyPair()
      {
 using __do_converts = __and_<is_convertible<_U1&&, _T1>,
      is_convertible<const _U2&, _T2>>;
 using __converts = typename conditional<__implicit,
           __do_converts,
           __not_<__do_converts>>::type;
 return __and_<is_constructible<_T1, _U1&&>,
        is_constructible<_T2, const _U2&&>,
        __converts
        >::value;
      }
  };

  template <typename _T1, typename _T2>
    struct _PCC<false, _T1, _T2>
    {
      template <typename _U1, typename _U2>
      static constexpr bool _ConstructiblePair()
      {
 return false;
      }

      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyConvertiblePair()
      {
 return false;
      }

      template <typename _U1, typename _U2>
      static constexpr bool _MoveConstructiblePair()
      {
 return false;
      }

      template <typename _U1, typename _U2>
      static constexpr bool _ImplicitlyMoveConvertiblePair()
      {
 return false;
      }
  };


  template<typename _U1, typename _U2> class __pair_base
  {

    template<typename _T1, typename _T2> friend struct pair;
    __pair_base() = default;
    ~__pair_base() = default;
    __pair_base(const __pair_base&) = default;
    __pair_base& operator=(const __pair_base&) = delete;

  };
# 210 "/usr/include/c++/11/bits/stl_pair.h" 3
  template<typename _T1, typename _T2>
    struct pair
    : private __pair_base<_T1, _T2>
    {
      typedef _T1 first_type;
      typedef _T2 second_type;

      _T1 first;
      _T2 second;






      template <typename _U1 = _T1,
                typename _U2 = _T2,
                typename enable_if<__and_<
                                     __is_implicitly_default_constructible<_U1>,
                                     __is_implicitly_default_constructible<_U2>>
                                   ::value, bool>::type = true>

      constexpr pair()
      : first(), second() { }


      template <typename _U1 = _T1,
                typename _U2 = _T2,
                typename enable_if<__and_<
                       is_default_constructible<_U1>,
                       is_default_constructible<_U2>,
                       __not_<
                         __and_<__is_implicitly_default_constructible<_U1>,
                                __is_implicitly_default_constructible<_U2>>>>
                                   ::value, bool>::type = false>
      explicit constexpr pair()
      : first(), second() { }
# 256 "/usr/include/c++/11/bits/stl_pair.h" 3
      using _PCCP = _PCC<true, _T1, _T2>;



      template<typename _U1 = _T1, typename _U2=_T2, typename
        enable_if<_PCCP::template
      _ConstructiblePair<_U1, _U2>()
                  && _PCCP::template
      _ImplicitlyConvertiblePair<_U1, _U2>(),
                         bool>::type=true>
      constexpr pair(const _T1& __a, const _T2& __b)
      : first(__a), second(__b) { }


       template<typename _U1 = _T1, typename _U2=_T2, typename
  enable_if<_PCCP::template
       _ConstructiblePair<_U1, _U2>()
                   && !_PCCP::template
       _ImplicitlyConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
      explicit constexpr pair(const _T1& __a, const _T2& __b)
      : first(__a), second(__b) { }
# 288 "/usr/include/c++/11/bits/stl_pair.h" 3
      template <typename _U1, typename _U2>
        using _PCCFP = _PCC<!is_same<_T1, _U1>::value
       || !is_same<_T2, _U2>::value,
       _T1, _T2>;


      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _ConstructiblePair<_U1, _U2>()
                  && _PCCFP<_U1, _U2>::template
      _ImplicitlyConvertiblePair<_U1, _U2>(),
     bool>::type=true>
        constexpr pair(const pair<_U1, _U2>& __p)
        : first(__p.first), second(__p.second) { }

      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _ConstructiblePair<_U1, _U2>()
    && !_PCCFP<_U1, _U2>::template
      _ImplicitlyConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
 explicit constexpr pair(const pair<_U1, _U2>& __p)
 : first(__p.first), second(__p.second) { }



      constexpr pair(const pair&) = default;
      constexpr pair(pair&&) = default;


      template<typename _U1, typename
        enable_if<_PCCP::template
      _MoveCopyPair<true, _U1, _T2>(),
                         bool>::type=true>
       constexpr pair(_U1&& __x, const _T2& __y)
       : first(std::forward<_U1>(__x)), second(__y) { }

      template<typename _U1, typename
        enable_if<_PCCP::template
      _MoveCopyPair<false, _U1, _T2>(),
                         bool>::type=false>
       explicit constexpr pair(_U1&& __x, const _T2& __y)
       : first(std::forward<_U1>(__x)), second(__y) { }

      template<typename _U2, typename
        enable_if<_PCCP::template
      _CopyMovePair<true, _T1, _U2>(),
                         bool>::type=true>
       constexpr pair(const _T1& __x, _U2&& __y)
       : first(__x), second(std::forward<_U2>(__y)) { }

      template<typename _U2, typename
        enable_if<_PCCP::template
      _CopyMovePair<false, _T1, _U2>(),
                         bool>::type=false>
       explicit pair(const _T1& __x, _U2&& __y)
       : first(__x), second(std::forward<_U2>(__y)) { }

      template<typename _U1, typename _U2, typename
        enable_if<_PCCP::template
      _MoveConstructiblePair<_U1, _U2>()
     && _PCCP::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=true>
 constexpr pair(_U1&& __x, _U2&& __y)
 : first(std::forward<_U1>(__x)), second(std::forward<_U2>(__y)) { }

      template<typename _U1, typename _U2, typename
        enable_if<_PCCP::template
      _MoveConstructiblePair<_U1, _U2>()
     && !_PCCP::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
 explicit constexpr pair(_U1&& __x, _U2&& __y)
 : first(std::forward<_U1>(__x)), second(std::forward<_U2>(__y)) { }


      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _MoveConstructiblePair<_U1, _U2>()
     && _PCCFP<_U1, _U2>::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=true>
 constexpr pair(pair<_U1, _U2>&& __p)
 : first(std::forward<_U1>(__p.first)),
   second(std::forward<_U2>(__p.second)) { }

      template<typename _U1, typename _U2, typename
        enable_if<_PCCFP<_U1, _U2>::template
      _MoveConstructiblePair<_U1, _U2>()
     && !_PCCFP<_U1, _U2>::template
      _ImplicitlyMoveConvertiblePair<_U1, _U2>(),
                         bool>::type=false>
 explicit constexpr pair(pair<_U1, _U2>&& __p)
 : first(std::forward<_U1>(__p.first)),
   second(std::forward<_U2>(__p.second)) { }

      template<typename... _Args1, typename... _Args2>

        pair(piecewise_construct_t, tuple<_Args1...>, tuple<_Args2...>);

      pair&
      operator=(typename conditional<
  __and_<is_copy_assignable<_T1>,
         is_copy_assignable<_T2>>::value,
  const pair&, const __nonesuch&>::type __p)
      {
 first = __p.first;
 second = __p.second;
 return *this;
      }

      pair&
      operator=(typename conditional<
  __and_<is_move_assignable<_T1>,
         is_move_assignable<_T2>>::value,
  pair&&, __nonesuch&&>::type __p)
      noexcept(__and_<is_nothrow_move_assignable<_T1>,
        is_nothrow_move_assignable<_T2>>::value)
      {
 first = std::forward<first_type>(__p.first);
 second = std::forward<second_type>(__p.second);
 return *this;
      }

      template<typename _U1, typename _U2>

 typename enable_if<__and_<is_assignable<_T1&, const _U1&>,
      is_assignable<_T2&, const _U2&>>::value,
      pair&>::type
 operator=(const pair<_U1, _U2>& __p)
 {
   first = __p.first;
   second = __p.second;
   return *this;
 }

      template<typename _U1, typename _U2>

 typename enable_if<__and_<is_assignable<_T1&, _U1&&>,
      is_assignable<_T2&, _U2&&>>::value,
      pair&>::type
 operator=(pair<_U1, _U2>&& __p)
 {
   first = std::forward<_U1>(__p.first);
   second = std::forward<_U2>(__p.second);
   return *this;
 }


      void
      swap(pair& __p)
      noexcept(__and_<__is_nothrow_swappable<_T1>,
                      __is_nothrow_swappable<_T2>>::value)
      {
 using std::swap;
 swap(first, __p.first);
 swap(second, __p.second);
      }

    private:
      template<typename... _Args1, size_t... _Indexes1,
        typename... _Args2, size_t... _Indexes2>

        pair(tuple<_Args1...>&, tuple<_Args2...>&,
      _Index_tuple<_Indexes1...>, _Index_tuple<_Indexes2...>);

    };




  template<typename _T1, typename _T2> pair(_T1, _T2) -> pair<_T1, _T2>;



  template<typename _T1, typename _T2>
    inline constexpr bool
    operator==(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __x.first == __y.first && __x.second == __y.second; }
# 487 "/usr/include/c++/11/bits/stl_pair.h" 3
  template<typename _T1, typename _T2>
    inline constexpr bool
    operator<(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __x.first < __y.first
      || (!(__y.first < __x.first) && __x.second < __y.second); }


  template<typename _T1, typename _T2>
    inline constexpr bool
    operator!=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__x == __y); }


  template<typename _T1, typename _T2>
    inline constexpr bool
    operator>(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return __y < __x; }


  template<typename _T1, typename _T2>
    inline constexpr bool
    operator<=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__y < __x); }


  template<typename _T1, typename _T2>
    inline constexpr bool
    operator>=(const pair<_T1, _T2>& __x, const pair<_T1, _T2>& __y)
    { return !(__x < __y); }
# 524 "/usr/include/c++/11/bits/stl_pair.h" 3
  template<typename _T1, typename _T2>
    inline


    typename enable_if<__and_<__is_swappable<_T1>,
                              __is_swappable<_T2>>::value>::type



    swap(pair<_T1, _T2>& __x, pair<_T1, _T2>& __y)
    noexcept(noexcept(__x.swap(__y)))
    { __x.swap(__y); }


  template<typename _T1, typename _T2>
    typename enable_if<!__and_<__is_swappable<_T1>,
          __is_swappable<_T2>>::value>::type
    swap(pair<_T1, _T2>&, pair<_T1, _T2>&) = delete;
# 564 "/usr/include/c++/11/bits/stl_pair.h" 3
  template<typename _T1, typename _T2>
    constexpr pair<typename __decay_and_strip<_T1>::__type,
                   typename __decay_and_strip<_T2>::__type>
    make_pair(_T1&& __x, _T2&& __y)
    {
      typedef typename __decay_and_strip<_T1>::__type __ds_type1;
      typedef typename __decay_and_strip<_T2>::__type __ds_type2;
      typedef pair<__ds_type1, __ds_type2> __pair_type;
      return __pair_type(std::forward<_T1>(__x), std::forward<_T2>(__y));
    }
# 583 "/usr/include/c++/11/bits/stl_pair.h" 3

}
# 65 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 1 3
# 62 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
       
# 63 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
# 74 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{

# 93 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
  struct input_iterator_tag { };


  struct output_iterator_tag { };


  struct forward_iterator_tag : public input_iterator_tag { };



  struct bidirectional_iterator_tag : public forward_iterator_tag { };



  struct random_access_iterator_tag : public bidirectional_iterator_tag { };
# 125 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
  template<typename _Category, typename _Tp, typename _Distance = ptrdiff_t,
           typename _Pointer = _Tp*, typename _Reference = _Tp&>
    struct iterator
    {

      typedef _Category iterator_category;

      typedef _Tp value_type;

      typedef _Distance difference_type;

      typedef _Pointer pointer;

      typedef _Reference reference;
    };
# 149 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
  template<typename _Iterator>
    struct iterator_traits;




  template<typename _Iterator, typename = __void_t<>>
    struct __iterator_traits { };



  template<typename _Iterator>
    struct __iterator_traits<_Iterator,
        __void_t<typename _Iterator::iterator_category,
          typename _Iterator::value_type,
          typename _Iterator::difference_type,
          typename _Iterator::pointer,
          typename _Iterator::reference>>
    {
      typedef typename _Iterator::iterator_category iterator_category;
      typedef typename _Iterator::value_type value_type;
      typedef typename _Iterator::difference_type difference_type;
      typedef typename _Iterator::pointer pointer;
      typedef typename _Iterator::reference reference;
    };


  template<typename _Iterator>
    struct iterator_traits
    : public __iterator_traits<_Iterator> { };
# 209 "/usr/include/c++/11/bits/stl_iterator_base_types.h" 3
  template<typename _Tp>
    struct iterator_traits<_Tp*>
    {
      typedef random_access_iterator_tag iterator_category;
      typedef _Tp value_type;
      typedef ptrdiff_t difference_type;
      typedef _Tp* pointer;
      typedef _Tp& reference;
    };


  template<typename _Tp>
    struct iterator_traits<const _Tp*>
    {
      typedef random_access_iterator_tag iterator_category;
      typedef _Tp value_type;
      typedef ptrdiff_t difference_type;
      typedef const _Tp* pointer;
      typedef const _Tp& reference;
    };






  template<typename _Iter>
    inline constexpr
    typename iterator_traits<_Iter>::iterator_category
    __iterator_category(const _Iter&)
    { return typename iterator_traits<_Iter>::iterator_category(); }




  template<typename _Iter>
    using __iterator_category_t
      = typename iterator_traits<_Iter>::iterator_category;

  template<typename _InIter>
    using _RequireInputIter =
      __enable_if_t<is_convertible<__iterator_category_t<_InIter>,
       input_iterator_tag>::value>;

  template<typename _It,
    typename _Cat = __iterator_category_t<_It>>
    struct __is_random_access_iter
      : is_base_of<random_access_iterator_tag, _Cat>
    {
      typedef is_base_of<random_access_iterator_tag, _Cat> _Base;
      enum { __value = _Base::value };
    };








}
# 66 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 1 3
# 62 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 3
       
# 63 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 3

# 1 "/usr/include/c++/11/bits/concept_check.h" 1 3
# 33 "/usr/include/c++/11/bits/concept_check.h" 3
       
# 34 "/usr/include/c++/11/bits/concept_check.h" 3
# 65 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 2 3
# 1 "/usr/include/c++/11/debug/assertions.h" 1 3
# 66 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{




  template <typename> struct _List_iterator;
  template <typename> struct _List_const_iterator;


  template<typename _InputIterator>
    inline constexpr
    typename iterator_traits<_InputIterator>::difference_type
    __distance(_InputIterator __first, _InputIterator __last,
               input_iterator_tag)
    {

     

      typename iterator_traits<_InputIterator>::difference_type __n = 0;
      while (__first != __last)
 {
   ++__first;
   ++__n;
 }
      return __n;
    }

  template<typename _RandomAccessIterator>
    inline constexpr
    typename iterator_traits<_RandomAccessIterator>::difference_type
    __distance(_RandomAccessIterator __first, _RandomAccessIterator __last,
               random_access_iterator_tag)
    {

     

      return __last - __first;
    }



  template<typename _Tp>
    ptrdiff_t
    __distance(std::_List_iterator<_Tp>,
        std::_List_iterator<_Tp>,
        input_iterator_tag);

  template<typename _Tp>
    ptrdiff_t
    __distance(std::_List_const_iterator<_Tp>,
        std::_List_const_iterator<_Tp>,
        input_iterator_tag);
# 135 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 3
  template<typename _InputIterator>
    inline constexpr
    typename iterator_traits<_InputIterator>::difference_type
    distance(_InputIterator __first, _InputIterator __last)
    {

      return std::__distance(__first, __last,
        std::__iterator_category(__first));
    }

  template<typename _InputIterator, typename _Distance>
    inline constexpr void
    __advance(_InputIterator& __i, _Distance __n, input_iterator_tag)
    {

     
      do { if (__builtin_is_constant_evaluated() && !bool(__n >= 0)) __builtin_unreachable(); } while (false);
      while (__n--)
 ++__i;
    }

  template<typename _BidirectionalIterator, typename _Distance>
    inline constexpr void
    __advance(_BidirectionalIterator& __i, _Distance __n,
       bidirectional_iterator_tag)
    {

     

      if (__n > 0)
        while (__n--)
   ++__i;
      else
        while (__n++)
   --__i;
    }

  template<typename _RandomAccessIterator, typename _Distance>
    inline constexpr void
    __advance(_RandomAccessIterator& __i, _Distance __n,
              random_access_iterator_tag)
    {

     

      if (__builtin_constant_p(__n) && __n == 1)
 ++__i;
      else if (__builtin_constant_p(__n) && __n == -1)
 --__i;
      else
 __i += __n;
    }
# 200 "/usr/include/c++/11/bits/stl_iterator_base_funcs.h" 3
  template<typename _InputIterator, typename _Distance>
    inline constexpr void
    advance(_InputIterator& __i, _Distance __n)
    {

      typename iterator_traits<_InputIterator>::difference_type __d = __n;
      std::__advance(__i, __d, std::__iterator_category(__i));
    }



  template<typename _InputIterator>
    inline constexpr _InputIterator
    next(_InputIterator __x, typename
  iterator_traits<_InputIterator>::difference_type __n = 1)
    {

     
      std::advance(__x, __n);
      return __x;
    }

  template<typename _BidirectionalIterator>
    inline constexpr _BidirectionalIterator
    prev(_BidirectionalIterator __x, typename
  iterator_traits<_BidirectionalIterator>::difference_type __n = 1)
    {

     

      std::advance(__x, -__n);
      return __x;
    }




}
# 67 "/usr/include/c++/11/bits/stl_algobase.h" 2 3
# 1 "/usr/include/c++/11/bits/stl_iterator.h" 1 3
# 67 "/usr/include/c++/11/bits/stl_iterator.h" 3
# 1 "/usr/include/c++/11/bits/ptr_traits.h" 1 3
# 42 "/usr/include/c++/11/bits/ptr_traits.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{


  class __undefined;


  template<typename _Tp, typename _Up>
    struct __replace_first_arg
    { };

  template<template<typename, typename...> class _Template, typename _Up,
           typename _Tp, typename... _Types>
    struct __replace_first_arg<_Template<_Tp, _Types...>, _Up>
    { using type = _Template<_Up, _Types...>; };

  template<typename _Tp, typename _Up>
    using __replace_first_arg_t = typename __replace_first_arg<_Tp, _Up>::type;

  template<typename _Tp>
    using __make_not_void
      = typename conditional<is_void<_Tp>::value, __undefined, _Tp>::type;

  template<typename _Ptr>
    struct __ptr_traits_elem_1
    { };

  template<template<typename, typename...> class _SomePointer, typename _Tp,
    typename... _Args>
    struct __ptr_traits_elem_1<_SomePointer<_Tp, _Args...>>
    {
      using element_type = _Tp;
      using pointer = _SomePointer<_Tp, _Args...>;

      static pointer
      pointer_to(__make_not_void<element_type>& __e)
      { return pointer::pointer_to(__e); }
    };

  template<typename _Ptr, typename = void>
    struct __ptr_traits_elem : __ptr_traits_elem_1<_Ptr>
    { };

  template<typename _Ptr>
    struct __ptr_traits_elem<_Ptr, __void_t<typename _Ptr::element_type>>
    {
      using element_type = typename _Ptr::element_type;

      static _Ptr
      pointer_to(__make_not_void<element_type>& __e)
      { return _Ptr::pointer_to(__e); }
    };





  template<typename _Ptr>
    struct pointer_traits : __ptr_traits_elem<_Ptr>
    {
    private:
      template<typename _Tp>
 using __difference_type = typename _Tp::difference_type;

      template<typename _Tp, typename _Up, typename = void>
 struct __rebind : __replace_first_arg<_Tp, _Up> { };

      template<typename _Tp, typename _Up>
 struct __rebind<_Tp, _Up, __void_t<typename _Tp::template rebind<_Up>>>
 { using type = typename _Tp::template rebind<_Up>; };

    public:

      using pointer = _Ptr;


      using difference_type
 = __detected_or_t<ptrdiff_t, __difference_type, _Ptr>;


      template<typename _Up>
        using rebind = typename __rebind<_Ptr, _Up>::type;
    };





  template<typename _Tp>
    struct pointer_traits<_Tp*>
    {

      typedef _Tp* pointer;

      typedef _Tp element_type;

      typedef ptrdiff_t difference_type;

      template<typename _Up>
        using rebind = _Up*;






      static pointer
      pointer_to(__make_not_void<element_type>& __r) noexcept
      { return std::addressof(__r); }
    };


  template<typename _Ptr, typename _Tp>
    using __ptr_rebind = typename pointer_traits<_Ptr>::template rebind<_Tp>;

  template<typename _Tp>
    constexpr _Tp*
    __to_address(_Tp* __ptr) noexcept
    {
      static_assert(!std::is_function<_Tp>::value, "not a function pointer");
      return __ptr;
    }


  template<typename _Ptr>
    constexpr typename std::pointer_traits<_Ptr>::element_type*
    __to_address(const _Ptr& __ptr)
    { return std::__to_address(__ptr.operator->()); }
# 214 "/usr/include/c++/11/bits/ptr_traits.h" 3

}
# 68 "/usr/include/c++/11/bits/stl_iterator.h" 2 3
# 88 "/usr/include/c++/11/bits/stl_iterator.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{

# 127 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator>
    class reverse_iterator
    : public iterator<typename iterator_traits<_Iterator>::iterator_category,
        typename iterator_traits<_Iterator>::value_type,
        typename iterator_traits<_Iterator>::difference_type,
        typename iterator_traits<_Iterator>::pointer,
                      typename iterator_traits<_Iterator>::reference>
    {
      template<typename _Iter>
 friend class reverse_iterator;
# 146 "/usr/include/c++/11/bits/stl_iterator.h" 3
    protected:
      _Iterator current;

      typedef iterator_traits<_Iterator> __traits_type;

    public:
      typedef _Iterator iterator_type;
      typedef typename __traits_type::pointer pointer;

      typedef typename __traits_type::difference_type difference_type;
      typedef typename __traits_type::reference reference;
# 177 "/usr/include/c++/11/bits/stl_iterator.h" 3
      constexpr
      reverse_iterator() : current() { }




      explicit constexpr
      reverse_iterator(iterator_type __x) : current(__x) { }




      constexpr
      reverse_iterator(const reverse_iterator& __x)
      : current(__x.current) { }


      reverse_iterator& operator=(const reverse_iterator&) = default;






      template<typename _Iter>



 constexpr
        reverse_iterator(const reverse_iterator<_Iter>& __x)
 : current(__x.current) { }


      template<typename _Iter>




 constexpr
 reverse_iterator&
 operator=(const reverse_iterator<_Iter>& __x)
 {
   current = __x.current;
   return *this;
 }





      constexpr iterator_type
      base() const
      { return current; }
# 241 "/usr/include/c++/11/bits/stl_iterator.h" 3
      constexpr reference
      operator*() const
      {
 _Iterator __tmp = current;
 return *--__tmp;
      }






      constexpr pointer
      operator->() const




      {


 _Iterator __tmp = current;
 --__tmp;
 return _S_to_pointer(__tmp);
      }






      constexpr reverse_iterator&
      operator++()
      {
 --current;
 return *this;
      }






      constexpr reverse_iterator
      operator++(int)
      {
 reverse_iterator __tmp = *this;
 --current;
 return __tmp;
      }






      constexpr reverse_iterator&
      operator--()
      {
 ++current;
 return *this;
      }






      constexpr reverse_iterator
      operator--(int)
      {
 reverse_iterator __tmp = *this;
 ++current;
 return __tmp;
      }






      constexpr reverse_iterator
      operator+(difference_type __n) const
      { return reverse_iterator(current - __n); }







      constexpr reverse_iterator&
      operator+=(difference_type __n)
      {
 current -= __n;
 return *this;
      }






      constexpr reverse_iterator
      operator-(difference_type __n) const
      { return reverse_iterator(current + __n); }







      constexpr reverse_iterator&
      operator-=(difference_type __n)
      {
 current += __n;
 return *this;
      }






      constexpr reference
      operator[](difference_type __n) const
      { return *(*this + __n); }
# 395 "/usr/include/c++/11/bits/stl_iterator.h" 3
    private:
      template<typename _Tp>
 static constexpr _Tp*
 _S_to_pointer(_Tp* __p)
        { return __p; }

      template<typename _Tp>
 static constexpr pointer
 _S_to_pointer(_Tp __t)
        { return __t.operator->(); }
    };
# 418 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator>
    inline constexpr bool
    operator==(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return __x.base() == __y.base(); }

  template<typename _Iterator>
    inline constexpr bool
    operator<(const reverse_iterator<_Iterator>& __x,
       const reverse_iterator<_Iterator>& __y)
    { return __y.base() < __x.base(); }

  template<typename _Iterator>
    inline constexpr bool
    operator!=(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return !(__x == __y); }

  template<typename _Iterator>
    inline constexpr bool
    operator>(const reverse_iterator<_Iterator>& __x,
       const reverse_iterator<_Iterator>& __y)
    { return __y < __x; }

  template<typename _Iterator>
    inline constexpr bool
    operator<=(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return !(__y < __x); }

  template<typename _Iterator>
    inline constexpr bool
    operator>=(const reverse_iterator<_Iterator>& __x,
        const reverse_iterator<_Iterator>& __y)
    { return !(__x < __y); }




  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator==(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() == __y.base(); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator<(const reverse_iterator<_IteratorL>& __x,
       const reverse_iterator<_IteratorR>& __y)
    { return __x.base() > __y.base(); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator!=(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() != __y.base(); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator>(const reverse_iterator<_IteratorL>& __x,
       const reverse_iterator<_IteratorR>& __y)
    { return __x.base() < __y.base(); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator<=(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() >= __y.base(); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator>=(const reverse_iterator<_IteratorL>& __x,
        const reverse_iterator<_IteratorR>& __y)
    { return __x.base() <= __y.base(); }
# 575 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _IteratorL, typename _IteratorR>
    inline constexpr auto
    operator-(const reverse_iterator<_IteratorL>& __x,
       const reverse_iterator<_IteratorR>& __y)
    -> decltype(__y.base() - __x.base())
    { return __y.base() - __x.base(); }


  template<typename _Iterator>
    inline constexpr reverse_iterator<_Iterator>
    operator+(typename reverse_iterator<_Iterator>::difference_type __n,
       const reverse_iterator<_Iterator>& __x)
    { return reverse_iterator<_Iterator>(__x.base() - __n); }



  template<typename _Iterator>
    inline constexpr reverse_iterator<_Iterator>
    __make_reverse_iterator(_Iterator __i)
    { return reverse_iterator<_Iterator>(__i); }







  template<typename _Iterator>
    inline constexpr reverse_iterator<_Iterator>
    make_reverse_iterator(_Iterator __i)
    { return reverse_iterator<_Iterator>(__i); }
# 616 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator>
   
    auto
    __niter_base(reverse_iterator<_Iterator> __it)
    -> decltype(__make_reverse_iterator(__niter_base(__it.base())))
    { return __make_reverse_iterator(__niter_base(__it.base())); }

  template<typename _Iterator>
    struct __is_move_iterator<reverse_iterator<_Iterator> >
      : __is_move_iterator<_Iterator>
    { };

  template<typename _Iterator>
   
    auto
    __miter_base(reverse_iterator<_Iterator> __it)
    -> decltype(__make_reverse_iterator(__miter_base(__it.base())))
    { return __make_reverse_iterator(__miter_base(__it.base())); }
# 647 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Container>
    class back_insert_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {
    protected:
      _Container* container;

    public:

      typedef _Container container_type;







      explicit
      back_insert_iterator(_Container& __x)
      : container(std::__addressof(__x)) { }
# 687 "/usr/include/c++/11/bits/stl_iterator.h" 3
     
      back_insert_iterator&
      operator=(const typename _Container::value_type& __value)
      {
 container->push_back(__value);
 return *this;
      }

     
      back_insert_iterator&
      operator=(typename _Container::value_type&& __value)
      {
 container->push_back(std::move(__value));
 return *this;
      }



     
      back_insert_iterator&
      operator*()
      { return *this; }


     
      back_insert_iterator&
      operator++()
      { return *this; }


     
      back_insert_iterator
      operator++(int)
      { return *this; }
    };
# 734 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Container>
   
    inline back_insert_iterator<_Container>
    back_inserter(_Container& __x)
    { return back_insert_iterator<_Container>(__x); }
# 750 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Container>
    class front_insert_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {
    protected:
      _Container* container;

    public:

      typedef _Container container_type;







      explicit
      front_insert_iterator(_Container& __x)
      : container(std::__addressof(__x)) { }
# 790 "/usr/include/c++/11/bits/stl_iterator.h" 3
     
      front_insert_iterator&
      operator=(const typename _Container::value_type& __value)
      {
 container->push_front(__value);
 return *this;
      }

     
      front_insert_iterator&
      operator=(typename _Container::value_type&& __value)
      {
 container->push_front(std::move(__value));
 return *this;
      }



     
      front_insert_iterator&
      operator*()
      { return *this; }


     
      front_insert_iterator&
      operator++()
      { return *this; }


     
      front_insert_iterator
      operator++(int)
      { return *this; }
    };
# 837 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Container>
   
    inline front_insert_iterator<_Container>
    front_inserter(_Container& __x)
    { return front_insert_iterator<_Container>(__x); }
# 857 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Container>
    class insert_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {







      typedef typename _Container::iterator _Iter;

    protected:
      _Container* container;
      _Iter iter;


    public:

      typedef _Container container_type;
# 889 "/usr/include/c++/11/bits/stl_iterator.h" 3
     
      insert_iterator(_Container& __x, _Iter __i)
      : container(std::__addressof(__x)), iter(__i) {}
# 925 "/usr/include/c++/11/bits/stl_iterator.h" 3
     
      insert_iterator&
      operator=(const typename _Container::value_type& __value)
      {
 iter = container->insert(iter, __value);
 ++iter;
 return *this;
      }

     
      insert_iterator&
      operator=(typename _Container::value_type&& __value)
      {
 iter = container->insert(iter, std::move(__value));
 ++iter;
 return *this;
      }



     
      insert_iterator&
      operator*()
      { return *this; }


     
      insert_iterator&
      operator++()
      { return *this; }


     
      insert_iterator&
      operator++(int)
      { return *this; }
    };
# 981 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Container>
    inline insert_iterator<_Container>
    inserter(_Container& __x, typename _Container::iterator __i)
    { return insert_iterator<_Container>(__x, __i); }





}

namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

# 1003 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator, typename _Container>
    class __normal_iterator
    {
    protected:
      _Iterator _M_current;

      typedef std::iterator_traits<_Iterator> __traits_type;

    public:
      typedef _Iterator iterator_type;
      typedef typename __traits_type::iterator_category iterator_category;
      typedef typename __traits_type::value_type value_type;
      typedef typename __traits_type::difference_type difference_type;
      typedef typename __traits_type::reference reference;
      typedef typename __traits_type::pointer pointer;





      constexpr __normal_iterator() noexcept
      : _M_current(_Iterator()) { }

      explicit
      __normal_iterator(const _Iterator& __i) noexcept
      : _M_current(__i) { }


      template<typename _Iter>
       
        __normal_iterator(const __normal_iterator<_Iter,
     typename __enable_if<
              (std::__are_same<_Iter, typename _Container::pointer>::__value),
        _Container>::__type>& __i) noexcept
        : _M_current(__i.base()) { }


     
      reference
      operator*() const noexcept
      { return *_M_current; }

     
      pointer
      operator->() const noexcept
      { return _M_current; }

     
      __normal_iterator&
      operator++() noexcept
      {
 ++_M_current;
 return *this;
      }

     
      __normal_iterator
      operator++(int) noexcept
      { return __normal_iterator(_M_current++); }


     
      __normal_iterator&
      operator--() noexcept
      {
 --_M_current;
 return *this;
      }

     
      __normal_iterator
      operator--(int) noexcept
      { return __normal_iterator(_M_current--); }


     
      reference
      operator[](difference_type __n) const noexcept
      { return _M_current[__n]; }

     
      __normal_iterator&
      operator+=(difference_type __n) noexcept
      { _M_current += __n; return *this; }

     
      __normal_iterator
      operator+(difference_type __n) const noexcept
      { return __normal_iterator(_M_current + __n); }

     
      __normal_iterator&
      operator-=(difference_type __n) noexcept
      { _M_current -= __n; return *this; }

     
      __normal_iterator
      operator-(difference_type __n) const noexcept
      { return __normal_iterator(_M_current - __n); }

     
      const _Iterator&
      base() const noexcept
      { return _M_current; }
    };
# 1152 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _IteratorL, typename _IteratorR, typename _Container>
   
    inline bool
    operator==(const __normal_iterator<_IteratorL, _Container>& __lhs,
        const __normal_iterator<_IteratorR, _Container>& __rhs)
    noexcept
    { return __lhs.base() == __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline bool
    operator==(const __normal_iterator<_Iterator, _Container>& __lhs,
        const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() == __rhs.base(); }

  template<typename _IteratorL, typename _IteratorR, typename _Container>
   
    inline bool
    operator!=(const __normal_iterator<_IteratorL, _Container>& __lhs,
        const __normal_iterator<_IteratorR, _Container>& __rhs)
    noexcept
    { return __lhs.base() != __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline bool
    operator!=(const __normal_iterator<_Iterator, _Container>& __lhs,
        const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() != __rhs.base(); }


  template<typename _IteratorL, typename _IteratorR, typename _Container>
    inline bool
    operator<(const __normal_iterator<_IteratorL, _Container>& __lhs,
       const __normal_iterator<_IteratorR, _Container>& __rhs)
    noexcept
    { return __lhs.base() < __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline bool
    operator<(const __normal_iterator<_Iterator, _Container>& __lhs,
       const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() < __rhs.base(); }

  template<typename _IteratorL, typename _IteratorR, typename _Container>
    inline bool
    operator>(const __normal_iterator<_IteratorL, _Container>& __lhs,
       const __normal_iterator<_IteratorR, _Container>& __rhs)
    noexcept
    { return __lhs.base() > __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline bool
    operator>(const __normal_iterator<_Iterator, _Container>& __lhs,
       const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() > __rhs.base(); }

  template<typename _IteratorL, typename _IteratorR, typename _Container>
    inline bool
    operator<=(const __normal_iterator<_IteratorL, _Container>& __lhs,
        const __normal_iterator<_IteratorR, _Container>& __rhs)
    noexcept
    { return __lhs.base() <= __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline bool
    operator<=(const __normal_iterator<_Iterator, _Container>& __lhs,
        const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() <= __rhs.base(); }

  template<typename _IteratorL, typename _IteratorR, typename _Container>
    inline bool
    operator>=(const __normal_iterator<_IteratorL, _Container>& __lhs,
        const __normal_iterator<_IteratorR, _Container>& __rhs)
    noexcept
    { return __lhs.base() >= __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline bool
    operator>=(const __normal_iterator<_Iterator, _Container>& __lhs,
        const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() >= __rhs.base(); }






  template<typename _IteratorL, typename _IteratorR, typename _Container>


   
    inline auto
    operator-(const __normal_iterator<_IteratorL, _Container>& __lhs,
       const __normal_iterator<_IteratorR, _Container>& __rhs) noexcept
    -> decltype(__lhs.base() - __rhs.base())





    { return __lhs.base() - __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline typename __normal_iterator<_Iterator, _Container>::difference_type
    operator-(const __normal_iterator<_Iterator, _Container>& __lhs,
       const __normal_iterator<_Iterator, _Container>& __rhs)
    noexcept
    { return __lhs.base() - __rhs.base(); }

  template<typename _Iterator, typename _Container>
   
    inline __normal_iterator<_Iterator, _Container>
    operator+(typename __normal_iterator<_Iterator, _Container>::difference_type
       __n, const __normal_iterator<_Iterator, _Container>& __i)
    noexcept
    { return __normal_iterator<_Iterator, _Container>(__i.base() + __n); }


}

namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _Iterator, typename _Container>
   
    _Iterator
    __niter_base(__gnu_cxx::__normal_iterator<_Iterator, _Container> __it)
    noexcept(std::is_nothrow_copy_constructible<_Iterator>::value)
    { return __it.base(); }
# 1342 "/usr/include/c++/11/bits/stl_iterator.h" 3
  namespace __detail
  {
# 1358 "/usr/include/c++/11/bits/stl_iterator.h" 3
  }
# 1369 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator>
    class move_iterator



    {
      _Iterator _M_current;

      using __traits_type = iterator_traits<_Iterator>;

      using __base_ref = typename __traits_type::reference;


      template<typename _Iter2>
 friend class move_iterator;
# 1408 "/usr/include/c++/11/bits/stl_iterator.h" 3
    public:
      using iterator_type = _Iterator;
# 1422 "/usr/include/c++/11/bits/stl_iterator.h" 3
      typedef typename __traits_type::iterator_category iterator_category;
      typedef typename __traits_type::value_type value_type;
      typedef typename __traits_type::difference_type difference_type;

      typedef _Iterator pointer;


      typedef typename conditional<is_reference<__base_ref>::value,
    typename remove_reference<__base_ref>::type&&,
    __base_ref>::type reference;


      constexpr
      move_iterator()
      : _M_current() { }

      explicit constexpr
      move_iterator(iterator_type __i)
      : _M_current(std::move(__i)) { }

      template<typename _Iter>



 constexpr
 move_iterator(const move_iterator<_Iter>& __i)
 : _M_current(__i._M_current) { }

      template<typename _Iter>




 constexpr
 move_iterator& operator=(const move_iterator<_Iter>& __i)
 {
   _M_current = __i._M_current;
   return *this;
 }


      constexpr iterator_type
      base() const
      { return _M_current; }
# 1476 "/usr/include/c++/11/bits/stl_iterator.h" 3
      constexpr reference
      operator*() const



      { return static_cast<reference>(*_M_current); }


      constexpr pointer
      operator->() const
      { return _M_current; }

      constexpr move_iterator&
      operator++()
      {
 ++_M_current;
 return *this;
      }

      constexpr move_iterator
      operator++(int)
      {
 move_iterator __tmp = *this;
 ++_M_current;
 return __tmp;
      }







      constexpr move_iterator&
      operator--()
      {
 --_M_current;
 return *this;
      }

      constexpr move_iterator
      operator--(int)
      {
 move_iterator __tmp = *this;
 --_M_current;
 return __tmp;
      }

      constexpr move_iterator
      operator+(difference_type __n) const
      { return move_iterator(_M_current + __n); }

      constexpr move_iterator&
      operator+=(difference_type __n)
      {
 _M_current += __n;
 return *this;
      }

      constexpr move_iterator
      operator-(difference_type __n) const
      { return move_iterator(_M_current - __n); }

      constexpr move_iterator&
      operator-=(difference_type __n)
      {
 _M_current -= __n;
 return *this;
      }

      constexpr reference
      operator[](difference_type __n) const



      { return std::move(_M_current[__n]); }
# 1581 "/usr/include/c++/11/bits/stl_iterator.h" 3
    };

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator==(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)



    { return __x.base() == __y.base(); }
# 1600 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator!=(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)
    { return !(__x == __y); }


  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator<(const move_iterator<_IteratorL>& __x,
       const move_iterator<_IteratorR>& __y)



    { return __x.base() < __y.base(); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator<=(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)



    { return !(__y < __x); }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator>(const move_iterator<_IteratorL>& __x,
       const move_iterator<_IteratorR>& __y)



    { return __y < __x; }

  template<typename _IteratorL, typename _IteratorR>
    inline constexpr bool
    operator>=(const move_iterator<_IteratorL>& __x,
        const move_iterator<_IteratorR>& __y)



    { return !(__x < __y); }




  template<typename _Iterator>
    inline constexpr bool
    operator==(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return __x.base() == __y.base(); }
# 1659 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator>
    inline constexpr bool
    operator!=(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return !(__x == __y); }

  template<typename _Iterator>
    inline constexpr bool
    operator<(const move_iterator<_Iterator>& __x,
       const move_iterator<_Iterator>& __y)
    { return __x.base() < __y.base(); }

  template<typename _Iterator>
    inline constexpr bool
    operator<=(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return !(__y < __x); }

  template<typename _Iterator>
    inline constexpr bool
    operator>(const move_iterator<_Iterator>& __x,
       const move_iterator<_Iterator>& __y)
    { return __y < __x; }

  template<typename _Iterator>
    inline constexpr bool
    operator>=(const move_iterator<_Iterator>& __x,
        const move_iterator<_Iterator>& __y)
    { return !(__x < __y); }



  template<typename _IteratorL, typename _IteratorR>
    inline constexpr auto
    operator-(const move_iterator<_IteratorL>& __x,
       const move_iterator<_IteratorR>& __y)
    -> decltype(__x.base() - __y.base())
    { return __x.base() - __y.base(); }

  template<typename _Iterator>
    inline constexpr move_iterator<_Iterator>
    operator+(typename move_iterator<_Iterator>::difference_type __n,
       const move_iterator<_Iterator>& __x)
    { return __x + __n; }

  template<typename _Iterator>
    inline constexpr move_iterator<_Iterator>
    make_move_iterator(_Iterator __i)
    { return move_iterator<_Iterator>(std::move(__i)); }

  template<typename _Iterator, typename _ReturnType
    = typename conditional<__move_if_noexcept_cond
      <typename iterator_traits<_Iterator>::value_type>::value,
                _Iterator, move_iterator<_Iterator>>::type>
    inline constexpr _ReturnType
    __make_move_if_noexcept_iterator(_Iterator __i)
    { return _ReturnType(__i); }



  template<typename _Tp, typename _ReturnType
    = typename conditional<__move_if_noexcept_cond<_Tp>::value,
      const _Tp*, move_iterator<_Tp*>>::type>
    inline constexpr _ReturnType
    __make_move_if_noexcept_iterator(_Tp* __i)
    { return _ReturnType(__i); }
# 2447 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _Iterator>
   
    auto
    __niter_base(move_iterator<_Iterator> __it)
    -> decltype(make_move_iterator(__niter_base(__it.base())))
    { return make_move_iterator(__niter_base(__it.base())); }

  template<typename _Iterator>
    struct __is_move_iterator<move_iterator<_Iterator> >
    {
      enum { __value = 1 };
      typedef __true_type __type;
    };

  template<typename _Iterator>
   
    auto
    __miter_base(move_iterator<_Iterator> __it)
    -> decltype(__miter_base(__it.base()))
    { return __miter_base(__it.base()); }
# 2479 "/usr/include/c++/11/bits/stl_iterator.h" 3
  template<typename _InputIterator>
    using __iter_key_t = remove_const_t<
    typename iterator_traits<_InputIterator>::value_type::first_type>;

  template<typename _InputIterator>
    using __iter_val_t =
    typename iterator_traits<_InputIterator>::value_type::second_type;

  template<typename _T1, typename _T2>
    struct pair;

  template<typename _InputIterator>
    using __iter_to_alloc_t =
    pair<add_const_t<__iter_key_t<_InputIterator>>,
  __iter_val_t<_InputIterator>>;



}
# 68 "/usr/include/c++/11/bits/stl_algobase.h" 2 3

# 1 "/usr/include/c++/11/debug/debug.h" 1 3
# 48 "/usr/include/c++/11/debug/debug.h" 3
namespace std
{
  namespace __debug { }
}




namespace __gnu_debug
{
  using namespace std::__debug;

  template<typename _Ite, typename _Seq, typename _Cat>
    struct _Safe_iterator;
}
# 70 "/usr/include/c++/11/bits/stl_algobase.h" 2 3

# 1 "/usr/include/c++/11/bits/predefined_ops.h" 1 3
# 35 "/usr/include/c++/11/bits/predefined_ops.h" 3
namespace __gnu_cxx
{
namespace __ops
{
  struct _Iter_less_iter
  {
    template<typename _Iterator1, typename _Iterator2>
      constexpr
      bool
      operator()(_Iterator1 __it1, _Iterator2 __it2) const
      { return *__it1 < *__it2; }
  };

  constexpr
  inline _Iter_less_iter
  __iter_less_iter()
  { return _Iter_less_iter(); }

  struct _Iter_less_val
  {

    constexpr _Iter_less_val() = default;




   
    explicit
    _Iter_less_val(_Iter_less_iter) { }

    template<typename _Iterator, typename _Value>
     
      bool
      operator()(_Iterator __it, _Value& __val) const
      { return *__it < __val; }
  };

 
  inline _Iter_less_val
  __iter_less_val()
  { return _Iter_less_val(); }

 
  inline _Iter_less_val
  __iter_comp_val(_Iter_less_iter)
  { return _Iter_less_val(); }

  struct _Val_less_iter
  {

    constexpr _Val_less_iter() = default;




   
    explicit
    _Val_less_iter(_Iter_less_iter) { }

    template<typename _Value, typename _Iterator>
     
      bool
      operator()(_Value& __val, _Iterator __it) const
      { return __val < *__it; }
  };

 
  inline _Val_less_iter
  __val_less_iter()
  { return _Val_less_iter(); }

 
  inline _Val_less_iter
  __val_comp_iter(_Iter_less_iter)
  { return _Val_less_iter(); }

  struct _Iter_equal_to_iter
  {
    template<typename _Iterator1, typename _Iterator2>
     
      bool
      operator()(_Iterator1 __it1, _Iterator2 __it2) const
      { return *__it1 == *__it2; }
  };

 
  inline _Iter_equal_to_iter
  __iter_equal_to_iter()
  { return _Iter_equal_to_iter(); }

  struct _Iter_equal_to_val
  {
    template<typename _Iterator, typename _Value>
     
      bool
      operator()(_Iterator __it, _Value& __val) const
      { return *__it == __val; }
  };

 
  inline _Iter_equal_to_val
  __iter_equal_to_val()
  { return _Iter_equal_to_val(); }

 
  inline _Iter_equal_to_val
  __iter_comp_val(_Iter_equal_to_iter)
  { return _Iter_equal_to_val(); }

  template<typename _Compare>
    struct _Iter_comp_iter
    {
      _Compare _M_comp;

      explicit constexpr
      _Iter_comp_iter(_Compare __comp)
 : _M_comp(std::move(__comp))
      { }

      template<typename _Iterator1, typename _Iterator2>
        constexpr
        bool
        operator()(_Iterator1 __it1, _Iterator2 __it2)
        { return bool(_M_comp(*__it1, *__it2)); }
    };

  template<typename _Compare>
    constexpr
    inline _Iter_comp_iter<_Compare>
    __iter_comp_iter(_Compare __comp)
    { return _Iter_comp_iter<_Compare>(std::move(__comp)); }

  template<typename _Compare>
    struct _Iter_comp_val
    {
      _Compare _M_comp;

     
      explicit
      _Iter_comp_val(_Compare __comp)
 : _M_comp(std::move(__comp))
      { }

     
      explicit
      _Iter_comp_val(const _Iter_comp_iter<_Compare>& __comp)
 : _M_comp(__comp._M_comp)
      { }


     
      explicit
      _Iter_comp_val(_Iter_comp_iter<_Compare>&& __comp)
 : _M_comp(std::move(__comp._M_comp))
      { }


      template<typename _Iterator, typename _Value>

 bool
 operator()(_Iterator __it, _Value& __val)
 { return bool(_M_comp(*__it, __val)); }
    };

  template<typename _Compare>
   
    inline _Iter_comp_val<_Compare>
    __iter_comp_val(_Compare __comp)
    { return _Iter_comp_val<_Compare>(std::move(__comp)); }

  template<typename _Compare>
   
    inline _Iter_comp_val<_Compare>
    __iter_comp_val(_Iter_comp_iter<_Compare> __comp)
    { return _Iter_comp_val<_Compare>(std::move(__comp)); }

  template<typename _Compare>
    struct _Val_comp_iter
    {
      _Compare _M_comp;

     
      explicit
      _Val_comp_iter(_Compare __comp)
 : _M_comp(std::move(__comp))
      { }

     
      explicit
      _Val_comp_iter(const _Iter_comp_iter<_Compare>& __comp)
 : _M_comp(__comp._M_comp)
      { }


     
      explicit
      _Val_comp_iter(_Iter_comp_iter<_Compare>&& __comp)
 : _M_comp(std::move(__comp._M_comp))
      { }


      template<typename _Value, typename _Iterator>

 bool
 operator()(_Value& __val, _Iterator __it)
 { return bool(_M_comp(__val, *__it)); }
    };

  template<typename _Compare>
   
    inline _Val_comp_iter<_Compare>
    __val_comp_iter(_Compare __comp)
    { return _Val_comp_iter<_Compare>(std::move(__comp)); }

  template<typename _Compare>
   
    inline _Val_comp_iter<_Compare>
    __val_comp_iter(_Iter_comp_iter<_Compare> __comp)
    { return _Val_comp_iter<_Compare>(std::move(__comp)); }

  template<typename _Value>
    struct _Iter_equals_val
    {
      _Value& _M_value;

     
      explicit
      _Iter_equals_val(_Value& __value)
 : _M_value(__value)
      { }

      template<typename _Iterator>

 bool
 operator()(_Iterator __it)
 { return *__it == _M_value; }
    };

  template<typename _Value>
   
    inline _Iter_equals_val<_Value>
    __iter_equals_val(_Value& __val)
    { return _Iter_equals_val<_Value>(__val); }

  template<typename _Iterator1>
    struct _Iter_equals_iter
    {
      _Iterator1 _M_it1;

     
      explicit
      _Iter_equals_iter(_Iterator1 __it1)
 : _M_it1(__it1)
      { }

      template<typename _Iterator2>

 bool
 operator()(_Iterator2 __it2)
 { return *__it2 == *_M_it1; }
    };

  template<typename _Iterator>
   
    inline _Iter_equals_iter<_Iterator>
    __iter_comp_iter(_Iter_equal_to_iter, _Iterator __it)
    { return _Iter_equals_iter<_Iterator>(__it); }

  template<typename _Predicate>
    struct _Iter_pred
    {
      _Predicate _M_pred;

     
      explicit
      _Iter_pred(_Predicate __pred)
 : _M_pred(std::move(__pred))
      { }

      template<typename _Iterator>

 bool
 operator()(_Iterator __it)
 { return bool(_M_pred(*__it)); }
    };

  template<typename _Predicate>
   
    inline _Iter_pred<_Predicate>
    __pred_iter(_Predicate __pred)
    { return _Iter_pred<_Predicate>(std::move(__pred)); }

  template<typename _Compare, typename _Value>
    struct _Iter_comp_to_val
    {
      _Compare _M_comp;
      _Value& _M_value;

     
      _Iter_comp_to_val(_Compare __comp, _Value& __value)
 : _M_comp(std::move(__comp)), _M_value(__value)
      { }

      template<typename _Iterator>

 bool
 operator()(_Iterator __it)
 { return bool(_M_comp(*__it, _M_value)); }
    };

  template<typename _Compare, typename _Value>
    _Iter_comp_to_val<_Compare, _Value>
   
    __iter_comp_val(_Compare __comp, _Value &__val)
    {
      return _Iter_comp_to_val<_Compare, _Value>(std::move(__comp), __val);
    }

  template<typename _Compare, typename _Iterator1>
    struct _Iter_comp_to_iter
    {
      _Compare _M_comp;
      _Iterator1 _M_it1;

     
      _Iter_comp_to_iter(_Compare __comp, _Iterator1 __it1)
 : _M_comp(std::move(__comp)), _M_it1(__it1)
      { }

      template<typename _Iterator2>

 bool
 operator()(_Iterator2 __it2)
 { return bool(_M_comp(*__it2, *_M_it1)); }
    };

  template<typename _Compare, typename _Iterator>
   
    inline _Iter_comp_to_iter<_Compare, _Iterator>
    __iter_comp_iter(_Iter_comp_iter<_Compare> __comp, _Iterator __it)
    {
      return _Iter_comp_to_iter<_Compare, _Iterator>(
   std::move(__comp._M_comp), __it);
    }

  template<typename _Predicate>
    struct _Iter_negate
    {
      _Predicate _M_pred;

     
      explicit
      _Iter_negate(_Predicate __pred)
 : _M_pred(std::move(__pred))
      { }

      template<typename _Iterator>

 bool
 operator()(_Iterator __it)
 { return !bool(_M_pred(*__it)); }
    };

  template<typename _Predicate>
   
    inline _Iter_negate<_Predicate>
    __negate(_Iter_pred<_Predicate> __pred)
    { return _Iter_negate<_Predicate>(std::move(__pred._M_pred)); }

}
}
# 72 "/usr/include/c++/11/bits/stl_algobase.h" 2 3







namespace std __attribute__ ((__visibility__ ("default")))
{






  template<typename _Tp, typename _Up>
    constexpr
    inline int
    __memcmp(const _Tp* __first1, const _Up* __first2, size_t __num)
    {

      static_assert(sizeof(_Tp) == sizeof(_Up), "can be compared with memcmp");
# 105 "/usr/include/c++/11/bits/stl_algobase.h" 3
 return __builtin_memcmp(__first1, __first2, sizeof(_Tp) * __num);
    }
# 149 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _ForwardIterator1, typename _ForwardIterator2>
   
    inline void
    iter_swap(_ForwardIterator1 __a, _ForwardIterator2 __b)
    {

     

     
# 182 "/usr/include/c++/11/bits/stl_algobase.h" 3
      swap(*__a, *__b);

    }
# 198 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _ForwardIterator1, typename _ForwardIterator2>
   
    _ForwardIterator2
    swap_ranges(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
  _ForwardIterator2 __first2)
    {

     

     

      ;

      for (; __first1 != __last1; ++__first1, (void)++__first2)
 std::iter_swap(__first1, __first2);
      return __first2;
    }
# 227 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _Tp>
    constexpr
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b)
    {

     

      if (__b < __a)
 return __b;
      return __a;
    }
# 251 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _Tp>
    constexpr
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b)
    {

     

      if (__a < __b)
 return __b;
      return __a;
    }
# 275 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _Tp, typename _Compare>
    constexpr
    inline const _Tp&
    min(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {

      if (__comp(__b, __a))
 return __b;
      return __a;
    }
# 297 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _Tp, typename _Compare>
    constexpr
    inline const _Tp&
    max(const _Tp& __a, const _Tp& __b, _Compare __comp)
    {

      if (__comp(__a, __b))
 return __b;
      return __a;
    }



  template<typename _Iterator>
   
    inline _Iterator
    __niter_base(_Iterator __it)
    noexcept(std::is_nothrow_copy_constructible<_Iterator>::value)
    { return __it; }

  template<typename _Ite, typename _Seq>
    _Ite
    __niter_base(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq,
   std::random_access_iterator_tag>&);




  template<typename _From, typename _To>
   
    inline _From
    __niter_wrap(_From __from, _To __res)
    { return __from + (__res - std::__niter_base(__from)); }


  template<typename _Iterator>
   
    inline _Iterator
    __niter_wrap(const _Iterator&, _Iterator __res)
    { return __res; }







  template<bool _IsMove, bool _IsSimple, typename _Category>
    struct __copy_move
    {
      template<typename _II, typename _OI>

 static _OI
 __copy_m(_II __first, _II __last, _OI __result)
 {
   for (; __first != __last; ++__result, (void)++__first)
     *__result = *__first;
   return __result;
 }
    };


  template<typename _Category>
    struct __copy_move<true, false, _Category>
    {
      template<typename _II, typename _OI>

 static _OI
 __copy_m(_II __first, _II __last, _OI __result)
 {
   for (; __first != __last; ++__result, (void)++__first)
     *__result = std::move(*__first);
   return __result;
 }
    };


  template<>
    struct __copy_move<false, false, random_access_iterator_tag>
    {
      template<typename _II, typename _OI>

 static _OI
 __copy_m(_II __first, _II __last, _OI __result)
 {
   typedef typename iterator_traits<_II>::difference_type _Distance;
   for(_Distance __n = __last - __first; __n > 0; --__n)
     {
       *__result = *__first;
       ++__first;
       ++__result;
     }
   return __result;
 }
    };


  template<>
    struct __copy_move<true, false, random_access_iterator_tag>
    {
      template<typename _II, typename _OI>

 static _OI
 __copy_m(_II __first, _II __last, _OI __result)
 {
   typedef typename iterator_traits<_II>::difference_type _Distance;
   for(_Distance __n = __last - __first; __n > 0; --__n)
     {
       *__result = std::move(*__first);
       ++__first;
       ++__result;
     }
   return __result;
 }
    };


  template<bool _IsMove>
    struct __copy_move<_IsMove, true, random_access_iterator_tag>
    {
      template<typename _Tp>

 static _Tp*
 __copy_m(const _Tp* __first, const _Tp* __last, _Tp* __result)
 {

   using __assignable = conditional<_IsMove,
        is_move_assignable<_Tp>,
        is_copy_assignable<_Tp>>;

   static_assert( __assignable::type::value, "type is not assignable" );

   const ptrdiff_t _Num = __last - __first;
   if (_Num)
     __builtin_memmove(__result, __first, sizeof(_Tp) * _Num);
   return __result + _Num;
 }
    };



  template<typename _Tp, typename _Ref, typename _Ptr>
    struct _Deque_iterator;

  struct _Bit_iterator;





  template<typename _CharT>
    struct char_traits;

  template<typename _CharT, typename _Traits>
    class istreambuf_iterator;

  template<typename _CharT, typename _Traits>
    class ostreambuf_iterator;

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
      ostreambuf_iterator<_CharT, char_traits<_CharT> > >::__type
    __copy_move_a2(_CharT*, _CharT*,
     ostreambuf_iterator<_CharT, char_traits<_CharT> >);

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
      ostreambuf_iterator<_CharT, char_traits<_CharT> > >::__type
    __copy_move_a2(const _CharT*, const _CharT*,
     ostreambuf_iterator<_CharT, char_traits<_CharT> >);

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        _CharT*>::__type
    __copy_move_a2(istreambuf_iterator<_CharT, char_traits<_CharT> >,
     istreambuf_iterator<_CharT, char_traits<_CharT> >, _CharT*);

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<
      __is_char<_CharT>::__value,
      std::_Deque_iterator<_CharT, _CharT&, _CharT*> >::__type
    __copy_move_a2(
 istreambuf_iterator<_CharT, char_traits<_CharT> >,
 istreambuf_iterator<_CharT, char_traits<_CharT> >,
 std::_Deque_iterator<_CharT, _CharT&, _CharT*>);

  template<bool _IsMove, typename _II, typename _OI>
   
    inline _OI
    __copy_move_a2(_II __first, _II __last, _OI __result)
    {
      typedef typename iterator_traits<_II>::iterator_category _Category;





      return std::__copy_move<_IsMove, __memcpyable<_OI, _II>::__value,
         _Category>::__copy_m(__first, __last, __result);
    }

  template<bool _IsMove,
    typename _Tp, typename _Ref, typename _Ptr, typename _OI>
    _OI
    __copy_move_a1(std::_Deque_iterator<_Tp, _Ref, _Ptr>,
     std::_Deque_iterator<_Tp, _Ref, _Ptr>,
     _OI);

  template<bool _IsMove,
    typename _ITp, typename _IRef, typename _IPtr, typename _OTp>
    std::_Deque_iterator<_OTp, _OTp&, _OTp*>
    __copy_move_a1(std::_Deque_iterator<_ITp, _IRef, _IPtr>,
     std::_Deque_iterator<_ITp, _IRef, _IPtr>,
     std::_Deque_iterator<_OTp, _OTp&, _OTp*>);

  template<bool _IsMove, typename _II, typename _Tp>
    typename __gnu_cxx::__enable_if<
      __is_random_access_iter<_II>::__value,
      std::_Deque_iterator<_Tp, _Tp&, _Tp*> >::__type
    __copy_move_a1(_II, _II, std::_Deque_iterator<_Tp, _Tp&, _Tp*>);

  template<bool _IsMove, typename _II, typename _OI>
   
    inline _OI
    __copy_move_a1(_II __first, _II __last, _OI __result)
    { return std::__copy_move_a2<_IsMove>(__first, __last, __result); }

  template<bool _IsMove, typename _II, typename _OI>
   
    inline _OI
    __copy_move_a(_II __first, _II __last, _OI __result)
    {
      return std::__niter_wrap(__result,
  std::__copy_move_a1<_IsMove>(std::__niter_base(__first),
          std::__niter_base(__last),
          std::__niter_base(__result)));
    }

  template<bool _IsMove,
    typename _Ite, typename _Seq, typename _Cat, typename _OI>
    _OI
    __copy_move_a(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
    const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
    _OI);

  template<bool _IsMove,
    typename _II, typename _Ite, typename _Seq, typename _Cat>
    __gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>
    __copy_move_a(_II, _II,
    const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&);

  template<bool _IsMove,
    typename _IIte, typename _ISeq, typename _ICat,
    typename _OIte, typename _OSeq, typename _OCat>
    ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>
    __copy_move_a(const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
    const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
    const ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>&);

  template<typename _InputIterator, typename _Size, typename _OutputIterator>
   
    _OutputIterator
    __copy_n_a(_InputIterator __first, _Size __n, _OutputIterator __result,
        bool)
    {
      if (__n > 0)
 {
   while (true)
     {
       *__result = *__first;
       ++__result;
       if (--__n > 0)
  ++__first;
       else
  break;
     }
 }
      return __result;
    }

  template<typename _CharT, typename _Size>
    typename __gnu_cxx::__enable_if<
      __is_char<_CharT>::__value, _CharT*>::__type
    __copy_n_a(istreambuf_iterator<_CharT, char_traits<_CharT> >,
        _Size, _CharT*, bool);

  template<typename _CharT, typename _Size>
    typename __gnu_cxx::__enable_if<
      __is_char<_CharT>::__value,
      std::_Deque_iterator<_CharT, _CharT&, _CharT*> >::__type
    __copy_n_a(istreambuf_iterator<_CharT, char_traits<_CharT> >, _Size,
        std::_Deque_iterator<_CharT, _CharT&, _CharT*>,
        bool);
# 608 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _II, typename _OI>
   
    inline _OI
    copy(_II __first, _II __last, _OI __result)
    {

     
     

      ;

      return std::__copy_move_a<__is_move_iterator<_II>::__value>
      (std::__miter_base(__first), std::__miter_base(__last), __result);
    }
# 641 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _II, typename _OI>
   
    inline _OI
    move(_II __first, _II __last, _OI __result)
    {

     
     

      ;

      return std::__copy_move_a<true>(std::__miter_base(__first),
          std::__miter_base(__last), __result);
    }






  template<bool _IsMove, bool _IsSimple, typename _Category>
    struct __copy_move_backward
    {
      template<typename _BI1, typename _BI2>

 static _BI2
 __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
 {
   while (__first != __last)
     *--__result = *--__last;
   return __result;
 }
    };


  template<typename _Category>
    struct __copy_move_backward<true, false, _Category>
    {
      template<typename _BI1, typename _BI2>

 static _BI2
 __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
 {
   while (__first != __last)
     *--__result = std::move(*--__last);
   return __result;
 }
    };


  template<>
    struct __copy_move_backward<false, false, random_access_iterator_tag>
    {
      template<typename _BI1, typename _BI2>

 static _BI2
 __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
 {
   typename iterator_traits<_BI1>::difference_type
     __n = __last - __first;
   for (; __n > 0; --__n)
     *--__result = *--__last;
   return __result;
 }
    };


  template<>
    struct __copy_move_backward<true, false, random_access_iterator_tag>
    {
      template<typename _BI1, typename _BI2>

 static _BI2
 __copy_move_b(_BI1 __first, _BI1 __last, _BI2 __result)
 {
   typename iterator_traits<_BI1>::difference_type
     __n = __last - __first;
   for (; __n > 0; --__n)
     *--__result = std::move(*--__last);
   return __result;
 }
    };


  template<bool _IsMove>
    struct __copy_move_backward<_IsMove, true, random_access_iterator_tag>
    {
      template<typename _Tp>

 static _Tp*
 __copy_move_b(const _Tp* __first, const _Tp* __last, _Tp* __result)
 {

   using __assignable = conditional<_IsMove,
        is_move_assignable<_Tp>,
        is_copy_assignable<_Tp>>;

   static_assert( __assignable::type::value, "type is not assignable" );

   const ptrdiff_t _Num = __last - __first;
   if (_Num)
     __builtin_memmove(__result - _Num, __first, sizeof(_Tp) * _Num);
   return __result - _Num;
 }
    };

  template<bool _IsMove, typename _BI1, typename _BI2>
   
    inline _BI2
    __copy_move_backward_a2(_BI1 __first, _BI1 __last, _BI2 __result)
    {
      typedef typename iterator_traits<_BI1>::iterator_category _Category;





      return std::__copy_move_backward<_IsMove,
           __memcpyable<_BI2, _BI1>::__value,
           _Category>::__copy_move_b(__first,
         __last,
         __result);
    }

  template<bool _IsMove, typename _BI1, typename _BI2>
   
    inline _BI2
    __copy_move_backward_a1(_BI1 __first, _BI1 __last, _BI2 __result)
    { return std::__copy_move_backward_a2<_IsMove>(__first, __last, __result); }

  template<bool _IsMove,
    typename _Tp, typename _Ref, typename _Ptr, typename _OI>
    _OI
    __copy_move_backward_a1(std::_Deque_iterator<_Tp, _Ref, _Ptr>,
       std::_Deque_iterator<_Tp, _Ref, _Ptr>,
       _OI);

  template<bool _IsMove,
    typename _ITp, typename _IRef, typename _IPtr, typename _OTp>
    std::_Deque_iterator<_OTp, _OTp&, _OTp*>
    __copy_move_backward_a1(
   std::_Deque_iterator<_ITp, _IRef, _IPtr>,
   std::_Deque_iterator<_ITp, _IRef, _IPtr>,
   std::_Deque_iterator<_OTp, _OTp&, _OTp*>);

  template<bool _IsMove, typename _II, typename _Tp>
    typename __gnu_cxx::__enable_if<
      __is_random_access_iter<_II>::__value,
      std::_Deque_iterator<_Tp, _Tp&, _Tp*> >::__type
    __copy_move_backward_a1(_II, _II,
       std::_Deque_iterator<_Tp, _Tp&, _Tp*>);

  template<bool _IsMove, typename _II, typename _OI>
   
    inline _OI
    __copy_move_backward_a(_II __first, _II __last, _OI __result)
    {
      return std::__niter_wrap(__result,
  std::__copy_move_backward_a1<_IsMove>
    (std::__niter_base(__first), std::__niter_base(__last),
     std::__niter_base(__result)));
    }

  template<bool _IsMove,
    typename _Ite, typename _Seq, typename _Cat, typename _OI>
    _OI
    __copy_move_backward_a(
  const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
  const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
  _OI);

  template<bool _IsMove,
    typename _II, typename _Ite, typename _Seq, typename _Cat>
    __gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>
    __copy_move_backward_a(_II, _II,
  const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&);

  template<bool _IsMove,
    typename _IIte, typename _ISeq, typename _ICat,
    typename _OIte, typename _OSeq, typename _OCat>
    ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>
    __copy_move_backward_a(
  const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
  const ::__gnu_debug::_Safe_iterator<_IIte, _ISeq, _ICat>&,
  const ::__gnu_debug::_Safe_iterator<_OIte, _OSeq, _OCat>&);
# 845 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _BI1, typename _BI2>
   
    inline _BI2
    copy_backward(_BI1 __first, _BI1 __last, _BI2 __result)
    {

     
     
     


      ;

      return std::__copy_move_backward_a<__is_move_iterator<_BI1>::__value>
      (std::__miter_base(__first), std::__miter_base(__last), __result);
    }
# 881 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _BI1, typename _BI2>
   
    inline _BI2
    move_backward(_BI1 __first, _BI1 __last, _BI2 __result)
    {

     
     
     


      ;

      return std::__copy_move_backward_a<true>(std::__miter_base(__first),
            std::__miter_base(__last),
            __result);
    }






  template<typename _ForwardIterator, typename _Tp>
   
    inline typename
    __gnu_cxx::__enable_if<!__is_scalar<_Tp>::__value, void>::__type
    __fill_a1(_ForwardIterator __first, _ForwardIterator __last,
       const _Tp& __value)
    {
      for (; __first != __last; ++__first)
 *__first = __value;
    }

  template<typename _ForwardIterator, typename _Tp>
   
    inline typename
    __gnu_cxx::__enable_if<__is_scalar<_Tp>::__value, void>::__type
    __fill_a1(_ForwardIterator __first, _ForwardIterator __last,
       const _Tp& __value)
    {
      const _Tp __tmp = __value;
      for (; __first != __last; ++__first)
 *__first = __tmp;
    }


  template<typename _Tp>
   
    inline typename
    __gnu_cxx::__enable_if<__is_byte<_Tp>::__value, void>::__type
    __fill_a1(_Tp* __first, _Tp* __last, const _Tp& __c)
    {
      const _Tp __tmp = __c;
# 943 "/usr/include/c++/11/bits/stl_algobase.h" 3
      if (const size_t __len = __last - __first)
 __builtin_memset(__first, static_cast<unsigned char>(__tmp), __len);
    }

  template<typename _Ite, typename _Cont, typename _Tp>
   
    inline void
    __fill_a1(::__gnu_cxx::__normal_iterator<_Ite, _Cont> __first,
       ::__gnu_cxx::__normal_iterator<_Ite, _Cont> __last,
       const _Tp& __value)
    { std::__fill_a1(__first.base(), __last.base(), __value); }

  template<typename _Tp, typename _VTp>
    void
    __fill_a1(const std::_Deque_iterator<_Tp, _Tp&, _Tp*>&,
       const std::_Deque_iterator<_Tp, _Tp&, _Tp*>&,
       const _VTp&);

  void
  __fill_a1(std::_Bit_iterator, std::_Bit_iterator,
     const bool&);

  template<typename _FIte, typename _Tp>
   
    inline void
    __fill_a(_FIte __first, _FIte __last, const _Tp& __value)
    { std::__fill_a1(__first, __last, __value); }

  template<typename _Ite, typename _Seq, typename _Cat, typename _Tp>
    void
    __fill_a(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
      const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>&,
      const _Tp&);
# 989 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _ForwardIterator, typename _Tp>
   
    inline void
    fill(_ForwardIterator __first, _ForwardIterator __last, const _Tp& __value)
    {

     

      ;

      std::__fill_a(__first, __last, __value);
    }


  inline constexpr int
  __size_to_integer(int __n) { return __n; }
  inline constexpr unsigned
  __size_to_integer(unsigned __n) { return __n; }
  inline constexpr long
  __size_to_integer(long __n) { return __n; }
  inline constexpr unsigned long
  __size_to_integer(unsigned long __n) { return __n; }
  inline constexpr long long
  __size_to_integer(long long __n) { return __n; }
  inline constexpr unsigned long long
  __size_to_integer(unsigned long long __n) { return __n; }
# 1041 "/usr/include/c++/11/bits/stl_algobase.h" 3
  inline constexpr long long
  __size_to_integer(float __n) { return (long long)__n; }
  inline constexpr long long
  __size_to_integer(double __n) { return (long long)__n; }
  inline constexpr long long
  __size_to_integer(long double __n) { return (long long)__n; }





  template<typename _OutputIterator, typename _Size, typename _Tp>
   
    inline typename
    __gnu_cxx::__enable_if<!__is_scalar<_Tp>::__value, _OutputIterator>::__type
    __fill_n_a1(_OutputIterator __first, _Size __n, const _Tp& __value)
    {
      for (; __n > 0; --__n, (void) ++__first)
 *__first = __value;
      return __first;
    }

  template<typename _OutputIterator, typename _Size, typename _Tp>
   
    inline typename
    __gnu_cxx::__enable_if<__is_scalar<_Tp>::__value, _OutputIterator>::__type
    __fill_n_a1(_OutputIterator __first, _Size __n, const _Tp& __value)
    {
      const _Tp __tmp = __value;
      for (; __n > 0; --__n, (void) ++__first)
 *__first = __tmp;
      return __first;
    }

  template<typename _Ite, typename _Seq, typename _Cat, typename _Size,
    typename _Tp>
    ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>
    __fill_n_a(const ::__gnu_debug::_Safe_iterator<_Ite, _Seq, _Cat>& __first,
        _Size __n, const _Tp& __value,
        std::input_iterator_tag);

  template<typename _OutputIterator, typename _Size, typename _Tp>
   
    inline _OutputIterator
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value,
        std::output_iterator_tag)
    {

      static_assert(is_integral<_Size>{}, "fill_n must pass integral size");

      return __fill_n_a1(__first, __n, __value);
    }

  template<typename _OutputIterator, typename _Size, typename _Tp>
   
    inline _OutputIterator
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value,
        std::input_iterator_tag)
    {

      static_assert(is_integral<_Size>{}, "fill_n must pass integral size");

      return __fill_n_a1(__first, __n, __value);
    }

  template<typename _OutputIterator, typename _Size, typename _Tp>
   
    inline _OutputIterator
    __fill_n_a(_OutputIterator __first, _Size __n, const _Tp& __value,
        std::random_access_iterator_tag)
    {

      static_assert(is_integral<_Size>{}, "fill_n must pass integral size");

      if (__n <= 0)
 return __first;

      ;

      std::__fill_a(__first, __first + __n, __value);
      return __first + __n;
    }
# 1141 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _OI, typename _Size, typename _Tp>
   
    inline _OI
    fill_n(_OI __first, _Size __n, const _Tp& __value)
    {

     

      return std::__fill_n_a(__first, std::__size_to_integer(__n), __value,
          std::__iterator_category(__first));
    }

  template<bool _BoolType>
    struct __equal
    {
      template<typename _II1, typename _II2>

 static bool
 equal(_II1 __first1, _II1 __last1, _II2 __first2)
 {
   for (; __first1 != __last1; ++__first1, (void) ++__first2)
     if (!(*__first1 == *__first2))
       return false;
   return true;
 }
    };

  template<>
    struct __equal<true>
    {
      template<typename _Tp>

 static bool
 equal(const _Tp* __first1, const _Tp* __last1, const _Tp* __first2)
 {
   if (const size_t __len = (__last1 - __first1))
     return !std::__memcmp(__first1, __first2, __len);
   return true;
 }
    };

  template<typename _Tp, typename _Ref, typename _Ptr, typename _II>
    typename __gnu_cxx::__enable_if<
      __is_random_access_iter<_II>::__value, bool>::__type
    __equal_aux1(std::_Deque_iterator<_Tp, _Ref, _Ptr>,
   std::_Deque_iterator<_Tp, _Ref, _Ptr>,
   _II);

  template<typename _Tp1, typename _Ref1, typename _Ptr1,
    typename _Tp2, typename _Ref2, typename _Ptr2>
    bool
    __equal_aux1(std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
   std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
   std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>);

  template<typename _II, typename _Tp, typename _Ref, typename _Ptr>
    typename __gnu_cxx::__enable_if<
      __is_random_access_iter<_II>::__value, bool>::__type
    __equal_aux1(_II, _II,
  std::_Deque_iterator<_Tp, _Ref, _Ptr>);

  template<typename _II1, typename _II2>
   
    inline bool
    __equal_aux1(_II1 __first1, _II1 __last1, _II2 __first2)
    {
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      const bool __simple = ((__is_integer<_ValueType1>::__value
         || __is_pointer<_ValueType1>::__value)
        && __memcmpable<_II1, _II2>::__value);
      return std::__equal<__simple>::equal(__first1, __last1, __first2);
    }

  template<typename _II1, typename _II2>
   
    inline bool
    __equal_aux(_II1 __first1, _II1 __last1, _II2 __first2)
    {
      return std::__equal_aux1(std::__niter_base(__first1),
          std::__niter_base(__last1),
          std::__niter_base(__first2));
    }

  template<typename _II1, typename _Seq1, typename _Cat1, typename _II2>
    bool
    __equal_aux(const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  _II2);

  template<typename _II1, typename _II2, typename _Seq2, typename _Cat2>
    bool
    __equal_aux(_II1, _II1,
  const ::__gnu_debug::_Safe_iterator<_II2, _Seq2, _Cat2>&);

  template<typename _II1, typename _Seq1, typename _Cat1,
    typename _II2, typename _Seq2, typename _Cat2>
    bool
    __equal_aux(const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_II1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_II2, _Seq2, _Cat2>&);

  template<typename, typename>
    struct __lc_rai
    {
      template<typename _II1, typename _II2>

 static _II1
 __newlast1(_II1, _II1 __last1, _II2, _II2)
 { return __last1; }

      template<typename _II>

 static bool
 __cnd2(_II __first, _II __last)
 { return __first != __last; }
    };

  template<>
    struct __lc_rai<random_access_iterator_tag, random_access_iterator_tag>
    {
      template<typename _RAI1, typename _RAI2>

 static _RAI1
 __newlast1(_RAI1 __first1, _RAI1 __last1,
     _RAI2 __first2, _RAI2 __last2)
 {
   const typename iterator_traits<_RAI1>::difference_type
     __diff1 = __last1 - __first1;
   const typename iterator_traits<_RAI2>::difference_type
     __diff2 = __last2 - __first2;
   return __diff2 < __diff1 ? __first1 + __diff2 : __last1;
 }

      template<typename _RAI>
 static bool
 __cnd2(_RAI, _RAI)
 { return true; }
    };

  template<typename _II1, typename _II2, typename _Compare>
   
    bool
    __lexicographical_compare_impl(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2,
       _Compare __comp)
    {
      typedef typename iterator_traits<_II1>::iterator_category _Category1;
      typedef typename iterator_traits<_II2>::iterator_category _Category2;
      typedef std::__lc_rai<_Category1, _Category2> __rai_type;

      __last1 = __rai_type::__newlast1(__first1, __last1, __first2, __last2);
      for (; __first1 != __last1 && __rai_type::__cnd2(__first2, __last2);
    ++__first1, (void)++__first2)
 {
   if (__comp(__first1, __first2))
     return true;
   if (__comp(__first2, __first1))
     return false;
 }
      return __first1 == __last1 && __first2 != __last2;
    }

  template<bool _BoolType>
    struct __lexicographical_compare
    {
      template<typename _II1, typename _II2>

 static bool
 __lc(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
 {
   using __gnu_cxx::__ops::__iter_less_iter;
   return std::__lexicographical_compare_impl(__first1, __last1,
           __first2, __last2,
           __iter_less_iter());
 }

      template<typename _II1, typename _II2>

 static int
 __3way(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
 {
   while (__first1 != __last1)
     {
       if (__first2 == __last2)
  return +1;
       if (*__first1 < *__first2)
  return -1;
       if (*__first2 < *__first1)
  return +1;
       ++__first1;
       ++__first2;
     }
   return int(__first2 == __last2) - 1;
 }
    };

  template<>
    struct __lexicographical_compare<true>
    {
      template<typename _Tp, typename _Up>

 static bool
 __lc(const _Tp* __first1, const _Tp* __last1,
      const _Up* __first2, const _Up* __last2)
 { return __3way(__first1, __last1, __first2, __last2) < 0; }

      template<typename _Tp, typename _Up>

 static ptrdiff_t
 __3way(const _Tp* __first1, const _Tp* __last1,
        const _Up* __first2, const _Up* __last2)
 {
   const size_t __len1 = __last1 - __first1;
   const size_t __len2 = __last2 - __first2;
   if (const size_t __len = std::min(__len1, __len2))
     if (int __result = std::__memcmp(__first1, __first2, __len))
       return __result;
   return ptrdiff_t(__len1 - __len2);
 }
    };

  template<typename _II1, typename _II2>
   
    inline bool
    __lexicographical_compare_aux1(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2)
    {
      typedef typename iterator_traits<_II1>::value_type _ValueType1;
      typedef typename iterator_traits<_II2>::value_type _ValueType2;
      const bool __simple =
 (__is_memcmp_ordered_with<_ValueType1, _ValueType2>::__value
  && __is_pointer<_II1>::__value
  && __is_pointer<_II2>::__value







  );

      return std::__lexicographical_compare<__simple>::__lc(__first1, __last1,
           __first2, __last2);
    }

  template<typename _Tp1, typename _Ref1, typename _Ptr1,
    typename _Tp2>
    bool
    __lexicographical_compare_aux1(
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 _Tp2*, _Tp2*);

  template<typename _Tp1,
    typename _Tp2, typename _Ref2, typename _Ptr2>
    bool
    __lexicographical_compare_aux1(_Tp1*, _Tp1*,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>);

  template<typename _Tp1, typename _Ref1, typename _Ptr1,
    typename _Tp2, typename _Ref2, typename _Ptr2>
    bool
    __lexicographical_compare_aux1(
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 std::_Deque_iterator<_Tp1, _Ref1, _Ptr1>,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>,
 std::_Deque_iterator<_Tp2, _Ref2, _Ptr2>);

  template<typename _II1, typename _II2>
   
    inline bool
    __lexicographical_compare_aux(_II1 __first1, _II1 __last1,
      _II2 __first2, _II2 __last2)
    {
      return std::__lexicographical_compare_aux1(std::__niter_base(__first1),
       std::__niter_base(__last1),
       std::__niter_base(__first2),
       std::__niter_base(__last2));
    }

  template<typename _Iter1, typename _Seq1, typename _Cat1,
    typename _II2>
    bool
    __lexicographical_compare_aux(
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  _II2, _II2);

  template<typename _II1,
    typename _Iter2, typename _Seq2, typename _Cat2>
    bool
    __lexicographical_compare_aux(
  _II1, _II1,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&);

  template<typename _Iter1, typename _Seq1, typename _Cat1,
    typename _Iter2, typename _Seq2, typename _Cat2>
    bool
    __lexicographical_compare_aux(
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_Iter1, _Seq1, _Cat1>&,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&,
  const ::__gnu_debug::_Safe_iterator<_Iter2, _Seq2, _Cat2>&);

  template<typename _ForwardIterator, typename _Tp, typename _Compare>
   
    _ForwardIterator
    __lower_bound(_ForwardIterator __first, _ForwardIterator __last,
    const _Tp& __val, _Compare __comp)
    {
      typedef typename iterator_traits<_ForwardIterator>::difference_type
 _DistanceType;

      _DistanceType __len = std::distance(__first, __last);

      while (__len > 0)
 {
   _DistanceType __half = __len >> 1;
   _ForwardIterator __middle = __first;
   std::advance(__middle, __half);
   if (__comp(__middle, __val))
     {
       __first = __middle;
       ++__first;
       __len = __len - __half - 1;
     }
   else
     __len = __half;
 }
      return __first;
    }
# 1487 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _ForwardIterator, typename _Tp>
   
    inline _ForwardIterator
    lower_bound(_ForwardIterator __first, _ForwardIterator __last,
  const _Tp& __val)
    {

     
     

      ;

      return std::__lower_bound(__first, __last, __val,
    __gnu_cxx::__ops::__iter_less_val());
    }



  inline constexpr int
  __lg(int __n)
  { return (int)sizeof(int) * 8 - 1 - __builtin_clz(__n); }

  inline constexpr unsigned
  __lg(unsigned __n)
  { return (int)sizeof(int) * 8 - 1 - __builtin_clz(__n); }

  inline constexpr long
  __lg(long __n)
  { return (int)sizeof(long) * 8 - 1 - __builtin_clzl(__n); }

  inline constexpr unsigned long
  __lg(unsigned long __n)
  { return (int)sizeof(long) * 8 - 1 - __builtin_clzl(__n); }

  inline constexpr long long
  __lg(long long __n)
  { return (int)sizeof(long long) * 8 - 1 - __builtin_clzll(__n); }

  inline constexpr unsigned long long
  __lg(unsigned long long __n)
  { return (int)sizeof(long long) * 8 - 1 - __builtin_clzll(__n); }


# 1543 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _II1, typename _II2>
   
    inline bool
    equal(_II1 __first1, _II1 __last1, _II2 __first2)
    {

     
     
     


      ;

      return std::__equal_aux(__first1, __last1, __first2);
    }
# 1574 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _IIter1, typename _IIter2, typename _BinaryPredicate>
   
    inline bool
    equal(_IIter1 __first1, _IIter1 __last1,
   _IIter2 __first2, _BinaryPredicate __binary_pred)
    {

     
     
      ;

      for (; __first1 != __last1; ++__first1, (void)++__first2)
 if (!bool(__binary_pred(*__first1, *__first2)))
   return false;
      return true;
    }



  template<typename _II1, typename _II2>
   
    inline bool
    __equal4(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
    {
      using _RATag = random_access_iterator_tag;
      using _Cat1 = typename iterator_traits<_II1>::iterator_category;
      using _Cat2 = typename iterator_traits<_II2>::iterator_category;
      using _RAIters = __and_<is_same<_Cat1, _RATag>, is_same<_Cat2, _RATag>>;
      if (_RAIters())
 {
   auto __d1 = std::distance(__first1, __last1);
   auto __d2 = std::distance(__first2, __last2);
   if (__d1 != __d2)
     return false;
   return std::equal(__first1, __last1, __first2);
 }

      for (; __first1 != __last1 && __first2 != __last2;
   ++__first1, (void)++__first2)
 if (!(*__first1 == *__first2))
   return false;
      return __first1 == __last1 && __first2 == __last2;
    }


  template<typename _II1, typename _II2, typename _BinaryPredicate>
   
    inline bool
    __equal4(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2,
      _BinaryPredicate __binary_pred)
    {
      using _RATag = random_access_iterator_tag;
      using _Cat1 = typename iterator_traits<_II1>::iterator_category;
      using _Cat2 = typename iterator_traits<_II2>::iterator_category;
      using _RAIters = __and_<is_same<_Cat1, _RATag>, is_same<_Cat2, _RATag>>;
      if (_RAIters())
 {
   auto __d1 = std::distance(__first1, __last1);
   auto __d2 = std::distance(__first2, __last2);
   if (__d1 != __d2)
     return false;
   return std::equal(__first1, __last1, __first2,
           __binary_pred);
 }

      for (; __first1 != __last1 && __first2 != __last2;
   ++__first1, (void)++__first2)
 if (!bool(__binary_pred(*__first1, *__first2)))
   return false;
      return __first1 == __last1 && __first2 == __last2;
    }
# 1664 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _II1, typename _II2>
   
    inline bool
    equal(_II1 __first1, _II1 __last1, _II2 __first2, _II2 __last2)
    {

     
     
     


      ;
      ;

      return std::__equal4(__first1, __last1, __first2, __last2);
    }
# 1697 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _IIter1, typename _IIter2, typename _BinaryPredicate>
   
    inline bool
    equal(_IIter1 __first1, _IIter1 __last1,
   _IIter2 __first2, _IIter2 __last2, _BinaryPredicate __binary_pred)
    {

     
     
      ;
      ;

      return std::__equal4(__first1, __last1, __first2, __last2,
          __binary_pred);
    }
# 1729 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _II1, typename _II2>
   
    inline bool
    lexicographical_compare(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2)
    {





     
     
     
     
      ;
      ;

      return std::__lexicographical_compare_aux(__first1, __last1,
      __first2, __last2);
    }
# 1764 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _II1, typename _II2, typename _Compare>
   
    inline bool
    lexicographical_compare(_II1 __first1, _II1 __last1,
       _II2 __first2, _II2 __last2, _Compare __comp)
    {

     
     
      ;
      ;

      return std::__lexicographical_compare_impl
 (__first1, __last1, __first2, __last2,
  __gnu_cxx::__ops::__iter_comp_iter(__comp));
    }
# 1877 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
   
    pair<_InputIterator1, _InputIterator2>
    __mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
        _InputIterator2 __first2, _BinaryPredicate __binary_pred)
    {
      while (__first1 != __last1 && __binary_pred(__first1, __first2))
 {
   ++__first1;
   ++__first2;
 }
      return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }
# 1905 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _InputIterator1, typename _InputIterator2>
   
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2)
    {

     
     
     


      ;

      return std::__mismatch(__first1, __last1, __first2,
        __gnu_cxx::__ops::__iter_equal_to_iter());
    }
# 1939 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
   
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2, _BinaryPredicate __binary_pred)
    {

     
     
      ;

      return std::__mismatch(__first1, __last1, __first2,
 __gnu_cxx::__ops::__iter_comp_iter(__binary_pred));
    }



  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
   
    pair<_InputIterator1, _InputIterator2>
    __mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
        _InputIterator2 __first2, _InputIterator2 __last2,
        _BinaryPredicate __binary_pred)
    {
      while (__first1 != __last1 && __first2 != __last2
      && __binary_pred(__first1, __first2))
 {
   ++__first1;
   ++__first2;
 }
      return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
    }
# 1988 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _InputIterator1, typename _InputIterator2>
   
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2, _InputIterator2 __last2)
    {

     
     
     


      ;
      ;

      return std::__mismatch(__first1, __last1, __first2, __last2,
        __gnu_cxx::__ops::__iter_equal_to_iter());
    }
# 2024 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _InputIterator1, typename _InputIterator2,
    typename _BinaryPredicate>
   
    inline pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1,
      _InputIterator2 __first2, _InputIterator2 __last2,
      _BinaryPredicate __binary_pred)
    {

     
     
      ;
      ;

      return std::__mismatch(__first1, __last1, __first2, __last2,
        __gnu_cxx::__ops::__iter_comp_iter(__binary_pred));
    }





  template<typename _InputIterator, typename _Predicate>
   
    inline _InputIterator
    __find_if(_InputIterator __first, _InputIterator __last,
       _Predicate __pred, input_iterator_tag)
    {
      while (__first != __last && !__pred(__first))
 ++__first;
      return __first;
    }


  template<typename _RandomAccessIterator, typename _Predicate>
   
    _RandomAccessIterator
    __find_if(_RandomAccessIterator __first, _RandomAccessIterator __last,
       _Predicate __pred, random_access_iterator_tag)
    {
      typename iterator_traits<_RandomAccessIterator>::difference_type
 __trip_count = (__last - __first) >> 2;

      for (; __trip_count > 0; --__trip_count)
 {
   if (__pred(__first))
     return __first;
   ++__first;

   if (__pred(__first))
     return __first;
   ++__first;

   if (__pred(__first))
     return __first;
   ++__first;

   if (__pred(__first))
     return __first;
   ++__first;
 }

      switch (__last - __first)
 {
 case 3:
   if (__pred(__first))
     return __first;
   ++__first;

 case 2:
   if (__pred(__first))
     return __first;
   ++__first;

 case 1:
   if (__pred(__first))
     return __first;
   ++__first;

 case 0:
 default:
   return __last;
 }
    }

  template<typename _Iterator, typename _Predicate>
   
    inline _Iterator
    __find_if(_Iterator __first, _Iterator __last, _Predicate __pred)
    {
      return __find_if(__first, __last, __pred,
         std::__iterator_category(__first));
    }

  template<typename _InputIterator, typename _Predicate>
   
    typename iterator_traits<_InputIterator>::difference_type
    __count_if(_InputIterator __first, _InputIterator __last, _Predicate __pred)
    {
      typename iterator_traits<_InputIterator>::difference_type __n = 0;
      for (; __first != __last; ++__first)
 if (__pred(__first))
   ++__n;
      return __n;
    }


  template<typename _ForwardIterator1, typename _ForwardIterator2,
    typename _BinaryPredicate>
   
    bool
    __is_permutation(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
       _ForwardIterator2 __first2, _BinaryPredicate __pred)
    {


      for (; __first1 != __last1; ++__first1, (void)++__first2)
 if (!__pred(__first1, __first2))
   break;

      if (__first1 == __last1)
 return true;



      _ForwardIterator2 __last2 = __first2;
      std::advance(__last2, std::distance(__first1, __last1));
      for (_ForwardIterator1 __scan = __first1; __scan != __last1; ++__scan)
 {
   if (__scan != std::__find_if(__first1, __scan,
     __gnu_cxx::__ops::__iter_comp_iter(__pred, __scan)))
     continue;

   auto __matches
     = std::__count_if(__first2, __last2,
   __gnu_cxx::__ops::__iter_comp_iter(__pred, __scan));
   if (0 == __matches ||
       std::__count_if(__scan, __last1,
   __gnu_cxx::__ops::__iter_comp_iter(__pred, __scan))
       != __matches)
     return false;
 }
      return true;
    }
# 2181 "/usr/include/c++/11/bits/stl_algobase.h" 3
  template<typename _ForwardIterator1, typename _ForwardIterator2>
   
    inline bool
    is_permutation(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
     _ForwardIterator2 __first2)
    {

     
     
     


      ;

      return std::__is_permutation(__first1, __last1, __first2,
       __gnu_cxx::__ops::__iter_equal_to_iter());
    }



}
# 40 "/usr/include/c++/11/bits/char_traits.h" 2 3

# 1 "/usr/include/c++/11/cwchar" 1 3
# 39 "/usr/include/c++/11/cwchar" 3
       
# 40 "/usr/include/c++/11/cwchar" 3
# 42 "/usr/include/c++/11/bits/char_traits.h" 2 3
# 50 "/usr/include/c++/11/bits/char_traits.h" 3
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

# 64 "/usr/include/c++/11/bits/char_traits.h" 3
  template<typename _CharT>
    struct _Char_types
    {
      typedef unsigned long int_type;
      typedef std::streampos pos_type;
      typedef std::streamoff off_type;
      typedef std::mbstate_t state_type;
    };
# 89 "/usr/include/c++/11/bits/char_traits.h" 3
  template<typename _CharT>
    struct char_traits
    {
      typedef _CharT char_type;
      typedef typename _Char_types<_CharT>::int_type int_type;
      typedef typename _Char_types<_CharT>::pos_type pos_type;
      typedef typename _Char_types<_CharT>::off_type off_type;
      typedef typename _Char_types<_CharT>::state_type state_type;




      static constexpr void
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, std::size_t __n);

      static constexpr std::size_t
      length(const char_type* __s);

      static constexpr const char_type*
      find(const char_type* __s, std::size_t __n, const char_type& __a);

      static char_type*
      move(char_type* __s1, const char_type* __s2, std::size_t __n);

      static char_type*
      copy(char_type* __s1, const char_type* __s2, std::size_t __n);

      static char_type*
      assign(char_type* __s, std::size_t __n, char_type __a);

      static constexpr char_type
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      static constexpr int_type
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(__c); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static constexpr int_type
      eof()
      { return static_cast<int_type>(-1); }

      static constexpr int_type
      not_eof(const int_type& __c)
      { return !eq_int_type(__c, eof()) ? __c : to_int_type(char_type()); }
    };

  template<typename _CharT>
    constexpr int
    char_traits<_CharT>::
    compare(const char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      for (std::size_t __i = 0; __i < __n; ++__i)
 if (lt(__s1[__i], __s2[__i]))
   return -1;
 else if (lt(__s2[__i], __s1[__i]))
   return 1;
      return 0;
    }

  template<typename _CharT>
    constexpr std::size_t
    char_traits<_CharT>::
    length(const char_type* __p)
    {
      std::size_t __i = 0;
      while (!eq(__p[__i], char_type()))
        ++__i;
      return __i;
    }

  template<typename _CharT>
    constexpr const typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    find(const char_type* __s, std::size_t __n, const char_type& __a)
    {
      for (std::size_t __i = 0; __i < __n; ++__i)
        if (eq(__s[__i], __a))
          return __s + __i;
      return 0;
    }

  template<typename _CharT>
   
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    move(char_type* __s1, const char_type* __s2, std::size_t __n)
    {
      if (__n == 0)
 return __s1;
# 233 "/usr/include/c++/11/bits/char_traits.h" 3
      __builtin_memmove(__s1, __s2, __n * sizeof(char_type));
      return __s1;
    }

  template<typename _CharT>
   
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    copy(char_type* __s1, const char_type* __s2, std::size_t __n)
    {

      std::copy(__s2, __s2 + __n, __s1);
      return __s1;
    }

  template<typename _CharT>
   
    typename char_traits<_CharT>::char_type*
    char_traits<_CharT>::
    assign(char_type* __s, std::size_t __n, char_type __a)
    {

      std::fill_n(__s, __n, __a);
      return __s;
    }


}

namespace std __attribute__ ((__visibility__ ("default")))
{

# 283 "/usr/include/c++/11/bits/char_traits.h" 3
  template<typename _CharT>
    inline __attribute__((__always_inline__)) constexpr bool
    __constant_string_p(const _CharT* __s)
    {

      (void) __s;

      return __builtin_is_constant_evaluated();





    }
# 306 "/usr/include/c++/11/bits/char_traits.h" 3
  template<typename _CharT>
    inline __attribute__((__always_inline__)) constexpr bool
    __constant_char_array_p(const _CharT* __a, size_t __n)
    {

      (void) __a;
      (void) __n;

      return __builtin_is_constant_evaluated();






    }
# 337 "/usr/include/c++/11/bits/char_traits.h" 3
  template<class _CharT>
    struct char_traits : public __gnu_cxx::char_traits<_CharT>
    { };



  template<>
    struct char_traits<char>
    {
      typedef char char_type;
      typedef int int_type;
      typedef streampos pos_type;
      typedef streamoff off_type;
      typedef mbstate_t state_type;




      static constexpr void
      assign(char_type& __c1, const char_type& __c2) noexcept
      { __c1 = __c2; }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2) noexcept
      {

 return (static_cast<unsigned char>(__c1)
  < static_cast<unsigned char>(__c2));
      }

      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return 0;

 if (__builtin_constant_p(__n)
     && __constant_char_array_p(__s1, __n)
     && __constant_char_array_p(__s2, __n))
   {
     for (size_t __i = 0; __i < __n; ++__i)
       if (lt(__s1[__i], __s2[__i]))
  return -1;
       else if (lt(__s2[__i], __s1[__i]))
  return 1;
     return 0;
   }

 return __builtin_memcmp(__s1, __s2, __n);
      }

      static constexpr size_t
      length(const char_type* __s)
      {

 if (__constant_string_p(__s))
   return __gnu_cxx::char_traits<char_type>::length(__s);

 return __builtin_strlen(__s);
      }

      static constexpr const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
 if (__n == 0)
   return 0;

 if (__builtin_constant_p(__n)
     && __builtin_constant_p(__a)
     && __constant_char_array_p(__s, __n))
   return __gnu_cxx::char_traits<char_type>::find(__s, __n, __a);

 return static_cast<const char_type*>(__builtin_memchr(__s, __a, __n));
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return static_cast<char_type*>(__builtin_memmove(__s1, __s2, __n));
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return static_cast<char_type*>(__builtin_memcpy(__s1, __s2, __n));
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
 if (__n == 0)
   return __s;




 return static_cast<char_type*>(__builtin_memset(__s, __a, __n));
      }

      static constexpr char_type
      to_char_type(const int_type& __c) noexcept
      { return static_cast<char_type>(__c); }



      static constexpr int_type
      to_int_type(const char_type& __c) noexcept
      { return static_cast<int_type>(static_cast<unsigned char>(__c)); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr int_type
      eof() noexcept
      { return static_cast<int_type>(-1); }

      static constexpr int_type
      not_eof(const int_type& __c) noexcept
      { return (__c == eof()) ? 0 : __c; }
  };




  template<>
    struct char_traits<wchar_t>
    {
      typedef wchar_t char_type;
      typedef wint_t int_type;
      typedef streamoff off_type;
      typedef wstreampos pos_type;
      typedef mbstate_t state_type;




      static constexpr void
      assign(char_type& __c1, const char_type& __c2) noexcept
      { __c1 = __c2; }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 < __c2; }

      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return 0;

 if (__builtin_constant_p(__n)
     && __constant_char_array_p(__s1, __n)
     && __constant_char_array_p(__s2, __n))
   return __gnu_cxx::char_traits<char_type>::compare(__s1, __s2, __n);

 return wmemcmp(__s1, __s2, __n);
      }

      static constexpr size_t
      length(const char_type* __s)
      {

 if (__constant_string_p(__s))
   return __gnu_cxx::char_traits<char_type>::length(__s);

 return wcslen(__s);
      }

      static constexpr const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
 if (__n == 0)
   return 0;

 if (__builtin_constant_p(__n)
     && __builtin_constant_p(__a)
     && __constant_char_array_p(__s, __n))
   return __gnu_cxx::char_traits<char_type>::find(__s, __n, __a);

 return wmemchr(__s, __a, __n);
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return wmemmove(__s1, __s2, __n);
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return wmemcpy(__s1, __s2, __n);
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
 if (__n == 0)
   return __s;




 return wmemset(__s, __a, __n);
      }

      static constexpr char_type
      to_char_type(const int_type& __c) noexcept
      { return char_type(__c); }

      static constexpr int_type
      to_int_type(const char_type& __c) noexcept
      { return int_type(__c); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr int_type
      eof() noexcept
      { return static_cast<int_type>((0xffffffffu)); }

      static constexpr int_type
      not_eof(const int_type& __c) noexcept
      { return eq_int_type(__c, eof()) ? 0 : __c; }
  };
# 722 "/usr/include/c++/11/bits/char_traits.h" 3

}



# 1 "/usr/include/c++/11/cstdint" 1 3
# 32 "/usr/include/c++/11/cstdint" 3
       
# 33 "/usr/include/c++/11/cstdint" 3
# 41 "/usr/include/c++/11/cstdint" 3
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stdint.h" 1 3 4
# 9 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stdint.h" 3 4
# 1 "/usr/include/stdint.h" 1 3 4
# 26 "/usr/include/stdint.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/stdint.h" 2 3 4
# 1 "/usr/include/bits/types.h" 1 3 4
# 27 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 28 "/usr/include/bits/types.h" 2 3 4
# 1 "/usr/include/bits/timesize.h" 1 3 4
# 29 "/usr/include/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;






typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;



typedef long int __quad_t;
typedef unsigned long int __u_quad_t;







typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;
# 141 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/typesizes.h" 1 3 4
# 142 "/usr/include/bits/types.h" 2 3 4
# 1 "/usr/include/bits/time64.h" 1 3 4
# 143 "/usr/include/bits/types.h" 2 3 4


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;




typedef int __sig_atomic_t;
# 28 "/usr/include/stdint.h" 2 3 4

# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 30 "/usr/include/stdint.h" 2 3 4




# 1 "/usr/include/bits/stdint-intn.h" 1 3 4
# 24 "/usr/include/bits/stdint-intn.h" 3 4
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
# 35 "/usr/include/stdint.h" 2 3 4


# 1 "/usr/include/bits/stdint-uintn.h" 1 3 4
# 24 "/usr/include/bits/stdint-uintn.h" 3 4
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
# 38 "/usr/include/stdint.h" 2 3 4





typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;


typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;





typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
# 71 "/usr/include/stdint.h" 3 4
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
# 87 "/usr/include/stdint.h" 3 4
typedef long int intptr_t;


typedef unsigned long int uintptr_t;
# 101 "/usr/include/stdint.h" 3 4
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;
# 10 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stdint.h" 2 3 4
# 42 "/usr/include/c++/11/cstdint" 2 3


namespace std
{

  using ::int8_t;
  using ::int16_t;
  using ::int32_t;
  using ::int64_t;

  using ::int_fast8_t;
  using ::int_fast16_t;
  using ::int_fast32_t;
  using ::int_fast64_t;

  using ::int_least8_t;
  using ::int_least16_t;
  using ::int_least32_t;
  using ::int_least64_t;

  using ::intmax_t;
  using ::intptr_t;

  using ::uint8_t;
  using ::uint16_t;
  using ::uint32_t;
  using ::uint64_t;

  using ::uint_fast8_t;
  using ::uint_fast16_t;
  using ::uint_fast32_t;
  using ::uint_fast64_t;

  using ::uint_least8_t;
  using ::uint_least16_t;
  using ::uint_least32_t;
  using ::uint_least64_t;

  using ::uintmax_t;
  using ::uintptr_t;





}
# 728 "/usr/include/c++/11/bits/char_traits.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{


  template<>
    struct char_traits<char16_t>
    {
      typedef char16_t char_type;

      typedef uint_least16_t int_type;





      typedef streamoff off_type;
      typedef u16streampos pos_type;
      typedef mbstate_t state_type;




      static constexpr void
      assign(char_type& __c1, const char_type& __c2) noexcept
      { __c1 = __c2; }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 < __c2; }

      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
 for (size_t __i = 0; __i < __n; ++__i)
   if (lt(__s1[__i], __s2[__i]))
     return -1;
   else if (lt(__s2[__i], __s1[__i]))
     return 1;
 return 0;
      }

      static constexpr size_t
      length(const char_type* __s)
      {
 size_t __i = 0;
 while (!eq(__s[__i], char_type()))
   ++__i;
 return __i;
      }

      static constexpr const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
 for (size_t __i = 0; __i < __n; ++__i)
   if (eq(__s[__i], __a))
     return __s + __i;
 return 0;
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return (static_cast<char_type*>
  (__builtin_memmove(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return (static_cast<char_type*>
  (__builtin_memcpy(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
 for (size_t __i = 0; __i < __n; ++__i)
   assign(__s[__i], __a);
 return __s;
      }

      static constexpr char_type
      to_char_type(const int_type& __c) noexcept
      { return char_type(__c); }

      static constexpr int_type
      to_int_type(const char_type& __c) noexcept
      { return __c == eof() ? int_type(0xfffd) : int_type(__c); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr int_type
      eof() noexcept
      { return static_cast<int_type>(-1); }

      static constexpr int_type
      not_eof(const int_type& __c) noexcept
      { return eq_int_type(__c, eof()) ? 0 : __c; }
    };

  template<>
    struct char_traits<char32_t>
    {
      typedef char32_t char_type;

      typedef uint_least32_t int_type;





      typedef streamoff off_type;
      typedef u32streampos pos_type;
      typedef mbstate_t state_type;




      static constexpr void
      assign(char_type& __c1, const char_type& __c2) noexcept
      { __c1 = __c2; }

      static constexpr bool
      eq(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr bool
      lt(const char_type& __c1, const char_type& __c2) noexcept
      { return __c1 < __c2; }

      static constexpr int
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      {
 for (size_t __i = 0; __i < __n; ++__i)
   if (lt(__s1[__i], __s2[__i]))
     return -1;
   else if (lt(__s2[__i], __s1[__i]))
     return 1;
 return 0;
      }

      static constexpr size_t
      length(const char_type* __s)
      {
 size_t __i = 0;
 while (!eq(__s[__i], char_type()))
   ++__i;
 return __i;
      }

      static constexpr const char_type*
      find(const char_type* __s, size_t __n, const char_type& __a)
      {
 for (size_t __i = 0; __i < __n; ++__i)
   if (eq(__s[__i], __a))
     return __s + __i;
 return 0;
      }

      static char_type*
      move(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return (static_cast<char_type*>
  (__builtin_memmove(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {
 if (__n == 0)
   return __s1;




 return (static_cast<char_type*>
  (__builtin_memcpy(__s1, __s2, __n * sizeof(char_type))));
      }

      static char_type*
      assign(char_type* __s, size_t __n, char_type __a)
      {
 for (size_t __i = 0; __i < __n; ++__i)
   assign(__s[__i], __a);
 return __s;
      }

      static constexpr char_type
      to_char_type(const int_type& __c) noexcept
      { return char_type(__c); }

      static constexpr int_type
      to_int_type(const char_type& __c) noexcept
      { return int_type(__c); }

      static constexpr bool
      eq_int_type(const int_type& __c1, const int_type& __c2) noexcept
      { return __c1 == __c2; }

      static constexpr int_type
      eof() noexcept
      { return static_cast<int_type>(-1); }

      static constexpr int_type
      not_eof(const int_type& __c) noexcept
      { return eq_int_type(__c, eof()) ? 0 : __c; }
    };
# 980 "/usr/include/c++/11/bits/char_traits.h" 3

}
# 41 "/usr/include/c++/11/ios" 2 3
# 1 "/usr/include/c++/11/bits/localefwd.h" 1 3
# 37 "/usr/include/c++/11/bits/localefwd.h" 3
       
# 38 "/usr/include/c++/11/bits/localefwd.h" 3


# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++locale.h" 1 3
# 39 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++locale.h" 3
       
# 40 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++locale.h" 3

# 1 "/usr/include/c++/11/clocale" 1 3
# 39 "/usr/include/c++/11/clocale" 3
       
# 40 "/usr/include/c++/11/clocale" 3


# 1 "/usr/include/locale.h" 1 3 4
# 28 "/usr/include/locale.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 29 "/usr/include/locale.h" 2 3 4
# 1 "/usr/include/bits/locale.h" 1 3 4
# 30 "/usr/include/locale.h" 2 3 4

extern "C" {
# 51 "/usr/include/locale.h" 3 4
struct lconv
{


  char *decimal_point;
  char *thousands_sep;





  char *grouping;





  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;

  char p_cs_precedes;

  char p_sep_by_space;

  char n_cs_precedes;

  char n_sep_by_space;






  char p_sign_posn;
  char n_sign_posn;


  char int_p_cs_precedes;

  char int_p_sep_by_space;

  char int_n_cs_precedes;

  char int_n_sep_by_space;






  char int_p_sign_posn;
  char int_n_sign_posn;
# 118 "/usr/include/locale.h" 3 4
};



extern char *setlocale (int __category, const char *__locale) noexcept (true);


extern struct lconv *localeconv (void) noexcept (true);
# 141 "/usr/include/locale.h" 3 4
extern locale_t newlocale (int __category_mask, const char *__locale,
      locale_t __base) noexcept (true);
# 176 "/usr/include/locale.h" 3 4
extern locale_t duplocale (locale_t __dataset) noexcept (true);



extern void freelocale (locale_t __dataset) noexcept (true);






extern locale_t uselocale (locale_t __dataset) noexcept (true);







}
# 43 "/usr/include/c++/11/clocale" 2 3
# 51 "/usr/include/c++/11/clocale" 3
namespace std
{
  using ::lconv;
  using ::setlocale;
  using ::localeconv;
}
# 42 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++locale.h" 2 3






namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{


  extern "C" __typeof(uselocale) __uselocale;


}


namespace std __attribute__ ((__visibility__ ("default")))
{


  typedef __locale_t __c_locale;





  inline int
  __convert_from_v(const __c_locale& __cloc __attribute__ ((__unused__)),
     char* __out,
     const int __size __attribute__ ((__unused__)),
     const char* __fmt, ...)
  {

    __c_locale __old = __gnu_cxx::__uselocale(__cloc);
# 88 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++locale.h" 3
    __builtin_va_list __args;
    __builtin_va_start(__args, __fmt);


    const int __ret = __builtin_vsnprintf(__out, __size, __fmt, __args);




    __builtin_va_end(__args);


    __gnu_cxx::__uselocale(__old);







    return __ret;
  }


}
# 41 "/usr/include/c++/11/bits/localefwd.h" 2 3

# 1 "/usr/include/c++/11/cctype" 1 3
# 39 "/usr/include/c++/11/cctype" 3
       
# 40 "/usr/include/c++/11/cctype" 3


# 1 "/usr/include/ctype.h" 1 3 4
# 28 "/usr/include/ctype.h" 3 4
extern "C" {
# 39 "/usr/include/ctype.h" 3 4
# 1 "/usr/include/bits/endian.h" 1 3 4
# 35 "/usr/include/bits/endian.h" 3 4
# 1 "/usr/include/bits/endianness.h" 1 3 4
# 36 "/usr/include/bits/endian.h" 2 3 4
# 40 "/usr/include/ctype.h" 2 3 4






enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
# 79 "/usr/include/ctype.h" 3 4
extern const unsigned short int **__ctype_b_loc (void)
     noexcept (true) __attribute__ ((__const__));
extern const __int32_t **__ctype_tolower_loc (void)
     noexcept (true) __attribute__ ((__const__));
extern const __int32_t **__ctype_toupper_loc (void)
     noexcept (true) __attribute__ ((__const__));
# 108 "/usr/include/ctype.h" 3 4
extern int isalnum (int) noexcept (true);
extern int isalpha (int) noexcept (true);
extern int iscntrl (int) noexcept (true);
extern int isdigit (int) noexcept (true);
extern int islower (int) noexcept (true);
extern int isgraph (int) noexcept (true);
extern int isprint (int) noexcept (true);
extern int ispunct (int) noexcept (true);
extern int isspace (int) noexcept (true);
extern int isupper (int) noexcept (true);
extern int isxdigit (int) noexcept (true);



extern int tolower (int __c) noexcept (true);


extern int toupper (int __c) noexcept (true);




extern int isblank (int) noexcept (true);




extern int isctype (int __c, int __mask) noexcept (true);






extern int isascii (int __c) noexcept (true);



extern int toascii (int __c) noexcept (true);



extern int _toupper (int) noexcept (true);
extern int _tolower (int) noexcept (true);
# 251 "/usr/include/ctype.h" 3 4
extern int isalnum_l (int, locale_t) noexcept (true);
extern int isalpha_l (int, locale_t) noexcept (true);
extern int iscntrl_l (int, locale_t) noexcept (true);
extern int isdigit_l (int, locale_t) noexcept (true);
extern int islower_l (int, locale_t) noexcept (true);
extern int isgraph_l (int, locale_t) noexcept (true);
extern int isprint_l (int, locale_t) noexcept (true);
extern int ispunct_l (int, locale_t) noexcept (true);
extern int isspace_l (int, locale_t) noexcept (true);
extern int isupper_l (int, locale_t) noexcept (true);
extern int isxdigit_l (int, locale_t) noexcept (true);

extern int isblank_l (int, locale_t) noexcept (true);



extern int __tolower_l (int __c, locale_t __l) noexcept (true);
extern int tolower_l (int __c, locale_t __l) noexcept (true);


extern int __toupper_l (int __c, locale_t __l) noexcept (true);
extern int toupper_l (int __c, locale_t __l) noexcept (true);
# 327 "/usr/include/ctype.h" 3 4
}
# 43 "/usr/include/c++/11/cctype" 2 3
# 62 "/usr/include/c++/11/cctype" 3
namespace std
{
  using ::isalnum;
  using ::isalpha;
  using ::iscntrl;
  using ::isdigit;
  using ::isgraph;
  using ::islower;
  using ::isprint;
  using ::ispunct;
  using ::isspace;
  using ::isupper;
  using ::isxdigit;
  using ::tolower;
  using ::toupper;
}







namespace std
{
  using ::isblank;
}
# 43 "/usr/include/c++/11/bits/localefwd.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{

# 55 "/usr/include/c++/11/bits/localefwd.h" 3
  class locale;

  template<typename _Facet>
    bool
    has_facet(const locale&) throw();

  template<typename _Facet>
    const _Facet&
    use_facet(const locale&);


  template<typename _CharT>
    bool
    isspace(_CharT, const locale&);

  template<typename _CharT>
    bool
    isprint(_CharT, const locale&);

  template<typename _CharT>
    bool
    iscntrl(_CharT, const locale&);

  template<typename _CharT>
    bool
    isupper(_CharT, const locale&);

  template<typename _CharT>
    bool
    islower(_CharT, const locale&);

  template<typename _CharT>
    bool
    isalpha(_CharT, const locale&);

  template<typename _CharT>
    bool
    isdigit(_CharT, const locale&);

  template<typename _CharT>
    bool
    ispunct(_CharT, const locale&);

  template<typename _CharT>
    bool
    isxdigit(_CharT, const locale&);

  template<typename _CharT>
    bool
    isalnum(_CharT, const locale&);

  template<typename _CharT>
    bool
    isgraph(_CharT, const locale&);


  template<typename _CharT>
    bool
    isblank(_CharT, const locale&);


  template<typename _CharT>
    _CharT
    toupper(_CharT, const locale&);

  template<typename _CharT>
    _CharT
    tolower(_CharT, const locale&);


  struct ctype_base;
  template<typename _CharT>
    class ctype;
  template<> class ctype<char>;

  template<> class ctype<wchar_t>;

  template<typename _CharT>
    class ctype_byname;


  class codecvt_base;
  template<typename _InternT, typename _ExternT, typename _StateT>
    class codecvt;
  template<> class codecvt<char, char, mbstate_t>;

  template<> class codecvt<wchar_t, char, mbstate_t>;


  template<> class codecvt<char16_t, char, mbstate_t>;
  template<> class codecvt<char32_t, char, mbstate_t>;





  template<typename _InternT, typename _ExternT, typename _StateT>
    class codecvt_byname;



  template<typename _CharT, typename _InIter = istreambuf_iterator<_CharT> >
    class num_get;
  template<typename _CharT, typename _OutIter = ostreambuf_iterator<_CharT> >
    class num_put;

namespace __cxx11 {
  template<typename _CharT> class numpunct;
  template<typename _CharT> class numpunct_byname;
}

namespace __cxx11 {

  template<typename _CharT>
    class collate;
  template<typename _CharT>
    class collate_byname;
}


  class time_base;
namespace __cxx11 {
  template<typename _CharT, typename _InIter = istreambuf_iterator<_CharT> >
    class time_get;
  template<typename _CharT, typename _InIter = istreambuf_iterator<_CharT> >
    class time_get_byname;
}
  template<typename _CharT, typename _OutIter = ostreambuf_iterator<_CharT> >
    class time_put;
  template<typename _CharT, typename _OutIter = ostreambuf_iterator<_CharT> >
    class time_put_byname;


  class money_base;
namespace __cxx11 {
  template<typename _CharT, typename _InIter = istreambuf_iterator<_CharT> >
    class money_get;
  template<typename _CharT, typename _OutIter = ostreambuf_iterator<_CharT> >
    class money_put;
}
namespace __cxx11 {
  template<typename _CharT, bool _Intl = false>
    class moneypunct;
  template<typename _CharT, bool _Intl = false>
    class moneypunct_byname;
}


  struct messages_base;
namespace __cxx11 {
  template<typename _CharT>
    class messages;
  template<typename _CharT>
    class messages_byname;
}


}
# 42 "/usr/include/c++/11/ios" 2 3
# 1 "/usr/include/c++/11/bits/ios_base.h" 1 3
# 37 "/usr/include/c++/11/bits/ios_base.h" 3
       
# 38 "/usr/include/c++/11/bits/ios_base.h" 3

# 1 "/usr/include/c++/11/ext/atomicity.h" 1 3
# 32 "/usr/include/c++/11/ext/atomicity.h" 3
       
# 33 "/usr/include/c++/11/ext/atomicity.h" 3


# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr.h" 1 3
# 30 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr.h" 3
#pragma GCC visibility push(default)
# 148 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr.h" 3
# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 1 3
# 35 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3
# 1 "/usr/include/pthread.h" 1 3 4
# 22 "/usr/include/pthread.h" 3 4
# 1 "/usr/include/sched.h" 1 3 4
# 29 "/usr/include/sched.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 30 "/usr/include/sched.h" 2 3 4

# 1 "/usr/include/bits/types/time_t.h" 1 3 4
# 10 "/usr/include/bits/types/time_t.h" 3 4
typedef __time_t time_t;
# 32 "/usr/include/sched.h" 2 3 4
# 1 "/usr/include/bits/types/struct_timespec.h" 1 3 4
# 11 "/usr/include/bits/types/struct_timespec.h" 3 4
struct timespec
{



  __time_t tv_sec;




  __syscall_slong_t tv_nsec;
# 31 "/usr/include/bits/types/struct_timespec.h" 3 4
};
# 33 "/usr/include/sched.h" 2 3 4





typedef __pid_t pid_t;




# 1 "/usr/include/bits/sched.h" 1 3 4
# 76 "/usr/include/bits/sched.h" 3 4
# 1 "/usr/include/bits/types/struct_sched_param.h" 1 3 4
# 23 "/usr/include/bits/types/struct_sched_param.h" 3 4
struct sched_param
{
  int sched_priority;
};
# 77 "/usr/include/bits/sched.h" 2 3 4

extern "C" {



extern int clone (int (*__fn) (void *__arg), void *__child_stack,
    int __flags, void *__arg, ...) noexcept (true);


extern int unshare (int __flags) noexcept (true);


extern int sched_getcpu (void) noexcept (true);


extern int getcpu (unsigned int *, unsigned int *) noexcept (true);


extern int setns (int __fd, int __nstype) noexcept (true);


}
# 44 "/usr/include/sched.h" 2 3 4
# 1 "/usr/include/bits/cpu-set.h" 1 3 4
# 32 "/usr/include/bits/cpu-set.h" 3 4
typedef unsigned long int __cpu_mask;






typedef struct
{
  __cpu_mask __bits[1024 / (8 * sizeof (__cpu_mask))];
} cpu_set_t;
# 115 "/usr/include/bits/cpu-set.h" 3 4
extern "C" {

extern int __sched_cpucount (size_t __setsize, const cpu_set_t *__setp)
     noexcept (true);
extern cpu_set_t *__sched_cpualloc (size_t __count) noexcept (true) ;
extern void __sched_cpufree (cpu_set_t *__set) noexcept (true);

}
# 45 "/usr/include/sched.h" 2 3 4






extern "C" {


extern int sched_setparam (__pid_t __pid, const struct sched_param *__param)
     noexcept (true);


extern int sched_getparam (__pid_t __pid, struct sched_param *__param) noexcept (true);


extern int sched_setscheduler (__pid_t __pid, int __policy,
          const struct sched_param *__param) noexcept (true);


extern int sched_getscheduler (__pid_t __pid) noexcept (true);


extern int sched_yield (void) noexcept (true);


extern int sched_get_priority_max (int __algorithm) noexcept (true);


extern int sched_get_priority_min (int __algorithm) noexcept (true);



extern int sched_rr_get_interval (__pid_t __pid, struct timespec *__t) noexcept (true);
# 130 "/usr/include/sched.h" 3 4
extern int sched_setaffinity (__pid_t __pid, size_t __cpusetsize,
         const cpu_set_t *__cpuset) noexcept (true);


extern int sched_getaffinity (__pid_t __pid, size_t __cpusetsize,
         cpu_set_t *__cpuset) noexcept (true);


}
# 23 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/time.h" 1 3 4
# 29 "/usr/include/time.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 30 "/usr/include/time.h" 2 3 4



# 1 "/usr/include/bits/time.h" 1 3 4
# 73 "/usr/include/bits/time.h" 3 4
# 1 "/usr/include/bits/timex.h" 1 3 4
# 22 "/usr/include/bits/timex.h" 3 4
# 1 "/usr/include/bits/types/struct_timeval.h" 1 3 4







struct timeval
{




  __time_t tv_sec;
  __suseconds_t tv_usec;

};
# 23 "/usr/include/bits/timex.h" 2 3 4



struct timex
{
# 58 "/usr/include/bits/timex.h" 3 4
  unsigned int modes;
  __syscall_slong_t offset;
  __syscall_slong_t freq;
  __syscall_slong_t maxerror;
  __syscall_slong_t esterror;
  int status;
  __syscall_slong_t constant;
  __syscall_slong_t precision;
  __syscall_slong_t tolerance;
  struct timeval time;
  __syscall_slong_t tick;
  __syscall_slong_t ppsfreq;
  __syscall_slong_t jitter;
  int shift;
  __syscall_slong_t stabil;
  __syscall_slong_t jitcnt;
  __syscall_slong_t calcnt;
  __syscall_slong_t errcnt;
  __syscall_slong_t stbcnt;

  int tai;


  int :32; int :32; int :32; int :32;
  int :32; int :32; int :32; int :32;
  int :32; int :32; int :32;

};
# 74 "/usr/include/bits/time.h" 2 3 4

extern "C" {


extern int clock_adjtime (__clockid_t __clock_id, struct timex *__utx) noexcept (true);
# 90 "/usr/include/bits/time.h" 3 4
}
# 34 "/usr/include/time.h" 2 3 4



# 1 "/usr/include/bits/types/clock_t.h" 1 3 4






typedef __clock_t clock_t;
# 38 "/usr/include/time.h" 2 3 4

# 1 "/usr/include/bits/types/struct_tm.h" 1 3 4






struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;


  long int tm_gmtoff;
  const char *tm_zone;




};
# 40 "/usr/include/time.h" 2 3 4






# 1 "/usr/include/bits/types/clockid_t.h" 1 3 4






typedef __clockid_t clockid_t;
# 47 "/usr/include/time.h" 2 3 4
# 1 "/usr/include/bits/types/timer_t.h" 1 3 4






typedef __timer_t timer_t;
# 48 "/usr/include/time.h" 2 3 4
# 1 "/usr/include/bits/types/struct_itimerspec.h" 1 3 4







struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
# 49 "/usr/include/time.h" 2 3 4
struct sigevent;
# 68 "/usr/include/time.h" 3 4
extern "C" {



extern clock_t clock (void) noexcept (true);



extern time_t time (time_t *__timer) noexcept (true);


extern double difftime (time_t __time1, time_t __time0)
     noexcept (true) __attribute__ ((__const__));


extern time_t mktime (struct tm *__tp) noexcept (true);
# 100 "/usr/include/time.h" 3 4
extern size_t strftime (char *__restrict __s, size_t __maxsize,
   const char *__restrict __format,
   const struct tm *__restrict __tp) noexcept (true);




extern char *strptime (const char *__restrict __s,
         const char *__restrict __fmt, struct tm *__tp)
     noexcept (true);






extern size_t strftime_l (char *__restrict __s, size_t __maxsize,
     const char *__restrict __format,
     const struct tm *__restrict __tp,
     locale_t __loc) noexcept (true);



extern char *strptime_l (const char *__restrict __s,
    const char *__restrict __fmt, struct tm *__tp,
    locale_t __loc) noexcept (true);






extern struct tm *gmtime (const time_t *__timer) noexcept (true);



extern struct tm *localtime (const time_t *__timer) noexcept (true);
# 154 "/usr/include/time.h" 3 4
extern struct tm *gmtime_r (const time_t *__restrict __timer,
       struct tm *__restrict __tp) noexcept (true);



extern struct tm *localtime_r (const time_t *__restrict __timer,
          struct tm *__restrict __tp) noexcept (true);
# 179 "/usr/include/time.h" 3 4
extern char *asctime (const struct tm *__tp) noexcept (true);



extern char *ctime (const time_t *__timer) noexcept (true);
# 197 "/usr/include/time.h" 3 4
extern char *asctime_r (const struct tm *__restrict __tp,
   char *__restrict __buf) noexcept (true);



extern char *ctime_r (const time_t *__restrict __timer,
        char *__restrict __buf) noexcept (true);
# 217 "/usr/include/time.h" 3 4
extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;




extern char *tzname[2];



extern void tzset (void) noexcept (true);



extern int daylight;
extern long int timezone;
# 249 "/usr/include/time.h" 3 4
extern time_t timegm (struct tm *__tp) noexcept (true);

extern time_t timelocal (struct tm *__tp) noexcept (true);
# 262 "/usr/include/time.h" 3 4
extern int dysize (int __year) noexcept (true) __attribute__ ((__const__));
# 272 "/usr/include/time.h" 3 4
extern int nanosleep (const struct timespec *__requested_time,
        struct timespec *__remaining);


extern int clock_getres (clockid_t __clock_id, struct timespec *__res) noexcept (true);


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) noexcept (true);


extern int clock_settime (clockid_t __clock_id, const struct timespec *__tp)
     noexcept (true);
# 311 "/usr/include/time.h" 3 4
extern int clock_nanosleep (clockid_t __clock_id, int __flags,
       const struct timespec *__req,
       struct timespec *__rem);
# 326 "/usr/include/time.h" 3 4
extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id) noexcept (true);




extern int timer_create (clockid_t __clock_id,
    struct sigevent *__restrict __evp,
    timer_t *__restrict __timerid) noexcept (true);


extern int timer_delete (timer_t __timerid) noexcept (true);



extern int timer_settime (timer_t __timerid, int __flags,
     const struct itimerspec *__restrict __value,
     struct itimerspec *__restrict __ovalue) noexcept (true);


extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     noexcept (true);
# 364 "/usr/include/time.h" 3 4
extern int timer_getoverrun (timer_t __timerid) noexcept (true);






extern int timespec_get (struct timespec *__ts, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 387 "/usr/include/time.h" 3 4
extern int timespec_getres (struct timespec *__ts, int __base)
     noexcept (true);
# 413 "/usr/include/time.h" 3 4
extern int getdate_err;
# 422 "/usr/include/time.h" 3 4
extern struct tm *getdate (const char *__string);
# 436 "/usr/include/time.h" 3 4
extern int getdate_r (const char *__restrict __string,
        struct tm *__restrict __resbufp);


}
# 24 "/usr/include/pthread.h" 2 3 4


# 1 "/usr/include/bits/pthreadtypes.h" 1 3 4
# 23 "/usr/include/bits/pthreadtypes.h" 3 4
# 1 "/usr/include/bits/thread-shared-types.h" 1 3 4
# 44 "/usr/include/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/bits/pthreadtypes-arch.h" 1 3 4
# 21 "/usr/include/bits/pthreadtypes-arch.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 22 "/usr/include/bits/pthreadtypes-arch.h" 2 3 4
# 45 "/usr/include/bits/thread-shared-types.h" 2 3 4




typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
# 74 "/usr/include/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/bits/struct_mutex.h" 1 3 4
# 22 "/usr/include/bits/struct_mutex.h" 3 4
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;

  unsigned int __nusers;



  int __kind;

  short __spins;
  short __elision;
  __pthread_list_t __list;
# 53 "/usr/include/bits/struct_mutex.h" 3 4
};
# 75 "/usr/include/bits/thread-shared-types.h" 2 3 4
# 87 "/usr/include/bits/thread-shared-types.h" 3 4
# 1 "/usr/include/bits/struct_rwlock.h" 1 3 4
# 23 "/usr/include/bits/struct_rwlock.h" 3 4
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;

  int __cur_writer;
  int __shared;
  signed char __rwelision;




  unsigned char __pad1[7];


  unsigned long int __pad2;


  unsigned int __flags;
# 55 "/usr/include/bits/struct_rwlock.h" 3 4
};
# 88 "/usr/include/bits/thread-shared-types.h" 2 3 4




struct __pthread_cond_s
{
  __extension__ union
  {
    __extension__ unsigned long long int __wseq;
    struct
    {
      unsigned int __low;
      unsigned int __high;
    } __wseq32;
  };
  __extension__ union
  {
    __extension__ unsigned long long int __g1_start;
    struct
    {
      unsigned int __low;
      unsigned int __high;
    } __g1_start32;
  };
  unsigned int __g_refs[2] ;
  unsigned int __g_size[2];
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct
{
  int __data ;
} __once_flag;
# 24 "/usr/include/bits/pthreadtypes.h" 2 3 4



typedef unsigned long int pthread_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;


union pthread_attr_t
{
  char __size[56];
  long int __align;
};

typedef union pthread_attr_t pthread_attr_t;




typedef union
{
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;


typedef union
{
  struct __pthread_cond_s __data;
  char __size[48];
  __extension__ long long int __align;
} pthread_cond_t;





typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef volatile int pthread_spinlock_t;




typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
# 27 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/bits/setjmp.h" 1 3 4
# 26 "/usr/include/bits/setjmp.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 27 "/usr/include/bits/setjmp.h" 2 3 4




typedef long int __jmp_buf[8];
# 28 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 29 "/usr/include/pthread.h" 2 3 4

# 1 "/usr/include/bits/types/__sigset_t.h" 1 3 4




typedef struct
{
  unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
} __sigset_t;
# 31 "/usr/include/pthread.h" 2 3 4
# 1 "/usr/include/bits/types/struct___jmp_buf_tag.h" 1 3 4
# 26 "/usr/include/bits/types/struct___jmp_buf_tag.h" 3 4
struct __jmp_buf_tag
  {




    __jmp_buf __jmpbuf;
    int __mask_was_saved;
    __sigset_t __saved_mask;
  };
# 32 "/usr/include/pthread.h" 2 3 4

# 1 "/usr/include/bits/pthread_stack_min-dynamic.h" 1 3 4
# 23 "/usr/include/bits/pthread_stack_min-dynamic.h" 3 4
extern "C" {
extern long int __sysconf (int __name) noexcept (true);
}
# 34 "/usr/include/pthread.h" 2 3 4



enum
{
  PTHREAD_CREATE_JOINABLE,

  PTHREAD_CREATE_DETACHED

};



enum
{
  PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_ADAPTIVE_NP

  ,
  PTHREAD_MUTEX_NORMAL = PTHREAD_MUTEX_TIMED_NP,
  PTHREAD_MUTEX_RECURSIVE = PTHREAD_MUTEX_RECURSIVE_NP,
  PTHREAD_MUTEX_ERRORCHECK = PTHREAD_MUTEX_ERRORCHECK_NP,
  PTHREAD_MUTEX_DEFAULT = PTHREAD_MUTEX_NORMAL



  , PTHREAD_MUTEX_FAST_NP = PTHREAD_MUTEX_TIMED_NP

};




enum
{
  PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_STALLED_NP = PTHREAD_MUTEX_STALLED,
  PTHREAD_MUTEX_ROBUST,
  PTHREAD_MUTEX_ROBUST_NP = PTHREAD_MUTEX_ROBUST
};





enum
{
  PTHREAD_PRIO_NONE,
  PTHREAD_PRIO_INHERIT,
  PTHREAD_PRIO_PROTECT
};
# 104 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_RWLOCK_PREFER_READER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NP,
  PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP,
  PTHREAD_RWLOCK_DEFAULT_NP = PTHREAD_RWLOCK_PREFER_READER_NP
};
# 124 "/usr/include/pthread.h" 3 4
enum
{
  PTHREAD_INHERIT_SCHED,

  PTHREAD_EXPLICIT_SCHED

};



enum
{
  PTHREAD_SCOPE_SYSTEM,

  PTHREAD_SCOPE_PROCESS

};



enum
{
  PTHREAD_PROCESS_PRIVATE,

  PTHREAD_PROCESS_SHARED

};
# 159 "/usr/include/pthread.h" 3 4
struct _pthread_cleanup_buffer
{
  void (*__routine) (void *);
  void *__arg;
  int __canceltype;
  struct _pthread_cleanup_buffer *__prev;
};


enum
{
  PTHREAD_CANCEL_ENABLE,

  PTHREAD_CANCEL_DISABLE

};
enum
{
  PTHREAD_CANCEL_DEFERRED,

  PTHREAD_CANCEL_ASYNCHRONOUS

};
# 197 "/usr/include/pthread.h" 3 4
extern "C" {




extern int pthread_create (pthread_t *__restrict __newthread,
      const pthread_attr_t *__restrict __attr,
      void *(*__start_routine) (void *),
      void *__restrict __arg) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));





extern void pthread_exit (void *__retval) __attribute__ ((__noreturn__));







extern int pthread_join (pthread_t __th, void **__thread_return);




extern int pthread_tryjoin_np (pthread_t __th, void **__thread_return) noexcept (true);
# 233 "/usr/include/pthread.h" 3 4
extern int pthread_timedjoin_np (pthread_t __th, void **__thread_return,
     const struct timespec *__abstime);
# 243 "/usr/include/pthread.h" 3 4
extern int pthread_clockjoin_np (pthread_t __th, void **__thread_return,
                                 clockid_t __clockid,
     const struct timespec *__abstime);
# 269 "/usr/include/pthread.h" 3 4
extern int pthread_detach (pthread_t __th) noexcept (true);



extern pthread_t pthread_self (void) noexcept (true) __attribute__ ((__const__));


extern int pthread_equal (pthread_t __thread1, pthread_t __thread2)
  noexcept (true) __attribute__ ((__const__));







extern int pthread_attr_init (pthread_attr_t *__attr) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_destroy (pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getdetachstate (const pthread_attr_t *__attr,
     int *__detachstate)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setdetachstate (pthread_attr_t *__attr,
     int __detachstate)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getguardsize (const pthread_attr_t *__attr,
          size_t *__guardsize)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setguardsize (pthread_attr_t *__attr,
          size_t __guardsize)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getschedparam (const pthread_attr_t *__restrict __attr,
           struct sched_param *__restrict __param)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedparam (pthread_attr_t *__restrict __attr,
           const struct sched_param *__restrict
           __param) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_getschedpolicy (const pthread_attr_t *__restrict
     __attr, int *__restrict __policy)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setschedpolicy (pthread_attr_t *__attr, int __policy)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getinheritsched (const pthread_attr_t *__restrict
      __attr, int *__restrict __inherit)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setinheritsched (pthread_attr_t *__attr,
      int __inherit)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getscope (const pthread_attr_t *__restrict __attr,
      int *__restrict __scope)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_attr_setscope (pthread_attr_t *__attr, int __scope)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_getstackaddr (const pthread_attr_t *__restrict
          __attr, void **__restrict __stackaddr)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__deprecated__));





extern int pthread_attr_setstackaddr (pthread_attr_t *__attr,
          void *__stackaddr)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__deprecated__));


extern int pthread_attr_getstacksize (const pthread_attr_t *__restrict
          __attr, size_t *__restrict __stacksize)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_attr_setstacksize (pthread_attr_t *__attr,
          size_t __stacksize)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_attr_getstack (const pthread_attr_t *__restrict __attr,
      void **__restrict __stackaddr,
      size_t *__restrict __stacksize)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2, 3)));




extern int pthread_attr_setstack (pthread_attr_t *__attr, void *__stackaddr,
      size_t __stacksize) noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int pthread_attr_setaffinity_np (pthread_attr_t *__attr,
     size_t __cpusetsize,
     const cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern int pthread_attr_getaffinity_np (const pthread_attr_t *__attr,
     size_t __cpusetsize,
     cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));


extern int pthread_getattr_default_np (pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_attr_setsigmask_np (pthread_attr_t *__attr,
           const __sigset_t *sigmask);




extern int pthread_attr_getsigmask_np (const pthread_attr_t *__attr,
           __sigset_t *sigmask);







extern int pthread_setattr_default_np (const pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_getattr_np (pthread_t __th, pthread_attr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (2)));







extern int pthread_setschedparam (pthread_t __target_thread, int __policy,
      const struct sched_param *__param)
     noexcept (true) __attribute__ ((__nonnull__ (3)));


extern int pthread_getschedparam (pthread_t __target_thread,
      int *__restrict __policy,
      struct sched_param *__restrict __param)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));


extern int pthread_setschedprio (pthread_t __target_thread, int __prio)
     noexcept (true);




extern int pthread_getname_np (pthread_t __target_thread, char *__buf,
          size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2)));


extern int pthread_setname_np (pthread_t __target_thread, const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (2)));





extern int pthread_getconcurrency (void) noexcept (true);


extern int pthread_setconcurrency (int __level) noexcept (true);



extern int pthread_yield (void) noexcept (true);

extern int pthread_yield (void) noexcept (true) __asm__ ("" "sched_yield")
  __attribute__ ((__deprecated__ ("pthread_yield is deprecated, use sched_yield instead")))
                                                      ;







extern int pthread_setaffinity_np (pthread_t __th, size_t __cpusetsize,
       const cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (3)));


extern int pthread_getaffinity_np (pthread_t __th, size_t __cpusetsize,
       cpu_set_t *__cpuset)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 509 "/usr/include/pthread.h" 3 4
extern int pthread_once (pthread_once_t *__once_control,
    void (*__init_routine) (void)) __attribute__ ((__nonnull__ (1, 2)));
# 521 "/usr/include/pthread.h" 3 4
extern int pthread_setcancelstate (int __state, int *__oldstate);



extern int pthread_setcanceltype (int __type, int *__oldtype);


extern int pthread_cancel (pthread_t __th);




extern void pthread_testcancel (void);




struct __cancel_jmp_buf_tag
{
  __jmp_buf __cancel_jmp_buf;
  int __mask_was_saved;
};

typedef struct
{
  struct __cancel_jmp_buf_tag __cancel_jmp_buf[1];
  void *__pad[4];
} __pthread_unwind_buf_t __attribute__ ((__aligned__));
# 557 "/usr/include/pthread.h" 3 4
struct __pthread_cleanup_frame
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;
};




class __pthread_cleanup_class
{
  void (*__cancel_routine) (void *);
  void *__cancel_arg;
  int __do_it;
  int __cancel_type;

 public:
  __pthread_cleanup_class (void (*__fct) (void *), void *__arg)
    : __cancel_routine (__fct), __cancel_arg (__arg), __do_it (1) { }
  ~__pthread_cleanup_class () { if (__do_it) __cancel_routine (__cancel_arg); }
  void __setdoit (int __newval) { __do_it = __newval; }
  void __defer () { pthread_setcanceltype (PTHREAD_CANCEL_DEFERRED,
        &__cancel_type); }
  void __restore () const { pthread_setcanceltype (__cancel_type, 0); }
};
# 766 "/usr/include/pthread.h" 3 4
extern int __sigsetjmp_cancel (struct __cancel_jmp_buf_tag __env[1], int __savemask) noexcept (true) __asm__ ("" "__sigsetjmp")


                     __attribute__ ((__returns_twice__));
# 781 "/usr/include/pthread.h" 3 4
extern int pthread_mutex_init (pthread_mutex_t *__mutex,
          const pthread_mutexattr_t *__mutexattr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_destroy (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_trylock (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_lock (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_mutex_timedlock (pthread_mutex_t *__restrict __mutex,
        const struct timespec *__restrict
        __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 817 "/usr/include/pthread.h" 3 4
extern int pthread_mutex_clocklock (pthread_mutex_t *__restrict __mutex,
        clockid_t __clockid,
        const struct timespec *__restrict
        __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 835 "/usr/include/pthread.h" 3 4
extern int pthread_mutex_unlock (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutex_getprioceiling (const pthread_mutex_t *
      __restrict __mutex,
      int *__restrict __prioceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutex_setprioceiling (pthread_mutex_t *__restrict __mutex,
      int __prioceiling,
      int *__restrict __old_ceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));




extern int pthread_mutex_consistent (pthread_mutex_t *__mutex)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutex_consistent_np (pthread_mutex_t *) noexcept (true) __asm__ ("" "pthread_mutex_consistent")
                                __attribute__ ((__nonnull__ (1)))
  __attribute__ ((__deprecated__ ("pthread_mutex_consistent_np is deprecated, use pthread_mutex_consistent")))
                                                                         ;
# 874 "/usr/include/pthread.h" 3 4
extern int pthread_mutexattr_init (pthread_mutexattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_destroy (pthread_mutexattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getpshared (const pthread_mutexattr_t *
      __restrict __attr,
      int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setpshared (pthread_mutexattr_t *__attr,
      int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_gettype (const pthread_mutexattr_t *__restrict
          __attr, int *__restrict __kind)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_getprotocol (const pthread_mutexattr_t *
       __restrict __attr,
       int *__restrict __protocol)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern int pthread_mutexattr_setprotocol (pthread_mutexattr_t *__attr,
       int __protocol)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_getprioceiling (const pthread_mutexattr_t *
          __restrict __attr,
          int *__restrict __prioceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_setprioceiling (pthread_mutexattr_t *__attr,
          int __prioceiling)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_mutexattr_getrobust (const pthread_mutexattr_t *__attr,
     int *__robustness)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_mutexattr_getrobust_np (pthread_mutexattr_t *, int *) noexcept (true) __asm__ ("" "pthread_mutexattr_getrobust")

                                   __attribute__ ((__nonnull__ (1)))
  __attribute__ ((__deprecated__ ("pthread_mutexattr_getrobust_np is deprecated, use pthread_mutexattr_getrobust")))
                                                                               ;






extern int pthread_mutexattr_setrobust (pthread_mutexattr_t *__attr,
     int __robustness)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_mutexattr_setrobust_np (pthread_mutexattr_t *, int) noexcept (true) __asm__ ("" "pthread_mutexattr_setrobust")

                                   __attribute__ ((__nonnull__ (1)))
  __attribute__ ((__deprecated__ ("pthread_mutexattr_setrobust_np is deprecated, use pthread_mutexattr_setrobust")))
                                                                               ;
# 967 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_init (pthread_rwlock_t *__restrict __rwlock,
    const pthread_rwlockattr_t *__restrict
    __attr) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_tryrdlock (pthread_rwlock_t *__rwlock)
  noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_rwlock_timedrdlock (pthread_rwlock_t *__restrict __rwlock,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 1004 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_clockrdlock (pthread_rwlock_t *__restrict __rwlock,
           clockid_t __clockid,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 1023 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlock_trywrlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int pthread_rwlock_timedwrlock (pthread_rwlock_t *__restrict __rwlock,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
# 1051 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_clockwrlock (pthread_rwlock_t *__restrict __rwlock,
           clockid_t __clockid,
           const struct timespec *__restrict
           __abstime) noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 1071 "/usr/include/pthread.h" 3 4
extern int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int pthread_rwlockattr_init (pthread_rwlockattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_destroy (pthread_rwlockattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getpshared (const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setpshared (pthread_rwlockattr_t *__attr,
       int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_rwlockattr_getkind_np (const pthread_rwlockattr_t *
       __restrict __attr,
       int *__restrict __pref)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_rwlockattr_setkind_np (pthread_rwlockattr_t *__attr,
       int __pref) noexcept (true) __attribute__ ((__nonnull__ (1)));







extern int pthread_cond_init (pthread_cond_t *__restrict __cond,
         const pthread_condattr_t *__restrict __cond_attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_destroy (pthread_cond_t *__cond)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_signal (pthread_cond_t *__cond)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_cond_broadcast (pthread_cond_t *__cond)
     noexcept (true) __attribute__ ((__nonnull__ (1)));






extern int pthread_cond_wait (pthread_cond_t *__restrict __cond,
         pthread_mutex_t *__restrict __mutex)
     __attribute__ ((__nonnull__ (1, 2)));
# 1145 "/usr/include/pthread.h" 3 4
extern int pthread_cond_timedwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       const struct timespec *__restrict __abstime)
     __attribute__ ((__nonnull__ (1, 2, 3)));
# 1171 "/usr/include/pthread.h" 3 4
extern int pthread_cond_clockwait (pthread_cond_t *__restrict __cond,
       pthread_mutex_t *__restrict __mutex,
       __clockid_t __clock_id,
       const struct timespec *__restrict __abstime)
     __attribute__ ((__nonnull__ (1, 2, 4)));
# 1194 "/usr/include/pthread.h" 3 4
extern int pthread_condattr_init (pthread_condattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_destroy (pthread_condattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_condattr_getpshared (const pthread_condattr_t *
     __restrict __attr,
     int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setpshared (pthread_condattr_t *__attr,
     int __pshared) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_condattr_getclock (const pthread_condattr_t *
          __restrict __attr,
          __clockid_t *__restrict __clock_id)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_condattr_setclock (pthread_condattr_t *__attr,
          __clockid_t __clock_id)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 1230 "/usr/include/pthread.h" 3 4
extern int pthread_spin_init (pthread_spinlock_t *__lock, int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_destroy (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_lock (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_trylock (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_spin_unlock (pthread_spinlock_t *__lock)
     noexcept (true) __attribute__ ((__nonnull__ (1)));






extern int pthread_barrier_init (pthread_barrier_t *__restrict __barrier,
     const pthread_barrierattr_t *__restrict
     __attr, unsigned int __count)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_destroy (pthread_barrier_t *__barrier)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrier_wait (pthread_barrier_t *__barrier)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern int pthread_barrierattr_init (pthread_barrierattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_destroy (pthread_barrierattr_t *__attr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_barrierattr_getpshared (const pthread_barrierattr_t *
        __restrict __attr,
        int *__restrict __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int pthread_barrierattr_setpshared (pthread_barrierattr_t *__attr,
        int __pshared)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 1297 "/usr/include/pthread.h" 3 4
extern int pthread_key_create (pthread_key_t *__key,
          void (*__destr_function) (void *))
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int pthread_key_delete (pthread_key_t __key) noexcept (true);


extern void *pthread_getspecific (pthread_key_t __key) noexcept (true);


extern int pthread_setspecific (pthread_key_t __key,
    const void *__pointer)
  noexcept (true) __attribute__ ((__access__ (__none__, 2)));




extern int pthread_getcpuclockid (pthread_t __thread_id,
      __clockid_t *__clock_id)
     noexcept (true) __attribute__ ((__nonnull__ (2)));
# 1332 "/usr/include/pthread.h" 3 4
extern int pthread_atfork (void (*__prepare) (void),
      void (*__parent) (void),
      void (*__child) (void)) noexcept (true);
# 1346 "/usr/include/pthread.h" 3 4
}
# 36 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 2 3
# 47 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3
typedef pthread_t __gthread_t;
typedef pthread_key_t __gthread_key_t;
typedef pthread_once_t __gthread_once_t;
typedef pthread_mutex_t __gthread_mutex_t;
typedef pthread_mutex_t __gthread_recursive_mutex_t;
typedef pthread_cond_t __gthread_cond_t;
typedef struct timespec __gthread_time_t;
# 102 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3



































# 299 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3
static inline int
__gthread_active_p (void)
{
  return 1;
}
# 659 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3
static inline int
__gthread_create (__gthread_t *__threadid, void *(*__func) (void*),
    void *__args)
{
  return pthread_create (__threadid, __null, __func, __args);
}

static inline int
__gthread_join (__gthread_t __threadid, void **__value_ptr)
{
  return pthread_join (__threadid, __value_ptr);
}

static inline int
__gthread_detach (__gthread_t __threadid)
{
  return pthread_detach (__threadid);
}

static inline int
__gthread_equal (__gthread_t __t1, __gthread_t __t2)
{
  return pthread_equal (__t1, __t2);
}

static inline __gthread_t
__gthread_self (void)
{
  return pthread_self ();
}

static inline int
__gthread_yield (void)
{
  return sched_yield ();
}

static inline int
__gthread_once (__gthread_once_t *__once, void (*__func) (void))
{
  if (__gthread_active_p ())
    return pthread_once (__once, __func);
  else
    return -1;
}

static inline int
__gthread_key_create (__gthread_key_t *__key, void (*__dtor) (void *))
{
  return pthread_key_create (__key, __dtor);
}

static inline int
__gthread_key_delete (__gthread_key_t __key)
{
  return pthread_key_delete (__key);
}

static inline void *
__gthread_getspecific (__gthread_key_t __key)
{
  return pthread_getspecific (__key);
}

static inline int
__gthread_setspecific (__gthread_key_t __key, const void *__ptr)
{
  return pthread_setspecific (__key, __ptr);
}

static inline void
__gthread_mutex_init_function (__gthread_mutex_t *__mutex)
{
  if (__gthread_active_p ())
    pthread_mutex_init (__mutex, __null);
}

static inline int
__gthread_mutex_destroy (__gthread_mutex_t *__mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_destroy (__mutex);
  else
    return 0;
}

static inline int
__gthread_mutex_lock (__gthread_mutex_t *__mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_lock (__mutex);
  else
    return 0;
}

static inline int
__gthread_mutex_trylock (__gthread_mutex_t *__mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_trylock (__mutex);
  else
    return 0;
}


static inline int
__gthread_mutex_timedlock (__gthread_mutex_t *__mutex,
      const __gthread_time_t *__abs_timeout)
{
  if (__gthread_active_p ())
    return pthread_mutex_timedlock (__mutex, __abs_timeout);
  else
    return 0;
}


static inline int
__gthread_mutex_unlock (__gthread_mutex_t *__mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_unlock (__mutex);
  else
    return 0;
}
# 808 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3
static inline int
__gthread_recursive_mutex_lock (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_lock (__mutex);
}

static inline int
__gthread_recursive_mutex_trylock (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_trylock (__mutex);
}


static inline int
__gthread_recursive_mutex_timedlock (__gthread_recursive_mutex_t *__mutex,
         const __gthread_time_t *__abs_timeout)
{
  return __gthread_mutex_timedlock (__mutex, __abs_timeout);
}


static inline int
__gthread_recursive_mutex_unlock (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_unlock (__mutex);
}

static inline int
__gthread_recursive_mutex_destroy (__gthread_recursive_mutex_t *__mutex)
{
  return __gthread_mutex_destroy (__mutex);
}
# 850 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr-default.h" 3
static inline int
__gthread_cond_broadcast (__gthread_cond_t *__cond)
{
  return pthread_cond_broadcast (__cond);
}

static inline int
__gthread_cond_signal (__gthread_cond_t *__cond)
{
  return pthread_cond_signal (__cond);
}

static inline int
__gthread_cond_wait (__gthread_cond_t *__cond, __gthread_mutex_t *__mutex)
{
  return pthread_cond_wait (__cond, __mutex);
}

static inline int
__gthread_cond_timedwait (__gthread_cond_t *__cond, __gthread_mutex_t *__mutex,
     const __gthread_time_t *__abs_timeout)
{
  return pthread_cond_timedwait (__cond, __mutex, __abs_timeout);
}

static inline int
__gthread_cond_wait_recursive (__gthread_cond_t *__cond,
          __gthread_recursive_mutex_t *__mutex)
{
  return __gthread_cond_wait (__cond, __mutex);
}

static inline int
__gthread_cond_destroy (__gthread_cond_t* __cond)
{
  return pthread_cond_destroy (__cond);
}
# 149 "/usr/include/c++/11/x86_64-redhat-linux/bits/gthr.h" 2 3


#pragma GCC visibility pop
# 36 "/usr/include/c++/11/ext/atomicity.h" 2 3
# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/atomic_word.h" 1 3
# 32 "/usr/include/c++/11/x86_64-redhat-linux/bits/atomic_word.h" 3
typedef int _Atomic_word;
# 37 "/usr/include/c++/11/ext/atomicity.h" 2 3

# 1 "/usr/include/sys/single_threaded.h" 1 3 4
# 24 "/usr/include/sys/single_threaded.h" 3 4
extern "C" {




extern char __libc_single_threaded;

}
# 39 "/usr/include/c++/11/ext/atomicity.h" 2 3


namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{


  __attribute__((__always_inline__))
  inline bool
  __is_single_threaded() noexcept
  {



    return ::__libc_single_threaded;



  }






  inline _Atomic_word
  __attribute__((__always_inline__))
  __exchange_and_add(volatile _Atomic_word* __mem, int __val)
  { return __atomic_fetch_add(__mem, __val, 4); }

  inline void
  __attribute__((__always_inline__))
  __atomic_add(volatile _Atomic_word* __mem, int __val)
  { __atomic_fetch_add(__mem, __val, 4); }
# 80 "/usr/include/c++/11/ext/atomicity.h" 3
  inline _Atomic_word
  __attribute__((__always_inline__))
  __exchange_and_add_single(_Atomic_word* __mem, int __val)
  {
    _Atomic_word __result = *__mem;
    *__mem += __val;
    return __result;
  }

  inline void
  __attribute__((__always_inline__))
  __atomic_add_single(_Atomic_word* __mem, int __val)
  { *__mem += __val; }

  inline _Atomic_word
  __attribute__ ((__always_inline__))
  __exchange_and_add_dispatch(_Atomic_word* __mem, int __val)
  {
    if (__is_single_threaded())
      return __exchange_and_add_single(__mem, __val);
    else
      return __exchange_and_add(__mem, __val);
  }

  inline void
  __attribute__ ((__always_inline__))
  __atomic_add_dispatch(_Atomic_word* __mem, int __val)
  {
    if (__is_single_threaded())
      __atomic_add_single(__mem, __val);
    else
      __atomic_add(__mem, __val);
  }


}
# 40 "/usr/include/c++/11/bits/ios_base.h" 2 3

# 1 "/usr/include/c++/11/bits/locale_classes.h" 1 3
# 37 "/usr/include/c++/11/bits/locale_classes.h" 3
       
# 38 "/usr/include/c++/11/bits/locale_classes.h" 3


# 1 "/usr/include/c++/11/string" 1 3
# 36 "/usr/include/c++/11/string" 3
       
# 37 "/usr/include/c++/11/string" 3




# 1 "/usr/include/c++/11/bits/allocator.h" 1 3
# 46 "/usr/include/c++/11/bits/allocator.h" 3
# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++allocator.h" 1 3
# 33 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++allocator.h" 3
# 1 "/usr/include/c++/11/ext/new_allocator.h" 1 3
# 40 "/usr/include/c++/11/ext/new_allocator.h" 3
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{

# 54 "/usr/include/c++/11/ext/new_allocator.h" 3
  template<typename _Tp>
    class new_allocator
    {
    public:
      typedef _Tp value_type;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

      typedef _Tp* pointer;
      typedef const _Tp* const_pointer;
      typedef _Tp& reference;
      typedef const _Tp& const_reference;

      template<typename _Tp1>
 struct rebind
 { typedef new_allocator<_Tp1> other; };





      typedef std::true_type propagate_on_container_move_assignment;


     
      new_allocator() noexcept { }

     
      new_allocator(const new_allocator&) noexcept { }

      template<typename _Tp1>

 new_allocator(const new_allocator<_Tp1>&) noexcept { }


      ~new_allocator() noexcept { }

      pointer
      address(reference __x) const noexcept
      { return std::__addressof(__x); }

      const_pointer
      address(const_reference __x) const noexcept
      { return std::__addressof(__x); }




      [[__nodiscard__]] _Tp*
      allocate(size_type __n, const void* = static_cast<const void*>(0))
      {



  static_assert(sizeof(_Tp) != 0, "cannot allocate incomplete types");


 if (__builtin_expect(__n > this->_M_max_size(), false))
   {


     if (__n > (std::size_t(-1) / sizeof(_Tp)))
       std::__throw_bad_array_new_length();
     std::__throw_bad_alloc();
   }


 if (alignof(_Tp) > 16)
   {
     std::align_val_t __al = std::align_val_t(alignof(_Tp));
     return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp), __al));
   }

 return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp)));
      }


      void
      deallocate(_Tp* __p, size_type __t __attribute__ ((__unused__)))
      {

 if (alignof(_Tp) > 16)
   {
     ::operator delete(__p,

         __t * sizeof(_Tp),

         std::align_val_t(alignof(_Tp)));
     return;
   }

 ::operator delete(__p

     , __t * sizeof(_Tp)

    );
      }


      size_type
      max_size() const noexcept
      { return _M_max_size(); }


      template<typename _Up, typename... _Args>
 void
 construct(_Up* __p, _Args&&... __args)
 noexcept(std::is_nothrow_constructible<_Up, _Args...>::value)
 { ::new((void *)__p) _Up(std::forward<_Args>(__args)...); }

      template<typename _Up>
 void
 destroy(_Up* __p)
 noexcept(std::is_nothrow_destructible<_Up>::value)
 { __p->~_Up(); }
# 181 "/usr/include/c++/11/ext/new_allocator.h" 3
      template<typename _Up>
 friend bool
 operator==(const new_allocator&, const new_allocator<_Up>&)
 noexcept
 { return true; }


      template<typename _Up>
 friend bool
 operator!=(const new_allocator&, const new_allocator<_Up>&)
 noexcept
 { return false; }


    private:
      constexpr size_type
      _M_max_size() const noexcept
      {

 return std::size_t(0x7fffffffffffffffL) / sizeof(_Tp);



      }
    };


}
# 34 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++allocator.h" 2 3


namespace std
{
# 47 "/usr/include/c++/11/x86_64-redhat-linux/bits/c++allocator.h" 3
  template<typename _Tp>
    using __allocator_base = __gnu_cxx::new_allocator<_Tp>;
}
# 47 "/usr/include/c++/11/bits/allocator.h" 2 3







namespace std __attribute__ ((__visibility__ ("default")))
{

# 71 "/usr/include/c++/11/bits/allocator.h" 3
  template<>
    class allocator<void>
    {
    public:
      typedef void value_type;
      typedef size_t size_type;
      typedef ptrdiff_t difference_type;



      typedef void* pointer;
      typedef const void* const_pointer;

      template<typename _Tp1>
 struct rebind
 { typedef allocator<_Tp1> other; };





      using propagate_on_container_move_assignment = true_type;

      using is_always_equal

 = true_type;
# 113 "/usr/include/c++/11/bits/allocator.h" 3
    };
# 123 "/usr/include/c++/11/bits/allocator.h" 3
  template<typename _Tp>
    class allocator : public __allocator_base<_Tp>
    {
    public:
      typedef _Tp value_type;
      typedef size_t size_type;
      typedef ptrdiff_t difference_type;



      typedef _Tp* pointer;
      typedef const _Tp* const_pointer;
      typedef _Tp& reference;
      typedef const _Tp& const_reference;

      template<typename _Tp1>
 struct rebind
 { typedef allocator<_Tp1> other; };





      using propagate_on_container_move_assignment = true_type;

      using is_always_equal

 = true_type;




     
      allocator() noexcept { }

     
      allocator(const allocator& __a) noexcept
      : __allocator_base<_Tp>(__a) { }



      allocator& operator=(const allocator&) = default;


      template<typename _Tp1>

 allocator(const allocator<_Tp1>&) noexcept { }




      ~allocator() noexcept { }
# 203 "/usr/include/c++/11/bits/allocator.h" 3
      friend bool
      operator==(const allocator&, const allocator&) noexcept
      { return true; }


      friend bool
      operator!=(const allocator&, const allocator&) noexcept
      { return false; }



    };

  template<typename _T1, typename _T2>
    inline bool
    operator==(const allocator<_T1>&, const allocator<_T2>&)
    noexcept
    { return true; }


  template<typename _T1, typename _T2>
    inline bool
    operator!=(const allocator<_T1>&, const allocator<_T2>&)
    noexcept
    { return false; }




  template<typename _Tp>
    class allocator<const _Tp>
    {
    public:
      typedef _Tp value_type;
      template<typename _Up> allocator(const allocator<_Up>&) { }
    };

  template<typename _Tp>
    class allocator<volatile _Tp>
    {
    public:
      typedef _Tp value_type;
      template<typename _Up> allocator(const allocator<_Up>&) { }
    };

  template<typename _Tp>
    class allocator<const volatile _Tp>
    {
    public:
      typedef _Tp value_type;
      template<typename _Up> allocator(const allocator<_Up>&) { }
    };






  extern template class allocator<char>;
  extern template class allocator<wchar_t>;






  template<typename _Alloc, bool = __is_empty(_Alloc)>
    struct __alloc_swap
    { static void _S_do_it(_Alloc&, _Alloc&) noexcept { } };

  template<typename _Alloc>
    struct __alloc_swap<_Alloc, false>
    {
      static void
      _S_do_it(_Alloc& __one, _Alloc& __two) noexcept
      {

 if (__one != __two)
   swap(__one, __two);
      }
    };


  template<typename _Alloc, bool = __is_empty(_Alloc)>
    struct __alloc_neq
    {
      static bool
      _S_do_it(const _Alloc&, const _Alloc&)
      { return false; }
    };

  template<typename _Alloc>
    struct __alloc_neq<_Alloc, false>
    {
      static bool
      _S_do_it(const _Alloc& __one, const _Alloc& __two)
      { return __one != __two; }
    };


  template<typename _Tp, bool
    = __or_<is_copy_constructible<typename _Tp::value_type>,
            is_nothrow_move_constructible<typename _Tp::value_type>>::value>
    struct __shrink_to_fit_aux
    { static bool _S_do_it(_Tp&) noexcept { return false; } };

  template<typename _Tp>
    struct __shrink_to_fit_aux<_Tp, true>
    {
      static bool
      _S_do_it(_Tp& __c) noexcept
      {

 try
   {
     _Tp(__make_move_if_noexcept_iterator(__c.begin()),
  __make_move_if_noexcept_iterator(__c.end()),
  __c.get_allocator()).swap(__c);
     return true;
   }
 catch(...)
   { return false; }



      }
    };



}
# 42 "/usr/include/c++/11/string" 2 3


# 1 "/usr/include/c++/11/bits/ostream_insert.h" 1 3
# 33 "/usr/include/c++/11/bits/ostream_insert.h" 3
       
# 34 "/usr/include/c++/11/bits/ostream_insert.h" 3


# 1 "/usr/include/c++/11/bits/cxxabi_forced.h" 1 3
# 34 "/usr/include/c++/11/bits/cxxabi_forced.h" 3
       
# 35 "/usr/include/c++/11/bits/cxxabi_forced.h" 3

#pragma GCC visibility push(default)


namespace __cxxabiv1
{







  class __forced_unwind
  {
    virtual ~__forced_unwind() throw();


    virtual void __pure_dummy() = 0;
  };
}


#pragma GCC visibility pop
# 37 "/usr/include/c++/11/bits/ostream_insert.h" 2 3


namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _CharT, typename _Traits>
    inline void
    __ostream_write(basic_ostream<_CharT, _Traits>& __out,
      const _CharT* __s, streamsize __n)
    {
      typedef basic_ostream<_CharT, _Traits> __ostream_type;
      typedef typename __ostream_type::ios_base __ios_base;

      const streamsize __put = __out.rdbuf()->sputn(__s, __n);
      if (__put != __n)
 __out.setstate(__ios_base::badbit);
    }

  template<typename _CharT, typename _Traits>
    inline void
    __ostream_fill(basic_ostream<_CharT, _Traits>& __out, streamsize __n)
    {
      typedef basic_ostream<_CharT, _Traits> __ostream_type;
      typedef typename __ostream_type::ios_base __ios_base;

      const _CharT __c = __out.fill();
      for (; __n > 0; --__n)
 {
   const typename _Traits::int_type __put = __out.rdbuf()->sputc(__c);
   if (_Traits::eq_int_type(__put, _Traits::eof()))
     {
       __out.setstate(__ios_base::badbit);
       break;
     }
 }
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    __ostream_insert(basic_ostream<_CharT, _Traits>& __out,
       const _CharT* __s, streamsize __n)
    {
      typedef basic_ostream<_CharT, _Traits> __ostream_type;
      typedef typename __ostream_type::ios_base __ios_base;

      typename __ostream_type::sentry __cerb(__out);
      if (__cerb)
 {
   try
     {
       const streamsize __w = __out.width();
       if (__w > __n)
  {
    const bool __left = ((__out.flags()
     & __ios_base::adjustfield)
           == __ios_base::left);
    if (!__left)
      __ostream_fill(__out, __w - __n);
    if (__out.good())
      __ostream_write(__out, __s, __n);
    if (__left && __out.good())
      __ostream_fill(__out, __w - __n);
  }
       else
  __ostream_write(__out, __s, __n);
       __out.width(0);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       __out._M_setstate(__ios_base::badbit);
       throw;
     }
   catch(...)
     { __out._M_setstate(__ios_base::badbit); }
 }
      return __out;
    }




  extern template ostream& __ostream_insert(ostream&, const char*, streamsize);


  extern template wostream& __ostream_insert(wostream&, const wchar_t*,
          streamsize);




}
# 45 "/usr/include/c++/11/string" 2 3



# 1 "/usr/include/c++/11/bits/stl_function.h" 1 3
# 63 "/usr/include/c++/11/bits/stl_function.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{

# 116 "/usr/include/c++/11/bits/stl_function.h" 3
  template<typename _Arg, typename _Result>
    struct unary_function
    {

      typedef _Arg argument_type;


      typedef _Result result_type;
    };





  template<typename _Arg1, typename _Arg2, typename _Result>
    struct binary_function
    {

      typedef _Arg1 first_argument_type;


      typedef _Arg2 second_argument_type;


      typedef _Result result_type;
    };
# 157 "/usr/include/c++/11/bits/stl_function.h" 3
  struct __is_transparent;

  template<typename _Tp = void>
    struct plus;

  template<typename _Tp = void>
    struct minus;

  template<typename _Tp = void>
    struct multiplies;

  template<typename _Tp = void>
    struct divides;

  template<typename _Tp = void>
    struct modulus;

  template<typename _Tp = void>
    struct negate;



  template<typename _Tp>
    struct plus : public binary_function<_Tp, _Tp, _Tp>
    {

      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x + __y; }
    };


  template<typename _Tp>
    struct minus : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x - __y; }
    };


  template<typename _Tp>
    struct multiplies : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x * __y; }
    };


  template<typename _Tp>
    struct divides : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x / __y; }
    };


  template<typename _Tp>
    struct modulus : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x % __y; }
    };


  template<typename _Tp>
    struct negate : public unary_function<_Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x) const
      { return -__x; }
    };





  template<>
    struct plus<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) + std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) + std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) + std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct minus<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) - std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) - std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) - std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct multiplies<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) * std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) * std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) * std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct divides<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) / std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) / std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) / std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct modulus<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) % std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) % std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) % std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct negate<void>
    {
      template <typename _Tp>
 constexpr
 auto
 operator()(_Tp&& __t) const
 noexcept(noexcept(-std::forward<_Tp>(__t)))
 -> decltype(-std::forward<_Tp>(__t))
 { return -std::forward<_Tp>(__t); }

      typedef __is_transparent is_transparent;
    };
# 344 "/usr/include/c++/11/bits/stl_function.h" 3
  template<typename _Tp = void>
    struct equal_to;

  template<typename _Tp = void>
    struct not_equal_to;

  template<typename _Tp = void>
    struct greater;

  template<typename _Tp = void>
    struct less;

  template<typename _Tp = void>
    struct greater_equal;

  template<typename _Tp = void>
    struct less_equal;



  template<typename _Tp>
    struct equal_to : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x == __y; }
    };


  template<typename _Tp>
    struct not_equal_to : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x != __y; }
    };


  template<typename _Tp>
    struct greater : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x > __y; }
    };


  template<typename _Tp>
    struct less : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x < __y; }
    };


  template<typename _Tp>
    struct greater_equal : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x >= __y; }
    };


  template<typename _Tp>
    struct less_equal : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x <= __y; }
    };


  template<typename _Tp>
    struct greater<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {


 if (__builtin_is_constant_evaluated())



   return __x > __y;

 return (long unsigned int)__x > (long unsigned int)__y;
      }
    };


  template<typename _Tp>
    struct less<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {


 if (__builtin_is_constant_evaluated())



   return __x < __y;

 return (long unsigned int)__x < (long unsigned int)__y;
      }
    };


  template<typename _Tp>
    struct greater_equal<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {


 if (__builtin_is_constant_evaluated())



   return __x >= __y;

 return (long unsigned int)__x >= (long unsigned int)__y;
      }
    };


  template<typename _Tp>
    struct less_equal<_Tp*> : public binary_function<_Tp*, _Tp*, bool>
    {
      constexpr bool
      operator()(_Tp* __x, _Tp* __y) const noexcept
      {


 if (__builtin_is_constant_evaluated())



   return __x <= __y;

 return (long unsigned int)__x <= (long unsigned int)__y;
      }
    };



  template<>
    struct equal_to<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) == std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) == std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) == std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct not_equal_to<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) != std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) != std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) != std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct greater<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) > std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) > std::forward<_Up>(__u))
 {
   return _S_cmp(std::forward<_Tp>(__t), std::forward<_Up>(__u),
   __ptr_cmp<_Tp, _Up>{});
 }

      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return greater<common_type_t<_Tp*, _Up*>>{}(__t, __u); }

      typedef __is_transparent is_transparent;

    private:
      template <typename _Tp, typename _Up>
 static constexpr decltype(auto)
 _S_cmp(_Tp&& __t, _Up&& __u, false_type)
 { return std::forward<_Tp>(__t) > std::forward<_Up>(__u); }

      template <typename _Tp, typename _Up>
 static constexpr bool
 _S_cmp(_Tp&& __t, _Up&& __u, true_type) noexcept
 {
   return greater<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
 }


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator>(std::declval<_Up>()))>>
 : false_type { };


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator>(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };

      template<typename _Tp, typename _Up>
 using __ptr_cmp = __and_<__not_overloaded<_Tp, _Up>,
       is_convertible<_Tp, const volatile void*>,
       is_convertible<_Up, const volatile void*>>;
    };


  template<>
    struct less<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) < std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) < std::forward<_Up>(__u))
 {
   return _S_cmp(std::forward<_Tp>(__t), std::forward<_Up>(__u),
   __ptr_cmp<_Tp, _Up>{});
 }

      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return less<common_type_t<_Tp*, _Up*>>{}(__t, __u); }

      typedef __is_transparent is_transparent;

    private:
      template <typename _Tp, typename _Up>
 static constexpr decltype(auto)
 _S_cmp(_Tp&& __t, _Up&& __u, false_type)
 { return std::forward<_Tp>(__t) < std::forward<_Up>(__u); }

      template <typename _Tp, typename _Up>
 static constexpr bool
 _S_cmp(_Tp&& __t, _Up&& __u, true_type) noexcept
 {
   return less<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
 }


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator<(std::declval<_Up>()))>>
 : false_type { };


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator<(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };

      template<typename _Tp, typename _Up>
 using __ptr_cmp = __and_<__not_overloaded<_Tp, _Up>,
       is_convertible<_Tp, const volatile void*>,
       is_convertible<_Up, const volatile void*>>;
    };


  template<>
    struct greater_equal<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) >= std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) >= std::forward<_Up>(__u))
 {
   return _S_cmp(std::forward<_Tp>(__t), std::forward<_Up>(__u),
   __ptr_cmp<_Tp, _Up>{});
 }

      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return greater_equal<common_type_t<_Tp*, _Up*>>{}(__t, __u); }

      typedef __is_transparent is_transparent;

    private:
      template <typename _Tp, typename _Up>
 static constexpr decltype(auto)
 _S_cmp(_Tp&& __t, _Up&& __u, false_type)
 { return std::forward<_Tp>(__t) >= std::forward<_Up>(__u); }

      template <typename _Tp, typename _Up>
 static constexpr bool
 _S_cmp(_Tp&& __t, _Up&& __u, true_type) noexcept
 {
   return greater_equal<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
 }


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator>=(std::declval<_Up>()))>>
 : false_type { };


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator>=(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };

      template<typename _Tp, typename _Up>
 using __ptr_cmp = __and_<__not_overloaded<_Tp, _Up>,
       is_convertible<_Tp, const volatile void*>,
       is_convertible<_Up, const volatile void*>>;
    };


  template<>
    struct less_equal<void>
    {
      template <typename _Tp, typename _Up>
 constexpr auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) <= std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) <= std::forward<_Up>(__u))
 {
   return _S_cmp(std::forward<_Tp>(__t), std::forward<_Up>(__u),
   __ptr_cmp<_Tp, _Up>{});
 }

      template<typename _Tp, typename _Up>
 constexpr bool
 operator()(_Tp* __t, _Up* __u) const noexcept
 { return less_equal<common_type_t<_Tp*, _Up*>>{}(__t, __u); }

      typedef __is_transparent is_transparent;

    private:
      template <typename _Tp, typename _Up>
 static constexpr decltype(auto)
 _S_cmp(_Tp&& __t, _Up&& __u, false_type)
 { return std::forward<_Tp>(__t) <= std::forward<_Up>(__u); }

      template <typename _Tp, typename _Up>
 static constexpr bool
 _S_cmp(_Tp&& __t, _Up&& __u, true_type) noexcept
 {
   return less_equal<const volatile void*>{}(
       static_cast<const volatile void*>(std::forward<_Tp>(__t)),
       static_cast<const volatile void*>(std::forward<_Up>(__u)));
 }


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded2 : true_type { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded2<_Tp, _Up, __void_t<
   decltype(std::declval<_Tp>().operator<=(std::declval<_Up>()))>>
 : false_type { };


      template<typename _Tp, typename _Up, typename = void>
 struct __not_overloaded : __not_overloaded2<_Tp, _Up> { };


      template<typename _Tp, typename _Up>
 struct __not_overloaded<_Tp, _Up, __void_t<
   decltype(operator<=(std::declval<_Tp>(), std::declval<_Up>()))>>
 : false_type { };

      template<typename _Tp, typename _Up>
 using __ptr_cmp = __and_<__not_overloaded<_Tp, _Up>,
       is_convertible<_Tp, const volatile void*>,
       is_convertible<_Up, const volatile void*>>;
    };
# 788 "/usr/include/c++/11/bits/stl_function.h" 3
  template<typename _Tp = void>
    struct logical_and;

  template<typename _Tp = void>
    struct logical_or;

  template<typename _Tp = void>
    struct logical_not;



  template<typename _Tp>
    struct logical_and : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x && __y; }
    };


  template<typename _Tp>
    struct logical_or : public binary_function<_Tp, _Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x || __y; }
    };


  template<typename _Tp>
    struct logical_not : public unary_function<_Tp, bool>
    {
      constexpr
      bool
      operator()(const _Tp& __x) const
      { return !__x; }
    };



  template<>
    struct logical_and<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) && std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) && std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) && std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct logical_or<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) || std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) || std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) || std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };


  template<>
    struct logical_not<void>
    {
      template <typename _Tp>
 constexpr
 auto
 operator()(_Tp&& __t) const
 noexcept(noexcept(!std::forward<_Tp>(__t)))
 -> decltype(!std::forward<_Tp>(__t))
 { return !std::forward<_Tp>(__t); }

      typedef __is_transparent is_transparent;
    };




  template<typename _Tp = void>
    struct bit_and;

  template<typename _Tp = void>
    struct bit_or;

  template<typename _Tp = void>
    struct bit_xor;

  template<typename _Tp = void>
    struct bit_not;




  template<typename _Tp>
    struct bit_and : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x & __y; }
    };

  template<typename _Tp>
    struct bit_or : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x | __y; }
    };

  template<typename _Tp>
    struct bit_xor : public binary_function<_Tp, _Tp, _Tp>
    {
      constexpr
      _Tp
      operator()(const _Tp& __x, const _Tp& __y) const
      { return __x ^ __y; }
    };

  template<typename _Tp>
    struct bit_not : public unary_function<_Tp, _Tp>
    {
    constexpr
      _Tp
      operator()(const _Tp& __x) const
      { return ~__x; }
    };


  template <>
    struct bit_and<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) & std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) & std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) & std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };

  template <>
    struct bit_or<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) | std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) | std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) | std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };

  template <>
    struct bit_xor<void>
    {
      template <typename _Tp, typename _Up>
 constexpr
 auto
 operator()(_Tp&& __t, _Up&& __u) const
 noexcept(noexcept(std::forward<_Tp>(__t) ^ std::forward<_Up>(__u)))
 -> decltype(std::forward<_Tp>(__t) ^ std::forward<_Up>(__u))
 { return std::forward<_Tp>(__t) ^ std::forward<_Up>(__u); }

      typedef __is_transparent is_transparent;
    };

  template <>
    struct bit_not<void>
    {
      template <typename _Tp>
 constexpr
 auto
 operator()(_Tp&& __t) const
 noexcept(noexcept(~std::forward<_Tp>(__t)))
 -> decltype(~std::forward<_Tp>(__t))
 { return ~std::forward<_Tp>(__t); }

      typedef __is_transparent is_transparent;
    };
# 1019 "/usr/include/c++/11/bits/stl_function.h" 3
  template<typename _Predicate>
    class unary_negate
    : public unary_function<typename _Predicate::argument_type, bool>
    {
    protected:
      _Predicate _M_pred;

    public:
      constexpr
      explicit
      unary_negate(const _Predicate& __x) : _M_pred(__x) { }

      constexpr
      bool
      operator()(const typename _Predicate::argument_type& __x) const
      { return !_M_pred(__x); }
    };


  template<typename _Predicate>
    constexpr
    inline unary_negate<_Predicate>
    not1(const _Predicate& __pred)
    { return unary_negate<_Predicate>(__pred); }


  template<typename _Predicate>
    class binary_negate
    : public binary_function<typename _Predicate::first_argument_type,
        typename _Predicate::second_argument_type, bool>
    {
    protected:
      _Predicate _M_pred;

    public:
      constexpr
      explicit
      binary_negate(const _Predicate& __x) : _M_pred(__x) { }

      constexpr
      bool
      operator()(const typename _Predicate::first_argument_type& __x,
   const typename _Predicate::second_argument_type& __y) const
      { return !_M_pred(__x, __y); }
    };


  template<typename _Predicate>
    constexpr
    inline binary_negate<_Predicate>
    not2(const _Predicate& __pred)
    { return binary_negate<_Predicate>(__pred); }
# 1098 "/usr/include/c++/11/bits/stl_function.h" 3
  template<typename _Arg, typename _Result>
    class pointer_to_unary_function : public unary_function<_Arg, _Result>
    {
    protected:
      _Result (*_M_ptr)(_Arg);

    public:
      pointer_to_unary_function() { }

      explicit
      pointer_to_unary_function(_Result (*__x)(_Arg))
      : _M_ptr(__x) { }

      _Result
      operator()(_Arg __x) const
      { return _M_ptr(__x); }
    };


  template<typename _Arg, typename _Result>
    inline pointer_to_unary_function<_Arg, _Result>
    ptr_fun(_Result (*__x)(_Arg))
    { return pointer_to_unary_function<_Arg, _Result>(__x); }


  template<typename _Arg1, typename _Arg2, typename _Result>
    class pointer_to_binary_function
    : public binary_function<_Arg1, _Arg2, _Result>
    {
    protected:
      _Result (*_M_ptr)(_Arg1, _Arg2);

    public:
      pointer_to_binary_function() { }

      explicit
      pointer_to_binary_function(_Result (*__x)(_Arg1, _Arg2))
      : _M_ptr(__x) { }

      _Result
      operator()(_Arg1 __x, _Arg2 __y) const
      { return _M_ptr(__x, __y); }
    };


  template<typename _Arg1, typename _Arg2, typename _Result>
    inline pointer_to_binary_function<_Arg1, _Arg2, _Result>
    ptr_fun(_Result (*__x)(_Arg1, _Arg2))
    { return pointer_to_binary_function<_Arg1, _Arg2, _Result>(__x); }


  template<typename _Tp>
    struct _Identity
    : public unary_function<_Tp, _Tp>
    {
      _Tp&
      operator()(_Tp& __x) const
      { return __x; }

      const _Tp&
      operator()(const _Tp& __x) const
      { return __x; }
    };


  template<typename _Tp> struct _Identity<const _Tp> : _Identity<_Tp> { };

  template<typename _Pair>
    struct _Select1st
    : public unary_function<_Pair, typename _Pair::first_type>
    {
      typename _Pair::first_type&
      operator()(_Pair& __x) const
      { return __x.first; }

      const typename _Pair::first_type&
      operator()(const _Pair& __x) const
      { return __x.first; }


      template<typename _Pair2>
        typename _Pair2::first_type&
        operator()(_Pair2& __x) const
        { return __x.first; }

      template<typename _Pair2>
        const typename _Pair2::first_type&
        operator()(const _Pair2& __x) const
        { return __x.first; }

    };

  template<typename _Pair>
    struct _Select2nd
    : public unary_function<_Pair, typename _Pair::second_type>
    {
      typename _Pair::second_type&
      operator()(_Pair& __x) const
      { return __x.second; }

      const typename _Pair::second_type&
      operator()(const _Pair& __x) const
      { return __x.second; }
    };
# 1223 "/usr/include/c++/11/bits/stl_function.h" 3
  template<typename _Ret, typename _Tp>
    class mem_fun_t : public unary_function<_Tp*, _Ret>
    {
    public:
      explicit
      mem_fun_t(_Ret (_Tp::*__pf)())
      : _M_f(__pf) { }

      _Ret
      operator()(_Tp* __p) const
      { return (__p->*_M_f)(); }

    private:
      _Ret (_Tp::*_M_f)();
    };


  template<typename _Ret, typename _Tp>
    class const_mem_fun_t : public unary_function<const _Tp*, _Ret>
    {
    public:
      explicit
      const_mem_fun_t(_Ret (_Tp::*__pf)() const)
      : _M_f(__pf) { }

      _Ret
      operator()(const _Tp* __p) const
      { return (__p->*_M_f)(); }

    private:
      _Ret (_Tp::*_M_f)() const;
    };


  template<typename _Ret, typename _Tp>
    class mem_fun_ref_t : public unary_function<_Tp, _Ret>
    {
    public:
      explicit
      mem_fun_ref_t(_Ret (_Tp::*__pf)())
      : _M_f(__pf) { }

      _Ret
      operator()(_Tp& __r) const
      { return (__r.*_M_f)(); }

    private:
      _Ret (_Tp::*_M_f)();
  };


  template<typename _Ret, typename _Tp>
    class const_mem_fun_ref_t : public unary_function<_Tp, _Ret>
    {
    public:
      explicit
      const_mem_fun_ref_t(_Ret (_Tp::*__pf)() const)
      : _M_f(__pf) { }

      _Ret
      operator()(const _Tp& __r) const
      { return (__r.*_M_f)(); }

    private:
      _Ret (_Tp::*_M_f)() const;
    };


  template<typename _Ret, typename _Tp, typename _Arg>
    class mem_fun1_t : public binary_function<_Tp*, _Arg, _Ret>
    {
    public:
      explicit
      mem_fun1_t(_Ret (_Tp::*__pf)(_Arg))
      : _M_f(__pf) { }

      _Ret
      operator()(_Tp* __p, _Arg __x) const
      { return (__p->*_M_f)(__x); }

    private:
      _Ret (_Tp::*_M_f)(_Arg);
    };


  template<typename _Ret, typename _Tp, typename _Arg>
    class const_mem_fun1_t : public binary_function<const _Tp*, _Arg, _Ret>
    {
    public:
      explicit
      const_mem_fun1_t(_Ret (_Tp::*__pf)(_Arg) const)
      : _M_f(__pf) { }

      _Ret
      operator()(const _Tp* __p, _Arg __x) const
      { return (__p->*_M_f)(__x); }

    private:
      _Ret (_Tp::*_M_f)(_Arg) const;
    };


  template<typename _Ret, typename _Tp, typename _Arg>
    class mem_fun1_ref_t : public binary_function<_Tp, _Arg, _Ret>
    {
    public:
      explicit
      mem_fun1_ref_t(_Ret (_Tp::*__pf)(_Arg))
      : _M_f(__pf) { }

      _Ret
      operator()(_Tp& __r, _Arg __x) const
      { return (__r.*_M_f)(__x); }

    private:
      _Ret (_Tp::*_M_f)(_Arg);
    };


  template<typename _Ret, typename _Tp, typename _Arg>
    class const_mem_fun1_ref_t : public binary_function<_Tp, _Arg, _Ret>
    {
    public:
      explicit
      const_mem_fun1_ref_t(_Ret (_Tp::*__pf)(_Arg) const)
      : _M_f(__pf) { }

      _Ret
      operator()(const _Tp& __r, _Arg __x) const
      { return (__r.*_M_f)(__x); }

    private:
      _Ret (_Tp::*_M_f)(_Arg) const;
    };



  template<typename _Ret, typename _Tp>
    inline mem_fun_t<_Ret, _Tp>
    mem_fun(_Ret (_Tp::*__f)())
    { return mem_fun_t<_Ret, _Tp>(__f); }

  template<typename _Ret, typename _Tp>
    inline const_mem_fun_t<_Ret, _Tp>
    mem_fun(_Ret (_Tp::*__f)() const)
    { return const_mem_fun_t<_Ret, _Tp>(__f); }

  template<typename _Ret, typename _Tp>
    inline mem_fun_ref_t<_Ret, _Tp>
    mem_fun_ref(_Ret (_Tp::*__f)())
    { return mem_fun_ref_t<_Ret, _Tp>(__f); }

  template<typename _Ret, typename _Tp>
    inline const_mem_fun_ref_t<_Ret, _Tp>
    mem_fun_ref(_Ret (_Tp::*__f)() const)
    { return const_mem_fun_ref_t<_Ret, _Tp>(__f); }

  template<typename _Ret, typename _Tp, typename _Arg>
    inline mem_fun1_t<_Ret, _Tp, _Arg>
    mem_fun(_Ret (_Tp::*__f)(_Arg))
    { return mem_fun1_t<_Ret, _Tp, _Arg>(__f); }

  template<typename _Ret, typename _Tp, typename _Arg>
    inline const_mem_fun1_t<_Ret, _Tp, _Arg>
    mem_fun(_Ret (_Tp::*__f)(_Arg) const)
    { return const_mem_fun1_t<_Ret, _Tp, _Arg>(__f); }

  template<typename _Ret, typename _Tp, typename _Arg>
    inline mem_fun1_ref_t<_Ret, _Tp, _Arg>
    mem_fun_ref(_Ret (_Tp::*__f)(_Arg))
    { return mem_fun1_ref_t<_Ret, _Tp, _Arg>(__f); }

  template<typename _Ret, typename _Tp, typename _Arg>
    inline const_mem_fun1_ref_t<_Ret, _Tp, _Arg>
    mem_fun_ref(_Ret (_Tp::*__f)(_Arg) const)
    { return const_mem_fun1_ref_t<_Ret, _Tp, _Arg>(__f); }




  template<typename _Func, typename _SfinaeType, typename = __void_t<>>
    struct __has_is_transparent
    { };

  template<typename _Func, typename _SfinaeType>
    struct __has_is_transparent<_Func, _SfinaeType,
    __void_t<typename _Func::is_transparent>>
    { typedef void type; };

  template<typename _Func, typename _SfinaeType>
    using __has_is_transparent_t
      = typename __has_is_transparent<_Func, _SfinaeType>::type;



}


# 1 "/usr/include/c++/11/backward/binders.h" 1 3
# 60 "/usr/include/c++/11/backward/binders.h" 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace std __attribute__ ((__visibility__ ("default")))
{

# 107 "/usr/include/c++/11/backward/binders.h" 3
  template<typename _Operation>
    class binder1st
    : public unary_function<typename _Operation::second_argument_type,
       typename _Operation::result_type>
    {
    protected:
      _Operation op;
      typename _Operation::first_argument_type value;

    public:
      binder1st(const _Operation& __x,
  const typename _Operation::first_argument_type& __y)
      : op(__x), value(__y) { }

      typename _Operation::result_type
      operator()(const typename _Operation::second_argument_type& __x) const
      { return op(value, __x); }



      typename _Operation::result_type
      operator()(typename _Operation::second_argument_type& __x) const
      { return op(value, __x); }
    } __attribute__ ((__deprecated__ ("use '" "std::bind" "' instead")));


  template<typename _Operation, typename _Tp>
    inline binder1st<_Operation>
    bind1st(const _Operation& __fn, const _Tp& __x)
    {
      typedef typename _Operation::first_argument_type _Arg1_type;
      return binder1st<_Operation>(__fn, _Arg1_type(__x));
    }


  template<typename _Operation>
    class binder2nd
    : public unary_function<typename _Operation::first_argument_type,
       typename _Operation::result_type>
    {
    protected:
      _Operation op;
      typename _Operation::second_argument_type value;

    public:
      binder2nd(const _Operation& __x,
  const typename _Operation::second_argument_type& __y)
      : op(__x), value(__y) { }

      typename _Operation::result_type
      operator()(const typename _Operation::first_argument_type& __x) const
      { return op(__x, value); }



      typename _Operation::result_type
      operator()(typename _Operation::first_argument_type& __x) const
      { return op(__x, value); }
    } __attribute__ ((__deprecated__ ("use '" "std::bind" "' instead")));


  template<typename _Operation, typename _Tp>
    inline binder2nd<_Operation>
    bind2nd(const _Operation& __fn, const _Tp& __x)
    {
      typedef typename _Operation::second_argument_type _Arg2_type;
      return binder2nd<_Operation>(__fn, _Arg2_type(__x));
    }



}

#pragma GCC diagnostic pop
# 1422 "/usr/include/c++/11/bits/stl_function.h" 2 3
# 49 "/usr/include/c++/11/string" 2 3





# 1 "/usr/include/c++/11/bits/range_access.h" 1 3
# 33 "/usr/include/c++/11/bits/range_access.h" 3
       
# 34 "/usr/include/c++/11/bits/range_access.h" 3


# 1 "/usr/include/c++/11/initializer_list" 1 3
# 33 "/usr/include/c++/11/initializer_list" 3
       
# 34 "/usr/include/c++/11/initializer_list" 3





#pragma GCC visibility push(default)



namespace std
{

  template<class _E>
    class initializer_list
    {
    public:
      typedef _E value_type;
      typedef const _E& reference;
      typedef const _E& const_reference;
      typedef size_t size_type;
      typedef const _E* iterator;
      typedef const _E* const_iterator;

    private:
      iterator _M_array;
      size_type _M_len;


      constexpr initializer_list(const_iterator __a, size_type __l)
      : _M_array(__a), _M_len(__l) { }

    public:
      constexpr initializer_list() noexcept
      : _M_array(0), _M_len(0) { }


      constexpr size_type
      size() const noexcept { return _M_len; }


      constexpr const_iterator
      begin() const noexcept { return _M_array; }


      constexpr const_iterator
      end() const noexcept { return begin() + size(); }
    };







  template<class _Tp>
    constexpr const _Tp*
    begin(initializer_list<_Tp> __ils) noexcept
    { return __ils.begin(); }







  template<class _Tp>
    constexpr const _Tp*
    end(initializer_list<_Tp> __ils) noexcept
    { return __ils.end(); }
}

#pragma GCC visibility pop
# 37 "/usr/include/c++/11/bits/range_access.h" 2 3



namespace std __attribute__ ((__visibility__ ("default")))
{







  template<typename _Container>
    inline constexpr auto
    begin(_Container& __cont) -> decltype(__cont.begin())
    { return __cont.begin(); }






  template<typename _Container>
    inline constexpr auto
    begin(const _Container& __cont) -> decltype(__cont.begin())
    { return __cont.begin(); }






  template<typename _Container>
    inline constexpr auto
    end(_Container& __cont) -> decltype(__cont.end())
    { return __cont.end(); }






  template<typename _Container>
    inline constexpr auto
    end(const _Container& __cont) -> decltype(__cont.end())
    { return __cont.end(); }





  template<typename _Tp, size_t _Nm>
    inline constexpr _Tp*
    begin(_Tp (&__arr)[_Nm]) noexcept
    { return __arr; }






  template<typename _Tp, size_t _Nm>
    inline constexpr _Tp*
    end(_Tp (&__arr)[_Nm]) noexcept
    { return __arr + _Nm; }



  template<typename _Tp> class valarray;

  template<typename _Tp> _Tp* begin(valarray<_Tp>&) noexcept;
  template<typename _Tp> const _Tp* begin(const valarray<_Tp>&) noexcept;
  template<typename _Tp> _Tp* end(valarray<_Tp>&) noexcept;
  template<typename _Tp> const _Tp* end(const valarray<_Tp>&) noexcept;






  template<typename _Container>
    inline constexpr auto
    cbegin(const _Container& __cont) noexcept(noexcept(std::begin(__cont)))
      -> decltype(std::begin(__cont))
    { return std::begin(__cont); }






  template<typename _Container>
    inline constexpr auto
    cend(const _Container& __cont) noexcept(noexcept(std::end(__cont)))
      -> decltype(std::end(__cont))
    { return std::end(__cont); }






  template<typename _Container>
    inline constexpr auto
    rbegin(_Container& __cont) -> decltype(__cont.rbegin())
    { return __cont.rbegin(); }






  template<typename _Container>
    inline constexpr auto
    rbegin(const _Container& __cont) -> decltype(__cont.rbegin())
    { return __cont.rbegin(); }






  template<typename _Container>
    inline constexpr auto
    rend(_Container& __cont) -> decltype(__cont.rend())
    { return __cont.rend(); }






  template<typename _Container>
    inline constexpr auto
    rend(const _Container& __cont) -> decltype(__cont.rend())
    { return __cont.rend(); }






  template<typename _Tp, size_t _Nm>
    inline constexpr reverse_iterator<_Tp*>
    rbegin(_Tp (&__arr)[_Nm]) noexcept
    { return reverse_iterator<_Tp*>(__arr + _Nm); }






  template<typename _Tp, size_t _Nm>
    inline constexpr reverse_iterator<_Tp*>
    rend(_Tp (&__arr)[_Nm]) noexcept
    { return reverse_iterator<_Tp*>(__arr); }






  template<typename _Tp>
    inline constexpr reverse_iterator<const _Tp*>
    rbegin(initializer_list<_Tp> __il) noexcept
    { return reverse_iterator<const _Tp*>(__il.end()); }






  template<typename _Tp>
    inline constexpr reverse_iterator<const _Tp*>
    rend(initializer_list<_Tp> __il) noexcept
    { return reverse_iterator<const _Tp*>(__il.begin()); }






  template<typename _Container>
    inline constexpr auto
    crbegin(const _Container& __cont) -> decltype(std::rbegin(__cont))
    { return std::rbegin(__cont); }






  template<typename _Container>
    inline constexpr auto
    crend(const _Container& __cont) -> decltype(std::rend(__cont))
    { return std::rend(__cont); }
# 243 "/usr/include/c++/11/bits/range_access.h" 3
  template <typename _Container>
    constexpr auto
    size(const _Container& __cont) noexcept(noexcept(__cont.size()))
    -> decltype(__cont.size())
    { return __cont.size(); }




  template <typename _Tp, size_t _Nm>
    constexpr size_t
    size(const _Tp (&)[_Nm]) noexcept
    { return _Nm; }





  template <typename _Container>
    [[nodiscard]] constexpr auto
    empty(const _Container& __cont) noexcept(noexcept(__cont.empty()))
    -> decltype(__cont.empty())
    { return __cont.empty(); }




  template <typename _Tp, size_t _Nm>
    [[nodiscard]] constexpr bool
    empty(const _Tp (&)[_Nm]) noexcept
    { return false; }





  template <typename _Tp>
    [[nodiscard]] constexpr bool
    empty(initializer_list<_Tp> __il) noexcept
    { return __il.size() == 0;}





  template <typename _Container>
    constexpr auto
    data(_Container& __cont) noexcept(noexcept(__cont.data()))
    -> decltype(__cont.data())
    { return __cont.data(); }





  template <typename _Container>
    constexpr auto
    data(const _Container& __cont) noexcept(noexcept(__cont.data()))
    -> decltype(__cont.data())
    { return __cont.data(); }





  template <typename _Tp, size_t _Nm>
    constexpr _Tp*
    data(_Tp (&__array)[_Nm]) noexcept
    { return __array; }





  template <typename _Tp>
    constexpr const _Tp*
    data(initializer_list<_Tp> __il) noexcept
    { return __il.begin(); }
# 341 "/usr/include/c++/11/bits/range_access.h" 3

}
# 55 "/usr/include/c++/11/string" 2 3
# 1 "/usr/include/c++/11/bits/basic_string.h" 1 3
# 37 "/usr/include/c++/11/bits/basic_string.h" 3
       
# 38 "/usr/include/c++/11/bits/basic_string.h" 3


# 1 "/usr/include/c++/11/ext/alloc_traits.h" 1 3
# 32 "/usr/include/c++/11/ext/alloc_traits.h" 3
       
# 33 "/usr/include/c++/11/ext/alloc_traits.h" 3

# 1 "/usr/include/c++/11/bits/alloc_traits.h" 1 3
# 33 "/usr/include/c++/11/bits/alloc_traits.h" 3
# 1 "/usr/include/c++/11/bits/stl_construct.h" 1 3
# 73 "/usr/include/c++/11/bits/stl_construct.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{



  template <typename _Tp>
    inline void
    destroy_at(_Tp* __location)
    {
      if constexpr (201703L > 201703L && is_array_v<_Tp>)
 {
   for (auto& __x : *__location)
     std::destroy_at(std::__addressof(__x));
 }
      else
 __location->~_Tp();
    }
# 106 "/usr/include/c++/11/bits/stl_construct.h" 3
  template<typename _Tp, typename... _Args>
   
    inline void
    _Construct(_Tp* __p, _Args&&... __args)
    {
# 119 "/usr/include/c++/11/bits/stl_construct.h" 3
      ::new((void*)__p) _Tp(std::forward<_Args>(__args)...);
    }
# 132 "/usr/include/c++/11/bits/stl_construct.h" 3
  template<typename _T1>
    inline void
    _Construct_novalue(_T1* __p)
    { ::new((void*)__p) _T1; }

  template<typename _ForwardIterator>
    void
    _Destroy(_ForwardIterator __first, _ForwardIterator __last);




  template<typename _Tp>
    constexpr inline void
    _Destroy(_Tp* __pointer)
    {



      __pointer->~_Tp();

    }

  template<bool>
    struct _Destroy_aux
    {
      template<typename _ForwardIterator>
 static void
 __destroy(_ForwardIterator __first, _ForwardIterator __last)
 {
   for (; __first != __last; ++__first)
     std::_Destroy(std::__addressof(*__first));
 }
    };

  template<>
    struct _Destroy_aux<true>
    {
      template<typename _ForwardIterator>
        static void
        __destroy(_ForwardIterator, _ForwardIterator) { }
    };






  template<typename _ForwardIterator>
    inline void
    _Destroy(_ForwardIterator __first, _ForwardIterator __last)
    {
      typedef typename iterator_traits<_ForwardIterator>::value_type
                       _Value_type;


      static_assert(is_destructible<_Value_type>::value,
      "value type is destructible");





      std::_Destroy_aux<__has_trivial_destructor(_Value_type)>::
 __destroy(__first, __last);
    }

  template<bool>
    struct _Destroy_n_aux
    {
      template<typename _ForwardIterator, typename _Size>
 static _ForwardIterator
 __destroy_n(_ForwardIterator __first, _Size __count)
 {
   for (; __count > 0; (void)++__first, --__count)
     std::_Destroy(std::__addressof(*__first));
   return __first;
 }
    };

  template<>
    struct _Destroy_n_aux<true>
    {
      template<typename _ForwardIterator, typename _Size>
        static _ForwardIterator
        __destroy_n(_ForwardIterator __first, _Size __count)
 {
   std::advance(__first, __count);
   return __first;
 }
    };






  template<typename _ForwardIterator, typename _Size>
    inline _ForwardIterator
    _Destroy_n(_ForwardIterator __first, _Size __count)
    {
      typedef typename iterator_traits<_ForwardIterator>::value_type
                       _Value_type;


      static_assert(is_destructible<_Value_type>::value,
      "value type is destructible");





      return std::_Destroy_n_aux<__has_trivial_destructor(_Value_type)>::
 __destroy_n(__first, __count);
    }


  template <typename _ForwardIterator>
    inline void
    destroy(_ForwardIterator __first, _ForwardIterator __last)
    {
      std::_Destroy(__first, __last);
    }

  template <typename _ForwardIterator, typename _Size>
    inline _ForwardIterator
    destroy_n(_ForwardIterator __first, _Size __count)
    {
      return std::_Destroy_n(__first, __count);
    }



}
# 34 "/usr/include/c++/11/bits/alloc_traits.h" 2 3







namespace std __attribute__ ((__visibility__ ("default")))
{






  struct __allocator_traits_base
  {
    template<typename _Tp, typename _Up, typename = void>
      struct __rebind : __replace_first_arg<_Tp, _Up> { };

    template<typename _Tp, typename _Up>
      struct __rebind<_Tp, _Up,
        __void_t<typename _Tp::template rebind<_Up>::other>>
      { using type = typename _Tp::template rebind<_Up>::other; };

  protected:
    template<typename _Tp>
      using __pointer = typename _Tp::pointer;
    template<typename _Tp>
      using __c_pointer = typename _Tp::const_pointer;
    template<typename _Tp>
      using __v_pointer = typename _Tp::void_pointer;
    template<typename _Tp>
      using __cv_pointer = typename _Tp::const_void_pointer;
    template<typename _Tp>
      using __pocca = typename _Tp::propagate_on_container_copy_assignment;
    template<typename _Tp>
      using __pocma = typename _Tp::propagate_on_container_move_assignment;
    template<typename _Tp>
      using __pocs = typename _Tp::propagate_on_container_swap;
    template<typename _Tp>
      using __equal = typename _Tp::is_always_equal;
  };

  template<typename _Alloc, typename _Up>
    using __alloc_rebind
      = typename __allocator_traits_base::template __rebind<_Alloc, _Up>::type;
# 89 "/usr/include/c++/11/bits/alloc_traits.h" 3
  template<typename _Alloc>
    struct allocator_traits : __allocator_traits_base
    {

      typedef _Alloc allocator_type;

      typedef typename _Alloc::value_type value_type;






      using pointer = __detected_or_t<value_type*, __pointer, _Alloc>;

    private:

      template<template<typename> class _Func, typename _Tp, typename = void>
 struct _Ptr
 {
   using type = typename pointer_traits<pointer>::template rebind<_Tp>;
 };

      template<template<typename> class _Func, typename _Tp>
 struct _Ptr<_Func, _Tp, __void_t<_Func<_Alloc>>>
 {
   using type = _Func<_Alloc>;
 };


      template<typename _A2, typename _PtrT, typename = void>
 struct _Diff
 { using type = typename pointer_traits<_PtrT>::difference_type; };

      template<typename _A2, typename _PtrT>
 struct _Diff<_A2, _PtrT, __void_t<typename _A2::difference_type>>
 { using type = typename _A2::difference_type; };


      template<typename _A2, typename _DiffT, typename = void>
 struct _Size : make_unsigned<_DiffT> { };

      template<typename _A2, typename _DiffT>
 struct _Size<_A2, _DiffT, __void_t<typename _A2::size_type>>
 { using type = typename _A2::size_type; };

    public:






      using const_pointer = typename _Ptr<__c_pointer, const value_type>::type;







      using void_pointer = typename _Ptr<__v_pointer, void>::type;







      using const_void_pointer = typename _Ptr<__cv_pointer, const void>::type;







      using difference_type = typename _Diff<_Alloc, pointer>::type;







      using size_type = typename _Size<_Alloc, difference_type>::type;







      using propagate_on_container_copy_assignment
 = __detected_or_t<false_type, __pocca, _Alloc>;







      using propagate_on_container_move_assignment
 = __detected_or_t<false_type, __pocma, _Alloc>;







      using propagate_on_container_swap
 = __detected_or_t<false_type, __pocs, _Alloc>;







      using is_always_equal
 = __detected_or_t<typename is_empty<_Alloc>::type, __equal, _Alloc>;

      template<typename _Tp>
 using rebind_alloc = __alloc_rebind<_Alloc, _Tp>;
      template<typename _Tp>
 using rebind_traits = allocator_traits<rebind_alloc<_Tp>>;

    private:
      template<typename _Alloc2>
 static constexpr auto
 _S_allocate(_Alloc2& __a, size_type __n, const_void_pointer __hint, int)
 -> decltype(__a.allocate(__n, __hint))
 { return __a.allocate(__n, __hint); }

      template<typename _Alloc2>
 static constexpr pointer
 _S_allocate(_Alloc2& __a, size_type __n, const_void_pointer, ...)
 { return __a.allocate(__n); }

      template<typename _Tp, typename... _Args>
 struct __construct_helper
 {
   template<typename _Alloc2,
     typename = decltype(std::declval<_Alloc2*>()->construct(
    std::declval<_Tp*>(), std::declval<_Args>()...))>
     static true_type __test(int);

   template<typename>
     static false_type __test(...);

   using type = decltype(__test<_Alloc>(0));
 };

      template<typename _Tp, typename... _Args>
 using __has_construct
   = typename __construct_helper<_Tp, _Args...>::type;

      template<typename _Tp, typename... _Args>
 static constexpr _Require<__has_construct<_Tp, _Args...>>
 _S_construct(_Alloc& __a, _Tp* __p, _Args&&... __args)
 noexcept(noexcept(__a.construct(__p, std::forward<_Args>(__args)...)))
 { __a.construct(__p, std::forward<_Args>(__args)...); }

      template<typename _Tp, typename... _Args>
 static constexpr
 _Require<__and_<__not_<__has_construct<_Tp, _Args...>>,
          is_constructible<_Tp, _Args...>>>
 _S_construct(_Alloc&, _Tp* __p, _Args&&... __args)
 noexcept(std::is_nothrow_constructible<_Tp, _Args...>::value)
 {

   ::new((void*)__p) _Tp(std::forward<_Args>(__args)...);



 }

      template<typename _Alloc2, typename _Tp>
 static constexpr auto
 _S_destroy(_Alloc2& __a, _Tp* __p, int)
 noexcept(noexcept(__a.destroy(__p)))
 -> decltype(__a.destroy(__p))
 { __a.destroy(__p); }

      template<typename _Alloc2, typename _Tp>
 static constexpr void
 _S_destroy(_Alloc2&, _Tp* __p, ...)
 noexcept(std::is_nothrow_destructible<_Tp>::value)
 { std::_Destroy(__p); }

      template<typename _Alloc2>
 static constexpr auto
 _S_max_size(_Alloc2& __a, int)
 -> decltype(__a.max_size())
 { return __a.max_size(); }

      template<typename _Alloc2>
 static constexpr size_type
 _S_max_size(_Alloc2&, ...)
 {


   return __gnu_cxx::__numeric_traits<size_type>::__max
     / sizeof(value_type);
 }

      template<typename _Alloc2>
 static constexpr auto
 _S_select(_Alloc2& __a, int)
 -> decltype(__a.select_on_container_copy_construction())
 { return __a.select_on_container_copy_construction(); }

      template<typename _Alloc2>
 static constexpr _Alloc2
 _S_select(_Alloc2& __a, ...)
 { return __a; }

    public:
# 316 "/usr/include/c++/11/bits/alloc_traits.h" 3
      [[__nodiscard__]] static pointer
      allocate(_Alloc& __a, size_type __n)
      { return __a.allocate(__n); }
# 331 "/usr/include/c++/11/bits/alloc_traits.h" 3
      [[__nodiscard__]] static pointer
      allocate(_Alloc& __a, size_type __n, const_void_pointer __hint)
      { return _S_allocate(__a, __n, __hint, 0); }
# 343 "/usr/include/c++/11/bits/alloc_traits.h" 3
      static void
      deallocate(_Alloc& __a, pointer __p, size_type __n)
      { __a.deallocate(__p, __n); }
# 358 "/usr/include/c++/11/bits/alloc_traits.h" 3
      template<typename _Tp, typename... _Args>
 static auto
 construct(_Alloc& __a, _Tp* __p, _Args&&... __args)
 noexcept(noexcept(_S_construct(__a, __p,
           std::forward<_Args>(__args)...)))
 -> decltype(_S_construct(__a, __p, std::forward<_Args>(__args)...))
 { _S_construct(__a, __p, std::forward<_Args>(__args)...); }
# 374 "/usr/include/c++/11/bits/alloc_traits.h" 3
      template<typename _Tp>
 static void
 destroy(_Alloc& __a, _Tp* __p)
 noexcept(noexcept(_S_destroy(__a, __p, 0)))
 { _S_destroy(__a, __p, 0); }
# 388 "/usr/include/c++/11/bits/alloc_traits.h" 3
      static size_type
      max_size(const _Alloc& __a) noexcept
      { return _S_max_size(__a, 0); }
# 400 "/usr/include/c++/11/bits/alloc_traits.h" 3
      static _Alloc
      select_on_container_copy_construction(const _Alloc& __rhs)
      { return _S_select(__rhs, 0); }
    };






  template<typename _Tp>
    struct allocator_traits<allocator<_Tp>>
    {

      using allocator_type = allocator<_Tp>;


      using value_type = _Tp;


      using pointer = _Tp*;


      using const_pointer = const _Tp*;


      using void_pointer = void*;


      using const_void_pointer = const void*;


      using difference_type = std::ptrdiff_t;


      using size_type = std::size_t;


      using propagate_on_container_copy_assignment = false_type;


      using propagate_on_container_move_assignment = true_type;


      using propagate_on_container_swap = false_type;


      using is_always_equal = true_type;

      template<typename _Up>
 using rebind_alloc = allocator<_Up>;

      template<typename _Up>
 using rebind_traits = allocator_traits<allocator<_Up>>;
# 462 "/usr/include/c++/11/bits/alloc_traits.h" 3
      [[__nodiscard__]] static pointer
      allocate(allocator_type& __a, size_type __n)
      { return __a.allocate(__n); }
# 476 "/usr/include/c++/11/bits/alloc_traits.h" 3
      [[__nodiscard__]] static pointer
      allocate(allocator_type& __a, size_type __n, const_void_pointer __hint)
      {

 return __a.allocate(__n, __hint);



      }
# 494 "/usr/include/c++/11/bits/alloc_traits.h" 3
      static void
      deallocate(allocator_type& __a, pointer __p, size_type __n)
      { __a.deallocate(__p, __n); }
# 509 "/usr/include/c++/11/bits/alloc_traits.h" 3
      template<typename _Up, typename... _Args>
 static void
 construct(allocator_type& __a __attribute__((__unused__)), _Up* __p,
    _Args&&... __args)
 noexcept(std::is_nothrow_constructible<_Up, _Args...>::value)
 {

   __a.construct(__p, std::forward<_Args>(__args)...);



 }
# 529 "/usr/include/c++/11/bits/alloc_traits.h" 3
      template<typename _Up>
 static void
 destroy(allocator_type& __a __attribute__((__unused__)), _Up* __p)
 noexcept(is_nothrow_destructible<_Up>::value)
 {

   __a.destroy(__p);



 }






      static size_type
      max_size(const allocator_type& __a __attribute__((__unused__))) noexcept
      {

 return __a.max_size();



      }






      static allocator_type
      select_on_container_copy_construction(const allocator_type& __rhs)
      { return __rhs; }
    };


  template<>
    struct allocator_traits<allocator<void>>
    {

      using allocator_type = allocator<void>;


      using value_type = void;


      using pointer = void*;


      using const_pointer = const void*;


      using void_pointer = void*;


      using const_void_pointer = const void*;


      using difference_type = std::ptrdiff_t;


      using size_type = std::size_t;


      using propagate_on_container_copy_assignment = false_type;


      using propagate_on_container_move_assignment = true_type;


      using propagate_on_container_swap = false_type;


      using is_always_equal = true_type;

      template<typename _Up>
 using rebind_alloc = allocator<_Up>;

      template<typename _Up>
 using rebind_traits = allocator_traits<allocator<_Up>>;


      static void*
      allocate(allocator_type&, size_type, const void* = nullptr) = delete;


      static void
      deallocate(allocator_type&, void*, size_type) = delete;
# 631 "/usr/include/c++/11/bits/alloc_traits.h" 3
      template<typename _Up, typename... _Args>
 static void
 construct(allocator_type&, _Up* __p, _Args&&... __args)
 noexcept(std::is_nothrow_constructible<_Up, _Args...>::value)
 { std::_Construct(__p, std::forward<_Args>(__args)...); }
# 644 "/usr/include/c++/11/bits/alloc_traits.h" 3
      template<typename _Up>
 static void
 destroy(allocator_type&, _Up* __p)
 noexcept(is_nothrow_destructible<_Up>::value)
 { std::_Destroy(__p); }


      static size_type
      max_size(const allocator_type&) = delete;






      static allocator_type
      select_on_container_copy_construction(const allocator_type& __rhs)
      { return __rhs; }
    };
# 676 "/usr/include/c++/11/bits/alloc_traits.h" 3
  template<typename _Alloc>
    constexpr inline void
    __alloc_on_copy(_Alloc& __one, const _Alloc& __two)
    {
      typedef allocator_traits<_Alloc> __traits;
      typedef typename __traits::propagate_on_container_copy_assignment __pocca;

      if constexpr (__pocca::value)
 __one = __two;



    }

  template<typename _Alloc>
    constexpr _Alloc
    __alloc_on_copy(const _Alloc& __a)
    {
      typedef allocator_traits<_Alloc> __traits;
      return __traits::select_on_container_copy_construction(__a);
    }
# 708 "/usr/include/c++/11/bits/alloc_traits.h" 3
  template<typename _Alloc>
    constexpr inline void
    __alloc_on_move(_Alloc& __one, _Alloc& __two)
    {
      typedef allocator_traits<_Alloc> __traits;
      typedef typename __traits::propagate_on_container_move_assignment __pocma;

      if constexpr (__pocma::value)
 __one = std::move(__two);



    }
# 735 "/usr/include/c++/11/bits/alloc_traits.h" 3
  template<typename _Alloc>
    constexpr inline void
    __alloc_on_swap(_Alloc& __one, _Alloc& __two)
    {
      typedef allocator_traits<_Alloc> __traits;
      typedef typename __traits::propagate_on_container_swap __pocs;

      if constexpr (__pocs::value)
 {
   using std::swap;
   swap(__one, __two);
 }



    }

  template<typename _Alloc, typename _Tp,
    typename _ValueT = __remove_cvref_t<typename _Alloc::value_type>,
    typename = void>
    struct __is_alloc_insertable_impl
    : false_type
    { };

  template<typename _Alloc, typename _Tp, typename _ValueT>
    struct __is_alloc_insertable_impl<_Alloc, _Tp, _ValueT,
      __void_t<decltype(allocator_traits<_Alloc>::construct(
     std::declval<_Alloc&>(), std::declval<_ValueT*>(),
     std::declval<_Tp>()))>>
    : true_type
    { };




  template<typename _Alloc>
    struct __is_copy_insertable
    : __is_alloc_insertable_impl<_Alloc,
     typename _Alloc::value_type const&>::type
    { };


  template<typename _Tp>
    struct __is_copy_insertable<allocator<_Tp>>
    : is_copy_constructible<_Tp>
    { };




  template<typename _Alloc>
    struct __is_move_insertable
    : __is_alloc_insertable_impl<_Alloc, typename _Alloc::value_type>::type
    { };


  template<typename _Tp>
    struct __is_move_insertable<allocator<_Tp>>
    : is_move_constructible<_Tp>
    { };


  template<typename _Alloc, typename = void>
    struct __is_allocator : false_type { };

  template<typename _Alloc>
    struct __is_allocator<_Alloc,
      __void_t<typename _Alloc::value_type,
        decltype(std::declval<_Alloc&>().allocate(size_t{}))>>
    : true_type { };

  template<typename _Alloc>
    using _RequireAllocator
      = typename enable_if<__is_allocator<_Alloc>::value, _Alloc>::type;

  template<typename _Alloc>
    using _RequireNotAllocator
      = typename enable_if<!__is_allocator<_Alloc>::value, _Alloc>::type;
# 829 "/usr/include/c++/11/bits/alloc_traits.h" 3
  template<typename _ForwardIterator, typename _Allocator>
    void
    _Destroy(_ForwardIterator __first, _ForwardIterator __last,
      _Allocator& __alloc)
    {
      for (; __first != __last; ++__first)



 allocator_traits<_Allocator>::destroy(__alloc,
           std::__addressof(*__first));

    }

  template<typename _ForwardIterator, typename _Tp>
    inline void
    _Destroy(_ForwardIterator __first, _ForwardIterator __last,
      allocator<_Tp>&)
    {
      _Destroy(__first, __last);
    }


}
# 35 "/usr/include/c++/11/ext/alloc_traits.h" 2 3




namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{






template<typename _Alloc, typename = typename _Alloc::value_type>
  struct __alloc_traits

  : std::allocator_traits<_Alloc>

  {
    typedef _Alloc allocator_type;

    typedef std::allocator_traits<_Alloc> _Base_type;
    typedef typename _Base_type::value_type value_type;
    typedef typename _Base_type::pointer pointer;
    typedef typename _Base_type::const_pointer const_pointer;
    typedef typename _Base_type::size_type size_type;
    typedef typename _Base_type::difference_type difference_type;

    typedef value_type& reference;
    typedef const value_type& const_reference;
    using _Base_type::allocate;
    using _Base_type::deallocate;
    using _Base_type::construct;
    using _Base_type::destroy;
    using _Base_type::max_size;

  private:
    template<typename _Ptr>
      using __is_custom_pointer
 = std::__and_<std::is_same<pointer, _Ptr>,
        std::__not_<std::is_pointer<_Ptr>>>;

  public:

    template<typename _Ptr, typename... _Args>
      static constexpr
      std::__enable_if_t<__is_custom_pointer<_Ptr>::value>
      construct(_Alloc& __a, _Ptr __p, _Args&&... __args)
      noexcept(noexcept(_Base_type::construct(__a, std::__to_address(__p),
           std::forward<_Args>(__args)...)))
      {
 _Base_type::construct(__a, std::__to_address(__p),
         std::forward<_Args>(__args)...);
      }


    template<typename _Ptr>
      static constexpr
      std::__enable_if_t<__is_custom_pointer<_Ptr>::value>
      destroy(_Alloc& __a, _Ptr __p)
      noexcept(noexcept(_Base_type::destroy(__a, std::__to_address(__p))))
      { _Base_type::destroy(__a, std::__to_address(__p)); }

    static constexpr _Alloc _S_select_on_copy(const _Alloc& __a)
    { return _Base_type::select_on_container_copy_construction(__a); }

    static constexpr void _S_on_swap(_Alloc& __a, _Alloc& __b)
    { std::__alloc_on_swap(__a, __b); }

    static constexpr bool _S_propagate_on_copy_assign()
    { return _Base_type::propagate_on_container_copy_assignment::value; }

    static constexpr bool _S_propagate_on_move_assign()
    { return _Base_type::propagate_on_container_move_assignment::value; }

    static constexpr bool _S_propagate_on_swap()
    { return _Base_type::propagate_on_container_swap::value; }

    static constexpr bool _S_always_equal()
    { return _Base_type::is_always_equal::value; }

    static constexpr bool _S_nothrow_move()
    { return _S_propagate_on_move_assign() || _S_always_equal(); }

    template<typename _Tp>
      struct rebind
      { typedef typename _Base_type::template rebind_alloc<_Tp> other; };
# 166 "/usr/include/c++/11/ext/alloc_traits.h" 3
  };


}
# 41 "/usr/include/c++/11/bits/basic_string.h" 2 3







# 1 "/usr/include/c++/11/string_view" 1 3
# 36 "/usr/include/c++/11/string_view" 3
       
# 37 "/usr/include/c++/11/string_view" 3





# 1 "/usr/include/c++/11/bits/functional_hash.h" 1 3
# 33 "/usr/include/c++/11/bits/functional_hash.h" 3
       
# 34 "/usr/include/c++/11/bits/functional_hash.h" 3




namespace std __attribute__ ((__visibility__ ("default")))
{

# 50 "/usr/include/c++/11/bits/functional_hash.h" 3
  template<typename _Result, typename _Arg>
    struct __hash_base
    {
      typedef _Result result_type [[__deprecated__]];
      typedef _Arg argument_type [[__deprecated__]];
    };


  template<typename _Tp>
    struct hash;

  template<typename _Tp, typename = void>
    struct __poison_hash
    {
      static constexpr bool __enable_hash_call = false;
    private:

      __poison_hash(__poison_hash&&);
      ~__poison_hash();
    };

  template<typename _Tp>
    struct __poison_hash<_Tp, __void_t<decltype(hash<_Tp>()(declval<_Tp>()))>>
    {
      static constexpr bool __enable_hash_call = true;
    };


  template<typename _Tp, bool = is_enum<_Tp>::value>
    struct __hash_enum
    {
    private:

      __hash_enum(__hash_enum&&);
      ~__hash_enum();
    };


  template<typename _Tp>
    struct __hash_enum<_Tp, true> : public __hash_base<size_t, _Tp>
    {
      size_t
      operator()(_Tp __val) const noexcept
      {
       using __type = typename underlying_type<_Tp>::type;
       return hash<__type>{}(static_cast<__type>(__val));
      }
    };



  template<typename _Tp>
    struct hash : __hash_enum<_Tp>
    { };


  template<typename _Tp>
    struct hash<_Tp*> : public __hash_base<size_t, _Tp*>
    {
      size_t
      operator()(_Tp* __p) const noexcept
      { return reinterpret_cast<size_t>(__p); }
    };
# 125 "/usr/include/c++/11/bits/functional_hash.h" 3
  template<> struct hash<bool> : public __hash_base<size_t, bool> { size_t operator()(bool __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<char> : public __hash_base<size_t, char> { size_t operator()(char __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<signed char> : public __hash_base<size_t, signed char> { size_t operator()(signed char __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<unsigned char> : public __hash_base<size_t, unsigned char> { size_t operator()(unsigned char __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<wchar_t> : public __hash_base<size_t, wchar_t> { size_t operator()(wchar_t __val) const noexcept { return static_cast<size_t>(__val); } };







  template<> struct hash<char16_t> : public __hash_base<size_t, char16_t> { size_t operator()(char16_t __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<char32_t> : public __hash_base<size_t, char32_t> { size_t operator()(char32_t __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<short> : public __hash_base<size_t, short> { size_t operator()(short __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<int> : public __hash_base<size_t, int> { size_t operator()(int __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<long> : public __hash_base<size_t, long> { size_t operator()(long __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<long long> : public __hash_base<size_t, long long> { size_t operator()(long long __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<unsigned short> : public __hash_base<size_t, unsigned short> { size_t operator()(unsigned short __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<unsigned int> : public __hash_base<size_t, unsigned int> { size_t operator()(unsigned int __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<unsigned long> : public __hash_base<size_t, unsigned long> { size_t operator()(unsigned long __val) const noexcept { return static_cast<size_t>(__val); } };


  template<> struct hash<unsigned long long> : public __hash_base<size_t, unsigned long long> { size_t operator()(unsigned long long __val) const noexcept { return static_cast<size_t>(__val); } };
# 193 "/usr/include/c++/11/bits/functional_hash.h" 3
  struct _Hash_impl
  {
    static size_t
    hash(const void* __ptr, size_t __clength,
  size_t __seed = static_cast<size_t>(0xc70f6907UL))
    { return _Hash_bytes(__ptr, __clength, __seed); }

    template<typename _Tp>
      static size_t
      hash(const _Tp& __val)
      { return hash(&__val, sizeof(__val)); }

    template<typename _Tp>
      static size_t
      __hash_combine(const _Tp& __val, size_t __hash)
      { return hash(&__val, sizeof(__val), __hash); }
  };


  struct _Fnv_hash_impl
  {
    static size_t
    hash(const void* __ptr, size_t __clength,
  size_t __seed = static_cast<size_t>(2166136261UL))
    { return _Fnv_hash_bytes(__ptr, __clength, __seed); }

    template<typename _Tp>
      static size_t
      hash(const _Tp& __val)
      { return hash(&__val, sizeof(__val)); }

    template<typename _Tp>
      static size_t
      __hash_combine(const _Tp& __val, size_t __hash)
      { return hash(&__val, sizeof(__val), __hash); }
  };


  template<>
    struct hash<float> : public __hash_base<size_t, float>
    {
      size_t
      operator()(float __val) const noexcept
      {

 return __val != 0.0f ? std::_Hash_impl::hash(__val) : 0;
      }
    };


  template<>
    struct hash<double> : public __hash_base<size_t, double>
    {
      size_t
      operator()(double __val) const noexcept
      {

 return __val != 0.0 ? std::_Hash_impl::hash(__val) : 0;
      }
    };


  template<>
    struct hash<long double>
    : public __hash_base<size_t, long double>
    {
      __attribute__ ((__pure__)) size_t
      operator()(long double __val) const noexcept;
    };


  template<>
    struct hash<nullptr_t> : public __hash_base<size_t, nullptr_t>
    {
      size_t
      operator()(nullptr_t) const noexcept
      { return 0; }
    };
# 279 "/usr/include/c++/11/bits/functional_hash.h" 3
  template<typename _Hash>
    struct __is_fast_hash : public std::true_type
    { };

  template<>
    struct __is_fast_hash<hash<long double>> : public std::false_type
    { };


}
# 43 "/usr/include/c++/11/string_view" 2 3
# 51 "/usr/include/c++/11/string_view" 3
namespace std __attribute__ ((__visibility__ ("default")))
{








  constexpr size_t
  __sv_check(size_t __size, size_t __pos, const char* __s)
  {
    if (__pos > __size)
      __throw_out_of_range_fmt(("%s: __pos (which is %zu) > __size " "(which is %zu)")
                        , __s, __pos, __size);
    return __pos;
  }



  constexpr size_t
  __sv_limit(size_t __size, size_t __pos, size_t __off) noexcept
  {
   const bool __testoff = __off < __size - __pos;
   return __testoff ? __off : __size - __pos;
  }
# 97 "/usr/include/c++/11/string_view" 3
  template<typename _CharT, typename _Traits = std::char_traits<_CharT>>
    class basic_string_view
    {
      static_assert(!is_array_v<_CharT>);
      static_assert(is_trivial_v<_CharT> && is_standard_layout_v<_CharT>);
      static_assert(is_same_v<_CharT, typename _Traits::char_type>);

    public:


      using traits_type = _Traits;
      using value_type = _CharT;
      using pointer = value_type*;
      using const_pointer = const value_type*;
      using reference = value_type&;
      using const_reference = const value_type&;
      using const_iterator = const value_type*;
      using iterator = const_iterator;
      using const_reverse_iterator = std::reverse_iterator<const_iterator>;
      using reverse_iterator = const_reverse_iterator;
      using size_type = size_t;
      using difference_type = ptrdiff_t;
      static constexpr size_type npos = size_type(-1);



      constexpr
      basic_string_view() noexcept
      : _M_len{0}, _M_str{nullptr}
      { }

      constexpr basic_string_view(const basic_string_view&) noexcept = default;

      __attribute__((__nonnull__)) constexpr
      basic_string_view(const _CharT* __str) noexcept
      : _M_len{traits_type::length(__str)},
 _M_str{__str}
      { }

      constexpr
      basic_string_view(const _CharT* __str, size_type __len) noexcept
      : _M_len{__len}, _M_str{__str}
      { }
# 171 "/usr/include/c++/11/string_view" 3
      constexpr basic_string_view&
      operator=(const basic_string_view&) noexcept = default;



      constexpr const_iterator
      begin() const noexcept
      { return this->_M_str; }

      constexpr const_iterator
      end() const noexcept
      { return this->_M_str + this->_M_len; }

      constexpr const_iterator
      cbegin() const noexcept
      { return this->_M_str; }

      constexpr const_iterator
      cend() const noexcept
      { return this->_M_str + this->_M_len; }

      constexpr const_reverse_iterator
      rbegin() const noexcept
      { return const_reverse_iterator(this->end()); }

      constexpr const_reverse_iterator
      rend() const noexcept
      { return const_reverse_iterator(this->begin()); }

      constexpr const_reverse_iterator
      crbegin() const noexcept
      { return const_reverse_iterator(this->end()); }

      constexpr const_reverse_iterator
      crend() const noexcept
      { return const_reverse_iterator(this->begin()); }



      constexpr size_type
      size() const noexcept
      { return this->_M_len; }

      constexpr size_type
      length() const noexcept
      { return _M_len; }

      constexpr size_type
      max_size() const noexcept
      {
 return (npos - sizeof(size_type) - sizeof(void*))
  / sizeof(value_type) / 4;
      }

      [[nodiscard]] constexpr bool
      empty() const noexcept
      { return this->_M_len == 0; }



      constexpr const_reference
      operator[](size_type __pos) const noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(__pos < this->_M_len)) __builtin_unreachable(); } while (false);
 return *(this->_M_str + __pos);
      }

      constexpr const_reference
      at(size_type __pos) const
      {
 if (__pos >= _M_len)
   __throw_out_of_range_fmt(("basic_string_view::at: __pos " "(which is %zu) >= this->size() " "(which is %zu)")

                            , __pos, this->size());
 return *(this->_M_str + __pos);
      }

      constexpr const_reference
      front() const noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(this->_M_len > 0)) __builtin_unreachable(); } while (false);
 return *this->_M_str;
      }

      constexpr const_reference
      back() const noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(this->_M_len > 0)) __builtin_unreachable(); } while (false);
 return *(this->_M_str + this->_M_len - 1);
      }

      constexpr const_pointer
      data() const noexcept
      { return this->_M_str; }



      constexpr void
      remove_prefix(size_type __n) noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(this->_M_len >= __n)) __builtin_unreachable(); } while (false);
 this->_M_str += __n;
 this->_M_len -= __n;
      }

      constexpr void
      remove_suffix(size_type __n) noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(this->_M_len >= __n)) __builtin_unreachable(); } while (false);
 this->_M_len -= __n;
      }

      constexpr void
      swap(basic_string_view& __sv) noexcept
      {
 auto __tmp = *this;
 *this = __sv;
 __sv = __tmp;
      }



     
      size_type
      copy(_CharT* __str, size_type __n, size_type __pos = 0) const
      {
 ;
 __pos = std::__sv_check(size(), __pos, "basic_string_view::copy");
 const size_type __rlen = std::min(__n, _M_len - __pos);


 traits_type::copy(__str, data() + __pos, __rlen);
 return __rlen;
      }

      constexpr basic_string_view
      substr(size_type __pos = 0, size_type __n = npos) const noexcept(false)
      {
 __pos = std::__sv_check(size(), __pos, "basic_string_view::substr");
 const size_type __rlen = std::min(__n, _M_len - __pos);
 return basic_string_view{_M_str + __pos, __rlen};
      }

      constexpr int
      compare(basic_string_view __str) const noexcept
      {
 const size_type __rlen = std::min(this->_M_len, __str._M_len);
 int __ret = traits_type::compare(this->_M_str, __str._M_str, __rlen);
 if (__ret == 0)
   __ret = _S_compare(this->_M_len, __str._M_len);
 return __ret;
      }

      constexpr int
      compare(size_type __pos1, size_type __n1, basic_string_view __str) const
      { return this->substr(__pos1, __n1).compare(__str); }

      constexpr int
      compare(size_type __pos1, size_type __n1,
       basic_string_view __str, size_type __pos2, size_type __n2) const
      {
 return this->substr(__pos1, __n1).compare(__str.substr(__pos2, __n2));
      }

      __attribute__((__nonnull__)) constexpr int
      compare(const _CharT* __str) const noexcept
      { return this->compare(basic_string_view{__str}); }

      __attribute__((__nonnull__)) constexpr int
      compare(size_type __pos1, size_type __n1, const _CharT* __str) const
      { return this->substr(__pos1, __n1).compare(basic_string_view{__str}); }

      constexpr int
      compare(size_type __pos1, size_type __n1,
       const _CharT* __str, size_type __n2) const noexcept(false)
      {
 return this->substr(__pos1, __n1)
     .compare(basic_string_view(__str, __n2));
      }
# 400 "/usr/include/c++/11/string_view" 3
      constexpr size_type
      find(basic_string_view __str, size_type __pos = 0) const noexcept
      { return this->find(__str._M_str, __pos, __str._M_len); }

      constexpr size_type
      find(_CharT __c, size_type __pos = 0) const noexcept;

      constexpr size_type
      find(const _CharT* __str, size_type __pos, size_type __n) const noexcept;

      __attribute__((__nonnull__)) constexpr size_type
      find(const _CharT* __str, size_type __pos = 0) const noexcept
      { return this->find(__str, __pos, traits_type::length(__str)); }

      constexpr size_type
      rfind(basic_string_view __str, size_type __pos = npos) const noexcept
      { return this->rfind(__str._M_str, __pos, __str._M_len); }

      constexpr size_type
      rfind(_CharT __c, size_type __pos = npos) const noexcept;

      constexpr size_type
      rfind(const _CharT* __str, size_type __pos, size_type __n) const noexcept;

      __attribute__((__nonnull__)) constexpr size_type
      rfind(const _CharT* __str, size_type __pos = npos) const noexcept
      { return this->rfind(__str, __pos, traits_type::length(__str)); }

      constexpr size_type
      find_first_of(basic_string_view __str, size_type __pos = 0) const noexcept
      { return this->find_first_of(__str._M_str, __pos, __str._M_len); }

      constexpr size_type
      find_first_of(_CharT __c, size_type __pos = 0) const noexcept
      { return this->find(__c, __pos); }

      constexpr size_type
      find_first_of(const _CharT* __str, size_type __pos,
      size_type __n) const noexcept;

      __attribute__((__nonnull__)) constexpr size_type
      find_first_of(const _CharT* __str, size_type __pos = 0) const noexcept
      { return this->find_first_of(__str, __pos, traits_type::length(__str)); }

      constexpr size_type
      find_last_of(basic_string_view __str,
     size_type __pos = npos) const noexcept
      { return this->find_last_of(__str._M_str, __pos, __str._M_len); }

      constexpr size_type
      find_last_of(_CharT __c, size_type __pos=npos) const noexcept
      { return this->rfind(__c, __pos); }

      constexpr size_type
      find_last_of(const _CharT* __str, size_type __pos,
     size_type __n) const noexcept;

      __attribute__((__nonnull__)) constexpr size_type
      find_last_of(const _CharT* __str, size_type __pos = npos) const noexcept
      { return this->find_last_of(__str, __pos, traits_type::length(__str)); }

      constexpr size_type
      find_first_not_of(basic_string_view __str,
   size_type __pos = 0) const noexcept
      { return this->find_first_not_of(__str._M_str, __pos, __str._M_len); }

      constexpr size_type
      find_first_not_of(_CharT __c, size_type __pos = 0) const noexcept;

      constexpr size_type
      find_first_not_of(const _CharT* __str,
   size_type __pos, size_type __n) const noexcept;

      __attribute__((__nonnull__)) constexpr size_type
      find_first_not_of(const _CharT* __str, size_type __pos = 0) const noexcept
      {
 return this->find_first_not_of(__str, __pos,
           traits_type::length(__str));
      }

      constexpr size_type
      find_last_not_of(basic_string_view __str,
         size_type __pos = npos) const noexcept
      { return this->find_last_not_of(__str._M_str, __pos, __str._M_len); }

      constexpr size_type
      find_last_not_of(_CharT __c, size_type __pos = npos) const noexcept;

      constexpr size_type
      find_last_not_of(const _CharT* __str,
         size_type __pos, size_type __n) const noexcept;

      __attribute__((__nonnull__)) constexpr size_type
      find_last_not_of(const _CharT* __str,
         size_type __pos = npos) const noexcept
      {
 return this->find_last_not_of(__str, __pos,
          traits_type::length(__str));
      }

    private:

      static constexpr int
      _S_compare(size_type __n1, size_type __n2) noexcept
      {
 using __limits = __gnu_cxx::__int_traits<int>;
 const difference_type __diff = __n1 - __n2;
 if (__diff > __limits::__max)
   return __limits::__max;
 if (__diff < __limits::__min)
   return __limits::__min;
 return static_cast<int>(__diff);
      }

      size_t _M_len;
      const _CharT* _M_str;
    };
# 536 "/usr/include/c++/11/string_view" 3
  template<typename _CharT, typename _Traits>
    constexpr bool
    operator==(basic_string_view<_CharT, _Traits> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.size() == __y.size() && __x.compare(__y) == 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator==(basic_string_view<_CharT, _Traits> __x,
               __type_identity_t<basic_string_view<_CharT, _Traits>> __y)
    noexcept
    { return __x.size() == __y.size() && __x.compare(__y) == 0; }
# 565 "/usr/include/c++/11/string_view" 3
  template<typename _CharT, typename _Traits>
    constexpr bool
    operator==(__type_identity_t<basic_string_view<_CharT, _Traits>> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.size() == __y.size() && __x.compare(__y) == 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator!=(basic_string_view<_CharT, _Traits> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return !(__x == __y); }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator!=(basic_string_view<_CharT, _Traits> __x,
               __type_identity_t<basic_string_view<_CharT, _Traits>> __y)
    noexcept
    { return !(__x == __y); }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator!=(__type_identity_t<basic_string_view<_CharT, _Traits>> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return !(__x == __y); }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator< (basic_string_view<_CharT, _Traits> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) < 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator< (basic_string_view<_CharT, _Traits> __x,
               __type_identity_t<basic_string_view<_CharT, _Traits>> __y)
    noexcept
    { return __x.compare(__y) < 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator< (__type_identity_t<basic_string_view<_CharT, _Traits>> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) < 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator> (basic_string_view<_CharT, _Traits> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) > 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator> (basic_string_view<_CharT, _Traits> __x,
               __type_identity_t<basic_string_view<_CharT, _Traits>> __y)
    noexcept
    { return __x.compare(__y) > 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator> (__type_identity_t<basic_string_view<_CharT, _Traits>> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) > 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator<=(basic_string_view<_CharT, _Traits> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) <= 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator<=(basic_string_view<_CharT, _Traits> __x,
               __type_identity_t<basic_string_view<_CharT, _Traits>> __y)
    noexcept
    { return __x.compare(__y) <= 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator<=(__type_identity_t<basic_string_view<_CharT, _Traits>> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) <= 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator>=(basic_string_view<_CharT, _Traits> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) >= 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator>=(basic_string_view<_CharT, _Traits> __x,
               __type_identity_t<basic_string_view<_CharT, _Traits>> __y)
    noexcept
    { return __x.compare(__y) >= 0; }

  template<typename _CharT, typename _Traits>
    constexpr bool
    operator>=(__type_identity_t<basic_string_view<_CharT, _Traits>> __x,
               basic_string_view<_CharT, _Traits> __y) noexcept
    { return __x.compare(__y) >= 0; }



  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __os,
        basic_string_view<_CharT,_Traits> __str)
    { return __ostream_insert(__os, __str.data(), __str.size()); }




  using string_view = basic_string_view<char>;

  using wstring_view = basic_string_view<wchar_t>;




  using u16string_view = basic_string_view<char16_t>;
  using u32string_view = basic_string_view<char32_t>;



  template<typename _Tp>
    struct hash;

  template<>
    struct hash<string_view>
    : public __hash_base<size_t, string_view>
    {
      size_t
      operator()(const string_view& __str) const noexcept
      { return std::_Hash_impl::hash(__str.data(), __str.length()); }
    };

  template<>
    struct __is_fast_hash<hash<string_view>> : std::false_type
    { };


  template<>
    struct hash<wstring_view>
    : public __hash_base<size_t, wstring_view>
    {
      size_t
      operator()(const wstring_view& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(),
                                     __s.length() * sizeof(wchar_t)); }
    };

  template<>
    struct __is_fast_hash<hash<wstring_view>> : std::false_type
    { };
# 736 "/usr/include/c++/11/string_view" 3
  template<>
    struct hash<u16string_view>
    : public __hash_base<size_t, u16string_view>
    {
      size_t
      operator()(const u16string_view& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(),
                                     __s.length() * sizeof(char16_t)); }
    };

  template<>
    struct __is_fast_hash<hash<u16string_view>> : std::false_type
    { };

  template<>
    struct hash<u32string_view>
    : public __hash_base<size_t, u32string_view>
    {
      size_t
      operator()(const u32string_view& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(),
                                     __s.length() * sizeof(char32_t)); }
    };

  template<>
    struct __is_fast_hash<hash<u32string_view>> : std::false_type
    { };

  inline namespace literals
  {
  inline namespace string_view_literals
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
    inline constexpr basic_string_view<char>
    operator""sv(const char* __str, size_t __len) noexcept
    { return basic_string_view<char>{__str, __len}; }


    inline constexpr basic_string_view<wchar_t>
    operator""sv(const wchar_t* __str, size_t __len) noexcept
    { return basic_string_view<wchar_t>{__str, __len}; }
# 786 "/usr/include/c++/11/string_view" 3
    inline constexpr basic_string_view<char16_t>
    operator""sv(const char16_t* __str, size_t __len) noexcept
    { return basic_string_view<char16_t>{__str, __len}; }

    inline constexpr basic_string_view<char32_t>
    operator""sv(const char32_t* __str, size_t __len) noexcept
    { return basic_string_view<char32_t>{__str, __len}; }

#pragma GCC diagnostic pop
  }
  }
# 812 "/usr/include/c++/11/string_view" 3

}

# 1 "/usr/include/c++/11/bits/string_view.tcc" 1 3
# 37 "/usr/include/c++/11/bits/string_view.tcc" 3
       
# 38 "/usr/include/c++/11/bits/string_view.tcc" 3



namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find(const _CharT* __str, size_type __pos, size_type __n) const noexcept
    {
      ;

      if (__n == 0)
 return __pos <= _M_len ? __pos : npos;
      if (__pos >= _M_len)
 return npos;

      const _CharT __elem0 = __str[0];
      const _CharT* __first = _M_str + __pos;
      const _CharT* const __last = _M_str + _M_len;
      size_type __len = _M_len - __pos;

      while (__len >= __n)
 {

   __first = traits_type::find(__first, __len - __n + 1, __elem0);
   if (!__first)
     return npos;



   if (traits_type::compare(__first, __str, __n) == 0)
     return __first - _M_str;
   __len = __last - ++__first;
 }
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find(_CharT __c, size_type __pos) const noexcept
    {
      size_type __ret = npos;
      if (__pos < this->_M_len)
 {
   const size_type __n = this->_M_len - __pos;
   const _CharT* __p = traits_type::find(this->_M_str + __pos, __n, __c);
   if (__p)
     __ret = __p - this->_M_str;
 }
      return __ret;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    rfind(const _CharT* __str, size_type __pos, size_type __n) const noexcept
    {
      ;

      if (__n <= this->_M_len)
 {
   __pos = std::min(size_type(this->_M_len - __n), __pos);
   do
     {
       if (traits_type::compare(this->_M_str + __pos, __str, __n) == 0)
  return __pos;
     }
   while (__pos-- > 0);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    rfind(_CharT __c, size_type __pos) const noexcept
    {
      size_type __size = this->_M_len;
      if (__size > 0)
 {
   if (--__size > __pos)
     __size = __pos;
   for (++__size; __size-- > 0; )
     if (traits_type::eq(this->_M_str[__size], __c))
       return __size;
 }
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find_first_of(const _CharT* __str, size_type __pos,
    size_type __n) const noexcept
    {
      ;
      for (; __n && __pos < this->_M_len; ++__pos)
 {
   const _CharT* __p = traits_type::find(__str, __n,
      this->_M_str[__pos]);
   if (__p)
     return __pos;
 }
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find_last_of(const _CharT* __str, size_type __pos,
   size_type __n) const noexcept
    {
      ;
      size_type __size = this->size();
      if (__size && __n)
 {
   if (--__size > __pos)
     __size = __pos;
   do
     {
       if (traits_type::find(__str, __n, this->_M_str[__size]))
  return __size;
     }
   while (__size-- != 0);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find_first_not_of(const _CharT* __str, size_type __pos,
        size_type __n) const noexcept
    {
      ;
      for (; __pos < this->_M_len; ++__pos)
 if (!traits_type::find(__str, __n, this->_M_str[__pos]))
   return __pos;
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find_first_not_of(_CharT __c, size_type __pos) const noexcept
    {
      for (; __pos < this->_M_len; ++__pos)
 if (!traits_type::eq(this->_M_str[__pos], __c))
   return __pos;
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find_last_not_of(const _CharT* __str, size_type __pos,
       size_type __n) const noexcept
    {
      ;
      size_type __size = this->_M_len;
      if (__size)
 {
   if (--__size > __pos)
     __size = __pos;
   do
     {
       if (!traits_type::find(__str, __n, this->_M_str[__size]))
  return __size;
     }
   while (__size--);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits>
    constexpr typename basic_string_view<_CharT, _Traits>::size_type
    basic_string_view<_CharT, _Traits>::
    find_last_not_of(_CharT __c, size_type __pos) const noexcept
    {
      size_type __size = this->_M_len;
      if (__size)
 {
   if (--__size > __pos)
     __size = __pos;
   do
     {
       if (!traits_type::eq(this->_M_str[__size], __c))
  return __size;
     }
   while (__size--);
 }
      return npos;
    }


}
# 816 "/usr/include/c++/11/string_view" 2 3
# 49 "/usr/include/c++/11/bits/basic_string.h" 2 3


namespace std __attribute__ ((__visibility__ ("default")))
{

# 65 "/usr/include/c++/11/bits/basic_string.h" 3
namespace __cxx11 {
# 84 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    class basic_string
    {
      typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
 rebind<_CharT>::other _Char_alloc_type;
      typedef __gnu_cxx::__alloc_traits<_Char_alloc_type> _Alloc_traits;


    public:
      typedef _Traits traits_type;
      typedef typename _Traits::char_type value_type;
      typedef _Char_alloc_type allocator_type;
      typedef typename _Alloc_traits::size_type size_type;
      typedef typename _Alloc_traits::difference_type difference_type;
      typedef typename _Alloc_traits::reference reference;
      typedef typename _Alloc_traits::const_reference const_reference;
      typedef typename _Alloc_traits::pointer pointer;
      typedef typename _Alloc_traits::const_pointer const_pointer;
      typedef __gnu_cxx::__normal_iterator<pointer, basic_string> iterator;
      typedef __gnu_cxx::__normal_iterator<const_pointer, basic_string>
       const_iterator;
      typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
      typedef std::reverse_iterator<iterator> reverse_iterator;


      static const size_type npos = static_cast<size_type>(-1);

    protected:




      typedef const_iterator __const_iterator;


    private:


      typedef basic_string_view<_CharT, _Traits> __sv_type;

      template<typename _Tp, typename _Res>
 using _If_sv = enable_if_t<
   __and_<is_convertible<const _Tp&, __sv_type>,
   __not_<is_convertible<const _Tp*, const basic_string*>>,
   __not_<is_convertible<const _Tp&, const _CharT*>>>::value,
   _Res>;


      static __sv_type
      _S_to_string_view(__sv_type __svt) noexcept
      { return __svt; }





      struct __sv_wrapper
      {
 explicit __sv_wrapper(__sv_type __sv) noexcept : _M_sv(__sv) { }
 __sv_type _M_sv;
      };







      explicit
      basic_string(__sv_wrapper __svw, const _Alloc& __a)
      : basic_string(__svw._M_sv.data(), __svw._M_sv.size(), __a) { }



      struct _Alloc_hider : allocator_type
      {




 _Alloc_hider(pointer __dat, const _Alloc& __a)
 : allocator_type(__a), _M_p(__dat) { }

 _Alloc_hider(pointer __dat, _Alloc&& __a = _Alloc())
 : allocator_type(std::move(__a)), _M_p(__dat) { }


 pointer _M_p;
      };

      _Alloc_hider _M_dataplus;
      size_type _M_string_length;

      enum { _S_local_capacity = 15 / sizeof(_CharT) };

      union
      {
 _CharT _M_local_buf[_S_local_capacity + 1];
 size_type _M_allocated_capacity;
      };

      void
      _M_data(pointer __p)
      { _M_dataplus._M_p = __p; }

      void
      _M_length(size_type __length)
      { _M_string_length = __length; }

      pointer
      _M_data() const
      { return _M_dataplus._M_p; }

      pointer
      _M_local_data()
      {

 return std::pointer_traits<pointer>::pointer_to(*_M_local_buf);



      }

      const_pointer
      _M_local_data() const
      {

 return std::pointer_traits<const_pointer>::pointer_to(*_M_local_buf);



      }

      void
      _M_capacity(size_type __capacity)
      { _M_allocated_capacity = __capacity; }

      void
      _M_set_length(size_type __n)
      {
 _M_length(__n);
 traits_type::assign(_M_data()[__n], _CharT());
      }

      bool
      _M_is_local() const
      { return _M_data() == _M_local_data(); }


      pointer
      _M_create(size_type&, size_type);

      void
      _M_dispose()
      {
 if (!_M_is_local())
   _M_destroy(_M_allocated_capacity);
      }

      void
      _M_destroy(size_type __size) throw()
      { _Alloc_traits::deallocate(_M_get_allocator(), _M_data(), __size + 1); }



      template<typename _InIterator>
        void
        _M_construct_aux(_InIterator __beg, _InIterator __end,
    std::__false_type)
 {
          typedef typename iterator_traits<_InIterator>::iterator_category _Tag;
          _M_construct(__beg, __end, _Tag());
 }



      template<typename _Integer>
        void
        _M_construct_aux(_Integer __beg, _Integer __end, std::__true_type)
 { _M_construct_aux_2(static_cast<size_type>(__beg), __end); }

      void
      _M_construct_aux_2(size_type __req, _CharT __c)
      { _M_construct(__req, __c); }

      template<typename _InIterator>
        void
        _M_construct(_InIterator __beg, _InIterator __end)
 {
   typedef typename std::__is_integer<_InIterator>::__type _Integral;
   _M_construct_aux(__beg, __end, _Integral());
        }


      template<typename _InIterator>
        void
        _M_construct(_InIterator __beg, _InIterator __end,
       std::input_iterator_tag);



      template<typename _FwdIterator>
        void
        _M_construct(_FwdIterator __beg, _FwdIterator __end,
       std::forward_iterator_tag);

      void
      _M_construct(size_type __req, _CharT __c);

      allocator_type&
      _M_get_allocator()
      { return _M_dataplus; }

      const allocator_type&
      _M_get_allocator() const
      { return _M_dataplus; }

    private:
# 317 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      _M_check(size_type __pos, const char* __s) const
      {
 if (__pos > this->size())
   __throw_out_of_range_fmt(("%s: __pos (which is %zu) > " "this->size() (which is %zu)")
                                         ,
       __s, __pos, this->size());
 return __pos;
      }

      void
      _M_check_length(size_type __n1, size_type __n2, const char* __s) const
      {
 if (this->max_size() - (this->size() - __n1) < __n2)
   __throw_length_error((__s));
      }



      size_type
      _M_limit(size_type __pos, size_type __off) const noexcept
      {
 const bool __testoff = __off < this->size() - __pos;
 return __testoff ? __off : this->size() - __pos;
      }


      bool
      _M_disjunct(const _CharT* __s) const noexcept
      {
 return (less<const _CharT*>()(__s, _M_data())
  || less<const _CharT*>()(_M_data() + this->size(), __s));
      }



      static void
      _S_copy(_CharT* __d, const _CharT* __s, size_type __n)
      {
 if (__n == 1)
   traits_type::assign(*__d, *__s);
 else
   traits_type::copy(__d, __s, __n);
      }

      static void
      _S_move(_CharT* __d, const _CharT* __s, size_type __n)
      {
 if (__n == 1)
   traits_type::assign(*__d, *__s);
 else
   traits_type::move(__d, __s, __n);
      }

      static void
      _S_assign(_CharT* __d, size_type __n, _CharT __c)
      {
 if (__n == 1)
   traits_type::assign(*__d, __c);
 else
   traits_type::assign(__d, __n, __c);
      }



      template<class _Iterator>
        static void
        _S_copy_chars(_CharT* __p, _Iterator __k1, _Iterator __k2)
        {
   for (; __k1 != __k2; ++__k1, (void)++__p)
     traits_type::assign(*__p, *__k1);
 }

      static void
      _S_copy_chars(_CharT* __p, iterator __k1, iterator __k2) noexcept
      { _S_copy_chars(__p, __k1.base(), __k2.base()); }

      static void
      _S_copy_chars(_CharT* __p, const_iterator __k1, const_iterator __k2)
      noexcept
      { _S_copy_chars(__p, __k1.base(), __k2.base()); }

      static void
      _S_copy_chars(_CharT* __p, _CharT* __k1, _CharT* __k2) noexcept
      { _S_copy(__p, __k1, __k2 - __k1); }

      static void
      _S_copy_chars(_CharT* __p, const _CharT* __k1, const _CharT* __k2)
      noexcept
      { _S_copy(__p, __k1, __k2 - __k1); }

      static int
      _S_compare(size_type __n1, size_type __n2) noexcept
      {
 const difference_type __d = difference_type(__n1 - __n2);

 if (__d > __gnu_cxx::__numeric_traits<int>::__max)
   return __gnu_cxx::__numeric_traits<int>::__max;
 else if (__d < __gnu_cxx::__numeric_traits<int>::__min)
   return __gnu_cxx::__numeric_traits<int>::__min;
 else
   return int(__d);
      }

      void
      _M_assign(const basic_string&);

      void
      _M_mutate(size_type __pos, size_type __len1, const _CharT* __s,
  size_type __len2);

      void
      _M_erase(size_type __pos, size_type __n);

    public:







      basic_string()
      noexcept(is_nothrow_default_constructible<_Alloc>::value)
      : _M_dataplus(_M_local_data())
      { _M_set_length(0); }




      explicit
      basic_string(const _Alloc& __a) noexcept
      : _M_dataplus(_M_local_data(), __a)
      { _M_set_length(0); }





      basic_string(const basic_string& __str)
      : _M_dataplus(_M_local_data(),
      _Alloc_traits::_S_select_on_copy(__str._M_get_allocator()))
      { _M_construct(__str._M_data(), __str._M_data() + __str.length()); }
# 469 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string(const basic_string& __str, size_type __pos,
     const _Alloc& __a = _Alloc())
      : _M_dataplus(_M_local_data(), __a)
      {
 const _CharT* __start = __str._M_data()
   + __str._M_check(__pos, "basic_string::basic_string");
 _M_construct(__start, __start + __str._M_limit(__pos, npos));
      }







      basic_string(const basic_string& __str, size_type __pos,
     size_type __n)
      : _M_dataplus(_M_local_data())
      {
 const _CharT* __start = __str._M_data()
   + __str._M_check(__pos, "basic_string::basic_string");
 _M_construct(__start, __start + __str._M_limit(__pos, __n));
      }
# 500 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string(const basic_string& __str, size_type __pos,
     size_type __n, const _Alloc& __a)
      : _M_dataplus(_M_local_data(), __a)
      {
 const _CharT* __start
   = __str._M_data() + __str._M_check(__pos, "string::string");
 _M_construct(__start, __start + __str._M_limit(__pos, __n));
      }
# 518 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string(const _CharT* __s, size_type __n,
     const _Alloc& __a = _Alloc())
      : _M_dataplus(_M_local_data(), __a)
      { _M_construct(__s, __s + __n); }
# 531 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename = _RequireAllocator<_Alloc>>

      basic_string(const _CharT* __s, const _Alloc& __a = _Alloc())
      : _M_dataplus(_M_local_data(), __a)
      {
 const _CharT* __end = __s ? __s + traits_type::length(__s)

   : reinterpret_cast<const _CharT*>(__alignof__(_CharT));
 _M_construct(__s, __end, random_access_iterator_tag());
      }
# 551 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename = _RequireAllocator<_Alloc>>

      basic_string(size_type __n, _CharT __c, const _Alloc& __a = _Alloc())
      : _M_dataplus(_M_local_data(), __a)
      { _M_construct(__n, __c); }
# 565 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string(basic_string&& __str) noexcept
      : _M_dataplus(_M_local_data(), std::move(__str._M_get_allocator()))
      {
 if (__str._M_is_local())
   {
     traits_type::copy(_M_local_buf, __str._M_local_buf,
         _S_local_capacity + 1);
   }
 else
   {
     _M_data(__str._M_data());
     _M_capacity(__str._M_allocated_capacity);
   }




 _M_length(__str.length());
 __str._M_data(__str._M_local_data());
 __str._M_set_length(0);
      }






      basic_string(initializer_list<_CharT> __l, const _Alloc& __a = _Alloc())
      : _M_dataplus(_M_local_data(), __a)
      { _M_construct(__l.begin(), __l.end()); }

      basic_string(const basic_string& __str, const _Alloc& __a)
      : _M_dataplus(_M_local_data(), __a)
      { _M_construct(__str.begin(), __str.end()); }

      basic_string(basic_string&& __str, const _Alloc& __a)
      noexcept(_Alloc_traits::_S_always_equal())
      : _M_dataplus(_M_local_data(), __a)
      {
 if (__str._M_is_local())
   {
     traits_type::copy(_M_local_buf, __str._M_local_buf,
         _S_local_capacity + 1);
     _M_length(__str.length());
     __str._M_set_length(0);
   }
 else if (_Alloc_traits::_S_always_equal()
     || __str.get_allocator() == __a)
   {
     _M_data(__str._M_data());
     _M_length(__str.length());
     _M_capacity(__str._M_allocated_capacity);
     __str._M_data(__str._M_local_buf);
     __str._M_set_length(0);
   }
 else
   _M_construct(__str.begin(), __str.end());
      }
# 633 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _InputIterator,
        typename = std::_RequireInputIter<_InputIterator>>



        basic_string(_InputIterator __beg, _InputIterator __end,
       const _Alloc& __a = _Alloc())
 : _M_dataplus(_M_local_data(), __a)
 { _M_construct(__beg, __end); }
# 651 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp,
        typename = enable_if_t<is_convertible_v<const _Tp&, __sv_type>>>
 basic_string(const _Tp& __t, size_type __pos, size_type __n,
       const _Alloc& __a = _Alloc())
 : basic_string(_S_to_string_view(__t).substr(__pos, __n), __a) { }






      template<typename _Tp, typename = _If_sv<_Tp, void>>
 explicit
 basic_string(const _Tp& __t, const _Alloc& __a = _Alloc())
 : basic_string(__sv_wrapper(_S_to_string_view(__t)), __a) { }





      ~basic_string()
      { _M_dispose(); }





      basic_string&
      operator=(const basic_string& __str)
      {
 return this->assign(__str);
      }





      basic_string&
      operator=(const _CharT* __s)
      { return this->assign(__s); }
# 699 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      operator=(_CharT __c)
      {
 this->assign(1, __c);
 return *this;
      }
# 716 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      operator=(basic_string&& __str)
      noexcept(_Alloc_traits::_S_nothrow_move())
      {
 if (!_M_is_local() && _Alloc_traits::_S_propagate_on_move_assign()
     && !_Alloc_traits::_S_always_equal()
     && _M_get_allocator() != __str._M_get_allocator())
   {

     _M_destroy(_M_allocated_capacity);
     _M_data(_M_local_data());
     _M_set_length(0);
   }

 std::__alloc_on_move(_M_get_allocator(), __str._M_get_allocator());

 if (__str._M_is_local())
   {



     if (__builtin_expect(std::__addressof(__str) != this, true))
       {
  if (__str.size())
    this->_S_copy(_M_data(), __str._M_data(), __str.size());
  _M_set_length(__str.size());
       }
   }
 else if (_Alloc_traits::_S_propagate_on_move_assign()
     || _Alloc_traits::_S_always_equal()
     || _M_get_allocator() == __str._M_get_allocator())
   {

     pointer __data = nullptr;
     size_type __capacity;
     if (!_M_is_local())
       {
  if (_Alloc_traits::_S_always_equal())
    {

      __data = _M_data();
      __capacity = _M_allocated_capacity;
    }
  else
    _M_destroy(_M_allocated_capacity);
       }

     _M_data(__str._M_data());
     _M_length(__str.length());
     _M_capacity(__str._M_allocated_capacity);
     if (__data)
       {
  __str._M_data(__data);
  __str._M_capacity(__capacity);
       }
     else
       __str._M_data(__str._M_local_buf);
   }
 else
   assign(__str);
 __str.clear();
 return *this;
      }





      basic_string&
      operator=(initializer_list<_CharT> __l)
      {
 this->assign(__l.begin(), __l.size());
 return *this;
      }







     template<typename _Tp>
       _If_sv<_Tp, basic_string&>
       operator=(const _Tp& __svt)
       { return this->assign(__svt); }





      operator __sv_type() const noexcept
      { return __sv_type(data(), size()); }







      iterator
      begin() noexcept
      { return iterator(_M_data()); }





      const_iterator
      begin() const noexcept
      { return const_iterator(_M_data()); }





      iterator
      end() noexcept
      { return iterator(_M_data() + this->size()); }





      const_iterator
      end() const noexcept
      { return const_iterator(_M_data() + this->size()); }






      reverse_iterator
      rbegin() noexcept
      { return reverse_iterator(this->end()); }






      const_reverse_iterator
      rbegin() const noexcept
      { return const_reverse_iterator(this->end()); }






      reverse_iterator
      rend() noexcept
      { return reverse_iterator(this->begin()); }






      const_reverse_iterator
      rend() const noexcept
      { return const_reverse_iterator(this->begin()); }






      const_iterator
      cbegin() const noexcept
      { return const_iterator(this->_M_data()); }





      const_iterator
      cend() const noexcept
      { return const_iterator(this->_M_data() + this->size()); }






      const_reverse_iterator
      crbegin() const noexcept
      { return const_reverse_iterator(this->end()); }






      const_reverse_iterator
      crend() const noexcept
      { return const_reverse_iterator(this->begin()); }


    public:



      size_type
      size() const noexcept
      { return _M_string_length; }



      size_type
      length() const noexcept
      { return _M_string_length; }


      size_type
      max_size() const noexcept
      { return (_Alloc_traits::max_size(_M_get_allocator()) - 1) / 2; }
# 944 "/usr/include/c++/11/bits/basic_string.h" 3
      void
      resize(size_type __n, _CharT __c);
# 957 "/usr/include/c++/11/bits/basic_string.h" 3
      void
      resize(size_type __n)
      { this->resize(__n, _CharT()); }


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

      void
      shrink_to_fit() noexcept
      { reserve(); }
#pragma GCC diagnostic pop






      size_type
      capacity() const noexcept
      {
 return _M_is_local() ? size_type(_S_local_capacity)
                      : _M_allocated_capacity;
      }
# 999 "/usr/include/c++/11/bits/basic_string.h" 3
      void
      reserve(size_type __res_arg);







      void
      reserve();




      void
      clear() noexcept
      { _M_set_length(0); }





      [[__nodiscard__]] bool
      empty() const noexcept
      { return this->size() == 0; }
# 1037 "/usr/include/c++/11/bits/basic_string.h" 3
      const_reference
      operator[] (size_type __pos) const noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(__pos <= size())) __builtin_unreachable(); } while (false);
 return _M_data()[__pos];
      }
# 1054 "/usr/include/c++/11/bits/basic_string.h" 3
      reference
      operator[](size_type __pos)
      {


 do { if (__builtin_is_constant_evaluated() && !bool(__pos <= size())) __builtin_unreachable(); } while (false);

 ;
 return _M_data()[__pos];
      }
# 1075 "/usr/include/c++/11/bits/basic_string.h" 3
      const_reference
      at(size_type __n) const
      {
 if (__n >= this->size())
   __throw_out_of_range_fmt(("basic_string::at: __n " "(which is %zu) >= this->size() " "(which is %zu)")

                            ,
       __n, this->size());
 return _M_data()[__n];
      }
# 1096 "/usr/include/c++/11/bits/basic_string.h" 3
      reference
      at(size_type __n)
      {
 if (__n >= size())
   __throw_out_of_range_fmt(("basic_string::at: __n " "(which is %zu) >= this->size() " "(which is %zu)")

                            ,
       __n, this->size());
 return _M_data()[__n];
      }






      reference
      front() noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(!empty())) __builtin_unreachable(); } while (false);
 return operator[](0);
      }





      const_reference
      front() const noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(!empty())) __builtin_unreachable(); } while (false);
 return operator[](0);
      }





      reference
      back() noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(!empty())) __builtin_unreachable(); } while (false);
 return operator[](this->size() - 1);
      }





      const_reference
      back() const noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(!empty())) __builtin_unreachable(); } while (false);
 return operator[](this->size() - 1);
      }
# 1159 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      operator+=(const basic_string& __str)
      { return this->append(__str); }






      basic_string&
      operator+=(const _CharT* __s)
      { return this->append(__s); }






      basic_string&
      operator+=(_CharT __c)
      {
 this->push_back(__c);
 return *this;
      }







      basic_string&
      operator+=(initializer_list<_CharT> __l)
      { return this->append(__l.begin(), __l.size()); }
# 1201 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 operator+=(const _Tp& __svt)
 { return this->append(__svt); }







      basic_string&
      append(const basic_string& __str)
      { return _M_append(__str._M_data(), __str.size()); }
# 1229 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      append(const basic_string& __str, size_type __pos, size_type __n = npos)
      { return _M_append(__str._M_data()
    + __str._M_check(__pos, "basic_string::append"),
    __str._M_limit(__pos, __n)); }







      basic_string&
      append(const _CharT* __s, size_type __n)
      {
 ;
 _M_check_length(size_type(0), __n, "basic_string::append");
 return _M_append(__s, __n);
      }






      basic_string&
      append(const _CharT* __s)
      {
 ;
 const size_type __n = traits_type::length(__s);
 _M_check_length(size_type(0), __n, "basic_string::append");
 return _M_append(__s, __n);
      }
# 1271 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      append(size_type __n, _CharT __c)
      { return _M_replace_aux(this->size(), size_type(0), __n, __c); }







      basic_string&
      append(initializer_list<_CharT> __l)
      { return this->append(__l.begin(), __l.size()); }
# 1295 "/usr/include/c++/11/bits/basic_string.h" 3
      template<class _InputIterator,
        typename = std::_RequireInputIter<_InputIterator>>



        basic_string&
        append(_InputIterator __first, _InputIterator __last)
        { return this->replace(end(), end(), __first, __last); }







      template<typename _Tp>
        _If_sv<_Tp, basic_string&>
        append(const _Tp& __svt)
        {
          __sv_type __sv = __svt;
          return this->append(__sv.data(), __sv.size());
        }
# 1325 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
        _If_sv<_Tp, basic_string&>
 append(const _Tp& __svt, size_type __pos, size_type __n = npos)
 {
   __sv_type __sv = __svt;
   return _M_append(__sv.data()
       + std::__sv_check(__sv.size(), __pos, "basic_string::append"),
       std::__sv_limit(__sv.size(), __pos, __n));
 }






      void
      push_back(_CharT __c)
      {
 const size_type __size = this->size();
 if (__size + 1 > this->capacity())
   this->_M_mutate(__size, size_type(0), 0, size_type(1));
 traits_type::assign(this->_M_data()[__size], __c);
 this->_M_set_length(__size + 1);
      }






      basic_string&
      assign(const basic_string& __str)
      {

 if (_Alloc_traits::_S_propagate_on_copy_assign())
   {
     if (!_Alloc_traits::_S_always_equal() && !_M_is_local()
  && _M_get_allocator() != __str._M_get_allocator())
       {


  if (__str.size() <= _S_local_capacity)
    {
      _M_destroy(_M_allocated_capacity);
      _M_data(_M_local_data());
      _M_set_length(0);
    }
  else
    {
      const auto __len = __str.size();
      auto __alloc = __str._M_get_allocator();

      auto __ptr = _Alloc_traits::allocate(__alloc, __len + 1);
      _M_destroy(_M_allocated_capacity);
      _M_data(__ptr);
      _M_capacity(__len);
      _M_set_length(__len);
    }
       }
     std::__alloc_on_copy(_M_get_allocator(), __str._M_get_allocator());
   }

 this->_M_assign(__str);
 return *this;
      }
# 1400 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      assign(basic_string&& __str)
      noexcept(_Alloc_traits::_S_nothrow_move())
      {


 return *this = std::move(__str);
      }
# 1423 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      assign(const basic_string& __str, size_type __pos, size_type __n = npos)
      { return _M_replace(size_type(0), this->size(), __str._M_data()
     + __str._M_check(__pos, "basic_string::assign"),
     __str._M_limit(__pos, __n)); }
# 1439 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      assign(const _CharT* __s, size_type __n)
      {
 ;
 return _M_replace(size_type(0), this->size(), __s, __n);
      }
# 1455 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      assign(const _CharT* __s)
      {
 ;
 return _M_replace(size_type(0), this->size(), __s,
     traits_type::length(__s));
      }
# 1472 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      assign(size_type __n, _CharT __c)
      { return _M_replace_aux(size_type(0), this->size(), __n, __c); }
# 1485 "/usr/include/c++/11/bits/basic_string.h" 3
      template<class _InputIterator,
        typename = std::_RequireInputIter<_InputIterator>>



        basic_string&
        assign(_InputIterator __first, _InputIterator __last)
        { return this->replace(begin(), end(), __first, __last); }







      basic_string&
      assign(initializer_list<_CharT> __l)
      { return this->assign(__l.begin(), __l.size()); }
# 1511 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 assign(const _Tp& __svt)
 {
   __sv_type __sv = __svt;
   return this->assign(__sv.data(), __sv.size());
 }
# 1526 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 assign(const _Tp& __svt, size_type __pos, size_type __n = npos)
 {
   __sv_type __sv = __svt;
   return _M_replace(size_type(0), this->size(),
       __sv.data()
       + std::__sv_check(__sv.size(), __pos, "basic_string::assign"),
       std::__sv_limit(__sv.size(), __pos, __n));
 }
# 1554 "/usr/include/c++/11/bits/basic_string.h" 3
      iterator
      insert(const_iterator __p, size_type __n, _CharT __c)
      {
 ;
 const size_type __pos = __p - begin();
 this->replace(__p, __p, __n, __c);
 return iterator(this->_M_data() + __pos);
      }
# 1596 "/usr/include/c++/11/bits/basic_string.h" 3
      template<class _InputIterator,
        typename = std::_RequireInputIter<_InputIterator>>
 iterator
        insert(const_iterator __p, _InputIterator __beg, _InputIterator __end)
        {
   ;
   const size_type __pos = __p - begin();
   this->replace(__p, __p, __beg, __end);
   return iterator(this->_M_data() + __pos);
 }
# 1632 "/usr/include/c++/11/bits/basic_string.h" 3
      iterator
      insert(const_iterator __p, initializer_list<_CharT> __l)
      { return this->insert(__p, __l.begin(), __l.end()); }
# 1659 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      insert(size_type __pos1, const basic_string& __str)
      { return this->replace(__pos1, size_type(0),
        __str._M_data(), __str.size()); }
# 1682 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      insert(size_type __pos1, const basic_string& __str,
      size_type __pos2, size_type __n = npos)
      { return this->replace(__pos1, size_type(0), __str._M_data()
        + __str._M_check(__pos2, "basic_string::insert"),
        __str._M_limit(__pos2, __n)); }
# 1705 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      insert(size_type __pos, const _CharT* __s, size_type __n)
      { return this->replace(__pos, size_type(0), __s, __n); }
# 1724 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      insert(size_type __pos, const _CharT* __s)
      {
 ;
 return this->replace(__pos, size_type(0), __s,
        traits_type::length(__s));
      }
# 1748 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      insert(size_type __pos, size_type __n, _CharT __c)
      { return _M_replace_aux(_M_check(__pos, "basic_string::insert"),
         size_type(0), __n, __c); }
# 1766 "/usr/include/c++/11/bits/basic_string.h" 3
      iterator
      insert(__const_iterator __p, _CharT __c)
      {
 ;
 const size_type __pos = __p - begin();
 _M_replace_aux(__pos, size_type(0), size_type(1), __c);
 return iterator(_M_data() + __pos);
      }
# 1782 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 insert(size_type __pos, const _Tp& __svt)
 {
   __sv_type __sv = __svt;
   return this->insert(__pos, __sv.data(), __sv.size());
 }
# 1798 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 insert(size_type __pos1, const _Tp& __svt,
        size_type __pos2, size_type __n = npos)
 {
   __sv_type __sv = __svt;
   return this->replace(__pos1, size_type(0),
       __sv.data()
       + std::__sv_check(__sv.size(), __pos2, "basic_string::insert"),
       std::__sv_limit(__sv.size(), __pos2, __n));
 }
# 1826 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      erase(size_type __pos = 0, size_type __n = npos)
      {
 _M_check(__pos, "basic_string::erase");
 if (__n == npos)
   this->_M_set_length(__pos);
 else if (__n != 0)
   this->_M_erase(__pos, _M_limit(__pos, __n));
 return *this;
      }
# 1845 "/usr/include/c++/11/bits/basic_string.h" 3
      iterator
      erase(__const_iterator __position)
      {

                           ;
 const size_type __pos = __position - begin();
 this->_M_erase(__pos, size_type(1));
 return iterator(_M_data() + __pos);
      }
# 1864 "/usr/include/c++/11/bits/basic_string.h" 3
      iterator
      erase(__const_iterator __first, __const_iterator __last)
      {

                        ;
        const size_type __pos = __first - begin();
 if (__last == end())
   this->_M_set_length(__pos);
 else
   this->_M_erase(__pos, __last - __first);
 return iterator(this->_M_data() + __pos);
      }







      void
      pop_back() noexcept
      {
 do { if (__builtin_is_constant_evaluated() && !bool(!empty())) __builtin_unreachable(); } while (false);
 _M_erase(size() - 1, 1);
      }
# 1908 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(size_type __pos, size_type __n, const basic_string& __str)
      { return this->replace(__pos, __n, __str._M_data(), __str.size()); }
# 1930 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(size_type __pos1, size_type __n1, const basic_string& __str,
       size_type __pos2, size_type __n2 = npos)
      { return this->replace(__pos1, __n1, __str._M_data()
        + __str._M_check(__pos2, "basic_string::replace"),
        __str._M_limit(__pos2, __n2)); }
# 1955 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(size_type __pos, size_type __n1, const _CharT* __s,
       size_type __n2)
      {
 ;
 return _M_replace(_M_check(__pos, "basic_string::replace"),
     _M_limit(__pos, __n1), __s, __n2);
      }
# 1980 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(size_type __pos, size_type __n1, const _CharT* __s)
      {
 ;
 return this->replace(__pos, __n1, __s, traits_type::length(__s));
      }
# 2004 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(size_type __pos, size_type __n1, size_type __n2, _CharT __c)
      { return _M_replace_aux(_M_check(__pos, "basic_string::replace"),
         _M_limit(__pos, __n1), __n2, __c); }
# 2022 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2,
       const basic_string& __str)
      { return this->replace(__i1, __i2, __str._M_data(), __str.size()); }
# 2042 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2,
       const _CharT* __s, size_type __n)
      {

                      ;
 return this->replace(__i1 - begin(), __i2 - __i1, __s, __n);
      }
# 2064 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2, const _CharT* __s)
      {
 ;
 return this->replace(__i1, __i2, __s, traits_type::length(__s));
      }
# 2085 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2, size_type __n,
       _CharT __c)
      {

                      ;
 return _M_replace_aux(__i1 - begin(), __i2 - __i1, __n, __c);
      }
# 2110 "/usr/include/c++/11/bits/basic_string.h" 3
      template<class _InputIterator,
        typename = std::_RequireInputIter<_InputIterator>>
        basic_string&
        replace(const_iterator __i1, const_iterator __i2,
  _InputIterator __k1, _InputIterator __k2)
        {
  
                        ;
   ;
   return this->_M_replace_dispatch(__i1, __i2, __k1, __k2,
        std::__false_type());
 }
# 2142 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2,
       _CharT* __k1, _CharT* __k2)
      {

                      ;
 ;
 return this->replace(__i1 - begin(), __i2 - __i1,
        __k1, __k2 - __k1);
      }

      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2,
       const _CharT* __k1, const _CharT* __k2)
      {

                      ;
 ;
 return this->replace(__i1 - begin(), __i2 - __i1,
        __k1, __k2 - __k1);
      }

      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2,
       iterator __k1, iterator __k2)
      {

                      ;
 ;
 return this->replace(__i1 - begin(), __i2 - __i1,
        __k1.base(), __k2 - __k1);
      }

      basic_string&
      replace(__const_iterator __i1, __const_iterator __i2,
       const_iterator __k1, const_iterator __k2)
      {

                      ;
 ;
 return this->replace(__i1 - begin(), __i2 - __i1,
        __k1.base(), __k2 - __k1);
      }
# 2201 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string& replace(const_iterator __i1, const_iterator __i2,
       initializer_list<_CharT> __l)
      { return this->replace(__i1, __i2, __l.begin(), __l.size()); }
# 2214 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 replace(size_type __pos, size_type __n, const _Tp& __svt)
 {
   __sv_type __sv = __svt;
   return this->replace(__pos, __n, __sv.data(), __sv.size());
 }
# 2231 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 replace(size_type __pos1, size_type __n1, const _Tp& __svt,
  size_type __pos2, size_type __n2 = npos)
 {
   __sv_type __sv = __svt;
   return this->replace(__pos1, __n1,
       __sv.data()
       + std::__sv_check(__sv.size(), __pos2, "basic_string::replace"),
       std::__sv_limit(__sv.size(), __pos2, __n2));
 }
# 2252 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, basic_string&>
 replace(const_iterator __i1, const_iterator __i2, const _Tp& __svt)
 {
   __sv_type __sv = __svt;
   return this->replace(__i1 - begin(), __i2 - __i1, __sv);
 }


    private:
      template<class _Integer>
 basic_string&
 _M_replace_dispatch(const_iterator __i1, const_iterator __i2,
       _Integer __n, _Integer __val, __true_type)
        { return _M_replace_aux(__i1 - begin(), __i2 - __i1, __n, __val); }

      template<class _InputIterator>
 basic_string&
 _M_replace_dispatch(const_iterator __i1, const_iterator __i2,
       _InputIterator __k1, _InputIterator __k2,
       __false_type);

      basic_string&
      _M_replace_aux(size_type __pos1, size_type __n1, size_type __n2,
       _CharT __c);

      basic_string&
      _M_replace(size_type __pos, size_type __len1, const _CharT* __s,
   const size_type __len2);

      basic_string&
      _M_append(const _CharT* __s, size_type __n);

    public:
# 2299 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      copy(_CharT* __s, size_type __n, size_type __pos = 0) const;
# 2309 "/usr/include/c++/11/bits/basic_string.h" 3
      void
      swap(basic_string& __s) noexcept;
# 2319 "/usr/include/c++/11/bits/basic_string.h" 3
      const _CharT*
      c_str() const noexcept
      { return _M_data(); }
# 2331 "/usr/include/c++/11/bits/basic_string.h" 3
      const _CharT*
      data() const noexcept
      { return _M_data(); }
# 2342 "/usr/include/c++/11/bits/basic_string.h" 3
      _CharT*
      data() noexcept
      { return _M_data(); }





      allocator_type
      get_allocator() const noexcept
      { return _M_get_allocator(); }
# 2366 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find(const _CharT* __s, size_type __pos, size_type __n) const
      noexcept;
# 2380 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find(const basic_string& __str, size_type __pos = 0) const
      noexcept
      { return this->find(__str.data(), __pos, __str.size()); }
# 2392 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, size_type>
 find(const _Tp& __svt, size_type __pos = 0) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return this->find(__sv.data(), __pos, __sv.size());
 }
# 2412 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find(const _CharT* __s, size_type __pos = 0) const noexcept
      {
 ;
 return this->find(__s, __pos, traits_type::length(__s));
      }
# 2429 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find(_CharT __c, size_type __pos = 0) const noexcept;
# 2442 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      rfind(const basic_string& __str, size_type __pos = npos) const
      noexcept
      { return this->rfind(__str.data(), __pos, __str.size()); }
# 2454 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, size_type>
 rfind(const _Tp& __svt, size_type __pos = npos) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return this->rfind(__sv.data(), __pos, __sv.size());
 }
# 2476 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      rfind(const _CharT* __s, size_type __pos, size_type __n) const
      noexcept;
# 2490 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      rfind(const _CharT* __s, size_type __pos = npos) const
      {
 ;
 return this->rfind(__s, __pos, traits_type::length(__s));
      }
# 2507 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      rfind(_CharT __c, size_type __pos = npos) const noexcept;
# 2521 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_of(const basic_string& __str, size_type __pos = 0) const
      noexcept
      { return this->find_first_of(__str.data(), __pos, __str.size()); }
# 2534 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, size_type>
 find_first_of(const _Tp& __svt, size_type __pos = 0) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return this->find_first_of(__sv.data(), __pos, __sv.size());
 }
# 2556 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
      noexcept;
# 2570 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_of(const _CharT* __s, size_type __pos = 0) const
      noexcept
      {
 ;
 return this->find_first_of(__s, __pos, traits_type::length(__s));
      }
# 2590 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_of(_CharT __c, size_type __pos = 0) const noexcept
      { return this->find(__c, __pos); }
# 2605 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_of(const basic_string& __str, size_type __pos = npos) const
      noexcept
      { return this->find_last_of(__str.data(), __pos, __str.size()); }
# 2618 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, size_type>
 find_last_of(const _Tp& __svt, size_type __pos = npos) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return this->find_last_of(__sv.data(), __pos, __sv.size());
 }
# 2640 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
      noexcept;
# 2654 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_of(const _CharT* __s, size_type __pos = npos) const
      noexcept
      {
 ;
 return this->find_last_of(__s, __pos, traits_type::length(__s));
      }
# 2674 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_of(_CharT __c, size_type __pos = npos) const noexcept
      { return this->rfind(__c, __pos); }
# 2688 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_not_of(const basic_string& __str, size_type __pos = 0) const
      noexcept
      { return this->find_first_not_of(__str.data(), __pos, __str.size()); }
# 2701 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, size_type>
 find_first_not_of(const _Tp& __svt, size_type __pos = 0) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return this->find_first_not_of(__sv.data(), __pos, __sv.size());
 }
# 2723 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_not_of(const _CharT* __s, size_type __pos,
   size_type __n) const noexcept;
# 2737 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_not_of(const _CharT* __s, size_type __pos = 0) const
      noexcept
      {
 ;
 return this->find_first_not_of(__s, __pos, traits_type::length(__s));
      }
# 2755 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_first_not_of(_CharT __c, size_type __pos = 0) const
      noexcept;
# 2770 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_not_of(const basic_string& __str, size_type __pos = npos) const
      noexcept
      { return this->find_last_not_of(__str.data(), __pos, __str.size()); }
# 2783 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, size_type>
 find_last_not_of(const _Tp& __svt, size_type __pos = npos) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return this->find_last_not_of(__sv.data(), __pos, __sv.size());
 }
# 2805 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_not_of(const _CharT* __s, size_type __pos,
         size_type __n) const noexcept;
# 2819 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_not_of(const _CharT* __s, size_type __pos = npos) const
      noexcept
      {
 ;
 return this->find_last_not_of(__s, __pos, traits_type::length(__s));
      }
# 2837 "/usr/include/c++/11/bits/basic_string.h" 3
      size_type
      find_last_not_of(_CharT __c, size_type __pos = npos) const
      noexcept;
# 2853 "/usr/include/c++/11/bits/basic_string.h" 3
      basic_string
      substr(size_type __pos = 0, size_type __n = npos) const
      { return basic_string(*this,
       _M_check(__pos, "basic_string::substr"), __n); }
# 2872 "/usr/include/c++/11/bits/basic_string.h" 3
      int
      compare(const basic_string& __str) const
      {
 const size_type __size = this->size();
 const size_type __osize = __str.size();
 const size_type __len = std::min(__size, __osize);

 int __r = traits_type::compare(_M_data(), __str.data(), __len);
 if (!__r)
   __r = _S_compare(__size, __osize);
 return __r;
      }







      template<typename _Tp>
 _If_sv<_Tp, int>
 compare(const _Tp& __svt) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   const size_type __size = this->size();
   const size_type __osize = __sv.size();
   const size_type __len = std::min(__size, __osize);

   int __r = traits_type::compare(_M_data(), __sv.data(), __len);
   if (!__r)
     __r = _S_compare(__size, __osize);
   return __r;
 }
# 2915 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, int>
 compare(size_type __pos, size_type __n, const _Tp& __svt) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return __sv_type(*this).substr(__pos, __n).compare(__sv);
 }
# 2934 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename _Tp>
 _If_sv<_Tp, int>
 compare(size_type __pos1, size_type __n1, const _Tp& __svt,
  size_type __pos2, size_type __n2 = npos) const
 noexcept(is_same<_Tp, __sv_type>::value)
 {
   __sv_type __sv = __svt;
   return __sv_type(*this)
     .substr(__pos1, __n1).compare(__sv.substr(__pos2, __n2));
 }
# 2965 "/usr/include/c++/11/bits/basic_string.h" 3
      int
      compare(size_type __pos, size_type __n, const basic_string& __str) const;
# 2991 "/usr/include/c++/11/bits/basic_string.h" 3
      int
      compare(size_type __pos1, size_type __n1, const basic_string& __str,
       size_type __pos2, size_type __n2 = npos) const;
# 3009 "/usr/include/c++/11/bits/basic_string.h" 3
      int
      compare(const _CharT* __s) const noexcept;
# 3033 "/usr/include/c++/11/bits/basic_string.h" 3
      int
      compare(size_type __pos, size_type __n1, const _CharT* __s) const;
# 3060 "/usr/include/c++/11/bits/basic_string.h" 3
      int
      compare(size_type __pos, size_type __n1, const _CharT* __s,
       size_type __n2) const;
# 3105 "/usr/include/c++/11/bits/basic_string.h" 3
      template<typename, typename, typename> friend class basic_stringbuf;
    };
}
# 6058 "/usr/include/c++/11/bits/basic_string.h" 3
namespace __cxx11 {
  template<typename _InputIterator, typename _CharT
      = typename iterator_traits<_InputIterator>::value_type,
    typename _Allocator = allocator<_CharT>,
    typename = _RequireInputIter<_InputIterator>,
    typename = _RequireAllocator<_Allocator>>
    basic_string(_InputIterator, _InputIterator, _Allocator = _Allocator())
      -> basic_string<_CharT, char_traits<_CharT>, _Allocator>;



  template<typename _CharT, typename _Traits,
    typename _Allocator = allocator<_CharT>,
    typename = _RequireAllocator<_Allocator>>
    basic_string(basic_string_view<_CharT, _Traits>, const _Allocator& = _Allocator())
      -> basic_string<_CharT, _Traits, _Allocator>;

  template<typename _CharT, typename _Traits,
    typename _Allocator = allocator<_CharT>,
    typename = _RequireAllocator<_Allocator>>
    basic_string(basic_string_view<_CharT, _Traits>,
   typename basic_string<_CharT, _Traits, _Allocator>::size_type,
   typename basic_string<_CharT, _Traits, _Allocator>::size_type,
   const _Allocator& = _Allocator())
      -> basic_string<_CharT, _Traits, _Allocator>;
}
# 6093 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT, _Traits, _Alloc>
    operator+(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    {
      basic_string<_CharT, _Traits, _Alloc> __str(__lhs);
      __str.append(__rhs);
      return __str;
    }







  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT,_Traits,_Alloc>
    operator+(const _CharT* __lhs,
       const basic_string<_CharT,_Traits,_Alloc>& __rhs);







  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT,_Traits,_Alloc>
    operator+(_CharT __lhs, const basic_string<_CharT,_Traits,_Alloc>& __rhs);







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       const _CharT* __rhs)
    {
      basic_string<_CharT, _Traits, _Alloc> __str(__lhs);
      __str.append(__rhs);
      return __str;
    }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(const basic_string<_CharT, _Traits, _Alloc>& __lhs, _CharT __rhs)
    {
      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef typename __string_type::size_type __size_type;
      __string_type __str(__lhs);
      __str.append(__size_type(1), __rhs);
      return __str;
    }


  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(basic_string<_CharT, _Traits, _Alloc>&& __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return std::move(__lhs.append(__rhs)); }

  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       basic_string<_CharT, _Traits, _Alloc>&& __rhs)
    { return std::move(__rhs.insert(0, __lhs)); }

  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(basic_string<_CharT, _Traits, _Alloc>&& __lhs,
       basic_string<_CharT, _Traits, _Alloc>&& __rhs)
    {

      using _Alloc_traits = allocator_traits<_Alloc>;
      bool __use_rhs = false;
      if constexpr (typename _Alloc_traits::is_always_equal{})
 __use_rhs = true;
      else if (__lhs.get_allocator() == __rhs.get_allocator())
 __use_rhs = true;
      if (__use_rhs)

 {
   const auto __size = __lhs.size() + __rhs.size();
   if (__size > __lhs.capacity() && __size <= __rhs.capacity())
     return std::move(__rhs.insert(0, __lhs));
 }
      return std::move(__lhs.append(__rhs));
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(const _CharT* __lhs,
       basic_string<_CharT, _Traits, _Alloc>&& __rhs)
    { return std::move(__rhs.insert(0, __lhs)); }

  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(_CharT __lhs,
       basic_string<_CharT, _Traits, _Alloc>&& __rhs)
    { return std::move(__rhs.insert(0, 1, __lhs)); }

  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(basic_string<_CharT, _Traits, _Alloc>&& __lhs,
       const _CharT* __rhs)
    { return std::move(__lhs.append(__rhs)); }

  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_string<_CharT, _Traits, _Alloc>
    operator+(basic_string<_CharT, _Traits, _Alloc>&& __lhs,
       _CharT __rhs)
    { return std::move(__lhs.append(1, __rhs)); }
# 6224 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator==(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept
    { return __lhs.compare(__rhs) == 0; }

  template<typename _CharT>
    inline
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value, bool>::__type
    operator==(const basic_string<_CharT>& __lhs,
        const basic_string<_CharT>& __rhs) noexcept
    { return (__lhs.size() == __rhs.size()
       && !std::char_traits<_CharT>::compare(__lhs.data(), __rhs.data(),
          __lhs.size())); }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator==(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const _CharT* __rhs)
    { return __lhs.compare(__rhs) == 0; }
# 6287 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator==(const _CharT* __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return __rhs.compare(__lhs) == 0; }
# 6300 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator!=(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept
    { return !(__lhs == __rhs); }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator!=(const _CharT* __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return !(__lhs == __rhs); }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator!=(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const _CharT* __rhs)
    { return !(__lhs == __rhs); }
# 6338 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator<(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept
    { return __lhs.compare(__rhs) < 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator<(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       const _CharT* __rhs)
    { return __lhs.compare(__rhs) < 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator<(const _CharT* __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return __rhs.compare(__lhs) > 0; }
# 6376 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator>(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept
    { return __lhs.compare(__rhs) > 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator>(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
       const _CharT* __rhs)
    { return __lhs.compare(__rhs) > 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator>(const _CharT* __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return __rhs.compare(__lhs) < 0; }
# 6414 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator<=(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept
    { return __lhs.compare(__rhs) <= 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator<=(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const _CharT* __rhs)
    { return __lhs.compare(__rhs) <= 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator<=(const _CharT* __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return __rhs.compare(__lhs) >= 0; }
# 6452 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator>=(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept
    { return __lhs.compare(__rhs) >= 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator>=(const basic_string<_CharT, _Traits, _Alloc>& __lhs,
        const _CharT* __rhs)
    { return __lhs.compare(__rhs) >= 0; }







  template<typename _CharT, typename _Traits, typename _Alloc>
    inline bool
    operator>=(const _CharT* __lhs,
      const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    { return __rhs.compare(__lhs) <= 0; }
# 6491 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline void
    swap(basic_string<_CharT, _Traits, _Alloc>& __lhs,
  basic_string<_CharT, _Traits, _Alloc>& __rhs)
    noexcept(noexcept(__lhs.swap(__rhs)))
    { __lhs.swap(__rhs); }
# 6511 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_istream<_CharT, _Traits>&
    operator>>(basic_istream<_CharT, _Traits>& __is,
        basic_string<_CharT, _Traits, _Alloc>& __str);

  template<>
    basic_istream<char>&
    operator>>(basic_istream<char>& __is, basic_string<char>& __str);
# 6529 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __os,
        const basic_string<_CharT, _Traits, _Alloc>& __str)
    {


      return __ostream_insert(__os, __str.data(), __str.size());
    }
# 6552 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_istream<_CharT, _Traits>&
    getline(basic_istream<_CharT, _Traits>& __is,
     basic_string<_CharT, _Traits, _Alloc>& __str, _CharT __delim);
# 6569 "/usr/include/c++/11/bits/basic_string.h" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_istream<_CharT, _Traits>&
    getline(basic_istream<_CharT, _Traits>& __is,
     basic_string<_CharT, _Traits, _Alloc>& __str)
    { return std::getline(__is, __str, __is.widen('\n')); }



  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_istream<_CharT, _Traits>&
    getline(basic_istream<_CharT, _Traits>&& __is,
     basic_string<_CharT, _Traits, _Alloc>& __str, _CharT __delim)
    { return std::getline(__is, __str, __delim); }


  template<typename _CharT, typename _Traits, typename _Alloc>
    inline basic_istream<_CharT, _Traits>&
    getline(basic_istream<_CharT, _Traits>&& __is,
     basic_string<_CharT, _Traits, _Alloc>& __str)
    { return std::getline(__is, __str); }


  template<>
    basic_istream<char>&
    getline(basic_istream<char>& __in, basic_string<char>& __str,
     char __delim);


  template<>
    basic_istream<wchar_t>&
    getline(basic_istream<wchar_t>& __in, basic_string<wchar_t>& __str,
     wchar_t __delim);



}



# 1 "/usr/include/c++/11/ext/string_conversions.h" 1 3
# 32 "/usr/include/c++/11/ext/string_conversions.h" 3
       
# 33 "/usr/include/c++/11/ext/string_conversions.h" 3
# 41 "/usr/include/c++/11/ext/string_conversions.h" 3
# 1 "/usr/include/c++/11/cstdlib" 1 3
# 39 "/usr/include/c++/11/cstdlib" 3
       
# 40 "/usr/include/c++/11/cstdlib" 3
# 75 "/usr/include/c++/11/cstdlib" 3
# 1 "/usr/include/stdlib.h" 1 3 4
# 25 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 26 "/usr/include/stdlib.h" 2 3 4





# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 32 "/usr/include/stdlib.h" 2 3 4

extern "C" {





# 1 "/usr/include/bits/waitflags.h" 1 3 4
# 40 "/usr/include/stdlib.h" 2 3 4
# 1 "/usr/include/bits/waitstatus.h" 1 3 4
# 41 "/usr/include/stdlib.h" 2 3 4
# 58 "/usr/include/stdlib.h" 3 4
typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;





__extension__ typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
# 97 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) noexcept (true) ;



extern double atof (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern int atoi (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;

extern long int atol (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;



__extension__ extern long long int atoll (const char *__nptr)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;



extern double strtod (const char *__restrict __nptr,
        char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern float strtof (const char *__restrict __nptr,
       char **__restrict __endptr) noexcept (true) __attribute__ ((__nonnull__ (1)));

extern long double strtold (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 140 "/usr/include/stdlib.h" 3 4
extern _Float32 strtof32 (const char *__restrict __nptr,
     char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float64 strtof64 (const char *__restrict __nptr,
     char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float128 strtof128 (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float32x strtof32x (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



extern _Float64x strtof64x (const char *__restrict __nptr,
       char **__restrict __endptr)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 176 "/usr/include/stdlib.h" 3 4
extern long int strtol (const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));

extern unsigned long int strtoul (const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));



__extension__
extern long long int strtoq (const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int strtoll (const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     noexcept (true) __attribute__ ((__nonnull__ (1)));




extern int strfromd (char *__dest, size_t __size, const char *__format,
       double __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));

extern int strfromf (char *__dest, size_t __size, const char *__format,
       float __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));

extern int strfroml (char *__dest, size_t __size, const char *__format,
       long double __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 232 "/usr/include/stdlib.h" 3 4
extern int strfromf32 (char *__dest, size_t __size, const char * __format,
         _Float32 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf64 (char *__dest, size_t __size, const char * __format,
         _Float64 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf128 (char *__dest, size_t __size, const char * __format,
   _Float128 __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf32x (char *__dest, size_t __size, const char * __format,
   _Float32x __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));



extern int strfromf64x (char *__dest, size_t __size, const char * __format,
   _Float64x __f)
     noexcept (true) __attribute__ ((__nonnull__ (3)));
# 274 "/usr/include/stdlib.h" 3 4
extern long int strtol_l (const char *__restrict __nptr,
     char **__restrict __endptr, int __base,
     locale_t __loc) noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

extern unsigned long int strtoul_l (const char *__restrict __nptr,
        char **__restrict __endptr,
        int __base, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

__extension__
extern long long int strtoll_l (const char *__restrict __nptr,
    char **__restrict __endptr, int __base,
    locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

__extension__
extern unsigned long long int strtoull_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 4)));

extern double strtod_l (const char *__restrict __nptr,
   char **__restrict __endptr, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));

extern float strtof_l (const char *__restrict __nptr,
         char **__restrict __endptr, locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));

extern long double strtold_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 316 "/usr/include/stdlib.h" 3 4
extern _Float32 strtof32_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float64 strtof64_l (const char *__restrict __nptr,
       char **__restrict __endptr,
       locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float128 strtof128_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float32x strtof32x_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));



extern _Float64x strtof64x_l (const char *__restrict __nptr,
         char **__restrict __endptr,
         locale_t __loc)
     noexcept (true) __attribute__ ((__nonnull__ (1, 3)));
# 385 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) noexcept (true) ;


extern long int a64l (const char *__s)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1))) ;




# 1 "/usr/include/sys/types.h" 1 3 4
# 27 "/usr/include/sys/types.h" 3 4
extern "C" {





typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;


typedef __loff_t loff_t;




typedef __ino_t ino_t;






typedef __ino64_t ino64_t;




typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;






typedef __off64_t off64_t;
# 103 "/usr/include/sys/types.h" 3 4
typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;
# 134 "/usr/include/sys/types.h" 3 4
typedef __useconds_t useconds_t;



typedef __suseconds_t suseconds_t;





# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 145 "/usr/include/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;







typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;


typedef int register_t __attribute__ ((__mode__ (__word__)));
# 176 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 35 "/usr/include/endian.h" 3 4
# 1 "/usr/include/bits/byteswap.h" 1 3 4
# 33 "/usr/include/bits/byteswap.h" 3 4
static __inline __uint16_t
__bswap_16 (__uint16_t __bsx)
{

  return __builtin_bswap16 (__bsx);



}






static __inline __uint32_t
__bswap_32 (__uint32_t __bsx)
{

  return __builtin_bswap32 (__bsx);



}
# 69 "/usr/include/bits/byteswap.h" 3 4
__extension__ static __inline __uint64_t
__bswap_64 (__uint64_t __bsx)
{

  return __builtin_bswap64 (__bsx);



}
# 36 "/usr/include/endian.h" 2 3 4
# 1 "/usr/include/bits/uintn-identity.h" 1 3 4
# 32 "/usr/include/bits/uintn-identity.h" 3 4
static __inline __uint16_t
__uint16_identity (__uint16_t __x)
{
  return __x;
}

static __inline __uint32_t
__uint32_identity (__uint32_t __x)
{
  return __x;
}

static __inline __uint64_t
__uint64_identity (__uint64_t __x)
{
  return __x;
}
# 37 "/usr/include/endian.h" 2 3 4
# 177 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/select.h" 1 3 4
# 30 "/usr/include/sys/select.h" 3 4
# 1 "/usr/include/bits/select.h" 1 3 4
# 31 "/usr/include/sys/select.h" 2 3 4


# 1 "/usr/include/bits/types/sigset_t.h" 1 3 4






typedef __sigset_t sigset_t;
# 34 "/usr/include/sys/select.h" 2 3 4
# 49 "/usr/include/sys/select.h" 3 4
typedef long int __fd_mask;
# 59 "/usr/include/sys/select.h" 3 4
typedef struct
  {



    __fd_mask fds_bits[1024 / (8 * (int) sizeof (__fd_mask))];





  } fd_set;






typedef __fd_mask fd_mask;
# 91 "/usr/include/sys/select.h" 3 4
extern "C" {
# 102 "/usr/include/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 127 "/usr/include/sys/select.h" 3 4
extern int pselect (int __nfds, fd_set *__restrict __readfds,
      fd_set *__restrict __writefds,
      fd_set *__restrict __exceptfds,
      const struct timespec *__restrict __timeout,
      const __sigset_t *__restrict __sigmask);
# 153 "/usr/include/sys/select.h" 3 4
}
# 180 "/usr/include/sys/types.h" 2 3 4





typedef __blksize_t blksize_t;






typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 219 "/usr/include/sys/types.h" 3 4
typedef __blkcnt64_t blkcnt64_t;
typedef __fsblkcnt64_t fsblkcnt64_t;
typedef __fsfilcnt64_t fsfilcnt64_t;
# 230 "/usr/include/sys/types.h" 3 4
}
# 395 "/usr/include/stdlib.h" 2 3 4






extern long int random (void) noexcept (true);


extern void srandom (unsigned int __seed) noexcept (true);





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) noexcept (true) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) noexcept (true) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));





extern int rand (void) noexcept (true);

extern void srand (unsigned int __seed) noexcept (true);



extern int rand_r (unsigned int *__seed) noexcept (true);







extern double drand48 (void) noexcept (true);
extern double erand48 (unsigned short int __xsubi[3]) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) noexcept (true);
extern long int nrand48 (unsigned short int __xsubi[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) noexcept (true);
extern long int jrand48 (unsigned short int __xsubi[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) noexcept (true);
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     noexcept (true) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) noexcept (true) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    __extension__ unsigned long long int __a;

  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     noexcept (true) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern void *malloc (size_t __size) noexcept (true) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) ;

extern void *calloc (size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (1, 2))) ;






extern void *realloc (void *__ptr, size_t __size)
     noexcept (true) __attribute__ ((__warn_unused_result__)) __attribute__ ((__alloc_size__ (2)));


extern void free (void *__ptr) noexcept (true);







extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__warn_unused_result__))
     __attribute__ ((__alloc_size__ (2, 3)))
    __attribute__ ((__malloc__ (__builtin_free, 1)));


extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     noexcept (true) __attribute__ ((__malloc__ (reallocarray, 1)));



# 1 "/usr/include/alloca.h" 1 3 4
# 24 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 25 "/usr/include/alloca.h" 2 3 4

extern "C" {





extern void *alloca (size_t __size) noexcept (true);





}
# 574 "/usr/include/stdlib.h" 2 3 4





extern void *valloc (size_t __size) noexcept (true) __attribute__ ((__malloc__))
     __attribute__ ((__alloc_size__ (1))) ;




extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     noexcept (true) __attribute__ ((__nonnull__ (1))) ;




extern void *aligned_alloc (size_t __alignment, size_t __size)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (2))) ;



extern void abort (void) noexcept (true) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) noexcept (true) __attribute__ ((__nonnull__ (1)));




extern "C++" int at_quick_exit (void (*__func) (void))
     noexcept (true) __asm ("at_quick_exit") __attribute__ ((__nonnull__ (1)));
# 615 "/usr/include/stdlib.h" 3 4
extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     noexcept (true) __attribute__ ((__nonnull__ (1)));





extern void exit (int __status) noexcept (true) __attribute__ ((__noreturn__));





extern void quick_exit (int __status) noexcept (true) __attribute__ ((__noreturn__));





extern void _Exit (int __status) noexcept (true) __attribute__ ((__noreturn__));




extern char *getenv (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1))) ;




extern char *secure_getenv (const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (1))) ;






extern int putenv (char *__string) noexcept (true) __attribute__ ((__nonnull__ (1)));





extern int setenv (const char *__name, const char *__value, int __replace)
     noexcept (true) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (const char *__name) noexcept (true) __attribute__ ((__nonnull__ (1)));






extern int clearenv (void) noexcept (true);
# 680 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) noexcept (true) __attribute__ ((__nonnull__ (1)));
# 693 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1))) ;
# 703 "/usr/include/stdlib.h" 3 4
extern int mkstemp64 (char *__template) __attribute__ ((__nonnull__ (1))) ;
# 715 "/usr/include/stdlib.h" 3 4
extern int mkstemps (char *__template, int __suffixlen) __attribute__ ((__nonnull__ (1))) ;
# 725 "/usr/include/stdlib.h" 3 4
extern int mkstemps64 (char *__template, int __suffixlen)
     __attribute__ ((__nonnull__ (1))) ;
# 736 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) noexcept (true) __attribute__ ((__nonnull__ (1))) ;
# 747 "/usr/include/stdlib.h" 3 4
extern int mkostemp (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) ;
# 757 "/usr/include/stdlib.h" 3 4
extern int mkostemp64 (char *__template, int __flags) __attribute__ ((__nonnull__ (1))) ;
# 767 "/usr/include/stdlib.h" 3 4
extern int mkostemps (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) ;
# 779 "/usr/include/stdlib.h" 3 4
extern int mkostemps64 (char *__template, int __suffixlen, int __flags)
     __attribute__ ((__nonnull__ (1))) ;
# 789 "/usr/include/stdlib.h" 3 4
extern int system (const char *__command) ;





extern char *canonicalize_file_name (const char *__name)
     noexcept (true) __attribute__ ((__nonnull__ (1))) __attribute__ ((__malloc__))
     __attribute__ ((__malloc__ (__builtin_free, 1))) ;
# 806 "/usr/include/stdlib.h" 3 4
extern char *realpath (const char *__restrict __name,
         char *__restrict __resolved) noexcept (true) ;






typedef int (*__compar_fn_t) (const void *, const void *);


typedef __compar_fn_t comparison_fn_t;



typedef int (*__compar_d_fn_t) (const void *, const void *, void *);




extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5))) ;







extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));

extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
       __compar_d_fn_t __compar, void *__arg)
  __attribute__ ((__nonnull__ (1, 4)));




extern int abs (int __x) noexcept (true) __attribute__ ((__const__)) ;
extern long int labs (long int __x) noexcept (true) __attribute__ ((__const__)) ;


__extension__ extern long long int llabs (long long int __x)
     noexcept (true) __attribute__ ((__const__)) ;






extern div_t div (int __numer, int __denom)
     noexcept (true) __attribute__ ((__const__)) ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     noexcept (true) __attribute__ ((__const__)) ;


__extension__ extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     noexcept (true) __attribute__ ((__const__)) ;
# 878 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (3))) ;




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4))) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     noexcept (true) __attribute__ ((__nonnull__ (3))) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     noexcept (true) __attribute__ ((__nonnull__ (3, 4, 5)));





extern int mblen (const char *__s, size_t __n) noexcept (true);


extern int mbtowc (wchar_t *__restrict __pwc,
     const char *__restrict __s, size_t __n) noexcept (true);


extern int wctomb (char *__s, wchar_t __wchar) noexcept (true);



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   const char *__restrict __s, size_t __n) noexcept (true)
    __attribute__ ((__access__ (__read_only__, 2)));

extern size_t wcstombs (char *__restrict __s,
   const wchar_t *__restrict __pwcs, size_t __n)
     noexcept (true)
  __attribute__ ((__access__ (__write_only__, 1, 3)))
  __attribute__ ((__access__ (__read_only__, 2)));






extern int rpmatch (const char *__response) noexcept (true) __attribute__ ((__nonnull__ (1))) ;
# 965 "/usr/include/stdlib.h" 3 4
extern int getsubopt (char **__restrict __optionp,
        char *const *__restrict __tokens,
        char **__restrict __valuep)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2, 3))) ;







extern int posix_openpt (int __oflag) ;







extern int grantpt (int __fd) noexcept (true);



extern int unlockpt (int __fd) noexcept (true);




extern char *ptsname (int __fd) noexcept (true) ;






extern int ptsname_r (int __fd, char *__buf, size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 2, 3)));


extern int getpt (void);






extern int getloadavg (double __loadavg[], int __nelem)
     noexcept (true) __attribute__ ((__nonnull__ (1)));
# 1021 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/bits/stdlib-float.h" 1 3 4
# 1022 "/usr/include/stdlib.h" 2 3 4
# 1033 "/usr/include/stdlib.h" 3 4
}
# 76 "/usr/include/c++/11/cstdlib" 2 3

# 1 "/usr/include/c++/11/bits/std_abs.h" 1 3
# 33 "/usr/include/c++/11/bits/std_abs.h" 3
       
# 34 "/usr/include/c++/11/bits/std_abs.h" 3
# 46 "/usr/include/c++/11/bits/std_abs.h" 3
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{


  using ::abs;


  inline long
  abs(long __i) { return __builtin_labs(__i); }



  inline long long
  abs(long long __x) { return __builtin_llabs (__x); }
# 70 "/usr/include/c++/11/bits/std_abs.h" 3
  inline constexpr double
  abs(double __x)
  { return __builtin_fabs(__x); }

  inline constexpr float
  abs(float __x)
  { return __builtin_fabsf(__x); }

  inline constexpr long double
  abs(long double __x)
  { return __builtin_fabsl(__x); }
# 107 "/usr/include/c++/11/bits/std_abs.h" 3

}
}
# 78 "/usr/include/c++/11/cstdlib" 2 3
# 121 "/usr/include/c++/11/cstdlib" 3
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{


  using ::div_t;
  using ::ldiv_t;

  using ::abort;

  using ::aligned_alloc;

  using ::atexit;


  using ::at_quick_exit;


  using ::atof;
  using ::atoi;
  using ::atol;
  using ::bsearch;
  using ::calloc;
  using ::div;
  using ::exit;
  using ::free;
  using ::getenv;
  using ::labs;
  using ::ldiv;
  using ::malloc;

  using ::mblen;
  using ::mbstowcs;
  using ::mbtowc;

  using ::qsort;


  using ::quick_exit;


  using ::rand;
  using ::realloc;
  using ::srand;
  using ::strtod;
  using ::strtol;
  using ::strtoul;
  using ::system;

  using ::wcstombs;
  using ::wctomb;



  inline ldiv_t
  div(long __i, long __j) { return ldiv(__i, __j); }




}
# 195 "/usr/include/c++/11/cstdlib" 3
namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{



  using ::lldiv_t;





  using ::_Exit;



  using ::llabs;

  inline lldiv_t
  div(long long __n, long long __d)
  { lldiv_t __q; __q.quot = __n / __d; __q.rem = __n % __d; return __q; }

  using ::lldiv;
# 227 "/usr/include/c++/11/cstdlib" 3
  using ::atoll;
  using ::strtoll;
  using ::strtoull;

  using ::strtof;
  using ::strtold;


}

namespace std
{

  using ::__gnu_cxx::lldiv_t;

  using ::__gnu_cxx::_Exit;

  using ::__gnu_cxx::llabs;
  using ::__gnu_cxx::div;
  using ::__gnu_cxx::lldiv;

  using ::__gnu_cxx::atoll;
  using ::__gnu_cxx::strtof;
  using ::__gnu_cxx::strtoll;
  using ::__gnu_cxx::strtoull;
  using ::__gnu_cxx::strtold;
}



}
# 42 "/usr/include/c++/11/ext/string_conversions.h" 2 3
# 1 "/usr/include/c++/11/cwchar" 1 3
# 39 "/usr/include/c++/11/cwchar" 3
       
# 40 "/usr/include/c++/11/cwchar" 3
# 43 "/usr/include/c++/11/ext/string_conversions.h" 2 3
# 1 "/usr/include/c++/11/cstdio" 1 3
# 39 "/usr/include/c++/11/cstdio" 3
       
# 40 "/usr/include/c++/11/cstdio" 3


# 1 "/usr/include/stdio.h" 1 3 4
# 27 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 28 "/usr/include/stdio.h" 2 3 4

extern "C" {



# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 34 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stdarg.h" 1 3 4
# 37 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/include/bits/types/__fpos_t.h" 1 3 4
# 10 "/usr/include/bits/types/__fpos_t.h" 3 4
typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
# 40 "/usr/include/stdio.h" 2 3 4
# 1 "/usr/include/bits/types/__fpos64_t.h" 1 3 4
# 10 "/usr/include/bits/types/__fpos64_t.h" 3 4
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;
# 41 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/include/bits/types/struct_FILE.h" 1 3 4
# 35 "/usr/include/bits/types/struct_FILE.h" 3 4
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;




typedef void _IO_lock_t;





struct _IO_FILE
{
  int _flags;


  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;


  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2;
  __off_t _old_offset;


  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;







  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  size_t __pad5;
  int _mode;

  char _unused2[15 * sizeof (int) - 4 * sizeof (void *) - sizeof (size_t)];
};
# 44 "/usr/include/stdio.h" 2 3 4


# 1 "/usr/include/bits/types/cookie_io_functions_t.h" 1 3 4
# 27 "/usr/include/bits/types/cookie_io_functions_t.h" 3 4
typedef __ssize_t cookie_read_function_t (void *__cookie, char *__buf,
                                          size_t __nbytes);







typedef __ssize_t cookie_write_function_t (void *__cookie, const char *__buf,
                                           size_t __nbytes);







typedef int cookie_seek_function_t (void *__cookie, __off64_t *__pos, int __w);


typedef int cookie_close_function_t (void *__cookie);






typedef struct _IO_cookie_io_functions_t
{
  cookie_read_function_t *read;
  cookie_write_function_t *write;
  cookie_seek_function_t *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
# 47 "/usr/include/stdio.h" 2 3 4





typedef __gnuc_va_list va_list;
# 84 "/usr/include/stdio.h" 3 4
typedef __fpos_t fpos_t;




typedef __fpos64_t fpos64_t;
# 133 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/stdio_lim.h" 1 3 4
# 134 "/usr/include/stdio.h" 2 3 4



extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;






extern int remove (const char *__filename) noexcept (true);

extern int rename (const char *__old, const char *__new) noexcept (true);



extern int renameat (int __oldfd, const char *__old, int __newfd,
       const char *__new) noexcept (true);
# 164 "/usr/include/stdio.h" 3 4
extern int renameat2 (int __oldfd, const char *__old, int __newfd,
        const char *__new, unsigned int __flags) noexcept (true);






extern int fclose (FILE *__stream);
# 182 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile (void)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
# 194 "/usr/include/stdio.h" 3 4
extern FILE *tmpfile64 (void)
   __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;



extern char *tmpnam (char[20]) noexcept (true) ;




extern char *tmpnam_r (char __s[20]) noexcept (true) ;
# 216 "/usr/include/stdio.h" 3 4
extern char *tempnam (const char *__dir, const char *__pfx)
   noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (__builtin_free, 1)));






extern int fflush (FILE *__stream);
# 233 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 243 "/usr/include/stdio.h" 3 4
extern int fcloseall (void);
# 252 "/usr/include/stdio.h" 3 4
extern FILE *fopen (const char *__restrict __filename,
      const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *freopen (const char *__restrict __filename,
        const char *__restrict __modes,
        FILE *__restrict __stream) ;
# 277 "/usr/include/stdio.h" 3 4
extern FILE *fopen64 (const char *__restrict __filename,
        const char *__restrict __modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;
extern FILE *freopen64 (const char *__restrict __filename,
   const char *__restrict __modes,
   FILE *__restrict __stream) ;




extern FILE *fdopen (int __fd, const char *__modes) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;





extern FILE *fopencookie (void *__restrict __magic_cookie,
     const char *__restrict __modes,
     cookie_io_functions_t __io_funcs) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;




extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1))) ;





extern __FILE *open_wmemstream (wchar_t **__bufloc, size_t *__sizeloc) noexcept (true)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (fclose, 1)));





extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) noexcept (true);



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) noexcept (true);




extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) noexcept (true);


extern void setlinebuf (FILE *__stream) noexcept (true);







extern int fprintf (FILE *__restrict __stream,
      const char *__restrict __format, ...);




extern int printf (const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      const char *__restrict __format, ...) noexcept (true);





extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, const char *__restrict __format,
       __gnuc_va_list __arg) noexcept (true);



extern int snprintf (char *__restrict __s, size_t __maxlen,
       const char *__restrict __format, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        const char *__restrict __format, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 3, 0)));





extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
        __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 0))) ;
extern int __asprintf (char **__restrict __ptr,
         const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3))) ;
extern int asprintf (char **__restrict __ptr,
       const char *__restrict __fmt, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3))) ;




extern int vdprintf (int __fd, const char *__restrict __fmt,
       __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));







extern int fscanf (FILE *__restrict __stream,
     const char *__restrict __format, ...) ;




extern int scanf (const char *__restrict __format, ...) ;

extern int sscanf (const char *__restrict __s,
     const char *__restrict __format, ...) noexcept (true);
# 428 "/usr/include/stdio.h" 3 4
extern int fscanf (FILE *__restrict __stream, const char *__restrict __format, ...) __asm__ ("" "__isoc99_fscanf")

                               ;
extern int scanf (const char *__restrict __format, ...) __asm__ ("" "__isoc99_scanf")
                              ;
extern int sscanf (const char *__restrict __s, const char *__restrict __format, ...) noexcept (true) __asm__ ("" "__isoc99_sscanf")

                      ;
# 453 "/usr/include/stdio.h" 3 4
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
      __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) ;





extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) ;


extern int vsscanf (const char *__restrict __s,
      const char *__restrict __format, __gnuc_va_list __arg)
     noexcept (true) __attribute__ ((__format__ (__scanf__, 2, 0)));





extern int vfscanf (FILE *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vfscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0))) ;
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg) __asm__ ("" "__isoc99_vscanf")

     __attribute__ ((__format__ (__scanf__, 1, 0))) ;
extern int vsscanf (const char *__restrict __s, const char *__restrict __format, __gnuc_va_list __arg) noexcept (true) __asm__ ("" "__isoc99_vsscanf")



     __attribute__ ((__format__ (__scanf__, 2, 0)));
# 507 "/usr/include/stdio.h" 3 4
extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);






extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
# 532 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream);
# 543 "/usr/include/stdio.h" 3 4
extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);
# 559 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);







extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __attribute__ ((__access__ (__write_only__, 1, 2)));
# 609 "/usr/include/stdio.h" 3 4
extern char *fgets_unlocked (char *__restrict __s, int __n,
        FILE *__restrict __stream)
    __attribute__ ((__access__ (__write_only__, 1, 2)));
# 626 "/usr/include/stdio.h" 3 4
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) ;
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) ;







extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) ;







extern int fputs (const char *__restrict __s, FILE *__restrict __stream);





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream) ;




extern size_t fwrite (const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s);
# 685 "/usr/include/stdio.h" 3 4
extern int fputs_unlocked (const char *__restrict __s,
      FILE *__restrict __stream);
# 696 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream) ;
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream);







extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream);
# 730 "/usr/include/stdio.h" 3 4
extern int fseeko (FILE *__stream, __off_t __off, int __whence);




extern __off_t ftello (FILE *__stream) ;
# 754 "/usr/include/stdio.h" 3 4
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, const fpos_t *__pos);
# 773 "/usr/include/stdio.h" 3 4
extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence);
extern __off64_t ftello64 (FILE *__stream) ;
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos);
extern int fsetpos64 (FILE *__stream, const fpos64_t *__pos);



extern void clearerr (FILE *__stream) noexcept (true);

extern int feof (FILE *__stream) noexcept (true) ;

extern int ferror (FILE *__stream) noexcept (true) ;



extern void clearerr_unlocked (FILE *__stream) noexcept (true);
extern int feof_unlocked (FILE *__stream) noexcept (true) ;
extern int ferror_unlocked (FILE *__stream) noexcept (true) ;







extern void perror (const char *__s);




extern int fileno (FILE *__stream) noexcept (true) ;




extern int fileno_unlocked (FILE *__stream) noexcept (true) ;
# 817 "/usr/include/stdio.h" 3 4
extern int pclose (FILE *__stream);





extern FILE *popen (const char *__command, const char *__modes)
  __attribute__ ((__malloc__)) __attribute__ ((__malloc__ (pclose, 1))) ;






extern char *ctermid (char *__s) noexcept (true)
  __attribute__ ((__access__ (__write_only__, 1)));





extern char *cuserid (char *__s)
  __attribute__ ((__access__ (__write_only__, 1)));




struct obstack;


extern int obstack_printf (struct obstack *__restrict __obstack,
      const char *__restrict __format, ...)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 3)));
extern int obstack_vprintf (struct obstack *__restrict __obstack,
       const char *__restrict __format,
       __gnuc_va_list __args)
     noexcept (true) __attribute__ ((__format__ (__printf__, 2, 0)));







extern void flockfile (FILE *__stream) noexcept (true);



extern int ftrylockfile (FILE *__stream) noexcept (true) ;


extern void funlockfile (FILE *__stream) noexcept (true);
# 879 "/usr/include/stdio.h" 3 4
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);
# 903 "/usr/include/stdio.h" 3 4
}
# 43 "/usr/include/c++/11/cstdio" 2 3
# 96 "/usr/include/c++/11/cstdio" 3
namespace std
{
  using ::FILE;
  using ::fpos_t;

  using ::clearerr;
  using ::fclose;
  using ::feof;
  using ::ferror;
  using ::fflush;
  using ::fgetc;
  using ::fgetpos;
  using ::fgets;
  using ::fopen;
  using ::fprintf;
  using ::fputc;
  using ::fputs;
  using ::fread;
  using ::freopen;
  using ::fscanf;
  using ::fseek;
  using ::fsetpos;
  using ::ftell;
  using ::fwrite;
  using ::getc;
  using ::getchar;




  using ::perror;
  using ::printf;
  using ::putc;
  using ::putchar;
  using ::puts;
  using ::remove;
  using ::rename;
  using ::rewind;
  using ::scanf;
  using ::setbuf;
  using ::setvbuf;
  using ::sprintf;
  using ::sscanf;
  using ::tmpfile;

  using ::tmpnam;

  using ::ungetc;
  using ::vfprintf;
  using ::vprintf;
  using ::vsprintf;
}
# 157 "/usr/include/c++/11/cstdio" 3
namespace __gnu_cxx
{
# 175 "/usr/include/c++/11/cstdio" 3
  using ::snprintf;
  using ::vfscanf;
  using ::vscanf;
  using ::vsnprintf;
  using ::vsscanf;

}

namespace std
{
  using ::__gnu_cxx::snprintf;
  using ::__gnu_cxx::vfscanf;
  using ::__gnu_cxx::vscanf;
  using ::__gnu_cxx::vsnprintf;
  using ::__gnu_cxx::vsscanf;
}
# 44 "/usr/include/c++/11/ext/string_conversions.h" 2 3
# 1 "/usr/include/c++/11/cerrno" 1 3
# 39 "/usr/include/c++/11/cerrno" 3
       
# 40 "/usr/include/c++/11/cerrno" 3


# 1 "/usr/include/errno.h" 1 3 4
# 28 "/usr/include/errno.h" 3 4
# 1 "/usr/include/bits/errno.h" 1 3 4
# 26 "/usr/include/bits/errno.h" 3 4
# 1 "/usr/include/linux/errno.h" 1 3 4
# 1 "/usr/include/asm/errno.h" 1 3 4
# 1 "/usr/include/asm-generic/errno.h" 1 3 4




# 1 "/usr/include/asm-generic/errno-base.h" 1 3 4
# 6 "/usr/include/asm-generic/errno.h" 2 3 4
# 2 "/usr/include/asm/errno.h" 2 3 4
# 2 "/usr/include/linux/errno.h" 2 3 4
# 27 "/usr/include/bits/errno.h" 2 3 4
# 29 "/usr/include/errno.h" 2 3 4





extern "C" {


extern int *__errno_location (void) noexcept (true) __attribute__ ((__const__));







extern char *program_invocation_name;
extern char *program_invocation_short_name;

# 1 "/usr/include/bits/types/error_t.h" 1 3 4
# 22 "/usr/include/bits/types/error_t.h" 3 4
typedef int error_t;
# 49 "/usr/include/errno.h" 2 3 4



}
# 43 "/usr/include/c++/11/cerrno" 2 3
# 45 "/usr/include/c++/11/ext/string_conversions.h" 2 3

namespace __gnu_cxx __attribute__ ((__visibility__ ("default")))
{



  template<typename _TRet, typename _Ret = _TRet, typename _CharT,
    typename... _Base>
    _Ret
    __stoa(_TRet (*__convf) (const _CharT*, _CharT**, _Base...),
    const char* __name, const _CharT* __str, std::size_t* __idx,
    _Base... __base)
    {
      _Ret __ret;

      _CharT* __endptr;

      struct _Save_errno {
 _Save_errno() : _M_errno((*__errno_location ())) { (*__errno_location ()) = 0; }
 ~_Save_errno() { if ((*__errno_location ()) == 0) (*__errno_location ()) = _M_errno; }
 int _M_errno;
      } const __save_errno;

      struct _Range_chk {
   static bool
   _S_chk(_TRet, std::false_type) { return false; }

   static bool
   _S_chk(_TRet __val, std::true_type)
   {
     return __val < _TRet(__numeric_traits<int>::__min)
       || __val > _TRet(__numeric_traits<int>::__max);
   }
      };

      const _TRet __tmp = __convf(__str, &__endptr, __base...);

      if (__endptr == __str)
 std::__throw_invalid_argument(__name);
      else if ((*__errno_location ()) == 34
   || _Range_chk::_S_chk(__tmp, std::is_same<_Ret, int>{}))
 std::__throw_out_of_range(__name);
      else
 __ret = __tmp;

      if (__idx)
 *__idx = __endptr - __str;

      return __ret;
    }


  template<typename _String, typename _CharT = typename _String::value_type>
    _String
    __to_xstring(int (*__convf) (_CharT*, std::size_t, const _CharT*,
     __builtin_va_list), std::size_t __n,
   const _CharT* __fmt, ...)
    {


      _CharT* __s = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
         * __n));

      __builtin_va_list __args;
      __builtin_va_start(__args, __fmt);

      const int __len = __convf(__s, __n, __fmt, __args);

      __builtin_va_end(__args);

      return _String(__s, __s + __len);
    }


}
# 6609 "/usr/include/c++/11/bits/basic_string.h" 2 3
# 1 "/usr/include/c++/11/bits/charconv.h" 1 3
# 33 "/usr/include/c++/11/bits/charconv.h" 3
       
# 34 "/usr/include/c++/11/bits/charconv.h" 3





namespace std __attribute__ ((__visibility__ ("default")))
{

namespace __detail
{

  template<typename _Tp>
    constexpr unsigned
    __to_chars_len(_Tp __value, int __base = 10) noexcept
    {
      static_assert(is_integral<_Tp>::value, "implementation bug");
      static_assert(is_unsigned<_Tp>::value, "implementation bug");

      unsigned __n = 1;
      const unsigned __b2 = __base * __base;
      const unsigned __b3 = __b2 * __base;
      const unsigned long __b4 = __b3 * __base;
      for (;;)
 {
   if (__value < (unsigned)__base) return __n;
   if (__value < __b2) return __n + 1;
   if (__value < __b3) return __n + 2;
   if (__value < __b4) return __n + 3;
   __value /= __b4;
   __n += 4;
 }
    }




  template<typename _Tp>
    void
    __to_chars_10_impl(char* __first, unsigned __len, _Tp __val) noexcept
    {
      static_assert(is_integral<_Tp>::value, "implementation bug");
      static_assert(is_unsigned<_Tp>::value, "implementation bug");

      static constexpr char __digits[201] =
 "0001020304050607080910111213141516171819"
 "2021222324252627282930313233343536373839"
 "4041424344454647484950515253545556575859"
 "6061626364656667686970717273747576777879"
 "8081828384858687888990919293949596979899";
      unsigned __pos = __len - 1;
      while (__val >= 100)
 {
   auto const __num = (__val % 100) * 2;
   __val /= 100;
   __first[__pos] = __digits[__num + 1];
   __first[__pos - 1] = __digits[__num];
   __pos -= 2;
 }
      if (__val >= 10)
 {
   auto const __num = __val * 2;
   __first[1] = __digits[__num + 1];
   __first[0] = __digits[__num];
 }
      else
 __first[0] = '0' + __val;
    }

}

}
# 6610 "/usr/include/c++/11/bits/basic_string.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{

namespace __cxx11 {



  inline int
  stoi(const string& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa<long, int>(&std::strtol, "stoi", __str.c_str(),
     __idx, __base); }

  inline long
  stol(const string& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::strtol, "stol", __str.c_str(),
        __idx, __base); }

  inline unsigned long
  stoul(const string& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::strtoul, "stoul", __str.c_str(),
        __idx, __base); }

  inline long long
  stoll(const string& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::strtoll, "stoll", __str.c_str(),
        __idx, __base); }

  inline unsigned long long
  stoull(const string& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::strtoull, "stoull", __str.c_str(),
        __idx, __base); }


  inline float
  stof(const string& __str, size_t* __idx = 0)
  { return __gnu_cxx::__stoa(&std::strtof, "stof", __str.c_str(), __idx); }

  inline double
  stod(const string& __str, size_t* __idx = 0)
  { return __gnu_cxx::__stoa(&std::strtod, "stod", __str.c_str(), __idx); }

  inline long double
  stold(const string& __str, size_t* __idx = 0)
  { return __gnu_cxx::__stoa(&std::strtold, "stold", __str.c_str(), __idx); }




  inline string
  to_string(int __val)
  {
    const bool __neg = __val < 0;
    const unsigned __uval = __neg ? (unsigned)~__val + 1u : __val;
    const auto __len = __detail::__to_chars_len(__uval);
    string __str(__neg + __len, '-');
    __detail::__to_chars_10_impl(&__str[__neg], __len, __uval);
    return __str;
  }

  inline string
  to_string(unsigned __val)
  {
    string __str(__detail::__to_chars_len(__val), '\0');
    __detail::__to_chars_10_impl(&__str[0], __str.size(), __val);
    return __str;
  }

  inline string
  to_string(long __val)
  {
    const bool __neg = __val < 0;
    const unsigned long __uval = __neg ? (unsigned long)~__val + 1ul : __val;
    const auto __len = __detail::__to_chars_len(__uval);
    string __str(__neg + __len, '-');
    __detail::__to_chars_10_impl(&__str[__neg], __len, __uval);
    return __str;
  }

  inline string
  to_string(unsigned long __val)
  {
    string __str(__detail::__to_chars_len(__val), '\0');
    __detail::__to_chars_10_impl(&__str[0], __str.size(), __val);
    return __str;
  }

  inline string
  to_string(long long __val)
  {
    const bool __neg = __val < 0;
    const unsigned long long __uval
      = __neg ? (unsigned long long)~__val + 1ull : __val;
    const auto __len = __detail::__to_chars_len(__uval);
    string __str(__neg + __len, '-');
    __detail::__to_chars_10_impl(&__str[__neg], __len, __uval);
    return __str;
  }

  inline string
  to_string(unsigned long long __val)
  {
    string __str(__detail::__to_chars_len(__val), '\0');
    __detail::__to_chars_10_impl(&__str[0], __str.size(), __val);
    return __str;
  }




  inline string
  to_string(float __val)
  {
    const int __n =
      __gnu_cxx::__numeric_traits<float>::__max_exponent10 + 20;
    return __gnu_cxx::__to_xstring<string>(&std::vsnprintf, __n,
        "%f", __val);
  }

  inline string
  to_string(double __val)
  {
    const int __n =
      __gnu_cxx::__numeric_traits<double>::__max_exponent10 + 20;
    return __gnu_cxx::__to_xstring<string>(&std::vsnprintf, __n,
        "%f", __val);
  }

  inline string
  to_string(long double __val)
  {
    const int __n =
      __gnu_cxx::__numeric_traits<long double>::__max_exponent10 + 20;
    return __gnu_cxx::__to_xstring<string>(&std::vsnprintf, __n,
        "%Lf", __val);
  }



  inline int
  stoi(const wstring& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa<long, int>(&std::wcstol, "stoi", __str.c_str(),
     __idx, __base); }

  inline long
  stol(const wstring& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::wcstol, "stol", __str.c_str(),
        __idx, __base); }

  inline unsigned long
  stoul(const wstring& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::wcstoul, "stoul", __str.c_str(),
        __idx, __base); }

  inline long long
  stoll(const wstring& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::wcstoll, "stoll", __str.c_str(),
        __idx, __base); }

  inline unsigned long long
  stoull(const wstring& __str, size_t* __idx = 0, int __base = 10)
  { return __gnu_cxx::__stoa(&std::wcstoull, "stoull", __str.c_str(),
        __idx, __base); }


  inline float
  stof(const wstring& __str, size_t* __idx = 0)
  { return __gnu_cxx::__stoa(&std::wcstof, "stof", __str.c_str(), __idx); }

  inline double
  stod(const wstring& __str, size_t* __idx = 0)
  { return __gnu_cxx::__stoa(&std::wcstod, "stod", __str.c_str(), __idx); }

  inline long double
  stold(const wstring& __str, size_t* __idx = 0)
  { return __gnu_cxx::__stoa(&std::wcstold, "stold", __str.c_str(), __idx); }



  inline wstring
  to_wstring(int __val)
  { return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf, 4 * sizeof(int),
         L"%d", __val); }

  inline wstring
  to_wstring(unsigned __val)
  { return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf,
         4 * sizeof(unsigned),
         L"%u", __val); }

  inline wstring
  to_wstring(long __val)
  { return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf, 4 * sizeof(long),
         L"%ld", __val); }

  inline wstring
  to_wstring(unsigned long __val)
  { return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf,
         4 * sizeof(unsigned long),
         L"%lu", __val); }

  inline wstring
  to_wstring(long long __val)
  { return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf,
         4 * sizeof(long long),
         L"%lld", __val); }

  inline wstring
  to_wstring(unsigned long long __val)
  { return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf,
         4 * sizeof(unsigned long long),
         L"%llu", __val); }

  inline wstring
  to_wstring(float __val)
  {
    const int __n =
      __gnu_cxx::__numeric_traits<float>::__max_exponent10 + 20;
    return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf, __n,
         L"%f", __val);
  }

  inline wstring
  to_wstring(double __val)
  {
    const int __n =
      __gnu_cxx::__numeric_traits<double>::__max_exponent10 + 20;
    return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf, __n,
         L"%f", __val);
  }

  inline wstring
  to_wstring(long double __val)
  {
    const int __n =
      __gnu_cxx::__numeric_traits<long double>::__max_exponent10 + 20;
    return __gnu_cxx::__to_xstring<wstring>(&std::vswprintf, __n,
         L"%Lf", __val);
  }



}

}







namespace std __attribute__ ((__visibility__ ("default")))
{






  template<>
    struct hash<string>
    : public __hash_base<size_t, string>
    {
      size_t
      operator()(const string& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(), __s.length()); }
    };

  template<>
    struct __is_fast_hash<hash<string>> : std::false_type
    { };



  template<>
    struct hash<wstring>
    : public __hash_base<size_t, wstring>
    {
      size_t
      operator()(const wstring& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(),
                                     __s.length() * sizeof(wchar_t)); }
    };

  template<>
    struct __is_fast_hash<hash<wstring>> : std::false_type
    { };
# 6919 "/usr/include/c++/11/bits/basic_string.h" 3
  template<>
    struct hash<u16string>
    : public __hash_base<size_t, u16string>
    {
      size_t
      operator()(const u16string& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(),
                                     __s.length() * sizeof(char16_t)); }
    };

  template<>
    struct __is_fast_hash<hash<u16string>> : std::false_type
    { };


  template<>
    struct hash<u32string>
    : public __hash_base<size_t, u32string>
    {
      size_t
      operator()(const u32string& __s) const noexcept
      { return std::_Hash_impl::hash(__s.data(),
                                     __s.length() * sizeof(char32_t)); }
    };

  template<>
    struct __is_fast_hash<hash<u32string>> : std::false_type
    { };





  inline namespace literals
  {
  inline namespace string_literals
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wliteral-suffix"
    __attribute ((__abi_tag__ ("cxx11")))
    inline basic_string<char>
    operator""s(const char* __str, size_t __len)
    { return basic_string<char>{__str, __len}; }


    __attribute ((__abi_tag__ ("cxx11")))
    inline basic_string<wchar_t>
    operator""s(const wchar_t* __str, size_t __len)
    { return basic_string<wchar_t>{__str, __len}; }
# 6977 "/usr/include/c++/11/bits/basic_string.h" 3
    __attribute ((__abi_tag__ ("cxx11")))
    inline basic_string<char16_t>
    operator""s(const char16_t* __str, size_t __len)
    { return basic_string<char16_t>{__str, __len}; }

    __attribute ((__abi_tag__ ("cxx11")))
    inline basic_string<char32_t>
    operator""s(const char32_t* __str, size_t __len)
    { return basic_string<char32_t>{__str, __len}; }

#pragma GCC diagnostic pop
  }
  }


  namespace __detail::__variant
  {
    template<typename> struct _Never_valueless_alt;



    template<typename _Tp, typename _Traits, typename _Alloc>
      struct _Never_valueless_alt<std::basic_string<_Tp, _Traits, _Alloc>>
      : __and_<
 is_nothrow_move_constructible<std::basic_string<_Tp, _Traits, _Alloc>>,
 is_nothrow_move_assignable<std::basic_string<_Tp, _Traits, _Alloc>>
 >::type
      { };
  }




}
# 56 "/usr/include/c++/11/string" 2 3
# 1 "/usr/include/c++/11/bits/basic_string.tcc" 1 3
# 42 "/usr/include/c++/11/bits/basic_string.tcc" 3
       
# 43 "/usr/include/c++/11/bits/basic_string.tcc" 3



namespace std __attribute__ ((__visibility__ ("default")))
{




  template<typename _CharT, typename _Traits, typename _Alloc>
    const typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::npos;

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    swap(basic_string& __s) noexcept
    {
      if (this == &__s)
 return;

      _Alloc_traits::_S_on_swap(_M_get_allocator(), __s._M_get_allocator());

      if (_M_is_local())
 if (__s._M_is_local())
   {
     if (length() && __s.length())
       {
  _CharT __tmp_data[_S_local_capacity + 1];
  traits_type::copy(__tmp_data, __s._M_local_buf,
      _S_local_capacity + 1);
  traits_type::copy(__s._M_local_buf, _M_local_buf,
      _S_local_capacity + 1);
  traits_type::copy(_M_local_buf, __tmp_data,
      _S_local_capacity + 1);
       }
     else if (__s.length())
       {
  traits_type::copy(_M_local_buf, __s._M_local_buf,
      _S_local_capacity + 1);
  _M_length(__s.length());
  __s._M_set_length(0);
  return;
       }
     else if (length())
       {
  traits_type::copy(__s._M_local_buf, _M_local_buf,
      _S_local_capacity + 1);
  __s._M_length(length());
  _M_set_length(0);
  return;
       }
   }
 else
   {
     const size_type __tmp_capacity = __s._M_allocated_capacity;
     traits_type::copy(__s._M_local_buf, _M_local_buf,
         _S_local_capacity + 1);
     _M_data(__s._M_data());
     __s._M_data(__s._M_local_buf);
     _M_capacity(__tmp_capacity);
   }
      else
 {
   const size_type __tmp_capacity = _M_allocated_capacity;
   if (__s._M_is_local())
     {
       traits_type::copy(_M_local_buf, __s._M_local_buf,
    _S_local_capacity + 1);
       __s._M_data(_M_data());
       _M_data(_M_local_buf);
     }
   else
     {
       pointer __tmp_ptr = _M_data();
       _M_data(__s._M_data());
       __s._M_data(__tmp_ptr);
       _M_capacity(__s._M_allocated_capacity);
     }
   __s._M_capacity(__tmp_capacity);
 }

      const size_type __tmp_length = length();
      _M_length(__s.length());
      __s._M_length(__tmp_length);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::pointer
    basic_string<_CharT, _Traits, _Alloc>::
    _M_create(size_type& __capacity, size_type __old_capacity)
    {


      if (__capacity > max_size())
 std::__throw_length_error(("basic_string::_M_create"));




      if (__capacity > __old_capacity && __capacity < 2 * __old_capacity)
 {
   __capacity = 2 * __old_capacity;

   if (__capacity > max_size())
     __capacity = max_size();
 }



      return _Alloc_traits::allocate(_M_get_allocator(), __capacity + 1);
    }





  template<typename _CharT, typename _Traits, typename _Alloc>
    template<typename _InIterator>
      void
      basic_string<_CharT, _Traits, _Alloc>::
      _M_construct(_InIterator __beg, _InIterator __end,
     std::input_iterator_tag)
      {
 size_type __len = 0;
 size_type __capacity = size_type(_S_local_capacity);

 while (__beg != __end && __len < __capacity)
   {
     _M_data()[__len++] = *__beg;
     ++__beg;
   }

 try
   {
     while (__beg != __end)
       {
  if (__len == __capacity)
    {

      __capacity = __len + 1;
      pointer __another = _M_create(__capacity, __len);
      this->_S_copy(__another, _M_data(), __len);
      _M_dispose();
      _M_data(__another);
      _M_capacity(__capacity);
    }
  _M_data()[__len++] = *__beg;
  ++__beg;
       }
   }
 catch(...)
   {
     _M_dispose();
     throw;
   }

 _M_set_length(__len);
      }

  template<typename _CharT, typename _Traits, typename _Alloc>
    template<typename _InIterator>
      void
      basic_string<_CharT, _Traits, _Alloc>::
      _M_construct(_InIterator __beg, _InIterator __end,
     std::forward_iterator_tag)
      {

 if (__gnu_cxx::__is_null_pointer(__beg) && __beg != __end)
   std::__throw_logic_error(("basic_string::" "_M_construct null not valid")
                                         );

 size_type __dnew = static_cast<size_type>(std::distance(__beg, __end));

 if (__dnew > size_type(_S_local_capacity))
   {
     _M_data(_M_create(__dnew, size_type(0)));
     _M_capacity(__dnew);
   }


 try
   { this->_S_copy_chars(_M_data(), __beg, __end); }
 catch(...)
   {
     _M_dispose();
     throw;
   }

 _M_set_length(__dnew);
      }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    _M_construct(size_type __n, _CharT __c)
    {
      if (__n > size_type(_S_local_capacity))
 {
   _M_data(_M_create(__n, size_type(0)));
   _M_capacity(__n);
 }

      if (__n)
 this->_S_assign(_M_data(), __n, __c);

      _M_set_length(__n);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    _M_assign(const basic_string& __str)
    {
      if (this != &__str)
 {
   const size_type __rsize = __str.length();
   const size_type __capacity = capacity();

   if (__rsize > __capacity)
     {
       size_type __new_capacity = __rsize;
       pointer __tmp = _M_create(__new_capacity, __capacity);
       _M_dispose();
       _M_data(__tmp);
       _M_capacity(__new_capacity);
     }

   if (__rsize)
     this->_S_copy(_M_data(), __str._M_data(), __rsize);

   _M_set_length(__rsize);
 }
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    reserve(size_type __res)
    {
      const size_type __capacity = capacity();




      if (__res <= __capacity)
 return;

      pointer __tmp = _M_create(__res, __capacity);
      this->_S_copy(__tmp, _M_data(), length() + 1);
      _M_dispose();
      _M_data(__tmp);
      _M_capacity(__res);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    _M_mutate(size_type __pos, size_type __len1, const _CharT* __s,
       size_type __len2)
    {
      const size_type __how_much = length() - __pos - __len1;

      size_type __new_capacity = length() + __len2 - __len1;
      pointer __r = _M_create(__new_capacity, capacity());

      if (__pos)
 this->_S_copy(__r, _M_data(), __pos);
      if (__s && __len2)
 this->_S_copy(__r + __pos, __s, __len2);
      if (__how_much)
 this->_S_copy(__r + __pos + __len2,
        _M_data() + __pos + __len1, __how_much);

      _M_dispose();
      _M_data(__r);
      _M_capacity(__new_capacity);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    _M_erase(size_type __pos, size_type __n)
    {
      const size_type __how_much = length() - __pos - __n;

      if (__how_much && __n)
 this->_S_move(_M_data() + __pos, _M_data() + __pos + __n, __how_much);

      _M_set_length(length() - __n);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    reserve()
    {
      if (_M_is_local())
 return;

      const size_type __length = length();
      const size_type __capacity = _M_allocated_capacity;

      if (__length <= size_type(_S_local_capacity))
 {
   this->_S_copy(_M_local_data(), _M_data(), __length + 1);
   _M_destroy(__capacity);
   _M_data(_M_local_data());
 }

      else if (__length < __capacity)
 try
   {
     pointer __tmp
       = _Alloc_traits::allocate(_M_get_allocator(), __length + 1);
     this->_S_copy(__tmp, _M_data(), __length + 1);
     _M_dispose();
     _M_data(__tmp);
     _M_capacity(__length);
   }
 catch (const __cxxabiv1::__forced_unwind&)
   { throw; }
 catch (...)
   { }

    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    void
    basic_string<_CharT, _Traits, _Alloc>::
    resize(size_type __n, _CharT __c)
    {
      const size_type __size = this->size();
      if (__size < __n)
 this->append(__n - __size, __c);
      else if (__n < __size)
 this->_M_set_length(__n);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT, _Traits, _Alloc>&
    basic_string<_CharT, _Traits, _Alloc>::
    _M_append(const _CharT* __s, size_type __n)
    {
      const size_type __len = __n + this->size();

      if (__len <= this->capacity())
 {
   if (__n)
     this->_S_copy(this->_M_data() + this->size(), __s, __n);
 }
      else
 this->_M_mutate(this->size(), size_type(0), __s, __n);

      this->_M_set_length(__len);
      return *this;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    template<typename _InputIterator>
      basic_string<_CharT, _Traits, _Alloc>&
      basic_string<_CharT, _Traits, _Alloc>::
      _M_replace_dispatch(const_iterator __i1, const_iterator __i2,
     _InputIterator __k1, _InputIterator __k2,
     std::__false_type)
      {


 const basic_string __s(__k1, __k2, this->get_allocator());
 const size_type __n1 = __i2 - __i1;
 return _M_replace(__i1 - begin(), __n1, __s._M_data(),
     __s.size());
      }

  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT, _Traits, _Alloc>&
    basic_string<_CharT, _Traits, _Alloc>::
    _M_replace_aux(size_type __pos1, size_type __n1, size_type __n2,
     _CharT __c)
    {
      _M_check_length(__n1, __n2, "basic_string::_M_replace_aux");

      const size_type __old_size = this->size();
      const size_type __new_size = __old_size + __n2 - __n1;

      if (__new_size <= this->capacity())
 {
   pointer __p = this->_M_data() + __pos1;

   const size_type __how_much = __old_size - __pos1 - __n1;
   if (__how_much && __n1 != __n2)
     this->_S_move(__p + __n2, __p + __n1, __how_much);
 }
      else
 this->_M_mutate(__pos1, __n1, 0, __n2);

      if (__n2)
 this->_S_assign(this->_M_data() + __pos1, __n2, __c);

      this->_M_set_length(__new_size);
      return *this;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT, _Traits, _Alloc>&
    basic_string<_CharT, _Traits, _Alloc>::
    _M_replace(size_type __pos, size_type __len1, const _CharT* __s,
        const size_type __len2)
    {
      _M_check_length(__len1, __len2, "basic_string::_M_replace");

      const size_type __old_size = this->size();
      const size_type __new_size = __old_size + __len2 - __len1;

      if (__new_size <= this->capacity())
 {
   pointer __p = this->_M_data() + __pos;

   const size_type __how_much = __old_size - __pos - __len1;
   if (_M_disjunct(__s))
     {
       if (__how_much && __len1 != __len2)
  this->_S_move(__p + __len2, __p + __len1, __how_much);
       if (__len2)
  this->_S_copy(__p, __s, __len2);
     }
   else
     {

       if (__len2 && __len2 <= __len1)
  this->_S_move(__p, __s, __len2);
       if (__how_much && __len1 != __len2)
  this->_S_move(__p + __len2, __p + __len1, __how_much);
       if (__len2 > __len1)
  {
    if (__s + __len2 <= __p + __len1)
      this->_S_move(__p, __s, __len2);
    else if (__s >= __p + __len1)
      {


        const size_type __poff = (__s - __p) + (__len2 - __len1);
        this->_S_copy(__p, __p + __poff, __len2);
      }
    else
      {
        const size_type __nleft = (__p + __len1) - __s;
        this->_S_move(__p, __s, __nleft);
        this->_S_copy(__p + __nleft, __p + __len2,
        __len2 - __nleft);
      }
  }
     }
 }
      else
 this->_M_mutate(__pos, __len1, __s, __len2);

      this->_M_set_length(__new_size);
      return *this;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    copy(_CharT* __s, size_type __n, size_type __pos) const
    {
      _M_check(__pos, "basic_string::copy");
      __n = _M_limit(__pos, __n);
      ;
      if (__n)
 _S_copy(__s, _M_data() + __pos, __n);

      return __n;
    }
# 1167 "/usr/include/c++/11/bits/basic_string.tcc" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT, _Traits, _Alloc>
    operator+(const _CharT* __lhs,
       const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    {
      ;
      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef typename __string_type::size_type __size_type;
      typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
 rebind<_CharT>::other _Char_alloc_type;
      typedef __gnu_cxx::__alloc_traits<_Char_alloc_type> _Alloc_traits;
      const __size_type __len = _Traits::length(__lhs);
      __string_type __str(_Alloc_traits::_S_select_on_copy(
          __rhs.get_allocator()));
      __str.reserve(__len + __rhs.size());
      __str.append(__lhs, __len);
      __str.append(__rhs);
      return __str;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_string<_CharT, _Traits, _Alloc>
    operator+(_CharT __lhs, const basic_string<_CharT, _Traits, _Alloc>& __rhs)
    {
      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef typename __string_type::size_type __size_type;
      typedef typename __gnu_cxx::__alloc_traits<_Alloc>::template
 rebind<_CharT>::other _Char_alloc_type;
      typedef __gnu_cxx::__alloc_traits<_Char_alloc_type> _Alloc_traits;
      __string_type __str(_Alloc_traits::_S_select_on_copy(
          __rhs.get_allocator()));
      const __size_type __len = __rhs.size();
      __str.reserve(__len + 1);
      __str.append(__size_type(1), __lhs);
      __str.append(__rhs);
      return __str;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find(const _CharT* __s, size_type __pos, size_type __n) const
    noexcept
    {
      ;
      const size_type __size = this->size();

      if (__n == 0)
 return __pos <= __size ? __pos : npos;
      if (__pos >= __size)
 return npos;

      const _CharT __elem0 = __s[0];
      const _CharT* const __data = data();
      const _CharT* __first = __data + __pos;
      const _CharT* const __last = __data + __size;
      size_type __len = __size - __pos;

      while (__len >= __n)
 {

   __first = traits_type::find(__first, __len - __n + 1, __elem0);
   if (!__first)
     return npos;



   if (traits_type::compare(__first, __s, __n) == 0)
     return __first - __data;
   __len = __last - ++__first;
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find(_CharT __c, size_type __pos) const noexcept
    {
      size_type __ret = npos;
      const size_type __size = this->size();
      if (__pos < __size)
 {
   const _CharT* __data = _M_data();
   const size_type __n = __size - __pos;
   const _CharT* __p = traits_type::find(__data + __pos, __n, __c);
   if (__p)
     __ret = __p - __data;
 }
      return __ret;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    rfind(const _CharT* __s, size_type __pos, size_type __n) const
    noexcept
    {
      ;
      const size_type __size = this->size();
      if (__n <= __size)
 {
   __pos = std::min(size_type(__size - __n), __pos);
   const _CharT* __data = _M_data();
   do
     {
       if (traits_type::compare(__data + __pos, __s, __n) == 0)
  return __pos;
     }
   while (__pos-- > 0);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    rfind(_CharT __c, size_type __pos) const noexcept
    {
      size_type __size = this->size();
      if (__size)
 {
   if (--__size > __pos)
     __size = __pos;
   for (++__size; __size-- > 0; )
     if (traits_type::eq(_M_data()[__size], __c))
       return __size;
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
    noexcept
    {
      ;
      for (; __n && __pos < this->size(); ++__pos)
 {
   const _CharT* __p = traits_type::find(__s, __n, _M_data()[__pos]);
   if (__p)
     return __pos;
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
    noexcept
    {
      ;
      size_type __size = this->size();
      if (__size && __n)
 {
   if (--__size > __pos)
     __size = __pos;
   do
     {
       if (traits_type::find(__s, __n, _M_data()[__size]))
  return __size;
     }
   while (__size-- != 0);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find_first_not_of(const _CharT* __s, size_type __pos, size_type __n) const
    noexcept
    {
      ;
      for (; __pos < this->size(); ++__pos)
 if (!traits_type::find(__s, __n, _M_data()[__pos]))
   return __pos;
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find_first_not_of(_CharT __c, size_type __pos) const noexcept
    {
      for (; __pos < this->size(); ++__pos)
 if (!traits_type::eq(_M_data()[__pos], __c))
   return __pos;
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find_last_not_of(const _CharT* __s, size_type __pos, size_type __n) const
    noexcept
    {
      ;
      size_type __size = this->size();
      if (__size)
 {
   if (--__size > __pos)
     __size = __pos;
   do
     {
       if (!traits_type::find(__s, __n, _M_data()[__size]))
  return __size;
     }
   while (__size--);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    typename basic_string<_CharT, _Traits, _Alloc>::size_type
    basic_string<_CharT, _Traits, _Alloc>::
    find_last_not_of(_CharT __c, size_type __pos) const noexcept
    {
      size_type __size = this->size();
      if (__size)
 {
   if (--__size > __pos)
     __size = __pos;
   do
     {
       if (!traits_type::eq(_M_data()[__size], __c))
  return __size;
     }
   while (__size--);
 }
      return npos;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    int
    basic_string<_CharT, _Traits, _Alloc>::
    compare(size_type __pos, size_type __n, const basic_string& __str) const
    {
      _M_check(__pos, "basic_string::compare");
      __n = _M_limit(__pos, __n);
      const size_type __osize = __str.size();
      const size_type __len = std::min(__n, __osize);
      int __r = traits_type::compare(_M_data() + __pos, __str.data(), __len);
      if (!__r)
 __r = _S_compare(__n, __osize);
      return __r;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    int
    basic_string<_CharT, _Traits, _Alloc>::
    compare(size_type __pos1, size_type __n1, const basic_string& __str,
     size_type __pos2, size_type __n2) const
    {
      _M_check(__pos1, "basic_string::compare");
      __str._M_check(__pos2, "basic_string::compare");
      __n1 = _M_limit(__pos1, __n1);
      __n2 = __str._M_limit(__pos2, __n2);
      const size_type __len = std::min(__n1, __n2);
      int __r = traits_type::compare(_M_data() + __pos1,
         __str.data() + __pos2, __len);
      if (!__r)
 __r = _S_compare(__n1, __n2);
      return __r;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    int
    basic_string<_CharT, _Traits, _Alloc>::
    compare(const _CharT* __s) const noexcept
    {
      ;
      const size_type __size = this->size();
      const size_type __osize = traits_type::length(__s);
      const size_type __len = std::min(__size, __osize);
      int __r = traits_type::compare(_M_data(), __s, __len);
      if (!__r)
 __r = _S_compare(__size, __osize);
      return __r;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    int
    basic_string <_CharT, _Traits, _Alloc>::
    compare(size_type __pos, size_type __n1, const _CharT* __s) const
    {
      ;
      _M_check(__pos, "basic_string::compare");
      __n1 = _M_limit(__pos, __n1);
      const size_type __osize = traits_type::length(__s);
      const size_type __len = std::min(__n1, __osize);
      int __r = traits_type::compare(_M_data() + __pos, __s, __len);
      if (!__r)
 __r = _S_compare(__n1, __osize);
      return __r;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    int
    basic_string <_CharT, _Traits, _Alloc>::
    compare(size_type __pos, size_type __n1, const _CharT* __s,
     size_type __n2) const
    {
      ;
      _M_check(__pos, "basic_string::compare");
      __n1 = _M_limit(__pos, __n1);
      const size_type __len = std::min(__n1, __n2);
      int __r = traits_type::compare(_M_data() + __pos, __s, __len);
      if (!__r)
 __r = _S_compare(__n1, __n2);
      return __r;
    }


  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_istream<_CharT, _Traits>&
    operator>>(basic_istream<_CharT, _Traits>& __in,
        basic_string<_CharT, _Traits, _Alloc>& __str)
    {
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef typename __istream_type::ios_base __ios_base;
      typedef typename __istream_type::int_type __int_type;
      typedef typename __string_type::size_type __size_type;
      typedef ctype<_CharT> __ctype_type;
      typedef typename __ctype_type::ctype_base __ctype_base;

      __size_type __extracted = 0;
      typename __ios_base::iostate __err = __ios_base::goodbit;
      typename __istream_type::sentry __cerb(__in, false);
      if (__cerb)
 {
   try
     {

       __str.erase();
       _CharT __buf[128];
       __size_type __len = 0;
       const streamsize __w = __in.width();
       const __size_type __n = __w > 0 ? static_cast<__size_type>(__w)
                                : __str.max_size();
       const __ctype_type& __ct = use_facet<__ctype_type>(__in.getloc());
       const __int_type __eof = _Traits::eof();
       __int_type __c = __in.rdbuf()->sgetc();

       while (__extracted < __n
       && !_Traits::eq_int_type(__c, __eof)
       && !__ct.is(__ctype_base::space,
     _Traits::to_char_type(__c)))
  {
    if (__len == sizeof(__buf) / sizeof(_CharT))
      {
        __str.append(__buf, sizeof(__buf) / sizeof(_CharT));
        __len = 0;
      }
    __buf[__len++] = _Traits::to_char_type(__c);
    ++__extracted;
    __c = __in.rdbuf()->snextc();
  }
       __str.append(__buf, __len);

       if (__extracted < __n && _Traits::eq_int_type(__c, __eof))
  __err |= __ios_base::eofbit;
       __in.width(0);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       __in._M_setstate(__ios_base::badbit);
       throw;
     }
   catch(...)
     {



       __in._M_setstate(__ios_base::badbit);
     }
 }

      if (!__extracted)
 __err |= __ios_base::failbit;
      if (__err)
 __in.setstate(__err);
      return __in;
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    basic_istream<_CharT, _Traits>&
    getline(basic_istream<_CharT, _Traits>& __in,
     basic_string<_CharT, _Traits, _Alloc>& __str, _CharT __delim)
    {
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef typename __istream_type::ios_base __ios_base;
      typedef typename __istream_type::int_type __int_type;
      typedef typename __string_type::size_type __size_type;

      __size_type __extracted = 0;
      const __size_type __n = __str.max_size();
      typename __ios_base::iostate __err = __ios_base::goodbit;
      typename __istream_type::sentry __cerb(__in, true);
      if (__cerb)
 {
   try
     {
       __str.erase();
       const __int_type __idelim = _Traits::to_int_type(__delim);
       const __int_type __eof = _Traits::eof();
       __int_type __c = __in.rdbuf()->sgetc();

       while (__extracted < __n
       && !_Traits::eq_int_type(__c, __eof)
       && !_Traits::eq_int_type(__c, __idelim))
  {
    __str += _Traits::to_char_type(__c);
    ++__extracted;
    __c = __in.rdbuf()->snextc();
  }

       if (_Traits::eq_int_type(__c, __eof))
  __err |= __ios_base::eofbit;
       else if (_Traits::eq_int_type(__c, __idelim))
  {
    ++__extracted;
    __in.rdbuf()->sbumpc();
  }
       else
  __err |= __ios_base::failbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       __in._M_setstate(__ios_base::badbit);
       throw;
     }
   catch(...)
     {



       __in._M_setstate(__ios_base::badbit);
     }
 }
      if (!__extracted)
 __err |= __ios_base::failbit;
      if (__err)
 __in.setstate(__err);
      return __in;
    }
# 1627 "/usr/include/c++/11/bits/basic_string.tcc" 3
  extern template class basic_string<char>;







  extern template
    basic_istream<char>&
    operator>>(basic_istream<char>&, string&);
  extern template
    basic_ostream<char>&
    operator<<(basic_ostream<char>&, const string&);
  extern template
    basic_istream<char>&
    getline(basic_istream<char>&, string&, char);
  extern template
    basic_istream<char>&
    getline(basic_istream<char>&, string&);



  extern template class basic_string<wchar_t>;





  extern template
    basic_istream<wchar_t>&
    operator>>(basic_istream<wchar_t>&, wstring&);
  extern template
    basic_ostream<wchar_t>&
    operator<<(basic_ostream<wchar_t>&, const wstring&);
  extern template
    basic_istream<wchar_t>&
    getline(basic_istream<wchar_t>&, wstring&, wchar_t);
  extern template
    basic_istream<wchar_t>&
    getline(basic_istream<wchar_t>&, wstring&);




}
# 57 "/usr/include/c++/11/string" 2 3


namespace std __attribute__ ((__visibility__ ("default")))
{

  namespace pmr {
    template<typename _Tp> class polymorphic_allocator;
    template<typename _CharT, typename _Traits = char_traits<_CharT>>
      using basic_string = std::basic_string<_CharT, _Traits,
          polymorphic_allocator<_CharT>>;
    using string = basic_string<char>;



    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;

    using wstring = basic_string<wchar_t>;

  }

  template<typename _Str>
    struct __hash_string_base
    : public __hash_base<size_t, _Str>
    {
      size_t
      operator()(const _Str& __s) const noexcept
      { return hash<basic_string_view<typename _Str::value_type>>{}(__s); }
    };

  template<>
    struct hash<pmr::string>
    : public __hash_string_base<pmr::string>
    { };






  template<>
    struct hash<pmr::u16string>
    : public __hash_string_base<pmr::u16string>
    { };
  template<>
    struct hash<pmr::u32string>
    : public __hash_string_base<pmr::u32string>
    { };

  template<>
    struct hash<pmr::wstring>
    : public __hash_string_base<pmr::wstring>
    { };



}
# 41 "/usr/include/c++/11/bits/locale_classes.h" 2 3


namespace std __attribute__ ((__visibility__ ("default")))
{

# 62 "/usr/include/c++/11/bits/locale_classes.h" 3
  class locale
  {
  public:


    typedef int category;


    class facet;
    class id;
    class _Impl;

    friend class facet;
    friend class _Impl;

    template<typename _Facet>
      friend bool
      has_facet(const locale&) throw();

    template<typename _Facet>
      friend const _Facet&
      use_facet(const locale&);

    template<typename _Cache>
      friend struct __use_cache;
# 98 "/usr/include/c++/11/bits/locale_classes.h" 3
    static const category none = 0;
    static const category ctype = 1L << 0;
    static const category numeric = 1L << 1;
    static const category collate = 1L << 2;
    static const category time = 1L << 3;
    static const category monetary = 1L << 4;
    static const category messages = 1L << 5;
    static const category all = (ctype | numeric | collate |
        time | monetary | messages);
# 117 "/usr/include/c++/11/bits/locale_classes.h" 3
    locale() throw();
# 126 "/usr/include/c++/11/bits/locale_classes.h" 3
    locale(const locale& __other) throw();
# 136 "/usr/include/c++/11/bits/locale_classes.h" 3
    explicit
    locale(const char* __s);
# 151 "/usr/include/c++/11/bits/locale_classes.h" 3
    locale(const locale& __base, const char* __s, category __cat);
# 162 "/usr/include/c++/11/bits/locale_classes.h" 3
    explicit
    locale(const std::string& __s) : locale(__s.c_str()) { }
# 177 "/usr/include/c++/11/bits/locale_classes.h" 3
    locale(const locale& __base, const std::string& __s, category __cat)
    : locale(__base, __s.c_str(), __cat) { }
# 192 "/usr/include/c++/11/bits/locale_classes.h" 3
    locale(const locale& __base, const locale& __add, category __cat);
# 205 "/usr/include/c++/11/bits/locale_classes.h" 3
    template<typename _Facet>
      locale(const locale& __other, _Facet* __f);


    ~locale() throw();
# 219 "/usr/include/c++/11/bits/locale_classes.h" 3
    const locale&
    operator=(const locale& __other) throw();
# 234 "/usr/include/c++/11/bits/locale_classes.h" 3
    template<typename _Facet>
      locale
      combine(const locale& __other) const;






    __attribute ((__abi_tag__ ("cxx11")))
    string
    name() const;
# 254 "/usr/include/c++/11/bits/locale_classes.h" 3
    bool
    operator==(const locale& __other) const throw();
# 264 "/usr/include/c++/11/bits/locale_classes.h" 3
    bool
    operator!=(const locale& __other) const throw()
    { return !(this->operator==(__other)); }
# 284 "/usr/include/c++/11/bits/locale_classes.h" 3
    template<typename _Char, typename _Traits, typename _Alloc>
      bool
      operator()(const basic_string<_Char, _Traits, _Alloc>& __s1,
   const basic_string<_Char, _Traits, _Alloc>& __s2) const;
# 300 "/usr/include/c++/11/bits/locale_classes.h" 3
    static locale
    global(const locale& __loc);




    static const locale&
    classic();

  private:

    _Impl* _M_impl;


    static _Impl* _S_classic;


    static _Impl* _S_global;





    static const char* const* const _S_categories;
# 335 "/usr/include/c++/11/bits/locale_classes.h" 3
    enum { _S_categories_size = 6 + 6 };


    static __gthread_once_t _S_once;


    explicit
    locale(_Impl*) throw();

    static void
    _S_initialize();

    static void
    _S_initialize_once() throw();

    static category
    _S_normalize_category(category);

    void
    _M_coalesce(const locale& __base, const locale& __add, category __cat);


    static const id* const _S_twinned_facets[];

  };
# 373 "/usr/include/c++/11/bits/locale_classes.h" 3
  class locale::facet
  {
  private:
    friend class locale;
    friend class locale::_Impl;

    mutable _Atomic_word _M_refcount;


    static __c_locale _S_c_locale;


    static const char _S_c_name[2];


    static __gthread_once_t _S_once;


    static void
    _S_initialize_once();

  protected:
# 404 "/usr/include/c++/11/bits/locale_classes.h" 3
    explicit
    facet(size_t __refs = 0) throw() : _M_refcount(__refs ? 1 : 0)
    { }


    virtual
    ~facet();

    static void
    _S_create_c_locale(__c_locale& __cloc, const char* __s,
         __c_locale __old = 0);

    static __c_locale
    _S_clone_c_locale(__c_locale& __cloc) throw();

    static void
    _S_destroy_c_locale(__c_locale& __cloc);

    static __c_locale
    _S_lc_ctype_c_locale(__c_locale __cloc, const char* __s);



    static __c_locale
    _S_get_c_locale();

    __attribute__ ((__const__)) static const char*
    _S_get_c_name() throw();
# 440 "/usr/include/c++/11/bits/locale_classes.h" 3
    facet(const facet&) = delete;

    facet&
    operator=(const facet&) = delete;


  private:
    void
    _M_add_reference() const throw()
    { __gnu_cxx::__atomic_add_dispatch(&_M_refcount, 1); }

    void
    _M_remove_reference() const throw()
    {

      ;
      if (__gnu_cxx::__exchange_and_add_dispatch(&_M_refcount, -1) == 1)
 {
          ;
   try
     { delete this; }
   catch(...)
     { }
 }
    }

    const facet* _M_sso_shim(const id*) const;
    const facet* _M_cow_shim(const id*) const;

  protected:
    class __shim;
  };
# 485 "/usr/include/c++/11/bits/locale_classes.h" 3
  class locale::id
  {
  private:
    friend class locale;
    friend class locale::_Impl;

    template<typename _Facet>
      friend const _Facet&
      use_facet(const locale&);

    template<typename _Facet>
      friend bool
      has_facet(const locale&) throw();




    mutable size_t _M_index;


    static _Atomic_word _S_refcount;

    void
    operator=(const id&);

    id(const id&);

  public:



    id() { }

    size_t
    _M_id() const throw();
  };



  class locale::_Impl
  {
  public:

    friend class locale;
    friend class locale::facet;

    template<typename _Facet>
      friend bool
      has_facet(const locale&) throw();

    template<typename _Facet>
      friend const _Facet&
      use_facet(const locale&);

    template<typename _Cache>
      friend struct __use_cache;

  private:

    _Atomic_word _M_refcount;
    const facet** _M_facets;
    size_t _M_facets_size;
    const facet** _M_caches;
    char** _M_names;
    static const locale::id* const _S_id_ctype[];
    static const locale::id* const _S_id_numeric[];
    static const locale::id* const _S_id_collate[];
    static const locale::id* const _S_id_time[];
    static const locale::id* const _S_id_monetary[];
    static const locale::id* const _S_id_messages[];
    static const locale::id* const* const _S_facet_categories[];

    void
    _M_add_reference() throw()
    { __gnu_cxx::__atomic_add_dispatch(&_M_refcount, 1); }

    void
    _M_remove_reference() throw()
    {

      ;
      if (__gnu_cxx::__exchange_and_add_dispatch(&_M_refcount, -1) == 1)
 {
          ;
   try
     { delete this; }
   catch(...)
     { }
 }
    }

    _Impl(const _Impl&, size_t);
    _Impl(const char*, size_t);
    _Impl(size_t) throw();

   ~_Impl() throw();

    _Impl(const _Impl&);

    void
    operator=(const _Impl&);

    bool
    _M_check_same_name()
    {
      bool __ret = true;
      if (_M_names[1])

 for (size_t __i = 0; __ret && __i < _S_categories_size - 1; ++__i)
   __ret = __builtin_strcmp(_M_names[__i], _M_names[__i + 1]) == 0;
      return __ret;
    }

    void
    _M_replace_categories(const _Impl*, category);

    void
    _M_replace_category(const _Impl*, const locale::id* const*);

    void
    _M_replace_facet(const _Impl*, const locale::id*);

    void
    _M_install_facet(const locale::id*, const facet*);

    template<typename _Facet>
      void
      _M_init_facet(_Facet* __facet)
      { _M_install_facet(&_Facet::id, __facet); }

    template<typename _Facet>
      void
      _M_init_facet_unchecked(_Facet* __facet)
      {
 __facet->_M_add_reference();
 _M_facets[_Facet::id._M_id()] = __facet;
      }

    void
    _M_install_cache(const facet*, size_t);

    void _M_init_extra(facet**);
    void _M_init_extra(void*, void*, const char*, const char*);




  };
# 647 "/usr/include/c++/11/bits/locale_classes.h" 3
  template<typename _CharT>
    class __cxx11:: collate : public locale::facet
    {
    public:



      typedef _CharT char_type;
      typedef basic_string<_CharT> string_type;


    protected:


      __c_locale _M_c_locale_collate;

    public:

      static locale::id id;
# 674 "/usr/include/c++/11/bits/locale_classes.h" 3
      explicit
      collate(size_t __refs = 0)
      : facet(__refs), _M_c_locale_collate(_S_get_c_locale())
      { }
# 688 "/usr/include/c++/11/bits/locale_classes.h" 3
      explicit
      collate(__c_locale __cloc, size_t __refs = 0)
      : facet(__refs), _M_c_locale_collate(_S_clone_c_locale(__cloc))
      { }
# 705 "/usr/include/c++/11/bits/locale_classes.h" 3
      int
      compare(const _CharT* __lo1, const _CharT* __hi1,
       const _CharT* __lo2, const _CharT* __hi2) const
      { return this->do_compare(__lo1, __hi1, __lo2, __hi2); }
# 724 "/usr/include/c++/11/bits/locale_classes.h" 3
      string_type
      transform(const _CharT* __lo, const _CharT* __hi) const
      { return this->do_transform(__lo, __hi); }
# 738 "/usr/include/c++/11/bits/locale_classes.h" 3
      long
      hash(const _CharT* __lo, const _CharT* __hi) const
      { return this->do_hash(__lo, __hi); }


      int
      _M_compare(const _CharT*, const _CharT*) const throw();

      size_t
      _M_transform(_CharT*, const _CharT*, size_t) const throw();

  protected:

      virtual
      ~collate()
      { _S_destroy_c_locale(_M_c_locale_collate); }
# 767 "/usr/include/c++/11/bits/locale_classes.h" 3
      virtual int
      do_compare(const _CharT* __lo1, const _CharT* __hi1,
   const _CharT* __lo2, const _CharT* __hi2) const;
# 781 "/usr/include/c++/11/bits/locale_classes.h" 3
      virtual string_type
      do_transform(const _CharT* __lo, const _CharT* __hi) const;
# 794 "/usr/include/c++/11/bits/locale_classes.h" 3
      virtual long
      do_hash(const _CharT* __lo, const _CharT* __hi) const;
    };

  template<typename _CharT>
    locale::id collate<_CharT>::id;


  template<>
    int
    collate<char>::_M_compare(const char*, const char*) const throw();

  template<>
    size_t
    collate<char>::_M_transform(char*, const char*, size_t) const throw();


  template<>
    int
    collate<wchar_t>::_M_compare(const wchar_t*, const wchar_t*) const throw();

  template<>
    size_t
    collate<wchar_t>::_M_transform(wchar_t*, const wchar_t*, size_t) const throw();



  template<typename _CharT>
    class __cxx11:: collate_byname : public collate<_CharT>
    {
    public:


      typedef _CharT char_type;
      typedef basic_string<_CharT> string_type;


      explicit
      collate_byname(const char* __s, size_t __refs = 0)
      : collate<_CharT>(__refs)
      {
 if (__builtin_strcmp(__s, "C") != 0
     && __builtin_strcmp(__s, "POSIX") != 0)
   {
     this->_S_destroy_c_locale(this->_M_c_locale_collate);
     this->_S_create_c_locale(this->_M_c_locale_collate, __s);
   }
      }


      explicit
      collate_byname(const string& __s, size_t __refs = 0)
      : collate_byname(__s.c_str(), __refs) { }


    protected:
      virtual
      ~collate_byname() { }
    };


}

# 1 "/usr/include/c++/11/bits/locale_classes.tcc" 1 3
# 37 "/usr/include/c++/11/bits/locale_classes.tcc" 3
       
# 38 "/usr/include/c++/11/bits/locale_classes.tcc" 3

namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _Facet>
    locale::
    locale(const locale& __other, _Facet* __f)
    {
      _M_impl = new _Impl(*__other._M_impl, 1);

      try
 { _M_impl->_M_install_facet(&_Facet::id, __f); }
      catch(...)
 {
   _M_impl->_M_remove_reference();
   throw;
 }
      delete [] _M_impl->_M_names[0];
      _M_impl->_M_names[0] = 0;
    }

  template<typename _Facet>
    locale
    locale::
    combine(const locale& __other) const
    {
      _Impl* __tmp = new _Impl(*_M_impl, 1);
      try
 {
   __tmp->_M_replace_facet(__other._M_impl, &_Facet::id);
 }
      catch(...)
 {
   __tmp->_M_remove_reference();
   throw;
 }
      return locale(__tmp);
    }

  template<typename _CharT, typename _Traits, typename _Alloc>
    bool
    locale::
    operator()(const basic_string<_CharT, _Traits, _Alloc>& __s1,
        const basic_string<_CharT, _Traits, _Alloc>& __s2) const
    {
      typedef std::collate<_CharT> __collate_type;
      const __collate_type& __collate = use_facet<__collate_type>(*this);
      return (__collate.compare(__s1.data(), __s1.data() + __s1.length(),
    __s2.data(), __s2.data() + __s2.length()) < 0);
    }
# 102 "/usr/include/c++/11/bits/locale_classes.tcc" 3
  template<typename _Facet>
    bool
    has_facet(const locale& __loc) throw()
    {
      const size_t __i = _Facet::id._M_id();
      const locale::facet** __facets = __loc._M_impl->_M_facets;
      return (__i < __loc._M_impl->_M_facets_size

       && dynamic_cast<const _Facet*>(__facets[__i]));



    }
# 130 "/usr/include/c++/11/bits/locale_classes.tcc" 3
  template<typename _Facet>
    const _Facet&
    use_facet(const locale& __loc)
    {
      const size_t __i = _Facet::id._M_id();
      const locale::facet** __facets = __loc._M_impl->_M_facets;
      if (__i >= __loc._M_impl->_M_facets_size || !__facets[__i])
        __throw_bad_cast();

      return dynamic_cast<const _Facet&>(*__facets[__i]);



    }



  template<typename _CharT>
    int
    collate<_CharT>::_M_compare(const _CharT*, const _CharT*) const throw ()
    { return 0; }


  template<typename _CharT>
    size_t
    collate<_CharT>::_M_transform(_CharT*, const _CharT*, size_t) const throw ()
    { return 0; }

  template<typename _CharT>
    int
    collate<_CharT>::
    do_compare(const _CharT* __lo1, const _CharT* __hi1,
        const _CharT* __lo2, const _CharT* __hi2) const
    {


      const string_type __one(__lo1, __hi1);
      const string_type __two(__lo2, __hi2);

      const _CharT* __p = __one.c_str();
      const _CharT* __pend = __one.data() + __one.length();
      const _CharT* __q = __two.c_str();
      const _CharT* __qend = __two.data() + __two.length();




      for (;;)
 {
   const int __res = _M_compare(__p, __q);
   if (__res)
     return __res;

   __p += char_traits<_CharT>::length(__p);
   __q += char_traits<_CharT>::length(__q);
   if (__p == __pend && __q == __qend)
     return 0;
   else if (__p == __pend)
     return -1;
   else if (__q == __qend)
     return 1;

   __p++;
   __q++;
 }
    }

  template<typename _CharT>
    typename collate<_CharT>::string_type
    collate<_CharT>::
    do_transform(const _CharT* __lo, const _CharT* __hi) const
    {
      string_type __ret;


      const string_type __str(__lo, __hi);

      const _CharT* __p = __str.c_str();
      const _CharT* __pend = __str.data() + __str.length();

      size_t __len = (__hi - __lo) * 2;

      _CharT* __c = new _CharT[__len];

      try
 {



   for (;;)
     {

       size_t __res = _M_transform(__c, __p, __len);


       if (__res >= __len)
  {
    __len = __res + 1;
    delete [] __c, __c = 0;
    __c = new _CharT[__len];
    __res = _M_transform(__c, __p, __len);
  }

       __ret.append(__c, __res);
       __p += char_traits<_CharT>::length(__p);
       if (__p == __pend)
  break;

       __p++;
       __ret.push_back(_CharT());
     }
 }
      catch(...)
 {
   delete [] __c;
   throw;
 }

      delete [] __c;

      return __ret;
    }

  template<typename _CharT>
    long
    collate<_CharT>::
    do_hash(const _CharT* __lo, const _CharT* __hi) const
    {
      unsigned long __val = 0;
      for (; __lo < __hi; ++__lo)
 __val =
   *__lo + ((__val << 7)
     | (__val >> (__gnu_cxx::__numeric_traits<unsigned long>::
    __digits - 7)));
      return static_cast<long>(__val);
    }




  extern template class collate<char>;
  extern template class collate_byname<char>;

  extern template
    const collate<char>&
    use_facet<collate<char> >(const locale&);

  extern template
    bool
    has_facet<collate<char> >(const locale&);


  extern template class collate<wchar_t>;
  extern template class collate_byname<wchar_t>;

  extern template
    const collate<wchar_t>&
    use_facet<collate<wchar_t> >(const locale&);

  extern template
    bool
    has_facet<collate<wchar_t> >(const locale&);




}
# 858 "/usr/include/c++/11/bits/locale_classes.h" 2 3
# 42 "/usr/include/c++/11/bits/ios_base.h" 2 3




# 1 "/usr/include/c++/11/system_error" 1 3
# 32 "/usr/include/c++/11/system_error" 3
       
# 33 "/usr/include/c++/11/system_error" 3






# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/error_constants.h" 1 3
# 34 "/usr/include/c++/11/x86_64-redhat-linux/bits/error_constants.h" 3
# 1 "/usr/include/c++/11/cerrno" 1 3
# 39 "/usr/include/c++/11/cerrno" 3
       
# 40 "/usr/include/c++/11/cerrno" 3
# 35 "/usr/include/c++/11/x86_64-redhat-linux/bits/error_constants.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{


  enum class errc
    {
      address_family_not_supported = 97,
      address_in_use = 98,
      address_not_available = 99,
      already_connected = 106,
      argument_list_too_long = 7,
      argument_out_of_domain = 33,
      bad_address = 14,
      bad_file_descriptor = 9,


      bad_message = 74,


      broken_pipe = 32,
      connection_aborted = 103,
      connection_already_in_progress = 114,
      connection_refused = 111,
      connection_reset = 104,
      cross_device_link = 18,
      destination_address_required = 89,
      device_or_resource_busy = 16,
      directory_not_empty = 39,
      executable_format_error = 8,
      file_exists = 17,
      file_too_large = 27,
      filename_too_long = 36,
      function_not_supported = 38,
      host_unreachable = 113,


      identifier_removed = 43,


      illegal_byte_sequence = 84,
      inappropriate_io_control_operation = 25,
      interrupted = 4,
      invalid_argument = 22,
      invalid_seek = 29,
      io_error = 5,
      is_a_directory = 21,
      message_size = 90,
      network_down = 100,
      network_reset = 102,
      network_unreachable = 101,
      no_buffer_space = 105,
      no_child_process = 10,


      no_link = 67,


      no_lock_available = 37,


      no_message_available = 61,


      no_message = 42,
      no_protocol_option = 92,
      no_space_on_device = 28,


      no_stream_resources = 63,


      no_such_device_or_address = 6,
      no_such_device = 19,
      no_such_file_or_directory = 2,
      no_such_process = 3,
      not_a_directory = 20,
      not_a_socket = 88,


      not_a_stream = 60,


      not_connected = 107,
      not_enough_memory = 12,


      not_supported = 95,



      operation_canceled = 125,


      operation_in_progress = 115,
      operation_not_permitted = 1,
      operation_not_supported = 95,
      operation_would_block = 11,


      owner_dead = 130,


      permission_denied = 13,


      protocol_error = 71,


      protocol_not_supported = 93,
      read_only_file_system = 30,
      resource_deadlock_would_occur = 35,
      resource_unavailable_try_again = 11,
      result_out_of_range = 34,


      state_not_recoverable = 131,



      stream_timeout = 62,



      text_file_busy = 26,


      timed_out = 110,
      too_many_files_open_in_system = 23,
      too_many_files_open = 24,
      too_many_links = 31,
      too_many_symbolic_link_levels = 40,


      value_too_large = 75,




      wrong_protocol_type = 91
    };


}
# 40 "/usr/include/c++/11/system_error" 2 3

# 1 "/usr/include/c++/11/stdexcept" 1 3
# 36 "/usr/include/c++/11/stdexcept" 3
       
# 37 "/usr/include/c++/11/stdexcept" 3




namespace std __attribute__ ((__visibility__ ("default")))
{





  struct __cow_string
  {
    union {
      const char* _M_p;
      char _M_bytes[sizeof(const char*)];
    };

    __cow_string();
    __cow_string(const std::string&);
    __cow_string(const char*, size_t);
    __cow_string(const __cow_string&) noexcept;
    __cow_string& operator=(const __cow_string&) noexcept;
    ~__cow_string();

    __cow_string(__cow_string&&) noexcept;
    __cow_string& operator=(__cow_string&&) noexcept;

  };

  typedef basic_string<char> __sso_string;
# 113 "/usr/include/c++/11/stdexcept" 3
  class logic_error : public exception
  {
    __cow_string _M_msg;

  public:

    explicit
    logic_error(const string& __arg) ;


    explicit
    logic_error(const char*) ;

    logic_error(logic_error&&) noexcept;
    logic_error& operator=(logic_error&&) noexcept;



    logic_error(const logic_error&) noexcept;
    logic_error& operator=(const logic_error&) noexcept;





    virtual ~logic_error() noexcept;



    virtual const char*
    what() const noexcept;





  };



  class domain_error : public logic_error
  {
  public:
    explicit domain_error(const string& __arg) ;

    explicit domain_error(const char*) ;
    domain_error(const domain_error&) = default;
    domain_error& operator=(const domain_error&) = default;
    domain_error(domain_error&&) = default;
    domain_error& operator=(domain_error&&) = default;

    virtual ~domain_error() noexcept;
  };


  class invalid_argument : public logic_error
  {
  public:
    explicit invalid_argument(const string& __arg) ;

    explicit invalid_argument(const char*) ;
    invalid_argument(const invalid_argument&) = default;
    invalid_argument& operator=(const invalid_argument&) = default;
    invalid_argument(invalid_argument&&) = default;
    invalid_argument& operator=(invalid_argument&&) = default;

    virtual ~invalid_argument() noexcept;
  };



  class length_error : public logic_error
  {
  public:
    explicit length_error(const string& __arg) ;

    explicit length_error(const char*) ;
    length_error(const length_error&) = default;
    length_error& operator=(const length_error&) = default;
    length_error(length_error&&) = default;
    length_error& operator=(length_error&&) = default;

    virtual ~length_error() noexcept;
  };



  class out_of_range : public logic_error
  {
  public:
    explicit out_of_range(const string& __arg) ;

    explicit out_of_range(const char*) ;
    out_of_range(const out_of_range&) = default;
    out_of_range& operator=(const out_of_range&) = default;
    out_of_range(out_of_range&&) = default;
    out_of_range& operator=(out_of_range&&) = default;

    virtual ~out_of_range() noexcept;
  };






  class runtime_error : public exception
  {
    __cow_string _M_msg;

  public:

    explicit
    runtime_error(const string& __arg) ;


    explicit
    runtime_error(const char*) ;

    runtime_error(runtime_error&&) noexcept;
    runtime_error& operator=(runtime_error&&) noexcept;



    runtime_error(const runtime_error&) noexcept;
    runtime_error& operator=(const runtime_error&) noexcept;





    virtual ~runtime_error() noexcept;



    virtual const char*
    what() const noexcept;





  };


  class range_error : public runtime_error
  {
  public:
    explicit range_error(const string& __arg) ;

    explicit range_error(const char*) ;
    range_error(const range_error&) = default;
    range_error& operator=(const range_error&) = default;
    range_error(range_error&&) = default;
    range_error& operator=(range_error&&) = default;

    virtual ~range_error() noexcept;
  };


  class overflow_error : public runtime_error
  {
  public:
    explicit overflow_error(const string& __arg) ;

    explicit overflow_error(const char*) ;
    overflow_error(const overflow_error&) = default;
    overflow_error& operator=(const overflow_error&) = default;
    overflow_error(overflow_error&&) = default;
    overflow_error& operator=(overflow_error&&) = default;

    virtual ~overflow_error() noexcept;
  };


  class underflow_error : public runtime_error
  {
  public:
    explicit underflow_error(const string& __arg) ;

    explicit underflow_error(const char*) ;
    underflow_error(const underflow_error&) = default;
    underflow_error& operator=(const underflow_error&) = default;
    underflow_error(underflow_error&&) = default;
    underflow_error& operator=(underflow_error&&) = default;

    virtual ~underflow_error() noexcept;
  };




}
# 42 "/usr/include/c++/11/system_error" 2 3




namespace std __attribute__ ((__visibility__ ("default")))
{






  class error_code;
  class error_condition;
  class system_error;


  template<typename _Tp>
    struct is_error_code_enum : public false_type { };


  template<typename _Tp>
    struct is_error_condition_enum : public false_type { };

  template<>
    struct is_error_condition_enum<errc>
    : public true_type { };


  template <typename _Tp>
    inline constexpr bool is_error_code_enum_v =
      is_error_code_enum<_Tp>::value;
  template <typename _Tp>
    inline constexpr bool is_error_condition_enum_v =
      is_error_condition_enum<_Tp>::value;



  inline namespace _V2 {
# 97 "/usr/include/c++/11/system_error" 3
  class error_category
  {
  public:
    constexpr error_category() noexcept = default;

    virtual ~error_category();

    error_category(const error_category&) = delete;
    error_category& operator=(const error_category&) = delete;

    virtual const char*
    name() const noexcept = 0;






  private:
    __attribute ((__abi_tag__ ("cxx11")))
    virtual __cow_string
    _M_message(int) const;

  public:
    __attribute ((__abi_tag__ ("cxx11")))
    virtual string
    message(int) const = 0;
# 133 "/usr/include/c++/11/system_error" 3
  public:
    virtual error_condition
    default_error_condition(int __i) const noexcept;

    virtual bool
    equivalent(int __i, const error_condition& __cond) const noexcept;

    virtual bool
    equivalent(const error_code& __code, int __i) const noexcept;

    bool
    operator==(const error_category& __other) const noexcept
    { return this == &__other; }






    bool
    operator!=(const error_category& __other) const noexcept
    { return this != &__other; }

    bool
    operator<(const error_category& __other) const noexcept
    { return less<const error_category*>()(this, &__other); }

  };




  __attribute__ ((__const__)) const error_category& generic_category() noexcept;


  __attribute__ ((__const__)) const error_category& system_category() noexcept;


  }





  error_code make_error_code(errc) noexcept;
# 196 "/usr/include/c++/11/system_error" 3
  class error_code
  {
  public:
    error_code() noexcept
    : _M_value(0), _M_cat(&system_category()) { }

    error_code(int __v, const error_category& __cat) noexcept
    : _M_value(__v), _M_cat(&__cat) { }

    template<typename _ErrorCodeEnum, typename = typename
      enable_if<is_error_code_enum<_ErrorCodeEnum>::value>::type>
      error_code(_ErrorCodeEnum __e) noexcept
      { *this = make_error_code(__e); }

    void
    assign(int __v, const error_category& __cat) noexcept
    {
      _M_value = __v;
      _M_cat = &__cat;
    }

    void
    clear() noexcept
    { assign(0, system_category()); }


    template<typename _ErrorCodeEnum>
      typename enable_if<is_error_code_enum<_ErrorCodeEnum>::value,
    error_code&>::type
      operator=(_ErrorCodeEnum __e) noexcept
      { return *this = make_error_code(__e); }

    int
    value() const noexcept { return _M_value; }

    const error_category&
    category() const noexcept { return *_M_cat; }

    error_condition
    default_error_condition() const noexcept;

    __attribute ((__abi_tag__ ("cxx11")))
    string
    message() const
    { return category().message(value()); }

    explicit operator bool() const noexcept
    { return _M_value != 0; }


  private:
    int _M_value;
    const error_category* _M_cat;
  };





  inline error_code
  make_error_code(errc __e) noexcept
  { return error_code(static_cast<int>(__e), generic_category()); }
# 268 "/usr/include/c++/11/system_error" 3
  inline bool
  operator<(const error_code& __lhs, const error_code& __rhs) noexcept
  {
    return (__lhs.category() < __rhs.category()
     || (__lhs.category() == __rhs.category()
  && __lhs.value() < __rhs.value()));
  }


  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __os, const error_code& __e)
    { return (__os << __e.category().name() << ':' << __e.value()); }



  error_condition make_error_condition(errc) noexcept;
# 297 "/usr/include/c++/11/system_error" 3
  class error_condition
  {
  public:
    error_condition() noexcept
    : _M_value(0), _M_cat(&generic_category()) { }

    error_condition(int __v, const error_category& __cat) noexcept
    : _M_value(__v), _M_cat(&__cat) { }

    template<typename _ErrorConditionEnum, typename = typename
  enable_if<is_error_condition_enum<_ErrorConditionEnum>::value>::type>
      error_condition(_ErrorConditionEnum __e) noexcept
      { *this = make_error_condition(__e); }

    void
    assign(int __v, const error_category& __cat) noexcept
    {
      _M_value = __v;
      _M_cat = &__cat;
    }


    template<typename _ErrorConditionEnum>
      typename enable_if<is_error_condition_enum
    <_ErrorConditionEnum>::value, error_condition&>::type
      operator=(_ErrorConditionEnum __e) noexcept
      { return *this = make_error_condition(__e); }

    void
    clear() noexcept
    { assign(0, generic_category()); }


    int
    value() const noexcept { return _M_value; }

    const error_category&
    category() const noexcept { return *_M_cat; }

    __attribute ((__abi_tag__ ("cxx11")))
    string
    message() const
    { return category().message(value()); }

    explicit operator bool() const noexcept
    { return _M_value != 0; }


  private:
    int _M_value;
    const error_category* _M_cat;
  };





  inline error_condition
  make_error_condition(errc __e) noexcept
  { return error_condition(static_cast<int>(__e), generic_category()); }




  inline bool
  operator==(const error_code& __lhs, const error_code& __rhs) noexcept
  { return (__lhs.category() == __rhs.category()
     && __lhs.value() == __rhs.value()); }


  inline bool
  operator==(const error_code& __lhs, const error_condition& __rhs) noexcept
  {
    return (__lhs.category().equivalent(__lhs.value(), __rhs)
     || __rhs.category().equivalent(__lhs, __rhs.value()));
  }


  inline bool
  operator==(const error_condition& __lhs,
      const error_condition& __rhs) noexcept
  {
    return (__lhs.category() == __rhs.category()
     && __lhs.value() == __rhs.value());
  }
# 397 "/usr/include/c++/11/system_error" 3
  inline bool
  operator<(const error_condition& __lhs,
     const error_condition& __rhs) noexcept
  {
    return (__lhs.category() < __rhs.category()
     || (__lhs.category() == __rhs.category()
  && __lhs.value() < __rhs.value()));
  }


  inline bool
  operator==(const error_condition& __lhs, const error_code& __rhs) noexcept
  {
    return (__rhs.category().equivalent(__rhs.value(), __lhs)
     || __lhs.category().equivalent(__rhs, __lhs.value()));
  }


  inline bool
  operator!=(const error_code& __lhs, const error_code& __rhs) noexcept
  { return !(__lhs == __rhs); }


  inline bool
  operator!=(const error_code& __lhs, const error_condition& __rhs) noexcept
  { return !(__lhs == __rhs); }


  inline bool
  operator!=(const error_condition& __lhs, const error_code& __rhs) noexcept
  { return !(__lhs == __rhs); }


  inline bool
  operator!=(const error_condition& __lhs,
      const error_condition& __rhs) noexcept
  { return !(__lhs == __rhs); }
# 446 "/usr/include/c++/11/system_error" 3
  class system_error : public std::runtime_error
  {
  private:
    error_code _M_code;

  public:
    system_error(error_code __ec = error_code())
    : runtime_error(__ec.message()), _M_code(__ec) { }

    system_error(error_code __ec, const string& __what)
    : runtime_error(__what + ": " + __ec.message()), _M_code(__ec) { }

    system_error(error_code __ec, const char* __what)
    : runtime_error(__what + (": " + __ec.message())), _M_code(__ec) { }

    system_error(int __v, const error_category& __ecat, const char* __what)
    : system_error(error_code(__v, __ecat), __what) { }

    system_error(int __v, const error_category& __ecat)
    : runtime_error(error_code(__v, __ecat).message()),
      _M_code(__v, __ecat) { }

    system_error(int __v, const error_category& __ecat, const string& __what)
    : runtime_error(__what + ": " + error_code(__v, __ecat).message()),
      _M_code(__v, __ecat) { }


    system_error (const system_error &) = default;
    system_error &operator= (const system_error &) = default;


    virtual ~system_error() noexcept;

    const error_code&
    code() const noexcept { return _M_code; }
  };


}



namespace std __attribute__ ((__visibility__ ("default")))
{






  template<>
    struct hash<error_code>
    : public __hash_base<size_t, error_code>
    {
      size_t
      operator()(const error_code& __e) const noexcept
      {
 const size_t __tmp = std::_Hash_impl::hash(__e.value());
 return std::_Hash_impl::__hash_combine(&__e.category(), __tmp);
      }
    };






  template<>
    struct hash<error_condition>
    : public __hash_base<size_t, error_condition>
    {
      size_t
      operator()(const error_condition& __e) const noexcept
      {
 const size_t __tmp = std::_Hash_impl::hash(__e.value());
 return std::_Hash_impl::__hash_combine(&__e.category(), __tmp);
      }
    };



}
# 47 "/usr/include/c++/11/bits/ios_base.h" 2 3


namespace std __attribute__ ((__visibility__ ("default")))
{






  enum _Ios_Fmtflags
    {
      _S_boolalpha = 1L << 0,
      _S_dec = 1L << 1,
      _S_fixed = 1L << 2,
      _S_hex = 1L << 3,
      _S_internal = 1L << 4,
      _S_left = 1L << 5,
      _S_oct = 1L << 6,
      _S_right = 1L << 7,
      _S_scientific = 1L << 8,
      _S_showbase = 1L << 9,
      _S_showpoint = 1L << 10,
      _S_showpos = 1L << 11,
      _S_skipws = 1L << 12,
      _S_unitbuf = 1L << 13,
      _S_uppercase = 1L << 14,
      _S_adjustfield = _S_left | _S_right | _S_internal,
      _S_basefield = _S_dec | _S_oct | _S_hex,
      _S_floatfield = _S_scientific | _S_fixed,
      _S_ios_fmtflags_end = 1L << 16,
      _S_ios_fmtflags_max = 0x7fffffff,
      _S_ios_fmtflags_min = ~0x7fffffff
    };

  inline constexpr _Ios_Fmtflags
  operator&(_Ios_Fmtflags __a, _Ios_Fmtflags __b)
  { return _Ios_Fmtflags(static_cast<int>(__a) & static_cast<int>(__b)); }

  inline constexpr _Ios_Fmtflags
  operator|(_Ios_Fmtflags __a, _Ios_Fmtflags __b)
  { return _Ios_Fmtflags(static_cast<int>(__a) | static_cast<int>(__b)); }

  inline constexpr _Ios_Fmtflags
  operator^(_Ios_Fmtflags __a, _Ios_Fmtflags __b)
  { return _Ios_Fmtflags(static_cast<int>(__a) ^ static_cast<int>(__b)); }

  inline constexpr _Ios_Fmtflags
  operator~(_Ios_Fmtflags __a)
  { return _Ios_Fmtflags(~static_cast<int>(__a)); }

  inline const _Ios_Fmtflags&
  operator|=(_Ios_Fmtflags& __a, _Ios_Fmtflags __b)
  { return __a = __a | __b; }

  inline const _Ios_Fmtflags&
  operator&=(_Ios_Fmtflags& __a, _Ios_Fmtflags __b)
  { return __a = __a & __b; }

  inline const _Ios_Fmtflags&
  operator^=(_Ios_Fmtflags& __a, _Ios_Fmtflags __b)
  { return __a = __a ^ __b; }


  enum _Ios_Openmode
    {
      _S_app = 1L << 0,
      _S_ate = 1L << 1,
      _S_bin = 1L << 2,
      _S_in = 1L << 3,
      _S_out = 1L << 4,
      _S_trunc = 1L << 5,
      _S_ios_openmode_end = 1L << 16,
      _S_ios_openmode_max = 0x7fffffff,
      _S_ios_openmode_min = ~0x7fffffff
    };

  inline constexpr _Ios_Openmode
  operator&(_Ios_Openmode __a, _Ios_Openmode __b)
  { return _Ios_Openmode(static_cast<int>(__a) & static_cast<int>(__b)); }

  inline constexpr _Ios_Openmode
  operator|(_Ios_Openmode __a, _Ios_Openmode __b)
  { return _Ios_Openmode(static_cast<int>(__a) | static_cast<int>(__b)); }

  inline constexpr _Ios_Openmode
  operator^(_Ios_Openmode __a, _Ios_Openmode __b)
  { return _Ios_Openmode(static_cast<int>(__a) ^ static_cast<int>(__b)); }

  inline constexpr _Ios_Openmode
  operator~(_Ios_Openmode __a)
  { return _Ios_Openmode(~static_cast<int>(__a)); }

  inline const _Ios_Openmode&
  operator|=(_Ios_Openmode& __a, _Ios_Openmode __b)
  { return __a = __a | __b; }

  inline const _Ios_Openmode&
  operator&=(_Ios_Openmode& __a, _Ios_Openmode __b)
  { return __a = __a & __b; }

  inline const _Ios_Openmode&
  operator^=(_Ios_Openmode& __a, _Ios_Openmode __b)
  { return __a = __a ^ __b; }


  enum _Ios_Iostate
    {
      _S_goodbit = 0,
      _S_badbit = 1L << 0,
      _S_eofbit = 1L << 1,
      _S_failbit = 1L << 2,
      _S_ios_iostate_end = 1L << 16,
      _S_ios_iostate_max = 0x7fffffff,
      _S_ios_iostate_min = ~0x7fffffff
    };

  inline constexpr _Ios_Iostate
  operator&(_Ios_Iostate __a, _Ios_Iostate __b)
  { return _Ios_Iostate(static_cast<int>(__a) & static_cast<int>(__b)); }

  inline constexpr _Ios_Iostate
  operator|(_Ios_Iostate __a, _Ios_Iostate __b)
  { return _Ios_Iostate(static_cast<int>(__a) | static_cast<int>(__b)); }

  inline constexpr _Ios_Iostate
  operator^(_Ios_Iostate __a, _Ios_Iostate __b)
  { return _Ios_Iostate(static_cast<int>(__a) ^ static_cast<int>(__b)); }

  inline constexpr _Ios_Iostate
  operator~(_Ios_Iostate __a)
  { return _Ios_Iostate(~static_cast<int>(__a)); }

  inline const _Ios_Iostate&
  operator|=(_Ios_Iostate& __a, _Ios_Iostate __b)
  { return __a = __a | __b; }

  inline const _Ios_Iostate&
  operator&=(_Ios_Iostate& __a, _Ios_Iostate __b)
  { return __a = __a & __b; }

  inline const _Ios_Iostate&
  operator^=(_Ios_Iostate& __a, _Ios_Iostate __b)
  { return __a = __a ^ __b; }


  enum _Ios_Seekdir
    {
      _S_beg = 0,
      _S_cur = 1,
      _S_end = 2,
      _S_ios_seekdir_end = 1L << 16
    };



  enum class io_errc { stream = 1 };

  template <> struct is_error_code_enum<io_errc> : public true_type { };

  const error_category& iostream_category() noexcept;

  inline error_code
  make_error_code(io_errc __e) noexcept
  { return error_code(static_cast<int>(__e), iostream_category()); }

  inline error_condition
  make_error_condition(io_errc __e) noexcept
  { return error_condition(static_cast<int>(__e), iostream_category()); }
# 228 "/usr/include/c++/11/bits/ios_base.h" 3
  class ios_base
  {
# 246 "/usr/include/c++/11/bits/ios_base.h" 3
  public:
# 255 "/usr/include/c++/11/bits/ios_base.h" 3
    class __attribute ((__abi_tag__ ("cxx11"))) failure : public system_error
    {
    public:
      explicit
      failure(const string& __str);


      explicit
      failure(const string&, const error_code&);

      explicit
      failure(const char*, const error_code& = io_errc::stream);


      virtual
      ~failure() throw();

      virtual const char*
      what() const throw();
    };
# 341 "/usr/include/c++/11/bits/ios_base.h" 3
    typedef _Ios_Fmtflags fmtflags;


    static const fmtflags boolalpha = _S_boolalpha;


    static const fmtflags dec = _S_dec;


    static const fmtflags fixed = _S_fixed;


    static const fmtflags hex = _S_hex;




    static const fmtflags internal = _S_internal;



    static const fmtflags left = _S_left;


    static const fmtflags oct = _S_oct;



    static const fmtflags right = _S_right;


    static const fmtflags scientific = _S_scientific;



    static const fmtflags showbase = _S_showbase;



    static const fmtflags showpoint = _S_showpoint;


    static const fmtflags showpos = _S_showpos;


    static const fmtflags skipws = _S_skipws;


    static const fmtflags unitbuf = _S_unitbuf;



    static const fmtflags uppercase = _S_uppercase;


    static const fmtflags adjustfield = _S_adjustfield;


    static const fmtflags basefield = _S_basefield;


    static const fmtflags floatfield = _S_floatfield;
# 416 "/usr/include/c++/11/bits/ios_base.h" 3
    typedef _Ios_Iostate iostate;



    static const iostate badbit = _S_badbit;


    static const iostate eofbit = _S_eofbit;




    static const iostate failbit = _S_failbit;


    static const iostate goodbit = _S_goodbit;
# 447 "/usr/include/c++/11/bits/ios_base.h" 3
    typedef _Ios_Openmode openmode;


    static const openmode app = _S_app;


    static const openmode ate = _S_ate;




    static const openmode binary = _S_bin;


    static const openmode in = _S_in;


    static const openmode out = _S_out;


    static const openmode trunc = _S_trunc;
# 479 "/usr/include/c++/11/bits/ios_base.h" 3
    typedef _Ios_Seekdir seekdir;


    static const seekdir beg = _S_beg;


    static const seekdir cur = _S_cur;


    static const seekdir end = _S_end;
# 512 "/usr/include/c++/11/bits/ios_base.h" 3
    enum event
    {
      erase_event,
      imbue_event,
      copyfmt_event
    };
# 529 "/usr/include/c++/11/bits/ios_base.h" 3
    typedef void (*event_callback) (event __e, ios_base& __b, int __i);
# 541 "/usr/include/c++/11/bits/ios_base.h" 3
    void
    register_callback(event_callback __fn, int __index);

  protected:
    streamsize _M_precision;
    streamsize _M_width;
    fmtflags _M_flags;
    iostate _M_exception;
    iostate _M_streambuf_state;



    struct _Callback_list
    {

      _Callback_list* _M_next;
      ios_base::event_callback _M_fn;
      int _M_index;
      _Atomic_word _M_refcount;

      _Callback_list(ios_base::event_callback __fn, int __index,
       _Callback_list* __cb)
      : _M_next(__cb), _M_fn(__fn), _M_index(__index), _M_refcount(0) { }

      void
      _M_add_reference() { __gnu_cxx::__atomic_add_dispatch(&_M_refcount, 1); }


      int
      _M_remove_reference()
      {

        ;
        int __res = __gnu_cxx::__exchange_and_add_dispatch(&_M_refcount, -1);
        if (__res == 0)
          {
            ;
          }
        return __res;
      }
    };

     _Callback_list* _M_callbacks;

    void
    _M_call_callbacks(event __ev) throw();

    void
    _M_dispose_callbacks(void) throw();


    struct _Words
    {
      void* _M_pword;
      long _M_iword;
      _Words() : _M_pword(0), _M_iword(0) { }
    };


    _Words _M_word_zero;



    enum { _S_local_word_size = 8 };
    _Words _M_local_word[_S_local_word_size];


    int _M_word_size;
    _Words* _M_word;

    _Words&
    _M_grow_words(int __index, bool __iword);


    locale _M_ios_locale;

    void
    _M_init() throw();

  public:





    class Init
    {
      friend class ios_base;
    public:
      Init();
      ~Init();


      Init(const Init&) = default;
      Init& operator=(const Init&) = default;


    private:
      static _Atomic_word _S_refcount;
      static bool _S_synced_with_stdio;
    };






    fmtflags
    flags() const
    { return _M_flags; }
# 659 "/usr/include/c++/11/bits/ios_base.h" 3
    fmtflags
    flags(fmtflags __fmtfl)
    {
      fmtflags __old = _M_flags;
      _M_flags = __fmtfl;
      return __old;
    }
# 675 "/usr/include/c++/11/bits/ios_base.h" 3
    fmtflags
    setf(fmtflags __fmtfl)
    {
      fmtflags __old = _M_flags;
      _M_flags |= __fmtfl;
      return __old;
    }
# 692 "/usr/include/c++/11/bits/ios_base.h" 3
    fmtflags
    setf(fmtflags __fmtfl, fmtflags __mask)
    {
      fmtflags __old = _M_flags;
      _M_flags &= ~__mask;
      _M_flags |= (__fmtfl & __mask);
      return __old;
    }







    void
    unsetf(fmtflags __mask)
    { _M_flags &= ~__mask; }
# 718 "/usr/include/c++/11/bits/ios_base.h" 3
    streamsize
    precision() const
    { return _M_precision; }






    streamsize
    precision(streamsize __prec)
    {
      streamsize __old = _M_precision;
      _M_precision = __prec;
      return __old;
    }







    streamsize
    width() const
    { return _M_width; }






    streamsize
    width(streamsize __wide)
    {
      streamsize __old = _M_width;
      _M_width = __wide;
      return __old;
    }
# 769 "/usr/include/c++/11/bits/ios_base.h" 3
    static bool
    sync_with_stdio(bool __sync = true);
# 781 "/usr/include/c++/11/bits/ios_base.h" 3
    locale
    imbue(const locale& __loc) throw();
# 792 "/usr/include/c++/11/bits/ios_base.h" 3
    locale
    getloc() const
    { return _M_ios_locale; }
# 803 "/usr/include/c++/11/bits/ios_base.h" 3
    const locale&
    _M_getloc() const
    { return _M_ios_locale; }
# 822 "/usr/include/c++/11/bits/ios_base.h" 3
    static int
    xalloc() throw();
# 838 "/usr/include/c++/11/bits/ios_base.h" 3
    long&
    iword(int __ix)
    {
      _Words& __word = ((unsigned)__ix < (unsigned)_M_word_size)
   ? _M_word[__ix] : _M_grow_words(__ix, true);
      return __word._M_iword;
    }
# 859 "/usr/include/c++/11/bits/ios_base.h" 3
    void*&
    pword(int __ix)
    {
      _Words& __word = ((unsigned)__ix < (unsigned)_M_word_size)
   ? _M_word[__ix] : _M_grow_words(__ix, false);
      return __word._M_pword;
    }
# 876 "/usr/include/c++/11/bits/ios_base.h" 3
    virtual ~ios_base();

  protected:
    ios_base() throw ();
# 890 "/usr/include/c++/11/bits/ios_base.h" 3
  public:
    ios_base(const ios_base&) = delete;

    ios_base&
    operator=(const ios_base&) = delete;

  protected:
    void
    _M_move(ios_base&) noexcept;

    void
    _M_swap(ios_base& __rhs) noexcept;

  };



  inline ios_base&
  boolalpha(ios_base& __base)
  {
    __base.setf(ios_base::boolalpha);
    return __base;
  }


  inline ios_base&
  noboolalpha(ios_base& __base)
  {
    __base.unsetf(ios_base::boolalpha);
    return __base;
  }


  inline ios_base&
  showbase(ios_base& __base)
  {
    __base.setf(ios_base::showbase);
    return __base;
  }


  inline ios_base&
  noshowbase(ios_base& __base)
  {
    __base.unsetf(ios_base::showbase);
    return __base;
  }


  inline ios_base&
  showpoint(ios_base& __base)
  {
    __base.setf(ios_base::showpoint);
    return __base;
  }


  inline ios_base&
  noshowpoint(ios_base& __base)
  {
    __base.unsetf(ios_base::showpoint);
    return __base;
  }


  inline ios_base&
  showpos(ios_base& __base)
  {
    __base.setf(ios_base::showpos);
    return __base;
  }


  inline ios_base&
  noshowpos(ios_base& __base)
  {
    __base.unsetf(ios_base::showpos);
    return __base;
  }


  inline ios_base&
  skipws(ios_base& __base)
  {
    __base.setf(ios_base::skipws);
    return __base;
  }


  inline ios_base&
  noskipws(ios_base& __base)
  {
    __base.unsetf(ios_base::skipws);
    return __base;
  }


  inline ios_base&
  uppercase(ios_base& __base)
  {
    __base.setf(ios_base::uppercase);
    return __base;
  }


  inline ios_base&
  nouppercase(ios_base& __base)
  {
    __base.unsetf(ios_base::uppercase);
    return __base;
  }


  inline ios_base&
  unitbuf(ios_base& __base)
  {
     __base.setf(ios_base::unitbuf);
     return __base;
  }


  inline ios_base&
  nounitbuf(ios_base& __base)
  {
     __base.unsetf(ios_base::unitbuf);
     return __base;
  }



  inline ios_base&
  internal(ios_base& __base)
  {
     __base.setf(ios_base::internal, ios_base::adjustfield);
     return __base;
  }


  inline ios_base&
  left(ios_base& __base)
  {
    __base.setf(ios_base::left, ios_base::adjustfield);
    return __base;
  }


  inline ios_base&
  right(ios_base& __base)
  {
    __base.setf(ios_base::right, ios_base::adjustfield);
    return __base;
  }



  inline ios_base&
  dec(ios_base& __base)
  {
    __base.setf(ios_base::dec, ios_base::basefield);
    return __base;
  }


  inline ios_base&
  hex(ios_base& __base)
  {
    __base.setf(ios_base::hex, ios_base::basefield);
    return __base;
  }


  inline ios_base&
  oct(ios_base& __base)
  {
    __base.setf(ios_base::oct, ios_base::basefield);
    return __base;
  }



  inline ios_base&
  fixed(ios_base& __base)
  {
    __base.setf(ios_base::fixed, ios_base::floatfield);
    return __base;
  }


  inline ios_base&
  scientific(ios_base& __base)
  {
    __base.setf(ios_base::scientific, ios_base::floatfield);
    return __base;
  }






  inline ios_base&
  hexfloat(ios_base& __base)
  {
    __base.setf(ios_base::fixed | ios_base::scientific, ios_base::floatfield);
    return __base;
  }


  inline ios_base&
  defaultfloat(ios_base& __base)
  {
    __base.unsetf(ios_base::floatfield);
    return __base;
  }



}
# 43 "/usr/include/c++/11/ios" 2 3
# 1 "/usr/include/c++/11/streambuf" 1 3
# 36 "/usr/include/c++/11/streambuf" 3
       
# 37 "/usr/include/c++/11/streambuf" 3
# 45 "/usr/include/c++/11/streambuf" 3
namespace std __attribute__ ((__visibility__ ("default")))
{




  template<typename _CharT, typename _Traits>
    streamsize
    __copy_streambufs_eof(basic_streambuf<_CharT, _Traits>*,
     basic_streambuf<_CharT, _Traits>*, bool&);
# 121 "/usr/include/c++/11/streambuf" 3
  template<typename _CharT, typename _Traits>
    class basic_streambuf
    {
    public:






      typedef _CharT char_type;
      typedef _Traits traits_type;
      typedef typename traits_type::int_type int_type;
      typedef typename traits_type::pos_type pos_type;
      typedef typename traits_type::off_type off_type;




      typedef basic_streambuf<char_type, traits_type> __streambuf_type;


      friend class basic_ios<char_type, traits_type>;
      friend class basic_istream<char_type, traits_type>;
      friend class basic_ostream<char_type, traits_type>;
      friend class istreambuf_iterator<char_type, traits_type>;
      friend class ostreambuf_iterator<char_type, traits_type>;

      friend streamsize
      __copy_streambufs_eof<>(basic_streambuf*, basic_streambuf*, bool&);

      template<bool _IsMove, typename _CharT2>
        friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
            _CharT2*>::__type
        __copy_move_a2(istreambuf_iterator<_CharT2>,
         istreambuf_iterator<_CharT2>, _CharT2*);

      template<typename _CharT2>
        friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
      istreambuf_iterator<_CharT2> >::__type
        find(istreambuf_iterator<_CharT2>, istreambuf_iterator<_CharT2>,
      const _CharT2&);

      template<typename _CharT2, typename _Distance>
        friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
            void>::__type
        advance(istreambuf_iterator<_CharT2>&, _Distance);

      friend void __istream_extract(istream&, char*, streamsize);

      template<typename _CharT2, typename _Traits2, typename _Alloc>
        friend basic_istream<_CharT2, _Traits2>&
        operator>>(basic_istream<_CharT2, _Traits2>&,
     basic_string<_CharT2, _Traits2, _Alloc>&);

      template<typename _CharT2, typename _Traits2, typename _Alloc>
        friend basic_istream<_CharT2, _Traits2>&
        getline(basic_istream<_CharT2, _Traits2>&,
  basic_string<_CharT2, _Traits2, _Alloc>&, _CharT2);

    protected:







      char_type* _M_in_beg;
      char_type* _M_in_cur;
      char_type* _M_in_end;
      char_type* _M_out_beg;
      char_type* _M_out_cur;
      char_type* _M_out_end;


      locale _M_buf_locale;

  public:

      virtual
      ~basic_streambuf()
      { }
# 213 "/usr/include/c++/11/streambuf" 3
      locale
      pubimbue(const locale& __loc)
      {
 locale __tmp(this->getloc());
 this->imbue(__loc);
 _M_buf_locale = __loc;
 return __tmp;
      }
# 230 "/usr/include/c++/11/streambuf" 3
      locale
      getloc() const
      { return _M_buf_locale; }
# 243 "/usr/include/c++/11/streambuf" 3
      basic_streambuf*
      pubsetbuf(char_type* __s, streamsize __n)
      { return this->setbuf(__s, __n); }
# 255 "/usr/include/c++/11/streambuf" 3
      pos_type
      pubseekoff(off_type __off, ios_base::seekdir __way,
   ios_base::openmode __mode = ios_base::in | ios_base::out)
      { return this->seekoff(__off, __way, __mode); }
# 267 "/usr/include/c++/11/streambuf" 3
      pos_type
      pubseekpos(pos_type __sp,
   ios_base::openmode __mode = ios_base::in | ios_base::out)
      { return this->seekpos(__sp, __mode); }




      int
      pubsync() { return this->sync(); }
# 288 "/usr/include/c++/11/streambuf" 3
      streamsize
      in_avail()
      {
 const streamsize __ret = this->egptr() - this->gptr();
 return __ret ? __ret : this->showmanyc();
      }
# 302 "/usr/include/c++/11/streambuf" 3
      int_type
      snextc()
      {
 int_type __ret = traits_type::eof();
 if (__builtin_expect(!traits_type::eq_int_type(this->sbumpc(),
             __ret), true))
   __ret = this->sgetc();
 return __ret;
      }
# 320 "/usr/include/c++/11/streambuf" 3
      int_type
      sbumpc()
      {
 int_type __ret;
 if (__builtin_expect(this->gptr() < this->egptr(), true))
   {
     __ret = traits_type::to_int_type(*this->gptr());
     this->gbump(1);
   }
 else
   __ret = this->uflow();
 return __ret;
      }
# 342 "/usr/include/c++/11/streambuf" 3
      int_type
      sgetc()
      {
 int_type __ret;
 if (__builtin_expect(this->gptr() < this->egptr(), true))
   __ret = traits_type::to_int_type(*this->gptr());
 else
   __ret = this->underflow();
 return __ret;
      }
# 361 "/usr/include/c++/11/streambuf" 3
      streamsize
      sgetn(char_type* __s, streamsize __n)
      { return this->xsgetn(__s, __n); }
# 376 "/usr/include/c++/11/streambuf" 3
      int_type
      sputbackc(char_type __c)
      {
 int_type __ret;
 const bool __testpos = this->eback() < this->gptr();
 if (__builtin_expect(!__testpos ||
        !traits_type::eq(__c, this->gptr()[-1]), false))
   __ret = this->pbackfail(traits_type::to_int_type(__c));
 else
   {
     this->gbump(-1);
     __ret = traits_type::to_int_type(*this->gptr());
   }
 return __ret;
      }
# 401 "/usr/include/c++/11/streambuf" 3
      int_type
      sungetc()
      {
 int_type __ret;
 if (__builtin_expect(this->eback() < this->gptr(), true))
   {
     this->gbump(-1);
     __ret = traits_type::to_int_type(*this->gptr());
   }
 else
   __ret = this->pbackfail();
 return __ret;
      }
# 428 "/usr/include/c++/11/streambuf" 3
      int_type
      sputc(char_type __c)
      {
 int_type __ret;
 if (__builtin_expect(this->pptr() < this->epptr(), true))
   {
     *this->pptr() = __c;
     this->pbump(1);
     __ret = traits_type::to_int_type(__c);
   }
 else
   __ret = this->overflow(traits_type::to_int_type(__c));
 return __ret;
      }
# 454 "/usr/include/c++/11/streambuf" 3
      streamsize
      sputn(const char_type* __s, streamsize __n)
      { return this->xsputn(__s, __n); }

    protected:
# 468 "/usr/include/c++/11/streambuf" 3
      basic_streambuf()
      : _M_in_beg(0), _M_in_cur(0), _M_in_end(0),
      _M_out_beg(0), _M_out_cur(0), _M_out_end(0),
      _M_buf_locale(locale())
      { }
# 486 "/usr/include/c++/11/streambuf" 3
      char_type*
      eback() const { return _M_in_beg; }

      char_type*
      gptr() const { return _M_in_cur; }

      char_type*
      egptr() const { return _M_in_end; }
# 502 "/usr/include/c++/11/streambuf" 3
      void
      gbump(int __n) { _M_in_cur += __n; }
# 513 "/usr/include/c++/11/streambuf" 3
      void
      setg(char_type* __gbeg, char_type* __gnext, char_type* __gend)
      {
 _M_in_beg = __gbeg;
 _M_in_cur = __gnext;
 _M_in_end = __gend;
      }
# 533 "/usr/include/c++/11/streambuf" 3
      char_type*
      pbase() const { return _M_out_beg; }

      char_type*
      pptr() const { return _M_out_cur; }

      char_type*
      epptr() const { return _M_out_end; }
# 549 "/usr/include/c++/11/streambuf" 3
      void
      pbump(int __n) { _M_out_cur += __n; }
# 559 "/usr/include/c++/11/streambuf" 3
      void
      setp(char_type* __pbeg, char_type* __pend)
      {
 _M_out_beg = _M_out_cur = __pbeg;
 _M_out_end = __pend;
      }
# 580 "/usr/include/c++/11/streambuf" 3
      virtual void
      imbue(const locale& __loc __attribute__ ((__unused__)))
      { }
# 595 "/usr/include/c++/11/streambuf" 3
      virtual basic_streambuf<char_type,_Traits>*
      setbuf(char_type*, streamsize)
      { return this; }
# 606 "/usr/include/c++/11/streambuf" 3
      virtual pos_type
      seekoff(off_type, ios_base::seekdir,
       ios_base::openmode = ios_base::in | ios_base::out)
      { return pos_type(off_type(-1)); }
# 618 "/usr/include/c++/11/streambuf" 3
      virtual pos_type
      seekpos(pos_type,
       ios_base::openmode = ios_base::in | ios_base::out)
      { return pos_type(off_type(-1)); }
# 631 "/usr/include/c++/11/streambuf" 3
      virtual int
      sync() { return 0; }
# 653 "/usr/include/c++/11/streambuf" 3
      virtual streamsize
      showmanyc() { return 0; }
# 669 "/usr/include/c++/11/streambuf" 3
      virtual streamsize
      xsgetn(char_type* __s, streamsize __n);
# 691 "/usr/include/c++/11/streambuf" 3
      virtual int_type
      underflow()
      { return traits_type::eof(); }
# 704 "/usr/include/c++/11/streambuf" 3
      virtual int_type
      uflow()
      {
 int_type __ret = traits_type::eof();
 const bool __testeof = traits_type::eq_int_type(this->underflow(),
       __ret);
 if (!__testeof)
   {
     __ret = traits_type::to_int_type(*this->gptr());
     this->gbump(1);
   }
 return __ret;
      }
# 728 "/usr/include/c++/11/streambuf" 3
      virtual int_type
      pbackfail(int_type __c __attribute__ ((__unused__)) = traits_type::eof())
      { return traits_type::eof(); }
# 746 "/usr/include/c++/11/streambuf" 3
      virtual streamsize
      xsputn(const char_type* __s, streamsize __n);
# 772 "/usr/include/c++/11/streambuf" 3
      virtual int_type
      overflow(int_type __c __attribute__ ((__unused__)) = traits_type::eof())
      { return traits_type::eof(); }
# 799 "/usr/include/c++/11/streambuf" 3
      void
      __safe_gbump(streamsize __n) { _M_in_cur += __n; }

      void
      __safe_pbump(streamsize __n) { _M_out_cur += __n; }




    protected:

      basic_streambuf(const basic_streambuf&);

      basic_streambuf&
      operator=(const basic_streambuf&);


      void
      swap(basic_streambuf& __sb)
      {
 std::swap(_M_in_beg, __sb._M_in_beg);
 std::swap(_M_in_cur, __sb._M_in_cur);
 std::swap(_M_in_end, __sb._M_in_end);
 std::swap(_M_out_beg, __sb._M_out_beg);
 std::swap(_M_out_cur, __sb._M_out_cur);
 std::swap(_M_out_end, __sb._M_out_end);
 std::swap(_M_buf_locale, __sb._M_buf_locale);
      }

    };


  template<typename _CharT, typename _Traits>
    std::basic_streambuf<_CharT, _Traits>::
    basic_streambuf(const basic_streambuf&) = default;

  template<typename _CharT, typename _Traits>
    std::basic_streambuf<_CharT, _Traits>&
    std::basic_streambuf<_CharT, _Traits>::
    operator=(const basic_streambuf&) = default;



  template<>
    streamsize
    __copy_streambufs_eof(basic_streambuf<char>* __sbin,
     basic_streambuf<char>* __sbout, bool& __ineof);

  template<>
    streamsize
    __copy_streambufs_eof(basic_streambuf<wchar_t>* __sbin,
     basic_streambuf<wchar_t>* __sbout, bool& __ineof);





}

# 1 "/usr/include/c++/11/bits/streambuf.tcc" 1 3
# 37 "/usr/include/c++/11/bits/streambuf.tcc" 3
       
# 38 "/usr/include/c++/11/bits/streambuf.tcc" 3

namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _CharT, typename _Traits>
    streamsize
    basic_streambuf<_CharT, _Traits>::
    xsgetn(char_type* __s, streamsize __n)
    {
      streamsize __ret = 0;
      while (__ret < __n)
 {
   const streamsize __buf_len = this->egptr() - this->gptr();
   if (__buf_len)
     {
       const streamsize __remaining = __n - __ret;
       const streamsize __len = std::min(__buf_len, __remaining);
       traits_type::copy(__s, this->gptr(), __len);
       __ret += __len;
       __s += __len;
       this->__safe_gbump(__len);
     }

   if (__ret < __n)
     {
       const int_type __c = this->uflow();
       if (!traits_type::eq_int_type(__c, traits_type::eof()))
  {
    traits_type::assign(*__s++, traits_type::to_char_type(__c));
    ++__ret;
  }
       else
  break;
     }
 }
      return __ret;
    }

  template<typename _CharT, typename _Traits>
    streamsize
    basic_streambuf<_CharT, _Traits>::
    xsputn(const char_type* __s, streamsize __n)
    {
      streamsize __ret = 0;
      while (__ret < __n)
 {
   const streamsize __buf_len = this->epptr() - this->pptr();
   if (__buf_len)
     {
       const streamsize __remaining = __n - __ret;
       const streamsize __len = std::min(__buf_len, __remaining);
       traits_type::copy(this->pptr(), __s, __len);
       __ret += __len;
       __s += __len;
       this->__safe_pbump(__len);
     }

   if (__ret < __n)
     {
       int_type __c = this->overflow(traits_type::to_int_type(*__s));
       if (!traits_type::eq_int_type(__c, traits_type::eof()))
  {
    ++__ret;
    ++__s;
  }
       else
  break;
     }
 }
      return __ret;
    }




  template<typename _CharT, typename _Traits>
    streamsize
    __copy_streambufs_eof(basic_streambuf<_CharT, _Traits>* __sbin,
     basic_streambuf<_CharT, _Traits>* __sbout,
     bool& __ineof)
    {
      streamsize __ret = 0;
      __ineof = true;
      typename _Traits::int_type __c = __sbin->sgetc();
      while (!_Traits::eq_int_type(__c, _Traits::eof()))
 {
   __c = __sbout->sputc(_Traits::to_char_type(__c));
   if (_Traits::eq_int_type(__c, _Traits::eof()))
     {
       __ineof = false;
       break;
     }
   ++__ret;
   __c = __sbin->snextc();
 }
      return __ret;
    }

  template<typename _CharT, typename _Traits>
    inline streamsize
    __copy_streambufs(basic_streambuf<_CharT, _Traits>* __sbin,
        basic_streambuf<_CharT, _Traits>* __sbout)
    {
      bool __ineof;
      return __copy_streambufs_eof(__sbin, __sbout, __ineof);
    }




  extern template class basic_streambuf<char>;
  extern template
    streamsize
    __copy_streambufs(basic_streambuf<char>*,
        basic_streambuf<char>*);
  extern template
    streamsize
    __copy_streambufs_eof(basic_streambuf<char>*,
     basic_streambuf<char>*, bool&);


  extern template class basic_streambuf<wchar_t>;
  extern template
    streamsize
    __copy_streambufs(basic_streambuf<wchar_t>*,
        basic_streambuf<wchar_t>*);
  extern template
    streamsize
    __copy_streambufs_eof(basic_streambuf<wchar_t>*,
     basic_streambuf<wchar_t>*, bool&);




}
# 859 "/usr/include/c++/11/streambuf" 2 3
# 44 "/usr/include/c++/11/ios" 2 3
# 1 "/usr/include/c++/11/bits/basic_ios.h" 1 3
# 33 "/usr/include/c++/11/bits/basic_ios.h" 3
       
# 34 "/usr/include/c++/11/bits/basic_ios.h" 3



# 1 "/usr/include/c++/11/bits/locale_facets.h" 1 3
# 37 "/usr/include/c++/11/bits/locale_facets.h" 3
       
# 38 "/usr/include/c++/11/bits/locale_facets.h" 3

# 1 "/usr/include/c++/11/cwctype" 1 3
# 39 "/usr/include/c++/11/cwctype" 3
       
# 40 "/usr/include/c++/11/cwctype" 3
# 50 "/usr/include/c++/11/cwctype" 3
# 1 "/usr/include/wctype.h" 1 3 4
# 38 "/usr/include/wctype.h" 3 4
# 1 "/usr/include/bits/wctype-wchar.h" 1 3 4
# 38 "/usr/include/bits/wctype-wchar.h" 3 4
typedef unsigned long int wctype_t;
# 56 "/usr/include/bits/wctype-wchar.h" 3 4
enum
{
  __ISwupper = 0,
  __ISwlower = 1,
  __ISwalpha = 2,
  __ISwdigit = 3,
  __ISwxdigit = 4,
  __ISwspace = 5,
  __ISwprint = 6,
  __ISwgraph = 7,
  __ISwblank = 8,
  __ISwcntrl = 9,
  __ISwpunct = 10,
  __ISwalnum = 11,

  _ISwupper = ((__ISwupper) < 8 ? (int) ((1UL << (__ISwupper)) << 24) : ((__ISwupper) < 16 ? (int) ((1UL << (__ISwupper)) << 8) : ((__ISwupper) < 24 ? (int) ((1UL << (__ISwupper)) >> 8) : (int) ((1UL << (__ISwupper)) >> 24)))),
  _ISwlower = ((__ISwlower) < 8 ? (int) ((1UL << (__ISwlower)) << 24) : ((__ISwlower) < 16 ? (int) ((1UL << (__ISwlower)) << 8) : ((__ISwlower) < 24 ? (int) ((1UL << (__ISwlower)) >> 8) : (int) ((1UL << (__ISwlower)) >> 24)))),
  _ISwalpha = ((__ISwalpha) < 8 ? (int) ((1UL << (__ISwalpha)) << 24) : ((__ISwalpha) < 16 ? (int) ((1UL << (__ISwalpha)) << 8) : ((__ISwalpha) < 24 ? (int) ((1UL << (__ISwalpha)) >> 8) : (int) ((1UL << (__ISwalpha)) >> 24)))),
  _ISwdigit = ((__ISwdigit) < 8 ? (int) ((1UL << (__ISwdigit)) << 24) : ((__ISwdigit) < 16 ? (int) ((1UL << (__ISwdigit)) << 8) : ((__ISwdigit) < 24 ? (int) ((1UL << (__ISwdigit)) >> 8) : (int) ((1UL << (__ISwdigit)) >> 24)))),
  _ISwxdigit = ((__ISwxdigit) < 8 ? (int) ((1UL << (__ISwxdigit)) << 24) : ((__ISwxdigit) < 16 ? (int) ((1UL << (__ISwxdigit)) << 8) : ((__ISwxdigit) < 24 ? (int) ((1UL << (__ISwxdigit)) >> 8) : (int) ((1UL << (__ISwxdigit)) >> 24)))),
  _ISwspace = ((__ISwspace) < 8 ? (int) ((1UL << (__ISwspace)) << 24) : ((__ISwspace) < 16 ? (int) ((1UL << (__ISwspace)) << 8) : ((__ISwspace) < 24 ? (int) ((1UL << (__ISwspace)) >> 8) : (int) ((1UL << (__ISwspace)) >> 24)))),
  _ISwprint = ((__ISwprint) < 8 ? (int) ((1UL << (__ISwprint)) << 24) : ((__ISwprint) < 16 ? (int) ((1UL << (__ISwprint)) << 8) : ((__ISwprint) < 24 ? (int) ((1UL << (__ISwprint)) >> 8) : (int) ((1UL << (__ISwprint)) >> 24)))),
  _ISwgraph = ((__ISwgraph) < 8 ? (int) ((1UL << (__ISwgraph)) << 24) : ((__ISwgraph) < 16 ? (int) ((1UL << (__ISwgraph)) << 8) : ((__ISwgraph) < 24 ? (int) ((1UL << (__ISwgraph)) >> 8) : (int) ((1UL << (__ISwgraph)) >> 24)))),
  _ISwblank = ((__ISwblank) < 8 ? (int) ((1UL << (__ISwblank)) << 24) : ((__ISwblank) < 16 ? (int) ((1UL << (__ISwblank)) << 8) : ((__ISwblank) < 24 ? (int) ((1UL << (__ISwblank)) >> 8) : (int) ((1UL << (__ISwblank)) >> 24)))),
  _ISwcntrl = ((__ISwcntrl) < 8 ? (int) ((1UL << (__ISwcntrl)) << 24) : ((__ISwcntrl) < 16 ? (int) ((1UL << (__ISwcntrl)) << 8) : ((__ISwcntrl) < 24 ? (int) ((1UL << (__ISwcntrl)) >> 8) : (int) ((1UL << (__ISwcntrl)) >> 24)))),
  _ISwpunct = ((__ISwpunct) < 8 ? (int) ((1UL << (__ISwpunct)) << 24) : ((__ISwpunct) < 16 ? (int) ((1UL << (__ISwpunct)) << 8) : ((__ISwpunct) < 24 ? (int) ((1UL << (__ISwpunct)) >> 8) : (int) ((1UL << (__ISwpunct)) >> 24)))),
  _ISwalnum = ((__ISwalnum) < 8 ? (int) ((1UL << (__ISwalnum)) << 24) : ((__ISwalnum) < 16 ? (int) ((1UL << (__ISwalnum)) << 8) : ((__ISwalnum) < 24 ? (int) ((1UL << (__ISwalnum)) >> 8) : (int) ((1UL << (__ISwalnum)) >> 24))))
};



extern "C" {







extern int iswalnum (wint_t __wc) noexcept (true);





extern int iswalpha (wint_t __wc) noexcept (true);


extern int iswcntrl (wint_t __wc) noexcept (true);



extern int iswdigit (wint_t __wc) noexcept (true);



extern int iswgraph (wint_t __wc) noexcept (true);




extern int iswlower (wint_t __wc) noexcept (true);


extern int iswprint (wint_t __wc) noexcept (true);




extern int iswpunct (wint_t __wc) noexcept (true);




extern int iswspace (wint_t __wc) noexcept (true);




extern int iswupper (wint_t __wc) noexcept (true);




extern int iswxdigit (wint_t __wc) noexcept (true);





extern int iswblank (wint_t __wc) noexcept (true);
# 155 "/usr/include/bits/wctype-wchar.h" 3 4
extern wctype_t wctype (const char *__property) noexcept (true);



extern int iswctype (wint_t __wc, wctype_t __desc) noexcept (true);






extern wint_t towlower (wint_t __wc) noexcept (true);


extern wint_t towupper (wint_t __wc) noexcept (true);

}
# 39 "/usr/include/wctype.h" 2 3 4





extern "C" {



typedef const __int32_t *wctrans_t;



extern wctrans_t wctrans (const char *__property) noexcept (true);


extern wint_t towctrans (wint_t __wc, wctrans_t __desc) noexcept (true);







extern int iswalnum_l (wint_t __wc, locale_t __locale) noexcept (true);





extern int iswalpha_l (wint_t __wc, locale_t __locale) noexcept (true);


extern int iswcntrl_l (wint_t __wc, locale_t __locale) noexcept (true);



extern int iswdigit_l (wint_t __wc, locale_t __locale) noexcept (true);



extern int iswgraph_l (wint_t __wc, locale_t __locale) noexcept (true);




extern int iswlower_l (wint_t __wc, locale_t __locale) noexcept (true);


extern int iswprint_l (wint_t __wc, locale_t __locale) noexcept (true);




extern int iswpunct_l (wint_t __wc, locale_t __locale) noexcept (true);




extern int iswspace_l (wint_t __wc, locale_t __locale) noexcept (true);




extern int iswupper_l (wint_t __wc, locale_t __locale) noexcept (true);




extern int iswxdigit_l (wint_t __wc, locale_t __locale) noexcept (true);




extern int iswblank_l (wint_t __wc, locale_t __locale) noexcept (true);



extern wctype_t wctype_l (const char *__property, locale_t __locale)
     noexcept (true);



extern int iswctype_l (wint_t __wc, wctype_t __desc, locale_t __locale)
     noexcept (true);






extern wint_t towlower_l (wint_t __wc, locale_t __locale) noexcept (true);


extern wint_t towupper_l (wint_t __wc, locale_t __locale) noexcept (true);



extern wctrans_t wctrans_l (const char *__property, locale_t __locale)
     noexcept (true);


extern wint_t towctrans_l (wint_t __wc, wctrans_t __desc,
      locale_t __locale) noexcept (true);



}
# 51 "/usr/include/c++/11/cwctype" 2 3
# 80 "/usr/include/c++/11/cwctype" 3
namespace std
{
  using ::wctrans_t;
  using ::wctype_t;
  using ::wint_t;

  using ::iswalnum;
  using ::iswalpha;

  using ::iswblank;

  using ::iswcntrl;
  using ::iswctype;
  using ::iswdigit;
  using ::iswgraph;
  using ::iswlower;
  using ::iswprint;
  using ::iswpunct;
  using ::iswspace;
  using ::iswupper;
  using ::iswxdigit;
  using ::towctrans;
  using ::towlower;
  using ::towupper;
  using ::wctrans;
  using ::wctype;
}
# 40 "/usr/include/c++/11/bits/locale_facets.h" 2 3
# 1 "/usr/include/c++/11/cctype" 1 3
# 39 "/usr/include/c++/11/cctype" 3
       
# 40 "/usr/include/c++/11/cctype" 3
# 41 "/usr/include/c++/11/bits/locale_facets.h" 2 3
# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/ctype_base.h" 1 3
# 36 "/usr/include/c++/11/x86_64-redhat-linux/bits/ctype_base.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{



  struct ctype_base
  {

    typedef const int* __to_type;



    typedef unsigned short mask;
    static const mask upper = _ISupper;
    static const mask lower = _ISlower;
    static const mask alpha = _ISalpha;
    static const mask digit = _ISdigit;
    static const mask xdigit = _ISxdigit;
    static const mask space = _ISspace;
    static const mask print = _ISprint;
    static const mask graph = _ISalpha | _ISdigit | _ISpunct;
    static const mask cntrl = _IScntrl;
    static const mask punct = _ISpunct;
    static const mask alnum = _ISalpha | _ISdigit;

    static const mask blank = _ISblank;

  };


}
# 42 "/usr/include/c++/11/bits/locale_facets.h" 2 3






# 1 "/usr/include/c++/11/bits/streambuf_iterator.h" 1 3
# 33 "/usr/include/c++/11/bits/streambuf_iterator.h" 3
       
# 34 "/usr/include/c++/11/bits/streambuf_iterator.h" 3




namespace std __attribute__ ((__visibility__ ("default")))
{

# 49 "/usr/include/c++/11/bits/streambuf_iterator.h" 3
  template<typename _CharT, typename _Traits>
    class istreambuf_iterator
    : public iterator<input_iterator_tag, _CharT, typename _Traits::off_type,
        _CharT*, _CharT>
    {
    public:
# 66 "/usr/include/c++/11/bits/streambuf_iterator.h" 3
      typedef _CharT char_type;
      typedef _Traits traits_type;
      typedef typename _Traits::int_type int_type;
      typedef basic_streambuf<_CharT, _Traits> streambuf_type;
      typedef basic_istream<_CharT, _Traits> istream_type;


      template<typename _CharT2>
 friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
        ostreambuf_iterator<_CharT2> >::__type
 copy(istreambuf_iterator<_CharT2>, istreambuf_iterator<_CharT2>,
      ostreambuf_iterator<_CharT2>);

      template<bool _IsMove, typename _CharT2>
 friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
            _CharT2*>::__type
 __copy_move_a2(istreambuf_iterator<_CharT2>,
         istreambuf_iterator<_CharT2>, _CharT2*);

      template<typename _CharT2, typename _Size>
 friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
            _CharT2*>::__type
 __copy_n_a(istreambuf_iterator<_CharT2>, _Size, _CharT2*, bool);

      template<typename _CharT2>
 friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
        istreambuf_iterator<_CharT2> >::__type
 find(istreambuf_iterator<_CharT2>, istreambuf_iterator<_CharT2>,
      const _CharT2&);

      template<typename _CharT2, typename _Distance>
 friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
            void>::__type
 advance(istreambuf_iterator<_CharT2>&, _Distance);

    private:







      mutable streambuf_type* _M_sbuf;
      int_type _M_c;

    public:

      constexpr istreambuf_iterator() noexcept
      : _M_sbuf(0), _M_c(traits_type::eof()) { }







      istreambuf_iterator(const istreambuf_iterator&) noexcept = default;

      ~istreambuf_iterator() = default;



      istreambuf_iterator(istream_type& __s) noexcept
      : _M_sbuf(__s.rdbuf()), _M_c(traits_type::eof()) { }


      istreambuf_iterator(streambuf_type* __s) noexcept
      : _M_sbuf(__s), _M_c(traits_type::eof()) { }


      istreambuf_iterator&
      operator=(const istreambuf_iterator&) noexcept = default;





      char_type
      operator*() const
      {
 int_type __c = _M_get();
# 156 "/usr/include/c++/11/bits/streambuf_iterator.h" 3
 return traits_type::to_char_type(__c);
      }


      istreambuf_iterator&
      operator++()
      {



                        ;

 _M_sbuf->sbumpc();
 _M_c = traits_type::eof();
 return *this;
      }


      istreambuf_iterator
      operator++(int)
      {



                        ;

 istreambuf_iterator __old = *this;
 __old._M_c = _M_sbuf->sbumpc();
 _M_c = traits_type::eof();
 return __old;
      }





      bool
      equal(const istreambuf_iterator& __b) const
      { return _M_at_eof() == __b._M_at_eof(); }

    private:
      int_type
      _M_get() const
      {
 int_type __ret = _M_c;
 if (_M_sbuf && _S_is_eof(__ret) && _S_is_eof(__ret = _M_sbuf->sgetc()))
   _M_sbuf = 0;
 return __ret;
      }

      bool
      _M_at_eof() const
      { return _S_is_eof(_M_get()); }

      static bool
      _S_is_eof(int_type __c)
      {
 const int_type __eof = traits_type::eof();
 return traits_type::eq_int_type(__c, __eof);
      }






    };

  template<typename _CharT, typename _Traits>
    inline bool
    operator==(const istreambuf_iterator<_CharT, _Traits>& __a,
        const istreambuf_iterator<_CharT, _Traits>& __b)
    { return __a.equal(__b); }

  template<typename _CharT, typename _Traits>
    inline bool
    operator!=(const istreambuf_iterator<_CharT, _Traits>& __a,
        const istreambuf_iterator<_CharT, _Traits>& __b)
    { return !__a.equal(__b); }


  template<typename _CharT, typename _Traits>
    class ostreambuf_iterator
    : public iterator<output_iterator_tag, void, void, void, void>
    {
    public:






      typedef _CharT char_type;
      typedef _Traits traits_type;
      typedef basic_streambuf<_CharT, _Traits> streambuf_type;
      typedef basic_ostream<_CharT, _Traits> ostream_type;


      template<typename _CharT2>
 friend typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value,
        ostreambuf_iterator<_CharT2> >::__type
 copy(istreambuf_iterator<_CharT2>, istreambuf_iterator<_CharT2>,
      ostreambuf_iterator<_CharT2>);

    private:
      streambuf_type* _M_sbuf;
      bool _M_failed;

    public:
# 273 "/usr/include/c++/11/bits/streambuf_iterator.h" 3
      ostreambuf_iterator(ostream_type& __s) noexcept
      : _M_sbuf(__s.rdbuf()), _M_failed(!_M_sbuf) { }


      ostreambuf_iterator(streambuf_type* __s) noexcept
      : _M_sbuf(__s), _M_failed(!_M_sbuf) { }


      ostreambuf_iterator&
      operator=(_CharT __c)
      {
 if (!_M_failed &&
     _Traits::eq_int_type(_M_sbuf->sputc(__c), _Traits::eof()))
   _M_failed = true;
 return *this;
      }


      ostreambuf_iterator&
      operator*()
      { return *this; }


      ostreambuf_iterator&
      operator++(int)
      { return *this; }


      ostreambuf_iterator&
      operator++()
      { return *this; }


      bool
      failed() const noexcept
      { return _M_failed; }

      ostreambuf_iterator&
      _M_put(const _CharT* __ws, streamsize __len)
      {
 if (__builtin_expect(!_M_failed, true)
     && __builtin_expect(this->_M_sbuf->sputn(__ws, __len) != __len,
    false))
   _M_failed = true;
 return *this;
      }
    };


  template<typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        ostreambuf_iterator<_CharT> >::__type
    copy(istreambuf_iterator<_CharT> __first,
  istreambuf_iterator<_CharT> __last,
  ostreambuf_iterator<_CharT> __result)
    {
      if (__first._M_sbuf && !__last._M_sbuf && !__result._M_failed)
 {
   bool __ineof;
   __copy_streambufs_eof(__first._M_sbuf, __result._M_sbuf, __ineof);
   if (!__ineof)
     __result._M_failed = true;
 }
      return __result;
    }

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        ostreambuf_iterator<_CharT> >::__type
    __copy_move_a2(_CharT* __first, _CharT* __last,
     ostreambuf_iterator<_CharT> __result)
    {
      const streamsize __num = __last - __first;
      if (__num > 0)
 __result._M_put(__first, __num);
      return __result;
    }

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        ostreambuf_iterator<_CharT> >::__type
    __copy_move_a2(const _CharT* __first, const _CharT* __last,
     ostreambuf_iterator<_CharT> __result)
    {
      const streamsize __num = __last - __first;
      if (__num > 0)
 __result._M_put(__first, __num);
      return __result;
    }

  template<bool _IsMove, typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        _CharT*>::__type
    __copy_move_a2(istreambuf_iterator<_CharT> __first,
     istreambuf_iterator<_CharT> __last, _CharT* __result)
    {
      typedef istreambuf_iterator<_CharT> __is_iterator_type;
      typedef typename __is_iterator_type::traits_type traits_type;
      typedef typename __is_iterator_type::streambuf_type streambuf_type;
      typedef typename traits_type::int_type int_type;

      if (__first._M_sbuf && !__last._M_sbuf)
 {
   streambuf_type* __sb = __first._M_sbuf;
   int_type __c = __sb->sgetc();
   while (!traits_type::eq_int_type(__c, traits_type::eof()))
     {
       const streamsize __n = __sb->egptr() - __sb->gptr();
       if (__n > 1)
  {
    traits_type::copy(__result, __sb->gptr(), __n);
    __sb->__safe_gbump(__n);
    __result += __n;
    __c = __sb->underflow();
  }
       else
  {
    *__result++ = traits_type::to_char_type(__c);
    __c = __sb->snextc();
  }
     }
 }
      return __result;
    }

  template<typename _CharT, typename _Size>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        _CharT*>::__type
    __copy_n_a(istreambuf_iterator<_CharT> __it, _Size __n, _CharT* __result,
        bool __strict __attribute__((__unused__)))
    {
      if (__n == 0)
 return __result;

     

                            ;
      _CharT* __beg = __result;
      __result += __it._M_sbuf->sgetn(__beg, __n);
     

                            ;
      return __result;
    }

  template<typename _CharT>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
          istreambuf_iterator<_CharT> >::__type
    find(istreambuf_iterator<_CharT> __first,
  istreambuf_iterator<_CharT> __last, const _CharT& __val)
    {
      typedef istreambuf_iterator<_CharT> __is_iterator_type;
      typedef typename __is_iterator_type::traits_type traits_type;
      typedef typename __is_iterator_type::streambuf_type streambuf_type;
      typedef typename traits_type::int_type int_type;
      const int_type __eof = traits_type::eof();

      if (__first._M_sbuf && !__last._M_sbuf)
 {
   const int_type __ival = traits_type::to_int_type(__val);
   streambuf_type* __sb = __first._M_sbuf;
   int_type __c = __sb->sgetc();
   while (!traits_type::eq_int_type(__c, __eof)
   && !traits_type::eq_int_type(__c, __ival))
     {
       streamsize __n = __sb->egptr() - __sb->gptr();
       if (__n > 1)
  {
    const _CharT* __p = traits_type::find(__sb->gptr(),
       __n, __val);
    if (__p)
      __n = __p - __sb->gptr();
    __sb->__safe_gbump(__n);
    __c = __sb->sgetc();
  }
       else
  __c = __sb->snextc();
     }

   __first._M_c = __eof;
 }

      return __first;
    }

  template<typename _CharT, typename _Distance>
    typename __gnu_cxx::__enable_if<__is_char<_CharT>::__value,
        void>::__type
    advance(istreambuf_iterator<_CharT>& __i, _Distance __n)
    {
      if (__n == 0)
 return;

      do { if (__builtin_is_constant_evaluated() && !bool(__n > 0)) __builtin_unreachable(); } while (false);
     

                           ;

      typedef istreambuf_iterator<_CharT> __is_iterator_type;
      typedef typename __is_iterator_type::traits_type traits_type;
      typedef typename __is_iterator_type::streambuf_type streambuf_type;
      typedef typename traits_type::int_type int_type;
      const int_type __eof = traits_type::eof();

      streambuf_type* __sb = __i._M_sbuf;
      while (__n > 0)
 {
   streamsize __size = __sb->egptr() - __sb->gptr();
   if (__size > __n)
     {
       __sb->__safe_gbump(__n);
       break;
     }

   __sb->__safe_gbump(__size);
   __n -= __size;
   if (traits_type::eq_int_type(__sb->underflow(), __eof))
     {
      

                      ;
       break;
     }
 }

      __i._M_c = __eof;
    }




}
# 49 "/usr/include/c++/11/bits/locale_facets.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{

# 74 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<typename _Tp>
    void
    __convert_to_v(const char*, _Tp&, ios_base::iostate&,
     const __c_locale&) throw();


  template<>
    void
    __convert_to_v(const char*, float&, ios_base::iostate&,
     const __c_locale&) throw();

  template<>
    void
    __convert_to_v(const char*, double&, ios_base::iostate&,
     const __c_locale&) throw();

  template<>
    void
    __convert_to_v(const char*, long double&, ios_base::iostate&,
     const __c_locale&) throw();



  template<typename _CharT, typename _Traits>
    struct __pad
    {
      static void
      _S_pad(ios_base& __io, _CharT __fill, _CharT* __news,
      const _CharT* __olds, streamsize __newlen, streamsize __oldlen);
    };






  template<typename _CharT>
    _CharT*
    __add_grouping(_CharT* __s, _CharT __sep,
     const char* __gbeg, size_t __gsize,
     const _CharT* __first, const _CharT* __last);




  template<typename _CharT>
    inline
    ostreambuf_iterator<_CharT>
    __write(ostreambuf_iterator<_CharT> __s, const _CharT* __ws, int __len)
    {
      __s._M_put(__ws, __len);
      return __s;
    }


  template<typename _CharT, typename _OutIter>
    inline
    _OutIter
    __write(_OutIter __s, const _CharT* __ws, int __len)
    {
      for (int __j = 0; __j < __len; __j++, ++__s)
 *__s = __ws[__j];
      return __s;
    }
# 152 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<typename _CharT>
    class __ctype_abstract_base : public locale::facet, public ctype_base
    {
    public:


      typedef _CharT char_type;
# 171 "/usr/include/c++/11/bits/locale_facets.h" 3
      bool
      is(mask __m, char_type __c) const
      { return this->do_is(__m, __c); }
# 188 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      is(const char_type *__lo, const char_type *__hi, mask *__vec) const
      { return this->do_is(__lo, __hi, __vec); }
# 204 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      scan_is(mask __m, const char_type* __lo, const char_type* __hi) const
      { return this->do_scan_is(__m, __lo, __hi); }
# 220 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      scan_not(mask __m, const char_type* __lo, const char_type* __hi) const
      { return this->do_scan_not(__m, __lo, __hi); }
# 234 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      toupper(char_type __c) const
      { return this->do_toupper(__c); }
# 249 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      toupper(char_type *__lo, const char_type* __hi) const
      { return this->do_toupper(__lo, __hi); }
# 263 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      tolower(char_type __c) const
      { return this->do_tolower(__c); }
# 278 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      tolower(char_type* __lo, const char_type* __hi) const
      { return this->do_tolower(__lo, __hi); }
# 295 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      widen(char __c) const
      { return this->do_widen(__c); }
# 314 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char*
      widen(const char* __lo, const char* __hi, char_type* __to) const
      { return this->do_widen(__lo, __hi, __to); }
# 333 "/usr/include/c++/11/bits/locale_facets.h" 3
      char
      narrow(char_type __c, char __dfault) const
      { return this->do_narrow(__c, __dfault); }
# 355 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      narrow(const char_type* __lo, const char_type* __hi,
       char __dfault, char* __to) const
      { return this->do_narrow(__lo, __hi, __dfault, __to); }

    protected:
      explicit
      __ctype_abstract_base(size_t __refs = 0): facet(__refs) { }

      virtual
      ~__ctype_abstract_base() { }
# 380 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual bool
      do_is(mask __m, char_type __c) const = 0;
# 399 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_is(const char_type* __lo, const char_type* __hi,
     mask* __vec) const = 0;
# 418 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_scan_is(mask __m, const char_type* __lo,
   const char_type* __hi) const = 0;
# 437 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_scan_not(mask __m, const char_type* __lo,
    const char_type* __hi) const = 0;
# 455 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_toupper(char_type __c) const = 0;
# 472 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_toupper(char_type* __lo, const char_type* __hi) const = 0;
# 488 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_tolower(char_type __c) const = 0;
# 505 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_tolower(char_type* __lo, const char_type* __hi) const = 0;
# 524 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_widen(char __c) const = 0;
# 545 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char*
      do_widen(const char* __lo, const char* __hi, char_type* __to) const = 0;
# 566 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char
      do_narrow(char_type __c, char __dfault) const = 0;
# 591 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_narrow(const char_type* __lo, const char_type* __hi,
  char __dfault, char* __to) const = 0;
    };
# 614 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<typename _CharT>
    class ctype : public __ctype_abstract_base<_CharT>
    {
    public:

      typedef _CharT char_type;
      typedef typename __ctype_abstract_base<_CharT>::mask mask;


      static locale::id id;

      explicit
      ctype(size_t __refs = 0) : __ctype_abstract_base<_CharT>(__refs) { }

   protected:
      virtual
      ~ctype();

      virtual bool
      do_is(mask __m, char_type __c) const;

      virtual const char_type*
      do_is(const char_type* __lo, const char_type* __hi, mask* __vec) const;

      virtual const char_type*
      do_scan_is(mask __m, const char_type* __lo, const char_type* __hi) const;

      virtual const char_type*
      do_scan_not(mask __m, const char_type* __lo,
    const char_type* __hi) const;

      virtual char_type
      do_toupper(char_type __c) const;

      virtual const char_type*
      do_toupper(char_type* __lo, const char_type* __hi) const;

      virtual char_type
      do_tolower(char_type __c) const;

      virtual const char_type*
      do_tolower(char_type* __lo, const char_type* __hi) const;

      virtual char_type
      do_widen(char __c) const;

      virtual const char*
      do_widen(const char* __lo, const char* __hi, char_type* __dest) const;

      virtual char
      do_narrow(char_type, char __dfault) const;

      virtual const char_type*
      do_narrow(const char_type* __lo, const char_type* __hi,
  char __dfault, char* __to) const;
    };

  template<typename _CharT>
    locale::id ctype<_CharT>::id;
# 683 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<>
    class ctype<char> : public locale::facet, public ctype_base
    {
    public:


      typedef char char_type;

    protected:

      __c_locale _M_c_locale_ctype;
      bool _M_del;
      __to_type _M_toupper;
      __to_type _M_tolower;
      const mask* _M_table;
      mutable char _M_widen_ok;
      mutable char _M_widen[1 + static_cast<unsigned char>(-1)];
      mutable char _M_narrow[1 + static_cast<unsigned char>(-1)];
      mutable char _M_narrow_ok;


    public:

      static locale::id id;

      static const size_t table_size = 1 + static_cast<unsigned char>(-1);
# 720 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      ctype(const mask* __table = 0, bool __del = false, size_t __refs = 0);
# 733 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      ctype(__c_locale __cloc, const mask* __table = 0, bool __del = false,
     size_t __refs = 0);
# 746 "/usr/include/c++/11/bits/locale_facets.h" 3
      inline bool
      is(mask __m, char __c) const;
# 761 "/usr/include/c++/11/bits/locale_facets.h" 3
      inline const char*
      is(const char* __lo, const char* __hi, mask* __vec) const;
# 775 "/usr/include/c++/11/bits/locale_facets.h" 3
      inline const char*
      scan_is(mask __m, const char* __lo, const char* __hi) const;
# 789 "/usr/include/c++/11/bits/locale_facets.h" 3
      inline const char*
      scan_not(mask __m, const char* __lo, const char* __hi) const;
# 804 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      toupper(char_type __c) const
      { return this->do_toupper(__c); }
# 821 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      toupper(char_type *__lo, const char_type* __hi) const
      { return this->do_toupper(__lo, __hi); }
# 837 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      tolower(char_type __c) const
      { return this->do_tolower(__c); }
# 854 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      tolower(char_type* __lo, const char_type* __hi) const
      { return this->do_tolower(__lo, __hi); }
# 874 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      widen(char __c) const
      {
 if (_M_widen_ok)
   return _M_widen[static_cast<unsigned char>(__c)];
 this->_M_widen_init();
 return this->do_widen(__c);
      }
# 901 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char*
      widen(const char* __lo, const char* __hi, char_type* __to) const
      {
 if (_M_widen_ok == 1)
   {
     if (__builtin_expect(__hi != __lo, true))
       __builtin_memcpy(__to, __lo, __hi - __lo);
     return __hi;
   }
 if (!_M_widen_ok)
   _M_widen_init();
 return this->do_widen(__lo, __hi, __to);
      }
# 933 "/usr/include/c++/11/bits/locale_facets.h" 3
      char
      narrow(char_type __c, char __dfault) const
      {
 if (_M_narrow[static_cast<unsigned char>(__c)])
   return _M_narrow[static_cast<unsigned char>(__c)];
 const char __t = do_narrow(__c, __dfault);
 if (__t != __dfault)
   _M_narrow[static_cast<unsigned char>(__c)] = __t;
 return __t;
      }
# 966 "/usr/include/c++/11/bits/locale_facets.h" 3
      const char_type*
      narrow(const char_type* __lo, const char_type* __hi,
      char __dfault, char* __to) const
      {
 if (__builtin_expect(_M_narrow_ok == 1, true))
   {
     if (__builtin_expect(__hi != __lo, true))
       __builtin_memcpy(__to, __lo, __hi - __lo);
     return __hi;
   }
 if (!_M_narrow_ok)
   _M_narrow_init();
 return this->do_narrow(__lo, __hi, __dfault, __to);
      }





      const mask*
      table() const throw()
      { return _M_table; }


      static const mask*
      classic_table() throw();
    protected:







      virtual
      ~ctype();
# 1016 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_toupper(char_type __c) const;
# 1033 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_toupper(char_type* __lo, const char_type* __hi) const;
# 1049 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_tolower(char_type __c) const;
# 1066 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_tolower(char_type* __lo, const char_type* __hi) const;
# 1086 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_widen(char __c) const
      { return __c; }
# 1109 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char*
      do_widen(const char* __lo, const char* __hi, char_type* __to) const
      {
 if (__builtin_expect(__hi != __lo, true))
   __builtin_memcpy(__to, __lo, __hi - __lo);
 return __hi;
      }
# 1136 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char
      do_narrow(char_type __c, char __dfault __attribute__((__unused__))) const
      { return __c; }
# 1162 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_narrow(const char_type* __lo, const char_type* __hi,
  char __dfault __attribute__((__unused__)), char* __to) const
      {
 if (__builtin_expect(__hi != __lo, true))
   __builtin_memcpy(__to, __lo, __hi - __lo);
 return __hi;
      }

    private:
      void _M_narrow_init() const;
      void _M_widen_init() const;
    };
# 1188 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<>
    class ctype<wchar_t> : public __ctype_abstract_base<wchar_t>
    {
    public:


      typedef wchar_t char_type;
      typedef wctype_t __wmask_type;

    protected:
      __c_locale _M_c_locale_ctype;


      bool _M_narrow_ok;
      char _M_narrow[128];
      wint_t _M_widen[1 + static_cast<unsigned char>(-1)];


      mask _M_bit[16];
      __wmask_type _M_wmask[16];

    public:


      static locale::id id;
# 1221 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      ctype(size_t __refs = 0);
# 1232 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      ctype(__c_locale __cloc, size_t __refs = 0);

    protected:
      __wmask_type
      _M_convert_to_wmask(const mask __m) const throw();


      virtual
      ~ctype();
# 1256 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual bool
      do_is(mask __m, char_type __c) const;
# 1275 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_is(const char_type* __lo, const char_type* __hi, mask* __vec) const;
# 1293 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_scan_is(mask __m, const char_type* __lo, const char_type* __hi) const;
# 1311 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_scan_not(mask __m, const char_type* __lo,
    const char_type* __hi) const;
# 1328 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_toupper(char_type __c) const;
# 1345 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_toupper(char_type* __lo, const char_type* __hi) const;
# 1361 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_tolower(char_type __c) const;
# 1378 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_tolower(char_type* __lo, const char_type* __hi) const;
# 1398 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_widen(char __c) const;
# 1420 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char*
      do_widen(const char* __lo, const char* __hi, char_type* __to) const;
# 1443 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char
      do_narrow(char_type __c, char __dfault) const;
# 1469 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual const char_type*
      do_narrow(const char_type* __lo, const char_type* __hi,
  char __dfault, char* __to) const;


      void
      _M_initialize_ctype() throw();
    };



  template<typename _CharT>
    class ctype_byname : public ctype<_CharT>
    {
    public:
      typedef typename ctype<_CharT>::mask mask;

      explicit
      ctype_byname(const char* __s, size_t __refs = 0);


      explicit
      ctype_byname(const string& __s, size_t __refs = 0)
      : ctype_byname(__s.c_str(), __refs) { }


    protected:
      virtual
      ~ctype_byname() { }
    };


  template<>
    class ctype_byname<char> : public ctype<char>
    {
    public:
      explicit
      ctype_byname(const char* __s, size_t __refs = 0);


      explicit
      ctype_byname(const string& __s, size_t __refs = 0);


    protected:
      virtual
      ~ctype_byname();
    };


  template<>
    class ctype_byname<wchar_t> : public ctype<wchar_t>
    {
    public:
      explicit
      ctype_byname(const char* __s, size_t __refs = 0);


      explicit
      ctype_byname(const string& __s, size_t __refs = 0);


    protected:
      virtual
      ~ctype_byname();
    };



}


# 1 "/usr/include/c++/11/x86_64-redhat-linux/bits/ctype_inline.h" 1 3
# 37 "/usr/include/c++/11/x86_64-redhat-linux/bits/ctype_inline.h" 3
namespace std __attribute__ ((__visibility__ ("default")))
{


  bool
  ctype<char>::
  is(mask __m, char __c) const
  { return _M_table[static_cast<unsigned char>(__c)] & __m; }

  const char*
  ctype<char>::
  is(const char* __low, const char* __high, mask* __vec) const
  {
    while (__low < __high)
      *__vec++ = _M_table[static_cast<unsigned char>(*__low++)];
    return __high;
  }

  const char*
  ctype<char>::
  scan_is(mask __m, const char* __low, const char* __high) const
  {
    while (__low < __high
    && !(_M_table[static_cast<unsigned char>(*__low)] & __m))
      ++__low;
    return __low;
  }

  const char*
  ctype<char>::
  scan_not(mask __m, const char* __low, const char* __high) const
  {
    while (__low < __high
    && (_M_table[static_cast<unsigned char>(*__low)] & __m) != 0)
      ++__low;
    return __low;
  }


}
# 1542 "/usr/include/c++/11/bits/locale_facets.h" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{



  class __num_base
  {
  public:


    enum
      {
 _S_ominus,
 _S_oplus,
 _S_ox,
 _S_oX,
 _S_odigits,
 _S_odigits_end = _S_odigits + 16,
 _S_oudigits = _S_odigits_end,
 _S_oudigits_end = _S_oudigits + 16,
 _S_oe = _S_odigits + 14,
 _S_oE = _S_oudigits + 14,
 _S_oend = _S_oudigits_end
      };






    static const char* _S_atoms_out;



    static const char* _S_atoms_in;

    enum
    {
      _S_iminus,
      _S_iplus,
      _S_ix,
      _S_iX,
      _S_izero,
      _S_ie = _S_izero + 14,
      _S_iE = _S_izero + 20,
      _S_iend = 26
    };



    static void
    _S_format_float(const ios_base& __io, char* __fptr, char __mod) throw();
  };

  template<typename _CharT>
    struct __numpunct_cache : public locale::facet
    {
      const char* _M_grouping;
      size_t _M_grouping_size;
      bool _M_use_grouping;
      const _CharT* _M_truename;
      size_t _M_truename_size;
      const _CharT* _M_falsename;
      size_t _M_falsename_size;
      _CharT _M_decimal_point;
      _CharT _M_thousands_sep;





      _CharT _M_atoms_out[__num_base::_S_oend];





      _CharT _M_atoms_in[__num_base::_S_iend];

      bool _M_allocated;

      __numpunct_cache(size_t __refs = 0)
      : facet(__refs), _M_grouping(0), _M_grouping_size(0),
 _M_use_grouping(false),
 _M_truename(0), _M_truename_size(0), _M_falsename(0),
 _M_falsename_size(0), _M_decimal_point(_CharT()),
 _M_thousands_sep(_CharT()), _M_allocated(false)
 { }

      ~__numpunct_cache();

      void
      _M_cache(const locale& __loc);

    private:
      __numpunct_cache&
      operator=(const __numpunct_cache&);

      explicit
      __numpunct_cache(const __numpunct_cache&);
    };

  template<typename _CharT>
    __numpunct_cache<_CharT>::~__numpunct_cache()
    {
      if (_M_allocated)
 {
   delete [] _M_grouping;
   delete [] _M_truename;
   delete [] _M_falsename;
 }
    }

namespace __cxx11 {
# 1672 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<typename _CharT>
    class numpunct : public locale::facet
    {
    public:



      typedef _CharT char_type;
      typedef basic_string<_CharT> string_type;

      typedef __numpunct_cache<_CharT> __cache_type;

    protected:
      __cache_type* _M_data;

    public:

      static locale::id id;






      explicit
      numpunct(size_t __refs = 0)
      : facet(__refs), _M_data(0)
      { _M_initialize_numpunct(); }
# 1710 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      numpunct(__cache_type* __cache, size_t __refs = 0)
      : facet(__refs), _M_data(__cache)
      { _M_initialize_numpunct(); }
# 1724 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      numpunct(__c_locale __cloc, size_t __refs = 0)
      : facet(__refs), _M_data(0)
      { _M_initialize_numpunct(__cloc); }
# 1738 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      decimal_point() const
      { return this->do_decimal_point(); }
# 1751 "/usr/include/c++/11/bits/locale_facets.h" 3
      char_type
      thousands_sep() const
      { return this->do_thousands_sep(); }
# 1782 "/usr/include/c++/11/bits/locale_facets.h" 3
      string
      grouping() const
      { return this->do_grouping(); }
# 1795 "/usr/include/c++/11/bits/locale_facets.h" 3
      string_type
      truename() const
      { return this->do_truename(); }
# 1808 "/usr/include/c++/11/bits/locale_facets.h" 3
      string_type
      falsename() const
      { return this->do_falsename(); }

    protected:

      virtual
      ~numpunct();
# 1825 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_decimal_point() const
      { return _M_data->_M_decimal_point; }
# 1837 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual char_type
      do_thousands_sep() const
      { return _M_data->_M_thousands_sep; }
# 1850 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual string
      do_grouping() const
      { return _M_data->_M_grouping; }
# 1863 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual string_type
      do_truename() const
      { return _M_data->_M_truename; }
# 1876 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual string_type
      do_falsename() const
      { return _M_data->_M_falsename; }


      void
      _M_initialize_numpunct(__c_locale __cloc = 0);
    };

  template<typename _CharT>
    locale::id numpunct<_CharT>::id;

  template<>
    numpunct<char>::~numpunct();

  template<>
    void
    numpunct<char>::_M_initialize_numpunct(__c_locale __cloc);


  template<>
    numpunct<wchar_t>::~numpunct();

  template<>
    void
    numpunct<wchar_t>::_M_initialize_numpunct(__c_locale __cloc);



  template<typename _CharT>
    class numpunct_byname : public numpunct<_CharT>
    {
    public:
      typedef _CharT char_type;
      typedef basic_string<_CharT> string_type;

      explicit
      numpunct_byname(const char* __s, size_t __refs = 0)
      : numpunct<_CharT>(__refs)
      {
 if (__builtin_strcmp(__s, "C") != 0
     && __builtin_strcmp(__s, "POSIX") != 0)
   {
     __c_locale __tmp;
     this->_S_create_c_locale(__tmp, __s);
     this->_M_initialize_numpunct(__tmp);
     this->_S_destroy_c_locale(__tmp);
   }
      }


      explicit
      numpunct_byname(const string& __s, size_t __refs = 0)
      : numpunct_byname(__s.c_str(), __refs) { }


    protected:
      virtual
      ~numpunct_byname() { }
    };

}


# 1954 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<typename _CharT, typename _InIter>
    class num_get : public locale::facet
    {
    public:



      typedef _CharT char_type;
      typedef _InIter iter_type;



      static locale::id id;
# 1975 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      num_get(size_t __refs = 0) : facet(__refs) { }
# 2001 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, bool& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }
# 2038 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, long& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, unsigned short& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, unsigned int& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, unsigned long& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }


      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, long long& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, unsigned long long& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }
# 2098 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, float& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, double& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, long double& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }
# 2141 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      get(iter_type __in, iter_type __end, ios_base& __io,
   ios_base::iostate& __err, void*& __v) const
      { return this->do_get(__in, __end, __io, __err, __v); }

    protected:

      virtual ~num_get() { }

      __attribute ((__abi_tag__ ("cxx11")))
      iter_type
      _M_extract_float(iter_type, iter_type, ios_base&, ios_base::iostate&,
         string&) const;

      template<typename _ValueT>
 __attribute ((__abi_tag__ ("cxx11")))
 iter_type
 _M_extract_int(iter_type, iter_type, ios_base&, ios_base::iostate&,
         _ValueT&) const;

      template<typename _CharT2>
      typename __gnu_cxx::__enable_if<__is_char<_CharT2>::__value, int>::__type
 _M_find(const _CharT2*, size_t __len, _CharT2 __c) const
 {
   int __ret = -1;
   if (__len <= 10)
     {
       if (__c >= _CharT2('0') && __c < _CharT2(_CharT2('0') + __len))
  __ret = __c - _CharT2('0');
     }
   else
     {
       if (__c >= _CharT2('0') && __c <= _CharT2('9'))
  __ret = __c - _CharT2('0');
       else if (__c >= _CharT2('a') && __c <= _CharT2('f'))
  __ret = 10 + (__c - _CharT2('a'));
       else if (__c >= _CharT2('A') && __c <= _CharT2('F'))
  __ret = 10 + (__c - _CharT2('A'));
     }
   return __ret;
 }

      template<typename _CharT2>
      typename __gnu_cxx::__enable_if<!__is_char<_CharT2>::__value,
          int>::__type
 _M_find(const _CharT2* __zero, size_t __len, _CharT2 __c) const
 {
   int __ret = -1;
   const char_type* __q = char_traits<_CharT2>::find(__zero, __len, __c);
   if (__q)
     {
       __ret = __q - __zero;
       if (__ret > 15)
  __ret -= 6;
     }
   return __ret;
 }
# 2214 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual iter_type
      do_get(iter_type, iter_type, ios_base&, ios_base::iostate&, bool&) const;

      virtual iter_type
      do_get(iter_type __beg, iter_type __end, ios_base& __io,
      ios_base::iostate& __err, long& __v) const
      { return _M_extract_int(__beg, __end, __io, __err, __v); }

      virtual iter_type
      do_get(iter_type __beg, iter_type __end, ios_base& __io,
      ios_base::iostate& __err, unsigned short& __v) const
      { return _M_extract_int(__beg, __end, __io, __err, __v); }

      virtual iter_type
      do_get(iter_type __beg, iter_type __end, ios_base& __io,
      ios_base::iostate& __err, unsigned int& __v) const
      { return _M_extract_int(__beg, __end, __io, __err, __v); }

      virtual iter_type
      do_get(iter_type __beg, iter_type __end, ios_base& __io,
      ios_base::iostate& __err, unsigned long& __v) const
      { return _M_extract_int(__beg, __end, __io, __err, __v); }


      virtual iter_type
      do_get(iter_type __beg, iter_type __end, ios_base& __io,
      ios_base::iostate& __err, long long& __v) const
      { return _M_extract_int(__beg, __end, __io, __err, __v); }

      virtual iter_type
      do_get(iter_type __beg, iter_type __end, ios_base& __io,
      ios_base::iostate& __err, unsigned long long& __v) const
      { return _M_extract_int(__beg, __end, __io, __err, __v); }


      virtual iter_type
      do_get(iter_type, iter_type, ios_base&, ios_base::iostate&, float&) const;

      virtual iter_type
      do_get(iter_type, iter_type, ios_base&, ios_base::iostate&,
      double&) const;
# 2266 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual iter_type
      do_get(iter_type, iter_type, ios_base&, ios_base::iostate&,
      long double&) const;


      virtual iter_type
      do_get(iter_type, iter_type, ios_base&, ios_base::iostate&, void*&) const;
# 2294 "/usr/include/c++/11/bits/locale_facets.h" 3
    };

  template<typename _CharT, typename _InIter>
    locale::id num_get<_CharT, _InIter>::id;
# 2312 "/usr/include/c++/11/bits/locale_facets.h" 3
  template<typename _CharT, typename _OutIter>
    class num_put : public locale::facet
    {
    public:



      typedef _CharT char_type;
      typedef _OutIter iter_type;



      static locale::id id;
# 2333 "/usr/include/c++/11/bits/locale_facets.h" 3
      explicit
      num_put(size_t __refs = 0) : facet(__refs) { }
# 2351 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill, bool __v) const
      { return this->do_put(__s, __io, __fill, __v); }
# 2393 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill, long __v) const
      { return this->do_put(__s, __io, __fill, __v); }

      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill,
   unsigned long __v) const
      { return this->do_put(__s, __io, __fill, __v); }


      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill, long long __v) const
      { return this->do_put(__s, __io, __fill, __v); }

      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill,
   unsigned long long __v) const
      { return this->do_put(__s, __io, __fill, __v); }
# 2456 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill, double __v) const
      { return this->do_put(__s, __io, __fill, __v); }

      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill,
   long double __v) const
      { return this->do_put(__s, __io, __fill, __v); }
# 2481 "/usr/include/c++/11/bits/locale_facets.h" 3
      iter_type
      put(iter_type __s, ios_base& __io, char_type __fill,
   const void* __v) const
      { return this->do_put(__s, __io, __fill, __v); }

    protected:
      template<typename _ValueT>
 iter_type
 _M_insert_float(iter_type, ios_base& __io, char_type __fill,
   char __mod, _ValueT __v) const;

      void
      _M_group_float(const char* __grouping, size_t __grouping_size,
       char_type __sep, const char_type* __p, char_type* __new,
       char_type* __cs, int& __len) const;

      template<typename _ValueT>
 iter_type
 _M_insert_int(iter_type, ios_base& __io, char_type __fill,
        _ValueT __v) const;

      void
      _M_group_int(const char* __grouping, size_t __grouping_size,
     char_type __sep, ios_base& __io, char_type* __new,
     char_type* __cs, int& __len) const;

      void
      _M_pad(char_type __fill, streamsize __w, ios_base& __io,
      char_type* __new, const char_type* __cs, int& __len) const;


      virtual
      ~num_put() { }
# 2529 "/usr/include/c++/11/bits/locale_facets.h" 3
      virtual iter_type
      do_put(iter_type __s, ios_base& __io, char_type __fill, bool __v) const;

      virtual iter_type
      do_put(iter_type __s, ios_base& __io, char_type __fill, long __v) const
      { return _M_insert_int(__s, __io, __fill, __v); }

      virtual iter_type
      do_put(iter_type __s, ios_base& __io, char_type __fill,
      unsigned long __v) const
      { return _M_insert_int(__s, __io, __fill, __v); }


      virtual iter_type
      do_put(iter_type __s, ios_base& __io, char_type __fill,
      long long __v) const
      { return _M_insert_int(__s, __io, __fill, __v); }

      virtual iter_type
      do_put(iter_type __s, ios_base& __io, char_type __fill,
      unsigned long long __v) const
      { return _M_insert_int(__s, __io, __fill, __v); }


      virtual iter_type
      do_put(iter_type, ios_base&, char_type, double) const;






      virtual iter_type
      do_put(iter_type, ios_base&, char_type, long double) const;


      virtual iter_type
      do_put(iter_type, ios_base&, char_type, const void*) const;
# 2581 "/usr/include/c++/11/bits/locale_facets.h" 3
    };

  template <typename _CharT, typename _OutIter>
    locale::id num_put<_CharT, _OutIter>::id;









  template<typename _CharT>
    inline bool
    isspace(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::space, __c); }


  template<typename _CharT>
    inline bool
    isprint(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::print, __c); }


  template<typename _CharT>
    inline bool
    iscntrl(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::cntrl, __c); }


  template<typename _CharT>
    inline bool
    isupper(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::upper, __c); }


  template<typename _CharT>
    inline bool
    islower(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::lower, __c); }


  template<typename _CharT>
    inline bool
    isalpha(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::alpha, __c); }


  template<typename _CharT>
    inline bool
    isdigit(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::digit, __c); }


  template<typename _CharT>
    inline bool
    ispunct(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::punct, __c); }


  template<typename _CharT>
    inline bool
    isxdigit(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::xdigit, __c); }


  template<typename _CharT>
    inline bool
    isalnum(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::alnum, __c); }


  template<typename _CharT>
    inline bool
    isgraph(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::graph, __c); }



  template<typename _CharT>
    inline bool
    isblank(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).is(ctype_base::blank, __c); }



  template<typename _CharT>
    inline _CharT
    toupper(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).toupper(__c); }


  template<typename _CharT>
    inline _CharT
    tolower(_CharT __c, const locale& __loc)
    { return use_facet<ctype<_CharT> >(__loc).tolower(__c); }


}

# 1 "/usr/include/c++/11/bits/locale_facets.tcc" 1 3
# 33 "/usr/include/c++/11/bits/locale_facets.tcc" 3
       
# 34 "/usr/include/c++/11/bits/locale_facets.tcc" 3

namespace std __attribute__ ((__visibility__ ("default")))
{




  template<typename _Facet>
    struct __use_cache
    {
      const _Facet*
      operator() (const locale& __loc) const;
    };


  template<typename _CharT>
    struct __use_cache<__numpunct_cache<_CharT> >
    {
      const __numpunct_cache<_CharT>*
      operator() (const locale& __loc) const
      {
 const size_t __i = numpunct<_CharT>::id._M_id();
 const locale::facet** __caches = __loc._M_impl->_M_caches;
 if (!__caches[__i])
   {
     __numpunct_cache<_CharT>* __tmp = 0;
     try
       {
  __tmp = new __numpunct_cache<_CharT>;
  __tmp->_M_cache(__loc);
       }
     catch(...)
       {
  delete __tmp;
  throw;
       }
     __loc._M_impl->_M_install_cache(__tmp, __i);
   }
 return static_cast<const __numpunct_cache<_CharT>*>(__caches[__i]);
      }
    };

  template<typename _CharT>
    void
    __numpunct_cache<_CharT>::_M_cache(const locale& __loc)
    {
      const numpunct<_CharT>& __np = use_facet<numpunct<_CharT> >(__loc);

      char* __grouping = 0;
      _CharT* __truename = 0;
      _CharT* __falsename = 0;
      try
 {
   const string& __g = __np.grouping();
   _M_grouping_size = __g.size();
   __grouping = new char[_M_grouping_size];
   __g.copy(__grouping, _M_grouping_size);
   _M_use_grouping = (_M_grouping_size
        && static_cast<signed char>(__grouping[0]) > 0
        && (__grouping[0]
     != __gnu_cxx::__numeric_traits<char>::__max));

   const basic_string<_CharT>& __tn = __np.truename();
   _M_truename_size = __tn.size();
   __truename = new _CharT[_M_truename_size];
   __tn.copy(__truename, _M_truename_size);

   const basic_string<_CharT>& __fn = __np.falsename();
   _M_falsename_size = __fn.size();
   __falsename = new _CharT[_M_falsename_size];
   __fn.copy(__falsename, _M_falsename_size);

   _M_decimal_point = __np.decimal_point();
   _M_thousands_sep = __np.thousands_sep();

   const ctype<_CharT>& __ct = use_facet<ctype<_CharT> >(__loc);
   __ct.widen(__num_base::_S_atoms_out,
       __num_base::_S_atoms_out
       + __num_base::_S_oend, _M_atoms_out);
   __ct.widen(__num_base::_S_atoms_in,
       __num_base::_S_atoms_in
       + __num_base::_S_iend, _M_atoms_in);

   _M_grouping = __grouping;
   _M_truename = __truename;
   _M_falsename = __falsename;
   _M_allocated = true;
 }
      catch(...)
 {
   delete [] __grouping;
   delete [] __truename;
   delete [] __falsename;
   throw;
 }
    }
# 139 "/usr/include/c++/11/bits/locale_facets.tcc" 3
  __attribute__ ((__pure__)) bool
  __verify_grouping(const char* __grouping, size_t __grouping_size,
      const string& __grouping_tmp) throw ();



  template<typename _CharT, typename _InIter>
    __attribute ((__abi_tag__ ("cxx11")))
    _InIter
    num_get<_CharT, _InIter>::
    _M_extract_float(_InIter __beg, _InIter __end, ios_base& __io,
       ios_base::iostate& __err, string& __xtrc) const
    {
      typedef char_traits<_CharT> __traits_type;
      typedef __numpunct_cache<_CharT> __cache_type;
      __use_cache<__cache_type> __uc;
      const locale& __loc = __io._M_getloc();
      const __cache_type* __lc = __uc(__loc);
      const _CharT* __lit = __lc->_M_atoms_in;
      char_type __c = char_type();


      bool __testeof = __beg == __end;


      if (!__testeof)
 {
   __c = *__beg;
   const bool __plus = __c == __lit[__num_base::_S_iplus];
   if ((__plus || __c == __lit[__num_base::_S_iminus])
       && !(__lc->_M_use_grouping && __c == __lc->_M_thousands_sep)
       && !(__c == __lc->_M_decimal_point))
     {
       __xtrc += __plus ? '+' : '-';
       if (++__beg != __end)
  __c = *__beg;
       else
  __testeof = true;
     }
 }


      bool __found_mantissa = false;
      int __sep_pos = 0;
      while (!__testeof)
 {
   if ((__lc->_M_use_grouping && __c == __lc->_M_thousands_sep)
       || __c == __lc->_M_decimal_point)
     break;
   else if (__c == __lit[__num_base::_S_izero])
     {
       if (!__found_mantissa)
  {
    __xtrc += '0';
    __found_mantissa = true;
  }
       ++__sep_pos;

       if (++__beg != __end)
  __c = *__beg;
       else
  __testeof = true;
     }
   else
     break;
 }


      bool __found_dec = false;
      bool __found_sci = false;
      string __found_grouping;
      if (__lc->_M_use_grouping)
 __found_grouping.reserve(32);
      const char_type* __lit_zero = __lit + __num_base::_S_izero;

      if (!__lc->_M_allocated)

 while (!__testeof)
   {
     const int __digit = _M_find(__lit_zero, 10, __c);
     if (__digit != -1)
       {
  __xtrc += '0' + __digit;
  __found_mantissa = true;
       }
     else if (__c == __lc->_M_decimal_point
       && !__found_dec && !__found_sci)
       {
  __xtrc += '.';
  __found_dec = true;
       }
     else if ((__c == __lit[__num_base::_S_ie]
        || __c == __lit[__num_base::_S_iE])
       && !__found_sci && __found_mantissa)
       {

  __xtrc += 'e';
  __found_sci = true;


  if (++__beg != __end)
    {
      __c = *__beg;
      const bool __plus = __c == __lit[__num_base::_S_iplus];
      if (__plus || __c == __lit[__num_base::_S_iminus])
        __xtrc += __plus ? '+' : '-';
      else
        continue;
    }
  else
    {
      __testeof = true;
      break;
    }
       }
     else
       break;

     if (++__beg != __end)
       __c = *__beg;
     else
       __testeof = true;
   }
      else
 while (!__testeof)
   {


     if (__lc->_M_use_grouping && __c == __lc->_M_thousands_sep)
       {
  if (!__found_dec && !__found_sci)
    {


      if (__sep_pos)
        {
   __found_grouping += static_cast<char>(__sep_pos);
   __sep_pos = 0;
        }
      else
        {


   __xtrc.clear();
   break;
        }
    }
  else
    break;
       }
     else if (__c == __lc->_M_decimal_point)
       {
  if (!__found_dec && !__found_sci)
    {



      if (__found_grouping.size())
        __found_grouping += static_cast<char>(__sep_pos);
      __xtrc += '.';
      __found_dec = true;
    }
  else
    break;
       }
     else
       {
  const char_type* __q =
    __traits_type::find(__lit_zero, 10, __c);
  if (__q)
    {
      __xtrc += '0' + (__q - __lit_zero);
      __found_mantissa = true;
      ++__sep_pos;
    }
  else if ((__c == __lit[__num_base::_S_ie]
     || __c == __lit[__num_base::_S_iE])
    && !__found_sci && __found_mantissa)
    {

      if (__found_grouping.size() && !__found_dec)
        __found_grouping += static_cast<char>(__sep_pos);
      __xtrc += 'e';
      __found_sci = true;


      if (++__beg != __end)
        {
   __c = *__beg;
   const bool __plus = __c == __lit[__num_base::_S_iplus];
   if ((__plus || __c == __lit[__num_base::_S_iminus])
       && !(__lc->_M_use_grouping
     && __c == __lc->_M_thousands_sep)
       && !(__c == __lc->_M_decimal_point))
        __xtrc += __plus ? '+' : '-';
   else
     continue;
        }
      else
        {
   __testeof = true;
   break;
        }
    }
  else
    break;
       }

     if (++__beg != __end)
       __c = *__beg;
     else
       __testeof = true;
   }



      if (__found_grouping.size())
        {

   if (!__found_dec && !__found_sci)
     __found_grouping += static_cast<char>(__sep_pos);

          if (!std::__verify_grouping(__lc->_M_grouping,
          __lc->_M_grouping_size,
          __found_grouping))
     __err = ios_base::failbit;
        }

      return __beg;
    }

  template<typename _CharT, typename _InIter>
    template<typename _ValueT>
      __attribute ((__abi_tag__ ("cxx11")))
      _InIter
      num_get<_CharT, _InIter>::
      _M_extract_int(_InIter __beg, _InIter __end, ios_base& __io,
       ios_base::iostate& __err, _ValueT& __v) const
      {
        typedef char_traits<_CharT> __traits_type;
 using __gnu_cxx::__add_unsigned;
 typedef typename __add_unsigned<_ValueT>::__type __unsigned_type;
 typedef __numpunct_cache<_CharT> __cache_type;
 __use_cache<__cache_type> __uc;
 const locale& __loc = __io._M_getloc();
 const __cache_type* __lc = __uc(__loc);
 const _CharT* __lit = __lc->_M_atoms_in;
 char_type __c = char_type();


 const ios_base::fmtflags __basefield = __io.flags()
                                        & ios_base::basefield;
 const bool __oct = __basefield == ios_base::oct;
 int __base = __oct ? 8 : (__basefield == ios_base::hex ? 16 : 10);


 bool __testeof = __beg == __end;


 bool __negative = false;
 if (!__testeof)
   {
     __c = *__beg;
     __negative = __c == __lit[__num_base::_S_iminus];
     if ((__negative || __c == __lit[__num_base::_S_iplus])
  && !(__lc->_M_use_grouping && __c == __lc->_M_thousands_sep)
  && !(__c == __lc->_M_decimal_point))
       {
  if (++__beg != __end)
    __c = *__beg;
  else
    __testeof = true;
       }
   }



 bool __found_zero = false;
 int __sep_pos = 0;
 while (!__testeof)
   {
     if ((__lc->_M_use_grouping && __c == __lc->_M_thousands_sep)
  || __c == __lc->_M_decimal_point)
       break;
     else if (__c == __lit[__num_base::_S_izero]
       && (!__found_zero || __base == 10))
       {
  __found_zero = true;
  ++__sep_pos;
  if (__basefield == 0)
    __base = 8;
  if (__base == 8)
    __sep_pos = 0;
       }
     else if (__found_zero
       && (__c == __lit[__num_base::_S_ix]
    || __c == __lit[__num_base::_S_iX]))
       {
  if (__basefield == 0)
    __base = 16;
  if (__base == 16)
    {
      __found_zero = false;
      __sep_pos = 0;
    }
  else
    break;
       }
     else
       break;

     if (++__beg != __end)
       {
  __c = *__beg;
  if (!__found_zero)
    break;
       }
     else
       __testeof = true;
   }



 const size_t __len = (__base == 16 ? __num_base::_S_iend
         - __num_base::_S_izero : __base);


 typedef __gnu_cxx::__numeric_traits<_ValueT> __num_traits;
 string __found_grouping;
 if (__lc->_M_use_grouping)
   __found_grouping.reserve(32);
 bool __testfail = false;
 bool __testoverflow = false;
 const __unsigned_type __max =
   (__negative && __num_traits::__is_signed)
   ? -static_cast<__unsigned_type>(__num_traits::__min)
   : __num_traits::__max;
 const __unsigned_type __smax = __max / __base;
 __unsigned_type __result = 0;
 int __digit = 0;
 const char_type* __lit_zero = __lit + __num_base::_S_izero;

 if (!__lc->_M_allocated)

   while (!__testeof)
     {
       __digit = _M_find(__lit_zero, __len, __c);
       if (__digit == -1)
  break;

       if (__result > __smax)
  __testoverflow = true;
       else
  {
    __result *= __base;
    __testoverflow |= __result > __max - __digit;
    __result += __digit;
    ++__sep_pos;
  }

       if (++__beg != __end)
  __c = *__beg;
       else
  __testeof = true;
     }
 else
   while (!__testeof)
     {


       if (__lc->_M_use_grouping && __c == __lc->_M_thousands_sep)
  {


    if (__sep_pos)
      {
        __found_grouping += static_cast<char>(__sep_pos);
        __sep_pos = 0;
      }
    else
      {
        __testfail = true;
        break;
      }
  }
       else if (__c == __lc->_M_decimal_point)
  break;
       else
  {
    const char_type* __q =
      __traits_type::find(__lit_zero, __len, __c);
    if (!__q)
      break;

    __digit = __q - __lit_zero;
    if (__digit > 15)
      __digit -= 6;
    if (__result > __smax)
      __testoverflow = true;
    else
      {
        __result *= __base;
        __testoverflow |= __result > __max - __digit;
        __result += __digit;
        ++__sep_pos;
      }
  }

       if (++__beg != __end)
  __c = *__beg;
       else
  __testeof = true;
     }



 if (__found_grouping.size())
   {

     __found_grouping += static_cast<char>(__sep_pos);

     if (!std::__verify_grouping(__lc->_M_grouping,
     __lc->_M_grouping_size,
     __found_grouping))
       __err = ios_base::failbit;
   }



 if ((!__sep_pos && !__found_zero && !__found_grouping.size())
     || __testfail)
   {
     __v = 0;
     __err = ios_base::failbit;
   }
 else if (__testoverflow)
   {
     if (__negative && __num_traits::__is_signed)
       __v = __num_traits::__min;
     else
       __v = __num_traits::__max;
     __err = ios_base::failbit;
   }
 else
   __v = __negative ? -__result : __result;

 if (__testeof)
   __err |= ios_base::eofbit;
 return __beg;
      }



  template<typename _CharT, typename _InIter>
    _InIter
    num_get<_CharT, _InIter>::
    do_get(iter_type __beg, iter_type __end, ios_base& __io,
           ios_base::iostate& __err, bool& __v) const
    {
      if (!(__io.flags() & ios_base::boolalpha))
        {



   long __l = -1;
          __beg = _M_extract_int(__beg, __end, __io, __err, __l);
   if (__l == 0 || __l == 1)
     __v = bool(__l);
   else
     {


       __v = true;
       __err = ios_base::failbit;
       if (__beg == __end)
  __err |= ios_base::eofbit;
     }
        }
      else
        {

   typedef __numpunct_cache<_CharT> __cache_type;
   __use_cache<__cache_type> __uc;
   const locale& __loc = __io._M_getloc();
   const __cache_type* __lc = __uc(__loc);

   bool __testf = true;
   bool __testt = true;
   bool __donef = __lc->_M_falsename_size == 0;
   bool __donet = __lc->_M_truename_size == 0;
   bool __testeof = false;
   size_t __n = 0;
   while (!__donef || !__donet)
     {
       if (__beg == __end)
  {
    __testeof = true;
    break;
  }

       const char_type __c = *__beg;

       if (!__donef)
  __testf = __c == __lc->_M_falsename[__n];

       if (!__testf && __donet)
  break;

       if (!__donet)
  __testt = __c == __lc->_M_truename[__n];

       if (!__testt && __donef)
  break;

       if (!__testt && !__testf)
  break;

       ++__n;
       ++__beg;

       __donef = !__testf || __n >= __lc->_M_falsename_size;
       __donet = !__testt || __n >= __lc->_M_truename_size;
     }
   if (__testf && __n == __lc->_M_falsename_size && __n)
     {
       __v = false;
       if (__testt && __n == __lc->_M_truename_size)
  __err = ios_base::failbit;
       else
  __err = __testeof ? ios_base::eofbit : ios_base::goodbit;
     }
   else if (__testt && __n == __lc->_M_truename_size && __n)
     {
       __v = true;
       __err = __testeof ? ios_base::eofbit : ios_base::goodbit;
     }
   else
     {


       __v = false;
       __err = ios_base::failbit;
       if (__testeof)
  __err |= ios_base::eofbit;
     }
 }
      return __beg;
    }

  template<typename _CharT, typename _InIter>
    _InIter
    num_get<_CharT, _InIter>::
    do_get(iter_type __beg, iter_type __end, ios_base& __io,
    ios_base::iostate& __err, float& __v) const
    {
      string __xtrc;
      __xtrc.reserve(32);
      __beg = _M_extract_float(__beg, __end, __io, __err, __xtrc);
      std::__convert_to_v(__xtrc.c_str(), __v, __err, _S_get_c_locale());
      if (__beg == __end)
 __err |= ios_base::eofbit;
      return __beg;
    }

  template<typename _CharT, typename _InIter>
    _InIter
    num_get<_CharT, _InIter>::
    do_get(iter_type __beg, iter_type __end, ios_base& __io,
           ios_base::iostate& __err, double& __v) const
    {
      string __xtrc;
      __xtrc.reserve(32);
      __beg = _M_extract_float(__beg, __end, __io, __err, __xtrc);
      std::__convert_to_v(__xtrc.c_str(), __v, __err, _S_get_c_locale());
      if (__beg == __end)
 __err |= ios_base::eofbit;
      return __beg;
    }
# 735 "/usr/include/c++/11/bits/locale_facets.tcc" 3
  template<typename _CharT, typename _InIter>
    _InIter
    num_get<_CharT, _InIter>::
    do_get(iter_type __beg, iter_type __end, ios_base& __io,
           ios_base::iostate& __err, long double& __v) const
    {
      string __xtrc;
      __xtrc.reserve(32);
      __beg = _M_extract_float(__beg, __end, __io, __err, __xtrc);
      std::__convert_to_v(__xtrc.c_str(), __v, __err, _S_get_c_locale());
      if (__beg == __end)
 __err |= ios_base::eofbit;
      return __beg;
    }

  template<typename _CharT, typename _InIter>
    _InIter
    num_get<_CharT, _InIter>::
    do_get(iter_type __beg, iter_type __end, ios_base& __io,
           ios_base::iostate& __err, void*& __v) const
    {

      typedef ios_base::fmtflags fmtflags;
      const fmtflags __fmt = __io.flags();
      __io.flags((__fmt & ~ios_base::basefield) | ios_base::hex);

      typedef __gnu_cxx::__conditional_type<(sizeof(void*)
          <= sizeof(unsigned long)),
 unsigned long, unsigned long long>::__type _UIntPtrType;

      _UIntPtrType __ul;
      __beg = _M_extract_int(__beg, __end, __io, __err, __ul);


      __io.flags(__fmt);

      __v = reinterpret_cast<void*>(__ul);
      return __beg;
    }
# 795 "/usr/include/c++/11/bits/locale_facets.tcc" 3
  template<typename _CharT, typename _OutIter>
    void
    num_put<_CharT, _OutIter>::
    _M_pad(_CharT __fill, streamsize __w, ios_base& __io,
    _CharT* __new, const _CharT* __cs, int& __len) const
    {


      __pad<_CharT, char_traits<_CharT> >::_S_pad(__io, __fill, __new,
        __cs, __w, __len);
      __len = static_cast<int>(__w);
    }



  template<typename _CharT, typename _ValueT>
    int
    __int_to_char(_CharT* __bufend, _ValueT __v, const _CharT* __lit,
    ios_base::fmtflags __flags, bool __dec)
    {
      _CharT* __buf = __bufend;
      if (__builtin_expect(__dec, true))
 {

   do
     {
       *--__buf = __lit[(__v % 10) + __num_base::_S_odigits];
       __v /= 10;
     }
   while (__v != 0);
 }
      else if ((__flags & ios_base::basefield) == ios_base::oct)
 {

   do
     {
       *--__buf = __lit[(__v & 0x7) + __num_base::_S_odigits];
       __v >>= 3;
     }
   while (__v != 0);
 }
      else
 {

   const bool __uppercase = __flags & ios_base::uppercase;
   const int __case_offset = __uppercase ? __num_base::_S_oudigits
                                         : __num_base::_S_odigits;
   do
     {
       *--__buf = __lit[(__v & 0xf) + __case_offset];
       __v >>= 4;
     }
   while (__v != 0);
 }
      return __bufend - __buf;
    }



  template<typename _CharT, typename _OutIter>
    void
    num_put<_CharT, _OutIter>::
    _M_group_int(const char* __grouping, size_t __grouping_size, _CharT __sep,
   ios_base&, _CharT* __new, _CharT* __cs, int& __len) const
    {
      _CharT* __p = std::__add_grouping(__new, __sep, __grouping,
     __grouping_size, __cs, __cs + __len);
      __len = __p - __new;
    }

  template<typename _CharT, typename _OutIter>
    template<typename _ValueT>
      _OutIter
      num_put<_CharT, _OutIter>::
      _M_insert_int(_OutIter __s, ios_base& __io, _CharT __fill,
      _ValueT __v) const
      {
 using __gnu_cxx::__add_unsigned;
 typedef typename __add_unsigned<_ValueT>::__type __unsigned_type;
 typedef __numpunct_cache<_CharT> __cache_type;
 __use_cache<__cache_type> __uc;
 const locale& __loc = __io._M_getloc();
 const __cache_type* __lc = __uc(__loc);
 const _CharT* __lit = __lc->_M_atoms_out;
 const ios_base::fmtflags __flags = __io.flags();


 const int __ilen = 5 * sizeof(_ValueT);
 _CharT* __cs = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
            * __ilen));



 const ios_base::fmtflags __basefield = __flags & ios_base::basefield;
 const bool __dec = (__basefield != ios_base::oct
       && __basefield != ios_base::hex);
 const __unsigned_type __u = ((__v > 0 || !__dec)
         ? __unsigned_type(__v)
         : -__unsigned_type(__v));
  int __len = __int_to_char(__cs + __ilen, __u, __lit, __flags, __dec);
 __cs += __ilen - __len;


 if (__lc->_M_use_grouping)
   {


     _CharT* __cs2 = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
          * (__len + 1)
          * 2));
     _M_group_int(__lc->_M_grouping, __lc->_M_grouping_size,
    __lc->_M_thousands_sep, __io, __cs2 + 2, __cs, __len);
     __cs = __cs2 + 2;
   }


 if (__builtin_expect(__dec, true))
   {

     if (__v >= 0)
       {
  if (bool(__flags & ios_base::showpos)
      && __gnu_cxx::__numeric_traits<_ValueT>::__is_signed)
    *--__cs = __lit[__num_base::_S_oplus], ++__len;
       }
     else
       *--__cs = __lit[__num_base::_S_ominus], ++__len;
   }
 else if (bool(__flags & ios_base::showbase) && __v)
   {
     if (__basefield == ios_base::oct)
       *--__cs = __lit[__num_base::_S_odigits], ++__len;
     else
       {

  const bool __uppercase = __flags & ios_base::uppercase;
  *--__cs = __lit[__num_base::_S_ox + __uppercase];

  *--__cs = __lit[__num_base::_S_odigits];
  __len += 2;
       }
   }


 const streamsize __w = __io.width();
 if (__w > static_cast<streamsize>(__len))
   {
     _CharT* __cs3 = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
          * __w));
     _M_pad(__fill, __w, __io, __cs3, __cs, __len);
     __cs = __cs3;
   }
 __io.width(0);



 return std::__write(__s, __cs, __len);
      }

  template<typename _CharT, typename _OutIter>
    void
    num_put<_CharT, _OutIter>::
    _M_group_float(const char* __grouping, size_t __grouping_size,
     _CharT __sep, const _CharT* __p, _CharT* __new,
     _CharT* __cs, int& __len) const
    {



      const int __declen = __p ? __p - __cs : __len;
      _CharT* __p2 = std::__add_grouping(__new, __sep, __grouping,
      __grouping_size,
      __cs, __cs + __declen);


      int __newlen = __p2 - __new;
      if (__p)
 {
   char_traits<_CharT>::copy(__p2, __p, __len - __declen);
   __newlen += __len - __declen;
 }
      __len = __newlen;
    }
# 989 "/usr/include/c++/11/bits/locale_facets.tcc" 3
  template<typename _CharT, typename _OutIter>
    template<typename _ValueT>
      _OutIter
      num_put<_CharT, _OutIter>::
      _M_insert_float(_OutIter __s, ios_base& __io, _CharT __fill, char __mod,
         _ValueT __v) const
      {
 typedef __numpunct_cache<_CharT> __cache_type;
 __use_cache<__cache_type> __uc;
 const locale& __loc = __io._M_getloc();
 const __cache_type* __lc = __uc(__loc);


 const streamsize __prec = __io.precision() < 0 ? 6 : __io.precision();

 const int __max_digits =
   __gnu_cxx::__numeric_traits<_ValueT>::__digits10;


 int __len;

 char __fbuf[16];
 __num_base::_S_format_float(__io, __fbuf, __mod);



 const bool __use_prec =
   (__io.flags() & ios_base::floatfield) != ios_base::floatfield;



 int __cs_size = __max_digits * 3;
 char* __cs = static_cast<char*>(__builtin_alloca(__cs_size));
 if (__use_prec)
   __len = std::__convert_from_v(_S_get_c_locale(), __cs, __cs_size,
     __fbuf, __prec, __v);
 else
   __len = std::__convert_from_v(_S_get_c_locale(), __cs, __cs_size,
     __fbuf, __v);


 if (__len >= __cs_size)
   {
     __cs_size = __len + 1;
     __cs = static_cast<char*>(__builtin_alloca(__cs_size));
     if (__use_prec)
       __len = std::__convert_from_v(_S_get_c_locale(), __cs, __cs_size,
         __fbuf, __prec, __v);
     else
       __len = std::__convert_from_v(_S_get_c_locale(), __cs, __cs_size,
         __fbuf, __v);
   }
# 1062 "/usr/include/c++/11/bits/locale_facets.tcc" 3
 const ctype<_CharT>& __ctype = use_facet<ctype<_CharT> >(__loc);

 _CharT* __ws = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
            * __len));
 __ctype.widen(__cs, __cs + __len, __ws);


 _CharT* __wp = 0;
 const char* __p = char_traits<char>::find(__cs, __len, '.');
 if (__p)
   {
     __wp = __ws + (__p - __cs);
     *__wp = __lc->_M_decimal_point;
   }




 if (__lc->_M_use_grouping
     && (__wp || __len < 3 || (__cs[1] <= '9' && __cs[2] <= '9'
          && __cs[1] >= '0' && __cs[2] >= '0')))
   {


     _CharT* __ws2 = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
          * __len * 2));

     streamsize __off = 0;
     if (__cs[0] == '-' || __cs[0] == '+')
       {
  __off = 1;
  __ws2[0] = __ws[0];
  __len -= 1;
       }

     _M_group_float(__lc->_M_grouping, __lc->_M_grouping_size,
      __lc->_M_thousands_sep, __wp, __ws2 + __off,
      __ws + __off, __len);
     __len += __off;

     __ws = __ws2;
   }


 const streamsize __w = __io.width();
 if (__w > static_cast<streamsize>(__len))
   {
     _CharT* __ws3 = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
          * __w));
     _M_pad(__fill, __w, __io, __ws3, __ws, __len);
     __ws = __ws3;
   }
 __io.width(0);



 return std::__write(__s, __ws, __len);
      }

  template<typename _CharT, typename _OutIter>
    _OutIter
    num_put<_CharT, _OutIter>::
    do_put(iter_type __s, ios_base& __io, char_type __fill, bool __v) const
    {
      const ios_base::fmtflags __flags = __io.flags();
      if ((__flags & ios_base::boolalpha) == 0)
        {
          const long __l = __v;
          __s = _M_insert_int(__s, __io, __fill, __l);
        }
      else
        {
   typedef __numpunct_cache<_CharT> __cache_type;
   __use_cache<__cache_type> __uc;
   const locale& __loc = __io._M_getloc();
   const __cache_type* __lc = __uc(__loc);

   const _CharT* __name = __v ? __lc->_M_truename
                              : __lc->_M_falsename;
   int __len = __v ? __lc->_M_truename_size
                   : __lc->_M_falsename_size;

   const streamsize __w = __io.width();
   if (__w > static_cast<streamsize>(__len))
     {
       const streamsize __plen = __w - __len;
       _CharT* __ps
  = static_cast<_CharT*>(__builtin_alloca(sizeof(_CharT)
       * __plen));

       char_traits<_CharT>::assign(__ps, __plen, __fill);
       __io.width(0);

       if ((__flags & ios_base::adjustfield) == ios_base::left)
  {
    __s = std::__write(__s, __name, __len);
    __s = std::__write(__s, __ps, __plen);
  }
       else
  {
    __s = std::__write(__s, __ps, __plen);
    __s = std::__write(__s, __name, __len);
  }
       return __s;
     }
   __io.width(0);
   __s = std::__write(__s, __name, __len);
 }
      return __s;
    }

  template<typename _CharT, typename _OutIter>
    _OutIter
    num_put<_CharT, _OutIter>::
    do_put(iter_type __s, ios_base& __io, char_type __fill, double __v) const
    { return _M_insert_float(__s, __io, __fill, char(), __v); }
# 1187 "/usr/include/c++/11/bits/locale_facets.tcc" 3
  template<typename _CharT, typename _OutIter>
    _OutIter
    num_put<_CharT, _OutIter>::
    do_put(iter_type __s, ios_base& __io, char_type __fill,
    long double __v) const
    { return _M_insert_float(__s, __io, __fill, 'L', __v); }

  template<typename _CharT, typename _OutIter>
    _OutIter
    num_put<_CharT, _OutIter>::
    do_put(iter_type __s, ios_base& __io, char_type __fill,
           const void* __v) const
    {
      const ios_base::fmtflags __flags = __io.flags();
      const ios_base::fmtflags __fmt = ~(ios_base::basefield
      | ios_base::uppercase);
      __io.flags((__flags & __fmt) | (ios_base::hex | ios_base::showbase));

      typedef __gnu_cxx::__conditional_type<(sizeof(const void*)
          <= sizeof(unsigned long)),
 unsigned long, unsigned long long>::__type _UIntPtrType;

      __s = _M_insert_int(__s, __io, __fill,
     reinterpret_cast<_UIntPtrType>(__v));
      __io.flags(__flags);
      return __s;
    }
# 1224 "/usr/include/c++/11/bits/locale_facets.tcc" 3

# 1233 "/usr/include/c++/11/bits/locale_facets.tcc" 3
  template<typename _CharT, typename _Traits>
    void
    __pad<_CharT, _Traits>::_S_pad(ios_base& __io, _CharT __fill,
       _CharT* __news, const _CharT* __olds,
       streamsize __newlen, streamsize __oldlen)
    {
      const size_t __plen = static_cast<size_t>(__newlen - __oldlen);
      const ios_base::fmtflags __adjust = __io.flags() & ios_base::adjustfield;


      if (__adjust == ios_base::left)
 {
   _Traits::copy(__news, __olds, __oldlen);
   _Traits::assign(__news + __oldlen, __plen, __fill);
   return;
 }

      size_t __mod = 0;
      if (__adjust == ios_base::internal)
 {



          const locale& __loc = __io._M_getloc();
   const ctype<_CharT>& __ctype = use_facet<ctype<_CharT> >(__loc);

   if (__ctype.widen('-') == __olds[0]
       || __ctype.widen('+') == __olds[0])
     {
       __news[0] = __olds[0];
       __mod = 1;
       ++__news;
     }
   else if (__ctype.widen('0') == __olds[0]
     && __oldlen > 1
     && (__ctype.widen('x') == __olds[1]
         || __ctype.widen('X') == __olds[1]))
     {
       __news[0] = __olds[0];
       __news[1] = __olds[1];
       __mod = 2;
       __news += 2;
     }

 }
      _Traits::assign(__news, __plen, __fill);
      _Traits::copy(__news + __plen, __olds + __mod, __oldlen - __mod);
    }

  template<typename _CharT>
    _CharT*
    __add_grouping(_CharT* __s, _CharT __sep,
     const char* __gbeg, size_t __gsize,
     const _CharT* __first, const _CharT* __last)
    {
      size_t __idx = 0;
      size_t __ctr = 0;

      while (__last - __first > __gbeg[__idx]
      && static_cast<signed char>(__gbeg[__idx]) > 0
      && __gbeg[__idx] != __gnu_cxx::__numeric_traits<char>::__max)
 {
   __last -= __gbeg[__idx];
   __idx < __gsize - 1 ? ++__idx : ++__ctr;
 }

      while (__first != __last)
 *__s++ = *__first++;

      while (__ctr--)
 {
   *__s++ = __sep;
   for (char __i = __gbeg[__idx]; __i > 0; --__i)
     *__s++ = *__first++;
 }

      while (__idx--)
 {
   *__s++ = __sep;
   for (char __i = __gbeg[__idx]; __i > 0; --__i)
     *__s++ = *__first++;
 }

      return __s;
    }




  extern template class __cxx11:: numpunct<char>;
  extern template class __cxx11:: numpunct_byname<char>;
  extern template class num_get<char>;
  extern template class num_put<char>;
  extern template class ctype_byname<char>;

  extern template
    const ctype<char>&
    use_facet<ctype<char> >(const locale&);

  extern template
    const numpunct<char>&
    use_facet<numpunct<char> >(const locale&);

  extern template
    const num_put<char>&
    use_facet<num_put<char> >(const locale&);

  extern template
    const num_get<char>&
    use_facet<num_get<char> >(const locale&);

  extern template
    bool
    has_facet<ctype<char> >(const locale&);

  extern template
    bool
    has_facet<numpunct<char> >(const locale&);

  extern template
    bool
    has_facet<num_put<char> >(const locale&);

  extern template
    bool
    has_facet<num_get<char> >(const locale&);


  extern template class __cxx11:: numpunct<wchar_t>;
  extern template class __cxx11:: numpunct_byname<wchar_t>;
  extern template class num_get<wchar_t>;
  extern template class num_put<wchar_t>;
  extern template class ctype_byname<wchar_t>;

  extern template
    const ctype<wchar_t>&
    use_facet<ctype<wchar_t> >(const locale&);

  extern template
    const numpunct<wchar_t>&
    use_facet<numpunct<wchar_t> >(const locale&);

  extern template
    const num_put<wchar_t>&
    use_facet<num_put<wchar_t> >(const locale&);

  extern template
    const num_get<wchar_t>&
    use_facet<num_get<wchar_t> >(const locale&);

 extern template
    bool
    has_facet<ctype<wchar_t> >(const locale&);

  extern template
    bool
    has_facet<numpunct<wchar_t> >(const locale&);

  extern template
    bool
    has_facet<num_put<wchar_t> >(const locale&);

  extern template
    bool
    has_facet<num_get<wchar_t> >(const locale&);




}
# 2683 "/usr/include/c++/11/bits/locale_facets.h" 2 3
# 38 "/usr/include/c++/11/bits/basic_ios.h" 2 3



namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _Facet>
    inline const _Facet&
    __check_facet(const _Facet* __f)
    {
      if (!__f)
 __throw_bad_cast();
      return *__f;
    }
# 66 "/usr/include/c++/11/bits/basic_ios.h" 3
  template<typename _CharT, typename _Traits>
    class basic_ios : public ios_base
    {
    public:






      typedef _CharT char_type;
      typedef typename _Traits::int_type int_type;
      typedef typename _Traits::pos_type pos_type;
      typedef typename _Traits::off_type off_type;
      typedef _Traits traits_type;






      typedef ctype<_CharT> __ctype_type;
      typedef num_put<_CharT, ostreambuf_iterator<_CharT, _Traits> >
           __num_put_type;
      typedef num_get<_CharT, istreambuf_iterator<_CharT, _Traits> >
           __num_get_type;



    protected:
      basic_ostream<_CharT, _Traits>* _M_tie;
      mutable char_type _M_fill;
      mutable bool _M_fill_init;
      basic_streambuf<_CharT, _Traits>* _M_streambuf;


      const __ctype_type* _M_ctype;

      const __num_put_type* _M_num_put;

      const __num_get_type* _M_num_get;

    public:
# 117 "/usr/include/c++/11/bits/basic_ios.h" 3
      explicit operator bool() const
      { return !this->fail(); }





      bool
      operator!() const
      { return this->fail(); }
# 136 "/usr/include/c++/11/bits/basic_ios.h" 3
      iostate
      rdstate() const
      { return _M_streambuf_state; }
# 147 "/usr/include/c++/11/bits/basic_ios.h" 3
      void
      clear(iostate __state = goodbit);







      void
      setstate(iostate __state)
      { this->clear(this->rdstate() | __state); }




      void
      _M_setstate(iostate __state)
      {


 _M_streambuf_state |= __state;
 if (this->exceptions() & __state)
   throw;
      }







      bool
      good() const
      { return this->rdstate() == 0; }







      bool
      eof() const
      { return (this->rdstate() & eofbit) != 0; }
# 200 "/usr/include/c++/11/bits/basic_ios.h" 3
      bool
      fail() const
      { return (this->rdstate() & (badbit | failbit)) != 0; }







      bool
      bad() const
      { return (this->rdstate() & badbit) != 0; }
# 221 "/usr/include/c++/11/bits/basic_ios.h" 3
      iostate
      exceptions() const
      { return _M_exception; }
# 256 "/usr/include/c++/11/bits/basic_ios.h" 3
      void
      exceptions(iostate __except)
      {
        _M_exception = __except;
        this->clear(_M_streambuf_state);
      }







      explicit
      basic_ios(basic_streambuf<_CharT, _Traits>* __sb)
      : ios_base(), _M_tie(0), _M_fill(), _M_fill_init(false), _M_streambuf(0),
 _M_ctype(0), _M_num_put(0), _M_num_get(0)
      { this->init(__sb); }







      virtual
      ~basic_ios() { }
# 294 "/usr/include/c++/11/bits/basic_ios.h" 3
      basic_ostream<_CharT, _Traits>*
      tie() const
      { return _M_tie; }
# 306 "/usr/include/c++/11/bits/basic_ios.h" 3
      basic_ostream<_CharT, _Traits>*
      tie(basic_ostream<_CharT, _Traits>* __tiestr)
      {
        basic_ostream<_CharT, _Traits>* __old = _M_tie;
        _M_tie = __tiestr;
        return __old;
      }







      basic_streambuf<_CharT, _Traits>*
      rdbuf() const
      { return _M_streambuf; }
# 346 "/usr/include/c++/11/bits/basic_ios.h" 3
      basic_streambuf<_CharT, _Traits>*
      rdbuf(basic_streambuf<_CharT, _Traits>* __sb);
# 360 "/usr/include/c++/11/bits/basic_ios.h" 3
      basic_ios&
      copyfmt(const basic_ios& __rhs);







      char_type
      fill() const
      {
 if (!_M_fill_init)
   {
     _M_fill = this->widen(' ');
     _M_fill_init = true;
   }
 return _M_fill;
      }
# 389 "/usr/include/c++/11/bits/basic_ios.h" 3
      char_type
      fill(char_type __ch)
      {
 char_type __old = this->fill();
 _M_fill = __ch;
 return __old;
      }
# 409 "/usr/include/c++/11/bits/basic_ios.h" 3
      locale
      imbue(const locale& __loc);
# 429 "/usr/include/c++/11/bits/basic_ios.h" 3
      char
      narrow(char_type __c, char __dfault) const
      { return __check_facet(_M_ctype).narrow(__c, __dfault); }
# 448 "/usr/include/c++/11/bits/basic_ios.h" 3
      char_type
      widen(char __c) const
      { return __check_facet(_M_ctype).widen(__c); }

    protected:







      basic_ios()
      : ios_base(), _M_tie(0), _M_fill(char_type()), _M_fill_init(false),
 _M_streambuf(0), _M_ctype(0), _M_num_put(0), _M_num_get(0)
      { }







      void
      init(basic_streambuf<_CharT, _Traits>* __sb);


      basic_ios(const basic_ios&) = delete;
      basic_ios& operator=(const basic_ios&) = delete;

      void
      move(basic_ios& __rhs)
      {
 ios_base::_M_move(__rhs);
 _M_cache_locale(_M_ios_locale);
 this->tie(__rhs.tie(nullptr));
 _M_fill = __rhs._M_fill;
 _M_fill_init = __rhs._M_fill_init;
 _M_streambuf = nullptr;
      }

      void
      move(basic_ios&& __rhs)
      { this->move(__rhs); }

      void
      swap(basic_ios& __rhs) noexcept
      {
 ios_base::_M_swap(__rhs);
 _M_cache_locale(_M_ios_locale);
 __rhs._M_cache_locale(__rhs._M_ios_locale);
 std::swap(_M_tie, __rhs._M_tie);
 std::swap(_M_fill, __rhs._M_fill);
 std::swap(_M_fill_init, __rhs._M_fill_init);
      }

      void
      set_rdbuf(basic_streambuf<_CharT, _Traits>* __sb)
      { _M_streambuf = __sb; }


      void
      _M_cache_locale(const locale& __loc);
    };


}

# 1 "/usr/include/c++/11/bits/basic_ios.tcc" 1 3
# 33 "/usr/include/c++/11/bits/basic_ios.tcc" 3
       
# 34 "/usr/include/c++/11/bits/basic_ios.tcc" 3

namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _CharT, typename _Traits>
    void
    basic_ios<_CharT, _Traits>::clear(iostate __state)
    {
      if (this->rdbuf())
 _M_streambuf_state = __state;
      else
   _M_streambuf_state = __state | badbit;
      if (this->exceptions() & this->rdstate())
 __throw_ios_failure(("basic_ios::clear"));
    }

  template<typename _CharT, typename _Traits>
    basic_streambuf<_CharT, _Traits>*
    basic_ios<_CharT, _Traits>::rdbuf(basic_streambuf<_CharT, _Traits>* __sb)
    {
      basic_streambuf<_CharT, _Traits>* __old = _M_streambuf;
      _M_streambuf = __sb;
      this->clear();
      return __old;
    }

  template<typename _CharT, typename _Traits>
    basic_ios<_CharT, _Traits>&
    basic_ios<_CharT, _Traits>::copyfmt(const basic_ios& __rhs)
    {


      if (this != &__rhs)
 {




   _Words* __words = (__rhs._M_word_size <= _S_local_word_size) ?
                      _M_local_word : new _Words[__rhs._M_word_size];


   _Callback_list* __cb = __rhs._M_callbacks;
   if (__cb)
     __cb->_M_add_reference();
   _M_call_callbacks(erase_event);
   if (_M_word != _M_local_word)
     {
       delete [] _M_word;
       _M_word = 0;
     }
   _M_dispose_callbacks();


   _M_callbacks = __cb;
   for (int __i = 0; __i < __rhs._M_word_size; ++__i)
     __words[__i] = __rhs._M_word[__i];
   _M_word = __words;
   _M_word_size = __rhs._M_word_size;

   this->flags(__rhs.flags());
   this->width(__rhs.width());
   this->precision(__rhs.precision());
   this->tie(__rhs.tie());
   this->fill(__rhs.fill());
   _M_ios_locale = __rhs.getloc();
   _M_cache_locale(_M_ios_locale);

   _M_call_callbacks(copyfmt_event);


   this->exceptions(__rhs.exceptions());
 }
      return *this;
    }


  template<typename _CharT, typename _Traits>
    locale
    basic_ios<_CharT, _Traits>::imbue(const locale& __loc)
    {
      locale __old(this->getloc());
      ios_base::imbue(__loc);
      _M_cache_locale(__loc);
      if (this->rdbuf() != 0)
 this->rdbuf()->pubimbue(__loc);
      return __old;
    }

  template<typename _CharT, typename _Traits>
    void
    basic_ios<_CharT, _Traits>::init(basic_streambuf<_CharT, _Traits>* __sb)
    {

      ios_base::_M_init();


      _M_cache_locale(_M_ios_locale);
# 146 "/usr/include/c++/11/bits/basic_ios.tcc" 3
      _M_fill = _CharT();
      _M_fill_init = false;

      _M_tie = 0;
      _M_exception = goodbit;
      _M_streambuf = __sb;
      _M_streambuf_state = __sb ? goodbit : badbit;
    }

  template<typename _CharT, typename _Traits>
    void
    basic_ios<_CharT, _Traits>::_M_cache_locale(const locale& __loc)
    {
      if (__builtin_expect(has_facet<__ctype_type>(__loc), true))
 _M_ctype = std::__addressof(use_facet<__ctype_type>(__loc));
      else
 _M_ctype = 0;

      if (__builtin_expect(has_facet<__num_put_type>(__loc), true))
 _M_num_put = std::__addressof(use_facet<__num_put_type>(__loc));
      else
 _M_num_put = 0;

      if (__builtin_expect(has_facet<__num_get_type>(__loc), true))
 _M_num_get = std::__addressof(use_facet<__num_get_type>(__loc));
      else
 _M_num_get = 0;
    }




  extern template class basic_ios<char>;


  extern template class basic_ios<wchar_t>;




}
# 517 "/usr/include/c++/11/bits/basic_ios.h" 2 3
# 45 "/usr/include/c++/11/ios" 2 3
# 39 "/usr/include/c++/11/ostream" 2 3


namespace std __attribute__ ((__visibility__ ("default")))
{

# 57 "/usr/include/c++/11/ostream" 3
  template<typename _CharT, typename _Traits>
    class basic_ostream : virtual public basic_ios<_CharT, _Traits>
    {
    public:

      typedef _CharT char_type;
      typedef typename _Traits::int_type int_type;
      typedef typename _Traits::pos_type pos_type;
      typedef typename _Traits::off_type off_type;
      typedef _Traits traits_type;


      typedef basic_streambuf<_CharT, _Traits> __streambuf_type;
      typedef basic_ios<_CharT, _Traits> __ios_type;
      typedef basic_ostream<_CharT, _Traits> __ostream_type;
      typedef num_put<_CharT, ostreambuf_iterator<_CharT, _Traits> >
             __num_put_type;
      typedef ctype<_CharT> __ctype_type;
# 83 "/usr/include/c++/11/ostream" 3
      explicit
      basic_ostream(__streambuf_type* __sb)
      { this->init(__sb); }






      virtual
      ~basic_ostream() { }


      class sentry;
      friend class sentry;
# 107 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      operator<<(__ostream_type& (*__pf)(__ostream_type&))
      {



 return __pf(*this);
      }

      __ostream_type&
      operator<<(__ios_type& (*__pf)(__ios_type&))
      {



 __pf(*this);
 return *this;
      }

      __ostream_type&
      operator<<(ios_base& (*__pf) (ios_base&))
      {



 __pf(*this);
 return *this;
      }
# 165 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      operator<<(long __n)
      { return _M_insert(__n); }

      __ostream_type&
      operator<<(unsigned long __n)
      { return _M_insert(__n); }

      __ostream_type&
      operator<<(bool __n)
      { return _M_insert(__n); }

      __ostream_type&
      operator<<(short __n);

      __ostream_type&
      operator<<(unsigned short __n)
      {


 return _M_insert(static_cast<unsigned long>(__n));
      }

      __ostream_type&
      operator<<(int __n);

      __ostream_type&
      operator<<(unsigned int __n)
      {


 return _M_insert(static_cast<unsigned long>(__n));
      }


      __ostream_type&
      operator<<(long long __n)
      { return _M_insert(__n); }

      __ostream_type&
      operator<<(unsigned long long __n)
      { return _M_insert(__n); }
# 219 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      operator<<(double __f)
      { return _M_insert(__f); }

      __ostream_type&
      operator<<(float __f)
      {


 return _M_insert(static_cast<double>(__f));
      }

      __ostream_type&
      operator<<(long double __f)
      { return _M_insert(__f); }
# 244 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      operator<<(const void* __p)
      { return _M_insert(__p); }


      __ostream_type&
      operator<<(nullptr_t)
      { return *this << "nullptr"; }
# 282 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      operator<<(__streambuf_type* __sb);
# 315 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      put(char_type __c);






      void
      _M_write(const char_type* __s, streamsize __n)
      {
 const streamsize __put = this->rdbuf()->sputn(__s, __n);
 if (__put != __n)
   this->setstate(ios_base::badbit);
      }
# 347 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      write(const char_type* __s, streamsize __n);
# 360 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      flush();
# 370 "/usr/include/c++/11/ostream" 3
      pos_type
      tellp();
# 381 "/usr/include/c++/11/ostream" 3
      __ostream_type&
      seekp(pos_type);
# 393 "/usr/include/c++/11/ostream" 3
       __ostream_type&
      seekp(off_type, ios_base::seekdir);

    protected:
      basic_ostream()
      { this->init(0); }



      basic_ostream(basic_iostream<_CharT, _Traits>&) { }

      basic_ostream(const basic_ostream&) = delete;

      basic_ostream(basic_ostream&& __rhs)
      : __ios_type()
      { __ios_type::move(__rhs); }



      basic_ostream& operator=(const basic_ostream&) = delete;

      basic_ostream&
      operator=(basic_ostream&& __rhs)
      {
 swap(__rhs);
 return *this;
      }

      void
      swap(basic_ostream& __rhs)
      { __ios_type::swap(__rhs); }


      template<typename _ValueT>
 __ostream_type&
 _M_insert(_ValueT __v);
    };
# 438 "/usr/include/c++/11/ostream" 3
  template <typename _CharT, typename _Traits>
    class basic_ostream<_CharT, _Traits>::sentry
    {

      bool _M_ok;
      basic_ostream<_CharT, _Traits>& _M_os;

    public:
# 457 "/usr/include/c++/11/ostream" 3
      explicit
      sentry(basic_ostream<_CharT, _Traits>& __os);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"







      ~sentry()
      {

 if (bool(_M_os.flags() & ios_base::unitbuf) && !uncaught_exception())
   {

     if (_M_os.rdbuf() && _M_os.rdbuf()->pubsync() == -1)
       _M_os.setstate(ios_base::badbit);
   }
      }
#pragma GCC diagnostic pop
# 489 "/usr/include/c++/11/ostream" 3
      explicit

      operator bool() const
      { return _M_ok; }
    };
# 511 "/usr/include/c++/11/ostream" 3
  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __out, _CharT __c)
    { return __ostream_insert(__out, &__c, 1); }

  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __out, char __c)
    { return (__out << __out.widen(__c)); }


  template<typename _Traits>
    inline basic_ostream<char, _Traits>&
    operator<<(basic_ostream<char, _Traits>& __out, char __c)
    { return __ostream_insert(__out, &__c, 1); }


  template<typename _Traits>
    inline basic_ostream<char, _Traits>&
    operator<<(basic_ostream<char, _Traits>& __out, signed char __c)
    { return (__out << static_cast<char>(__c)); }

  template<typename _Traits>
    inline basic_ostream<char, _Traits>&
    operator<<(basic_ostream<char, _Traits>& __out, unsigned char __c)
    { return (__out << static_cast<char>(__c)); }
# 592 "/usr/include/c++/11/ostream" 3
  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __out, const _CharT* __s)
    {
      if (!__s)
 __out.setstate(ios_base::badbit);
      else
 __ostream_insert(__out, __s,
    static_cast<streamsize>(_Traits::length(__s)));
      return __out;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits> &
    operator<<(basic_ostream<_CharT, _Traits>& __out, const char* __s);


  template<typename _Traits>
    inline basic_ostream<char, _Traits>&
    operator<<(basic_ostream<char, _Traits>& __out, const char* __s)
    {
      if (!__s)
 __out.setstate(ios_base::badbit);
      else
 __ostream_insert(__out, __s,
    static_cast<streamsize>(_Traits::length(__s)));
      return __out;
    }


  template<typename _Traits>
    inline basic_ostream<char, _Traits>&
    operator<<(basic_ostream<char, _Traits>& __out, const signed char* __s)
    { return (__out << reinterpret_cast<const char*>(__s)); }

  template<typename _Traits>
    inline basic_ostream<char, _Traits> &
    operator<<(basic_ostream<char, _Traits>& __out, const unsigned char* __s)
    { return (__out << reinterpret_cast<const char*>(__s)); }
# 682 "/usr/include/c++/11/ostream" 3
  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    endl(basic_ostream<_CharT, _Traits>& __os)
    { return flush(__os.put(__os.widen('\n'))); }
# 694 "/usr/include/c++/11/ostream" 3
  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    ends(basic_ostream<_CharT, _Traits>& __os)
    { return __os.put(_CharT()); }






  template<typename _CharT, typename _Traits>
    inline basic_ostream<_CharT, _Traits>&
    flush(basic_ostream<_CharT, _Traits>& __os)
    { return __os.flush(); }
# 726 "/usr/include/c++/11/ostream" 3
  template<typename _Tp>
    using _Require_derived_from_ios_base
      = _Require<is_class<_Tp>, __not_<is_same<_Tp, ios_base>>,
   is_convertible<typename add_pointer<_Tp>::type, ios_base*>>;

  template<typename _Os, typename _Tp,
    typename = _Require_derived_from_ios_base<_Os>,
    typename
      = decltype(std::declval<_Os&>() << std::declval<const _Tp&>())>
    using __rvalue_stream_insertion_t = _Os&&;
# 748 "/usr/include/c++/11/ostream" 3
  template<typename _Ostream, typename _Tp>
    inline __rvalue_stream_insertion_t<_Ostream, _Tp>
    operator<<(_Ostream&& __os, const _Tp& __x)
    {
      __os << __x;
      return std::move(__os);
    }
# 826 "/usr/include/c++/11/ostream" 3

}

# 1 "/usr/include/c++/11/bits/ostream.tcc" 1 3
# 37 "/usr/include/c++/11/bits/ostream.tcc" 3
       
# 38 "/usr/include/c++/11/bits/ostream.tcc" 3



namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>::sentry::
    sentry(basic_ostream<_CharT, _Traits>& __os)
    : _M_ok(false), _M_os(__os)
    {

      if (__os.tie() && __os.good())
 __os.tie()->flush();

      if (__os.good())
 _M_ok = true;
      else
 __os.setstate(ios_base::failbit);
    }

  template<typename _CharT, typename _Traits>
    template<typename _ValueT>
      basic_ostream<_CharT, _Traits>&
      basic_ostream<_CharT, _Traits>::
      _M_insert(_ValueT __v)
      {
 sentry __cerb(*this);
 if (__cerb)
   {
     ios_base::iostate __err = ios_base::goodbit;
     try
       {
  const __num_put_type& __np = __check_facet(this->_M_num_put);
  if (__np.put(*this, *this, this->fill(), __v).failed())
    __err |= ios_base::badbit;
       }
     catch(__cxxabiv1::__forced_unwind&)
       {
  this->_M_setstate(ios_base::badbit);
  throw;
       }
     catch(...)
       { this->_M_setstate(ios_base::badbit); }
     if (__err)
       this->setstate(__err);
   }
 return *this;
      }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    operator<<(short __n)
    {


      const ios_base::fmtflags __fmt = this->flags() & ios_base::basefield;
      if (__fmt == ios_base::oct || __fmt == ios_base::hex)
 return _M_insert(static_cast<long>(static_cast<unsigned short>(__n)));
      else
 return _M_insert(static_cast<long>(__n));
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    operator<<(int __n)
    {


      const ios_base::fmtflags __fmt = this->flags() & ios_base::basefield;
      if (__fmt == ios_base::oct || __fmt == ios_base::hex)
 return _M_insert(static_cast<long>(static_cast<unsigned int>(__n)));
      else
 return _M_insert(static_cast<long>(__n));
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    operator<<(__streambuf_type* __sbin)
    {
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this);
      if (__cerb && __sbin)
 {
   try
     {
       if (!__copy_streambufs(__sbin, this->rdbuf()))
  __err |= ios_base::failbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::failbit); }
 }
      else if (!__sbin)
 __err |= ios_base::badbit;
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    put(char_type __c)
    {






      sentry __cerb(*this);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       const int_type __put = this->rdbuf()->sputc(__c);
       if (traits_type::eq_int_type(__put, traits_type::eof()))
  __err |= ios_base::badbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    write(const _CharT* __s, streamsize __n)
    {







      sentry __cerb(*this);
      if (__cerb)
 {
   try
     { _M_write(__s, __n); }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    flush()
    {



      ios_base::iostate __err = ios_base::goodbit;
      try
 {
   if (this->rdbuf() && this->rdbuf()->pubsync() == -1)
     __err |= ios_base::badbit;
 }
      catch(__cxxabiv1::__forced_unwind&)
 {
   this->_M_setstate(ios_base::badbit);
   throw;
 }
      catch(...)
 { this->_M_setstate(ios_base::badbit); }
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    typename basic_ostream<_CharT, _Traits>::pos_type
    basic_ostream<_CharT, _Traits>::
    tellp()
    {
      pos_type __ret = pos_type(-1);
      try
 {
   if (!this->fail())
     __ret = this->rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out);
 }
      catch(__cxxabiv1::__forced_unwind&)
 {
   this->_M_setstate(ios_base::badbit);
   throw;
 }
      catch(...)
 { this->_M_setstate(ios_base::badbit); }
      return __ret;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    seekp(pos_type __pos)
    {
      ios_base::iostate __err = ios_base::goodbit;
      try
 {
   if (!this->fail())
     {


       const pos_type __p = this->rdbuf()->pubseekpos(__pos,
            ios_base::out);


       if (__p == pos_type(off_type(-1)))
  __err |= ios_base::failbit;
     }
 }
      catch(__cxxabiv1::__forced_unwind&)
 {
   this->_M_setstate(ios_base::badbit);
   throw;
 }
      catch(...)
 { this->_M_setstate(ios_base::badbit); }
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    basic_ostream<_CharT, _Traits>::
    seekp(off_type __off, ios_base::seekdir __dir)
    {
      ios_base::iostate __err = ios_base::goodbit;
      try
 {
   if (!this->fail())
     {


       const pos_type __p = this->rdbuf()->pubseekoff(__off, __dir,
            ios_base::out);


       if (__p == pos_type(off_type(-1)))
  __err |= ios_base::failbit;
     }
 }
      catch(__cxxabiv1::__forced_unwind&)
 {
   this->_M_setstate(ios_base::badbit);
   throw;
 }
      catch(...)
 { this->_M_setstate(ios_base::badbit); }
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_ostream<_CharT, _Traits>&
    operator<<(basic_ostream<_CharT, _Traits>& __out, const char* __s)
    {
      if (!__s)
 __out.setstate(ios_base::badbit);
      else
 {


   const size_t __clen = char_traits<char>::length(__s);
   try
     {
       struct __ptr_guard
       {
  _CharT *__p;
  __ptr_guard (_CharT *__ip): __p(__ip) { }
  ~__ptr_guard() { delete[] __p; }
  _CharT* __get() { return __p; }
       } __pg (new _CharT[__clen]);

       _CharT *__ws = __pg.__get();
       for (size_t __i = 0; __i < __clen; ++__i)
  __ws[__i] = __out.widen(__s[__i]);
       __ostream_insert(__out, __ws, __clen);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       __out._M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { __out._M_setstate(ios_base::badbit); }
 }
      return __out;
    }




  extern template class basic_ostream<char>;
  extern template ostream& endl(ostream&);
  extern template ostream& ends(ostream&);
  extern template ostream& flush(ostream&);
  extern template ostream& operator<<(ostream&, char);
  extern template ostream& operator<<(ostream&, unsigned char);
  extern template ostream& operator<<(ostream&, signed char);
  extern template ostream& operator<<(ostream&, const char*);
  extern template ostream& operator<<(ostream&, const unsigned char*);
  extern template ostream& operator<<(ostream&, const signed char*);

  extern template ostream& ostream::_M_insert(long);
  extern template ostream& ostream::_M_insert(unsigned long);
  extern template ostream& ostream::_M_insert(bool);

  extern template ostream& ostream::_M_insert(long long);
  extern template ostream& ostream::_M_insert(unsigned long long);

  extern template ostream& ostream::_M_insert(double);
  extern template ostream& ostream::_M_insert(long double);
  extern template ostream& ostream::_M_insert(const void*);


  extern template class basic_ostream<wchar_t>;
  extern template wostream& endl(wostream&);
  extern template wostream& ends(wostream&);
  extern template wostream& flush(wostream&);
  extern template wostream& operator<<(wostream&, wchar_t);
  extern template wostream& operator<<(wostream&, char);
  extern template wostream& operator<<(wostream&, const wchar_t*);
  extern template wostream& operator<<(wostream&, const char*);

  extern template wostream& wostream::_M_insert(long);
  extern template wostream& wostream::_M_insert(unsigned long);
  extern template wostream& wostream::_M_insert(bool);

  extern template wostream& wostream::_M_insert(long long);
  extern template wostream& wostream::_M_insert(unsigned long long);

  extern template wostream& wostream::_M_insert(double);
  extern template wostream& wostream::_M_insert(long double);
  extern template wostream& wostream::_M_insert(const void*);




}
# 830 "/usr/include/c++/11/ostream" 2 3
# 40 "/usr/include/c++/11/iostream" 2 3
# 1 "/usr/include/c++/11/istream" 1 3
# 36 "/usr/include/c++/11/istream" 3
       
# 37 "/usr/include/c++/11/istream" 3




namespace std __attribute__ ((__visibility__ ("default")))
{

# 57 "/usr/include/c++/11/istream" 3
  template<typename _CharT, typename _Traits>
    class basic_istream : virtual public basic_ios<_CharT, _Traits>
    {
    public:

      typedef _CharT char_type;
      typedef typename _Traits::int_type int_type;
      typedef typename _Traits::pos_type pos_type;
      typedef typename _Traits::off_type off_type;
      typedef _Traits traits_type;


      typedef basic_streambuf<_CharT, _Traits> __streambuf_type;
      typedef basic_ios<_CharT, _Traits> __ios_type;
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef num_get<_CharT, istreambuf_iterator<_CharT, _Traits> >
        __num_get_type;
      typedef ctype<_CharT> __ctype_type;

    protected:





      streamsize _M_gcount;

    public:







      explicit
      basic_istream(__streambuf_type* __sb)
      : _M_gcount(streamsize(0))
      { this->init(__sb); }






      virtual
      ~basic_istream()
      { _M_gcount = streamsize(0); }


      class sentry;
      friend class sentry;
# 119 "/usr/include/c++/11/istream" 3
      __istream_type&
      operator>>(__istream_type& (*__pf)(__istream_type&))
      { return __pf(*this); }

      __istream_type&
      operator>>(__ios_type& (*__pf)(__ios_type&))
      {
 __pf(*this);
 return *this;
      }

      __istream_type&
      operator>>(ios_base& (*__pf)(ios_base&))
      {
 __pf(*this);
 return *this;
      }
# 167 "/usr/include/c++/11/istream" 3
      __istream_type&
      operator>>(bool& __n)
      { return _M_extract(__n); }

      __istream_type&
      operator>>(short& __n);

      __istream_type&
      operator>>(unsigned short& __n)
      { return _M_extract(__n); }

      __istream_type&
      operator>>(int& __n);

      __istream_type&
      operator>>(unsigned int& __n)
      { return _M_extract(__n); }

      __istream_type&
      operator>>(long& __n)
      { return _M_extract(__n); }

      __istream_type&
      operator>>(unsigned long& __n)
      { return _M_extract(__n); }


      __istream_type&
      operator>>(long long& __n)
      { return _M_extract(__n); }

      __istream_type&
      operator>>(unsigned long long& __n)
      { return _M_extract(__n); }
# 213 "/usr/include/c++/11/istream" 3
      __istream_type&
      operator>>(float& __f)
      { return _M_extract(__f); }

      __istream_type&
      operator>>(double& __f)
      { return _M_extract(__f); }

      __istream_type&
      operator>>(long double& __f)
      { return _M_extract(__f); }
# 234 "/usr/include/c++/11/istream" 3
      __istream_type&
      operator>>(void*& __p)
      { return _M_extract(__p); }
# 258 "/usr/include/c++/11/istream" 3
      __istream_type&
      operator>>(__streambuf_type* __sb);
# 268 "/usr/include/c++/11/istream" 3
      streamsize
      gcount() const
      { return _M_gcount; }
# 301 "/usr/include/c++/11/istream" 3
      int_type
      get();
# 315 "/usr/include/c++/11/istream" 3
      __istream_type&
      get(char_type& __c);
# 342 "/usr/include/c++/11/istream" 3
      __istream_type&
      get(char_type* __s, streamsize __n, char_type __delim);
# 353 "/usr/include/c++/11/istream" 3
      __istream_type&
      get(char_type* __s, streamsize __n)
      { return this->get(__s, __n, this->widen('\n')); }
# 376 "/usr/include/c++/11/istream" 3
      __istream_type&
      get(__streambuf_type& __sb, char_type __delim);
# 386 "/usr/include/c++/11/istream" 3
      __istream_type&
      get(__streambuf_type& __sb)
      { return this->get(__sb, this->widen('\n')); }
# 415 "/usr/include/c++/11/istream" 3
      __istream_type&
      getline(char_type* __s, streamsize __n, char_type __delim);
# 426 "/usr/include/c++/11/istream" 3
      __istream_type&
      getline(char_type* __s, streamsize __n)
      { return this->getline(__s, __n, this->widen('\n')); }
# 450 "/usr/include/c++/11/istream" 3
      __istream_type&
      ignore(streamsize __n, int_type __delim);

      __istream_type&
      ignore(streamsize __n);

      __istream_type&
      ignore();
# 467 "/usr/include/c++/11/istream" 3
      int_type
      peek();
# 485 "/usr/include/c++/11/istream" 3
      __istream_type&
      read(char_type* __s, streamsize __n);
# 504 "/usr/include/c++/11/istream" 3
      streamsize
      readsome(char_type* __s, streamsize __n);
# 521 "/usr/include/c++/11/istream" 3
      __istream_type&
      putback(char_type __c);
# 537 "/usr/include/c++/11/istream" 3
      __istream_type&
      unget();
# 555 "/usr/include/c++/11/istream" 3
      int
      sync();
# 570 "/usr/include/c++/11/istream" 3
      pos_type
      tellg();
# 585 "/usr/include/c++/11/istream" 3
      __istream_type&
      seekg(pos_type);
# 601 "/usr/include/c++/11/istream" 3
      __istream_type&
      seekg(off_type, ios_base::seekdir);


    protected:
      basic_istream()
      : _M_gcount(streamsize(0))
      { this->init(0); }


      basic_istream(const basic_istream&) = delete;

      basic_istream(basic_istream&& __rhs)
      : __ios_type(), _M_gcount(__rhs._M_gcount)
      {
 __ios_type::move(__rhs);
 __rhs._M_gcount = 0;
      }



      basic_istream& operator=(const basic_istream&) = delete;

      basic_istream&
      operator=(basic_istream&& __rhs)
      {
 swap(__rhs);
 return *this;
      }

      void
      swap(basic_istream& __rhs)
      {
 __ios_type::swap(__rhs);
 std::swap(_M_gcount, __rhs._M_gcount);
      }


      template<typename _ValueT>
 __istream_type&
 _M_extract(_ValueT& __v);
    };


  template<>
    basic_istream<char>&
    basic_istream<char>::
    getline(char_type* __s, streamsize __n, char_type __delim);

  template<>
    basic_istream<char>&
    basic_istream<char>::
    ignore(streamsize __n);

  template<>
    basic_istream<char>&
    basic_istream<char>::
    ignore(streamsize __n, int_type __delim);


  template<>
    basic_istream<wchar_t>&
    basic_istream<wchar_t>::
    getline(char_type* __s, streamsize __n, char_type __delim);

  template<>
    basic_istream<wchar_t>&
    basic_istream<wchar_t>::
    ignore(streamsize __n);

  template<>
    basic_istream<wchar_t>&
    basic_istream<wchar_t>::
    ignore(streamsize __n, int_type __delim);
# 685 "/usr/include/c++/11/istream" 3
  template<typename _CharT, typename _Traits>
    class basic_istream<_CharT, _Traits>::sentry
    {

      bool _M_ok;

    public:

      typedef _Traits traits_type;
      typedef basic_streambuf<_CharT, _Traits> __streambuf_type;
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef typename __istream_type::__ctype_type __ctype_type;
      typedef typename _Traits::int_type __int_type;
# 721 "/usr/include/c++/11/istream" 3
      explicit
      sentry(basic_istream<_CharT, _Traits>& __is, bool __noskipws = false);
# 732 "/usr/include/c++/11/istream" 3
      explicit

      operator bool() const
      { return _M_ok; }
    };
# 750 "/usr/include/c++/11/istream" 3
  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    operator>>(basic_istream<_CharT, _Traits>& __in, _CharT& __c);

  template<class _Traits>
    inline basic_istream<char, _Traits>&
    operator>>(basic_istream<char, _Traits>& __in, unsigned char& __c)
    { return (__in >> reinterpret_cast<char&>(__c)); }

  template<class _Traits>
    inline basic_istream<char, _Traits>&
    operator>>(basic_istream<char, _Traits>& __in, signed char& __c)
    { return (__in >> reinterpret_cast<char&>(__c)); }



  template<typename _CharT, typename _Traits>
    void
    __istream_extract(basic_istream<_CharT, _Traits>&, _CharT*, streamsize);

  void __istream_extract(istream&, char*, streamsize);
# 800 "/usr/include/c++/11/istream" 3
  template<typename _CharT, typename _Traits>
    __attribute__((__nonnull__(2), __access__(__write_only__, 2)))
    inline basic_istream<_CharT, _Traits>&
    operator>>(basic_istream<_CharT, _Traits>& __in, _CharT* __s)
    {
# 834 "/usr/include/c++/11/istream" 3
 {

   streamsize __n = __gnu_cxx::__numeric_traits<streamsize>::__max;
   __n /= sizeof(_CharT);
   std::__istream_extract(__in, __s, __n);
 }
      return __in;
    }

  template<class _Traits>
    __attribute__((__nonnull__(2), __access__(__write_only__, 2)))
    inline basic_istream<char, _Traits>&
    operator>>(basic_istream<char, _Traits>& __in, unsigned char* __s)
    { return __in >> reinterpret_cast<char*>(__s); }

  template<class _Traits>
    __attribute__((__nonnull__(2), __access__(__write_only__, 2)))
    inline basic_istream<char, _Traits>&
    operator>>(basic_istream<char, _Traits>& __in, signed char* __s)
    { return __in >> reinterpret_cast<char*>(__s); }
# 889 "/usr/include/c++/11/istream" 3
  template<typename _CharT, typename _Traits>
    class basic_iostream
    : public basic_istream<_CharT, _Traits>,
      public basic_ostream<_CharT, _Traits>
    {
    public:



      typedef _CharT char_type;
      typedef typename _Traits::int_type int_type;
      typedef typename _Traits::pos_type pos_type;
      typedef typename _Traits::off_type off_type;
      typedef _Traits traits_type;


      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef basic_ostream<_CharT, _Traits> __ostream_type;







      explicit
      basic_iostream(basic_streambuf<_CharT, _Traits>* __sb)
      : __istream_type(__sb), __ostream_type(__sb) { }




      virtual
      ~basic_iostream() { }

    protected:
      basic_iostream()
      : __istream_type(), __ostream_type() { }


      basic_iostream(const basic_iostream&) = delete;

      basic_iostream(basic_iostream&& __rhs)
      : __istream_type(std::move(__rhs)), __ostream_type(*this)
      { }



      basic_iostream& operator=(const basic_iostream&) = delete;

      basic_iostream&
      operator=(basic_iostream&& __rhs)
      {
 swap(__rhs);
 return *this;
      }

      void
      swap(basic_iostream& __rhs)
      { __istream_type::swap(__rhs); }

    };
# 972 "/usr/include/c++/11/istream" 3
  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    ws(basic_istream<_CharT, _Traits>& __is);
# 988 "/usr/include/c++/11/istream" 3
  template<typename _Is, typename _Tp,
    typename = _Require_derived_from_ios_base<_Is>,
    typename = decltype(std::declval<_Is&>() >> std::declval<_Tp>())>
    using __rvalue_stream_extraction_t = _Is&&;
# 1004 "/usr/include/c++/11/istream" 3
  template<typename _Istream, typename _Tp>
    inline __rvalue_stream_extraction_t<_Istream, _Tp>
    operator>>(_Istream&& __is, _Tp&& __x)
    {
      __is >> std::forward<_Tp>(__x);
      return std::move(__is);
    }



}

# 1 "/usr/include/c++/11/bits/istream.tcc" 1 3
# 37 "/usr/include/c++/11/bits/istream.tcc" 3
       
# 38 "/usr/include/c++/11/bits/istream.tcc" 3



namespace std __attribute__ ((__visibility__ ("default")))
{


  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>::sentry::
    sentry(basic_istream<_CharT, _Traits>& __in, bool __noskip) : _M_ok(false)
    {
      ios_base::iostate __err = ios_base::goodbit;
      if (__in.good())
 try
   {
     if (__in.tie())
       __in.tie()->flush();
     if (!__noskip && bool(__in.flags() & ios_base::skipws))
       {
  const __int_type __eof = traits_type::eof();
  __streambuf_type* __sb = __in.rdbuf();
  __int_type __c = __sb->sgetc();

  const __ctype_type& __ct = __check_facet(__in._M_ctype);
  while (!traits_type::eq_int_type(__c, __eof)
         && __ct.is(ctype_base::space,
      traits_type::to_char_type(__c)))
    __c = __sb->snextc();




  if (traits_type::eq_int_type(__c, __eof))
    __err |= ios_base::eofbit;
       }
   }
 catch(__cxxabiv1::__forced_unwind&)
   {
     __in._M_setstate(ios_base::badbit);
     throw;
   }
 catch(...)
   { __in._M_setstate(ios_base::badbit); }

      if (__in.good() && __err == ios_base::goodbit)
 _M_ok = true;
      else
 {
   __err |= ios_base::failbit;
   __in.setstate(__err);
 }
    }

  template<typename _CharT, typename _Traits>
    template<typename _ValueT>
      basic_istream<_CharT, _Traits>&
      basic_istream<_CharT, _Traits>::
      _M_extract(_ValueT& __v)
      {
 sentry __cerb(*this, false);
 if (__cerb)
   {
     ios_base::iostate __err = ios_base::goodbit;
     try
       {
  const __num_get_type& __ng = __check_facet(this->_M_num_get);
  __ng.get(*this, 0, *this, __err, __v);
       }
     catch(__cxxabiv1::__forced_unwind&)
       {
  this->_M_setstate(ios_base::badbit);
  throw;
       }
     catch(...)
       { this->_M_setstate(ios_base::badbit); }
     if (__err)
       this->setstate(__err);
   }
 return *this;
      }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    operator>>(short& __n)
    {


      sentry __cerb(*this, false);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       long __l;
       const __num_get_type& __ng = __check_facet(this->_M_num_get);
       __ng.get(*this, 0, *this, __err, __l);



       if (__l < __gnu_cxx::__numeric_traits<short>::__min)
  {
    __err |= ios_base::failbit;
    __n = __gnu_cxx::__numeric_traits<short>::__min;
  }
       else if (__l > __gnu_cxx::__numeric_traits<short>::__max)
  {
    __err |= ios_base::failbit;
    __n = __gnu_cxx::__numeric_traits<short>::__max;
  }
       else
  __n = short(__l);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    operator>>(int& __n)
    {


      sentry __cerb(*this, false);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       long __l;
       const __num_get_type& __ng = __check_facet(this->_M_num_get);
       __ng.get(*this, 0, *this, __err, __l);



       if (__l < __gnu_cxx::__numeric_traits<int>::__min)
  {
    __err |= ios_base::failbit;
    __n = __gnu_cxx::__numeric_traits<int>::__min;
  }
       else if (__l > __gnu_cxx::__numeric_traits<int>::__max)
  {
    __err |= ios_base::failbit;
    __n = __gnu_cxx::__numeric_traits<int>::__max;
  }
       else
  __n = int(__l);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    operator>>(__streambuf_type* __sbout)
    {
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this, false);
      if (__cerb && __sbout)
 {
   try
     {
       bool __ineof;
       if (!__copy_streambufs_eof(this->rdbuf(), __sbout, __ineof))
  __err |= ios_base::failbit;
       if (__ineof)
  __err |= ios_base::eofbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::failbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::failbit); }
 }
      else if (!__sbout)
 __err |= ios_base::failbit;
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    typename basic_istream<_CharT, _Traits>::int_type
    basic_istream<_CharT, _Traits>::
    get(void)
    {
      const int_type __eof = traits_type::eof();
      int_type __c = __eof;
      _M_gcount = 0;
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   try
     {
       __c = this->rdbuf()->sbumpc();

       if (!traits_type::eq_int_type(__c, __eof))
  _M_gcount = 1;
       else
  __err |= ios_base::eofbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
 }
      if (!_M_gcount)
 __err |= ios_base::failbit;
      if (__err)
 this->setstate(__err);
      return __c;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    get(char_type& __c)
    {
      _M_gcount = 0;
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   try
     {
       const int_type __cb = this->rdbuf()->sbumpc();

       if (!traits_type::eq_int_type(__cb, traits_type::eof()))
  {
    _M_gcount = 1;
    __c = traits_type::to_char_type(__cb);
  }
       else
  __err |= ios_base::eofbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
 }
      if (!_M_gcount)
 __err |= ios_base::failbit;
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    get(char_type* __s, streamsize __n, char_type __delim)
    {
      _M_gcount = 0;
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   try
     {
       const int_type __idelim = traits_type::to_int_type(__delim);
       const int_type __eof = traits_type::eof();
       __streambuf_type* __sb = this->rdbuf();
       int_type __c = __sb->sgetc();

       while (_M_gcount + 1 < __n
       && !traits_type::eq_int_type(__c, __eof)
       && !traits_type::eq_int_type(__c, __idelim))
  {
    *__s++ = traits_type::to_char_type(__c);
    ++_M_gcount;
    __c = __sb->snextc();
  }
       if (traits_type::eq_int_type(__c, __eof))
  __err |= ios_base::eofbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
 }


      if (__n > 0)
 *__s = char_type();
      if (!_M_gcount)
 __err |= ios_base::failbit;
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    get(__streambuf_type& __sb, char_type __delim)
    {
      _M_gcount = 0;
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   try
     {
       const int_type __idelim = traits_type::to_int_type(__delim);
       const int_type __eof = traits_type::eof();
       __streambuf_type* __this_sb = this->rdbuf();
       int_type __c = __this_sb->sgetc();
       char_type __c2 = traits_type::to_char_type(__c);
       unsigned long long __gcount = 0;

       while (!traits_type::eq_int_type(__c, __eof)
       && !traits_type::eq_int_type(__c, __idelim)
       && !traits_type::eq_int_type(__sb.sputc(__c2), __eof))
  {
    ++__gcount;
    __c = __this_sb->snextc();
    __c2 = traits_type::to_char_type(__c);
  }
       if (traits_type::eq_int_type(__c, __eof))
  __err |= ios_base::eofbit;


       if (__gcount <= __gnu_cxx::__numeric_traits<streamsize>::__max)
  _M_gcount = __gcount;
       else
  _M_gcount = __gnu_cxx::__numeric_traits<streamsize>::__max;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
 }
      if (!_M_gcount)
 __err |= ios_base::failbit;
      if (__err)
 this->setstate(__err);
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    getline(char_type* __s, streamsize __n, char_type __delim)
    {
      _M_gcount = 0;
      ios_base::iostate __err = ios_base::goodbit;
      sentry __cerb(*this, true);
      if (__cerb)
        {
          try
            {
              const int_type __idelim = traits_type::to_int_type(__delim);
              const int_type __eof = traits_type::eof();
              __streambuf_type* __sb = this->rdbuf();
              int_type __c = __sb->sgetc();

              while (_M_gcount + 1 < __n
                     && !traits_type::eq_int_type(__c, __eof)
                     && !traits_type::eq_int_type(__c, __idelim))
                {
                  *__s++ = traits_type::to_char_type(__c);
                  __c = __sb->snextc();
                  ++_M_gcount;
                }
              if (traits_type::eq_int_type(__c, __eof))
                __err |= ios_base::eofbit;
              else
                {
                  if (traits_type::eq_int_type(__c, __idelim))
                    {
                      __sb->sbumpc();
                      ++_M_gcount;
                    }
                  else
                    __err |= ios_base::failbit;
                }
            }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
          catch(...)
            { this->_M_setstate(ios_base::badbit); }
        }


      if (__n > 0)
 *__s = char_type();
      if (!_M_gcount)
        __err |= ios_base::failbit;
      if (__err)
        this->setstate(__err);
      return *this;
    }




  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    ignore(void)
    {
      _M_gcount = 0;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       const int_type __eof = traits_type::eof();
       __streambuf_type* __sb = this->rdbuf();

       if (traits_type::eq_int_type(__sb->sbumpc(), __eof))
  __err |= ios_base::eofbit;
       else
  _M_gcount = 1;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    ignore(streamsize __n)
    {
      _M_gcount = 0;
      sentry __cerb(*this, true);
      if (__cerb && __n > 0)
        {
          ios_base::iostate __err = ios_base::goodbit;
          try
            {
              const int_type __eof = traits_type::eof();
              __streambuf_type* __sb = this->rdbuf();
              int_type __c = __sb->sgetc();
# 528 "/usr/include/c++/11/bits/istream.tcc" 3
       bool __large_ignore = false;
       while (true)
  {
    while (_M_gcount < __n
    && !traits_type::eq_int_type(__c, __eof))
      {
        ++_M_gcount;
        __c = __sb->snextc();
      }
    if (__n == __gnu_cxx::__numeric_traits<streamsize>::__max
        && !traits_type::eq_int_type(__c, __eof))
      {
        _M_gcount =
   __gnu_cxx::__numeric_traits<streamsize>::__min;
        __large_ignore = true;
      }
    else
      break;
  }

       if (__n == __gnu_cxx::__numeric_traits<streamsize>::__max)
  {
    if (__large_ignore)
      _M_gcount = __gnu_cxx::__numeric_traits<streamsize>::__max;

    if (traits_type::eq_int_type(__c, __eof))
      __err |= ios_base::eofbit;
  }
       else if (_M_gcount < __n)
  {
    if (traits_type::eq_int_type(__c, __eof))
      __err |= ios_base::eofbit;
  }
            }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
          catch(...)
            { this->_M_setstate(ios_base::badbit); }
          if (__err)
            this->setstate(__err);
        }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    ignore(streamsize __n, int_type __delim)
    {
      _M_gcount = 0;
      sentry __cerb(*this, true);
      if (__cerb && __n > 0)
        {
          ios_base::iostate __err = ios_base::goodbit;
          try
            {
              const int_type __eof = traits_type::eof();
              __streambuf_type* __sb = this->rdbuf();
              int_type __c = __sb->sgetc();


       bool __large_ignore = false;
       while (true)
  {
    while (_M_gcount < __n
    && !traits_type::eq_int_type(__c, __eof)
    && !traits_type::eq_int_type(__c, __delim))
      {
        ++_M_gcount;
        __c = __sb->snextc();
      }
    if (__n == __gnu_cxx::__numeric_traits<streamsize>::__max
        && !traits_type::eq_int_type(__c, __eof)
        && !traits_type::eq_int_type(__c, __delim))
      {
        _M_gcount =
   __gnu_cxx::__numeric_traits<streamsize>::__min;
        __large_ignore = true;
      }
    else
      break;
  }

       if (__n == __gnu_cxx::__numeric_traits<streamsize>::__max)
  {
    if (__large_ignore)
      _M_gcount = __gnu_cxx::__numeric_traits<streamsize>::__max;

    if (traits_type::eq_int_type(__c, __eof))
      __err |= ios_base::eofbit;
    else
      {
        if (_M_gcount != __n)
   ++_M_gcount;
        __sb->sbumpc();
      }
  }
       else if (_M_gcount < __n)
  {
    if (traits_type::eq_int_type(__c, __eof))
      __err |= ios_base::eofbit;
    else
      {
        ++_M_gcount;
        __sb->sbumpc();
      }
  }
            }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
          catch(...)
            { this->_M_setstate(ios_base::badbit); }
          if (__err)
            this->setstate(__err);
        }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    typename basic_istream<_CharT, _Traits>::int_type
    basic_istream<_CharT, _Traits>::
    peek(void)
    {
      int_type __c = traits_type::eof();
      _M_gcount = 0;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       __c = this->rdbuf()->sgetc();
       if (traits_type::eq_int_type(__c, traits_type::eof()))
  __err |= ios_base::eofbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return __c;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    read(char_type* __s, streamsize __n)
    {
      _M_gcount = 0;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       _M_gcount = this->rdbuf()->sgetn(__s, __n);
       if (_M_gcount != __n)
  __err |= (ios_base::eofbit | ios_base::failbit);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    streamsize
    basic_istream<_CharT, _Traits>::
    readsome(char_type* __s, streamsize __n)
    {
      _M_gcount = 0;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {

       const streamsize __num = this->rdbuf()->in_avail();
       if (__num > 0)
  _M_gcount = this->rdbuf()->sgetn(__s, std::min(__num, __n));
       else if (__num == -1)
  __err |= ios_base::eofbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return _M_gcount;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    putback(char_type __c)
    {


      _M_gcount = 0;

      this->clear(this->rdstate() & ~ios_base::eofbit);
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       const int_type __eof = traits_type::eof();
       __streambuf_type* __sb = this->rdbuf();
       if (!__sb
    || traits_type::eq_int_type(__sb->sputbackc(__c), __eof))
  __err |= ios_base::badbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    unget(void)
    {


      _M_gcount = 0;

      this->clear(this->rdstate() & ~ios_base::eofbit);
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       const int_type __eof = traits_type::eof();
       __streambuf_type* __sb = this->rdbuf();
       if (!__sb
    || traits_type::eq_int_type(__sb->sungetc(), __eof))
  __err |= ios_base::badbit;
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    int
    basic_istream<_CharT, _Traits>::
    sync(void)
    {


      int __ret = -1;
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       __streambuf_type* __sb = this->rdbuf();
       if (__sb)
  {
    if (__sb->pubsync() == -1)
      __err |= ios_base::badbit;
    else
      __ret = 0;
  }
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return __ret;
    }

  template<typename _CharT, typename _Traits>
    typename basic_istream<_CharT, _Traits>::pos_type
    basic_istream<_CharT, _Traits>::
    tellg(void)
    {


      pos_type __ret = pos_type(-1);
      sentry __cerb(*this, true);
      if (__cerb)
 {
   try
     {
       if (!this->fail())
  __ret = this->rdbuf()->pubseekoff(0, ios_base::cur,
        ios_base::in);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
 }
      return __ret;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    seekg(pos_type __pos)
    {



      this->clear(this->rdstate() & ~ios_base::eofbit);
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       if (!this->fail())
  {

    const pos_type __p = this->rdbuf()->pubseekpos(__pos,
         ios_base::in);


    if (__p == pos_type(off_type(-1)))
      __err |= ios_base::failbit;
  }
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }

  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    basic_istream<_CharT, _Traits>::
    seekg(off_type __off, ios_base::seekdir __dir)
    {



      this->clear(this->rdstate() & ~ios_base::eofbit);
      sentry __cerb(*this, true);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       if (!this->fail())
  {

    const pos_type __p = this->rdbuf()->pubseekoff(__off, __dir,
         ios_base::in);


    if (__p == pos_type(off_type(-1)))
      __err |= ios_base::failbit;
  }
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       this->_M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { this->_M_setstate(ios_base::badbit); }
   if (__err)
     this->setstate(__err);
 }
      return *this;
    }


  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    operator>>(basic_istream<_CharT, _Traits>& __in, _CharT& __c)
    {
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef typename __istream_type::int_type __int_type;

      typename __istream_type::sentry __cerb(__in, false);
      if (__cerb)
 {
   ios_base::iostate __err = ios_base::goodbit;
   try
     {
       const __int_type __cb = __in.rdbuf()->sbumpc();
       if (!_Traits::eq_int_type(__cb, _Traits::eof()))
  __c = _Traits::to_char_type(__cb);
       else
  __err |= (ios_base::eofbit | ios_base::failbit);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       __in._M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { __in._M_setstate(ios_base::badbit); }
   if (__err)
     __in.setstate(__err);
 }
      return __in;
    }

  template<typename _CharT, typename _Traits>
    void
    __istream_extract(basic_istream<_CharT, _Traits>& __in, _CharT* __s,
        streamsize __num)
    {
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef basic_streambuf<_CharT, _Traits> __streambuf_type;
      typedef typename _Traits::int_type int_type;
      typedef _CharT char_type;
      typedef ctype<_CharT> __ctype_type;

      streamsize __extracted = 0;
      ios_base::iostate __err = ios_base::goodbit;
      typename __istream_type::sentry __cerb(__in, false);
      if (__cerb)
 {
   try
     {

       streamsize __width = __in.width();
       if (0 < __width && __width < __num)
  __num = __width;

       const __ctype_type& __ct = use_facet<__ctype_type>(__in.getloc());

       const int_type __eof = _Traits::eof();
       __streambuf_type* __sb = __in.rdbuf();
       int_type __c = __sb->sgetc();

       while (__extracted < __num - 1
       && !_Traits::eq_int_type(__c, __eof)
       && !__ct.is(ctype_base::space,
     _Traits::to_char_type(__c)))
  {
    *__s++ = _Traits::to_char_type(__c);
    ++__extracted;
    __c = __sb->snextc();
  }

       if (__extracted < __num - 1
    && _Traits::eq_int_type(__c, __eof))
  __err |= ios_base::eofbit;



       *__s = char_type();
       __in.width(0);
     }
   catch(__cxxabiv1::__forced_unwind&)
     {
       __in._M_setstate(ios_base::badbit);
       throw;
     }
   catch(...)
     { __in._M_setstate(ios_base::badbit); }
 }
      if (!__extracted)
 __err |= ios_base::failbit;
      if (__err)
 __in.setstate(__err);
    }


  template<typename _CharT, typename _Traits>
    basic_istream<_CharT, _Traits>&
    ws(basic_istream<_CharT, _Traits>& __in)
    {
      typedef basic_istream<_CharT, _Traits> __istream_type;
      typedef basic_streambuf<_CharT, _Traits> __streambuf_type;
      typedef typename __istream_type::int_type __int_type;
      typedef ctype<_CharT> __ctype_type;

      const __ctype_type& __ct = use_facet<__ctype_type>(__in.getloc());
      const __int_type __eof = _Traits::eof();
      __streambuf_type* __sb = __in.rdbuf();
      __int_type __c = __sb->sgetc();

      while (!_Traits::eq_int_type(__c, __eof)
      && __ct.is(ctype_base::space, _Traits::to_char_type(__c)))
 __c = __sb->snextc();

       if (_Traits::eq_int_type(__c, __eof))
  __in.setstate(ios_base::eofbit);
      return __in;
    }




  extern template class basic_istream<char>;
  extern template istream& ws(istream&);
  extern template istream& operator>>(istream&, char&);
  extern template istream& operator>>(istream&, unsigned char&);
  extern template istream& operator>>(istream&, signed char&);

  extern template istream& istream::_M_extract(unsigned short&);
  extern template istream& istream::_M_extract(unsigned int&);
  extern template istream& istream::_M_extract(long&);
  extern template istream& istream::_M_extract(unsigned long&);
  extern template istream& istream::_M_extract(bool&);

  extern template istream& istream::_M_extract(long long&);
  extern template istream& istream::_M_extract(unsigned long long&);

  extern template istream& istream::_M_extract(float&);
  extern template istream& istream::_M_extract(double&);
  extern template istream& istream::_M_extract(long double&);
  extern template istream& istream::_M_extract(void*&);

  extern template class basic_iostream<char>;


  extern template class basic_istream<wchar_t>;
  extern template wistream& ws(wistream&);
  extern template wistream& operator>>(wistream&, wchar_t&);
  extern template void __istream_extract(wistream&, wchar_t*, streamsize);

  extern template wistream& wistream::_M_extract(unsigned short&);
  extern template wistream& wistream::_M_extract(unsigned int&);
  extern template wistream& wistream::_M_extract(long&);
  extern template wistream& wistream::_M_extract(unsigned long&);
  extern template wistream& wistream::_M_extract(bool&);

  extern template wistream& wistream::_M_extract(long long&);
  extern template wistream& wistream::_M_extract(unsigned long long&);

  extern template wistream& wistream::_M_extract(float&);
  extern template wistream& wistream::_M_extract(double&);
  extern template wistream& wistream::_M_extract(long double&);
  extern template wistream& wistream::_M_extract(void*&);

  extern template class basic_iostream<wchar_t>;




}
# 1017 "/usr/include/c++/11/istream" 2 3
# 41 "/usr/include/c++/11/iostream" 2 3

namespace std __attribute__ ((__visibility__ ("default")))
{

# 60 "/usr/include/c++/11/iostream" 3
  extern istream cin;
  extern ostream cout;
  extern ostream cerr;
  extern ostream clog;


  extern wistream wcin;
  extern wostream wcout;
  extern wostream wcerr;
  extern wostream wclog;




  static ios_base::Init __ioinit;


}
# 43 "Nc3Values.cc" 2

# 1 "/usr/include/c++/11/cstring" 1 3
# 39 "/usr/include/c++/11/cstring" 3
       
# 40 "/usr/include/c++/11/cstring" 3


# 1 "/usr/include/string.h" 1 3 4
# 26 "/usr/include/string.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/string.h" 2 3 4

extern "C" {




# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 34 "/usr/include/string.h" 2 3 4
# 43 "/usr/include/string.h" 3 4
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
       size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern void *memmove (void *__dest, const void *__src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));





extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
        int __c, size_t __n)
    noexcept (true) __attribute__ ((__nonnull__ (1, 2))) __attribute__ ((__access__ (__write_only__, 1, 4)));




extern void *memset (void *__s, int __c, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));



extern "C++"
{
extern void *memchr (void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const void *memchr (const void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 89 "/usr/include/string.h" 3 4
}
# 99 "/usr/include/string.h" 3 4
extern "C++" void *rawmemchr (void *__s, int __c)
     noexcept (true) __asm ("rawmemchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern "C++" const void *rawmemchr (const void *__s, int __c)
     noexcept (true) __asm ("rawmemchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));







extern "C++" void *memrchr (void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)))
      __attribute__ ((__access__ (__read_only__, 1, 3)));
extern "C++" const void *memrchr (const void *__s, int __c, size_t __n)
      noexcept (true) __asm ("memrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)))
      __attribute__ ((__access__ (__read_only__, 1, 3)));
# 125 "/usr/include/string.h" 3 4
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));

extern char *strncat (char *__restrict __dest, const char *__restrict __src,
        size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern int strcmp (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strcoll (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));

extern size_t strxfrm (char *__restrict __dest,
         const char *__restrict __src, size_t __n)
    noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 1, 3)));






extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));


extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
    locale_t __l) noexcept (true) __attribute__ ((__nonnull__ (2, 4)))
     __attribute__ ((__access__ (__write_only__, 1, 3)));





extern char *strdup (const char *__s)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));






extern char *strndup (const char *__string, size_t __n)
     noexcept (true) __attribute__ ((__malloc__)) __attribute__ ((__nonnull__ (1)));
# 208 "/usr/include/string.h" 3 4
extern "C++"
{
extern char *strchr (char *__s, int __c)
     noexcept (true) __asm ("strchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *strchr (const char *__s, int __c)
     noexcept (true) __asm ("strchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 228 "/usr/include/string.h" 3 4
}






extern "C++"
{
extern char *strrchr (char *__s, int __c)
     noexcept (true) __asm ("strrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *strrchr (const char *__s, int __c)
     noexcept (true) __asm ("strrchr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 255 "/usr/include/string.h" 3 4
}
# 265 "/usr/include/string.h" 3 4
extern "C++" char *strchrnul (char *__s, int __c)
     noexcept (true) __asm ("strchrnul") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern "C++" const char *strchrnul (const char *__s, int __c)
     noexcept (true) __asm ("strchrnul") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 277 "/usr/include/string.h" 3 4
extern size_t strcspn (const char *__s, const char *__reject)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern size_t strspn (const char *__s, const char *__accept)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern "C++"
{
extern char *strpbrk (char *__s, const char *__accept)
     noexcept (true) __asm ("strpbrk") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern const char *strpbrk (const char *__s, const char *__accept)
     noexcept (true) __asm ("strpbrk") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 305 "/usr/include/string.h" 3 4
}






extern "C++"
{
extern char *strstr (char *__haystack, const char *__needle)
     noexcept (true) __asm ("strstr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern const char *strstr (const char *__haystack, const char *__needle)
     noexcept (true) __asm ("strstr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 332 "/usr/include/string.h" 3 4
}







extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     noexcept (true) __attribute__ ((__nonnull__ (2)));



extern char *__strtok_r (char *__restrict __s,
    const char *__restrict __delim,
    char **__restrict __save_ptr)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));

extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
         char **__restrict __save_ptr)
     noexcept (true) __attribute__ ((__nonnull__ (2, 3)));





extern "C++" char *strcasestr (char *__haystack, const char *__needle)
     noexcept (true) __asm ("strcasestr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
extern "C++" const char *strcasestr (const char *__haystack,
         const char *__needle)
     noexcept (true) __asm ("strcasestr") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
# 373 "/usr/include/string.h" 3 4
extern void *memmem (const void *__haystack, size_t __haystacklen,
       const void *__needle, size_t __needlelen)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 3)))
    __attribute__ ((__access__ (__read_only__, 1, 2)))
    __attribute__ ((__access__ (__read_only__, 3, 4)));



extern void *__mempcpy (void *__restrict __dest,
   const void *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern void *mempcpy (void *__restrict __dest,
        const void *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern size_t strlen (const char *__s)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern size_t strnlen (const char *__string, size_t __maxlen)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




extern char *strerror (int __errnum) noexcept (true);
# 428 "/usr/include/string.h" 3 4
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen)
     noexcept (true) __attribute__ ((__nonnull__ (2))) __attribute__ ((__access__ (__write_only__, 2, 3)));




extern const char *strerrordesc_np (int __err) noexcept (true);

extern const char *strerrorname_np (int __err) noexcept (true);





extern char *strerror_l (int __errnum, locale_t __l) noexcept (true);



# 1 "/usr/include/strings.h" 1 3 4
# 23 "/usr/include/strings.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 24 "/usr/include/strings.h" 2 3 4






extern "C" {



extern int bcmp (const void *__s1, const void *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern void bcopy (const void *__src, void *__dest, size_t __n)
  noexcept (true) __attribute__ ((__nonnull__ (1, 2)));


extern void bzero (void *__s, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)));



extern "C++"
{
extern char *index (char *__s, int __c)
     noexcept (true) __asm ("index") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *index (const char *__s, int __c)
     noexcept (true) __asm ("index") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 66 "/usr/include/strings.h" 3 4
}







extern "C++"
{
extern char *rindex (char *__s, int __c)
     noexcept (true) __asm ("rindex") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
extern const char *rindex (const char *__s, int __c)
     noexcept (true) __asm ("rindex") __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));
# 94 "/usr/include/strings.h" 3 4
}
# 104 "/usr/include/strings.h" 3 4
extern int ffs (int __i) noexcept (true) __attribute__ ((__const__));





extern int ffsl (long int __l) noexcept (true) __attribute__ ((__const__));
__extension__ extern int ffsll (long long int __ll)
     noexcept (true) __attribute__ ((__const__));



extern int strcasecmp (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern int strncasecmp (const char *__s1, const char *__s2, size_t __n)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));






extern int strcasecmp_l (const char *__s1, const char *__s2, locale_t __loc)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 3)));



extern int strncasecmp_l (const char *__s1, const char *__s2,
     size_t __n, locale_t __loc)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2, 4)));


}
# 447 "/usr/include/string.h" 2 3 4



extern void explicit_bzero (void *__s, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)))
    __attribute__ ((__access__ (__write_only__, 1, 2)));



extern char *strsep (char **__restrict __stringp,
       const char *__restrict __delim)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern char *strsignal (int __sig) noexcept (true);



extern const char *sigabbrev_np (int __sig) noexcept (true);


extern const char *sigdescr_np (int __sig) noexcept (true);



extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));



extern char *__stpncpy (char *__restrict __dest,
   const char *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));
extern char *stpncpy (char *__restrict __dest,
        const char *__restrict __src, size_t __n)
     noexcept (true) __attribute__ ((__nonnull__ (1, 2)));




extern int strverscmp (const char *__s1, const char *__s2)
     noexcept (true) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));


extern char *strfry (char *__string) noexcept (true) __attribute__ ((__nonnull__ (1)));


extern void *memfrob (void *__s, size_t __n) noexcept (true) __attribute__ ((__nonnull__ (1)))
    __attribute__ ((__access__ (__write_only__, 1, 2)));







extern "C++" char *basename (char *__filename)
     noexcept (true) __asm ("basename") __attribute__ ((__nonnull__ (1)));
extern "C++" const char *basename (const char *__filename)
     noexcept (true) __asm ("basename") __attribute__ ((__nonnull__ (1)));
# 523 "/usr/include/string.h" 3 4
}
# 43 "/usr/include/c++/11/cstring" 2 3
# 71 "/usr/include/c++/11/cstring" 3
extern "C++"
{
namespace std __attribute__ ((__visibility__ ("default")))
{


  using ::memchr;
  using ::memcmp;
  using ::memcpy;
  using ::memmove;
  using ::memset;
  using ::strcat;
  using ::strcmp;
  using ::strcoll;
  using ::strcpy;
  using ::strcspn;
  using ::strerror;
  using ::strlen;
  using ::strncat;
  using ::strncmp;
  using ::strncpy;
  using ::strspn;
  using ::strtok;
  using ::strxfrm;
  using ::strchr;
  using ::strpbrk;
  using ::strrchr;
  using ::strstr;
# 122 "/usr/include/c++/11/cstring" 3

}
}
# 45 "Nc3Values.cc" 2
# 1 "../include/Ncxx/Nc3Values.hh" 1
# 46 "../include/Ncxx/Nc3Values.hh"
# 1 "/usr/include/c++/11/sstream" 1 3
# 36 "/usr/include/c++/11/sstream" 3
       
# 37 "/usr/include/c++/11/sstream" 3
# 48 "/usr/include/c++/11/sstream" 3
namespace std __attribute__ ((__visibility__ ("default")))
{

namespace __cxx11 {
# 71 "/usr/include/c++/11/sstream" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    class basic_stringbuf : public basic_streambuf<_CharT, _Traits>
    {
      struct __xfer_bufptrs;


      using allocator_traits = std::allocator_traits<_Alloc>;
      using _Noexcept_swap
 = __or_<typename allocator_traits::propagate_on_container_swap,
  typename allocator_traits::is_always_equal>;


    public:

      typedef _CharT char_type;
      typedef _Traits traits_type;


      typedef _Alloc allocator_type;
      typedef typename traits_type::int_type int_type;
      typedef typename traits_type::pos_type pos_type;
      typedef typename traits_type::off_type off_type;

      typedef basic_streambuf<char_type, traits_type> __streambuf_type;
      typedef basic_string<char_type, _Traits, _Alloc> __string_type;
      typedef typename __string_type::size_type __size_type;

    protected:

      ios_base::openmode _M_mode;


      __string_type _M_string;

    public:
# 114 "/usr/include/c++/11/sstream" 3
      basic_stringbuf()
      : __streambuf_type(), _M_mode(ios_base::in | ios_base::out), _M_string()
      { }
# 125 "/usr/include/c++/11/sstream" 3
      explicit
      basic_stringbuf(ios_base::openmode __mode)
      : __streambuf_type(), _M_mode(__mode), _M_string()
      { }
# 138 "/usr/include/c++/11/sstream" 3
      explicit
      basic_stringbuf(const __string_type& __str,
        ios_base::openmode __mode = ios_base::in | ios_base::out)
      : __streambuf_type(), _M_mode(),
 _M_string(__str.data(), __str.size(), __str.get_allocator())
      { _M_stringbuf_init(__mode); }


      basic_stringbuf(const basic_stringbuf&) = delete;

      basic_stringbuf(basic_stringbuf&& __rhs)
      : basic_stringbuf(std::move(__rhs), __xfer_bufptrs(__rhs, this))
      { __rhs._M_sync(const_cast<char_type*>(__rhs._M_string.data()), 0, 0); }
# 202 "/usr/include/c++/11/sstream" 3
      basic_stringbuf&
      operator=(const basic_stringbuf&) = delete;

      basic_stringbuf&
      operator=(basic_stringbuf&& __rhs)
      {
 __xfer_bufptrs __st{__rhs, this};
 const __streambuf_type& __base = __rhs;
 __streambuf_type::operator=(__base);
 this->pubimbue(__rhs.getloc());
 _M_mode = __rhs._M_mode;
 _M_string = std::move(__rhs._M_string);
 __rhs._M_sync(const_cast<char_type*>(__rhs._M_string.data()), 0, 0);
 return *this;
      }

      void
      swap(basic_stringbuf& __rhs) noexcept(_Noexcept_swap::value)
      {
 __xfer_bufptrs __l_st{*this, std::__addressof(__rhs)};
 __xfer_bufptrs __r_st{__rhs, this};
 __streambuf_type& __base = __rhs;
 __streambuf_type::swap(__base);
 __rhs.pubimbue(this->pubimbue(__rhs.getloc()));
 std::swap(_M_mode, __rhs._M_mode);
 std::swap(_M_string, __rhs._M_string);
      }
# 241 "/usr/include/c++/11/sstream" 3
      __string_type
      str() const
      {
 __string_type __ret(_M_string.get_allocator());
 if (char_type* __hi = _M_high_mark())
   __ret.assign(this->pbase(), __hi);
 else
   __ret = _M_string;
 return __ret;
      }
# 294 "/usr/include/c++/11/sstream" 3
      void
      str(const __string_type& __s)
      {


 _M_string.assign(__s.data(), __s.size());
 _M_stringbuf_init(_M_mode);
      }
# 323 "/usr/include/c++/11/sstream" 3
    protected:

      void
      _M_stringbuf_init(ios_base::openmode __mode)
      {
 _M_mode = __mode;
 __size_type __len = 0;
 if (_M_mode & (ios_base::ate | ios_base::app))
   __len = _M_string.size();
 _M_sync(const_cast<char_type*>(_M_string.data()), 0, __len);
      }

      virtual streamsize
      showmanyc()
      {
 streamsize __ret = -1;
 if (_M_mode & ios_base::in)
   {
     _M_update_egptr();
     __ret = this->egptr() - this->gptr();
   }
 return __ret;
      }

      virtual int_type
      underflow();

      virtual int_type
      pbackfail(int_type __c = traits_type::eof());

      virtual int_type
      overflow(int_type __c = traits_type::eof());
# 367 "/usr/include/c++/11/sstream" 3
      virtual __streambuf_type*
      setbuf(char_type* __s, streamsize __n)
      {
 if (__s && __n >= 0)
   {






     _M_string.clear();


     _M_sync(__s, __n, 0);
   }
 return this;
      }

      virtual pos_type
      seekoff(off_type __off, ios_base::seekdir __way,
       ios_base::openmode __mode = ios_base::in | ios_base::out);

      virtual pos_type
      seekpos(pos_type __sp,
       ios_base::openmode __mode = ios_base::in | ios_base::out);




      void
      _M_sync(char_type* __base, __size_type __i, __size_type __o);



      void
      _M_update_egptr()
      {
 if (char_type* __pptr = this->pptr())
   {
     char_type* __egptr = this->egptr();
     if (!__egptr || __pptr > __egptr)
       {
  if (_M_mode & ios_base::in)
    this->setg(this->eback(), this->gptr(), __pptr);
  else
    this->setg(__pptr, __pptr, __pptr);
       }
   }
      }



      void
      _M_pbump(char_type* __pbeg, char_type* __pend, off_type __off);

    private:




      __attribute__((__always_inline__))
      char_type*
      _M_high_mark() const noexcept
      {
 if (char_type* __pptr = this->pptr())
   {
     char_type* __egptr = this->egptr();
     if (!__egptr || __pptr > __egptr)
       return __pptr;
     else
       return __egptr;
   }
 return 0;
      }





      struct __xfer_bufptrs
      {
 __xfer_bufptrs(const basic_stringbuf& __from, basic_stringbuf* __to)
 : _M_to{__to}, _M_goff{-1, -1, -1}, _M_poff{-1, -1, -1}
 {
   const _CharT* const __str = __from._M_string.data();
   const _CharT* __end = nullptr;
   if (__from.eback())
     {
       _M_goff[0] = __from.eback() - __str;
       _M_goff[1] = __from.gptr() - __str;
       _M_goff[2] = __from.egptr() - __str;
       __end = __from.egptr();
     }
   if (__from.pbase())
     {
       _M_poff[0] = __from.pbase() - __str;
       _M_poff[1] = __from.pptr() - __from.pbase();
       _M_poff[2] = __from.epptr() - __str;
       if (!__end || __from.pptr() > __end)
  __end = __from.pptr();
     }


   if (__end)
     {


       auto& __mut_from = const_cast<basic_stringbuf&>(__from);
       __mut_from._M_string._M_length(__end - __str);
     }
 }

 ~__xfer_bufptrs()
 {
   char_type* __str = const_cast<char_type*>(_M_to->_M_string.data());
   if (_M_goff[0] != -1)
     _M_to->setg(__str+_M_goff[0], __str+_M_goff[1], __str+_M_goff[2]);
   if (_M_poff[0] != -1)
     _M_to->_M_pbump(__str+_M_poff[0], __str+_M_poff[2], _M_poff[1]);
 }

 basic_stringbuf* _M_to;
 off_type _M_goff[3];
 off_type _M_poff[3];
      };
# 503 "/usr/include/c++/11/sstream" 3
      basic_stringbuf(basic_stringbuf&& __rhs, __xfer_bufptrs&&)
      : __streambuf_type(static_cast<const __streambuf_type&>(__rhs)),
      _M_mode(__rhs._M_mode), _M_string(std::move(__rhs._M_string))
      { }
# 518 "/usr/include/c++/11/sstream" 3
    };
# 536 "/usr/include/c++/11/sstream" 3
  template<typename _CharT, typename _Traits, typename _Alloc>
    class basic_istringstream : public basic_istream<_CharT, _Traits>
    {
    public:

      typedef _CharT char_type;
      typedef _Traits traits_type;


      typedef _Alloc allocator_type;
      typedef typename traits_type::int_type int_type;
      typedef typename traits_type::pos_type pos_type;
      typedef typename traits_type::off_type off_type;


      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef basic_stringbuf<_CharT, _Traits, _Alloc> __stringbuf_type;
      typedef basic_istream<char_type, traits_type> __istream_type;

    private:
      __stringbuf_type _M_stringbuf;

    public:
# 570 "/usr/include/c++/11/sstream" 3
      basic_istringstream()
      : __istream_type(), _M_stringbuf(ios_base::in)
      { this->init(&_M_stringbuf); }
# 586 "/usr/include/c++/11/sstream" 3
      explicit
      basic_istringstream(ios_base::openmode __mode)
      : __istream_type(), _M_stringbuf(__mode | ios_base::in)
      { this->init(&_M_stringbuf); }
# 604 "/usr/include/c++/11/sstream" 3
      explicit
      basic_istringstream(const __string_type& __str,
     ios_base::openmode __mode = ios_base::in)
      : __istream_type(), _M_stringbuf(__str, __mode | ios_base::in)
      { this->init(&_M_stringbuf); }







      ~basic_istringstream()
      { }


      basic_istringstream(const basic_istringstream&) = delete;

      basic_istringstream(basic_istringstream&& __rhs)
      : __istream_type(std::move(__rhs)),
      _M_stringbuf(std::move(__rhs._M_stringbuf))
      { __istream_type::set_rdbuf(&_M_stringbuf); }
# 661 "/usr/include/c++/11/sstream" 3
      basic_istringstream&
      operator=(const basic_istringstream&) = delete;

      basic_istringstream&
      operator=(basic_istringstream&& __rhs)
      {
 __istream_type::operator=(std::move(__rhs));
 _M_stringbuf = std::move(__rhs._M_stringbuf);
 return *this;
      }

      void
      swap(basic_istringstream& __rhs)
      {
 __istream_type::swap(__rhs);
 _M_stringbuf.swap(__rhs._M_stringbuf);
      }
# 687 "/usr/include/c++/11/sstream" 3
      __stringbuf_type*
      rdbuf() const
      { return const_cast<__stringbuf_type*>(&_M_stringbuf); }





      __string_type
      str() const
      { return _M_stringbuf.str(); }
# 722 "/usr/include/c++/11/sstream" 3
      void
      str(const __string_type& __s)
      { _M_stringbuf.str(__s); }
# 739 "/usr/include/c++/11/sstream" 3
    };
# 757 "/usr/include/c++/11/sstream" 3
  template <typename _CharT, typename _Traits, typename _Alloc>
    class basic_ostringstream : public basic_ostream<_CharT, _Traits>
    {
    public:

      typedef _CharT char_type;
      typedef _Traits traits_type;


      typedef _Alloc allocator_type;
      typedef typename traits_type::int_type int_type;
      typedef typename traits_type::pos_type pos_type;
      typedef typename traits_type::off_type off_type;


      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef basic_stringbuf<_CharT, _Traits, _Alloc> __stringbuf_type;
      typedef basic_ostream<char_type, traits_type> __ostream_type;

    private:
      __stringbuf_type _M_stringbuf;

    public:
# 791 "/usr/include/c++/11/sstream" 3
      basic_ostringstream()
      : __ostream_type(), _M_stringbuf(ios_base::out)
      { this->init(&_M_stringbuf); }
# 807 "/usr/include/c++/11/sstream" 3
      explicit
      basic_ostringstream(ios_base::openmode __mode)
      : __ostream_type(), _M_stringbuf(__mode | ios_base::out)
      { this->init(&_M_stringbuf); }
# 825 "/usr/include/c++/11/sstream" 3
      explicit
      basic_ostringstream(const __string_type& __str,
     ios_base::openmode __mode = ios_base::out)
      : __ostream_type(), _M_stringbuf(__str, __mode | ios_base::out)
      { this->init(&_M_stringbuf); }







      ~basic_ostringstream()
      { }


      basic_ostringstream(const basic_ostringstream&) = delete;

      basic_ostringstream(basic_ostringstream&& __rhs)
      : __ostream_type(std::move(__rhs)),
      _M_stringbuf(std::move(__rhs._M_stringbuf))
      { __ostream_type::set_rdbuf(&_M_stringbuf); }
# 882 "/usr/include/c++/11/sstream" 3
      basic_ostringstream&
      operator=(const basic_ostringstream&) = delete;

      basic_ostringstream&
      operator=(basic_ostringstream&& __rhs)
      {
 __ostream_type::operator=(std::move(__rhs));
 _M_stringbuf = std::move(__rhs._M_stringbuf);
 return *this;
      }

      void
      swap(basic_ostringstream& __rhs)
      {
 __ostream_type::swap(__rhs);
 _M_stringbuf.swap(__rhs._M_stringbuf);
      }
# 908 "/usr/include/c++/11/sstream" 3
      __stringbuf_type*
      rdbuf() const
      { return const_cast<__stringbuf_type*>(&_M_stringbuf); }





      __string_type
      str() const
      { return _M_stringbuf.str(); }
# 943 "/usr/include/c++/11/sstream" 3
      void
      str(const __string_type& __s)
      { _M_stringbuf.str(__s); }
# 960 "/usr/include/c++/11/sstream" 3
    };
# 978 "/usr/include/c++/11/sstream" 3
  template <typename _CharT, typename _Traits, typename _Alloc>
    class basic_stringstream : public basic_iostream<_CharT, _Traits>
    {
    public:

      typedef _CharT char_type;
      typedef _Traits traits_type;


      typedef _Alloc allocator_type;
      typedef typename traits_type::int_type int_type;
      typedef typename traits_type::pos_type pos_type;
      typedef typename traits_type::off_type off_type;


      typedef basic_string<_CharT, _Traits, _Alloc> __string_type;
      typedef basic_stringbuf<_CharT, _Traits, _Alloc> __stringbuf_type;
      typedef basic_iostream<char_type, traits_type> __iostream_type;

    private:
      __stringbuf_type _M_stringbuf;

    public:
# 1012 "/usr/include/c++/11/sstream" 3
      basic_stringstream()
      : __iostream_type(), _M_stringbuf(ios_base::out | ios_base::in)
      { this->init(&_M_stringbuf); }
# 1026 "/usr/include/c++/11/sstream" 3
      explicit
      basic_stringstream(ios_base::openmode __m)
      : __iostream_type(), _M_stringbuf(__m)
      { this->init(&_M_stringbuf); }
# 1042 "/usr/include/c++/11/sstream" 3
      explicit
      basic_stringstream(const __string_type& __str,
    ios_base::openmode __m = ios_base::out | ios_base::in)
      : __iostream_type(), _M_stringbuf(__str, __m)
      { this->init(&_M_stringbuf); }







      ~basic_stringstream()
      { }


      basic_stringstream(const basic_stringstream&) = delete;

      basic_stringstream(basic_stringstream&& __rhs)
      : __iostream_type(std::move(__rhs)),
      _M_stringbuf(std::move(__rhs._M_stringbuf))
      { __iostream_type::set_rdbuf(&_M_stringbuf); }
# 1101 "/usr/include/c++/11/sstream" 3
      basic_stringstream&
      operator=(const basic_stringstream&) = delete;

      basic_stringstream&
      operator=(basic_stringstream&& __rhs)
      {
 __iostream_type::operator=(std::move(__rhs));
 _M_stringbuf = std::move(__rhs._M_stringbuf);
 return *this;
      }

      void
      swap(basic_stringstream& __rhs)
      {
 __iostream_type::swap(__rhs);
 _M_stringbuf.swap(__rhs._M_stringbuf);
      }
# 1127 "/usr/include/c++/11/sstream" 3
      __stringbuf_type*
      rdbuf() const
      { return const_cast<__stringbuf_type*>(&_M_stringbuf); }





      __string_type
      str() const
      { return _M_stringbuf.str(); }
# 1162 "/usr/include/c++/11/sstream" 3
      void
      str(const __string_type& __s)
      { _M_stringbuf.str(__s); }
# 1179 "/usr/include/c++/11/sstream" 3
    };



  template <class _CharT, class _Traits, class _Allocator>
    inline void
    swap(basic_stringbuf<_CharT, _Traits, _Allocator>& __x,
  basic_stringbuf<_CharT, _Traits, _Allocator>& __y)
    noexcept(noexcept(__x.swap(__y)))
    { __x.swap(__y); }


  template <class _CharT, class _Traits, class _Allocator>
    inline void
    swap(basic_istringstream<_CharT, _Traits, _Allocator>& __x,
  basic_istringstream<_CharT, _Traits, _Allocator>& __y)
    { __x.swap(__y); }


  template <class _CharT, class _Traits, class _Allocator>
    inline void
    swap(basic_ostringstream<_CharT, _Traits, _Allocator>& __x,
  basic_ostringstream<_CharT, _Traits, _Allocator>& __y)
    { __x.swap(__y); }


  template <class _CharT, class _Traits, class _Allocator>
    inline void
    swap(basic_stringstream<_CharT, _Traits, _Allocator>& __x,
  basic_stringstream<_CharT, _Traits, _Allocator>& __y)
    { __x.swap(__y); }


}

}



# 1 "/usr/include/c++/11/bits/sstream.tcc" 1 3
# 37 "/usr/include/c++/11/bits/sstream.tcc" 3
       
# 38 "/usr/include/c++/11/bits/sstream.tcc" 3

namespace std __attribute__ ((__visibility__ ("default")))
{


  template <class _CharT, class _Traits, class _Alloc>
    typename basic_stringbuf<_CharT, _Traits, _Alloc>::int_type
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    pbackfail(int_type __c)
    {
      int_type __ret = traits_type::eof();
      if (this->eback() < this->gptr())
 {


   const bool __testeof = traits_type::eq_int_type(__c, __ret);
   if (!__testeof)
     {
       const bool __testeq = traits_type::eq(traits_type::
          to_char_type(__c),
          this->gptr()[-1]);
       const bool __testout = this->_M_mode & ios_base::out;
       if (__testeq || __testout)
  {
    this->gbump(-1);
    if (!__testeq)
      *this->gptr() = traits_type::to_char_type(__c);
    __ret = __c;
  }
     }
   else
     {
       this->gbump(-1);
       __ret = traits_type::not_eof(__c);
     }
 }
      return __ret;
    }

  template <class _CharT, class _Traits, class _Alloc>
    typename basic_stringbuf<_CharT, _Traits, _Alloc>::int_type
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    overflow(int_type __c)
    {
      const bool __testout = this->_M_mode & ios_base::out;
      if (__builtin_expect(!__testout, false))
 return traits_type::eof();

      const bool __testeof = traits_type::eq_int_type(__c, traits_type::eof());
      if (__builtin_expect(__testeof, false))
 return traits_type::not_eof(__c);

      const __size_type __capacity = _M_string.capacity();


      if (size_t(this->epptr() - this->pbase()) < __capacity)
 {

   char_type* __base = const_cast<char_type*>(_M_string.data());
   _M_pbump(__base, __base + __capacity, this->pptr() - this->pbase());
   if (_M_mode & ios_base::in)
     {
       const __size_type __nget = this->gptr() - this->eback();
       const __size_type __eget = this->egptr() - this->eback();
       this->setg(__base, __base + __nget, __base + __eget + 1);
     }
   *this->pptr() = traits_type::to_char_type(__c);
   this->pbump(1);
   return __c;
 }


      const __size_type __max_size = _M_string.max_size();
      const bool __testput = this->pptr() < this->epptr();
      if (__builtin_expect(!__testput && __capacity == __max_size, false))
 return traits_type::eof();



      const char_type __conv = traits_type::to_char_type(__c);
      if (!__testput)
 {
# 129 "/usr/include/c++/11/bits/sstream.tcc" 3
   const __size_type __opt_len = std::max(__size_type(2 * __capacity),
       __size_type(512));
   const __size_type __len = std::min(__opt_len, __max_size);
   __string_type __tmp(_M_string.get_allocator());
   __tmp.reserve(__len);
   if (this->pbase())
     __tmp.assign(this->pbase(), this->epptr() - this->pbase());
   __tmp.push_back(__conv);
   _M_string.swap(__tmp);
   _M_sync(const_cast<char_type*>(_M_string.data()),
    this->gptr() - this->eback(), this->pptr() - this->pbase());
 }
      else
 *this->pptr() = __conv;
      this->pbump(1);
      return __c;
    }

  template <class _CharT, class _Traits, class _Alloc>
    typename basic_stringbuf<_CharT, _Traits, _Alloc>::int_type
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    underflow()
    {
      int_type __ret = traits_type::eof();
      const bool __testin = this->_M_mode & ios_base::in;
      if (__testin)
 {

   _M_update_egptr();

   if (this->gptr() < this->egptr())
     __ret = traits_type::to_int_type(*this->gptr());
 }
      return __ret;
    }

  template <class _CharT, class _Traits, class _Alloc>
    typename basic_stringbuf<_CharT, _Traits, _Alloc>::pos_type
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    seekoff(off_type __off, ios_base::seekdir __way, ios_base::openmode __mode)
    {
      pos_type __ret = pos_type(off_type(-1));
      bool __testin = (ios_base::in & this->_M_mode & __mode) != 0;
      bool __testout = (ios_base::out & this->_M_mode & __mode) != 0;
      const bool __testboth = __testin && __testout && __way != ios_base::cur;
      __testin &= !(__mode & ios_base::out);
      __testout &= !(__mode & ios_base::in);



      const char_type* __beg = __testin ? this->eback() : this->pbase();
      if ((__beg || !__off) && (__testin || __testout || __testboth))
 {
   _M_update_egptr();

   off_type __newoffi = __off;
   off_type __newoffo = __newoffi;
   if (__way == ios_base::cur)
     {
       __newoffi += this->gptr() - __beg;
       __newoffo += this->pptr() - __beg;
     }
   else if (__way == ios_base::end)
     __newoffo = __newoffi += this->egptr() - __beg;

   if ((__testin || __testboth)
       && __newoffi >= 0
       && this->egptr() - __beg >= __newoffi)
     {
       this->setg(this->eback(), this->eback() + __newoffi,
    this->egptr());
       __ret = pos_type(__newoffi);
     }
   if ((__testout || __testboth)
       && __newoffo >= 0
       && this->egptr() - __beg >= __newoffo)
     {
       _M_pbump(this->pbase(), this->epptr(), __newoffo);
       __ret = pos_type(__newoffo);
     }
 }
      return __ret;
    }

  template <class _CharT, class _Traits, class _Alloc>
    typename basic_stringbuf<_CharT, _Traits, _Alloc>::pos_type
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    seekpos(pos_type __sp, ios_base::openmode __mode)
    {
      pos_type __ret = pos_type(off_type(-1));
      const bool __testin = (ios_base::in & this->_M_mode & __mode) != 0;
      const bool __testout = (ios_base::out & this->_M_mode & __mode) != 0;

      const char_type* __beg = __testin ? this->eback() : this->pbase();
      if ((__beg || !off_type(__sp)) && (__testin || __testout))
 {
   _M_update_egptr();

   const off_type __pos(__sp);
   const bool __testpos = (0 <= __pos
      && __pos <= this->egptr() - __beg);
   if (__testpos)
     {
       if (__testin)
  this->setg(this->eback(), this->eback() + __pos,
      this->egptr());
       if (__testout)
  _M_pbump(this->pbase(), this->epptr(), __pos);
       __ret = __sp;
     }
 }
      return __ret;
    }

  template <class _CharT, class _Traits, class _Alloc>
    void
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    _M_sync(char_type* __base, __size_type __i, __size_type __o)
    {
      const bool __testin = _M_mode & ios_base::in;
      const bool __testout = _M_mode & ios_base::out;
      char_type* __endg = __base + _M_string.size();
      char_type* __endp = __base + _M_string.capacity();

      if (__base != _M_string.data())
 {

   __endg += __i;
   __i = 0;
   __endp = __endg;
 }

      if (__testin)
 this->setg(__base, __base + __i, __endg);
      if (__testout)
 {
   _M_pbump(__base, __endp, __o);



   if (!__testin)
     this->setg(__endg, __endg, __endg);
 }
    }

  template <class _CharT, class _Traits, class _Alloc>
    void
    basic_stringbuf<_CharT, _Traits, _Alloc>::
    _M_pbump(char_type* __pbeg, char_type* __pend, off_type __off)
    {
      this->setp(__pbeg, __pend);
      while (__off > __gnu_cxx::__numeric_traits<int>::__max)
 {
   this->pbump(__gnu_cxx::__numeric_traits<int>::__max);
   __off -= __gnu_cxx::__numeric_traits<int>::__max;
 }
      this->pbump(__off);
    }




  extern template class basic_stringbuf<char>;
  extern template class basic_istringstream<char>;
  extern template class basic_ostringstream<char>;
  extern template class basic_stringstream<char>;


  extern template class basic_stringbuf<wchar_t>;
  extern template class basic_istringstream<wchar_t>;
  extern template class basic_ostringstream<wchar_t>;
  extern template class basic_stringstream<wchar_t>;




}
# 1219 "/usr/include/c++/11/sstream" 2 3
# 47 "../include/Ncxx/Nc3Values.hh" 2
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/limits.h" 1 3 4
# 34 "/usr/lib/gcc/x86_64-redhat-linux/11/include/limits.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/syslimits.h" 1 3 4






# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/limits.h" 1 3 4
# 203 "/usr/lib/gcc/x86_64-redhat-linux/11/include/limits.h" 3 4
# 1 "/usr/include/limits.h" 1 3 4
# 26 "/usr/include/limits.h" 3 4
# 1 "/usr/include/bits/libc-header-start.h" 1 3 4
# 27 "/usr/include/limits.h" 2 3 4
# 195 "/usr/include/limits.h" 3 4
# 1 "/usr/include/bits/posix1_lim.h" 1 3 4
# 27 "/usr/include/bits/posix1_lim.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 28 "/usr/include/bits/posix1_lim.h" 2 3 4
# 161 "/usr/include/bits/posix1_lim.h" 3 4
# 1 "/usr/include/bits/local_lim.h" 1 3 4
# 38 "/usr/include/bits/local_lim.h" 3 4
# 1 "/usr/include/linux/limits.h" 1 3 4
# 39 "/usr/include/bits/local_lim.h" 2 3 4
# 162 "/usr/include/bits/posix1_lim.h" 2 3 4
# 196 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/bits/posix2_lim.h" 1 3 4
# 200 "/usr/include/limits.h" 2 3 4



# 1 "/usr/include/bits/xopen_lim.h" 1 3 4
# 64 "/usr/include/bits/xopen_lim.h" 3 4
# 1 "/usr/include/bits/uio_lim.h" 1 3 4
# 65 "/usr/include/bits/xopen_lim.h" 2 3 4
# 204 "/usr/include/limits.h" 2 3 4
# 204 "/usr/lib/gcc/x86_64-redhat-linux/11/include/limits.h" 2 3 4
# 8 "/usr/lib/gcc/x86_64-redhat-linux/11/include/syslimits.h" 2 3 4
# 35 "/usr/lib/gcc/x86_64-redhat-linux/11/include/limits.h" 2 3 4
# 48 "../include/Ncxx/Nc3Values.hh" 2
# 1 "/usr/include/netcdf.h" 1 3 4
# 16 "/usr/include/netcdf.h" 3 4
# 1 "/usr/lib/gcc/x86_64-redhat-linux/11/include/stddef.h" 1 3 4
# 17 "/usr/include/netcdf.h" 2 3 4
# 25 "/usr/include/netcdf.h" 3 4
typedef int nc_type;


extern "C" {
# 528 "/usr/include/netcdf.h" 3 4
 extern const char *
nc_inq_libvers(void);

 extern const char *
nc_strerror(int ncerr);


typedef struct NC_Dispatch NC_Dispatch;
 extern int
nc_def_user_format(int mode_flag, NC_Dispatch *dispatch_table, char *magic_number);

 extern int
nc_inq_user_format(int mode_flag, NC_Dispatch **dispatch_table, char *magic_number);

 extern int
nc__create(const char *path, int cmode, size_t initialsz,
         size_t *chunksizehintp, int *ncidp);

 extern int
nc_create(const char *path, int cmode, int *ncidp);

 extern int
nc__open(const char *path, int mode,
        size_t *chunksizehintp, int *ncidp);

 extern int
nc_open(const char *path, int mode, int *ncidp);


 extern int
nc_inq_path(int ncid, size_t *pathlen, char *path);



 extern int
nc_inq_ncid(int ncid, const char *name, int *grp_ncid);



 extern int
nc_inq_grps(int ncid, int *numgrps, int *ncids);


 extern int
nc_inq_grpname(int ncid, char *name);



 extern int
nc_inq_grpname_full(int ncid, size_t *lenp, char *full_name);


 extern int
nc_inq_grpname_len(int ncid, size_t *lenp);


 extern int
nc_inq_grp_parent(int ncid, int *parent_ncid);


 extern int
nc_inq_grp_ncid(int ncid, const char *grp_name, int *grp_ncid);


 extern int
nc_inq_grp_full_ncid(int ncid, const char *full_name, int *grp_ncid);


 extern int
nc_inq_varids(int ncid, int *nvars, int *varids);



 extern int
nc_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents);



 extern int
nc_inq_typeids(int ncid, int *ntypes, int *typeids);


 extern int
nc_inq_type_equal(int ncid1, nc_type typeid1, int ncid2,
                  nc_type typeid2, int *equal);


 extern int
nc_def_grp(int parent_ncid, const char *name, int *new_ncid);


 extern int
nc_rename_grp(int grpid, const char *name);




 extern int
nc_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp);


 extern int
nc_insert_compound(int ncid, nc_type xtype, const char *name,
                   size_t offset, nc_type field_typeid);


 extern int
nc_insert_array_compound(int ncid, nc_type xtype, const char *name,
                         size_t offset, nc_type field_typeid,
                         int ndims, const int *dim_sizes);


 extern int
nc_inq_type(int ncid, nc_type xtype, char *name, size_t *size);


 extern int
nc_inq_typeid(int ncid, const char *name, nc_type *typeidp);


 extern int
nc_inq_compound(int ncid, nc_type xtype, char *name, size_t *sizep,
                size_t *nfieldsp);


 extern int
nc_inq_compound_name(int ncid, nc_type xtype, char *name);


 extern int
nc_inq_compound_size(int ncid, nc_type xtype, size_t *sizep);


 extern int
nc_inq_compound_nfields(int ncid, nc_type xtype, size_t *nfieldsp);


 extern int
nc_inq_compound_field(int ncid, nc_type xtype, int fieldid, char *name,
                      size_t *offsetp, nc_type *field_typeidp, int *ndimsp,
                      int *dim_sizesp);


 extern int
nc_inq_compound_fieldname(int ncid, nc_type xtype, int fieldid,
                          char *name);


 extern int
nc_inq_compound_fieldindex(int ncid, nc_type xtype, const char *name,
                           int *fieldidp);


 extern int
nc_inq_compound_fieldoffset(int ncid, nc_type xtype, int fieldid,
                            size_t *offsetp);


 extern int
nc_inq_compound_fieldtype(int ncid, nc_type xtype, int fieldid,
                          nc_type *field_typeidp);



 extern int
nc_inq_compound_fieldndims(int ncid, nc_type xtype, int fieldid,
                           int *ndimsp);



 extern int
nc_inq_compound_fielddim_sizes(int ncid, nc_type xtype, int fieldid,
                               int *dim_sizes);


typedef struct {
    size_t len;
    void *p;
} nc_vlen_t;
# 715 "/usr/include/netcdf.h" 3 4
 extern int
nc_def_vlen(int ncid, const char *name, nc_type base_typeid, nc_type *xtypep);


 extern int
nc_inq_vlen(int ncid, nc_type xtype, char *name, size_t *datum_sizep,
            nc_type *base_nc_typep);





 extern int
nc_free_vlen(nc_vlen_t *vl);

 extern int
nc_free_vlens(size_t len, nc_vlen_t vlens[]);


 extern int
nc_put_vlen_element(int ncid, int typeid1, void *vlen_element,
                    size_t len, const void *data);

 extern int
nc_get_vlen_element(int ncid, int typeid1, const void *vlen_element,
                    size_t *len, void *data);





 extern int
nc_free_string(size_t len, char **data);


 extern int
nc_inq_user_type(int ncid, nc_type xtype, char *name, size_t *size,
                 nc_type *base_nc_typep, size_t *nfieldsp, int *classp);


 extern int
nc_put_att(int ncid, int varid, const char *name, nc_type xtype,
           size_t len, const void *op);


 extern int
nc_get_att(int ncid, int varid, const char *name, void *ip);





 extern int
nc_def_enum(int ncid, nc_type base_typeid, const char *name,
            nc_type *typeidp);



 extern int
nc_insert_enum(int ncid, nc_type xtype, const char *name,
               const void *value);



 extern int
nc_inq_enum(int ncid, nc_type xtype, char *name, nc_type *base_nc_typep,
            size_t *base_sizep, size_t *num_membersp);



 extern int
nc_inq_enum_member(int ncid, nc_type xtype, int idx, char *name,
                   void *value);



 extern int
nc_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier);




 extern int
nc_def_opaque(int ncid, size_t size, const char *name, nc_type *xtypep);


 extern int
nc_inq_opaque(int ncid, nc_type xtype, char *name, size_t *sizep);


 extern int
nc_put_var(int ncid, int varid, const void *op);


 extern int
nc_get_var(int ncid, int varid, void *ip);


 extern int
nc_put_var1(int ncid, int varid, const size_t *indexp,
            const void *op);


 extern int
nc_get_var1(int ncid, int varid, const size_t *indexp, void *ip);


 extern int
nc_put_vara(int ncid, int varid, const size_t *startp,
            const size_t *countp, const void *op);


 extern int
nc_get_vara(int ncid, int varid, const size_t *startp,
            const size_t *countp, void *ip);


 extern int
nc_put_vars(int ncid, int varid, const size_t *startp,
            const size_t *countp, const ptrdiff_t *stridep,
            const void *op);


 extern int
nc_get_vars(int ncid, int varid, const size_t *startp,
            const size_t *countp, const ptrdiff_t *stridep,
            void *ip);


 extern int
nc_put_varm(int ncid, int varid, const size_t *startp,
            const size_t *countp, const ptrdiff_t *stridep,
            const ptrdiff_t *imapp, const void *op);


 extern int
nc_get_varm(int ncid, int varid, const size_t *startp,
            const size_t *countp, const ptrdiff_t *stridep,
            const ptrdiff_t *imapp, void *ip);





 extern int
nc_def_var_deflate(int ncid, int varid, int shuffle, int deflate,
                   int deflate_level);


 extern int
nc_inq_var_deflate(int ncid, int varid, int *shufflep,
                   int *deflatep, int *deflate_levelp);


 extern int nc_def_var_szip(int ncid, int varid, int options_mask,
                            int pixels_per_block);


 extern int
nc_inq_var_szip(int ncid, int varid, int *options_maskp, int *pixels_per_blockp);



 extern int
nc_def_var_fletcher32(int ncid, int varid, int fletcher32);


 extern int
nc_inq_var_fletcher32(int ncid, int varid, int *fletcher32p);



 extern int
nc_def_var_chunking(int ncid, int varid, int storage, const size_t *chunksizesp);


 extern int
nc_inq_var_chunking(int ncid, int varid, int *storagep, size_t *chunksizesp);



 extern int
nc_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value);


 extern int
nc_inq_var_fill(int ncid, int varid, int *no_fill, void *fill_valuep);


 extern int
nc_def_var_endian(int ncid, int varid, int endian);


 extern int
nc_inq_var_endian(int ncid, int varid, int *endianp);


 extern int
nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms);


 extern int
nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparams, unsigned int* params);


 extern int
nc_set_fill(int ncid, int fillmode, int *old_modep);



 extern int
nc_set_default_format(int format, int *old_formatp);


 extern int
nc_set_chunk_cache(size_t size, size_t nelems, float preemption);


 extern int
nc_get_chunk_cache(size_t *sizep, size_t *nelemsp, float *preemptionp);


 extern int
nc_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems,
                       float preemption);


 extern int
nc_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp,
                       float *preemptionp);

 extern int
nc_redef(int ncid);


 extern int
nc__enddef(int ncid, size_t h_minfree, size_t v_align,
        size_t v_minfree, size_t r_align);

 extern int
nc_enddef(int ncid);

 extern int
nc_sync(int ncid);

 extern int
nc_abort(int ncid);

 extern int
nc_close(int ncid);

 extern int
nc_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp);

 extern int
nc_inq_ndims(int ncid, int *ndimsp);

 extern int
nc_inq_nvars(int ncid, int *nvarsp);

 extern int
nc_inq_natts(int ncid, int *nattsp);

 extern int
nc_inq_unlimdim(int ncid, int *unlimdimidp);


 extern int
nc_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp);


 extern int
nc_inq_format(int ncid, int *formatp);


 extern int
nc_inq_format_extended(int ncid, int *formatp, int* modep);



 extern int
nc_def_dim(int ncid, const char *name, size_t len, int *idp);

 extern int
nc_inq_dimid(int ncid, const char *name, int *idp);

 extern int
nc_inq_dim(int ncid, int dimid, char *name, size_t *lenp);

 extern int
nc_inq_dimname(int ncid, int dimid, char *name);

 extern int
nc_inq_dimlen(int ncid, int dimid, size_t *lenp);

 extern int
nc_rename_dim(int ncid, int dimid, const char *name);




 extern int
nc_inq_att(int ncid, int varid, const char *name,
           nc_type *xtypep, size_t *lenp);

 extern int
nc_inq_attid(int ncid, int varid, const char *name, int *idp);

 extern int
nc_inq_atttype(int ncid, int varid, const char *name, nc_type *xtypep);

 extern int
nc_inq_attlen(int ncid, int varid, const char *name, size_t *lenp);

 extern int
nc_inq_attname(int ncid, int varid, int attnum, char *name);

 extern int
nc_copy_att(int ncid_in, int varid_in, const char *name, int ncid_out, int varid_out);

 extern int
nc_rename_att(int ncid, int varid, const char *name, const char *newname);

 extern int
nc_del_att(int ncid, int varid, const char *name);



 extern int
nc_put_att_text(int ncid, int varid, const char *name,
                size_t len, const char *op);

 extern int
nc_get_att_text(int ncid, int varid, const char *name, char *ip);

 extern int
nc_put_att_string(int ncid, int varid, const char *name,
                  size_t len, const char **op);

 extern int
nc_get_att_string(int ncid, int varid, const char *name, char **ip);

 extern int
nc_put_att_uchar(int ncid, int varid, const char *name, nc_type xtype,
                 size_t len, const unsigned char *op);

 extern int
nc_get_att_uchar(int ncid, int varid, const char *name, unsigned char *ip);

 extern int
nc_put_att_schar(int ncid, int varid, const char *name, nc_type xtype,
                 size_t len, const signed char *op);

 extern int
nc_get_att_schar(int ncid, int varid, const char *name, signed char *ip);

 extern int
nc_put_att_short(int ncid, int varid, const char *name, nc_type xtype,
                 size_t len, const short *op);

 extern int
nc_get_att_short(int ncid, int varid, const char *name, short *ip);

 extern int
nc_put_att_int(int ncid, int varid, const char *name, nc_type xtype,
               size_t len, const int *op);

 extern int
nc_get_att_int(int ncid, int varid, const char *name, int *ip);

 extern int
nc_put_att_long(int ncid, int varid, const char *name, nc_type xtype,
                size_t len, const long *op);

 extern int
nc_get_att_long(int ncid, int varid, const char *name, long *ip);

 extern int
nc_put_att_float(int ncid, int varid, const char *name, nc_type xtype,
                 size_t len, const float *op);

 extern int
nc_get_att_float(int ncid, int varid, const char *name, float *ip);

 extern int
nc_put_att_double(int ncid, int varid, const char *name, nc_type xtype,
                  size_t len, const double *op);

 extern int
nc_get_att_double(int ncid, int varid, const char *name, double *ip);

 extern int
nc_put_att_ushort(int ncid, int varid, const char *name, nc_type xtype,
                  size_t len, const unsigned short *op);

 extern int
nc_get_att_ushort(int ncid, int varid, const char *name, unsigned short *ip);

 extern int
nc_put_att_uint(int ncid, int varid, const char *name, nc_type xtype,
                size_t len, const unsigned int *op);

 extern int
nc_get_att_uint(int ncid, int varid, const char *name, unsigned int *ip);

 extern int
nc_put_att_longlong(int ncid, int varid, const char *name, nc_type xtype,
                 size_t len, const long long *op);

 extern int
nc_get_att_longlong(int ncid, int varid, const char *name, long long *ip);

 extern int
nc_put_att_ulonglong(int ncid, int varid, const char *name, nc_type xtype,
                     size_t len, const unsigned long long *op);

 extern int
nc_get_att_ulonglong(int ncid, int varid, const char *name,
                     unsigned long long *ip);





 extern int
nc_def_var(int ncid, const char *name, nc_type xtype, int ndims,
           const int *dimidsp, int *varidp);

 extern int
nc_inq_var(int ncid, int varid, char *name, nc_type *xtypep,
           int *ndimsp, int *dimidsp, int *nattsp);

 extern int
nc_inq_varid(int ncid, const char *name, int *varidp);

 extern int
nc_inq_varname(int ncid, int varid, char *name);

 extern int
nc_inq_vartype(int ncid, int varid, nc_type *xtypep);

 extern int
nc_inq_varndims(int ncid, int varid, int *ndimsp);

 extern int
nc_inq_vardimid(int ncid, int varid, int *dimidsp);

 extern int
nc_inq_varnatts(int ncid, int varid, int *nattsp);

 extern int
nc_rename_var(int ncid, int varid, const char *name);

 extern int
nc_copy_var(int ncid_in, int varid, int ncid_out);
# 1179 "/usr/include/netcdf.h" 3 4
 extern int
nc_put_var1_text(int ncid, int varid, const size_t *indexp, const char *op);

 extern int
nc_get_var1_text(int ncid, int varid, const size_t *indexp, char *ip);

 extern int
nc_put_var1_uchar(int ncid, int varid, const size_t *indexp,
                  const unsigned char *op);

 extern int
nc_get_var1_uchar(int ncid, int varid, const size_t *indexp,
                  unsigned char *ip);

 extern int
nc_put_var1_schar(int ncid, int varid, const size_t *indexp,
                  const signed char *op);

 extern int
nc_get_var1_schar(int ncid, int varid, const size_t *indexp,
                  signed char *ip);

 extern int
nc_put_var1_short(int ncid, int varid, const size_t *indexp,
                  const short *op);

 extern int
nc_get_var1_short(int ncid, int varid, const size_t *indexp,
                  short *ip);

 extern int
nc_put_var1_int(int ncid, int varid, const size_t *indexp, const int *op);

 extern int
nc_get_var1_int(int ncid, int varid, const size_t *indexp, int *ip);

 extern int
nc_put_var1_long(int ncid, int varid, const size_t *indexp, const long *op);

 extern int
nc_get_var1_long(int ncid, int varid, const size_t *indexp, long *ip);

 extern int
nc_put_var1_float(int ncid, int varid, const size_t *indexp, const float *op);

 extern int
nc_get_var1_float(int ncid, int varid, const size_t *indexp, float *ip);

 extern int
nc_put_var1_double(int ncid, int varid, const size_t *indexp, const double *op);

 extern int
nc_get_var1_double(int ncid, int varid, const size_t *indexp, double *ip);

 extern int
nc_put_var1_ushort(int ncid, int varid, const size_t *indexp,
                   const unsigned short *op);

 extern int
nc_get_var1_ushort(int ncid, int varid, const size_t *indexp,
                   unsigned short *ip);

 extern int
nc_put_var1_uint(int ncid, int varid, const size_t *indexp,
                 const unsigned int *op);

 extern int
nc_get_var1_uint(int ncid, int varid, const size_t *indexp,
                 unsigned int *ip);

 extern int
nc_put_var1_longlong(int ncid, int varid, const size_t *indexp,
                     const long long *op);

 extern int
nc_get_var1_longlong(int ncid, int varid, const size_t *indexp,
                  long long *ip);

 extern int
nc_put_var1_ulonglong(int ncid, int varid, const size_t *indexp,
                   const unsigned long long *op);

 extern int
nc_get_var1_ulonglong(int ncid, int varid, const size_t *indexp,
                   unsigned long long *ip);

 extern int
nc_put_var1_string(int ncid, int varid, const size_t *indexp,
                   const char **op);

 extern int
nc_get_var1_string(int ncid, int varid, const size_t *indexp,
                   char **ip);




 extern int
nc_put_vara_text(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const char *op);

 extern int
nc_get_vara_text(int ncid, int varid, const size_t *startp,
                 const size_t *countp, char *ip);

 extern int
nc_put_vara_uchar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const unsigned char *op);

 extern int
nc_get_vara_uchar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, unsigned char *ip);

 extern int
nc_put_vara_schar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const signed char *op);

 extern int
nc_get_vara_schar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, signed char *ip);

 extern int
nc_put_vara_short(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const short *op);

 extern int
nc_get_vara_short(int ncid, int varid, const size_t *startp,
                  const size_t *countp, short *ip);

 extern int
nc_put_vara_int(int ncid, int varid, const size_t *startp,
                const size_t *countp, const int *op);

 extern int
nc_get_vara_int(int ncid, int varid, const size_t *startp,
                const size_t *countp, int *ip);

 extern int
nc_put_vara_long(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const long *op);

 extern int
nc_get_vara_long(int ncid, int varid,
        const size_t *startp, const size_t *countp, long *ip);

 extern int
nc_put_vara_float(int ncid, int varid,
        const size_t *startp, const size_t *countp, const float *op);

 extern int
nc_get_vara_float(int ncid, int varid,
        const size_t *startp, const size_t *countp, float *ip);

 extern int
nc_put_vara_double(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const double *op);

 extern int
nc_get_vara_double(int ncid, int varid, const size_t *startp,
                   const size_t *countp, double *ip);

 extern int
nc_put_vara_ushort(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const unsigned short *op);

 extern int
nc_get_vara_ushort(int ncid, int varid, const size_t *startp,
                   const size_t *countp, unsigned short *ip);

 extern int
nc_put_vara_uint(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const unsigned int *op);

 extern int
nc_get_vara_uint(int ncid, int varid, const size_t *startp,
                 const size_t *countp, unsigned int *ip);

 extern int
nc_put_vara_longlong(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const long long *op);

 extern int
nc_get_vara_longlong(int ncid, int varid, const size_t *startp,
                  const size_t *countp, long long *ip);

 extern int
nc_put_vara_ulonglong(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const unsigned long long *op);

 extern int
nc_get_vara_ulonglong(int ncid, int varid, const size_t *startp,
                   const size_t *countp, unsigned long long *ip);

 extern int
nc_put_vara_string(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const char **op);

 extern int
nc_get_vara_string(int ncid, int varid, const size_t *startp,
                   const size_t *countp, char **ip);




 extern int
nc_put_vars_text(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const char *op);

 extern int
nc_get_vars_text(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        char *ip);

 extern int
nc_put_vars_uchar(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const unsigned char *op);

 extern int
nc_get_vars_uchar(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        unsigned char *ip);

 extern int
nc_put_vars_schar(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const signed char *op);

 extern int
nc_get_vars_schar(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        signed char *ip);

 extern int
nc_put_vars_short(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const short *op);

 extern int
nc_get_vars_short(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  short *ip);

 extern int
nc_put_vars_int(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const int *op);

 extern int
nc_get_vars_int(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        int *ip);

 extern int
nc_put_vars_long(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const long *op);

 extern int
nc_get_vars_long(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        long *ip);

 extern int
nc_put_vars_float(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const float *op);

 extern int
nc_get_vars_float(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        float *ip);

 extern int
nc_put_vars_double(int ncid, int varid,
        const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
        const double *op);

 extern int
nc_get_vars_double(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   double *ip);

 extern int
nc_put_vars_ushort(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const unsigned short *op);

 extern int
nc_get_vars_ushort(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   unsigned short *ip);

 extern int
nc_put_vars_uint(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const unsigned int *op);

 extern int
nc_get_vars_uint(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 unsigned int *ip);

 extern int
nc_put_vars_longlong(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const long long *op);

 extern int
nc_get_vars_longlong(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  long long *ip);

 extern int
nc_put_vars_ulonglong(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const unsigned long long *op);

 extern int
nc_get_vars_ulonglong(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   unsigned long long *ip);

 extern int
nc_put_vars_string(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const char **op);

 extern int
nc_get_vars_string(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   char **ip);




 extern int
nc_put_varm_text(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const ptrdiff_t *imapp, const char *op);

 extern int
nc_get_varm_text(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const ptrdiff_t *imapp, char *ip);

 extern int
nc_put_varm_uchar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, const unsigned char *op);

 extern int
nc_get_varm_uchar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, unsigned char *ip);

 extern int
nc_put_varm_schar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, const signed char *op);

 extern int
nc_get_varm_schar(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, signed char *ip);

 extern int
nc_put_varm_short(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, const short *op);

 extern int
nc_get_varm_short(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, short *ip);

 extern int
nc_put_varm_int(int ncid, int varid, const size_t *startp,
                const size_t *countp, const ptrdiff_t *stridep,
                const ptrdiff_t *imapp, const int *op);

 extern int
nc_get_varm_int(int ncid, int varid, const size_t *startp,
                const size_t *countp, const ptrdiff_t *stridep,
                const ptrdiff_t *imapp, int *ip);

 extern int
nc_put_varm_long(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const ptrdiff_t *imapp, const long *op);

 extern int
nc_get_varm_long(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const ptrdiff_t *imapp, long *ip);

 extern int
nc_put_varm_float(int ncid, int varid,const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, const float *op);

 extern int
nc_get_varm_float(int ncid, int varid,const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t *imapp, float *ip);

 extern int
nc_put_varm_double(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t *imapp, const double *op);

 extern int
nc_get_varm_double(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, double *ip);

 extern int
nc_put_varm_ushort(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, const unsigned short *op);

 extern int
nc_get_varm_ushort(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, unsigned short *ip);

 extern int
nc_put_varm_uint(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const ptrdiff_t * imapp, const unsigned int *op);

 extern int
nc_get_varm_uint(int ncid, int varid, const size_t *startp,
                 const size_t *countp, const ptrdiff_t *stridep,
                 const ptrdiff_t * imapp, unsigned int *ip);

 extern int
nc_put_varm_longlong(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t * imapp, const long long *op);

 extern int
nc_get_varm_longlong(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t * imapp, long long *ip);

 extern int
nc_put_varm_ulonglong(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, const unsigned long long *op);

 extern int
nc_get_varm_ulonglong(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, unsigned long long *ip);

 extern int
nc_put_varm_string(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, const char **op);

 extern int
nc_get_varm_string(int ncid, int varid, const size_t *startp,
                   const size_t *countp, const ptrdiff_t *stridep,
                   const ptrdiff_t * imapp, char **ip);




 extern int
nc_put_var_text(int ncid, int varid, const char *op);

 extern int
nc_get_var_text(int ncid, int varid, char *ip);

 extern int
nc_put_var_uchar(int ncid, int varid, const unsigned char *op);

 extern int
nc_get_var_uchar(int ncid, int varid, unsigned char *ip);

 extern int
nc_put_var_schar(int ncid, int varid, const signed char *op);

 extern int
nc_get_var_schar(int ncid, int varid, signed char *ip);

 extern int
nc_put_var_short(int ncid, int varid, const short *op);

 extern int
nc_get_var_short(int ncid, int varid, short *ip);

 extern int
nc_put_var_int(int ncid, int varid, const int *op);

 extern int
nc_get_var_int(int ncid, int varid, int *ip);

 extern int
nc_put_var_long(int ncid, int varid, const long *op);

 extern int
nc_get_var_long(int ncid, int varid, long *ip);

 extern int
nc_put_var_float(int ncid, int varid, const float *op);

 extern int
nc_get_var_float(int ncid, int varid, float *ip);

 extern int
nc_put_var_double(int ncid, int varid, const double *op);

 extern int
nc_get_var_double(int ncid, int varid, double *ip);

 extern int
nc_put_var_ushort(int ncid, int varid, const unsigned short *op);

 extern int
nc_get_var_ushort(int ncid, int varid, unsigned short *ip);

 extern int
nc_put_var_uint(int ncid, int varid, const unsigned int *op);

 extern int
nc_get_var_uint(int ncid, int varid, unsigned int *ip);

 extern int
nc_put_var_longlong(int ncid, int varid, const long long *op);

 extern int
nc_get_var_longlong(int ncid, int varid, long long *ip);

 extern int
nc_put_var_ulonglong(int ncid, int varid, const unsigned long long *op);

 extern int
nc_get_var_ulonglong(int ncid, int varid, unsigned long long *ip);

 extern int
nc_put_var_string(int ncid, int varid, const char **op);

 extern int
nc_get_var_string(int ncid, int varid, char **ip);


 extern int
nc_put_att_ubyte(int ncid, int varid, const char *name, nc_type xtype,
                 size_t len, const unsigned char *op);
 extern int
nc_get_att_ubyte(int ncid, int varid, const char *name,
                 unsigned char *ip);
 extern int
nc_put_var1_ubyte(int ncid, int varid, const size_t *indexp,
                  const unsigned char *op);
 extern int
nc_get_var1_ubyte(int ncid, int varid, const size_t *indexp,
                  unsigned char *ip);
 extern int
nc_put_vara_ubyte(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const unsigned char *op);
 extern int
nc_get_vara_ubyte(int ncid, int varid, const size_t *startp,
                  const size_t *countp, unsigned char *ip);
 extern int
nc_put_vars_ubyte(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const unsigned char *op);
 extern int
nc_get_vars_ubyte(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  unsigned char *ip);
 extern int
nc_put_varm_ubyte(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t * imapp, const unsigned char *op);
 extern int
nc_get_varm_ubyte(int ncid, int varid, const size_t *startp,
                  const size_t *countp, const ptrdiff_t *stridep,
                  const ptrdiff_t * imapp, unsigned char *ip);
 extern int
nc_put_var_ubyte(int ncid, int varid, const unsigned char *op);
 extern int
nc_get_var_ubyte(int ncid, int varid, unsigned char *ip);




 extern int
nc_set_log_level(int new_level);






 extern int
nc_show_metadata(int ncid);




 extern int
nc_delete(const char *path);
# 1796 "/usr/include/netcdf.h" 3 4
 extern int
nc__create_mp(const char *path, int cmode, size_t initialsz, int basepe,
         size_t *chunksizehintp, int *ncidp);

 extern int
nc__open_mp(const char *path, int mode, int basepe,
        size_t *chunksizehintp, int *ncidp);

 extern int
nc_delete_mp(const char *path, int basepe);

 extern int
nc_set_base_pe(int ncid, int pe);

 extern int
nc_inq_base_pe(int ncid, int *pe);


 extern int
nctypelen(nc_type datatype);
# 1839 "/usr/include/netcdf.h" 3 4
 extern int ncerr;
# 1853 "/usr/include/netcdf.h" 3 4
 extern int ncopts;

 extern void
nc_advise(const char *cdf_routine_name, int err, const char *fmt,...);






typedef int nclong;

 extern int
nccreate(const char* path, int cmode);

 extern int
ncopen(const char* path, int mode);

 extern int
ncsetfill(int ncid, int fillmode);

 extern int
ncredef(int ncid);

 extern int
ncendef(int ncid);

 extern int
ncsync(int ncid);

 extern int
ncabort(int ncid);

 extern int
ncclose(int ncid);

 extern int
ncinquire(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimp);

 extern int
ncdimdef(int ncid, const char *name, long len);

 extern int
ncdimid(int ncid, const char *name);

 extern int
ncdiminq(int ncid, int dimid, char *name, long *lenp);

 extern int
ncdimrename(int ncid, int dimid, const char *name);

 extern int
ncattput(int ncid, int varid, const char *name, nc_type xtype,
        int len, const void *op);

 extern int
ncattinq(int ncid, int varid, const char *name, nc_type *xtypep, int *lenp);

 extern int
ncattget(int ncid, int varid, const char *name, void *ip);

 extern int
ncattcopy(int ncid_in, int varid_in, const char *name, int ncid_out,
        int varid_out);

 extern int
ncattname(int ncid, int varid, int attnum, char *name);

 extern int
ncattrename(int ncid, int varid, const char *name, const char *newname);

 extern int
ncattdel(int ncid, int varid, const char *name);

 extern int
ncvardef(int ncid, const char *name, nc_type xtype,
        int ndims, const int *dimidsp);

 extern int
ncvarid(int ncid, const char *name);

 extern int
ncvarinq(int ncid, int varid, char *name, nc_type *xtypep,
        int *ndimsp, int *dimidsp, int *nattsp);

 extern int
ncvarput1(int ncid, int varid, const long *indexp, const void *op);

 extern int
ncvarget1(int ncid, int varid, const long *indexp, void *ip);

 extern int
ncvarput(int ncid, int varid, const long *startp, const long *countp,
        const void *op);

 extern int
ncvarget(int ncid, int varid, const long *startp, const long *countp,
        void *ip);

 extern int
ncvarputs(int ncid, int varid, const long *startp, const long *countp,
        const long *stridep, const void *op);

 extern int
ncvargets(int ncid, int varid, const long *startp, const long *countp,
        const long *stridep, void *ip);

 extern int
ncvarputg(int ncid, int varid, const long *startp, const long *countp,
        const long *stridep, const long *imapp, const void *op);

 extern int
ncvargetg(int ncid, int varid, const long *startp, const long *countp,
        const long *stridep, const long *imapp, void *ip);

 extern int
ncvarrename(int ncid, int varid, const char *name);

 extern int
ncrecinq(int ncid, int *nrecvarsp, int *recvaridsp, long *recsizesp);

 extern int
ncrecget(int ncid, long recnum, void **datap);

 extern int
ncrecput(int ncid, long recnum, void *const *datap);




 extern int nc_initialize(void);





 extern int nc_finalize(void);


}
# 49 "../include/Ncxx/Nc3Values.hh" 2





# 53 "../include/Ncxx/Nc3Values.hh"
typedef signed char ncbyte;
typedef unsigned short int ushort;
# 67 "../include/Ncxx/Nc3Values.hh"
enum Nc3Type
  {
    nc3NoType = ((nc_type)0),
    nc3Byte = 
# 70 "../include/Ncxx/Nc3Values.hh" 3 4
             1
# 70 "../include/Ncxx/Nc3Values.hh"
                    ,
    nc3Char = 
# 71 "../include/Ncxx/Nc3Values.hh" 3 4
             2
# 71 "../include/Ncxx/Nc3Values.hh"
                    ,
    nc3Short = 
# 72 "../include/Ncxx/Nc3Values.hh" 3 4
              3
# 72 "../include/Ncxx/Nc3Values.hh"
                      ,
    nc3Ushort = 
# 73 "../include/Ncxx/Nc3Values.hh" 3 4
               8
# 73 "../include/Ncxx/Nc3Values.hh"
                        ,
    nc3Int = 
# 74 "../include/Ncxx/Nc3Values.hh" 3 4
            4
# 74 "../include/Ncxx/Nc3Values.hh"
                  ,
    nc3Long = 
# 75 "../include/Ncxx/Nc3Values.hh" 3 4
             4
# 75 "../include/Ncxx/Nc3Values.hh"
                    ,
    nc3Float = 
# 76 "../include/Ncxx/Nc3Values.hh" 3 4
              5
# 76 "../include/Ncxx/Nc3Values.hh"
                      ,
    nc3Double = 
# 77 "../include/Ncxx/Nc3Values.hh" 3 4
               6
# 77 "../include/Ncxx/Nc3Values.hh"
                        ,
    nc3Int64 = 
# 78 "../include/Ncxx/Nc3Values.hh" 3 4
              10
  
# 79 "../include/Ncxx/Nc3Values.hh"
 };


static const ncbyte nc3Bad_byte = 
# 82 "../include/Ncxx/Nc3Values.hh" 3 4
                                 ((signed char)-127)
# 82 "../include/Ncxx/Nc3Values.hh"
                                             ;
static const char nc3Bad_char = 
# 83 "../include/Ncxx/Nc3Values.hh" 3 4
                               ((char)0)
# 83 "../include/Ncxx/Nc3Values.hh"
                                           ;
static const short nc3Bad_short = 
# 84 "../include/Ncxx/Nc3Values.hh" 3 4
                                 ((short)-32767)
# 84 "../include/Ncxx/Nc3Values.hh"
                                              ;
static const ushort nc3Bad_ushort = 
# 85 "../include/Ncxx/Nc3Values.hh" 3 4
                                   (65535)
# 85 "../include/Ncxx/Nc3Values.hh"
                                                 ;
static const nclong nc3Bad_nclong = 
# 86 "../include/Ncxx/Nc3Values.hh" 3 4
                                   (-2147483647)
# 86 "../include/Ncxx/Nc3Values.hh"
                                            ;
static const int nc3Bad_int = 
# 87 "../include/Ncxx/Nc3Values.hh" 3 4
                             (-2147483647)
# 87 "../include/Ncxx/Nc3Values.hh"
                                        ;
static const long nc3Bad_long = 
# 88 "../include/Ncxx/Nc3Values.hh" 3 4
                               (-2147483647)
# 88 "../include/Ncxx/Nc3Values.hh"
                                        ;
static const float nc3Bad_float = 
# 89 "../include/Ncxx/Nc3Values.hh" 3 4
                                 (9.9692099683868690e+36f)
# 89 "../include/Ncxx/Nc3Values.hh"
                                              ;
static const double nc3Bad_double = 
# 90 "../include/Ncxx/Nc3Values.hh" 3 4
                                   (9.9692099683868690e+36)
# 90 "../include/Ncxx/Nc3Values.hh"
                                                 ;
# 298 "../include/Ncxx/Nc3Values.hh"
class Nc3Values
{
public:
  Nc3Values( void );
  Nc3Values(Nc3Type, long);
  virtual ~Nc3Values( void );
  virtual long num( void );
  virtual std::ostream& print(std::ostream&) const = 0;
  virtual void* base( void ) const = 0;
  virtual int bytes_for_one( void ) const = 0;




  virtual ncbyte as_ncbyte( long n ) const = 0;
  virtual char as_char( long n ) const = 0;
  virtual short as_short( long n ) const = 0;
  virtual ushort as_ushort( long n ) const = 0;
  virtual int as_int( long n ) const = 0;
  virtual int64_t as_int64( long n ) const = 0;
  virtual int as_nclong( long n ) const = 0;
  virtual long as_long( long n ) const = 0;
  virtual float as_float( long n ) const = 0;
  virtual double as_double( long n ) const = 0;
  virtual char* as_string( long n ) const = 0;

protected:
  Nc3Type the_type;
  long the_number;
  friend std::ostream& operator<< (std::ostream&, const Nc3Values&);
};

class Nc3Values_ncbyte : public Nc3Values { public: Nc3Values_ncbyte( void ); Nc3Values_ncbyte(long num); Nc3Values_ncbyte(long num, const ncbyte* vals); Nc3Values_ncbyte(const Nc3Values_ncbyte&); virtual Nc3Values_ncbyte& operator=(const Nc3Values_ncbyte&); virtual ~Nc3Values_ncbyte( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: ncbyte* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_char : public Nc3Values { public: Nc3Values_char( void ); Nc3Values_char(long num); Nc3Values_char(long num, const char* vals); Nc3Values_char(const Nc3Values_char&); virtual Nc3Values_char& operator=(const Nc3Values_char&); virtual ~Nc3Values_char( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: char* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_short : public Nc3Values { public: Nc3Values_short( void ); Nc3Values_short(long num); Nc3Values_short(long num, const short* vals); Nc3Values_short(const Nc3Values_short&); virtual Nc3Values_short& operator=(const Nc3Values_short&); virtual ~Nc3Values_short( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: short* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_ushort : public Nc3Values { public: Nc3Values_ushort( void ); Nc3Values_ushort(long num); Nc3Values_ushort(long num, const ushort* vals); Nc3Values_ushort(const Nc3Values_ushort&); virtual Nc3Values_ushort& operator=(const Nc3Values_ushort&); virtual ~Nc3Values_ushort( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: ushort* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_int : public Nc3Values { public: Nc3Values_int( void ); Nc3Values_int(long num); Nc3Values_int(long num, const int* vals); Nc3Values_int(const Nc3Values_int&); virtual Nc3Values_int& operator=(const Nc3Values_int&); virtual ~Nc3Values_int( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: int* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_nclong : public Nc3Values { public: Nc3Values_nclong( void ); Nc3Values_nclong(long num); Nc3Values_nclong(long num, const nclong* vals); Nc3Values_nclong(const Nc3Values_nclong&); virtual Nc3Values_nclong& operator=(const Nc3Values_nclong&); virtual ~Nc3Values_nclong( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: nclong* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_long : public Nc3Values { public: Nc3Values_long( void ); Nc3Values_long(long num); Nc3Values_long(long num, const long* vals); Nc3Values_long(const Nc3Values_long&); virtual Nc3Values_long& operator=(const Nc3Values_long&); virtual ~Nc3Values_long( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: long* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_int64_t : public Nc3Values { public: Nc3Values_int64_t( void ); Nc3Values_int64_t(long num); Nc3Values_int64_t(long num, const int64_t* vals); Nc3Values_int64_t(const Nc3Values_int64_t&); virtual Nc3Values_int64_t& operator=(const Nc3Values_int64_t&); virtual ~Nc3Values_int64_t( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: int64_t* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_float : public Nc3Values { public: Nc3Values_float( void ); Nc3Values_float(long num); Nc3Values_float(long num, const float* vals); Nc3Values_float(const Nc3Values_float&); virtual Nc3Values_float& operator=(const Nc3Values_float&); virtual ~Nc3Values_float( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: float* the_values; std::ostream& print(std::ostream&) const; };
class Nc3Values_double : public Nc3Values { public: Nc3Values_double( void ); Nc3Values_double(long num); Nc3Values_double(long num, const double* vals); Nc3Values_double(const Nc3Values_double&); virtual Nc3Values_double& operator=(const Nc3Values_double&); virtual ~Nc3Values_double( void ); virtual void* base( void ) const; virtual int bytes_for_one( void ) const; virtual ncbyte as_ncbyte( long n ) const; virtual char as_char( long n ) const; virtual short as_short( long n ) const; virtual ushort as_ushort( long n ) const; virtual int as_int( long n ) const; virtual int as_nclong( long n ) const; virtual long as_long( long n ) const; virtual int64_t as_int64( long n ) const; virtual float as_float( long n ) const; virtual double as_double( long n ) const; virtual char* as_string( long n ) const; virtual int invalid( void ) const; private: double* the_values; std::ostream& print(std::ostream&) const; };
# 46 "Nc3Values.cc" 2

Nc3Values::Nc3Values( void ) : the_type(nc3NoType), the_number(0)
{}

Nc3Values::Nc3Values(Nc3Type type, long num)
 : the_type(type), the_number(num)
{}

Nc3Values::~Nc3Values( void )
{}

long Nc3Values::num( void )
{
    return the_number;
}

std::ostream& operator<< (std::ostream& os, const Nc3Values& vals)
{
    return vals.print(os);
}

Nc3Values_ncbyte::Nc3Values_ncbyte( void ) : Nc3Values(nc3Byte, 0), the_values(0) {} Nc3Values_ncbyte::Nc3Values_ncbyte(long num, const ncbyte* vals) : Nc3Values(nc3Byte, num) { the_values = new ncbyte[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_ncbyte::Nc3Values_ncbyte(long num) : Nc3Values(nc3Byte, num), the_values(new ncbyte[num]) {} Nc3Values_ncbyte::Nc3Values_ncbyte(const Nc3Values_ncbyte& v) : Nc3Values(v) { delete[] the_values; the_values = new ncbyte[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_ncbyte& Nc3Values_ncbyte::operator=(const Nc3Values_ncbyte& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new ncbyte[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_ncbyte::base( void ) const { return the_values; } Nc3Values_ncbyte::~Nc3Values_ncbyte( void ) { delete[] the_values; } int Nc3Values_ncbyte::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_byte) return 1; return 0; }
Nc3Values_char::Nc3Values_char( void ) : Nc3Values(nc3Char, 0), the_values(0) {} Nc3Values_char::Nc3Values_char(long num, const char* vals) : Nc3Values(nc3Char, num) { the_values = new char[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_char::Nc3Values_char(long num) : Nc3Values(nc3Char, num), the_values(new char[num]) {} Nc3Values_char::Nc3Values_char(const Nc3Values_char& v) : Nc3Values(v) { delete[] the_values; the_values = new char[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_char& Nc3Values_char::operator=(const Nc3Values_char& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new char[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_char::base( void ) const { return the_values; } Nc3Values_char::~Nc3Values_char( void ) { delete[] the_values; } int Nc3Values_char::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_char) return 1; return 0; }
Nc3Values_short::Nc3Values_short( void ) : Nc3Values(nc3Short, 0), the_values(0) {} Nc3Values_short::Nc3Values_short(long num, const short* vals) : Nc3Values(nc3Short, num) { the_values = new short[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_short::Nc3Values_short(long num) : Nc3Values(nc3Short, num), the_values(new short[num]) {} Nc3Values_short::Nc3Values_short(const Nc3Values_short& v) : Nc3Values(v) { delete[] the_values; the_values = new short[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_short& Nc3Values_short::operator=(const Nc3Values_short& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new short[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_short::base( void ) const { return the_values; } Nc3Values_short::~Nc3Values_short( void ) { delete[] the_values; } int Nc3Values_short::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_short) return 1; return 0; }
Nc3Values_ushort::Nc3Values_ushort( void ) : Nc3Values(nc3Ushort, 0), the_values(0) {} Nc3Values_ushort::Nc3Values_ushort(long num, const ushort* vals) : Nc3Values(nc3Ushort, num) { the_values = new ushort[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_ushort::Nc3Values_ushort(long num) : Nc3Values(nc3Ushort, num), the_values(new ushort[num]) {} Nc3Values_ushort::Nc3Values_ushort(const Nc3Values_ushort& v) : Nc3Values(v) { delete[] the_values; the_values = new ushort[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_ushort& Nc3Values_ushort::operator=(const Nc3Values_ushort& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new ushort[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_ushort::base( void ) const { return the_values; } Nc3Values_ushort::~Nc3Values_ushort( void ) { delete[] the_values; } int Nc3Values_ushort::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_ushort) return 1; return 0; }
Nc3Values_int::Nc3Values_int( void ) : Nc3Values(nc3Int, 0), the_values(0) {} Nc3Values_int::Nc3Values_int(long num, const int* vals) : Nc3Values(nc3Int, num) { the_values = new int[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_int::Nc3Values_int(long num) : Nc3Values(nc3Int, num), the_values(new int[num]) {} Nc3Values_int::Nc3Values_int(const Nc3Values_int& v) : Nc3Values(v) { delete[] the_values; the_values = new int[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_int& Nc3Values_int::operator=(const Nc3Values_int& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new int[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_int::base( void ) const { return the_values; } Nc3Values_int::~Nc3Values_int( void ) { delete[] the_values; } int Nc3Values_int::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_int) return 1; return 0; }
Nc3Values_nclong::Nc3Values_nclong( void ) : Nc3Values(nc3Long, 0), the_values(0) {} Nc3Values_nclong::Nc3Values_nclong(long num, const nclong* vals) : Nc3Values(nc3Long, num) { the_values = new nclong[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_nclong::Nc3Values_nclong(long num) : Nc3Values(nc3Long, num), the_values(new nclong[num]) {} Nc3Values_nclong::Nc3Values_nclong(const Nc3Values_nclong& v) : Nc3Values(v) { delete[] the_values; the_values = new nclong[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_nclong& Nc3Values_nclong::operator=(const Nc3Values_nclong& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new nclong[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_nclong::base( void ) const { return the_values; } Nc3Values_nclong::~Nc3Values_nclong( void ) { delete[] the_values; } int Nc3Values_nclong::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_nclong) return 1; return 0; }
Nc3Values_long::Nc3Values_long( void ) : Nc3Values(nc3Long, 0), the_values(0) {} Nc3Values_long::Nc3Values_long(long num, const long* vals) : Nc3Values(nc3Long, num) { the_values = new long[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_long::Nc3Values_long(long num) : Nc3Values(nc3Long, num), the_values(new long[num]) {} Nc3Values_long::Nc3Values_long(const Nc3Values_long& v) : Nc3Values(v) { delete[] the_values; the_values = new long[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_long& Nc3Values_long::operator=(const Nc3Values_long& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new long[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_long::base( void ) const { return the_values; } Nc3Values_long::~Nc3Values_long( void ) { delete[] the_values; } int Nc3Values_long::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_long) return 1; return 0; }
Nc3Values_float::Nc3Values_float( void ) : Nc3Values(nc3Float, 0), the_values(0) {} Nc3Values_float::Nc3Values_float(long num, const float* vals) : Nc3Values(nc3Float, num) { the_values = new float[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_float::Nc3Values_float(long num) : Nc3Values(nc3Float, num), the_values(new float[num]) {} Nc3Values_float::Nc3Values_float(const Nc3Values_float& v) : Nc3Values(v) { delete[] the_values; the_values = new float[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_float& Nc3Values_float::operator=(const Nc3Values_float& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new float[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_float::base( void ) const { return the_values; } Nc3Values_float::~Nc3Values_float( void ) { delete[] the_values; } int Nc3Values_float::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_float) return 1; return 0; }
Nc3Values_double::Nc3Values_double( void ) : Nc3Values(nc3Double, 0), the_values(0) {} Nc3Values_double::Nc3Values_double(long num, const double* vals) : Nc3Values(nc3Double, num) { the_values = new double[num]; for(int i = 0; i < num; i++) the_values[i] = vals[i]; } Nc3Values_double::Nc3Values_double(long num) : Nc3Values(nc3Double, num), the_values(new double[num]) {} Nc3Values_double::Nc3Values_double(const Nc3Values_double& v) : Nc3Values(v) { delete[] the_values; the_values = new double[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } Nc3Values_double& Nc3Values_double::operator=(const Nc3Values_double& v) { if ( &v != this) { Nc3Values::operator=(v); delete[] the_values; the_values = new double[v.the_number]; for(int i = 0; i < v.the_number; i++) the_values[i] = v.the_values[i]; } return *this; } void* Nc3Values_double::base( void ) const { return the_values; } Nc3Values_double::~Nc3Values_double( void ) { delete[] the_values; } int Nc3Values_double::invalid( void ) const { for(int i=0;i<the_number;i++) if (the_values[i] == nc3Bad_double) return 1; return 0; }

int Nc3Values_ncbyte::bytes_for_one( void ) const { return sizeof(ncbyte); }
int Nc3Values_char::bytes_for_one( void ) const { return sizeof(char); }
int Nc3Values_short::bytes_for_one( void ) const { return sizeof(short); }
int Nc3Values_ushort::bytes_for_one( void ) const { return sizeof(ushort); }
int Nc3Values_int::bytes_for_one( void ) const { return sizeof(int); }
int Nc3Values_nclong::bytes_for_one( void ) const { return sizeof(nclong); }
int Nc3Values_long::bytes_for_one( void ) const { return sizeof(long); }
int Nc3Values_float::bytes_for_one( void ) const { return sizeof(float); }
int Nc3Values_double::bytes_for_one( void ) const { return sizeof(double); }

ncbyte Nc3Values_short::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 87 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 87 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_ushort::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 88 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 88 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_int::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 89 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 89 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_nclong::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 90 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 90 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_long::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 91 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 91 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_int64_t::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 92 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 92 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_float::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 93 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 93 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }
ncbyte Nc3Values_double::as_ncbyte( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 94 "Nc3Values.cc" 3 4
(0x7f * 2 + 1)
# 94 "Nc3Values.cc"
) return nc3Bad_byte; return (ncbyte) the_values[n]; }

inline ncbyte Nc3Values_char::as_ncbyte( long n ) const
{
    return the_values[n];
}

inline ncbyte Nc3Values_ncbyte::as_ncbyte( long n ) const
{
    return the_values[n];
}

char Nc3Values_short::as_char( long n ) const { if (the_values[n] < 
# 106 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 106 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_ushort::as_char( long n ) const { if (the_values[n] < 
# 107 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 107 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_int::as_char( long n ) const { if (the_values[n] < 
# 108 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 108 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_nclong::as_char( long n ) const { if (the_values[n] < 
# 109 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 109 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_long::as_char( long n ) const { if (the_values[n] < 
# 110 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 110 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_int64_t::as_char( long n ) const { if (the_values[n] < 
# 111 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 111 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_float::as_char( long n ) const { if (the_values[n] < 
# 112 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 112 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }
char Nc3Values_double::as_char( long n ) const { if (the_values[n] < 
# 113 "Nc3Values.cc" 3 4
(-0x7f - 1) 
# 113 "Nc3Values.cc"
|| the_values[n] > 0x7f) return nc3Bad_char; return (char) the_values[n]; }

inline char Nc3Values_ncbyte::as_char( long n ) const
{
    return the_values[n] > 0x7f ? nc3Bad_char : (char) the_values[n];
}

inline char Nc3Values_char::as_char( long n ) const
{
    return the_values[n];
}

short Nc3Values_ushort::as_short( long n ) const { if (the_values[n] < 
# 125 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 125 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }
short Nc3Values_int::as_short( long n ) const { if (the_values[n] < 
# 126 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 126 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }
short Nc3Values_nclong::as_short( long n ) const { if (the_values[n] < 
# 127 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 127 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }
short Nc3Values_long::as_short( long n ) const { if (the_values[n] < 
# 128 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 128 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }
short Nc3Values_int64_t::as_short( long n ) const { if (the_values[n] < 
# 129 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 129 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }
short Nc3Values_float::as_short( long n ) const { if (the_values[n] < 
# 130 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 130 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }
short Nc3Values_double::as_short( long n ) const { if (the_values[n] < 
# 131 "Nc3Values.cc" 3 4
(-0x7fff - 1) 
# 131 "Nc3Values.cc"
|| the_values[n] > 0x7fff) return nc3Bad_short; return (short) the_values[n]; }

inline short Nc3Values_ncbyte::as_short( long n ) const
{
    return the_values[n];
}

inline short Nc3Values_char::as_short( long n ) const
{
    return the_values[n];
}

inline short Nc3Values_short::as_short( long n ) const
{
    return the_values[n];
}

ushort Nc3Values_short::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 148 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 148 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }
ushort Nc3Values_int::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 149 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 149 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }
ushort Nc3Values_nclong::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 150 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 150 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }
ushort Nc3Values_long::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 151 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 151 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }
ushort Nc3Values_int64_t::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 152 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 152 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }
ushort Nc3Values_float::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 153 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 153 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }
ushort Nc3Values_double::as_ushort( long n ) const { if (the_values[n] < 0 || the_values[n] > 
# 154 "Nc3Values.cc" 3 4
(0x7fff * 2 + 1)
# 154 "Nc3Values.cc"
) return nc3Bad_ushort; return (ushort) the_values[n]; }

inline ushort Nc3Values_ncbyte::as_ushort( long n ) const
{
    return the_values[n];
}

inline ushort Nc3Values_char::as_ushort( long n ) const
{
    return the_values[n];
}

inline ushort Nc3Values_ushort::as_ushort( long n ) const
{
    return the_values[n];
}

int Nc3Values_float::as_int( long n ) const { if (the_values[n] < 
# 171 "Nc3Values.cc" 3 4
(-0x7fffffff - 1) 
# 171 "Nc3Values.cc"
|| the_values[n] > 0x7fffffff) return nc3Bad_int; return (int) the_values[n]; }
int Nc3Values_double::as_int( long n ) const { if (the_values[n] < 
# 172 "Nc3Values.cc" 3 4
(-0x7fffffff - 1) 
# 172 "Nc3Values.cc"
|| the_values[n] > 0x7fffffff) return nc3Bad_int; return (int) the_values[n]; }

inline int Nc3Values_ncbyte::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_char::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_short::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_ushort::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_int::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_nclong::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_long::as_int( long n ) const
{
    return the_values[n];
}

inline int Nc3Values_int64_t::as_int( long n ) const
{
    return the_values[n];
}

nclong Nc3Values_float::as_nclong( long n ) const { if (the_values[n] < 
# 214 "Nc3Values.cc" 3 4
(-0x7fffffff - 1) 
# 214 "Nc3Values.cc"
|| the_values[n] > 0x7fffffff) return nc3Bad_nclong; return (nclong) the_values[n]; }
nclong Nc3Values_double::as_nclong( long n ) const { if (the_values[n] < 
# 215 "Nc3Values.cc" 3 4
(-0x7fffffff - 1) 
# 215 "Nc3Values.cc"
|| the_values[n] > 0x7fffffff) return nc3Bad_nclong; return (nclong) the_values[n]; }

inline nclong Nc3Values_ncbyte::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_char::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_short::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_ushort::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_int::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_nclong::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_long::as_nclong( long n ) const
{
    return the_values[n];
}

inline nclong Nc3Values_int64_t::as_nclong( long n ) const
{
    return the_values[n];
}

long Nc3Values_float::as_long( long n ) const { if (the_values[n] < 
# 257 "Nc3Values.cc" 3 4
(-0x7fffffffffffffffL - 1L) 
# 257 "Nc3Values.cc"
|| the_values[n] > 0x7fffffffffffffffL) return nc3Bad_long; return (long) the_values[n]; }
long Nc3Values_double::as_long( long n ) const { if (the_values[n] < 
# 258 "Nc3Values.cc" 3 4
(-0x7fffffffffffffffL - 1L) 
# 258 "Nc3Values.cc"
|| the_values[n] > 0x7fffffffffffffffL) return nc3Bad_long; return (long) the_values[n]; }

inline long Nc3Values_ncbyte::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_char::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_short::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_ushort::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_int::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_nclong::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_long::as_long( long n ) const
{
    return the_values[n];
}

inline long Nc3Values_int64_t::as_long( long n ) const
{
    return the_values[n];
}


int64_t Nc3Values_float::as_int64( long n ) const { if (the_values[n] < 
# 301 "Nc3Values.cc" 3 4
(-0x7fffffffffffffffLL -1) 
# 301 "Nc3Values.cc"
|| the_values[n] > 0x7fffffffffffffffLL) return nc3Bad_int; return (int64_t) the_values[n]; }
int64_t Nc3Values_double::as_int64( long n ) const { if (the_values[n] < 
# 302 "Nc3Values.cc" 3 4
(-0x7fffffffffffffffLL -1) 
# 302 "Nc3Values.cc"
|| the_values[n] > 0x7fffffffffffffffLL) return nc3Bad_int; return (int64_t) the_values[n]; }

inline int64_t Nc3Values_ncbyte::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_char::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_short::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_ushort::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_int::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_nclong::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_long::as_int64( long n ) const
{
    return the_values[n];
}

inline int64_t Nc3Values_int64_t::as_int64( long n ) const
{
    return the_values[n];
}

inline float Nc3Values_ncbyte::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_char::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_short::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_ushort::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_int::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_nclong::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_long::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_int64_t::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_float::as_float( long n ) const { return (float) the_values[n]; }
inline float Nc3Values_double::as_float( long n ) const { return (float) the_values[n]; }

inline double Nc3Values_ncbyte::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_char::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_short::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_ushort::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_int::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_nclong::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_long::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_int64_t::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_float::as_double( long n ) const { return (double) the_values[n]; }
inline double Nc3Values_double::as_double( long n ) const { return (double) the_values[n]; }

char* Nc3Values_short::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_ushort::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_int::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_nclong::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_long::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_int64_t::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_float::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }
char* Nc3Values_double::as_string( long n ) const { char* s = new char[32]; std::ostringstream ostr; ostr << the_values[n]; ostr.str().copy(s, std::string::npos); s[ostr.str().length()] = 0; return s; }

inline char* Nc3Values_ncbyte::as_string( long n ) const
{
    char* s = new char[the_number + 1];
    s[the_number] = '\0';
    strncpy(s, (const char*)the_values + n, (int)the_number);
    return s;
}

inline char* Nc3Values_char::as_string( long n ) const
{
    char* s = new char[the_number + 1];
    s[the_number] = '\0';
    strncpy(s, (const char*)the_values + n, (int)the_number);
    return s;
}

std::ostream& Nc3Values_short::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_ushort::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_int::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_nclong::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_long::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_ncbyte::print(std::ostream& os) const
{
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    return os;
}

std::ostream& Nc3Values_char::print(std::ostream& os) const
{
    os << '"';
    long len = the_number;
    while (the_values[--len] == '\0')
 ;
    for(int i = 0; i <= len; i++)
 os << the_values[i] ;
    os << '"';

    return os;
}

std::ostream& Nc3Values_float::print(std::ostream& os) const
{
    std::streamsize save=os.precision();
    os.precision(7);
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1] ;
    os.precision(save);
    return os;
}

std::ostream& Nc3Values_double::print(std::ostream& os) const
{
    std::streamsize save=os.precision();
    os.precision(15);
    for(int i = 0; i < the_number - 1; i++)
      os << the_values[i] << ", ";
    if (the_number > 0)
      os << the_values[the_number-1];
    os.precision(save);
    return os;
}
