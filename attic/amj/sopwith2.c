/* Source code for Sopwith 2
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2000 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/
#include <ctype.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
//#include <dos.h>
//#include <alloc.h>
#include <string.h>
//#include <conio.h>
#include "sopasm.h"
#include "def.h"

#define huge 

struct snd
{
  int t2v,deltat2v;
  struct snd *next,*prev;
};

typedef struct snd snd;

struct object
{
  int status;
  int x,y,xv,yv,rotation;
  bool inverted;
  int kind;
  int speed,deltav,deltarot;
  bool firing;
  int score,ammo;
  int counter;
  int fuel;
  struct object *source;
  int height,width;
  bool bombing;
  int bombs,colour;
  unsigned int xfrac,yfrac,xvfrac,yvfrac;
  struct object *next,*prev;
  int index;
  int oldx,oldy;
  bool onscreen,oldonscreen;
  unsigned char *oldsprite;
  void (*soundfunc)(struct object *p);
  bool (*updatefunc)(struct object *p);
  struct object *nextx,*prevx;
  int deaths;
  unsigned char *sprite;
  int bombtime;
  bool homing;
  int type;
  bool ishome;
  snd *sndp;
};

typedef struct object object;

typedef struct
{
  int colour,x,y;
} mapobject;

typedef struct
{
  int *planeposes;
  bool *planeflip;
  int *buildingpositions;
  int *buildingtypes;
} landscape;

unsigned char writecharcol=3;
int regionleft[4]={0,1155,0,2089};
int regionright[4]={0,2088,1154,10000};
object *maxobjectp=NULL;
int latency=-1;
int sine[16]={0x000, 0x062, 0x0b5, 0x0ed, 0x100, 0x0ed, 0x0b5, 0x062,
              0x000,-0x062,-0x0b5,-0x0ed,-0x100,-0x0ed,-0x0b5,-0x062};
int keysacknowledged,keysnowpressed,keyspressed,joykeysacknowledged;
int joykeyspressed,joykeysnow;
bool keypressedflag;

int joykeys[9]={KEY_DESCEND,KEY_FLIP, KEY_DESCEND,
                KEY_BRAKE,  0,        KEY_ACCEL,
                KEY_DESCEND,KEY_CLIMB,KEY_DESCEND}; /* Weird. */

unsigned char scrground[320];
char enginepower[16]={0,-1,-2,-3,-4,-3,-2,-1,0,1,2,3,4,3,2,1};
int soundtype=0x7fff,soundpitch=0x7fff;
object *soundobj=NULL;
unsigned int t2v=0;
object *sndobj=NULL;
void (*soundperiodicfunc)(void)=NULL;
char *tune1[7]={
  "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
  "e4/g8/g2/r16/>a8/<g8/e8/d2./",
  "b4/d8/d2/r16/c8/b8/a8/b4./c4./c+4./d4./",
  "e4/>a8/a2/r16/<g8/f+8/e8/d2./",
  "d8/g2/r16/g8/g+2/r16/g+8/>a2/r16/a8/c2/r16/",
  "b8/a8/<g8/>b4/<g8/>b4/<g8/>a4./<g1/",
  ""};
int enginestutter[50]={ /* No idea what the more significant bits are */
  0x90b9,0xbcfb,0x6564,0x3313,0x3190,0xa980,0xbcf0,0x6f97,0x37f4,0x064b,
  0x9fd8,0x595b,0x1eee,0x820c,0x4201,0x651e,0x848e,0x15d5,0x1de7,0x1585,
  0xa850,0x213b,0x3953,0x1eb0,0x97a7,0x35dd,0xaf2f,0x1629,0xbe9b,0x243f,
  0x847d,0x313a,0x3295,0xbc11,0x6e6d,0x3398,0xad43,0x51ce,0x8f95,0x507e,
  0x499e,0x3bc1,0x5243,0x2017,0x9510,0x9865,0x65f6,0x6b56,0x36b9,0x5026};
int stutterp=0;
int majorscale[7]={0,2,3,5,7,8,10};
int notefreq[12]={440,466,494,523,554,587,622,659,698,740,784,831};

int singleplanes[2]={0,7};
int computerplanes[4]={0,7,1,6};
int multipleplanes[8]={0,7,3,4,2,5,1,6};
int flockx[4]={370,1370,1630,2630};
int flocky[4]={199,199,199,199};
int flockxv[4]={2,2,-2,-2};
int fscatterx[8]={8,3,0,6,7,14,10,12};
int fscattery[8]={16,1,8,3,12,10,7,14};
int fscatterxv[8]={-2,2,-3,3,-1,1,0,0};
int fscatteryv[8]={-1,-2,-1,-2,-1,-2,-1,-2};
int cowx[2]={1376,1608};
int cowy[2]={80,91};
int crater[8]={1,2,2,3,3,2,2,1};

#include "ground.c"
#include "sprites.c"

int worldplaneposes[8]={1270,588,1330,1360,1630,1660,2456,1720};
                                                            /* 2456 was 2464 */
bool worldplaneflip[8]={FALSE,FALSE,FALSE,FALSE,TRUE,TRUE,TRUE,TRUE};
int worldbuildingpositions[20]={
  191,284,409,539,685,807,934,1210,1240,1440,
  1550,1750,1780,2024,2159,2279,2390,2549,2678,2763};
int worldbuildingtypes[20]={
  BUILDING_CHIM,BUILDING_TANK,BUILDING_CHIM,BUILDING_CHIM,BUILDING_TANK,
  BUILDING_FLAG,BUILDING_CHIM,BUILDING_FUEL,BUILDING_FLAG,BUILDING_TANK,
  BUILDING_TANK,BUILDING_FLAG,BUILDING_FUEL,BUILDING_CHIM,BUILDING_CHIM,
  BUILDING_TANK,BUILDING_TANK,BUILDING_FLAG,BUILDING_FLAG,BUILDING_CHIM};

landscape worlds[1]={
  worldplaneposes,worldplaneflip,worldbuildingpositions,worldbuildingtypes};

int autodeltarot[3]={0,-1,1};
bool finishflag;

int huge *historybuf;
long historyspace;
FILE *outputfile;
int huge *historyp;
int historykey;
long historyfilelen;
bool recording;

int gamemode;
bool multiplayer;
landscape *world=NULL;
object *buildings[20];
int buildingc[2];

int timertick,frametick,speedtick;
int level,radarrange2;

unsigned char scrbuf[0x2000];

bool hiresflag;

bool notitlesflag;
bool soundflag,statsflag,joyrequested,ibmkeyboard,keynotascii=FALSE;

int scnleft,scnright,pixtoscroll;
unsigned int scrseg,scroff,interlacediff;
bool groundnotdrawn;

object *objects;
object homes[4];
object *lastobject,*firstobject,*nextobject,*firstdeleted,*lastdeleted;
object objleft,objright;
object *attackees[4];
int finaleflags[4],finaletime,lastplane,planeindex;
bool planeisplayer,planeisenemy;
mapobject mapobjects[100];
bool finaleflying,cratertobedrawn;
char *playbackfilename,*recordfilename;
int randno;
bool break1,break2;
int latencycount,finaleflag,maxlives;
jmp_buf startjbuf;

bool exiting=FALSE;
snd sndlist[100];
snd *lastsnd,*nextsnd;
int slidec,numshrapnelsnd,tuneptr2,tuneline2;
unsigned int t2v2;
int tunetime2,octavemul2;
bool tune3f;
int tuneptr3,tuneline3;
unsigned int t2v3;
int tunetime3,octavemul3;
char **tune;
int tuneline,tuneptr,t2v1,tunedur,octavemultiplier;
int ot2v;

bool deceased;
int score;
object *collobj1[200];
object *collobj2[200];
int collc;
int collxv[4],collyv[4];
object *collobjs[4];
int colln,collxvr,collyvr;
unsigned char workingground[3000];
object tplane;
int autoheight;
int autod2[3];
bool autodoomed[3];
int autoalt[3];
int leftbuilding,rightbuilding;

void main(int argc,char *argv[]);
void keyboardhandler(void);
int inkeys(void);
void updatejoy(void);
int readjoychannel(int channel);
void flushbuf(void);
void setcolour(int c);
void updatescreen(void);
void updateground(void);
void drawground(unsigned char *scrground);
void clearscorearea(void);
void drawobject(int x,int y,object *obj);
void putpixel(int x,int y,int c);
int bitblt(int x,int y,object *obj);
int pixel(int x,int y,int c);
void putimage(int x,int y,unsigned char *p,object *obj);
void updateobjects(void);
bool updateplayerplane(object *plane);
void processkeys(object *p,int keys);
bool updateenemyplane(object *plane);
bool updateplane(object *plane);
void attackable(object *obj);
void refuel(object *plane);
bool increment(int *val,int max);
bool updatebullet(object *bullet);
bool updatebomb(object *bomb);
int direction(object *obj);
bool updatebuilding(object *building);
bool updateshrapnel(object *shrapnel);
bool updatesmoke(object *smoke);
bool updateflock(object *flock);
bool updatebird(object *bird);
bool updatecow(object *cow);
void killplane(object *plane);
void catchfire(object *plane);
void gointospin(object *plane);
void insxlist(object *ins,object *list);
void delxlist(object *del);
void initsndlist(void);
void initsound(int type,int pitch,object *obj);
void updatesound(void);
void updatesoundint(void);
void soundslide(void);
void soundtoggle(void);
void updatetune2(void);
void setsound2(void);
void updatetune3(void);
void sound3(void);
void objectsound(object *obj,int sn);
snd *createsnd(void);
void destroysnd(snd *s);
void killsnd(object *o);
void ssound(unsigned int t2);
void snosound(void);
int getenginestutter(int f);
void updatetune(void);
void init(int argc,char *argv[]);
void initrand(void);
void restart(void);
void initdifficulty(void);
void clearwin(void);
void getcontrol(void);
int getgamemode(void);
int getgamenumber(void);
bool testbreak(void);
void copyground(void);
void initscreen(bool drawnground);
void writescores(void);
void livesbar(object *p);
void fuelbar(object *p);
void bombsbar(object *p);
void ammobar(object *p);
void statbar(int x,int h,int hmax,int col);
void drawmapground(void);
void drawmapbuildings(void);
void useoffscreenbuf(void);
void useonscreenbuf(void);
void copyoffscbuftoscn(void);
void clearscrbuf(void);
void initlists(void);
void createenemyplane(object *pl);
void createplayerplane(object *player);
object *createplane(object *pl);
void firebullet(object *obj,object *at);
int distance(int x0,int y0,int x1,int y1);
void dropbomb(object *plane);
void createbuildings(void);
void createexplosion(object *obj);
void createsmoke(object *plane);
void createflocks(void);
void createbird(object *flock,int birdno);
void createcows(void);
void checkcollisions(void);
void checkcollision(object *obj1,object *obj2);
void checkcrash(object *obj);
void docollision(object *obj1,object *obj2);
bool collidescore(int type,object *obj,int score);
void addscore(object *destroyed,int score);
void score50(object *destroyed);
void writescore(object *p);
void digcrater(object *obj);
void titles(void);
void soundoff(void);
void timerint(void);
void soundbomb(object *bomb);
void soundshrapnel(object *shrapnel);
void soundbuilding(object *building);
void soundplane(object *plane);
void drawmapobject(object *obj);
void finish(char *msg,bool closef);
void declarewinner(int n);
void createsun(object *player);
void gameover(object *plane);
void eogstats(void);
object *newobject(void);
void deleteobject(object *obj);
void enemylogic(object *plane);
void enemyattack(object *plane,object *attackee);
void enemyflyhome(object *plane);
void flyhome(object *plane);
bool ontarget(object *target);
void autopilot(object *plane,int destx,int desty,object *destobj,bool recursing);
void initbuildingcheck(int x,int y);
bool doomed(int x,int y,int alt);
int distsqr(int x0,int y0,int x1,int y1);
int getmaxplayers(void);
bool updateremoteplane(object *plane);
int inithistory(int randno);
void freehistorybuf(void);
void freeandexit(char *errmsg);
int history(int key);
void flushhistory(void);
void initcomms(void);
char *finishcomms(bool closef);
char *finishserial(void);
int getasynchkeys(object *plane);
int getmultiplekeys(object *plane);
void setvel(object *obj,int v,int dir);
void printstr(char *str);
void moveobject(object *obj,int *x,int *y);
void getopt(int *argc,char **argv[],char *format,...);
void writenum(int n,int width);
void createmultipleremoteplanes(void);
void initasynch(void);
void createasynchremoteplane(void);
int inkey(void);

