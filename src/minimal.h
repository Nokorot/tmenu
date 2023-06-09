#ifndef MINIMAL_H__
#define MINIMAL_H__

#include <termios.h>
#include <unistd.h>

/* Restore terminal to original settings
*/
void terminal_done(void);

/* "Default" signal handler: restore terminal, then exit.
*/
void terminal_signal(int signum);

/* Initialize terminal for non-canonical, non-echo mode,
 * that should be compatible with standard C I/O.
 * Returns 0 if success, nonzero errno otherwise.
*/
int terminal_init(void);
#endif
