#include "SM_Manager.h"

SM_Manager::SM_Manager (RM_Manager &rmManager) : rmm(rmManager) { 		 
	bOpen = false;
}

SM_Manager::~SM_Manager () { 
}

RC SM_Manager::CreateDb (const char *dbName) { 
	int ret;
	/* Store the current work directory */
	getcwd (cwd, 80);
	/* Create a directory for the database */
	ret = mkdir (dbName, 00700);
	if (ret == -1) { return PF_UNIX; }
	/* Directory created; chdir to directory */
	ret = chdir (dbName);
	if (ret == -1) { return PF_UNIX; }

	RM_FileHandle fileHandle;
	RID rid;
	SM_Tuple T;
	char *pData;
	RC rc;

	/* Create the relation catalog */
	rc = rmm.CreateFile (relName_relcat, MAXNAME+2*INT_SIZE);
	if (rc != SUCCESS) { return rc; }
	/* Open the relation catalog */
	rc = rmm.OpenFile (relName_relcat, fileHandle);
	if (rc != SUCCESS) { return rc; }
	/* Insert info about relcat into this catalog */
	T = SM_Tuple(dataAttrInfo_relcat, 3, "relcat", MAXNAME+2*INT_SIZE, 3);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	/* Insert info about attrcat into this catalog */
	T = SM_Tuple(dataAttrInfo_relcat, 3, "attrcat", 2*MAXNAME+3*INT_SIZE, 5);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	/* Write pages to disk */
	rc = fileHandle.ForceAllPages ();
	if (rc != SUCCESS) { return rc; }
	/* Close the relation catalog */
	rc = rmm.CloseFile (fileHandle);
	if (rc != SUCCESS) { return rc; }

	/* Create the attribute catalog */
	rc = rmm.CreateFile (relName_attrcat, 2*MAXNAME+3*INT_SIZE);
	if (rc != SUCCESS) { return rc; }
	/* Open the attribute catalog */
	rc = rmm.OpenFile (relName_attrcat, fileHandle);
	if (rc != SUCCESS) { return rc; }
	/* Insert info about relcat into this catalog */
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "relcat", "relName", 0,
			TYPE_STRING, MAXNAME);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "relcat", "tupleLength",
			MAXNAME, TYPE_INT, INT_SIZE);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "relcat", "attrCount",
			MAXNAME+INT_SIZE, TYPE_INT, INT_SIZE);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	/* Insert info about attrcat into this catalog */
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "attrcat", "relName", 0,
			TYPE_STRING, MAXNAME);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "attrcat", "attrName",
			MAXNAME, TYPE_STRING, MAXNAME);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "attrcat", "offset",
			2*MAXNAME, TYPE_INT, INT_SIZE);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "attrcat", "attrType",
			2*MAXNAME+INT_SIZE, TYPE_INT, INT_SIZE);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	T = SM_Tuple(dataAttrInfo_attrcat, 5, "attrcat", "attrLength",
			2*MAXNAME+2*INT_SIZE, TYPE_INT, INT_SIZE);
		T.GetData(pData);
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { return rc; }
	/* Write pages to disk */
	rc = fileHandle.ForceAllPages ();
	if (rc != SUCCESS) { return rc; }
	/* Close the relation catalog */
	rc = rmm.CloseFile (fileHandle);
	if (rc != SUCCESS) { return rc; }

	/* Catalogs created; return to the original directory */
	chdir (cwd);

	return SUCCESS;
}

RC SM_Manager::DestroyDb (const char *dbName) { 
	char cmd[80];
	/* Build system command to delete directory */
	sprintf (cmd, "rm -r %s", dbName);
	/* Execute sys command */
	if (system (cmd) == -1) {
		return PF_UNIX;
	}
	return SUCCESS;
}

RC SM_Manager::OpenDb (const char *dbName) { 
	/* Check if we are already connected to a database */
	if (bOpen) { return SM_DBOPEN; }

	getcwd (cwd, 80);
	/* chdir to the DB directory */
	if (chdir(dbName) < 0) {
		fprintf (stderr, "could not change directory to %s\n", dbName);
		return PF_UNIX;
	}

	RC rc;
	/* Open a handle to the relation catalog */
	rc = rmm.OpenFile (relName_relcat, fileHandle_relcat);
	if (rc != SUCCESS) { return rc; }
	/* Open a handle to the attribute catalog */
	rc = rmm.OpenFile (relName_attrcat, fileHandle_attrcat);
	if (rc != SUCCESS) { return rc; }

	/* Successfully opened catalog files */
	bOpen = true;
	return SUCCESS;
}

