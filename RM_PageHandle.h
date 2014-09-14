/* A header is stored in each data page of a paged file page. An
 * RM_PageSubHeader object is this entire header minus the bitmap for
 * records. */

#ifndef RM_PAGEHEADER_H
#define RM_PAGEHEADER_H

#include "RM.h"

typedef struct {
	/* Stores the number of records this page currently holds */
	int nRecords;
} RM_PageSubHeader;
#define RM_PAGESUBHDR_SIZE sizeof(RM_PageSubHeader)


/* A slot is a storage area for a record in a page */
typedef enum {
	EMPTY_SLOT,  /* Slot contains no record */
	OCCUPIED_SLOT /* Slot contains a record */
} SlotType;

class RM_PageHandle {
private:
	/* Ptr to contents (data) of the page */
	char *pData;
	/* Stores a copy of the file-sub-header */
	RM_FileSubHeader fileSubHeader;
	/* Stores a copy of the page-sub-header */
	RM_PageSubHeader pageSubHeader;
	/* Ptr to the bitmap for slots */
	char *pBitmap;

public:
	  RM_PageHandle (const PF_PageHandle &pfPageHandle, 
			  RM_FileSubHeader fileSubHeader);
	
	/* Functions for getting/setting slot types */
	SlotType GetSlotType (SlotNum slotNum) const;
	void SetSlotType (SlotNum slotNum, SlotType slotType);

	/* Returns a ptr to the record at the position slotNum */
	void GetRecData (SlotNum slotNum, char *&pData);

	/* Returns true if the page cannot accomodate any more records */
	bool PageFull () const;
};

#endif
