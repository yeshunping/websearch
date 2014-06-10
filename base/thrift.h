// Copyright 2010. All Rights Reserved.
// Author: shunpingye@gmail.com (Shunping Ye)

#ifndef BASE_THRIFT_H_
#define BASE_THRIFT_H_

#include <string>

#include "thirdparty/thrift/include/concurrency/ThreadManager.h"
#include "thirdparty/thrift/include/concurrency/PosixThreadFactory.h"
#include "thirdparty/thrift/include/protocol/TBinaryProtocol.h"
#include "thirdparty/thrift/include/server/TThreadPoolServer.h"
#include "thirdparty/thrift/include/transport/TServerSocket.h"
#include "thirdparty/thrift/include/transport/TBufferTransports.h"
#include "thirdparty/thrift/include/server/TThreadedServer.h"
#include "thirdparty/thrift/include/transport/TSocket.h"
#include "thirdparty/thrift/include/transport/TTransportUtils.h"

#include "base/logging.h"
#include "thirdparty/thrift/include/protocol/TDebugProtocol.h"
#include "thirdparty/thrift/include/protocol/TJSONProtocol.h"
#include "thirdparty/thrift/include/protocol/TBinaryProtocol.h"

namespace base {

using ::apache::thrift::TProcessor;
using ::apache::thrift::transport::TServerTransport;
using ::apache::thrift::transport::TServerSocket;
using ::apache::thrift::transport::TTransportFactory;
using ::apache::thrift::protocol::TProtocolFactory;
using ::apache::thrift::transport::TBufferedTransportFactory;
using ::apache::thrift::protocol::TBinaryProtocolFactory;
using ::apache::thrift::server::TThreadPoolServer;
using ::apache::thrift::server::TThreadedServer;
using ::apache::thrift::transport::TTransport;
using ::apache::thrift::transport::TSocket;
using ::apache::thrift::protocol::TProtocol;
using ::apache::thrift::protocol::TBinaryProtocol;

template<typename ThriftStruct>
std::string ThriftToDebugString(const ThriftStruct& ts) {
  return apache::thrift::ThriftDebugString(ts);
}

//  NOTE: this function has not been implemented yet,
//  since TDebugProtocol hasn't support read yet.
template<typename ThriftStruct>
bool FromDebugStringToThrift(const std::string& buff,
                            ThriftStruct* ts) {
  using namespace apache::thrift::transport;  // NOLINT
  using namespace apache::thrift::protocol;  // NOLINT
  TMemoryBuffer* buffer = new TMemoryBuffer;
  buffer->write((const uint8_t*)buff.data(), buff.size());
  boost::shared_ptr<TTransport> trans(buffer);
  TDebugProtocol protocol(trans);
  ts->read(&protocol);
  return true;
}

template<typename ThriftStruct>
std::string ThriftToJsonString(const ThriftStruct& ts) {
  return apache::thrift::ThriftJSONString(ts);
}

template<typename ThriftStruct>
bool FromJsonStringToThrift(const std::string& buff,
                            ThriftStruct* ts) {
  using namespace apache::thrift::transport;  // NOLINT
  using namespace apache::thrift::protocol;  // NOLINT
  TMemoryBuffer* buffer = new TMemoryBuffer;
  buffer->write((const uint8_t*)buff.data(), buff.size());
  boost::shared_ptr<TTransport> trans(buffer);
  TJSONProtocol protocol(trans);
  ts->read(&protocol);
  return true;
}

template<typename ThriftStruct>
std::string ThriftToString(const ThriftStruct& ts) {
  using namespace apache::thrift::transport;  // NOLINT
  using namespace apache::thrift::protocol;  // NOLINT
  TMemoryBuffer* buffer = new TMemoryBuffer;
  boost::shared_ptr<TTransport> trans(buffer);
  TBinaryProtocol protocol(trans);
  ts.write(&protocol);
  uint8_t* buf;
  uint32_t size;
  buffer->getBuffer(&buf, &size);
  return std::string((char*)buf, (unsigned int)size);  // NOLINT
}

template<typename ThriftStruct>
std::string FromThriftToString(ThriftStruct* ts) {
  return ThriftToString(*ts);
}

template<typename ThriftStruct>
bool FromStringToThrift(const std::string& buff,
                        ThriftStruct* ts) {
  using namespace apache::thrift::transport;  // NOLINT
  using namespace apache::thrift::protocol;  // NOLINT
  TMemoryBuffer* buffer = new TMemoryBuffer;
  buffer->write((const uint8_t*)buff.data(), buff.size());
  boost::shared_ptr<TTransport> trans(buffer);
  TBinaryProtocol protocol(trans);
  ts->read(&protocol);
  return true;
}

class ThriftObjReader {
 public:
  template<typename ThriftStruct>
  // TODO(yesp) : improve it
  bool FromStringToThrift(const std::string& value, ThriftStruct* ts) {
    return FromStringToThrift(value, ts);
  }
 private:
};

class ThriftObjWriter {
 public:
  // TODO(yesp) : improve it
  template<typename ThriftStruct>
  bool FromThriftToString(const ThriftStruct& ts, std::string* out) {
    out->assign(ThriftToString(ts));
    return true;
  }
 private:
};

//  Use TThreadPoolServer
#define THRIFT_SERVER_RUN(service_name, thread_number, listen_port, handler) {\
  ThriftServer<service_name##Handler, service_name##Processor>::Run(thread_number, listen_port, handler); \
}

