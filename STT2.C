/*********************************************************************/
/* DEFINES						   	     */
/*********************************************************************/

#define WI_KIND (NAME | UPARROW | DNARROW | VSLIDE)
#define SOURCE 0
#define DESTINATION 1
#define MEMERR 2
#define RESTERR 3
#define DIERR 4
#define	LOADABLE	/* for loadable resource file */

/*********************************************************************/
/* INCLUDE FILES						     */
/*********************************************************************/

#include "obdefs.h"
#include "define.h"
#include "gemdefs.h"
#include "osbind.h"
#ifndef LOADABLE
#include "portab.h"
#include "\sttools\stools.rsh"
#endif
#include "\sttools\stools.h"


/*********************************************************************/
/* EXTERNALS						   	     */
/*********************************************************************/

#include "stext.h"		/* external definitions */
extern int curobj;

/*********************************************************************/
/* globals for this module					     */
/*********************************************************************/

int	docopy(),docompare();
char	verstr1[100] = "[1][ Verify Error on ";
char	verstr2[] = "  ][Continue|Cancel]";
OBJECT  *form_obj;
char	mapstr1[40] = "Map represents 1/";
char	mapstr2[] = " of total disk";
char	mhedstr[] = "     Map of Drive A:";
char	compstr[80] = "[1][ Mismatch on ";
char	nofindstr[] = 
       "[1][ End of disk reached. | The search string was | not found!][ OK ]";
char	memerr[] = "[3][ Insufficient memory | for this operation!][Abort]";
char	trackstr[3];
char	norecov[] = 
"[3][ Unable to recover | error(s) on track 00.][Abort|Continue]";
char	errtrack[] = "   on track 00.";
char	ntrstr[3];
char	nsecstr[3];
char	fat1str[] = "1st FAT area sector: 999,999";
char	fat2str[] = "2nd FAT area sector: 999,999";
char	dat1str[] = "Data area 1st sector: 999,999";
char	rootstr[] = "Root directory sector: 999,999";
char	nsdstr[] = "Number of sectors on drive: 999,999";
char	dbhstr[] = "Information for drive A:";

/************************************************************************/
/* diskmapping maps a disk						*/
/************************************************************************/
diskmapping()
{ register int percent,i,j,k;
  int	diskcls,fatval;
  char tempstr[4];

  diskinfo(defdrive);
  mhedstr[18] = 'A'+defdrive;
  set_text(MAPPING,MAPHEAD,mhedstr,FALSE);
  fatsecs = fatbuff;

  mapstr1[17] = '\0';
  diskcls = bpbptr->numcl+(bpbptr->datrec/2);
  percent = diskcls;
  percent /= 360;
  if (percent < 1)
    percent = 1;
  uitoa(percent,tempstr);
  strcat(mapstr1,tempstr);
  strcat(mapstr1,mapstr2);
  set_text(MAPPING,MAPINFO,mapstr1,FALSE);

  curdrive = defdrive;

  for (i=0;i<792;i++)
    diskbuff[i] = ' ';    

  graf_mouse(HOURGLASS,0x0L);
  if (bpbptr->fsiz <= 6)
    error = myrwabs(0,fatsecs,bpbptr->fsiz,bpbptr->fatrec,curdrive);
  if (dir_error())
    return;

  j = diskcls;
  if (diskcls > 360)
    diskcls = 360;
  for (i=0;i<j;i++)
    { fatval = get_fatword(i);
      k = ((i/percent)*2);	/* fat pointer */
      k += (i/(5*percent)); /* plus spaces */
      if (i < ((bpbptr->fatrec+bpbptr->fsiz)*2))
	{ diskbuff[k] = 'F';
	  diskbuff[k+1] = 'F';
	}
      else if (i < bpbptr->datrec-bpbptr->rdlen)
	{ diskbuff[k] = 'D';
	  diskbuff[k+1] = 'D';
	}
      else if (fatval == 0)
	{ diskbuff[k] = 'A';
	  diskbuff[k+1] = 'A';
	}
      else if ((fatval & 0xfff) == 0xff7)
	{ diskbuff[k] = 'B';
	  diskbuff[k+1] = 'B';
	}
      else if (((fatval & 0xfff) >= 0xff0) && ((fatval & 0xfff) <= 0xff6))
	{ diskbuff[k] = 'R';
	  diskbuff[k+1] = 'R';
	}
      else
	{ diskbuff[k] = 'U';
	  diskbuff[k+1] = 'U';
	}
    }
  
  graf_mouse(ARROW,0x0L);

  out_form(MAPPING);
  hide_mouse();
  for(i=0;i<792;i++)
    outchar((i % 66)+7,(i / 66)+7,diskbuff[i]);

  show_mouse();
  exit_obj = (form_do(form_addr,0) & 0x7fff);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  undo_obj(MAPPING,exit_obj,SELECTED);
}

