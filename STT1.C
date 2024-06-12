/*********************************************************************/
/* STTools Disk Utility 					     */
/*	started 7/08/86 K.H.   Copyright (c) 1986 Kenneth L. Hurley  */
/*********************************************************************/

/*********************************************************************/
/* DEFINES						   	     */
/*********************************************************************/

#define WI_KIND (NAME | UPARROW | DNARROW | VSLIDE)
#define SOURCE 0
#define DESTINATION 1
#define MEMERR 2
#define RESTERR 3
/* #define	LOADABLE	/* for loadable resource file */

#define DISKEDIT 0	/* available routines to use edit */
#define FILEEDIT 1
#define SEAREDIT 2
#define UNEREDIT 3
#define low8bits(x) ((x)&0xff)

/*********************************************************************/
/* INCLUDE FILES						     */
/*********************************************************************/

#include "obdefs.h"
#include "gemdefs.h"
#include "osbind.h"

#ifndef LOADABLE

#include "portab.h"
#include "stools.rsh"

#else

#include "define.h"

#endif

#include "stools.h"

/*********************************************************************/
/* EXTERNALS						   	     */
/*********************************************************************/

extern int	gl_apid;
extern int	global[16];
extern char	diskbuff[];
extern int	fileclust,filesec,fnumcl;
extern	struct	dirfiles {
	  char 	fname[8];
	  char 	fext[3];
	  char 	fattr;
	  char 	fresvrd[10];
	  unsigned int ftime;
	  unsigned int fdate;
	  unsigned int fsclust;
	  long	fsize;
	} *dirfiles;

/*********************************************************************/
/* GLOBAL VARIABLES					   	     */
/*********************************************************************/

FDB	undo_mfdb,screen_mfdb;		/* form definition blocks */

int	gl_hchar;
int	gl_wchar;
int	gl_wbox;
int	gl_hbox;	/* system sizes */

int 	phys_handle;	/* physical workstation handle */
int 	handle;		/* virtual workstation handle */
int	wi_handle;	/* window handle */
int	top_window;	/* handle of topped window */

int	xdesk,ydesk,hdesk,wdesk;
int	xold,yold,hold,wold;
int	xwork,ywork,hwork,wwork;	/* desktop and work areas */

int	msgbuff[8];	/* event message buffer */
int 	event;
int	keycode;	/* keycode returned by event-keyboard */
int	mx,my;		/* mouse x and y pos. */
int	butdown;	/* button state tested for, UP/DOWN */
int	ret;		/* dummy return variable */

int	hidden;		/* current state of cursor */

int	fulled;		/* current state of window */

int	contrl[12];
int	intin[128];
int	ptsin[128];
int	intout[128];
int	ptsout[128];	/* storage wasted for idiotic bindings */

int work_in[11];	/* Input to GSX parameter array */
int work_out[57];	/* Output from GSX parameter array */
int pxyarray[10];	/* input point array */

char	undo_area[32000];	/* undo area for screen */

int	cursor,line;	/* cursor position */

int	fo_gx,fo_gy,fo_gw,fo_gh;	/* global form sizes */
long	form_addr;
OBJECT	*sel_obj;	/* pointer to object array */
int	exit_obj;	/* object that was selected to exit with */

long	gl_menu;	/* global menu pointer in this case 1 form */

int	error;		/* error return code */
long	max_tracks;	/* maximum tracks of allocated memory */

OBJECT	*obj_ptr;
TEDINFO	*txt_ptr;
int	num_flops,cur_flop;	/* drive input from system call */
int	numdrives;

char	ins_str[80] = "[2][ |Insert ";
char	st_str[] = 
"[2][ |Insert Original write|protected disk in Drive ?:|and Duplicate in Drive ?:.][ OK |Abort]";
char	eofsstr[] = 
"[3][ End of file reached and | the search string was | not found.][ OK ]";

int	tr_start,tr_end;	/* track start and end pointers */
int	si_start,si_end;
int	i;

int	gr_mkmx,gr_mkmy,gr_mkmstate,gr_mkkstate;
int	obj_selected,new_obj,exit_prog;
int	bcon,scfon,scnfon,dsson;
int	orginal,duplicat;

long	gl_mw;
int	globntracks,globnsecs,globnsides;
int	x,y,w,h;

int	rez,version,defdrive,menuedit;
int	track,sector,side;
int	restdr[3],readok,xcurs,ycurs,editon;
int	readon;

char	trsecstr[80] = " Track: ";
char	logstr[] = " Logical Sector: 00000    Cluster: 00000";
int	scrtype;
unsigned int	logsec;
char	intstr1[6];
char	intstr2[6];
char	gofstr[] = "@99999";
char	grelstr[] = "@99999";
char	seastr[21] = "";
char	seahex[41] = "";

struct bpb {
       unsigned int  	recsiz,		/* sector size in bytes */
			clsiz,		/* cluster size in sectors */
			clsizb,		/* cluster size in bytes */
			rdlen,		/* root directory length in sectors */
			fsiz,		/* FAT size in sectors	*/
			fatrec,		/* sector # of 2nd FAT area */
			datrec,		/* sector # of 1st data sector */
			numcl,		/* number of data clusters on disk */
			bflags;		/* various flags? */
	} *bpbptr, mybpb;

struct bpb defbpb = { 512, 2, 1024, 7, 5, 6, 18, 351, 0 };

long 	avdriv;
int	curedit;
char	klhmdc[] = "!!!";

/****************************************************************/
/*  GSX UTILITY ROUTINES.					*/
/****************************************************************/

hide_mouse()
{
	if(! hidden){
		graf_mouse(M_OFF,0x0L);
		hidden=TRUE;
	}
}

show_mouse()
{
	if(hidden){
		graf_mouse(M_ON,0x0L);
		hidden=FALSE;
	}
}

/****************************************************************/
/* open virtual workstation					*/
/****************************************************************/
open_vwork()
{
int i;
	for(i=0;i<10;work_in[i++]=1);
	work_in[10]=2;
	handle=phys_handle;
	v_opnvwk(work_in,&handle,work_out);

	screen_mfdb.fd_w = work_out[0] + 1;
	screen_mfdb.fd_h = work_out[1] + 1;
	screen_mfdb.fd_wdwidth = screen_mfdb.fd_w >> 4;
	screen_mfdb.fd_stand = FALSE;

	vq_extnd(handle,1,work_out);

	screen_mfdb.fd_nplanes = work_out[4];
	screen_mfdb.fd_addr = Logbase();	/* set up form definition */

	undo_mfdb.fd_w = screen_mfdb.fd_w;
	undo_mfdb.fd_h = screen_mfdb.fd_h;	/* copy width and height */
	undo_mfdb.fd_wdwidth = screen_mfdb.fd_wdwidth;
	undo_mfdb.fd_stand = FALSE;
	undo_mfdb.fd_nplanes = screen_mfdb.fd_nplanes;
	undo_mfdb.fd_addr = &undo_area;		/* set up undo area form */
}

/****************************************************************/
/* set clipping rectangle					*/
/****************************************************************/
set_clip(x,y,w,h)
int x,y,w,h;
{
int clip[4];
	clip[0]=x;
	clip[1]=y;
	clip[2]=x+w;
	clip[3]=y+h;
	vs_clip(handle,1,clip);
}

