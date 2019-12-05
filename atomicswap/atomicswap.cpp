#include "atomicswap.h"
#include "rpccall.h"
#include "transaction.h"
#include <boost/algorithm/string/split.hpp>
#include <openssl/rand.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

string CFOS_FEE_SYM = "ABC";

RPC_conf InitiatorACoin;
RPC_conf ParticipantACoin;
RPC_conf InitiatorBCoin;
RPC_conf ParticipantBCoin;

bool NoRpcError(const Object &object)
{
    Value error = object.at("error");
    if(!error.isNull())
        return AtoError(object.at("error").getObject().at("message").getString().c_str());
    return true;
}

/**
 * Initiate.
 * @param fromAddress   Cfos asset from address.
 * @param themAddressStr   Send to address.
 * @param feeAddress   Cfos fee address.
 * @param feeChangeAddress   Cfos fee change address.
 * @param assetChangeAddress   Cfos asset change address.
 * @param cfosType   Cfos coin type.
 * @param refundAddressStr   Refund address after locktime.
 * @param amount   Send amount.
 * @param response   Rpc response.
 * @param coinType   Blackcoin,Bitcoin or Cfos.
 * @param rpcAddress   Rpc ip.
 * @param rpcPort   Rpc port.
 * @param rpcUserName   Rpc user name.
 * @param rpcPassword   Rpc password.
 * @returns
 */
bool Initiate(const string &fromAddress, const string &themAddressStr, const string &feeAddress, const string &feeChangeAddress,
            const string &assetChangeAddress, const string &cfosType, const string &refundAddressStr, double amount, Json::Value &response,
            const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword)
{
	CBitcoinAddress themAddress(themAddressStr);
	CBitcoinAddress refundAddress(refundAddressStr);
	RPC_conf nowRpc;
    nowRpc.type = coinType;
    nowRpc.address = rpcAddress;
    nowRpc.port = rpcPort;
    nowRpc.rpcuser = rpcUserName;
    nowRpc.rpcpassword = rpcPassword;

	//Generate secret
	unsigned char secret[32];
    RAND_bytes(secret, 32);
    vector<unsigned char> secvec(secret, secret + 32);
    cout << "Secret:" << endl << HexStr(secvec) << endl;
	response["Secret"] = HexStr(secvec);
    //Get secret RIPEMD160 hash
    unsigned char secHash[20];
    RIPEMD160(secret, 32, secHash);
    vector<unsigned char> secHashVec(secHash, secHash + 20);
    cout << "Secret hash:" << endl << HexStr(secHashVec) << endl;
	response["Secret hash"] = HexStr(secHashVec);

	//Get locktime
    time_t lockTime = time(NULL) + INITIATE_LOCKTIME;
	cout << "Locktime:" << endl << lockTime << endl;

	cout << "Their address:" << endl << themAddress.ToString() << endl;	
	//Get their KeyID
	CKeyID themKeyID;
    themAddress.GetKeyID(themKeyID, coinType);

    cout << "Refund address:" << endl <<  refundAddress.ToString() << endl;
	response["Refund address"] = refundAddress.ToString();
	//Get refund KeyID
	CKeyID refundKeyID;
	refundAddress.GetKeyID(refundKeyID, coinType);

	//Build contract script
	CScript contract;
	AtomicSwapContract(refundKeyID, themKeyID, lockTime, secHashVec, contract);
	cout << "Contract:" << endl << contract.ToString() << endl;
	cout << "Contract hex:" << endl << HexStr(contract) << endl;
	response["Contract hex"] = HexStr(contract);
	//Get contract address
	CScriptID contractID = contract.GetID();
    CBitcoinAddress contractAddress;
    vector<unsigned char> base58Prefixes_Script_Address;
    GetPrefixes_Script_Address(coinType, base58Prefixes_Script_Address);
	contractAddress.Set(base58Prefixes_Script_Address, contractID);
    string contractAddressStr = contractAddress.ToString();
	cout << "Contract address:" << endl << contractAddressStr << endl;
	response["Contract address"] = contractAddressStr;

	//RPC:sendtoaddress
    Array params;	
    Object reply;
	string error;
	if(coinType == "cfos")
	{
		params.push_back(fromAddress);
		params.push_back(contractAddressStr);
        params.push_back(amount);
		params.push_back(feeAddress);
		params.push_back(feeChangeAddress);
		params.push_back(assetChangeAddress);
		if(!CallRPC("sendassettoaddress", params, nowRpc, reply, error))
            return AtoError(error.c_str());
	}
	else
	{
		params.push_back(contractAddressStr);
    	params.push_back(amount);
		if(!CallRPC("sendtoaddress", params, nowRpc, reply, error))
			return AtoError(error.c_str());
	}

	cout << write_string(Value(reply), false) + '\n';
	if(!NoRpcError(reply))
		return false;
	if(coinType == "cfos")
		response["txid"] = reply.at("result").getObject().at("txid").getString();
	else
		response["txid"] = reply.at("result").getString();

	return true;
}

