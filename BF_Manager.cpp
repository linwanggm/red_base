#include "BF_Manager.h"

/* The buffer pool */
BF_Manager bfm;

BF_Manager::BF_Manager () {
	/* Initialise the buffer pool */
	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		frame[i].bDirty   = false;
		frame[i].pinCount = 0;
		frame[i].accTime  = time(NULL);
		frame[i].fileName = NULL;
		frame[i].fileDesc = -1;
	}
	/* Initialise the statistics */
	nReads = nWrites = 0;
}

BF_Manager::~BF_Manager () {
	/* Free up memory */
	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if (frame[i].pinCount > 0) {
			printf ("Pin count of page is > 0! Forgot to unpin it, eh?\n");
			PrintFrame (frame[i]);
		}
		if (frame[i].fileName) {
			free (frame[i].fileName);
		}
	}


}

/* Find an LRU slot in the buffer for replacement.
   If none exist return -1. */
int BF_Manager::LRU () const {
	// _1 variables are for pages that are not dirty and pinCount = 0
	// _2 variables are for pages that are dirty and pinCount = 0

	// To ensure a lazy write, we try to return the index of that 
	// frame that is not dirty and the pinCount of which is 0.
	// If such a frame is not found, we try to return the index of 
	// a frame that is dirty and which has a pinCount of 0.

	int	lru_1, lru_2;
	time_t	min_1, min_2;

	lru_1 = lru_2 = -1;
	min_1 = min_2 = time (NULL);

	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if (frame[i].pinCount == 0  &&  frame[i].bDirty == false) {
			if (frame[i].accTime <= min_1) {
				min_1 = frame[i].accTime;
				lru_1 = i;
			}
		}
		else {
			if (frame[i].pinCount == 0) {
				if (frame[i].accTime <= min_2) {
					min_2 = frame[i].accTime;
					lru_2 = i;
				}
			}
		}
	}
	
	/* We give preference to clean and unpinned pages 
	   over dirty and unpinned pages. So we try to return
	   lru_1 first. */
	if ( lru_1 != -1 )
	{
	    if (DEBUG) 
			printf("...LRU %d\n",lru_1);
		return lru_1;
	}

	if ( lru_2 != -1 )
    {
		if (DEBUG) 
			printf("...LRU %d\n",lru_1);
		return lru_1;
	}


	return -1;
}

RC BF_Manager::Alloc (const char *fileName, PageNum pageNum, int fileDesc, Frame *&pFrame) {
	/* Find the index of the LRU page */
	int i = LRU ();
	if (i == -1) {
		return PF_NOBUF;
	}

	/* If we are replacing a dirty page, we write it to disk first */
	if (frame[i].bDirty == true) {
		WritePage (frame[i].fileDesc, frame[i]);
	}

	/* Initialise the variables for this frame */
	frame[i].bDirty   = false;
	frame[i].pinCount = 1;
	frame[i].accTime  = time(NULL);
	frame[i].fileDesc = fileDesc;
	/* Free memory obtained from previous allocation */
	if (frame[i].fileName) { 
		free (frame[i].fileName); 
	}
	// Strangely enough, popeye doesn't recognize strdup!
	//frame[i].fileName = strdup (fileName);
	frame[i].fileName = (char *) malloc(strlen(fileName)+1);
	strcpy (frame[i].fileName, fileName);

	frame[i].page.pageNum = pageNum;
	/* Clear the page contents */
	memset (frame[i].page.pData, 0, PF_PAGE_SIZE);

	/* Assign ptr */
	pFrame = &frame[i];

	return SUCCESS;
}

RC BF_Manager::Free (Frame *pFrame) {
	/* Set this frame's pin count to zero
	   and mark it as not dirty */
	pFrame->pinCount = 0;
	pFrame->bDirty   = false;
	pFrame->fileDesc = -1;
	free (pFrame->fileName); 
	pFrame->fileName = NULL;
	return SUCCESS;
}

