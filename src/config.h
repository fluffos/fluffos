/* Minimal config.h for static analysis */
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define PROJECT_VERSION "FluffOS unknown"
#define COMPILER "Microsoft Visual C++"
#define CXXFLAGS "Minimal"

// Windows specific
#ifdef _WIN32
#define HAVE_TIME_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_SYS_STAT_H 1
#endif

#endif /* _CONFIG_H_ */