/************************************************************************/
/* diskformat formats a disk						*/
/************************************************************************/
diskformat()

{ set_otext(FORMAT,FORMSTR1,"Format a disk");
  set_otext(FORMAT,FORMOK,"Format");

  fdiskout(&cformat,
             "[1][ Error while formatting. | Disk may be unusable. ][ OK ]",
	    TRUE);
}

/************************************************************************/
/* fdiskout puts out the format disk form				*/
/************************************************************************/
fdiskout(inrtn,errstr,flag)

int (*inrtn)();
char *errstr;
int flag;

{ int dblon;
  register int i,error;

  out_form(FORMAT);
  rsrc_gaddr(R_TREE,FORMAT,&form_obj);
  sel_obj = form_obj+FORMBAR;
  x = (form_obj->ob_x)+(sel_obj->ob_x+1);
  y = (form_obj->ob_y)+(sel_obj->ob_y);
  h = sel_obj->ob_height-1;
  h *= rez;
  vswr_mode(handle,1);
  vsl_type(handle,5);
  vsl_width(handle,1);
  vsl_color(handle,1);
  do {
    exit_obj = (form_do(form_addr,0) & 0x7fff);
    graf_mouse(HOURGLASS,0x0L);
    undo_obj(FORMAT,exit_obj,SELECTED);
    dblon = get_bit(FORMAT,FORMDBL,SELECTED);
    if (exit_obj != FORMCAN)
      { if (!restdr[defdrive])
	  { crest(defdrive);
	    restdr[defdrive] = TRUE;
          }
      for (i=0;i<80;i++)
        { error = (* inrtn)(i,0,defdrive);
          if ((dblon) && (!error))
            error = (*inrtn)(i,1,defdrive);
          if (error)
            { form_alert(1,errstr);
	      i = 80;  
            }
	  hide_mouse();
	  pxyarray[0] = x+(i*3);
	  pxyarray[1] = y;
	  pxyarray[2] = x+(i*3);
	  pxyarray[3] = y+h;
	  v_pline(handle,2,pxyarray);
	  show_mouse();
         }
        if ((!error) && (flag == TRUE));
	  { Protobt(&diskbuff,0x10000000,2+(dblon != 0),0);
	    error = myrwabs(3,&diskbuff,1,0,defdrive);
	    if (!error)
	      { for (i=0;i<2560;i++)
		  diskbuff[i] = 0;
		diskbuff[0] = 0xf7;
		diskbuff[1] = 0xff;
		diskbuff[2] = 0xff;
		error = myrwabs(3,&diskbuff,5,1,defdrive);
		if (!error)
		  error = myrwabs(3,&diskbuff,5,6,defdrive);
	      }
	  }
        objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh); /* draw it */
       }
      graf_mouse(ARROW,0x0L);
     }
   while (exit_obj != FORMCAN);
   form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* diskcompare compares 2 disks						*/
/************************************************************************/
diskcompare()
{ register int i,error;

  set_otext(COMPDISK,COMPSTR,"Compare 2 disks");
  set_otext(COMPDISK,COMPOK,"Compare");

  compformout(&docompare);
}

/************************************************************************/
/* disksearch searches disk for a string				*/
/************************************************************************/
disksearch(startoff)
long startoff;
{ long totmem,retval;
  char *memptr;
  register int i,j,k,l;

  logsec = 0;
  diskinfo(defdrive);
  totmem = Malloc(-1L);
  i = totmem / 512;		/* number of sectors to search */
  totmem = (long)(i) * 512;
  if (totmem == 0)
    { form_alert(1,memerr);
      return;
    }
  memptr = Malloc(totmem);	/* we now have an address */
  j = ((bpbptr->numcl) * 2)+bpbptr->datrec;
  if (j < i)
    i = j;

  k = startoff/512;
  l = startoff % 512;
  readok = FALSE;
  while (k<j)
    { if ((k+i) > j)
	i = (j-k);
      error = myrwabs(2,memptr,i,k,defdrive);
      if (error)
	return;
      retval = qsearch(&memptr[l],seastr,totmem,strlen(seastr));
      retval += l;
      if (retval != -1)
        { logsec = k+(retval / 512);
	  i = retval % 512;
	  doxy(i);
	  k = j;
	  i = 0;
	  readok = TRUE;
      	}
      k += i;
      l = 0;
    }
  Mfree(memptr);
  graf_mouse(ARROW,0x0L);
  if (readok == FALSE)
    form_alert(1,nofindstr);
  else
    { menu_bar(gl_menu,FALSE);		/* turn off main menu */
      do_obj(EDITMENU,EMEN4S6,CHECKED);
      undo_obj(EDITMENU,EMEN4S7,CHECKED);
      rsrc_gaddr(R_TREE,EDITMENU,&gl_menu);
      menu_bar(gl_menu,TRUE);
      menuedit = TRUE;
      scrtype = 0;
      readon = TRUE;
      clr_window();
      docursor(xcurs,ycurs);
      editon = TRUE;
      curedit = SEAREDIT;
      do_rw();
      show_mouse();
    }
}

