/* Source code for Sopwith 1
   Reverse-engineered by Andrew Jenner

   Copyright (c) 1984-2000 David L Clark
   Copyright (c) 1999-2000 Andrew Jenner

   All rights reserved except as specified in the file license.txt.
   Distribution of this file without the license.txt file accompanying is
   prohibited.
*/

#include <stdio.h>
#include <dos.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "sopasm.h"
#include "def.h"

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
  int width;
  bool bombing;
  int bombs,colour;
  unsigned int xfrac,yfrac,xvfrac,yvfrac;
  struct object *next,*prev;
  int index;
  int oldx,oldy;
  bool onscreen;
  unsigned char *oldsprite;
  struct object *nexterase,*nextdraw;
  void (*drawfunc)(struct object *p,int x,int y);
  bool (*updatefunc)(struct object *p);
  void (*erasefunc)(struct object *p,int x,int y);
  struct object *nextx,*prevx;
  int deaths;
  unsigned char *sprite;
  int bombtime;
  bool homing;
  int xaux[3],yaux[3];
  int type;
};

typedef struct object object;

typedef struct
{
  int deltarot,deltav,xv,yv,xvfrac,yvfrac,newspd,alt,d2;
} autoinf;

typedef struct
{
  int *planeposes;
  bool *planeflip;
  int *buildingpositions;
  int *buildingtypes;
} landscape;

typedef struct
{
  object *erase;
  object *draw;
} redrawlist;

typedef struct
{
  int colour,x,y;
} mapobject;

int latency=-1;
int sine[16]={0x000, 0x062, 0x0b5, 0x0ed, 0x100, 0x0ed, 0x0b5, 0x062,
              0x000,-0x062,-0x0b5,-0x0ed,-0x100,-0x0ed,-0x0b5,-0x062};
int soundtype=0x7fff,soundpitch=0x7fff;
object *soundobj=NULL;
unsigned int t2v=0;
object *sndobj=NULL;
void (*soundperiodicfunc)(void)=NULL;
char *tune1[7]={
  ">e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d1/",
  "d4./c8/b4/c4/d4/c+4/d4/b4/c4/c4/c4/c1/<g4./g+8/",
  ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>d4/d4/d2./<g4./g+8/",
  ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>e4/e4/e2./",
  "e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d2/c4/<g+4/>a4/",
  "d2/e2/g1/",
  ""};
char *tune2[7]={
  ">e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d1/",
  "d4./c8/b4/c4/d4/c+4/d4/b4/c4/c4/c4/c1/<g4./g+8/",
  ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>d4/d4/d2./<g4./g+8/",
  ">a4./a-8/<g4./g+8/>a4/a-4/<g4/>e4/e4/e2./",
  "e4./d8/c4/d4/e4/d+4/e4/c4/d4/d4/d4/d2/c4/<g+4/>a4/",
  "d2/e2/g1/",
  ""};
int enginestutter[50]={
  0x90b9,0xbcfb,0x6564,0x3313,0x3190,0xa980,0xbcf0,0x6f97,0x37f4,0x064b,
  0x9fd8,0x595b,0x1eee,0x820c,0x4201,0x651e,0x848e,0x15d5,0x1de7,0x1585,
  0xa850,0x213b,0x3953,0x1eb0,0x97a7,0x35dd,0xaf2f,0x1629,0xbe9b,0x243f,
  0x847d,0x3a31,0x3295,0xbc11,0x6e6d,0x3398,0xad43,0x51ce,0x8f95,0x507e,
  0x499e,0x3bc1,0x5243,0x2017,0x9510,0x9865,0x65f6,0x6b56,0x36b9,0x5026};
int stutterp=0;
int majorscale[7]={0,2,3,5,7,8,10};
int notefreq[12]={440,466,494,523,554,587,622,659,698,740,784,831};
int singleplanes[2]={0,7};
int computerplanes[4]={0,7,1,6};
int multipleplanes[8]={0,7,3,4,2,5,1,6};
unsigned char writecharcol=3;
int crater[8]={1,2,2,3,3,2,2,1};

int worldplaneposes[8]={1270,588,1330,1360,1630,1660,2464,1720};
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

bool finishflag=FALSE;

autoinf choices[5]={{ 0, 0,0,0,0,0,0,0,0},
                    {-1, 0,0,0,0,0,0,0,0},
                    { 1, 0,0,0,0,0,0,0,0},
                    { 0,-1,0,0,0,0,0,0,0},
                    { 0, 1,0,0,0,0,0,0,0}};

#include "sprites.c"
#include "ground.c"

int gamemode=0;
landscape *world=NULL;
object *buildings[20];
int buildingstodestroy[2];
int timertick=0;
int frametick=0;

bool hiresflag=FALSE;
bool posindflag=FALSE;
bool notitlesflag=FALSE;
bool soundflag=FALSE;
int scnleft=0;
int scnright=0;
int pixtoscroll=0;
int scrseg=0;
int scroff=0;
int interlacediff=0;

unsigned char scrbuf[0x2000];
bool objsnotonscreen;
object objects[150];
object homes[4];
object *lastobject,*firstobject,*nextobject,*firstdeleted,*lastdeleted;
object objleft,objright;
object *attackees[4];
int finaleflags[4];
int finaletime,playerme;
redrawlist ylist[200];
mapobject mapobjects[150];
int finalesx,finalesy,finalex,finaley;
bool finaleflying;
unsigned char finalesqrt[101];

int latencycount;
int finaleflag;
int sndt2v[100];
int snddeltat2v[100];
int slidec,pa11c,tuneptr2,tuneline2,t2v2,tunetime2,octavemul2;
bool tune3f;
int tuneptr3,tuneline3,t2v3,tunetime3,octavemul3;
char **tune;
int tuneline,tuneptr;
int t2v1,tunedur;
int octavemultiplier;
int ot2v;

object *collobj1[150],*collobj2[150];
int collc;

bool dontcheckcoll,autoisplayer;
object *objlist[50];
int objcnt;
bool keyhandlerinstalled=FALSE;

void main(int argc,char *argv[]);
void printstr(char *str);
void updatescreen(void);
void updateobjects(void);
void update(object *obj);
bool updateplayerplane(object *p);
void processkey(object *p,char key);
void updatescroll(object *player);
bool updateenemyplane(object *plane);
bool updateremoteplane(object *plane);
bool updateplane(object *plane);
void attackable(object *obj);
bool ishome(object *plane);
bool isatfinalepos(object *plane);
void refuel(object *plane);
void increment(int *val,int max);
bool updatebullet(object *bullet);
bool updatebomb(object *bomb);
int direction(object *obj);
bool updatebuilding(object *building);
bool updateshrapnel(object *shrapnel);
bool updatesmoke(object *smoke);
void killplane(object *plane);
void catchfire(object *plane);
void gointospin(object *plane);
void insxlist(object *ins,object *list);
void delxlist(object *del);
void initsound(int type,int pitch,object *obj);
void updatesound(void);
void updatesoundint(void);
void soundslide(void);
void soundtoggle(void);
void updatetune2(void);
void setsound2(void);
void updatetune3(void);
void sound3(void);
void objectsound(object *obj,int snd);
void killsnd(object *o);
void ssound(int t2);
void snosound(void);
int getenginestutter(int f);
void updatetune(void);
void init(int argc,char *argv[]);
void clearwin(void);
void getgamemode(void);
int getgamenumber(void);
bool testbreak(void);
void initscreen(bool drawnground);
void writescores(void);
void livesbar(object *p);
void fuelbar(object *p);
void bombsbar(object *p);
void ammobar(object *p);
void statbar(int x,int h,int hmax,int col);
void drawinitialground(void);
void drawmapground(void);
void drawmapbuildings(void);
void useoffscreenbuf(void);
void useonscreenbuf(void);
void copyoffscbuftoscn(void);
void initlists(void);
void createenemyplane(object *pl);
void createplayerplane(object *player);
object *createplane(object *pl);
void firebullet(object *plane);
void dropbomb(object *plane);
void createbuildings(void);
void createexplosion(object *obj);
void createsmoke(object *plane);
void setaux(object *obj);
void initcomms(void);
void createmultiplanes(void);
int getmaxplayers(void);
int inkey(void);
int clearbuf(void);
void setcolour(int c);
void checkcollisions(void);
void checkcollision(object *obj1,object *obj2);
void checkcrash(object *obj);
void docollision(object *obj1,object *obj2);
void score(object *destroyed,int score);
void score50(object *destroyed);
void writescore(object *p);
void writenum(int n,int width);
void digcrater(object *obj,int depth);
void moveobject(object *obj,int *x,int *y);
void moveaux(object *obj,int *x,int *y,int i);
void setvel(object *obj,int v,int dir);
void updateground(void);
int suny(int x);
unsigned char draw(int x,int y,object *p);
void erase(int x,int y,object *p);
void drawplayerplane(object *plane,int x,int y);
void drawbullet(object *bullet,int x,int y);
void drawbomb(object *bomb,int x,int y);
void drawbuilding(object *building,int x,int y);
void drawshrapnel(object *shrapnel,int x,int y);
void drawsmoke(object *smoke,int x,int y);
void drawmapobject(object *obj);
void drawenemyplane(object *plane,int x,int y);
void drawremoteplane(object *plane,int x,int y);
void planesound(object *plane);
void erasebullet(object *bullet,int x,int y);
void erasebomb(object *bomb,int x,int y);
void erasebuilding(object *buliding,int x,int y);
void eraseshrapnel(object *shrapnel,int x,int y);
void erasesmoke(object *smoke,int x,int y);
void eraseplayerplane(object *plane,int x,int y);
void eraseenemyplane(object *plane,int x,int y);
void eraseremoteplane(object *plane,int x,int y);
void finish(char *msg,bool closef);
void declarewinner(int n);
void createsun(object *player);
void gameover(object *plane);
int pixel(int x,int y,int c);
object *newobject(void);
void deleteobject(object *obj);
void titles(void);
void startsound(void);
void timerint(void);
void enemylogic(object *plane);
void enemyattack(object *plane,object *attackee);
bool ontarget(object *plane,object *target);
int flyhome(object *plane);
int flytofinale(object *plane);
int autopilot(object *plane,int destx,int desty,object *bpa);
bool ailogic(object *plane,int ch1,int ch2,int rdepth);
bool checkcollide(object *plane,int xleft,int ybottom,int rot);
void mkobjlist(object *plane);
object *hitobj(object *plane,int rdepth,int xleft,int ybottom);
int distsqr(int x0,int y0,int x1,int y1);
unsigned char bitblt(int y,int x,unsigned char *p,int height,int width);
void getopt(int *argc,char **argv[],char *format,...);
char *finishcomms(bool closef);
char getremotekey(object *player);

