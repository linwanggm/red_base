#include "RM_FileHandle.h"

RM_FileHandle::RM_FileHandle () {
	bOpen         = false;
	pPFManager    = NULL;
	pData_FileHdr = NULL;
}

RM_FileHandle::~RM_FileHandle () {
	if (bOpen) {
		Close ();
	}
}

RC RM_FileHandle::Open (PF_Manager &pfManager, const char *fileName) {
	/* Close file handle if it is already open */
	if (bOpen) {
		Close ();
	}

	/* Store a ptr to the paged file manager */
	this->pPFManager = &pfManager;

	RC rc;
	/* Open the file */
	rc = pPFManager->OpenFile (fileName, pfFileHandle);
	if (rc != SUCCESS) { return rc; }

	/* Get a handle to the RM file header (which is the first page) */
	rc = pfFileHandle.GetFirstPage (pfPageHandle_FileHdr);
	if (rc != SUCCESS) { return rc; }
	/* Get a ptr to the contents (data) of the file header */
	rc = pfPageHandle_FileHdr.GetData (pData_FileHdr);
	if (rc != SUCCESS) { return rc; }
	/* Get the page number of the header */
	rc = pfPageHandle_FileHdr.GetPageNum (pageNum_FileHdr);
	if (rc != SUCCESS) { return rc; }
	
	/* Read in the file header successfully; now get the sub-header
	 * information from the header */
	memcpy (&fileSubHeader, &pData_FileHdr[0], RM_FILESUBHDR_SIZE);

	if (DEBUG) {
		printf ("# records in file = %d, record size = %d\n",
			fileSubHeader.nRecords, fileSubHeader.recordSize);
		printf ("records per page = %d, first record offset = %d\n",
			fileSubHeader.recordsPerPage,
			fileSubHeader.firstRecordOffset);
	}

	/* We have successfully opened a handle to the file */
	bOpen = true;

	return SUCCESS;
}

RC RM_FileHandle::Close () {
	/* If handle is already closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	RC rc;
	/* Unpin the RM header page */
	rc = pfFileHandle.UnpinPage (pageNum_FileHdr);
	if (rc != SUCCESS) { return rc; }

	/* Close the file handle */
	rc = pPFManager->CloseFile (pfFileHandle);
	if (rc != SUCCESS) { return rc; }
	/* Reset ptr's */
	pPFManager    = NULL;
	pData_FileHdr = NULL;

	/* File handle is no longer open */
	bOpen = false;
	return SUCCESS;
}

RC RM_FileHandle::InsertRec (const char *pData, RID &rid) {
	if (!bOpen) { return RM_FHCLOSED; }

	PageNum pageNum = pageNum_FileHdr + 1;
	do {
		if (pfFileHandle.GetPageType(pageNum) == ALLOCATED_PAGE
				&& GetPageType(pageNum) == NON_FULL_PAGE) {
			break;
		}
		else {
			pageNum++;
		}
	} while (pageNum <= pfFileHandle.GetPageCount());


	PF_PageHandle pfPageHandle;
	RC rc;
	/* Create a new page if there are no allocated, NON_FULL pages and
	 * obtain a handle to it */
	if (pageNum > pfFileHandle.GetPageCount()) {
		rc = pfFileHandle.AllocatePage (pfPageHandle);
		if (rc != SUCCESS) { return rc; }
		/* Get the new page number */
		rc = pfPageHandle.GetPageNum (pageNum);
		if (rc != SUCCESS) { return rc; }
		/* We don't need to initialise the page-sub-header and the
		 * bitmap because they are all 0's */
	}
	/* Otherwise, obtain a page handle to the allocated, NON_FULL page
	 * by fetching the page from disk */
	else {
		rc = pfFileHandle.GetThisPage (pageNum, pfPageHandle);
		if (rc != SUCCESS) { return rc; }
	}

	/* Create a page handle object */
	RM_PageHandle rmPageHandle (pfPageHandle, fileSubHeader);

	SlotNum slotNum;
	/* Find an empty slot in the page */
	for (slotNum = 0; slotNum < fileSubHeader.recordsPerPage; slotNum++) {
		if (rmPageHandle.GetSlotType(slotNum) == EMPTY_SLOT) {
			break;
		}
	}

	char *pRecData;
	/* Get the data contained in the slot */
	rmPageHandle.GetRecData (slotNum, pRecData);
	/* Copy the passed data into the slot */
	memcpy (pRecData, pData, fileSubHeader.recordSize);
	/* Mark this slot as an OCCUPIED_SLOT */
	rmPageHandle.SetSlotType (slotNum, OCCUPIED_SLOT);
	/* Update file-header information if the page is full */
	if (rmPageHandle.PageFull()) {
		SetPageType (pageNum, FULL_PAGE);
	}
	/* Increment record count in file header */
	fileSubHeader.nRecords++;
	/* Update the file header */
	rc = UpdateFileHeader ();
	if (rc != SUCCESS) { return rc; }
	/* Mark the page as dirty because we modified it */
	rc = pfFileHandle.MarkDirty (pageNum);
	if (rc != SUCCESS) { return rc; }
	/* Unpin the page */
	rc = pfFileHandle.UnpinPage (pageNum);
	if (rc != SUCCESS) { return rc; }

	/* Set the passed RID's page number and slot number */
	rid = RID (pageNum, slotNum);
	/* Successfuly inserted record */
	return SUCCESS;
}

