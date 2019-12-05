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
  params["contract"] = "63a614d311e7e8dc61bf9d6b5b5ec04c9efd5d7fc01aae8876a914518da2265fcc005b0327e0ea94b627f6744a7aed67040d515f5ab17576a9147f51c777600559a0621146e9c6b1c5981aa635db6888ac";
  params["txid"] = "9a04d024d3049b67e8677f66949d9f8197c1f33342702b7f38e2118f5bd5eccd";
  params["secret"] = "7571da404c85f526aa3cbf206b12017ef1208089e9d19fe2450f93ca45e3ca66";
  params["redeemaddress"] = "1N469NBibtZpNg9RZuigs4edELJN2zc7s5";
  params["cointype"] = "bitcoin";
  params["rpcaddress"] = "192.168.10.184";
  params["rpcport"] = "8332";
  params["rpcusername"] = "bitcoin";
  params["rpcpassword"] = "BUG7UEbUkDTnETVnUicmc66iYqQtUuoHhkhwv9pDGphT";

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

