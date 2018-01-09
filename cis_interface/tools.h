#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

#ifndef CISTOOLS_H_
#define CISTOOLS_H_

// Platform specific
#ifdef _WIN32
#include <windows.h>
#include <regex>
#define getpid GetCurrentProcessId
#define sleep(tsec) Sleep(1000*tsec)
#define usleep(usec) Sleep(usec/1000)
#else
#include <unistd.h>
#include <regex.h>
#endif

/*! @brief Maximum message size. */
#ifdef IPCDEF
#define CIS_MSG_MAX 2048
#else
#define CIS_MSG_MAX 1048576
#endif
/*! @brief End of file message. */
#define CIS_MSG_EOF "EOF!!!"
/*! @brief Resonable size for buffer. */
#define CIS_MSG_BUF 2048

/*! @brief Define old style names for compatibility. */
#define PSI_MSG_MAX CIS_MSG_MAX
#define PSI_MSG_EOF CIS_MSG_MAX
#ifdef PSI_DEBUG
#define CIS_DEBUG PSI_DEBUG
#endif


//==============================================================================
/*!
  Logging

  Alliases are set at compile-time based on the value of CIS_CLIENT_DEBUG. If 
  set to INFO, only messages logged with info or error alias are printed. If
  set to DEBUG, messages logged with error, info or debug aliases are printed.
  Otherwise, only error messages are printed. If the CIS_CLIENT_DEBUG is
  changed, any code including this header must be recompiled for the change to
  take effect.

*/
//==============================================================================

/*!
  @brief Print a log message.
  Prints a formatted message, prepending it with the process id and appending
  it with a newline.
  @param[in] prefix a constant character pointer to the prefix that should
  preceed the message and process id.
  @param[in] fmt a constant character pointer to a format string.
  @param[in] ap va_list of arguments to be formatted in the format string.
 */
static inline
void cisLog(const char* prefix, const char* fmt, va_list ap) {
  fprintf(stdout, "%s: %d: ", prefix, getpid());
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
};

/*!
  @brief Print an info log message.
  Prints a formatted message, prepending it with INFO and the process id. A
  newline character is added to the end of the message.
  @param[in] fmt a constant character pointer to a format string.
  @param[in] ... arguments to be formatted in the format string.
 */
static inline
void cisInfo(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  cisLog("INFO", fmt, ap);
  va_end(ap);
};
  
/*!
  @brief Print an debug log message.
  Prints a formatted message, prepending it with DEBUG and the process id. A
  newline character is added to the end of the message.
  @param[in] fmt a constant character pointer to a format string.
  @param[in] ... arguments to be formatted in the format string.
 */
static inline
void cisDebug(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  cisLog("DEBUG", fmt, ap);
  va_end(ap);
};
  
/*!
  @brief Print an error log message.
  Prints a formatted message, prepending it with ERROR and the process id. A
  newline character is added to the end of the message.
  @param[in] fmt a constant character pointer to a format string.
  @param[in] ... arguments to be formatted in the format string.
 */
static inline
void cisError(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  cisLog("ERROR", fmt, ap);
  va_end(ap);
};
  
#ifdef CIS_DEBUG
#if CIS_DEBUG <= 10
#define cislog_error cisError
#define cislog_info cisInfo
#define cislog_debug cisDebug
#elif CIS_DEBUG <= 20
#define cislog_error cisError
#define cislog_info cisInfo
#define cislog_debug while (0) cisDebug
#elif CIS_DEBUG <= 40
#define cislog_error cisError
#define cislog_info while (0) cisInfo
#define cislog_debug while (0) cisDebug
#else
#define cislog_error while (0) cisError
#define cislog_info while (0) cisInfo
#define cislog_debug while (0) cisDebug
#endif
#else
#define cislog_error cisError
#define cislog_info while (0) cisInfo
#define cislog_debug while (0) cisDebug
#endif

/*!
  @brief Check if a character array matches the internal EOF message.
  @param[in] buf constant character pointer to string that should be checked.
  @returns int 1 if buf is the EOF message, 0 otherwise.
 */
static inline
int is_eof(const char *buf) {
  if (strcmp(buf, CIS_MSG_EOF) == 0) {
    return 1;
  } else {
    return 0;
  }
};


#endif /*CISTOOLS_H_*/
