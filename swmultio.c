/*
        swmultio -      SW multiple plane communications I/O

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-02-10        Development
                        85-11-15        Atari
                        87-03-09        Microsoft compiler.
                        96-12-27        Module replaced by swnetio.c.
                                        This module is maintained and
                                        compiled for historical reasons,
                                        but is not linked.
                        2000-10-29      Re-inserted into project for
                                          dependable compile/link.
*/
#include        "sw.h"

// sdh: this file is well and truly broken

extern  int     playmode;               /* Mode of play                     */
extern  GAMES   swgames[], *currgame;   /* Game parameters and current game */
extern  MULTIO  *multbuff;              /* Communications buffer            */
extern  int     multkey;                /* Keystroke to be passed           */
extern  int     player;                 /* Pointer to player's object       */
extern  OBJECTS *nobjects;              /* Objects list.                    */
extern  OBJECTS oobjects[];             /* Original plane object description*/
extern  int     keydelay;               /*  Number of displays per keystr   */
extern  BOOL    repflag;                /* Report statistics flag           */
extern  int     counttick, countmove;   /* Performance counters             */
extern  int     multtick;               /*  Multiple user tick delay        */
extern  char    auxdisp[];              /* Auxiliary display area used as a */
                                        /* disk buffer                      */
extern  unsigned explseed;              /* explosion seed                 */

extern  int     dispmult(),             /*  Display and move functions      */
                movemult();

extern  BOOL    compplane;              /* Moving computer plane flag      */
extern  BOOL    plyrplane;              /* Moving player plane flag        */
extern  int     currobx;                /* Current object index             */
extern  int     endsts[];               /* End of game status and move count*/
extern  int     dispcnt;                /* Displays to delay keyboard       */
extern  int     endstat;                /* End of game status for curr. move*/
extern  int     multaddr;               /* Diskette adapter address         */


char    multstck[400];                  /* Stack area for I/O processes     */
int     *multstack;                     /* Initial stack pointer            */

char    *multfile = COMM_FILE;          /* Multiple user files              */
char    *cmndfile = COMM_CMD;

extern  OBJECTS *initpln();


struct  dirent {                        /* Directory entry structure        */
    char    dc[32];
};

#define DIR_NAME        0
#define DIR_EXT         8
#define DIR_ATTR        11
#define DIR_RES         12
#define DIR_TIME        22
#define DIR_DATE        24
#define DIR_CLU         26
#define DIR_SIZE        28


struct bpbtab {                         /* Disk BPB structure               */
    char    bc[19];
};

#define BP_SECSIZE      0
#define BP_CLUSIZE      2
#define BP_RESERVED     3
#define BP_NFAT         5
#define BP_DIRENT       6
#define BP_NSECTORS     8
#define BP_FATID        10
#define BP_FATSIZE      11
#define BP_SECPTRK      13
#define BP_NHEADS       15
#define BP_NHIDDEN      17

//static char *prtbuf = auxdisp;
static struct bpbtab *swbpb;
static BIOFD  devfd;


static  int     multdriv,  commdriv,   /* Communications file parms        */
                multhead,  commhead,
                multtrk,   commtrk,
                multsect,  commsect;
static  long    multasect, commasect;

static  BOOL     first;                 /* first player flag              */
static  char    *errormsg;
static  BOOL    errflg1;
static  BOOL    errorflg = FALSE;
static  BOOL    multterm = FALSE;
static  BOOL    swlocked  = FALSE;

static  unsigned errtab[50];
static  unsigned errwhr[50];
static  int      errptr = -1;
#define OREAD           1
#define OWRITE          2
#define OREADPLAY       3
#define OWRITEPLAY      4




_word( ptr )
char    *ptr;
{
        return( ( *ptr & 0x00FF ) | ( ( *(ptr+1) & 0x00FF ) << 8 ) );
}



_byte( ptr )
char    *ptr;
{
        return( *ptr & 0x00FF );
}



_fromintel()
{
#ifdef ATARI
register int    *mu, word, i;

        mu = (int *) multbuff;
        for ( i = sizeof( MULTIO ) >> 1; i; --i ) {
                word = *mu;
                *mu++ = ( ( word >> 8 ) & 0x00FF ) | ( word << 8 );
        }
#endif
}



