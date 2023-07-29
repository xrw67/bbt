#ifndef BBT_HTTP_REPLY_H_
#define BBT_HTTP_REPLY_H_


#include <string>
#include <vector>
#include <asio.hpp>
#include "header.h"

namespace bbt {
namespace http {


/// A reply to be sent to a client.
struct Reply
{
  /// The status of the reply.
  enum status_type
  {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } status;

  /// The headers to be included in the reply.
  std::vector<Header> headers;

  /// The content to be sent in the reply.
  std::string content;

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  std::vector<asio::const_buffer> to_buffers();

  /// Get a stock reply.
  static Reply stock_reply(status_type status);
};

}  // namespace http
}  // namespace bbt

#endif  // BBT_HTTP_REPLY_H_