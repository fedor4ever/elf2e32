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

#include <cstring>
#include "pl_elfimports.h"
#include "pl_elfimportrelocation.h"

ElfImports::ElfImports(){}

/**
Destructor for class ElfImports to release allocated memory
@internalComponent
@released
*/
ElfImports::~ElfImports()
{
	if(iImports.size())
	{
        for(auto x: iImports)
		{
			RelocationList *rlistTemp = &(x.second);

            for(auto y: *rlistTemp)
			{
				ElfImportRelocation *temp = y;
				delete temp;
				temp = nullptr;
			}
			rlistTemp->clear();
		}
	}
	iImports.clear();
}


/**
Function to add imports
@param aDllName - Dll name
@param aReloc - Elf import relocation
@internalComponent
@released
*/
void ElfImports::Add(ElfImportRelocation *aReloc){
    char *aDllName = aReloc->iVerRecord->iLinkAs;
	iImports[aDllName].insert(iImports[aDllName].end(), aReloc);
}


/**
Function to get import size
@return import size
@internalComponent
@released
*/
PLUINT32 ElfImports::GetImportSize(){
	PLUINT32	aSize = 0;
	for(auto x: iImports){
		RelocationList aList = x.second;
		aSize += aList.size();
	}
	return aSize;
}


/**
Overloaded operator comparing the symbol names.
@return True if lhs string is less then rhs, otherwise false
@internalComponent
@released
*/
bool ElfImports::StringPtrLess::operator() (const char * lhs, const char * rhs) const
{
	return strcmp(lhs, rhs) < 0;
}

/**
Function to get imports
@return imports
@internalComponent
@released
*/
ElfImports::ImportLibs& ElfImports::GetImports()
{
	return iImports;
}
