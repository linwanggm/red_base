#include "PF.h"
#include <string.h>
void PrintError (RC rc) {
	char err[256];
	switch (rc) {
		case SUCCESS:	strcpy(err,"Success"); break;

		/* PF Errors */
		case PF_INVALIDNAME:	strcpy(err,"Invalid filename"); break;
		case PF_UNIX:		strcpy(err,"Unix error"); break;
		case PF_FHOPEN:		strcpy(err,"PF-FileHandle already open"); break;
		case PF_FHCLOSED:	strcpy(err,"PF-FileHandle closed"); break;
		case PF_PHOPEN:		strcpy(err,"PF-PageHandle already open"); break;
		case PF_PHCLOSED:	strcpy(err,"PF-PageHandle closed"); break;
		case PF_EOF:		strcpy(err,"End-Of-File"); break;
		case PF_INVALIDPAGENUM:	strcpy(err,"Invalid page number"); break;
		case PF_NOTINBUF:	strcpy(err,"Page not in buffer"); break;
		case PF_PAGEPINNED:	strcpy(err,"Page pinned in memory"); break;
		case PF_PAGEUNPINNED:	strcpy(err,"Page unpinned in memory"); break;
		case PF_NOBUF:		strcpy(err,"No buffer space"); break;
		case PF_DBFULL:		strcpy(err,"Database full"); break;
		case PF_READINC:	strcpy(err,"Read incomplete"); break;
		case PF_WRITEINC:	strcpy(err,"Write incomplete"); break;

		/* RM Errors */
	        case RM_INVALIDRID:	strcpy(err,"Invalid RID"); break;
		case RM_INVALIDREC:	strcpy(err,"Invalid record"); break;
		case RM_INVALIDRECSIZE:	strcpy(err,"Invalid record size"); break;
		case RM_FHCLOSED:	strcpy(err,"RM File-handle closed"); break;
		case RM_FSOPEN:		strcpy(err,"RM FileScan open"); break;
		case RM_FSCLOSED:	strcpy(err,"RM FileScan closed"); break;
		case RM_NOMORERECINMEM: strcpy(err,"No more records in memory to fetch"); break;

		/* SM Errors */
		case SM_TUPLENOTINIT:	strcpy(err,"Tuple not initialised"); break;
		case SM_INVALIDATTRIX:	strcpy(err,"Invalid attribute index"); break;
		case SM_RELNAMECAT:	strcpy(err,"Cannot open catalog files for this operation"); break;
		case SM_RELNOTFOUND:	strcpy(err,"Relation not found"); break;
		case SM_ATTRNOTFOUND:	strcpy(err,"Attribute not found"); break;
		case SM_RELEXISTS:	strcpy(err,"Relation exists in database"); break;
		case SM_BADINPUT:	strcpy(err,"Error loading data from file"); break;
		case SM_DBCLOSED:	strcpy(err,"Not connected to a database"); break;
		case SM_DBOPEN:		strcpy(err,"Another databse is open"); break;

		/* PR (Parser) Errors */
		case PR_TOOMANYATTRS:	strcpy(err,"Too many attributes in relation"); break;
		case PR_INVALIDATTRLEN:	strcpy(err,"Bad attribute name length"); break;

		/* Unknown return constant */
		default:		strcpy(err,"--- Unkown error ---: Please report to TA!"); break;	
	}
	fprintf (stderr, "ERR: %s\n", err); 
}
