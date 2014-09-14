#ifndef SM_TUPLE_H
#define SM_TUPLE_H

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "SM.h"

class SM_Tuple {
	/* True if tuple has been initialised */
	bool bValid;
	/* True if tuple has a copy of a record's data */
	bool bCopy;

	int attrCount; /* The number of attributes present in the this tuple */
	/* Stores attribute information that includes attribute length,
	 * attribute type, its offset, etc. */
	DataAttrInfo *info;

	/* Values of this tuple's attributes are stores in this array */
	char *pData;
	/* The tuple size in bytes (the size of the storage area) */
	int tupleSize;

public:
	  /* Initialise tuple with given data */
  	  SM_Tuple (const DataAttrInfo dataAttrInfo[], int attrCount, char *pData);
	  /* Initialise tuple with given data */
	  SM_Tuple (const DataAttrInfo dataAttrInfo[], int attrCount, ...);
	  /* Copy constructor */
	  SM_Tuple (const SM_Tuple &T);
	  /* Create an unitialised tuple */
	  SM_Tuple ();
	  /* Destructor */
	  ~SM_Tuple ();

	/* Get an attribute's value. Caller must allocated memory for the
	 * variable "value" before calling this function. */
	RC GetValue (int attrIx, void *value) const;
	RC SetValue (int attrIx, const void *value);

	/* Retrives attribute type of attribute with index attrix */
	RC GetAttrType (int attrIx, AttrType &attrType) const;
	/* Retrives attribute length of attribute with index attrix */
	RC GetAttrLength (int attrIx, int &attrLength) const;
	/* Returns a ptr (not a copy) of the attribute name */
	RC GetAttrName (int attrIx, char *&attrName) const;

	/* Retrive the data contain in the tuple as a char ptr */
	RC GetData (char *&pData);

	/* Copies tuple T to this tuple object */
	SM_Tuple& operator= (const SM_Tuple &T);
	/* Creates and returns a new tuple object that is the concatenation of
	 * this tuple object and the tuple object T.  If either this object 
	 * or T is not intialised, then the return value is undefined. */
	SM_Tuple operator+ (const SM_Tuple &T) const;

	/* Is attribute ix1 of this tuple equal to attribute ix2 of the
	 * tuple T? Return value is false if this object or T is
	 * uninitialised. */
	bool AttrEq (int ix1, const SM_Tuple &T, int ix2);
	
	/* Prints out tuple on the given stream */
	RC PrintData (FILE *stream = stdout, char delimiter = '|');
};

#endif