/**
 * Participate.
 * @param fromAddress   Cfos asset from address.
 * @param themAddressStr   Send to address.
 * @param feeAddress   Cfos fee address.
 * @param feeChangeAddress   Cfos fee change address.
 * @param assetChangeAddress   Cfos asset change address.
 * @param cfosType   Cfos coin type.
 * @param refundAddressStr   Refund address after locktime.
 * @param amount   Send amount.
 * @param secHash   The secret hash when initiate.
 * @param response   Rpc response.
 * @param coinType   Blackcoin,Bitcoin or Cfos.
 * @param rpcAddress   Rpc ip.
 * @param rpcPort   Rpc port.
 * @param rpcUserName   Rpc user name.
 * @param rpcPassword   Rpc password.
 * @returns
 */
bool Participate(const string &fromAddress, const string &themAddressStr, const string &feeAddress, const string &feeChangeAddress,
			const string &assetChangeAddress, const string &cfosType, const string &refundAddressStr, double amount, const string &secHash,
			Json::Value &response, const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword)
{
	CBitcoinAddress themAddress(themAddressStr);
    CBitcoinAddress refundAddress(refundAddressStr);
	vector<unsigned char> secHashVec = ParseHex(secHash);
	RPC_conf nowRpc;
    nowRpc.type = coinType;
    nowRpc.address = rpcAddress;
    nowRpc.port = rpcPort;
    nowRpc.rpcuser = rpcUserName;
    nowRpc.rpcpassword = rpcPassword;

	//Get locktime
    time_t lockTime = time(NULL) + PARTICIPATE_LOCKTIME;
    cout << "Locktime:" << endl << lockTime << endl;

	cout << "Their address:" << endl << themAddress.ToString() << endl;
    //Get their KeyID
    CKeyID themKeyID;
    themAddress.GetKeyID(themKeyID, coinType);

    cout << "Refund address:" << endl <<  refundAddress.ToString() << endl;
	response["Refund address"] = refundAddress.ToString();

	//Get refund KeyID
    CKeyID refundKeyID;
    refundAddress.GetKeyID(refundKeyID, coinType);

    //Build contract script
    CScript contract;
    AtomicSwapContract(refundKeyID, themKeyID, lockTime, secHashVec, contract);
    cout << "Contract:" << endl << contract.ToString() << endl;
    cout << "Contract hex:" << endl << HexStr(contract) << endl;
	response["Contract hex"] = HexStr(contract);

    //Get contract address
    CScriptID contractID = contract.GetID();
    CBitcoinAddress contractAddress;
	vector<unsigned char> base58Prefixes_Script_Address;
	GetPrefixes_Script_Address(coinType, base58Prefixes_Script_Address);
    contractAddress.Set(base58Prefixes_Script_Address, contractID);
    string contractAddressStr = contractAddress.ToString();
    cout << "Contract address:" << endl << contractAddressStr << endl;
	response["Contract address"] = contractAddressStr;

	//RPC:sendtoaddress
    Array params;
    Object reply;
    string error;
    if(coinType == "cfos")
    {
        params.push_back(fromAddress);
        params.push_back(contractAddressStr);
        params.push_back(amount);
        params.push_back(feeAddress);
        params.push_back(feeChangeAddress);
        params.push_back(assetChangeAddress);
        if(!CallRPC("sendassettoaddress", params, nowRpc, reply, error))
            return AtoError(error.c_str());
    }
    else
    {
        params.push_back(contractAddressStr);
        params.push_back(amount);
        if(!CallRPC("sendtoaddress", params, nowRpc, reply, error))
            return AtoError(error.c_str());
    }

    cout << write_string(Value(reply), false) + '\n';
    if(!NoRpcError(reply))
        return false;
    if(coinType == "cfos")
        response["txid"] = reply.at("result").getObject().at("txid").getString();
    else
        response["txid"] = reply.at("result").getString();

	return true;
}