RC BF_Manager::Find (const char *fileName, PageNum pageNum, Frame *&pFrame) {
	/* Search for the frame with the given filename and page number */
	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if ((frame[i].fileName != NULL)
			&& (strcmp(frame[i].fileName, fileName) == 0)
			&& (frame[i].page.pageNum == pageNum)) {
				pFrame = &frame[i];
				return SUCCESS;
		}
	}

	/* Return eror if not found */
	return PF_NOTINBUF;
}

RC BF_Manager::GetPageHandle (Frame *pFrame, PF_PageHandle &pageHandle) {
	return pageHandle.Open (*pFrame);
}

RC BF_Manager::ReadPage (int fileDesc, Frame& frame) {
	Page &page = frame.page;
	int rc;
	/* Seek to page location */
	off_t offset = (PF_PAGE_SIZE+PF_PAGENUM_SIZE)*(page.pageNum);
	rc = lseek (fileDesc, offset, SEEK_SET);
	if (rc == offset-1) {
		return PF_UNIX;
	}
	/* Fetch page */
	rc = read (fileDesc, &page, sizeof(page));
	if (rc < (int)(PF_PAGE_SIZE+PF_PAGENUM_SIZE)) {
		return PF_UNIX;
	}
	/* Update frame's access time */
	frame.accTime = time(NULL);
	/* Update I/O statistics */
	nReads++;

	if (DEBUG) {
		printf ("-->> reading page %d of %s\n", page.pageNum,
					frame.fileName);
	}

	return SUCCESS;
}

RC BF_Manager::WritePage (int fileDesc, Frame& frame) {
	const Page &page = frame.page;
	int rc;
	/* Seek to page location */
	off_t offset = (PF_PAGE_SIZE+PF_PAGENUM_SIZE)*(page.pageNum);
	rc = lseek (fileDesc, offset, SEEK_SET);
	if (rc == offset-1) {
		return PF_UNIX;
	}
	/* Write page */
	rc = write (fileDesc, &page, sizeof(page));
	if (rc < (int)(PF_PAGE_SIZE+PF_PAGENUM_SIZE)) {
		return PF_UNIX;
	}
	/* Update frame's access time */
	frame.accTime = time(NULL);
	/* Update I/O statistics */
	nWrites++;

	if (DEBUG) {
		printf ("<<-- writing page %d of %s\n", page.pageNum,
					frame.fileName);
	}
	return SUCCESS;
}

int BF_Manager::GetNumFreeFrames () const {
	int nFree = 0;
	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if (frame[i].pinCount == 0) {
			nFree++;
		}
	}
	return nFree;
}

void BF_Manager::PrintFrame (Frame &frame) {
	printf ("file: %s, page number: %d, ", frame.fileName, frame.page.pageNum);
	printf ("pin-count: %d, ", frame.pinCount);
	if (frame.bDirty) { printf ("dirty, "); }
	printf ("last accessed: %s", ctime(&frame.accTime));
}

void BF_Manager::PrintIOStat () {
	/* Print info about I/O statistics */
	printf ("Total page reads = %d, total page writes = %d\n", 
			nReads, nWrites);
}
void BF_Manager::ResetIOStat ()    { nReads = nWrites = 0; }

void BF_Manager::PrintBuffer () {
	printf ("Buffer pool size is %d. ", PF_BUFFER_SIZE);
	printf ("Pages currently stored in buffer:\n");
	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if (frame[i].fileName) {
			PrintFrame (frame[i]);
		}
	}
}

void BF_Manager::ResetBuffer () {
	for (int i = 0; i < PF_BUFFER_SIZE; i++) {
		if (frame[i].pinCount == 0) {
			if (frame[i].bDirty) {
				RC rc;
				rc = WritePage (frame[i].fileDesc, frame[i]);
				if (rc != SUCCESS) {
	                fprintf (stderr, "ResetBuffer() WARNING: page write failed for the following frame:\n");
					PrintFrame (frame[i]);
					continue;
				}
				else {
					frame[i].bDirty = false;
				}
			}
			if (frame[i].fileName != NULL)
				free (frame[i].fileName);
			frame[i].fileName = NULL;
		}
	}
}
