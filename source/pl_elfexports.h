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
// Implementation of the Class ElfExports for the elf2e32 tool
// @internalComponent
// @released
//
//

#if !defined(_PL_ELFEXPORTS_H_)
#define _PL_ELFEXPORTS_H_

#include "pl_common.h"
#include <vector>
#include <functional>
#include <algorithm>

using std::vector;
using std::binary_function;

class ElfImage;
class Symbol;

/**
This class is for exports coming from elf file.
(Corresponding to the exported symbols defined in the given DLL. These exports are filtered
by individual targets as per their requirements(relating to class impedimenta). While
processing these exports, they are also to update the ordinal numbers before these exports
are written into the dso file.
@internalComponent
@released
*/
class ElfExports
{

public:
	typedef std::vector<Symbol*>	Exports;


	struct PtrELFExportNameCompare :
		binary_function<const Symbol *, const Symbol *, bool>
	{
		bool operator()(const Symbol * lhs, const Symbol * rhs) const;
	};

	struct PtrELFExportOrdinalCompare :
		binary_function<const Symbol *, const Symbol *, bool>
	{
		bool operator()(const Symbol * lhs, const Symbol * rhs) const;
	};

	struct PtrELFExportNameCompareUpdateAttributes :
		binary_function<Symbol *, Symbol *, bool>
	{
		bool operator()(Symbol * lhs, Symbol * rhs) const;
	};


	ElfExports();
	~ElfExports();

	bool ValidExportP(ElfImage * aElfImage, Symbol * aSym);
	void FilterExports();
	Symbol* Add(char *aDll, ElfImage * aExecutable, Symbol *aSym);
	void Add(char *aDll, Symbol *aSym);
	void Sort();
	void ExportsFilteredP(bool aExportsFilteredP)
	{
		iExportsFilteredP = aExportsFilteredP;
	}
	bool ExportsFilteredP() {return iExportsFilteredP;}

	char* DllName();
	Exports& GetExports(bool) ;
	Exports& GetExportsInOrdinalOrder();
	size_t GetNumExports();
	Exports iFilteredExports;

private:
	Exports	iElfExports;
	char		*iDllName;
	bool iSorted;
	bool iExportsFilteredP;
};




#endif // !defined(_PL_ELFEXPORTS_H_)
