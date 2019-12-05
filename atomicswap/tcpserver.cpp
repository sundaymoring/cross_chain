#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <vector>

#include <jsonrpc/jsonrpc.h>

#include "tcpserver.h"
#include "atomicswap.h"

/**
 * \var g_run
 * \brief Running state of the program.
 */
static volatile sig_atomic_t g_run = 0;

/**
 * \brief Signal management.
 * \param code signal code
 */
static void signal_handler(int code)
{
  switch(code)
  {
    case SIGINT:
    case SIGTERM:
      g_run = 0;
      break;
    default:
      break;
  }
}

int startserver(const std::string &ip, uint16_t port)
{
	AtomRpc a;
	Json::Rpc::TcpServer server(ip, port);

	if(!networking::init())
	{
		std::cerr << "Networking initialization failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(signal(SIGTERM, signal_handler) == SIG_ERR)
	{
		std::cout << "Error signal SIGTERM will not be handled" << std::endl;
	}

	if(signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cout << "Error signal SIGINT will not be handled" << std::endl;
	}

	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomGetInfo,
		std::string("getinfo")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomInitiate,
		std::string("initiate")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomInitiateCfos,
        std::string("initiatecfos")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomParticipate,
        std::string("participate")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomParticipateCfos,
        std::string("participatecfos")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomRedeemCoin,
        std::string("redeem")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomRefundCoin,
        std::string("refund")));
	server.AddMethod(new Json::Rpc::RpcMethod<AtomRpc>(a, &AtomRpc::AtomExtractSecret,
        std::string("extractsecret")));
  
	/* server.SetEncapsulatedFormat(Json::Rpc::NETSTRING); */
	if(!server.Bind())
	{
		std::cout << "Bind failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	if(!server.Listen())
	{
		std::cout << "Listen failed" << std::endl;
		exit(EXIT_FAILURE);
	}

	g_run = 1;

	std::cout << "Start JSON-RPC TCP server" << std::endl;

	while(g_run)
	{
		server.WaitMessage(1000);
	}

	std::cout << "Stop JSON-RPC TCP server" << std::endl;
	server.Close();
	networking::cleanup();

	return EXIT_SUCCESS;
}