char    *_tointel()
{
#ifdef IBMPC
        return( (char *) multbuff );
#endif


#ifdef ATARI
static   MULTIO mubuff;
register int    *mu1, *mu2, i, word;

        mu1 = (int *) multbuff;
        mu2 = (int *) &mubuff;
        for ( i = sizeof( MULTIO ) >> 1; i; --i ) {
                word = *mu1++;
                *mu2++ = ( ( word >> 8 ) & 0x00FF ) | ( word << 8 );
        }
        return( (char *) &mubuff );
#endif
}




movemult( obp )
OBJECTS *obp;
{
register OBJECTS *ob;

        plyrplane = compplane = FALSE;

        endstat = endsts[currobx = ( ob = obp )->ob_index];

        if ( !dispcnt )
                interpret( ob, ( playmode == MULTIPLE ) ? multget( ob )
                                                        : asynget( ob ) );
        else {
                ob->ob_flaps = 0;
                ob->ob_bombing = FALSE;
        }
        if ( ( ( ob->ob_state == CRASHED ) || ( ob->ob_state == GHOSTCRASHED ))
                && ( ob->ob_hitcount <= 0 ) )
                if ( ob->ob_life > QUIT ) {
                        ++ob->ob_crashcnt;
                        initpln( ob );
                }
        return( movepln( ob ) );
}


multopen( cmndfile, multfile )
char    *cmndfile, *multfile;
{


        multparm( cmndfile );

        commdriv = multdriv;
        commhead = multhead;
        commtrk  = multtrk;
        commsect = multsect;
        commasect = multasect;

        multparm( multfile );

}



multparm( multfile )
char    *multfile;
{
#ifdef IBMPC
char    dev[3];
BIOFD   fd;
register int    i;
register char   *mf;

        mf = multfile;
        dev[2] = 0;
        if ( ( ( multdriv = ( dev[0] = tolower( *mf++ ) ) - 'a' ) < 0 )
                || ( multdriv > 25 )
                || ( ( dev[1] = *mf++ ) != ':' )
                || ( !( fd = devfd = bopen( dev, "rw") ) ) )
                swend( "Improper device specification", NO );


        i = 0;
        while ( mf[i] ) {
                mf[i] = toupper(mf[i]);
                ++i;
        }
        if ( !( multsect = name_to_sec( fd, mf )) )
                swend( "File not found", NO );

        multasect = --multsect;
        sectparm();
#endif
}



static  name_to_sec( fd, name )
BIOFD   fd;
char    *name;
{
#ifdef IBMPC
struct  bpbtab  bpb;
struct  dirent  entry;

        make_bpb( fd, &bpb );
        if ( !get_ent( fd, &bpb, name, &entry ))
                return(NO);
        return( clu_to_sec( _word( entry.dc + DIR_CLU ), &bpb ) );
#endif
}



static  clu_to_sec( cluster, bpb )
int             cluster;
struct  bpbtab  *bpb;
{
        return( ( cluster - 2 ) * _byte( bpb->bc + BP_CLUSIZE )
                + ( _word( bpb->bc + BP_DIRENT ) >> 4 )
                + 1
                + _word( bpb->bc + BP_RESERVED )
                + ( _byte( bpb->bc + BP_NFAT ) * _word( bpb->bc + BP_FATSIZE ) )
              );
}



static  make_bpb( fd, bpb )
BIOFD           fd;
struct  bpbtab  *bpb;
{
#ifdef IBMPC
        bseek( fd, 0l, 0);
        bread( prtbuf, 512, fd);
        movmem( prtbuf + 11, bpb, sizeof( struct bpbtab ) );
        swbpb = bpb;
#endif
}




static  get_ent( fd, bpb, name, entry )
BIOFD           fd;
struct  bpbtab  *bpb;
char            *name;
struct  dirent  *entry;
{
        if ( *name == '\\' )
                ++name;
        return( lookup( fd, bpb, name, entry ) );
}



static  lookup( fd, bpb, name, entry )
BIOFD           fd;
struct  bpbtab  *bpb;
char            *name;
struct  dirent  *entry;
{
#ifdef IBMPC
long             block;
int              strtdir;
register int     i;
register struct  dirent  *current;
char             want[12];

