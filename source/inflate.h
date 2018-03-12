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
// Class for inflater
// @internalComponent
// @released
//
//

#ifndef __INFLATE_H__
#define __INFLATE_H__

#include "portable.h"
#include "huffman.h"


/**
Class for inflater
@internalComponent
@released
*/
class CInflater
{
	public:
		enum {EBufSize = 0x800, ESafetyZone=8};
	public:
		static CInflater* NewLC(TBitInput& aInput);
		~CInflater();
		TInt ReadL(TUint8* aBuffer,TInt aLength);
		TInt SkipL(TInt aLength);
	private:
		CInflater(TBitInput& aInput);
		void ConstructL();
		void InitL();
		TInt InflateL();
	private:
		TBitInput* iBits;
		const TUint8* iRptr;			// partial segment
		TInt iLen;
		const TUint8* iAvail;			// available data
		const TUint8* iLimit;
		TEncoding* iEncoding;
		TUint8* iOut;					// circular buffer for distance matches
		TUint8 iHuff[EBufSize+ESafetyZone];	// huffman data
};


#endif