/************************************************************************/
/* diskerase erases a disk						*/
/************************************************************************/
diskerase()
{ 
  set_otext(FORMAT,FORMSTR1,"Erase a disk");
  set_otext(FORMAT,FORMOK,"Erase");

  fdiskout(&cerase,
            "[3][ Error while erasing. | Disk may be unusable. ][ OK ]",
	    FALSE);
}

/************************************************************************/
/* diskverify verifies a disk						*/
/************************************************************************/
diskverify()
{ int exit_but;
  register unsigned int i,j,error;
  char	tempstr[6];

  
  out_form(VERIFY);
  rsrc_gaddr(R_TREE,VERIFY,&form_obj);
  sel_obj = form_obj+VERIBAR;
  x = (form_obj->ob_x)+(sel_obj->ob_x+1);
  y = (form_obj->ob_y)+(sel_obj->ob_y);
  h = sel_obj->ob_height-1;
  h *= rez;
  vswr_mode(handle,1);
  vsl_type(handle,5);
  vsl_width(handle,1);
  vsl_color(handle,1);
  do {
    exit_obj = (form_do(form_addr,0) & 0x7fff);
    graf_mouse(HOURGLASS,0x0L);
    undo_obj(VERIFY,exit_obj,SELECTED);
    if (exit_obj != VERICAN)
      { graf_mouse(HOURGLASS,0x0L);
	diskinfo(defdrive);
        j = bpbptr->numcl+bpbptr->datrec;
	for (i=0;i<j;i++)
          { error = myrwabs(2,diskbuff,1,i,defdrive);
	    if (error)
	      { verstr1[20] = '\0';
		strcat(verstr1,"| Logical Sector: ");
		uitoa(i,tempstr);
		strcat(verstr1,tempstr);
		strcat(verstr1,verstr2);
		if (defdrive < 2)
	          { comptss(i);
		    verstr1[20] = '\0';
		    strcat(verstr1,"| Track: ");
		    uitoa(track,tempstr);
		    strcat(verstr1,tempstr);
		    strcat(verstr1,"| Sector: ");
		    uitoa(sector,tempstr);
		    strcat(verstr1,tempstr);
		    strcat(verstr1,"| Side: 1");
		    verstr1[46] = '1'+side;
		    strcat(verstr1,verstr2);
		  }
		graf_mouse(ARROW,0x0L);
		exit_but = form_alert(1,verstr1);
		graf_mouse(HOURGLASS,0x0L);
	        if (exit_but == 2)
                  i = j;
              }
	    if (!(i % (j / 80)))
	      { hide_mouse();
	  	pxyarray[0] = x+((i/(j/80))*3);
	  	pxyarray[1] = y;
	  	pxyarray[2] = x+((i/(j/80))*3);
	  	pxyarray[3] = y+h;
	  	v_pline(handle,2,pxyarray);
	  	show_mouse();
              }
           }
         objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh); 
        }
      graf_mouse(ARROW,0x0L);
     }
   while (exit_obj != VERICAN);
   form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* diskcopy copies a disk						*/
/************************************************************************/
diskcopy()
{ 
  set_otext(COMPDISK,COMPSTR,"  Copy a disk");
  set_otext(COMPDISK,COMPOK,"Copy");

  compformout(&docopy);
}

