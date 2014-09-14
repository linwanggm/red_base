#include "RM_FileScan.h"

RM_FileScan::RM_FileScan () {
	bOpen = false;
	pRMFileHandle = NULL;
	value = NULL;
}

/* This method initializes a scan over the records in the open file
 * referred to by parameter fileHandle. During the scan, only those records
 * whose specified attribute satisfies the specified condition (a
 * comparison with a value) should be retrieved. If "value" is a null
 * pointer, then there is no condition and all records are retrieved during
 * the scan. If value is not a null pointer, then "value" points to the value
 * that attributes are to be compared with. */
RC RM_FileScan::OpenScan (const RM_FileHandle &fileHandle,  
			     AttrType      attrType,
			     int           attrLength,
			     int           attrOffset,
			     CompOp        compOp,
			     const void    *value,
			     ClientHint    pinHint) {
	/* If scan is already open, return error */
	if (bOpen) {
		return RM_FSOPEN;
	}

	/* Initialise local copies of passed parameters */
	this->pRMFileHandle = &fileHandle;
	this->attrType    = attrType;			
	this->attrLength  = attrLength;			
	this->attrOffset  = attrOffset;			
	this->compOp      = compOp;
	this->pinHint     = pinHint;
	if (compOp != NO_OP) {
		this->value = malloc (attrLength);
		memcpy (this->value, value, attrLength);
	}

	/* Initialise variables */
	pinnedPageCount = 0;
	phIx = 0;
	snIx = 0;
	pnLast = 1; 

	/* Store a copy of the RM_FileSubHeader for convenience */
	rmFileSubHeader = pRMFileHandle->fileSubHeader;

	/* Opened file scan successfully */
	bOpen = true;
	return SUCCESS;
}

RC RM_FileScan::GetNextRec (RM_Record &rec) {
	if (!bOpen) { return RM_FSCLOSED; }

	RC rc;
	/* Try to fetch the next record from memory */
	rc = GetNextRecInMem (rec);

	if (rc == RM_NOMORERECINMEM) {
		/* Fetch pages if there are no more records in memory */
		rc = FetchPages ();
		/* Check if we successfully fetched pages from disk */
		if (rc != SUCCESS) { return rc; }
		/* Once pages have been fetched from disk, return the first record
		 * in memory. */
		return GetNextRecInMem (rec);
	}
	else  {
		/* If we get an error code that is not RM_NOMORERECINMEM or
		 * SUCCESS, then return the error code */
		if (rc != SUCCESS) { return rc; }
	}

	/* Successfully fetched a record */
	return SUCCESS;
}

RC RM_FileScan::CloseScan () {
	/* If file scan is already closed, return error */
	if (!bOpen) { return RM_FSCLOSED; }

	/* Free up memory */
	free (value);

	RC rc;
	/* Unpin any pinned pages */
	for (int i = 0; i < pinnedPageCount; i++)
	{
		PageNum pageNum;
		/* Determine page number of the pinned page */
		rc = pfPageHandles[i].GetPageNum (pageNum);
		if (rc != SUCCESS) { return rc; }
		/* Unpin page */
		rc = pRMFileHandle->pfFileHandle.UnpinPage (pageNum);
		if (rc != SUCCESS) { return rc; }
	}
	/* Reset variables */
	pinnedPageCount = 0;
	phIx = 0;
	snIx = 0;
	pnLast = 1;


	/* Reset ptr's */
	value = NULL;
	pRMFileHandle = NULL;
	/* Scan is no longer open */
	bOpen = false;

	return SUCCESS;
}

RC RM_FileScan::FetchPages () {
	if (!bOpen) { return RM_FSCLOSED; }

	/* We first unpin previously pinned pages */
	if (pinnedPageCount > 0) {
		/* Unpin previously pinned pages */
		for (int i = 0; i < pinnedPageCount; i++)
		{
			PageNum pageNum;
			RC rc;
			/* Determine page number of the pinned page */
			rc = pfPageHandles[i].GetPageNum (pageNum);
			if (rc != SUCCESS) { return rc; }
			/* Unpin page */
			rc = pRMFileHandle->pfFileHandle.UnpinPage (pageNum);
			if (rc != SUCCESS) { return rc; }
		}
	}


	/* Set pinned page count */
	pinnedPageCount = 0;
	/* Set phIx to point to the first handle in the list of
	 * page handles */
	phIx = 0;
	/* Set snIx to point to the first slot */
	snIx = 0;

	/* Determine number of pages to be fetched and pinned in buffer */
	switch (pinHint) {
		case NO_HINT:
			N = 1;
			break;
		case EFFICIENT:
			/* N = # of pages to be fetched */
			N = bfm.GetNumFreeFrames() - 1;
			break;
	}

	RC rc;
	/* Fetch and pin N pages from disk */
	for (int i = 0; i < N; i++)
	{
		/* Page number of last scanned page is pnLast; use
		 * PF_FileHandle::GetNextPage() to get the next
		 * page in the paged file. */
		rc = pRMFileHandle->pfFileHandle.GetNextPage
			(pnLast, pfPageHandles[pinnedPageCount]);
		/* Check if we hit EOF */
		if (rc != SUCCESS) { 
			break; 
			/* We just break here instead of returning the
			 * error code because the file could
			 * contain less than N pages to fetch. */
		}
		
		/* Set pnLast to page number of page pinned last */
		rc = pfPageHandles[pinnedPageCount].GetPageNum (pnLast);
		if (rc != SUCCESS) { return rc; }

		/* Increment count of pinned pages */
		pinnedPageCount++;
	}

	/* Check if we hit EOF without fetching any pages */
	if (pinnedPageCount == 0) {
		return PF_EOF;
	}

	/* Successfully fetched pages */
	return SUCCESS;
}

