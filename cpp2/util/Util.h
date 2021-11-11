#pragma once

#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp2/async/HeaderClientChannel.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/server/BaseThriftServer.h>
#include <thrift/lib/cpp2/transport/core/ThriftClient.h>
#include <thrift/lib/cpp2/transport/core/testutil/ServerConfigsMock.h>
#include <thrift/lib/cpp2/transport/http2/client/H2ClientConnection.h>

using apache::thrift::ClientConnectionIf;
using apache::thrift::H2ClientConnection;
using apache::thrift::HeaderClientChannel;
using apache::thrift::RocketClientChannel;
using apache::thrift::ThriftClient;
using apache::thrift::ThriftServerAsyncProcessorFactory;
using apache::thrift::async::TAsyncSSLSocket;
using apache::thrift::server::ServerConfigsMock;


folly::AsyncSocket::UniquePtr myGetSocket(
    folly::EventBase* evb,
    folly::SocketAddress const& addr,
    bool encrypted,
    std::list<std::string> advertizedProtocols = {}){
  folly::AsyncSocket::UniquePtr sock(new folly::AsyncSocket(evb, addr));
  if (encrypted) {
    auto sslContext = std::make_shared<folly::SSLContext>();
    sslContext->setAdvertisedNextProtocols(advertizedProtocols);
    auto sslSock = new TAsyncSSLSocket(
        sslContext, evb, sock->detachNetworkSocket(), false);
    sslSock->sslConn(nullptr);
    sock.reset(sslSock);
  }
  sock->setZeroCopy(true);
  return sock;};


template <typename AsyncClient>
static std::unique_ptr<AsyncClient> newHeaderClient(
    folly::EventBase* evb, folly::SocketAddress const& addr) {
  auto sock = myGetSocket(evb, addr, false);
  auto chan = HeaderClientChannel::newChannel(std::move(sock));
  return std::make_unique<AsyncClient>(std::move(chan));
}

template <typename AsyncClient>
static std::unique_ptr<AsyncClient> newHTTP2Client(
    folly::EventBase* evb, folly::SocketAddress const& addr, bool encrypted) {
  auto sock = myGetSocket(evb, addr, encrypted, {"h2"});
  std::shared_ptr<ClientConnectionIf> conn =
      H2ClientConnection::newHTTP2Connection(std::move(sock));
  auto client = ThriftClient::Ptr(new ThriftClient(conn, evb));
  client->setProtocolId(apache::thrift::protocol::T_COMPACT_PROTOCOL);
  client->setTimeout(500);
  return std::make_unique<AsyncClient>(std::move(client));
}

template <typename AsyncClient>
static std::unique_ptr<AsyncClient> newRocketClient(
    folly::EventBase* evb, folly::SocketAddress const& addr, bool encrypted) {
  auto sock = myGetSocket(evb, addr, encrypted, {"rs2"});
  RocketClientChannel::Ptr channel =
      RocketClientChannel::newChannel(std::move(sock));
  return std::make_unique<AsyncClient>(std::move(channel));
}

template <typename AsyncClient>
static std::unique_ptr<AsyncClient> newClient(
    folly::EventBase* evb,
    folly::SocketAddress const& addr,
    folly::StringPiece transport,
    bool encrypted = false) {
  if (transport == "header") {
    return newHeaderClient<AsyncClient>(evb, addr);
  }
  if (transport == "rocket") {
    return newRocketClient<AsyncClient>(evb, addr, encrypted);
  }
  if (transport == "http2") {
    return newHTTP2Client<AsyncClient>(evb, addr, encrypted);
  }
  return nullptr;
}

template <typename AsyncClient>
class ConnectionThread : public folly::ScopedEventBaseThread {
 public:
  ~ConnectionThread() {
    getEventBase()->runInEventBaseThreadAndWait([&] { connection_.reset(); });
  }

  std::shared_ptr<AsyncClient> newSyncClient(
      folly::SocketAddress const& addr,
      folly::StringPiece transport,
      bool encrypted = false) {
    DCHECK(connection_ == nullptr);
    getEventBase()->runInEventBaseThreadAndWait([&]() {
      connection_ =
          newClient<AsyncClient>(getEventBase(), addr, transport, encrypted);
    });
    return connection_;
  }

 private:
  std::shared_ptr<AsyncClient> connection_;
};
