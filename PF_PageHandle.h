/*
 *  This file defines the PF_PageHandle class. The methods
 *  of this class can be used to manipulate a page present in
 *  the main memory (the buffer).
 */

#ifndef PF_PAGEHANDLE_H
#define PF_PAGEHANDLE_H

#include "PF.h"

class PF_PageHandle {
private:
	bool  bOpen; /* True if page handle is open */
	Frame *pFrame; /* Ptr to the page's frame for which this object is
			  a handle */

public:
	  PF_PageHandle ();
	  ~PF_PageHandle ();

 	/* Attaches frame's page to page handle */
	RC Open (Frame &frame);
 	/* Detaches page from page handle */
	RC Close ();

	/* Gets page number of the page */
	RC GetPageNum (PageNum &pageNum) const;
	/* Gets a ptr to the data part of the page */
	RC GetData (char *&pData) const;
};

#endif
