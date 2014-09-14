#include <stdio.h>
#include "SM_Manager.h"

int main (int argc, char *argv[]) {
	if (argc < 2) {
		printf ("Usage: %s <db-name>\n", argv[0]);
		return 0;
	}

	PF_Manager pfm;
	RM_Manager rmm(pfm);
	SM_Manager smm(rmm);
	RC rc;

	rc = smm.DestroyDb (argv[1]);
	if (rc != SUCCESS) {
		PrintError (rc);
		return rc;
	}
	else {
		printf ("Database destroyed\n");
	}

	return 0;
}