bool Redeem(const string &contract, const string &txid, const string &secret, const string &redeemAddressStr,
			Json::Value &response, const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword)
{
	vector<unsigned char> contractVec = ParseHex(contract);
	vector<unsigned char> secretVec = ParseHex(secret);
	CScript contractScript(contractVec.begin(), contractVec.end());
	cout << "Contract:" << endl << contractScript.ToString() << endl;
	cout << "Contract hex:" << endl << HexStr(contractScript) << endl;
	CScriptID contractID = contractScript.GetID();
	CBitcoinAddress contractAddress;
	vector<unsigned char> base58Prefixes_Script_Address;
	GetPrefixes_Script_Address(coinType, base58Prefixes_Script_Address);
    contractAddress.Set(base58Prefixes_Script_Address, contractID);
	CBitcoinAddress redeemAddress(redeemAddressStr);
	RPC_conf nowRpc;
    nowRpc.type = coinType;
    nowRpc.address = rpcAddress;
    nowRpc.port = rpcPort;
    nowRpc.rpcuser = rpcUserName;
    nowRpc.rpcpassword = rpcPassword;
	
	int outIndex = -1;
	double amount = 0;
	string outScript;
	
	AtomicSwapData data;	
	if(contractScript.ISAtomicSwap())
	{
		cout << "Contract is atomicswap script." << endl;
		ExtractAtomicSwapData(contractScript, data);
		cout << "Secret hash:" << endl << HexStr(data.secHash) << endl;
		cout << "Their pubkey hash:" << endl << HexStr(data.recipientPubKeyHash) << endl;
		cout << "My pubkey hash:" << endl << HexStr(data.refundPubKeyHash) << endl;
		cout << "Locktime:" << endl << HexStr(data.lockTime) << endl;
	}
	else
	{
		cout << "Contract is not atomicswap script." << endl;
		return AtoError("Contract is not atomicswap script.");
	}

	Array rawtxParams;
    rawtxParams.push_back(txid);
    Object rawtxReply;
	string error1;
	if(!CallRPC("getrawtransaction", rawtxParams, nowRpc, rawtxReply, error1))
		return AtoError(error1.c_str());
	cout << "Raw transaction:" << endl << write_string(Value(rawtxReply), false) + '\n';
    if(!NoRpcError(rawtxReply))
        return false;

	Array txParams;
    txParams.push_back(rawtxReply.at("result").getString());
    Object txReply;
	string error2;
	if(!CallRPC("decoderawtransaction", txParams, nowRpc, txReply, error2))
		return AtoError(error2.c_str());
    cout << "Transaction:" << endl << write_string(Value(txReply), false) + '\n';
	if(!NoRpcError(txReply))
		return false;	
	Value txResult = txReply.at("result");
	Array vouts = txResult.getObject().at("vout").getArray();
	
	for(int i = 0; i < vouts.size(); i++)
	{
		Value scriptPubKey = vouts[i].getObject().at("scriptPubKey");
		double value = vouts[i].getObject().at("value").getReal();
		Value addressV = scriptPubKey.getObject().at("addresses");
		string scriptHex = scriptPubKey.getObject().at("hex").getString();
		Array addressArray = addressV.getArray();
		if(addressArray.size() != 1) continue;
		string addressStr = addressArray[0].getString();
		CBitcoinAddress address(addressStr);
		vector<unsigned char> scriptVec = ParseHex(scriptPubKey.getObject().at("hex").getString());
		CScript script(scriptVec.begin(), scriptVec.end());
		if(script.IsPayToScriptHash() && address == contractAddress)
		{
			outIndex = i;
			amount = value;
			outScript = scriptHex;
			cout << "Find the P2SH vout, index: " << i << " asm: " << scriptPubKey.getObject().at("asm").getString() << endl;
			break;
		}
	}
	
	if(outIndex == -1)
		return AtoError("Can not find P2SH out when redeem.");

	//Create redeem transaction
	CTransaction redeemTransaction(coinType);
	CKeyID redeemKeyID;
	int64_t fee = 0;
	if(coinType == "bitcoin")
		fee = BITCOIN_MIN_TX_FEE;
	else if(coinType == "blackcoin")
		fee = BLACKCOIN_MIN_TX_FEE;
	if(!redeemAddress.GetKeyID(redeemKeyID, coinType))
		return AtoError("Get redeem key error when redeem.");
	CScript scriptPubKey;
	CScript scriptSig;
	uint256 txHash(txid);
	scriptPubKey << OP_DUP << OP_HASH160 << redeemKeyID << OP_EQUALVERIFY << OP_CHECKSIG;
	if(coinType == "cfos")
		redeemTransaction.vout.push_back(CTxOut(amount * CFOSCOIN, scriptPubKey, coinType));
	else
		redeemTransaction.vout.push_back(CTxOut(amount * COIN - fee, scriptPubKey, coinType));
	redeemTransaction.vin.push_back(CTxIn(txHash, outIndex, scriptSig, numeric_limits<unsigned int>::max()-1));

	//Create recipient address
	CBitcoinAddress recipientAddr;
    uint160 id;
    memcpy(&id, &data.recipientPubKeyHash[0], 20);
	vector<unsigned char> base58Prefixes_PubKey_Address;
	GetPrefixes_PubKey_Address(coinType, base58Prefixes_PubKey_Address);
    recipientAddr.Set(base58Prefixes_PubKey_Address, CKeyID(id));
	//Get recipient address private key
    Array dumpParams;
    string recipientAddressStr = recipientAddr.ToString();
    dumpParams.push_back(recipientAddressStr);
    Object dumpReply;
	string error3;
	if(!CallRPC("dumpprivkey", dumpParams, nowRpc, dumpReply, error3))
		return AtoError(error3.c_str());
    if(!NoRpcError(dumpReply))
        return false;
    string recipientPrikeyStr = dumpReply.at("result").getString();
	
	//Sign the hash use recipient private key
	uint256 signHash = SignatureHash(contractScript, redeemTransaction, 0, SIGHASH_ALL);
	CBitcoinSecret vchSecret;
    if(!vchSecret.SetString(recipientPrikeyStr, coinType))
		return AtoError("Set recipient private key error when redeem.");
	CKey recipientKey = vchSecret.GetKey();
	CPubKey recipientPubKey = recipientKey.GetPubKey();
	vector<unsigned char> sig;
	if(!recipientKey.Sign(signHash, sig, SIGHASH_ALL))
		return AtoError("Sign error when redeem.");
	CScript fullSig;
	fullSig << sig;
	fullSig << recipientPubKey;
	fullSig << secretVec;
	fullSig << OP_TRUE;
	fullSig << contractVec;
	redeemTransaction.vin[0].scriptSig = fullSig;

	if(coinType == "cfos")
	{
		Array feeParams;
		feeParams.push_back(CFOS_FEE_SYM);
		Object feeReply;
		string errorFee;
		int feeIndex = -1;
		if(!CallRPC("listunspent", feeParams, nowRpc, feeReply, errorFee))
			return AtoError(errorFee.c_str());
		cout << "Send raw transaction reply:" << endl << write_string(Value(feeReply), false) + '\n';
		Array unspents = feeReply.at("result").getArray();
		for(int i = 0; i < unspents.size(); i++)
		{
			double amount = unspents[i].getObject().at("amount").getReal();
			if(amount >= BLACKCOIN_MIN_TX_FEE / COIN)
			{
				feeIndex = i;
				break;
			}
			
		}
		if(feeIndex == -1)
			return AtoError("Not enough Fee.");
		
		uint256 feeTxHash(unspents[feeIndex].getObject().at("txid").getString());
		CScript feeScriptSig;
		CScript feeScriptPubKey;
		string scriptPubKeyStr = unspents[feeIndex].getObject().at("scriptPubKey").getString();
		CKeyID feeKeyID;
		string feeAddressStr = unspents[feeIndex].getObject().at("address").getString();
		CBitcoinAddress feeAddress(feeAddressStr);
		if(!feeAddress.GetKeyID(feeKeyID, coinType))
        	return AtoError("Get redeem key error when redeem.");
		feeScriptPubKey << OP_DUP << OP_HASH160 << redeemKeyID << OP_EQUALVERIFY << OP_CHECKSIG;
		redeemTransaction.vout.push_back(CTxOut(unspents[feeIndex].getObject().at("amount").getReal() * CFOSCOIN - BLACKCOIN_MIN_TX_FEE, feeScriptPubKey, coinType));
		redeemTransaction.vin.push_back(CTxIn(feeTxHash, feeIndex, feeScriptSig, numeric_limits<unsigned int>::max()-1));

		//Get fee address private key
    	Array dumpParams;
    	dumpParams.push_back(feeAddressStr);
    	Object dumpReply;
    	string errorDum;
    	if(!CallRPC("dumpprivkey", dumpParams, nowRpc, dumpReply, errorDum))
        	return AtoError(errorDum.c_str());
    	if(!NoRpcError(dumpReply))
        	return false;
    	string feePrikeyStr = dumpReply.at("result").getString();

    	//Sign the hash use fee address private key
		vector<unsigned char> scriptVec = ParseHex(scriptPubKeyStr);
    	CScript scriptPubKey(scriptVec.begin(), scriptVec.end());
    	uint256 signHash = SignatureHash(scriptPubKey, redeemTransaction, 1, SIGHASH_ALL);
    	CBitcoinSecret vchSecret;
    	if(!vchSecret.SetString(feePrikeyStr, coinType))
        	return AtoError("Set fee private key error when redeem.");
    	CKey feeKey = vchSecret.GetKey();
    	CPubKey feePubKey = recipientKey.GetPubKey();
    	vector<unsigned char> sig;
    	if(!recipientKey.Sign(signHash, sig, SIGHASH_ALL))
        	return AtoError("Sign error when redeem.");
		CScript feeSig;
    	feeSig << sig;
		redeemTransaction.vin[0].scriptSig = feeSig;
	}

	CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << redeemTransaction;
    string redeemTransactionHex = HexStr(ssTx.begin(), ssTx.end());
    cout << "Redeem transaction:" << endl << redeemTransactionHex << endl;

	//Send redeem raw transaction
    Array rawParams;
    rawParams.push_back(redeemTransactionHex);
	Object sendReply;
	string error4;
	if(!CallRPC("sendrawtransaction", rawParams, nowRpc, sendReply, error4))
		return AtoError(error4.c_str());
    cout << "Send raw transaction reply:" << endl << write_string(Value(sendReply), false) + '\n';
	if(!NoRpcError(sendReply))
        return false;

	response["txid"] = sendReply.at("result").getString();

	return true;
}

