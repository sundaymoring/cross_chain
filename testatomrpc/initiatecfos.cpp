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
  Json::Rpc::TcpClient tcpClient(std::string("192.168.10.193"), 14159);
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
  query["method"] = "initiatecfos";

  Json::Value params;
  params["fromaddress"] = "1Coq6AFqgHrYhweybEKALMoGsiuD6beiGT";
  params["themaddress"] = "15DnDaF1GEjS9eYbys1k8Rt1Y7mCTeoT7J";
  params["feeaddress"] = "12F3sD2e9UPBiBctZuSfhU6KvxEbY6BfrK";
  params["feechangeaddress"] = "12F3sD2e9UPBiBctZuSfhU6KvxEbY6BfrK";
  params["assetchangeaddress"] = "1Coq6AFqgHrYhweybEKALMoGsiuD6beiGT";
  params["cfostype"] = "TEST";
  params["refundaddress"] = "14HgQS1TeydXz8M5uNDw3x6xB1QWq3sGA6";
  params["amount"] = 1;
  params["cointype"] = "cfos";
  params["rpcaddress"] = "192.168.10.175";
  params["rpcport"] = "20002";
  params["rpcusername"] = "aliyun-taiyi-sc-passport-01";
  params["rpcpassword"] = "TYXVGS*)-%3fd";

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

