*************************************************************************
* 	STTOOLS ASSEMBLER FILE 7/01/86	KENNETH L. HURLEY		*
*************************************************************************

*************************************************************************
*	Equates section starts here					*
*************************************************************************

diskctl	equ	$ffff8604	; disk contoler data access
fifo	equ	$ffff8606	; DMA mode control/status
dmahigh	equ	$ffff8609	; DMA base high
dmamid	equ	$ffff860b	; DMA base medium
dmalow	equ	$ffff860d	; DMA base low
flock	equ	$43e		; lock floppies for no vblank
_frclock equ	$466		; count of clock cycles
vblsem	equ	$452		; exclusion of vblanks
nvbls	equ	$454		; number of vblanks in table
_vblqueue equ	$456		; pointer to table

*------ GI sound chip
giselect equ	$ffff8800	; sound chip register select
giread	equ	$ffff8800	; sound chip read-data
giwrite	equ	$ffff8802	; sound chip write-data
giporta	equ	$e		; GI register fo I/O port A

*------ 1772 select values
cmdreg	equ	$80		; select the command register
trkreg	equ	$82		; select the track register
secreg	equ	$84		; select the sector register
datareg	equ	$86		; select the data register

*------	68901 "mfp" flouride chip
mfp	equ	$fffffa00	; mfp base address
gpip	equ	mfp+1		; general porpoise I/O

*------	miscellaneous equates
timeout	equ	$40000		; short time out wait number
ltimeout equ	$60000		; long time out wait number
read_whole equ	$e0		; read whole track command
write_whole equ	$f0		; write whole track command
lineainit equ	$a000		; initialize line a

	.text
	.globl	_main
	.globl	_ctrl_cnts
	.globl	_crystal
	.globl	_nflops
	.globl	_crest
	.globl	_cread_one
	.globl	_cwrite_one
	.globl	_cformat
	.globl	_cerase
	.globl	_outscreen
	.globl	_read_sector
	.globl	_write_sector
	.globl	_diskbuff
	.globl	_docursor
	.globl	_outchar
	.globl	_scrtype
	.globl	_charstrout
	.globl	_rez
	.globl	_qcompare
	.globl	_qsearch
	.globl	_uitoa
	.globl	_ltoa
	.globl	_strlen

*************************************************************************
* start of the program area is here					*
*************************************************************************

startup:
	move.l	sp,a5		; save a7 so we can get the base address
	move.l	#ustk,sp	; set local stack
	move.l	4(a5),a5	; get basepage address
	move.l	$c(a5),d0
	add.l	$14(a5),d0
	add.l	$1c(a5),d0	; add in bss and data segment lengths
	add.l	#$100,d0	; plus base page
	move.l	d0,-(sp)	; shrink number of bytes
	move.l	a5,-(sp)
	move	d0,-(sp)	; junk word
	move	#$4a,-(sp)	; Mshrink call
	trap	#1
	add.l	#12,sp

	dc.w	lineainit	; initialize line a variables
	move.l	$4(a1),a0
	move.l	$4c(a0),d0
	move.l	d0,fonttable	; set where 8x8 system font is

	move.w	#3,-(sp)	; get current screen location
	trap	#14		; call xbios
	addq	#2,sp		; clean up
	move.l	d0,screen1	; move into screen 1 location
	
	bsr	super_in	; into supervisor mode
	clr.w	vblsem		; not while i'm setting up
	clr.w	d1
femptvbl:
	move.w	d1,d0
	lsl.w	#2,d0		; offset into table
	move.l	_vblqueue,a0	; get pointer to vblanks
	add.w	d0,a0		; point to next open slot
	tst.l	(a0)		; is it blank?
	beq	emptyslot	; found one alright!
	addq	#1,d1
	cmp.w	nvbls,d1	; are there no more slots?
	blt	femptvbl	; find next slot
	bra	byebye		; no empty slots so exit
emptyslot:
	move.l	#flopvbl,(a0)	; set my vblank slot
	move.l	a0,slotaddr	; save slot address
	move.w	#1,vblsem	; reset vertical blanks
	move.w	#trkreg,fifo	; select track register
	jsr	super_out

	jsr 	_main		; go do what you want to

	jsr	super_in
	clr.w	vblsem		; not when i'm here
	move.l	slotaddr,a0
	clr.l	(a0)		; empty my slot	
	move.w	#1,vblsem
	jsr	super_out
byebye:
	move.l	#0,-(sp)	; exit current process
	trap	#1

*************************************************************************
* writes command to the disk controller must be in super mode		*
*************************************************************************

wdiskctl:
	bsr	rwdelay		; need some delay
	move.w	d7,diskctl	; write the sucker
	bra	rwdelay		; exit through delay

*************************************************************************
* read command from the controller					*
*************************************************************************

rdiskctl:
	clr.l	d0
	bsr	rwdelay		; delay
	move.w	diskctl,d0	; get it
	and.w	#$00ff,d0	; only byte counts
	bra	rwdelay

*************************************************************************
* rwdelay dalys for a while						*
*************************************************************************

rwdelay:
	move	sr,-(sp)	; save flags 
	move.w	d7,-(sp)	; save counter register
	move.w	#$20,d7		; $20 should be enoug
rwdly1:
	dbra	d7,rwdly1	; loop giving 1772 some time to itself
	move.w	(sp)+,d7	; recover register
	move	(sp)+,sr	; recover status register
	rts

*************************************************************************
* _read_sector reads a sector from the disk into diskbuff		*
*************************************************************************
_read_sector:
	bsr	save_regs	; save the registers for 'C'
	bsr	super_in	; get into supervisor mode
	st	flock
	move.w	4(sp),track_num	; get the track number
	move.w	6(sp),d3	; the sector number
	move.w	8(sp),side_no	; get the side number
	move.w	10(sp),dev_no	; the device number
	move.l	12(sp),sec_addr	; set xfer address
	bsr	select		; select the drive
	move.w	dev_no,d0
	bsr	set_trak	; set track for this device
	bsr	hard_seek
	clr.w	d0
	bsr	read_one	; go read the sector
	sf	flock
	bsr	super_out
	bsr	rest_regs
	rts

*************************************************************************
* _write_sector writes a sector from diskbuff to the disk 		*
*************************************************************************
_write_sector:
	bsr	save_regs	; save the registers for 'C'
	bsr	super_in	; get into supervisor mode
	st	flock
	move.w	4(sp),track_num	; get the track number
	move.w	6(sp),d3	; the sector number
	move.w	8(sp),side_no	; get the side number
	move.w	10(sp),dev_no	; the device number
	move.l	12(sp),sec_addr	; set xfer address
	bsr	select		; select the drive
	move.w	dev_no,d0
	bsr	set_trak	; set track number for this device #
	bsr	hard_seek
	clr.w	d0
	move.l	#$200,d4
	bsr	write_one	; go write the sector
	sf	flock
	bsr	super_out
	bsr	rest_regs
	rts

