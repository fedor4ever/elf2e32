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
// Implementation of the Class ExeTarget for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "exetarget.h"

/**
Constructor for the ExeTarget Class

@internalComponent
@released
*/
ExeTarget::ExeTarget(ParameterManager* aParameterManager):
ElfFileSupplied(aParameterManager)
{
}

/**
Destructor for the ExeTarget Class

@internalComponent
@released
*/
ExeTarget::~ExeTarget()
{
}

/**
Function to process the symbols to be exported. This function is empty since there are no
exports in case of EXE Target. However, this dummy implementation is included since
this is a virtual function in ElfFileSuplied Class.

@internalComponent
@released
*/
void ExeTarget::ProcessExports()
{
}

/**
Function to check if the provided input is a DLL.

@internalComponent
@released

@result False since EXE Target is not a DLL.
*/
bool ExeTarget::ImageIsDll()
{
	return false;
}
