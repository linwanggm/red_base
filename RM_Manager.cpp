#include "RM_Manager.h"

RM_Manager::RM_Manager (PF_Manager &pfManager) : pfm(pfManager) {
}

RC RM_Manager::CreateFile (const char *fileName, int recordSize) {
	/* Determine the number of records that can be stored in a single
	 * page. If this number is less than 1, return an error. */
	int recordsPerPage;
	double x = (double)(PF_PAGE_SIZE - RM_PAGESUBHDR_SIZE) - 1.0;
	double y = (double)recordSize + 0.125;
	recordsPerPage = (int)(x/y);

	if (recordsPerPage < 1) {
		return RM_INVALIDRECSIZE;
	}
	if (DEBUG) {
		printf ("recordsPerPage = %d\n", recordsPerPage);
	}

	RC rc;
	/* Create file by using the paged file manager */
	rc = pfm.CreateFile (fileName);
	if (rc != SUCCESS) { return rc; }

	PF_FileHandle pfFileHandle;
	/* Open the created file */
	rc = pfm.OpenFile (fileName, pfFileHandle);
	if (rc != SUCCESS) { return rc; }

	PF_PageHandle pfPageHandle;
	/* Allocate a new page for the RM header page */
	rc = pfFileHandle.AllocatePage (pfPageHandle);
	if (rc != SUCCESS) { return rc; }

	char *pData;
	PageNum pageNum;
	/* Get ptr to the contents (data) of the RM header page */
	rc = pfPageHandle.GetData (pData);
	if (rc != SUCCESS) { return rc; }
	/* Get the page number of the RM header page */
	rc = pfPageHandle.GetPageNum (pageNum);
	if (rc != SUCCESS) { return rc; }

	/* Construct the sub-header */
	RM_FileSubHeader 
		fileSubHeader = { 0,/* # of records currently stored in file */
				  recordSize,
				  recordsPerPage,
				/* offset in bytes of the first record */
			    	  RM_PAGESUBHDR_SIZE+(recordsPerPage+8)/8 
			      	};
	/* Copy	the sub-header to the page */
	memcpy (&pData[0], &fileSubHeader, RM_FILESUBHDR_SIZE);
	/* Initialise the rest of the page. The rest of the page is
	 * reserved for a bitmap for NON_FULL, FULL data pages. NON_FULL
	 * data pages are those pages that have some empty slots in them.
	 * FULL data pages do not have empty slots and therefore cannot
	 * store additional records. */
	memset (&pData[RM_FILESUBHDR_SIZE], 0, PF_PAGE_SIZE-RM_FILESUBHDR_SIZE);
	/* Because we modified the RM header page, we write it to disk */
	rc = pfFileHandle.ForcePage (pageNum);
	if (rc != SUCCESS) { return rc; }
	/* We now unpin the header page because we are done modifying it */
	rc = pfFileHandle.UnpinPage (pageNum);
	if (rc != SUCCESS) { return rc; }

	return SUCCESS;
}

RC RM_Manager::DestroyFile (const char *fileName) {
	return pfm.DestroyFile (fileName);
}

RC RM_Manager::OpenFile (const char *fileName, RM_FileHandle &fileHandle) {
	return fileHandle.Open (pfm, fileName);
}

RC RM_Manager::CloseFile (RM_FileHandle &fileHandle) {
	return fileHandle.Close ();
}
