/* chess for Numworks calculator */

#include <eadk.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "pieces.h"
#include "gnuchess.h"

#define XOFF 52
#define YOFF 21

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Chess";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

struct cursor
{
  bool active;
  int i,j;
};

void statuslinemsg(const char * msg) {
  uint16_t c=0xfda6;
  eadk_display_push_rect_uniform((eadk_rect_t){0,0,320,18}, c);
  eadk_display_draw_string(msg, (eadk_point_t){160-(int)(3.5*strlen(msg)), 3}, false, eadk_color_white, c);
}

void statusmsg(const char * msg, int delay) {
  uint16_t c=0xfda6;
  eadk_display_push_rect_uniform((eadk_rect_t){60,121,200,18}, c);
  eadk_display_draw_string(msg, (eadk_point_t){160-(int)(3.5*strlen(msg)), 124}, false, eadk_color_white, c);
  eadk_timing_msleep(delay);
}

void busymsg(bool show, char * msg) {
  uint16_t c=0xfda6;
  if(show)
  {
    eadk_display_push_rect_uniform((eadk_rect_t){270,21,40,18}, c);
    eadk_display_draw_string(msg, (eadk_point_t){290-(int)(3.5*strlen(msg)), 24}, false, eadk_color_white, c);
  } else
  {
    eadk_display_push_rect_uniform((eadk_rect_t){270,21,40,18}, eadk_color_black);
  }
}

void drawtile(int i,int j)
{
  int pmap[7]={0,4,3,1,6,5,2} ;// mapping from gnuchess order to pieces.h order
  int c,p, offset, index;
  eadk_color_t * tile;
  index=(opponent==white) ? (i+(7-j)*8) : (7-i+j*8);
  c=color[index];
  p=board[index];
  offset=13*((i+j+1)%2); // color of tile
  offset+=((c==white) ? 6 : 0); // color of piece
  offset+=pmap[p]; // piece
  tile=((eadk_color_t*) pieces_r5g6b5)+(offset*TILESIZE*TILESIZE);
  eadk_display_push_rect((eadk_rect_t){XOFF+i*TILESIZE,YOFF+j*TILESIZE,TILESIZE,TILESIZE}, tile);
}

void drawboard()
{
  for(int j=0;j<8;j++)
    for(int i=0;i<8;i++)
      drawtile(i,j);
}

void switch_player()
{
  if (opponent == white)
  {
    opponent = black;
    computer = white;
  } else 
  {
    opponent = white;
    computer = black;
  }
}

void drawcursor(struct cursor cur, eadk_color_t c)
{
    static eadk_color_t buf[TILESIZE*TILESIZE];
    eadk_display_pull_rect((eadk_rect_t){XOFF+cur.i*TILESIZE,YOFF+(7-cur.j)*TILESIZE,TILESIZE,TILESIZE}, buf);
    for(int i=0;i<TILESIZE;i++)
      for(int j=0;j<TILESIZE;j++)
        if(i<=1 || j<=1 || i>=TILESIZE-2 || j>=TILESIZE-2)
          buf[i+j*TILESIZE]=c;
    eadk_display_push_rect((eadk_rect_t){XOFF+cur.i*TILESIZE,YOFF+(7-cur.j)*TILESIZE,TILESIZE,TILESIZE}, buf);  
}

void drawcursors(struct cursor cur, struct cursor sel)
{
  if(cur.active) drawcursor(cur, eadk_color_white);
  if(sel.active) drawcursor(sel, eadk_color_green);
}

static void boardpos ( int x, int y, int *c, int *r ) {
    if (computer == black ) {
        *c = x ;
        *r = y ;
    } else {
        *c = 7 - x ;
        *r = 7 - y ;
    }
}

int generate_move(struct cursor cur, struct cursor sel, char*move_buffer)
{
  char mv_s[5];
  unsigned short mv;
  int i,j;
  boardpos(sel.i,sel.j,&i,&j);
  mv_s[0] = 'a' + i;
  mv_s[1] = '1' + j;
  boardpos(cur.i,cur.j,&i,&j);
  mv_s[2] = 'a' + i;
  mv_s[3] = '1' + j;
  mv_s[4] = '\00';
  return VerifyMove(opponent, mv_s , 0 , &mv, move_buffer);
}

void thinking_cb()
{
  eadk_keyboard_state_t keyboard = eadk_keyboard_scan();
  if (eadk_keyboard_key_down(keyboard, eadk_key_ans)) timeout=true;
  //~ if (eadk_keyboard_key_down(keyboard, eadk_key_back)) timeout=true;
  if (eadk_keyboard_key_down(keyboard, eadk_key_on_off)) timeout=true;
}

void do_computer_move(char *move_buffer)
{
  busymsg(true,"busy!");
  SelectMove ( computer , 0 , thinking_cb, move_buffer);
  busymsg(false,"");

}

