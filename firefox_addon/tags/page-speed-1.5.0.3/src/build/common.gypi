# Copyright 2009 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'includes': [
    '../third_party/chromium/src/build/common.gypi',
  ],
  'target_defaults': {
    'defines': [
      'PAGESPEED_GYP_BUILD',
    ],
    'conditions': [
      ['OS == "linux"', {
        'cflags': [
          # We're building a shared library, so everything needs to be built
          # with Position-Independent Code.
          '-fPIC',
        ],
        # As of r30253, Chromium's src/build/common.gypi turns on
        # -fvisibility=hidden under certain conditions.  However, that breaks
        # our build for some reason, so the setting below turns it back off.  A
        # better fix for the future might be to add visibility pragmas to our
        # code, or something.  (mdsteele)
        'cflags!': [
          '-fvisibility=hidden',
        ],
        'scons_variable_settings': {
          # Hack to disable flock, which is not available on older
          # versions of Ubuntu.
          'FLOCK_LINK!': ['flock', '$TOP_BUILDDIR/linker.lock'],
          'FLOCK_SHLINK!': ['flock', '$TOP_BUILDDIR/linker.lock'],
          'FLOCK_LDMODULE!': ['flock', '$TOP_BUILDDIR/linker.lock'],
        },
      }],
      ['OS == "mac"', {
        'xcode_settings': {
          'OTHER_CFLAGS': [
            '-fPIC',  # See note for -fPIC above.
          ],
          # We must build for 10.4 for compatibility with Firefox.
          'MACOSX_DEPLOYMENT_TARGET': '10.4',
          # This is equivalent to turning off -fvisibility-hidden, as above.
          'GCC_SYMBOLS_PRIVATE_EXTERN': 'NO',
        },
      }],
    ],
  },
}