/************************************************************************/
/* disklabel relabels a disk						*/
/************************************************************************/
disklabel()
{ register int i,j,k;
  int wrent,wrsec,dum;
  long longmem;

  diskinfo(defdrive);
  curdrive = defdrive;
  isroot = TRUE;

  longmem = (long)(Malloc(-1L));
  maxdirs = longmem / 32;
  dfptr = (long)Malloc(longmem);	/* allocation of directories */
  if (dfptr == 0)
    { graf_mouse(ARROW,0x0L);
      form_alert(1,"[3][ Insufficient memory | to find disk label. ][Abort]");
      Mfree(dfptr);
      return(1);		/* return cancel button */
    }

  i = do_ffind("*.*",8,&wrsec,&wrent,&dum,TRUE);
  if (i == -1)
    { graf_mouse(ARROW,0x0L);
      Mfree(dfptr);
      return;
    }
  if (i == 2)
    { graf_mouse(ARROW,0x0L);
      form_alert(1,
   "[3][ Insufficient room | in main directory | to change label. ][Abort]");
      Mfree(dfptr);
      return;
    }

  dirfiles->fattr = 0;

  strip_spc(dirfiles->fname);

  set_text(DISKLAB,DLABSTR,dirfiles->fname,FALSE);
  set_tlen(DISKLAB,DLABSTR,12);

  graf_mouse(ARROW,0x0L);
  out_form(DISKLAB);
  j = (form_do(form_addr,0) & 0x7fff);
  graf_mouse(HOURGLASS,0x0L);  
  undo_obj(DISKLAB,j,SELECTED);
  dirfiles->fattr |= 0x8;		/* set as directory entry */
  for (i=0;i<12;i++)
    if (dirfiles->fname[i] == '\0')
      dirfiles->fname[i] = ' ';
  
  if (j == DLABOK)
    { error = myrwabs(3,&dfptr[wrent/16],1,
		(bpbptr->datrec-bpbptr->rdlen)+wrsec,defdrive);
      if (error)
         { graf_mouse(ARROW,0x0L);
	   form_alert(1,
   "[3][ Write error on disk! | Unable to change | disk label . ][Abort]");
	 }
    }

  Mfree(dfptr);
  graf_mouse(ARROW,0x0L);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* diskrecover recovers a disk error(s)					*/
/************************************************************************/
diskrecover()
{ int trnum,singtrak;
  register int i,j,k;
  long	memloc;
  
  diskinfo(defdrive);
  graf_mouse(ARROW,0x0L);

  trackstr[0] = '\0';
  
  singtrak = FALSE;
  bset_char(DISKERR,REDISK,'\10',FALSE);
  bset_char(DISKERR,RETRACK,' ',FALSE);

  out_form(DISKERR);
  do {
    exit_obj = (form_do(form_addr,0) & 0x7fff);
    switch (exit_obj)
      { case REDISK:
	  do_obj(DISKERR,RETRNUM,DISABLED);
	  trackstr[0] = '\0';
	  set_text(DISKERR,RETRNUM,trackstr,FALSE);
	  set_tlen(DISKERR,RETRNUM,1);
	  objc_draw(form_addr,RETRNUM,0,fo_gx,fo_gy,fo_gw,fo_gh); 
	  bset_char(DISKERR,REDISK,'\10',TRUE);
	  bset_char(DISKERR,RETRACK,' ',TRUE);
	  singtrak = FALSE;
	  break;
	case RETRACK:
	  undo_obj(DISKERR,RETRNUM,DISABLED);
	  set_text(DISKERR,RETRNUM,trackstr,FALSE);
	  set_tlen(DISKERR,RETRNUM,3);
	  objc_draw(form_addr,RETRNUM,0,fo_gx,fo_gy,fo_gw,fo_gh); 
	  bset_char(DISKERR,REDISK,' ',TRUE);
	  bset_char(DISKERR,RETRACK,'\10',TRUE);
	  singtrak = TRUE;
	  break;
      }
    }
   while ((exit_obj != REOK) && (exit_obj != RECAN));
   undo_obj(DISKERR,exit_obj,SELECTED);
   if (singtrak == TRUE)
     trnum = atoui(trackstr)+1;
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  
  if ((trnum > 80) && (singtrak == TRUE))
    { form_alert(1,"[3][ Track is out | of range!][ Abort ]");
      return;
    }
  if (exit_obj == REOK)
    { set_text(SHOWFILE,SFHEAD,"Searching for errors:",FALSE);
      set_text(SHOWFILE,SFFILE1," ",FALSE);
      set_text(SHOWFILE,SFFILE2," ",FALSE);	
      out_form(SHOWFILE);
      form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,
			fo_gy,fo_gw,fo_gh);
      k = 0;
      if (singtrak == TRUE)
        k = trnum;
      else
	trnum = 80;
     	
      memloc = Malloc(9216L);
      if (memloc == 0)
	{ form_alert(1,memerr);
     	  return;
    	}
      for (i=k;i<trnum;i++)
	{ uitoa(i,trackstr);
	  errtrack[14] = '\0';
	  strcat(errtrack,trackstr);
	  strcat(errtrack,".");
	  set_text(SHOWFILE,SFFILE1,errtrack,TRUE);
	  if ((defdrive == 1) && (num_flops == 0))
	    defdrive = 0;
	  error = myrwabs(2,memloc,globnsecs,0,defdrive);
	  if (error)
	    { set_text(SHOWFILE,SFHEAD,"  Recovering errors: ",TRUE);
	      cformat(i,0,defdrive);
	      error = myrwabs(3,memloc,globnsecs,0,defdrive);
	      if (error)
		{ norecov[43] = '\0';
		  uitoa(i,trackstr);
		  strcat(norecov,trackstr);
		  strcat(norecov,".][Abort|Continue]");
		  if (form_alert(1,norecov) == 1)
		    { Mfree(memloc);
		      return;
		    }
	          set_text(SHOWFILE,SFHEAD,"Searching for errors:",TRUE);
		}
      	    }
	  if (globnsides != 1)
	    { error = myrwabs(3,memloc,globnsecs,0,defdrive);
	      if (error)
	    	{ set_text(SHOWFILE,SFHEAD,"  Recovering errors: ",TRUE);
	      	  cformat(i,1,defdrive);
	      	  error = myrwabs(3,memloc,globnsecs,1,defdrive);
	      	  if (error)
		    { norecov[42] = '\0';
		      uitoa(i,trackstr);
		      strcat(norecov,trackstr);
		      strcat(norecov,".][Abort|Continue]");
		      if (form_alert(1,norecov) == 1)
		        { Mfree(memloc);
			  return;
			}
	              set_text(SHOWFILE,SFHEAD,"Searching for errors:",TRUE);
		    }
      	        }
	    }
        }
    }
  Mfree(memloc);
}