        for ( i = 0; *name && ( i < 12 ); ++name ) {
                if ( *name == '.')
                        while ( i < 8 )
                                want[i++] = ' ';
                else
                        want[i++] = *name;
        }
        while ( i < 11 )
                want[i++] = ' ';
        want[11] = 0;

        block = 1 + _word( bpb->bc + BP_RESERVED )
               + ( _byte( bpb->bc + BP_NFAT ) * _word( bpb->bc + BP_FATSIZE ) );
        bseek( fd, (long)( ( block - 1 ) << 9 ), 0);
        for ( strtdir = 0;
                strtdir < _word( bpb->bc+BP_DIRENT );
                strtdir += 0x10 ) {
                bread( prtbuf, 512, fd);
                for ( current = (struct dirent *)prtbuf, i = 0;
                        i < 0x10;
                        ++i, ++current) {
                        if ( ( current->dc + DIR_NAME )[0] == 0 )
                                return(NO);
                        if ( strncmp( want, current->dc + DIR_NAME,11 ) == 0) {
                                movmem( current, entry, sizeof(struct dirent));
                                return(YES);
                        }
                }
        }
#endif
        return(NO);

}



sectparm()
{
int     key;
#ifdef  IBMPC
struct  regval  reg;
struct  {
        char     fill1[55];
        unsigned firstdrive;
        char     fill2[2];
        unsigned diskbase;
        char     fill3[5];
        unsigned signat;
}       iocbuf;
int     spc;

        spc = _word( swbpb->bc+BP_SECPTRK ) * _word( swbpb->bc+BP_NHEADS );
        multtrk = multsect / spc;
        multsect -= multtrk * spc;
        multhead = multsect / _word( swbpb->bc+BP_SECPTRK );
        multsect -= (multhead * _word( swbpb->bc+BP_SECPTRK ) ) - 1;

        reg.axr = 0x4404;
        reg.bxr = multdriv + 1;
        reg.cxr = sizeof( iocbuf );
        reg.dxr = (unsigned) &iocbuf;
        reg.dsr = dsseg();
        sysint21( &reg, &reg );
        if ( iocbuf.signat == 0x4003 ) {
                multaddr = iocbuf.diskbase;
                multdriv -= iocbuf.firstdrive;
                multhead |= ( ( multdriv % 3 ) + 1 ) << 6;
                multdriv = 3 - ( multdriv / 3 );
        }
#endif

#ifdef  ATARI
        multhead = 0;
        multtrk = 0;
#endif

/*      puts( "\r\nAddress: " );
        dispd( multaddr, 6 );
        puts( "\r\nDrive  : " );
        dispd( multdriv, 6 );
        puts( "\r\nHead   : " );
        dispd( multhead, 6 );
        puts( "\r\nTrack  : " );
        dispd( multtrk,  6 );
        puts( "\r\nSector : " );
        dispd( multsect, 6 );
        puts( "\r\nOK? (Y/N)" );
        FOREVER {
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( key = toupper( swgetc() & 0x00FF ) ) == 'Y' )
                        break;
                if ( key == 'N' )
                        swend( NULL, NO );
        }
*/
}



multread()
{
#ifdef IBMPC
        bseek( devfd, commasect << 9, 0 );
        if ( bread( prtbuf, 512, devfd ) != 512 ) {
                errlog( ( bioerr() & 0xFF00 ) + 1, OREAD );
                return( 0 );
        }
        if ( *prtbuf != 0xFE )
                return( -1 );

        *prtbuf = 0xFF;
        bseek( devfd, commasect << 9, 0 );
        if ( bwrite( prtbuf, 512, devfd ) != 512 ) {
                errlog( ( bioerr() & 0xFF00 ) + 2, OREAD );
                return( 0 );
        }

        bseek( devfd, commasect << 9, 0 );
        if ( bread( prtbuf, 512, devfd ) != 512 ) {
                errlog( ( bioerr() & 0xFF00 ) + 3, OREAD );
                return( 0 );
        }
        if ( *prtbuf != 0xFF )
                return( -1 );

        swlocked = TRUE;

        bseek( devfd, multasect << 9, 0 );
        if ( bread( multbuff, 512, devfd ) != 512 ) {
                errlog( ( bioerr() & 0xFF00 ) + 4, OREAD );
                return( 0 );
        }
        _fromintel();
#endif
        return( 1 );
}





