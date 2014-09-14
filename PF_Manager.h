/*
 *  The PF_Manager class contains methods that can be used
 *  to create/destroy or open/close paged files.
 */

#ifndef PF_MANAGER_H
#define PF_MANAGER_H

#include "PF_FileHandle.h"

class PF_Manager {
public:
	/* Creates a paged file */
	RC CreateFile (const char *fileName) const;
	/* Destroys a paged file */
	RC DestroyFile (const char *fileName) const;
	/* Opens a paged file and returns a handle to it */
	RC OpenFile (const char *fileName, PF_FileHandle &fileHandle) const;
	/* Closes the paged file attached to the given handle */
	RC CloseFile (PF_FileHandle &fileHandle) const;
};

#endif
