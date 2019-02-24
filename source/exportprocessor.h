#ifndef EXPORTPROCESSOR_H
#define EXPORTPROCESSOR_H

#include <list>

class Symbol;
class ParameterManager;

typedef std::list<Symbol*> Symbols;

Symbols GetExports(ParameterManager *param);

class ExportProcessor
{
public:
    ExportProcessor(ParameterManager *man);
    ~ExportProcessor();
    Symbols Run();

private:
    void SymbolsFromSysdefOption(Symbols& s);
    void SymbolsFromDEF(Symbols& s);
    void CmpSymbolsFrmDEFandSysdefopt(Symbols& defSymbol, Symbols& sysDefSymbol);
private:
    Symbols iExports;
    ParameterManager *iMan = nullptr;
};

#endif // EXPORTPROCESSOR_H