multwrite()
{
#ifdef IBMPC
register int     dkerr;
register char    *buff;

        FOREVER  {
                bseek( devfd, multasect << 9, 0 );
                buff = _tointel();
                if ( bwrite( buff, 512, devfd ) == 512 )
                        return( 1 );
                dkerr = 0xFF00 & bioerr();
                errlog( dkerr, OWRITE );
                if ( dkerr != 0x0300 )
                        return( 0 );
        }
#endif
}




multunlock()
{
#ifdef IBMPC
        if ( !swlocked )
                return( 1 );

        *prtbuf = 0xFE;
        bseek( devfd, commasect << 9, 0 );
        if ( bwrite( prtbuf, 512, devfd ) != 512 )
                return( 0 );

        swlocked = FALSE;
#endif
        return( 1 );
}




multwait()
{
#ifdef IBMPC
int     _multwait();

        _dkproc( _multwait, multstack );
        while ( _dkiosts() );
        if ( errorflg )
                swend( errormsg, errflg1 );
#endif
}





multget( ob )
OBJECTS *ob;
{
#ifdef IBMPC
register int     o;

        if ( errorflg )
                 swend( errormsg, errflg1 );

        while ( _dkiosts() );
        if ( errorflg )
                swend( errormsg, errflg1 );

        o = ob->ob_index;
        if ( o != player )
                updstate( ob, multbuff->mu_state[o] );
        return( histmult( o, multbuff->mu_key[o] ) );
#endif
}



static  updstate( obp, statep )
OBJECTS *obp;
int     statep;
{
register OBJECTS *ob;
register int     state;

        if ( ( ( ob = obp )->ob_state != ( state = statep ) )
                && ( ( state == FINISHED )
                || ( state == WAITING )
                || ( ob->ob_state == FINISHED )
                || ( ob->ob_state == WAITING ) ) ) {
                ob->ob_state = state;
                setvdisp();
                dispwobj( ob );
        }
}




multput()
{
#ifdef IBMPC
int     _multput();

        while ( _dkiosts() );
        if ( errorflg )
                 swend( errormsg, errflg1 );
        _dkproc( _multput, multstack );
#endif
}





char    *multclos( update )
BOOL    update;
{

#ifdef IBMPC

register int     rc, n;
char             *closeret  = NULL;
BOOL             alldone;
int              tickwait;

        if ( repflag ) {
                errrep();
                delayrep();
                statrep();
        }

        multterm = TRUE;
        while ( _dkiosts() );

        if ( update ) {
                for ( n = 0; n < 25; ++n ) {
                        if ( ( rc = multread() ) >= 0 )
                                break;
                        intsoff();
                        tickwait = 18;
                        counttick = 0;
                        intson();
                        while ( counttick < tickwait );
                }
                if ( !rc )
                        closeret =  "Read error on communications file during close";
                else
                        if ( rc < 0 )
                            closeret = "Communications file locked during close" ;
                        else
                            if ( editnum() )
                                closeret = "Bad player counter";
                            else {
                                multbuff->mu_state[player] = FINISHED;
                                multbuff->mu_key[player] = 0;
                                alldone = TRUE;
                                for ( n = 0; n < multbuff->mu_maxplyr; ++n )
                                        if ( multbuff->mu_state[n] != FINISHED ) {
                                                alldone = FALSE;
                                                break;
                                        }
                                if ( alldone ) {
                                        multbuff->mu_numplyr = 0;
                                        multbuff->mu_lstplyr = 0;
                                        for ( n = 0; n < MAX_PLYR; ++n )
                                                multbuff->mu_state[n] = WAITING;
                                }

                                if ( !multwrite() )
                                        closeret
                                        = "Write error on communications file";
                            }
        }

        if ( swlocked )
                 if ( !multunlock() )
                         closeret = "Unlock error on communications file";

        return( closeret );
#endif
}





