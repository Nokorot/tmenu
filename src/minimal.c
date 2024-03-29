#include "minimal.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

struct termios terminal_original;
struct termios terminal_settings;

int terminal_descriptor = -1;

void terminal_done(void) {
  printf("%s", "\x1B[\?1049l");
  if (terminal_descriptor != -1)
      tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);
}

void terminal_signal(int signum) {
    printf("%s", "\x1B[\?1049l");
    if (terminal_descriptor != -1)
        tcsetattr(terminal_descriptor, TCSANOW, &terminal_original);

    /* exit() is not async-signal safe, but _exit() is.
     * Use the common idiom of 128 + signal number for signal exits.
     * Alternative approach is to reset the signal to default handler,
     * and immediately raise() it. */
    _exit(128 + signum);
}

int terminal_init(void) {
    struct sigaction act;

    // TODO: See if this needs to be moved. A bit of abrstraction would be nice
    printf("%s", "\x1B[\?1049h");

    /* Already initialized? */
    if (terminal_descriptor != -1)
        return errno = 0;

    /* Which standard stream is connected to our TTY? */
    if (isatty(STDERR_FILENO))
        terminal_descriptor = STDERR_FILENO;
    else
    if (isatty(STDIN_FILENO))
        terminal_descriptor = STDIN_FILENO;
    else
    if (isatty(STDOUT_FILENO))
        terminal_descriptor = STDOUT_FILENO;
    else
        return errno = ENOTTY;

    /* Obtain terminal settings. */
    if (tcgetattr(terminal_descriptor, &terminal_original) ||
        tcgetattr(terminal_descriptor, &terminal_settings))
        return errno = ENOTSUP;

    /* Disable buffering for terminal streams. */
    if (isatty(STDIN_FILENO))
        setvbuf(stdin, NULL, _IONBF, 0);
    if (isatty(STDOUT_FILENO))
        setvbuf(stdout, NULL, _IONBF, 0);
    if (isatty(STDERR_FILENO))
        setvbuf(stderr, NULL, _IONBF, 0);

    /* At exit() or return from main(),
     * restore the original settings. */
    if (atexit(terminal_done))
        return errno = ENOTSUP;

    /* Set new "default" handlers for typical signals,
     * so that if this process is killed by a signal,
     * the terminal settings will still be restored first. */
    sigemptyset(&act.sa_mask);
    act.sa_handler = terminal_signal;
    act.sa_flags = 0;
    if (sigaction(SIGHUP,  &act, NULL) ||
        sigaction(SIGINT,  &act, NULL) ||
        sigaction(SIGQUIT, &act, NULL) ||
        sigaction(SIGTERM, &act, NULL) ||
#ifdef SIGXCPU
        sigaction(SIGXCPU, &act, NULL) ||
#endif
#ifdef SIGXFSZ
        sigaction(SIGXFSZ, &act, NULL) ||
#endif
#ifdef SIGIO
        sigaction(SIGIO,   &act, NULL) ||
#endif
        sigaction(SIGPIPE, &act, NULL) ||
        sigaction(SIGALRM, &act, NULL))
        return errno = ENOTSUP;

    /* Let BREAK cause a SIGINT in input. */
    terminal_settings.c_iflag &= ~IGNBRK;
    terminal_settings.c_iflag |=  BRKINT;

    /* Ignore framing and parity errors in input. */
    terminal_settings.c_iflag |=  IGNPAR;
    terminal_settings.c_iflag &= ~PARMRK;

    /* Do not strip eighth bit on input. */
    terminal_settings.c_iflag &= ~ISTRIP;

    /* Do not do newline translation on input. */
    terminal_settings.c_iflag &= ~(INLCR | IGNCR | ICRNL);

#ifdef IUCLC
    /* Do not do uppercase-to-lowercase mapping on input. */
    terminal_settings.c_iflag &= ~IUCLC;
#endif

    /* Use 8-bit characters. This too may affect standard streams,
     * but any sane C library can deal with 8-bit characters. */
    terminal_settings.c_cflag &= ~CSIZE;
    terminal_settings.c_cflag |=  CS8;

    /* Enable receiver. */
    terminal_settings.c_cflag |=  CREAD;

    /* Let INTR/QUIT/SUSP/DSUSP generate the corresponding signals. */
    terminal_settings.c_lflag |=  ISIG;

    /* Enable noncanonical mode.
     * This is the most important bit, as it disables line buffering etc. */
    terminal_settings.c_lflag &= ~ICANON;

    /* Disable echoing input characters. */
    terminal_settings.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

    /* Disable implementation-defined input processing. */
    terminal_settings.c_lflag &= ~IEXTEN;

    /* To maintain best compatibility with normal behaviour of terminals,
     * we set TIME=0 and MAX=1 in noncanonical mode. This means that
     * read() will block until at least one byte is available. */
    terminal_settings.c_cc[VTIME] = 0;
    terminal_settings.c_cc[VMIN] = 1;

    /* Set the new terminal settings.
     * Note that we don't actually check which ones were successfully
     * set and which not, because there isn't much we can do about it. */
    tcsetattr(terminal_descriptor, TCSANOW, &terminal_settings);

    /* Done. */
    return errno = 0;
}