/************************************************************************/
/* diskboot gets information for the boot sector and/or sets it		*/
/************************************************************************/
diskboot()
{ register int i;
  register long temp;
  char tempstr[8];
  
  dbhstr[22] = 'A'+defdrive;
  set_text(DBINFO,DBIHEAD,dbhstr);

  temp = bpbptr->fatrec-bpbptr->fsiz;
  ltoa(temp,tempstr);
  reverse(tempstr);
  fat1str[21] = '\0';
  strcat(fat1str,tempstr);
  set_text(DBINFO,FAT1SEC,fat1str);

  temp = bpbptr->fatrec;
  ltoa(temp,tempstr);
  reverse(tempstr);
  fat2str[21] = '\0';
  strcat(fat2str,tempstr);
  set_text(DBINFO,FAT2SEC,fat2str);

  temp = bpbptr->datrec;
  ltoa(temp,tempstr);
  reverse(tempstr);
  dat1str[22] = '\0';
  strcat(dat1str,tempstr);
  set_text(DBINFO,DAFSEC,dat1str);

  temp = bpbptr->fatrec+bpbptr->fsiz;
  ltoa(temp,tempstr);
  reverse(tempstr);
  rootstr[23] = '\0';
  strcat(rootstr,tempstr);
  set_text(DBINFO,RDSEC,rootstr);

  temp = bpbptr->datrec+(bpbptr->numcl*2);
  ltoa(temp,tempstr);
  reverse(tempstr);
  nsdstr[28] = '\0';
  strcat(nsdstr,tempstr);
  set_text(DBINFO,NDSECS,nsdstr);

  nsecstr[0] = '\0';
  ntrstr[0] = '\0';
  set_text(DBINFO,DBFNOT,ntrstr);
  set_text(DBINFO,DBFNSPT,nsecstr);
  set_tlen(DBINFO,DBFNOT,1);
  set_tlen(DBINFO,DBFNSPT,1);
  undo_obj(DBINFO,DBIOK,DISABLED);
  if (globnsides != 1)
    { undo_obj(DBINFO,FNSIDES1,SELECTED);
      do_obj(DBINFO,FNSIDES2,SELECTED);
    }
  else
    { do_obj(DBINFO,FNSIDES1,SELECTED);
      undo_obj(DBINFO,FNSIDES2,SELECTED);
    }
  if (defdrive < 2)
    { for (i=DBFHEAD;i<DBIHEAD;i++)
	undo_obj(DBINFO,i,DISABLED);
      uitoa(globntracks,ntrstr);
      uitoa(globnsecs,nsecstr);
      set_tlen(DBINFO,DBFNOT,3);
      set_tlen(DBINFO,DBFNSPT,3);
      if (globnsides == 1)
	{ do_obj(DBINFO,FNSIDES1,SELECTED);
	  undo_obj(DBINFO,FNSIDES2,SELECTED);
	}
      else
	{ do_obj(DBINFO,FNSIDES1,SELECTED);
	  undo_obj(DBINFO,FNSIDES2,SELECTED);
	}
      do_obj(DBINFO,DBIOK,DISABLED);
      i = DBFNOT;
    }
  else
    { for (i=DBFHEAD;i<DBIHEAD;i++)
	do_obj(DBINFO,i,DISABLED);
      i = 0;
    }
  out_form(DBINFO);
  exit_obj = (form_do(form_addr,i) & 0x7fff);
  undo_obj(DBINFO,exit_obj,SELECTED);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  if ((exit_obj == DBFCAN) || (exit_obj == DBIOK))
    return;
  i = atoui(ntrstr);
  if (i != 0)
    globntracks = i;
  i = atoui(nsecstr);
  if (i != 0)
    globnsecs = i;
  globnsides = 1;
  if (get_bit(DBINFO,FNSIDES2,SELECTED) == SELECTED)
    globnsides = 2;
  i2u(globnsides,diskbuff+26);
  i2u(globnsecs,diskbuff+24);
  i = globnsides * globnsecs;
  i = globntracks * i;
  i2u(i,diskbuff+19);
  bpbptr->numcl = (i-bpbptr->datrec) / bpbptr->clsiz;
  if (exit_obj == DBFCHANG)
    myrwabs(3,diskbuff,1,0,defdrive);
}