bool Refund(const string &contract, const string &txid, const string &refundAddressStr, Json::Value &response,
            const string &coinType, const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword)
{
	vector<unsigned char> contractVec = ParseHex(contract);
	CScript contractScript(contractVec.begin(), contractVec.end());
	cout << "Contract:" << endl << contractScript.ToString() << endl;
	cout << "Contract hex:" << endl << HexStr(contractScript) << endl;
	CScriptID contractID = contractScript.GetID();
	CBitcoinAddress contractAddress;
	vector<unsigned char> base58Prefixes_Script_Address;
	GetPrefixes_Script_Address(coinType, base58Prefixes_Script_Address);
	contractAddress.Set(base58Prefixes_Script_Address, contractID);
	CBitcoinAddress refundAddress(refundAddressStr);
	RPC_conf nowRpc;
	nowRpc.type = coinType;
	nowRpc.address = rpcAddress;
	nowRpc.port = rpcPort;
	nowRpc.rpcuser = rpcUserName;
	nowRpc.rpcpassword = rpcPassword;

	int outIndex = -1;
	double amount = 0;
	string outScript;

	AtomicSwapData data;
	if(contractScript.ISAtomicSwap())
	{
		cout << "Contract is atomicswap script." << endl;
		ExtractAtomicSwapData(contractScript, data);
		cout << "Secret hash:" << endl << HexStr(data.secHash) << endl;
		cout << "Their pubkey hash:" << endl << HexStr(data.recipientPubKeyHash) << endl;
		cout << "My pubkey hash:" << endl << HexStr(data.refundPubKeyHash) << endl;
		cout << "Locktime:" << endl << HexStr(data.lockTime) << endl;
	}
	else
	{
		cout << "Contract is not atomicswap script." << endl;
		return AtoError("Contract is not atomicswap script.");
	}

	Array rawtxParams;
	rawtxParams.push_back(txid);
	Object rawtxReply;
	string error1;
	if(!CallRPC("getrawtransaction", rawtxParams, nowRpc, rawtxReply, error1))
		return AtoError(error1.c_str());
	cout << "Raw transaction:" << endl << write_string(Value(rawtxReply), false) + '\n';
    if(!NoRpcError(rawtxReply))
        return false;

	Array txParams;
	txParams.push_back(rawtxReply.at("result").getString());
	Object txReply;
	string error2;
	if(!CallRPC("decoderawtransaction", txParams, nowRpc, txReply, error2))
		return AtoError(error2.c_str());
    cout << "Transaction:" << endl << write_string(Value(txReply), false) + '\n';
	if(!NoRpcError(txReply))
		return false;	
	Value txResult = txReply.at("result");
	Array vouts = txResult.getObject().at("vout").getArray();
	
	for(int i = 0; i < vouts.size(); i++)
	{
		Value scriptPubKey = vouts[i].getObject().at("scriptPubKey");
		double value = vouts[i].getObject().at("value").getReal();
		Value addressV = scriptPubKey.getObject().at("addresses");
		string scriptHex = scriptPubKey.getObject().at("hex").getString();
		Array addressArray = addressV.getArray();
		if(addressArray.size() != 1) continue;
		string addressStr = addressArray[0].getString();
		CBitcoinAddress address(addressStr);
		vector<unsigned char> scriptVec = ParseHex(scriptPubKey.getObject().at("hex").getString());
		CScript script(scriptVec.begin(), scriptVec.end());
		if(script.IsPayToScriptHash() && address == contractAddress)
		{
			outIndex = i;
			amount = value;
			outScript = scriptHex;
			cout << "Find the P2SH vout, index: " << i << " asm: " << scriptPubKey.getObject().at("asm").getString() << endl;
			break;
		}
	}

	if(outIndex == -1)
		return AtoError("Can not find P2SH out when redeem.");

	//Create redeem transaction
	CTransaction refundTransaction(coinType);
	CKeyID refundKeyID;
	int64_t fee;
	if(coinType == "bitcoin")
		fee = BITCOIN_MIN_TX_FEE;
	else if(coinType == "blackcoin")
		fee = BLACKCOIN_MIN_TX_FEE;
	if(!refundAddress.GetKeyID(refundKeyID, coinType))
		return AtoError("Get redeem key error when redeem.");
	CScript scriptPubKey;
	CScript scriptSig;
	uint256 txHash(txid);
	scriptPubKey << OP_DUP << OP_HASH160 << refundKeyID << OP_EQUALVERIFY << OP_CHECKSIG;
	refundTransaction.vout.push_back(CTxOut(amount * COIN - fee, scriptPubKey, coinType));
	refundTransaction.vin.push_back(CTxIn(txHash, outIndex, scriptSig, numeric_limits<unsigned int>::max()-1));
	refundTransaction.nLockTime = htonl(strtol(HexStr(data.lockTime).c_str(), NULL, 16));

	//Create recipient address
	CBitcoinAddress recipientAddr;
	uint160 id;
	memcpy(&id, &data.refundPubKeyHash[0], 20);
	vector<unsigned char> base58Prefixes_PubKey_Address;
	GetPrefixes_PubKey_Address(coinType, base58Prefixes_PubKey_Address);
	recipientAddr.Set(base58Prefixes_PubKey_Address, CKeyID(id));
	//Get recipient address private key
	Array dumpParams;
	string recipientAddressStr = recipientAddr.ToString();
	dumpParams.push_back(recipientAddressStr);
	Object dumpReply;
	string error3;
	if(!CallRPC("dumpprivkey", dumpParams, nowRpc, dumpReply, error3))
		return AtoError(error3.c_str());
	if(!NoRpcError(dumpReply))
		return false;
	string recipientPrikeyStr = dumpReply.at("result").getString();

	//Sign the hash use recipient private key
	uint256 signHash = SignatureHash(contractScript, refundTransaction, 0, SIGHASH_ALL);
	CBitcoinSecret vchSecret;
	if(!vchSecret.SetString(recipientPrikeyStr, coinType))
		return AtoError("Set recipient private key error when redeem.");
	CKey recipientKey = vchSecret.GetKey();
	CPubKey recipientPubKey = recipientKey.GetPubKey();
	vector<unsigned char> sig;
	if(!recipientKey.Sign(signHash, sig, SIGHASH_ALL))
		return AtoError("Sign error when redeem.");
	CScript fullSig;
	fullSig << sig;
	fullSig << recipientPubKey;
	fullSig << OP_FALSE;
	fullSig << contractVec;
	refundTransaction.vin[0].scriptSig = fullSig;

	CDataStream ssTx(SER_NETWORK, PROTOCOL_VERSION);
    ssTx << refundTransaction;
    string refundTransactionHex = HexStr(ssTx.begin(), ssTx.end());
	cout << "Refund transaction:" << endl << refundTransactionHex << endl;

	//Send redeem raw transaction
    Array rawParams;
    rawParams.push_back(refundTransactionHex);
	Object sendReply;
	string error4;
	if(!CallRPC("sendrawtransaction", rawParams, nowRpc, sendReply, error4))
		return AtoError(error4.c_str());
    cout << "Send raw transaction reply:" << endl << write_string(Value(sendReply), false) + '\n';
	if(!NoRpcError(sendReply))
        return false;

	response["txid"] = sendReply.at("result").getString();

	return true;
}

