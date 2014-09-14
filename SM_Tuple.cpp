#include "SM_Tuple.h"

SM_Tuple::SM_Tuple (const DataAttrInfo dataAttrInfo[], int attrCount, char *pRecData) {
	/* Make a local copy of attribute information */
	info = (DataAttrInfo *) malloc (attrCount*sizeof(DataAttrInfo));
	memcpy (info, dataAttrInfo, attrCount*sizeof(DataAttrInfo));
	this->attrCount = attrCount;
	/* Determine the tuple size */
	tupleSize = info[attrCount-1].offset + info[attrCount-1].attrLength;
	/* _Don't_ create a local copy of the record data */
	pData = pRecData;
	bCopy = false;
	/* Tuple is now a valid tuple */
	bValid = true;
}

SM_Tuple::SM_Tuple (const DataAttrInfo dataAttrInfo[], int attrCount, ...) {
	/* Make a local copy of attribute information */
	info = (DataAttrInfo *) malloc (attrCount*sizeof(DataAttrInfo));
	memcpy (info, dataAttrInfo, attrCount*sizeof(DataAttrInfo));
	this->attrCount = attrCount;

	/* Determine the tuple size */
	tupleSize = info[attrCount-1].offset + info[attrCount-1].attrLength;
	/* Allocate memory for the tuple data */
	pData = (char *) malloc (tupleSize);
	/* Initialise memory */
	memset (pData, 0, tupleSize);
	/* Make the copy */
	va_list ap; /* ap = argument pointer */
	/* Initialise scan of VA (variable argument) list */
	va_start(ap, attrCount);
	/* Scan the VA list */
	for (int a = 0; a < attrCount; a++) {
		DataAttrInfo &d = info[a];
		int i; float f; char *s;
		/* Insert scanned argument into the appropriate position in
		 * the tuple */
		switch (d.attrType) {
			case TYPE_INT:
				i = va_arg(ap,int);
				memcpy(&pData[d.offset], &i, 4);
				break;
			case TYPE_FLOAT:
				f = va_arg(ap,double);
				memcpy(&pData[d.offset], &f, 4);
				break;
			case TYPE_STRING:
				s = va_arg(ap,char *);
				memcpy(&pData[d.offset], s, strlen(s));
				break;
		}
	}
	/* Terminate scan */
	va_end(ap);
	/* Because we have a copy of the tuple data, set bCopy to true */
	bCopy = true;

	/* Tuple is now a valid tuple */
	bValid = true;
}

SM_Tuple::SM_Tuple (const SM_Tuple &other) {
	bValid = false; // Set bValid to false because we shouldn't call 
			// free(info) in operator=()
	bCopy = false; // Set bCopy to false because we shouldn't call 
			// free(pData) in operator=()
	*this = other;
}

SM_Tuple::SM_Tuple () {
	/* This tuple does not contain any data */
	bValid = false;
	bCopy  = false;
	info   = NULL;
	pData  = NULL;
	attrCount = 0;
	tupleSize = 0;
} 

SM_Tuple::~SM_Tuple () {
	/* If tuple does not contain any data, just return */
	if (!bValid) { return; }

	/* Free up memory */
	free (info);
	if (bCopy) { free (pData); }
}

RC SM_Tuple::GetValue (int attrIx, void *value) const {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }
	if (attrIx < 0 || attrIx >= attrCount) { return SM_INVALIDATTRIX; }
	/* Return copy of data */
	memcpy (value, &pData[info[attrIx].offset],
			info[attrIx].attrLength);
	return SUCCESS;
}

RC SM_Tuple::SetValue (int attrIx, const void *value) {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }
	if (attrIx < 0 || attrIx >= attrCount) { return SM_INVALIDATTRIX; }
	/* Copy value */
	memcpy (&pData[info[attrIx].offset], value, info[attrIx].attrLength);
	return SUCCESS;
}

RC SM_Tuple::GetAttrType (int attrIx, AttrType &attrType) const {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }
	if (attrIx < 0 || attrIx >= attrCount) { return SM_INVALIDATTRIX; }
	/* Return ptr to data */
	attrType = info[attrIx].attrType;
	return SUCCESS;
}

RC SM_Tuple::GetAttrLength (int attrIx, int &attrLength) const {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }
	if (attrIx < 0 || attrIx >= attrCount) { return SM_INVALIDATTRIX; }
	/* Return ptr to data */
	attrLength = info[attrIx].attrLength;
	return SUCCESS;
}

