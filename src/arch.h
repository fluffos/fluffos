/* arch.h: __ARCH__ - a predefined macro exported to all LPC objects */

#ifndef _ARCH_H_
#define _ARCH_H_

#ifdef sequent
#define ARCH "Sequent DYNIX"
#endif

#ifdef _SEQUENT_
#define ARCH "Sequent DYNIX/ptx"
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

#if defined(sun) && !defined(SunOS_5)
#ifdef sun4
#define ARCH "Sun4"
#else

#ifdef sun3
#define ARCH "Sun3"
#else
#define ARCH "Sun"
#endif

#endif
#endif

#ifdef SunOS_5
#define ARCH "Solaris"
#endif
#ifdef __386BSD__
#define ARCH "386bsd"
#endif
#ifdef _AUX_SOURCE
#define ARCH "A/UX"
#endif
#ifdef linux
#define ARCH "Linux"
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
#ifdef __alpha
#define ARCH "Alpha"
#endif

#if !defined(ARCH) && defined(__bsdi__)
#define ARCH "BSDI"
#endif

#if !defined(ARCH) && defined(ultrix)
#define ARCH "Ultrix"
#endif

#if !defined(ARCH) && defined(hpux)
#define ARCH "HP/UX"
#endif

#ifndef ARCH_H
#define ARCH_H

#if !defined(ARCH) && defined(sgi)
#define ARCH "IRIX"
#endif

#if (!defined(ARCH) && defined(SVR4))
#define ARCH "SVR4"
#endif

#if !defined(ARCH) && defined(OSF)
#define ARCH "OSF/1"
#endif

#if !defined(ARCH) && defined(LATTICE)
#define ARCH "Amiga"
#endif

#ifndef ARCH
#define ARCH "stuf!"
#endif

#endif
#endif
