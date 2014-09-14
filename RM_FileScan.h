/*
 * The RM_FileScan class provides clients the capability to perform scans
 * over the records of an RM component file, where a scan may be based on a
 * specified condition.
 */

#ifndef RM_FILESCAN_H
#define RM_FILESCAN_H

#include "RM_FileHandle.h"

class RM_FileScan {
private:
	bool bOpen; /* Is the scan open? */

	/* A ptr to an RM file handle. This ptr is used to retrieve records
	 * using RM_FileHandle::GetRec() */
	const RM_FileHandle *pRMFileHandle;

	AttrType attrType; /* Specifies attribute type. Valid types are
			      defined in RM.h */
	int attrLength; /* Specifies the attribute's length */
	int attrOffset;	/* Specifies the offset of the attribute in the
			   record */
	CompOp compOp; /* Operator to be use for comparison. Valid compOp
			  operators are defined in RM.h */
	void *value;	/* The value the attribute should be compared with */
	ClientHint pinHint; /* Used by RM client components to specify a
			       page pinning strategy */

	/* Returns true if record rec satisfies condition */
	bool SatisfiesCond (const char *pData) const;



        int N; // Number of pages to be pinned in buffer. This
                        // number is determined from the specified
                        // page pinning strategy.
        int pinnedPageCount; // Actual number of pages pinned
                        // in buffer. In general, pinnedPageCount = N,
                        // but near the end of the file, pinnedPageCount
                        // can be less than N.
        PF_PageHandle pfPageHandles[PF_BUFFER_SIZE]; 
			// List of page-handles to pages pinned
                        // in buffer.

	/* Index (into pfPageHandles) of handle to page 
	   being currently scanned */
	int phIx;
	/* Slot number of slot to be scanned next */
	SlotNum snIx;
	/* Page number of page pinned last */
	PageNum pnLast;

	/* Store a copy of the RM file sub-header */
	RM_FileSubHeader rmFileSubHeader;

public:
	  RM_FileScan ();

	RC OpenScan (const RM_FileHandle &fileHandle,  // Initialize file scan
		     AttrType      attrType,
		     int           attrLength,
		     int           attrOffset,
		     CompOp        compOp,
		     const void    *value,
		     ClientHint    pinHint = NO_HINT);

	RC GetNextRec (RM_Record &rec);  // Get next matching record

	RC CloseScan ();  // Terminate file scan

	/* Fetches pages from disk according to the pinning strategy
	 * specified by pinHint */
	RC FetchPages ();
	/* Jumps to the first record in memory */
	RC JumpToFirstRecInMem ();
	/* Fetches the next record in memory */
	RC GetNextRecInMem (RM_Record &rec);
};

#endif
