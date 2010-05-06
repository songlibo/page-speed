// Copyright 2010 Google Inc.
// Author: jmarantz@google.com (Joshua Marantz)

#include "net/instaweb/util/public/cache_url_async_fetcher.h"
#include "net/instaweb/util/public/cache_url_fetcher.h"
#include "net/instaweb/util/public/http_cache.h"
#include "net/instaweb/util/public/simple_meta_data.h"
#include "net/instaweb/util/public/string_writer.h"

namespace net_instaweb {

namespace {

// This version of the caching async fetcher callback uses the
// caller-supplied response-header buffer, and also forwards the
// content to the client before caching it.
class ForwardingAsyncFetch : public CacheUrlFetcher::AsyncFetch {
 public:
  ForwardingAsyncFetch(const StringPiece& url, HTTPCache* cache,
                       MessageHandler* handler, Callback* callback,
                       Writer* writer, MetaData* response_headers)
      : CacheUrlFetcher::AsyncFetch(url, cache, handler),
        callback_(callback),
        client_writer_(writer),
        response_headers_(response_headers) {
  }

  virtual void Done(bool success) {
    // Copy the data to the client even with a failure; there may be useful
    // error messages in the content.
    client_writer_->Write(content_.data(), content_.size(), message_handler_);

    // Update the cache before calling the client Done callback, which might
    // delete the headers.
    if (success) {
      UpdateCache();
    }

    callback_->Done(success);
    delete this;
  }

  virtual MetaData* ResponseHeaders() { return response_headers_; }

 private:
  Callback* callback_;
  Writer* client_writer_;
  MetaData* response_headers_;
};

}  // namespace

CacheUrlAsyncFetcher::~CacheUrlAsyncFetcher() {
}

void CacheUrlAsyncFetcher::StreamingFetch(
    const std::string& url, const MetaData& request_headers,
    MetaData* response_headers, Writer* writer, MessageHandler* handler,
    Callback* callback) {
  if (http_cache_->Get(url.c_str(), response_headers, writer, handler)) {
    callback->Done(true);
  } else {
    ForwardingAsyncFetch* fetch = new ForwardingAsyncFetch(
        url, http_cache_, handler, callback, writer, response_headers);
    fetch->Start(fetcher_, request_headers);
  }
}

}  // namespace net_instaweb