RC RM_FileHandle::GetRec (const RID &rid, RM_Record &rec) const {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	PageNum pageNum;
	SlotNum slotNum;
	RC rc;

	/* Get the rid's page and slot numbers */
	rc = GetPageNumSlotNum (rid, pageNum, slotNum);
	if (rc != SUCCESS) { return rc; }

	PF_PageHandle pfPageHandle;
	/* Get a page handle to the page with page number pageNum */
	rc = pfFileHandle.GetThisPage (pageNum, pfPageHandle);
	if (rc != SUCCESS) { return rc; }

	/* Create a page handle */
	RM_PageHandle rmPageHandle (pfPageHandle, fileSubHeader);
	/* Check for validity of slot number */
	if (rmPageHandle.GetSlotType(slotNum) == EMPTY_SLOT) {
		return RM_INVALIDRID;
	}

	char *pRecData;
	/* Get record's contents */
	rmPageHandle.GetRecData (slotNum, pRecData);	
	/* Free up previously allocated memory */
	if (rec.pData) { free (rec.pData); }	
	/* Create a copy of the data */
	rec.pData = (char *) malloc (fileSubHeader.recordSize);
	/* Copy the contents */
	memcpy (rec.pData, pRecData, fileSubHeader.recordSize);
	/* Set the rid */
	rec.rid = rid;
	/* Record is now a valid record */
	rec.bValid = true;
	/* Unpin the page we fetched from disk */
	rc = pfFileHandle.UnpinPage (pageNum);
	if (rc != SUCCESS) { return rc; }

	/* Successfuly retrieved record */
	return SUCCESS;
}

RC RM_FileHandle::DeleteRec (const RID &rid) {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	PageNum pageNum;
	SlotNum slotNum;
	RC rc;

	/* Get the rid's page and slot numbers */
	rc = GetPageNumSlotNum (rid, pageNum, slotNum);
	if (rc != SUCCESS) { return rc; }

	PF_PageHandle pfPageHandle;
	/* Get a page handle to the page with page number pageNum */
	rc = pfFileHandle.GetThisPage (pageNum, pfPageHandle);
	if (rc != SUCCESS) { return rc; }

	/* Create a page handle */
	RM_PageHandle rmPageHandle (pfPageHandle, fileSubHeader);
	/* Check for validity of slot number */
	if (rmPageHandle.GetSlotType(slotNum) == EMPTY_SLOT) {
		return RM_INVALIDRID;
	}
	/* Update the slot type for the given rid */
	rmPageHandle.SetSlotType (slotNum, EMPTY_SLOT);
	if (rc != SUCCESS) { return rc; }
	/* Update file-header information */
	SetPageType (pageNum, NON_FULL_PAGE);
	/* Decrement record count in file header */
	fileSubHeader.nRecords--;
	/* Update the file header */
	rc = UpdateFileHeader ();
	if (rc != SUCCESS) { return rc; }
	/* Mark the page as dirty because we modified it */
	rc = pfFileHandle.MarkDirty (pageNum);
	if (rc != SUCCESS) { return rc; }
	/* Unpin the page we fetched from disk */
	rc = pfFileHandle.UnpinPage (pageNum);
	if (rc != SUCCESS) { return rc; }

	/* Record successfully deleted */
	return SUCCESS;
}

