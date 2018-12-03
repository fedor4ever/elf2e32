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

class Elf2e32Error : public ErrorHandler
{
public:
	explicit Elf2e32Error(int aMessageIndex);
	explicit Elf2e32Error(int aMessageIndex, string aName);
	explicit Elf2e32Error(int aMessageIndex, string aName, string aParam);
	virtual ~Elf2e32Error(){}
	void Report(){}
};

/**
Class for handling Symbol Missing From Elf Errors.
@internalComponent
@released
*/
class SymbolMissingFromElfError : public ErrorHandler
{
public:
	SymbolMissingFromElfError(int aMessageIndex, list<string> &aSymbols, const char * aName);
	virtual ~SymbolMissingFromElfError(){}
	void Report();
	string iMissedSymbol;
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

#endif

