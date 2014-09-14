/*
 *  This file defines the PF_FileHandle class. The methods
 *  of this class can be used to manipulate a paged file.
 */

#ifndef PF_FILEHANDLE_H
#define PF_FILEHANDLE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "PF_PageHandle.h"
#include "BF_Manager.h"


/* Defines the structure of a paged file's sub-header */
typedef struct {
	/* Stores the page number of the last page on disk 
	   (be it a free page or an allocated page) */
	PageNum pageCount;
	/* Stores the number of allocated pages (including the header
	 * page) on disk */
	int nAllocatedPages;
} PF_FileSubHeader;
#define PF_FILESUBHDR_SIZE sizeof(PF_FileSubHeader)


/* Defines page types */
typedef enum {
	FREE_PAGE,
	ALLOCATED_PAGE
} PageType;


class PF_FileHandle {
private:
	bool bOpen; /* Is the handle attached to a file? */
	char *fileName; /* The file this handle is attached to */
	int  fileDesc; /* File descriptor of the attached file */

	Frame *pHdrFrame; /* Ptr to the PF header frame */
	Page  *pHdrPage; /* Ptr to the PF header page */
	char  *pBitmap; /* Ptr to the bitmap part of the header */
	PF_FileSubHeader fileSubHeader; /* Stores the sub header */
	PageNum &pageCount; /* Stores page number of the last allocated
			       page. This is a reference to a variable in
			       the subheader. */

	/* Returns the page type of page given by page number */
	PageType GetPageType (PageNum pageNum) const; 
	/* Sets the page type of page given by page number */
	void SetPageType (PageNum pageNum, PageType pageType);

public:
	  PF_FileHandle ();
	  ~PF_FileHandle ();

	/* Attaches file to this handle */
	RC Open (const char *fileName);
	/* Detaches file */
	RC Close ();

	/* Returns handle to first page */
	RC GetFirstPage (PF_PageHandle &pageHandle) const;
	/* Returns handle to page after current page */
	RC GetNextPage (PageNum current, PF_PageHandle &pageHandle) const;
	/* Returns handle to page given by page number */
	RC GetThisPage (PageNum pageNum, PF_PageHandle &pageHandle) const;

	/* Allocates a new page returns a handle to it */
	RC AllocatePage (PF_PageHandle &pageHandle);
	/* Disposes page given by page number */
	RC DisposePage (PageNum pageNum);

	/* Marks a page as dirty */
	RC MarkDirty (PageNum pageNum) const;
	/* Unpins a page in buffer */
	RC UnpinPage (PageNum pageNum) const;
	/* Writes a specific page to disk */
	RC ForcePage (PageNum pageNum) const;
	/* Writes all pages to disk */
	RC ForceAllPages () const;

	/* Returns the page number of the last page on disk (be it a
	 * free page or an allocated page) */
	PageNum GetPageCount () const;
	/* Returns the number of allocated pages on disk */
	int GetNumAllocatedPages () const;

	friend class RM_FileHandle;
	friend class RM_FileScan;
};

#endif
