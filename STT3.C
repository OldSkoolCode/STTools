/*********************************************************************/
/* DEFINES						   	     */
/*********************************************************************/

#define WI_KIND (NAME | UPARROW | DNARROW | VSLIDE)
#define SOURCE 0
#define DESTINATION 1
#define MEMERR 2
#define RESTERR 3

/*********************************************************************/
/* INCLUDE FILES						     */
/*********************************************************************/

#include "obdefs.h"
#include "define.h"
#include "gemdefs.h"
#include "osbind.h"
#include "\sttools\stools.h"

/*********************************************************************/
/* EXTERNALS						   	     */
/*********************************************************************/

#include "stext.h"			/* external defintions */

/************************************************************************/
/* global definitions for this module					*/
/************************************************************************/

char	tmpstr[3];
int	x1,y1,x2,y2;
int 	index,objindex;
OBJECT  *form_obj;
int 	curobj;
struct	dirfiles *fileptr;
int	savesdirs[200][2];	/* maximum directory levels == 100 */
int	printnames(),erasefiles(),mapfiles(),copyfiles();
int	counter,globmoveon;
int 	diskcls,percent,dum;
int	numsdirs,globclust;

struct	dirfiles onedir,*tempdir = onedir;
char	newfstr[] = "[3][Please insert disk B | into drive A: ][ Ok ]";
char	dierrstr[] = 
"[2][ Data on drive A: | might be damaged. | Please check disk drive.][ Abort | Retry ]";
char	wrprostr[] = 
"[2][ Disk in drive A: | is write protected. | Please correct and retry.][ Abort | Retry ]";