/************************************************************************/
/* i2u changes an 68000 flavored integer to an 8086 integer		*/
/************************************************************************/
i2u(innum,outnum)
unsigned int innum;
char *outnum;
{  *outnum = innum;
   *(outnum+1) = (innum >> 8);
}

/************************************************************************/
/* set_otext set the text of an object					*/
/************************************************************************/
set_otext(tr_num,tr_obj,str_in)

int	tr_num,tr_obj;
char	*str_in;

{ char	*str_ptr;
  int	temp_len,k;

  rsrc_gaddr(R_TREE,tr_num,&sel_obj);
  sel_obj = sel_obj + tr_obj;
  sel_obj->ob_spec = str_in;
}

/************************************************************************/
/* docopy copies disk sectors						*/
/************************************************************************/
int	docopy()

{ long  memloc,memret;
  int 	sing_sec,error,maxtracks;
  register int i,j,k,l;


  diskinfo();
  sing_sec = ((globnsides+1) * 4608L);
  memloc = Malloc(-1L);
  if (memloc < sing_sec)
    return(MEMERR);
  maxtracks = memloc / sing_sec;

  rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
  sel_obj = form_obj+COMPBAR1;
  
  x = (form_obj->ob_x)+(sel_obj->ob_x+1);
  y = (form_obj->ob_y)+(sel_obj->ob_y);
  h = sel_obj->ob_height-1;
  h *= rez;

  memret = memloc = Malloc((long)(maxtracks * sing_sec));

  i = 0;
  j = 0;
  k = 0;
  l = 0;
   do
    { do 
        { error = myrwabs(2,memloc,i,k,orginal);
	  if (error)
	    { Mfree(memret);
	      return(DIERR);
	    }
          memloc += 4608L;
          k += 1;
	 }
       while (k<=globnsides);
       k = 0;

       hide_mouse();
       pxyarray[0] = x+(i*3);
       pxyarray[1] = y;
       pxyarray[2] = x+(i*3);
       pxyarray[3] = y+h;
       v_pline(handle,2,pxyarray);
       show_mouse();

     if (!( (i+1) % maxtracks) || (i==globntracks))
       { rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
      	 sel_obj = form_obj+COMPBAR2;
	 x = (form_obj->ob_x)+(sel_obj->ob_x+1);
	 y = (form_obj->ob_y)+(sel_obj->ob_y);
	 h = sel_obj->ob_height-1;
	 h *= 2;

	 rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
      	 sel_obj = form_obj+COMPBAR2;
      	 x = (form_obj->ob_x)+(sel_obj->ob_x+1);
      	 y = (form_obj->ob_y)+(sel_obj->ob_y);
      	 h = sel_obj->ob_height-1;
	 h *= rez;

	 memloc = memret;
	 do
           { do 
	       {  error = myrwabs(3,memloc,i,k,duplicate);
	          if (error)
	            { Mfree(memret);
		      return(DIERR);
		    }
	          memloc += 4608L;
		  l += 1;
		}
	      while (l <= globnsides);
	     l = 0;
	     hide_mouse();
	     pxyarray[0] = x+(j*3);
	     pxyarray[1] = y;
	     pxyarray[2] = x+(j*3);
	     pxyarray[3] = y+h;
	     v_pline(handle,2,pxyarray);
	     show_mouse();
	     j += 1;
	   }
	 while (j<=i);
	 memloc = memret;
	 rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
  	 sel_obj = form_obj+COMPBAR1;
  	 x = (form_obj->ob_x)+(sel_obj->ob_x+1);
  	 y = (form_obj->ob_y)+(sel_obj->ob_y);
  	 h = sel_obj->ob_height-1;
	 h *= rez;
	}
       i += 1;
     }
  while (i<=globntracks);
  Mfree(memret);
  return(0);
}

