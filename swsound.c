/*
        swsound  -      SW sound generation

                        Copyright (C) 1984-2000 David L. Clark.

                        All rights reserved except as specified in the
                        file license.txt.  Distribution of this file
                        without the license.txt file accompanying is
                        prohibited.

                        Author: Dave Clark

        Modification History:
                        84-04-11        Development
                        87-03-10        Microsoft compiler.
*/
#include        "sw.h"



#define         TIMER   0x40
#define         PORTB   0x61
#define         SNDSIZE 100


extern  int     soundflg;               /*  Sound flag                      */
extern  int     dispdbg;                /*  Debug value to be displayed   */


static  int      soundtype = 32767;      /*  Current sound type and          */
static  int      soundparm = 32767;      /*     and priority parameter       */
static  OBJECTS  *soundobj =  NULL;      /*  Object making sound             */
static  unsigned lastfreq  = 0;          /*  Last frequency used             */
static  OBJECTS  *lastobj   = NULL;      /*  Previous object making sound    */
static  int      ( *toneadj ) () = NULL; /*  Tone adjustment on clock tick   */

static  TONETAB  tonetab[SNDSIZE];       /*  Continuous tone table           */
static  TONETAB  *frsttone, *freetone;   /*  Tone list and free list         */
static  unsigned soundticks;             /*  Ticks since last sound selection*/

static  int      numexpls;               /*  Number of explosions currently  */
                                         /*  active                          */
static  int      explplace;              /*  Place in explosion tune;        */
static  int      explline;               /* Line in explosion tune           */
static  unsigned expltone;               /*  Current explosion tone          */
static  int      explticks;              /*  Ticks until note change         */
static  int      exploctv;               /*  Octave                          */
static  char     *expltune[7] = {
       "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
       "e4/g8/g2/r16/>a8/<g8/e8/d2./",
       "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
       "e4/>a8/a2/r16/<g8/f+8/e8/d2./",
       "d8/g2/r16/g8/g+2/r16/g+8/>a2/r16/a8/c2/r16/",
       "b8/a8/<g8/>b4/<g8/>b4/<g8/>a4./<g1/",
       ""
};

static  BOOL     titleflg;               /* Playing title tune               */
static  int      titlplace;              /*  Place in title tune;            */
static  int      titlline;               /* Line in title tune               */
static  unsigned titltone;               /*  Current title tone              */
static  int      titlticks;              /*  Ticks until note change         */
static  int      titloctv;               /*  Octave                          */



static  char     **tune;                  /* Tune player statics              */
static  int      line;
static  int      place;
static  unsigned tunefreq;
static  int      tunedura;
static  int      octavefactor;




initsndt()
{
register TONETAB *tt;
register int     i;

        for ( i = 0, tt = tonetab; i < ( SNDSIZE - 1 ); ++i, ++tt )
                tt->tt_next = tt + 1;
        tt->tt_next = NULL;
        frsttone = NULL;
        freetone = tonetab;
}



sound( type, parm, ob )
int     type, parm;
OBJECTS *ob;
{

        if ( type < soundtype ) {
                soundtype = type;
                soundparm = parm;
                soundobj = ob;
        } else
                if ( ( type == soundtype ) && ( parm < soundparm ) ) {
                        soundparm = parm;
                        soundobj = ob;
                }
}




swsound()
{
unsigned          rand();
int               adjcont(),  adjshot();
register TONETAB  *tt;

        intsoff();
        tt = frsttone;
        while ( tt ) {
                tt->tt_tone += ( tt->tt_chng * soundticks );
                tt = tt->tt_next;
        }

        soundticks = 0;
        titleflg = FALSE;

        switch ( soundtype ) {

                case 0:
                case 32767:
                default:
                        soundoff();
                        lastobj = NULL;
                        toneadj = NULL;
                        break;

                case S_PLANE:
                        if ( soundparm == 0 )
                                tone( 0xF000 );
                        else
                                tone( 0xD000 + soundparm * 0x1000 );
                        lastobj = NULL;
                        toneadj = NULL;
                        break;

                case S_BOMB:
                        if ( soundobj == lastobj )
                                break;
                        toneadj = adjcont;
                        lastobj = soundobj;
                        adjcont();
                        break;

                case S_FALLING:
                        if ( soundobj == lastobj )
                                break;
                        toneadj = adjcont;
                        lastobj = soundobj;
                        adjcont();
                        break;

                case S_HIT:
                        tone( rand( 2 ) ? 0x9000 : 0xF000 );
                        lastobj = NULL;
                        toneadj = NULL;
                        break;

                case S_EXPLOSION:
                        tone( expltone );
                        toneadj = NULL;
                        lastobj = NULL;
                        break;

                case S_SHOT:
                        tone( 0x1000 );
                        toneadj = adjshot;
                        lastobj = NULL;
                        break;

                case S_TITLE:
                        titlline = 0;
                        titlplace = 0;
                        titlnote();
                        toneadj = NULL;
                        lastobj = NULL;
                        titleflg = TRUE;
                        break;

        }

        intson();
        soundtype = soundparm = 32767;
}





