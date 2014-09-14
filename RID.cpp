#include "RID.h"

RID::RID () : bValid(false) {
}

RID::RID (PageNum pageNum, SlotNum slotNum) {
	this->pageNum = pageNum;
	this->slotNum = slotNum;
	bValid = true;
}

RC RID::GetPageNum (PageNum &pageNum) const {
	if (!bValid) {
		return RM_INVALIDRID;
	}
	pageNum = this->pageNum;
	return SUCCESS;
}

RC RID::GetSlotNum (SlotNum &slotNum) const {
	if (!bValid) {
		return RM_INVALIDRID;
	}
	slotNum = this->slotNum;
	return SUCCESS;
}

void RID::PrintRid () const {
	if (!bValid) { 
		printf ("Cannot print an invalid Rid\n");
	}
	printf ("(%d,%d)", pageNum, slotNum);
}
