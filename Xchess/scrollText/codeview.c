/*
 * C Code Viewer for X
 *
 * David Harrison
 * University of California,  1986
 */

#include <stdio.h>
#include <ctype.h>
#include <X/Xlib.h>
#include "st.h"

st_table *keywords = (st_table *) 0;
int in_comment = 0;
int in_string  = 0;

/* Table to define which delimiters are interesting */
#define MAXCHARS	256
char specialChar[MAXCHARS];

extern int strcmp();
extern char *strcpy();
extern char *strncpy();

main(argc,argv)
int argc;
char *argv[];
{
    FILE *incFile;
    Display *theDisp;
    Window theWin;
    OpaqueFrame theFrame;
    XEvent theEvent;
    FontInfo *theFont;
    char *dispname;
    char *fontNames[4];
    int colorPixels[4];
    char *filename;
    char *ProcessLine();
    char linebuf[2048];
    int index, count;
    int bgPixel, fgPixel, curPixel, borPixel;

    /* read the argument list */
    dispname = "";
    filename = (char *) 0;
    fontNames[0] = "timrom12";
    fontNames[1] = "timrom12i";
    fontNames[2] = "timrom12b";
    fontNames[3] = "helv12";
    for (index = 0;  index < 4;  index++) colorPixels[index] = BlackPixel;
    bgPixel = WhitePixel;
    curPixel = BlackPixel;
    borPixel = BlackPixel;
    index = 1;
    while (index < argc) {
	if (argv[index][0] == '-') {
	    /* Its an option */
	    switch (argv[index][1]) {
	    case 'f':
		/* Read file into buffer */
		filename = argv[index + 1];
		index += 2;
		break;
	    default:
		printf("Unknown option\n");
		printf("format: %s [-f file] [host:display]\n",
		       argv[0]);
		exit(1);
	    }
	} else {
	    /* Its the display */
	    dispname = argv[index];
	    index++;
	}
    }

    theDisp = XOpenDisplay(dispname);
    if (!theDisp) {
	printf("Unable to open display\n");
	exit(1);
    }
    readDefaults(argv[0], fontNames, colorPixels, &bgPixel, &fgPixel,
		 &curPixel, &borPixel);
    theFrame.bdrwidth = 2;
    theFrame.border = XMakeTile(borPixel);
    theFrame.background = XMakeTile(bgPixel);
    theWin = XCreate("C Code Viewer", "codeView",
		     "", "400x400+100+100", &theFrame, 50, 50);
    XFreePixmap(theFrame.border);
    XFreePixmap(theFrame.background);
    if (!theWin) exit(1);
    XSelectInput(theWin, ExposeRegion|ExposeCopy|ButtonReleased);
    XMapWindow(theWin);

    theFont = XOpenFont(fontNames[0]);
    if (!TxtGrab(theWin, "codeView", theFont,
		 bgPixel, fgPixel, curPixel)) {
	printf("Text grab failed\n");
	exit(1);
    }
    XFlush();

    for (index = 0;  index < 4;  index++) {
	if (fontNames[index]) {
	    TxtAddFont(theWin, index, XOpenFont(fontNames[index]),
		       colorPixels[index]);
	}
    }

    incFile = stdin;
    if (filename) {
	incFile = fopen(filename, "r");
	if (!incFile) {
	    printf("can't read %s\n", filename);
	    exit(1);
	}
    }

    /* Initialize tables */
    InitTable();

    /* Jam in the text */
    while (fgets(linebuf, 2047, incFile)) {
	TxtJamStr(theWin, ProcessLine(linebuf));
    }

    /* Main event loop */
    count = 0;
    for (;;) {
	XNextEvent(&theEvent);
	if (!TxtFilter(&theEvent)) {
	    switch (theEvent.type) {
	    case ButtonReleased:
		if ((((XButtonEvent *) &theEvent)->detail & ValueMask) ==
		    MiddleButton)
		  {
		      count++;
		      if (count > 1) {
			  printf("done\n");
			  exit(0);
		      }
		  }
		else {
		    XFeep(0);
		}
	    }
	}
    }
}

readDefaults(name, fontNames, colorPixels, bg, fg, cur, bor)
char *name;
char *fontNames[3];
int colorPixels[3];
int *bg, *fg, *cur, *bor;
/*
 * Reads all of the X defaults
 */
{
    char *value;
    char buffer[1024];
    Color clrdef;

    value = XGetDefault(name, "Normal.Font");
    if (value) fontNames[0] = value;
    value = XGetDefault(name, "Comment.Font");
    if (value) fontNames[1] = value;
    value = XGetDefault(name, "Keyword.Font");
    if (value) fontNames[2] = value;
    value = XGetDefault(name, "String.Font");
    if (value) fontNames[3] = value;

    value = XGetDefault(name, "Background");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	*bg = clrdef.pixel;
    }
    value = XGetDefault(name, "Foreground");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	*fg = clrdef.pixel;
	colorPixels[0] = clrdef.pixel;
    }
    value = XGetDefault(name, "Border");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	*bor = clrdef.pixel;
    }
    value = XGetDefault(name, "Cursor");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	*cur = clrdef.pixel;
    }

    value = XGetDefault(name, "Normal.Color");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	colorPixels[0] = clrdef.pixel;
    }
    value = XGetDefault(name, "Comment.Color");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	colorPixels[1] = clrdef.pixel;
    }
    value = XGetDefault(name, "Keyword.Color");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	colorPixels[2] = clrdef.pixel;
    }
    value = XGetDefault(name, "String.Color");
    strcpy(buffer, value);
    if (value && XParseColor(buffer, &clrdef) && XGetHardwareColor(&clrdef)) {
	colorPixels[3] = clrdef.pixel;
    }
}


