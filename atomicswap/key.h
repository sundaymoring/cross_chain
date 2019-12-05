#include <openssl/ec.h>
#include "hash.h"

#define NID_secp256k1	714

template<typename T>
string HexStr(const T itbegin, const T itend, bool fSpaces=false)
{
    std::string rv;
    static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    rv.reserve((itend-itbegin)*3);
    for(T it = itbegin; it < itend; ++it)
    {
        unsigned char val = (unsigned char)(*it);
        if(fSpaces && it != itbegin)
            rv.push_back(' ');
        rv.push_back(hexmap[val>>4]);
        rv.push_back(hexmap[val&15]);
    }

    return rv;
}

template<typename T>
inline string HexStr(const T& vch, bool fSpaces=false)
{
    return HexStr(vch.begin(), vch.end(), fSpaces);
}

class CKeyID : public uint160
{
public:
    CKeyID() : uint160(0) { }
    CKeyID(const uint160 &in) : uint160(in) { }
};

class CScriptID : public uint160
{
public:
    CScriptID() : uint160(0) { }
    CScriptID(const uint160 &in) : uint160(in) { }
};

class CPubKey {
private:
    unsigned char vch[65];

	unsigned int static GetLen(unsigned char chHeader) {
        if (chHeader == 2 || chHeader == 3)
            return 33;
        if (chHeader == 4 || chHeader == 6 || chHeader == 7)
            return 65;
        return 0;
    }
	
	void Invalidate() {
        vch[0] = 0xFF;
    }

public:
	CPubKey() {
        Invalidate();
    }

	template<typename T>
    void Set(const T pbegin, const T pend) {
        int len = pend == pbegin ? 0 : GetLen(pbegin[0]);
        if (len && len == (pend-pbegin))
            memcpy(vch, (unsigned char*)&pbegin[0], len);
        else
            Invalidate();
    }

	unsigned int size() const { return GetLen(vch[0]); }
	const unsigned char *begin() const { return vch; }
    const unsigned char *end() const { return vch+size(); }

	CKeyID GetID() const {
        return CKeyID(Hash160(vch, vch+size()));
    }
};

class CECKey {
private:
    EC_KEY *pkey;

public:
	CECKey() {
        pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
        assert(pkey != NULL);
    }

    ~CECKey() {
        EC_KEY_free(pkey);
    }

	void SetSecretBytes(const unsigned char vch[32]);
	void GetPubKey(CPubKey &pubkey, bool fCompressed);
	bool Sign(const uint256 &hash, std::vector<unsigned char>& vchSig);
};

class CKey {
private:
	bool fValid;
	bool fCompressed;
	unsigned char vch[32];

	bool static Check(const unsigned char *vch);

public:
	void MakeNewKey(bool fCompressed);

	template<typename T>
    void Set(const T pbegin, const T pend, bool fCompressedIn) {
        if (pend - pbegin != 32) {
            fValid = false;
            return;
        }
        if (Check(&pbegin[0])) {
            memcpy(vch, (unsigned char*)&pbegin[0], 32);
            fValid = true;
            fCompressed = fCompressedIn;
        } else {
            fValid = false;
        }
    }

	CPubKey GetPubKey() const;

	bool IsValid() const { return fValid; }
	bool IsCompressed() const { return fCompressed; }

	unsigned int size() const { return (fValid ? 32 : 0); }
    const unsigned char *begin() const { return vch; }
    const unsigned char *end() const { return vch + size(); }
	bool Sign(const uint256 &hash, vector<unsigned char>& vchSig, int nHashType) const;
};

CPubKey GenerateNewKey(bool fCompressed);
