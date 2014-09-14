#include "PF_PageHandle.h"

PF_PageHandle::PF_PageHandle () {
	bOpen  = false;
	pFrame = NULL;
}

PF_PageHandle::~PF_PageHandle () {
	if (bOpen) {
		Close ();
	}
}

RC PF_PageHandle::Open (Frame &frame) {
	if (bOpen) {
		Close ();
	}
	pFrame = &frame;
	bOpen  = true;
	return SUCCESS;
}

RC PF_PageHandle::Close () {
	if (!bOpen) {
		return PF_PHCLOSED;
	}
	pFrame = NULL;
	bOpen  = false;
	return SUCCESS;
}

RC PF_PageHandle::GetPageNum (PageNum &pageNum) const {
	if (!bOpen) {
		return PF_PHCLOSED;
	}
	pageNum = pFrame->page.pageNum;
	return SUCCESS;
}

RC PF_PageHandle::GetData (char *&pData) const {
	if (!bOpen) {
		return PF_PHCLOSED;
	}
	pData = pFrame->page.pData;
	/* Update access time of this frame */
	pFrame->accTime = time(NULL);
	return SUCCESS;
}
