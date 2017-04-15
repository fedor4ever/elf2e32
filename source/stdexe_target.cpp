// Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Implementation of the Class StdExeTarget for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "stdexe_target.h"

/**
Constructor for the StdExeTarget Class

@internalComponent
@released
*/
StdExeTarget::StdExeTarget(ParameterManager* aParameterManager):
DLLTarget(aParameterManager)
{
}

/**
Destructor for the StdExeTarget Class

@internalComponent
@released
*/
StdExeTarget::~StdExeTarget(){

}

/**
Function to write E32 image. Here, no def file or Dso file is generated.

@internalComponent
@released

@result void
*/
void StdExeTarget::GenerateOutput()
{
	WriteE32();
}

bool StdExeTarget::WarnForNewExports()
{
	return false;
}


