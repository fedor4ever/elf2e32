// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar.
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
// Implementation of the Class Symbol for the elf2e32 tool
// @internalComponent
// @released
//
//


#if !defined(_PL_SYMBOL_H_)
#define _PL_SYMBOL_H_

#include <string>
#include "pl_common.h"
#include "pl_sym_type.h"

#define UnAssignedOrdNum -1;
using std::string;

enum SymbolStatus {Matching,Missing,New};
/**
 * This class is shared among all that use the symbol information. To name  them,
 * DefFileHandler, UseCaseController, DSOHandler. To be finalized  by
 * DefFileHandler.
 */
class Symbol
{

public:

    Symbol(string aName, SymbolType aCodeDataType);

	Symbol(char* aName, SymbolType aType, Elf32_Sym* aElfSym, PLUINT32 aSymbolIndex);

	Symbol(Symbol& aSymbol, SymbolType aCodeDataType, bool aAbsent);

	Symbol(Symbol& aSymbol);
	/*Symbol(char* aName,CDType aCodeDataType, PLUINT32	aOrd = UnAssignedOrdNum,\
		char* aComment = nullptr, bool aAbsent = false);*/
	~Symbol();

	bool operator==(const Symbol* aSym) const;
	const char* SymbolName() const;
	const char* ExportName();
	PLUINT32 OrdNum() const;
	SymbolType CodeDataType();

	bool R3unused();
	bool Absent();
	void SetAbsent(bool aAbsent);
	string Comment();
	int GetSymbolStatus();
	void SetOrdinal(PLUINT32 aOrdinalNum);
	void SetSymbolStatus(SymbolStatus aSymbolStatus);
	void SetSymbolName(char *aSymbolName);

	void SymbolName(char *aSymbolName);
	void Comment(char *aComment);
	void CodeDataType(SymbolType aType);
	void R3Unused(bool aR3Unused);
	void ExportName(char *aExportName);
	void SetSymbolSize(PLUINT32 aSz);
	PLUINT32 SymbolSize();

    Elf32_Sym	*iElfSym = nullptr;
	/**
	 * The index of this symbol in the symbol table(required for the hash table while
	 * creating the dso).
	 */
	PLUINT32		iSymbolIndex = 0;

private:
/** TODO (Administrator#1#04/20/17): Find why and where this used unitialized!!!! */
	SymbolStatus iSymbolStatus;// = Missing; /*should fail if not init!!! */
	string		iSymbolName;
	string		iExportName;
	SymbolType	iSymbolType = SymbolTypeNotDefined; //should fail if not init!!!
	PLUINT32	iOrdinalNumber  = -1; // default value in ctor
	string		iComment;
	bool		iAbsent = false;
	bool		iR3Unused = false;
	PLUINT32	iSize = 0;
};

#endif // !defined(_PL_SYMBOL_H_)