void main(int argc,char *argv[])
{
  objects=(object *)malloc(100*sizeof(object));
  if (objects==NULL) {
    printf("Cannot allocate memory for object list.\n");
    exit(1);
  }
  delay(0);
  init(argc,argv);
  setjmp(startjbuf);
  while (1) {
    while (speedtick<2);
    speedtick=0;
/*    delay(10000); */
    updateobjects();
    updatejoy();
    updatescreen();
    updatejoy();
    checkcollisions();
    updatejoy();
    updatesound();
  }
}

void keybint(void)
{
  int scancode,k;
  keypressedflag=TRUE;
  if (!ibmkeyboard)
    return;
  scancode=inportb(PORT_KEYB);
  switch (scancode&0x7f) {
    case SC_X:     k=KEY_ACCEL;   break;
    case SC_Z:     k=KEY_BRAKE;   break;
    case SC_COMMA: k=KEY_CLIMB;   break;
    case SC_SLASH: k=KEY_DESCEND; break;
    case SC_DOT:   k=KEY_FLIP;    break;
    case SC_SPACE: k=KEY_FIRE;    break;
    case SC_B:     k=KEY_BOMB;    break;
    case SC_H:     k=KEY_HOME;    break;
    case SC_S:     k=KEY_SOUND;   break;
    case SC_BREAK: k=KEY_BREAK;   break;
    default: k=0;
  }
  if (k!=0)
    if (scancode&0x80) {
      if (k&keysacknowledged)
        keyspressed&=~k;
      keysnowpressed&=~k;
    }
    else {
      keysnowpressed|=k;
      keyspressed|=k;
    }
}

int inkeys(void)
{
  int k;
  if (!keynotascii)
    return history(inkey());
  if (ibmkeyboard) {
    k=keysacknowledged=keyspressed;
    keyspressed=keysnowpressed;
    while (kbhit()) getch();
  }
  else {
    switch(inkey()) {
      case 'X': case 'x': k=KEY_ACCEL;   break;
      case 'Z': case 'z': k=KEY_BRAKE;   break;
      case '<': case ',': k=KEY_CLIMB;   break;
      case '?': case '/': k=KEY_DESCEND; break;
      case '>': case '.': k=KEY_FLIP;    break;
      case ' ':           k=KEY_FIRE;    break;
      case 'B': case 'b': k=KEY_BOMB;    break;
      case 'H': case 'h': k=KEY_HOME;    break;
      case 'S': case 's': k=KEY_SOUND;   break;
      default: k=0;
    }
    if (testbreak())
      k|=KEY_BREAK;
  }
  if (joyrequested) {
    joykeysacknowledged=joykeysnow;
    joykeysnow=joykeyspressed;
    k|=joykeysacknowledged;
  }
  return history(k);
}

int inkey(void)
{
  if (kbhit())
    return getch();
  return 0;
}

void updatejoy(void)
{
  int x,y,j,r;
  if (joyrequested) {
    x=readjoychannel(0);
    y=readjoychannel(1);
    j=joykeys[(x<=640 ? 0 : (x>=1920 ? 2 : 1))+
              (y<=640 ? 0 : (y>=1920 ? 6 : 3))];
    r=inportb(PORT_JOY);     /* Joystick */
    if ((r&JOY_BUT1)==0)
      j|=KEY_FIRE;
    if ((r&JOY_BUT2)==0)
      j|=KEY_BOMB;
    joykeyspressed=j;
    joykeysnow|=j;
    joykeysnow&=j|~joykeysacknowledged;
  }
}

int readjoychannel(int channel)
{
  int t;
  if ((inportb(PORT_JOY)&(1<<channel))!=0)
    return 2560;
  keypressedflag=FALSE;
  outportb(PORT_TIMERC,0);
  t=getwordfromport(PORT_TIMER0);
  outportb(PORT_JOY,0);
  while ((inportb(PORT_JOY)&(1<<channel))!=0);
  if (keypressedflag)
    return 640;
  return t-getwordfromport(PORT_TIMER0);
}

void flushbuf(void)
{
  if (!keynotascii || !ibmkeyboard)
    while (kbhit())
      getch();
}

void setcolour(int c)
{
  writecharcol=c;
}

void updatescreen(void)
{
  object *obj;
  useonscreenbuf();
  for (obj=firstobject;obj!=NULL;obj=obj->next)
    if (obj->oldonscreen && obj->onscreen && obj->height!=1 &&
        obj->oldsprite==obj->sprite && obj->y==obj->oldy &&
        obj->oldx+scnleft==obj->x) {
      if (obj->soundfunc!=NULL)
        obj->soundfunc(obj);
    }
    else {
      if (obj->oldonscreen)
        putimage(obj->oldx,obj->oldy,obj->oldsprite,obj);
      if (!obj->onscreen)
        continue;
      if (obj->x>=scnleft && obj->x<=scnright) {
        putimage(obj->oldx=obj->x-scnleft,obj->oldy=obj->y,obj->sprite,obj);
        if (obj->soundfunc!=NULL)
          obj->soundfunc(obj);
      }
      else
        obj->onscreen=FALSE;
    }
  for (obj=firstdeleted;obj!=NULL;obj=obj->next)
    if (obj->oldonscreen)
      putimage(obj->oldx,obj->oldy,obj->oldsprite,obj);
  updateground();
}

void updateground(void)
{
  if (!groundnotdrawn) {
    if (pixtoscroll==0 && !cratertobedrawn)
      return;
    drawground(scrground);
  }
  memcpy(scrground,workingground+scnleft,320);
  drawground(scrground);
  groundnotdrawn=FALSE;
  cratertobedrawn=FALSE;
}

void drawground(unsigned char *scrground)
{
  int y=*scrground,y2;
  int x;
  for (x=0;x<320;x++) {
    y2=*(scrground++);
    if (y2==y)
      putpixel(x,y,131);
    else
      if (y2>y)
        do
          putpixel(x,y++,131);
        while (y!=y2);
      else
        do
          putpixel(x,y--,131);
        while (y!=y2);
  }
}

void updateobjects(void)
{
  object *current,*next;
  if (lastdeleted!=NULL) {
    lastdeleted->next=nextobject;
    nextobject=firstdeleted;
    lastdeleted=firstdeleted=NULL;
  }
  latencycount++;
  if (latencycount>=latency)
    latencycount=0;
  current=firstobject;
  while (current!=NULL) {
    next=current->next;
    current->oldonscreen=current->onscreen;
    current->oldsprite=current->sprite;
    current->onscreen=current->updatefunc(current);
    current=next;
  }
  frametick++;
}

bool updateplayerplane(object *plane)
{
  bool result;
  int x,keys;
  planeisenemy=FALSE;
  planeisplayer=TRUE;
  planeindex=plane->index;
  finaleflag=finaleflags[lastplane];
  if (finaleflag!=FINALE_NONE) {
    finaletime--;
    if (finaletime<=0) {
      if (gamemode!=GAME_MULTIPLE && !exiting)
        restart();
      finish(NULL,TRUE);
    }
  }
  if (latencycount==0) {
    if (gamemode==GAME_MULTIPLE)
      keys=getmultiplekeys(plane);
    else {
      if (gamemode==GAME_ASYNCH)
        keys=getasynchkeys(plane);
      else {
        keys=inkeys();
        flushbuf();
      }
    }
    processkeys(plane,keys);
  }
  else {
    plane->deltarot=0;
    plane->bombing=FALSE;
  }
  if ((plane->status==STATUS_DEAD || plane->status==STATUS_ELIMDEAD) &&
      plane->counter<=0) {
    plane->deaths++;
    if (finaleflag!=FINALE_WON &&
        (plane->fuel<-5000 || (!multiplayer && plane->deaths>=5))) {
      if (finaleflag==FINALE_NONE)
        gameover(plane);
    }
    else {
      createplayerplane(plane);
      initscreen(TRUE);
      if (exiting) {
        if (testbreak())
          finish(NULL,TRUE);
        createsun(plane);
      }
    }
  }
  x=plane->x;
  result=updateplane(plane);
  if (x<=180 || x>=2820)
    pixtoscroll=0;
  else {
    pixtoscroll=plane->x-x;
    scnleft+=pixtoscroll;
    scnright+=pixtoscroll;
  }
  if (!plane->ishome) {
    useonscreenbuf();
    if (plane->firing)
      ammobar(plane);
    if (plane->bombing)
      bombsbar(plane);
  }
  return result;
}

void processkeys(object *p,int keys)
{
  int status=p->status;
  p->deltarot=0;
  p->firing=FALSE;
  p->bombing=FALSE;
  if (status!=STATUS_NORMAL && status!=STATUS_SPINNING &&
      status!=STATUS_FALLING && status!=STATUS_ELIMNORM &&
      status!=STATUS_ELIMSPIN)
    return;
  if (status!=STATUS_FALLING) {
    if (finaleflag!=FINALE_NONE) {
      if (finaleflag==FINALE_LOST && planeisplayer)
        flyhome(p);
      return;
    }
    if ((keys&KEY_BREAK)!=0) {
      p->fuel=-5000;
      p->homing=FALSE;
      if (p->ishome) {
        status=(status>=STATUS_ELIMINATED) ? STATUS_ELIMDEAD : STATUS_DEAD;
        p->status=status;
        p->counter=0;
      }
      if (planeisplayer)
        exiting=TRUE;
    }
    if ((keys&KEY_HOME)!=0 &&
        (status==STATUS_NORMAL || status==STATUS_ELIMNORM))
      p->homing=TRUE;
  }
  if ((keys&KEY_CLIMB)!=0) {
    p->deltarot++;
    p->homing=FALSE;
  }
  if ((keys&KEY_DESCEND)!=0) {
    p->deltarot--;
    p->homing=FALSE;
  }
  if ((keys&KEY_FLIP)!=0) {
    p->inverted=!p->inverted;
    p->homing=FALSE;
  }
  if ((keys&KEY_BRAKE)!=0) {
    if (p->deltav!=0)
      p->deltav--;
    p->homing=FALSE;
  }
  if ((keys&KEY_ACCEL)!=0) {
    if (p->deltav<4)
      p->deltav++;
    p->homing=FALSE;
  }
  if ((keys&KEY_FIRE)!=0 && status<STATUS_ELIMINATED)
    p->firing=TRUE;
  if ((keys&KEY_BOMB)!=0 && status<STATUS_ELIMINATED)
    p->bombing=TRUE;
  if ((keys&KEY_SOUND)!=0 && planeisplayer) {
    if (soundflag) {
      initsound(SOUND_OFF,0,NULL);
      updatesound();
    }
    soundflag=!soundflag;
  }
  if (p->homing)
    flyhome(p);
}

bool updateenemyplane(object *plane)
{
  planeisenemy=TRUE;
  planeisplayer=FALSE;
  plane->deltarot=0;
  plane->bombing=FALSE;
  planeindex=plane->index;
  finaleflag=finaleflags[planeindex];
  if (latencycount==0)
    plane->firing=FALSE;
  switch (plane->status) {
    case STATUS_NORMAL:
    case STATUS_SPINNING:
      if (finaleflag!=FINALE_NONE)
        flyhome(plane);
      else
        if (latencycount==0)
          enemylogic(plane);
      break;
    case STATUS_DEAD:
      plane->firing=FALSE;
      if (plane->counter<=0)
        if (finaleflag==FINALE_NONE)
          createenemyplane(plane);
      break;
    default:
      plane->firing=FALSE;
  }
  return updateplane(plane);
}

