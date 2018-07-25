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
// Contributors: Strizhniou Fiodar - fix build and runtime errors, refactoring.
//
// Description:
// Implementation of the Class Symbol for the elf2e32 tool
// @internalComponent
// @released
//
//

#include <cstring>
#include "pl_symbol.h"

Symbol::Symbol(std::string aName, SymbolType aCodeDataType):
    iSymbolName(aName), iSymbolType(aCodeDataType)
    {}

/**
This constructor sets the symbol members.
@internalComponent
@released
*/
Symbol::Symbol(Symbol& aSymbol, SymbolType aCodeDataType, bool aAbsent):
		iSymbolType(aCodeDataType), iAbsent(aAbsent)
{
	iSymbolName = aSymbol.SymbolName();
	iOrdinalNumber = aSymbol.OrdNum();
}

/**
Constructor for class Symbol
@param aName - symbol name
@param aType - symbol type
@param aElfSym - elf symbol
@param aSymbolIndex - index in the symbol table
@internalComponent
@released
*/
Symbol::Symbol(char* aName, SymbolType aType, Elf32_Sym* aElfSym,
    PLUINT32 aSymbolIndex): iElfSym(aElfSym), iSymbolIndex(aSymbolIndex),
    iSymbolName(aName), iSymbolType(aType)
{}

/**
This copy constructor copies the symbol members from the input symbol.
@param aSymbol - The symbol from which the members are to be copied.
@internalComponent
@released
*/
Symbol::Symbol(Symbol& aSymbol)
{
	iElfSym = aSymbol.iElfSym;
	iSymbolIndex = aSymbol.iSymbolIndex;
	iSymbolStatus = aSymbol.iSymbolStatus;
	iSymbolType = aSymbol.iSymbolType;
	iOrdinalNumber = aSymbol.iOrdinalNumber;
	iAbsent = aSymbol.iAbsent;
	iR3Unused = aSymbol.iR3Unused;
	iSize = aSymbol.iSize;

	iSymbolName = aSymbol.SymbolName();

	if(aSymbol.Comment().empty())
	{
		iComment = aSymbol.Comment();
	}

	if(aSymbol.ExportName())
	{
		iExportName = aSymbol.ExportName();
	}
}

Symbol::~Symbol() {}

/**
This function sets the symbol name.
@param aSymbolName - The symbol name
@internalComponent
@released
*/
void Symbol::SetSymbolName(char *aSymbolName)
{
	iSymbolName.assign(aSymbolName);
}

/**
This function compares the symbol for equality.
@param aSym - The symbol that is compared with this symbol
Return - It returns true if the 2 symbols are equal.
@internalComponent
@released
*/
bool Symbol::operator==(const Symbol* aSym) const {
	if(iSymbolName.compare(aSym->iSymbolName) != 0)
		return false;
	if( iSymbolType != aSym->iSymbolType )
		return false;

	return true;
}

/**
This function returns the symbol name.
@internalComponent
@released
*/
const char* Symbol::SymbolName() const {
	return iSymbolName.c_str();
}

/**
This function returns the aliased symbol name.
@internalComponent
@released
*/
const char* Symbol::ExportName() {
	 return iExportName.c_str();
}

/**
This function returns the ordinal number of the symbol.
@internalComponent
@released
*/
PLUINT32 Symbol::OrdNum() const {
	 return iOrdinalNumber;
}

/**
This function returns if the symbol is code or a data symbol.
@internalComponent
@released
*/
SymbolType Symbol::CodeDataType() {
	return iSymbolType;
}

/**
This function returns if r3unused is true.
@internalComponent
@released
*/
bool Symbol::R3unused() {
	return iR3Unused;
}

/**
This function returns if the symbol is marked absent in the def file.
@internalComponent
@released
*/
bool Symbol::Absent() {
	return iAbsent;
}

/**
This function sets the symbol to be absent.
@param aAbsent - bool value
@internalComponent
@released
*/
void Symbol::SetAbsent(bool aAbsent) {
	iAbsent = aAbsent;
}

/**
This function returns the comment against this def file.
@internalComponent
@released
*/
std::string Symbol::Comment() {
	return iComment;
}

/**
This function returns the symbol is a matching/missing/new symbol in the def file.
@internalComponent
@released
*/
int Symbol::GetSymbolStatus() {
	return  iSymbolStatus;
}

/**
This function sets the ordinal number for this symbol.
@internalComponent
@released
*/
void Symbol::SetOrdinal(PLUINT32 aOrdinalNum) {
	iOrdinalNumber=aOrdinalNum;
}

/**
This function sets the status of the symbol i.e., whether it is
a matching/missing/new symbol.
@internalComponent
@released
*/
void Symbol::SetSymbolStatus(SymbolStatus aSymbolStatus) {
	iSymbolStatus = aSymbolStatus;
}

/**
This function sets the export name of the symbol.
@param aComment - aExportName
@internalComponent
@released
*/
void Symbol::ExportName(char *aExportName)
{
	iExportName = aExportName;
}

/**
This function sets the comment against the symbol.
@param aComment - aComment
@internalComponent
@released
*/
void Symbol::Comment(std::string aComment)
{
	iComment = aComment;
}

/**
This function sets the symbol type if it is Code or Data symbol.
@param aType - Symbol Type
@internalComponent
@released
*/
void Symbol::CodeDataType(SymbolType aType)
{
	iSymbolType = aType;
}

/**
This function sets if R3Unused is true for this symbol.
@param aR3Unused - bool value
@internalComponent
@released
*/
void Symbol::R3Unused(bool aR3Unused)
{
	iR3Unused = aR3Unused;
}

/**
This function sets if R3Unused is true for this symbol.
@param aSize - size of the symbol
@internalComponent
@released
*/
void Symbol::SetSymbolSize(PLUINT32 aSize){
	iSize = aSize;
}

/**
This function gets the size of this symbol.
@internalComponent
@released
*/
PLUINT32 Symbol::SymbolSize(){
	return iSize;
}
