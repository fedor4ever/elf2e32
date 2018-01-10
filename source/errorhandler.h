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
// Error Handler Classes for elf2e32 tool
// @internalComponent
// @released
//
//


#ifndef _ERROR_HANDLER_
#define _ERROR_HANDLER_

#include "message.h"
#include <list>
#include <string>

using std::list;
using std::string;

/**
Base class from which all other error handler classes are derived from.
@internalComponent
@released
*/
class ErrorHandler
{
public:
	explicit ErrorHandler(int aMessageIndex);
	virtual ~ErrorHandler(){}
	virtual void Report() =0;

	string iMessage;
	int iMessageIndex;
	string iName;
};

/**
Base class for File Errors.
@internalComponent
@released
*/
class FileError : public ErrorHandler
{
public:
	FileError(int aMessageIndex, char * aName);
	virtual ~FileError(){}
	void Report();
};

/**
Base class for ELFFormat Errors.
@internalComponent
@released
*/
class ELFFormatError : public ErrorHandler
{
public:
	ELFFormatError(int aMessageIndex, char * aName);
	virtual ~ELFFormatError(){}
	void Report();
};

/**
Base class for DEF File Errors.
@internalComponent
@released
*/
class DEFFileError : public ErrorHandler
{
public:
	DEFFileError(int aMessageIndex, char * aName, int aLineNo,char * aToken);
	virtual ~DEFFileError(){}
	void Report();

	int iLineNo;
	string iToken;
};

/**
Base class for Parameter Parser Errors.
@internalComponent
@released
*/
class ParameterParserError : public ErrorHandler
{
public:
	ParameterParserError(int aMessageIndex, char * aName);
	virtual ~ParameterParserError(){}
	void Report();
};

/**
Class for Invalid Argument Errors.
@internalComponent
@released
*/
class InvalidArgumentError : public ErrorHandler
{
public:
	InvalidArgumentError(int aMessageIndex, const char * aValue, char * aOption);
	virtual ~InvalidArgumentError(){}
	void Report();

	string iValue;
	string iOption;
};

/**
Base class for E32Image Compression Errors.
@internalComponent
@released
*/
class E32ImageCompressionError : public ErrorHandler
{
public:
	explicit E32ImageCompressionError(int aMessageIndex);
	virtual ~E32ImageCompressionError(){}
	void Report();
};

/**
Base class for Capability Errors.
@internalComponent
@released
*/
class CapabilityError : public ErrorHandler
{
public:
	explicit CapabilityError(int aMessageIndex);
	virtual ~CapabilityError(){}
	void Report();
};

/**
Class for handling Unrecognised Capability Errors.
@internalComponent
@released
*/
class UnrecognisedCapabilityError : public CapabilityError
{
public:
	UnrecognisedCapabilityError(int aMessageIndex, string aName);
	virtual ~UnrecognisedCapabilityError(){}
	void Report();
};

/**
Base class for ELF File Errors.
@internalComponent
@released
*/
class ELFFileError : public ErrorHandler
{
public:
	explicit ELFFileError(int aMessageIndex, const char * aName);
	virtual ~ELFFileError(){}
	void Report();
	string iSymbolName;
};

/**
Class for handling Undefined Symbol Errors.
@internalComponent
@released
*/
class UndefinedSymbolError : public ELFFileError
{
public:
	UndefinedSymbolError(int aMessageIndex, char * aName, char *aSymbolName);
	virtual ~UndefinedSymbolError(){}
	void Report();
};

/**
Class for handling Import relocation to Data segment
@internalComponent
@released
*/
class ImportRelocationError : public ELFFileError
{
public:
	ImportRelocationError(int aMessageIndex, char * aName, char *aSymbolName);
	virtual ~ImportRelocationError(){}
	void Report();
};

/**
Class for handling Symbol Missing From Elf Errors.
@internalComponent
@released
*/
class SymbolMissingFromElfError : public ErrorHandler
{
public:
	SymbolMissingFromElfError(int aMessageIndex, list<string> &aSymbolList, const char * aName);
	virtual ~SymbolMissingFromElfError(){}
	void Report();
	uint32_t MissedSymbol = 0;
};

/**
Class for handling Memory Allocation Errors.
@internalComponent
@released
*/
class MemoryAllocationError : public ErrorHandler
{
public:
	MemoryAllocationError(int aMessageIndex, char * aName);
	virtual ~MemoryAllocationError(){}
	void Report();
};

/**
Class for handling E32 Image Errors.
@internalComponent
@released
*/
class E32ImageError : public ErrorHandler
{
public:
	explicit E32ImageError(int aMessageIndex);
	virtual ~E32ImageError(){}
	void Report();
};

/**
Class for handling Invalid Invocation Errors.
@internalComponent
@released
*/
class InvalidInvocationError : public ErrorHandler
{
public:
	explicit InvalidInvocationError(int aMessageIndex);
	virtual ~InvalidInvocationError(){}
	void Report();
};

/**
Base class for handling Target Type Errors.
@internalComponent
@released
*/
class TargetTypeError : public ErrorHandler
{
public:
	explicit TargetTypeError(int aMessageIndex);
	virtual ~TargetTypeError(){}
	void Report();
};

/**
Class for handling Unsupported Target Type Errors.
@internalComponent
@released
*/
class UnsupportedTargetTypeError : public TargetTypeError
{
public:
	UnsupportedTargetTypeError(int aMessageIndex, char * aName);
	virtual ~UnsupportedTargetTypeError(){}
	void Report();
};

/**
Class for handling Message Errors.
@internalComponent
@released
*/
class MessageError : public ErrorHandler
{
public:
	MessageError(int aMessageIndex, int aIndexValue);
	virtual ~MessageError(){}
	void Report();

	int iIndexValue;
};

/**
Class for handling No Message File Errors.
@internalComponent
@released
*/
class NoMessageFileError : public ErrorHandler
{
public:
	explicit NoMessageFileError(int aMessageIndex);
	virtual ~NoMessageFileError(){}
	void Report();
};

/**
Class for handling Symbol that are passed through --sysdef
not matching with the ones in the DEF file.
@internalComponent
@released
*/
class SysDefMismatchError : public ErrorHandler
{
public:
	SysDefMismatchError(int aMessageIndex, list<string> &aSymbolList, const char * aName);
	virtual ~SysDefMismatchError(){}
	void Report();

	string iSymbolNames;
};

/**
Class for handling Invalid E32 Image Error
@internalComponent
@released
*/
class InvalidE32ImageError : public ErrorHandler
{
public:
	InvalidE32ImageError(int aMessageIndex, char * aName);
	virtual ~InvalidE32ImageError(){}
	void Report();
};

#endif

