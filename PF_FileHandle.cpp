#include "PF_FileHandle.h"

PF_FileHandle::PF_FileHandle () : pageCount(fileSubHeader.pageCount) {
	bOpen     = false;
	fileDesc  = -1;
	fileName  = NULL;
	pHdrFrame = NULL;
	pHdrPage  = NULL;
	pBitmap   = NULL;
}

PF_FileHandle::~PF_FileHandle () {
	if (bOpen) {
		Close ();
	}
}

RC PF_FileHandle::Open (const char *fileName) {
	/* If handle is already open, close it */
	if (bOpen) {
		Close ();
	}

	/* Copy the filename */
	//this->fileName = strdup (fileName);
	this->fileName = (char *) malloc(strlen(fileName)+1);
	strcpy (this->fileName, fileName);

	/* Open the file */
	fileDesc = open (fileName, O_RDWR);
	if (fileDesc == -1) {
		free (this->fileName);
		return PF_INVALIDNAME;
	}

	RC rc;
	/* Allocate space for the header in the buffer pool */
	rc = bfm.Alloc (fileName, 0, fileDesc, pHdrFrame);
	if (rc != SUCCESS) {
		free (this->fileName);
		close (fileDesc);
		return rc;
	}
	/* Assign ptr */
	pHdrPage = &(pHdrFrame->page);

	/* Read header from disk */
	rc = bfm.ReadPage (fileDesc, *pHdrFrame);
	/* Check for errors */
	if (rc != SUCCESS) {
		free (this->fileName);
		bfm.Free (pHdrFrame);
		close (fileDesc);
		return rc;
	}
 	/* Page count takes up first few bytes of page data */
	memcpy (&fileSubHeader, &pHdrPage->pData[0], PF_FILESUBHDR_SIZE);
	/* Bitmap starts right after the page count */
	pBitmap = &(pHdrPage->pData[PF_FILESUBHDR_SIZE]); 

	/* File has been successfully attached to this handle */
	bOpen = true;

	return SUCCESS;
}

RC PF_FileHandle::Close () {
	/* If handle is already closed return error */
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	/* Check if header frame is dirty. If it is, then it should be
	 * written to disk. */
	if (pHdrFrame->bDirty) {
		RC rc = bfm.WritePage (fileDesc, *pHdrFrame);
		if (rc != SUCCESS) {
			return rc;
		}
		pHdrFrame->bDirty = false;
	}

	/* Free space taken up by the header */
	bfm.Free (pHdrFrame);
	/* Free memory */
	free (fileName);
	/* Close file descriptor */
	close (fileDesc);
	/* Reset member variables */
	pHdrFrame = NULL;
	pHdrPage  = NULL;
	pBitmap = NULL;

	/* File has been successfully detached */
	bOpen = false;

	return SUCCESS;
}

RC PF_FileHandle::GetFirstPage (PF_PageHandle &pageHandle) const {
	return GetNextPage (0, pageHandle);
}

RC PF_FileHandle::GetNextPage (PageNum current, PF_PageHandle &pageHandle) const {
	/* Search for the next allocated page on disk */
	for (PageNum i = current+1; i <= pageCount; i++) {
		if (GetPageType(i) == ALLOCATED_PAGE) {
			return GetThisPage(i, pageHandle);
		}
	}
	/* If search failed return EOF */
	return PF_EOF;
}

RC PF_FileHandle::GetThisPage (PageNum pageNum, PF_PageHandle &pageHandle) const {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	if (pageNum < 1 
		|| pageNum > pageCount 
		|| GetPageType(pageNum) == FREE_PAGE) {
		return PF_INVALIDPAGENUM;
	}

	Frame *pFrame;
	RC rc;
	/* Find frame in buffer that correspods to 
	   the given filename and page number */
	rc = bfm.Find (fileName, pageNum, pFrame);
	if (rc == SUCCESS) { /* If page is found in buffer, increment pin count */
		pFrame->pinCount++;
	}
	else { /* If page is not found in buffer, fetch it from disk */
		/* Request buffer space */
		rc = bfm.Alloc (fileName, pageNum, fileDesc, pFrame);
		if (rc != SUCCESS) {
			return rc;
		}
		/* Fetch page from disk */
		rc = bfm.ReadPage (fileDesc, *pFrame);
		if (rc != SUCCESS) {
			bfm.Free (pFrame);
			return rc;
		}
	}

	/* pFrame now points to the frame containing the request page.
	   We return a page handle to it. */
	return pageHandle.Open (*pFrame);
}

