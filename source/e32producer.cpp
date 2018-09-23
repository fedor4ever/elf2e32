
#include <fstream>

#include "e32common.h"
#include "e32parser.h"
#include "e32producer.h"
#include "parametermanager.h"

using std::ofstream;

void DeflateCompress(char *buf, size_t size, ofstream & os);
void CompressPages(uint8_t *buf, int32_t size, ofstream& os);

E32Producer::E32Producer(ParameterManager *args) : iMan(args)
{
    //ctor
}

E32Producer::~E32Producer()
{
    //dtor
}

void E32Producer::Run()
{
    ReCompress();
    MakeE32();
}

/** @brief Decompress and saves E32Image
  *
  * If compression not set saves uncompressed image
  */
void E32Producer::ReCompress()
{
    if( !(iMan->E32Input() && iMan->E32ImageOutput()) )
     return;

    E32Parser *parser = new E32Parser(iMan->E32Input());
    iE32Hdr = parser->GetFileLayout();
    iE32Hdr->iCompressionType = iMan->CompressionMethod();

    SaveE32(parser->GetBufferedImage(), parser->GetFileSize());

    delete parser;
}

void E32Producer::MakeE32()
{
    return;
}

void E32Producer::SaveE32(const char* s, size_t size)
{
    ofstream fs(iMan->E32ImageOutput(), ofstream::binary|ofstream::out);
    if(!fs)
        throw Elf2e32Error(FILEOPENERROR, iMan->E32ImageOutput());

    uint32_t compression = iE32Hdr->iCompressionType;
    if(compression > 0)
    {
        uint32_t offset = iE32Hdr->iCodeOffset;
        fs.write(s, offset);

        if(compression == KUidCompressionDeflate)
            DeflateCompress((char*)s + offset, size - offset, fs);

        else if (compression == KUidCompressionBytePair)
        {
            // Compress and write out code part
            CompressPages( (uint8_t*)(s + offset), iE32Hdr->iCodeSize, fs);

            // Compress and write out data part
			offset += iE32Hdr->iCodeSize;
			CompressPages( (uint8_t*)(s + offset), size - offset, fs);
        }
    }
    else
        fs.write(s, size);

    fs.close();
}

