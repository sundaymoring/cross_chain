#include "params.h"

void GetPrefixes_PubKey_Address(const string &type, vector<unsigned char> &vec)
{
    if(type == "bitcoin" || type == "cfos")
        vec = base58_Bitcoin_Prefixes_PubKey_Address;
    else if(type == "blackcoin")
        vec = base58_Blackcoin_Prefixes_PubKey_Address;
}

void GetPrefixes_Script_Address(const string &type, vector<unsigned char> &vec)
{
    if(type == "bitcoin" || type == "cfos")
        vec = base58_Bitcoin_Prefixes_Script_Address;
    else if(type == "blackcoin")
        vec = base58_Blackcoin_Prefixes_Script_Address;
}

void GetPrefixes_SECRET_KEY(const string &type, vector<unsigned char> &vec)
{
    if(type == "bitcoin" || type == "cfos")
        vec = base58_Bitcoin_Prefixes_SECRET_KEY;
    else if(type == "blackcoin")
        vec = base58_Blackcoin_Prefixes_SECRET_KEY;
}