bool updateplane(object *plane)
{
  int rotation,speed,x,y,status,newstatus,idealspeed,bp_4;
  bool spinning;
  status=plane->status;
  switch(status) {
    case STATUS_ELIMINATED:
    case STATUS_UNKNOWN:
      return FALSE;
    case STATUS_DEAD:
    case STATUS_ELIMDEAD:
      plane->counter--;
      break;
    case STATUS_FALLING:
      plane->counter-=2;
      if (plane->yv<0 && plane->xv!=0)
        if (((plane->inverted ? 1 : 0)^(plane->xv<0 ? 1 : 0))!=0)
          plane->counter-=plane->deltarot;
        else
          plane->counter+=plane->deltarot;
      if (plane->counter<=0) {
        if (plane->yv<0) {
          if (plane->xv<0)
            plane->xv++;
          else
            if (plane->xv>0)
              plane->xv--;
            else
              plane->inverted=!plane->inverted;
        }
        if (plane->yv>-10)
          plane->yv--;
        plane->counter=10;
      }
      plane->rotation=direction(plane)<<1;
      if (plane->yv<=0)
        objectsound(plane,SOUND_FALLING);
      break;
    case STATUS_SPINNING:
    case STATUS_ELIMSPIN:
    case STATUS_NORMAL:
    case STATUS_ELIMNORM:
      if (status==STATUS_SPINNING)
        newstatus=STATUS_NORMAL;
      if (status==STATUS_ELIMSPIN)
        newstatus=STATUS_ELIMNORM;
      if (status==STATUS_SPINNING || status==STATUS_ELIMSPIN) {
        spinning=(plane->rotation!=12 || plane->speed<level+4);
        if (!spinning)
          plane->status=status=newstatus;
      }
      else {
        spinning=(plane->y>=200);
        if (spinning) {
          gointospin(plane);
          status=plane->status;
        }
      }
      if (finaleflying)
        break;
      if (plane->fuel<=0 && !plane->ishome &&
          (status==STATUS_NORMAL || status==STATUS_SPINNING)) {
        catchfire(plane);
        score50(plane);
        return updateplane(plane);
      }
      rotation=plane->rotation;
      speed=plane->speed;
      /* bp_4=0; */  /* I think this may be a bug - I think it should be before
                        the if (frametick&3==0) line */
      bp_4=plane->deltarot;
      if (bp_4!=0) {
        if (plane->inverted)
          rotation-=bp_4;
        else
          rotation+=bp_4;
        rotation&=15;
      }
      if ((frametick&3)==0)
        if (!spinning && speed<level+4) {
          speed--;
          bp_4=1;
        }
        else {
          idealspeed=enginepower[rotation]+plane->deltav+level+4;
          if (speed<idealspeed) {
            speed++;
            bp_4=1;
          }
          else
            if (speed>idealspeed) {
              speed--;
              bp_4=1;
            }
        }
      if (bp_4!=0) {
        if (plane->ishome) {
          if (plane->deltav || plane->deltarot)
            speed=level+4;
          else
            speed=0;
        }
        else
          if (speed<=0 && !spinning) {
            gointospin(plane);
            return updateplane(plane);
          }
        plane->speed=speed;
        plane->rotation=rotation;
        if (spinning) {
          plane->yvfrac=plane->xvfrac=plane->xv=0;
          plane->yv=-speed;
        }
        else
          setvel(plane,speed,rotation);
      }
      if (spinning) {
        plane->counter--;
        if (plane->counter==0) {
          plane->inverted=!plane->inverted;
          plane->rotation=(8-plane->rotation)&15;
          plane->counter=6;
        }
      }
      if (plane->firing)
        firebullet(plane,NULL);
      if (plane->bombing)
        dropbomb(plane);
      if (!planeisenemy) {
        if (planeisplayer && plane->speed>plane->fuel%900) {
          useonscreenbuf();
          fuelbar(plane);
        }
        plane->fuel-=plane->speed;
      }
      if (plane->speed!=0)
        plane->ishome=FALSE;
  }
  if (finaleflag==FINALE_WON && planeisplayer && finaleflying)
    plane->sprite=finalesprites[0][finaletime/18];
  else
    if (plane->status==STATUS_ELIMINATED)
      plane->sprite=NULL;
    else
      if (plane->status==STATUS_FALLING && plane->xv==0 && plane->yv<0)
        plane->sprite=fallingsprites[plane->inverted ? 1 : 0];
      else
        plane->sprite=planesprites[0][plane->inverted ? 1 : 0][plane->rotation];
  moveobject(plane,&x,&y);
  if (x<0)
    plane->x=x=0;
  else
    if (x>=2984)
      plane->x=x=2984;
  if (!planeisenemy &&
      (plane->status==STATUS_NORMAL || plane->status==STATUS_SPINNING) &&
      finaleflags[lastplane]==FINALE_NONE)
    attackable(plane);
  delxlist(plane);
  insxlist(plane,plane->nextx);
  if (plane->bombtime)
    plane->bombtime--;
  if (!planeisenemy && plane->ishome && plane->status==STATUS_NORMAL)
    refuel(plane);
  if (y<200 && y>=0) {
    if (plane->status==STATUS_FALLING)
      createsmoke(plane);
    useonscreenbuf();
    drawmapobject(plane);
    if (!planeisplayer)
      return (plane->status<STATUS_ELIMINATED);
    return TRUE;
  }
  return FALSE;
}

void attackable(object *obj)
{
  object *plane,*attackee;
  int i,x=obj->x,colour=obj->source->colour;
  for (plane=firstobject+1,i=1;plane->type==OBJ_PLANE;plane++,i++)
    if (plane->source->colour!=colour && plane->updatefunc==updateenemyplane &&
        (gamemode!=GAME_COMPUTER || (regionleft[i]<=x &&
                                     regionright[i]>=x))) {
      attackee=attackees[i];
      if (attackee==NULL || abs(x-plane->x)<abs(attackee->x-plane->x))
        attackees[i]=obj;
    }
}

void refuel(object *plane)
{
  useonscreenbuf();
  if (increment(&(plane->fuel),9000))
    fuelbar(plane);
  if (increment(&(plane->ammo),200))
    ammobar(plane);
  if (increment(&(plane->bombs),5))
    bombsbar(plane);
}

bool increment(int *val,int max)
{
  bool redraw=FALSE;
  if (*val==max)
    return redraw;
  if (max<20) {
    if (frametick%20==0) {
      (*val)++;
      redraw=planeisplayer;
    }
  }
  else {
    *val+=max/100;
    redraw=planeisplayer;
  }
  if (*val>max)
    *val=max;
  return redraw;
}

bool updatebullet(object *bullet)
{
  int x,y;
  delxlist(bullet);
  bullet->fuel--;
  if (bullet->fuel==0) {
    deleteobject(bullet);
    return FALSE;
  }
  moveobject(bullet,&x,&y);
  if (x<0 || x>=3000 || y<=workingground[x] || y>=200) {
    deleteobject(bullet);
    return FALSE;
  }
  insxlist(bullet,bullet->nextx);
  bullet->sprite=(unsigned char *)131;
  return TRUE;
}

bool updatebomb(object *bomb)
{
  int x,y;
  delxlist(bomb);
  if (bomb->fuel<0) {
    deleteobject(bomb);
    bomb->status=STATUS_ELIMINATED;
    useonscreenbuf();
    drawmapobject(bomb);
    return FALSE;
  }
  bomb->fuel--;
  if (bomb->fuel==0) {
    if (bomb->yv<0)
      if (bomb->xv<0)
        bomb->xv++;
      else
        if (bomb->xv>0)
          bomb->xv--;
    if (bomb->yv>-10)
      bomb->yv--;
    bomb->fuel=5;
  }
  if (bomb->yv<=0)
    objectsound(bomb,SOUND_BOMB);
  moveobject(bomb,&x,&y);
  if (x<0 || x>=3000 || y<0) {
    deleteobject(bomb);
    killsnd(bomb);
    bomb->status=STATUS_ELIMINATED;
    useonscreenbuf();
    drawmapobject(bomb);
    return FALSE;
  }
  if (y>=200) {
    insxlist(bomb,bomb->nextx);
    return FALSE;
  }
  insxlist(bomb,bomb->nextx);
  bomb->sprite=bombsprites[0][direction(bomb)];
  useonscreenbuf();
  drawmapobject(bomb);
  return TRUE;
}

int direction(object *obj)
{
  int xv=obj->xv,yv=obj->yv;
  if (yv>0)  { if (xv<0) return 3; if (xv==0) return 2; return 1; }
  if (yv==0) { if (xv<0) return 4; if (xv==0) return 6; return 0; }
               if (xv<0) return 5; if (xv==0) return 6; return 7;
}

bool updatebuilding(object *building)
{
  int d2;
  object *at=firstobject;
  building->firing=FALSE;
  if (level!=0 && building->status==STATUS_INTACT &&
      (at->status==STATUS_NORMAL || at->status==STATUS_SPINNING) &&
      (building->colour!=at->colour) && (level>1 || frametick&1)) {
    d2=distsqr(building->x,building->y,at->x,at->y);
    if (d2>0 && d2<radarrange2) {
      building->firing=TRUE; /* was at */
      firebullet(building,at);
    }
  }
  building->counter--;
  if (building->counter<0)
    building->counter=0;
  if (building->status==STATUS_INTACT)
    building->sprite=buildingsprites[0][building->kind];
  else
    building->sprite=debrissprites[0];
  return TRUE;
}

bool updateshrapnel(object *shrapnel)
{
  int x,y,kind;
  kind=shrapnel->kind;
  delxlist(shrapnel);
  if (shrapnel->fuel<0 && kind!=0) {
    killsnd(shrapnel);
    deleteobject(shrapnel);
    return FALSE;
  }
  shrapnel->fuel--;
  if (shrapnel->fuel==0) {
    if (shrapnel->yv<0)
      if (shrapnel->xv<0)
        shrapnel->xv++;
      else
        if (shrapnel->xv>0)
          shrapnel->xv--;
    if ((shrapnel->kind!=0 && shrapnel->yv>-10) ||
        (shrapnel->kind==0 && shrapnel->yv>-(level+4)))
      shrapnel->yv--;
    shrapnel->fuel=3;
  }
  moveobject(shrapnel,&x,&y);
  if (x<0 || x>=3000 || y<=workingground[x]) {
    if (kind!=0)
      killsnd(shrapnel);
    deleteobject(shrapnel);
    return FALSE;
  }
  shrapnel->counter++;
  if (y>=200) {
    insxlist(shrapnel,shrapnel->nextx);
    return FALSE;
  }
  insxlist(shrapnel,shrapnel->nextx);
  shrapnel->sprite=shrapnelsprites[shrapnel->kind];
  return TRUE;
}

bool updatesmoke(object *smoke)
{
  smoke->fuel--;
  if (smoke->fuel==0 ||
      (smoke->source->status!=STATUS_FALLING &&
       smoke->source->status!=STATUS_DEAD)) {
    deleteobject(smoke);
    return FALSE;
  }
  smoke->sprite=(unsigned char *)(smoke->colour+128);
  return TRUE;
}

bool updateflock(object *flock)
{
  int x,y;
  delxlist(flock);
  if (flock->fuel==-1) {
    useonscreenbuf();
    drawmapobject(flock);
    deleteobject(flock);
    return FALSE;
  }
  flock->fuel--;
  if (flock->fuel==0) {
    flock->inverted=!flock->inverted;
    flock->fuel=5;
  }
  if (flock->x<370 || flock->x>2630)
    flock->xv=-flock->xv;
  moveobject(flock,&x,&y);
  insxlist(flock,flock->nextx);
  flock->sprite=flocksprites[flock->inverted ? 1 : 0];
  useonscreenbuf();
  drawmapobject(flock);
  return TRUE;
}

bool updatebird(object *bird)
{
  int x,y;
  delxlist(bird);
  if (bird->fuel==-1) {
    deleteobject(bird);
    return FALSE;
  }
  if (bird->fuel==-2) {
    bird->yv=-bird->yv;
    bird->xv=(frametick&7)-4;
    bird->fuel=4;
  }
  else {
    bird->fuel--;
    if (bird->fuel==0) {
      bird->inverted=!bird->inverted;
      bird->fuel=4;
    }
  }
  moveobject(bird,&x,&y);
  if (x<0 || x>=3000 || y<=workingground[x] || y>=200) {
    bird->y-=bird->yv;
    bird->fuel=-2;
    insxlist(bird,bird->nextx);
    return FALSE;
  }
  insxlist(bird,bird->nextx);
  bird->sprite=birdsprites[bird->inverted ? 1 : 0];
  return TRUE;
}

bool updatecow(object *cow)
{
  cow->sprite=cowsprites[cow->status==STATUS_INTACT ? 0 : 1];
  return TRUE;
}

void declarewinner(int n)
{
  int colour;
  object *player;
  if (multiplayer /* && netinf->maxplayers!=1 */)
    if (firstobject[1].score==firstobject[0].score)
      colour=3-n;
    else
      colour=(firstobject[1].score>firstobject[0].score ? 1 : 0)+1;
  else
    colour=1;
  for (player=firstobject;player->type==OBJ_PLANE;player=player->next)
    if (finaleflags[player->index]==FINALE_NONE) {
      if (player->colour==colour && (player->deaths<4 || (player->deaths<5 &&
          (player->status==STATUS_NORMAL || player->status==STATUS_SPINNING))))
        createsun(player);
      else
        gameover(player);
    }
}

void createsun(object *player)
{
  int index;
  finaleflags[index=player->index]=FINALE_WON;
  if (index==lastplane) {
    finaletime=72;
    finaleflying=TRUE;
    player->yvfrac=0;
    player->xvfrac=0;
    player->yv=0;
    player->xv=0;
    player->status=STATUS_NORMAL;
    player->fuel=9000;
    player->speed=4;
  }
}

