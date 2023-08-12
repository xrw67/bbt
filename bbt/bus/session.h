#ifndef BBT_BUS_SESSION_H_
#define BBT_BUS_SESSION_H_

#include <map>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>

#include "bbt/bus/msg.h"
#include "bbt/bus/method.h"

namespace bbt {
namespace bus {

typedef std::function<void(const MsgPtr&)> MsgWriter;

class BusSession {
 public:
  explicit BusSession(const std::string& name);

  void set_msg_writer(const MsgWriter& w) { msg_writer_ = w; };

  Status AddMethod(const std::string& name, MethodFunc func);
  Status Call(const std::string& method, const In& in, Out* out);
  Status ACall(const std::string& method, const In& in, Result* result);

  // call when receive bus message coming
  void HandleMessage(const MsgPtr& msg);

 private:
  MsgId NextMsgId() noexcept { return next_id_.fetch_add(1); }

  void HandleRequestMessage(const MsgPtr& msg);
  void HandleResponseMessage(const MsgPtr& msg);
  void OnMethodRequest(const Msg& req, Msg* resp);

  std::string name_;
  std::atomic<MsgId> next_id_;

  /// 等待结果的列表
  std::mutex mutex_;
  std::map<MsgId, Result*> waitings_;  // TODO: result的析构函数要weakup再析构

  std::map<std::string, MethodFunc> methods_;

  MsgWriter msg_writer_;
};

}  // namespace bus
}  // namespace bbt
#endif  // BBT_BUS_SESSION_H_