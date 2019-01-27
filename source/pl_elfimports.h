// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017-2018 Strizhniou Fiodar.
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
// Implementation of the Class ElfImports for the elf2e32 tool
// @internalComponent
// @released
//
//


#if !defined(_PL_ELFIMPORTS_H_)
#define _PL_ELFIMPORTS_H_

#include "pl_common.h"
#include <vector>
#include <map>

using std::binary_function;

class ElfRelocation;

/**
This class get the list of symbols imported from a list of DSOs.
@internalComponent
@released
*/
class ElfImports
{

public:
	struct StringPtrLess : binary_function<const char *, const char *, bool>
	{
		bool operator() (const char * lhs, const char * rhs) const;
	};

	typedef std::vector<ElfRelocation*> RelocationList;

	typedef std::map<const char*, RelocationList, StringPtrLess> ImportLibs;

	ElfImports();
	~ElfImports();

	void Add(ElfRelocation *aReloc);
	PLUINT32 GetImportSize();
	ImportLibs& GetImports();

private:
	ImportLibs  iImports;

};

#endif // !defined(_PL_ELFIMPORTS_H_)
