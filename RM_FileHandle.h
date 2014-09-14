/*
 * The RM_FileHandle class is used to manipulate the records in an open RM
 * component file. To manipulate the records in a file, a client first
 * creates an instance of this class and passes it to the
 * RM_Manager::OpenFile method described above.
 */

#ifndef RM_FILEHANDLE_H
#define RM_FILEHANDLE_H

#include "RM.h"
#include "RID.h"
#include "RM_Record.h"
#include "RM_PageHandle.h"


class RM_FileHandle {
private:
	/* Is the file handle open? */
	bool bOpen;
	/* Ptr to a paged file manager */
	PF_Manager *pPFManager;
	/* PF File Handle to a paged file */
	PF_FileHandle pfFileHandle;

	/* Handle to the RM header page */
	PF_PageHandle pfPageHandle_FileHdr;
	/* Page number of the RM header page */
	PageNum pageNum_FileHdr;
	/* Stores a copy of the file sub-header data */
	RM_FileSubHeader fileSubHeader;
	/* Ptr to the header HeaderPagepage's contents */
	char *pData_FileHdr;

	/* Returns the page type of the page with number pageNum. If the
	 * page can accomodate more records, the return value is NON_FULL_PAGE
	 * returned; if the page is full, the return value is FULL_PAGE. */
	RM_PageType GetPageType (PageNum pageNum) const;
	/* Sets the page type of the page with number pageNum */
	void SetPageType (PageNum pageNum, RM_PageType pageType);

	/* Gets page and slot numbers */
	RC GetPageNumSlotNum (const RID &rid, PageNum &pageNum, 
			SlotNum &slotNum) const;
	/* Updates header page and marks it dirty */
	RC UpdateFileHeader () const;

public:
	  RM_FileHandle ();
	  ~RM_FileHandle ();

	/* Attaches a file to this handle */
	RC Open (PF_Manager &pfManager, const char *fileName);
	/* Detaches file from this handle */
	RC Close ();

	/* Retrieves the record with identifier rid from the file */
	RC GetRec (const RID &rid, RM_Record &rec) const;
	/* Inserts the data pointed to by pData as 
	 * a new record in the file */
	RC InsertRec (const char *pData, RID &rid);
	/* Deletes the record with identifier rid from the file */
	RC DeleteRec (const RID &rid);
	/* Updates the contents of the record in the file that is
	 * associated with rec */
	RC UpdateRec (const RM_Record &rec);

	/* If the page with number pageNum is dirty, its contents are
	 * copied from the buffer pool to disk. */
	RC ForcePage (PageNum pageNum) const;
	/* Copies the contents of all dirty pages of the file from the
	 * buffer pool to disk. */
	RC ForceAllPages () const;

	/* Returns the number of records in the file  */
	RC GetNumRecords (int &numRecords) const;
	/* Returns the number of pages the file occupies */
	RC GetNumPages (int &numPages) const;

	friend class RM_FileScan;
};

#endif