//  Use TThreadPoolServer
#define DEFAULT_THRIFT_SERVER_RUN(service_name, thread_number, listen_port) {\
  ThriftServer<service_name##Handler, service_name##Processor>::Run(thread_number, listen_port); \
}

//  Use TThreadedServer
#define THREADED_THRIFT_SERVER_RUN(service_name, listen_port, handler) {\
  ThriftServer<service_name##Handler, service_name##Processor>::Run(listen_port, handler); \
}

//  Use TThreadedServer
#define THREADED_DEFAULT_THRIFT_SERVER_RUN(service_name, listen_port) {\
  ThriftServer<service_name##Handler, service_name##Processor>::Run(listen_port); \
}
// Use macro THRIFT_SERVER_RUN or DEFAULT_THRIFT_SERVER_RUN instead of ThriftServer
template <class H, class P>
class ThriftServer {
 public:
  ThriftServer<H, P>() {}
  ~ThriftServer<H, P>() {}
  static void Run(int thread_number, int listen_port);
  static void Run(int thread_number, int listen_port, boost::shared_ptr<H> handler);
  static void Run(int listen_port);
  static void Run(int listen_port, boost::shared_ptr<H> handler);
};

template<class H, class P>
void ThriftServer<H, P>::Run(int thread_number, int listen_port, boost::shared_ptr<H> handler) {
  boost::shared_ptr<TProcessor> processor(new P(handler));
  boost::shared_ptr<TServerTransport> serverTransport(
        new TServerSocket(listen_port));
  boost::shared_ptr<TTransportFactory> transportFactory(
        new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  boost::shared_ptr<apache::thrift::concurrency::ThreadManager> threadManager =
      apache::thrift::concurrency::ThreadManager::newSimpleThreadManager(thread_number);
  boost::shared_ptr<apache::thrift::concurrency::PosixThreadFactory> threadFactory =
      boost::shared_ptr<apache::thrift::concurrency::PosixThreadFactory>(
          new apache::thrift::concurrency::PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();
  TThreadPoolServer server(processor,
               serverTransport,
               transportFactory,
               protocolFactory,
               threadManager);

  LOG(INFO) << "thrift server start, thread number:" << thread_number
            << ", listen port:" << listen_port;
  try {
    server.serve();
  } catch (apache::thrift::transport::TTransportException &ex) {
    LOG(FATAL) << ex.what();
  }
}

template<class H, class P>
void ThriftServer<H, P>::Run(int listen_port,
                             boost::shared_ptr<H> handler) {
  boost::shared_ptr<TProcessor> processor(new P(handler));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(listen_port));
  boost::shared_ptr<TTransportFactory> transportFactory(
      new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TThreadedServer server(processor,
                         serverTransport,
                         transportFactory,
                         protocolFactory);

  LOG(INFO)<< "thrift server start, listen port:" << listen_port;
  try {
    server.serve();
  } catch (apache::thrift::transport::TTransportException &ex) {
    LOG(FATAL) << ex.what();
  }
}

template<class H, class P>
void ThriftServer<H, P>::Run(int thread_number, int listen_port) {
  boost::shared_ptr<H> handler(new H);
  Run(thread_number, listen_port, handler);
}

template<class H, class P>
void ThriftServer<H, P>::Run(int listen_port) {
  boost::shared_ptr<H> handler(new H);
  Run(listen_port, handler);
}

template <class T>
class ThriftClient {
 public:
  ThriftClient(const std::string& host, int port)
    : host_(host), port_(port) { }

  ~ThriftClient() {
    if (transport_.get() && transport_->isOpen()) {
      transport_->close();
    }
  }
  void Init() {
    socket_.reset(new TSocket(host_, port_));
    transport_.reset(new apache::thrift::transport::TBufferedTransport(socket_));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport_));
    transport_->open();
    client_.reset(new T(protocol));
  }

  TSocket* socket() {
    return socket_.get();
  }
  T* operator->() const  {
    return client_.get();
  }

  T* get() const { return client_.get(); }

  const std::string& host() const {
    return host_;
  }

  int port() const {
    return port_;
  }

private:
  std::string host_;
  int port_;
  boost::shared_ptr<T> client_;
  boost::shared_ptr<TSocket> socket_;
  boost::shared_ptr<TTransport> transport_;
};

}
#endif  // BASE_THRIFT_H_