static  _multwait()
{
#ifdef IBMPC
register MULTIO  *mu;
register int     i;
int              count, dkerr;

        mu = multbuff;
        FOREVER {
                _dktick( 18 );
                for ( count = 0; count < 25; ++count ) {
                        if ( ctlbreak() ) {
                                error( NULL, YES );
                                return;
                        }
                        if ( !( dkerr = 0xFF00 & _dkio( 0x2, multdriv, multhead,
                                                        multtrk, multsect,
                                                        1, mu, dsseg() ) ) )
                                break;
                        errlog( dkerr, OREADPLAY );
                        _dkio( 0x0 );
                }
                _fromintel();
                if ( count == 25 ) {
                        error( "Read error during wait", YES );
                        return;
                }
                if ( editnum() ) {
                        error( "Bad player count", YES );
                        return;
                }
                for ( i = 0; i < mu->mu_maxplyr; ++i )
                        if ( ( mu->mu_state[i] != FLYING )
                                && ( mu->mu_state[i] != FINISHED ) )
                                break;
                if ( i == mu->mu_maxplyr )
                        return;
        }
#endif
}




static  unsigned curtry    = 0;



static  _multput()
{
#ifdef IBMPC
register MULTIO  *mu;
register OBJECTS *ob;
int              count, dkerr;
int              tickwait;
static   BOOL    first = TRUE;
char             *buff;

        ob = &nobjects[player];
        mu = multbuff;

        delaymov();
        updated( 0, player );

        if ( multterm )
                return;
        if ( ctlbreak() ) {
                error( NULL, YES );
                return;
        }

        if  ( first )
                first = FALSE;
        else
                mu->mu_key[player] = swgetc();

        mu->mu_state[player] = ob->ob_state;
        mu->mu_lstplyr = player;
        swflush();

        intsoff();
        tickwait = 180;
        counttick = 0;
        intson();
        count = 0;
        buff = _tointel();
        while ( count < 25 ) {
                if ( !(dkerr = 0xFF00 & _dkio( 0x3, multdriv, multhead, multtrk,
                                               multsect, 1, buff, dsseg() ) ) )
                       break;
                _dkio( 0x0 );

                if ( ( counttick > tickwait ) && ctlbreak() ) {
                        error( NULL, YES );
                        return;
                }

                delay();
                errlog( dkerr, OWRITEPLAY );
                if ( dkerr != 0x0300 )
                        ++count;
        }
        if ( count == 25 ) {
                error( "Write error during play", YES );
                return;
        }

        delay();

        if ( editnum() )
                error( "Bad Player count", YES );

        updated( player + 1, mu->mu_maxplyr );
        changedelay();
#endif
}



static  updated( n1, n2 )
int     n1, n2;
{
#ifdef IBMPC
int              n, count;
register MULTIO  *mu;
register OBJECTS *ob;
BOOL             done;
int              dkerr, last;
int              tickwait;
BOOL             readdone = FALSE;

        mu = multbuff;
        intsoff();
        tickwait = 180;
        counttick = 0;
        intson();
        FOREVER {
                last = mu->mu_lstplyr;
                done = TRUE;
                for ( n = n1; n < n2; ++n )
                        if ( mu->mu_state[n] != FINISHED )
                                break;
                if ( n < n2 )
                        if ( player == last )
                                done = FALSE;
                        else
                                if ( last >= n )
                                        for ( ; n < n2; ++n )
                                                if ( mu->mu_state[n]!=FINISHED )
                                                        done = ( n == last );
                if ( done )
                        return;

                if ( readdone )
                        delay();

                for ( count = 0; count < 25; ++count ) {
                        if ( !( dkerr = 0xFF00 & _dkio( 0x2, multdriv, multhead,
                                                        multtrk, multsect,
                                                        1, mu, dsseg() ) ) )
                                break;
                        errlog( dkerr, OREADPLAY );
                        _dkio( 0x0 );

                        if ( ( counttick > tickwait ) && ctlbreak() ) {
                                error( NULL, YES );
                                return;
                        }
                }
                _fromintel();
                if ( count == 25 ) {
                        error( "Read error during play", YES );
                        return;
                }
                if ( editnum() )
                        error( "Bad player count", YES );

                readdone = TRUE;
                ++curtry;
        }
#endif
}




static error( msg, flag1 )
char    *msg;
BOOL    flag1;
{

        errorflg = TRUE;
        errflg1 = flag1;
        errormsg = msg;
}




