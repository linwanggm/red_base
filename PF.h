/*
 *  This file defines various data structures and limits used
 *  by the PF component of Redbase. 
 */

#ifndef PF_H
#define PF_H

#include <stdio.h>
#include <time.h>

/* This limit defines the number of bytes
   a single page can store */
#define PF_PAGE_SIZE 4092

/* Defines the PageNum data type */
typedef unsigned int PageNum;
#define PF_PAGENUM_SIZE sizeof(PageNum)

/* Macro for sizeof(int) */
#define INT_SIZE sizeof(int)

/* Defines the structure of a disk page/block */
typedef struct {
	PageNum pageNum; 
	char pData[PF_PAGE_SIZE];
} Page;

/* A frame is essentialy a page fetched into memory -- on disk, it is called 
   a "page"; in memory, it is called a "frame" */
typedef struct {
 	/* TRUE if page contents have been modified; FALSE otherwise */
	bool bDirty;
	/* The number of "processes" currently accessing this page */
	unsigned int pinCount; 	
 	/* Time this page was last accessed */
	time_t 	accTime;
 	/* The file the page came from */
	char *fileName;
 	/* The file descriptor associated with the file */
	int fileDesc;
 	/* The actual page that was fetched from disk */
	Page page;
} Frame;


/* Enumeration of errors returned by 
   various Redbase classes */
typedef enum {
	SUCCESS = 0,

	/* PF Errors */
	PF_INVALIDNAME,
	PF_UNIX,
	PF_FHOPEN,
	PF_FHCLOSED,
	PF_PHOPEN,
	PF_PHCLOSED,
	PF_EOF,
	PF_INVALIDPAGENUM,
	PF_NOTINBUF,
	PF_PAGEPINNED,
	PF_PAGEUNPINNED,
	PF_NOBUF,
	PF_DBFULL,
	PF_READINC,
	PF_WRITEINC,

	/* RM Errors */
        RM_INVALIDRID,
        RM_INVALIDREC,
        RM_INVALIDRECSIZE,
	RM_FHCLOSED,
	RM_FSOPEN,
	RM_FSCLOSED,
	RM_NOMORERECINMEM,

	/* SM Errors */
	SM_TUPLENOTINIT,
	SM_INVALIDATTRIX,
	SM_RELNAMECAT,
	SM_DBOPEN,
	SM_DBCLOSED,
	SM_RELNOTFOUND,
	SM_ATTRNOTFOUND,
	SM_RELEXISTS,
	SM_BADINPUT,

	/* PR (Parser) Errors */
	PR_TOOMANYATTRS,
	PR_INVALIDATTRLEN
} RC;

/* Utility functions */
void PrintError (RC rc);

#endif
