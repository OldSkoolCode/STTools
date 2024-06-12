/*********************************************************************/
/* DEFINES						   	     */
/*********************************************************************/

#define WI_KIND (NAME | UPARROW | DNARROW | VSLIDE)
#define SOURCE 0
#define DESTINATION 1
#define MEMERR 2
#define RESTERR 3
#define	LOADABLE 1	/* for loadable resource file */
#define STMOD4 0	/* which module this is */
#define toupper(x) ((x >= 'a') && (x <= 'z')) ? x-('a'-'A') : x
#define DISKFULL 127
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

#include "stext.h"		/* external definitions */

/************************************************************************/
/* global definitions for this module					*/
/************************************************************************/

char	*fatsecs;
char	fatbuff[6144];
char	filenames[2574];
char	dirbuff[2048];

struct	fstrs {
	  int dirfnum;			/* directory entry number */
	  int selnum;
	  char fname[17];
        } *fstrs;

struct fstrs *fstrsptr;

struct	dirfiles {
	  char 	fname[8];
	  char 	fext[3];
	  char 	fattr;
	  char 	fresvrd[10];
	  unsigned int ftime;
	  unsigned int fdate;
	  unsigned int fsclust;
	  long	fsize;
	} *dfptr;

struct dirfiles *dirfiles;

char	dirstr[41];
char	olddirstr[41];
char	holdstr[41];
char	filestr[13];
int 	lafatsec,numfiles,dirstart,maxdirs,numskipped;
char 	tempstr[14];
char 	savepat[14];
int	numsels,curdrive,driv1,driv2;
long	slibox;
int	curclust,newfile;
char	eraser1[70] = "[3][ Error trying to erase | file ";
char	eraser2[] = ". ][ Next | Stop ]";

char	rener1[70] = "[3][ Error trying to rename | file ";
char	rener2[] = ". ][ Next | Stop ]";

char	timestr[7],datestr[8];
char	chster1[80] = "[3][ Error trying to change | file ";
char	chster2[] = " attributes. ][ Next | Stop ]";

char	edstr1[80];
char	edstr2[] = "[1][ Proceed to edit | file "; 
char	edstr3[] = "? ][ Yes | Stop ]";
char	fsstr1[] = "[1][ Proceed to search | file ";

char	fmstr1[80] = "[1][ Proceed to map | file "; 
char	fmstr2[] = "? ][ Yes | Stop ]";

int	fnumcl,filenum,globftype;
unsigned int fileclust,dirclust;
int	filesec,prfclust;
long	fsize;
char	headstr[30];

char	prstr1[] = 
"[2][ Eject last page after | printing each file? ][ Yes | No | Stop ]";
char	prstr2[90] = "[3][ Error reading file ";
char	prstr3[] = ". | What should be done? ][ Next | Stop ]";
char	prstr4[] = "[2][ Printer not ready. | What now? ][Retry| Stop ]";

char	vfstr1[80] = "[3][ Error reading file ";
char	vfstr2[] = ",| cluster number ";
char	vfstr3[] = ".][Continue| Next | Stop ]";
int	isroot,fatu_on,upd_sec;
char	*upd_mem;

char	movstr1[80] = "[3][ ";
char	movstr2[] = " file cannot | be moved onto itself! ][ Next | Stop ]";
char	copstr1[] = " file cannot | be copied onto itself! ][ Next | Stop ]";
char	fcomstr1[] = "[3][ Insufficient comparison | files selected!][ Stop ]";
char	difstr[] = "[3][ Disk is full! | What should be done?][ Next | Stop ]";

char	festr1[70] = "[2][ ";
char	festr2[] = " exists! Should| it be overwritten?][ Yes | No ]";

struct	bpb *bpbp1,*bpbp2;
struct	dirfiles *dir1f;
unsigned int dir1cl,dir2cl;
char	soustr[30] = "Source: ";
char	deststr[30] = "Destination: ";
char	fmheadstr[] = "Map of file 12345678.123";
char	nofstr1[] = 
	   "[3][ Cannot find file 12345678.123 | on current drive!][ Next ]";
char	nofstr2[] = "| on current drive!][ Next ]";
char	fsizerr[] = 
"[3][ File sizes do not match. | Proceed with the compare? ][Continue| Next | Stop ]";

char	fnoma1[100] = "[1][ Mismatch at offset ";
char	fnoma2[] = "| between files ";