void gameover(object *plane)
{
  int player=plane->index;
  finaleflags[player]=FINALE_LOST;
  if (player==lastplane) {
    setcolour(130);
    poscurs(16,12);
    printstr("THE END");
    finaletime=20;
  }
}

void enemylogic(object *plane)
{
  object *attackee=attackees[planeindex];
  if (attackee!=NULL)
    enemyattack(plane,attackee);
  else
    if (!plane->ishome)
      enemyflyhome(plane);
  attackees[planeindex]=NULL;
}

void enemyattack(object *plane,object *attackee)
{
  autoheight=((frametick&31)<16 ? 16 : 0);
  if (attackee->speed!=0)
    autopilot(plane,attackee->x-(fcos(32,attackee->rotation)>>8),
              attackee->y-(fsin(32,attackee->rotation)>>8),attackee,FALSE);
  else
    autopilot(plane,attackee->x,attackee->y+4,attackee,FALSE);
}

void enemyflyhome(object *plane)
{
  int x;
  autoheight=((frametick&31)<16 ? 16 : 0);
  x=homes[planeindex].x;
  autopilot(plane,autoheight+(x<1000 ? 1000 : (x>2000 ? 2000 : x)),
            150-(autoheight>>1),NULL,FALSE);
}

void flyhome(object *plane)
{
  object *home;
  if (plane->ishome)
    return;
  home=&homes[planeindex];
  autoheight=((frametick&31)>16 ? 16 : 0);
  if (abs(plane->x-home->x)<16 && abs(plane->y-home->y)<16) {
    if (planeisplayer) {
      createplayerplane(plane);
      initscreen(TRUE);
    }
    else
      if (planeisenemy)
        createenemyplane(plane);
      else
        createplane(plane);
    return;
  }
  autopilot(plane,home->x,home->y,NULL,FALSE);
}

bool ontarget(object *target)
{
  object tbullet,ttarget;
  int bulletx,bullety,targetx,targety,d2,i;
  memcpy(&tbullet,&tplane,sizeof(object));
  memcpy(&ttarget,target,sizeof(object));
  setvel(&tbullet,tbullet.speed+10,tbullet.rotation);
  tbullet.x+=8;
  tbullet.y-=8;
  for (i=0;i<10;i++) {
    moveobject(&tbullet,&bulletx,&bullety);
    moveobject(&ttarget,&targetx,&targety);
    d2=distsqr(bulletx,bullety,targetx,targety);
    if (d2<0 || d2>125*125)
      return FALSE;
    if (bulletx>=targetx && targetx+15>=bulletx && bullety<=targety &&
        targety-15<=bullety)
      return TRUE;
  }
  return FALSE;
}


void initbuildingcheck(int x,int y)
{
  int i,left,right;
  left=x-(40+level);
  right=x+40+level;
  leftbuilding=-1;
  rightbuilding=0;
  for (i=0;i<20;i++)
    if (buildings[i]->x>=left) {
      leftbuilding=i;
      break;
    }
  if (leftbuilding==-1)
    return;
  for (;i<20;i++)
    if (buildings[i]->x>=right)
      break;
  rightbuilding=i-1;
}

bool doomed(int x,int y,int alt)
{
  object *building;
  int i,left,right,xx,yy;
  if (alt>50)
    return FALSE;
  if (alt<20)
    return TRUE;
  left=x-32;
  right=x+32;
  for (i=leftbuilding;i<=rightbuilding;i++) {
    building=buildings[i];
    xx=building->x;
    if (xx>=left) {
      if (xx>right)
        return FALSE;
      yy=building->y+(building->status==STATUS_INTACT ? 16 : 8);
      if (y<=yy)
        return TRUE;
    }
  }
  return FALSE;
}

int distsqr(int x0,int y0,int x1,int y1)
{
  int x=abs(x0-x1),y=abs(y0-y1),t;
  if (x<125 && y<125)
    return x*x+y*y;
  if (x<y) { t=x; x=y; y=t; }
  return -((x*7+(y<<2))>>3);
}

void autopilot(object *plane,int destx,int desty,object *destobj,bool recursing)
{
  int px,py,distx,disty,i,newx,newy,ch,newrot,newspd,d2,mindist2;
  if (plane->status==STATUS_SPINNING && plane->rotation!=12) {
    plane->deltarot=-1;
    return;
  }
  px=plane->x;
  py=plane->y;
  distx=px-destx;
  if (abs(distx)>200) {
    if (plane->xv!=0 && (distx<0)==(plane->xv<0)) {
      if (plane->counter==0)
        plane->counter=(py>150) ? 2 : 1;
      autopilot(plane,px,plane->counter==1 ? py+25 : py-25,NULL,TRUE);
      return;
    }
    plane->counter=0;
    autopilot(plane,(distx<0 ? 150 : -150)+px,
              (py=(py+100))>(150-autoheight) ? 150-autoheight : py,NULL,TRUE);
    return;
  }
  if (!recursing)
    plane->counter=0;
  if (plane->speed!=0) {
    disty=py-desty;
    if (disty!=0 && abs(disty)<6)
      plane->y=(disty<0 ? ++py : --py);
    else
      if (distx!=0 && abs(distx)<6)
        plane->x=(distx<0 ? ++px : --px);
  }
  initbuildingcheck(px,py);
  memcpy(&tplane,plane,sizeof(object));
  newspd=tplane.speed+1;
  if (newspd>level+8)
    newspd=level+8;
  else
    if (newspd<level+4)
      newspd=level+4;
  for (i=0;i<3;i++) {
    newrot=(tplane.rotation+(tplane.inverted ? -autodeltarot[i] :
                                                autodeltarot[i]))&15;
    setvel(&tplane,newspd,newrot);
    moveobject(&tplane,&newx,&newy);
    autod2[i]=distsqr(newx,newy,destx,desty);
    autoalt[i]=newy-ground[newx+8];
    autodoomed[i]=doomed(newx,newy,autoalt[i]);
    memcpy(&tplane,plane,sizeof(object));
  }
  if (destobj!=NULL && ontarget(destobj))
    plane->firing=TRUE;
  mindist2=32767;
  for (i=0;i<3;i++) {
    d2=autod2[i];
    if (d2>=0 && d2<mindist2 && !autodoomed[i]) {
      mindist2=d2;
      ch=i;
    }
  }
  if (mindist2==32767) {
    mindist2=-32767;
    for (i=0;i<3;i++) {
      d2=autod2[i];
      if (d2<0 && d2>mindist2 && !autodoomed[i]) {
        mindist2=d2;
        ch=i;
      }
    }
  }
  if (plane->speed<level+4)
    plane->deltav=4;
  if (mindist2!=-32767) {
    if (plane->deltav<4)
      plane->deltav++;
  }
  else {
    if (plane->deltav!=0)
      plane->deltav--;
    ch=0;
    disty=autoalt[0];
    if (autoalt[1]>disty) {
      disty=autoalt[1];
      ch=1;
    }
    if (autoalt[2]>disty)
      ch=2;
  }
  plane->deltarot=autodeltarot[ch];
  if (plane->deltarot==0 && plane->speed!=0)
    plane->inverted=(plane->xv<0);
}

void killplane(object *plane)
{
  if (plane->xv<0)
    plane->rotation=(plane->rotation+2)&15;
  else
    plane->rotation=(plane->rotation-2)&15;
  plane->status=(plane->status>=STATUS_ELIMNORM ? STATUS_ELIMDEAD : STATUS_DEAD);
  plane->ishome=FALSE;
  plane->speed=0;
  plane->yvfrac=0;
  plane->xvfrac=0;
  plane->yv=0;
  plane->xv=0;
  plane->counter=10;
}

void catchfire(object *plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->counter=10;
  plane->status=STATUS_FALLING;
  plane->ishome=FALSE;
}

void gointospin(object *plane)
{
  plane->xv=0;
  plane->yv=0;
  plane->speed=0;
  plane->inverted=FALSE;
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->rotation=14;
  plane->counter=6;
  plane->status=(plane->status>=STATUS_ELIMNORM ? STATUS_ELIMSPIN : STATUS_SPINNING);
  plane->ishome=FALSE;
}

void insxlist(object *ins,object *list)
{
  if (ins->x<list->x)
    do
      list=list->prevx;
    while (ins->x<list->x);
  else {
    while (ins->x>=list->x)
      list=list->nextx;
    list=list->prevx;
  }
  ins->nextx=list->nextx;
  ins->prevx=list;
  list->nextx->prevx=ins;
  list->nextx=ins;
}

void delxlist(object *del)
{
  del->nextx->prevx=del->prevx;
  del->prevx->nextx=del->nextx;
}

void initsndlist(void)
{
  snd *p;
  int i;
  for (i=0,p=&sndlist[0];i<99;i++,p++)
    p->next=p+1;
  p->next=NULL;
  lastsnd=NULL;
  nextsnd=&sndlist[0];
}

void initsound(int type,int pitch,object *obj)
{
  if (type<soundtype) {
    soundtype=type;
    soundpitch=pitch;
    soundobj=obj;
  }
  else
    if (type==soundtype && pitch<soundpitch) {
      soundpitch=pitch;
      soundobj=obj;
    }
}

void updatesound(void)
{
  snd *s;
/*  disable(); */
  for (s=lastsnd;s!=NULL;s=s->next)
    s->t2v+=s->deltat2v*slidec;
  slidec=0;
  tune3f=FALSE;
  switch (soundtype) {
    case SOUND_OFF:
    case SOUND_NONE:
    default:
      snosound();
      sndobj=NULL;
      soundperiodicfunc=NULL;
      break;
    case SOUND_ENGINE:
      if (soundpitch==0)
        ssound(0xf000);
      else
        ssound(soundpitch*0x1000+0xd000);
      sndobj=NULL;
      soundperiodicfunc=NULL;
      break;
    case SOUND_BOMB:
      if (soundobj==sndobj)
        break;
      soundperiodicfunc=soundslide;
      sndobj=soundobj;
      soundslide();
      break;
    case SOUND_FALLING:
      if (soundobj==sndobj)
        break;
      soundperiodicfunc=soundslide;
      sndobj=soundobj;
      soundslide();
      break;
    case SOUND_STUTTER:
      ssound(getenginestutter(2)!=0 ? 0x9000 : 0xf000);
      sndobj=NULL;
      soundperiodicfunc=NULL;
      break;
    case SOUND_SHRAPNEL:
      ssound(t2v2);
      soundperiodicfunc=NULL;
      sndobj=NULL;
      break;
    case SOUND_FIRING:
      ssound(0x1000);
      soundperiodicfunc=soundtoggle;
      sndobj=NULL;
      break;
    case SOUND_TUNE:
      tuneline3=0;
      tuneptr3=0;
      sound3();
      soundperiodicfunc=NULL;
      sndobj=NULL;
      tune3f=TRUE;
  }
  soundpitch=0x7fff;
  soundtype=SOUND_NONE;
}

void updatesoundint(void)
{
  slidec++;
  if (t2v!=0 && soundperiodicfunc!=NULL)
    soundperiodicfunc();
  if (numshrapnelsnd!=0)
    updatetune2();
  if (tune3f)
    updatetune3();
}

void soundslide(void)
{
  snd *s=sndobj->sndp;
  ssound(s->t2v+s->deltat2v*slidec);
}

void soundtoggle(void)
{
  if (t2v==0xf000)
    ssound(ot2v);
  else {
    ot2v=t2v;
    ssound(0xf000);
  }
}

void updatetune2(void)
{
  tunetime2--;
  if (tunetime2<0)
    setsound2();
}

void setsound2(void)
{
  tuneline=tuneline2;
  tuneptr=tuneptr2;
  tune=tune1;
  octavemultiplier=octavemul2;
  updatetune();
  tuneline2=tuneline;
  tuneptr2=tuneptr;
  t2v2=t2v1;
  tunetime2+=tunedur;  /* Interrupts originally cleared for this instruction */
  octavemul2=octavemultiplier;
}

void updatetune3(void)
{
  tunetime3--;
  if (tunetime3<0)
    sound3();
}

void sound3(void)
{
  tuneline=tuneline3;
  tuneptr=tuneptr3;
  tune=tune1;
  octavemultiplier=octavemul3;
  updatetune();
  tuneline3=tuneline;
  tuneptr3=tuneptr;
  t2v3=t2v1;
  tunetime3+=tunedur;  /* Interrupts originally cleared for this instruction */
  octavemul3=octavemultiplier;
  snosound();
  ssound(t2v3);
}

