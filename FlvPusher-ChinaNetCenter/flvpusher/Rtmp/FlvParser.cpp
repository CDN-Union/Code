#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "FlvParser.h"

using namespace std;

#define CheckBuffer(x) { if ((nBufSize-nOffset)<(x)) { nUsedLen = nOffset; return 0;} }

int CFlvParser::CAudioTag::_aacProfile;
int CFlvParser::CAudioTag::_sampleRateIndex;
int CFlvParser::CAudioTag::_channelConfig;

static const unsigned int nH264StartCode = 0x01000000;

CFlvParser::CFlvParser(onTagFunc cb)
{
    _pFlvHeader = NULL;
    _cb = cb;
}

CFlvParser::~CFlvParser()
{
    DestroyFlvHeader(_pFlvHeader);
    _pFlvHeader = NULL;
}

int CFlvParser::Parse(unsigned char *pBuf, int nBufSize, int &nUsedLen)
{
    int nOffset = 0;

    if (_pFlvHeader == 0)
    {
        CheckBuffer(9);
        _pFlvHeader = CreateFlvHeader(pBuf+nOffset);
        nOffset += _pFlvHeader->nHeadSize;
    }

    while (1)
    {
        CheckBuffer(15);
        int nPrevSize = ShowU32(pBuf + nOffset);
        nOffset += 4;

        Tag *pTag = CreateTag(pBuf + nOffset, nBufSize-nOffset);
        if (pTag == NULL)
        {
            nOffset -= 4;
            break;
        }
        else {
            _cb(pTag);
        }

        nOffset += (11 + pTag->_header.nDataSize);
        DestroyTag(pTag);
    }

    nUsedLen = nOffset;
    return 0;
}

CFlvParser::FlvHeader *CFlvParser::CreateFlvHeader(unsigned char *pBuf)
{
    FlvHeader *pHeader = new FlvHeader;
    pHeader->nVersion = pBuf[3];
    pHeader->bHaveAudio = (pBuf[4] >> 2) & 0x01;
    pHeader->bHaveVideo = (pBuf[4] >> 0) & 0x01;
    pHeader->nHeadSize = ShowU32(pBuf + 5);

    pHeader->pFlvHeader = new unsigned char[pHeader->nHeadSize];
    memcpy(pHeader->pFlvHeader, pBuf, pHeader->nHeadSize);

    return pHeader;
}

int CFlvParser::DestroyFlvHeader(FlvHeader *pHeader)
{
    if (pHeader == NULL)
        return 0;

    delete pHeader->pFlvHeader;
    return 1;
}

void CFlvParser::Tag::Init(TagHeader *pHeader, unsigned char *pBuf, int nLeftLen)
{
    memcpy(&_header, pHeader, sizeof(TagHeader));

    _pTagHeader = new unsigned char[11];
    memcpy(_pTagHeader, pBuf, 11);

    _pTagData = new unsigned char[_header.nDataSize];
    memcpy(_pTagData, pBuf + 11, _header.nDataSize);

}

CFlvParser::CVideoTag::CVideoTag(TagHeader *pHeader, unsigned char *pBuf, int nLeftLen, CFlvParser *pParser)
{
    _sps = NULL;
    _pps = NULL;
    Init(pHeader, pBuf, nLeftLen);

    unsigned char *pd = _pTagData;
    _nFrameType = (pd[0] & 0xf0) >> 4;
    _nCodecID = pd[0] & 0x0f;
    if (_header.nType == 0x09 && _nCodecID == 7)
    {
        ParseH264Tag(pParser);
    }
}

CFlvParser::CAudioTag::CAudioTag(TagHeader *pHeader, unsigned char *pBuf, int nLeftLen, CFlvParser *pParser)
{
    _bIsFirst = 0;
    Init(pHeader, pBuf, nLeftLen);

    unsigned char *pd = _pTagData;
    _nSoundFormat = (pd[0] & 0xf0) >> 4;
    _nSoundRate = (pd[0] & 0x0c) >> 2;
    _nSoundSize = (pd[0] & 0x02) >> 1;
    _nSoundType = (pd[0] & 0x01);
    if (_nSoundFormat == 10) // AAC
    {
        ParseAACTag(pParser);
    }
}

int CFlvParser::CAudioTag::ParseAACTag(CFlvParser *pParser)
{
    unsigned char *pd = _pTagData;
    int nAACPacketType = pd[1];

    if (nAACPacketType == 0)
    {
        _bIsFirst = 1;
        ParseAudioSpecificConfig(pParser, pd);
    }
    else if (nAACPacketType == 1)
    {
        ParseRawAAC(pParser, pd);
    }
    else
    {

    }

    return 1;
}

int CFlvParser::CAudioTag::ParseAudioSpecificConfig(CFlvParser *pParser, unsigned char *pTagData)
{
    unsigned char *pd = _pTagData;

    _aacProfile = ((pd[2]&0xf8)>>3) - 1;
    _sampleRateIndex = ((pd[2]&0x07)<<1) | (pd[3]>>7);
    _channelConfig = (pd[3]>>3) & 0x0f;

    _pMedia = NULL;
    _nMediaLen = 0;

    return 1;
}

