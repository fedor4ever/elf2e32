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
// Contributors:  Strizhniou Fiodar - fix build and runtime errors.
//
// Description:
// Implementation of the Header file for Class Elf2E32
// @internalComponent
// @released
//
//

#ifndef ELF2E32_H
#define ELF2E32_H

#include "pl_common.h"
/**
This class calls the single instance of the ParameterManager.

@internalComponent
@released
*/
struct Elf2E32
{
	Elf2E32();
	Elf2E32(int argc, char** argv);
	~Elf2E32(){}
	int Execute();
};


#endif // ELF2E32_H