InitTable()
/*
 * Initializes hash table of keywords and special character array
 */
{
    int index;

    for (index = 0;  index < MAXCHARS;  index++) {
	specialChar[index] = 0;
    }
    specialChar['\0'] = 1;	/* End of file */
    specialChar[ ' '] = 1;	/* Space       */
    specialChar['\t'] = 1;	/* Tab	       */
    specialChar['\n'] = 1;	/* End of line */
    specialChar[ '/'] = 1;	/* Slash       */
    specialChar[ '*'] = 1;	/* Star        */
    specialChar[ '"'] = 1;	/* Quote       */

    keywords = st_init_table(strcmp, st_strhash);
    st_insert(keywords, "int", (char *) 0);
    st_insert(keywords, "char", (char *) 0);
    st_insert(keywords, "float", (char *) 0);
    st_insert(keywords, "double", (char *) 0);
    st_insert(keywords, "struct", (char *) 0);
    st_insert(keywords, "union", (char *) 0);
    st_insert(keywords, "long", (char *) 0);
    st_insert(keywords, "short", (char *) 0);
    st_insert(keywords, "unsigned", (char *) 0);
    st_insert(keywords, "auto", (char *) 0);
    st_insert(keywords, "extern", (char *) 0);
    st_insert(keywords, "register", (char *) 0);
    st_insert(keywords, "typedef", (char *) 0);
    st_insert(keywords, "static", (char *) 0);
    st_insert(keywords, "goto", (char *) 0);
    st_insert(keywords, "return", (char *) 0);
    st_insert(keywords, "sizeof", (char *) 0);
    st_insert(keywords, "break", (char *) 0);
    st_insert(keywords, "continue", (char *) 0);
    st_insert(keywords, "if", (char *) 0);
    st_insert(keywords, "else", (char *) 0);
    st_insert(keywords, "for", (char *) 0);
    st_insert(keywords, "do", (char *) 0);
    st_insert(keywords, "while", (char *) 0);
    st_insert(keywords, "switch", (char *) 0);
    st_insert(keywords, "case", (char *) 0);
    st_insert(keywords, "default", (char *) 0);
    st_insert(keywords, "entry", (char *) 0);
}


#define SPACE		1
#define TAB		2
#define COMBEGIN	3
#define COMEND		4
#define IDENT		5
#define USRIDENT	6
#define STR		7
#define OTHER		8
#define ENDLINE		9

char *ProcessLine(someline)
char *someline;
/*
 * This routine takes a line,  expands out its tabs into spaces
 * italicizes comments and boldfaces keywords.
 */
{
    static char outbuf[2048];
    int start, len, token, outspot, index, target;

    /* Reinitializes the token generator */
    get_token((char *) 0, (int *) 0, (int *) 0, (int *) 0);
    outspot = 0;

    while (get_token(someline, &start, &len, &token)) {
#ifdef
print_token(someline, start, len, token);
#endif
	switch (token) {
	case SPACE:
	    for (index = 0;  index < len;  index++) {
		outbuf[outspot] = ' ';
		outspot++;
	    }
	    break;
	case TAB:
	    /* Expand out using spaces */
	    target = ((outspot / 8) + 1) * 8;
	    for (index = outspot;  index < target;  index++) {
		outbuf[index] = ' ';
		outspot++;
	    }
	    break;
	case COMBEGIN:
	    /* Put it in and then change fonts */
	    if (!in_string) {
		outbuf[outspot++] = '\006';
		outbuf[outspot++] = '1';
		in_comment = 1;
	    }
	    outbuf[outspot++] = '/';
	    outbuf[outspot++] = '*';
	    break;
	case COMEND:
	    /* Change font back to normal and output */
	    outbuf[outspot++] = '*';
	    outbuf[outspot++] = '/';
	    if (!in_string) {
		outbuf[outspot++] = '\006';
		outbuf[outspot++] = '0';
		in_comment = 0;
	    }
	    break;
	case STR:
	    if (in_comment) {
		outbuf[outspot++] = '"';
	    } else {
		if (in_string) {
		    /* Change font back to normal and output */
		    outbuf[outspot++] = '"';
		    outbuf[outspot++] = '\006';
		    outbuf[outspot++] = '0';
		    in_string = 0;
		} else {
		    /* Put in string mode and change fonts */
		    outbuf[outspot++] = '\006';
		    outbuf[outspot++] = '3';
		    outbuf[outspot++] = '"';
		    in_string = 1;
		}
	    }
	    break;
	case IDENT:
	    /* Output in boldface if not in comment */
	    if (!(in_comment || in_string)) {
		outbuf[outspot++] = '\006';
		outbuf[outspot++] = '2';
	    }
	    for (index = 0;  index < len;  index++) {
		outbuf[outspot++] = someline[start+index];
	    }
	    if (!(in_comment || in_string)) {
		outbuf[outspot++] = '\006';
		outbuf[outspot++] = '0';
	    }
	    break;
	case USRIDENT:
	case OTHER:
	    /* Simply output */
	    for (index = 0;  index < len;  index++) {
		outbuf[outspot++] = someline[start+index];
	    }
	    break;
	case ENDLINE:
	    /* Output it */
	    outbuf[outspot++] = '\n';
	    break;
	}
    }
    outbuf[outspot] = '\0';
    return outbuf;
}