void objectsound(object *obj,int sn)
{
  snd *s;
  if (obj->sndp!=NULL)
    return;
  if (obj->type==OBJ_SHRAPNEL) {
/*    disable(); */
    numshrapnelsnd++;
    if (numshrapnelsnd==1) {
      tuneptr2=tuneline2=0;
      setsound2();
    }
    obj->sndp=(snd *)1; /* Suspect this is a bug */
/*    enable(); */
    return;
  }
  s=createsnd();
  if (s!=NULL) {
/*    disable(); */
    switch (sn) {
      case SOUND_BOMB:
        s->t2v=0x300;
        s->deltat2v=8;
        break;
      case SOUND_FALLING:
        s->t2v=0x1200;
        s->deltat2v=-8;
        break;
    }
    obj->sndp=s;
/*    enable(); */
    return;
  }
}

snd *createsnd(void)
{
  snd *p;
  if (nextsnd==NULL)
    return NULL;
  p=nextsnd;
  nextsnd=p->next;
  p->next=lastsnd;
  p->prev=NULL;
  if (lastsnd!=NULL)
    lastsnd->prev=p;
  return lastsnd=p;
}

void destroysnd(snd *s)
{
  snd *p=s->prev;
  if (p!=NULL)
    p->next=s->next;
  else
    lastsnd=s->next;
  p=s->next;
  if (p!=NULL)
    p->prev=s->prev;
  s->next=nextsnd;
  nextsnd=s;
}

void killsnd(object *o)
{
  snd *s=o->sndp;
  if (s==NULL)
    return;
/*  disable(); */
  if (o->type==OBJ_SHRAPNEL)
    numshrapnelsnd--;
  else
    destroysnd(s);
  o->sndp=NULL;
/*  enable(); */
}

void ssound(unsigned int t2)
{
  if (!soundflag)
    return;
  if (t2v==t2)
    return;
  if (t2v==0)
    outportb(PORT_TIMERC,0xb6);
  outportb(PORT_TIMER2,t2);
  outportb(PORT_TIMER2,t2>>8);
  if (t2v==0)
    outportb(PORT_SPKR,inportb(PORT_SPKR)|3);
  t2v=t2;
}

void snosound(void)
{
  if (t2v!=0) {
    outportb(PORT_SPKR,inportb(PORT_SPKR)&0xfc);
    t2v=0;
  }
}

int getenginestutter(int f)
{
  if (stutterp>=50)
    stutterp=0;
  return enginestutter[stutterp++]%f;
}

char tempobuf[5];

void updatetune(void)
{
  bool notgotchar=TRUE;
  int sharpen=0,tempobufp=0,dotdur=2,overflowoct=0x100,semitone,tempo,freq;
  char tunechar,tunechar2;
  while (TRUE) {
    if (tuneline==0 && tuneptr==0)
      octavemultiplier=0x100;
    tunechar=toupper(tune[tuneline][tuneptr++]);
    if (tunechar==0) {
      tunechar=tune[++tuneline][tuneptr=0];
      if (tunechar==0)
        tuneline=0;
      if (!notgotchar)
        break;
    }
    else {
      notgotchar=FALSE;
      if (tunechar=='/')
        break;
      if (isalpha(tunechar)) {
        semitone=majorscale[tunechar-'A'];
        tunechar2=tunechar;
      }
      else {
        switch(tunechar) {
          case '>':
            octavemultiplier<<=1;
            break;
          case '<':
            octavemultiplier>>=1;
            break;
          case '+':
            sharpen++;
            break;
          case '-':
            sharpen--;
            break;
          case '.':
            dotdur=3;
            break;
          default:
            if (isdigit(tunechar))
              tempobuf[tempobufp++]=tunechar;
        }
      }
    }
  }
  tempobuf[tempobufp]=0;
  tempo=atoi(tempobuf);
  if (tempo<=0)
    tempo=4;
  if (tunechar2=='R')
    freq=0x7d00;
  else {
    semitone+=sharpen;
    while (semitone<0) {
      semitone+=12;
      overflowoct>>=1;
    }
    while (semitone>=12) {
      semitone-=12;
      overflowoct<<=1;
    }
    freq=(short)(((long)notefreq[semitone]*(long)octavemultiplier*
                  (long)overflowoct)>>16);
  }
  t2v1=(short)(1193181l/freq);
  tunedur=((dotdur*1440)/(tempo*60))>>1;
}

void init(int argc,char *argv[])
{
  int nmodes,ncontrols,i;
  bool modes=FALSE,modec=FALSE,modem=FALSE,modea=FALSE,keybflag=FALSE;
  getopt(&argc,&argv,"w&y#q&e&g#s&c&m&a&j&k&i&h*v*:sopwith2*",
         &hiresflag,        /* -w */
         &latency,          /* -y[num] */
         &soundflag,        /* -q (quiet) */
         &statsflag,        /* -e (print end of game statistics) */
         &level,            /* -g[num] */
         &modes,            /* -s */
         &modec,            /* -c */
         &modem,            /* -m */
         &modea,            /* -a */
         &joyrequested,     /* -j */
         &keybflag,         /* -k */
         &ibmkeyboard,      /* -i */
         &recordfilename,   /* -h[string] (record) */
         &playbackfilename  /* -v[string] (playback) */
        );
  soundflag=!soundflag;
  nmodes=(modes ? 1 : 0)+(modec ? 1 : 0)+(modem ? 1 : 0)+(modea ? 1 : 0);
  if (nmodes>1) {
    printstr("\r\nOnly one mode: -s -c -m -a  may be specified\r\n");
    exit(1);
  }
  ncontrols=(keybflag ? 1 : 0)+(joyrequested ? 1 : 0);
  if (ncontrols>1) {
    printstr("\r\nOnly one of -j and -k may be specified\r\n");
    exit(1);
  }
  if (nmodes>0 && ncontrols>0)
    notitlesflag=TRUE;
  initrand();
  randno=inithistory(randno);
  initsndlist();
  init_timer(timerint);
  titles();
  if (nmodes>0)
    gamemode=(modes ? GAME_SINGLE : (modec ? GAME_COMPUTER :
             (modem ? GAME_MULTIPLE : GAME_ASYNCH)));
  else
    gamemode=getgamemode();
  multiplayer=(gamemode==GAME_MULTIPLE || gamemode==GAME_ASYNCH);
  if (ncontrols==0)
    getcontrol();
  if (multiplayer) {
    maxlives=10;
    if (gamemode==GAME_MULTIPLE)
      initcomms();
    else
      initasynch();
    copyground();
    initlists();
    if (gamemode==GAME_MULTIPLE)
      createmultipleremoteplanes();
    else
      createasynchremoteplane();
    createbuildings();
    initscreen(FALSE);
    if (latency==-1)
      latency=1;
  }
  else {
    if (latency==-1)
      latency=1;
    maxlives=5;
    world=&worlds[0];
    clearwin();
    copyground();
    initlists();
    createplayerplane(NULL);
    for (i=0;i<3;i++)
      createenemyplane(NULL);
    createbuildings();
    initscreen(FALSE);
  }
  createflocks();
  createcows();
  initdifficulty();
  init_keyb(keybint);
  keynotascii=TRUE;
}

void titles(void)
{
  if (hiresflag)
    setgmode(6);
  else
    setgmode(4);
  if (notitlesflag)
    return;
  initsound(SOUND_TUNE,0,NULL);
  updatesound();
  setcolour(3);
  poscurs(13,8);
  printstr("S O P W I T H");
  poscurs(12,11);
  setcolour(1);
  printstr("BMB ");
  setcolour(3);
  printstr("Compuscience");
}

void initrand(void)
{
  while (randno==0) {
    outportb(PORT_TIMERC,0);
    randno=inportb(PORT_TIMER0);
    randno|=(inportb(PORT_TIMER0)<<8);
  }
}

void restart(void)
{
  int sc,i;
  object *plane;
  if (finaleflags[lastplane]==FINALE_WON) {
    plane=&objects[lastplane];
    sc=0;
    while (plane->deaths++<maxlives) {
      sc+=25;
      plane->score+=sc;
      useonscreenbuf();
      livesbar(plane);
      writescore(plane);
      timertick=0; while (timertick<5);
    }
    if (level<=5)
      level++;
    score=plane->score;
  }
  else
    level=score=0;
  initsndlist();
  copyground();
  initlists();
  createplayerplane(NULL);
  for (i=0;i<3;i++)
    createenemyplane(NULL);
  createbuildings();
  initscreen(FALSE);
  createflocks();
  createcows();
  initdifficulty();
  longjmp(startjbuf,0);
}

void initdifficulty(void)
{
  radarrange2=100;
  if (level<6)
    radarrange2-=(6-level)*10;
  radarrange2*=radarrange2;
}

void clearwin(void)
{
  int i;
  for (i=20;i<24;i++) {
    poscurs(0,i);
    clearline();
  }
  poscurs(0,20);
}

void getcontrol(void)
{
  int k;
  clearwin();
  printstr("Key: 1 - Joystick with IBM Keyboard\r\n");
  printstr("     2 - Joystick with non-IBM Keyboard\r\n");
  printstr("     3 - IBM Keyboard only\r\n");
  printstr("     4 - Non-IBM keyboard only\r\n");
  while (1) {
    if (testbreak())
      finish(NULL,FALSE);
    k=inkeys();
    if (k>='1' && k<='4')
      break;
  }
  joyrequested=(k<='2');
  ibmkeyboard=(k=='1' || k=='3');
}

int getgamemode(void)
{
  clearwin();
  printstr("Key: S - single player\r\n");
  printstr("     C - single player against computer\r\n");
  printstr("     M - multiple players on network\r\n");
  printstr("     A - 2 players on asynchronous line");
  while (TRUE) {
    if (testbreak())
      finish(NULL,FALSE);
    switch (toupper(inkeys())) {
      case 'S':
        return GAME_SINGLE;
      case 'M':
        return GAME_MULTIPLE;
      case 'C':
        return GAME_COMPUTER;
      case 'A':
        return GAME_ASYNCH;
    }
  }
}

int getgamenumber(void)
{
  int n;
  clearwin();
  printstr("         Key a game number");
  while (1) {
    if (testbreak())
      finish(NULL,FALSE);
    n=inkeys()-'0';
    if (n>=0 && n<=7)
      return n;
  }
}

bool testbreak(void)
{
  return (break1 || break2);
}

void copyground(void)
{
  memcpy(&workingground,&ground,3000);
}

void initscreen(bool drawnground)
{
  object *plane;
  if (drawnground==0) {
    clearscrbuf();
    useoffscreenbuf();
    drawmapground();
    soundoff();
    deceased=FALSE;
  }
  copyoffscbuftoscn();
  useonscreenbuf();
  drawmapbuildings();
  writescores();
  plane=&objects[lastplane];
  if (deceased) {
    poscurs(16,24);
    setcolour(plane->colour);
    printstr("\2"); /* :-) */
  }
  else {
    fuelbar(plane);
    bombsbar(plane);
    ammobar(plane);
    livesbar(plane);
  }
  groundnotdrawn=TRUE;
}

void writescores(void)
{
  if (score!=0) {
    objects[0].score=score;
    score=0;
  }
  writescore(objects);
/*  if (multiplayer && netinf->maxplayers>1)
    writescore(&objects[1]); */
}

void livesbar(object *p)
{
  statbar(127,maxlives-p->deaths,maxlives,p->colour);
}

void fuelbar(object *p)
{
  statbar(132,p->fuel>>4,562,p->colour);
}

void bombsbar(object *p)
{
  statbar(137,p->bombs,5,3-p->colour);
}

void ammobar(object *p)
{
  statbar(142,p->ammo,200,3);
}

void statbar(int x,int h,int hmax,int col)
{
  int y;
  if (deceased)
    return;
  h=(h*10)/hmax-1;
  if (h>9)
    h=9;
  for (y=0;y<=h;y++)
    putpixel(x,y,col);
  for (;y<=9;y++)
    putpixel(x,y,0);
}

void drawmapground(void)
{
  int c,newy,x,sx,y;
  c=0;
  sx=152;
  newy=15;
  for (y=0;y<=newy;y++)
    putpixel(sx,y,3);
  newy=y=0;
  for (x=0;x<3000;x++) {
    if (workingground[x]>newy)
      newy=workingground[x];
    c++;
    if (c==19) {
      newy/=13;
      if (newy==y)
        putpixel(sx,newy,3);
      else
        if (newy>y)
          for (y++;y<=newy;y++)
            putpixel(sx,y,3);
        else
          for (y--;y>=newy;y--)
            putpixel(sx,y,3);
      y=newy;
      putpixel(sx,0,3);
      sx++;
      c=newy=0;
    }
  }
  newy=15;
  for (y=0;y<=newy;y++)
    putpixel(sx,y,3);
  for (x=0;x<320;x++)
    putpixel(x,18,3);
}

