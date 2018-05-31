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
// Implementation of the Class PolyDLL Rebuild Target for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "polydll_rebuild_target.h"
#include "deffile.h"
#include "errorhandler.h"

#include <iostream>
#include <cstring>

using std::list;

/**
Constructor for the POLYDLLRebuildTarget Class

@internalComponent
@released
*/
POLYDLLRebuildTarget::POLYDLLRebuildTarget(ParameterManager* aParams) : ExportTypeRebuildTarget(aParams) {
}

/**
Destructor for the POLYDLLRebuildTarget Class

@internalComponent
@released
*/
POLYDLLRebuildTarget::~POLYDLLRebuildTarget()
{
}

/**
Function to process the symbols to be exported. In case of Poly DLL, there might be
predefined symbols passed to '--sydef' option, need to consider them alongwith the
symbols coming from the DEF file and ELF file.

@internalComponent
@released
*/
void POLYDLLRebuildTarget::ProcessExports()
{
    Symbols iSysdefin;
    GetSymbolsFromSysdefoption(iSysdefin);

    DefFile *iDefFile = new DefFile();
	iDefExports = iDefFile->ReadDefFile(iParameterManager->DefInput());
	delete iDefFile;

	// Check if the Sysdefs and the DEF file are matching.

	auto aBegin = iSysdefin.begin();
	auto aEnd = iSysdefin.end();

	auto aDefBegin = iDefExports->begin();
	auto aDefEnd = iDefExports->end();

	std::list<string> aMissingSysDefList;

	while ((aDefBegin != aDefEnd) && (aBegin != aEnd))
	{
		if (strcmp((*aBegin)->SymbolName(), (*aDefBegin)->SymbolName()))
			aMissingSysDefList.push_back((*aBegin)->SymbolName());
		++aBegin;
		++aDefBegin;
	}

	if( aMissingSysDefList.empty() )
    {
		throw SysDefMismatchError(SYSDEFSMISMATCHERROR, aMissingSysDefList, UseCaseBase::DefInput());
    }

	ValidateDefExports(*iDefExports);
	CreateExports();
}