/************************************************************************/
/* ins_disk tells the user to insert a disk in the drive		*/
/************************************************************************/
ins_disk(disk_num)

int	disk_num;

{ int	dev1_bit,dev2_bit;
  char	dev_str[3];
  
  if (orginal == duplicate)
    { dev_str[0] = 'A'+ orginal;
      dev_str[1] = ':';
      dev_str[2] = 0;
      ins_str[13] = 0;
      if (disk_num)
        strcat(ins_str,"Duplicate disk |into Drive ");
      else
        strcat(ins_str,"Original disk|into Drive ");
      strcat(ins_str,&dev_str);
      strcat(ins_str,"][ OK | Abort ]");
      return(form_alert(1,&ins_str));
    }
}

/************************************************************************/
/* docompare compares two different disks				*/
/************************************************************************/
int docompare()
{ long  memloc,memret,mem1,mem2,nbytes;
  int 	sing_sec,error,maxtracks,retval,exit_but,temp;
  register int i,j,k,l;
  char tempstr[4];

  diskinfo();

  sing_sec = ((globnsides+1) * 9216);
  memloc = Malloc(-1L);
  if (memloc < sing_sec)
    return(MEMERR);
  maxtracks = memloc / sing_sec;

  rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
  sel_obj = form_obj+COMPBAR1;
  
  x = (form_obj->ob_x)+(sel_obj->ob_x+1);
  y = (form_obj->ob_y)+(sel_obj->ob_y);
  h = sel_obj->ob_height-1;
  h *= rez;

  memret = memloc = Malloc((long)(maxtracks * sing_sec));

  i = 0;
  j = 0;
  k = 0;
  l = 0;
   do
    { do 
        { error = myrwabs(2,memloc,i,k,orginal);
	  if (error)
	    { Mfree(memret);
	      return(DIERR);
	    }
          memloc += 4608L;
          k += 1;
	 }
       while (k<=globnsides);
       k = 0;

       hide_mouse();
       pxyarray[0] = x+(i*3);
       pxyarray[1] = y;
       pxyarray[2] = x+(i*3);
       pxyarray[3] = y+h;
       v_pline(handle,2,pxyarray);
       show_mouse();

     if (!( (i+1) % maxtracks) || (i==globntracks))
       { rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
      	 sel_obj = form_obj+COMPBAR2;
	 x = (form_obj->ob_x)+(sel_obj->ob_x+1);
	 y = (form_obj->ob_y)+(sel_obj->ob_y);
	 h = sel_obj->ob_height-1;
	 h *= 2;

	 rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
      	 sel_obj = form_obj+COMPBAR2;
      	 x = (form_obj->ob_x)+(sel_obj->ob_x+1);
      	 y = (form_obj->ob_y)+(sel_obj->ob_y);
      	 h = sel_obj->ob_height-1;
	 h *= rez;

	 memloc = memret+(maxtracks * 4608L);
	 do
           { do 
	       {  error = myrwabs(2,memloc,i,k,duplicate);
	          if (error)
	            { Mfree(memret);
		      return(DIERR);
		    }
		  mem1 = memloc-(maxtracks * 4608L);
		  mem2 = memloc;
		  nbytes = 4608L;
		  do {
		    retval = qcompare(mem1,mem2,nbytes);
		    if (retval != -1)
		      { retval += 1;
			temp = (mem2-memloc)+retval;
			mem1 += retval;
			mem2 += retval;
			nbytes -= retval;
			compstr[17] = '\0';
		        strcat(compstr,"| Track: ");
		        uitoa(j,tempstr);
		        strcat(compstr,tempstr);
		        strcat(compstr,"| Sector: ");
		        uitoa((temp/512)+1,tempstr);
		        strcat(compstr,tempstr);
		        if (l == 0)
		          strcat(compstr,"| Side: 1");
		        else
		          strcat(compstr,"| Side: 2");
		        strcat(compstr,"| Offset: ");
			temp -= 1;
		        uitoa((temp % 512),tempstr);
		        strcat(compstr,tempstr);
		        strcat(compstr,verstr2);
		        graf_mouse(ARROW,0x0L);
		        exit_but = form_alert(1,compstr);
		        graf_mouse(HOURGLASS,0x0L);
		        error = FALSE;
	                if (exit_but == 2)
                          { Mfree(memret);
			    return(0);
		          }
                      }
		    }
		  while (retval != -1);
	          memloc += 4608L;
		  l += 1;
		}
	      while (l <= globnsides);
	     l = 0;
	     hide_mouse();
	     pxyarray[0] = x+(j*3);
	     pxyarray[1] = y;
	     pxyarray[2] = x+(j*3);
	     pxyarray[3] = y+h;
	     v_pline(handle,2,pxyarray);
	     show_mouse();
	     j += 1;
	   }
	 while (j<=i);
	 memloc = memret;
	 rsrc_gaddr(R_TREE,COMPDISK,&form_obj);
  	 sel_obj = form_obj+COMPBAR1;
  	 x = (form_obj->ob_x)+(sel_obj->ob_x+1);
  	 y = (form_obj->ob_y)+(sel_obj->ob_y);
  	 h = sel_obj->ob_height-1;
	 h *= rez;
	}
       i += 1;
     }
  while (i<=globntracks);
  Mfree(memret);
  return(0);
}