void main(int argc,char *argv[])
{
  init(argc,argv);
  while (1) {
    updateobjects();
    checkcollisions();
    updatescreen();
    updatesound();
  }
}

void printstr(char *str)
{
  while (*str!=0)
    writechar(*(str++));
}

void updatescreen(void)
{
  int bytestomove,bytestoscroll,offset,ioffset,scrolltop,y,temp;
  object *obj;
  redrawlist *objl;
  useonscreenbuf();
  bytestoscroll=pixtoscroll>>2;
  if (bytestoscroll<0) {
    bytestomove=bytestoscroll+80;
    offset=0;
  }
  else {
    offset=bytestoscroll;
    bytestomove=80-bytestoscroll;
  }
  scrolltop=75;
  if (ground[scnleft-pixtoscroll]>=scrolltop)
    scrolltop=ground[scnleft-pixtoscroll];
  if (ground[scnright-pixtoscroll]>=scrolltop)
    scrolltop=ground[scnright-pixtoscroll];
  if (finaleflying)
    if (finalesy+50>=scrolltop)
      scrolltop=finalesy+50;
  scrolltop|=1;
  offset+=((199-scrolltop)>>1)*80;
  ioffset=interlacediff+offset;
  objl=&ylist[199];
  for (y=199;y>=0;y--) {
    obj=objl->erase;
    while (obj!=NULL && !objsnotonscreen) {
      obj->erasefunc(obj,obj->oldx,obj->oldy);
      obj=obj->nexterase;
    }
    objl->erase=NULL;
    if (y>=16 && y<=scrolltop) {
      farmemmove(MK_FP(scrseg,offset),MK_FP(scrseg,offset-bytestoscroll),
                 bytestomove);
      temp=ioffset;
      ioffset=offset+80;
      offset=temp;
    }
    obj=objl->draw;
    while (obj!=NULL) {
      obj->onscreen=FALSE;
      if (obj->x>=scnleft && obj->x<=scnright) {
        obj->onscreen=TRUE;
        obj->oldx=obj->x-scnleft;
        obj->oldy=obj->y;
        obj->drawfunc(obj,obj->oldx,obj->oldy);
      }
      obj=obj->nextdraw;
    }
    objl->draw=NULL;
    objl--;
  }
  objsnotonscreen=FALSE;
  updateground();
}

void updateobjects(void)
{
  object *next,*current;
  if (firstdeleted!=NULL) {
    lastdeleted->next=nextobject;
    nextobject=firstdeleted;
    firstdeleted=lastdeleted=NULL;
  }
  latencycount++;
  if (latencycount>=latency)
    latencycount=0;
  current=firstobject;
  while (current!=NULL) {
    next=current->next;
    update(current);
    current=next;
  }
  frametick++;
}

void update(object *obj)
{
  bool oldonscreen;
  int ydraw,yerase;
  redrawlist *ydlist;
  oldonscreen=obj->onscreen;
  if (oldonscreen) {
    yerase=obj->y;
    ydraw=yerase-obj->width+1;
  }
  obj->oldsprite=obj->sprite;
  if (obj->updatefunc(obj)) {
    if (obj->onscreen)
      if (obj->y<yerase)
        ydraw=(obj->y-obj->width)+1;
      else
        yerase=obj->y;
    else
      ydraw=(obj->y-obj->width)+1;
    ydlist=&ylist[ydraw];
    obj->nextdraw=ydlist->draw;
    ydlist->draw=obj;
  }
  else
    obj->onscreen=FALSE;
  if (oldonscreen) {
    ydlist=&ylist[yerase];
    obj->nexterase=ydlist->erase;
    ydlist->erase=obj;
  }
}

bool updateplayerplane(object *player)
{
  bool retval;
  int key;
  finaleflag=finaleflags[player->index];
  if (finaleflag!=0) {
    finaletime--;
    if (finaletime<=0)
      finish(NULL,TRUE);
  }
  if (latencycount==0) {
    if (gamemode==GAME_MULTIPLE)
      key=getremotekey(player);
    else {
      if (testbreak())
        key=1;
      else
        key=inkey();
      clearbuf();
    }
    processkey(player,key);
  }
  else {
    player->deltarot=0;
    player->deltav=0;
    player->bombing=FALSE;
  }
  if (player->status==STATUS_DEAD && player->counter<=0) {
    if (finaleflag!=1 && ((++(player->deaths))>=5 || player->fuel<=-5000)) {
      if (finaleflag==0) {
        gameover(player);
        if (gamemode==GAME_MULTIPLE)
          declarewinner(1);
      }
    }
    else {
      createplayerplane(player);
      initscreen(TRUE);
      if (finaleflag==1) {
        if (testbreak())
          finish(NULL,TRUE);
        createsun(player);
      }
    }
  }
  retval=updateplane(player);
  updatescroll(player);
  if (posindflag) {
    poscurs(0,24);
    printstr("Pos: ");
    writenum(player->x,5);
    writenum(player->y,3);
  }
  fuelbar(player);
  if (ishome(player)) {
    ammobar(player);
    bombsbar(player);
    livesbar(player);
  }
  else {
    if (player->firing)
      ammobar(player);
    if (player->bombing)
      bombsbar(player);
  }
  return retval;
}

void processkey(object *p,char key)
{
  p->deltarot=0;
  p->deltav=0;
  p->bombing=FALSE;
  p->firing=FALSE;
  if (p->status!=STATUS_NORMAL && p->status!=STATUS_SPINNING)
    return;
  if (finaleflag!=0) {
    if (finaleflag==1 && p->index==playerme)
      flytofinale(p);
    else
      flyhome(p);
    return;
  }
  switch (key) {
    case 1:
      p->fuel=-5000;
      p->homing=FALSE;
      if (ishome(p)) {
        p->status=STATUS_DEAD;
        p->counter=0;
      }
      break;
    case ',':
      p->deltarot=1;
      p->homing=FALSE;
      break;
    case '/':
      p->deltarot=-1;
      p->homing=FALSE;
      break;
    case '.':
      p->inverted=!(p->inverted);
      p->homing=FALSE;
      break;
    case '\\':
      p->deltav=-1;
      p->homing=FALSE;
      break;
    case 'x':
      p->deltav=1;
      p->homing=FALSE;
      break;
    case ' ':
      p->firing=TRUE;
      break;
    case 'b':
      p->bombing=TRUE;
      break;
    case 'h':
      p->homing=TRUE;
      break;
    case 's':
      if (p->index==playerme) {
        if (soundflag) {
          initsound(SOUND_OFF,0,NULL);
          updatesound();
        }
        soundflag=!soundflag;
      }
      break;
    default:
      break;
  }
  if (p->homing)
    flyhome(p);
}

void updatescroll(object *player)
{
  int xv,x;
  if (finaleflag==2 || (finaleflag==1 && isatfinalepos(player))) {
    pixtoscroll=0;
    return;
  }
  x=player->x-scnleft;
  xv=player->xv;
  if (x<152) {
    if (xv<0)
      pixtoscroll=-((-xv)|3)-1;
  }
  else
    if (x>152 && xv>0)
      pixtoscroll=1+(xv|3);
  if (pixtoscroll<0) {
    if (x>=232)
      pixtoscroll=0;
  }
  else
    if (pixtoscroll>0 && x<=72)
      pixtoscroll=0;
  if (pixtoscroll+scnleft<0 || pixtoscroll+scnright>=3000)
    pixtoscroll=0;
  scnleft+=pixtoscroll;
  scnright+=pixtoscroll;
}

