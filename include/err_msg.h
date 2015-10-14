#ifndef ERR_MSG_H
#define ERR_MSG_H
#include <stdio.h>

/* We adopt the rule that functions do not exit if an error occurs.
   Instead they set a global error message and return 0 as error code.
   This makes it easy to test a function's error handling behavior.
   We only need to test the function's exit code and the value of the
   global error message. */

#define set_err_msg(...) snprintf(err_msg, ERR_MSG_MAXLEN, __VA_ARGS__)
#define clear_err_msg() err_msg[0] = '\0'
#define pr_err_msg() fprintf(stderr, "%s\n", err_msg)

enum { ERR_MSG_MAXLEN = 200 };
extern char err_msg[];

#endif  /* ERR_MSG_H */