RC PF_FileHandle::AllocatePage (PF_PageHandle &pageHandle) {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	PageNum i;
	/* Search for a free page using the PF header */
	for (i = 1; i <= pageCount; i++) {
		if (GetPageType(i) == FREE_PAGE) {
			break;
		}
	}
	/* If increasing file size exceeds limit, return error */
	if (i >= 8*(PF_PAGE_SIZE-PF_FILESUBHDR_SIZE)) {
		return PF_DBFULL;
	}

	Frame *pFrame;
	RC rc;
	/* If the paged file has no free pages, increase the file size */
	if (i > pageCount) {
 		/* Increment page count */
		pageCount += 1;
		/* Request buffer space for writing a page to disk */
		rc = bfm.Alloc (fileName, i, fileDesc, pFrame);
		if (rc != SUCCESS) {
			pageCount -= 1;
			return rc;
		}
		/* Append page to file */
		rc = bfm.WritePage (fileDesc, *pFrame);
		if (rc != SUCCESS) {
			bfm.Free (pFrame);
			pageCount -= 1;
			return rc;
		}
	}
	else {
		/* Allocated buffer space */
		rc = bfm.Alloc (fileName, i, fileDesc, pFrame);
		if (rc != SUCCESS) {
			bfm.Free (pFrame);
			return rc;
		}
	}

	/* Do not reverse the order of these operations as only
	 * SetPageType() writes the updated header to disk */
	/* Update sub-header information */
	fileSubHeader.nAllocatedPages++;
	/* Update bitmap */
	SetPageType (i, ALLOCATED_PAGE);

	/* Open page handle */
	rc = pageHandle.Open (*pFrame);
	if (rc != SUCCESS) {
		return rc;
	}
	return SUCCESS;
}

RC PF_FileHandle::DisposePage (PageNum pageNum) {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	if (pageNum < 1 
		|| pageNum > pageCount 
		|| GetPageType(pageNum) == FREE_PAGE) {
		return PF_INVALIDPAGENUM;
	}

	Frame *pFrame;
	RC rc;
	/* Try to find the page in buffer */
	rc = bfm.Find (fileName, pageNum, pFrame);
	if (rc == SUCCESS) { /* If page is in buffer .. */
 		/* If page is fixed return error */
		if (pFrame->pinCount > 0) {
			return PF_PAGEPINNED;
		}
		/* If page is dirty, mark it as not dirty.
		   We do not have to write the page to disk
		   because we are disposing it. */
		if (pFrame->bDirty) {
			pFrame->bDirty = false;
		}
	}

	/* Do not reverse the order of these operations as only
	 * SetPageType() writes the updated header to disk */
	/* Update sub-header information */
	fileSubHeader.nAllocatedPages--;
	/* Page has been disposed; update header to reflect changes */
	SetPageType (pageNum, FREE_PAGE);

	return SUCCESS;
}

RC PF_FileHandle::MarkDirty (PageNum pageNum) const {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	if (pageNum < 1 
		|| pageNum > pageCount 
		|| GetPageType(pageNum) == FREE_PAGE) {
		return PF_INVALIDPAGENUM;
	}

	Frame *pFrame;
	RC rc;
	/* Try to find the page in buffer */
	rc = bfm.Find (fileName, pageNum, pFrame);
	if (rc == SUCCESS) { /* If page is in buffer .. */
		pFrame->bDirty = true;
		return SUCCESS;
	}
	else { /* If page is not in buffer, return error */
		return PF_NOTINBUF;
	}
}

