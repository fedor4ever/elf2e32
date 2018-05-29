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
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
// Error Handler Operations for elf2e32 tool
// @internalComponent
// @released
//
//

#ifdef _MSC_VER
	#pragma warning(disable: 4514) // unreferenced inline function has been removed
	#pragma warning(disable: 4702) // unreachable code
	#pragma warning(disable: 4710) // function not inlined
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <cstring>
#include "errorhandler.h"
#include "message.h"

using std::cerr;
using std::endl;

constexpr char *errMssgPrefix="elf2e32 : Error: E";
constexpr char *colonSpace=": ";

void ErrorReport(ErrorHandler *handle);
char *GetMessage(int aMessageIndex);

char *GetMessage(int aMessageIndex)
{
    return Message::GetInstance()->GetMessageString(aMessageIndex);
}

/**
ErrorHandler constructor for doing common thing required for derived class functions.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
ErrorHandler::ErrorHandler(int aMessageIndex) :
    iMessageIndex(aMessageIndex)
{
	iMessage=errMssgPrefix;
//	iMessage+=std::to_string(BASEMSSGNO+iMessageIndex); // hack: doesn't available in mingw

    std::ostringstream s;
    s << BASEMSSGNO+iMessageIndex;
    iMessage+=s.str();
	iMessage+=colonSpace;
};

/**
DEFFileError constructor for initializing message index, argument name and line number and token.
@param aMessageIndex - Message Index
@param aFileName - DEF File name
@param aLineNo - DEF File line number
@param aToken - Token in export entry
*/
DEFFileError::DEFFileError(int aMessageIndex, char * aFileName, int aLineNo,char * aToken) :
     ErrorHandler(aMessageIndex), iLineNo(aLineNo)
{
    iName = aFileName;
	iToken=aToken;
	if(iToken[iToken.size()-1]=='\r')
		iToken[iToken.size()-1]='\0';
}

void DEFFileError::Report()
{
    iMessage+=GetMessage(iMessageIndex);
    char *buf = new char[strlen(iMessage.c_str()) + strlen(iToken.c_str()) + strlen(iName.c_str()) + 1];
    sprintf(buf, iMessage.c_str(), iName.c_str(), iLineNo, iToken.c_str());
    Message::GetInstance()->Output(buf);
}


Elf2e32Error::Elf2e32Error(int aMessageIndex) : ErrorHandler(aMessageIndex)
{
    Message::GetInstance()->ReportMessage(ERROR, iMessageIndex);
}

Elf2e32Error::Elf2e32Error(int aMessageIndex, string aName) : ErrorHandler(aMessageIndex)
{
    Message::GetInstance()->ReportMessage(ERROR, iMessageIndex, aName.c_str());
}

Elf2e32Error::Elf2e32Error(int aMessageIndex, string aName, string aParam) : ErrorHandler(aMessageIndex)
{
    Message::GetInstance()->ReportMessage(ERROR, iMessageIndex, aName.c_str(), aParam.c_str());
}

/**
SymbolMissingFromElfError constructor for initializing message index, symbol name list and argument name.
@param aMessageIndex - Message Index
@param aSymbolList - List of symbols
@param aName - File Name
@internalComponent
@released
*/
SymbolMissingFromElfError::SymbolMissingFromElfError(int aMessageIndex,
		list<string> &aSymbolList, const char * aName) :
				ErrorHandler(aMessageIndex)
{
    MissedSymbol = aSymbolList.size();
    iName = aName;
}

/**
Function to report Symbol Missing From Elf Error.
@internalComponent
@released
*/
void SymbolMissingFromElfError::Report()
{
    iMessage+=GetMessage(iMessageIndex);
    char *buf = new char[strlen(iMessage.c_str()) + strlen(iName.c_str()) + 1];
    sprintf(buf, iMessage.c_str(), MissedSymbol, iName.c_str());
    Message::GetInstance()->Output(buf);
}

/**
MessageError constructor for initializing message index and index value.
@param aMessageIndex - Message Index
@param aIndexValue - Value of Index
@internalComponent
@released
*/
MessageError::MessageError(int aMessageIndex, int aIndexValue) :
		ErrorHandler(aMessageIndex), iIndexValue(aIndexValue) {}

/**
Function to report Message Errors.
@internalComponent
@released
*/
void MessageError::Report()
{
	ErrorReport(this);
}

/**
SysDefMismatchError constructor for initializing message index, symbol name list and argument name.
@param aMessageIndex - Message Index
@param aSymbolList - list of symbols
@param aName - sysdef
@internalComponent
@released
*/
SysDefMismatchError::SysDefMismatchError(int aMessageIndex,
             list<string> &aSymbolList, const char * aName) :
                 ErrorHandler(aMessageIndex)
{
    iName = aName;
	std::list<string>::iterator aItr = aSymbolList.begin();
	std::list<string>::iterator last = aSymbolList.end();

	while(aItr != last)
	{
		iSymbolNames+=*aItr;
		++aItr;
		if(aItr != last)
		{
			iSymbolNames+=",";
		}
	}

}

/**
Function to report SysDef Mismatch Error.
@internalComponent
@released
*/
void SysDefMismatchError::Report()
{
	ErrorReport(this);
}

void ErrorReport(ErrorHandler *handle)
{
	char *errMessage=GetMessage(handle->iMessageIndex);
	if(errMessage)
	{
		char *tempMssg = new char[strlen(errMessage)+strlen(handle->iName.c_str())];
		sprintf(tempMssg, errMessage, handle->iName.c_str());
		handle->iMessage+=tempMssg;
		Message::GetInstance()->Output(handle->iMessage);
		delete[] tempMssg;
	}
}