RC SM_Manager::CloseDb () { 
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	RC rc;
	/* Flush dirty catalog pages to disk */
	rc = fileHandle_relcat.ForceAllPages ();
	if (rc != SUCCESS) { return rc; }
	rc = fileHandle_attrcat.ForceAllPages ();
	if (rc != SUCCESS) { return rc; }
	/* Close handle to the relation catalog */
	rc = rmm.CloseFile (fileHandle_relcat);
	if (rc != SUCCESS) { return rc; }
	/* Close handle to the attribute catalog */
	rc = rmm.CloseFile (fileHandle_attrcat);
	if (rc != SUCCESS) { return rc; }

	/* Get back to the previous directory */
	chdir (cwd);

	/* Successfully closed catalog files */
	bOpen = false;
	return SUCCESS;
}

RC SM_Manager::CreateTable (const char *relName, 
				 int attrCount,
				 AttrInfo *attributes) {
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	/* Check if relName is either "relcat" or "attrcat" */
	if (strcmp(relName, relName_relcat) == 0
		|| strcmp(relName, relName_attrcat) ==0 ) {
			return SM_RELNAMECAT;
	}

	DataAttrInfo *info; 
	int nAttr;
	/* Check if the relation already exits */
	if (GetDataAttrInfo(relName, info, nAttr) == SUCCESS) {
		free (info);
		return SM_RELEXISTS;
	}

	int recordSize = 0;
	/* Determine record size of the relation to be created */
	for (int i = 0; i < attrCount; i++) {
		recordSize += attributes[i].attrLength;
	}

	RC rc;
	/* Create a new file for this relation */
	rc = rmm.CreateFile (relName, recordSize);
	if (rc != SUCCESS) { 
		return rc; 
	}

	SM_Tuple T;
	RID rid_relcat;
	char *pData;
	/* Update the relation catalog */
	T = SM_Tuple(dataAttrInfo_relcat, 3, relName, recordSize, attrCount);
	T.GetData(pData);
	rc = fileHandle_relcat.InsertRec (pData, rid_relcat);
	/* Clean up if an error occurs */
	if (rc != SUCCESS) { 
		rmm.DestroyFile (relName);
		return rc; 
	}	

	RID rid_attrcat;
	/* Update the attribute catalog */
	int offset = 0;
	for (int i = 0; i < attrCount; i++) {
		/* Construct data ptr */
		T = SM_Tuple(dataAttrInfo_attrcat, 5, relName,
				attributes[i].attrName, offset,
				attributes[i].attrType,
				attributes[i].attrLength);
		T.GetData(pData);
		offset += attributes[i].attrLength;
		/* Insert record into the catalog */
		rc = fileHandle_attrcat.InsertRec (pData, rid_attrcat);
		/* Clean up and return if an error occurs */
		if (rc != SUCCESS) { 
			rmm.DestroyFile (relName);
			return rc; 
		}
	}

	/* Successfully created table */
	return SUCCESS;
}