*************************************************************************
* _cread_one is a routine callable from 'C' that will read a sector 	*
*************************************************************************
_cread_one:
	bsr	save_regs	; save the registers for 'C'
	bsr	super_in	; get into supervisor mode
	st	flock
	move.w	4(sp),track_num	; set the track number
	move.w	6(sp),side_no
	move.w	8(sp),dev_no
	move.l	10(sp),sec_addr	; xfer point
	bsr	select		; select the drive
	move.w	dev_no,d0
	bsr	set_trak	; set track number for this device #
	bsr	hard_seek
	clr.l	d6
	moveq	#1,d3		; sector number
cread_l1:
	move.l	#$200,d4
	bsr	read_one	; go write the sector
	or.l	d0,d5
	add.l	#$200,sec_addr	; next sector address 
	addq	#1,d3
	cmp.w	#9,d3
	ble	cread_l1
	move.w	dev_no,d0
	bsr	set_trak	; set track for this device
	bsr	hard_seek	; reseek w/o verify
	move.w	d5,d0
	sf	flock
	bsr	super_out
	bsr	rest_regs
	rts

*************************************************************************
* read_one reads one sector returns error in d0,d3=sector		*
*************************************************************************
read_one:
	move.w	#3-1,d1
st_read:
	move.w	#trkreg,fifo	; set track register
	move.w	track_num,d7	; move in track number
	bsr	wdiskctl	; set track
	move.w	#secreg,fifo
	move.w	d3,d7		; sector number
	bsr	wdiskctl

	move.l	#$200,a2
	add.l	sec_addr,a2	; end of DMA
	move.l	sec_addr,tmpdma
	move.b	tmpdma+3,dmalow
	move.b	tmpdma+2,dmamid
	move.b	tmpdma+1,dmahigh	; set DMA address

	move.w	#$090,fifo
	move.w	#$190,fifo
	move.w	#$090,fifo	; toggle DMA to clear
	move.w	#1,diskctl	; number of 512 sector max xfer
	move.w	#$80,fifo
	move.w	#$80,d7		; set read one sector
	bsr	wdiskctl	; write the disk controller
	move.l	#timeout,d7
ro_l1:
	btst.b	#5,gpip		; see if controller done
	beq	ro_l3
	subq.l	#1,d7
	beq	ro_err		; timeout error
	move.b	dmahigh,tmpdma+1
	move.b	dmamid,tmpdma+2
	move.b	dmalow,tmpdma+3	; get dma pointer
	cmp.l	tmpdma,a2
	bgt	ro_l1
ro_l3:
	move.w	#$090,fifo	; exmain dma
	move.w	fifo,d0
	btst	#0,d0		; any errors
	beq	ro_err

	move.w	#cmdreg,fifo
	bsr	rdiskctl
	and.b	#$18,d0		; check for read errors
	bra	ro_l4
ro_err:
	bsr	reset_disk
	dbra	d1,st_read
	move.b	#$01,d0		; set an error
ro_l4:
	rts

*************************************************************************
* _cwrite_one is a routine callable from 'C' that will write a sector 	*
*************************************************************************
_cwrite_one:
	bsr	save_regs	; save the registers for 'C'
	bsr	super_in	; get into supervisor mode
	st	flock
	move.w	4(sp),track_num	; set the track number
	move.w	6(sp),side_no
	move.w	8(sp),dev_no
	move.l	10(sp),sec_addr	; set xfer address
	bsr	select		; select the drive
	move.w	dev_no,d0
	bsr	set_trak	; set track number for this device #
	bsr	hard_seek
	clr.l	d6
	moveq	#1,d3		; sector number
cwrite_l1:
	move.l	#$200,d4
	bsr	write_one	; go write the sector
	or.l	d0,d5
	add.l	#$200,sec_addr
	addq	#1,d3
	cmp.w	#9,d3
	ble	cwrite_l1
	move.w	dev_no,d0
	bsr	set_trak	; set track for this device
	bsr	hard_seek	; reseek w/o verify
	move.w	d5,d0
	sf	flock
	bsr	super_out
	bsr	rest_regs
	rts

*************************************************************************
* write_one writes one sector returns error in d0,d3=sector		*
*************************************************************************
write_one:
	move.w	#3-1,d1
st_write:
	move.w	#trkreg,fifo	; set track register
	move.w	track_num,d7	; move in track number
	bsr	wdiskctl	; set track
	move.w	#secreg,fifo
	move.w	d3,d7		; sector number
	bsr	wdiskctl

	move.l	sec_addr,tmpdma
	move.b	tmpdma+3,dmalow
	move.b	tmpdma+2,dmamid
	move.b	tmpdma+1,dmahigh	; set DMA address

	move.w	#$190,fifo
	move.w	#$090,fifo
	move.w	#$190,fifo	; toggle DMA to clear
	move.w	#$01,d7
	bsr	wdiskctl	; number of sectors
	move.w	#$180,fifo
	move.w	#$a0,d7
	bsr	wdiskctl	; write the disk controller
	move.l	#timeout,d7

wro_l1:
	btst.b	#5,gpip		; see if controller done
	beq	wro_l3
	subq.l	#1,d7
	bne	wro_l1		; timeout error
	bra	wro_err

wro_l3:
	move.w	#$180,fifo
	bsr	rdiskctl
	and.b	#$5c,d0		; check for write errors
	bra	wro_l4
wro_err:
	dbra	d1,st_write
	bsr	reset_disk
	move.b	#$01,d0		; set an error
wro_l4:
	rts

*************************************************************************
* _cformat formats the desired track					*
*************************************************************************
_cformat:
	bsr	save_regs
	bsr	super_in	; into supervisor
	st	flock
	move.w	4(sp),track_num	; get the track number
	move.w	6(sp),side_no	; side number is it
	move.w	8(sp),dev_no
	bsr	select		; select device
	move.w	dev_no,d0
	bsr	set_trak	; set track number for this device
	bsr	hard_seek	; seek to the track
	bsr	format
	sf	flock
	bsr	super_out	; out of supervisor mode
	bsr	rest_regs	; back to other registers
	rts

*************************************************************************
* _cerase erases the desired track					*
*************************************************************************
_cerase:
	bsr	save_regs
	bsr	super_in	; into supervisor
	st	flock
	move.w	4(sp),track_num	; get the track number
	move.w	6(sp),side_no	; side number is it
	move.w	8(sp),dev_no
	bsr	select		; select device
	move.w	dev_no,d0
	bsr	set_trak	; set track number for this device
	bsr	hard_seek	; seek to the track
	move.w	#$680-1,d0
	lea	diskbuff,a0
cerl1:
	clr.l	(a0)+
	dbra	d0,cerl1
	bsr	formeras
	move.w	d0,-(sp)
	move.w	dev_no,d0
	bsr	set_trak	; set track for this device
	bsr	hard_seek	; reseek w/o verify
	move.w	(sp)+,d0
	sf	flock
	bsr	super_out	; out of supervisor mode
	bsr	rest_regs	; back to other registers
	rts

*************************************************************************
* format formats the track to the specified track			*
*************************************************************************
format:
	bsr	set_fmt		; set up the format buffer
