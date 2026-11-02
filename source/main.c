#include <nds.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* ELDORIA V0.1 - original, procedural pixel-art prototype.
   Target: Nintendo DS/DSi, libnds, 256x192 per screen. */
#define SW 256
#define SH 192
#define TILE 16
#define MAP_W 48
#define MAP_H 48
#define MAX_ENEMIES 6
#define PLAYER_SPEED 1

typedef struct { int x,y,hp,alive,dir,hitTimer; } Actor;
static uint16_t *topPixels, *bottomPixels;
static Actor player = { 24*TILE, 24*TILE, 20, 1, 0, 0 };
static Actor enemies[MAX_ENEMIES] = {
 {27*TILE,22*TILE,3,1,0,0}, {31*TILE,25*TILE,3,1,0,0},
 {20*TILE,28*TILE,3,1,0,0}, {35*TILE,30*TILE,3,1,0,0},
 {16*TILE,20*TILE,3,1,0,0}, {39*TILE,18*TILE,3,1,0,0}
};
static int worldMap[MAP_H][MAP_W];
static int level=1, expPoints=0, gold=0, kills=0, attackCooldown=0;
static bool showHelp=true;
static int camX,camY;

#define RGB(r,g,b) ((uint16_t)(((r)&31)|(((g)&31)<<5)|(((b)&31)<<10)))
#define C_GRASS RGB(10,23,8)
#define C_GRASS2 RGB(12,26,9)
#define C_PATH RGB(23,18,10)
#define C_PATH2 RGB(26,21,12)
#define C_WATER RGB(5,14,25)
#define C_TREE RGB(4,15,5)
#define C_TREE2 RGB(7,21,7)
#define C_SKIN RGB(29,20,14)
#define C_RED RGB(29,5,5)
#define C_WHITE RGB(31,31,31)
#define C_BLACK RGB(0,0,0)
#define C_GOLD RGB(31,24,3)

