/*
 *  This file defines the class for buffer management. You can
 *  call methods of this class to allocate/de-allocate buffer
 *  space. 
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <time.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <string.h>
#include <stdlib.h>

#include "PF_PageHandle.h"

/* This values defines the size of the buffer pool */
#define PF_BUFFER_SIZE 40

class BF_Manager {
private:
	/* Returns the index of the Least Recently Used frame */
	int LRU () const;
	/* The buffer pool is a set of frames */
	Frame frame[PF_BUFFER_SIZE];

	/* I/O Statistics */
	int nReads, nWrites; 
public:
	  BF_Manager ();
	  ~BF_Manager ();

	/* Allocates space in the buffer pool and returns
	   a pointer to the allocated frame. Caller must use Free()
	   to free up space allocated by this function.  */
	RC Alloc (const char *fileName, PageNum pageNum, int fileDesc, Frame *&pFrame);
	/* Frees up space in the buffer pool */
	RC Free (Frame *pFrame);

	/* Finds ptr to frame with the given file name and page number */
	RC Find (const char *fileName, PageNum pageNum, Frame *&pFrame);
	/* Attaches the frame's page to the given page handle */
	RC GetPageHandle (Frame *pFrame, PF_PageHandle &pageHandle);

	/* Reads page from disk into frame */
	RC ReadPage (int fileDesc, Frame &frame);
	/* Writes page in frame to disk */
	RC WritePage (int fileDesc, Frame &frame);

	/* Returns the number of free frames in the buffer */
	int GetNumFreeFrames () const;

	/* Useful for debugging: prints out frame contents */
	void PrintFrame (Frame &frame);

	void ResetIOStat  ();     // Reset I/O statistics 
	void PrintIOStat  ();     // Print I/O statistics
	void ResetBuffer  ();   // Remove all unpinned pages from the
				//buffer pool
	void PrintBuffer  ();   // Print basic information about the
				// pages stored in the buffer pool

	friend class PF_FileHandle;
};

extern BF_Manager bfm;
extern bool       bLogIOStat;

#endif