RC PF_FileHandle::UnpinPage (PageNum pageNum) const {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	if (pageNum < 1 
		|| pageNum > pageCount 
		|| GetPageType(pageNum) == FREE_PAGE) {
		return PF_INVALIDPAGENUM;
	}

	Frame *pFrame;
	RC rc;
	/* Try to find the page in buffer */
	rc = bfm.Find (fileName, pageNum, pFrame);
	if (rc == SUCCESS) { /* If page is in buffer .. */
		/* If page is pinned, decrement its pin count */
		if (pFrame->pinCount > 0) {
			pFrame->pinCount--;
			return SUCCESS;
		}
		else { /* If it is alrady unpinned, return error */
			bfm.PrintFrame (*pFrame);
			return PF_PAGEUNPINNED;
		}
	}
	else { /* If page is not in buffer, return error */
		return PF_NOTINBUF;
	}
}

RC PF_FileHandle::ForcePage (PageNum pageNum) const {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	if (pageNum < 1 
		|| pageNum > pageCount 
		|| GetPageType(pageNum) == FREE_PAGE) {
		return PF_INVALIDPAGENUM;
	}

	Frame *pFrame;
	RC rc;
	/* Try to find the page in buffer */
	rc = bfm.Find (fileName, pageNum, pFrame);
	if (rc == SUCCESS) { /* If page is in buffer .. */
		/* Write page to disk */
		rc = bfm.WritePage (fileDesc, *pFrame);
		if (rc != SUCCESS) {
			return rc;
		}
		/* Page is no longer dirty */
		pFrame->bDirty = false;
		return SUCCESS;
	}
	else { /* If page is not in buffer, return error */
		return PF_NOTINBUF;
	}
}

RC PF_FileHandle::ForceAllPages () const {
	if (!bOpen) {
		return PF_FHCLOSED;
	}

	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if (bfm.frame[i].fileName != NULL) {
			/* Find all pages from the same file */
			if (strcmp(bfm.frame[i].fileName, fileName) == 0
					&& bfm.frame[i].bDirty) {
				RC rc = bfm.WritePage (fileDesc, bfm.frame[i]);
				if (rc != SUCCESS) {
					return rc;
				}
				/* Page is no longer dirty */
				bfm.frame[i].bDirty = false;
			}
		}
	}

	return SUCCESS;
}

PageType PF_FileHandle::GetPageType (PageNum pageNum) const {
	/* Determine the byte and bit position of given page number */
	int byte = pageNum/8;
	int bit  = pageNum - byte*8;
	/* Determine the value of the bit */
	int v = (pBitmap[byte] & (1<<bit)) >> bit;
	if (v == 0) {
		return FREE_PAGE;
	}
	else {
		return ALLOCATED_PAGE;
	}
}

void PF_FileHandle::SetPageType (PageNum pageNum, PageType pageType) {
	/* Determine the byte and bit position of given page number */
	int byte = pageNum/8;
	int bit  = pageNum - byte*8;
	/* Determine the value of the bit */
	int v = (pBitmap[byte] & (1<<bit)) >> bit;

	/* If page type is same as the one being set, issue a warning */
	if (v == pageType) {
		fprintf (stderr, "==> SetPageType WARNING: pageType is same");
		return;
	}
	else {
		/* Flip the page type */
		pBitmap[byte] ^= (1<<bit);
	}

	/* Write the PF header after each update */
	/* Copy sub-header to first few bytes of header page's data section */
	memcpy (&pHdrPage->pData[0], &fileSubHeader, PF_FILESUBHDR_SIZE);

	/* Mark the header frame as dirty */
	pHdrFrame->bDirty = true;
}

PageNum PF_FileHandle::GetPageCount () const {
	return fileSubHeader.pageCount;
}

int PF_FileHandle::GetNumAllocatedPages () const {
	/*reduce 1 becase when creae file it is 1(see the file:PF_Manger.cpp)  --wln*/
	return (fileSubHeader.nAllocatedPages-1);
}