RC SM_Manager::DropTable (const char *relName) { 
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	/* Check if relName is either "relcat" or "attrcat" */
	if (strcmp(relName, relName_relcat) == 0
		|| strcmp(relName, relName_attrcat) ==0 ) {
			return SM_RELNAMECAT;
	}

	RC rc;
	/* Drop the file used by this relation */
	rc = rmm.DestroyFile (relName);
	if (rc != SUCCESS) { return rc; }

	RM_FileScan fileScan;
	RM_Record record;
	RID rid;

	char rName[MAXNAME+1];
	memset (rName, 0, MAXNAME+1);
	/* Make a copy of relName for use in file scans */
	strcpy (rName, relName);
	/* Update the relation catalog */
	rc = fileScan.OpenScan (fileHandle_relcat, TYPE_STRING,
			MAXNAME, 0, EQ_OP, rName, NO_HINT);
	if (rc != SUCCESS) { return rc; }
	/* Get the tuple corresponding to the deleted relation */
	rc = fileScan.GetNextRec (record);
	if (rc != SUCCESS) { return rc; }
	/* Get the tuple's rid */
	record.GetRid (rid);
	/* Delete the tuple */
	rc = fileHandle_relcat.DeleteRec (rid);
	if (rc != SUCCESS) { return rc; }
	/* Close the scan */
	rc = fileScan.CloseScan ();
	if (rc != SUCCESS) { return rc; }

	/* Update the attribute catalog */
	rc = fileScan.OpenScan (fileHandle_attrcat, TYPE_STRING,
			MAXNAME, 0, EQ_OP, rName, NO_HINT);
	if (rc != SUCCESS) { return rc; }
	/* Get the tuples corresponding to the deleted relation */
	while (fileScan.GetNextRec (record) == SUCCESS) {
		/* Get tuple's rid */
		rc = record.GetRid (rid);
		if (rc != SUCCESS) { 
			fileScan.CloseScan ();
			return rc;
		}
		/* Delete tuple */
		rc = fileHandle_attrcat.DeleteRec (rid);
		/* Clean up before returning from an error */
		if (rc != SUCCESS) { 
			fileScan.CloseScan ();
			return rc; 
		}
	}
	/* Close the scan */
	rc = fileScan.CloseScan ();
	if (rc != SUCCESS) { return rc; }

	/* Successfully deleted the relation and updated the catalogs */
	return SUCCESS;
}

RC SM_Manager::DeleteRecords (const char *relName) {
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }
	/* Check if relName is either "relcat" or "attrcat" */
	if (strcmp(relName, relName_relcat) == 0
		|| strcmp(relName, relName_attrcat) ==0 ) {
			return SM_RELNAMECAT;
	}

	/* Step 1 */
	RC rc;
	/* Determine the structure of the relation */
	DataAttrInfo *info;
	int attrCount;
	rc = GetDataAttrInfo (relName, info, attrCount);
	if (rc != SUCCESS) { return rc; }
	/* We don't need the structure information for deleting records */
	/* Free up memory */
	free (info);

	/* Step 2 */
	/* Scan the relation and delete each record */
	RM_FileHandle fileHandle;
	/* Get a handle to the file first */
	rc = rmm.OpenFile (relName, fileHandle);
	if (rc != SUCCESS) { return rc; }

	RM_FileScan fileScan;
	/* Open a file scan using the file handle */
	rc = fileScan.OpenScan (fileHandle, 
			TYPE_STRING, MAXNAME, 0, /* <-- Doesn't matter what
						    these values are
						    because the scan is
						    based on NO_OP */
			NO_OP, NULL, NO_HINT);
	if (rc != SUCCESS) { return rc; }

	RM_Record rec;
	/* Get each record from the file */
	while (fileScan.GetNextRec(rec) == SUCCESS) {
		char *pData;
		/* Get the record's contents (data) */
		rc = rec.GetData (pData);
		/* Clean up before returning from an error */
		if (rc != SUCCESS) {
			fileScan.CloseScan ();
			return rc;
		}
		RID rid;
		/* Get the record's rid */
		rc = rec.GetRid (rid);
		if (rc != SUCCESS) { 
			fileScan.CloseScan ();
			return rc; 
		}
		/* Delete the record */
		rc = fileHandle.DeleteRec (rid);
		if (rc != SUCCESS) { 
			fileScan.CloseScan ();
			return rc; 
		}
	}

	/* Close the scan */
	rc = fileScan.CloseScan ();
	if (rc != SUCCESS) { return rc; }

	/* Flush dirty pages to disk */
	rc = fileHandle.ForceAllPages ();
	if (rc != SUCCESS) { return rc; }
	/* Close the file handle */
	rc = rmm.CloseFile (fileHandle);
	if (rc != SUCCESS) { return rc; }

	/* Successfully printed relation */
	return SUCCESS;
}