soundadj()
{

        ++soundticks;

        if ( lastfreq && toneadj )
                ( *toneadj )();

        if ( numexpls )
                adjexpl();

        if ( titleflg )
                adjtitl();
}




static  adjcont()
{
register TONETAB *tt;

        if ( tt = lastobj->ob_sound )
                tone( tt->tt_tone + tt->tt_chng * soundticks );
}




static  adjshot()
{
static  unsigned savefreq;

        if ( lastfreq == 0xF000 )
                tone( savefreq );
        else {
                savefreq = lastfreq;
                tone( 0xF000 );
        }
}




static adjexpl()
{
        if ( --explticks >= 0 )
                return;

        explnote();
}




static explnote()
{
        line = explline;
        place = explplace;
        tune = expltune;
        octavefactor = exploctv;
        playnote();
        explline = line;
        explplace = place;
        expltone = tunefreq;
        intsoff();
        explticks += tunedura;
        intson();
        exploctv = octavefactor;
}




static adjtitl()
{
        if ( --titlticks >= 0 )
                return;
        titlnote();
}




static titlnote()
{

        line = titlline;
        place = titlplace;
        tune = expltune;
        octavefactor = titloctv;
        playnote();
        titlline = line;
        titlplace = place;
        titltone = tunefreq;
        intsoff();
        titlticks += tunedura;
        intson();
        titloctv = octavefactor;
        soundoff();
        tone( titltone );
}





initsound( obp, type )
OBJECTS *obp;
int     type;
{
register OBJECTS *ob;
register TONETAB *tt;
TONETAB          *allocton();

        if ( ( ob = obp )->ob_sound )
                return;

        if ( ob->ob_type == EXPLOSION ) {
                intsoff();
                if ( ++numexpls == 1 ) {
                        explline = 0;
                        explplace = 0;
                        explnote();
                }
                ob->ob_sound = (struct tt *) 1;
                intson();
                return;
        }

        if ( tt = allocton() ) {
                intsoff();
                switch ( type ) {
                        case S_BOMB:
                                tt->tt_tone = 0x0300;
                                tt->tt_chng = 8;
                                break;
                        case S_FALLING:
                                tt->tt_tone = 0x1200;
                                tt->tt_chng = -8;
                                break;
                        default:
                                break;
                }
                ob->ob_sound = tt;
                intson();
                return;
        }
}




static  TONETAB *allocton()
{
register TONETAB *tt;

        if ( !freetone )
                return( 0 );

        tt = freetone;
        freetone = tt->tt_next;

        tt->tt_next = frsttone;
        tt->tt_prev = NULL;

        if ( frsttone )
                frsttone->tt_prev = tt;

        return( frsttone = tt );
}



static  deallton( ttp )
TONETAB *ttp;
{
register TONETAB *ttb, *tt;


        if ( ttb = ( tt = ttp )->tt_prev )
                ttb->tt_next = tt->tt_next;
        else
                frsttone = tt->tt_next;

        if ( ttb = tt->tt_next )
                ttb->tt_prev = tt->tt_prev;

        tt->tt_next = freetone;
        freetone = tt;
}








stopsound( ob )
OBJECTS *ob;
{
TONETAB *tt;

        if ( !( tt = ob->ob_sound ) )
                return;

        intsoff();
        if ( ob->ob_type == EXPLOSION )
                --numexpls;
        else
                deallton( tt );
        ob->ob_sound = NULL;
        intson();
}






