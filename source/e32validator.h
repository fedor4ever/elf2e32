#ifndef E32VALIDATOR_H
#define E32VALIDATOR_H

#include <cstdint>

class E32Parser;
class E32ImageHeader;
class E32ImageHeaderV;

int32_t ValidateE32Image(const char *buffer, uint32_t size);

class E32Validator
{
    public:
        E32Validator(const char *buffer, uint32_t size);
        ~E32Validator();
        int32_t ValidateE32Image();
    private:
        int32_t ValidateHeader();
        int32_t ValidateRelocations(uint32_t offset, uint32_t sectionSize);
        int32_t ValidateImports();
        int32_t ValidateExportDescription() const;
    private:
        E32Parser *iParser = nullptr;
        E32ImageHeader *iHdr = nullptr;
        E32ImageHeaderV *iHdrV = nullptr;
        uint32_t iBufSize = 0;
        uint32_t iPointerAlignMask = 0;
        bool iIsParsed = false;
};

#endif // E32VALIDATOR_H
