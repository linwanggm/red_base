/*
 * The RM_Manager class handles the creation, deletion, opening, and
 * closing of files of records in the RM component. Your program should
 * create exactly one instance of this class, and all requests for RM
 * component file management should be directed to that instance.
 */

#ifndef RM_MANAGER_H
#define RM_MANAGER_H

#include "RM_FileHandle.h"

class RM_Manager {
private:
	PF_Manager &pfm;

public:
	  /* This constructor takes as a parameter an instance of the
	   * PF_Manager class, which you should already have created. */
	  RM_Manager (PF_Manager &pfm);

	/* Creates a file that can store records of the given size */
	RC CreateFile (const char *fileName, int recordSize);
	/* Destroys the file */
	RC DestroyFile (const char *fileName);
	/* Opens the file and returns a handle to it */
	RC OpenFile (const char *fileName, RM_FileHandle &fileHandle);
	/* Closes the file attached to the given handle */
	RC CloseFile (RM_FileHandle &fileHandle);
};

#endif
