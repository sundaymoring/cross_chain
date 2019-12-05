#include <vector>
#include <string>
using namespace std;

const vector<unsigned char> base58_Bitcoin_Prefixes_PubKey_Address = vector<unsigned char>(1, 0);
const vector<unsigned char> base58_Bitcoin_Prefixes_Script_Address = vector<unsigned char>(1, 5);
const vector<unsigned char> base58_Bitcoin_Prefixes_SECRET_KEY = vector<unsigned char>(1, 128);

const vector<unsigned char> base58_Blackcoin_Prefixes_PubKey_Address = vector<unsigned char>(1, 25);
const vector<unsigned char> base58_Blackcoin_Prefixes_Script_Address = vector<unsigned char>(1, 85);
const vector<unsigned char> base58_Blackcoin_Prefixes_SECRET_KEY = vector<unsigned char>(1, 153);

void GetPrefixes_PubKey_Address(const string &type, vector<unsigned char> &vec);

void GetPrefixes_Script_Address(const string &type, vector<unsigned char> &vec);

void GetPrefixes_SECRET_KEY(const string &type, vector<unsigned char> &vec);