void show_illegal_move(struct cursor cur, int delay)
{
  drawcursor(cur, eadk_color_red);
  eadk_timing_msleep(delay);
}

int selected_own_color(struct cursor cur)
{
  if(opponent==white)
    return color[cur.i+8*cur.j]==opponent;
  else
    return color[(7-cur.i)+8*(7-cur.j)]==opponent;
}

void setlevel () {
  char msg[20];
  sprintf(msg,"Setting level %d", (int) Level);
  statusmsg(msg,500);
  switch (Level) {
      case 1 :
          TCmoves = 60;
          TCminutes = 5;
          break;
      case 2 :
          TCmoves = 60;
          TCminutes = 15;
          break;
      case 3 :
          TCmoves = 60;
          TCminutes = 30;
          break;
      case 4 :
          TCmoves = 40;
          TCminutes = 30;
          break;
      case 5 :
          TCmoves = 40;
          TCminutes = 60;
          break;
      case 6 :
          TCmoves = 40;
          TCminutes = 120;
          break;
      case 7 :
          TCmoves = 40;
          TCminutes = 240;
          break;
  }
  TCflag = (TCmoves > 1);
  SetTimeControl();
}

void mainloop()
{
  bool update,computer_move, done;
  struct cursor cur={false,0,0},sel={false,0,0};
  char move_buffer[20]="";
  statuslinemsg("CHESS");
  
  GNUChess_Initialize();
  //~ short myboard[64]=
   //~ {rook,0,0,0,king,0,0,0,
    //~ 0,0,0,0,0,0,0,0,
    //~ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    //~ pawn,0,0,0,0,0,0,0,
    //~ 0,0,0,0,king,0,0,0};
  //~ short mycolor[64]=
   //~ {white,2,2,2,white,2,2,2,
    //~ 2,2,2,2,2,2,2,2,
    //~ 2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    //~ white,2,2,2,2,2,2,2,
    //~ 2,2,2,2,black,2,2,2};
  //~ for(int i=0;i<64;i++)
  //~ {
    //~ board[i]=myboard[i];
    //~ color[i]=mycolor[i];
  //~ }
  //~ InitializeStats();

  drawboard();
  
  update=false;
  computer_move=false;
  done=0;
  while (true) 
  {
    int32_t timeout = 1000;
    eadk_event_t ev = eadk_event_get(&timeout); 
    switch (ev)
    {
      case eadk_key_on_off:
        return;
      case eadk_event_back:
        switch_player();
        if(!done) computer_move=true;
        update=true;
        break;
      case eadk_event_left:
        if(cur.active) drawtile(cur.i,7-cur.j);
        if(cur.i>0) cur.i-=1;
        cur.active=true;
        break;
      case eadk_event_right:
        if(cur.active) drawtile(cur.i,7-cur.j);
        if(cur.i<7) cur.i+=1;
        cur.active=true;
        break;
      case eadk_event_up:
        if(cur.active) drawtile(cur.i,7-cur.j);
        if(cur.j<7) cur.j+=1;
        cur.active=true;
        break;
      case eadk_event_down:
        if(cur.active) drawtile(cur.i,7-cur.j);
        if(cur.j>0) cur.j-=1;
        cur.active=true;
        break;
      case eadk_event_ok:
        if(sel.active && !done) 
        {
          if(!generate_move(cur,sel, move_buffer)) 
          {
            show_illegal_move(cur,250);
            sel.active=false;
          }
          else
          {
            sel.active=false;
            cur.active=false;
            if(!done) computer_move=true;
          }
        } else
        {
          if(selected_own_color(cur))
          {
            cur.active=true;
            sel=cur;
          } else
          {
            show_illegal_move(cur,250);
          }
        }
        update=true;
        break;
    case eadk_event_minus:
        if(Level>1) Level-=1;
        setlevel();
        update=true;
        break;
    case eadk_event_plus:
        if(Level<7) Level+=1;
        setlevel();
        update=true;
        break;
    case eadk_event_exe:
        GNUChess_Initialize();
        done=0;
        computer_move=false;
        update=true;
        busymsg(false, "");
        break;
    }
    if(update) 
    {
      drawboard();
      update=false;
    }
    drawcursors(cur, sel);
    if(computer_move)
    {
      cur.active=false;
      sel.active=false;
      do_computer_move(move_buffer);
      drawboard();
      computer_move=false;
    }
    done=rootflags()&0x0004;
    done=done+mate; //todo count material
    if(mate && move_buffer[4]=='+') busymsg(true, "Mate");
    if(rootflags()&0x0004 || (mate && move_buffer[4]!='+')) busymsg(true, "Done"); // sometimes it sees draw as mate?
  }

}

int main(int argc, char * argv[]) {
  eadk_timing_millis();
  eadk_display_push_rect_uniform((eadk_rect_t){0,0,EADK_SCREEN_WIDTH,EADK_SCREEN_HEIGHT}, 0x0);
  mainloop();
}