formeras:
	move.l	#diskbuff,sec_addr
	move.b	sec_addr+3,dmalow
	move.b  sec_addr+2,dmamid
	move.b	sec_addr+1,dmahigh

	move.w	#$190,fifo	; set the disk->DMA
	move.w	#$090,fifo
	move.w	#$190,fifo
	move.w	#$1f,d7		; absurd sector count
	bsr	wdiskctl	; write it

	move.w	#$180,fifo	; select write track command
	move.w	#write_whole,d7	; write with pre-comp
	bsr	wdiskctl	; go do it

	move.l	#timeout,d7	; number to timeout to
ftw1:
	btst.b	#5,gpip		; is 1772 done yet?
	beq	ftw2
	subq.l	#1,d7
	bne	ftw1		; keep waiting until it is or timeout
	bsr	reset_disk	; whoops
fterr:
	moveq	#1,d0
	rts
ftw2:
	move.w	#$190,fifo
	move.w	fifo,d0		; toggle dma to get error
	btst	#0,d0		; any errors
	beq	fterr
	move.w	#$180,fifo	; disk->dma
	bsr	rdiskctl	; get the error that occured
	and.b	#$44,d0		; mask out errors
	rts

*************************************************************************
* set format sets the format buffer to format a track only called once	*
*************************************************************************
set_fmt:
	lea	diskbuff,a0	; set the buffer
	move.w	#1,last_sec	; set the first sector
	move.w	#60-1,d1	; track header
	move.b	#$4e,d0
	bsr	wmult		; go write it to buffer
setfm_l1:
	move.w	#12-1,d1
	clr.b	d0
	bsr	wmult		; set up the track
	move.w	#3-1,d1		; 3 X $F5
	move.b	#$f5,d0	
	bsr	wmult		; go do it
	move.b	#$fe,(a0)+
	move.w	track_num,d0	; get track number
	move.b	d0,(a0)+
	move.b	side_no,(a0)+	; set the side number
	move.w	last_sec,d0
	move.b	d0,(a0)+	; set the sector number
	move.b	#$02,(a0)+
	move.b	#$f7,(a0)+	; set checksum

	move.w	#22-1,d1	; 22 X $4e
	move.b	#$4e,d0
	bsr	wmult		; go do it
	move.w	#12-1,d1
	clr.b	d0
	bsr	wmult		; 12 X $00
	move.w	#3-1,d1
	move.b	#$f5,d0	
	bsr	wmult		; multiply the number

	move.b	#$fb,(a0)+	; set the data header
	move.w	#512-1,d1
	move.b	#$e5,d0
	bsr	wmult		; go do $E5e5 for data for now.
	move.b	#$f7,(a0)+	; check sum ok?
	move.w	#40-1,d1
	move.b	#$4e,d0		; number of gap bytes
	bsr	wmult

	move.w	last_sec,d0	; sector number
	add.w	#1,d0
	move.w	d0,last_sec	; set next sector
	cmp.w	#$09,d0		; are we pass end yet?
	ble	setfm_l1
	
	move.w	#1400,d1
	move.b	#$4e,d0
	bsr	wmult		; end of track of trailer

	rts


*************************************************************************
* set track sets the track for the current drive d0= device number	*
*************************************************************************
set_trak:
	tst	d0
	bne	set_t2
	move.w	dr1_trk,d7
	bra	set_t3
set_t2:
	move.w	dr2_trk,d7
set_t3:
	move.w	#trkreg,fifo	; set track write register
	bsr	wdiskctl
	rts

*************************************************************************
* hard_seek seeks to the track desired and stays there			*
*************************************************************************

hard_seek:
	move.w	dr2_trk,d0
	tst.w	dev_no
	bne	hardl1
	move.w	dr1_trk,d0
hardl1:
	cmp.w	track_num,d0
	beq	hardl5
	move.w	track_num,d7	; get the track number
	move.w	#datareg,fifo	; select the data register
	bsr	wdiskctl
	move.w	#$13,d7		; seek command with access time
	bsr 	flop_cmd	; go do the command
	move.w	track_num,d0
	tst.w	dev_no		; device = 0?
	bne	hardl2
	move.w	d0,dr1_trk	; set drive 1's track
	bra	hardl3
hardl2:
	move.w	d0,dr2_trk
hardl3:
	move.w	#$600,d7	; head settle time
hardl4:
	dbra	d7,hardl4
hardl5:
	rts

*************************************************************************
* flop_cmd executes the command with a timeout checker			*
*************************************************************************

flop_cmd:
	move.l	#timeout,d6	; timeout wait number
	move.w	#cmdreg,fifo	; command register select
	bsr	rdiskctl
	btst	#7,d0		; motor up to speed?
	bne	flop_c2
	move.l	#ltimeout,d6	; get long timeout number
flop_c2:
	bsr	wdiskctl	; write the command
flop_cw:
	subq.l	#1,d6
	beq	flop_cerr	; timeout error
	btst.b	#5,gpip		; check if 1772 is finished
	bne 	flop_cw		; go back to timeout loop
	bsr 	rdiskctl	; read error status
	clr.w	d7		; no error
	rts
flop_cerr:
	bsr	reset_disk	; reset the drive
	moveq	#1,d7		; set error code
	rts

*************************************************************************
* reset_disk reset the disk status					*
*************************************************************************

reset_disk:
	move.w	#cmdreg,fifo	; set to write to command
	move.w	#$d8,d7		; immediate reset
	bsr	wdiskctl	; do it
	move.w	#$d0,d7		; reset command
	bsr	wdiskctl
	move.w	#15,d7		; a short delay number
rd_loop:
	dbra	d7,rd_loop	; wait for a while
	bsr	rdiskctl	; read controller status
	rts

*************************************************************************
* restore takes the track to zero					*
*************************************************************************

_crest:
	jsr	save_regs
	jsr	super_in
	st	flock
	move.w	4(sp),dev_no	; get device number
	tst.w	dev_no
	bne	crset_t2
	clr.w	dr1_trk
	bra	crsel
crset_t2:
	clr.w	dr2_trk
crsel:
	bsr	select		; select the drive
	move.w	#trkreg,fifo	; select track register
	move.w	#$ff,d7
	bsr	wdiskctl	; write max track number
	move.w	#3-1,d3		; number of retries
restore:
	moveq	#15,d7		; restore command with motor spin up
	bsr	flop_cmd	; go do the comand
	bne	res_error	; timeout error
	btst	#2,d0		; test TRK00 bit
	eor	#$04,ccr	; flip the zero bit
	bne	res_error
	clr.w	d0
	bra	res_exit
res_error:
	dbra	d3,restore
	move.w	#1,d0
res_exit:
	sf	flock
	jsr	super_out
	jsr	rest_regs
	rts			; return to caller

*************************************************************************
* select selects the drive and side for access				*
*************************************************************************

select:
	move.w	dev_no,d0	; get the device number
	addq.b	#1,d0		; shift it to the right place
	lsl.b	#1,d0
	or.w	side_no,d0	; or in side number
	eor.b	#7,d0		; opposite for hardware
	and.b	#7,d0		; maks out misc. bits
	bsr	setporta	; go set them

	clr.w	deselflag	; deselect flags
	rts

*************************************************************************
* set porta set floppy bits in port a					*
*************************************************************************