int CFlvParser::CAudioTag::ParseRawAAC(CFlvParser *pParser, unsigned char *pTagData)
{
    uint64_t bits = 0;
    int dataSize = _header.nDataSize - 2;

    WriteU64(bits, 12, 0xFFF);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 2, 0);
    WriteU64(bits, 1, 1);
    WriteU64(bits, 2, _aacProfile);
    WriteU64(bits, 4, _sampleRateIndex);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 3, _channelConfig);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 13, 7 + dataSize);
    WriteU64(bits, 11, 0x7FF);
    WriteU64(bits, 2, 0);

    _nMediaLen = 7 + dataSize;
    _pMedia = new unsigned char[_nMediaLen];
    unsigned char p64[8];
    p64[0] = (unsigned char)(bits >> 56);
    p64[1] = (unsigned char)(bits >> 48);
    p64[2] = (unsigned char)(bits >> 40);
    p64[3] = (unsigned char)(bits >> 32);
    p64[4] = (unsigned char)(bits >> 24);
    p64[5] = (unsigned char)(bits >> 16);
    p64[6] = (unsigned char)(bits >> 8);
    p64[7] = (unsigned char)(bits);

    memcpy(_pMedia, p64+1, 7);
    memcpy(_pMedia + 7, pTagData + 2, dataSize);

    return 1;
}

CFlvParser::Tag *CFlvParser::CreateTag(unsigned char *pBuf, int nLeftLen)
{
    TagHeader header;
    header.nType = ShowU8(pBuf+0);
    header.nDataSize = ShowU24(pBuf + 1);
    header.nTimeStamp = ShowU24(pBuf + 4);
    header.nTSEx = ShowU8(pBuf + 7);
    header.nStreamID = ShowU24(pBuf + 8);
    header.nTotalTS = (unsigned int)((header.nTSEx << 24)) + header.nTimeStamp;
    //cout << "total TS : " << header.nTotalTS << endl;
    //cout << "nLeftLen : " << nLeftLen << " , nDataSize : " << pTag->header.nDataSize << endl;
    if ((header.nDataSize + 11) > nLeftLen)
    {
        return NULL;
    }

    Tag *pTag;
    switch (header.nType) {
    case 0x09:
        pTag = new CVideoTag(&header, pBuf, nLeftLen, this);
        break;
    case 0x08:
        pTag = new CAudioTag(&header, pBuf, nLeftLen, this);
        break;
    default:
        pTag = new Tag();
        pTag->Init(&header, pBuf, nLeftLen);
    }
    
    return pTag;
}

int CFlvParser::DestroyTag(Tag *pTag)
{
    if (pTag->_pMedia != NULL)
        delete []pTag->_pMedia;
    if (pTag->_pTagData!=NULL)
        delete []pTag->_pTagData;
    if (pTag->_pTagHeader != NULL)
        delete []pTag->_pTagHeader;

    return 1;
}

int CFlvParser::CVideoTag::ParseH264Tag(CFlvParser *pParser)
{
    unsigned char *pd = _pTagData;
    int nAVCPacketType = pd[1];
    int nCompositionTime = CFlvParser::ShowU24(pd + 2);

    if (nAVCPacketType == 0)
    {
        ParseH264Configuration(pParser, pd);
    }
    else if (nAVCPacketType == 1)
    {
        ParseNalu(pParser, pd);
    }
    else
    {

    }
    return 1;
}

int CFlvParser::CVideoTag::ParseH264Configuration(CFlvParser *pParser, unsigned char *pTagData)
{
    unsigned char *pd = pTagData;

    pParser->_nNalUnitLength = (pd[9] & 0x03) + 1;

    int sps_size, pps_size;
    sps_size = CFlvParser::ShowU16(pd + 11);
    pps_size = CFlvParser::ShowU16(pd + 11 + (2 + sps_size) + 1);
    
    _nMediaLen = 4 + sps_size + 4 + pps_size;
    _pMedia = new unsigned char[_nMediaLen];
    memcpy(_pMedia, &nH264StartCode, 4);
    memcpy(_pMedia + 4, pd + 11 + 2, sps_size);

  _sps = new unsigned char[sps_size];
  memcpy(_sps, pd + 11 + 2, sps_size);
  _sps_size = sps_size;

    memcpy(_pMedia + 4 + sps_size, &nH264StartCode, 4);
  memcpy(_pMedia + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);

  _pps = new unsigned char[sps_size];
  memcpy(_pps, pd + 11 + 2 + sps_size + 2 + 1, sps_size);
  _pps_size = pps_size;

    return 1;
}

int CFlvParser::CVideoTag::ParseNalu(CFlvParser *pParser, unsigned char *pTagData)
{
    unsigned char *pd = pTagData;
    int nOffset = 0;

    _pMedia = new unsigned char[_header.nDataSize+10];
    _nMediaLen = 0;

    nOffset = 5;
    while (1)
    {
        if (nOffset >= _header.nDataSize)
            break;

        int nNaluLen;
        switch (pParser->_nNalUnitLength)
        {
        case 4:
            nNaluLen = CFlvParser::ShowU32(pd + nOffset);
            break;
        case 3:
            nNaluLen = CFlvParser::ShowU24(pd + nOffset);
            break;
        case 2:
            nNaluLen = CFlvParser::ShowU16(pd + nOffset);
            break;
        default:
            nNaluLen = CFlvParser::ShowU8(pd + nOffset);
        }
        memcpy(_pMedia + _nMediaLen, &nH264StartCode, 4);
        memcpy(_pMedia + _nMediaLen + 4, pd + nOffset + pParser->_nNalUnitLength, nNaluLen);
        _nMediaLen += (4 + nNaluLen);
        nOffset += (pParser->_nNalUnitLength + nNaluLen);
    }

    return 1;
}