/************************************************************************/
/* getsearch gets the string and hexidecimal to search for		*/
/************************************************************************/
int getsearch()
{  int dum;
   register int i,j,k;
 
   rsrc_gaddr(R_TREE,SEARCH,&obj_ptr);
   obj_ptr = obj_ptr + SSTRING;
   txt_ptr = obj_ptr->ob_spec;
   txt_ptr->te_ptext = seastr;
   txt_ptr->te_txtlen = 21;
   rsrc_gaddr(R_TREE,SEARCH,&obj_ptr);
   obj_ptr = obj_ptr + HSTRING1;
   txt_ptr = obj_ptr->ob_spec;
   txt_ptr->te_ptext = seahex;
   txt_ptr->te_txtlen = 41;
   rsrc_gaddr(R_TREE,SEARCH,&obj_ptr);
   out_form(SEARCH);
   objindex = 0;
   curobj = SSTRING;
   objc_edit(form_addr,curobj,0,&objindex,EDINIT);

   do
     {
      event = evnt_multi( MU_BUTTON | MU_KEYBD,
		1,1,butdown,
		0,0,0,0,0,
		0,0,0,0,0,
		msgbuff,0,0,&mx,&my,&ret,&ret,&keycode,&ret);

	if (event & MU_BUTTON)
	  { if(butdown) 
	      { butdown = FALSE;
		graf_mkstate(&gr_mkmx,&gr_mkmy,&gr_mkmstate,&gr_mkkstate);
		obj_selected = objc_find(form_addr,ROOT,MAX_DEPTH,
					   gr_mkmx,gr_mkmy);
	      }
	   else 
	     { butdown = TRUE;
	       switch (obj_selected)
		 { case SEAOK:
		     do_obj(SEARCH,SEAOK,SELECTED);
	             objc_draw(form_addr,SEAOK,0,fo_gx,fo_gy,fo_gw,fo_gh);
		     keycode = 0x1c0d;
		     break;
		   case SEACAN:
		     do_obj(SEARCH,SEACAN,SELECTED);
	             objc_draw(form_addr,SEACAN,0,fo_gx,fo_gy,fo_gw,fo_gh);
		     keycode = 0x1c0d;
		     break;
		   case SSTRING:
		     event |= MU_KEYBD;
		     keycode = 0x4800;
		     break;
		   case HSTRING1:
		     event |= MU_KEYBD;
		     keycode = 0x5000;
		     break;
		 }
	     }
         }

      if (event & MU_KEYBD)
	switch (keycode)
	  {  case 0x0f09:
	     case 0x5000:
	       if (curobj == SSTRING)
		 { objc_edit(form_addr,curobj,0,&objindex,EDEND);
		   curobj = HSTRING1;
		   objindex = strlen(seahex);
		   objc_edit(form_addr,curobj,0,&objindex,EDINIT);
		 }
	       break;
	     case 0x4800:
	       if (curobj == HSTRING1)
		 { objc_edit(form_addr,curobj,0,&objindex,EDEND);
		   curobj = SSTRING;
		   objindex = strlen(seastr);
		   objc_edit(form_addr,curobj,0,&objindex,EDINIT);
		 }
	     case 0x4d00:
	     case 0x4b00:
	       objc_edit(form_addr,curobj,keycode,&objindex,EDCHAR);
	       break;	       
	     case 0x537f:
	     case 0x0e08:
	       if (curobj == SSTRING)
	         { dum = objindex * 2;
		   i = seahex[dum];
		   seahex[dum] = '\0';
		   objc_edit(form_addr,SSTRING,keycode,&objindex,EDCHAR);
		   objc_edit(form_addr,HSTRING1,0,&dum,EDINIT);
		   seahex[dum] = i;
		   objc_edit(form_addr,HSTRING1,keycode,&dum,EDCHAR);
		   objc_edit(form_addr,HSTRING1,keycode,&dum,EDCHAR);
		   objc_edit(form_addr,HSTRING1,0,&dum,EDEND);
		 }
	       else
	         { objc_edit(form_addr,HSTRING1,keycode,&objindex,EDCHAR);
		   redosea(objindex);
		 }
	       break;
	     case 0x1c0d:
	       do_obj(SEARCH,SEAOK,SELECTED);
	       objc_draw(form_addr,SEAOK,0,fo_gx,fo_gy,fo_gw,fo_gh);
	       obj_selected = SEAOK;
	       break;
	     case 0x011b:
	       objc_edit(form_addr,curobj,0,&objindex,EDEND);
	       dum = 0;
	       objc_edit(form_addr,SSTRING,0,&dum,EDINIT);
	       objc_edit(form_addr,SSTRING,keycode,&dum,EDCHAR);
	       objc_edit(form_addr,SSTRING,0,&dum,EDEND);
	       dum = 0;
	       objc_edit(form_addr,HSTRING1,0,&dum,EDINIT);
	       objc_edit(form_addr,HSTRING1,keycode,&dum,EDCHAR);
	       objc_edit(form_addr,HSTRING1,0,&dum,EDEND);
	       objc_edit(form_addr,curobj,0,&objindex,EDINIT);
	       break;	       
	     default:
	       keycode &= 0xff;
	       if ((curobj == SSTRING) && (objindex != 20))
	         { if (keycode == 0)
		     objc_edit(form_addr,curobj,' ',&objindex,EDCHAR);
		   else
		     objc_edit(form_addr,curobj,keycode,&objindex,EDCHAR);
		   j = (keycode / 16);
		   j = (j > 9) ? ('A'+(j-10)) : '0'+j;
		   dum = (objindex-1) * 2;
		   i = seahex[dum];
		   seahex[dum] = '\0';
		   objc_edit(form_addr,HSTRING1,0,&dum,EDINIT);
		   seahex[dum] = i;
		   objc_edit(form_addr,HSTRING1,j,&dum,EDCHAR);
		   j = (keycode % 16);
		   j = (j > 9) ? ('A'+(j-10)) : '0'+j;
		   objc_edit(form_addr,HSTRING1,j,&dum,EDCHAR);
		   objc_edit(form_addr,HSTRING1,0,&dum,EDEND);
		 }
	       else if (objindex != 40)
		 { if (((keycode >= '0') && (keycode <= '9')) ||
			((keycode >= 'A') && (keycode <= 'F')))
		     { i = objindex;
		       objc_edit(form_addr,curobj,keycode,&objindex,EDCHAR);
		       redosea(i);
		      }
		 }
	  }
     }
   while (keycode != 0x1c0d);
		
  undo_obj(SEARCH,obj_selected,SELECTED);
		   
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  event = FALSE;
  return(obj_selected);
}

