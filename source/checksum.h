// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Implementation of the Header file for CheckSum implementation of the elf2e32 tool
// @internalComponent
// @released
//
//

#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <cstddef>
uint32_t checkSum(const void *aPtr);
void Crc(uint16_t & aCrc,const void * aPtr,uint32_t aLength);
void Crc32(uint32_t & aCrc, const void * aPtr, uint32_t aLength);

#endif // CHECKSUM_H