setporta:
	move	sr,-(sp)	; save out butt
	or	#$0700,sr	; start critiacl section
	move.b	#giporta,giselect	; select port on gi chip
	move.b	giread,d1	; get current bits
	move.b	d1,d2		; save old bits for caller
	and.b	#$ff-7,d1	; and out those xtra bits
	or.b	d0,d1
	move.b	d1,giwrite	; or bits and write them
	move	(sp)+,sr	; restore IPL
	rts

*************************************************************************
* hand_err handles the error which came through				*
*************************************************************************

hand_err:
	moveq.l	#1,d7		; set an error
	rts


*************************************************************************
* wmult multiply byte in d0 by d1+1 into a0,a0+1,...			*
*************************************************************************
wmult:
	move.b	d0,(a0)+
	dbra	d1,wmult
	rts

*************************************************************************
* save registers for later restoring					*
*************************************************************************
save_regs:
	movem.l	d1-d7/a0-a2,reg_save	; save the registers
	rts

*************************************************************************
* restore the registers from previously saved				*
*************************************************************************
rest_regs:
	movem.l	reg_save,d1-d7/a0-a2	; restore dem
	rts

*************************************************************************
* go into super mode							*
*************************************************************************
super_in:
	clr.l	-(sp)		; want our own stack in super mode
	move.w	#$20,-(sp)	; get/set super mode
	trap	#1
	addq	#6,sp		; clean up
	move.l	d0,save_ssp	; save the old stack pointer
	rts

*************************************************************************
* get out of super mode							*
*************************************************************************
super_out:
	move.l	d0,save_d0
	move.l	save_ssp,-(sp)	; old stack pointer
	move.w	#$20,-(sp)	; get/set stack pointer
	trap	#1
	addq	#6,sp		; clean up
	move.l	save_d0,d0	; restore d0 register
	rts

*************************************************************************
* nflops gets the number of actual floppies available			*
*************************************************************************
_nflops:
	jsr	super_in	; super mode have to to access
	move.w	$4a6,d2		; get the number
	jsr	super_out
	move.w	d2,d0		; get the damn thing back 
	rts

*************************************************************************
* flopvbl to replace theres so we can access wp status and deselflag	*
*************************************************************************
flopvbl:
	tst.w	flock		; locked floppies?
	bne	fvblr		; bye bye

	tst.w	deselflag	; floppies already deselected?
	bne	fvblr
	move.w	#cmdreg,fifo	; select status and command register

	bsr	rdiskctl	; read 1772 status register
	btst	#7,d0		; motor still on?
	bne	fvblr		; nope in process
	move.b	#7,d0
	bsr	setporta	; turn motors off
	move.w	#1,deselflag	; indicate floppies deselected
fvblr:
	rts

*
* -----	_outscreen outputs a screen dump real quick like
*
_outscreen:
	movem.l	d2-d7/a2-a3,-(sp)	; save ones used
	
	jsr	super_in
	tst.w	_scrtype	; is it in hexidecimal
	bne	isittext	; nope so test for text output
	jsr	hexscrout	; go output hex screen
	bra	osdone		; out screen is done
isittext:
	cmp.w	#1,_scrtype	; is it in text
	bne	isitdir		; check for directory format
	jsr	textscrout	; dump text screen out
	bra	osdone
isitdir:
	jsr	dirscrout	; dump screen out in directory format
osdone:
	jsr	super_out
	movem.l	(sp)+,d2-d7/a2-a3	; restore registers
	rts

*
* -----	hexscrout dumps a screen out in hexidecimal
*
hexscrout:
	lea	diskbuff,a2	; buffer pointer
	move.w	#3,d6		; the y position
osl1:
	move.w	#1,d5		; the x position
osl2:
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1

	clr.w	d7
	move.b	(a2)+,d7	; get byte to output
	move.w	d7,d2		; into parameter register
	sub.w	#1,d3		; subtract orginal position
	lsr.w	#1,d3		; divide by 2 for nibbles output
	add.w	#52,d3		; where it should go
	jsr	outchar		; go output character
	
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1
	move.w	d7,d2		; into parameter register
	lsr.w	#4,d2		; get high nibble of byte
	lea	hexnums,a0	; get address of hex numbers
	move.b	(a0,d2.w),d2	; get byte to output
	jsr	outchar		; output the character
	move.w	d5,d3		; set x parameter
	move.w	d6,d1		; set y parameter
	add.w	#1,d3		; next x position
	move.w	d7,d2		; get byte to output
	and.w	#$f,d2		; mask out nibble
	lea	hexnums,a0
	move.b	(a0,d2.w),d2	; get ascii of hex nibble
	jsr	outchar

	cmp.w	#24,d6		; don't do these 8 if last line
	beq	osl3

	move.w	d5,d3		; get into parameter place
	move.w	d6,d1

	clr.w	d7
	move.b	7(a2),d7	; get byte to output
	move.w	d7,d2		; into parameter register
	sub.w	#1,d3		; subtract orginal position
	lsr.w	#1,d3		; divide by 2 for nibbles output
	add.w	#60,d3		; where it should go
	jsr	outchar		; go output character
	
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1
	add.w	#17,d3
	move.w	d7,d2		; into parameter register
	lsr.w	#4,d2		; get high nibble of byte
	lea	hexnums,a0	; get address of hex numbers
	move.b	(a0,d2.w),d2	; get byte to output
	jsr	outchar		; output the character
	move.w	d5,d3		; set x parameter
	move.w	d6,d1		; set y parameter
	add.w	#18,d3
	move.w	d7,d2		; get byte to output
	and.w	#$f,d2		; mask out nibble
	lea	hexnums,a0
	move.b	(a0,d2.w),d2	; get ascii of hex nibble
	jsr	outchar

	move.w	d5,d3		; get into parameter place
	move.w	d6,d1

	clr.w	d7
	move.b	15(a2),d7	; get byte to output
	move.w	d7,d2		; into parameter register
	sub.w	#1,d3		; subtract orginal position
	lsr.w	#1,d3		; divide by 2 for nibbles output
	add.w	#68,d3		; where it should go
	jsr	outchar		; go output character
	
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1
	add.w	#34,d3
	move.w	d7,d2		; into parameter register
	lsr.w	#4,d2		; get high nibble of byte
	lea	hexnums,a0	; get address of hex numbers
	move.b	(a0,d2.w),d2	; get byte to output
	jsr	outchar		; output the character
	move.w	d5,d3		; set x parameter
	move.w	d6,d1		; set y parameter
	add.w	#35,d3
	move.w	d7,d2		; get byte to output
	and.w	#$f,d2		; mask out nibble
	lea	hexnums,a0
	move.b	(a0,d2.w),d2	; get ascii of hex nibble
	jsr	outchar
osl3:
	add.w	#2,d5		; pass one space and for next hex digit
	cmp.w	#17,d5		; at end of screen ->?
	blt	osl2		; nope keep going

	adda.l	#16,a2		; skip bytes layed down already
	add.w	#1,d6		; next position down
	cmp.w	#24,d6
	ble	osl1
	rts
	