RC SM_Manager::Load (const char *relName, const char *fileName) {
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }
	/* Check if relName is either "relcat" or "attrcat" */
	if (strcmp(relName, relName_relcat) == 0
		|| strcmp(relName, relName_attrcat) ==0 ) {
			return SM_RELNAMECAT;
	}

	DataAttrInfo *info;
	int attrCount;
	RC rc;
	/* Get the structure of the relation */
	rc = GetDataAttrInfo (relName, info, attrCount);
	if (rc != SUCCESS) { return rc; }

	int tupleSize = 0;
	/* Determine the tuple size of this relation */
	tupleSize = info[attrCount-1].offset +
		info[attrCount-1].attrLength;

	RM_FileHandle fileHandle;
	/* Open the table */
	rc = rmm.OpenFile (relName, fileHandle);
	if (rc != SUCCESS) { 
		free (info);
		return rc; 
	}

	/* Open the file for loading */
	FILE *ifp; // Input file pointer
	ifp = fopen (fileName, "r");
	if (ifp == NULL) {
		free (info);
		perror ("ERR");
		return PF_UNIX;
	}

	/* Store line number information */
	int lineNum = 1;

	/* Read in each line of the file */
	while (!feof(ifp)) {
		char *pData;
		/* Create an "empty" tuple */
		pData = (char *) malloc(tupleSize);
		memset (pData, 0, tupleSize);
		SM_Tuple T (info, attrCount, pData);	
				// Store each line of input in tuple T 
				// first. Then use this tuple to insert 
				// a record into the file.

		int ret = 0;
		/* Parse the input buffer */
		for (int i = 0; i < attrCount; i++) {
			ret = 0;
			switch (info[i].attrType) {
				case TYPE_INT:
					int j;
					ret = fscanf (ifp, " %d ", &j);
					if (ret < 1) { break; }
					//printf ("int = %d\n", j);
					T.SetValue (i, &j);
					break;
				case TYPE_FLOAT:
					float f;
					ret = fscanf (ifp, " %f ", &f);
					if (ret < 1) { break; }
					//printf ("float = %f\n", f);
					T.SetValue (i, &f);
					break;
				case TYPE_STRING:
					char s[MAXSTRINGLEN+1];
					memset (s, 0, MAXSTRINGLEN+1);
					ret = fscanf (ifp, " '%[^'\n]' ", s);
					if (ret < 1) { break; }
					//printf ("string = %s\n", s);
					T.SetValue (i, s);
					break;
			}
			/* Check if an error occured */
			/* If we reached end of file exit the loop */
			if (ret == -1 && i==0) { 
				break; 
			}
			/* If we encountered an error, clean up and return */
			else if (ret < 1) {
			  	fprintf (stderr, "*** Error reading attribute ***");
				fprintf (stderr, " %s at line %d ***\n", info[i].attrName, lineNum);
				fprintf (stderr, "Saving the %d tuples read so far..\n", lineNum-1);
				/* Save records read so far */
				fileHandle.ForceAllPages ();
				/* Free up memory */
				free (info);
				fclose (ifp);
				/* Close file */
				rmm.CloseFile (fileHandle);
				/* Return error */
				return SM_BADINPUT;
			}
		}
		/* Exit loop if an error has occured */
		if (ret < 1) {
			break;
		}
		/* Get the tuple's contents */
		T.GetData (pData);
		/* Insert record */
		RID rid;
		rc = fileHandle.InsertRec (pData, rid);
		if (rc != SUCCESS) { 
			free (info);
			fclose (ifp);
			return rc; 
		}
		/* free up memory now (after inserting record) */
		free (pData); pData = NULL;
		/* Increment line number */
		lineNum++;
	}


	/* Close the input file */
	fclose (ifp);
	/* Free allocated memory */
	free (info);
	/* Flush dirty pages to disk */
	rc = fileHandle.ForceAllPages ();
	if (rc != SUCCESS) { return rc; }
	/* Close the file handle */
	rc = rmm.CloseFile (fileHandle);
	if (rc != SUCCESS) { return rc; }

	printf ("%d tuples read\n", lineNum-1);
	/* Successfully loaded data into the file */
	return SUCCESS;
}

