// Copyright 2010 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef HTML_REWRITER_APR_FILE_SYSTEM_H_
#define HTML_REWRITER_APR_FILE_SYSTEM_H_

#include "net/instaweb/htmlparse/public/file_system.h"
#include "net/instaweb/htmlparse/public/message_handler.h"

struct apr_pool_t;
using net_instaweb::FileSystem;
using net_instaweb::MessageHandler;


namespace html_rewriter {

class AprFileSystem : public FileSystem {
 public:
  explicit AprFileSystem(apr_pool_t* pool);
  ~AprFileSystem();

  virtual InputFile* OpenInputFile(
      const char* file, MessageHandler* message_handler);
  virtual OutputFile* OpenOutputFile(
      const char* file, MessageHandler* message_handler);

 private:
  apr_pool_t* pool_;
};

}  // namespace html_rewriter

#endif  // HTML_REWRITER_APR_FILE_SYSTEM_H_