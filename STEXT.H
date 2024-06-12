extern 	int 	defdrive;
extern	int	gl_hchar;
extern	int	gl_wchar;
extern	int	gl_wbox;
extern	int	gl_hbox;	/* system sizes */

extern	int 	phys_handle;	/* physical workstation handle */
extern	int 	handle;		/* virtual workstation handle */
extern	int	wi_handle;	/* window handle */
extern	int	top_window;	/* handle of topped window */

extern	int	xdesk,ydesk,hdesk,wdesk;
extern	int	xold,yold,hold,wold;
extern	int	xwork,ywork,hwork,wwork;	/* desktop and work areas */

extern	int	msgbuff[8];	/* event message buffer */
extern	int	keycode;	/* keycode returned by event-keyboard */
extern	int	mx,my;		/* mouse x and y pos. */
extern	int	butdown;	/* button state tested for, UP/DOWN */
extern	int	ret;		/* dummy return variable */

extern	int	hidden;		/* current state of cursor */

extern	int	fulled;		/* current state of window */

extern	int	contrl[12];
extern	int	intin[128];
extern	int	ptsin[128];
extern	int	intout[128];
extern	int	ptsout[128];	/* storage wasted for idiotic bindings */

extern	int 	work_in[11];	/* Input to GSX parameter array */
extern	int 	work_out[57];	/* Output from GSX parameter array */
extern	int 	pxyarray[10];	/* input point array */

extern	char	undo_area[32000];	/* undo area for screen */

extern	int	cursor,line;	/* cursor position */

extern	int	fo_gx,fo_gy,fo_gw,fo_gh;	/* global form sizes */
extern	long	form_addr;
extern	OBJECT	*sel_obj;
extern	int	exit_obj;	/* object that was selected to exit with */

extern	long	gl_menu;	/* global menu pointer in this case 1 form */

extern	int	error;		/* error return code */
extern	long	max_tracks;	/* maximum tracks of allocated memory */

extern	OBJECT	*obj_ptr;
extern	TEDINFO	*txt_ptr;

extern	int	num_flops,cur_flop;	/* drive input from system call */

extern	int	tr_start,tr_end;	/* track start and end pointers */
extern	int	si_start,si_end;
extern	int	i,event;

extern	int	gr_mkmx,gr_mkmy,gr_mkmstate,gr_mkkstate;
extern	int	obj_selected,new_obj,exit_prog;
extern	int	bcon,scfon,scnfon,dsson;
extern	int	orginal,duplicate;

extern	long	gl_mw;
extern	int	globntracks,globnsecs,globnsides;
extern	int	x,y,w,h;

extern	int	rez,version,menuedit;
extern	int	track,sector,side;
extern	int	restdr[3],readok,xcurs,ycurs,editon;
extern	int	readon;

extern  int	curedit,scrtype,firdatcl;

extern	char	diskbuff[1024];

extern	char	verstr1[];
extern	char	verstr2[];

extern	char	ins_str[];
extern	char	st_str[];

extern	char	seastr[];
extern	char	seahex[];
extern	char	pval_shex[2][];
extern	char	ptmp_shex[2][];
extern	unsigned int logsec;

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
	} extern *bpbptr;



#ifndef STMOD4
extern	char	*fatsecs;
extern	char	fatbuff[6144];
extern	char	filenames[2574];
extern	char	dirbuff[2048];

extern	struct	fstrs {
	  int dirfnum;			/* directory entry number */
	  int selnum;
	  char fname[17];
        } *fstrs;

extern 	struct fstrs *fstrsptr;

extern	struct	dirfiles {
	  char 	fname[8];
	  char 	fext[3];
	  char 	fattr;
	  char 	fresvrd[10];
	  unsigned int ftime;
	  unsigned int fdate;
	  unsigned int fsclust;
	  long	fsize;
	} *dfptr;

extern	struct dirfiles *dirfiles;

extern 	char	dirstr[41];
extern	char	holdstr[41];
extern	char	filestr[13];
extern	int 	lafatsec,numfiles,dirstart,maxdirs,numskipped;
extern	char 	savepat[14];
extern	int	numsels,curdrive,driv1,driv2;
extern	int	curclust,newfile;
extern	char	eraser1[70];
extern	char	eraser2[];

extern	char	rener1[70];
extern	char	rener2[];

extern	char	timestr[7],datestr[8];
extern	char	chster1[80];
extern	char	chster2[];

extern	char	edstr1[80];
extern	char	edstr2[];
extern	char	edstr3[];
extern	char	fsstr1[];

extern	char	fmstr1[80];
extern	char	fmstr2[];

extern	int	fnumcl,filenum,globftype;
extern	unsigned int fileclust,dirclust;
extern	int	filesec,prfclust;
extern	long	fsize;
extern	char	headstr[30];

extern	char	prstr1[];
extern	char	prstr2[90];
extern	char	prstr3[];
extern	char	prstr4[];

extern	char	vfstr1[80];
extern	char	vfstr2[];
extern	char	vfstr3[];

extern	char	movstr1[80];
extern	char	movstr2[];
extern	char	copstr1[];
extern	char	fcomstr1[];
extern	char	difstr[];

extern	char	festr1[70];
extern	char	festr2[];

extern	struct	bpb *bpbp1,*bpbp2;
extern	struct	dirfiles *dir1f;
extern	unsigned int dir1cl,dir2cl;
extern	char	soustr[30];
extern	char	deststr[30];
extern	char	fmheadstr[];
extern	char	nofstr1[];
extern	char	nofstr2[];
extern	char	fsizerr[];

extern	char	fnoma1[100];
extern	char	fnoma2[];

extern	long	slibox;
extern	int	isroot,fatu_on,upd_sec;
extern	char	*upd_mem;

#endif

extern	char	mapstr1[],mapstr2[];
extern  int	cformat(),cerase();
extern	long	qcompare();

/************************************************************************/
/* global defines for all modules					*/
/************************************************************************/

#define DISKEDIT 0	/* available routines to use edit */
#define FILEEDIT 1
#define SEAREDIT 2
#define UNEREDIT 3

long	qsearch();
