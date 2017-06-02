// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// Copyright (c) 2017 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors: Strizhniou Fiodar - fix build and runtime errors.
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//


 #ifndef __H_UTL_H__
 #define __H_UTL_H__

#include "portable.h"
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

 /**
 Convert string to number.
 @internalComponent
 @released
 */
 template <class T>
 TInt Val(T& aVal, char* aStr)
 {
	T x;
	istringstream val(aStr);
	val >> x;
	if (!val.eof() || val.fail())
		return KErrGeneral;
	aVal=x;
	return KErrNone;
 }


 //enum for decompose flag
 enum TDecomposeFlag
 {
     EUidPresent=1,
     EVerPresent=2
 };

 /**
 class for FileNameInfo
 @internalComponent
 @released
 */
 class TFileNameInfo
 {
     public:
         TFileNameInfo(const char* aFileName, bool aLookForUid);
     public:
         const char* iFileName;
         TInt iTotalLength;
         TInt iBaseLength;
         TInt iExtPos;
         TUint32 iUid3;
         TUint32 iModuleVersion;
         TUint32 iFlags;
 };

 extern char* NormaliseFileName(const char* aName);



 #ifdef __LINUX__
 // Case insensitive comparison functions are named differently on Linux
 #define stricmp strcasecmp
 #define strnicmp strncasecmp

 // Convert the provided string to Uppercase
 char* strupr(char *a);
 #endif // __LINUX__

 #endif // __H_UTL_H__