/************************************************************************/
/* fi_map maps the fi what else do you want				*/
/************************************************************************/
fi_map()
{ register int i,j,k,l;
  int diskcls,percent,dum;
  char tempstr[4];

  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if ((exit_obj != 0) || (numsels == 0))
    return;

  
  out_form(MAPPING);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
  for (i=0;i<numsels;i++)
    { fmstr1[27] = '\0';
      sqfname(&filenames[i*13]);
      strcat(fmstr1,&filenames[i*13]);
      strcat(fmstr1,fmstr2);
      graf_mouse(ARROW,0x0L);
      j = form_alert(1,fmstr1);
      if (j == 2)
        return;
      curclust = dirclust;
      graf_mouse(HOURGLASS,0x0L);
      do_ffind(&filenames[i*13],0,&dum,&dum,&newfile,isroot);
      if (newfile == TRUE)
        noffind(&filenames[i*13]);
      else
	{ fileinfo();
      	  fmheadstr[12] = '\0';
      	  strcat(fmheadstr,&filenames[i*13]);
	  set_text(MAPPING,MAPHEAD,"                            ",TRUE);
      	  set_text(MAPPING,MAPHEAD,fmheadstr,TRUE);
      	  mapstr1[17] = '\0';
      	  diskcls = bpbptr->numcl+(bpbptr->datrec/2);
      	  percent = diskcls;
      	  percent /= 360;
      	  if (percent < 1)
	    percent = 1;
      	  uitoa(percent,tempstr);
      	  strcat(mapstr1,tempstr);
      	  strcat(mapstr1,mapstr2);
	  set_text(MAPPING,MAPINFO,"                                    ",TRUE);
      	  set_text(MAPPING,MAPINFO,mapstr1,TRUE);

      	  for (j=0;j<792;j++)
            diskbuff[j] = ' ';    

      	  graf_mouse(HOURGLASS,0x0L);

	  l = fileclust;
      	  for (j=0;j<fnumcl;j++)
	    { k = ((l/percent)*2);	/* fat pointer */
  	      k += (l/(5*percent)); /* plus spaces */
	      diskbuff[k] = 'U';
	      diskbuff[k+1] = 'U';
	      l = get_fatword(l);
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
     }
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* fi_unerase unerase the fi						*/
/************************************************************************/
fi_unerase()
{
  fatsecs = &fatbuff;
  exit_obj = fiselect(2,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  out_form(UNERASE);
  form_do(form_addr,0);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* fi_compare compare 2 files						*/
/************************************************************************/
fi_compare()
{ register int i,j,k;
  long	memloc,retmem;
 
  error = FALSE;
  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  holdstr[0] = '\0';
  strcat(holdstr,dirstr);		/* save old directory string */
  driv1 = curdrive;
  dir1cl = dirclust;
  bpbp1 = bpbptr;
  i = numsels;
  fatsecs = &fatbuff[3072];
  exit_obj = fiselect(1,&filenames[1287]);
  k = numsels;
  numsels = i;
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  driv2 = curdrive;
  dir2cl = dirclust; 
  bpbp2 = bpbptr;
  set_text(SHOWFILE,SFHEAD,"  Comparing files:",FALSE);
  set_text(SHOWFILE,SFFILE1," ",FALSE);
  set_text(SHOWFILE,SFFILE2," ",FALSE);
  out_form(SHOWFILE);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,
			fo_gy,fo_gw,fo_gh);

  for (i=0;i<numsels;i++)
    { j = i * 13;
      sqfname(&filenames[j]);
      if (i >= k)
	{ j = form_alert(1,fcomstr1);
	  return;
	}
      else
        { graf_mouse(HOURGLASS,0x0L);
	  sqfname(&filenames[j+1287]);
	  set_text(SHOWFILE,SFFILE1,"            ",TRUE);
	  set_text(SHOWFILE,SFFILE2,"            ",TRUE);
	  soustr[8] = '\0';
	  deststr[13] = '\0';
	  strcat(soustr,&filenames[j]);
	  strcat(deststr,&filenames[1287+j]);
	  set_text(SHOWFILE,SFFILE1,soustr,TRUE);
	  set_text(SHOWFILE,SFFILE2,deststr,TRUE);
	  compfiles(&filenames[j],&filenames[j+1287]);
	  if ((error) && (error != 2))
	    i = numsels;
	
	}
    }
}

/************************************************************************/
/* compfiles compares two file names					*/
/************************************************************************/
compfiles(name1,name2)
char name1[],name2[];
{ long longmem,retval,nbytes,temp;
  unsigned int  fcl1,fcl2,maxcls;
  char *ptr1,*ptr2,tempstr[10];
  int fsect,fent,exit_but;
  register int i,j;
  register long k;

  longmem = Malloc(-1L);
  maxcls = longmem/2048;
  longmem = (long)(maxcls) * 2048;
  if (longmem < 1)
    { error = -1;
      return;
    }
  ptr1 = Malloc(longmem);
  ptr2 = &ptr1[(long)(maxcls) * 1024];

  dfptr = dirbuff;
  maxdirs = 1;
  curdrive = driv1;
  bpbptr = bpbp1;
  dirclust = dir1cl;
  fatsecs = &fatbuff;
  curclust = dirclust;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;

  do_ffind(name1,0,&fsect,&fent,&newfile,isroot);
  if (newfile == TRUE)
    { Mfree(ptr1);
      noffind(name1);
      return;
    }
  fileinfo();
  if (error)
    { Mfree(ptr1); 
      return;
    }

  fcl1 = fileclust;
  i = fnumcl;
  dir1f = dirfiles;
  curdrive = driv2;
  bpbptr = bpbp2;
  dirclust = dir2cl;
  dfptr = &dirbuff[1024];
  fatsecs = &fatbuff[3072];
  curclust = dirclust;
  maxdirs = 1;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;

  do_ffind(name2,0,&fsect,&fent,&newfile,isroot);
  if (newfile == TRUE)
    { Mfree(ptr1);
      noffind(name1);
      return;
    }
  fileinfo();
  if (error)
    { Mfree(ptr1);
      return;
    }
  fcl2 = fileclust;
  if (i != fnumcl)
    { j = form_alert(1,fsizerr);
      if (j != 1)
	{ Mfree(ptr1);
	  error = j;
	  return;
	}
    }
  if (i > fnumcl)
    fnumcl = i;
  j = 0;
  i = 0;
  do {
    dirclust = dir1cl;
    fileclust = fcl1;
    curdrive = driv1;
    bpbptr = bpbp1;
    fatsecs = &fatbuff;
    isroot = FALSE;
    if (dir1cl == 0)
      isroot = TRUE;
    do {
      k = (long)(i % maxcls) * 1024;
      myfread(&ptr1[k]);
      if (error)
        { Mfree(ptr1);
	  return;
        }
      i++;
      }
    while (((i % maxcls) != 0) && (i <= fnumcl));
    fcl1 = fileclust;
    dirclust = dir2cl;
    fileclust = fcl2;
    curdrive = driv2;
    bpbptr = bpbp2;
    fatsecs = &fatbuff[3072];
    isroot = FALSE;
    if (dir2cl == 0)
      isroot = TRUE;
    do {
      k = (long)(j % maxcls) * 1024;
      myfread(&ptr2[k]);
      if (error)
        { Mfree(ptr1);
 	  return;
	}
      j++;
      }
    while (j<=i);
    j = i;
    fcl2 = fileclust;
    retval = 0;
    k = i-1;
    if (k == fnumcl)
      { if ((fsize % 1024) != 0)
	  k -= 1;
	nbytes = (long)(k % maxcls) * 1024;
	nbytes += fsize % 1024;
      }
    else
      nbytes = (long)(k % maxcls) * 1024;
    do {
      temp = qcompare(&ptr1[retval],&ptr2[retval],nbytes);
      if (temp != -1)
        { retval += (temp+1);
	  nbytes -= retval;
	  fnoma1[24] = '\0';
	  temp = (long)((k / maxcls) * maxcls);
	  temp *= 1024;
  	  temp += retval;
	  ltoa(temp,tempstr);
	  reverse(tempstr);
	  strcat(fnoma1,tempstr);
	  strcat(fnoma1,fnoma2);
	  strcat(fnoma1,name1);
	  strcat(fnoma1," & ");
	  strcat(fnoma1,name2);
	  strcat(fnoma1,vfstr3);
	  graf_mouse(ARROW,0x0L);
	  exit_but = form_alert(1,fnoma1);
	  graf_mouse(HOURGLASS,0x0L);
	  error = FALSE;
	  if (exit_but != 1)
            { Mfree(ptr1);
	      error = exit_but;
	      return;
	    }
	}
       }
     while (temp != -1);
    }
  while (i <= fnumcl);
  Mfree(ptr1);
}
  
/************************************************************************/
/* searchfile searches a file for a string				*/
/************************************************************************/
long searchfile(name1,startoff)
char name1[];
long startoff;

{ long longmem,retval,nbytes,temp;
  unsigned int  maxcls,tempfcl;
  char *ptr1,tempstr[10];
  int fsect,fent,exit_but;
  register int i,j,l;
  register long k;

  longmem = Malloc(-1L);
  maxcls = longmem/1024;
  longmem = (long)(maxcls) * 1024;
  if (longmem < 1)
    { error = -1;
      return(-1);
    }
  ptr1 = Malloc(longmem);

  if (startoff == 0)
    { dfptr = dirbuff;
      maxdirs = 1;
      curclust = dirclust;
      isroot = FALSE;
      if (dirclust == 0)
        isroot = TRUE;

      do_ffind(name1,0,&fsect,&fent,&newfile,isroot);
      if (newfile == TRUE)
        { Mfree(ptr1);
          noffind(name1);
          return(-1);
        }
     }

  fileinfo();		/* establish file info every time */
  if (error)
    { Mfree(ptr1); 
      return(-1);
    }

  i = startoff / 1024;
  fnumcl -= i;
  fsize -= (i * 1024);
  for (j=0;j<i;j++)
    { fileclust = get_fatword(fileclust);
      if ((error) || (fileclust == -1))
	{ Mfree(ptr1);
	  return(-1);
	}
    }
  tempfcl = fileclust;		/* hold on to file cluster */
  i = 0;
  do {
    do {
      k = (long)(i % maxcls) * 1024;
      myfread(&ptr1[k]);
      if (error)
        { Mfree(ptr1);
	  return(-1);
        }
      i++;
      }
    while (((i % maxcls) != 0) && (i <= fnumcl));
    retval = 0;
    k = i-1;
    if (k == fnumcl)
      { if ((fsize % 1024) != 0)
	  k -= 1;
	nbytes = (long)(k % maxcls) * 1024;
	nbytes += fsize % 1024;
      }
    else
      nbytes = (long)(k % maxcls) * 1024;
    nbytes -= (startoff % 1024);
    retval = qsearch(&ptr1[(startoff % 1024)],seastr,nbytes,strlen(seastr));
    if (retval != -1)
      { temp = ((long)(k) / maxcls) * maxcls;
	temp *= 1024;
	retval += temp;
	j = retval / 1024;
	fileclust = tempfcl;
	for (i=0;i<j;i++)
	  { fileclust = get_fatword(fileclust);
	    if (error)
	      { Mfree(ptr1);
		return(-1);
	      }
	  }
	retval += (long)(startoff / 1024) * 1024;
	i = fnumcl+1;
      }
    }
  while (i <= fnumcl);
  Mfree(ptr1);
  return(retval);
}
  
/************************************************************************/
/* fi_erase erases the file 						*/
/************************************************************************/
fi_erase()
{ register int i,j;
  int fsect,fent,nextclust;

  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  dfptr = dirbuff;
  maxdirs = 1;

  set_text(SHOWFILE,SFHEAD,"  Erasing file:",FALSE);
  set_text(SHOWFILE,SFFILE1," ",FALSE);
  set_text(SHOWFILE,SFFILE2," ",FALSE);	
  out_form(SHOWFILE);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,
			fo_gy,fo_gw,fo_gh);

  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  graf_mouse(HOURGLASS,0x0L);
  for (i=0;i<numsels;i++)
    { curclust = dirclust;
      graf_mouse(HOURGLASS,0x0L);
      sqfname(&filenames[i*13]);
      set_text(SHOWFILE,SFFILE1,"            ",TRUE);
      set_text(SHOWFILE,SFFILE1,&filenames[i*13],TRUE);
      erasename(&filenames[i*13]);
      if (error)
	{ eraser1[34] = '\0';
	  strcat(eraser1,&filenames[i*13]);
	  strcat(eraser1,eraser2);
	  graf_mouse(ARROW,0x0L);
	  j = form_alert(1,eraser1);
	  if (j == 2)
	    i = numsels;
	}
    }
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);      
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* erasename erases a file pointed to by name				*/
/************************************************************************/
erasename(name)
char *name;
{ int fsect,fent;
  register int j;
  unsigned int fclust;

  curclust = dirclust;
  do_ffind(name,0,&fsect,&fent,&newfile,isroot);
  if (newfile == TRUE)
    { noffind(name);
      return;
    }
  dirfiles->fname[0] = 0xe5;
  fclust = ibmword(dirfiles->fsclust);
  if (isroot == TRUE)
    j = (bpbptr->datrec-bpbptr->rdlen)+fsect;
  else
    j = bpbptr->datrec+((dirclust-2) * 2)+fsect;
  error = myrwabs(3,dirbuff,1,j,curdrive);
  if (error == 0)
    { do {
         fclust = set_fatword(fclust,0);
         }
      while ((fclust != -1) && (error == 0));
      if (error == 0)
	upd_fat();
    }
}

/************************************************************************/
/* fi_find finds the file						*/
/************************************************************************/
fi_find()
{

  filestr[0] = '\0';
  set_text(FINDFILE,FINDFNAM,filestr,FALSE);
  set_tlen(FINDFILE,FINDFNAM,12);

  graf_mouse(ARROW,0x0L);
  out_form(FINDFILE);
  exit_obj = (form_do(form_addr,FINDFNAM) & 0xff);
  undo_obj(FINDFILE,exit_obj,SELECTED);	  
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
}

/************************************************************************/
/* copyname copies from one file to another				*/
/************************************************************************/
copyname(name1)
char *name1;
{ long longmem;
  unsigned int  fcl1,fcl2,maxcls;
  char *ptr1;
  int fsect,fent;
  register int i,j;
  register long k;

  longmem = Malloc(-1L);
  maxcls = longmem/1024;
  longmem = (long)(maxcls) * 1024;
  if (longmem < 1)
    { error = -1;
      return;
    }
  ptr1 = Malloc(longmem);

  dfptr = dirbuff;
  maxdirs = 1;
  curdrive = driv1;
  bpbptr = bpbp1;
  dirclust = dir1cl;
  fatsecs = &fatbuff;
  curclust = dirclust;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;

  do_ffind(name1,0,&fsect,&fent,&newfile,isroot);
  if (newfile == TRUE)
    { Mfree(ptr1);
      noffind(name1);
      return;
    }
  fileinfo();
  if (error)
    { Mfree(ptr1); 
      return;
    }
  fcl1 = ibmword(dirfiles->fsclust);
  dir1f = dirfiles;
  curdrive = driv2;
  bpbptr = bpbp2;
  dirclust = dir2cl;
  dfptr = &dirbuff[1024];
  fatsecs = &fatbuff[3072];
  curclust = dirclust;
  maxdirs = 1;
  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  error = myfcreate(name1,&fsect,&fent);
  if (error)
    { Mfree(ptr1);
      return;
    }
  fcl2 = ibmword(dirfiles->fsclust);
  for (i=12;i<32;i++)			/* everything except filename */
    dirfiles->fname[i] = dir1f->fname[i];
  dirfiles->fsclust = ibmword(fcl2);
  if (isroot == TRUE)
    j = (bpbptr->datrec-bpbptr->rdlen)+fsect;
  else
    j = bpbptr->datrec+((dirclust-2) * 2)+fsect;
  error = myrwabs(3,&dirbuff[1024],1,j,driv2);
  if (error)
    { Mfree(ptr1);
      return;
    }
  j = 0;
  i = 0;
  do {
    dirclust = dir1cl;
    fileclust = fcl1;
    curdrive = driv1;
    bpbptr = bpbp1;
    fatsecs = &fatbuff;
    isroot = FALSE;
    if (dir1cl == 0)
      isroot = TRUE;
    do {
      k = (long)(i % maxcls) * 1024;
      myfread(&ptr1[k]);
      if (error)
        { Mfree(ptr1);
	  return;
        }
      i++;
      }
    while (((i%maxcls) != 0) && (i <= fnumcl));
    fcl1 = fileclust;
    dirclust = dir2cl;
    fileclust = fcl2;
    curdrive = driv2;
    bpbptr = bpbp2;
    fatsecs = &fatbuff[3072];
    isroot = FALSE;
    if (dir2cl == 0)
      isroot = TRUE;
    do {
      k = (long)(j % maxcls) * 1024;
      myfwrite(&ptr1[k]);
      if (error)
        { dfptr = dirbuff;
	  erasename(name1);
	  Mfree(ptr1);
 	  return;
	}
      j++;
      }
    while (j<=i);
    j = i;
    fcl2 = fileclust;
    }
  while (i <= fnumcl);
  upd_fat();
  Mfree(ptr1);
}

/************************************************************************/
/* fi_verify verifies the file	 					*/
/************************************************************************/
fi_verify()
{ register int i,j,k,l;
  int eject,dum;
  int memcl;
  char *memptr;
  long	longmem;

  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;

  dfptr = dirbuff;
  maxdirs = 1;

  longmem = (long)(Malloc(-1L));
  memcl = (int)(longmem/1024);
  memptr = (long)Malloc(1024L);
  if (memcl < 1)
    { memerrout();
      Mfree(memptr);
    }

  set_text(SHOWFILE,SFHEAD,"  Verifing file:",FALSE);
  set_text(SHOWFILE,SFFILE1," ",FALSE);
  set_text(SHOWFILE,SFFILE2," ",FALSE);
  out_form(SHOWFILE);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,
			fo_gy,fo_gw,fo_gh);

  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  for (i=0;i<numsels;i++)
    { curclust = dirclust;
      graf_mouse(HOURGLASS,0x0L);
      do_ffind(&filenames[i*13],0,&dum,&dum,&newfile,isroot);
      if (newfile == TRUE)
        noffind(&filenames[i*13]);
      else
	{ fileinfo();
          sqfname(&filenames[i*13]);
          set_text(SHOWFILE,SFFILE1,"            ",TRUE);
          set_text(SHOWFILE,SFFILE1,&filenames[i*13],TRUE);
          do {
	    myfread(&memptr[0]);
	    if (error)
	      { vfstr1[24] = '\0';
	        strcat(vfstr1,&filenames[i*13]);
	    	strcat(vfstr1,vfstr2);
	    	uitoa(k/2,tempstr);
	    	strcat(vfstr1,tempstr);
	    	strcat(vfstr1,vfstr3);
	    	graf_mouse(ARROW,0x0L);
	    	j = form_alert(1,vfstr1);
	    	if (j == 2)
	      	  { Mfree(memptr);
	            return;
	      	  }
	    	else 
	      	  fileclust = -1;
	      }
           }
         while (fileclust != -1);
      }
   }
  Mfree(memptr);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* fi_edit edits the file						*/
/************************************************************************/
fi_edit(searchon)
int searchon;
{ register int i,j;
  int dum;
  long retval;

  if (curedit != FILEEDIT)
    { fatsecs = &fatbuff;
      exit_obj = fiselect(1,filenames);
      graf_mouse(ARROW,0x0L);
      if ((exit_obj != 0) || (numsels == 0))
        return;

      dfptr = dirbuff;
      maxdirs = 1;

      menu_bar(gl_menu,FALSE);		/* turn off main menu */
      rsrc_gaddr(R_TREE,EDITMENU,&gl_menu);
      menu_bar(gl_menu,TRUE);
      menuedit = TRUE;
      isroot = FALSE;
      if (dirclust == 0)
        isroot = TRUE;
      filenum = -1;
/*      for (i=0;i<8;i++)
	do_obj(EDITMENU,EMEN5S1+i,DISABLED); */
     }

  do {
    filenum++;
    i = filenum;
    if (i >= numsels)
      { curedit = DISKEDIT;
        return;
      }

    edstr1[0] = '\0';
    if (searchon)
      strcat(edstr1,fsstr1);
    else
      strcat(edstr1,edstr2);
    sqfname(&filenames[i*13]);
    strcat(edstr1,&filenames[i*13]);
    strcat(edstr1,edstr3);
    graf_mouse(ARROW,0x0L);
    j = form_alert(1,edstr1);
    if (j == 2)
      { curedit = DISKEDIT;
        return;
      }
    filesec = 0;
      
    xcurs = 2;
    ycurs = 4;
    graf_mouse(HOURGLASS,0x0L);
    curclust = dirclust;
    if (searchon)
      { retval = searchfile(&filenames[i*13],0L);
	if (retval == -1)
	  newfile = TRUE;
	else
	  { newfile = FALSE;
	    filesec = (retval / 512);
	  }
	doxy((int)(retval % 512));
      }
    else
      { do_ffind(&filenames[i*13],0,&dum,&dum,&newfile,isroot);
        if (newfile == TRUE)
          { noffind(&filenames[i*13]);
 	    graf_mouse(ARROW,0x0L);
          }
	else
          fileinfo();
      }
    }
  while (newfile == TRUE);

  do_obj(EDITMENU,EMEN4S6,CHECKED);
  undo_obj(EDITMENU,EMEN4S7,CHECKED);
  do_obj(EDITMENU,EMEN4S5,DISABLED);
  hide_mouse();
  i = filesec;
  while (i-- > 0)
    fileclust = get_fatword(fileclust);
  logsec = ((fileclust-2)*2)+bpbptr->datrec+(filesec & 1);
  scrtype = 0;
  readon = TRUE;
  clr_window();
  if (scrtype == 0)
    docursor(xcurs,ycurs);
  readok = TRUE;
  editon = TRUE;
  curedit = FILEEDIT;
  do_rw();
  show_mouse();
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* fi_copy copies the file 						*/
/************************************************************************/
fi_copy(moveon)
int	moveon;
{ register int i,j,k;
  
  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  holdstr[0] = '\0';
  strcat(holdstr,dirstr);		/* save old directory string */
  driv1 = curdrive;
  dir1cl = dirclust;
  bpbp1 = bpbptr;
  i = numsels;
  fatsecs = &fatbuff[3072];
  exit_obj = fiselect(1,&filenames[1287]);
  k = numsels;
  numsels = i;
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  driv2 = curdrive;
  dir2cl = dirclust;
  bpbp2 = bpbptr;
  if (moveon)
    set_text(SHOWFILE,SFHEAD,"  Moving file:",FALSE);
  else
    set_text(SHOWFILE,SFHEAD,"  Copying file:",FALSE);
  set_text(SHOWFILE,SFFILE1," ",FALSE);
  set_text(SHOWFILE,SFFILE2," ",FALSE);
  out_form(SHOWFILE);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,
			fo_gy,fo_gw,fo_gh);

  for (i=0;i<numsels;i++)
    { j = i * 13;
      sqfname(&filenames[j]);
      if (i >= k)
	{ filenames[1287+j] = '\0';
	  strcat(&filenames[1287+j],&filenames[j]);
	}
       if (((dir1cl == dir2cl) && (driv1 == driv2)) &&
	     (!strcmp(&filenames[j],&filenames[1287+j])))
	{ movstr1[5] = '\0';
	  strcat(movstr1,&filenames[j]);
	  if (moveon)
	    strcat(movstr1,movstr2);
	  else
	    strcat(movstr1,copstr1);
	  j = form_alert(1,movstr1);
	  if (j == 2)
	    return;
	}
      else
        { graf_mouse(HOURGLASS,0x0L);
	  sqfname(&filenames[1287+j]);
	  set_text(SHOWFILE,SFFILE1,"            ",TRUE);
	  set_text(SHOWFILE,SFFILE2,"            ",TRUE);
	  soustr[8] = '\0';
	  deststr[13] = '\0';
	  strcat(soustr,&filenames[j]);
	  strcat(deststr,&filenames[1287+j]);
	  set_text(SHOWFILE,SFFILE1,soustr,TRUE);
	  set_text(SHOWFILE,SFFILE2,deststr,TRUE);
	  j += 1287;		/* then use it			*/
	  copyname(&filenames[j]);
          if (error == DISKFULL)
	    { j = form_alert(2,difstr);
	      if (j == 2)
		return;
	    }
	  else if (error != FALSE)
	    return;
	  if (moveon)
	    { dirclust = dir1cl;
	      curdrive = driv1;
	      bpbptr = bpbp1;
	      if (driv1 != driv2)
	        fatsecs = &fatbuff;
	      curclust = dirclust;
	      isroot = FALSE;
	      if (dirclust == 0)
		isroot = TRUE;
	      dfptr = dirbuff;
	      erasename(&filenames[i*13]);
	      if (error)
		return;
	    }
	}
    }
}

/************************************************************************/
/* fi_rename renames the file	 					*/
/************************************************************************/
fi_rename()
{ register int i,j;
  int fsect,fent,nextclust;
  unsigned int fclust;

  set_text(RENAME,RENNNAME,filestr,FALSE);
  set_tlen(RENAME,RENNNAME,12);

  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  dfptr = dirbuff;
  maxdirs = 1;

  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  for (i=0;i<numsels;i++)
    { curclust = dirclust;
      graf_mouse(HOURGLASS,0x0L);
      do_ffind(&filenames[i*13],0,&fsect,&fent,&newfile,isroot);
      if (newfile == TRUE)
	{ noffind(&filenames[i*13]);
	  exit_obj = RECAN;
	}
      else
	{ filestr[0] = '\0';
      	  strcat(filestr,&filenames[i*13]);
      	  set_text(RENAME,RENNNAME,filestr,FALSE);
 	  graf_mouse(ARROW,0x0L);
      	  out_form(RENAME);
          exit_obj = (form_do(form_addr,0) & 0x7fff);
          form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
          form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
	}
      if (exit_obj == RENOK)
        { j = dirfiles->fattr;
	  exp_str(filestr,dirfiles->fname);
	  dirfiles->fattr = j;
	  if (isroot == TRUE)
	    j = (bpbptr->datrec-bpbptr->rdlen)+fsect;
	  else
	    j = bpbptr->datrec+((dirclust-2) * 2)+fsect;
          error = myrwabs(3,dirbuff,1,j,curdrive);

          if (error)
	    { rener1[35] = '\0';
	      sqfname(&filenames[i*13]);
	      strcat(rener1,&filenames[i*13]);
	      strcat(rener1,rener2);
	      graf_mouse(ARROW,0x0L);
	      j = form_alert(1,rener1);
	      if (j == 2)
	        i = numsels;
	    }
	 }
       else if (exit_obj != RENCAN)
	 i = numsels;
    }
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* fi_print prints the file						*/
/************************************************************************/
fi_print()
{ register int i,j,k,l;
  int eject,dum;
  int memcl;
  char *memptr;
  long	longmem;
  register long m;

  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  dfptr = dirbuff;
  maxdirs = 1;

  eject = form_alert(1,prstr1);
  if (eject == 3)
    return;

  longmem = (long)(Malloc(-1L));
  memcl = (int)(longmem/1024);
  memptr = (long)Malloc(longmem);
  if (memcl < 1)
    { memerrout();
      Mfree(memptr);
      graf_mouse(ARROW,0x0L);
      return;
    }

  set_text(SHOWFILE,SFHEAD,"  Printing file:",FALSE);
  set_text(SHOWFILE,SFFILE1,"",FALSE);
  set_text(SHOWFILE,SFFILE2,"",FALSE);
  out_form(SHOWFILE);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,
			fo_gy,fo_gw,fo_gh);

  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  for (i=0;i<numsels;i++)
    { curclust = dirclust;
      graf_mouse(ARROW,0x0L);
      do_ffind(&filenames[i*13],0,&dum,&dum,&newfile,isroot);
      if (newfile == TRUE)
	noffind(&filenames[i*13]);
      else
	{ fileinfo();
          sqfname(&filenames[i*13]);
       	  set_text(SHOWFILE,SFFILE1,"            ",TRUE);
          set_text(SHOWFILE,SFFILE1,&filenames[i*13],TRUE);
      	  do {
	    k = 0;
	    do {
	      m = (long)(k % memcl) * 1024;
	      myfread(&memptr[m]);
	      if (error)
	        { prstr2[24] = '\0';
	      	  strcat(prstr2,&filenames[i*13]);
	      	  strcat(prstr2,prstr3);	    
	      	  graf_mouse(ARROW,0x0L);
	      	  j = form_alert(1,prstr2);
	      	  if (j == 2)
	            { Mfree(memptr);
		      return;
		    }
	          else 
	            fileclust = -1;
	        }
	       k++;
	      }
	    while (((k%memcl) != 0) && (fileclust != -1));
            if ((k%memcl) != 0)
	      k = ((k-1) * 1024)+(fsize % 1024);
	    else
	      k *= 1024;
	    for (j=0;j<k;j++)
	      if (lprint(memptr[j]))
		  { Mfree(memptr);
		    return;
		  }
           }
         while (fileclust != -1);
         if (eject == 1)
           Cprnout(12);
      }
   }
  Mfree(memptr);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* fi_chstat changes the status of the file				*/
/************************************************************************/
fi_chstat()
{ register int i,j,exitrtn;
  int fsect,fent,nextclust;
  unsigned int fclust;
  register unsigned int temp;
  char tempstr[9],ampm;
  
  set_text(FILESTAT,FATIME,timestr,FALSE);
  set_text(FILESTAT,FADATE,datestr,FALSE);
  set_tlen(FILESTAT,FATIME,8);
  set_tlen(FILESTAT,FADATE,9);

  set_text(FILESTAT,FSHEAD,"                             ",FALSE);
  fatsecs = &fatbuff;
  exit_obj = fiselect(1,filenames);
  graf_mouse(ARROW,0x0L);
  if (exit_obj != 0)
    return;
  dfptr = dirbuff;
  maxdirs = 1;

  isroot = FALSE;
  if (dirclust == 0)
    isroot = TRUE;
  out_form(FILESTAT);
  form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);

  for (i=0;i<numsels;i++)
    { curclust = dirclust;
      graf_mouse(HOURGLASS,0x0L);
      do_ffind(&filenames[i*13],0,&fsect,&fent,&newfile,isroot);
      if (newfile == TRUE)
	{ noffind(&filenames[i*13]);
	  goto fcsbyp;
	}
      headstr[0] = '\0';
      sqfname(&filenames[i*13]);
      strcat(headstr,&filenames[i*13]);
      strcat(headstr," file Attributes");
      set_text(FILESTAT,FSHEAD,"                             ",TRUE);
      set_text(FILESTAT,FSHEAD,headstr,TRUE);
      temp = ibmword(dirfiles->ftime);
      temp = (temp >> 11) & 0x1f;
      timestr[0] = '\0';
      timestr[1] = '\0';
      ampm = 'A';
      if (temp >= 12)
	{ temp -= 12;
	  ampm = 'P';
	}
      if (temp <= 9)
        { timestr[0] = '0';
	  timestr[1] = '\0';
	}
      uitoa(temp,tempstr);
      strcat(timestr,tempstr);
      temp = ibmword(dirfiles->ftime);
      temp = (temp >> 5) & 0x3f;
      if (temp <= 9)
	{ timestr[2] = '0';
	  timestr[3] = '\0';
	}
      uitoa(temp,tempstr);
      strcat(timestr,tempstr);
      temp = ibmword(dirfiles->ftime);
      temp = (temp & 0x1f) << 1;
      if (temp <= 9)
	{ timestr[4] = '0';
	  timestr[5] = '\0';
	}
      uitoa(temp,tempstr);
      strcat(timestr,tempstr);
      timestr[6] = ampm;
      timestr[7] = '\0';
      
      objc_draw(form_addr,FATIME,0,fo_gx,fo_gy,fo_gw,fo_gh);

      datestr[0] = '\0';
      temp = ibmword(dirfiles->fdate);
      temp = (temp >> 5) & 0xf;
      if (temp <= 9)
	{ datestr[0] = '0';
	  datestr[1] = '\0';
	}
      uitoa(temp,tempstr);
      strcat(datestr,tempstr);
      temp = ibmword(dirfiles->fdate);
      temp = temp & 0x1f;
      if (temp <= 9)
	{ datestr[2] = '0';
	  datestr[3] = '\0';
	}
      uitoa(temp,tempstr);
      strcat(datestr,tempstr);
      temp = ibmword(dirfiles->fdate);
      temp = ((temp >> 9)& 0x7f)+1980;
      uitoa(temp,tempstr);
      strcat(datestr,tempstr);

      objc_draw(form_addr,FADATE,0,fo_gx,fo_gy,fo_gw,fo_gh);

      temp = dirfiles->fattr;
      if ((temp & 1) == 1)
        bset_char(FILESTAT,FAROFILE,'\010',TRUE);
      else
        bset_char(FILESTAT,FAROFILE,' ',TRUE);
      if ((temp & 2) == 2)
        bset_char(FILESTAT,FAHFILE,'\010',TRUE);
      else
        bset_char(FILESTAT,FAHFILE,' ',TRUE);
      if ((temp & 4) == 4)
        bset_char(FILESTAT,FASFILE,'\010',TRUE);
      else
        bset_char(FILESTAT,FASFILE,' ',TRUE);
      if ((temp & 32) == 32)
        bset_char(FILESTAT,FAAFILE,'\010',TRUE);
      else
        bset_char(FILESTAT,FAAFILE,' ',TRUE);
      exitrtn = FALSE;
      graf_mouse(ARROW,0x0L);
      do { 
        exit_obj = (form_do(form_addr,0) & 0x7fff);
        switch (exit_obj)
          { case FAROFILE:
	      temp = (temp & 0xfe) | ((temp ^ 1) & 1);
      	      if ((temp & 1) == 1)
        	bset_char(FILESTAT,FAROFILE,'\010',TRUE);
              else
        	bset_char(FILESTAT,FAROFILE,' ',TRUE);
	      break;
	    case FAHFILE:
	      temp = (temp & 0xfd) | ((temp ^ 2) & 2);
      	      if ((temp & 2) == 2)
        	bset_char(FILESTAT,FAHFILE,'\010',TRUE);
              else
        	bset_char(FILESTAT,FAHFILE,' ',TRUE);
	      break;
	    case FASFILE:
	      temp = (temp & 0xfb) | ((temp ^ 4) & 4);
      	      if ((temp & 4) == 4)
        	bset_char(FILESTAT,FASFILE,'\010',TRUE);
              else
        	bset_char(FILESTAT,FASFILE,' ',TRUE);
	      break;
	    case FAAFILE:
	      temp = (temp & 0xdf) | ((temp ^ 32) & 32);
      	      if ((temp & 32) == 32)
        	bset_char(FILESTAT,FAAFILE,'\010',TRUE);
              else
        	bset_char(FILESTAT,FAAFILE,' ',TRUE);
	      break;
	    default:
	      undo_obj(FILESTAT,exit_obj,SELECTED);
	      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	      exitrtn = TRUE;
	      break;
          }
       }
     while (exitrtn == FALSE);
	      
      if (exit_obj == FAOK)
        { dirfiles->fattr = temp;
	  tempstr[0]= '\0';
	  strcat(tempstr,timestr);
	  tempstr[2] = '\0';
          temp = (atoui(tempstr) & 0x1f);
	  if (timestr[6] == 'P')
	    temp += 12;
	  temp <<= 11;
	  tempstr[0]= '\0';
	  strcat(tempstr,&timestr[2]);
	  tempstr[2] = '\0';
	  temp |= (atoui(tempstr) & 0x3f) << 5;
	  tempstr[0]= '\0';
	  strcat(tempstr,&timestr[4]);
	  tempstr[2] = '\0';
	  temp |= (atoui(tempstr) & 0x1f) >> 1;
          dirfiles->ftime = ibmword(temp);

	  tempstr[0] = '\0';
	  strcat(tempstr,datestr);
	  tempstr[2] = '\0';
	  temp = (atoui(tempstr) & 0xf) << 5;
	  tempstr[0] = '\0';
	  strcat(tempstr,&datestr[2]);
	  tempstr[2] = '\0';
	  temp |= (atoui(tempstr) & 0x1f);
	  tempstr[0] = '\0';
	  strcat(tempstr,&datestr[4]);
	  tempstr[4] = '\0';
	  temp |= ((atoui(tempstr) - 1980) & 0x7f) << 9;
	  dirfiles->fdate = ibmword(temp);
	  
          fclust = ibmword(dirfiles->fsclust);
	  if (isroot == TRUE)
	    j = (bpbptr->datrec-bpbptr->rdlen)+fsect;
	  else
	    j = bpbptr->datrec+((dirclust-2) * 2)+fsect;
          error = myrwabs(3,dirbuff,1,j,curdrive);

          if (error)
	    { chster1[35] = '\0';
	      strcat(chster1,&filenames[i*13]);
	      strcat(chster1,chster2);
	      graf_mouse(ARROW,0x0L);
	      j = form_alert(1,chster1);
	      if (j == 2)
	        i = numsels;
	    }
	 }
       else if (exit_obj != FACAN)
	 i = numsels;
fcsbyp:
    }
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  graf_mouse(ARROW,0x0L);
}

/************************************************************************/
/* fileinfo gets the size and the number of clusters for dirfiles->fname*/
/************************************************************************/
fileinfo()
 { register unsigned int i,j;

   fsize = dirfiles->fsize;
   i = fsize;
   j = fsize >> 16;
   i = ibmword(i);
   j = ibmword(j);
   fsize = j | ((long)(i) << 16);
   fnumcl = fsize / bpbptr->clsizb;
   fnumcl += ((fsize % bpbptr->clsizb) != 0);
   fileclust = ibmword(dirfiles->fsclust);
 }

/************************************************************************/
/* fiselect selects a filename						*/
/************************************************************************/
fiselect(ftypes,fbuff)
int ftypes;		/* file types wanted */
char *fbuff;

{ register int i,j,k;
  int	file_obj = 0,edstr,dclick;
  long 	longmem;
  int 	exitrtn;

  globftypes = ftypes;
  if (ftypes == 0)
    { do_obj(FSELECT,FSELSEL,DISABLED);
      do_obj(FSELECT,FSELDSEL,DISABLED);
    }
  error = 0;
  longmem = (long)(Malloc(-1L));
  maxdirs = (int)(longmem / 54);
  fstrs = (long)Malloc(longmem);	/* allocation of directories */
  dfptr = &fstrs[maxdirs];
  if ((dfptr == 0) || (fstrs == 0))
    { form_alert(1,"[3][ Insufficient memory | for file selection. | ][Abort]");
      Mfree(fstrs);
      return(1);			/* return cancel button */
    }

  for (i=FSNAME1;i<=FSNAME9;i++)
    set_text(FSELECT,i,"     ",FALSE);	/* zero any previous names */

  numsels = 0;
  dirstr[0] = 'A'+defdrive;		/* set the drive in string */
  dirstr[1] = '\0';
  olddirstr[0] = '\0';
  strcat(dirstr,":\\*.*");		/* concat \*.* to directory */

  set_text(FSELECT,DIRSTR,dirstr,FALSE);
  set_tlen(FSELECT,DIRSTR,41);
  filestr[0] = '\0';			/* zero out file name */
  set_text(FSELECT,SELSTR,filestr,FALSE);
  set_tlen(FSELECT,SELSTR,12);

  out_form(FSELECT);
  edstr = DIRSTR;

  curdrive = defdrive;
  do {
    graf_mouse(HOURGLASS,0x0L);
    dirstart = 0;
    graf_mouse(ARROW,0x0L);
    exitrtn = 0;
    do 
      {
	if (strcmp(dirstr,olddirstr) != 0)
	  { i = 0;
	    if (numsels != 0)
	      i = form_alert(1,
 "[1][ Warning! All current | file selections | will be lost. ][Abort| OK ]");
	    if (i == 1)
	      { dirstr[0] = '\0';
	        strcat(dirstr,olddirstr);
  		objc_draw(form_addr,DIRSTR,0,fo_gx,fo_gy,fo_gw,fo_gh);
	      }
	    else
	      { graf_mouse(HOURGLASS,0x0L);
		get_dirptr(dirstr);
	        if (error)
	          { Mfree(fstrs);
		    return(1);
	          }
		dirclust = curclust;
	        olddirstr[0] = '\0';
	        strcat(olddirstr,dirstr);
	        do_directory(dirfiles);
		strippat();
	        strcat(dirstr,savepat);
	        set_text(FSELECT,DIRBOX,savepat,TRUE);
		graf_mouse(ARROW,0x0L);
	      }
	  }
        exit_obj = form_do(form_addr,edstr);
        if (exit_obj & 0x8000)
	  dclick = TRUE;
        else
	  dclick = FALSE;
        exit_obj &= 0x7fff;
        switch (exit_obj)
          { case FSNAME1:
	    case FSNAME2:
	    case FSNAME3:
	    case FSNAME4:
	    case FSNAME5:
	    case FSNAME6:
	    case FSNAME7:
	    case FSNAME8:
	    case FSNAME9:
	      if ((file_obj != 0) && (file_obj != exit_obj))
	        { undo_obj(FSELECT,file_obj,SELECTED);
  		  objc_draw(form_addr,file_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	        }
	      fstrsptr = &fstrs[(exit_obj-FSNAME1)+dirstart];
	      if (((fstrsptr->fname[3]) == '\007') && 
			(fstrsptr->fname[5] != 0xe5))
                { do_obj(FSELECT,exit_obj,SELECTED);
  	          objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
		  undo_obj(FSELECT,exit_obj,SELECTED);
	          if (numsels != 0)
		    { i = form_alert(1,
 "[1][ Warning! All current | file selections | will be lost. ][Abort| OK ]");
		      if (i == 1)
		        break;
	    	    }
		  numsels = 0;
	          graf_mouse(HOURGLASS,0x0L);
	          file_obj = 0;
	          filestr[0] = '\0';
  	          objc_draw(form_addr,SELSTR,0,fo_gx,fo_gy,fo_gw,fo_gh);
		  dirfiles = &dfptr[fstrsptr->dirfnum];
 		  adddir(dirfiles->fname);
	          graf_mouse(ARROW,0x0L);
		  if (error)
                    { Mfree(fstrs);
		      return(1);
		    }
	        }
	      else
                {
	          filestr[0] = '\0';
	          strcat(filestr,&fstrsptr->fname[5]);
  	          objc_draw(form_addr,SELSTR,0,fo_gx,fo_gy,fo_gw,fo_gh);
	          file_obj = exit_obj;
	          do_obj(FSELECT,file_obj,SELECTED);
  	          objc_draw(form_addr,file_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	          edstr = SELSTR;
		  if (dclick)
		    { seldesfile(exit_obj); /* select or deselect the filename*/
		      if (fstrsptr->fname[3] == '\007')
			exitrtn = 2;
		    }
	        }
	      break;
	    case DIRCLOSE:
	      if ((file_obj != 0) && (file_obj != exit_obj))
	        { undo_obj(FSELECT,file_obj,SELECTED);
  		  objc_draw(form_addr,file_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	        }
	      file_obj = 0;
 	      undo_obj(FSELECT,exit_obj,SELECTED);
  	      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	      if (numsels != 0)
	        { i = form_alert(1,
 "[1][ Warning! All current | file selections | will be lost. ][Abort| OK ]");
	          if (i == 1)
	            break;
	        }	      
	      numsels = 0;
	      graf_mouse(HOURGLASS,0x0L);
	      if (dfptr->fname[0] != '.')
	        (dfptr+1)->fsclust = 0;
	      deldir();
	      graf_mouse(ARROW,0x0L);
	      if (error)
                { Mfree(fstrs);
		  return(1);
		}
	      break;
	    case FSELSEL:
	      undo_obj(FSELECT,exit_obj,SELECTED);
  	      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	      if (file_obj != 0)
	        { selfile(file_obj);
	          fstrsptr = &fstrs[(file_obj-FSNAME1)+dirstart];
	          if ((fstrsptr->fname[3] == '\007') &&
			(fstrsptr->fname[5] == 0xe5))
		    exitrtn = 2;
		  else
	            objc_draw(form_addr,file_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	     	}
	       for (i=0;i<12;i++)
	         if (filestr[i] != fstrsptr->fname[i])
		  { for (i=0;i<numfiles;i++)
		      { fstrsptr = &fstrs[i];
		       if (checkmatch(filestr,&(fstrsptr->fname[5])))
		         if (selfile((i+FSNAME1)-dirstart) == 99)
		  	   form_alert(1,
	    "[3][ The maximum of 99 | selections has | been reached. ][ OK ]");
		      }
		    newnames();
		    i = 12;
		  }
	      break;
	    case FSELDSEL:
	      undo_obj(FSELECT,exit_obj,SELECTED);
  	      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	      fstrsptr = &fstrs[(file_obj-FSNAME1)+dirstart];
	      if (file_obj != 0)
	        { dselfile(file_obj);
	          fstrsptr = &fstrs[(file_obj-FSNAME1)+dirstart];
	          if ((fstrsptr->fname[3] == '\007') &&
			(fstrsptr->fname[5] == 0xe5))
		    exitrtn = 2;
		  else
		    newnames();
	        }
	       for (i=0;i<12;i++)	     
	         if (filestr[i] != fstrsptr->fname[i])
	           { for (i=0;i<numfiles;i++)
	               { fstrsptr = &fstrs[i];
			 if (checkmatch(filestr,&(fstrsptr->fname[5])))
	                   dselfile((i+FSNAME1)-dirstart);
		       }
		     newnames();
		     i = 12;
		   }
	      break;
	    case UPFILE:
	      undo_obj(FSELECT,exit_obj,SELECTED);
  	      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);	    
	      if (dirstart > 0)
	        { dirstart -= 1;
		  newnames();
	         }
	      break;
	    case DNFILE:
	      undo_obj(FSELECT,exit_obj,SELECTED);
  	      objc_draw(form_addr,exit_obj,0,fo_gx,fo_gy,fo_gw,fo_gh);
	      if ((dirstart+9) < numfiles)
	        { dirstart += 1;
	          newnames();
	        }
	      break;
	    case SLIDEBOX:
	      slibox = graf_slidebox(form_addr,SCRBAR,SLIDEBOX,1);
  	      rsrc_gaddr(R_TREE,FSELECT,&obj_ptr);
	      sel_obj = obj_ptr;
  	      obj_ptr += SCRBAR;
	      sel_obj += SLIDEBOX;
	      slibox *= obj_ptr->ob_height;
	      slibox /= 1000;		/* down to perspective please */
	      sel_obj->ob_y = (int)slibox;
	      objc_draw(form_addr,SCRBAR,1,fo_gx,fo_gy,fo_gw,fo_gh);
	      break;
	    case FSELGO:
	      exitrtn = 2;
	      if (strcmp(dirstr,olddirstr) != 0)
	        get_dirptr(dirstr);
	      break;  
	    case FSELCAN:
	      exitrtn = 2;
	      break;
           }
      }   
    while (exitrtn == 0);
   }
  while (exitrtn == 1);   
  if (file_obj != 0)
    undo_obj(FSELECT,file_obj,SELECTED);
  undo_obj(FSELECT,exit_obj,SELECTED);
  undo_obj(FSELECT,FSELSEL,DISABLED);
  undo_obj(FSELECT,FSELDSEL,DISABLED);
  for (i=0;i<numsels;i++)
    for (j=0;j<numfiles;j++)
      { fstrsptr = &fstrs[j];
	if (fstrsptr->selnum == (i+1))
	  { j = numfiles;
	    fbuff[i*13] = '\0';
	    strcat(&fbuff[i*13],&(fstrsptr->fname[5]));
	  }
      }

  Mfree(fstrs);
  form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
  hide_mouse();
  wind_rest(fo_gx,fo_gy,fo_gw,fo_gh);
  show_mouse();
  return(FSELGO-exit_obj);
}

/************************************************************************/
/* strip_spc strips the spaces off the end of a string			*/
/************************************************************************/
strip_spc(instr)
char *instr;

{ register int i;

  i = strlen(instr);

  if (i==0)
    return;

  instr += strlen(instr)-1;

  for (;i>=0;i--)
   { if (*instr == ' ')
       *instr-- = '\0';
     else
       i = 0;
   }
}
  
/************************************************************************/
/* do_directory takes the directory and gets new filenames		*/
/************************************************************************/
do_directory(dirptr)
struct dirfiles *dirptr;	/* pointer to directory structure */

{ unsigned int nextclust;
  int exitrtn;
  register int i;

  nextclust = ibmword(dirptr->fsclust);

  filestr[0] = '\0';				/* zero out file name */
  set_text(FSELECT,SELSTR,filestr,FALSE);

  numfiles = 0;
  if (nextclust == 0)
    { numskipped = 0;
      do_root();
      return;
    }

  if (nextclust == -1)
    { numfiles = 0;
      newnames();
      return;
    }

  exitrtn = 0;
  i = 0;
  numskipped = 0;
  dirstart = 0;
  do {

    logsec = bpbptr->datrec + ((nextclust-2) * 2);
    error = myrwabs(2,dfptr+((long)(i)*32),2,logsec,curdrive);
    grab_fnames(i);

    if (dir_error())
      return(1);

    i += 1;

    nextclust = get_fatword(nextclust);
    if (error)
      return;

    if (nextclust == -1)
      exitrtn = 1;

    if (i == maxdirs)
      { Bconout(2,'\007');		/* oops to many directory names */
	exitrtn = 1;
      }
     }
   while (exitrtn != 1);

   newnames();
}

/************************************************************************/
/* get_nfree get the next free cluster					*/
/************************************************************************/
get_nfree()
 { register int i,j,k;

   j = bpbptr->fsiz * 512;
   if ((bpbptr->bflags & 1) == 1)
     j /= 4;
   else
     j /= 3;
   for (i=2;i<=j;i++)
     { k = get_fatword(i);
       if (k==0)
	 return(i);
     }
   return(-1);
  }

/************************************************************************/
/* prevclust gets the previous cluster in a file			*/
/************************************************************************/
prevclust()
{ register int i,j,k;

  j = ibmword(dirfiles->fsclust);
  if (fileclust == j)
    return(j);

  for (i=0;1 != 0;i++)
    { k = j;
      j = get_fatword(j);
      if (j == fileclust)
	return(k);
    }
}
   
/************************************************************************/
/* get_fatword get the fat sector and extract word from it		*/
/************************************************************************/
get_fatword(inclust)

int inclust;

{  int sector,*intptr,temp;
   char *byteptr;
   
   error = FALSE;

   if (((bpbptr->bflags & 1) == 1) && (bpbptr->fsiz > 6))
     { sector = (inclust / 256) + bpbptr->fatrec;
       if (lafatsec != sector)
	 { error = myrwabs(2,fatsecs,1,sector,curdrive);
	   lafatsec = sector;
	 }
     }
  else if (bpbptr->fsiz > 6)
    { sector = inclust / 2;	/* for fat entry number */
      inclust = sector % 1024; 	/* offset into this batch of six */
      sector *= 3;		/* number of nibbles per fat entry */
      sector /= 3072;		/* six sectors of data at a time */
      sector *= 6;		/* six sectors of data */
      sector += bpbptr->fatrec;	/* add in fat record */
      if (lafatsec != sector)
	{ error = myrwabs(2,fatsecs,6,sector,curdrive);
	  lafatsec = sector;
	}
    }

  if (dir_error())
    return;

   if ((bpbptr->bflags & 1) == 1)
       { intptr = fatsecs;
         intptr += inclust % 256;
         return(ibmword(*intptr));
       }
   else
     { temp = inclust / 2;
       temp *= 3;
       temp += ((inclust % 2) != 0);
       byteptr = &fatsecs[temp];
       sector = ((int)(*(byteptr+1)) << 8) | (int)(*byteptr) & 0xff;
       if ((inclust & 1) == 1)
	 sector >>= 4;
       sector &= 0xfff;
       if (sector >= 0xff8)
	 return(-1);
       else
	 return(sector);
       }
}

/************************************************************************/
/* set_fatword sets the fat word and writes the sector			*/
/************************************************************************/
set_fatword(inclust,fword)

int inclust,fword;

{  register int i;

   int sector,*intptr,retval,temp;
   char *byteptr;
   
   error = FALSE;

   if (((bpbptr->bflags & 1) == 1) && (bpbptr->fsiz > 6))
     { sector = (inclust / 256) + bpbptr->fatrec;
       if (lafatsec != sector)
	 { if (fatu_on == TRUE)
	     upd_fat();
	   error = myrwabs(2,fatsecs,1,sector,curdrive);
	   lafatsec = sector;
	 }
     }
  else if (bpbptr->fsiz > 6)
    { sector = inclust / 2;	/* for fat entry number */
      inclust = sector % 1024;	/* remainder into cluster */
      sector *= 3;		/* number of nibbles per fat entry */
      sector /= 3072;		/* six sectors of data at a time */
      sector += bpbptr->fatrec;	/* add in fat record */
      if (lafatsec != sector)
	{ if (fatu_on == TRUE)
	    upd_fat();
	  error = myrwabs(2,fatsecs,6,sector,curdrive);
	  lafatsec = sector;
	}
      sector -= bpbptr->fatrec;	/* we add in below */
    }
  else 
    sector = 0;

  if (dir_error())
    return;

   if ((bpbptr->bflags & 1) == 1)
       { intptr = fatsecs;
         intptr += inclust % 256;
	 retval = *intptr;
	 *intptr = fword;
	 upd_sec = sector;
	 upd_mem = fatsecs;
	 fatu_on = TRUE;
	 return(retval);
       }
   else
     { temp = inclust / 2;
       temp *= 3;
       temp += ((inclust % 2) != 0);
       byteptr = &fatsecs[temp];
       fword &= 0xfff;
       sector += temp/512;
       retval = ((int)(*(byteptr+1)) << 8) | (int)(*byteptr) & 0xff;
       if ((inclust & 1) == 1)
         { retval >>= 4;
	   fword = fword << 4;
	   *byteptr &= 0xf;
	   *byteptr |= fword;
	   *(byteptr+1) = (fword >> 8);
	 }
       else
         { *(byteptr+1) &= 0xf0; 
	   *(byteptr+1) |= ((fword & 0xf00) >> 8);
	   *byteptr = fword;
	 }
       retval &= 0xfff;
       i = bpbptr->fatrec+sector;
       upd_mem = &fatsecs[(sector % 6)];
       upd_sec = i;
       fatu_on = TRUE;
       if (retval >= 0xff8)
	 return(-1);
       else
	 return(retval);
     }
}

/************************************************************************/
/* upd_fat updates the fat sectors when needed				*/
/************************************************************************/
upd_fat()
{  int numsecs;

   if (((bpbptr->bflags & 1) == 1) || (bpbptr->fsiz > 6))
     numsecs = 1;
   else
     numsecs = bpbptr->fsiz;
   error = myrwabs(3,upd_mem,numsecs,upd_sec,curdrive);
   if (error == 0)
     { upd_sec -= bpbptr->fsiz;
       error = myrwabs(3,upd_mem,numsecs,upd_sec,curdrive);
     }
   fatu_on = FALSE;
}

/************************************************************************/
/* grab_fnames gets up to 32 names from the cluster			*/
/************************************************************************/
grab_fnames(inclust)
int inclust;
{ register int i,j,k;

  for (j=0;j<32;j++)
    { i = j+(inclust * 32);
      fstrsptr = &fstrs[i-numskipped];
      dirfiles = (dfptr + i);
      k = 1;
      if ((dirfiles->fname[0] == '\0') || (dirfiles->fname[0] == '.') ||
	     ((dirfiles->fattr & 8) == 8) )
	k = 0;
      if (globftype == 0)
	if (((dirfiles->fattr & 0x10) != 0x10) || (dirfiles->fname[0] == 0xe5))
	  k = 0;
      if ((globftype & 1) == 1)
	if (dirfiles->fname[0] == 0xe5)
	  k = 0;
      if ((globftype & 2) == 2)
	{ if ((dirfiles->fname[0] == 0xe5) && ((dirfiles->fattr & 0x10) == 0x10))
	    k = 0;
	  if ((dirfiles->fname[0] != 0xe5) && ((dirfiles->fattr & 0x10) != 0x10))
	    k = 0;
	  if (dirfiles->fname[1] == 0xe5)
	    k = 0;
	}
      if ((globftype & 4) == 4) 
	if (((dirfiles->fattr & 0x10) != 0x10) || (dirfiles->fname[1] == 0xe5))
	  k = 0;
      if (k == 0)
	{ fstrsptr->fname[0] = '\0';
	  strcat(fstrsptr->fname,"     ");
	  numskipped += 1;
	}
      else 
        { for (k=0;k<5;k++)
            fstrsptr->fname[k] = ' ';
	  for (k=0;k<11;k++)
            fstrsptr->fname[5+k] = dirfiles->fname[k];
	  fstrsptr->fname[16] = '\0';
	  fstrsptr->dirfnum = i;
	  fstrsptr->selnum = 0;			   /* not yet selected */
          if ((dirfiles->fattr & 0x10) == 0x10)
            { fstrsptr->fname[3] = '\007';
	      numfiles += 1;
	    }
	  else if (patmatch(&fstrsptr->fname[5]) == FALSE)
	      numskipped += 1;
	  else
	    numfiles += 1;
        }	
     }
}

/************************************************************************/
/* ibmword converts the ibm word to standard integer			*/
/************************************************************************/
ibmword(inint)
unsigned int inint;
  {
    return((inint >> 8) | (inint << 8));
  }

/************************************************************************/
/* do_root gets the root directory filenames				*/
/************************************************************************/
do_root()
  { register int i,j;
    char temp[8];
    long k;

   dirstart = 0;
   i = 0;
   numskipped = 0;
   do {
    error = myrwabs(2,dfptr+((long)(i)*32),2,
			(bpbptr->datrec-bpbptr->rdlen)+(i*2),curdrive);
    if (dir_error())
      return;
    grab_fnames(i);
    i += 1;
  }
  while ((i < maxdirs) && (i < (bpbptr->rdlen/2)));

  if ((bpbptr->rdlen & 1) == 1)		/* if not in clusters */
    { error = myrwabs(2,dfptr+((long)(i)*32),1,
		(bpbptr->datrec-bpbptr->rdlen)+(i*2),curdrive);
      if (dir_error())
        return;
      k = ((long)(i) * 32) + 16;
      dirfiles = &dfptr[k];
      for (j=0;j<128;j++)
        *dirfiles++ = 0;	/* this is bullshit ATARI */
      grab_fnames(i);
    }

  if (i == maxdirs)
    Bconout(2,'\007');

  newnames();
}

/************************************************************************/
/* dir_error output a directory error and returns			*/
/************************************************************************/
dir_error()
  {
  if (error)
    { graf_mouse(ARROW,0x0L);
      form_alert(1,
	"[3][ Error Reading Directory.| Can't get file | names. ][Abort]");
       Mfree(fstrs);
       form_dial(FMD_SHRINK,xdesk,ydesk,(fo_gw/2),(fo_gh/2),
		fo_gx,fo_gy,fo_gw,fo_gh);
       form_dial(FMD_FINISH,xdesk,ydesk,wdesk,hdesk,fo_gx,fo_gy,fo_gw,fo_gh);
       return(TRUE);
    }
  return(FALSE);
 }

/************************************************************************/
/* adddir add the directory string to end of the directory string	*/
/************************************************************************/
adddir(instr)
char instr[];
  {  char tempstr[12];
     register int i,j;

  strippat();
  tempstr[0] = '\0';
  instr[11] = '\0';
  strcat(tempstr,instr);
  sqfname(tempstr);

  strcat(dirstr,tempstr);
  strcat(dirstr,"\\");
  strcat(dirstr,savepat);
  filestr[0] = '\0';
  set_text(FSELECT,SELSTR,filestr,TRUE);
  set_text(FSELECT,DIRSTR,dirstr,TRUE);
  }
  
/************************************************************************/
/* deldir deletes the last directory string tacked on			*/
/************************************************************************/
deldir()
{  register int i;

    strippat();

    for (i=strlen(dirstr)-2;i>0;i--)
       if ((dirstr[i] == '\\') || (dirstr[i] == ':'))
         { dirstr[i+1] = '\0';
	   i = 0;
	 }
    i = strlen(dirstr);
    if (dirstr[i-1] != '\\')
      strcat(dirstr,"\\");
    strcat(dirstr,savepat);
    filestr[0] = '\0';
    set_text(FSELECT,SELSTR,filestr,TRUE);
    set_text(FSELECT,DIRSTR,dirstr,TRUE);
}

/************************************************************************/
/* strippat strips the pattern match string off the end of dir string	*/
/* it also saves the pattern in savepat string				*/
/************************************************************************/
strippat()
  { register int i;

    for (i=strlen(dirstr)-1;i>0;i--)
       if ((dirstr[i] == '\\') || (dirstr[i] == ':'))
         { savepat[0] = '\0';
	   strcat(savepat,&dirstr[i+1]);
	   dirstr[i+1] = '\0';
	   i = 0;
	 }
  }

/************************************************************************/
/* patmatch matches string with the current pattern in directory	*/
/************************************************************************/
patmatch(instr)
char instr[];
  { register int i;

    for (i=strlen(dirstr)-1;i>0;i--)
       if ((dirstr[i] == '\\') || (dirstr[i] == ':'))
         { savepat[0] = '\0';
	   strcat(savepat,&dirstr[i+1]);
	   i = 0;
	 }
    checkmatch(savepat,instr);
}

/************************************************************************/
/* checkmatch checks pattern in matchstr to instr and returns		*/
/************************************************************************/
checkmatch(matchstr,instr)
char matchstr[],instr[];
  { register int match,i,j;
    char expmtstr[12],expinstr[12];

    exp_str(matchstr,expmtstr);
    exp_str(instr,expinstr);

    match = TRUE;
    for (i=0;i<8;i++)
      { if (expmtstr[i] == '*')
	  i = 8;
        else
	  { if (expmtstr[i] != '?')
	      if (instr[i] != expmtstr[i])
	        { i = 8;
	          match = FALSE;
	        }
	  }
      }
    if (match == TRUE)
      for (i=8;i<11;i++)
        { if (expmtstr[i] == '*')
	    i = 11;
          else
	    { if (expmtstr[i] != '?')
	        if (instr[i] != expmtstr[i])
	          { i = 11;
	            match = FALSE;
	          }
	    }
      }
  return(match);
}
/************************************************************************/
/* exp_str expands a string to filename format				*/
/************************************************************************/
exp_str(instr,outstr)
char instr[],outstr[];
{ register int i,j;

  for (i=0;i<11;i++)
    outstr[i] = ' ';

  outstr[11] = '\0';

  i = 0;
  for (j=0;j<12,instr[j] != '\0';j++)
    { if (instr[j] == '.')	/* expand out matching string */
	i = 7;
      else if (i < 11)
        outstr[i] = instr[j];
      i++;
     }
}

/************************************************************************/
/* seldesfile selects or deselects file name depending on cur setting	*/
/************************************************************************/
seldesfile(inobj)
int inobj;
  { register int i,j;

    if ((get_bit(FSELECT,FSELSEL,DISABLED) & DISABLED) == DISABLED)
      return;
    fstrsptr = &fstrs[(inobj-FSNAME1)+dirstart];
    if (fstrsptr->selnum != 0)
      { dselfile(inobj);
	newnames();
      }
    else
      { if (selfile(inobj) == 99)
	  form_alert(1,
	    "[3][ Only 99 selections | can be made at | one time. ][ OK ]");
      }
   objc_draw(form_addr,inobj,0,fo_gx,fo_gy,fo_gw,fo_gh);
}     

/************************************************************************/
/* dselfile deselects the file if not already zero			*/
/* fstrsptr is already set up to point to file to be deselected		*/
/************************************************************************/
dselfile(inobj)
int inobj;
 {  register int i,j;
    char tempstr[3];

    j = fstrsptr->selnum;
    if ((j == 0) || (fstrsptr->fname[5] == '\0'))
      return(0);	/* no need for this routine in this case */
    fstrsptr->selnum = 0;
    fstrsptr->fname[0] = ' ';
    fstrsptr->fname[1] = ' ';
    numsels -= 1;
    for (i=0;i<numfiles;i++)
      { fstrsptr = &fstrs[i];
        if (fstrsptr->selnum > j)
          { fstrsptr->selnum -= 1;
            uitoa(fstrsptr->selnum,tempstr);
            fstrsptr->fname[0] = tempstr[0];
	    if (tempstr[1] != '\0')
              fstrsptr->fname[1] = tempstr[1];
	    else 
	      fstrsptr->fname[1] = ' ';
	   }
      }
    fstrsptr = &fstrs[(inobj-FSNAME1)+dirstart];
    return(0);
 }

/************************************************************************/
/* selfile selects the file if zero					*/
/* fstrsptr is already set up to point to file to be selected		*/
/************************************************************************/
selfile(inobj)
int inobj;
 {  register int i,j;
    char tempstr[3];

    if ((fstrsptr->selnum != 0) || (fstrsptr->fname[5] == '\0'))
      return(0);

    j = 0;
    for (i=0;i<numfiles;i++)
      { fstrsptr = &fstrs[i];
        if (fstrsptr->selnum > j)
          j = fstrsptr->selnum;
      }
    if (j == 99)
      return(99);
    fstrsptr = &fstrs[(inobj-FSNAME1)+dirstart];
    fstrsptr->selnum = j+1;
    uitoa(j+1,tempstr);
    fstrsptr->fname[0] = tempstr[0];
    if (tempstr[1] != '\0')	/* <= 9 == one digit */
      fstrsptr->fname[1] = tempstr[1];
    numsels += 1;
    return(0);
  }

/************************************************************************/
/* newnames redoes the names in the field				*/
/************************************************************************/
newnames()
  { register int i;

    for (i=0;i<9;i++)
      { fstrsptr = &fstrs[i+dirstart];
        if ((fstrsptr->fname[5] == '\0') || ((i+dirstart) >= numfiles))
          { set_text(FSELECT,FSNAME1+i,"     ",TRUE); /* zero names */
	    fstrsptr->fname[0] = '\0';
	    strcat(fstrsptr->fname,"     ");
	  }
        else 
          { strip_spc(&(fstrsptr->fname[5]));
      	    set_text(FSELECT,FSNAME1+i,fstrsptr->fname,TRUE);
          }
       }
  }

/************************************************************************/
/* myfread reads a cluster into memptr from file			*/
/************************************************************************/
myfread(memptr)
char	*memptr;
{  register int i;

   i = bpbptr->datrec+((fileclust-2) * 2);
   error = myrwabs(2,memptr,2,i,curdrive);
   if (error == 0)
     fileclust = get_fatword(fileclust);
}

/************************************************************************/
/* myfcreate creates a file 						*/
/************************************************************************/
myfcreate(name,fsect,fent)
char	*name;
int	*fsect,*fent;
{ int	retval;
  register int i,j;

  do 
    { retval = do_ffind(name,0,fsect,fent,&newfile,isroot);
      if (retval == -1)
        return(-1);
      if ((retval == 2) && (isroot == TRUE))
        { form_alert(1,
	      "[3][ No more room in root | directory for filename.][Abort]");
          return(-1);
        }
      else if (retval == 2)
        { i = dirclust;
          do {
	    j = i;
	    i = get_fatword(j);
	    }
          while (i != -1);
          i = get_nfree();
          if (i != -1)
	    { set_fatword(j,i);
	      set_fatword(i,-1);
	      upd_fat();
	      if (dir_error())
	        return(-1);
	      newfile = FALSE;
	    }
	  else
	    { form_alert(1,
	      "[3][ Error trying to create | file!  Disk is full. ][Abort]");
              return(-1);
            }
	}
      else if (newfile == FALSE)
	{ if ((dirfiles->fattr & 0x10) == 0x10)
	    { form_alert(1,
"[3][ Unable to create file! | A directory posseses that name.][Abort]");
	      return(-1);
	    }
	  festr1[5] = '\0';
	  sqfname(name);
	  strcat(festr1,name);
	  strcat(festr1,festr2);
	  i = form_alert(2,festr1);
	  if (i == 2)
	    return(-1);
	  i = ibmword(dirfiles->fsclust);
	  do {
	    i = set_fatword(i,0);
	    }
	  while (i != -1);
	  if (dir_error())
	    return(-1);
	  newfile = TRUE;	  
	}
     }
   while (newfile == FALSE);
   i = get_nfree();
   if (i != -1)
     { dirfiles->fname[0] = '\0';
       exp_str(name,dirfiles->fname);
       dirfiles->fsclust = ibmword(i);
       set_fatword(i,-1);
       upd_fat();
       if (dir_error())
         return(-1);
     }
   else
     return(DISKFULL);
  return(0);
}
          
/************************************************************************/
/* myfwrite writes a cluster from memptr to file			*/
/************************************************************************/
myfwrite(memptr)
char	*memptr;
{  register int i,j;

   if (fileclust != -1)
     i = ((fileclust-2)*2)+bpbptr->datrec;
   else
     { j = get_nfree();
       if (j == -1)
         return(DISKFULL);		/* disk full error */
       i = ((j-2)*2)+bpbptr->datrec;
       set_fatword(j,-1);
       set_fatword(prfclust,j);
       prfclust = j;
     }
   error = myrwabs(3,memptr,2,i,curdrive);
   if ((error == 0) && (fileclust != -1))
     { prfclust = fileclust;
       fileclust = get_fatword(fileclust);
     }
}

/************************************************************************/
/* get_dirptr gets a directory pointer to use with myfread		*/
/************************************************************************/
get_dirptr(name)
char name[];
{ register int i,j,k,l;
  int dum,outerr;
  
  if (name[1] == ':')
    { name[0] = toupper(name[0]);
      curdrive = name[0] - 'A';
    }
  else
    curdrive = defdrive;

  diskinfo(curdrive);
  isroot = TRUE;
  l = 0;
  dirfiles = &dfptr[0];
  dirfiles->fsclust = 0;
  curclust = 0;
  j = strlen(name);

  if (bpbptr->fsiz <= 6)
    error = myrwabs(2,fatsecs,bpbptr->fsiz,bpbptr->fatrec,curdrive);
  if (dir_error())
    return(-1);

  k = 0;
  for (i=0;i<j;i++)
    { if (name[i] == '\\')	/* start of a directory? */
        k += 1;
      else if (k==1)
	tempstr[l++] = name[i];
      if (k==2)
	{ k = 1;
	  tempstr[l] = '\0';
	  l = 0;
	  do_ffind(tempstr,0,&dum,&dum,&newfile,isroot);
	  if (error != 0)
	    return;
	  if (newfile == TRUE)
	    { dirfiles->fsclust = -1;
	      return;
	    }
	  curclust = ibmword(dirfiles->fsclust);
	  isroot = FALSE;
	}
    }
}

/************************************************************************/
/* do_ffind finds a file or allocates a new filename 			*/
/************************************************************************/
do_ffind(name,attr,outsec,outent,newfile,isroot)
char *name;
int attr,*outsec,*outent,*newfile,isroot;
{ long longmem;
  register int i,j,k;
  int wrsec,wrent;
  int fdfsec,fdfent;
  int eof;
  struct dirfiles *fileptr;
  
  dirfiles = &dfptr[0];

   wrsec = 0;
   fdfsec = 0;
   wrent = -1;
   fdfent = -1;
   j = 0;
   eof = FALSE;
   *newfile = FALSE;
   do {
     i = 0;
     wrsec = j;
     do {
     next_sect(dirfiles+((long)(i)*16),j,&eof,isroot);
     if (error)
       return(-1);

      i += 1;
      j += 1;
    }
    while ((i < (maxdirs * 2)) && (!eof));

  i = (i*16);

  for (k=0;k<i;k++)
    { fileptr = &dirfiles[k];
      if (((checkmatch(name,fileptr->fname)) && 
	    ((fileptr->fattr & attr) == attr)) || (fileptr->fname[0] == 0))
	  { wrent = k;
	    wrsec += k/16;
	    k = i;
	    eof = TRUE;
	    if (fileptr->fname[0] == 0)
	      *newfile = TRUE;
	  }
      if ((fileptr->fname[0] == 0xe5) && (fdfent == -1))
	{ fdfent = k % 16;
	  fdfsec = wrsec + (k/16);
	}
     }
   }
  while (!eof);

  if ((wrent == -1) && (fdfent == -1))
    return(2);

  if (wrent == -1)
    { next_sect(dirfiles,fdfsec,&eof,isroot);
      if (dir_error())
	return(-1);

      fileptr = &dirfiles[fdfent];
      fileptr->fname[0] = '\0';
      wrsec = 0;
      wrent = fdfent;
      *newfile = TRUE;
    }
   
  dirfiles = &dirfiles[wrent];
  *outsec = wrsec;
  *outent = wrent;
  return(0);
}

/************************************************************************/
/* next_sect gets the next sector for the root or a directory entry	*/
/************************************************************************/
next_sect(memptr,secnum,eof,isroot)
char *memptr;
int secnum,*eof,isroot;
{ 
  if (isroot)
    { error = myrwabs(2,memptr,1,(bpbptr->datrec-bpbptr->rdlen)+secnum,curdrive);
      if (secnum == bpbptr->rdlen)
	*eof = TRUE; 
      else
	*eof = FALSE;
      return;
    }

    logsec = bpbptr->datrec + ((curclust-2) * 2) + (secnum & 1);
    error = myrwabs(2,memptr,1,logsec,curdrive);
    if (secnum & 1)
      curclust = get_fatword(curclust);
    if (curclust == -1)
      *eof = TRUE;
    else
      *eof = FALSE;
}

/************************************************************************/
/* strcmp string compare routine					*/
/************************************************************************/
strcmp(str1,str2)
char str1[],str2[];
 { register int i,j;

   i = 0;
   while (str1[i] == str2[i])
     if (str1[i++] == '\0')
       return(0);

   j = (int)(str1[i]) & 0xff;
   j -= (int)(str2[i]) & 0xff;
   return(j);
  }

/************************************************************************/
/* sqfname squeezes the spaces out of filenames at the right places	*/
/************************************************************************/
sqfname(inname)
char inname[];
{ char tempstr[10],extstr[4];
  register int i,j;

  i = 0;
  while ((i < 8) && (inname[i] != '.') && (inname[i] != '\0'))
    { tempstr[i] = inname[i];
      i++;
    }

  tempstr[i] = '\0';
  if (inname[i] == '.')
    i++;
  j = i;
  while ((j < (i+3)) && (inname[j] != '\0'))
    extstr[(j-i)] = inname[j++];

  extstr[(j-i)] = '\0';
  strip_spc(tempstr);
  strip_spc(extstr);
  
  j = strlen(tempstr);
  if (extstr[0] != '\0')
    { tempstr[j++] = '.';
      tempstr[j++] = '\0';
    }
  inname[0] = '\0';
  strcat(inname,tempstr);
  strcat(inname,extstr);
}

/************************************************************************/
/* memerrout puts out an error message for lack of memory		*/
/************************************************************************/
memerrout()
{
  form_alert(1,"[3][ Insufficient memory for | use with file(s). ][Abort]");
}

/************************************************************************/
/* noffind puts out a form telling that a file couldn't be found	*/
/************************************************************************/
noffind(name)
char	name[];
{ 
  sqfname(name);
  nofstr1[22] = '\0';
  strcat(nofstr1,name);
  strcat(nofstr1,nofstr2);
  form_alert(1,nofstr1);
}

