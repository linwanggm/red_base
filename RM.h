#ifndef RM_H
#define RM_H

#include "PF_Manager.h"

/* Define SlotNum */
typedef int SlotNum;

/* Defines an RM page type */
typedef enum {
	NON_FULL_PAGE, /* A page that can hold additional records */
	FULL_PAGE /* A page that cannot hold any more records */
} RM_PageType;

/* Defines comparison operators that can be used in file scans */
typedef enum {
	EQ_OP, LT_OP, GT_OP, /* Equal to, less than, greater than */
	NE_OP, LE_OP, GE_OP, /* Not equal to, less than or equal to,
				greater than  or equal to*/
	NO_OP /* No comparison. Should be used when file scan value is NULL. */
} CompOp;


/* Defines a limit on the maximum length a string attribute type in Redbase
 * can have */
#define MAXSTRINGLEN 255
/* Defines limit on number of attributes a relation can have */
#define MAXATTRS 40
/* Defines limit on size of relation and attribute names */
#define MAXNAME 24

/* Defines attribute types */
typedef enum {
	TYPE_INT, /* Integer attributes are 4 bytes in length */
	TYPE_FLOAT, /* Floating point attributes are also 4 bytes in length */
	TYPE_STRING /* String attributes can have a length of at least 1
		       byte and at most MAXSTRINGLEN bytes */
} AttrType;

/* Defines various page pinning strategies that RM clients can specify when
 * using RM_FileScan to retrieve records. */
typedef enum {
	NO_HINT,
	EFFICIENT
} ClientHint;


/* This structure defines the data is stored in the file header */
typedef struct {
	/* Stores the number of records currently contained in the file */
	int nRecords;
	/* Stores the size of each record */
	int recordSize;
	/* Stores the number of records that can fit in one page */
	int recordsPerPage;
    /* Stores offset (in bytes) of the first record */
    int firstRecordOffset;
} RM_FileSubHeader;
#define RM_FILESUBHDR_SIZE sizeof(RM_FileSubHeader)

#endif