bool updateenemyplane(object *plane)
{
  plane->deltarot=0;
  plane->deltav=0;
  plane->bombing=FALSE;
  finaleflag=finaleflags[plane->index];
  if (latencycount==0)
    plane->firing=FALSE;
  switch (plane->status) {
    case STATUS_NORMAL:
    case STATUS_SPINNING:
      if (finaleflag)
        flyhome(plane);
      else
        if (latencycount==0)
          enemylogic(plane);
      break;
    case STATUS_DEAD:
      plane->firing=FALSE;
      if (plane->counter<=0 && finaleflag==0)
        createenemyplane(plane);
      break;
    default:
      plane->firing=FALSE;
  }
  return updateplane(plane);
}

bool updateremoteplane(object *plane)
{
  finaleflag=finaleflags[plane->index];
  if (latencycount==0)
    processkey(plane,getremotekey(plane));
  else {
    plane->deltarot=0;
    plane->deltav=0;
    plane->bombing=FALSE;
  }
  if (plane->status==STATUS_DEAD && plane->counter<=0 && plane->fuel>-5000 &&
      (++(plane->deaths))<5)
    createplane(plane);
  return updateplane(plane);
}

bool updateplane(object *plane)
{
  int rotation,maxv,x,y,speed;
  switch (plane->status) {
    case STATUS_ELIMINATED:
    case STATUS_UNKNOWN:
      return FALSE;
    case STATUS_DEAD:
      plane->counter--;
      break;
    case STATUS_FALLING:
      plane->counter--;
      if (plane->counter==0) {
        if (plane->yv>-10)
          plane->yv--;
       plane->counter=5;
      }
      if (plane->yv<=0)
        objectsound(plane,SOUND_FALLING);
      break;
    case STATUS_NORMAL:
    case STATUS_SPINNING:
      if (plane->fuel<=0) {
        if (!ishome(plane)) {
          catchfire(plane);
          score50(plane);
          return updateplane(plane);
        }
      }
      if (plane->status==STATUS_SPINNING && plane->rotation==12)
        plane->status=STATUS_NORMAL;
      if (plane->status==STATUS_NORMAL && plane->y>=200)
        gointospin(plane);
      if (plane->deltarot!=0 || plane->deltav!=0) {
        speed=plane->speed;
        rotation=plane->rotation;
        speed+=plane->deltav;
        if (plane->inverted)
          rotation-=plane->deltarot;
        else
          rotation+=plane->deltarot;
        maxv=(finaleflying ? 4 : 8);
        if (speed<4)
          speed=4;
        else
          if (speed>maxv)
            speed=maxv;
        plane->speed=speed;
        rotation&=15;
        plane->rotation=rotation;
        if (plane->status==STATUS_NORMAL)
          setvel(plane,speed,rotation);
        else {
          plane->xv=0;
          plane->yv=-speed;
          plane->xvfrac=0;
          plane->yvfrac=0;
        }
      }
      if (plane->status==STATUS_SPINNING) {
        plane->counter--;
        if (plane->counter==0) {
          plane->inverted=!(plane->inverted);
          plane->rotation=(8-plane->rotation)&15;
          plane->counter=2;
        }
      }
      if (plane->firing)
        firebullet(plane);
      if (plane->bombing)
        dropbomb(plane);
      plane->fuel-=plane->speed;
  }
  if (finaleflag==1 && plane->index==playerme && isatfinalepos(plane))
    plane->sprite=finalesprites[(plane->colour-1)&1][finaletime/18];
  else
    if (plane->status==STATUS_ELIMINATED)
      plane->sprite=NULL;
    else
      if (plane->status==STATUS_FALLING)
        plane->sprite=planesprites[(plane->colour-1)&1]
                                  [plane->inverted ? 1 : 0]
                                  [direction(plane)<<1];
      else
        plane->sprite=planesprites[(plane->colour-1)&1]
                                  [plane->inverted ? 1 : 0]
                                  [plane->rotation];
  moveobject(plane,&x,&y);
  if (x<0)
    x=plane->x=0;
  else
    if (x>=2984)
      x=plane->x=2984;
  if (plane->status==STATUS_NORMAL || plane->status==STATUS_SPINNING)
    attackable(plane);
  delxlist(plane);
  insxlist(plane,plane->nextx);
  if (plane->bombtime!=0)
    plane->bombtime--;
  if (ishome(plane))
    refuel(plane);
  if (y<200 && y>=0) {
    if (plane->status==STATUS_FALLING)
      createsmoke(plane);
    useonscreenbuf();
    drawmapobject(plane);
    return TRUE;
  }
  return FALSE;
}

void attackable(object *obj)
{
  object *plane,*attackee;
  int i;
  for (i=0,plane=firstobject;i<4;i++,plane++) {
    if (plane->drawfunc==drawenemyplane &&
        obj->source->colour!=plane->source->colour) {
      attackee=attackees[i];
      if (attackee==NULL ||
          (attackee->type==OBJ_BUILDING && obj->type==OBJ_PLANE)) {
        if (gamemode==GAME_COMPUTER && abs(homes[i].x-obj->x)<=600)
          attackees[i]=obj;
      }
      else
        if (attackee->type!=OBJ_PLANE || obj->type!=OBJ_BUILDING &&
            abs(obj->x-plane->x)<abs(attackee->x-plane->x))
          attackees[i]=obj;
    }
  }
}

bool ishome(object *plane)
{
  object *home=&homes[plane->index];
  return (plane->x==home->x && plane->y==home->y && plane->speed==0 &&
          plane->status==STATUS_NORMAL);
}

bool isatfinalepos(object *plane)
{
  return (plane->x==finalex && plane->y==finaley && plane->speed==0 &&
          plane->status==STATUS_NORMAL);
}

void refuel(object *plane)
{
  increment(&(plane->fuel),9000);
  increment(&(plane->ammo),200);
  increment(&(plane->bombs),5);
}

