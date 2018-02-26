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
POLYDLLRebuildTarget::POLYDLLRebuildTarget(ParameterManager* aParameterManager) : ExportTypeRebuildTarget(aParameterManager) {
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
	int count = iParameterManager->SysDefCount();
	ParameterManager::Sys aSysDefSymbols[10];

	int i = 0;
	while (i < count)
	{
		aSysDefSymbols[i] = iParameterManager->SysDefSymbols(i);
		++i;
	}

	Symbol *aSymbolEntry = nullptr;

	Symbols *iSysDefExports = new Symbols;

	iDefExports = iDefFile->ReadDefFile(iParameterManager->DefInput());

	for (int k=0; k < count; k++)
	{
		SymbolType aType = SymbolTypeCode;
		aSymbolEntry = new Symbol(aSysDefSymbols[k].iSysDefSymbolName, aType);
		aSymbolEntry->SetOrdinal(aSysDefSymbols[k].iSysDefOrdinalNum);
		iSysDefExports->push_back(aSymbolEntry);
	}

	// Check if the Sysdefs and the DEF file are matching.

	auto aBegin = iSysDefExports->begin();
	auto aEnd = iSysDefExports->end();

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

	if( aMissingSysDefList.size() )
    {
        delete iSysDefExports;
		throw SysDefMismatchError(SYSDEFSMISMATCHERROR, aMissingSysDefList, UseCaseBase::DefInput());
    }

    delete iSysDefExports;
    iSysDefExports = nullptr;
    if(aSymbolEntry) delete aSymbolEntry;
	ValidateDefExports(iDefExports);
	CreateExports();
}

/**
Function to generate the output E32 image. Incase of an output DEF file, then the
DEF file and the corresponding DSO file should be generated.

@internalComponent
@released
*/
void POLYDLLRebuildTarget::BuildAll() {

	if( UseCaseBase::DefOutput() ) {
		WriteDefFile();
	}
	if(UseCaseBase::DSOOutput() && UseCaseBase::LinkAsDLLName()) {
		WriteDSOFile();
	}
	WriteE32();
}
