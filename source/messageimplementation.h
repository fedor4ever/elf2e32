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
// Message Implementation Class for elf2e32 tool
// @internalComponent
// @released
//
//


#ifndef _MESSAGE_IMPLEMENTATION_
#define _MESSAGE_IMPLEMENTATION_

#ifdef _MSC_VER
	#pragma warning(disable: 4514) // unreferenced inline function has been removed
	#pragma warning(disable: 4702) // unreachable code
	#pragma warning(disable: 4710) // function not inlined
	#pragma warning(disable: 4786) // identifier was truncated to '255' characters in the debug information
#endif

#include <string>
#include <map>

typedef std::map<int,char*> Map;

enum { MAXMSSGNOLENGTH=5, BASEMSSGNO=1000 };

enum { ERROR = 0,
	   WARNING,
	   INFORMATION
};

enum {	FILEOPENERROR=1,
		FILEREADERROR,
		FILEWRITEERROR,
		ELFMAGICERROR,
		ELFCLASSERROR,
		ELFABIVERSIONERROR,
		ELFLEERROR,
		ELFARMERROR,
		ELFEXECUTABLEERROR,
		ELFSHSTRINDEXERROR,
		NAMELIBRARYNOTCORRECT,
		ORDINALSEQUENCEERROR,
		ARGUMENTNAMEERROR,
		OPTIONNAMEERROR,
		NOARGUMENTERROR,
		OPTIONPREFIXERROR,
		NOREQUIREDOPTIONERROR,
		NOFILENAMEERROR,
		INVALIDARGUMENTERROR,
		HUFFMANBUFFEROVERFLOWERROR,
		HUFFMANTOOMANYCODESERROR,
		HUFFMANINVALIDCODINGERROR,
		CAPABILITYALLINVERSIONERROR,
		CAPABILITYNONEINVERSIONERROR,
		UNRECOGNISEDCAPABILITYERROR,
		NOSTATICSYMBOLSERROR,
		DLLHASINITIALISEDDATAERROR,
		DLLHASUNINITIALISEDDATAERROR,
		ENTRYPOINTCORRUPTERROR,
		ENTRYPOINTNOTSUPPORTEDERROR,
		EXCEPTIONDESCRIPTOROUTSIDEROERROR,
		NOEXCEPTIONDESCRIPTORERROR,
		NEEDSECTIONVIEWERROR,
		DSONOTFOUNDERROR,
		UNDEFINEDSYMBOLERROR,
		SYMBOLMISSINGFROMELFERROR,
		MEMORYALLOCATIONERROR,
		E32IMAGEERROR,
		INVALIDINVOCATIONERROR,
		TARGETTYPENOTSPECIFIEDERROR,
		UNSUPPORTEDTARGETTYPEERROR,
		INDEXNOMESSAGEERROR,
		INDEXNOTREQUIREDERROR,
		INDEXNOTFOUNDERROR,
		NOMESSAGEFILEERROR,
		ENTRYPOINTNOTSETERROR,
		UNDEFINEDENTRYPOINTERROR,
		ORDINALNOTANUMBER,
		UNRECOGNIZEDTOKEN,
		NONAMEMISSING,
		EXPORTSEXPECTED,
		ATRATEMISSING,
		SYSDEFSMISMATCHERROR,
		SYSDEFERROR,
		INVALIDE32IMAGEERROR,
		HUFFMANBUFFERUNDERFLOWERROR,
		HUFFMANINCONSISTENTSIZEERROR,
		MULTIPLESYSDEFERROR,
		SYSDEFNOSYMBOLERROR,
		VALUEIGNOREDWARNING,
		ELFFILEERROR,
		SYMBOLCOUNTMISMATCHERROR,
		POSTLINKERERROR,
		BYTEPAIRINCONSISTENTSIZEERROR,
		ILLEGALEXPORTFROMDATASEGMENT,
		VALIDATIONERROR
};


class Message
{
    public:
		Message();
		~Message();
		char* GetMessageString(int errorIndex);
		void Output(const std::string &aInfo);
		void StartLogging(char *fileName);
	//	void ReportWarning(int warnIndex,...);
		void ReportMessage(int aMsgType, int aMsgIndex,...);
		void CreateMessageFile(char *fileName);
		void InitializeMessages(char *fileName);
    private:

		bool iLogging;
		char* iLogFileName;
		FILE *iLogPtr;
		Map iMessage;
};

/**
Structure for Messages.
@internalComponent
@released
*/
struct EnglishMessage
{
	int index;
	char message[1024];
};

#endif