static inline void px(uint16_t *fb,int x,int y,uint16_t c){ if((unsigned)x<SW && (unsigned)y<SH) fb[y*SW+x]=c; }
static void rect(uint16_t *fb,int x,int y,int w,int h,uint16_t c){
 for(int yy=0;yy<h;yy++) for(int xx=0;xx<w;xx++) px(fb,x+xx,y+yy,c);
}
static void border(uint16_t *fb,int x,int y,int w,int h,uint16_t c){
 rect(fb,x,y,w,1,c);rect(fb,x,y+h-1,w,1,c);rect(fb,x,y,1,h,c);rect(fb,x+w-1,y,1,h,c);
}
static void initMap(void){
 for(int y=0;y<MAP_H;y++) for(int x=0;x<MAP_W;x++){
  int v=0;
  if(x<2||y<2||x>MAP_W-3||y>MAP_H-3) v=3;
  if((x==8||x==9||x==38||x==39) && y>5 && y<42) v=3;
  if((y==8||y==9||y==38||y==39) && x>5 && x<42) v=3;
  if((x>19&&x<29&&y>17&&y<29)) v=1;
  if((x>5&&x<42&&y>22&&y<25) || (y>8&&y<39&&x>22&&x<25)) v=2;
  if((x==14||x==33) && (y==15||y==32)) v=3;
  worldMap[y][x]=v;
 }
}
static bool blocked(int wx,int wy){
 int tx=wx/TILE,ty=wy/TILE;
 if(tx<0||ty<0||tx>=MAP_W||ty>=MAP_H) return true;
 return worldMap[ty][tx]==3 || worldMap[ty][tx]==1;
}
static void drawTile(uint16_t *fb,int sx,int sy,int type,int tx,int ty){
 uint16_t base= type==2?C_PATH:(type==3?C_TREE:C_GRASS);
 rect(fb,sx,sy,TILE,TILE,base);
 if(type==0){
  px(fb,sx+3+(tx*7+ty*3)%9,sy+4+(tx+ty)%8,C_GRASS2);
  px(fb,sx+11,sy+12,C_GRASS2);
 } else if(type==2){
  px(fb,sx+4,sy+5,C_PATH2);px(fb,sx+12,sy+11,C_PATH2);
 } else if(type==3){
  rect(fb,sx+2,sy+2,12,11,C_TREE2);rect(fb,sx+4,sy+4,8,7,C_TREE);
  rect(fb,sx+7,sy+12,3,4,RGB(16,10,5));
 }
}
static void drawActor(uint16_t *fb,int sx,int sy,bool goblin,int facing){
 // small original 12x14 character sprite, shadow included
 rect(fb,sx+2,sy+12,11,3,RGB(3,7,3));
 if(goblin){
  rect(fb,sx+3,sy+3,9,8,RGB(12,24,7));
  rect(fb,sx+2,sy+1,11,5,RGB(16,27,8));
  px(fb,sx+5,sy+5,C_RED);px(fb,sx+10,sy+5,C_RED);
  rect(fb,sx+5,sy+10,5,3,RGB(10,13,5));
 } else {
  rect(fb,sx+4,sy+1,7,5,C_SKIN);
  rect(fb,sx+3,sy+6,9,6,RGB(8,12,26));
  rect(fb,sx+2,sy+7,3,5,RGB(21,21,24));
  rect(fb,sx+10,sy+7,3,5,RGB(21,21,24));
  rect(fb,sx+4,sy+12,3,3,RGB(17,11,7));rect(fb,sx+9,sy+12,3,3,RGB(17,11,7));
  if(facing==0) rect(fb,sx+11,sy+6,2,7,RGB(25,25,27));
  if(facing==1) rect(fb,sx+1,sy+6,2,7,RGB(25,25,27));
 }
}
static void drawWorld(void){
 rect(topPixels,0,0,SW,SH,C_BLACK);
 camX=player.x-120;camY=player.y-88;
 int startX=camX/TILE,startY=camY/TILE;
 for(int sy=0;sy<SH;sy+=TILE) for(int sx=0;sx<SW;sx+=TILE){
  int tx=startX+sx/TILE,ty=startY+sy/TILE;
  int t=(tx>=0&&ty>=0&&tx<MAP_W&&ty<MAP_H)?worldMap[ty][tx]:3;
  drawTile(topPixels,sx-(camX%TILE),sy-(camY%TILE),t,tx,ty);
 }
 // small clearing / landmark sign
 int signX=17*TILE-camX,signY=18*TILE-camY;
 rect(topPixels,signX+5,signY+4,5,10,RGB(17,10,4));rect(topPixels,signX,signY,15,7,C_GOLD);
 for(int i=0;i<MAX_ENEMIES;i++) if(enemies[i].alive){
  int sx=enemies[i].x-camX,sy=enemies[i].y-camY;
  drawActor(topPixels,sx,sy,true,0);
  rect(topPixels,sx,sy-3,15,2,C_BLACK);rect(topPixels,sx,sy-3,enemies[i].hp*5,2,C_RED);
 }
 drawActor(topPixels,player.x-camX,player.y-camY,false,player.dir);
 // attack flash
 if(attackCooldown>5){ int ax=player.x-camX+(player.dir==0?13:(player.dir==1?-8:2)); int ay=player.y-camY+(player.dir==2?-8:(player.dir==3?13:3)); rect(topPixels,ax,ay,7,7,C_GOLD); }
 border(topPixels,0,0,SW,SH,RGB(6,9,5));
}
static void drawBar(uint16_t *fb,int x,int y,int w,int value,int max,uint16_t color){
 rect(fb,x,y,w,7,C_BLACK);int fill=max>0?(w*value/max):0;if(fill<0)fill=0;if(fill>w)fill=w;
 rect(fb,x+1,y+1,fill>1?fill-2:0,5,color);border(fb,x,y,w,7,RGB(18,18,18));
}
static void drawPanel(void){
 rect(bottomPixels,0,0,SW,SH,RGB(3,5,12));
 rect(bottomPixels,5,5,246,182,RGB(8,11,19));border(bottomPixels,5,5,246,182,RGB(22,24,31));
 // title bar
 rect(bottomPixels,10,10,236,20,RGB(14,19,30));
 for(int i=0;i<9;i++) rect(bottomPixels,14+i*2,15,1,1,C_GOLD);
 consoleClear();
 iprintf("\x1b[2;2HELDORIA  -  FOREST\n");
 iprintf("\x1b[4;2HCLASS: ADVENTURER   LV %d\n",level);
 iprintf("\x1b[6;2HHP"); drawBar(bottomPixels,42,49,190,player.hp,20,RGB(4,25,7));
 iprintf("\x1b[8;2HEXP %d / %d",expPoints,level*10);
 iprintf("\x1b[10;2HGOLD: %d",gold);
 iprintf("\x1b[12;2HGOBLINS DEFEATED: %d",kills);
 iprintf("\x1b[15;2HD-PAD: MOVE   A: ATTACK");
 iprintf("\x1b[17;2HSTART: HELP / PAUSE");
 if(showHelp) iprintf("\x1b[20;2HFind goblins in the forest!");
}
static void attack(void){
 if(attackCooldown) return; attackCooldown=12;
 for(int i=0;i<MAX_ENEMIES;i++) if(enemies[i].alive){
  int dx=enemies[i].x-player.x,dy=enemies[i].y-player.y;
  int forward=(player.dir<2)?(player.dir==0?dx:-dx):(player.dir==3?dy:-dy);
  int side=(player.dir<2)?dy:dx;
  if(forward>=-3&&forward<23&&side>-12&&side<12){
   enemies[i].hp--; if(enemies[i].hp<=0){enemies[i].alive=0;kills++;expPoints+=5;gold+=2;}
   break;
  }
 }
}
static void updateEnemies(void){
 for(int i=0;i<MAX_ENEMIES;i++) if(enemies[i].alive){
  int dx=player.x-enemies[i].x,dy=player.y-enemies[i].y;
  if((dx*dx+dy*dy)<(75*75)){
   int nx=enemies[i].x,ny=enemies[i].y;
   if(dx>4) nx++; else if(dx<-4) nx--; else if(dy>4) ny++; else if(dy<-4) ny--;
   if(!blocked(nx+5,ny+7)) enemies[i].x=nx;
   if(!blocked(nx+5,ny+7)) enemies[i].y=ny;
   if(dx*dx+dy*dy<150 && player.hitTimer==0){player.hp--;player.hitTimer=45;if(player.hp<0)player.hp=0;}
  }
 }
}
int main(void){
 videoSetMode(MODE_5_2D|DISPLAY_BG2_ACTIVE);
 videoSetModeSub(MODE_5_2D|DISPLAY_BG2_ACTIVE);
 vramSetBankA(VRAM_A_MAIN_BG);vramSetBankC(VRAM_C_SUB_BG);
 int bgTop=bgInit(2,BgType_Bmp16,BgSize_B16_256x256,0,0);
 int bgBottom=bgInitSub(2,BgType_Bmp16,BgSize_B16_256x256,0,0);
 topPixels=(uint16_t*)bgGetGfxPtr(bgTop);bottomPixels=(uint16_t*)bgGetGfxPtr(bgBottom);
 Console *bottomConsole = consoleInit(NULL,0,BgType_Text4bpp,BgSize_T_256x256,31,0,false,true);
 consoleSelect(bottomConsole);
 initMap();
 while(1){
  scanKeys();u16 keys=keysHeld();u16 down=keysDown();
  int nx=player.x,ny=player.y;
  if(keys&KEY_LEFT){nx-=PLAYER_SPEED;player.dir=1;}
  if(keys&KEY_RIGHT){nx+=PLAYER_SPEED;player.dir=0;}
  if(keys&KEY_UP){ny-=PLAYER_SPEED;player.dir=2;}
  if(keys&KEY_DOWN){ny+=PLAYER_SPEED;player.dir=3;}
  if(!blocked(nx+5,player.y+8))player.x=nx;
  if(!blocked(player.x+5,ny+8))player.y=ny;
  if(down&KEY_A) attack();
  if(down&KEY_START) showHelp=!showHelp;
  if(attackCooldown>0)attackCooldown--;
  if(player.hitTimer>0)player.hitTimer--;
  updateEnemies();
  if(expPoints>=level*10){expPoints-=level*10;level++;}
  if(player.hp<=0){player.x=24*TILE;player.y=24*TILE;player.hp=20;gold=gold>2?gold-2:0;}
  drawWorld();drawPanel();
  swiWaitForVBlank();
 }
 return 0;
}
