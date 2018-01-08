// Copyright (c) 1996-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#ifndef BYTE_PAIR_H
#define BYTE_PAIR_H

#include <portable.h>

TInt BytePairCompress(TUint8* dst, TUint8* src, TInt size);
TInt Pak(TUint8* dst, TUint8* src, TInt size);
TInt Unpak(TUint8* dst, TInt dstSize, TUint8* src, TInt srcSize, TUint8*& srcNext);

#endif
