/*      structures used for sysint and sysint21
*/

#ifndef _SYSINT_INCLUDED
#define _SYSINT_INCLUDED

/* used by calls to sysint to hold machine register contents */
struct regval {
    unsigned axr, bxr, cxr, dxr, sir, dir, dsr, esr;
    };

/* used to hold machine segment register contents */
struct segreg {
    unsigned cs, ss, ds, es;
    };

#ifndef _INTREG_DEFINED
/* used to save the registers for interrupt handling */
struct intreg {
    unsigned axr, bxr, cxr, dxr, sir, dir, dsr, esr, bpr, spr, ssr, ifr;
    };
#define _INTREG_DEFINED
#endif

#ifndef NOPROTO
extern  unsigned _cseg (void);
extern  unsigned _dseg (void);
extern  void    cli (void);
extern  void    sti (void);
#endif
#endif

