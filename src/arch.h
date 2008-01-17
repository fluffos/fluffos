/* arch.h: __ARCH__ - a predefined macro exported to all LPC objects */

#ifndef _ARCH_H_
#define _ARCH_H_

#ifdef sequent
#define ARCH "Sequent DYNIX"
#endif

#ifdef WINNT
#define ARCH "Microsoft Windows NT"
#endif

#ifdef WIN95
#define ARCH "Microsoft Windows 95"
#endif

#ifdef _SEQUENT_
#define ARCH "Sequent DYNIX/ptx"
#endif

#ifdef __FreeBSD__
#define ARCH "FreeBSD"
#endif

#ifdef __NetBSD__
#ifdef sparc
#define ARCH "NetBSD/sparc"
#else
#define ARCH "NetBSD"
#endif
#endif

#ifdef NeXT
#ifdef m68k
#define ARCH "NeXT/68k"
#else
#define ARCH "NeXT"
#endif
#endif

#ifdef _AIX
#define ARCH "AIX"
#endif

#ifdef accel
#define ARCH "Accel"
#endif

/* Attempt to guess whether we are running Solaris or not */
#if defined(sun) && !defined(SunOS_5)
#  if defined(__svr4__) || defined(__sol__) || defined(SVR4)
#    define SunOS_5
#  else
#    ifdef sun4
#      define ARCH "Sun4"
#    else

#      ifdef sun3
#        define ARCH "Sun3"
#      else
#        define ARCH "Sun"
#      endif
#    endif
#  endif
#endif

#if defined(SunOS_5)
#define ARCH "Solaris"
#endif

#ifdef _AUX_SOURCE
#define ARCH "A/UX"
#endif

#ifdef linux
#  ifdef __mc68000
#    define ARCH "Linux/m68k"
#  else
#    ifdef __alpha
#      define ARCH "Linux/alpha"
#    else
#      define ARCH "Linux"
#    endif
#  endif
#endif

#ifdef hp68k
#define ARCH "HP/68k"
#endif

#ifdef hppa
#define ARCH "HP/PA-RISC"
#endif

#ifdef cray
#define ARCH "Cray"
#endif

#if !defined(ARCH) && defined(__alpha)
#define ARCH "Alpha"
#endif

#if !defined(ARCH) && defined(__bsdi__)
#define ARCH "BSDI"
#endif

#if !defined(ARCH) && defined(__386BSD__)
#define ARCH "386bsd"
#endif

#if !defined(ARCH) && defined(ultrix)
#define ARCH "Ultrix"
#endif

#if !defined(ARCH) && defined(hpux)
#define ARCH "HP/UX"
#endif

#if !defined(ARCH) && defined(sgi)
#define ARCH "IRIX"
#endif

#if (!defined(ARCH) && defined(SVR4))
#define ARCH "SVR4"
#endif

#if !defined(ARCH) && defined(OSF)
#define ARCH "OSF/1"
#endif

#if !defined(ARCH) && defined(__APPLE__) && defined(__GNUC__)
#define ARCH "Mac OS X"
#endif

#ifndef ARCH
#define ARCH "unknown architecture"
#endif

#endif