/************************************************************************/
/* compformout() outputs a form to the screen like compare		*/
/************************************************************************/
compformout(inrtn)

int (*inrtn)();

{ register int i,error;

  if (defdrive == 0)
    { bset_char(COMPDISK,COMPODRA,'\10',FALSE);
      bset_char(COMPDISK,COMPODRB,' ',FALSE);
      bset_char(COMPDISK,COMPDDRA,' ',FALSE);
      bset_char(COMPDISK,COMPDDRB,'\10',FALSE);
      duplicate = 1;
      orginal = 0;
    }
  else
   { bset_char(COMPDISK,COMPODRA,' ',FALSE);
     bset_char(COMPDISK,COMPDDRA,'\10',FALSE);
     bset_char(COMPDISK,COMPODRB,'\10',FALSE);
     bset_char(COMPDISK,COMPDDRB,' ',FALSE);
     orginal = 1;
     duplicate = 0;
   }

  out_form(COMPDISK);
  vswr_mode(handle,1);
  vsl_type(handle,5);
  vsl_width(handle,1);
  vsl_color(handle,1);
  do {
    exit_obj = (form_do(form_addr,0) & 0x7fff);
    switch (exit_obj)
      {
        case COMPODRA:
	  bset_char(COMPDISK,COMPODRA,'\10',TRUE);
	  bset_char(COMPDISK,COMPODRB,' ',TRUE);
	  orginal = 0;
	  break;
	case COMPODRB:
	  bset_char(COMPDISK,COMPODRA,' ',TRUE);
	  bset_char(COMPDISK,COMPODRB,'\10',TRUE);
	  orginal = 1;
	  break;
	case COMPDDRA:
	  bset_char(COMPDISK,COMPDDRA,'\10',TRUE);
	  bset_char(COMPDISK,COMPDDRB,' ',TRUE);
	  duplicate = 0;
	  break;
	case COMPDDRB:
	  bset_char(COMPDISK,COMPDDRA,' ',TRUE);
	  bset_char(COMPDISK,COMPDDRB,'\10',TRUE);
	  duplicate = 1;
	  break;
       }
    undo_obj(COMPDISK,exit_obj,SELECTED);
    if (exit_obj == COMPOK)
      { diskinfo(defdrive);
	graf_mouse(HOURGLASS,0x0L);

	if (num_flops == 0)
    	  { duplicate = 0;
      	    orginal = 0;
    	  }

	error = (*inrtn)();
        objc_draw(form_addr,ROOT,MAX_DEPTH,fo_gx,fo_gy,fo_gw,fo_gh); 
        graf_mouse(ARROW,0x0L);
       }
     }
   while (exit_obj != COMPCAN);
   form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* doxy plots the xcurs,ycurs according to offset and what side		*/
/************************************************************************/
doxy(offset)
int offset;
{ register int i,k;

  i = offset;
  ycurs = (i/24)+4;
  k = i % 24;
  if (curobj == HSTRING1)
    xcurs = ((k * 2)+(k/8))+2;
  else
    xcurs = 53+k;
}