/****************************************************************/
/*		Accessory Init. Until First Event_Multi		*/
/****************************************************************/
main()
{ register int i,j;

	appl_init();
	phys_handle=graf_handle(&gl_wchar,&gl_hchar,&gl_wbox,&gl_hbox);
	wind_get(0, WF_WORKXYWH, &xdesk, &ydesk, &wdesk, &hdesk);
	open_vwork();

	graf_mouse(ARROW,0x0L);		/* graf to arrow */

	hidden=FALSE;
	fulled=FALSE;
	butdown=TRUE;
	menuedit = FALSE;
	curedit = DISKEDIT;
	exit_prog = FALSE;

	version = Sversion();		/* get version number */

	num_flops = nflops();
	defdrive = Dgetdrv();
	rez = Getrez();
	avdriv = Drvmap();
	
#ifdef LOADABLE

	error = load_rsrc();
	if (error)
          return(1);
#else

	set_rsrc();	/* go load the resource file */

#endif
	error = FALSE;
 	if (rez == 0)
	  { error = TRUE;
	    form_alert(1,"[3][ |Please use High or | Medium resolution.][ Abort ]");
	  }
	if (!error)
	  { numdrives = 0;
	    for (i=4,j=0;i<=128;i *= 2,j++)
	      { if ((avdriv & i) != i)
	          { do_obj(MAINMENU,MENU5S3+j,DISABLED);
	       	    do_obj(EDITMENU,EMEN5S3+j,DISABLED);
	          }
		else
		  numdrives += 1;
	      }
	    rsrc_gaddr(R_TREE,MAPPING,&obj_ptr);
	    obj_ptr += MAPBOX;
	    obj_ptr->ob_y += 4;
	    obj_ptr->ob_height += 2;
	    do_obj(FORMAT,FORMSING,SELECTED);	     
	    cur_flop = 0;
	    globntracks = 80;
	    globnsecs = 9;
	    globnsides = 1;
	    out_main();		/* go output the main form */
	    graf_mouse(HOURGLASS,0x0L);
	    newdrive(defdrive);
	    graf_mouse(ARROW,0x0L);
 	    multi();
	    close_all();	/* clean up everything */
          }
}

/****************************************************************/
/* dispatches all accessory tasks				*/
/****************************************************************/
multi()

{

      do {
	event = evnt_multi(MU_MESAG | MU_BUTTON | MU_KEYBD,
			1,1,butdown,
			0,0,0,0,0,
			0,0,0,0,0,
			msgbuff,0,0,&mx,&my,&ret,&ret,&keycode,&ret);

	wind_update(TRUE);

	if (event & MU_MESAG)
	  switch (msgbuff[0]) 
	    {

	      case WM_REDRAW:
		do_redraw(msgbuff[4],msgbuff[5],msgbuff[6],msgbuff[7]);
		break;

	      case WM_NEWTOP:
	      case WM_TOPPED:
		wind_set(wi_handle,WF_TOP,0,0,0,0);
		break;

	      case MN_SELECTED:
		menu_tnormal(gl_menu,msgbuff[3],TRUE);
		do_menu();
		break;

	      case WM_VSLID:
		do_vslide();
		break;
	      case WM_ARROWED:
		switch (msgbuff[4])
		  { 
		    case 0:
		      if (editon)
			{ 
		      wind_get(wi_handle,WF_VSLIDE,&msgbuff[4],&ret,&ret,&ret);
			  msgbuff[4] -= 30;
			  do_vslide();
		        }
		      break;
		    case 1:
		      if (editon)
			{
		      wind_get(wi_handle,WF_VSLIDE,&msgbuff[4],&ret,&ret,&ret);
			  msgbuff[4] += 30;
			  do_vslide();
		        }
		      break;
		    case 2:
		      if (editon)
			{ msgbuff[4] = EMEN4S2;
			  do_menu();
			}
		      break;
		    case 3:
		      if (editon)
			{ msgbuff[4] = EMEN4S1;
			  do_menu();
		        }
		      break;
	         }
               break;		    
	
	    } /* switch (msgbuff[0]) */

	if ((event & MU_BUTTON) && (scrtype == 0))
	  { if(butdown) 
	      { butdown = FALSE;
		if (editon == TRUE)
		  { hide_mouse();
		    docursor(xcurs,ycurs);
		    graf_mkstate(&gr_mkmx,&gr_mkmy,&gr_mkmstate,&gr_mkkstate);
	            ycurs = (gr_mkmy / (8*rez)) + 1;
		    xcurs = (gr_mkmx / (8*rez)) + 1;
		    if (xcurs < 2)
		      xcurs = 2;	/* check left bounds */
		    if (xcurs > 76)
		      xcurs = 76;	/* check right bounds */
		    if (ycurs < 4)
		      ycurs = 4;	/* check up bounds */
		    if (ycurs > 25)
		      ycurs = 25;	/* check down bounds */

		    if ((ycurs > 3) && (ycurs < 25))
		      if ((xcurs > 1) && (xcurs < 77))
			{ if ((xcurs == 18) || (xcurs == 35) || 
			      (xcurs == 52))
			    xcurs -= 1;
			  if ((xcurs > 52) && (xcurs < 53))
			    xcurs = 53;
			}
		    if (ycurs == 25)
		      { if ((xcurs > 17) && (xcurs < 35))
			  xcurs = 17;
			if ((xcurs > 34) && (xcurs < 53))
			  xcurs = 53;
			if (xcurs > 60)
			  xcurs = 60;
		      }
		    docursor(xcurs,ycurs);
		    wind_save();
		    show_mouse();
		   }
	      }
	   else 
	     butdown = TRUE;
         }
         if (event & MU_KEYBD)
	   if (editon == TRUE)
	     editkey();
	 
      	wind_update(FALSE);

      }
    while(!exit_prog);
}

#ifdef LOADABLE

/************************************************************************/
/* load_rsrc loads a resource file from the disk			*/
/************************************************************************/
int	load_rsrc()

{ graf_mouse(HOURGLASS,0X0L);
  if (!rsrc_load("STOOLS.RSC"))
   { graf_mouse(ARROW,0x0L);
     form_alert(1,&"[3][ | Whoops! | STOOLS.RSC| File Not Found][Abort]");
     return(TRUE);
   }
  rsrc_gaddr(R_TREE,MAINMENU,&gl_menu);
  graf_mouse(ARROW,0x0L);
  return(FALSE);
}

#else

#include "setrsrc.c" 

#endif
  
/************************************************************************/
/*    outputs the form for ST TOOLS					*/
/************************************************************************/

out_main()

{ 
  rsrc_gaddr(R_TREE,MAINMENU,&gl_menu);	/* address of menu bar */
  menu_bar(gl_menu,TRUE);		/* graf it to the screen */
  open_window();			/* open the window */
}

/************************************************************************/
/*    outputs the form for selection             			*/
/************************************************************************/

out_form(form_num)

  int	form_num;
{ rsrc_gaddr(R_TREE,form_num,&form_addr);	/* address of form to output */
  form_center(form_addr,&fo_gx,&fo_gy,&fo_gw,&fo_gh);
  form_dial(FMD_START,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_GROW,xdesk,ydesk,(fo_gw/2),(fo_gh/2),fo_gx,fo_gy,fo_gw,fo_gh);
  objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh); /* draw it */
}

/************************************************************************/
/*    deselects the objects specified bits            			*/
/************************************************************************/