RC RM_FileScan::JumpToFirstRecInMem () {
	if (!bOpen) { return RM_FSCLOSED; }

	/* Reset phIx to point to the first handle in the list of
	 * page handles */
	phIx = 0;
	/* Reset snIx to point to the first slot */
	snIx = 0;

	return SUCCESS;
}

RC RM_FileScan::GetNextRecInMem (RM_Record &rec) {
	if (!bOpen) { return RM_FSCLOSED; }

	if (phIx >= pinnedPageCount) {
		return RM_NOMORERECINMEM;
	}

	RC rc;
	/* Scan records in each of the pinned pages starting from where we
	 * stopped last */
	for (; phIx < pinnedPageCount; phIx++)
	{
		RM_PageHandle rmPageHandle (pfPageHandles[phIx],
						rmFileSubHeader);
		PageNum pnIx;
		/* Determine the page number of the handle at position phIx */
		rc = pfPageHandles[phIx].GetPageNum (pnIx);
		if (rc != SUCCESS) { return rc; }

		for (; snIx < rmFileSubHeader.recordsPerPage; snIx++)
		{
			if (rmPageHandle.GetSlotType(snIx) != EMPTY_SLOT)
			{
				char *pData;
				/* Get the record's contents */
				rmPageHandle.GetRecData (snIx, pData);
				/* Check if it satisfies the given
				 * condition */
				if (SatisfiesCond(pData) == false) { 
					continue; 
				}
				/* If record satisfies condition,
				   copy the contents to rec */
				if (rec.pData) { free(rec.pData); }
				rec.pData = (char *) malloc
					(rmFileSubHeader.recordSize);
				memcpy (rec.pData, pData,
					rmFileSubHeader.recordSize);
				/* Set rid */
				rec.rid = RID (pnIx, snIx);
				/* Record is now a valid record */
				rec.bValid = true;
				/* Increment snIx to point to the next slot */
				snIx++;
				return SUCCESS;
			}
		} /* End of slotNum loop */
		/* Reset slot number after we loop over all the records in
		 * a single page */
		snIx = 0;
	} /* End of pageNum loop */

	/* There are no more records in memory; so return an error */
	return RM_NOMORERECINMEM;
}

bool RM_FileScan::SatisfiesCond (const char *pData) const {
	/* If operation is NO_OP return true */
	if (compOp == NO_OP) {
		return true;
	}

	int   i1 = 0, i2 = 0;
	float f1 = 0, f2 = 0;
	const char *c1 = 0, *c2 = 0;
	/* Get value of attribute at offset attrOffset */
	switch (attrType) {
		case TYPE_INT:
			memcpy (&i1, &pData[attrOffset], 4);
			i2 = *(int *)value;
			break;
		case TYPE_FLOAT:
			memcpy (&f1, &pData[attrOffset], 4);
			f2 = *(float *)value;
			break;
		case TYPE_STRING:
			c1 = &pData[attrOffset];
			c2 = (char *)value;	
			break;
	}

	bool bSatisfy = false;
	/* Check if data satisifies condition */
	switch (compOp) {
		case NO_OP:
			bSatisfy = true;
			break;
		case EQ_OP:
			switch (attrType) {
				case TYPE_INT:
					bSatisfy = (i1 == i2);
					break;
				case TYPE_FLOAT:
					bSatisfy = (f1 == f2);
					break;
				case TYPE_STRING:
					bSatisfy = (strncmp(c1,c2,attrLength)==0);
					break;
			}
			break;
		case LT_OP:
			switch (attrType) {
				case TYPE_INT:
					bSatisfy = (i1 < i2);
					break;
				case TYPE_FLOAT:
					bSatisfy = (f1 < f2);
					break;
				case TYPE_STRING:
					bSatisfy = (strncmp(c1,c2,attrLength)<0);
					break;
			}
			break;
		case GT_OP:
			switch (attrType) {
				case TYPE_INT:
					bSatisfy = (i1 > i2);
					break;
				case TYPE_FLOAT:
					bSatisfy = (f1 > f2);
					break;
				case TYPE_STRING:
					bSatisfy = (strncmp(c1,c2,attrLength)>0);
					break;
			}
			break;
		case LE_OP:
			switch (attrType) {
				case TYPE_INT:
					bSatisfy = (i1 <= i2);
					break;
				case TYPE_FLOAT:
					bSatisfy = (f1 <= f2);
					break;
				case TYPE_STRING:
					bSatisfy = (strncmp(c1,c2,attrLength)<=0);
					break;
			}
			break;
		case GE_OP:
			switch (attrType) {
				case TYPE_INT:
					bSatisfy = (i1 >= i2);
					break;
				case TYPE_FLOAT:
					bSatisfy = (f1 >= f2);
					break;
				case TYPE_STRING:
					bSatisfy = (strncmp(c1,c2,attrLength)>=0);
					break;
			}
			break;
		case NE_OP:
			switch (attrType) {
				case TYPE_INT:
					bSatisfy = (i1 != i2);
					break;
				case TYPE_FLOAT:
					bSatisfy = (f1 != f2);
					break;
				case TYPE_STRING:
					bSatisfy = (strncmp(c1,c2,attrLength)!=0);
					break;
			}
			break;
	}

	return bSatisfy;
}
