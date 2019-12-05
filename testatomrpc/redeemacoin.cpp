/*
 *  JsonRpc-Cpp - JSON-RPC implementation.
 *  Copyright (C) 2008-2011 Sebastien Vincent <sebastien.vincent@cppextrem.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file tcp-client.cpp
 * \brief Simple JSON-RPC TCP client.
 * \author Sebastien Vincent
 */

#include <cstdio>
#include <cstdlib>

#include <jsonrpc/jsonrpc.h>

/**
 * \brief Entry point of the program.
 * \param argc number of argument
 * \param argv array of arguments
 * \return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char** argv)
{
  Json::Rpc::TcpClient tcpClient(std::string("127.0.0.1"), 14159);
  Json::Value query;
  Json::FastWriter writer;
  std::string queryStr;
  std::string responseStr;
  
  /* avoid compilation warnings */
  (void)argc;
  (void)argv;

  if(!networking::init())
  {
    std::cerr << "Networking initialization failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  if(!tcpClient.Connect())
  {
    std::cerr << "Cannot connect to remote peer!" << std::endl;
    exit(EXIT_FAILURE);
  }

  /* build JSON-RPC query */
  query["jsonrpc"] = "2.0";
  query["id"] = 1;
  query["method"] = "redeem";

  Json::Value params;
  params["contract"] = "63a614d311e7e8dc61bf9d6b5b5ec04c9efd5d7fc01aae8876a914403b3c4499be6a5a2e6a535ef8a5970852bfc6da6704eb9e605ab17576a914b102e1819a1e3bfe5993090f237a16cf153b7a3c6888ac";
  params["txid"] = "35b2c74f677663d0a5b8a47bc7c552427a5d9bffd7b7a13d52de6acf531eeb75";
  params["secret"] = "7571da404c85f526aa3cbf206b12017ef1208089e9d19fe2450f93ca45e3ca66";
  params["redeemaddress"] = "BD5AauaQAcpP8wYaPn7zfnCSKuxtUnrn7z";
  params["cointype"] = "blackcoin";
  params["rpcaddress"] = "192.168.10.186";
  params["rpcport"] = "55650";
  params["rpcusername"] = "ccrpc";
  params["rpcpassword"] = "J6G5vvbmnqDZV5RWnrhFk8pjPZwERhjvCKMbnQ2Jq3E5";

  query["params"] = params;

  queryStr = writer.write(query);
  std::cout << "Query is: " << queryStr << std::endl;

  if(tcpClient.Send(queryStr) == -1)
  {
    std::cerr << "Error while sending data!" << std::endl;
    exit(EXIT_FAILURE);
  }

  /* wait the response */
  if(tcpClient.Recv(responseStr) != -1)
  {
    std::cout << "Received: " << responseStr << std::endl;
  }
  else
  {
    std::cerr << "Error while receiving data!" << std::endl;
  }

  tcpClient.Close();
  networking::cleanup();

  return EXIT_SUCCESS;
}

