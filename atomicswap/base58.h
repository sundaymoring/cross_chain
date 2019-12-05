#include <string>
#include "script.h"
#include "params.h"
#include "allocators.h"

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

inline string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend)
{
    CAutoBN_CTX pctx;
    CBigNum bn58 = 58;
    CBigNum bn0 = 0;

    // Convert big endian data to little endian
    // Extra zero at the end make sure bignum will interpret as a positive number
    vector<unsigned char> vchTmp(pend-pbegin+1, 0);
    reverse_copy(pbegin, pend, vchTmp.begin());

    // Convert little endian data to bignum
    CBigNum bn;
    bn.setvch(vchTmp);

    // Convert bignum to std::string
    string str;
    // Expected size increase from base58 conversion is approximately 137%
    // use 138% to be safe
    str.reserve((pend - pbegin) * 138 / 100 + 1);
    CBigNum dv;
    CBigNum rem;
    while (bn > bn0)
    {
        if (!BN_div(&dv, &rem, &bn, &bn58, pctx))
            throw bignum_error("EncodeBase58 : BN_div failed");
        bn = dv;
        unsigned int c = rem.getulong();
        str += pszBase58[c];
    }

    // Leading zeroes encoded as base58 zeros
    for (const unsigned char* p = pbegin; p < pend && *p == 0; p++)
        str += pszBase58[0];

    // Convert little endian std::string to big endian
    reverse(str.begin(), str.end());
    return str;
}

inline bool DecodeBase58(const char* psz, vector<unsigned char>& vchRet)
{
    CAutoBN_CTX pctx;
    vchRet.clear();
    CBigNum bn58 = 58;
    CBigNum bn = 0;
    CBigNum bnChar;
    while (isspace(*psz))
        psz++;

    // Convert big endian string to bignum
    for (const char* p = psz; *p; p++)
    {
        const char* p1 = strchr(pszBase58, *p);
        if (p1 == NULL)
        {
            while (isspace(*p))
                p++;
            if (*p != '\0')
                return false;
            break;
        }
        bnChar.setulong(p1 - pszBase58);
        if (!BN_mul(&bn, &bn, &bn58, pctx))
            throw bignum_error("DecodeBase58 : BN_mul failed");
        bn += bnChar;
    }

    // Get bignum as little endian data
    vector<unsigned char> vchTmp = bn.getvch();

    // Trim off sign byte if present
    if (vchTmp.size() >= 2 && vchTmp.end()[-1] == 0 && vchTmp.end()[-2] >= 0x80)
        vchTmp.erase(vchTmp.end()-1);

    // Restore leading zeros
    int nLeadingZeros = 0;
    for (const char* p = psz; *p == pszBase58[0]; p++)
        nLeadingZeros++;
    vchRet.assign(nLeadingZeros + vchTmp.size(), 0);

    // Convert little endian data to big endian
    reverse_copy(vchTmp.begin(), vchTmp.end(), vchRet.end() - vchTmp.size());
    return true;
}

inline bool DecodeBase58(const std::string& str, std::vector<unsigned char>& vchRet)
{
    return DecodeBase58(str.c_str(), vchRet);
}

inline string EncodeBase58Check(const vector<unsigned char>& vchIn)
{
    vector<unsigned char> vch(vchIn);
    uint256 hash = Hash(vch.begin(), vch.end());
    vch.insert(vch.end(), (unsigned char*)&hash, (unsigned char*)&hash + 4);
    return EncodeBase58(&vch[0], &vch[0] + vch.size());
}

inline bool DecodeBase58Check(const char* psz, vector<unsigned char>& vchRet)
{
    if (!DecodeBase58(psz, vchRet))
        return false;
    if (vchRet.size() < 4)
    {
        vchRet.clear();
        return false;
    }
    uint256 hash = Hash(vchRet.begin(), vchRet.end()-4);
    if (memcmp(&hash, &vchRet.end()[-4], 4) != 0)
    {
        vchRet.clear();
        return false;
    }
    vchRet.resize(vchRet.size()-4);
    return true;
}

class CBase58Data
{
public:
    vector<unsigned char> vchVersion;
	// the actually encoded data
    typedef std::vector<unsigned char, zero_after_free_allocator<unsigned char> > vector_uchar;
    vector_uchar vchData;

    CBase58Data()
    {
        vchVersion.clear();
        vchData.clear();
    }

    void SetData(const vector<unsigned char> &vchVersionIn, const void* pdata, size_t nSize)
    {
        vchVersion = vchVersionIn;
        vchData.resize(nSize);
        if (!vchData.empty())
            memcpy(&vchData[0], pdata, nSize);
    }

	bool SetString(const char* psz, unsigned int nVersionBytes = 1)
    {
        std::vector<unsigned char> vchTemp;
        DecodeBase58Check(psz, vchTemp);
        if (vchTemp.size() < nVersionBytes)
        {
            vchData.clear();
            vchVersion.clear();
            return false;
        }
        vchVersion.assign(vchTemp.begin(), vchTemp.begin() + nVersionBytes);
        vchData.resize(vchTemp.size() - nVersionBytes);
        if (!vchData.empty())
            memcpy(&vchData[0], &vchTemp[nVersionBytes], vchData.size());
        OPENSSL_cleanse(&vchTemp[0], vchData.size());
        return true;
    }

