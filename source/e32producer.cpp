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
// Class implementation for creation E32 image in the elf2e32 tool
// @internalComponent
// @released
//
//

#include <fstream>

#include "e32common.h"
#include "e32parser.h"
#include "e32producer.h"
#include "errorhandler.h"
#include "e32validator.h"
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
    ValidateE32Image(s, size);

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

uint32_t checkSum(const void *aPtr);
uint32_t GetUidChecksum(uint32_t uid1, uint32_t uid2, uint32_t uid3)
{
    const uint32_t KMaxCheckedUid=3;
    uint32_t uids[KMaxCheckedUid] = {0};
    uids[0]=uid1;
	uids[1]=uid2;
	uids[2]=uid3;
    return (checkSum(&( (uint8_t*)uids)[1] ) << 16) | checkSum(uids);
}
