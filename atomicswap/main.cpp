/*#include <iostream>
#include <stdio.h>
#include <openssl/rand.h>
#include "base58.h"
#include "params.h"
*/
#include "tcpserver.h"
//#include "atomicswap.h"

int  main()
{
	//Load settings
    //LoadSettings();
	startserver("127.0.0.1", 14159);

	//Initiate test
	/*double amount = 2;
	Initiate("BAJhpjz2e4n7uEagVoXKTAJtnjeeeqFQit", "BLb2fqVG5fd5Aez7zRT44js61V1EmwjTv8", amount);*/

	//Participate test
	/*double amount = 3;
	Participate("14GVJY42rwMKYQFM6Nkpn8hvC5UPky8q1B", "18vmBDq57ZSvjzXKhk375BwRGSZ7eppAT7", amount, "a25aeab421663d7a3f76a964980133b2e20c5dde");*/

	//Test error log
	//return AtoError("CAlert::CheckSignature() : verify signature failed");

	//RedeemBcoin test
    /*string contract = "63a614a25aeab421663d7a3f76a964980133b2e20c5dde8876a91423d63e0c6b275bd3d6332c426c2db8c5970834a76704f9f8455ab17576a91456f3cd8c15d582f370e70f00bb8fe65a1d2edc756888ac";
    string txid = "ba153ed01b61ff0993eecb1083f6e88d23dd1beb835ae10b95987b1f78a955ac";
    string secret = "54344f531d6e014cae399d7866aa2cb5a7f3d09a16496352a76554f24c708cb5";
    string redeemAddress = "16FEXyhpXwAhU1RCVS874H3q3mKWRE2Fgj";
    RedeemBcoin(contract, txid, secret, redeemAddress);*/

	//Test extractSecret
    /*string secret;
    ExtractSecret("290df816fafde77762fdda1f75c30f5482dddcc97ce907ae1bdd539bd59e15b4", "a25aeab421663d7a3f76a964980133b2e20c5dde", secret);
    cout << secret << endl;*/

	//RedeemAcoin test
	/*string contract = "63a614a25aeab421663d7a3f76a964980133b2e20c5dde8876a914403b3c4499be6a5a2e6a535ef8a5970852bfc6da6704db49475ab17576a914b102e1819a1e3bfe5993090f237a16cf153b7a3c6888ac";
	string txid = "7606365509afe466326f5b665a09453e7b746f38743efd2ab059a41f82781afa";
	string secret = "54344f531d6e014cae399d7866aa2cb5a7f3d09a16496352a76554f24c708cb5";
	string redeemAddress = "B6t5qRxStymKL4bK5Ef6WN6FNuk1EevAqK";
	RedeemAcoin(contract, txid, secret, redeemAddress);*/

	/*CBitcoinAddress baddress("bH9nNw8p2rEGaRBajXSdcy8nmrP8MXEwhS");
	CScript script;
	CScriptID scriptID;
	if(baddress.GetScriptID(scriptID))
	{
		script << scriptID;
		cout << "script" << endl << HexStr(script) << endl;
	}*/

	//Rpc test
	/*RPC_conf rpc{"192.168.10.184", "55650", "ccrpc", "BUG7UEbUkDTnETVnUicmc66iYqQtUuoHhkhwv9pDGphT"};
	Array params;
    Object reply = CallRPC("getinfo", params, rpc);
	cout << write_string(Value(reply), false) + '\n';*/

	//Generate address
	/*CPubKey newKey = GenerateNewKey(true);
    CKeyID meKeyID = newKey.GetID();
    CBitcoinAddress pubkeyaddress;
    //33 bytes compressed  pubkey
    cout << "New pubkey:" << endl << HexStr(newKey) << endl;
    pubkeyaddress.Set(base58Prefixes_PubKey_Address, meKeyID);
    cout << "New address:" << endl <<  pubkeyaddress.ToString() << endl;

	CBitcoinAddress address2(pubkeyaddress.ToString());
	CKeyID id2;
	if(address2.GetKeyID(id2))
		cout << "get key id ok" << endl;
	if(id2 == meKeyID)
		cout << "2 id is same" << endl;
	else
		cout << "2 id is not same" << endl;*/

	/*string contractstr1 = "63";
	vector<unsigned char> contractVec1 = ParseHex(contractstr1);
    CScript contractScript1(contractVec1.begin(), contractVec1.end());
    cout << "Contract:" << endl << contractScript1.ToString() << endl;
    CScriptID parconID1 = contractScript1.GetID();
	CScript parscript;
	parscript << parconID1;
	CBitcoinAddress address1;
    address1.Set(base58Prefixes_Script_Address, parconID1);
	cout << "parscript:" << endl << HexStr(parscript) << endl;
	cout << "address1:" << endl << address1.ToString() << endl;*/

	//cout << "Sleep 5s." << endl;
	//sleep(5);

	/*string contractstr2 = "63";
    vector<unsigned char> contractVec2 = ParseHex(contractstr2);
    CScript contractScript2(contractVec2.begin(), contractVec2.end());
    cout << "Contract:" << endl << contractScript2.ToString() << endl;
    CScriptID parconID2 = contractScript2.GetID();
	CBitcoinAddress address2;
    address2.Set(base58Prefixes_Script_Address, parconID2);*/
	
	/*CScript contract;
    contract << OP_IF;
    cout << "Contract:" << endl << contract.ToString() << endl;
    cout << "Contract hex:" << endl << HexStr(contract) << endl;*/
	/*cout << "address1" << endl << address1.ToString() << endl;
	cout << "address2" << endl << address2.ToString() << endl;
	if(address1 == address2)
		cout << "address1 == address2" << endl;
	else
		cout << "address1 != address2" << endl;

	CBitcoinAddress address3("bFhgEkCEub21rUJM7Tx5u1BvDFRWFgBPFH");
	CBitcoinAddress address4("bZCs4tMdWSQpTahahqU1G9XuLZee5y2CVS");
	if(address3 == address4)
		cout << "address3 == address4" << endl;
	else
		cout << "address3 != address4" << endl;*/
	//Get contract address
    //string contractAddressStr = getContractAddress(contract);
    /*CScriptID contractID = contract.GetID();
    CBitcoinAddress contractAddress;
    contractAddress.Set(base58Prefixes_Script_Address, contractID);
    string contractAddressStr = contractAddress.ToString();*/
    //cout << "Contract address:" << contractAddressStr.size() << endl << contractAddressStr << endl;
	//CBitcoinAddress address(contractAddressStr);
	//CBitcoinAddress address = getContractAddress(contract);
	/*CScriptID contractID = contract.GetID();
    CBitcoinAddress address;
    address.Set(base58Prefixes_Script_Address, contractID);
	CScriptID scriptID;
	address.GetScriptID(scriptID);

	string str = address.ToString();
	CBitcoinAddress address1(str);
	CScriptID scriptID1;
	address1.GetScriptID(scriptID1);
	
    cout << "Contract address:" << endl << address.ToString() << endl;
	cout << "address1:" << endl << address1.ToString() << endl;

	if()
	if(parconID == scriptID)
		cout << "parconID == scriptID" << endl;
	else
		cout << "parconID != scriptID" << endl;

	if(parconID == scriptID1)
        cout << "parconID == scriptID1" << endl;
    else
        cout << "parconID != scriptID1" << endl;*/

	/*size_t n;
	int i;
	unsigned char in[]="3dsferyewyrtetegvbzVEgarhaggavxcv";
	unsigned char out[20];
	n=strlen((const char*)in);
	RIPEMD160(in,n,out);
	printf("RIPEMD160 digest result :\n");
	for(i=0;i<20;i++)
		printf("%x ",out[i]);
	cout << endl;
	SHA256(in,n,out);
	printf("\n\nSHA256 digest result :\n");
	for(i=0;i<32;i++)
		printf("%x ",out[i]);
	cout << endl;
	return 1;*/
}