RC SM_Manager::Help () { 
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	printf ("Tables in this database:\n");
	return Print (relName_relcat);
}
RC SM_Manager::Help (const char *relName) { 
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	DataAttrInfo *info;
	int attrCount;
	RC rc;
	/* Determine the structure of the relation */
	rc = GetDataAttrInfo (relName, info, attrCount);
	if (rc != SUCCESS) { return rc; }

	printf ("Help for %s:\n", relName);
	/* Print the tuple header */
	PrintTupleHeader (dataAttrInfo_attrcat, 5);
	/* Print out the structure */
	for (int i = 0; i < attrCount; i++) {
		SM_Tuple T (dataAttrInfo_attrcat, 5, 
				info[i].relName, 
				info[i].attrName,
				info[i].offset,
				info[i].attrType,
				info[i].attrLength);
		T.PrintData ();
	}
	
	/* Free up memory allocated to info */
	free (info);
	/* Successfully printed help */
	return SUCCESS;
}

RC SM_Manager::CrossProduct (const char *rel1, const char *rel2) {
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	const char *relName_outer = rel1;
	const char *relName_inner = rel2;

	RC rc;
	RM_FileHandle fileHandle_outer, fileHandle_inner;
	/* Get a handle to the outer relation */
	rc = rmm.OpenFile (relName_outer, fileHandle_outer);
	if (rc != SUCCESS) { return rc; }
	/* Get a handle to the inner relation */
	rc = rmm.OpenFile (relName_inner, fileHandle_inner);
	if (rc != SUCCESS) { return rc; }

	RM_FileScan fileScan_outer, fileScan_inner;
	/* Open a file scan on the outer relation */
	rc = fileScan_outer.OpenScan (fileHandle_outer, 
			TYPE_STRING, MAXNAME, 0, /* <-- Doesn't matter what
						    these 3 values are
						    because the scan is
						    based on NO_OP */
			NO_OP, NULL, NO_HINT);
	if (rc != SUCCESS) { return rc; }
	/* Open a file scan on the inner relation */
	rc = fileScan_inner.OpenScan (fileHandle_inner, 
			TYPE_STRING, MAXNAME, 0, /* <-- Doesn't matter what
						    these 3 values are
						    because the scan is
						    based on NO_OP */
			NO_OP, NULL, NO_HINT);
	if (rc != SUCCESS) { return rc; }

	/* Determine the attributes present in each relation */
	DataAttrInfo *info_outer, *info_inner;
	int attrCount_outer, attrCount_inner;
	/* Get attribute information for the outer relation */
	rc = GetDataAttrInfo (relName_outer, info_outer, attrCount_outer);
	if (rc != SUCCESS) { return rc; }
	/* Get attribute information for the inner relation */
	rc = GetDataAttrInfo (relName_inner, info_inner, attrCount_inner);
	if (rc != SUCCESS) { 
		free (info_outer);
		return rc; 
	}

	RM_Record rec_outer, rec_inner;
	char *pData;
	int  nRecords = 0; /* # of records in cross-product */

	/* Do the cross product */
	while (fileScan_outer.FetchPages() == SUCCESS) {
		while (fileScan_inner.FetchPages() == SUCCESS) {
			while (fileScan_outer.GetNextRecInMem(rec_outer) == SUCCESS) {
				/* Build an SM_Tuple object from the outer record */
				rec_outer.GetData (pData);
				SM_Tuple T_outer (info_outer, attrCount_outer, pData);
				while (fileScan_inner.GetNextRecInMem(rec_inner) == SUCCESS) {
					/* Build an SM_Tuple object from the inner record */
					rec_inner.GetData (pData);
					SM_Tuple T_inner (info_inner, attrCount_inner, pData);

					/* Combine the two tuples */
					SM_Tuple T = T_outer + T_inner;
					/* Print the tuple */
					T.PrintData ();

					/* Increment count */
					nRecords++;
				}
				fileScan_inner.JumpToFirstRecInMem();
			}
			fileScan_outer.JumpToFirstRecInMem();
		}

		/* Re-open a file scan on the inner relation */
		fileScan_inner.CloseScan();
		rc = fileScan_inner.OpenScan (fileHandle_inner, TYPE_STRING, 
				MAXNAME, 0, NO_OP, NULL, NO_HINT);
		if (rc != SUCCESS) { 
			free (info_outer); free (info_inner);
			return rc; 
		}
	}

	/* Print # of records in the cross-product */
	printf ("%d records in cross-product\n", nRecords);

	/* Free up attribute information for the inner relation */
	free (info_inner);
	/* Free up attribute information for the outer relation */
	free (info_outer);
	/* Close the inner file scan */
	rc = fileScan_inner.CloseScan ();
	if (rc != SUCCESS) { return rc; }
	/* Close the outer file scan */
	rc = fileScan_outer.CloseScan ();
	if (rc != SUCCESS) { return rc; }
	/* Close the inner file handle */
	rc = rmm.CloseFile (fileHandle_inner);
	if (rc != SUCCESS) { return rc; }
	/* Close the outer file handle */
	rc = rmm.CloseFile (fileHandle_outer);
	if (rc != SUCCESS) { return rc; }
	
	/* Successfully joined relations */
	return SUCCESS;
}