void drawmapbuildings(void)
{
  int i;
  object *building;
  mapobject *mapobj;
  for (i=0,building=objects,mapobj=mapobjects;i<100;i++,building++,mapobj++) {
    building->oldonscreen=0;
    building->onscreen=0;
    mapobj->colour=0;
  }
  for (i=0;i<20;i++) {
    building=buildings[i];
    if (building->status!=STATUS_ELIMINATED)
      drawmapobject(building);
  }
}

void useoffscreenbuf(void)
{
  scrseg=_DS;
  scroff=((int)&scrbuf)-0x1000;
  interlacediff=0x1000;
}

void useonscreenbuf(void)
{
  scrseg=0xb800;
  scroff=0;
  interlacediff=0x2000;
}

void copyoffscbuftoscn(void)
{
  farmemset(MK_FP(0xb800,0),0x1000,0);
  farmemset(MK_FP(0xb800,0x2000),0x1000,0);
  farmemmove(MK_FP(_DS,&scrbuf),MK_FP(0xb800,0x1000),0x1000);
  farmemmove(MK_FP(_DS,(int)(&scrbuf)+0x1000),MK_FP(0xb800,0x3000),0x1000);
}

void clearscrbuf(void)
{
  memset(&scrbuf,0x2000,0);
}

void initlists(void)
{
  int i;
  object *o;
  objleft.nextx=objleft.next=&objright;
  objright.prevx=objright.prev=&objleft;
  objleft.x=-32767;
  objright.x=32767;
  lastobject=firstobject=firstdeleted=lastdeleted=NULL;
  nextobject=o=objects;
  for (i=0;i<100;i++) {
    o->next=o+1;
    (o++)->index=i;
  }
  (o-1)->next=0;
}

void createenemyplane(object *pl)
{
  object *plane=createplane(pl);
  if (plane!=NULL) {
    plane->soundfunc=soundplane;
    plane->updatefunc=updateenemyplane;
    plane->colour=2;
    if (!multiplayer)
      plane->source=&objects[1];
    else
      if (plane->index==1)
        plane->source=plane;
      else
        plane->source=plane-2;
    memcpy(&homes[plane->index],plane,sizeof(object));
  }
  if (gamemode==GAME_SINGLE) {
    plane->status=STATUS_ELIMINATED;
    delxlist(plane);
  }
}

void createplayerplane(object *player)
{
  object *plane=createplane(player);
  if (player==NULL) {
    plane->soundfunc=soundplane;
    plane->updatefunc=updateplayerplane;
    plane->colour=(plane->index&1)+1;
    plane->source=plane;
    memcpy(&homes[plane->index],plane,sizeof(object));
    finaletime=0;
    finaleflying=FALSE;
  }
  scnleft=plane->x-152;
  scnright=scnleft+319;
  flushbuf();
}

object *createplane(object *pl)
{
  int y,x,left,right,pos;
  object *plane=(pl!=NULL ? pl : newobject());
  switch (gamemode) {
    case GAME_SINGLE:
      pos=singleplanes[plane->index];
      break;
    case GAME_MULTIPLE:
    case GAME_ASYNCH:
      pos=multipleplanes[plane->index];
      break;
    case GAME_COMPUTER:
      pos=computerplanes[plane->index];
  }
  plane->type=OBJ_PLANE;
  left=plane->x=world->planeposes[pos];
  right=plane->x+20;
  y=0;
  for (x=left;x<=right;x++)
    if (workingground[x]>y)
      y=workingground[x];
  plane->y=y+13;
  plane->bombtime=0;
  plane->counter=0;
  plane->deltav=0;
  plane->deltarot=0;
  plane->speed=0;
  plane->yfrac=0;
  plane->xfrac=0;
  setvel(plane,0,0);
  plane->inverted=world->planeflip[pos];
  plane->rotation=(plane->inverted ? 8 : 0);
  plane->homing=FALSE;
  plane->bombing=FALSE;
  plane->firing=FALSE;
  plane->height=16;
  plane->width=16;
  plane->ishome=TRUE;
  if (pl==0 || plane->status==STATUS_DEAD || plane->status==STATUS_ELIMDEAD) {
    plane->ammo=200;
    plane->bombs=5;
    plane->fuel=9000;
  }
  if (pl==0) {
    attackees[plane->index]=NULL;
    finaleflags[plane->index]=FINALE_NONE;
    plane->deaths=0;
/*    plane->si1c=0; */
    plane->score=0;
    insxlist(plane,&objleft);
  }
  else {
    delxlist(plane);
    insxlist(plane,plane->nextx);
  }
  if (multiplayer && plane->deaths>=maxlives) {
    plane->status=STATUS_ELIMNORM;
    if (plane->index==lastplane)
      deceased=TRUE;
  }
  else
    plane->status=STATUS_NORMAL;
  return plane;
}

void firebullet(object *obj,object *at)
{
  object *bullet;
  int xv,yv,dist,speed,atx,aty;
  if (at==NULL && !planeisenemy && obj->ammo==0)
    return;
  bullet=newobject();
  if (bullet==NULL)
    return;
  obj->ammo--;
  speed=level+10;
  if (at!=NULL) {
    atx=at->x+(at->xv<<2);
    aty=at->y+(at->yv<<2);
    xv=atx-obj->x;
    yv=aty-obj->y;
    dist=distance(atx,aty,obj->x,obj->y);
    if (dist<1) {
      deleteobject(bullet);
      return;
    }
    bullet->xv=(speed*xv)/dist;
    bullet->yv=(speed*yv)/dist;
    bullet->yvfrac=0;
    bullet->xvfrac=0;
  }
  else
    setvel(bullet,obj->speed+speed,obj->rotation);
  bullet->type=OBJ_BULLET;
  bullet->x=obj->x+8;
  bullet->y=obj->y-8;
  bullet->xfrac=obj->xfrac;
  bullet->yfrac=obj->yfrac;
  bullet->fuel=10;
  bullet->source=obj;
  bullet->colour=obj->colour;
  bullet->width=1;
  bullet->height=1;
  bullet->soundfunc=NULL;
  bullet->updatefunc=updatebullet;
  bullet->speed=0;
  insxlist(bullet,obj);
}

int distance(int x0,int y0,int x1,int y1)
{
  int x=abs(x0-x1),y=abs(y0-y1),t;
  if (x>100 || y>100)
    return -1;
  if (x<y) { t=x; x=y; y=t; }
  return (x*7+(y<<2))>>3;
}

void dropbomb(object *plane)
{
  object *bomb;
  int rot;
  if (!planeisenemy && (plane->bombs==0 || plane->bombtime!=0))
    return;
  bomb=newobject();
  if (bomb==NULL)
    return;
  plane->bombs--;
  plane->bombtime=10;
  bomb->type=OBJ_BOMB;
  bomb->status=STATUS_FALLING;
  bomb->xv=plane->xv;
  bomb->yv=plane->yv;
  rot=(plane->inverted ? ((plane->rotation+4)&15) : ((plane->rotation-4)&15));
  bomb->x=plane->x+(fcos(10,rot)>>8)+4;
  bomb->y=plane->y+(fsin(10,rot)>>8)-4;
  bomb->yvfrac=0;
  bomb->xvfrac=0;
  bomb->yfrac=0;
  bomb->xfrac=0;
  bomb->fuel=5;
  bomb->source=plane;
  bomb->colour=plane->colour;
  bomb->width=8;
  bomb->height=8;
  bomb->soundfunc=soundbomb;
  bomb->updatefunc=updatebomb;
  insxlist(bomb,plane);
}

void createbuildings(void)
{
  int i,minaltitude,maxaltitude,y,gx,left,right;
  object *building;
  int *positions=world->buildingpositions;
  int *types=world->buildingtypes;
  if (multiplayer /* && netinf->maxplayers!=1 */)
    buildingc[0]=buildingc[1]=10;
  else {
    buildingc[0]=0;
    buildingc[1]=17;
  }
  for (i=0;i<20;i++) {
    building=newobject();
    buildings[i]=building;
    building->x=left=*positions;
    right=left+15;
    minaltitude=999;
    maxaltitude=0;
    for (gx=left;gx<=right;gx++) {
      if (workingground[gx]>maxaltitude)
        maxaltitude=workingground[gx];
      if (workingground[gx]<minaltitude)
        minaltitude=workingground[gx];
    }
    for (y=(maxaltitude+minaltitude)>>1;(building->y=y+16)>=200;y--);
    for (gx=left;gx<=right;gx++)
      workingground[gx]=y;
    building->counter=0;
    building->rotation=0;
    building->yvfrac=0;
    building->xvfrac=0;
    building->yfrac=0;
    building->xfrac=0;
    building->yv=0;
    building->xv=0;
    building->type=OBJ_BUILDING;
    building->status=STATUS_INTACT;
    building->kind=*types;
    building->fuel=i;
    if (multiplayer /* && netinf->maxplayers!=1 */)
      building->source=&objects[i<10 ? 0 : 1];
    else
      building->source=&objects[i<10 && i>6 ? 0 : 1];
    building->colour=building->source->colour;
    building->width=16;
    building->height=16;
    building->soundfunc=soundbuilding;
    building->updatefunc=updatebuilding;
    insxlist(building,&objleft);
    positions++;
    types++;
  }
}

void createexplosion(object *obj)
{
  object *shrapnel;
  int i,step,l,x,y,xv,yv,colour,type,kind;
  unsigned int randno1;
  bool soundf;
  x=obj->x+(obj->width>>1);
  y=obj->y+(obj->height>>1);
  xv=obj->xv;
  yv=obj->yv;
  colour=obj->colour;
  type=obj->type;
  if (type==OBJ_BUILDING && obj->kind==BUILDING_FUEL) {
    step=1;
    l=level+8;
  }
  else {
    step=(type==OBJ_PLANE ? 6 : 2);
    l=(level+4)>>1;
  }
  soundf=(type==OBJ_PLANE && obj->status==STATUS_NORMAL);
  for (i=1;i<=15;i+=step) {
    shrapnel=newobject();
    if (shrapnel==NULL)
      return;
    shrapnel->type=OBJ_SHRAPNEL;
    setvel(shrapnel,l,i);
    shrapnel->xv+=xv;
    shrapnel->yv+=yv;
    shrapnel->y=shrapnel->yv+y;
    shrapnel->x=shrapnel->xv+x;
    randno1=(shrapnel->y)*(shrapnel->x)*randno*0x1d43;
    shrapnel->fuel=3;
    kind=shrapnel->kind=(int)(((unsigned long)((unsigned short)((unsigned long)randno1*(unsigned long)i))*8UL)>>16);
    if (soundf && (kind==0 || kind==7)) {
      kind=shrapnel->kind=0;
      soundf=FALSE;
      shrapnel->xv=xv;
      shrapnel->yv=-(level+4);
    }
    shrapnel->speed=0;
    shrapnel->counter=0;
    shrapnel->yfrac=0;
    shrapnel->xfrac=0;
    shrapnel->source=obj;
    shrapnel->colour=colour;
    shrapnel->width=8;
    shrapnel->height=8;
    shrapnel->soundfunc=soundshrapnel;
    shrapnel->updatefunc=updateshrapnel;
    if (kind!=0)
      objectsound(shrapnel,SOUND_SHRAPNEL);
    insxlist(shrapnel,obj);
  }
}

void createsmoke(object *plane)
{
  object *smoke=newobject();
  if (smoke==NULL)
    return;
  smoke->type=OBJ_SMOKE;
  smoke->x=plane->x+8;
  smoke->y=plane->y-8;
  smoke->xv=plane->xv;
  smoke->yv=plane->yv;
  smoke->yvfrac=0;
  smoke->xvfrac=0;
  smoke->yfrac=0;
  smoke->xfrac=0;
  smoke->fuel=10;
  smoke->source=plane;
  smoke->width=1;
  smoke->height=1;
  smoke->soundfunc=NULL;
  smoke->updatefunc=updatesmoke;
  smoke->colour=plane->colour;
}

void createflocks(void)
{
  object *flock;
  int i,bird;
  for (i=0;i<4;i++) {
    flock=newobject();
    if (flock==NULL)
      return;
    flock->type=OBJ_FLOCK;
    flock->status=STATUS_NORMAL;
    flock->x=flockx[i];
    flock->y=flocky[i];
    flock->xv=flockxv[i];
    flock->yvfrac=0;
    flock->xvfrac=0;
    flock->yfrac=0;
    flock->xfrac=0;
    flock->yv=0;
    flock->inverted=FALSE;
    flock->fuel=5;
    flock->source=flock;
    flock->height=flock->width=16;
    flock->soundfunc=NULL;
    flock->updatefunc=updateflock;
    flock->colour=1;
    insxlist(flock,&objleft);
    for (bird=0;bird<SPAREBIRDS;bird++)
      createbird(flock,1);
  }
}