RC SM_Tuple::GetAttrName (int attrIx, char *&attrName) const {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }
	if (attrIx < 0 || attrIx >= attrCount) { return SM_INVALIDATTRIX; }
	/* Return ptr to data */
	attrName = info[attrIx].attrName;
	return SUCCESS;
}

RC SM_Tuple::GetData (char *&pData) {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }
	/* Return ptr to pData */
	pData = this->pData;
	return SUCCESS;
}

SM_Tuple& SM_Tuple::operator= (const SM_Tuple &T) {
	/* If this tuple object is a valid tuple, we have to free memory
	 * allocated to info and pData before copying tuple T. */
	if (bValid) {
		free (info);
	}
	if (bCopy) {
		free (pData);
	}

	/* Copy all non-ptr variables */
	bValid    = T.bValid;
	attrCount = T.attrCount;
	tupleSize = T.tupleSize;

	/* Copy attribute information */
	info = (DataAttrInfo *) malloc (attrCount*sizeof(DataAttrInfo));
	memcpy (info, T.info, attrCount*sizeof(DataAttrInfo));

	/* Copy tuple data */
	bCopy = T.bCopy;
	if (bCopy == true) {
		pData = (char *) malloc (tupleSize);
		memcpy (pData, T.pData, tupleSize);
	}
	else {
		pData = T.pData;
	}

	return *this;
}

SM_Tuple SM_Tuple::operator+ (const SM_Tuple &other) const {
	/* Create a new tuple for the concatenation */
	SM_Tuple T;

	if (!bValid || !other.bValid) { 
		return T;
	}

	/* Initialise variables */
	T.bValid     = true;
	T.attrCount  = this->attrCount + other.attrCount;
	T.tupleSize  = this->tupleSize + other.tupleSize;

	/* Copy tuple data */
	T.bCopy = true;
	T.pData = (char *) malloc (T.tupleSize);
		memcpy (&T.pData[0], this->pData, this->tupleSize);
		memcpy (&T.pData[this->tupleSize], other.pData, other.tupleSize);

	/* Copy attribute information */
	T.info = (DataAttrInfo *) malloc (T.attrCount*sizeof(DataAttrInfo));
	for (int i = 0; i < T.attrCount; i++) {
		if (i < this->attrCount) {
			T.info[i] = this->info[i];
		}
		else {
			T.info[i] = other.info[i-this->attrCount];
			T.info[i].offset += this->tupleSize;
		}
	}

	return T;
}

bool SM_Tuple::AttrEq (int ix1, const SM_Tuple &other, int ix2) {
	/* If either one is not a valid tuple, return false */
	if (!bValid || !other.bValid) { 
		return false; 
	}
	/* If attribute types do not match, return false */
	if (info[ix1].attrType != other.info[ix2].attrType) {
		return false;
	}
	
	void *p1 = &pData[info[ix1].offset];
	void *p2 = &other.pData[other.info[ix2].offset];
	int  len1 = info[ix1].attrLength;
	int  len2 = other.info[ix2].attrLength;
	int  min_len = (len1 < len2) ? len1 : len2;

	bool bMatch = (memcmp(p1,p2,min_len) == 0);

	if (info[ix1].attrType == TYPE_STRING && len1 != len2)
	{
		if (len1 < len2)
			bMatch = bMatch && (((char *)p2)[len1] == '\0');
		else
			bMatch = bMatch && (((char *)p1)[len2] == '\0');
	}

	return bMatch;
}

RC SM_Tuple::PrintData (FILE *stream, char delim) {
	/* Check if tuple and index are valid */
	if (!bValid) { return SM_TUPLENOTINIT; }

	for (int a = 0; a < attrCount; a++) {
		/* Retrieve and print values based on attribute types */
		switch (info[a].attrType) {
			case TYPE_INT:
				int i;
				GetValue(a,&i);
				fprintf (stream, "%10d", i);
				break;
			case TYPE_FLOAT:
				float f;
				GetValue(a,&f);
				fprintf (stream, "%10.3f", f);
				break;
			case TYPE_STRING:
				int len = info[a].attrLength;
				char *s = (char *) malloc (len+1);
				memset (s, 0, len+1);
				GetValue (a,s);
				fprintf (stream, "%-*s", len, s);
				free (s);
				break;
		}
		/* Seperate printed values using the given delimiter */
		if (a < attrCount-1) {
			fprintf (stream, "%c", delim);
		}
	}
	putchar ('\n');

	return SUCCESS;
}
