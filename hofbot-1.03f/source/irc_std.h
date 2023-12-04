/*
 * irc_std.h: header to define things used in all the programs ircii
 * comes with
 *
 * hacked together from various other files by matthew green
 * copyright(c) 1993 
 *
 * See the copyright file, or do a help ircii copyright 
 *
 * @(#)$Id: irc_std.h,v 1.10 1994/07/23 16:26:53 mrg Exp $
 */

#ifndef __irc_std_h
#define __irc_std_h

#ifdef _IBMR2
# include <sys/errno.h>
# include <sys/select.h>
#else
# include <errno.h>
extern	int	errno;
#endif /* _IBMR2 */

#ifdef TRUE
# define TRUE 1
#endif /* TRUE */

#ifndef NBBY
# define NBBY	8		/* number of bits in a byte */
#endif /* NBBY */

#ifndef ALPHA
# define NFDBITS	(sizeof(long) * NBBY)	/* bits per mask */
#endif 

#ifndef FD_SET
# define FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#endif /* FD_SET */

#ifndef FD_CLR
# define FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#endif /* FD_CLR */

#ifndef FD_ISSET
# define FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#endif /* FD_ISSET */

#ifndef FD_ZERO
# define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#endif /* FD_ZERO */

#ifndef	FD_SETSIZE
# define FD_SETSIZE	32
#endif

#ifdef USE_SIGACTION

typedef RETSIGTYPE sigfunc __P((int));
sigfunc *my_signal __P((int, sigfunc *, int));

# define MY_SIGNAL(s_n, s_h, m_f) my_signal(s_n, s_h, m_f)
#else
# if USE_SIGSET
#  define MY_SIGNAL(s_n, s_h, m_f) sigset(s_n, s_h)
# else
#  define MY_SIGNAL(s_n, s_h, m_f) signal(s_n, s_h)
# endif /* USE_SIGSET */
#endif /* USE_SIGACTION */

#if defined(USE_SIGACTION) || defined(USE_SIGSET)
# undef SYSVSIGNALS
#endif

#if defined(__svr4__) && !defined(SVR4)
# define SVR4
#else
# if defined(SVR4) && !defined(__svr4__)
#  define __svr4__
# endif
#endif

#ifdef _SEQUENT_
# define	u_short	ushort
# define	u_char	unchar
# define	u_long	ulong
# define	u_int	uint
# define	USE_TERMIO
# ifndef POSIX
#  define POSIX
# endif
# ifndef SYSV
#  define SYSV
# endif
#endif /* _SEQUENT_ */

#ifdef STDC_HEADERS
# include <stdlib.h>
#endif

#ifndef NeXT
# if defined(STDC_HEADERS) || defined(HAVE_STRING_H)
#  include <string.h>
#  if defined(HAVE_MEMORY_H)
#   include <memory.h>
#  endif /* HAVE_MEMORY_H */
#  undef index
#  undef rindex
#  undef bcopy
#  undef bzero
#  undef bcmp
#  define index strchr
#  define rindex strrchr
#  ifdef HAVE_MEMMOVE
#   define bcopy(s, d, n) memmove((d), (s), (n))
#  else
#   define bcopy(s, d, n) memcpy ((d), (s), (n))
#  endif
#  define bcmp(s, t, n) memcmp ((s), (t), (n))
#  define bzero(s, n) memset ((s), 0, (n))
# else /* STDC_HEADERS || HAVE_STRING_H */
#  include <strings.h>
# endif /* STDC_HEADERS || HAVE_STRING_H */
#endif /* !NeXT */

#ifndef SYS_ERRLIST_DECLARED
extern	char	*sys_errlist[];
#endif

/* we need an unsigned 32 bit integer for dcc, how lame */

#ifdef UNSIGNED_LONG32

typedef		unsigned long		u_32int_t;

#else
# ifdef UNSIGNED_INT32

typedef		unsigned int		u_32int_t;

# else

typedef		unsigned long		u_32int_t;

# endif /* UNSIGNED_INT32 */
#endif /* UNSIGNED_LONG32 */

#endif /* __irc_std_h */