/************************************************************************/
/* redosea redoes the search string for hexstr				*/
/************************************************************************/
redosea(inindex)
int inindex;
{ register int i,j,k;
  int dum,hexlen;

  j = inindex & 0xfe;
  hexlen = strlen(seahex) & 0xfe;
  dum = (hexlen / 2)-1;
  for (k=j;k<hexlen;k+=2)
    { i = seahex[k];
      j = (i > '9') ? (i+10) - 'A' : i - '0';
      j &= 0xf;
      j <<= 4;
      i = seahex[k+1];
      i = (i > '9') ? (i+10) - 'A' : i - '0';
      i &= 0xf;
      j |= i;
      dum = k / 2;
      if (j == 0)
        seastr[dum] = 0x20;
      else
        seastr[dum] = j;
    }
  seastr[dum+1] = '\0';
  objc_draw(form_addr,SSTRING,0,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* fold_map maps the folder						*/
/************************************************************************/
fold_map()
{ register unsigned int i,j,k,l;
  char tempstr[4];

  fatsecs = &fatbuff;
  exit_obj = fiselect(0,filenames);
  graf_mouse(ARROW,0x0L);
  if ((exit_obj != 0) || (dirclust == 0))
    return;

  dfptr = dirbuff;
  maxdirs = 1;

  diskcls = bpbptr->numcl+(bpbptr->datrec/2);
  percent = diskcls;
  percent /= 360;
  if (percent < 1)
    percent = 1;

  graf_mouse(HOURGLASS,0x0L);

  for (j=0;j<792;j++)
   diskbuff[j] = ' ';    

  gdirnames(&mapfiles,TRUE);

  prevdir();
  set_text(MAPPING,MAPHEAD,"",FALSE);
  set_text(MAPPING,MAPINFO,"",FALSE);
  out_form(MAPPING);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  do_ffind(filestr,0,&dum,&dum,&newfile,isroot);
  if (newfile == TRUE)
    noffind(filestr);
  else
    { fileinfo();
      fmheadstr[7] = '\0';
      strcat(fmheadstr,"Folder ");
      strcat(fmheadstr,filestr);
      set_text(MAPPING,MAPHEAD,fmheadstr,TRUE);
      mapstr1[17] = '\0';
      uitoa(percent,tempstr);
      strcat(mapstr1,tempstr);
      strcat(mapstr1,mapstr2);
      set_text(MAPPING,MAPINFO,mapstr1,TRUE);

      l = fileclust;
      fnumcl = 0xffff;
      for (j=0;j<fnumcl;j++)
	{ k = ((l/percent)*2);	/* fat pointer */
  	  k += (l/(5*percent)); /* plus spaces */
	  diskbuff[k] = 'U';
	  diskbuff[k+1] = 'U';
	  l = get_fatword(l);
	  if (l == 0xffff)
	    j = 0xffff;
	}

      hide_mouse();

      for(k=0;k<792;k++)
	outchar((k % 66)+7,(k / 66)+7,diskbuff[k]);

      show_mouse();
      graf_mouse(ARROW,0x0L);
      exit_obj = (form_do(form_addr,0) & 0x7fff);
      undo_obj(MAPPING,exit_obj,SELECTED);
      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
     }
  fmheadstr[7] = '\0';
  strcat(fmheadstr,"File ");
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* mapfiles maps the incoming files for a folder			*/
/************************************************************************/
mapfiles(firtime)
int firtime;
{ register int j,k;
  register unsigned l;

  if (!firtime)
    return;

  dirfiles = fileptr;

  fileinfo();
  l = fileclust;
  if ((fileptr->fattr & 0x10) == 0x10)
    fnumcl = 0xffff;
  for (j=0;j<fnumcl;j++)
    { k = ((l/percent)*2);	/* fat pointer */
      k += (l/(5*percent)); /* plus spaces */
      diskbuff[k] = 'U';
      diskbuff[k+1] = 'U';
      l = get_fatword(l);
      if (l == 0xffff)
	j = fnumcl;

      if (error)
	return;
    }
}

/************************************************************************/
/* fold_unerase unerases a folder					*/
/************************************************************************/
fold_unerase()
{
  fatsecs = &fatbuff;
  exit_obj = fiselect(4,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
}

/************************************************************************/
/* fold_erase erases a folder but not contents				*/
/************************************************************************/
fold_erase()
{
  fatsecs = &fatbuff;
  exit_obj = fiselect(0,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  maxdirs = 1;
  dfptr = dirbuff;
  gdirnames(&erasefiles,TRUE);

  prevdir();
  if (filestr[0] != '\0')
    erasename(filestr);
}

/************************************************************************/
/*  fold_sort sorts the folder						*/
/************************************************************************/
fold_sort()
{ int sortorder[4],numsort,l;
  register unsigned int i;
  register int j,k,m,n;
  long longmem;
  int saveclust,howmany,eof;

  numsort = 1;
  for (i=0;i<4;i++)
    sortorder[i] = ' ';

  fatsecs = &fatbuff;
  exit_obj = fiselect(0,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;

  bset_char(DIRSORT,SORTNAME,sortorder[0],FALSE);
  bset_char(DIRSORT,SORTEXT,sortorder[1],FALSE);
  bset_char(DIRSORT,SORTDATE,sortorder[2],FALSE);
  bset_char(DIRSORT,SORTTIME,sortorder[3],FALSE);

  out_form(DIRSORT);
  graf_mouse(ARROW,0x0L);
  do {
    exit_obj = (form_do(form_addr,0) & 0x7fff);
    j = 0;
    switch (exit_obj)
      { case SORTNAME:
	  if (sortorder[0] != ' ')
	    { j = sortorder[0] - '0';
	      sortorder[0] = ' ';
	      numsort--;
	    }
	  else 
	    { sortorder[0] = '0'+numsort;
	      numsort++;
	    }
	  break;
	case SORTEXT:
	  if (sortorder[1] != ' ')
	    { j = sortorder[1] - '0';
	      sortorder[1] = ' ';
	      numsort--;
	    }
	  else 
	    { sortorder[1] = '0'+numsort;
	      numsort++;
	    }
	  break;
	case SORTDATE:
	  if (sortorder[2] != ' ')
	    { j = sortorder[2] - '0';
	      sortorder[2] = ' ';
	      numsort--;
	    }
	  else 
	    { sortorder[2] = '0'+numsort;
	      numsort++;
	    }
	  break;
	case SORTTIME:
	  if (sortorder[3] != ' ')
	    { j = sortorder[3] - '0';
	      sortorder[3] = ' ';
	      numsort--;
	    }
	  else 
	    { sortorder[3] = '0'+numsort;
	      numsort++;
	    }
	  break;
       }

     if (j != 0)
       for (i=0;i<4;i++)
         { k = sortorder[i] - '0';
	   if (k > j)
	     sortorder[i] -= 1;
         }

     bset_char(DIRSORT,SORTNAME,sortorder[0],TRUE);
     bset_char(DIRSORT,SORTEXT,sortorder[1],TRUE);
     bset_char(DIRSORT,SORTDATE,sortorder[2],TRUE);
     bset_char(DIRSORT,SORTTIME,sortorder[3],TRUE);
    }
  while ((exit_obj != SORTOK) && (exit_obj != SORTCAN));

  numsort--;
  for (i=0;i<4;i++)
    sortorder[i] -= '1';

  undo_obj(DIRSORT,exit_obj,SELECTED);
  saveclust = dirclust;
  if ((exit_obj == SORTOK) && (numsort > 0))
    { longmem = Malloc(-1L);
      i = longmem / 32;		/* number of clusters memory can hold */
      longmem = (long)(i) * 32;
      if (i == 0)
        { memerrout();
	  return;
	}
      dfptr = Malloc(longmem);
      dirfiles = dfptr;
      j = 0;
      do {
     	next_sect(dirfiles+((long)(j)*16),j,&eof,isroot);
        if (error)
	  return;
        j += 1;
      	}
      while (((j / 2) < i) && (!eof));

      if (!eof)
	{ form_alert(1,
		"[3][ Insufficient memory to | load directory!][ Stop ]");
	  Mfree(dfptr);
	  return;
	}
      j *= 16;
      i = 0;
      if (isroot == FALSE)
	dfptr += 2;		/* don't sort . and .. of subdirectory */

      while (i < j)
        { dirfiles = &dfptr[i];
          if (dirfiles->fname[0] == '\0')
	    j = i;
	    i++;
	}
      for (i=0;i<numsort;i++)
        for (k=0;k<4;k++)
          if (sortorder[k] == i)
	    { dirfiles = dfptr;
	      howmany = -1;
	      for (n=0;n<j;n++)
	        { m = 0;
	          for (l=0;l<4;l++)
		    if (sortorder[l] < i)
	  	      m |= dircompare(l,n,n+1);
		  if (m == 0)
		    howmany += 1;
	          else
		    { dirfiles = &dfptr[n-howmany];
		      howmany += 1;
		      dirssort(howmany,k);
		      howmany = 0;
		    }
	        }
	      if (howmany != 1)
		{ dirfiles = &dfptr[n-howmany];
		  howmany += 1;
		  dirssort(howmany,k);
		  howmany = 0;
		}
	      k = 4;
            }
    }
	
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  if (isroot == FALSE)
    dfptr -= 2;

  Mfree(dfptr);
}

/************************************************************************/
/* dirssort is a shell sort for directory entries			*/
/************************************************************************/
dirssort(n,innum)
int n,innum;		/* n = number of things, innum = what in dirfiles */
{ 
   register int gap,i,j,k;
   struct dirfiles *dir1ptr,*dir2ptr;

   for (gap = n/2;gap > 0;gap /= 2)
      for (i=gap;i<n;i++)
	 for (j=i-gap;j>=0;j-=gap)
	    { if (dircompare(innum,j,j+gap) <= 0)
		break;
	      dir1ptr = &dirfiles[j];
	      dir2ptr = &dirfiles[j+gap];
	      for (k=0;k<32;k++)
		{ tempdir->fname[k] = dir1ptr->fname[k];
	          dir1ptr->fname[k] = dir2ptr->fname[k];
		  dir2ptr->fname[k] = tempdir->fname[k];
		}
	    }
}

/************************************************************************/
/* dircompare compares fields that are in a directory entry		*/
/************************************************************************/
dircompare(whatcomp,offset1,offset2)
int whatcomp,offset1,offset2;
{ char tempn1[9],tempn2[9];
  register int  i,j;
  struct dirfiles *dir1ptr,*dir2ptr;

  dir1ptr = &dirfiles[offset1];
  dir2ptr = &dirfiles[offset2];

  switch (whatcomp)
    { case 0:
	for (i=0;i<8;i++)
	  { tempn1[i] = dir1ptr->fname[i];
	    tempn2[i] = dir2ptr->fname[i];
	  }
	tempn1[8] = '\0';
	tempn2[8] = '\0';
	strip_spc(tempn1);
	strip_spc(tempn2);
	i = strcmp(tempn1,tempn2);
	break;
      case 1:
	for (i=0,j=8;i<3;i++,j++)
	  { tempn1[i] = dir1ptr->fname[j];
	    tempn2[i] = dir2ptr->fname[j];
	  }
	tempn1[4] = '\0';
	tempn2[4] = '\0';
	strip_spc(tempn1);
	strip_spc(tempn2);
	i = strcmp(tempn1,tempn2);
	break;
      case 2:
	i = ibmword(dir1ptr->fdate);
	j = ibmword(dir2ptr->fdate);
	i -= j;
	break;
      case 3:
	i = ibmword(dir1ptr->ftime);
	j = ibmword(dir2ptr->ftime);
	i -= j;
	break;
    }
  return(i);
}
  
   
/************************************************************************/
/* fold_copy copies the folder somewhere				*/
/************************************************************************/
fold_copy(moveon)
int	moveon;
{
  globmoveon = moveon;
  fatsecs = &fatbuff;
  exit_obj = fiselect(0,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  holdstr[0] = '\0';
  strcat(holdstr,dirstr);
  driv1 = curdrive;
  dir1cl = dirclust;
  bpbp1 = bpbptr;
  fatsecs = &fatbuff[3072];
  exit_obj = fiselect(0,&filenames[1188]);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  driv2 = curdrive;
  dir2cl = dirclust;
  bpbp2 = bpbptr;

  fatsecs = fatbuff;
  bpbptr = bpbp1;
  curdrive = driv1;
  dirclust = dir1cl;
  curclust = dirclust;
  maxdirs = 1;
  
  gdirnames(&copyfiles,TRUE);
}

/************************************************************************/
/* copyfiles copies the files and creates folders needed		*/
/************************************************************************/
copyfiles(firtime)
int firtime;
{ int fsect,fent,saveclust;
  register int i,j;

  if ((!firtime) && ((fileptr->fattr & 0x10) == 0x10) && (globmoveon))
    { erasename(&fileptr->fname[0]);
      return;
    }
  saveclust = curclust;
  dir1cl = dirclust;
  dirclust = dir2cl;
  curdrive = driv2;
  bpbptr = bpbp2;
  dfptr = &dirbuff[1024];
  fatsecs = &fatbuff[3072];
  isroot = FALSE;
  curclust = dirclust;
  if (dirclust == 0)
    isroot = TRUE;
  if ((fileptr->fattr & 0x10) == 0x10)
    { error = myfcreate(&fileptr->fname[0],&fsect,&fent);
      if (error)
	return;
      j = dirfiles->fsclust;
      globclust = ibmword(j);
      for (i=13;i<32;i++)
	dirfiles->fname[i] = fileptr->fname[i];
      dirfiles->fsclust = j;
      if (isroot)
	j = (bpbptr->datrec-bpbptr->rdlen)+fsect;
      else
	j = bpbptr->datrec+((dirclust-2) * 2)+fsect;
      error = myrwabs(1,&dirbuff[1024],1,j,driv2);
      if (!error)
	{ for (j=0;j<1024;j++)
	    diskbuff[j] = '\0';
	  dirfiles = &diskbuff[0];
	  dirfiles->fname[0] = '.';
	  for (i=13;i<32;i++)
	    dirfiles->fname[i] = fileptr->fname[i];
	  dirfiles->fsclust = ibmword(globclust);
	  dirfiles += 1;
	  dirfiles->fname[0] = '.';
	  dirfiles->fname[1] = '.';
	  for (i=13;i<32;i++)
	    dirfiles->fname[i] = fileptr->fname[i];
	  j = ((globclust-2) * 2)+bpbptr->datrec;
          error = myrwabs(1,diskbuff,2,j,driv2);
	}
    }
   else
     copyname(&fileptr->fname[0],&fileptr->fname[0]);
       
  dirclust = dir1cl;
  curdrive = driv1;
  bpbptr = bpbp1;
  dfptr = &dirbuff[0];
  if (driv1 != driv2)
    fatsecs = &fatbuff[0];
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;

  if ((!error) && ((fileptr->fattr & 0x10) == 0) && (globmoveon))
    erasename(&fileptr->fname[0]);

  fatsecs = &fatbuff[0];

  curclust = saveclust;
}

/************************************************************************/
/* fold_rename renames a folder						*/
/************************************************************************/
fold_rename()
{ register int i;
  int fsect,fent;

  fatsecs = &fatbuff;
  exit_obj = fiselect(0,filenames);
  graf_mouse(ARROW,0x0L);
  if ((exit_obj != 0) || (dirclust == 0))
    return;

  set_text(RENAME,RENNNAME,filestr,FALSE);
  set_tlen(RENAME,RENNNAME,12);

  dfptr = dirbuff;
  maxdirs = 1;

  prevdir();
  graf_mouse(HOURGLASS,0x0L);
  do_ffind(filestr,0,&fsect,&fent,&newfile,isroot);
  if (newfile == TRUE)
    { noffind(filestr);
      exit_obj = RECAN;
    }
  else
    { set_text(RENAME,RENNNAME,filestr,FALSE);
      graf_mouse(ARROW,0x0L);
      out_form(RENAME);
      exit_obj = (form_do(form_addr,0) & 0x7fff);
      form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
      form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
     }
  if (exit_obj == RENOK)
    { i = dirfiles->fattr;
      exp_str(filestr,dirfiles->fname);
      dirfiles->fattr = i;
      if (isroot == TRUE)
	i = (bpbptr->datrec-bpbptr->rdlen)+fsect;
      else
        i = bpbptr->datrec+((dirclust-2) * 2)+fsect;
      error = myrwabs(1,dirbuff,1,i,curdrive);

      if (error)
	{ rener1[30] = '\0';
	  strcat(rener1,"folder ");
	  sqfname(filestr);
	  strcat(rener1,filestr);
	  strcat(rener1,". ][ Stop ]");
	  graf_mouse(ARROW,0x0L);
	  i = form_alert(1,rener1);
	  rener1[30] = '\0';
	  strcat(rener1,"file ");
	}
     }
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* fold_print prints folder(s)						*/
/************************************************************************/
fold_print()
{ int subons;

  fatsecs = &fatbuff;
  exit_obj = fiselect(0,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  curclust = dirclust;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;

  strippat();   
  maxdirs = 1;
  bset_char(PRINT,PRINTCH2,'\010',FALSE);    
  bset_char(PRINT,PRINTCH1,' ',FALSE);    
  subons = TRUE;

  out_form(PRINT);
  do {
    exit_obj = (form_do(form_addr,0) & 0x7fff);
    switch (exit_obj)
      { case PRINTCH1:
	  bset_char(PRINT,PRINTCH1,'\010',TRUE);    
	  bset_char(PRINT,PRINTCH2,' ',TRUE);    
	  subons = TRUE;
          break;
	case PRINTCH2:
	  bset_char(PRINT,PRINTCH2,'\010',TRUE);    
	  bset_char(PRINT,PRINTCH1,' ',TRUE);    
	  subons = FALSE;
          break;
       }
   }
  while ((exit_obj != PRINTOK) && (exit_obj != PRINTCAN));

  undo_obj(PRINT,exit_obj,SELECTED);

  if (exit_obj == PRINTOK)
    { graf_mouse(HOURGLASS,0x0L);
      gdirnames(&printnames,subons);
      if (!error)
        if (!lprint('\r'))
	  lprint('\n');
    }
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* printnames prints the names of the files as they come in		*/
/************************************************************************/
printnames(firtime)
int firtime;
{ register int i,j;
  
  if (firtime == FALSE)
    { deldir();
      return;
    }

  if ((fileptr->fattr & 0x10) == 0x10)
    { if (lprint('\r'))
	return;
      if (lprint('\n'))
	return;
      if (lprint('\r'))
	return;
      if (lprint('\n'))
	return;
      for (i=0;i<11;i++)
	filestr[i] = fileptr->fname[i];
      filestr[11] = '\0';
      sqfname(filestr);
      i = strlen(filestr);
      if ((strlen(dirstr)+i) < 80)
	adddir(filestr);
      i = strlen(dirstr);
      for (j=0;j<i;j++)
 	if (lprint(dirstr[j]))
	  return;
    }
  else
    { counter++;
      for (i=0;i<8;i++)
        if (lprint(fileptr->fname[i]))
          return;
      if (lprint('.'))
        return;
      for (i=8;i<11;i++)
        if (lprint(fileptr->fname[i]))
          return;
      if (lprint(' '))
        return;
      if (lprint(' '))
        return;
      if (lprint(' '))
        return;
    }

  if (((fileptr->fattr & 0x10) == 0x10) || (counter >= 5))
    { counter = 0;
      if (lprint('\r'))
	return;
      lprint('\n');
    }

}

/************************************************************************/
/* erasefiles erases file names in the folders				*/
/************************************************************************/
erasefiles(firtime)
int firtime;
{ int saveclust;

  saveclust = curclust;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  if ((firtime) && ((fileptr->fattr & 0x10) == 0x10))
    return;
  erasename(&fileptr->fname[0]);
  curclust = saveclust;
}

/************************************************************************/
/* gdirnames gets the directory names and calls a routine for everyone	*/
/************************************************************************/
gdirnames(inrtn,subdirs)
int (*inrtn)(),subdirs;		/* subdirs = flag for doing sub-directories */
{ long longmem;
  register int i,j,k,l;
  int eof,prevclust,firtime;

  counter = 0;
  dirfiles = dfptr;		/* start of filenames */

  numsdirs = 0;
  j = 0;
  eof = FALSE;
  l = 0;
  firtime = TRUE;
  do {
   prevclust = curclust;
   if (l == 0)
     dodnames(inrtn);		/* do the directory names */
   if ((!subdirs) || (error))
     return;
   curclust = prevclust;	/* restore cluster number for search */
   i = 0;
   do {
      prevclust = curclust;
      next_sect(dirfiles+((long)(i)*16),j,&eof,isroot);
      if (error)
	return;
      i += 1;
      j += 1;
      }
    while ((i < (maxdirs * 2)) && (!eof));

    i *= 16;
    i -= l;
    for (k=l;k<i;k++)
      { fileptr = &dirfiles[k];
   	if (fileptr->fname[0] == '\0')
	  eof = TRUE;
	else 
	  if ((fileptr->fname[0] != 0xe5) && ((fileptr->fattr & 0x10) == 0x10)
	      && (fileptr->fname[0] != '.'))
	    { (*inrtn)(firtime);
	      dirfiles = dfptr;		/* restore pointer */
	      if (error)
	        return;
	      if ((numsdirs < 100) && (firtime == TRUE))
		{ savesdirs[numsdirs][0] = prevclust;
		  savesdirs[numsdirs][1] = ((j-2)*16)+k;
		  savesdirs[numsdirs+100][0] = dir2cl;
		  dir2cl = globclust;
		  numsdirs++;
		  curclust = ibmword(fileptr->fsclust);
		  isroot = FALSE;
		  j = 0;
		  k = i;
		  eof = FALSE;
		}
	      else
		firtime = TRUE;
	    }
       }

    l = 0;
    if ((numsdirs != 0) && (eof))
      { numsdirs--;
	i = savesdirs[numsdirs][1];
	curclust = savesdirs[numsdirs][0];
	dir2cl = savesdirs[numsdirs+100][0];
	dirclust = curclust;
 	if (curclust == 0)
	  isroot = TRUE;
	j = i / 16;
	l = i % 16;
	eof = FALSE;
	firtime = FALSE;
      }

    }
  while (!eof);
}

/************************************************************************/
/* dodnames does the directory names before going on to the next direc	*/
/************************************************************************/
dodnames(inrtn)
int (*inrtn)();
{ register int i,j,k;
  int eof,fclust;

   fclust = curclust;
   j = 0;
   do {   
     i = 0;
     do {
      next_sect(dirfiles+((long)(i)*16),j,&eof,isroot);
      if (error)
	return;
      i += 1;
      j += 1;
      }
    while ((i < (maxdirs * 2)) && (!eof));

    i *= 16;

    for (k=0;k<i;k++)
      { dirclust = fclust;
	fileptr = &dirfiles[k];
	if (fileptr->fname[0] == '\0')
	  return;
	if (((fileptr->fattr & 0x18) == 0) && (fileptr->fname[0] != 0xe5))
	  { (*inrtn)(TRUE);
	    dirfiles = dfptr;
	    if (error)
	      return;
	  }
      }
    }
  while (!eof);
}
	    
/************************************************************************/
/* lprint prints a charcter if device is ready else sets error and	*/
/* returns								*/
/************************************************************************/
lprint(inchar)
int inchar;
{ register int l;

  l = 0;
  while (Cprnos() != -1)
    { l++;
      if (l == 0x7fff)
	{ graf_mouse(ARROW,0x0L);
	  l = (form_alert(1,prstr4)-1);
	  if (l == 1)
	    { error = TRUE;
	      return(TRUE);
	    }
	}
     }
   Cprnout(inchar);
   return(FALSE);
}

/************************************************************************/
/* prevdir gets the previous directory and places 			*/
/* present name in filestr 						*/
/************************************************************************/
prevdir()
{
  strippat();
  filestr[0] = '\0';
  for (i=strlen(dirstr)-2;i>0;i--)
    if ((dirstr[i] == '\\') || (dirstr[i] == ':'))
      { strcat(filestr,&dirstr[i+1]);
	dirstr[i+1] = '\0';
	i = 0;
      }
  i = strlen(filestr)-1;
  if (filestr[i] == '\\')
    filestr[i] = '\0';
  i = strlen(dirstr)-1;
  if (dirstr[i] != '\\')
    strcat(dirstr,"\\");
  strcat(dirstr,savepat);
  get_dirptr(dirstr);
  dirclust = curclust;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
}

/************************************************************************/
/* myrwabs either calls Rwabs or if floppy my own fast routines		*/
/************************************************************************/
myrwabs(function,inbuff,number,insector,drive)
int function;
char *inbuff;
int number,insector,drive;
  { register int ftrack,fsector,fside,temp,i;

    if (drive >= 2)
      return (Rwabs(function,inbuff,number,insector,drive));

    do {
    temp = (globnsecs * globnsides);
    ftrack = insector/temp;
    fsector = (insector / globnsides) % temp;
    fsector += 1;
    fside = (insector % globnsides);

    if ((num_flops == 1) && (drive != cur_flop))
      { newfstr[23] = 'A'+drive;
	form_alert(1,newfstr);
	cur_flop = drive;
      }

    if ((function == 0) || (function == 2))      
      temp = read_sector(ftrack,fsector,fside,drive,inbuff);
    else
      temp = write_sector(ftrack,fsector,fside,drive,inbuff);

    if (temp != 0)
      { i = 'A'+drive;
	dierrstr[19] = i;
	wrprostr[19] = i;
	if ((temp & 0x40) == 0x40)
	  { i = form_alert(2,wrprostr);
	    if (i == 1)
	      return(-1);
	  }	  
	else
	  { i = form_alert(2,dierrstr);
	    if (i == 1)
	      return(-1);
	  }
	number++;		/* fake out --number because we need to retry */
      }
    else
      { inbuff += 512;
        insector++;
      }
    }
    while (--number > 0);
    return(0);
}
