#include "RM_PageHandle.h"

RM_PageHandle::RM_PageHandle (const PF_PageHandle &pfPageHandle, 
				RM_FileSubHeader fsHeader) {
	/* Get the data ptr */
	pfPageHandle.GetData (pData);
	/* Copy the file-sub-header */
	fileSubHeader = fsHeader;
	/* Obtain the page-subhdr and bitmap information */
	memcpy (&pageSubHeader, &pData[0], RM_PAGESUBHDR_SIZE);
	pBitmap = &pData[RM_PAGESUBHDR_SIZE];
}

SlotType RM_PageHandle::GetSlotType (SlotNum slotNum) const {
        /* Determine the byte and bit position of given slot number */
        int byte = slotNum/8;
        int bit  = slotNum - byte*8;
        /* Determine the value of the bit */
        int v = (pBitmap[byte] & (1<<bit)) >> bit;
        if (v == 0) {
                return EMPTY_SLOT;
        }
        else {
                return OCCUPIED_SLOT;
        }
}

void RM_PageHandle::SetSlotType (SlotNum slotNum, SlotType slotType) {
        /* Determine the byte and bit position of given slot number */
        int byte = slotNum/8;
        int bit  = slotNum - byte*8;
        /* Determine the value of the bit */
        int v = (pBitmap[byte] & (1<<bit)) >> bit;
                                                                                                                             
        /* If slot type is same as the one being set, issue a warning */
        if (v == slotType) {
                fprintf (stderr, "==> SetSlotType WARNING: slotType is same");
                return;
        }
        else {
                /* Flip the slot type */
                pBitmap[byte] ^= (1<<bit);
        }

	if (slotType == EMPTY_SLOT) {
		pageSubHeader.nRecords--;
	}
	else {
		pageSubHeader.nRecords++;
	}

	/* Copy back this page's sub-header information because we maintain a
	 * local copy of it. */
	memcpy (&pData[0], &pageSubHeader, RM_PAGESUBHDR_SIZE);
}

void RM_PageHandle::GetRecData (SlotNum slotNum, char *&pData) {
	/* Calculate record offset */
	int recordOffset = fileSubHeader.firstRecordOffset 
				+ slotNum*fileSubHeader.recordSize;
	/* Return a ptr to the record */
	pData = &(this->pData[recordOffset]);
}

bool RM_PageHandle::PageFull () const {
	if (pageSubHeader.nRecords == fileSubHeader.recordsPerPage) {
		return true;
	}
	else {
		return false;
	}
}