/*bool RedeemAcoin(const string &contract, const string &txid, const string &secret, const string &redeemAddressStr, Json::Value &response)
{
	return Redeem(contract, txid, secret, redeemAddressStr, ParticipantACoin, response);
}

bool RedeemBcoin(const string &contract, const string &txid, const string &secret, const string &redeemAddressStr, Json::Value &response)
{
    return Redeem(contract, txid, secret, redeemAddressStr, InitiatorBCoin, response);
}*/

void AtomicSwapContract(const CKeyID &meKeyID, const CKeyID &themKeyID, time_t lockTime, const vector<unsigned char> &secHash, CScript &contract)
{
	//Normal redeem path
	contract << OP_IF;
	{
		contract << OP_RIPEMD160;
		contract << secHash;
		contract << OP_EQUALVERIFY;
		contract << OP_DUP;
		contract << OP_HASH160;
		contract << themKeyID;
	}
	//Refund path
	contract << OP_ELSE;
	{
		contract << lockTime;
		contract << OP_CHECKLOCKTIMEVERIFY;
		contract << OP_DROP;
		contract << OP_DUP;
		contract << OP_HASH160;
		contract << meKeyID;
	}
	contract << OP_ENDIF;

	//Complete the signature check
	contract << OP_EQUALVERIFY;
	contract << OP_CHECKSIG;
}

