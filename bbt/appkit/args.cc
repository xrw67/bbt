#include "bbt/appkit/args.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include "bbt/base/str_util.h"

namespace bbt {

namespace {

enum ArgType { kBool, kLong, kString };

struct ArgFlag {
  ArgType type;
  char short_name;
  std::string long_name;
  std::string default_value;
  std::string help;
};

bool IsLetter(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool IsDigit(char c) { return (c >= '0' && c <= '9'); }

inline bool IsLetterOrDigit(char c) { return IsLetter(c) || IsDigit(c); }

std::vector<std::string> ParseArgLine(const std::string& line) {
  std::vector<std::string> result;
  auto tmp_args = StrSplit(line, ' ');

  for (const auto& i : tmp_args) {
    auto arg = StrTrim(i, " \r\t\n");
    if (!arg.empty()) result.push_back(to_string(arg));
  }

  return result;
}

}  // namespace

typedef std::shared_ptr<ArgFlag> ArgFlagPtr;

struct Args::Impl {
  std::map<char, ArgFlagPtr> short_flags_;
  std::map<std::string, ArgFlagPtr> long_flags_;

  // Bool : 不为空表示true，空字符串表示false
  // Long : 使用StrToLong 转换
  // String: 直接存取
  std::map<std::string, std::string> values_;

  void AddFlag(const ArgFlagPtr& flag) {
    if (flag->short_name) short_flags_[flag->short_name] = flag;
    if (flag->long_name.size() >= 2) long_flags_[flag->long_name] = flag;
  }

  std::string GetValue(const char* long_name) {
    if (!long_name) return {};

    auto it1 = values_.find(long_name);
    if (it1 != values_.end()) {
      return it1->second;
    }

    // default
    auto it2 = long_flags_.find(long_name);
    if (it2 != long_flags_.end()) {
      return it2->second->default_value;
    }

    return {};
  }

  ArgFlagPtr GetFlag(const std::string& name) {
    size_t len = name.length();

    // is short ?
    if (len == 2 && IsLetterOrDigit(name[1])) {
      auto it = short_flags_.find(name[1]);
      if (it != short_flags_.end()) {
        return it->second;
      }
    }

    // is long ?
    if (len >= 4 && name[1] == '-') {
      // check name
      for (size_t i = 2; i < len; i++) {
        if (!IsLetterOrDigit(name[i])) return {};
      }

      auto it = long_flags_.find(name.substr(2));
      if (it != long_flags_.end()) {
        return it->second;
      }
    }
    return {};
  }

  Status Parse(const std::vector<std::string>& args) {
    values_.clear();  // init

    if (args.empty()) return OkStatus();  // nothing

    std::map<std::string, std::string> result;
    ArgFlagPtr current_flag;
    size_t i = 0;
    if (args[0][0] != '-') {
      i++;  // skip program name whit first argument;
    }

    for (; i < args.size(); i++) {
      const auto& arg = args[i];
      if (arg[0] == '-') {
        if (current_flag) return InvalidArgumentError("flag no value");
        current_flag = GetFlag(arg);
        if (!current_flag) return InvalidArgumentError("flag not found");

        if (current_flag->type == ArgType::kBool) {
          result[current_flag->long_name] = "true";
          current_flag.reset();
          continue;
        }
      } else {
        if (!current_flag) return InvalidArgumentError("no flag");

        result[current_flag->long_name] = arg;
        current_flag.reset();
        continue;
      }
    }

    if (current_flag) return InvalidArgumentError("flag no value");

    values_.swap(result);
    return OkStatus();
  }
};

Args::Args() : impl_(new Impl()) {
  // insert help args
  ArgFlagPtr f(new ArgFlag(ArgFlag{kBool, 'h', "help", "", "This help text"}));
  impl_->AddFlag(f);
}

Args::~Args() { delete impl_; }

void Args::AddBool(char short_name, const char* long_name, const char* help) {
  if (short_name && long_name) {
    if (!help) help = "";
    ArgFlagPtr flag(
        new ArgFlag(ArgFlag{kBool, short_name, long_name, "", help}));
    impl_->AddFlag(flag);
  }
}

void Args::AddString(char short_name, const char* long_name,
                     const char* default_value, const char* help) {
  if (short_name && long_name) {
    if (!help) help = "";
    ArgFlagPtr flag(new ArgFlag(
        ArgFlag{kString, short_name, long_name, default_value, help}));
    impl_->AddFlag(flag);
  }
}

void Args::AddLong(char short_name, const char* long_name,
                   long long default_value, const char* help) {
  if (short_name && long_name) {
    if (!help) help = "";
    ArgFlagPtr flag(new ArgFlag(ArgFlag{kLong, short_name, long_name,
                                        std::to_string(default_value), help}));
    impl_->AddFlag(flag);
  }
}

bool Args::GetBool(const char* long_name) {
  std::string value = impl_->GetValue(long_name);
  return !value.empty();
}

std::string Args::GetString(const char* long_name) {
  return impl_->GetValue(long_name);
}

long long Args::GetLong(const char* long_name) {
  return std::stoll(impl_->GetValue(long_name));
}

// LCOV_EXCL_START
std::string Args::Help() {
  std::stringstream ss;

  ss << "Arguments:" << std::endl;

  for (const auto& i : impl_->long_flags_) {
    ArgFlagPtr flag(i.second);
    ss << StrPrintf("-%c, --%-15s %s", flag->short_name,
                    flag->long_name.c_str(), flag->help.c_str());
    if (!flag->default_value.empty()) {
      ss << " (default: " << flag->default_value << ")";
    }

    ss << std::endl;
  }
  return ss.str();
}
// LCOV_EXCL_STOP

Status Args::Parse(const char* arg_line) {
  auto args = ParseArgLine(arg_line);
  return impl_->Parse(args);
}

Status Args::Parse(int argc, const char* const argv[]) {
  std::vector<std::string> args;

  for (int i = 0; i < argc && argv[i]; i++) {
    auto a = StrTrim(argv[i], " \r\t\n");
    if (!a.empty()) args.push_back(to_string(a));
  }
  return impl_->Parse(args);
}

}  // namespace bbt