void createbird(object *flock,int birdno)
{
  object *bird=newobject();
  if (bird==NULL)
    return;
  bird->type=OBJ_BIRD;
  bird->x=flock->x+fscatterx[birdno];
  bird->y=flock->y-fscattery[birdno];
  bird->xv=fscatterxv[birdno];
  bird->yv=fscatteryv[birdno];
  bird->yvfrac=0;
  bird->xvfrac=0;
  bird->yfrac=0;
  bird->xfrac=0;
  bird->inverted=FALSE;
  bird->fuel=4;
  bird->source=flock;
  bird->height=2;
  bird->width=4;
  bird->soundfunc=NULL;
  bird->updatefunc=updatebird;
  bird->colour=flock->colour;
  insxlist(bird,flock);
}

void createcows(void)
{
  object *cow;
  int i;
  for (i=0;i<2;i++) {
    cow=newobject();
    if (cow==NULL)       /* Test for the NULL cow. */
      return;
    cow->type=OBJ_COW;
    cow->status=STATUS_INTACT;
    cow->x=cowx[i];
    cow->y=cowy[i];
    cow->yv=0;           /* This is a particularly amusing variable */
    cow->xv=0;
    cow->yvfrac=0;
    cow->xvfrac=0;
    cow->onscreen=0;
    cow->xfrac=0;
    cow->inverted=FALSE; /* But this one is just plain bizarre */
    cow->source=cow;
    cow->height=16;
    cow->width=16;
    cow->soundfunc=NULL;
    cow->updatefunc=updatecow;
    cow->colour=1;       /* How now, brown cow. Sorry, I'll shut up now. */
    insxlist(cow,&objleft);
  }
}

void checkcollisions(void)
{
  int right,top,bottom,i,type;
  object **obj1,**obj2;
  object *obj,*test;
  colln=collc=0;
  collxvr=2;
  collyvr=1;
  if (frametick&1) {
    collxvr=-collxvr;
    collyvr=-collyvr;
  }
  useoffscreenbuf();
  for (obj=objleft.nextx;obj!=&objright;obj=obj->nextx) {
    right=obj->x+obj->width-1;
    bottom=obj->y;
    top=bottom+1-obj->height;
    test=obj->nextx;
    for (test=obj->nextx;test->x<=right && test!=&objright;test=test->nextx)
      if (test->y>=top && test->y-test->height+1<=bottom)
        checkcollision(obj,test);
    type=obj->type;
    if ((type==OBJ_PLANE && obj->status!=STATUS_ELIMINATED && obj->status!=0 &&
         obj->y<workingground[obj->x+8]+24) ||
        (type==OBJ_BOMB && obj->y<workingground[obj->x+4]+12))
      checkcrash(obj);
  }
  for (i=0,obj1=collobj1,obj2=collobj2;i<collc;i++,obj1++,obj2++)
    docollision(*obj1,*obj2);
  for (i=0,obj1=collobjs;i<colln;i++,obj1++) {
    (obj=*obj1)->xv=collxv[i];
    obj->yv=collyv[i];
  }
}

void checkcollision(object *obj1,object *obj2)
{
  int type1,type2;
  object *t;
  type1=obj1->type;
  type2=obj2->type;
  if ((type1==OBJ_PLANE && obj1->status>=STATUS_ELIMINATED) ||
      (type2==OBJ_PLANE && obj2->status>=STATUS_ELIMINATED) ||
      (type1==OBJ_SHRAPNEL && type2==OBJ_SHRAPNEL))
    return;
  if (obj1->y<obj2->y) {
    t=obj1;
    obj1=obj2;
    obj2=t;
  }
  drawobject(15,15,obj1);
  if (bitblt(obj2->x-obj1->x+15,obj2->y-obj1->y+15,obj2) && collc<199) {
    collobj1[collc]=obj1;
    collobj2[collc++]=obj2;
    collobj1[collc]=obj2;
    collobj2[collc++]=obj1;
  }
  clearscorearea();
}

void checkcrash(object *obj)
{
  int x,right,h;
  bool f=FALSE;
  drawobject(15,15,obj);
  right=obj->x+obj->width-1;
  for (x=obj->x;x<=right;x++) {
    h=workingground[x]+15-obj->y;
    if (h>15) {
      f=TRUE;
      break;
    }
    if (h>=0) {
      f=pixel(x+15-obj->x,h,131);
      if (f!=0)
        break;
    }
  }
  clearscorearea();
  if (f && collc<200) {
    collobj1[collc]=obj;
    collobj2[collc++]=NULL;
  }
}

void docollision(object *obj1,object *obj2)
{
  int status,type2,i;
  type2=(obj2!=NULL) ? obj2->type : 0;
  if ((type2==OBJ_BIRD || type2==OBJ_FLOCK) && obj1->type!=OBJ_PLANE)
    return;
  switch (obj1->type) {
    case OBJ_BOMB:
      createexplosion(obj1);
      obj1->fuel=-1;
      if (obj2==NULL) /* No object - it hit the ground */
        digcrater(obj1);
      killsnd(obj1);
      return;
    case OBJ_BULLET:
      obj1->fuel=1;
      return;
    case OBJ_SHRAPNEL:
      if (obj2==NULL || obj1->inverted ||
          (!obj1->inverted && collidescore(type2,obj2,200))) {
        obj1->fuel=1;
        killsnd(obj1);
      }
      return;
    case OBJ_BUILDING:
      if (obj1->status!=STATUS_INTACT)
        return;
      if (type2==OBJ_SHRAPNEL)
        return;
      if (type2==OBJ_BULLET) {
        obj1->counter+=10;
        if (obj1->counter<(level+1)*10)
          return;
      }
      obj1->status=STATUS_ELIMINATED;
      createexplosion(obj1);
      useonscreenbuf();
      drawmapobject(obj1);
      useoffscreenbuf();
      addscore(obj1,obj1->kind==BUILDING_FUEL ? 200 : 100);
      buildingc[obj1->colour-1]--;
      if (buildingc[obj1->colour-1]==0)
        declarewinner(obj1->colour);
      return;
    case OBJ_PLANE:
      status=obj1->status;
      if (status==STATUS_DEAD || status==STATUS_ELIMDEAD)
        return;
      if (finaleflags[obj1->index]==FINALE_WON)
        return;
      if (type2==OBJ_BIRD && obj1->ishome)
        return;
      if (obj2==NULL) {
        if (status==STATUS_FALLING) {
          killsnd(obj1);
          createexplosion(obj1);
          digcrater(obj1);
        }
        else
          if (status<STATUS_ELIMINATED) {
            score50(obj1);
            createexplosion(obj1);
            digcrater(obj1);
          }
        killplane(obj1);
        return;
      }
      if (status==STATUS_FALLING || status>=STATUS_ELIMINATED)
        return;
      if (type2!=OBJ_BULLET) {
        createexplosion(obj1);
        if (type2==OBJ_PLANE) {
          collxvr=-collxvr;
          collxv[colln]=((obj1->xv+obj2->xv)>>1)+collxvr;
          collyvr=-collyvr;
          collyv[colln]=((obj1->yv+obj2->yv)>>1)+collyvr;
          collobjs[colln++]=obj1;
        }
      }
      catchfire(obj1);
      score50(obj1);
      return;
    case OBJ_BIRD:
      obj1->fuel=(collidescore(type2,obj2,25) ? -1 : -2);
      return;
    case OBJ_FLOCK:
      if (type2!=OBJ_FLOCK && type2!=OBJ_BIRD && obj1->status==STATUS_NORMAL) {
        for (i=0;i<8;i++)
          createbird(obj1,i);
        obj1->fuel=-1;
        obj1->status=STATUS_ELIMINATED;
      }
      return;
    case OBJ_COW:
      if (obj1->status!=STATUS_INTACT)
        return;
      if (type2==OBJ_SHRAPNEL)
        return;
      collidescore(type2,obj2,200);
      obj1->status=STATUS_ELIMINATED;
      return;
  }
}

void digcrater(object *obj)
{
  int left,right,x,ny,oy,miny,cx;
  left=obj->x+(obj->width-8)/2;
  right=left+7;
  for (x=left,cx=0;x<=right;x++,cx++) {
    oy=workingground[x];
    miny=ground[x]-20;
    if (miny<20)
      miny=20;
    ny=oy+1-crater[cx];
    if (ny<=miny)
      ny=miny+1;
    workingground[x]=ny-1;
  }
  cratertobedrawn=TRUE;
}

bool collidescore(int type,object *obj,int score)
{
  if ((type==OBJ_BULLET || type==OBJ_BOMB || type==OBJ_PLANE) &&
      (obj->status==STATUS_NORMAL ||
       (obj->status==STATUS_FALLING && obj->counter==10)) && !obj->ishome) {
    addscore(obj,score);
    return TRUE;
  }
  return FALSE;
}

void addscore(object *destroyed,int score)
{
  if (multiplayer /* && netinf->maxplayers==1 */) {
    objects[2-destroyed->colour].score+=score;
    writescore(&objects[2-destroyed->colour]);
  }
  else {
    if (destroyed->colour==1)
      objects->score-=score;
    else
      objects->score+=score;
    writescore(&objects[0]);
  }
}

void score50(object *destroyed)
{
  addscore(destroyed,50);
}

void writescore(object *p)
{
  poscurs(2+7*(p->colour-1),24);
  setcolour(p->colour);
  writenum(p->score,6);
}

void soundoff(void)
{
  if (notitlesflag)
    return;
  initsound(SOUND_OFF,0,NULL);
  updatesound();
}

void timerint(void)
{
  timertick++;
  speedtick++;
  updatesoundint();
}

void soundbomb(object *bomb)
{
  if (bomb->yv<=0)
    initsound(SOUND_BOMB,-bomb->y,bomb);
}

void soundshrapnel(object *shrapnel)
{
  if (shrapnel->kind!=0)
    initsound(SOUND_SHRAPNEL,shrapnel->counter,shrapnel);
}

void soundbuilding(object *building)
{
  if (building->firing)
    initsound(SOUND_FIRING,0,building);
}

void soundplane(object *plane)
{
  if (plane->firing)
    initsound(SOUND_FIRING,0,plane);
  else
    switch(plane->status) {
      case STATUS_FALLING:
        if (plane->yv>=0)
          initsound(SOUND_STUTTER,0,plane);
        else
          initsound(SOUND_FALLING,plane->yv,plane);
        break;
      case STATUS_NORMAL:
        initsound(SOUND_ENGINE,-plane->speed,plane);
        break;
      case STATUS_SPINNING:
        initsound(SOUND_STUTTER,0,plane);
    }
}

void drawmapobject(object *obj)
{
  mapobject *mapobj=&mapobjects[obj->index];
  int c;
  if (mapobj->colour!=0)
    putpixel(mapobj->x,mapobj->y,mapobj->colour-1);
  if (obj->status>=STATUS_ELIMINATED)
    mapobj->colour=0;
  else {
    c=pixel(mapobj->x=(obj->x+obj->width/2)/19+152,
            mapobj->y=(obj->y-obj->height/2)/13,obj->source->colour);
    if (c==0 || c==3) {
      mapobj->colour=c+1;
      return;
    }
    putpixel(mapobj->x,mapobj->y,c);
    mapobj->colour=0;
  }
}

void finish(char *msg,bool closef)
{
  char *errmsg=NULL;
  setgmode(3);
  initsound(SOUND_OFF,0,NULL);
  updatesound();
  if (statsflag)
    eogstats();
  if (gamemode==GAME_MULTIPLE)
    errmsg=finishcomms(closef);
  else
    if (gamemode==GAME_ASYNCH)
      errmsg=finishserial();
  restoreints();
  flushhistory();
  printstr("\r\n");
  if (errmsg) {
    printstr(errmsg);
    printstr("\r\n");
  }
  if (msg) {
    printstr(msg);
    printstr("\r\n");
  }
  flushbuf();
  if (msg!=NULL || errmsg!=NULL)
    exit(1);
  exit(0);
}

void eogstats(void)
{
  printstr("\r\nEnd of game statistics\r\n\r\n"); /* Was statictics */
  printstr("Objects used: ");
  writenum(((int)maxobjectp-(int)firstobject)/sizeof(object)+1,3);
  printstr("\r\n");
}

int getmaxplayers(void)
{
  int n;
  clearwin();
  printstr(" Key maximum number of players allowed");
  while (1) {
    if (testbreak())
      finish(NULL,FALSE);
    n=inkeys()-'0';
    if (n>=1 && n<=4)
      return n;
  }
}