bool ExtractSecret(const string &txid, const string &secHashStr, Json::Value &response, const string &coinType,
			const string &rpcAddress, const string &rpcPort, const string &rpcUserName, const string &rpcPassword)
{
    RPC_conf nowRpc;
    nowRpc.type = coinType;
    nowRpc.address = rpcAddress;
    nowRpc.port = rpcPort;
    nowRpc.rpcuser = rpcUserName;
    nowRpc.rpcpassword = rpcPassword;

	Array rawtxParams;
    rawtxParams.push_back(txid);
    Object rawtxReply;
	string error1;
	if(!CallRPC("getrawtransaction", rawtxParams, nowRpc, rawtxReply, error1))
		return AtoError(error1.c_str());
    cout << "Raw transaction:" << endl << write_string(Value(rawtxReply), false) + '\n';
	if(!NoRpcError(rawtxReply))
        return false;
    Array txParams;
    txParams.push_back(rawtxReply.at("result").getString());
    Object txReply;
	string error2;
	if(!CallRPC("decoderawtransaction", txParams, nowRpc, txReply, error2))
		return AtoError(error2.c_str());
    cout << "Transaction:" << endl << write_string(Value(txReply), false) + '\n';
    if(!NoRpcError(txReply))
        return false;

	Value txResult = txReply.at("result");
    Array vins = txResult.getObject().at("vin").getArray();

	if(vins.size() != 1)
		return AtoError("The transaction vin only allow one input.");

	string sigAsm = vins[0].getObject().at("scriptSig").getObject().at("asm").getString();
	vector<string> vSigs;
    boost::split(vSigs, sigAsm, boost::is_any_of(" "));
	string secStrTmp = vSigs[2];

	//Check secret hash
	vector<unsigned char> secretVecTmp = ParseHex(secStrTmp);
	unsigned char *buffer = new unsigned char[secretVecTmp.size()];
	memcpy(buffer, &secretVecTmp[0], secretVecTmp.size()*sizeof(unsigned char));
	unsigned char secHash[20];
    RIPEMD160(buffer, 32, secHash);
	delete []buffer;
	vector<unsigned char> secHashVec(secHash, secHash + 20);
	if(HexStr(secHashVec) == secHashStr)
		response["secret"] = secStrTmp;
	else
		return AtoError("Secret hash not equal.");
	return true;
}

void ExtractAtomicSwapData(const CScript &contract, AtomicSwapData &data)
{
	data.secHash.assign(contract.begin() + 3, contract.begin() + 23);
	data.recipientPubKeyHash.assign(contract.begin() + 27, contract.begin() + 47);
	data.refundPubKeyHash.assign(contract.begin() + 58, contract.begin() + 78);
	data.lockTime.assign(contract.begin() + 49, contract.begin() + 53);
}

void InitRpc(const Value &val, RPC_conf &rpc)
{
	Object obj = val.getObject();
	rpc.address = obj.at("ip").getString();
    rpc.port = obj.at("rpcport").getString();
    rpc.rpcuser = obj.at("rpcuser").getString();
    rpc.rpcpassword = obj.at("rpcpassword").getString();
}