undo_obj(tree_num,obj_num,wh_bit)

int	tree_num,obj_num,wh_bit;

{ rsrc_gaddr(R_TREE,tree_num,&sel_obj); /* get address of tree */
  sel_obj = sel_obj + obj_num;
  sel_obj->ob_state = (sel_obj->ob_state & ~wh_bit); /* set bit = 0 */
}

/************************************************************************/
/*    selects the objects specified bits            			*/
/************************************************************************/

do_obj(tree_num,obj_num,wh_bit)

int	tree_num,obj_num,wh_bit;

{ rsrc_gaddr(R_TREE,tree_num,&sel_obj); /* get address of tree */
  sel_obj = sel_obj + obj_num;
  sel_obj->ob_state = (sel_obj->ob_state | wh_bit); /* set bit = 1 */
}

/************************************************************************/
/*    gets the current bit settings of specified bit   			*/
/************************************************************************/

int	get_bit(tree_num,obj_num,wh_bit)

int	tree_num,obj_num,wh_bit;

{ rsrc_gaddr(R_TREE,tree_num,&sel_obj); /* get address of tree */
  sel_obj = sel_obj + obj_num;
  return((sel_obj->ob_state & wh_bit));	/* get bit of ob_state */
}

/************************************************************************/
/* strlen gets the string length of the string				*/
/************************************************************************/

int	strlen(str_ptr)

char	*str_ptr;

{ int	i;

  for (i=0;*str_ptr++;i++);
  return(i);

}

/**************************************************************************/
/* uitoa puts an integer into a string					  */
/**************************************************************************/
uitoa(inint,outstr)

unsigned int inint;
char outstr[];

{ int i;
  
  i = 0;
  do
    {
      outstr[i++] = inint % 10 + '0';
    }
  while ((inint /= 10) > 0);

  outstr[i] = '\0';
  reverse(outstr);
}

/**************************************************************************/
/* ltoa puts an long into a string					  */
/**************************************************************************/
ltoa(inlong,outstr)

long inlong;
char outstr[];

{ int i,j;
  
  i = 0;
  j = 0;
  do
    {
      outstr[i++] = inlong % 10 + '0';
      j += 1;
      if ((j == 3) && ((inlong / 10) > 0))
        { j = 0;
	  outstr[i++] = ',';
        }
    }
  while ((inlong /= 10) > 0);

  outstr[i] = '\0';
}

/**************************************************************************/
/* reverse reverses a string						  */
/**************************************************************************/
reverse(instr)
char instr[];
{
  int c,i,j;

  for (i=0,j=strlen(instr)-1;i<j;i++,j--)
    { c = instr[i];
      instr[i] = instr[j];
      instr[j] = c;
     }
}

/**************************************************************************/
/* strcat concats 2 strings together					  */
/**************************************************************************/

strcat(str1_ptr,str2_ptr)

char	*str1_ptr,*str2_ptr;

{ int	st1len,st2len,counter;

  st1len = strlen(str1_ptr);		/* get the string lengths */
  st2len = strlen(str2_ptr);

  str1_ptr = str1_ptr + st1len;

  for (counter=0;counter<=st2len;counter++)
    *str1_ptr++ = *str2_ptr++;
}


/************************************************************************/
/* set text set the text of an object					*/
/************************************************************************/
set_text(tr_num,tr_obj,str_in,draw)

int	tr_num,tr_obj;
char	*str_in;
int	draw;

