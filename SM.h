#ifndef SM_H
#define SM_H

#include "RM_Manager.h"
#include "RM_FileScan.h"

/* Names of the attribute and relation catalogs */
const char *const relName_attrcat = "attrcat";
const char *const relName_relcat  = "relcat";


typedef struct {
	char     attrName[MAXNAME+1];
	AttrType attrType;
	int      attrLength;
} AttrInfo;

typedef struct {
	char     relName[MAXNAME+1];
	char     attrName[MAXNAME+1];
	AttrType attrType;
	int      attrLength;
	int      offset;
} DataAttrInfo;

#endif