void increment(int *val,int max)
{
  if (*val==max)
    return;
  if (max<20) {
    if (frametick%20==0)
      (*val)++;
  }
  else
    *val+=max/100;
  if (*val>max)
    *val=max;
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
  if (x<0 || x>=3000 || y>=200 || y<=ground[x]) {
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
  if (!(y>=0 && x>=0 && x<3000)) {
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
  bomb->sprite=bombsprites[(bomb->source->colour-1)&1][direction(bomb)];
  useonscreenbuf();
  drawmapobject(bomb);
  return TRUE;
}

int direction(object *obj)
{
  int xv=obj->xv,yv=obj->yv;
  if (xv==0) {if (yv<0) return 6; if (yv>0) return 2; return 6; }
  if (xv>0)  {if (yv<0) return 7; if (yv>0) return 1; return 0; }
              if (yv<0) return 5; if (yv>0) return 3; return 4;
}

bool updatebuilding(object *building)
{
  if (building->status==STATUS_INTACT) {
    attackable(building);
    building->sprite=buildingsprites[(building->source->colour-1)&1]
                                    [building->kind];
  }
  else
    building->sprite=debrissprites[(building->source->colour-1)&1];
  return TRUE;
}

bool updateshrapnel(object *shrapnel)
{
  int x,y;
  delxlist(shrapnel);
  shrapnel->fuel--;
  if (shrapnel->fuel==0) {
    killsnd(shrapnel);
    deleteobject(shrapnel);
    return FALSE;
  }
  moveobject(shrapnel,&x,&y);
  if (x<0 || x>=3000 || y>=200 || y<=ground[x]) {
    killsnd(shrapnel);
    deleteobject(shrapnel);
    return FALSE;
  }
  insxlist(shrapnel,shrapnel->nextx);
  shrapnel->counter++;
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

void killplane(object *plane)
{
  if (plane->xv<0)
    plane->rotation=(plane->rotation+2)&15;
  else
    plane->rotation=(plane->rotation-2)&15;
  plane->status=STATUS_DEAD;
  plane->xv=0;
  plane->yv=0;
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->speed=0;
  plane->counter=10;
}

void catchfire(object *plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->rotation=12;
  plane->speed=0;
  plane->counter=5;
  plane->status=STATUS_FALLING;
}

void gointospin(object *plane)
{
  plane->xvfrac=0;
  plane->yvfrac=0;
  plane->rotation=14;
  plane->inverted=FALSE;
  plane->speed=6;
  plane->xv=0;
  plane->yv=-6;
  plane->counter=2;
  plane->status=STATUS_SPINNING;
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
  int i;
  int *deltat2vp,*t2vp;
/*  disable(); */
  for (i=0,t2vp=sndt2v,deltat2vp=snddeltat2v;i<100;i++,t2vp++,deltat2vp++)
    if (*t2vp!=0)
      (*t2vp)+=(*deltat2vp)*slidec;
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
      soundslide();
      sndobj=soundobj;
      soundperiodicfunc=soundslide;
      break;
    case SOUND_FALLING:
      if (soundobj==sndobj)
        break;
      soundslide();
      sndobj=soundobj;
      soundperiodicfunc=soundslide;
      break;
    case SOUND_STUTTER:
      ssound(getenginestutter(2)!=0 ? 0x9000 : 0xf000);
      sndobj=NULL;
      soundperiodicfunc=NULL;
      break;
    case SOUND_SHRAPNEL:
      ssound(t2v2);
      sndobj=NULL;
      soundperiodicfunc=NULL;
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
      break;
  }
/*  enable(); */
  soundpitch=0x7fff;
  soundtype=0x7fff;
}

void updatesoundint(void)
{
  slidec++;
  if (t2v!=0 && soundperiodicfunc!=NULL)
    soundperiodicfunc();
  if (pa11c!=0)
    updatetune2();
  if (tune3f)
    updatetune3();
}

void soundslide(void)
{
  int i=sndobj->speed;
  ssound(snddeltat2v[i]*slidec+sndt2v[i]);
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
  tunetime2+=tunedur; /* Interrupts originally cleared for this instruction */
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
  tune=tune2;
  octavemultiplier=octavemul3;
  updatetune();
  tuneline3=tuneline;
  tuneptr3=tuneptr;
  t2v3=t2v1;
  tunetime3+=tunedur; /* Interrupts originally cleared for this instruction */
  octavemul3=octavemultiplier;
  ssound(t2v3);
}

void objectsound(object *obj,int snd)
{
  int i;
  if (obj->speed!=0)
    return;
  if (obj->type==OBJ_SHRAPNEL) {
/*    disable(); */
    pa11c++;
    if (pa11c==1) {
      tuneline2=tuneptr2=0;
      setsound2();
    }
    obj->speed=1;
/*    enable(); */
    return;
  }
  for (i=0;i<100;i++)
    if (sndt2v[i]==0) {
/*      disable(); */
      switch(snd) {
        case SOUND_BOMB:
          sndt2v[i]=0x300;
          snddeltat2v[i]=8;
          break;
        case SOUND_FALLING:
          sndt2v[i]=0x1200;
          snddeltat2v[i]=-8;
          break;
      }
      obj->speed=i;
/*      enable(); */
      return;
    }
}

void killsnd(object *o)
{
  int s=o->speed;
  if (s==0)
    return;
/*  disable(); */
  if (o->type==OBJ_SHRAPNEL)
    pa11c--;
  else {
    sndt2v[s]=0;
    snddeltat2v[s]=0;
  }
  o->speed=0;
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
  outportb(PORT_TIMER2,t2&0xff);
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

/* This array ought to be local to updatetune(). However, sometimes this
   function is called via an interrupt, and sometimes on these occasions the
   interrupt will have occurred during some sort of system call. Sometimes when
   this happens the stack segment will be different from this program's stack
   segment, so SS will not be equal to DS, which means that if tempobuf[] is
   dynamically allocated (on the stack, i.e. in SS) and referenced through DS,
   memory will be corrupted. You have no idea how long this bug took to find.
   I can't think of a better way to fix it.
*/
char tempobuf[5];

void updatetune(void)
{
  bool notgotchar=TRUE;
  int overflowoct=0x100,freq,tempo,semitone,sharpen=0,dotdur=2,tempobufp=0;
  char tunechar,tunechar2;
  while (1) {
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
        switch (tunechar) {
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
  t2v1=(short)(1193181l/freq);    /* Was 1331000 */
  tunedur=((dotdur*1080)/(tempo*60))>>1;
}

void init(int argc,char *argv[])
{
  int i;
  getopt(&argc,&argv,"h&p&k#s&:sopwith*",
         &hiresflag,        /* -h (high res flag) */
         &posindflag,       /* -p (position indicator flag) */
         &latency,          /* -k[num] (control latency) */
         &soundflag         /* -s (sound flag) */
        );
  init_timer(timerint);
  titles();
  getgamemode();
  if (gamemode==GAME_MULTIPLE) {
    if (latency==-1)
      latency=2;
    initcomms();
    clearwin();
    initlists();
    createmultiplanes();
  }
  else {
    if (latency==-1)
      latency=1;
    world=&worlds[0];
    clearwin();
    initlists();
    createplayerplane(NULL);
    for (i=0;i<3;i++)
      createenemyplane(NULL);
  }
  createbuildings();
  initscreen(FALSE);
}

void clearwin(void)
{
  int i;
  for (i=20;i<23;i++) {
    poscurs(0,i);
    clearline();
  }
  poscurs(0,20);
}

void getgamemode(void)
{
  clearwin();
  printstr("Key: S - single player\r\n");
  printstr("     M - multiple players\r\n");
  printstr("     C - single player against computer");
  while (1) {
    if (testbreak())
      finish(NULL,FALSE);
    switch (inkey()) {
      case 's':
        gamemode=GAME_SINGLE;
        return;
      case 'm':
        gamemode=GAME_MULTIPLE;
        return;
      case 'c':
        gamemode=GAME_COMPUTER;
        return;
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
    n=inkey()-'0';
    if (n>=0 && n<=7)
      return n;
  }
}

bool testbreak(void)
{
  return 0;
}

void initscreen(bool drawnground)
{
  object *p;
  if (!drawnground) {
    useoffscreenbuf();
    drawinitialground();
    drawmapground();
    startsound();
  }
  copyoffscbuftoscn();
  useonscreenbuf();
  drawmapbuildings();
  writescores();
  p=&objects[playerme];
  fuelbar(p);
  bombsbar(p);
  ammobar(p);
  livesbar(p);
  objsnotonscreen=TRUE;
}

void writescores(void)
{
  writescore(&objects[0]);
}

void livesbar(object *p)
{
  statbar(127,5-p->deaths,5,p->colour);
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
  h=(h*10)/hmax-1;
  for (y=0;y<=h;y++)
    pixel(x,y,col);
  for (;y<=9;y++)
    pixel(x,y,0);
}

void drawinitialground(void)
{
  int gx=scnleft;
  int x,h,y;
  for (x=0;x<320;x++,gx++) {
    h=ground[gx];
    for (y=16;y<=h;y++)
      pixel(x,y,131);
  }
}

void drawmapground(void)
{
  int c,h,x,sx,y;
  c=h=0;
  sx=152;
  for (x=0;x<3000;x++) {
    if (ground[x]>h)
      h=ground[x];
    c++;
    if (c==19) {
      h/=13;
      for (y=0;y<=h;y++)
        pixel(sx,y,131);
      sx++;
      h=c=0;
    }
  }
}

void drawmapbuildings(void)
{
  object *building;
  int i;
  for (i=0;i<150;i++)
    mapobjects[i].colour=0;
  for (i=0;i<20;i++) {
    building=buildings[i];
    if (building!=NULL && building->status!=STATUS_ELIMINATED)
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

void initlists(void)
{
  int i;
  object *o;
  objleft.nextx=objleft.next=&objright;
  objright.prevx=objright.prev=&objleft;
  objleft.x=-32767;
  objright.x=32767;
  lastobject=firstobject=NULL;
  nextobject=o=&objects[0];
  firstdeleted=lastdeleted=NULL;
  for (i=0;i<150;i++) {
    o->next=o+1;
    (o++)->index=i;
  }
  (o-1)->next=NULL;
}

void createenemyplane(object *pl)
{
  object *plane=createplane(pl);
  if (pl==NULL) {
    plane->drawfunc=drawenemyplane;
    plane->updatefunc=updateenemyplane;
    plane->erasefunc=eraseenemyplane;
    plane->colour=2;
    if (gamemode!=GAME_MULTIPLE)
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
    plane->drawfunc=drawplayerplane;
    plane->updatefunc=updateplayerplane;
    plane->erasefunc=eraseplayerplane;
    plane->colour=((plane->index)&1)+1;
    plane->source=plane;
    memcpy(&homes[plane->index],plane,sizeof(object));
  }
  scnleft=((plane->x-153)|3)+1;
  scnright=scnleft+319;
  pixtoscroll=0;
  clearbuf();
}

object *createplane(object *pl)
{
  int plno,runwayright,runwayleft,x,runwayaltitude;
  object *plane=(pl!=NULL ? pl : newobject());
  switch(gamemode) {
    case GAME_SINGLE:
      plno=singleplanes[plane->index];
      break;
    case GAME_MULTIPLE:
      plno=multipleplanes[plane->index];
      break;
    case GAME_COMPUTER:
      plno=computerplanes[plane->index];
      break;
  }
  plane->type=OBJ_PLANE;
  plane->status=STATUS_NORMAL;
  runwayleft=plane->x=world->planeposes[plno];
  runwayright=runwayleft+20;
  runwayaltitude=0;
  for (x=runwayleft;x<=runwayright;x++)
    if (ground[x]>runwayaltitude)
      runwayaltitude=ground[x];
  plane->y=runwayaltitude+13;
  plane->xfrac=0;
  plane->yfrac=0;
  setvel(plane,0,0);
  setaux(plane);
  plane->speed=0;
  plane->inverted=world->planeflip[plno];
  plane->rotation=(plane->inverted ? 8 : 0);
  plane->deltarot=0;
  plane->deltav=0;
  plane->firing=FALSE;
  plane->bombing=FALSE;
  plane->counter=0;
  plane->width=16;
  plane->bombtime=0;
  plane->homing=FALSE;
  if (pl==NULL) {
    plane->score=0;
    plane->ammo=200;
    plane->bombs=5;
    plane->deaths=0;
    plane->fuel=9000;
    insxlist(plane,&objleft);
  }
  else {
    delxlist(plane);
    insxlist(plane,plane->nextx);
  }
  return plane;
}

void firebullet(object *plane)
{
  int i,v,dir,x,y;
  object *bullet;
  for (i=0;i<2;i++) {
    if (plane->ammo==0)
      return;
    bullet=newobject();
    if (bullet==NULL)
      return;
    plane->ammo--;
    bullet->type=OBJ_BULLET;
    v=plane->speed+10;
    dir=plane->rotation;
    setvel(bullet,v,dir);
    bullet->x=plane->x+8;
    bullet->y=plane->y-8;
    bullet->xfrac=plane->xfrac;
    bullet->yfrac=plane->yfrac;
    if (i!=0)
      moveobject(bullet,&x,&y);
    setaux(bullet);
    bullet->fuel=10;
    bullet->source=plane;
    bullet->colour=plane->colour;
    bullet->width=1;
    bullet->drawfunc=drawbullet;
    bullet->updatefunc=updatebullet;
    bullet->erasefunc=erasebullet;
    bullet->speed=0;
    insxlist(bullet,plane);
  }
}

void dropbomb(object *plane)
{
  object *bomb;
  int rot;
  if (plane->bombs==0 || plane->bombtime!=0)
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
  bomb->xvfrac=0;
  bomb->yvfrac=0;
  setaux(bomb);
  rot=(plane->inverted ? ((plane->rotation+4)&15) : ((plane->rotation-4)&15));
  bomb->x=plane->x+(fcos(8,rot)>>8)+4;
  bomb->y=plane->y+(fsin(8,rot)>>8)-4;
  bomb->xfrac=0;
  bomb->yfrac=0;
  bomb->fuel=5;
  bomb->source=plane;
  bomb->colour=plane->colour;
  bomb->width=8;
  bomb->drawfunc=drawbomb;
  bomb->updatefunc=updatebomb;
  bomb->erasefunc=erasebomb;
  insxlist(bomb,plane);
}

void createbuildings(void)
{
  int i,left,right,minaltitude,maxaltitude,gx,y;
  int *positions=world->buildingpositions;
  int *types=world->buildingtypes;
  object *building;
  buildingstodestroy[0]=0;
  buildingstodestroy[1]=17;
  for (i=0;i<20;i++) {
    building=newobject();
    buildings[i]=building;
    building->x=left=*positions;
    right=left+15;
    minaltitude=999;
    maxaltitude=0;
    for (gx=left;gx<=right;gx++) {
      if (ground[gx]>maxaltitude)
        maxaltitude=ground[gx];
      if (ground[gx]<minaltitude)
        minaltitude=ground[gx];
    }
    for (y=(maxaltitude+minaltitude)>>1;(building->y=y+16)>=200;y--);
    for (gx=left;gx<=right;gx++)
      ground[gx]=y;
    building->xv=0;
    building->yv=0;
    building->xfrac=0;
    building->yfrac=0;
    building->xvfrac=0;
    building->yvfrac=0;
    setaux(building);
    building->type=OBJ_BUILDING;
    building->status=STATUS_INTACT;
    building->kind=*types;
    building->rotation=0;
    building->fuel=i;
    building->source=&objects[i>=10 || i<=6 ? 1 : 0];
    building->colour=building->source->colour;
    building->width=16;
    building->drawfunc=drawbuilding;
    building->updatefunc=updatebuilding;
    building->erasefunc=erasebuilding;
    insxlist(building,&objleft);
    positions++;
    types++;
  }
}

void createexplosion(object *obj)
{
  int l,step,i;
  long randno;
  object *shrapnel;
  if (obj->drawfunc==drawbuilding && obj->kind==BUILDING_FUEL) {
    step=1;
    l=10;
  }
  else {
    step=2;
    l=5;
  }
  for (i=1;i<=15;i+=step) {
    shrapnel=newobject();
    if (shrapnel==0)
      return;
    shrapnel->type=OBJ_SHRAPNEL;
    setvel(shrapnel,8,i);
    shrapnel->x=shrapnel->xv+obj->x+obj->width/2;
    shrapnel->y=shrapnel->yv+obj->y-obj->width/2;
    shrapnel->xfrac=0;
    shrapnel->yfrac=0;
    setaux(shrapnel);
    randno=(long)shrapnel->x*(long)shrapnel->y*0xbe8b71f5l;
    shrapnel->fuel=(int)(((((unsigned long)((unsigned short)(randno*((unsigned long)i))))*((unsigned long)l))>>16)+1);
    shrapnel->kind=(int)((((unsigned long)((unsigned short)(randno*((unsigned long)i))))*8UL)>>16);
    shrapnel->counter=0;
    shrapnel->source=obj;
    shrapnel->colour=obj->colour;
    shrapnel->width=8;
    shrapnel->drawfunc=drawshrapnel;
    shrapnel->updatefunc=updateshrapnel;
    shrapnel->erasefunc=eraseshrapnel;
    shrapnel->speed=0;
    objectsound(shrapnel,SOUND_SHRAPNEL);
    insxlist(shrapnel,obj);
  }
}

void createsmoke(object *plane)
{
  object *smoke=newobject();
  if (smoke==0)
    return;
  smoke->type=OBJ_SMOKE;
  smoke->x=plane->x+8;
  smoke->y=plane->y-8;
  smoke->xfrac=0;
  smoke->yfrac=0;
  smoke->xv=plane->xv;
  smoke->yv=plane->yv;
  smoke->xvfrac=0;
  smoke->yvfrac=0;
  smoke->fuel=10;
  smoke->source=plane;
  smoke->width=1;
  smoke->drawfunc=drawsmoke;
  smoke->updatefunc=updatesmoke;
  smoke->erasefunc=erasesmoke;
  smoke->colour=plane->colour;
}

void setaux(object *obj)
{
  int i;
  for (i=0;i<3;i++) {
    obj->xaux[i]=obj->x;
    obj->yaux[i]=obj->y;
  }
}

void createmultiplanes(void)
{
}

int getmaxplayers(void)
{
  int n;
  clearwin();
  printstr(" Key maximum number of players allowed");
  while (1) {
    if (testbreak())
      finish(NULL,FALSE);
    n=inkey()-'0';
    if (n>=1 && n<=4)
      return n;
  }
}

int inkey(void)
{
  if (kbhit())
    return getch();
  return 0;
}

int clearbuf(void)
{
  while (kbhit())
    getch();
  return 0;
}

void setcolour(int c)
{
  writecharcol=c;
}

void checkcollisions(void)
{
  int i,left,bottom,top,right;
  object **obj1,**obj2;
  object *test,*obj;
  int w;
  collc=0;
  useoffscreenbuf();
  for (obj=objleft.nextx;obj!=&objright;obj=obj->nextx) {
    left=obj->x;
    w=obj->width-1;
    right=left+w;
    bottom=obj->y;
    top=bottom-w;
    for (test=obj->nextx;test->x<=right && test!=&objright;test=test->nextx) {
      left=test->x;
      if (test->y>top && test->y-test->width+1<=bottom)
        checkcollision(obj,test);
    }
    if (obj!=NULL)
      if ((obj->type==OBJ_PLANE && obj->status!=STATUS_ELIMINATED &&
           obj->status!=STATUS_UNKNOWN && bottom<ground[left+8]+24) ||
          (obj->type==OBJ_BOMB && bottom<ground[left+4]+12))
        checkcrash(obj);
  }
  for (i=0,obj1=collobj1,obj2=collobj2;i<collc;i++,obj1++,obj2++)
    docollision(*obj1,*obj2);
}

void checkcollision(object *obj1,object *obj2)
{
  int type1=(obj1!=NULL ? obj1->type : 0);
  int type2=(obj2!=NULL ? obj2->type : 0);
  object *t;
  if ((type1==OBJ_PLANE && obj1->status==STATUS_ELIMINATED) ||
      (type2==OBJ_PLANE && obj2->status==STATUS_ELIMINATED) ||
      (type1==OBJ_SHRAPNEL && type2==OBJ_SHRAPNEL))
    return;
  if (obj1->y<obj2->y) {
    t=obj1;
    obj1=obj2;
    obj2=t;
  }
  draw(15,15,obj1);
  if (draw(15+obj2->x-obj1->x,15+obj2->y-obj1->y,obj2) && collc<149) {
    collobj1[collc]=obj1;
    collobj2[collc++]=obj2;
    collobj1[collc]=obj2;
    collobj2[collc++]=obj1;
  }
  draw(15+obj2->x-obj1->x,15+obj2->y-obj1->y,obj2);
  draw(15,15,obj1);
}

void checkcrash(object *obj)
{
  int right,left,h;
  bool f=FALSE;
  draw(15,15,obj);
  right=obj->x+obj->width-1;
  for (left=obj->x;left<=right;left++) {
    h=ground[left]+15-obj->y;
    if (h>15) {
      f=TRUE;
      break;
    }
    if (h>=0) {
      f=pixel(left+15-obj->x,h,128);
      if (f)
        break;
    }
  }
  draw(15,15,obj);
  if (f && collc<150) {
    collobj1[collc]=obj;
    collobj2[collc++]=NULL;
  }
}

void docollision(object *obj1,object *obj2)
{
  if (obj1==NULL)
    return;
  switch (obj1->type) {
    case OBJ_BOMB:
      createexplosion(obj1);
      obj1->fuel=-1;
      if ((obj2!=NULL ? obj2->type : 0)==0)
        digcrater(obj1,1);
      killsnd(obj1);
      return;
    case OBJ_BULLET:
      obj1->fuel=1;
      return;
    case OBJ_SHRAPNEL:
      obj1->fuel=1;
      killsnd(obj1);
      return;
    case OBJ_BUILDING:
      if (obj1->status!=STATUS_INTACT)
        return;
      if ((obj2!=NULL ? obj2->type : 0)==OBJ_SHRAPNEL)
        return;
      obj1->status=STATUS_ELIMINATED;
      createexplosion(obj1);
      useonscreenbuf();
      drawmapobject(obj1);
      useoffscreenbuf();
      score(obj1,obj1->kind==BUILDING_FUEL ? 200 : 100);
      if ((--buildingstodestroy[obj1->colour-1])==0)
        declarewinner(obj1->colour);
      return;
    case OBJ_PLANE:
      if (obj1->status==STATUS_DEAD)
        return;
      if ((obj2!=NULL ? obj2->type : 0)==0) {
        createexplosion(obj1);
        if (obj1->status!=STATUS_FALLING) {
          score50(obj1);
          killsnd(obj1);
        }
        killplane(obj1);
        return;
      }
      if (obj1->status==STATUS_FALLING)
        return;
      if ((obj2!=NULL ? obj2->type : OBJ_NONE)!=OBJ_BULLET)
        createexplosion(obj1);
      catchfire(obj1);
      score50(obj1);
      return;
  }
}

void score(object *destroyed,int score)
{
  if (destroyed->colour==1)
    objects[0].score-=score;
  else
    objects[0].score+=score;
  writescore(&objects[0]);
}

void score50(object *destroyed)
{
  score(destroyed,50);
}

void writescore(object *p)
{
  poscurs(1+(p->colour-1)*7,24);
  setcolour(p->colour);
  writenum(p->score,6);
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

void digcrater(object *obj,int depth)
{
  int left,right,scleft,scright,sx,x,osleft,osright,osx,ny,y,oy,cx;
  depth=1;
  useonscreenbuf();
  left=obj->x+(obj->width-8)/2;
  right=obj->width+left-1;
  scleft=scnleft-pixtoscroll;
  scright=scnright-pixtoscroll;
  sx=left-scleft;
  osleft=((homes[playerme].x-153)|3)+1;
  osright=osleft+319;
  osx=left-osleft;
  for (x=left,cx=0;x<=right;x++,cx++,sx++,osx++) {
    oy=ground[x];
    ny=(oy-crater[cx]*depth)+1;
    if (ny<=loground[x])
      ny=loground[x]+1;
    ground[x]=ny-1;
    if (x>=scleft && x<=scright)
      for (y=oy;y>=ny;y--)
        pixel(sx,y,131);
    if (x>=osleft && x<=osright) {
      useoffscreenbuf();
      for (y=oy;y>=ny;y--)
        pixel(osx,y,131);
      useonscreenbuf();
    }
  }
  useoffscreenbuf();
}

void moveobject(object *obj,int *x,int *y)
{
  long pos,vel;
  int i;
  pos=(((long)(obj->x))<<16)+obj->xfrac;
  vel=(((long)(obj->xv))<<16)+obj->xvfrac;
  pos+=vel;
  obj->x=(short)(pos>>16);
  obj->xfrac=(short)pos;
  *x=obj->x;
  for (i=0;i<3;i++) {
    pos+=(vel<<1);
    obj->xaux[i]=(short)(pos>>16);
  }
  pos=(((long)(obj->y))<<16)+obj->yfrac;
  vel=(((long)(obj->yv))<<16)+obj->yvfrac;
  pos+=vel;
  obj->y=(short)(pos>>16);
  obj->yfrac=(short)pos;
  *y=obj->y;
  for (i=0;i<3;i++) {
    pos+=(vel<<1);
    obj->yaux[i]=(short)(pos>>16);
  }
}

/* Let me tell you a story. I had fixed all the bugs I found after decompiling,
except for one - the autopilot wasn't working - it decided that the best way to
get anywhere was to go straight up. I decompiled all the autopilot routines
again, checking everything I could possibly think of to check. Everything
seemed fine except for one minor detail - it didn't work. Since I didn't know
how the autopilot worked, I had no idea what execution path it should be
taking. So finally I decided the only way to find the bug was to compare it to
the original. By hand, I crafted and assembled a piece of code to automatically
send the plane home after a short journey, and patched into the executable with
a hex editor. I ran the C debugger in one DOS session and D86 in another, and
stepped through the programs in synchronization. Finally I found that this
routine was returning 0 in *x and *y when it shouldn't have been - I had
forgotten to convert to long before shifting left by 16. That bug sets a
personal record for how long it took me to find - 3 days. */

void moveaux(object *obj,int *x,int *y,int i)
{
  long pos,vel;
  pos=(((long)(obj->xaux[i]))<<16)|0x8000L;
  vel=(((long)(obj->xv))<<16)|((long)(obj->xvfrac));
  (*x)=obj->xaux[i]=(short)((pos+vel)>>16);
  pos=(((long)(obj->yaux[i]))<<16)|0x8000L;
  vel=(((long)(obj->yv))<<16)|((long)(obj->yvfrac));
  (*y)=obj->yaux[i]=(short)((pos+vel)>>16);
}

void setvel(object *obj,int v,int dir)
{
  int xv=fcos(v,dir),yv=fsin(v,dir);
  obj->xv=xv>>8;
  obj->xvfrac=xv<<8;
  obj->yv=yv>>8;
  obj->yvfrac=yv<<8;
}

void updateground(void)
{
  int xl,xr,x,y,xs,x0,x1,xs0,xs1;
  if (pixtoscroll<0) {
    xl=scnleft;
    xr=xl-(pixtoscroll+1);
    xs=0;
  }
  else {
    xr=scnright;
    xl=xr+1-pixtoscroll;
    xs=320-pixtoscroll;
  }
  for (x=xl;x<=xr;x++,xs++) {
    x1=ground[x];
    x0=ground[x-pixtoscroll];
    if (x1>x0)
      for (y=x1;y>x0;y--)
        pixel(xs,y,131);
    else
      for (y=x1+1;y<=x0;y++)
        pixel(xs,y,131);
    if (finaleflying) {
      xs0=suny(x);
      xs1=suny(x-pixtoscroll);
      for (y=x0+1;y<=xs1;y++)
        pixel(xs,y,130);
      for (y=x1+1;y<=xs0;y++)
        pixel(xs,y,130);
    }
  }
}

int suny(int x)
{
  if (x<finalesx-50 || x>finalesx+50)
    return ground[x];
  return finalesqrt[x+50-finalesx];
}

unsigned char draw(int x,int y,object *p)
{
  if (x>=0 && x<320 && y>=0 && y<200)
    return bitblt(y,x,p->sprite,p->width,p->width);
  return 0;
}

void erase(int x,int y,object *p)
{
  if (x>=0 && x<320 && y>=0 && y<200)
    bitblt(y,x,p->oldsprite,p->width,p->width);
}

void drawplayerplane(object *plane,int x,int y)
{
  if (plane->status!=STATUS_ELIMINATED) {
    draw(x,y,plane);
    planesound(plane);
  }
}

void drawbullet(object *bullet,int x,int y)
{
  draw(x,y,bullet);
}

void drawbomb(object *bomb,int x,int y)
{
  draw(x,y,bomb);
  if (bomb->yv<=0)
    initsound(SOUND_BOMB,-bomb->y,bomb);
}

void drawbuilding(object *building,int x,int y)
{
  draw(x,y,building);
}

void drawshrapnel(object *shrapnel,int x,int y)
{
  draw(x,y,shrapnel);
  initsound(SOUND_SHRAPNEL,shrapnel->counter,shrapnel);
}

void drawsmoke(object *smoke,int x,int y)
{
  draw(x,y,smoke);
}

void drawmapobject(object *obj)
{
  mapobject *mapobj=&mapobjects[obj->index];
  int c;
  if (mapobj->colour!=0)
    pixel(mapobj->x,mapobj->y,mapobj->colour-1);
  if (obj->status==STATUS_ELIMINATED)
    mapobj->colour=0;
  else {
    c=pixel(mapobj->x=(obj->x+obj->width/2)/19+152,
            mapobj->y=(obj->y-obj->width/2)/13,obj->source->colour);
    if (c==0 || c==3) {
      mapobj->colour=c+1;
      return;
    }
    pixel(mapobj->x,mapobj->y,c);
    mapobj->colour=0;
  }
}

void drawenemyplane(object *plane,int x,int y)
{
  if (plane->status!=STATUS_ELIMINATED) {
    draw(x,y,plane);
    planesound(plane);
  }
}

void drawremoteplane(object *plane,int x,int y)
{
  if (plane->status!=STATUS_ELIMINATED) {
    draw(x,y,plane);
    planesound(plane);
  }
}

void planesound(object *plane)
{
  if (plane->firing)
    initsound(SOUND_FIRING,0,plane);
  else
    switch(plane->status) {
      case STATUS_FALLING:
        if (plane->yv>=0)
          initsound(SOUND_STUTTER,0,plane);
        else
          initsound(SOUND_FALLING,plane->y,plane);
        break;
      case STATUS_NORMAL:
        initsound(SOUND_ENGINE,-plane->speed,plane);
        break;
      case STATUS_SPINNING:
        initsound(SOUND_STUTTER,0,plane);
    }
}

void erasebullet(object *bullet,int x,int y)
{
  erase(x,y,bullet);
}

void erasebomb(object *bomb,int x,int y)
{
  erase(x,y,bomb);
}

void erasebuilding(object *building,int x,int y)
{
  erase(x,y,building);
}

void eraseshrapnel(object *shrapnel,int x,int y)
{
  erase(x,y,shrapnel);
}

void erasesmoke(object *smoke,int x,int y)
{
  erase(x,y,smoke);
}

void eraseplayerplane(object *plane,int x,int y)
{
  erase(x,y,plane);
}

void eraseenemyplane(object *plane,int x,int y)
{
  if (plane->oldsprite!=NULL)
    erase(x,y,plane);
}

void eraseremoteplane(object *plane,int x,int y)
{
  if (plane->oldsprite!=NULL)
    erase(x,y,plane);
}

void finish(char *msg,bool closef)
{
  char *errmsg=NULL;
  setgmode(3);
  initsound(SOUND_OFF,0,NULL);
  updatesound();
  if (gamemode==GAME_MULTIPLE)
    errmsg=finishcomms(closef);
  restoreints();
  printstr("\r\n");
  if (errmsg!=NULL) {
    printstr(errmsg);
    printstr("\r\n");
  }
  if (msg!=NULL) {
    printstr(msg);
    printstr("\r\n");
  }
  clearbuf();
  if (msg!=NULL || errmsg!=NULL)
    exit(1);
  exit(0);
}

void declarewinner(int n)
{
  object *player;
  for (player=firstobject;player->type==OBJ_PLANE;player=player->next)
    if (finaleflags[player->index]==0)
      if (player->colour==1)
        createsun(player);
      else
        gameover(player);
}

void createsun(object *player)
{
  int index,x,gy,x0,y,yy;
  finaleflags[index=player->index]=1;
  if (index==playerme) {
    if (player->x>1500)
      finalesx=scnleft-60;
    else
      finalesx=scnright+60;
    finalesy=ground[finalesx-50];
    for (x=finalesx-49;finalesx+50>=x;x++) {
      gy=ground[x];
      if (gy<finalesy)
        finalesy=gy;
    }
    for (x=0;x<=100;x++) {
      x0=x-50;
      yy=50*50-x0*x0;
      for (y=0;y*y<yy;y++); /* 1 line sqrt */
      finalesqrt[x]=finalesy+y;
    }
    finaletime=200;
    finalex=finalesx-8;
    finaley=ground[finalex]+30;
    finaleflying=TRUE;
  }
}

void gameover(object *plane)
{
  int player=plane->index;
  finaleflags[player]=2;
  if (player==playerme) {
    setcolour(130);
    poscurs(16,12);
    printstr("THE END");
    finaletime=20;
  }
}

int pixel(int x,int y,int c)
{
  int prev,bit;
  unsigned char far *scp;
  bit=(3-x&3)<<1;
  scp=(unsigned char far *)MK_FP(scrseg,scroff+((((199-y)>>1)*320+x)>>2)+
                   (((~y)&1)==1 ? interlacediff : 0));
  prev=(*scp)&(3<<bit);
  if ((c&0x80)==0) {
    *scp^=prev;
    *scp|=c<<bit;
    return (prev<<bit);
  }
  *scp^=(c&0x7f)<<bit;
  return (prev<<bit);
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
  newobj->onscreen=FALSE;
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
  if (lastdeleted!=NULL)
    lastdeleted->next=obj;
  else
    firstdeleted=obj;
  lastdeleted=obj;
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
  poscurs(13,9);
  printstr("S O P W I T H");
  poscurs(12,12);
  setcolour(1);
  printstr("BMB ");
  setcolour(3);
  printstr("Compuscience");
}

void startsound(void)
{
  if (notitlesflag)
    return;
  initsound(SOUND_OFF,0,NULL);
  updatesound();
}

void timerint(void)
{
  timertick++;
  updatesoundint();
}

void enemylogic(object *plane)
{
  object *attackee=attackees[plane->index];
  autoisplayer=FALSE;
  if (attackee!=NULL)
    enemyattack(plane,attackee);
  else
    flyhome(plane);
  attackees[plane->index]=NULL;
}

void enemyattack(object *plane,object *attackee)
{
  if (plane->fuel<=abs(plane->x-homes[plane->index].x)+500 || plane->ammo<=0) {
    flyhome(plane);
    return;
  }
  if (attackee->type==OBJ_PLANE && attackee->speed!=0)
    autopilot(plane,attackee->x-(fcos(32,attackee->rotation)>>8),
              attackee->y-(fsin(32,attackee->rotation)>>8),attackee);
  else
    autopilot(plane,attackee->x,attackee->y,attackee);
}

bool ontarget(object *plane,object *target)
{
  object tbullet,ttarget;
  int bulletx,bullety,targetx,targety,d2,i;
  memcpy(&tbullet,plane,sizeof(object));
  memcpy(&ttarget,target,sizeof(object));
  setvel(&tbullet,tbullet.speed+10,tbullet.rotation);
  tbullet.x+=8;
  tbullet.y-=8;
  for (i=0;i<10;i++) {
    moveobject(&tbullet,&bulletx,&bullety);
    moveobject(&ttarget,&targetx,&targety);
    d2=distsqr(bulletx,bullety,targetx,targety);
    if (d2<0 || d2>75*75)
      return FALSE;
    if (bulletx>=targetx && targetx+15>=bulletx && bullety<=targety &&
        targety-15<=bullety)
      return TRUE;
  }
  return FALSE;
}

int flyhome(object *plane)
{
  object *home;
  if (ishome(plane))
    return 0;
  home=&homes[plane->index];
  if (abs(plane->x-home->x)<16 && abs(plane->y-home->y)<16) {
    if (plane->drawfunc==drawplayerplane) {
      createplayerplane(plane);
      initscreen(TRUE);
    }
    else
      if (plane->drawfunc==drawenemyplane)
        createenemyplane(plane);
      else
        createplane(plane);
    return 0;
  }
  autoisplayer=TRUE;
  return autopilot(plane,home->x,home->y,NULL);
}

int flytofinale(object *plane)
{
  if (isatfinalepos(plane))
    return 0;
  if (abs(plane->x-finalex)<16 && abs(plane->y-finaley)<16) {
    createplane(plane);
    plane->x=finalex;
    plane->y=finaley;
    finaletime=72;
    setcolour(130);
    poscurs(16,12);
    printstr("THE END");
    return 0;
  }
  autoisplayer=TRUE;
  return autopilot(plane,finalex,finaley,NULL);
}

int autopilot(object *plane,int destx,int desty,object *destobj)
{
  autoinf *choice;
  object tplane;
  object *ptplane;
  int px,py,distx,disty,i,ch,ch2,ch3;
  int newrot,newspd,maxspd,d2,mindist2;
  bool fire=FALSE;
  dontcheckcoll=FALSE;
  maxspd=finaleflying ? 4 : 8;
  if (plane->status==STATUS_SPINNING && plane->rotation!=12) {
    plane->deltarot=-1;
    return 0x8001;
  }
  if (ishome(plane) && (plane->fuel<6000 || plane->ammo<133 || plane->bombs<3))
    return 0x8001;
  px=plane->x;
  py=plane->y;
  distx=px-destx;
  if (abs(distx)>200) {
    py+=100;
    return autopilot(plane,px+(distx>=0 ? -150 : 150),
                     (py<=180 ? py : 180),destobj);
  }
  if (plane->speed!=0) {
    disty=py-desty;
    if (disty!=0 && abs(disty)<6)
      plane->y=((disty<0) ? ++py : --py);
    else
      if (distx && abs(distx)<6)
        plane->x=((distx<0) ? ++px : --px);
  }
  memcpy(ptplane=&tplane,plane,sizeof(object));
  for (i=0,choice=choices;i<5;i++,choice++) {
    newrot=(ptplane->rotation+(ptplane->inverted ? -choice->deltarot :
                                                   choice->deltarot))&15;
    newspd=ptplane->speed+choice->deltav;
    if (newspd>maxspd)
      newspd=maxspd;
    else
      if (newspd<4)
        newspd=4;
    setvel(ptplane,newspd,newrot);
    distx=choice->xv=ptplane->xv;
    disty=choice->yv=ptplane->yv;
    choice->xvfrac=ptplane->xvfrac;
    choice->yvfrac=ptplane->yvfrac;
    choice->newspd=newspd;
    choice->alt=disty+py-loground[distx+px+8];
    choice->d2=distsqr(px+distx,py+disty,destx,desty);
  }
  setaux(ptplane);
  mkobjlist(plane);
  d2=choices[0].d2;
  if (destobj!=NULL && d2>=0 && d2<=75*75 && ontarget(ptplane,destobj)) {
    ch=0;
    fire=TRUE;
  }
  else {
    mindist2=32767;
    for (i=0,choice=choices;i<5;i++,choice++) {
      d2=choice->d2;
      if (d2>=0 && d2<mindist2) {
        mindist2=d2;
        ch=i;
      }
    }
    if (mindist2==32767) {
      mindist2=-32767;
      for (i=0,choice=choices;i<5;i++,choice++) {
        d2=choice->d2;
        if (d2<0 && d2>mindist2) {
          mindist2=d2;
          ch=i;
        }
      }
    }
  }
  if (ailogic(ptplane,ch,ch,1)) {
    ch2=ch;
    fire=FALSE;
    dontcheckcoll=TRUE;
    disty=-32767;
    for (i=0,choice=choices;i<5;i++,choice++)
      if (i!=ch2 && choice->alt>disty) {
        disty=choice->alt;
        ch=i;
      }
    if (ailogic(ptplane,ch,ch,1)) {
      ch3=ch;
      disty=-32767;
      for (i=0,choice=choices;i<5;i++,choice++)
        if (i!=ch2 && i!=ch3 && (choice->xv!=0 || choice->yv>0) &&
            choice->alt>disty) {
          disty=choice->alt;
          ch=i;
        }
      if (disty==-32767 || ailogic(ptplane,ch,ch,1)) {
        dontcheckcoll=FALSE;
        disty=32767;
        if (py>107) {
          for (i=0,choice=choices;i<5;i++,choice++)
            if (i!=ch2 && choice->alt<disty) {
              disty=choice->alt;
              ch=i;
            }
          if (ailogic(ptplane,ch,ch,1))
            ch=ch2;
        }
        else
          ch=ch2;
      }
    }
  }
  plane->deltarot=choices[ch].deltarot;
  plane->deltav=choices[ch].deltav;
  if (fire)
    plane->firing=TRUE;
  if (plane->deltarot==0 && plane->deltav==0)
    if (ishome(plane))
      plane->deltav=-1;
    else
      if (plane->speed!=0)
        plane->inverted=(plane->xv<0);
  return choices[ch].d2;
}

bool ailogic(object *plane,int ch1,int ch2,int rdepth)
{
  autoinf *choice2=&choices[ch2];
  autoinf *choice1=&choices[ch1];
  int ch,newspd,newrot,xa1,ya1,xa0,ya0;
  bool f;

  xa0=plane->xaux[0];
  ya0=plane->yaux[0];
  newspd=choice1->newspd;
  newrot=(plane->rotation+(plane->inverted ? -choice2->deltarot :
                                              choice2->deltarot)*rdepth)&15;
  setvel(plane,newspd,newrot);
  moveaux(plane,&xa1,&ya1,0);
  moveaux(plane,&xa1,&ya1,0);
  if ((!dontcheckcoll && ground[xa1+8]+24>ya1 &&
       (!autoisplayer || checkcollide(plane,xa1,ya1,newrot))) || ya1>=200)
    f=TRUE;
  else
    if (hitobj(plane,rdepth,xa1,ya1)!=NULL)
      f=TRUE;
    else
      if (rdepth>=3)
        f=FALSE;
      else
        if (!ailogic(plane,ch1,ch2,++rdepth))
          f=FALSE;
        else {
          f=TRUE;
          for (ch=0;ch<3;ch++)
            if (ch!=ch2 && !ailogic(plane,ch1,ch,rdepth)) {
              f=FALSE;
              break;
            }
        }
  plane->xaux[0]=xa0;
  plane->yaux[0]=ya0;
  return f;
}

bool checkcollide(object *plane,int xleft,int ybottom,int rot)
{
  int bp_4,xright,x;
  bool collided=FALSE;
  useoffscreenbuf();
  bitblt(15,15,planesprites[0][plane->inverted ? 1 : 0][rot],16,16);
  xright=xleft+plane->width-1;
  for (x=xleft;x<=xright;x++) {
    bp_4=(ground[x]-ybottom)+15;
    if (bp_4>15) {
      collided=TRUE;
      break;
    }
    if (bp_4>=0) {
      collided=pixel((x-xleft)+15,bp_4,128);
      if (collided)
        break;
    }
  }
  bitblt(15,15,planesprites[0][plane->inverted ? 1 : 0][rot],16,16);
  return collided;
}

void mkobjlist(object *plane)
{
  object *obj;
  int sleft,sright,left,right,bottom,top,oleft,oright,obottom,otop,type;
  sleft=plane->x-160;
  sright=sleft+320;
  left=plane->x-48;
  right=plane->x+63;
  top=plane->y+48;
  bottom=plane->y-63;
  objcnt=-1;
  for (obj=plane->prevx;obj!=&objleft;obj=obj->prevx)
    if (obj->x<sleft)
      break;
  for (obj=obj->nextx;obj!=&objright;obj=obj->nextx) {
    if (obj->x>sright || objcnt>=49)
      break;
    if (obj!=plane) {
      type=(obj!=NULL) ? obj->si62 : 0;
      if ((type!=OBJ_PLANE || obj->status!=STATUS_ELIMINATED) &&
          (type!=OBJ_BULLET || obj->source!=plane)) {
        if (obj->xv>0) {
          oleft=obj->x;
          oright=obj->xaux[2]+obj->width-1;
        }
        else {
          oleft=obj->xaux[2];
          oright=obj->x+obj->width-1;
        }
        if (obj->yv>0) {
          obottom=(obj->y-obj->width)+1;
          otop=obj->yaux[2];
        }
        else {
          obottom=(obj->yaux[2]-obj->width)+1;
          otop=obj->y;
        }
        if (oleft<=right && otop>=bottom && oright>=left && obottom<=top)
          objlist[++objcnt]=obj;
      }
    }
  }
}

object *hitobj(object *plane,int rdepth,int xleft,int ybottom)
{
  object *obj;
  int left,right,top,bottom,x,y,objno;
  left=xleft;
  right=xleft+15;
  top=ybottom-15;
  bottom=ybottom;
  for (objno=0;objno<=objcnt;objno++) {
    obj=objlist[objno];
    x=obj->xaux[rdepth-1];
    y=obj->yaux[rdepth-1];
    if (x<=right && y>=top && x+obj->width-1>=left && y+1-obj->width<=bottom)
      return obj;
  }
  return NULL;
}

int distsqr(int x0,int y0,int x1,int y1)
{
  int x=abs(x0-x1),y=abs(y0-y1),t;
  if (x<100 && y<100)
    return x*x+y*y;
  if (x<y) { t=x; x=y; y=t; }
  return -((x*7+(y<<2))>>3);
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

unsigned char bitblt(int y,int x,unsigned char *p,int height,int width)
{
  unsigned int linediff,outpos,startpos,yleft,bltreg;
  unsigned char retval=0,shift,bltleft;
  unsigned char far *scp;
  int border;
  if (height==1 && width==1)
    return pixel(x,y,(int)p);
  scp=(unsigned char far *)MK_FP(scrseg,scroff);
  shift=8-((x&3)<<1);
  y=199-y;
  width>>=2;
  border=width-(80-(x>>2));
  if (border>0)
    width=80-(x>>2);
  yleft=(y+height>200 ? 200-y : height);
  linediff=interlacediff;
  startpos=((y>>1)*320+x)>>2;
  if (y&1) {
    startpos+=linediff;
    linediff=80-linediff;
  }
  for (y=0;y<yleft;y++) {
    outpos=startpos;
    bltleft=0;
    for (x=0;x<width;x++) {
      bltreg=((*(p++))<<shift)|(bltleft<<8);
      retval|=scp[outpos]&blut[bltreg>>8];
      scp[outpos++]^=(bltreg>>8);
      bltleft=bltreg;
    }
    if (border>=0)
      p+=border;
    else
      if (bltleft) {
        retval|=scp[outpos]&blut[bltleft];
        scp[outpos]^=bltleft;
      }
    startpos+=linediff;
    linediff=80-linediff;
  }
  return retval;
}

void initcomms(void)
{
  int networld=0;
  world=&worlds[networld];
}

char *finishcomms(bool closef)
{
  return NULL;
}

char getremotekey(object *player)
{
  int i,status=STATUS_UNKNOWN,key=0;
  if (finishflag)
    finish(NULL,TRUE);
  i=player->index;
  if (i!=playerme) {
    timertick=0;
    if (player->status!=status &&
        (status==STATUS_ELIMINATED || status==STATUS_UNKNOWN ||
         player->status==STATUS_ELIMINATED || player->status==STATUS_UNKNOWN)) {
      player->status=status;
      useonscreenbuf();
      drawmapobject(player);
    }
  }
  return key;
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
        if (isalpha(p[i])==0 || q[i]!='#')
          break;
      q+=i;
      if (*q)
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
    do {
      if (*arg==0 && *p!='&') {
        varg+=2;
        arg=*varg;
        carg--;
      }
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
      if (*p=='&')
        if (*arg==0)
          break;
    } while (1);
    varg+=2;
  }
  *argv=varg;
  *argc=carg;
}

