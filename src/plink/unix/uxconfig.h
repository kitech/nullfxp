/* uxconfig.h.  Generated from uxconfig.in by configure.  */
/* uxconfig.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `ptsname' function. */
#define HAVE_PTSNAME 1

/* Define to 1 if you have the `setresuid' function. */
#define HAVE_SETRESUID 1

/* Define to 1 if you have the `strsignal' function. */
#define HAVE_STRSIGNAL 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the `updwtmpx' function. */
#define HAVE_UPDWTMPX 1

/* Define to 1 if you have the <utmpx.h> header file. */
#define HAVE_UTMPX_H 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""


/* Convert autoconf definitions to ones that PuTTY wants. */

#ifndef HAVE_GETADDRINFO
# define NO_IPV6
#endif
#ifndef HAVE_SETRESUID
# define HAVE_NO_SETRESUID
#endif
#ifndef HAVE_STRSIGNAL
# define HAVE_NO_STRSIGNAL
#endif
#if !defined(HAVE_UTMPX_H) || !defined(HAVE_UPDWTMPX)
# define OMIT_UTMP
#endif
#ifndef HAVE_PTSNAME
# define BSD_PTYS
#endif
#ifndef HAVE_SYS_SELECT_H
# define HAVE_NO_SYS_SELECT_H
#endif

