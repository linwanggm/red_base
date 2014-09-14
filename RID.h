/*
 *  The RID class defines record identifier objects. A record identifier
 *  uniquely identifies a record within a given file, based on the record's
 *  page number in the file and slot number within that page.
 */

#ifndef RID_H
#define RID_H

#include "RM.h"

class RID {
private:
	PageNum pageNum;
	SlotNum slotNum;
	bool bValid; /* Is RID a valid record identifier? */
public:
	  /* Constructs a new RID object */
	  RID ();
	  /* Constructs a new RID object from the given page and slot
	   * numbers */
	  RID (PageNum pageNum, SlotNum slotNum);

	/* Sets page number to the record indentifier's page number */
	RC GetPageNum (PageNum &pageNum) const;
	/* Sets slot number to the record indentifier's slot number */
	RC GetSlotNum (SlotNum &slotNum) const;

	void PrintRid () const;
};

#endif