RC RM_FileHandle::UpdateRec (const RM_Record &rec) {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	PageNum pageNum;
	SlotNum slotNum;
	RC rc;
	RID rid;

	/* Get the record's rid first */
	rc = rec.GetRid (rid);
	if (rc != SUCCESS) { return rc; }
	/* Get the rid's page and slot numbers */
	rc = GetPageNumSlotNum (rid, pageNum, slotNum);
	if (rc != SUCCESS) { return rc; }

	PF_PageHandle pfPageHandle;
	/* Get a page handle to the page with page number pageNum */
	rc = pfFileHandle.GetThisPage (pageNum, pfPageHandle);
	if (rc != SUCCESS) { return rc; }

	/* Create a page handle */
	RM_PageHandle rmPageHandle (pfPageHandle, fileSubHeader);
	/* Check for validity of slot number */
	if (rmPageHandle.GetSlotType(slotNum) == EMPTY_SLOT) {
		return RM_INVALIDRID;
	}

	char *pRecData;
	/* Get a ptr to the record */
	rmPageHandle.GetRecData (slotNum, pRecData);
	/* Update record's contents */
	memcpy (pRecData, rec.pData, fileSubHeader.recordSize);
	/* Mark the page as dirty because we modified it */
	rc = pfFileHandle.MarkDirty (pageNum);
	if (rc != SUCCESS) { return rc; }
	/* Unpin the page we fetched from disk */
	rc = pfFileHandle.UnpinPage (pageNum);
	if (rc != SUCCESS) { return rc; }

	/* Record successfully updated */
	return SUCCESS;
}

RC RM_FileHandle::ForcePage (PageNum pageNum) const {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	return pfFileHandle.ForcePage (pageNum);
}

RC RM_FileHandle::ForceAllPages () const {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	return pfFileHandle.ForceAllPages ();
}

RM_PageType RM_FileHandle::GetPageType (PageNum pageNum) const {
	/* Ptr to the bitmap portion of the file header */
	char *pBitmap = &pData_FileHdr[RM_FILESUBHDR_SIZE];
        /* Determine the byte and bit position of given page number */
        int byte = pageNum/8;
        int bit  = pageNum - byte*8;
        /* Determine the value of the bit */
        int v = (pBitmap[byte] & (1<<bit)) >> bit;
        if (v == 0) {
                return NON_FULL_PAGE;
        }
        else {
                return FULL_PAGE;
        }
}

void RM_FileHandle::SetPageType (PageNum pageNum, RM_PageType pageType) {
	/* Ptr to the bitmap portion of the file header */
	char *pBitmap = &pData_FileHdr[RM_FILESUBHDR_SIZE];
        /* Determine the byte and bit position of given page number */
        int byte = pageNum/8;
        int bit  = pageNum - byte*8;
        /* Determine the value of the bit */
        int v = (pBitmap[byte] & (1<<bit)) >> bit;
                                                                                                                             
        /* If page type is same as the one being set, issue a warning */
        if (v == pageType) {
                /* fprintf (stderr, "==> RM-SetPageType WARNING: pageType
		 * is same"); */
                return;
        }
        else {
                /* Flip the page type */
                pBitmap[byte] ^= (1<<bit);
        }
}

RC RM_FileHandle::GetPageNumSlotNum (const RID &rid, PageNum &pageNum,
		SlotNum &slotNum) const {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	RC rc;

	/* Get page and slot numbers */
	rc = rid.GetPageNum (pageNum);
	if (rc != SUCCESS) { return rc; }
	rc = rid.GetSlotNum (slotNum);
	if (rc != SUCCESS) { return rc; }

	/* Check if page number is ok and if slot number is within bounds */
	if (slotNum >= fileSubHeader.recordsPerPage
		|| slotNum < 0
		|| pageNum < 2
		|| (pfFileHandle.GetPageType(pageNum) == FREE_PAGE
			&& pageNum <= pfFileHandle.GetPageCount()) ) { 
		return RM_INVALIDRID; 
	}

	return SUCCESS;
}

RC RM_FileHandle::UpdateFileHeader () const { 
	/* If handle is closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	/* Copy back the sub-header information because we maintain a local
	 * copy of it */
	memcpy (&pData_FileHdr[0], &fileSubHeader, RM_FILESUBHDR_SIZE);
	/* Because we have changed the header page, we should mark it as dirty */
	RC rc = pfFileHandle.MarkDirty (pageNum_FileHdr);
	return rc;
}

RC RM_FileHandle::GetNumRecords (int &numRecords) const {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	numRecords = fileSubHeader.nRecords;
	return SUCCESS;
}

RC RM_FileHandle::GetNumPages (int &numPages) const {
	/* If handle is  closed, return error */
	if (!bOpen) { return RM_FHCLOSED; }

	numPages = pfFileHandle.GetNumAllocatedPages ();
	return SUCCESS;
}
