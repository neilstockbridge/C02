/**************************************************************
 * C02 Compiler - (C) 2013 Curtis F Kaylor                    *
 *                                                            *
 * C02 is a simpified C-like language designed for the 6502   *
 *                                                            * 
 * This Compiler generates crasm compatible assembly language *
 *                                                            *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "common.h"  //Common Code used by all Modules
#include "files.h"   //Open and Close Files
#include "asm.h"     //Write out Assembly Language
#include "parse.h"   //General Code Parsing
#include "vars.h"    //Variable Parsing, Lookup, and Allocation
#include "expr.h"    //Expression Parsing
#include "label.h"   //Label Parsing, Generation, and Lookup
#include "cond.h"    //Conditional Parsing
#include "stmnt.h"   //Statement Compiling Code
#include "dclrtn.h"   //Statement Compiling Code
#include "include.h" //Include File Parsing

/* Initilize Compiler Variables */
void init(void) {
  DEBUG("Initializing Compiler Variables\n",0)
  concnt = 0;
  varcnt = 0;
  lblcnt = 0;
  curcol = 0;
  curlin = 0;
  strcpy(incdir, "../include/");
  alcvar = TRUE;
  inblck = FALSE;
  xstmnt[0] = 0;
  nxtwrd[0] = 0;
  nxtptr = 0;
  vrwrtn = FALSE;
  zpaddr = 0;
  invasc = FALSE;
  mskasc = FALSE;
  fcase = FALSE;
  xsnvar[0] = 0;
  ysnvar[0] = 0;
}

/* Reads and parses the next Word in Source File */
void pword(void) {
  lsrtrn = FALSE; //Clear RETURN flag
  getwrd();
  DEBUG("Parsing Word '%s'\n", word)
  if (xstmnt[0]) {
    if (wordis(xstmnt)) xstmnt[0] = 0;  //Clear xstmnt
    else ERROR("Expected '%s' statement\n", xstmnt, EXIT_FAILURE)
  }
  if (!pmodfr() && !ptype(MTNONE)) pstmnt();     //Parse Statement
}

/* Process a directive */
void pdrctv(void) {
  skpchr();            //skip '#'
  CCMNT('#');
  getwrd();           //read directive into word
  DEBUG("Processing directive '%s'\n", word)
  if      (wordis("DEFINE"))  pdefin();  //Parse Define
  else if (wordis("INCLUDE")) pincfl();  //Parse Include File
  else if (wordis("ERROR"))   ERROR("Error \n", 0, EXIT_FAILURE)
  else if (wordis("PRAGMA"))  pprgma();
  else ERROR("Illegal directive %s encountered\n", word, EXIT_FAILURE)
}

void prolog(void) {
  DEBUG("Writing Assembly Prolog\n", 0)
  asmlin(CPUOP,CPUARG);
  setcmt("Program ");
  addcmt(srcnam);
  cmtlin();
}

void epilog(void) {
  if (!vrwrtn) vartbl();  //Write Variable Table
}

/* Compile Source Code*/
void compile(void) {
  DEBUG("Starting Compilation\n", 0)
  prolog();
  phdrfl(); //Process Header File specified on Command Line
  skpchr();
  DEBUG("Parsing Code\n", 0)
  while (TRUE) {
    skpspc();
    if      (match(EOF)) break;         //Stop Parsing (End of File)
    else if (match('}')) endblk(TRUE);  //End Multi-Line Program Block
    else if (match('#')) pdrctv();      //Parse Directive
    else if (match('/')) skpcmt();      //Skip Comment
    else if (isalph())   pword();       //Parse Word
    else ERROR("Unexpected character '%c'\n", nxtchr, EXIT_FAILURE)
  }    
  epilog();
}

/* Display "Usage" text and exit*/ 
void usage(void) {
  printf("Usage: c02 sourcefile.c02\n");
  exit(EXIT_FAILURE);      
}

/* Parse Command Line Option */
int popt(int arg, int argc, char *argv[]) {
  char argstr[32]; //Argument String
  char opt;        //Option
  char optarg[32]; //Option Argument
  strncpy (argstr, argv[arg], 31);
  if (strlen(argstr) != 2) ERROR("malformed option %s\n", argstr, EXIT_FAILURE)
  opt = toupper(argstr[1]);
  if (strchr("H", opt)) {
    if (++arg >= argc) ERROR("Option -%c requires an argument\n", opt, EXIT_FAILURE)
    strncpy(optarg, argv[arg], 31);
  }
  DEBUG("Processing Command Line Option -%c\n", argstr[1])
  switch (opt) {
    case 'H':
      strcpy(hdrnam, optarg);
      DEBUG("Header Name set to '%s'\n", hdrnam)
      break;
    default:
      ERROR("Illegal option -%c\n", opt, EXIT_FAILURE)
  }
  return arg;
}

/* Parse Command Line Arguments                                 *   
 *  Sets: srcnam - Source File Name (from first arg)           *
 *        outnam - Output File Name (from optional second arg) */
void pargs(int argc, char *argv[]) { 
  int arg;
  srcnam[0] = 0;
  outnam[0] = 0;
  DEBUG("Parsing %d arguments\n", argc)
  if (argc == 0) usage(); //at least one argument is required
  for (arg = 1; arg<argc; arg++) {
    DEBUG("Parsing argument %d\n", arg);
    if (argv[arg][0] == '-') arg = popt(arg, argc, argv); //Process Command Line Option
    else if (srcnam[0] == 0) strcpy(srcnam, argv[arg]);   //set Source File Name to first arg    
    else if (outnam[0] == 0) strcpy(outnam, argv[arg]);   //set Out File Name to second arg
    else ERROR("Unexpected argument '%s'\n", argv[arg], EXIT_FAILURE)
  }
  if (srcnam[0]) DEBUG("srcnam set to '%s'\n", srcnam)
  if (outnam[0]) DEBUG("outnam set to '%s'\n", outnam)
}

int main(int argc, char *argv[]) {
  debug = TRUE;  //Output Debug Info
  gencmt = TRUE; //Generate Assembly Language Comments
  
  printf("C02 Compiler (C) 2012 Curtis F Kaylor\n" );

  init(); //Initialize Global Variables
  
  pargs(argc, argv); //Parse Command Line Arguments
  
  opnsrc();  //Open Source File
  opnout();  //Open Output File
  opnlog();  //Open Log File

  setsrc();  //Set Input to Source File

  compile();

  logstc();
  logcon();

  clssrc();  //Close Source File
  clsout();  //Close Output File
  clslog();  //Close Log File
}


