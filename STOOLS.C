#define T0OBJ 0
#define T1OBJ 49
#define FREEBB 0
#define FREEIMG 0
#define FREESTR 70

BYTE *rs_strings[] = {
" Desk ",
" File ",
" Disk ",
" Folder",
"  About ST Tools...",
"--------------------",
"  Desk Accessory 1  ",
"  Desk Accessory 2  ",
"  Desk Accessory 3  ",
"  Desk Accessory 4  ",
"  Desk Accessory 5  ",
"  Desk Accessory 6  ",
"  Mapping       ",
"  Unerase       ",
"  Compare       ",
"  Search        ",
"  Erase         ",
"  Find          ",
"  Verify        ",
"  Edit          ",
"  Change Status ",
"  Copy          ",
"  Move          ",
"  Mapping        ",
"  Format         ",
"  Compare        ",
"  Search         ",
"  Erase          ",
"  Verify         ",
"  Edit           ",
"  Change Label   ",
"  Recover Errors ",
"  Copy           ",
"  Mapping    ",
"  Unerase    ",
"  Erase      ",
"  Sort       ",
"-------------",
"  Quit       ",
"  Copy       ",
"  Move       ",
"File Selector",
"",
"",
"Directory:",
"",
"",
"",
"________________________________________",
"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
"",
"",
"",
"________.___",
"________.___",
"________.___",
"________.___",
"________.___",
"________.___",
"________.___",
"________.___",
"________.___",
"Selection:",
"",
"",
"________.___",
"______________",
"xxxxxxxx~xxx",
" OK ",
"Cancel"};

LONG rs_frstr[] = {
0};

BITBLK rs_bitblk[] = {
0};

LONG rs_frimg[] = {
0};

ICONBLK rs_iconblk[] = {
0};

TEDINFO rs_tedinfo[] = {
41L, 42L, 43L, 3, 6, 0, 0x1180, 0x0, -1, 14,1,
44L, 45L, 46L, 3, 6, 0, 0x1180, 0x0, -1, 11,1,
47L, 48L, 49L, 3, 6, 0, 0x1180, 0x0, -1, 1,41,
50L, 51L, 52L, 3, 6, 2, 0x11A1, 0x0, -1, 1,1,
62L, 63L, 64L, 3, 6, 0, 0x1180, 0x0, -1, 11,1,
65L, 66L, 67L, 3, 6, 0, 0x1180, 0x0, -1, 13,15};