	int CompareTo(const CBase58Data& b58) const
    {
        if (vchVersion < b58.vchVersion) return -1;
        if (vchVersion > b58.vchVersion) return  1;
        if (vchData < b58.vchData)   return -1;
        if (vchData > b58.vchData)   return  1;
        return 0;
    }

    bool operator==(const CBase58Data& b58) const { return CompareTo(b58) == 0; }
    bool operator<=(const CBase58Data& b58) const { return CompareTo(b58) <= 0; }
    bool operator>=(const CBase58Data& b58) const { return CompareTo(b58) >= 0; }
    bool operator< (const CBase58Data& b58) const { return CompareTo(b58) <  0; }
    bool operator> (const CBase58Data& b58) const { return CompareTo(b58) >  0; }
};

class CBitcoinAddress : public CBase58Data
{
public:
    CBitcoinAddress()
    {
    }
	
	bool SetString(const char* psz, unsigned int nVersionBytes = 1)
    {
        std::vector<unsigned char> vchTemp;
        DecodeBase58Check(psz, vchTemp);
        if (vchTemp.size() < nVersionBytes)
        {
            vchData.clear();
            vchVersion.clear();
            return false;
        }
        vchVersion.assign(vchTemp.begin(), vchTemp.begin() + nVersionBytes);
        vchData.resize(vchTemp.size() - nVersionBytes);
        if (!vchData.empty())
            memcpy(&vchData[0], &vchTemp[nVersionBytes], vchData.size());
        OPENSSL_cleanse(&vchTemp[0], vchData.size());
        return true;
    }

	bool SetString(const string& str)
    {
        return SetString(str.c_str());
    }

	CBitcoinAddress(const string& strAddress)
    {
        SetString(strAddress);
    }

    bool Set(const vector<unsigned char> &prefixes, const CScriptID &scriptid) {
        SetData(prefixes, &scriptid, 20);
        return true;
    }

	bool Set(const vector<unsigned char> &prefixes, const CKeyID &keyid) {
        SetData(prefixes, &keyid, 20);
        return true;
    }

    string ToString() const
    {
        vector<unsigned char> vch = vchVersion;
        vch.insert(vch.end(), vchData.begin(), vchData.end());
        return EncodeBase58Check(vch);
    }

	bool IsValid(const string &type) const
    {
		vector<unsigned char> base58Prefixes_PubKey_Address;
		vector<unsigned char> base58Prefixes_Script_Address;
		GetPrefixes_PubKey_Address(type, base58Prefixes_PubKey_Address);
		GetPrefixes_Script_Address(type, base58Prefixes_Script_Address);
		
        bool fCorrectSize = vchData.size() == 20;
        bool fKnownVersion = vchVersion == base58Prefixes_PubKey_Address ||
                             vchVersion == base58Prefixes_Script_Address;
		return fCorrectSize && fKnownVersion;
    }

	bool GetKeyID(CKeyID &keyID, const string &type) const {
		vector<unsigned char> base58Prefixes_PubKey_Address;
		GetPrefixes_PubKey_Address(type, base58Prefixes_PubKey_Address);
        if (!IsValid(type) || vchVersion != base58Prefixes_PubKey_Address)
            return false;
		uint160 id;
        memcpy(&id, &vchData[0], 20);
        keyID = CKeyID(id);
        return true;
    }

	bool GetScriptID(CScriptID &scriptID, const string &type) const {
		vector<unsigned char> base58Prefixes_Script_Address;
		GetPrefixes_Script_Address(type, base58Prefixes_Script_Address);
        if (!IsValid(type) || vchVersion != base58Prefixes_Script_Address)
            return false;
        uint160 id;
        memcpy(&id, &vchData[0], 20);
        scriptID = CScriptID(id);
        return true;
    }
};

class CBitcoinSecret : public CBase58Data
{
public:
    void SetKey(const CKey& vchSecret, const string &type)
    {
		vector<unsigned char> base58Prefixes_SECRET_KEY;
		GetPrefixes_SECRET_KEY(type, base58Prefixes_SECRET_KEY);
        assert(vchSecret.IsValid());
        SetData(base58Prefixes_SECRET_KEY, vchSecret.begin(), vchSecret.size());
        if (vchSecret.IsCompressed())
            vchData.push_back(1);
    }

    CKey GetKey()
    {
        CKey ret;
        ret.Set(&vchData[0], &vchData[32], vchData.size() > 32 && vchData[32] == 1);
        return ret;
    }

    bool IsValid(const string &type) const
    {
		vector<unsigned char> base58Prefixes_SECRET_KEY;
		GetPrefixes_SECRET_KEY(type, base58Prefixes_SECRET_KEY);
        bool fExpectedFormat = vchData.size() == 32 || (vchData.size() == 33 && vchData[32] == 1);
        bool fCorrectVersion = vchVersion == base58Prefixes_SECRET_KEY;
        return fExpectedFormat && fCorrectVersion;
    }

    bool SetString(const char* pszSecret, const string &type)
    {
        return CBase58Data::SetString(pszSecret) && IsValid(type);
    }

    bool SetString(const std::string& strSecret, const string &type)
    {
        return SetString(strSecret.c_str(), type);
    }

    CBitcoinSecret(const CKey& vchSecret, const string &type)
    {
        SetKey(vchSecret, type);
    }

    CBitcoinSecret()
    {
    }
};
