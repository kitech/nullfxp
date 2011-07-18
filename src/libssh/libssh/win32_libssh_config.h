

/* Define to 1 if you have the `snprintf' function. */
// #define HAVE_SNPRINTF 1
// #define HAVE__SNPRINTF 1
#define HAVE__SNPRINTF_S 1


/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE__VSNPRINTF 1

/* Define to 1 if you have the `strncpy' function. */
#define HAVE_STRNCPY 1

/* Define to 1 if you have the `getaddrinfo' function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `crypto' library (-lcrypto). */
#define HAVE_LIBCRYPTO 1

/**************************** OPTIONS ****************************/

/* Define to 1 if you want to enable ZLIB */
#define WITH_LIBZ 1

/* Define to 1 if you want to enable SFTP */
#define WITH_SFTP 1

/* Define to 1 if you want to enable SSH1 */
/* #undef WITH_SSH1 */
#define WITH_SSH1 1

/* Define to 1 if you want to enable server support */
#define WITH_SERVER 1

/* Define to 1 if you want to enable debug output for crypto functions */
/* #undef DEBUG_CRYPTO */

/* Define to 1 if you want to enable pcap output support (experimental) */
#define WITH_PCAP 1

/* Define to 1 if you want to enable calltrace debug output */
#define DEBUG_CALLTRACE 1
