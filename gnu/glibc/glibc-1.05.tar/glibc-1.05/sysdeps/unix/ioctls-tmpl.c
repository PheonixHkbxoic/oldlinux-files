#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>

#ifdef	SIOCGIFCONF
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <net/route.h>
#if	defined(SIOCGARP) && !defined(ARPOP_REQUEST)
#include <net/if_arp.h>
#endif
#ifdef	SIOCGNIT
#include <net/nit.h>
#endif
#endif

/* These exist on Sequents.  */
#ifdef SMIOSTATS
#include <sec/sec.h>
#include <sec/sm.h>
#endif
#ifdef SMIOGETREBOOT0
#include <i386/cfg.h>
#endif
#ifdef ZIOCBCMD
#include <zdc/zdc.h>
#endif

/* These exist under Ultrix, but I figured there may be others.  */
#ifdef DIOCGETPT
#include <ufs/fs.h>           /* for DIOC* */
#endif
#ifdef DEVGETGEOM
#include <sys/devio.h>
#endif

#ifdef ultrix
/* Ultrix has a conditional include that brings these in; we have to force
   their inclusion when we actually compile them.  */
#undef TCGETA
#undef TCSETA
#undef TCSETAW
#undef TCSETAF
#undef TCGETP
#undef TCSANOW
#undef TCSADRAIN
#undef TCSAFLUSH
#ifdef ELSETPID
#include <sys/un.h> /* get sockaddr_un for elcsd.h */
#include <elcsd.h>
#endif
#ifdef DKIOCDOP
#include <sys/dkio.h>
#endif
/* Couldn't find the header where the structures used by these are
   defined; it looks like an unbundled LAT package or something.  */
#undef LIOCSOL
#undef LIOCRES
#undef LIOCCMD
#undef LIOCINI
#undef LIOCTTYI
#undef LIOCCONN
/* struct mtop hasn't been in sys/mtio.h since 4.1 */
#undef MTIOCTOP
#undef MTIOCGET
#endif

#define	DEFINE(name, value) \
  printf("#define %s 0x%.8x\n", (name), (value))

int
main()
{
  REQUESTS

  exit(0);
  return 0;
}