static  errlog( err, where )
unsigned err, where;
{
register int     i;

        errtab[i = ++errptr % 50] = err;
        errwhr[i] = where;
}




static  errrep()
{
register int     i;

        puts( "\r\n\r\n" );
        dispd( errptr + 1, 5 );
        puts( " i/o errors recorded\r\n\r\n" );
        if ( errptr >= 50 ) {
                puts( "last 50:\r\n\r\n" );
                for ( i = errptr % 50 + 1; i < 50; ++i )
                        errrepl( i );
                for ( i = 0; i <= ( errptr % 50 ); ++i )
                        errrepl( i );
        } else
                for ( i = 0; i <= errptr; ++i )
                        errrepl( i );
}



static  errrepl( i )
int     i;
{

        switch ( errwhr[i] ) {
                case OREAD:     puts( "READ      " );
                                break;
                case OWRITE:    puts( "WRITE     " );
                                break;
                case OREADPLAY: puts( "READPLAY  " );
                                break;
                case OWRITEPLAY:puts( "WRITEPLAY " );
                                break;
        }
        switch ( errtab[i] ) {
#ifdef IBMPC
                case 0x8000:    puts( "TIME OUT" );
                                break;
                case 0x4000:    puts( "BAD SEEK" );
                                break;
                case 0x2000:    puts( "BAD NEC" );
                                break;
                case 0x1000:    puts( "BAD CRC" );
                                break;
                case 0x0900:    puts( "DMA BOUNDARY " );
                                break;
                case 0x0800:    puts( "BAD DMA" );
                                break;
                case 0x0400:    puts( "RECORD NOT FOUND" );
                                break;
                case 0x0300:    puts( "WRITE PROTECT" );
                                break;
                case 0x0200:    puts( "BAD ADDR MARK " );
                                break;
                case 0x0100:    puts( "BAD COMMAND" );
                                break;
#endif
                default:        dispd( errtab[i], 6 );
                                break;
        }
        puts( "\r\n" );
}




static  unsigned nextdelay = 2;
static  unsigned maxtry    = 0;
static  unsigned mintry    = 10000;
static  unsigned numtry    = 0;
static  unsigned nummov    = 0;
static  unsigned numdel    = 0;
static  unsigned numadjup  = 0;
static  unsigned numadjdn  = 0;

static delay()
{
#ifdef IBMPC
register int     t;

        if ( multtick == -1 )
                return;

        if ( multtick )
                t = multtick;
        else
                t = nextdelay;

        numdel += t;
        _dktick( t );
#endif
}




delaymov()
{
        ++nummov;
        curtry = 0;
}



changedelay()
{
static  unsigned upcnt, dncnt;

        numtry += curtry;
        if ( curtry > maxtry )
                maxtry = curtry;
        if ( curtry && ( curtry < mintry ) )
                mintry = curtry;

        if ( curtry <= 1 ) {
                if ( ++dncnt == 3 ) {
                        if ( nextdelay > 0 ) {
                                --nextdelay;
                                ++numadjdn;
                        }
                        dncnt = 0;
                }
                upcnt = 0;
        } else {
                if ( ++upcnt == 3 ) {
                        if ( nextdelay < 5 ) {
                                ++nextdelay;
                                ++numadjup;
                        }
                        upcnt = 0;
                }
                dncnt = 0;
        }
}




delayrep()
{
        puts( "\r\nNumber of moves:        " );   dispd( nummov, 6 );
        puts( "\r\nNumber of read tries:   " );   dispd( numtry, 6 );
        puts( "\r\nMinimum tries/move:     " );   if ( mintry == 10000 )
                                                        dispd( 0, 6 );
                                                else
                                                        dispd( mintry, 6 );
        puts( "\r\nMaximum tries/move:     " );   dispd( maxtry, 6 );
        puts( "\r\nNumber of tick delays:  " );   dispd( numdel, 6 );
        puts( "\r\n# of tick adjusts up:   " );   dispd( numadjup, 6 );
        puts( "\r\n# of tick adjusts down: " );   dispd( numadjdn, 6 );
}




