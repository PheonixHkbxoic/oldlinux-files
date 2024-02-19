/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	POSIX Standard: 7.1-2 General Terminal Interface	<termios.h>
 */

#ifndef	_TERMIOS_H

#define	_TERMIOS_H	1
#include <features.h>

/* This is supposed to be compatible with 4.4BSD.  */

/* Type of terminal control flag masks.  */
typedef unsigned long int tcflag_t;

/* Type of control characters.  */
typedef unsigned char cc_t;

/* Type of baud rate specifiers.  */
typedef long int speed_t;

/* Terminal control structure.  */
struct termios
  {
    /* Input modes.  */
    tcflag_t c_iflag;
#define	IGNBRK	(1 << 0)	/* Ignore break condition.  */
#define	BRKINT	(1 << 1)	/* Signal interrupt on break.  */
#define	IGNPAR	(1 << 2)	/* Ignore characters with parity errors.  */
#define	PARMRK	(1 << 3)	/* Mark parity and framing errors.  */
#define	INPCK	(1 << 4)	/* Enable input parity check.  */
#define	INLCR	(1 << 5)	/* Map NL to CR on input.  */
#define	ISTRIP	(1 << 6)	/* Strip 8th bit off characters.  */
#define	IGNCR	(1 << 7)	/* Ignore CR.  */
#define	ICRNL	(1 << 8)	/* Map CR to NL on input.  */
#define	IXON	(1 << 9)	/* Enable start/stop output control.  */
#define	IXOFF	(1 << 10)	/* Enable start/stop input control.  */
#ifdef	__USE_BSD
#define	IXANY	(1 << 11)	/* Any character will restart after stop.  */
#define	IMAXBEL	(1 << 13)	/* Ring bell when input queue is full.  */
#endif

    /* Output modes.  */
    tcflag_t c_oflag;
#define	OPOST	(1 << 0)	/* Perform output processing.  */
#ifdef	__USE_BSD
#define	ONLCR	(1 << 1)	/* Map NL to CR-NL on output.  */
#define	OXTABS	(1 << 2)	/* Expand tabs to spaces.  */
#define	ONOEOT	(1 << 8)	/* Discard EOT (^D) on output.  */
#endif

    /* Control modes.  */
    tcflag_t c_cflag;
#ifdef	__USE_BSD
#define	CIGNORE	(1 << 0)		/* Ignore these control flags.  */
#endif
#define	CSIZE	(CS5|CS6|CS7|CS8)	/* Number of bits per byte (mask).  */
#define	CS5	0			/* 5 bits per byte.  */
#define	CS6	(1 << 8)		/* 6 bits per byte.  */
#define	CS7	(1 << 9)		/* 7 bits per byte.  */
#define	CS8	(CS6|CS7)		/* 8 bits per byte.  */
#define	CSTOPB	(1 << 10)		/* Two stop bits instead of one.  */
#define	CREAD	(1 << 11)		/* Enable receiver.  */
#define	PARENB	(1 << 12)		/* Parity enable.  */
#define	PARODD	(1 << 13)		/* Odd parity instead of even.  */
#define	HUPCL	(1 << 14)		/* Hang up on last close.  */
#define	CLOCAL	(1 << 15)		/* Ignore modem status lines.  */
#ifdef	__USE_BSD
#define	CCTS_OFLOW	(1 << 16)	/* CTS flow control of output.  */
#define	CRTS_IFLOW	(1 << 17)	/* RTS flow control of input.  */
#define	MDMBUF		(1 << 20)	/* Carrier flow control of output.  */
#endif

    /* Local modes.  */
    tcflag_t c_lflag;
#ifdef	__USE_BSD
#define	ECHOKE	(1 << 0)	/* Visual erase for KILL.  */
#endif
#define	_ECHOE	(1 << 1)	/* Visual erase for ERASE.  */
#define	ECHOE	_ECHOE
#define	_ECHOK	(1 << 2)	/* Echo NL after KILL.  */
#define	ECHOK	_ECHOK
#define	_ECHO	(1 << 3)	/* Enable echo.  */
#define	ECHO	_ECHO
#define	_ECHONL	(1 << 4)	/* Echo NL even if ECHO is off.  */
#define	ECHONL	_ECHONL
#ifdef	__USE_BSD
#define	ECHOPRT	(1 << 5)	/* Hardcopy visual erase.  */
#define	ECHOCTL	(1 << 6)	/* Echo control characters as ^X.  */
#endif
#define	_ISIG	(1 << 7)	/* Enable signals.  */
#define	ISIG	_ISIG
#define	_ICANON	(1 << 8)	/* Do erase and kill processing.  */
#define	ICANON	_ICANON
#ifdef	__USE_BSD
#define	ALTWERASE (1 << 9)	/* Alternate WERASE algorithm.  */
#endif
#define	_IEXTEN	(1 << 10)	/* Enable DISCARD and LNEXT.  */
#define	IEXTEN	_IEXTEN
#define	EXTPROC	(1 << 11)	/* External processing.  */
#define	_TOSTOP	(1 << 22)	/* Send SIGTTOU for background output.  */
#define	TOSTOP	_TOSTOP
#ifdef	__USE_BSD
#define	FLUSHO	(1 << 23)	/* Output being flushed (state).  */
#define	NOKERNINFO (1 << 25)	/* Disable VSTATUS.  */
#define	PENDIN	(1 << 29)	/* Retype pending input (state).  */
#endif
#define	_NOFLSH	(1 << 31)	/* Disable flush after interrupt.  */
#define	NOFLSH	_NOFLSH

    /* Control characters.  */
#define	VEOF	0	/* End-of-file character [ICANON].  */
#define	VEOL	1	/* End-of-line character [ICANON].  */
#ifdef	__USE_BSD
#define	VEOL2	2	/* Second EOL character [ICANON].  */
#endif
#define	VERASE	3	/* Erase character [ICANON].  */
#ifdef	__USE_BSD
#define	VWERASE	4	/* Word-erase character [ICANON].  */
#endif
#define	VKILL	5	/* Kill-line character [ICANON].  */
#ifdef	__USE_BSD
#define	VREPRINT 6	/* Reprint-line character [ICANON].  */
#endif
#define	VINTR	8	/* Interrupt character [ISIG].  */
#define	VQUIT	9	/* Quit character [ISIG].  */
#define	VSUSP	10	/* Suspend character [ISIG].  */
#ifdef	__USE_BSD
#define	VDSUSP	11	/* Delayed suspend character [ISIG].  */
#endif
#define	VSTART	12	/* Start (X-ON) character [IXON, IXOFF].  */
#define	VSTOP	13	/* Stop (X-OFF) character [IXON, IXOFF].  */
#ifdef	__USE_BSD
#define	VLNEXT	14	/* Literal-next character [IEXTEN].  */
#define	VDISCARD 15	/* Discard character [IEXTEN].  */
#endif
#define	VMIN	16	/* Minimum number of bytes read at once [!ICANON].  */
#define	VTIME	17	/* Time-out value (tenths of a second) [!ICANON].  */
#ifdef	__USE_BSD
#define	VSTATUS	18	/* Status character [ICANON].  */
#endif
#define	NCCS	20
    cc_t c_cc[NCCS];

    /* Input and output baud rates.  */
    speed_t __ispeed, __ospeed;
#define	B0	0	/* Hang up.  */
#define	B50	50	/* 50 baud.  */
#define	B75	75	/* 75 baud.  */
#define	B110	110	/* 110 baud.  */
#define	B134	134	/* 134.5 baud.  */
#define	B150	150	/* 150 baud.  */
#define	B200	200	/* 200 baud.  */
#define	B300	300	/* 300 baud.  */
#define	B600	600	/* 600 baud.  */
#define	B1200	1200	/* 1200 baud.  */
#define	B1800	1800	/* 1800 baud.  */
#define	B2400	2400	/* 2400 baud.  */
#define	B4800	4800	/* 4800 baud.  */
#define	B9600	9600	/* 9600 baud.  */
#define	B19200	19200	/* 19200 baud.  */
#define	B38400	38400	/* 38400 baud.  */
#ifdef	__USE_BSD
#define	EXTA	19200
#define	EXTB	38400
#endif
  };