*
* -----	textscrout dumps text out to the screen
*
textscrout:
	lea	diskbuff,a2	; buffer pointer
	move.w	#3,d6		; the y position
tsol1:
	move.w	#7,d5		; the x position
tsol2:
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1

	clr.w	d2
	move.b	(a2)+,d2	; get byte to output
	jsr	outchar		; go output character
	add.w	#1,d5		; next x position
	cmp.w	#71,d5		; at end of x position
	ble	tsol2		; nope keep going
	add.w	#1,d6
	cmp.w	#11,d6		; see if at end of y positions
	ble	tsol1		; nope keep going
	rts
	
*
* -----	dirscrout outputs screen in directory format
*
dirscrout:
	move.w	#3,d6		; y position save
	move.w	#3,d5		; x position save
	lea	dirstr1,a2	; string to output
dsol1:
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	clr.w	d2		; clear hibyte
	move.b	(a2)+,d2	; get character to output
	beq	dsol2		; if zero then end of string
	jsr	outchar		; output character
	add.w	#1,d5		; next x position
	bra	dsol1		; branch till zero character found
dsol2:
	move.w	#4,d6		; y position save
	move.w	#3,d5		; x position save
	lea	dirstr2,a2	; string to output
dsol3:
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	clr.w	d2		; clear hibyte
	move.b	(a2)+,d2	; get character to output
	beq	dsol4		; if zero then end of string
	jsr	outchar		; output character
	add.w	#1,d5		; next x position
	bra	dsol3		; branch till zero character found

dsol4:	
	lea	diskbuff,a2	; buffer pointer
	move.w	#6,d6		; the y position
dsol5:
	move.w	#5,d5		; the x position
	clr.w	d2
	move.b	(a2),d2
	tst.b	d2		; see if unused directory
	bne	dsol20
	lea	unudirstr,a3	; yep so load up unused string
	jsr	charstrout	; go output the string
	add.l	#26,a2
	bra	dsol19
dsol20:
	cmp.b	#$3f,d2		; to low of a character
	ble	dsol22
	cmp.b	#$7a,d2		; invalid directory entry?
	ble	dsol6
dsol22:
	lea	invdirstr,a3	; get string address
	jsr	charstrout
	add.l	#26,a2
	bra	dsol19		; branch past all the junk
dsol6:
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1

	clr.w	d2
	move.b	(a2)+,d2	; get byte to output
	jsr	outchar		; go output character
	add.w	#1,d5		; next x position
	cmp.w	#12,d5		; at end of x position
	ble	dsol6		; nope keep going

	move.w	d5,d3		; get into parameter place
	move.w	d6,d1
	move.w	#$2e,d2		; output a period
	jsr	outchar		; go do it
	add.w	#1,d5

dsol7:
	move.w	d5,d3		; get into parameter place
	move.w	d6,d1

	clr.w	d2
	move.b	(a2)+,d2	; get byte to output
	jsr	outchar		; go output character
	add.w	#1,d5		; next x position
	cmp.w	#16,d5		; at end of x position
	ble	dsol7		; nope keep going

	add.w	#3,d5

	move.b	(a2)+,d7	; get file attribute byte
	btst	#0,d7		; is it a read-only file?
	beq	dsol9		; nope so don't check it
	move.w	d5,d3		; get parameters to pass
	move.w	d6,d1
	move.w	#8,d2		; character check mark
	jsr	outchar		; output the character
dsol9:
	add.w	#1,d5
	btst	#1,d7		; is it a hidden file?
	beq	dsol10		; nope so don't check it
	move.w	d5,d3		; get parameters to pass
	move.w	d6,d1
	move.w	#8,d2		; character check mark
	jsr	outchar		; output the character
dsol10:
	add.w	#1,d5
	btst	#2,d7		; is it a system file?
	beq	dsol11		; nope so don't check it
	move.w	d5,d3		; get parameters to pass
	move.w	d6,d1
	move.w	#8,d2		; character check mark
	jsr	outchar		; output the character
dsol11:
	add.w	#1,d5
	btst	#3,d7		; is it a disk label file?
	beq	dsol12		; nope so don't check it
	move.w	d5,d3		; get parameters to pass
	move.w	d6,d1
	move.w	#8,d2		; character check mark
	jsr	outchar		; output the character
dsol12:
	add.w	#1,d5
	btst	#4,d7		; is it a directory file?
	beq	dsol13		; nope so don't check it
	move.w	d5,d3		; get parameters to pass
	move.w	d6,d1
	move.w	#8,d2		; character check mark
	jsr	outchar		; output the character
dsol13:
	add.w	#1,d5
	btst	#5,d7		; is it a archive file?
	beq	dsol14		; nope so don't check it
	move.w	d5,d3		; get parameters to pass
	move.w	d6,d1
	move.w	#8,d2		; character check mark
	jsr	outchar		; output the character
dsol14:
	add.w	#4,d5
	add.l	#10,a2		; get to time word
	move.b	1(a2),d7	; get the time word
	lsl.w	#8,d7		; put in our format not ibms
	move.b	(a2)+,d7
	add.l	#1,a2
	move.l	a2,-(sp)
	move.l	d7,d2		; put in temp register
	and.l	#$f800,d2	; mask out hour hand
	move.w	#11,d0
	lsr.w	d0,d2		; shift left to get integer
	cmp.w	#12,d2
	blt	tam		; time is in am
	sub.w	#12,d2
	move.b	#'P',dirstr3+1	; make string pm
	bra	tpm
tam:
	move.b	#'A',dirstr3+1	; make string am
tpm:
	divu	#$0a,d2
	add.w	#$30,d2
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	jsr	outchar		; output character
	add.w	#1,d5		; next x position
	swap	d2
	add.w	#$30,d2
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	jsr	outchar		; output character
	add.w	#1,d5		; next x position

	move.w	d5,d3		; get parameters straight
	move.w	d6,d1
	move.w	#$3a,d2		; put out :
	jsr	outchar
	add.w	#1,d5

	move.w	d7,d2		; put in temp register
	and.l	#$07e0,d2	; mask out hour hand
	lsr.w	#5,d2		; shift left to get integer
	divu	#$0a,d2
	add.w	#$30,d2
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	jsr	outchar		; output character
	add.w	#1,d5		; next x position
	swap	d2
	add.w	#$30,d2
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	jsr	outchar		; output character
	add.w	#1,d5		; next x position

	move.w	d5,d3		; get parameters straight
	move.w	d6,d1
	move.w	#$3a,d2		; put out :
	jsr	outchar
	add.w	#1,d5

	move.w	d7,d2		; move into d2
	and.l	#$1f,d2		; and out bits for seconds
	divu	#$0a,d2
	add.w	#$30,d2
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	jsr	outchar		; output character
	add.w	#1,d5		; next x position
	swap	d2
	add.w	#$30,d2
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	jsr	outchar		; output character
	add.w	#1,d5		; next x position

	move.l	(sp)+,a2
	lea	dirstr3,a3	; string to output
dsol15:
	move.w	d5,d3		; put in for parameters
	move.w	d6,d1
	clr.w	d2		; clear hibyte
	move.b	(a3)+,d2	; get character to output
	beq	dsol16		; if zero then end of string
	jsr	outchar		; output character
	add.w	#1,d5		; next x position
	bra	dsol15		; branch till zero character found