statrep()
{
register OBJECTS *ob;
register int     i;

        puts( "\n\r\n\r" );
        if ( editnum() ) {
                puts( "Bad player count" );
                return;
        }

        for ( i = 0; i < multbuff->mu_maxplyr; ++i ) {
                ob = &nobjects[i];
                puts( "\r\nPlayer: " );
                dispd( i, 2 );
                dispd( ob->ob_state, 3 );
                dispd( multbuff->mu_state[i], 3 );
                if ( i == multbuff->mu_lstplyr )
                        puts( "  <- last update " );
        }
}





editnum()
{
register char    max, num, lst;

        return( ( ( max = multbuff->mu_maxplyr ) < 0 )
                || ( max > MAX_PLYR )
                || ( ( num = multbuff->mu_numplyr ) < 0 )
                || ( num > max )
                || ( ( lst = multbuff->mu_lstplyr ) < 0 )
                || ( lst >= max ) );
}




init1mul( reset, device )
BOOL    reset;
char    *device;
{
#ifdef IBMPC
register MULTIO *mu;
register int    n;
int      tickwait, rc;


        if ( *device )
                *cmndfile = *multfile = *device;

        multfile[9] = '0' + getgame();

        multopen( cmndfile, multfile );
        _dkioset( multaddr );

        multstack = (int *)( multstck + 398 );

        for ( n = 0; n < 25; ++n ) {
                if ( ( rc = multread() ) >= 0 )
                        break;
                intsoff();
                tickwait = 18;
                counttick = 0;
                intson();
                while ( counttick < tickwait );
        }
        if ( !rc )
                swend( "Read error on communications file ", NO );
        if ( rc < 0 )
                swend( "Communications file locked ", NO );

        if ( reset )
                mulreset();

        mu = multbuff;
        if ( first = !mu->mu_numplyr ) {
                mu->mu_maxplyr = getmaxplyr();
                mu->mu_numplyr = 0;
                mu->mu_explseed = explseed;
        } else
                explseed = mu->mu_explseed;

        clrprmpt();
        currgame = &swgames[0];
        if ( mu->mu_numplyr >= mu->mu_maxplyr )
                swend( "Mamimum number of players already playing", NO );
        ++mu->mu_numplyr;
#endif
}



init2mul()
{
register OBJECTS *ob;
int              n;
register MULTIO  *mu;
BOOL             playinit = FALSE;

        mu = multbuff;
        for ( n = 0; n < mu->mu_maxplyr; ++n ) {
                if ( ( !playinit ) && ( mu->mu_state[n] == WAITING ) ) {
                        player = n;
                        initplyr( NULL );
                        mu->mu_key[n] = 0;
                        mu->mu_state[n] = FLYING;
                        mu->mu_lstplyr = n;
                        playinit = TRUE;
                } else {
                        ob = initpln( NULL );
                        ob->ob_drawf = dispmult;
                        ob->ob_movef = movemult;
                        ob->ob_clr = ob->ob_index % 2 + 1;
                        ob->ob_owner = ob;
                        ob->ob_state = mu->mu_state[n];
                        movmem( ob, &oobjects[ob->ob_index], sizeof( OBJECTS ) );
                }
        }
        if ( mu->mu_maxplyr % 2 )
                initcomp( NULL );

        if ( !multwrite() )
                swend( "Write error on communications file", NO );
        if ( !multunlock() )
                swend( "Unlock error on communications file", YES );

        clrprmpt();
        puts( "      Waiting for other player(s)" );
        multwait();
}




static getmaxplyr()
{
#ifdef IBMPC
register int     max;

        clrprmpt();
        puts( " Key maximum number of players allowed" );
        FOREVER {
                if ( ctlbreak() )
                        swend( NULL, NO );
                if ( ( ( max = ( swgetc() & 0x00FF ) - '0' ) >= 1 )
                    && ( max <=  MAX_PLYR  ) )
                        return( max );
}
#endif
}



static mulreset()
{
register int     i;

        multbuff->mu_maxplyr = 0;
        multbuff->mu_numplyr = 0;
        multbuff->mu_lstplyr = 0;
        multbuff->mu_explseed = 0;

        for ( i = 0; i < MAX_PLYR; ++i ) {
                multbuff->mu_key[i] = 0;
                multbuff->mu_state[i] = WAITING;
        }
}