/* Return the output baud rate stored in *TERMIOS_P.  */
extern speed_t EXFUN(cfgetospeed, (CONST struct termios *__termios_p));

/* Return the input baud rate stored in *TERMIOS_P.  */
extern speed_t EXFUN(cfgetispeed, (CONST struct termios *__termios_p));

/* Set the output baud rate stored in *TERMIOS_P to SPEED.  */
extern int EXFUN(cfsetospeed, (struct termios *__termios_p, speed_t __speed));

/* Set the input baud rate stored in *TERMIOS_P to SPEED.  */
extern int EXFUN(cfsetispeed, (struct termios *__termios_p, speed_t __speed));

#ifdef	__OPTIMIZE__
#define	cfgetospeed(t)		((t)->__ospeed)
#define	cfgetispeed(t)		((t)->__ispeed)
#define	cfsetospeed(t, s)	((t)->__ospeed = (s))
#define	cfsetispeed(t, s)	((t)->__ispeed = (s))
#endif

#ifdef	__USE_BSD
/* Set both the input and output baud rates in *TERMIOS_OP to SPEED.  */
extern void EXFUN(cfsetspeed, (struct termios *__termios_p, speed_t __speed));
#endif


/* Put the state of FD into *TERMIOS_P.  */
extern int EXFUN(__tcgetattr, (int __fd, struct termios *__termios_p));
extern int EXFUN(tcgetattr, (int __fd, struct termios *__termios_p));

