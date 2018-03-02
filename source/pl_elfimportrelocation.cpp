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
// Contributors: Strizhniou Fiodar - fix build and runtime errors.
//
// Description:
// Implementation of the Class ElfImportRelocation for the elf2e32 tool
// @internalComponent
// @released
//
//

#include "pl_elfimportrelocation.h"
#include "pl_elfimage.h"


/**
Constructor for class ElfImportRelocation
@param aElfImage - Instance of class ElfImage
@param aAddr
@param aAddend
@param aIndex
@param aRelType
@param aRel
@internalComponent
@released
*/
ElfImportRelocation::ElfImportRelocation(ElfImage *aElfImage,PLMemAddr32 aAddr,
			PLUINT32 aAddend, PLUINT32 aIndex, PLUCHAR aRelType,
			Elf32_Rel* aRel):
			ElfRelocation(aElfImage, aAddr, aAddend, aIndex, aRelType, aRel)
{
	iSymbol = &(aElfImage->iElfDynSym[iSymNdx]);
	iSegment = aElfImage->GetSegmentAtAddr(iAddr);
	iSegmentType = aElfImage->SegmentType(iAddr);
}


ElfImportRelocation::~ElfImportRelocation(){
	delete iVerRecord;
}

/**
Function to add import relocation
@internalComponent
@released
*/
void ElfImportRelocation::Add() {
	iElfImage->AddToImports(this);
}