bool LoadSettings()
{
	string settings;
	string path = "./atom.conf";
	ifstream infile;
	infile.open(path.data());
	if(!infile.is_open())
		return AtoError("The file atom.conf open fail.");
	
	string s;
	while(getline(infile, s))
		settings += s;

	infile.close();
	Value val;
	if(!read_string(settings, val))
		return AtoError("File atom.conf to valur fail.");
	cout << write_string(val, true) + '\n';

	//Init ACoin
	Value acoin = val.getObject().at("acoin");
	Value initiatorACoinVal = acoin.getObject().at("initiator");
	Value participantACoinVal = acoin.getObject().at("participant");
	InitiatorACoin.type = acoin.getObject().at("type").getString();
	InitRpc(initiatorACoinVal, InitiatorACoin);	
	ParticipantACoin.type = acoin.getObject().at("type").getString();
	InitRpc(participantACoinVal, ParticipantACoin);
	
	//Init BCoin
    Value bcoin = val.getObject().at("bcoin");
    Value initiatorBCoinVal = bcoin.getObject().at("initiator");
    Value participantBCoinVal = bcoin.getObject().at("participant");
    InitiatorBCoin.type = bcoin.getObject().at("type").getString();
    InitRpc(initiatorBCoinVal, InitiatorBCoin);
    ParticipantBCoin.type = bcoin.getObject().at("type").getString();
    InitRpc(participantBCoinVal, ParticipantBCoin);

	/*cout << "ACoinInitiator" << endl << ACoinInitiator.type << endl << ACoinInitiator.address << endl << ACoinInitiator.port << endl << ACoinInitiator.rpcuser << endl << ACoinInitiator.rpcpassword << endl;
	cout << "ACoinParticipant" << endl << ACoinParticipant.type << endl << ACoinParticipant.address << endl << ACoinParticipant.port << endl << ACoinParticipant.rpcuser << endl << ACoinParticipant.rpcpassword << endl;
	cout << "BCoinInitiator" << endl << BCoinInitiator.type << endl << BCoinInitiator.address << endl << BCoinInitiator.port << endl << BCoinInitiator.rpcuser << endl << BCoinInitiator.rpcpassword << endl;
	cout << "BCoinParticipant" << endl << BCoinParticipant.type << endl << BCoinParticipant.address << endl << BCoinParticipant.port << endl << BCoinParticipant.rpcuser << endl << BCoinParticipant.rpcpassword << endl;*/

	return true;
}

bool AtomRpc::AtomGetInfo(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
	response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
	response["result"] = "success";
	response["version"] = 0.1;
	return true;
}