RC SM_Manager::EquiJoin (const char *rel1, const char *rel2,
			 const int *attrList1, const int *attrList2,
			 int condCount) {
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	/*
	   // Uncomment these lines
	const char *relName_outer = rel1;
	const char *relName_inner = rel2;
	const int  *attrList_outer = attrList1;
	const int  *attrList_inner = attrList2;
	*/

	/* ---- Write your equi-join code here ---- */


	/* Successfully joined relations */
	return SUCCESS;
}

RC SM_Manager::EquiJoin_Opt (const char *rel1, const char *rel2,
			 const int *attrList1, const int *attrList2,
			 int condCount) {
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	/*
	   // Uncomment these lines
	const char *relName_outer = rel1;
	const char *relName_inner = rel2;
	const int  *attrList_outer = attrList1;
	const int  *attrList_inner = attrList2;
	*/

	/* ---- Write your optimized equi-join code here ---- */


	/* Successfully joined relations */
	return SUCCESS;
}

RC SM_Manager::Print (const char *relName) { 
	/* Check if we are not connected to a database */
	if (!bOpen) { return SM_DBCLOSED; }

	/* Step 1 */
	RC rc;
	/* Determine the structure of the relation */
	DataAttrInfo *info;
	int attrCount;
	rc = GetDataAttrInfo (relName, info, attrCount);
	if (rc != SUCCESS) { return rc; }

	/* Step 2 */
	/* Scan the relation and print its tuples */
	RM_FileHandle fileHandle;
	/* Get a handle to the file first */
	rc = rmm.OpenFile (relName, fileHandle);
	if (rc != SUCCESS) { 
		free (info);
		return rc; 
	}

	RM_FileScan fileScan;
	/* Open a file scan using the file handle */
	rc = fileScan.OpenScan (fileHandle, 
			TYPE_STRING, MAXNAME, 0, /* <-- Doesn't matter what
						    these values are
						    because the scan is
						    based on NO_OP */
			NO_OP, NULL, NO_HINT);
	if (rc != SUCCESS) { 
		free (info);
		return rc; 
	}

	/* Print the tuple header */
	/* like this
	                      id1|       id2|
             ----------+----------+
	*/
	PrintTupleHeader (info, attrCount);

	RM_Record rec;
	/* Get each record from the file */
	while (fileScan.GetNextRec(rec) == SUCCESS) {
		char *pData;
		/* Get the record's contents (data) */
		rc = rec.GetData (pData);
		/* Clean up before returning from an error */
		if (rc != SUCCESS) {
			free (info);
			fileScan.CloseScan ();
			return rc;
		}
		/* Create a tuple from the record */
		SM_Tuple T (info, attrCount, pData);
		/* Print the tuple to the standard output */
		T.PrintData ();
	}

	/* Free up relation structure information */
	free (info);
	/* Close the scan */
	rc = fileScan.CloseScan ();
	if (rc != SUCCESS) { return rc; }

	/* Print info about relation */
	int nRecords, nPages;
	rc = fileHandle.GetNumRecords(nRecords);
	if (rc != SUCCESS) { return rc; }
	rc = fileHandle.GetNumPages(nPages);
	if (rc != SUCCESS) { return rc; }
	fprintf (stdout, "%d tuples occupying %d pages\n", nRecords, nPages);
	/* Close the file handle */
	rc = rmm.CloseFile (fileHandle);
	if (rc != SUCCESS) { return rc; }

	/* Successfully printed relation */
	return SUCCESS;
}

