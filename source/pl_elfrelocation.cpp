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
// Contributors: Strizhniou Fiodar - fix build and runtime errors.
//
// Description:
// Implementation of the Class ElfRelocation for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "pl_elfrelocation.h"
#include "pl_elflocalrelocation.h"
#include "pl_symbol.h"

/**
Constructor for class ElfRelocation
@param aElfExec - Instance of class ELfExecutable
@param aAddr
@param aAddend
@param aIndex
@param aRelType - Relocation type
@param aRel
@internalComponent
@released
*/
ElfRelocation::ElfRelocation(ElfImage *aElfExec, PLMemAddr32 aAddr,
		PLUINT32 aAddend, PLUINT32 aIndex, PLUCHAR aRelType,
		Elf32_Rel* aRel) : iAddr(aAddr), iAddend(aAddend),
		iSymNdx(aIndex), iRelType(aRelType), iRel(aRel),
		iElfImage(aElfExec) {}