{ TEDINFO	*ted_ptr;

  rsrc_gaddr(R_TREE,tr_num,&sel_obj);
  sel_obj += tr_obj;
  ted_ptr = sel_obj->ob_spec;
  ted_ptr->te_ptext = str_in;

  if (draw)
    objc_draw(form_addr,tr_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* set tlen sets the length of a text 					*/
/************************************************************************/
set_tlen(tr_num,tr_obj,inlen)

int	tr_num,tr_obj;
int	inlen;

{ TEDINFO	*ted_ptr;

  rsrc_gaddr(R_TREE,tr_num,&sel_obj);
  sel_obj += tr_obj;
  ted_ptr = sel_obj->ob_spec;
  ted_ptr->te_txtlen = inlen;

}

/************************************************************************/
/* get byte gets the text of an object					*/
/************************************************************************/
int get_byte(tr_num,tr_obj)

int	tr_num,tr_obj;

{ TEDINFO	*ted_ptr;
  char	*str_ptr;

  rsrc_gaddr(R_TREE,tr_num,&sel_obj);
  sel_obj = sel_obj + tr_obj;
  ted_ptr = sel_obj->ob_spec;
  str_ptr = ted_ptr->te_ptext;

  return (*str_ptr);
}

/************************************************************************/
/* set byte sets the text of an object					*/
/************************************************************************/
set_byte(tr_num,tr_obj,byte_in,byte_num)

int	tr_num,tr_obj;
char	byte_in;
int	byte_num;

{ TEDINFO	*ted_ptr;
  char	*str_ptr;

  rsrc_gaddr(R_TREE,tr_num,&sel_obj);
  sel_obj = sel_obj + tr_obj;
  ted_ptr = sel_obj->ob_spec;
  str_ptr = ted_ptr->te_ptext;

  *(str_ptr+byte_num) = byte_in;
}

/************************************************************************/
/* bset_char sets the character of a box_char				*/
/************************************************************************/
bset_char(tr_num,tr_obj,byte_in,flredraw)

int	tr_num,tr_obj,byte_in,flredraw;

{ OBJECT *treeaddr;
  
  rsrc_gaddr(R_TREE,tr_num,&treeaddr);
  treeaddr += tr_obj;
  treeaddr->ob_spec &= 0x00ffffffL;
  treeaddr->ob_spec |= ((long)(byte_in & 0xff) << 24);
  if (flredraw)
    { rsrc_gaddr(R_TREE,tr_num,&treeaddr);
      objc_draw(treeaddr,tr_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
    }
}

/************************************************************************/
/* open_window opens the main window					*/
/************************************************************************/
open_window()

{ 
	wi_handle = wind_create(WI_KIND,xdesk,ydesk,wdesk,hdesk);
	wind_set(wi_handle,WF_NAME," STTOOLS  ver 1.0 ",0,0);
	graf_growbox(xdesk+wdesk/2,ydesk+hdesk/2,gl_wbox,gl_hbox,xdesk,ydesk,
			wdesk,hdesk);
	wind_open(wi_handle,xdesk,ydesk,wdesk,hdesk);
	wind_get(wi_handle,WF_WORKXYWH,&xwork,&ywork,&wwork,&hwork);

	clr_window();
}

/************************************************************************/
/* clr_window clears the window to background color			*/
/************************************************************************/
clr_window()
  {

     wind_update(TRUE);
     hide_mouse();

     vsf_interior(handle,2);	/* interior design */
     vsf_style(handle,8);	/* ummm??? */
     vsf_color(handle,0);	/* color black */
     pxyarray[0] = xwork;
     pxyarray[1] = ywork;
     pxyarray[2] = xwork+wwork-1;
     pxyarray[3] = ywork+hwork-1;
     v_bar(handle,pxyarray);
     wind_save();

     show_mouse();
     wind_update(FALSE);
  }

/************************************************************************/
/* close_all closes all windows and cleans everything up		*/
/************************************************************************/

close_all()

{
  menu_bar(gl_menu,FALSE);
  wind_close(wi_handle);
  graf_shrinkbox(xwork+wwork/2,ywork+hwork/2,gl_wbox,gl_hbox,
			xwork,ywork,wwork,hwork);
  wind_delete(wi_handle);
  v_clsvwk(handle);

#ifdef LOADABLE

  rsrc_free();

#endif

  appl_exit();
}

/************************************************************************/
/* do_menu takes the menu item selected an executes appropriate routine	*/
/************************************************************************/
do_menu()
{ long	treeaddr;

  if (menuedit == TRUE)
    { do_edit();
      return(0);
    }

  switch (msgbuff[4])
    { 
	case MENU1S1:
	  out_form(ABSTOOLS);
	  form_do(form_addr,0);
	  undo_obj(ABSTOOLS,ABOKBUT,SELECTED);	  
  	  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
			fo_gx,fo_gy,fo_gw,fo_gh);
  	  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
	  break;
	case MENU2S1:
	  fi_map();
	  break;
	case MENU2S2:
	  fi_unerase();
	  break;
	case MENU2S3:
	  fi_comp();
	  break;
	case MENU2S4:
	  exit_obj = getsearch();	/* get the search string */
	  if (exit_obj == SEAOK)
	    fi_edit(TRUE);
	  break;
	case MENU2S5:
	  fi_erase();
	  break;
	case MENU2S6:
	  fi_find();
	  break;
	case MENU2S7:
	  fi_verify();
	  break;
	case MENU2S8:
	  fi_edit(FALSE);
	  break;
	case MENU2S9:
	  fi_copy(FALSE);
	  break;
	case MENU2S10:
	  fi_copy(TRUE);
	  break;
	case MENU2S11:
	  fi_rename();
	  break;
	case MENU2S12:
	  fi_print();
	  break;
	case MENU2S13:
	  fi_chstat();
	  break;
	case MENU3S1:
	  diskmapping();
	  break;
	case MENU3S2:
	  diskformat();
	  break;
	case MENU3S3:
	  diskcompare();
	  break;
	case MENU3S4:
	  exit_obj = getsearch();	/* get the search string */
	  if (exit_obj == SEAOK)
	    disksearch(0L);
	  break;
	case MENU3S5:
	  diskerase();
	  break;
	case MENU3S6:
	  diskverify();
	  break;
	case MENU3S7:
	  diskinfo(defdrive);
	  menu_bar(gl_menu,FALSE);		/* turn off main menu */
	  do_obj(EDITMENU,EMEN4S6,CHECKED);
	  undo_obj(EDITMENU,EMEN4S7,CHECKED);
	  logsec = 0;
	  readon = TRUE;
	  rsrc_gaddr(R_TREE,EDITMENU,&gl_menu);
	  menu_bar(gl_menu,TRUE);
	  menuedit = TRUE;
	  clr_window();
	  xcurs = 2;
	  ycurs = 4;
	  if (scrtype == 0)
	    docursor(xcurs,ycurs);
	  readok = TRUE;
	  editon = TRUE;
	  do_rw();
	  break;
	case MENU3S8:
	  diskcopy();
	  break;
	case MENU3S9:
	  disklabel();
	  break;
	case MENU3S10:
	  diskrecover();
	  break;
	case MENU4S1:
	  fold_map();
	  break;
	case MENU4S2:
	  fold_unerase();
	  break;
	case MENU4S3:
	  fold_erase();
	  break;
	case MENU4S4:
	  fold_sort();
	  break;
	case MENU4S5:
	  fold_copy(FALSE);
	  break;
	case MENU4S6:
	  fold_copy(TRUE);
	  break;
	case MENU4S7:
	  fold_rename();
	  break;
	case MENU4S8:
	  fold_print();
	  break;
	case MENU5S1:
	case MENU5S2:
	case MENU5S3:
	case MENU5S4:
	case MENU5S5:
	case MENU5S6:
	case MENU5S7:
	case MENU5S8:	
	  newdrive(msgbuff[4]-MENU5S1);
	  break;
	case MENU5S9:
	  exit_prog = TRUE;
	  break;
    }
}

/************************************************************************/
/* do_edit takes the menu item selected an executes appropriate routine	*/
/************************************************************************/
do_edit()
{ long	treeaddr,startoff;
  register int i,j;

  switch (msgbuff[4])
    {
	case EMEN1S1:
	  out_form(ABSTOOLS);
	  form_do(form_addr,0);
	  undo_obj(ABSTOOLS,ABOKBUT,SELECTED);	  
  	  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
			fo_gx,fo_gy,fo_gw,fo_gh);
  	  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
	  break;
	case EMEN2S1:
	  do_obj(EDITMENU,EMEN2S1,CHECKED);
	  undo_obj(EDITMENU,EMEN2S2,CHECKED);
	  undo_obj(EDITMENU,EMEN2S3,CHECKED);
	  scrtype = 0;
	  docursor(xcurs,ycurs);
	  clr_window();
	  hide_mouse();
	  wind_update(TRUE);
	  outscreen();
	  docursor(xcurs,ycurs);
	  wind_save();
	  wind_update(FALSE);
	  show_mouse();
	  break;
	case EMEN2S2:
	  undo_obj(EDITMENU,EMEN2S1,CHECKED);
	  do_obj(EDITMENU,EMEN2S2,CHECKED);
	  undo_obj(EDITMENU,EMEN2S3,CHECKED);
	  scrtype = 1;
	  docursor(xcurs,ycurs);
	  clr_window();
	  hide_mouse();
	  wind_update(TRUE);
	  outscreen();
	  wind_save();
	  wind_update(FALSE);
	  show_mouse();
	  break;
	case EMEN2S3:
	  undo_obj(EDITMENU,EMEN2S1,CHECKED);
	  undo_obj(EDITMENU,EMEN2S2,CHECKED);
	  do_obj(EDITMENU,EMEN2S3,CHECKED);
	  scrtype = 2;
	  docursor(xcurs,ycurs);
	  clr_window();
	  hide_mouse();
	  wind_update(TRUE);
	  outscreen();
	  wind_save();
	  wind_update(FALSE);
	  show_mouse();
	  break;
	case EMEN3S5:
	  if (xcurs < 52)
	    { i = (ycurs-4)*24;
	      j = (xcurs-2);
	      j -= j/17;
	      i += j/2;
	    }
	  else
	    { i = (ycurs-4)*24;
	      i += (xcurs-53);
	    }
	  startoff = (logsec * 512) + (i+1);
	  if (curedit == FILEEDIT)
	    startoff = (filesec * 512) + (i+1);
	  exit_obj = getsearch();
	  if (exit_obj == SEAOK)
	    { if (curedit == DISKEDIT)
	        disksearch(startoff);
	      else if (curedit == FILEEDIT)
	        { startoff = searchfile(0L,startoff);
	 	  if (startoff != -1)
	            { i = filesec;
		      filesec = startoff / 512;
		      i = (filesec - i)/2;
		      while (i-- > 0)
			fileclust = get_fatword(fileclust);			
		      i = startoff % 512;
		      doxy(i);
		      do_obj(EDITMENU,EMEN4S6,CHECKED);
		      undo_obj(EDITMENU,EMEN4S7,CHECKED);
		      do_obj(EDITMENU,EMEN4S5,DISABLED);
		      hide_mouse();		      
		      logsec = ((fileclust-2)*2)+bpbptr->datrec+(filesec & 1);
		      readon = TRUE;
		      clr_window();
		      if (scrtype == 0)
			docursor(xcurs,ycurs);
		      readok = TRUE;
		      editon = TRUE;
		      do_rw();
		      show_mouse();
		      graf_mouse(ARROW,0x0L);
		    }
		  else
		    form_alert(1,eofsstr);
		}
	    }
	  break;
	case EMEN3S6:
	  diskboot();
	  break;
	case EMEN4S1:
	  switch (curedit)
	    { case SEAREDIT:
	      case DISKEDIT:
	  	if (logsec < ((bpbptr->numcl*2)+bpbptr->datrec))
            	  { logsec += 1;
		    readok = TRUE;
            	  }
          	else
            	  readok = FALSE;
		break;
	      case FILEEDIT:
		readok = FALSE;
	        if ((get_fatword(fileclust)) != -1)
	          { filesec += 1;
		    if ((filesec & 1) == 0)
	              fileclust = get_fatword(fileclust);
		    logsec = ((fileclust-2)*2)+bpbptr->datrec+(filesec & 1);
		    readok = TRUE;
		  }
		break;
	      case UNEREDIT:
		break;
	    }
	  do_rw();
          break;
	case EMEN4S2:
	  switch (curedit)
	    { case DISKEDIT:
	      case SEAREDIT:
	   	if (logsec > 0)
          	  { logsec -= 1;
	 	    readok = TRUE;
          	  }
          	else
          	  readok = FALSE;
		break;
	      case FILEEDIT:
	        if ((fileclust != ibmword(dirfiles->fsclust)) 
			|| ((filesec & 1) == 1))
	      	  { filesec -= 1;
		    if ((filesec & 1) == 1)
	              fileclust = prevclust();
		    readok = TRUE;
		  }
	        else
		  readok = FALSE;
	        logsec = ((fileclust-2)*2)+bpbptr->datrec+(filesec & 1);
		break;
	      case UNEREDIT:
		break;
	    }
	  do_rw();
          break;
	case EMEN4S3:
	  gotooff();
	  break;
	case EMEN4S4:
	  gotorel();
	  break;
	case EMEN4S5:
	  gotoabs();
	  break;
	case EMEN4S6:
	  do_obj(EDITMENU,EMEN4S6,CHECKED);
	  undo_obj(EDITMENU,EMEN4S7,CHECKED);
	  readon = TRUE;
	  do_rw();
	  break;	  
	case EMEN4S7:
	  do_obj(EDITMENU,EMEN4S7,CHECKED);
	  undo_obj(EDITMENU,EMEN4S6,CHECKED);
	  readon = FALSE;
	  do_rw();
	  break;	  
	case EMEN5S1:
	case EMEN5S2:
	case EMEN5S3:
	case EMEN5S4:
	case EMEN5S5:
	case EMEN5S6:
	case EMEN5S7:
	case EMEN5S8:
	  newdrive(msgbuff[4]-EMEN5S1);
	  diskinfo(defdrive);
	  readok = TRUE;
	  do_rw();
	  break;
        case EMEN5S9:
	  switch (curedit)
	    { case FILEEDIT:
		fi_edit();
		if (curedit == DISKEDIT)
		  editoff();
		break;
	      case UNEREDIT:
		fi_unerase();
		if (curedit == DISKEDIT)
		  editoff();
		break;
	      case DISKEDIT:
	      case SEAREDIT:
		editoff();
		break;
	     }
	  break;
    }  
}

/************************************************************************/
/* newdrive takes the new drive and sets up the menus accordiangly	*/
/************************************************************************/
newdrive(indrive)
int	indrive;
{
  undo_obj(MAINMENU,MENU5S1+defdrive,CHECKED);
  undo_obj(EDITMENU,EMEN5S1+defdrive,CHECKED);
  defdrive = indrive;
  do_obj(MAINMENU,MENU5S1+defdrive,CHECKED);
  do_obj(EDITMENU,EMEN5S1+defdrive,CHECKED);
  
  if (defdrive < 2)
    {
      undo_obj(MAINMENU,MENU3S2,DISABLED);	/* undo format for floppy */
      undo_obj(MAINMENU,MENU3S3,DISABLED);	/* undo compare for floppy */
      undo_obj(MAINMENU,MENU3S5,DISABLED);	/* undo erase for floppy */
      undo_obj(MAINMENU,MENU3S8,DISABLED);	/* undo copy for floppy */
      undo_obj(MAINMENU,MENU3S10,DISABLED);	/* undo recover errors */
      undo_obj(EDITMENU,EMEN4S5,DISABLED);	/* undo goto absolute */
     }
  else						/* hard or other type */
    {
      do_obj(MAINMENU,MENU3S2,DISABLED);	/* can't format hard drive */
      do_obj(MAINMENU,MENU3S3,DISABLED);	/* can't compare hard disk */
      do_obj(MAINMENU,MENU3S5,DISABLED);	/* can't erase hard disk */
      do_obj(MAINMENU,MENU3S8,DISABLED);	/* can't copy hard disk */
      do_obj(MAINMENU,MENU3S10,DISABLED);	/* can't recover errors */
      do_obj(EDITMENU,EMEN4S5,DISABLED);	/* can't goto absolute */
    }
}

/************************************************************************/
/*	editkey takes care of keys used during edit			*/
/************************************************************************/
editkey()

{
  register int i,j,k;

  if (((char)(keycode) == 0) || (keycode == 0x0e08) || (keycode == 0x0f09))
    { switch (keycode)
      {
	case 0x3100:
	  if (get_bit(EDITMENU,EMEN4S1,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S1;
	  break;
	case 0x1900:
	  if (get_bit(EDITMENU,EMEN4S2,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S2;
	  break;
	case 0x1800:
	  if (get_bit(EDITMENU,EMEN4S3,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S3;
	  break;
	case 0x2200:
	  if (get_bit(EDITMENU,EMEN4S4,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S4;
	  break;
	case 0x1E00:
	  if (get_bit(EDITMENU,EMEN4S5,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S5;
	  break;
	case 0x1300:
	  if (get_bit(EDITMENU,EMEN4S6,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S6;
	  break;
	case 0x1100:
	  if (get_bit(EDITMENU,EMEN4S7,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN4S7;
	  break;
	case 0x2300:
	  if (get_bit(EDITMENU,EMEN2S1,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN2S1;
	  break;
	case 0x1400:
	  if (get_bit(EDITMENU,EMEN2S2,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN2S2;
	  break;
	case 0x2000:
	  if (get_bit(EDITMENU,EMEN2S3,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN2S3;
	  break;
	case 0x2E00:
	  if (get_bit(EDITMENU,EMEN3S1,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN3S1;
	  break;
	case 0x1600:
	  if (get_bit(EDITMENU,EMEN3S2,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN3S2;
	  break;
	case 0x2100:
	  if (get_bit(EDITMENU,EMEN3S3,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN3S3;
	  break;
	case 0x1F00:
	  if (get_bit(EDITMENU,EMEN3S4,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN3S4;
	  break;
	case 0x1700:
	  if (get_bit(EDITMENU,EMEN3S5,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN3S5;
	  break;	  
	case 0x7800:
	case 0x7900:
	case 0x7A00:
	case 0x7B00:
	case 0x7C00:
	case 0x7D00:
	case 0x7E00:
	case 0x7F00:
	  keycode >>= 8;
	  if (get_bit(EDITMENU,EMEN5S1+(keycode-0x78),DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN5S1+(keycode-0x78);
	  break;
	case 0x1200:
	  if (get_bit(EDITMENU,EMEN5S9,DISABLED) & DISABLED)
	    msgbuff[4] = 0;
	  else
	    msgbuff[4] = EMEN5S9;
	  break;
	case 0x5000:
	  hide_mouse();
	  docursor(xcurs,ycurs);
	  ycurs += 1;
	  if (ycurs > 25)
	    ycurs = 4;
	  if (ycurs == 25)
	    { if ((xcurs > 17) && (xcurs < 35))
		xcurs = 17;
	      if ((xcurs > 34) && (xcurs < 53))
		xcurs = 53;
	      if (xcurs > 60)
		xcurs = 60;
	    }
	  docursor(xcurs,ycurs);
	  msgbuff[4] = 0;
	  wind_save();
	  show_mouse();
	  break;
	case 0x4800:
	  hide_mouse();
	  docursor(xcurs,ycurs);
	  ycurs -= 1;
	  if (ycurs < 4)
	    ycurs = 25;
	  if (ycurs == 25)
	    { if ((xcurs > 17) && (xcurs < 35))
		xcurs = 17;
	      if ((xcurs > 34) && (xcurs < 53))
		xcurs = 53;
	      if (xcurs > 60)
		xcurs = 60;
	    }
	  docursor(xcurs,ycurs);
	  msgbuff[4] = 0;
	  wind_save();
	  show_mouse();
	  break;
	case 0x0e08:
	case 0x4b00:
	  hide_mouse();
	  docursor(xcurs,ycurs);
	  xcurs -= 1;
	  if (xcurs < 2)
	    xcurs = 76;
	  if ((ycurs > 3) && (ycurs < 25))
	    if ((xcurs > 1) && (xcurs < 77))
	      { if ((xcurs == 18) || (xcurs == 35) || 
		      (xcurs == 52))
	          xcurs -= 1;
	      }
	  if (ycurs == 25)
	    { if ((xcurs < 53) && (xcurs > 17))
	        xcurs = 17;
	      if (xcurs > 60)
	        xcurs = 60;
	     }
	  docursor(xcurs,ycurs);
	  msgbuff[4] = 0;
	  wind_save();
	  show_mouse();
	  break;
	case 0x4d00:
	  hide_mouse();
	  docursor(xcurs,ycurs);
	  xcurs += 1;
	  if (xcurs > 76)
	    xcurs = 2;
	  if ((ycurs > 3) && (ycurs < 25))
	    if ((xcurs > 1) && (xcurs < 77))
	      { if ((xcurs == 18) || (xcurs == 35) || 
		      (xcurs == 52))
	          xcurs += 1;
	      }
	  if (ycurs == 25)
	    { if ((xcurs < 53) && (xcurs > 17))
	        xcurs = 53;
	      if (xcurs > 60)
	        xcurs = 2;
	     }
	  docursor(xcurs,ycurs);
	  msgbuff[4] = 0;
	  wind_save();
	  show_mouse();
	  break;
	case 0x0f09:
	  hide_mouse();
	  docursor(xcurs,ycurs);
 	  if (xcurs < 53)
	    { j = (xcurs-2);
	      j -= j/17;
	      xcurs = 53+(j / 2);
	    }
	  else
	    { j = (xcurs - 53); 
	      i = ((j*2)+(j/8))+2;
	      xcurs = i;
	    }
	  docursor(xcurs,ycurs);
	  msgbuff[4] = 0;
	  wind_save();
	  show_mouse();
	  break;
	default:
	  msgbuff[4] = 0;
      }
    do_menu();
   }
  else if (scrtype == 0)
    { keycode &= 0xff;
      if (keycode == 0x1b)
	{ docursor(xcurs,ycurs);
	  clr_window();
	  hide_mouse();
	  wind_update(TRUE);
	  outscreen();
	  docursor(xcurs,ycurs);
	  wind_save();
	  wind_update(FALSE);
	  show_mouse();
	} 
      if (xcurs < 52)
        { if ((keycode >= 'a') && (keycode <= 'f'))
	    keycode -= ('a'-'A');
	  if (((keycode >= '0') && (keycode <= '9')) ||
	    ((keycode >= 'A') && (keycode <= 'F')))
	      { hide_mouse();
		docursor(xcurs,ycurs);
		outchar(xcurs,ycurs,keycode);
		i = (ycurs-4) * 24;
		j = (xcurs-2);
		j -= j/17;
		i += j/2;
		keycode = (keycode <= '9') ? keycode - '0' : (keycode - 'A')+10;
		if (j & 1)
		  diskbuff[i] = ((diskbuff[i] & 0xf0) | keycode);
		else
		  diskbuff[i] = ((diskbuff[i] & 0xf) | (keycode << 4));
		k = ((int)(diskbuff[i]) & 0xff);
		j /= 2;
		outchar(53+j,ycurs,k);
		xcurs += 1;
		if ((xcurs == 18) || (xcurs == 35))
		  xcurs += 1;
	   	if ((ycurs == 25) && (xcurs > 17))
	  	  xcurs = 2;
		if (xcurs == 52)
		  { if (ycurs < 25)
		      ycurs += 1;
		    xcurs = 2;
		  }
		docursor(xcurs,ycurs);
		wind_save();
		show_mouse();		  
	      }
	   }
        else
          { hide_mouse();
	    docursor(xcurs,ycurs);
	    outchar(xcurs,ycurs,keycode);
	    i = (ycurs-4) * 24;
	    i += (xcurs - 53);
	    diskbuff[i] = keycode;
	    j = (xcurs - 53); 
	    i = ((j*2)+(j/8))+2;
	    k = ((keycode >> 4) & 0xf);
	    k = (k <= 9) ? k + '0' : (k - 10)+'A';
	    outchar(i,ycurs,k);
	    k = (keycode & 0xf);
	    k = (k <= 9) ? k + '0' : (k - 10)+'A';
	    outchar(i+1,ycurs,k);
	    xcurs += 1;
	    if (ycurs == 25)
              { if (xcurs > 60)
                  xcurs = 53;
              }
            else
	      if (xcurs > 76)
	        { xcurs = 53;
	          ycurs += 1;
                }
	    docursor(xcurs,ycurs);
	    wind_save();
	    show_mouse();
	  }  	
     }
}

/**************************************************************************/
/* wind_save saves the windows to the undo buffer			  */
/**************************************************************************/

wind_save()

{ ptsin[0] = xwork;
  ptsin[1] = ywork;
  ptsin[2] = xwork+wwork-1;		/* save window 1 */
  ptsin[3] = ywork+hwork-1;
  ptsin[4] = xwork;
  ptsin[5] = ywork;
  ptsin[6] = xwork+wwork-1;
  ptsin[7] = ywork+hwork-1;

  vro_cpyfm(handle,3,ptsin,&screen_mfdb,&undo_mfdb);	/* raster copy */

}

/************************************************************************/
/* wind_rest() restores window from the undo buffer			*/
/************************************************************************/

wind_rest(xc_in,yc_in,wc_in,hc_in)

int	xc_in,yc_in,wc_in,hc_in;
{
  ptsin[0] = xc_in;
  ptsin[1] = yc_in;
  ptsin[2] = xc_in+wc_in-1;		/* restore window 1 */
  ptsin[3] = yc_in+hc_in-1;
  ptsin[4] = xc_in;
  ptsin[5] = yc_in;
  ptsin[6] = xc_in+wc_in-1;
  ptsin[7] = yc_in+hc_in-1;

  vro_cpyfm(handle,3,ptsin,&undo_mfdb,&screen_mfdb);	/* raster copy */
}

/****************************************************************/
/* find and redraw all clipping rectangles			*/
/****************************************************************/
do_redraw(xc,yc,wc,hc)
int xc,yc,wc,hc;
{
GRECT t1,t2;

	hide_mouse();
	wind_update(TRUE);
	t2.g_x=xc;
	t2.g_y=yc;
	t2.g_w=wc;
	t2.g_h=hc;
	wind_get(wi_handle,WF_FIRSTXYWH,&t1.g_x,&t1.g_y,&t1.g_w,&t1.g_h);
	while (t1.g_w && t1.g_h) {
	  if (rc_intersect(&t2,&t1))
	    { set_clip(t1.g_x,t1.g_y,t1.g_w,t1.g_h);
	      wind_rest(t1.g_x,t1.g_y,t1.g_w,t1.g_h);
	    }
	  wind_get(wi_handle,WF_NEXTXYWH,&t1.g_x,&t1.g_y,&t1.g_w,&t1.g_h);
	}
	wind_update(FALSE);
	show_mouse();
	vs_clip(handle,FALSE,ptsin);
}

/************************************************************************/
/* track_wind sets up the information about track and sector in window	*/
/************************************************************************/
track_wind()
{ 
  register int i,trsecnum,j;
  char	tempstr[5];
  long  temp;

  if (defdrive >= 2)
    {  logstr[17] = '\0';
       uitoa(logsec,tempstr);
       strcat(logstr,tempstr);
       strcat(logstr,"   Cluster: ");
       i = (logsec < bpbptr->datrec) ? 0 : (logsec-bpbptr->datrec)/2;
       uitoa(i,tempstr);
       strcat(logstr,tempstr);
       wind_set(wi_handle,WF_NAME,logstr,0,0);
       i = (bpbptr->numcl*2)+bpbptr->datrec;
       temp = (1000*logsec);
       i = temp/i;
       wind_set(wi_handle,WF_VSLIDE,i,0,0,0);
    }
  else
    {
      comptss(logsec);
      trsecstr[8] = '\0';
      uitoa(track,tempstr);
      strcat(trsecstr,tempstr);
      strcat(trsecstr,"   Sector: ");
      uitoa(sector,tempstr);
      strcat(trsecstr,tempstr);
      strcat(trsecstr,"   Side: ");
      uitoa(side+1,tempstr);
      strcat(trsecstr,tempstr);
      strcat(trsecstr,"   Cluster: ");
      i = (logsec < bpbptr->datrec) ? 0 : (logsec-bpbptr->datrec)/2;
      uitoa(i,tempstr);
      strcat(trsecstr,tempstr);
      wind_set(wi_handle,WF_NAME,trsecstr,0,0);
      i = (bpbptr->numcl*2)+bpbptr->datrec;
      temp = (1000*logsec);
      i = temp/i;
      wind_set(wi_handle,WF_VSLIDE,i,0,0,0);
    }
}

/************************************************************************/
/* do_rw does the reading or writing					*/
/************************************************************************/
do_rw()
  {
    if (readok == TRUE)
      {
	graf_mouse(HOURGLASS,0x0L);
        if (readon)
          myrwabs(2,diskbuff,1,logsec,defdrive);
	else
	  myrwabs(3,diskbuff,1,logsec,defdrive);
    	track_wind();
	if (scrtype == 0)
	  docursor(xcurs,ycurs);
	clr_window();
	graf_mouse(ARROW,0x0L);
	hide_mouse();
	wind_update(TRUE);
	outscreen();
	if (scrtype == 0)
	  docursor(xcurs,ycurs);
	wind_save();
	wind_update(FALSE);
	show_mouse();
     }
  }

/************************************************************************/
/* diskinfo gets information about the disk				*/
/************************************************************************/
diskinfo(indrive)
int indrive;

{ char *diptr;
  register int i,j;

  graf_mouse(HOURGLASS,0x0L);
  
  if (indrive < 2)
    { error = myrwabs(2,diskbuff,1,0,indrive);
      if (error != 0)
	bpbptr = 0;
      else if (!(i = u2i(diskbuff+11)) || !(j = low8bits(diskbuff[13])))
	bpbptr = 0;
      else
	{ bpbptr = &mybpb;
	  bpbptr->recsiz = i;
	  bpbptr->clsiz = j;
	  bpbptr->fsiz = u2i(diskbuff+22);
	  bpbptr->fatrec = bpbptr->fsiz + 1;
	  bpbptr->clsizb = i * j;
	  bpbptr->rdlen = (u2i(diskbuff+17) << 5) / i;
	  bpbptr->datrec = bpbptr->fatrec + bpbptr->rdlen + bpbptr->fsiz;
	  bpbptr->numcl = (u2i(diskbuff+19) - bpbptr->datrec) / bpbptr->clsiz;
	  bpbptr->bflags = 0;

	  globnsides = u2i(diskbuff+26);
	  globnsecs = u2i(diskbuff+24);
	  i = globnsides * globnsecs;
	  globntracks = u2i(diskbuff+19) / i;
        }
     }
  else
    bpbptr = Getbpb(indrive);
  if (bpbptr == 0)
    bpbptr = &defbpb;  
}

/************************************************************************/
/* convert 8088 flavored integer to 68000				*/
/************************************************************************/
u2i(loc)
char *loc;
{
	return(low8bits(*(loc+1)) << 8) | low8bits(*loc);
}

/************************************************************************/
/* gotoabs goes to the absolute position indicated			*/
/************************************************************************/
gotoabs()
{ int temptrak,tempsec;

  intstr1[0] = (track / 10) + '0';
  intstr1[1] = (track % 10) + '0';
  set_text(GOABS,GABTRACK,intstr1,FALSE);
  set_tlen(GOABS,GABTRACK,3);

  intstr2[0] = sector + '0';
  set_text(GOABS,GABSECT,intstr2,FALSE);
  set_tlen(GOABS,GABSECT,2);

  if (globnsides > 1)
    undo_obj(GOABS,GABSIDE2,DISABLED);
  else
    do_obj(GOABS,GABSIDE2,DISABLED);

  if (side == 0)
    { do_obj(GOABS,GABSIDE1,SELECTED);
      undo_obj(GOABS,GABSIDE2,SELECTED);
    }
  else
    { do_obj(GOABS,GABSIDE2,SELECTED);
      undo_obj(GOABS,GABSIDE1,SELECTED);
    }
      
  out_form(GOABS);
  do {
   exit_obj = (form_do(form_addr,GABTRACK) & 0x7fff);
   undo_obj(GOABS,exit_obj,SELECTED);
  if (exit_obj == GABOK)
    { temptrak = atoui(intstr1); 
      tempsec = atoui(intstr2);
      if ((temptrak < 0) || (temptrak > 79) || (tempsec == 0))
         { form_alert(1,
       "[3][ Track or Sector not | correctly set. | Please try again ][ OK ]");
	   exit_obj = 0;
           objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh);
	 }
       else
         { track = temptrak;
	   sector = tempsec;
	   side = (get_bit(GOABS,GABSIDE1,SELECTED) == 0);
	   logsec = complog(track,sector,side);
         }
       }
      }
    while ((exit_obj != GABCAN) && (exit_obj != GABOK));
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
    if (exit_obj == GABOK)
      do_rw();
}


/************************************************************************/
/* gotooff goes to the offset specified					*/
/************************************************************************/
gotooff()
{ int direction;
  register unsigned int k,j;
  register int	l;

  set_text(GOFF,GOFFDEC,gofstr,FALSE);
  set_tlen(GOFF,GOFFDEC,6);

  direction = 1;
  bset_char(GOFF,GOFFPLUS,'\010',FALSE);
  bset_char(GOFF,GOFFMINU,' ',FALSE);
  out_form(GOFF);
  do {
    exit_obj = (form_do(form_addr,GOFFDEC) & 0x7fff);
    switch (exit_obj) {
      case GOFFPLUS:
    	bset_char(GOFF,GOFFPLUS,'\010',TRUE);
      	bset_char(GOFF,GOFFMINU,' ',TRUE);
      	direction = TRUE;
	error = TRUE;
	break;
      case GOFFMINU:
    	bset_char(GOFF,GOFFMINU,'\010',TRUE);
      	bset_char(GOFF,GOFFPLUS,' ',TRUE);
      	direction = FALSE;
	error = TRUE;
	break;
      default:
  	error = FALSE;
	switch (curedit)
	  { case FILEEDIT:
	      j = atoui(gofstr);
	      if (direction)
	        l = filesec+j;
	      else
	        l = filesec-j;
	      if ((l >= 0) && (l < (fnumcl*2)))
	        { logsec = dirfiles->fsclust;
		  filesec = 2;
	          for (i=0;i<=l;i++)
		    { if ((filesec & 1) == 0)
		        logsec = get_fatword(logsec);
		      filesec += 1;
	            }
		  fileclust = logsec;
	          logsec = ((fileclust-2)*2)+bpbptr->datrec+(filesec & 1);
		  readok = TRUE;
	          do_rw();
	          exit_obj = GOFFCAN;
	        }
	      error = TRUE;
	      break;
	    case UNEREDIT:
	      break;
          }
  	if (!error)
    	  { j = atoui(gofstr);
	    k = logsec;
 
	      if (direction)
	        { if ((k+j) > ((bpbptr->numcl*2)+bpbptr->datrec))
	            error = TRUE;
	          else
	            logsec += j;
	         }
	       else
	        { if (j > k)
	            error = TRUE;
	          else
	            logsec -= j;
	        }
 	    }
  	    undo_obj(GOFF,exit_obj,SELECTED);
  	    if ((exit_obj != GOFFCAN) && (error == TRUE))
    	      { form_alert(1,
  	    "[1][ Offset sector is | out of range. | Please try again. ][ OK ]");
      	        objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh);
	      }
            break;
          }
  }
  while ((error == TRUE) && (exit_obj != GOFFCAN));
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  if (exit_obj != GOFFCAN)
    do_rw();
}

/************************************************************************/
/* gotorel goes to the relative sector specified			*/
/************************************************************************/
gotorel()
{ register int i,error;
  register unsigned int j;

  set_text(GRELA,GRELDEC,grelstr,FALSE,6);
  set_tlen(GRELA,GRELDEC,6);

  out_form(GRELA);
  
  do {
  exit_obj = (form_do(form_addr,GRELDEC) & 0x7fff);
  undo_obj(GRELA,exit_obj,SELECTED);

  error = FALSE;
  switch (curedit)
    { case FILEEDIT:
        j = atoui(grelstr);
        if (j < (fnumcl*2))
          { logsec = dirfiles->fsclust;
	    filesec = 2;
            for (i=0;i<=j;i++)
	      { if ((filesec & 1) == 0)
	          logsec = get_fatword(logsec);
	        filesec += 1;
	      }
	    fileclust = logsec;
            logsec = ((fileclust-2)*2)+bpbptr->datrec+(filesec & 1);
	    readok = TRUE;
            do_rw();
            exit_obj = GRELCAN;
          }
        error = TRUE;
	break;
      case UNEREDIT:
	break;
     }

  if (!error)
    logsec = atoui(grelstr); 

  if (logsec > ((bpbptr->numcl*2)+bpbptr->datrec))
    error = TRUE;
  else
    comptss(logsec);
  
  if ((exit_obj != GRELCAN) && (error == TRUE))
    { form_alert(1,
  "[1][ Relative sector is | out of range. | Please try again. ][ OK ]");
      objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh);
    }    
  }
  while ((error == TRUE) && (exit_obj != GRELCAN));

  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);

  if (exit_obj != GRELCAN)
    { readok = TRUE;
      do_rw();
    }
}

/************************************************************************/
/* comptss computes the track,sector,side from logical sector		*/
/************************************************************************/
comptss(inlogical)
unsigned int inlogical;

{ register unsigned int j;

   j = (globnsecs * globnsides);
   track = (inlogical / j);
   sector = ((inlogical % j) % globnsecs) + 1;
   side = (inlogical % j) / globnsecs;
}

/************************************************************************/
/* complog computes the logical sector from track,sector,side		*/
/************************************************************************/
int complog(intr,insec,inside)
int intr,insec,inside;

{ register unsigned int j;
  
  j = (globnsecs * globnsides);
  j = intr * j;
  j += side * globnsecs;
  j += sector - 1;
  
  return(j);
}

/************************************************************************/
/* atoui takes an ascii string and turns into unsigned integer		*/
/************************************************************************/
int atoui(instr)

char instr[];

{ unsigned register int j,k;
   register int i;

   strip_spc(instr);
   i = strlen(instr) - 1;
   for (j = 1,k = 0;i>=0;i--,j *= 10)
     k += (instr[i] - '0') * j;
   
   return(k);
}

/************************************************************************/
/* do_vslide takes msgbuff[4] and does the slide action			*/
/************************************************************************/
do_vslide()
  { unsigned register int i;

     if (editon)
       { i = msgbuff[4];
	 i *= (((bpbptr->numcl*2)+bpbptr->datrec)/1000)+1;
	 if (i > ((bpbptr->numcl*2)+bpbptr->datrec))
	   i = ((bpbptr->numcl*2)+bpbptr->datrec);
	 logsec = i;
	 wind_set(wi_handle,WF_VSLIDE,msgbuff[4],0,0,0);
	 do_rw();
      }
}

/************************************************************************/
/* editoff turns off the edit menu and turns on the main menu		*/
/************************************************************************/
editoff()
{
  menu_bar(gl_menu,FALSE);		/* turn off edit menu */
  rsrc_gaddr(R_TREE,MAINMENU,&gl_menu); /* get address of menu */
  menu_bar(gl_menu,TRUE);		/* turn on menu bar */
  menuedit = FALSE;			/* out of edit now */
  editon = FALSE;
  wind_set(wi_handle,WF_NAME," STTOOLS  ver 1.0 ",0,0);
  if (scrtype == 0)
    docursor(xcurs,ycurs);
  hide_mouse();
  clr_window();
  show_mouse();
  graf_mouse(ARROW,0x0L);
}