bool AtomRpc::AtomInitiate(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
	Json::Value params = root["params"];
	string themAddress = params["themaddress"].asString();
    string refundAddress = params["refundaddress"].asString();
    double amount = params["amount"].asDouble();
	string coinType = params["cointype"].asString();
	string rpcAddress = params["rpcaddress"].asString();
	string rpcPort = params["rpcport"].asString();
	string rpcUserName = params["rpcusername"].asString();
	string rpcPassword = params["rpcpassword"].asString();

	response["jsonrpc"] = "2.0";
	response["id"] = root["id"];
	if(themAddress.empty() || refundAddress.empty() || amount < 0 || coinType.empty() || rpcAddress.empty() ||
		rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
	{
    	response["result"] = "failed";
		cout << "Params wrong." << endl;
		return false;
	}

	if(!Initiate("", themAddress, "", "", "", "", refundAddress, amount, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
	{
		response["result"] = "failed";
		cout << "Initiate failed." << endl;
		return false;
	}

	response["result"] = "success";

	return true;
}

bool AtomRpc::AtomInitiateCfos(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
    Json::Value params = root["params"];
	string fromAddress = params["fromaddress"].asString();
    string themAddress = params["themaddress"].asString();
	string feeAddress = params["feeaddress"].asString();
	string feeChangeAddress = params["feechangeaddress"].asString();
	string assetChangeAddress = params["assetchangeaddress"].asString();
	string cfosType = params["cfostype"].asString();
    string refundAddress = params["refundaddress"].asString();
    double amount = params["amount"].asDouble();
    string coinType = params["cointype"].asString();
    string rpcAddress = params["rpcaddress"].asString();
    string rpcPort = params["rpcport"].asString();
    string rpcUserName = params["rpcusername"].asString();
    string rpcPassword = params["rpcpassword"].asString();

    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if(fromAddress.empty() || themAddress.empty() || feeAddress.empty() || feeChangeAddress.empty() || assetChangeAddress.empty() ||
		cfosType.empty() || refundAddress.empty() || amount < 0 || coinType.empty() || rpcAddress.empty() ||
        rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
    {
        response["result"] = "failed";
        cout << "Params wrong." << endl;
        return false;
    }

    if(!Initiate(fromAddress, themAddress, feeAddress, feeChangeAddress, assetChangeAddress, cfosType,
		refundAddress, amount, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
    {
        response["result"] = "failed";
        cout << "Initiate failed." << endl;
        return false;
    }

    response["result"] = "success";

    return true;
}

bool AtomRpc::AtomParticipate(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
    Json::Value params = root["params"];
    string themAddress = params["themaddress"].asString();
    string refundAddress = params["refundaddress"].asString();
    double amount = params["amount"].asDouble();
	string secretHash = params["secrethash"].asString();
    string coinType = params["cointype"].asString();
    string rpcAddress = params["rpcaddress"].asString();
    string rpcPort = params["rpcport"].asString();
    string rpcUserName = params["rpcusername"].asString();
    string rpcPassword = params["rpcpassword"].asString();

	response["jsonrpc"] = "2.0";
	response["id"] = root["id"];
    if(themAddress.empty() || refundAddress.empty() || amount < 0 || secretHash.empty() || coinType.empty() || rpcAddress.empty() ||
        rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
    {   
        response["result"] = "failed";
		cout << "Params wrong." << endl;
        return false;
    }

    if(!Participate("", themAddress, "", "", "", "", refundAddress, amount, secretHash, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
	{
		response["result"] = "failed";
		cout << "Participate failed." << endl;
		return false;
	}
	
	response["result"] = "success";

    return true;
}

bool AtomRpc::AtomParticipateCfos(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
    Json::Value params = root["params"];
	string fromAddress = params["fromaddress"].asString();
    string themAddress = params["themaddress"].asString();
	string feeAddress = params["feeaddress"].asString();
    string feeChangeAddress = params["feechangeaddress"].asString();
    string assetChangeAddress = params["assetchangeaddress"].asString();
    string cfosType = params["cfostype"].asString();
    string refundAddress = params["refundaddress"].asString();
    double amount = params["amount"].asDouble();
    string secretHash = params["secrethash"].asString();
    string coinType = params["cointype"].asString();
    string rpcAddress = params["rpcaddress"].asString();
    string rpcPort = params["rpcport"].asString();
    string rpcUserName = params["rpcusername"].asString();
    string rpcPassword = params["rpcpassword"].asString();

    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if(feeChangeAddress.empty() || themAddress.empty() || feeAddress.empty() || feeChangeAddress.empty() || assetChangeAddress.empty() ||
		cfosType.empty() || refundAddress.empty() || amount < 0 || secretHash.empty() || coinType.empty() || rpcAddress.empty() ||
        rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
    {
        response["result"] = "failed";
        cout << "Params wrong." << endl;
        return false;
    }

    if(!Participate(fromAddress, themAddress, feeAddress, feeChangeAddress, assetChangeAddress, cfosType,
		refundAddress, amount, secretHash, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
    {
        response["result"] = "failed";
        cout << "Participate failed." << endl;
        return false;
    }
    
    response["result"] = "success";

    return true;
}

bool AtomRpc::AtomRedeemCoin(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
    Json::Value params = root["params"];
    string contract = params["contract"].asString();
	string txid = params["txid"].asString();
	string secret = params["secret"].asString();
	string redeemAddress = params["redeemaddress"].asString();
    string coinType = params["cointype"].asString();
    string rpcAddress = params["rpcaddress"].asString();
    string rpcPort = params["rpcport"].asString();
    string rpcUserName = params["rpcusername"].asString();
    string rpcPassword = params["rpcpassword"].asString();
    
	response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if(contract.empty() || txid.empty() || secret.empty() || redeemAddress.empty() || coinType.empty() || rpcAddress.empty() ||
        rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
    {   
        response["result"] = "failed";
		cout << "Params wrong." << endl;
        return false;
    }

    if(!Redeem(contract, txid, secret, redeemAddress, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
	{
		response["result"] = "failed";
		cout << "Redeem failed." << endl;
		return false;
	}

	response["result"] = "success";

    return true;
}

bool AtomRpc::AtomRefundCoin(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
    Json::Value params = root["params"];
    string contract = params["contract"].asString();
    string txid = params["txid"].asString();
    string refundAddress = params["refundaddress"].asString();
    string coinType = params["cointype"].asString();
    string rpcAddress = params["rpcaddress"].asString();
    string rpcPort = params["rpcport"].asString();
    string rpcUserName = params["rpcusername"].asString();
    string rpcPassword = params["rpcpassword"].asString();

    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if(contract.empty() || txid.empty() || refundAddress.empty() || coinType.empty() || rpcAddress.empty() ||
        rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
    {
        response["result"] = "failed";
        cout << "Params wrong." << endl;
        return false;
    }

    if(!Refund(contract, txid, refundAddress, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
    {
        response["result"] = "failed";
        cout << "Refund failed." << endl;
        return false;
    }

    response["result"] = "success";

    return true;
}

bool AtomRpc::AtomExtractSecret(const Json::Value& root, Json::Value& response)
{
	std::cout << "Receive query: " << root << std::endl;
    Json::Value params = root["params"];
    string txid = params["txid"].asString();
    string secretHash = params["secrethash"].asString();
	string coinType = params["cointype"].asString();
    string rpcAddress = params["rpcaddress"].asString();
    string rpcPort = params["rpcport"].asString();
    string rpcUserName = params["rpcusername"].asString();
    string rpcPassword = params["rpcpassword"].asString();
    
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if(txid.empty() || secretHash.empty() || coinType.empty() || rpcAddress.empty() ||
        rpcPort.empty() || rpcUserName.empty() || rpcPassword.empty())
    {   
        response["result"] = "failed";
        cout << "Params wrong." << endl;
		return false;
    }
    
    if(!ExtractSecret(txid, secretHash, response, coinType, rpcAddress, rpcPort, rpcUserName, rpcPassword))
	{
		response["result"] = "failed";
		cout << "ExtractSecret failed." << endl;
		return false;
	}

	response["result"] = "success";
    
    return true;
}
