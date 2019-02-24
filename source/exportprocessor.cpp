#include <string>
#include <list>

#include <string.h>
#include "message.h"
#include "deffile.h"
#include "pl_symbol.h"
#include "errorhandler.h"
#include "exportprocessor.h"
#include "parametermanager.h"

using std::string;

Symbols SymbolsFromDef(const char *defFile);

Symbols GetExports(ParameterManager *param)
{
    ExportProcessor *p = new ExportProcessor(param);
    Symbols r = p->Run();
    delete p;
    return r;
}

ExportProcessor::ExportProcessor(ParameterManager *man): iMan(man)
{}

ExportProcessor::~ExportProcessor()
{
    //dtor
}

Symbols ExportProcessor::Run()
{
    Symbols def, sys;
    SymbolsFromDEF(def);
    SymbolsFromSysdefOption(sys);
    CmpSymbolsFrmDEFandSysdefopt(def, sys);
    return def;
}

/**
Function to convert params in --sysdef option to symbols for EPolyDll target
@internalComponent
@released
*/
/// FIXME (Administrator#1#05/31/18): Rewrite to allow symbols in any order from --sysdef option
void ExportProcessor::SymbolsFromSysdefOption(Symbols &s)
{
    if(EPolyDll != iMan->TargetTypeName())
        return;

    int count = iMan->SysDefCount();
	ParameterManager::Sys sysDefSyms[10];

	for(int i =  0; i < count; i++)
	{
		sysDefSyms[i] = iMan->SysDefSymbols(i);
		Symbol *sym = new Symbol(sysDefSyms[i].iSysDefSymbolName, SymbolTypeCode);
		sym->SetOrdinal(sysDefSyms[i].iSysDefOrdinalNum);
		s.push_back(sym);
	}
}

/**
Function to get symbols from .def file
@internalComponent
@released
*/
void ExportProcessor::SymbolsFromDEF(Symbols& s)
{
    char * def = iMan->DefInput();
    if(!def)
        return;
	Symbols tmp = SymbolsFromDef(def);
	s.splice(s.begin(), tmp, tmp.begin(), tmp.end());
}

/**
Function to compare symbols from .def file and --sysdef input
@internalComponent
@released
*/
void ExportProcessor::CmpSymbolsFrmDEFandSysdefopt(Symbols& def, Symbols& sys)
{
    if(EPolyDll != iMan->TargetTypeName())
        return;
    if(sys.empty())
        return;

    if(!iMan->DefInput() || def.empty()){
        def.splice(def.begin(), sys);
        return;
    }

	// Check if the Sysdefs and the DEF file are matching.

	auto defBegin = def.begin();
	auto defEnd = def.end();

	auto sysBegin = sys.begin();
	auto sysEnd = sys.end();

	std::list<string> missingSysDefList;

	while ((defBegin != defEnd) && (sysBegin != sysEnd))
	{
		if (strcmp((*sysBegin)->SymbolName(), (*defBegin)->SymbolName()))
			missingSysDefList.push_back((*sysBegin)->SymbolName());
		++sysBegin;
		++defBegin;
	}

	if( missingSysDefList.empty() )
		throw SysDefMismatchError(SYSDEFSMISMATCHERROR, missingSysDefList, iMan->DefInput());
}