bool updateremoteplane(object *plane)
{
  planeisplayer=FALSE;
  planeisenemy=FALSE;
  finaleflag=finaleflags[planeindex=plane->index];
  if (latencycount==0)
    processkeys(plane,gamemode==GAME_MULTIPLE ? getmultiplekeys(plane) :
                                                getasynchkeys(plane));
  else {
    plane->deltarot=0;
    plane->bombing=FALSE;
  }
  if ((plane->status==STATUS_DEAD || plane->status!=STATUS_ELIMDEAD) &&
      plane->counter<=0 && plane->fuel>-5000) {
    plane->deaths++;
    createplane(plane);
  }
  return updateplane(plane);
}

void initcomms(void)
{
  world=&worlds[0];
}

char *finishcomms(bool closef)
{
  return NULL;
}

char *finishserial(void)
{
  return NULL;
}

int inithistory(int randno)
{
  long historybufsize;
  FILE *inputfile;
  if (playbackfilename!=NULL || recordfilename!=NULL) {
    historybuf=(int huge *)farmalloc(historybufsize=farcoreleft());
    historyp=(int huge *)historybuf;
  }
  if (playbackfilename!=NULL) {
    inputfile=fopen(playbackfilename,"rb");
    if (inputfile==NULL)
      freeandexit("Unable to open history input file");
    fseek(inputfile,0,2);
    historyfilelen=ftell(inputfile);
    if (historybufsize<historyfilelen)
      freeandexit("Insufficient memory to load history file");
    historyspace=historybufsize-historyfilelen;
    fseek(inputfile,0,0);
    fread((void *)historybuf,(int)historyfilelen,1,inputfile);
    fclose(inputfile);
    randno=historyp[0];
    historyp++;
    historyfilelen-=4;
  }
  if (recordfilename!=NULL) {
    outputfile=fopen(recordfilename,"wb");
    if (outputfile==NULL)
      freeandexit("Unable to open history output file");
    if (playbackfilename==NULL) {
      historyfilelen=historybufsize;
      historyp[0]=randno;
      historyp[1]=(int)(historybufsize>>4);
      historyp++;
      historyfilelen-=2;
    }
  }
  return randno;
}

void freehistorybuf(void)
{
  if (historybuf!=NULL)
    farfree((void *)historybuf);
}

void freeandexit(char *errmsg)
{
  printstr(errmsg);
  freehistorybuf();
  exit(0);
}

int history(int key)
{
  if (playbackfilename!=NULL)
    if (key)
      playbackfilename=NULL;
    else {
      if (historyp[0]==frametick) {
        key=historykey=historyp[1];
        historyp+=2;
        historyfilelen-=4;
        if (historyfilelen==0) {
          playbackfilename=NULL;
          historyfilelen=historyspace;
        }
      }
      return historykey;
    }
  if (recordfilename!=NULL && key!=historykey) {
    historyp[0]=frametick;
    historyp[1]=key;
    recording=TRUE;
    historykey=key;
    historyp+=2;
    historyfilelen-=4;
    if (historyfilelen==0)
      recordfilename=NULL;
    return historykey;
  }
  return key;
}

void flushhistory(void)
{
  if (recording)
    fwrite((void *)historybuf,(int)(historyp-historybuf)*sizeof(int),1,
           outputfile);
  if (outputfile!=NULL) {
    fclose(outputfile);
    freehistorybuf();
  }
}

int getasynchkeys(object *plane)
{
  return 0;
}

int getmultiplekeys(object *plane)
{
  return 0;
}

void setvel(object *obj,int v,int dir)
{
  int xv=fcos(v,dir),yv=fsin(v,dir);
  obj->xv=xv>>8;
  obj->xvfrac=xv<<8;
  obj->yv=yv>>8;
  obj->yvfrac=yv<<8;
}

void printstr(char *str)
{
  while (*str!=0)
    writechar(*(str++));
}

void moveobject(object *obj,int *x,int *y)
{
  long pos,vel;
  pos=(((long)(obj->x))<<16)+obj->xfrac;
  vel=(((long)(obj->xv))<<16)+obj->xvfrac;
  pos+=vel;
  obj->x=(short)(pos>>16);
  obj->xfrac=(short)pos;
  *x=obj->x;
  pos=(((long)(obj->y))<<16)+obj->yfrac;
  vel=(((long)(obj->yv))<<16)+obj->yvfrac;
  pos+=vel;
  obj->y=(short)(pos>>16);
  obj->yfrac=(short)pos;
  *y=obj->y;
}

void writenum(int n,int width)
{
  int c=0,zerof=1,exponent,dig;
  if (n<0) {
    n=-n;
    writechar('-');
    c++;
  }
  for (exponent=10000;exponent>1;exponent/=10) {
    if ((dig=n/exponent)!=0 || zerof==0) {
      zerof=0;
      writechar(dig+'0');
      c++;
    }
    n=n%exponent;
  }
  writechar(n+'0');
  c++;
  do {
    c++;
    if (c>width)
      break;
    writechar(' ');
  } while (1);
}

void initasynch(void)
{
}

void createasynchremoteplane(void)
{
  object *plane;
  if (lastplane==0)
    createplayerplane(NULL);
  plane=createplane(NULL);
  plane->soundfunc=soundplane;
  plane->updatefunc=updateremoteplane;
  plane->colour=1+(plane->index&1);
  plane->source=plane;
  plane->status=STATUS_NORMAL;
  memcpy(&homes[plane->index],plane,sizeof(object));
  if (lastplane!=0)
    createplayerplane(NULL);
}

void createmultipleremoteplanes(void)
{
}

void getopt(int *argc,char **argv[],char *format,...)
{
  va_list ap;
  char **varg=*argv;
  int carg=*argc;
  char *arg,*p,*q,*a;
  int i,**valp;
  varg++;

  while ((--carg)>0) {
    arg=*varg;
    if (*(arg++)!='-')
      break;
    if (*arg=='-') {
      varg++;
      carg--;
      break;
    }
    p=arg;
    va_start(ap,format);
    q=format;
    while (*q!=0) {
      if (*q==':')
        break;
      i=0;
      while (isalpha(q[i]))
        i++;
      if (strncmp(p,q,i)==0)
        break;
      q+=i;
      if ((*q)!=0)
        q++;
      va_arg(ap,int);
    }
    if (*p!=*q) {
      printf("Usage: ");
      a=strchr(format,':');
      if (a!=0) {
        *(a++)=0;
        q=strchr(a,'*');
        if (q!=0) {
          *(q++)=0;
          printf("%s [-%s] %s\n",a,format,q);
        }
        else
          printf("%s\n",a);
      }
      else
        printf("cmd [-%s] args\n",format);
      exit(0);
    }
    p=q+i;
    arg+=i;
    valp=(int **)(&va_arg(ap,int));
    switch(*p) {
      case '#':
        a="%d";
        if (*arg=='0') {
          switch (*(++arg)) {
            case 'o':
              a="%o";
              arg++;
              break;
            case 'x':
              a="%x";
              arg++;
          }
        }
        sscanf(arg,a,*valp);
        break;
      case '*':
        **valp=(int)arg;
        break;
      case '&':
        **valp=(int)TRUE;
        break;
      default:
        printf("Unknown format %c.\n",*p);
    }
    varg++;
  }
  *argv=varg;
  *argc=carg;
}

object *newobject(void)
{
  object *newobj;
  if (nextobject==NULL)
    return NULL;
  newobj=nextobject;
  nextobject=newobj->next;
  newobj->next=NULL;
  newobj->prev=lastobject;
  if (lastobject!=NULL)
    lastobject->next=newobj;
  else
    firstobject=newobj;
  newobj->oldonscreen=newobj->onscreen=FALSE;
  newobj->sndp=NULL;
  if (newobj>maxobjectp)
    maxobjectp=newobj;
  return lastobject=newobj;
}

void deleteobject(object *obj)
{
  object *other=obj->prev;
  if (other!=NULL)
    other->next=obj->next;
  else
    firstobject=obj->next;
  other=obj->next;
  if (other!=NULL)
    other->prev=obj->prev;
  else
    lastobject=obj->prev;
  obj->next=NULL;
  if (lastdeleted)
    lastdeleted->next=obj;
  else
    firstdeleted=obj;
  lastdeleted=obj;
}

void clearscorearea(void)
{
  cgafbar(0,184,48,16,0);
}

void drawobject(int x,int y,object *obj)
{
  putimage(x,y,obj->sprite,obj);
}

unsigned char blut[256]={
0x00,0x03,0x03,0x03,0x0c,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0f,0x0c,0x0f,0x0f,0x0f,
0x30,0x33,0x33,0x33,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,
0x30,0x33,0x33,0x33,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,
0x30,0x33,0x33,0x33,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,0x3c,0x3f,0x3f,0x3f,
0xc0,0xc3,0xc3,0xc3,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xc0,0xc3,0xc3,0xc3,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xc0,0xc3,0xc3,0xc3,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,0xcc,0xcf,0xcf,0xcf,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,
0xf0,0xf3,0xf3,0xf3,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff,0xfc,0xff,0xff,0xff
};

int bitblt(int x,int y,object *obj)
{
  unsigned int linediff,bltreg;
  unsigned char far *scp;
  int width,height,border;
  unsigned char retval=0,shift,bltleft;
  unsigned char *p=obj->sprite;
  if (obj->height==1 && obj->width==1)
    return pixel(x,y,(int)p);
  shift=8-((x&3)<<1);
  width=(obj->width)>>2;
  border=width-(80-(x>>2));
  if (border>0)
    width=80-(x>>2);
  height=(obj->height>y+1 ? y+1 : obj->height);
  scp=(unsigned char far *)MK_FP(scrseg,scroff+((199-y)>>1)*80+(x>>2));
  linediff=interlacediff;
  if ((y&1)==0) {
    scp+=linediff;
    linediff=80-linediff;
  }
  do {
    bltleft=0;
    for (x=0;x<width;x++) {
      bltreg=((*p++)<<shift)|(bltleft<<8);
      retval|=(*scp)&blut[bltreg>>8];
      *(scp++)^=(bltreg>>8);
      bltleft=bltreg;
    }
    if (border>=0)
      p+=border;
    else
      if (bltleft!=0) {
        retval|=(*scp)&blut[bltleft];
        (*scp)^=bltleft;
      }
    scp+=linediff-width;
    linediff=80-linediff;
    height--;
  } while (height!=0);
  return retval;
}

void putimage(int x,int y,unsigned char *p,object *obj)
{
  unsigned int linediff,bltreg;
  unsigned char far *scp;
  int width,height,border;
  unsigned char shift,bltleft,col;
  if (obj->height==1 && obj->width==1) {
    putpixel(x,y,(int)p);
    return;
  }
  col=(obj->colour==1 ? 0 : 0xff);
  shift=8-((x&3)<<1);
  width=(obj->width)>>2;
  border=width-(80-(x>>2));
  if (border>0)
    width=80-(x>>2);
  height=(obj->height>y+1 ? y+1 : obj->height);
  scp=(unsigned char far *)MK_FP(scrseg,scroff+((199-y)>>1)*80+(x>>2));
  linediff=interlacediff;
  if ((y&1)==0) {
    scp+=linediff;
    linediff=80-linediff;
  }
  do {
    bltleft=0;
    for (x=0;x<width;x++) {
      bltreg=((*p++)<<shift)|(bltleft<<8);
      *(scp++)^=(bltreg>>8)^(blut[bltreg>>8]&col);
      bltleft=bltreg;
    }
    if (border>=0)
      p+=border;
    else
      if (bltleft!=0)
        (*scp)^=bltleft^(blut[bltleft]&col);
    scp+=linediff-width;
    linediff=80-linediff;
    height--;
  } while (height!=0);
}

void putpixel(int x,int y,int c)
{
  int shift=(3-(x&3))<<1;
  unsigned char far *scp=(unsigned char far *)MK_FP(scrseg,scroff+((199-y)>>1)*80+(x>>2)+((y&1)==0 ? interlacediff : 0));
  if ((c&0x80)==0)
    *scp=((*scp)&(~(3<<shift)))|(c<<shift);
  else
    *scp^=((c&0x7f)<<shift);
}

int pixel(int x,int y,int c)
{
  int shift=(3-(x&3))<<1;
  unsigned char far *scp=(unsigned char far *)MK_FP(scrseg,scroff+((199-y)>>1)*80+(x>>2)+((y&1)==0 ? interlacediff : 0));
  unsigned char o=(3<<shift)&(*scp);
  if ((c&0x80)==0) {
    *scp^=o;
    *scp|=(c<<shift);
  }
  else
    *scp^=(c&0x7f)<<shift;
  return o>>shift;
}