#ifdef	__OPTIMIZE__
#define	tcgetattr(fd, termios_p)	__tcgetattr((fd), (termios_p))
#endif	/* Optimizing.  */

/* Values for the OPTIONAL_ACTIONS argument to `tcsetattr'.  */
#define	TCSANOW		0	/* Change immediately.  */
#define	TCSADRAIN	1	/* Change when pending output is written.  */
#define	TCSAFLUSH	2	/* Flush pending input before changing.  */
#ifdef	__USE_BSD
#define	TCSASOFT	0x10	/* Flag: Don't alter hardware state.  */
#endif

/* Set the state of FD to *TERMIOS_P.  */
extern int EXFUN(tcsetattr, (int __fd, int __optional_actions,
			     CONST struct termios *__termios_p));


#ifdef	__USE_BSD
/* Set *TERMIOS_P to indicate raw mode.  */
extern void EXFUN(cfmakeraw, (struct termios *__termios_p));
#endif

/* Send zero bits on FD.  */
extern int EXFUN(tcsendbreak, (int __fd, int __duration));

/* Wait for pending output to be written on FD.  */
extern int EXFUN(tcdrain, (int __fd));

/* Values for the QUEUE_SELECTOR argument to `tcflush'.  */
#define	TCIFLUSH	1	/* Discard data received but not yet read.  */
#define	TCOFLUSH	2	/* Discard data written but not yet sent.  */
#define	TCIOFLUSH	3	/* Discard all pending data.  */

/* Flush pending data on FD.  */
extern int EXFUN(tcflush, (int __fd, int __queue_selector));

/* Values for the ACTION argument to `tcflow'.  */
#define	TCOOFF	1	/* Suspend output.  */
#define	TCOON	2	/* Restart suspended output.  */
#define	TCIOFF	3	/* Send a STOP character.  */
#define	TCION	4	/* Send a START character.  */

/* Suspend or restart transmission on FD.  */
extern int EXFUN(tcflow, (int __fd, int __action));


#endif	/* termios.h  */
