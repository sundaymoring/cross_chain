#include "base58.h"
#include <string>
#include <ctime>
#include <json/json.h>

using namespace std;

//48 * 60 * 60
#define INITIATE_LOCKTIME 172800
//24 * 60 * 60
#define PARTICIPATE_LOCKTIME 86400

struct AtomicSwapData
{
    vector<unsigned char> secHash;
    vector<unsigned char> recipientPubKeyHash;
    vector<unsigned char> refundPubKeyHash;
    vector<unsigned char> lockTime;
};

bool Initiate(const string &fromAddress, const string &themAddressStr, const string &feeAddress, const string &feeChangeAddress,
			const string &assetChangeAddress, const string &cfosType, const string &refundAddressStr, double amount, Json::Value &response,
			const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword);
bool Participate(const string &fromAddress, const string &themAddressStr, const string &feeAddress, const string &feeChangeAddress,
			const string &assetChangeAddress, const string &cfosType, const string &refundAddressStr, double amount, const string &secHash,
			Json::Value &response, const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword);
bool Redeem(const string &contract, const string &txid, const string &secret, const string &redeemAddressStr, Json::Value &response,
			const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword);
bool Refund(const string &contract, const string &txid, const string &refundAddressStr, Json::Value &response,
            const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword);
/*bool RedeemAcoin(const string &contract, const string &txid, const string &secret, const string &redeemAddress, Json::Value &response);
bool RedeemBcoin(const string &contract, const string &txid, const string &secret, const string &redeemAddress, Json::Value &response);*/
void AtomicSwapContract(const CKeyID &meKeyID, const CKeyID &themKeyID, time_t lockTime, const vector<unsigned char> &secHash, CScript &contract);
bool ExtractSecret(const string &txid, const string &secHashStr, Json::Value &response, const string &coinType,
			const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword);
bool LoadSettings();

//CBitcoinAddress GetContractAddress(CScript contract);
void ExtractAtomicSwapData(const CScript &contract, AtomicSwapData &data);

class AtomRpc
{
  public:
	bool AtomGetInfo(const Json::Value& root, Json::Value& response);

	bool AtomInitiate(const Json::Value& root, Json::Value& response);

	bool AtomInitiateCfos(const Json::Value& root, Json::Value& response);

	bool AtomParticipate(const Json::Value& root, Json::Value& response);

	bool AtomParticipateCfos(const Json::Value& root, Json::Value& response);

	bool AtomRedeemCoin(const Json::Value& root, Json::Value& response);

	bool AtomRefundCoin(const Json::Value& root, Json::Value& response);

	bool AtomExtractSecret(const Json::Value& root, Json::Value& response);
};
