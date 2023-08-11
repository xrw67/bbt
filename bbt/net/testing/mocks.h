#ifndef BBT_NET_TESTING_MOCKS_H_
#define BBT_NET_TESTING_MOCKS_H_

#include <vector>

#include "bbt/net/buffer.h"
#include "bbt/net/connection.h"

namespace bbt {
namespace net {
namespace testing {

struct MockConnectionPair : public Connection {
  std::weak_ptr<MockConnectionPair> peer_conn;

  void connect(const std::shared_ptr<MockConnectionPair>& other) {
    peer_conn = other;
  }

  void Send(const void* data, size_t len) {
    // 直接发给对端
    auto l = peer_conn.lock();
    if (l) l->HandleRead(data, len);
  }

  void HandleRead(const void* data, size_t len) {
    // 收到对端的数据，抛给上层
    Buffer buf;
    buf.Append((const char*)data, len);
    read_callback_(shared_from_this(), &buf);
  }
};

}  // namespace testing
}  // namespace net
}  // namespace bbt

#endif  // BBT_NET_TESTING_MOCKS_H_