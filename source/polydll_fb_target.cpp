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
// Implementation of the Class PolyDllFB Target for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "polydll_fb_target.h"

/**
Constructor for the POLYDLLFBTarget Class

@internalComponent
@released
*/
POLYDLLFBTarget::POLYDLLFBTarget(ParameterManager* aParams) :
    DLLTarget(aParams) {
}

/**
Destructor for the POLYDLLFBTarget Class

@internalComponent
@released
*/
POLYDLLFBTarget::~POLYDLLFBTarget()
{
	if(iDefExports)
	{
		for(auto x: *iDefExports)
		{
			delete x;
		}
		iDefExports->clear();
	}
}

/**
Function to process the symbols to be exported. In case of Poly DLL, there might be
predefined symbols passed to '--sydef' option, need to consider them alongwith the
symbols coming from the ELF file.

@internalComponent
@released
*/
void POLYDLLFBTarget::ProcessExports()
{
	int count = iParameterManager->SysDefCount();
	ParameterManager::Sys aSysDefSymbols[10];

	int i = 0;
	while (i < count)
	{
		aSysDefSymbols[i] = iParameterManager->SysDefSymbols(i);
		++i;
	}

	iDefExports = new Symbols;

	for (int k=0; k < count; k++)
	{
		SymbolType aType = SymbolTypeCode;
		Symbol *aSymbolEntry = new Symbol(aSysDefSymbols[k].iSysDefSymbolName, aType);
		aSymbolEntry->SetOrdinal(aSysDefSymbols[k].iSysDefOrdinalNum);
		iDefExports->push_back(aSymbolEntry);
	}

	ValidateDefExports(iDefExports);
	CreateExports();
}

