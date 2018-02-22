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
// Implementation of the Header file for Class LibraryTarget of the elf2e32 tool
// @internalComponent
// @released
//
//


#if !defined(SYMBIAN_LIBRARYTARGET_H_)
#define SYMBIAN_LIBRARYTARGET_H_

#include <list>

#include "elffilesupplied.h"
#include "usecasebase.h"

class Symbol;
class DefFile;

/**
This class is derived from the base class UseCaseBase and is responsible for creation of
Library Target.It passes the input DEF file to the DEFfile parser to get the Symbol list
and then passes the Symbol List to the ElfFileSupplied to generate the DSO file.

@internalComponent
@released
*/
class LibraryTarget : public ElfFileSupplied
{

public:
	explicit LibraryTarget(ParameterManager* aParameterManager);
	virtual ~LibraryTarget();
	int Execute();
	Symbols* ReadDefFile();

private:
	DefFile *iDefFile;
};



#endif // !defined(SYMBIAN_LIBRARYTARGET_H_)