static  tone( freq )
unsigned freq;
{

        if ( !soundflg )
                return;

        if ( lastfreq == freq )
                return;

#ifdef IBMPC
        if ( !lastfreq )
                outportb( TIMER + 3, 0xB6 );
        outportb( TIMER + 2, freq & 0x00FF );
        outportb( TIMER + 2, freq >> 8 );
        if ( !lastfreq )
                outportb( PORTB, 0x03 | inportb( PORTB ) );
#endif

        lastfreq = freq;
        dispdbg = freq;
}




soundoff()
{
        if ( lastfreq ) {
#ifdef IBMPC
                outportb( PORTB, 0xFC & inportb( PORTB ) );
#endif
                lastfreq = 0;
                dispdbg = 0;
        }
}





static  int       seed[50] = {
        0x90B9, 0xBCFB, 0x6564, 0x3313, 0x3190, 0xA980, 0xBCF0, 0x6F97,
        0x37F4, 0x064B, 0x9FD8, 0x595B, 0x1EEE, 0x820C, 0x4201, 0x651E,
        0x848E, 0x15D5, 0x1DE7, 0x1585, 0xA850, 0x213B, 0x3953, 0x1EB0,
        0x97A7, 0x35DD, 0xAF2F, 0x1629, 0xBE9B, 0x243F, 0x847D, 0x313A,
        0x3295, 0xBC11, 0x6E6D, 0x3398, 0xAD43, 0x51CE, 0x8F95, 0x507E,
        0x499E, 0x3BC1, 0x5243, 0x2017, 0x9510, 0x9865, 0x65F6, 0x6B56,
        0x36B9, 0x5026
};



static  unsigned  rand( modulo )
unsigned  modulo;
{
static    i = 0;

        if ( i >= 50 )
                i = 0;
        return( seed[i++] % modulo );
}







#define NOTEEND     '/'
#define UPOCTAVE    '>'
#define DOWNOCTAVE  '<'
#define SHARP       '+'
#define FLAT        '-'
#define DOT         '.'
#define REST        'R'





playnote()
  {

    static int noteindex[] = { 0,2,3,5,7,8,10 };
    static int notefreq[]  = {440,466,494,523,554,587,622,659,698,740,784,831};

    static int durplace, test, freq, duration;
    static int index;
    static int indexadj;

    static  char    durstring[5];
    static  char    charatplace, noteletter;

    static  int   noteoctavefactor;
    static  int   dottednote;

    BOOL          firstplace = TRUE;

    indexadj = 0;
    durplace = 0;
    dottednote = 2;
    noteoctavefactor = 256;

    FOREVER {
        if ( ( !line ) && ( !place ) )
                octavefactor = 256;

        if ( !( charatplace = toupper( tune[line][place++] ) ) ) {
                if ( !( charatplace = tune[++line][place = 0] ) ) {
                        line = 0;
                }

                if ( firstplace )
                        continue;
                break;
        }
        firstplace = FALSE;
        if ( charatplace == NOTEEND )
                break;

        if ( test = isalpha( charatplace )) {
            index = *(noteindex + (charatplace - 'A'));
            noteletter = charatplace;
        } else
            switch( charatplace ) {
                case UPOCTAVE : octavefactor <<= 1; break;
                case DOWNOCTAVE : octavefactor >>= 1; break;
                case SHARP    : indexadj++; break;
                case FLAT     : indexadj--; break;
                case DOT      : dottednote = 3; break;
                default       :
                        if ( test = isdigit(charatplace))
                            *(durstring + durplace++) = charatplace;
                        break;
            }

    }

    *(durstring + durplace) = '\0';
    duration = atoi( durstring );
    if (duration <= 0) duration = 4;
    duration = (1440 * dottednote / (60*duration)) >> 1;

    if (noteletter == REST)
        freq = 32000;
    else {
        index += indexadj;
        while (index < 0) {
            index += 12;
            noteoctavefactor >>= 1 ;
        }
        while ( index >= 12 ) {
            index -= 12;
            noteoctavefactor <<= 1 ;
        }
        freq = soundmul( *(notefreq+index), octavefactor, noteoctavefactor );
    }
    tunefreq = sounddiv( 1331000L, freq );
    tunedura = duration;
  }