dsol16:
	add.w	#3,d5
	move.b	1(a2),d7	; get hibyte first
	lsl.w	#8,d7		; shift into high postion
	move.b	(a2)+,d7	; get low byte next
	add.l	#1,a2		; plus index

	move.w	d7,d2		; into temporary registers
	and.l	#$1e0,d2	; get month variable
	sub.w	#1,d2
	lsr.w	#5,d2		; get into correct shift
	mulu	#3,d2		; for three letter months
	lea	monstr,a3
	add.w	d2,a3
dsol17:
	move.w	d5,d3
	move.w	d6,d1		; into parameter registers
	clr.w	d2		; clear for getting byte
	move.b	(a3)+,d2	; get byte to output
	jsr	outchar		; output the character
	add.w	#1,d5

	move.w	d5,d3	
	move.w	d6,d1		; into parameter registers
	clr.w	d2
	move.b	(a3)+,d2	; get next character
	jsr	outchar		; output the next character
	add.w	#1,d5

	move.w	d5,d3
	move.w	d6,d1		; into parameter registers
	clr.w	d2
	move.b	(a3)+,d2	; get last character
	jsr	outchar		; output last character
	add.w	#2,d5		; and skip one space
	
	move.w	d7,d2		; get into temp register
	and.l	#$1f,d2		; and out day part of word
	divu	#$0a,d2		; get high nibble
	add.w	#$30,d2		; into ascii format
	move.w	d5,d3
	move.w	d6,d1
	jsr	outchar		; output the character
	add.w	#1,d5
	
	swap	d2		; get low nibble of byte
	add.w	#$30,d2		; convert to ascii
	move.w	d5,d3		; into parameter registers
	move.w	d6,d1
	jsr	outchar
	add.w	#1,d5		; output character and next x position

	move.w	d5,d3
	move.w	d6,d1
	move.w	#$2c,d2		; comma character
	jsr	outchar		; output it
	add.w	#1,d5		; next x

	move.l	a2,-(sp)	; save a2 because of fucking compiler
	and.l	#$fe00,d7	; mask out year
	move.w	#9,d0		; number of shifts
	lsr.w	d0,d7		; shift into correct bit position
	add.w	#1980,d7	; add in base year
	pea	strout		; string to convert it to
	move.w	d7,-(sp)	; number to convert to ascii
	jsr	_uitoa		; go convert the sucker
	addq.l	#6,sp		; clean up
	add.w	#1,d5
	move.l	(sp)+,a2	; restore a2 register
	
	lea	strout,a3	; pointer register
	jsr	charstrout	; out put the string

	add.l	#2,a2
*	bra	dsol19		**** delete this
	move.b	3(a2),d0	; get first byte
	lsl.l	#8,d0		; shift into position
	move.b	2(a2),d0	; get next byte
	lsl.l	#8,d0		; next byte
	move.b	1(a2),d0	; get next byte
	lsl.l	#8,d0		; shift it
	move.b	(a2),d0		; ok now were in order
	pea	strout		; address to convert it to
	move.l	d0,-(sp)	; push length of file
	jsr	_ltoa		; go convert it
	add	#8,sp		; clean up the mess
	
	pea	strout		; parameter pass
	jsr	_strlen		; find the string length
	add	#4,sp
	add.w	d0,d5

	move.l	a2,-(sp)	; save it
	lea	strout,a3	; a3 new register
dsol18:
	tst.w	d0
	beq	dsol21		; end of string?
	sub.w	#1,d0
	move.w	d0,-(sp)	; save string length
	clr.w	d2
	move.b	(a3)+,d2	; get byte to output
	move.w	d5,d3
	move.w	d6,d1
	jsr	outchar		; go output the character
	move.w	(sp)+,d0	; now get do back again
	sub.w	#1,d5
	bra	dsol18		; branch always
dsol21:
	move.l	(sp)+,a2	; get back a2 register	
dsol19:
	add.l	#4,a2		; next filename

	add.w	#1,d6
	cmp.w	#21,d6		; see if at end of y positions
	ble	dsol5		; nope keep going
	rts

*
* -----	_charstrout outputs a string to current x,y from 'C'
*
_charstrout:
	movem.l	d2-d7/a3,-(sp)
	move.w	32(sp),d5	; get x position
	move.w	34(sp),d6	; get y position
	move.l	36(sp),a3	; long address of string
	sub.w	#1,d5		; look below for explanation idiot!
	sub.w	#1,d6		; into out coridinate system
	jsr	charstrout	; go output the string
	movem.l	(sp)+,d2-d7/a3	; restore registers
	rts
*
* -----	charstrout outputs a string to current x,y
*
charstrout:
	clr.w	d2
	move.b	(a3)+,d2	; get a byte
	beq	cosexit
	move.w	d5,d3		; into parameter registers
	move.w	d6,d1
	jsr	outchar		; output a character
	add.w	#1,d5
	bra	charstrout	; keep going until end of string is reached
cosexit:
	rts

*
* -----	medtextdraw draws a medium text shape to the screen location
*	parameters: d0.l = xshf/xbyte
*		    d1.w = y offset (y * $a0)
*		    a1.l = shape pointer
*
medtextdraw:
	move.l	screen1,a0	; get the current screen pointer
	adda.w	d1,a0		; add in y offset
	adda.w	d0,a0		; add in x byte offset
	swap	d0		; get x shift into low word
	move.w	#7,d3
mtdrawloop:
	clr.l	d1
	move.b	(a1),d1		; get a long word of shape
	swap	d1		; put the pixels on is high d1
	move.b	(a1),d1		; get a long word of shape
	lea	$100(a1),a1
	lsl.l	d0,d1		; rotate to put at right pixel
	and.l	d4,(a0)		; mask out screen bits behind shape
	or.l	d1,(a0)		; put shape there and point to next
	lea	$a0(a0),a0	; next screen line down
	dbra	d3,mtdrawloop	; go balls out on height
	rts

*
* -----	hitextdraw draws a high text shape to the screen location
*	parameters: d0.l = xshf/xbyte
*		    d1.w = y offset (y ; $a0)
*		    a1.l = shape pointer
*
hitextdraw:
	lsr.w	#1,d0		; for hi rez
	move.l	screen1,a0	; get the current screen pointer
	adda.w	d1,a0		; add in y offset
	adda.w	d0,a0		; add in x byte offset
	swap	d0		; get x shift into low word
	move.w	#7,d3
htdrawloop:
	clr.l	d1
	move.b	(a1),d1		; get a long word of shape
	swap	d1		; put low into high word
	move.b	(a1),d1		; get a long word of shape
	lea	$100(a1),a1
	lsr.l	d0,d1		; rotate to put at right pixel
	and.w	d4,(a0)		; mask out screen bits behind shape
	or.w	d1,(a0)		; put shape there and point to next
	lea	$50(a0),a0	; next screen line down
	and.w	d4,(a0)		; next line
	or.w	d1,(a0)		; or in also
	lea	$50(a0),a0	; keep going
	dbra	d3,htdrawloop	; go balls out on height
	rts

