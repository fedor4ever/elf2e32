// Copyright (c) 2018 Strizhniou Fiodar
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Strizhniou Fiodar - initial contribution.
//
// Contributors:
//
// Description:
// Implementation of the Class DefFile for the elf2e32 tool
// @internalComponent
// @released
//
// .def file specification:
//  1) every file may have comments started from ";" till end line
//  2) every file must have header statement "EXPORTS" on separate line
//  3) every line on file followed header statement may be empty or commem
//    or have symbol statement
//  4) every line on file with symbol statement must have instructions that order:
//    [symbol-name] "@" [export ordinal] "NONAME" [optional instructions]
//    example with all instructions:
//        BIGNUM_it=B_it @ 2717 NONAME DATA R3UNUSED ABSENT 28; some comment
//  5) symbol-name instruction may looks like symbol-name=alias-name,
//    white spaces not allowed
//  6) export ordinal must contain numbers only
//  7) optional instructions
//    they must followed in that order
//        NONAME DATA R3UNUSED ABSENT 28
//    if "DATA" instruction present, statement must have numbers at end line or before comment
//
//    Note: original class was replaced at commit 2c9b13fc8073d97b4f66e6622880c2b6f0520915

#ifndef _DEF_FILE_
#define _DEF_FILE_

#include <list>
#include <string>
#include <vector>

class Symbol;
typedef std::list <Symbol*>	Symbols;

/**
Class for DEF File Handler.
@internalComponent
@released
*/
class DefFile
{
	public:
		Symbols GetSymbols(const char *defFile);
		void WriteDefFile(const char *fileName, const Symbols &aSymbols);

	private:
		void ReadDefFile(const char *defFile);
		void ParseDefFile();
		void Tokenizer(std::string aLine, size_t aIndex);
    private:
		Symbols iSymbols;
		Symbol *iSymbol = nullptr;
		std::vector<std::string> iDefFile;
		std::string iFileName;
};

#endif