/* Macro to find special delimiters - see table initialization in main */
#define isspecial(c)	(specialChar[c])

/* Macro to ram don't care delimiters into an OTHER token */
#define EATDELIM \
  while (!isalnum(line[pos]) && !isspecial(line[pos])) pos++; \
  *len = pos - *start; \
  *token = OTHER;

int get_token(line, start, len, token)
char *line;
int *start, *len, *token;
/*
 * Reads a token from 'line'.  If all are zero,  resets the lexing
 * system.
 */
{
    static int pos;
    static char ident[2048];
    char *item;

    if ((line == 0) && (start == 0) && (len == 0) && (token == 0)) {
	pos = 0;
	return 0;
    }

    *start = pos;
    if (isspecial(line[pos])) {
	/* One of the special punctuation characters */
	if (line[pos] == '\0') {
	    return 0;
	} else if (line[pos] == '\t') {
	    /* Generate a tab token */
	    pos++;
	    *len = 1;
	    *token = TAB;
	} else if (line[pos] == '\n') {
	    /* End of line */
	    *len = 1;
	    *token = ENDLINE;
	    pos++;
	} else if (line[pos] == ' ') {
	    /* Generate a space token */
	    while (line[pos] == ' ') pos++;
	    *len = pos - *start;
	    *token = SPACE;
	} else if (line[pos] == '/') {
	    /* Possible beginning of comment */
	    if (line[++pos] == '*') {
		/* Start of comment */
		*len = 2;
		*token = COMBEGIN;
		pos++;
	    } else {
		/* Something else */
		EATDELIM;
	    }
	} else if (line[pos] == '*') {
	    /* Could be an end to a comment */
	    if (line[++pos] == '/') {
		/* End of a comment */
		*len = 2;
		*token = COMEND;
		pos++;
	    } else {
		/* Something else */
		EATDELIM;
	    }
	} else if (line[pos] == '"') {
	    /* Possible start or end of string */
	    if ((pos == 0) ||
		(((line[pos-1] == '\'') ? (line[pos+1] != '\'') : 1)
		 && (line[pos] != '\\')))
	      {
		  *len = 1;
		  *token = STR;
		  pos++;
	      } else {
		  /* Something else */
		  pos++;
		  EATDELIM;
	      }
	}
    } else if (isalnum(line[pos])) {
	/* Go get an identifier */
	do pos++; while (isalnum(line[pos]));
	*len = pos - *start;
	strncpy(ident, &(line[*start]), *len);
	ident[*len] = '\0';
	/* Try to look it up */
	if (st_lookup(keywords, ident, &item)) {
	    *token = IDENT;
	} else {
	    *token = USRIDENT;
	}
    } else {
	/* Random delimiters */
	pos++;
	EATDELIM;
    }
    return 1;
}


/* Debugging */

print_token(someline, start, len, token)
char *someline;
int start, len, token;
/* Prints out a token */
{
    char buffer[1024];
    int index;

    switch (token) {
    case SPACE:
	printf("space\n");
	break;
    case TAB:
	printf("tab\n");
	break;
	/*
	 * We try to break it by doing this
	 * many times over.
	 */
    case COMBEGIN:
	printf("start comment\n");
	break;
    case COMEND:
	printf("end comment\n");
	break;
    case STR:
	printf("string\n");
	break;
    case IDENT:
	printf("identifier: '");
	for (index = start;  index < start+len;  index++) {
	    putchar(someline[index]);
	}
	printf("'\n");
	break;
    case USRIDENT:
	printf("user identifier: '");
	for (index = start;  index < start+len;  index++) {
	    putchar(someline[index]);
	}
	printf("'\n");
	break;
    case OTHER:
	printf("delimiters: I");
	for (index = start;  index < start+len;  index++) {
	    putchar(someline[index]);
	}
	printf("I\n");
	break;
    }
}
