// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors: Strizhniou Fiodar - fix build and runtime errors.
//
// Description:
// Implementation of the Class LibraryTarget for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "librarytarget.h"
#include "deffile.h"
#include <iostream>
#include <stdlib.h>

/**
Constructor for the Library Class

@internalComponent
@released
*/
LibraryTarget::LibraryTarget(ParameterManager* aParameterManager):
ElfFileSupplied(aParameterManager)
{
	iDefFile = new DefFile();
}


/**
Destructor for the Library Class

@internalComponent
@released
*/
LibraryTarget::~LibraryTarget()
{
	delete iDefFile;
}

/**
Execute Function for the library target creation. The symbols are read from the input
DEF file. The DSO file is generated on passing the symbols.

@internalComponent
@released

@return EXIT_SUCCESS, on successful creation of library.
*/
int LibraryTarget::Execute()
{
    Symbols *tmp = ReadInputDefFile();
    iSymbols.splice(iSymbols.begin(), *tmp);
	WriteDSOFile();
	return EXIT_SUCCESS;
}

/**
Function to read the symbols from the DEF file.

@internalComponent
@released

@return the list of symbols read from the DEF file.
*/
Symbols* LibraryTarget::ReadInputDefFile()
{
	char * aDEFFileName = UseCaseBase::DefInput();

	return iDefFile->ReadDefFile(aDEFFileName);
}

