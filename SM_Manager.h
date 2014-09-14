#ifndef SM_MANAGER_H
#define SM_MANAGER_H

#include "SM.h"
#include "SM_Tuple.h"

class SM_Manager {
private:
	bool bOpen; /* Is the SM_Manager attached to a DB? */	
	RM_Manager &rmm; /* Reference to RM_Manager for manipulating records 
			    in catalogs and relations */

	/* Working directory of the open DB */
	char cwd[81];
	/* File handle to the relation catalog of the open DB */
	RM_FileHandle fileHandle_relcat;
	/* File handle to the attribute catalog of the open DB */
	RM_FileHandle fileHandle_attrcat;
	/* Gets the structure of the given relation. This structure is
	 * stored in info. */
	RC GetDataAttrInfo (const char *relName, DataAttrInfo *&info, int
			&attrCount) const;
	/* Prints the tuple header (attribute names) for the given structure */
	void PrintTupleHeader (const DataAttrInfo *info, int attrCount) const;

public:
	  SM_Manager  (RM_Manager &rmm);  		   // Constructor
	  ~SM_Manager ();                                  // Destructor

	RC CreateDb     (const char *dbName);              // Create database
	RC DestroyDb    (const char *dbName);              // Destroy database

	RC OpenDb      (const char *dbName);                // Open database
	RC CloseDb     ();                                  // Close database

	RC CreateTable (const char *relName,                // Create relation
		       int        attrCount,
		       AttrInfo   *attributes);
	RC DropTable   (const char *relName);               // Destroy relation
	RC DeleteRecords (const char *relName);		    // Delete all records in relation

	RC Load        (const char *relName,                // Load utility
		       const char *fileName);

	RC Help        ();                                  // Help for database
	RC Help        (const char *relName);               // Help for relation

	RC Print       (const char *relName);               // Print relation

	RC CrossProduct (const char *rel1, const char *rel2);	  // Join relations

	RC EquiJoin    (const char *rel1, const char *rel2,	  // Join relations
			 const int *attrList1, const int *attrList2,
			 int condCount);

	RC EquiJoin_Opt (const char *rel1, const char *rel2,	  // Join relations
			 const int *attrList1, const int *attrList2,
			 int condCount);
};

/* Attribute information for relation catalog */
const DataAttrInfo dataAttrInfo_relcat[3] = {
		{ "relcat", "relName", TYPE_STRING, MAXNAME, 0 },
		{ "relcat", "tupleLength", TYPE_INT, INT_SIZE, MAXNAME },
		{ "relcat", "attrCount", TYPE_INT, INT_SIZE, MAXNAME+INT_SIZE }};

/* Attribute information for attribute catalog */
const DataAttrInfo dataAttrInfo_attrcat[5] = {
		{ "attrcat", "relName", TYPE_STRING, MAXNAME, 0 },
		{ "attrcat", "attrName", TYPE_STRING, MAXNAME, MAXNAME },
		{ "attrcat", "offset", TYPE_INT, INT_SIZE, 2*MAXNAME},
		{ "attrcat", "attrType", TYPE_INT, INT_SIZE, 2*MAXNAME+INT_SIZE },
		{ "attrcat", "attrLength", TYPE_INT, INT_SIZE, 2*MAXNAME+2*INT_SIZE }};

#endif