OBJECT rs_object[] = {
-1, 1, 7, G_IBOX, NONE, NORMAL, 0x0L, 0,0, 80,25,
7, 2, 2, G_BOX, NONE, NORMAL, 0x1100L, 0,0, 80,513,
1, 3, 6, G_IBOX, NONE, NORMAL, 0x0L, 2,0, 25,769,
4, -1, -1, G_TITLE, NONE, NORMAL, 0x0L, 0,0, 6,769,
5, -1, -1, G_TITLE, NONE, NORMAL, 0x1L, 6,0, 6,769,
6, -1, -1, G_TITLE, NONE, NORMAL, 0x2L, 12,0, 6,769,
2, -1, -1, G_TITLE, NONE, NORMAL, 0x3L, 18,0, 7,769,
0, 8, 40, G_IBOX, NONE, NORMAL, 0x0L, 0,769, 80,19,
17, 9, 16, G_BOX, NONE, NORMAL, 0xFF1100L, 2,0, 20,8,
10, -1, -1, G_STRING, NONE, NORMAL, 0x4L, 0,0, 20,1,
11, -1, -1, G_STRING, NONE, DISABLED, 0x5L, 0,1, 20,1,
12, -1, -1, G_STRING, NONE, NORMAL, 0x6L, 0,2, 20,1,
13, -1, -1, G_STRING, NONE, NORMAL, 0x7L, 0,3, 20,1,
14, -1, -1, G_STRING, NONE, NORMAL, 0x8L, 0,4, 20,1,
15, -1, -1, G_STRING, NONE, NORMAL, 0x9L, 0,5, 20,1,
16, -1, -1, G_STRING, NONE, NORMAL, 0xAL, 0,6, 20,1,
8, -1, -1, G_STRING, NONE, NORMAL, 0xBL, 0,7, 20,1,
29, 18, 28, G_BOX, NONE, NORMAL, 0xFF1100L, 8,0, 16,11,
19, -1, -1, G_STRING, NONE, NORMAL, 0xCL, 0,0, 16,1,
20, -1, -1, G_STRING, NONE, NORMAL, 0xDL, 0,1, 16,1,
21, -1, -1, G_STRING, NONE, NORMAL, 0xEL, 0,2, 16,1,
22, -1, -1, G_STRING, NONE, NORMAL, 0xFL, 0,3, 16,1,
23, -1, -1, G_STRING, NONE, NORMAL, 0x10L, 0,4, 16,1,
24, -1, -1, G_STRING, NONE, NORMAL, 0x11L, 0,5, 16,1,
25, -1, -1, G_STRING, NONE, NORMAL, 0x12L, 0,6, 16,1,
26, -1, -1, G_STRING, NONE, NORMAL, 0x13L, 0,7, 16,1,
27, -1, -1, G_STRING, NONE, NORMAL, 0x14L, 0,10, 16,1,
28, -1, -1, G_STRING, NONE, NORMAL, 0x15L, 0,8, 16,1,
17, -1, -1, G_STRING, NONE, NORMAL, 0x16L, 0,9, 16,1,
40, 30, 39, G_BOX, NONE, NORMAL, 0xFF1100L, 14,0, 17,10,
31, -1, -1, G_STRING, NONE, NORMAL, 0x17L, 0,0, 17,1,
32, -1, -1, G_STRING, NONE, NORMAL, 0x18L, 0,1, 17,1,
33, -1, -1, G_STRING, NONE, NORMAL, 0x19L, 0,2, 17,1,
34, -1, -1, G_STRING, NONE, NORMAL, 0x1AL, 0,3, 17,1,
35, -1, -1, G_STRING, NONE, NORMAL, 0x1BL, 0,4, 17,1,
36, -1, -1, G_STRING, NONE, NORMAL, 0x1CL, 0,5, 17,1,
37, -1, -1, G_STRING, NONE, NORMAL, 0x1DL, 0,6, 17,1,
38, -1, -1, G_STRING, NONE, NORMAL, 0x1EL, 0,8, 17,1,
39, -1, -1, G_STRING, NONE, NORMAL, 0x1FL, 0,9, 17,1,
29, -1, -1, G_STRING, NONE, NORMAL, 0x20L, 0,7, 17,1,
7, 41, 48, G_BOX, NONE, NORMAL, 0xFF1100L, 20,0, 13,8,
42, -1, -1, G_STRING, NONE, NORMAL, 0x21L, 0,0, 13,1,
43, -1, -1, G_STRING, NONE, NORMAL, 0x22L, 0,1, 13,1,
44, -1, -1, G_STRING, NONE, NORMAL, 0x23L, 0,2, 13,1,
45, -1, -1, G_STRING, NONE, NORMAL, 0x24L, 0,3, 13,1,
46, -1, -1, G_STRING, NONE, DISABLED, 0x25L, 0,6, 13,1,
47, -1, -1, G_STRING, NONE, NORMAL, 0x26L, 0,7, 13,1,
48, -1, -1, G_STRING, NONE, NORMAL, 0x27L, 0,4, 13,1,
40, -1, -1, G_STRING, LASTOB, NORMAL, 0x28L, 0,5, 13,1,
-1, 1, 23, G_BOX, NONE, OUTLINED, 0x21100L, 0,0, 45,19,
2, -1, -1, G_TEXT, NONE, NORMAL, 0x0L, 17,1, 13,1,
3, -1, -1, G_TEXT, NONE, NORMAL, 0x1L, 3,3, 11,1,
4, -1, -1, G_FTEXT, EDITABLE, NORMAL, 0x2L, 3,4, 40,1,
20, 5, 16, G_BOX, EXIT, NORMAL, 0xFF1100L, 3,6, 20,12,
6, -1, -1, G_BOXCHAR, 0x5, NORMAL, 0x5FF1100L, 0,0, 2,1,
7, -1, -1, G_BOXTEXT, NONE, NORMAL, 0x3L, 2,0, 18,1,
8, -1, -1, G_STRING, NONE, NORMAL, 0x35L, 3,2, 12,1,
9, -1, -1, G_STRING, NONE, NORMAL, 0x36L, 3,3, 12,1,
10, -1, -1, G_STRING, NONE, NORMAL, 0x37L, 3,4, 12,1,
11, -1, -1, G_STRING, NONE, NORMAL, 0x38L, 3,5, 12,1,
12, -1, -1, G_STRING, NONE, NORMAL, 0x39L, 3,6, 12,1,
13, -1, -1, G_STRING, NONE, NORMAL, 0x3AL, 3,7, 12,1,
14, -1, -1, G_STRING, NONE, NORMAL, 0x3BL, 3,8, 12,1,
15, -1, -1, G_STRING, NONE, NORMAL, 0x3CL, 3,9, 12,1,
16, -1, -1, G_STRING, NONE, NORMAL, 0x3DL, 3,10, 12,1,
4, 17, 19, G_BOX, EXIT, NORMAL, 0xFF1111L, 18,1, 2,11,
18, -1, -1, G_BOXCHAR, 0x5, NORMAL, 0x1FF1100L, 0,0, 2,1,
19, -1, -1, G_BOXCHAR, 0x5, NORMAL, 0x2FF1100L, 0,10, 2,1,
16, -1, -1, G_BOX, NONE, NORMAL, 0xFF1100L, 0,1, 2,1,
21, -1, -1, G_TEXT, NONE, NORMAL, 0x4L, 26,6, 10,1,
22, -1, -1, G_FTEXT, EDITABLE, NORMAL, 0x5L, 26,7, 14,1,
23, -1, -1, G_BUTTON, 0x7, NORMAL, 0x44L, 28,12, 8,1,
0, -1, -1, G_BUTTON, 0x21, NORMAL, 0x45L, 28,14, 8,1};

LONG rs_trindex[] = {
0L,
49L};

struct foobar {
	WORD	dummy;
	WORD	*image;
	} rs_imdope[] = {
0};

#define NUM_STRINGS 70
#define NUM_FRSTR 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_TI 6
#define NUM_OBS 73
#define NUM_TREE 2

BYTE pname[] = "UTIL.RSC";