*
* -----	medxor xor's a medium cursor on the screen
*	parameters: d0.l = xshf/xbyte
*		    d1.l = y offset (y * $a0)
*		    a1.l = shape pointer
*
medxor:
	move.l	screen1,a0	; get the current screen pointer
	add.l	d1,a0		; add in y offset
	add.w	d0,a0		; add in x byte offset
	swap	d0		; get x shift into low word
	move.w	#7,d2
mxorloop:
	move.l	#$ff00ff00,d1	; get a long word of shape
	lsr.l	d0,d1		; rotate to put at right pixel
	eor.l	d1,(a0)		; put shape there and point to next
	add.l	#$a0,a0		; next screen line down
	dbra	d2,mxorloop	; go balls out on height
	rts

*
* -----	hidxor xor's a high cursor on the screen
*	parameters: d0.l = xshf/xbyte
*		    d1.l = y offset (y * $a0)
*		    a1.l = shape pointer
*
hixor:
	lsr.w	#1,d0		; half for high rez
	move.l	screen1,a0	; get the current screen pointer
	add.l	d1,a0		; add in y offset
	add.w	d0,a0		; add in x byte offset
	swap	d0		; get x shift into low word
	move.w	#7,d2
hxorloop:
	move.w	#$ff00,d1	; get a  word of mask
	lsr.w	d0,d1		; rotate to put at right pixel
	eor.w	d1,(a0)		; put shape there and point to next
	add.l	#$50,a0		; next screen line down
	eor.w	d1,(a0)		; do next line also
	add.l	#$50,a0		; next address
	dbra	d2,hxorloop	; go balls out on height
	rts

*
*	_docursor eor the character at x,y
*		4(sp) = x position
*		6(sp) = y position
*
_docursor:
	move.w	4(sp),d3	; get x parameter
	move.w	6(sp),d1	; get y parameter
	sub.w	#1,d3		; set to our cooridinate system
	sub.w	#1,d1		; ditto
	moveq.l	#0,d0		; clear out d0
	btst	#0,d3		; see if x is odd
	beq	dcl1		; nope so go ahead
	move.l	#$80000,d0	; make x shift 8
	andi.b	#$fe,d3		; get rid of odd and make even with shift
dcl1:
	lsl.w	#1,d3		; multiply x by two
	lsl.w	#3,d1		; multiply y by eight
	sub.w	#1,d1
	mulu.w	#$a0,d1		; times number of bytes per line
	move.w	d3,d0		; put in d0 for call to medxor
	cmp.w	#1,_rez		; in medium rez
	bne	dcl2
	jsr	medxor
	bra	dcl3
dcl2:
	jsr	hixor		; do a high cursor
dcl3:
	rts

*
* -----	_outchar outputs character from 'C'
*
_outchar:
	movem.l	d2-d4,-(sp)	; save registers
	move.w	16(sp),d3	; get x character position
	move.w	18(sp),d1	; get y character position
	move.w	20(sp),d2	; get character to output
	sub.w	#1,d3		; set to our cooridinate system
	sub.w	#1,d1		; ditto
	jsr	outchar		; go output character
	movem.l	(sp)+,d2-d4	; recover registers
	rts

*
* -----	outchar outputs a character to the screen
*	parameters:
*	  d2 = character output
*	  d3 = x character pos 0-79
*	  d1 = y character pos 0-24
*
outchar:
	move.l	#$80000,d0	; shift left by eight
	move.l	#$00ff00ff,d4	; mask for even boundary
	btst	#0,d3		; see if we are on even boundary
	beq	outcl1		; yep so bypass resetting mask
	moveq.l	#0,d0		; zero d0 meaning no shift
	move.l	#$ff00ff00,d4	; odd mask boundary
	andi.b	#$fe,d3		; actually put on word boundary
outcl1:
	lsl.w	#1,d3		; double for color plane
	lsl.w	#3,d1		; times 8 for scan lines/character
	sub.w	#1,d1		; minus one for window
	move.w	d1,d0		; save y position
	lsl.w	#7,d0		; multiply by $80
	lsl.w	#5,d1		; multiply by $20
	add.w	d0,d1		; together we have multiplied by $a0
	move.l	fonttable,a1	; address of character data
	adda.w	d2,a1		; get to offset
	move.w	d3,d0		; and add for byte offset into scan line
	cmp.w	#1,_rez		; in medium resolution?
	bne	outcl2
	jsr	medtextdraw
	bra	outcl3
outcl2:
	jsr	hitextdraw
outcl3:
	rts

*
*	_qcompare compares bytes rather quickly
*
_qcompare:
	move.l	4(sp),a0	; get first address
	move.l	8(sp),a1	; get second address
	move.l	12(sp),d1	; length of search
	move.l	d1,d0
qcl1:
	cmpm.b	(a0)+,(a1)+	; see if equal
	bne	qcl2		; nope not equal
	sub.l	#1,d1		; keep checking
	bne	qcl1
	moveq.l	#-1,d0		; it checked
qcl2:
	sub.l	d1,d0		; byte offset that didn't compare
	rts

*
*	_qsearch searches for a string
*
_qsearch:
	move.l	4(sp),a0	; get first address
	move.l	8(sp),a2	; get second address
	move.l	12(sp),d1	; length of search buffer
	move.w	16(sp),d2	; number of bytes to check -1
	subq.w	#1,d2
qsl1:
	move.l	a2,a1		; set up pointer to string
	move.w	d2,d0		; length of string to check
qsl2:
	cmpm.b	(a0)+,(a1)+	; see if equal
	bne	qsl3		; nope not equal
	dbra	d0,qsl2
	sub.w	16(sp),a0	; buffer pointer to string match
	sub.l	4(sp),a0	; number of bytes into buffer
	move.l	a0,d0		; into return address
	bra	qsl4
qsl3:
	subq.l	#1,d1		; keep checking
	bne	qsl1
	moveq.l	#-1,d0		; no match
qsl4:
	rts
	
	
*
*	For GEMAES calls from AESBIND.ARC or cryslib.o
*
_crystal:
	move.l	4(a7),d1
	move.w	#200,d0
	trap	#2
	rts


*
*
	.bss
	.even
	.ds.l	256		; stack area
ustk:	.ds.l	1

slotaddr:
	.ds.l	1		; slot address of vblank

_diskbuff:
diskbuff:
	.ds.b	7000		; max size sector

screen1:
	.ds.l	1
sec_addr:
	.ds.l	1		; sector address pointer
deselflag:
	.ds.w	1

reg_save:
	.ds.l	16		; register save area

save_ssp:
	.ds.l	1		; supervisor save stack pointer
save_d0:
	.ds.l	1		; save for reg. d0

last_sec:
	.ds.w	1
track_num:
	.ds.w	1		; track number
side_no:
	.ds.w	1		; side number for drive

dev_no:
	.ds.w	1		; device number for drive

tmpdma:
	.ds.l	1		; temp. dma pointer

savea1:
	.ds.l	1		; save area for a1 for buffer pointer
savea0:
	.ds.l	1		; see above
