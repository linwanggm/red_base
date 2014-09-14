/*
 * The RM_Record class defines record objects. To materialize a record, a
 * client creates an instance of this class and passes it to one of the
 * RM_FileHandle or RM_FileScan methods that reads a record.
 */

#ifndef RM_RECORD_H
#define RM_RECORD_H

#include "RM.h"
#include "RID.h"

class RM_Record {
private:
	bool bValid; /* False if a record has not already been read into
			this RM_Record object using RM_FileScan::GetRec()
			or RM_FileHandle::GetNextRec() */
	RID  rid; /* Stores the RID of this record */
public:
	char *pData; /* Stores the data contained in this record */
	  RM_Record ();
	  ~RM_Record ();

	/* This method provides access to the contents (data) of the
	 * record. If the method succeeds, pData should point to the
	 * contents of the copy of the record created by
	 * RM_FileHandle::GetRec() or RM_FileScan::GetNextRec(). */
	RC GetData (char *&pData) const;
	/* If this method succeeds, rid should contain the identifier for
	 * the record. */
	RC GetRid (RID &rid) const;

	friend class RM_FileHandle;
	friend class RM_FileScan;
};

#endif
