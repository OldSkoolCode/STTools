/************************************************************************/
/*           set up relation between parents and children 		*/
/************************************************************************/

extern 	int global[];

set_rsrc()

{ struct foobar *imagptr;
  OBJECT *rsrcptr;
  OBJECT *objptr;
  register int i,j;
  register long k;

  graf_mouse(HOURGLASS,0X0L);	/* turn off mouse just for kicks */

  for (i=0;i<NUM_TREE;i++)
    { rs_trindex[i] = &(rs_object[rs_trindex[i]]);
      objptr = rs_trindex[i];
      do {      
            for (j=objptr->ob_head-(objptr == rs_trindex[i]);
			j<=objptr->ob_tail;j++)
	      rsrc_obfix(rs_trindex[i],j);
            objptr += (objptr->ob_tail-(objptr->ob_head-1));
	  }
      while (objptr->ob_tail != -1);
      
    }
  k = (long)(rs_trindex);		/* pointer to pointer stupid GEM */
  global[5] = (int)(k >> 16);		/* set in global so GEM */
  global[6] = (int)(k);			/* knows where it is at */

  for (i=0;i<NUM_OBS;i++)
    { 
      rsrcptr = &rs_object[i];	/* get address of object number i */
      j = rsrcptr->ob_type;	/* get the object type */
      switch (j)
        {
	  case G_TEXT:
	  case G_BOXTEXT:
	  case G_FTEXT:
	  case G_FBOXTEXT:
	    rsrcptr->ob_spec = &rs_tedinfo[rsrcptr->ob_spec]; /* link tedinfo */
	    rsrcptr = rsrcptr->ob_spec;	/* get pointer to tedinfo */
	    rsrcptr->te_ptext = rs_strings[rsrcptr->te_ptext]; /* link text */
	    rsrcptr->te_ptmplt = rs_strings[rsrcptr->te_ptmplt]; /* template */
	    rsrcptr->te_pvalid = rs_strings[rsrcptr->te_pvalid]; /* valid */
	    break;

	  case G_BUTTON:
          case G_STRING:
	  case G_TITLE:
	    rsrcptr->ob_spec = rs_strings[rsrcptr->ob_spec]; /* link text */
	    break;

	  case G_IMAGE:
	    rsrcptr->ob_spec = &rs_bitblk[rsrcptr->ob_spec]; /* link image */
	    break;

	  case G_ICON:
	    rsrcptr->ob_spec = &rs_iconblk[rsrcptr->ob_spec]; /* set icon ptr */
	    rsrcptr = rsrcptr->ob_spec;	/* get mask data number */
	    imagptr = &rs_imdope[rsrcptr->ib_pmask];	/* get icon mask ptr */
	    rsrcptr->ib_pmask = imagptr->image;	/* set in icon structure */
	    imagptr = &rs_imdope[rsrcptr->ib_pdata];	/* get data ptr */
	    rsrcptr->ib_pdata = imagptr->image;		/* set data ptr */
	    rsrcptr->ib_ptext = rs_strings[rsrcptr->ib_ptext]; /* set text ptr*/
	    break;
        }  
    }

  graf_mouse(ARROW,0x0L);
}

