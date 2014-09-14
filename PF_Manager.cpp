#include "PF_Manager.h"

RC PF_Manager::CreateFile (const char *fileName) const {
	/* Create the file */
	int fileDesc = open (fileName, O_EXCL|O_CREAT|O_WRONLY, 00600);
	if (fileDesc == -1) {
		return PF_INVALIDNAME;
	}

	Frame *pFrame;
	RC rc;
	/* Request a page from buffer for writing the PF header*/
	rc = bfm.Alloc (fileName, 0, fileDesc, pFrame);
	if (rc != SUCCESS) {
		close (fileDesc);
		return rc;
	}
	/* Initialise the sub-header */
	PF_FileSubHeader fileSubHeader 
		= { 0, /* page number of last page on disk */
		    1  /* number of allocated pages */ };
	memcpy (&pFrame->page.pData[0], &fileSubHeader, PF_FILESUBHDR_SIZE);
	/* Set the first bit to 1 in the header. This is to indicate that
	 * the Header page is not a FREE_PAGE. */
	pFrame->page.pData[PF_FILESUBHDR_SIZE] = 1;
	/* Write the header to disk */
	rc = bfm.WritePage (fileDesc, *pFrame);
	if (rc != SUCCESS) {
		close (fileDesc);
		return rc;
	}
	/* Unpin the page */
	bfm.Free (pFrame);
	/* Close the file */
	close (fileDesc);

	return SUCCESS;
}

RC PF_Manager::DestroyFile (const char *fileName) const {
	int rc = unlink (fileName);
	if (rc == -1) {
		return PF_UNIX;
	}
	else {
		return SUCCESS;
	}
}

RC PF_Manager::OpenFile (const char *fileName, PF_FileHandle &fileHandle) const {
	return fileHandle.Open (fileName);
}

RC PF_Manager::CloseFile (PF_FileHandle &fileHandle) const {
	return fileHandle.Close ();
}
