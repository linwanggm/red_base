#include "RM_Record.h"

RM_Record::RM_Record () {
	pData = NULL;
	bValid = false;
}

RM_Record::~RM_Record () {
	if (pData) {
		free (pData);
	}
}

RC RM_Record::GetData (char *&pData) const {
	if (!bValid) {
		return RM_INVALIDREC;
	}
	pData = this->pData;
	return SUCCESS;
}

RC RM_Record::GetRid (RID &rid) const {
	if (!bValid) {
		return RM_INVALIDREC;
	}
	rid = this->rid;
	return SUCCESS;
}