strout:
	.ds.b	6		; a storage for returned integers string

fonttable:
	.ds.l	1		; where the font table is at
*
*
	.data
	.even

hexnums:
	.dc.b	'0123456789ABCDEF'

dirstr1:
	.dc.b	'  Name           RHSLDA   Time          Date          Size',0
dirstr2:
	.dc.b	'  ------------   ------   -----------   -----------   ---------',0   
dirstr3:
	.dc.b	' AM',0
monstr:
	.dc.b	'JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC'
unudirstr:
	.dc.b	'Unused Directory Entry',0
invdirstr:
	.dc.b	'Invalid Directory Entry',0
dr1_trk:
	.dc.w	$ff		; drive 1's track number
dr2_trk:
	.dc.w	$ff		; drive 2's track number


_ctrl_cnts:
	.dc.b	0, 1, 0			; func 010		
    	.dc.b	2, 1, 1			; func 011		
    	.dc.b	2, 1, 1 		; func 012		
	.dc.b	0, 1, 1			; func 013		
	.dc.b	2, 1, 1			; func 014		
	.dc.b	1, 1, 1			; func 015		
	.dc.b	0, 0, 0			; func 016		
	.dc.b	0, 0, 0			; func 017		
	.dc.b	0, 0, 0			; func 008		
	.dc.b	0, 1, 0			; func 019		
*							 Event Manager
	.dc.b	0, 1, 0			; func 020		
	.dc.b	3, 5, 0			; func 021		
	.dc.b	5, 5, 0			; func 022		
	.dc.b	0, 1, 1			; func 023		
	.dc.b	2, 1, 0			; func 024		
	.dc.b	16, 7, 1 		; func 025		
	.dc.b	2, 1, 0			; func 026		
	.dc.b	0, 0, 0			; func 027		
	.dc.b	0, 0, 0			; func 028		
	.dc.b	0, 0, 0			; func 009		
*							 Menu Manager
	.dc.b	1, 1, 1			; func 030		
	.dc.b	2, 1, 1			; func 031		
	.dc.b	2, 1, 1			; func 032		
	.dc.b	2, 1, 1			; func 033		
	.dc.b	1, 1, 2			; func 034		
	.dc.b	1, 1, 1			; func 005		
	.dc.b	0, 0, 0			; func 006		
	.dc.b	0, 0, 0			; func 007		
	.dc.b	0, 0, 0			; func 008		
	.dc.b	0, 0, 0			; func 009		
*							 Object Manager
	.dc.b	2, 1, 1			; func 040		
	.dc.b	1, 1, 1			; func 041		
  	.dc.b	6, 1, 1			; func 042		
	.dc.b	4, 1, 1			; func 043		
	.dc.b	1, 3, 1			; func 044		
	.dc.b	2, 1, 1			; func 045		
	.dc.b	4, 2, 1			; func 046		
	.dc.b	8, 1, 1			; func 047		
	.dc.b	0, 0, 0			; func 048		
	.dc.b	0, 0, 0			; func 049		
*							 Form Manager
	.dc.b	1, 1, 1			; func 050		
	.dc.b	9, 1, 1			; func 051		
	.dc.b	1, 1, 1			; func 002		
	.dc.b	1, 1, 0			; func 003		
	.dc.b	0, 5, 1			; func 004		
	.dc.b	0, 0, 0			; func 005		
	.dc.b	0, 0, 0			; func 006		
	.dc.b	0, 0, 0			; func 007		
	.dc.b	0, 0, 0			; func 008		
	.dc.b	0, 0, 0			; func 009		
*							 Dialog Manager
	.dc.b	0, 0, 0			; func 060		
	.dc.b	0, 0, 0			; func 061		
	.dc.b	0, 0, 0			; func 062		
	.dc.b	0, 0, 0			; func 003		
	.dc.b	0, 0, 0			; func 004		
	.dc.b	0, 0, 0			; func 005		
	.dc.b	0, 0, 0			; func 006		
	.dc.b	0, 0, 0			; func 007		
	.dc.b	0, 0, 0			; func 008		
	.dc.b	0, 0, 0			; func 009		
*							Graphics Manager
	.dc.b	4, 3, 0			; func 070		
	.dc.b	8, 3, 0			; func 071		
	.dc.b	6, 1, 0			; func 072		
	.dc.b	8, 1, 0			; func 073		
	.dc.b	8, 1, 0			; func 074		
	.dc.b	4, 1, 1			; func 075		
	.dc.b	3, 1, 1			; func 076		
	.dc.b	0, 5, 0			; func 077		
	.dc.b	1, 1, 1			; func 078		
	.dc.b	0, 5, 0			; func 009		
*							Scrap Manager
	.dc.b	0, 1, 1			; func 080		
	.dc.b	0, 1, 1			; func 081		
	.dc.b	0, 0, 0			; func 082		
	.dc.b	0, 0, 0			; func 083		
	.dc.b	0, 0, 0			; func 084		
	.dc.b	0, 0, 0			; func 005		
	.dc.b	0, 0, 0			; func 006		
	.dc.b	0, 0, 0			; func 007		
	.dc.b	0, 0, 0			; func 008		
	.dc.b	0, 0, 0			; func 009		
*							fseler Manager
	.dc.b	0, 2, 2			; func 090		
	.dc.b	0, 0, 0			; func 091		
	.dc.b	0, 0, 0			; func 092		
	.dc.b	0, 0, 0			; func 003		
	.dc.b	0, 0, 0			; func 004		
	.dc.b	0, 0, 0			; func 005		
	.dc.b	0, 0, 0			; func 006		
	.dc.b	0, 0, 0			; func 007		
	.dc.b	0, 0, 0			; func 008		
	.dc.b	0, 0, 0 		; func 009		
*							Window Manager
	.dc.b	5, 1, 0			; func 100		
	.dc.b	5, 1, 0			; func 101		
	.dc.b	1, 1, 0			; func 102		
	.dc.b	1, 1, 0			; func 103		
	.dc.b	2, 5, 0			; func 104		
	.dc.b	6, 1, 0			; func 105		
	.dc.b	2, 1, 0			; func 106		
	.dc.b	1, 1, 0			; func 107		
	.dc.b	6, 5, 0			; func 108		
	.dc.b	0, 0, 0 		; func 009		
*							Resource Manger
	.dc.b	0, 1, 1			; func 110		
	.dc.b	0, 1, 0			; func 111		
	.dc.b	2, 1, 0			; func 112		
	.dc.b	2, 1, 1			; func 113		
	.dc.b	1, 1, 1			; func 114		
	.dc.b	0, 0, 0			; func 115		
	.dc.b	0, 0, 0			; func 006
	.dc.b	0, 0, 0			; func 007
	.dc.b	0, 0, 0			; func 008
	.dc.b	0, 0, 0			; func 009
*							Shell Manager
	.dc.b	0, 1, 2			; func 120
	.dc.b	3, 1, 2			; func 121
	.dc.b	1, 1, 1			; func 122
	.dc.b	1, 1, 1			; func 123
	.dc.b	0, 1, 1			; func 124
	.dc.b	0, 1, 2			; func 125

	.end