RC SM_Manager::GetDataAttrInfo (const char *relName, DataAttrInfo *&info,
		int &attrCount) const {
	/* Pad relName because we are going to use it in a file scan. */
	char rName[MAXNAME+1];
	memset (rName, 0, MAXNAME+1);
	strcpy (rName, relName);

	/* Step 1 */
	/* Use the relation catalog to verify that the relation exists */
	RM_FileScan fileScan;
	RM_Record rec;
	RC rc;
	/* Open a scan on relcat */
	rc = fileScan.OpenScan (fileHandle_relcat, 
			/* dataAttrInfo_relcat[0] corresponds to attribute relName 
			   of relation relcat */
				dataAttrInfo_relcat[0].attrType,
				dataAttrInfo_relcat[0].attrLength,
				dataAttrInfo_relcat[0].offset,
				EQ_OP, rName, NO_HINT);
	/* If there are no records corresponding to this relation, flag an
	 * error */
	if (fileScan.GetNextRec(rec) != SUCCESS) {
		fileScan.CloseScan ();
		return SM_RELNOTFOUND;
	}
	/* Close the scan */
	rc = fileScan.CloseScan ();
	if (rc != SUCCESS) { return rc; }

	/* Step 2 */
	/* We now know that the relation exists. Use the attribute catalog 
	   to determine the structure of the relation. */
	/* Open a scan on attrcat */
	
	   /*
        relName				   |tupleLength| attrCount|
        ------------------------+----------+--
       relcat				   |		   32| 		3
       attrcat				   |		   60| 		5
       t1					   |		     8| 		2

      */
	rc = fileScan.OpenScan (fileHandle_attrcat, 
			/* dataAttrInfo_attrcat[0] corresponds to attribute relName 
			   of relation attrcat */
				dataAttrInfo_attrcat[0].attrType,
				dataAttrInfo_attrcat[0].attrLength,
				dataAttrInfo_attrcat[0].offset,
				EQ_OP, rName, NO_HINT);
	/* initialise attrCount and info */
	attrCount = 0; info = NULL;
	/* get the structure of each attribute */
	while (fileScan.GetNextRec(rec) == SUCCESS) {
		char *pData;
		/* Get the content's of the record */
		rec.GetData (pData);
		/* Build a tuple from the retrieved record */
		SM_Tuple T (dataAttrInfo_attrcat, 5, pData);
		/* Retrieve the name,type,offset,length of each attribute */
       /*
               relName             |attrName       |    offset|  attrType|attrLength|
               ------------------------+------------------------+
              relcat                  |tupleLength    |       24|         0|         4
              relcat                  |attrCount       |       28|         0|         4
              attrcat                |relName         |         0|         2|        24
              attrcat                |attrName       |       24|         2|        24
              attrcat                |offset             |       48|         0|         4
              attrcat                |attrType         |       52|         0|         4
              attrcat                |attrLength      |       56|         0|         4

            */
		DataAttrInfo d;
			strcpy (d.relName, rName);
			T.GetValue(1, d.attrName); 
			T.GetValue(2, &d.offset);
			T.GetValue(3, &d.attrType);
			T.GetValue(4, &d.attrLength);
		/* Increase size of structure by 1 */
		attrCount++;
		info = (DataAttrInfo *) realloc (info,
				attrCount*sizeof(DataAttrInfo));
		/* Copy structure information into info */
		info[attrCount-1] = d;
	}
	/* Close the scan */
	rc = fileScan.CloseScan ();
	if (rc != SUCCESS) { return rc; }

	return SUCCESS;
}

void SM_Manager::PrintTupleHeader (const DataAttrInfo *info, 
					int attrCount) const {

    /* Print the tuple header */
	/* like this
	                      id1|       id2|
             ----------+----------+
	*/
	
	/* Print out the attribute names */
	for (int i = 0; i < attrCount; i++) {
		if (info[i].attrType == TYPE_STRING) {
			printf ("%-*s|", info[i].attrLength,
					info[i].attrName);
		}
		else {
			printf ("%*s|", 10,
					info[i].attrName);
		}
	}
	printf ("\n");
	for (int i = 0; i < attrCount; i++) {
		int d;
		switch (info[i].attrType) {
			case TYPE_INT:
			case TYPE_FLOAT:  d = 10; break;
			case TYPE_STRING: d = info[i].attrLength; break;
		}
		for (int j = 0; j < d; j++) {
			putchar ('-');
		}
		putchar ('+');
	}
	printf ("\n");
}
