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
// Implementation of the Header file for CheckSum implementation of the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <cstddef>
uint32_t checkSum(const void *aPtr);
uint16_t Crc(const void * aPtr,uint32_t aLength);
uint32_t Crc32(const void * aPtr, uint32_t aLength);

#endif // CHECKSUM_H



