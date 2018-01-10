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
#include <stdio.h>
#include <cstring>
#include "errorhandler.h"
#include "message.h"

using std::cerr;
using std::endl;

constexpr char *errMssgPrefix="elf2e32 : Error: E";
constexpr char *colonSpace=": ";

void ErrorReport(ErrorHandler *handle);
void ErrorReport2(ErrorHandler *handle);
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
	iMessage+=std::to_string(BASEMSSGNO+iMessageIndex);
	iMessage+=colonSpace;
};

/**
FileError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - File name
@internalComponent
@released
*/
FileError::FileError(int aMessageIndex, char * aName) :
		ErrorHandler(aMessageIndex) {iName = aName;}

/**
Function to report File Errors.
@internalComponent
@released
*/
void FileError::Report()
{
	ErrorReport(this);
}

/**
ELFFormatError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - ELF File name
@internalComponent
@released
*/
ELFFormatError::ELFFormatError(int aMessageIndex, char * aName) :
		ErrorHandler(aMessageIndex) {iName = aName;}

/**
Function to report ELF Format Errors.
@internalComponent
@released
*/
void ELFFormatError::Report()
{
	ErrorReport(this);
}

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

/**
ParameterParserError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@Param aName - Argument name
@internalComponent
@released
*/
ParameterParserError::ParameterParserError(int aMessageIndex, char * aName) :
		ErrorHandler(aMessageIndex) {iName = aName;}

/**
Function to report Parameter Parser Error.
@internalComponent
@released
*/
void ParameterParserError::Report()
{
	ErrorReport(this);
}

/**
InvalidArgumentError constructor for initializing message index, argument value and option.
@param aMessageIndex - Message Index
@param aValue - Argument value
@param aOption - Argument option
@internalComponent
@released
*/
InvalidArgumentError::InvalidArgumentError(int aMessageIndex,
		const char * aValue, char * aOption) :
			ErrorHandler(aMessageIndex), iValue(aValue), iOption(aOption) {}

/**
Function to report Invalid Argument Error.
@internalComponent
@released
*/
void InvalidArgumentError::Report()
{
	ErrorReport(this);
}

/**
E32ImageCompressionError constructor for initializing message index.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
E32ImageCompressionError::E32ImageCompressionError(int aMessageIndex) :
		ErrorHandler(aMessageIndex) {}

/**
Function to report E32 Image Compression Error.
@internalComponent
@released
*/
void E32ImageCompressionError::Report()
{
	ErrorReport2(this);
}

/**
CapabilityError constructor for initializing message index.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
CapabilityError::CapabilityError(int aMessageIndex) : ErrorHandler(aMessageIndex) {}

/**
Function to report Capability Error.
@internalComponent
@released
*/
void CapabilityError::Report()
{
	ErrorReport2(this);
}

/**
UnrecognisedCapabilityError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - Capability
@internalComponent
@released
*/
UnrecognisedCapabilityError::UnrecognisedCapabilityError(int aMessageIndex,
		string aName) : CapabilityError(aMessageIndex) {iName = aName;}

/**
Function to report Unrecognised Capability Error.
@internalComponent
@released
*/
void UnrecognisedCapabilityError::Report()
{
	ErrorReport(this);
}

/**
ELFFileError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - ELF File name
@internalComponent
@released
*/
ELFFileError::ELFFileError(int aMessageIndex, const char * aName) :
		ErrorHandler(aMessageIndex) {iName = aName;}

/**
Function to report ELF File Error.
@internalComponent
@released
*/
void ELFFileError::Report()
{
	ErrorReport(this);
}

/**
UndefinedSymbolError constructor for initializing message index, argument name and symbol name.
@param aMessageIndex - Message Index
@param aName - File Name
@param aSymbolName - Symbol Name
@internalComponent
@released
*/
UndefinedSymbolError::UndefinedSymbolError(int aMessageIndex,
		char * aName, char *aSymbolName) :
			ELFFileError(aMessageIndex,aName) {iSymbolName = aSymbolName;}

/**
Function to report Undefined Symbol Error.
@internalComponent
@released
*/
void UndefinedSymbolError::Report()
{
	ErrorReport(this);
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
MemoryAllocationError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - File Name
@internalComponent
@released
*/
MemoryAllocationError::MemoryAllocationError(int aMessageIndex, char * aName) :
		ErrorHandler(aMessageIndex) {iName = aName;}

/**
Function to report Memory Allocation Error.
@internalComponent
@released
*/
void MemoryAllocationError::Report()
{
	ErrorReport(this);
}

/**
E32ImageError constructor for initializing message index.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
E32ImageError::E32ImageError(int aMessageIndex) : ErrorHandler(aMessageIndex) {}

/**
Function to report E32 Image Error.
@internalComponent
@released
*/
void E32ImageError::Report()
{
	ErrorReport2(this);
}

/**
InvalidInvocationError constructor for initializing message index.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
InvalidInvocationError::InvalidInvocationError(int aMessageIndex) :
		ErrorHandler(aMessageIndex) {}

/**
Function to report Invalid Invocation Error.
@internalComponent
@released
*/
void InvalidInvocationError::Report()
{
	ErrorReport2(this);
}

/**
TargetTypeError constructor for initializing message index.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
TargetTypeError::TargetTypeError(int aMessageIndex) : ErrorHandler(aMessageIndex) {}

/**
Function to report Target Type Error.
@internalComponent
@released
*/
void TargetTypeError::Report()
{
	ErrorReport2(this);
}

/**
UnsupportedTargetTypeError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - Target type name
@internalComponent
@released
*/
UnsupportedTargetTypeError::UnsupportedTargetTypeError(int aMessageIndex, char * aName) :
		TargetTypeError(aMessageIndex) {iName = aName;}

/**
Function to report Unsupported Target Type Error.
@internalComponent
@released
*/
void UnsupportedTargetTypeError::Report()
{
	ErrorReport(this);
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
NoMessageFileError constructor for initializing message index.
@param aMessageIndex - Message Index
@internalComponent
@released
*/
NoMessageFileError::NoMessageFileError(int aMessageIndex) : ErrorHandler(aMessageIndex) {}

/**
Function to report No Message File Error.
@internalComponent
@released
*/
void NoMessageFileError::Report()
{
	ErrorReport2(this);
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

/**
InvalidE32ImageError constructor for initializing message index and argument name.
@param aMessageIndex - Message Index
@param aName - E32Image File name
@internalComponent
@released
*/
InvalidE32ImageError::InvalidE32ImageError(int aMessageIndex, char * aName) :
		ErrorHandler(aMessageIndex) {iName = aName;}

/**
Function to report Invalid E32 Image Error.
@internalComponent
@released
*/
void InvalidE32ImageError::Report()
{
	ErrorReport(this);
}

/**
ImportRelocationError constructor for initializing message index, argument name and symbol name.
@param aMessageIndex - Message Index
@param aName - File Name
@param aSymbolName - Symbol Name
@internalComponent
@released
*/
ImportRelocationError::ImportRelocationError(int aMessageIndex,
		char * aName, char *aSymbolName) : ELFFileError(aMessageIndex,aName)
{iSymbolName = aSymbolName;}

/**
Function to report Import Relocations references to Data Segment Error
@internalComponent
@released
*/
void ImportRelocationError::Report()
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

void ErrorReport2(ErrorHandler *handle)
{
	char *errMessage=GetMessage(handle->iMessageIndex);
	if(errMessage)
	{
		handle->iMessage+=errMessage;
		Message::GetInstance()->Output(handle->iMessage);
	}
}

