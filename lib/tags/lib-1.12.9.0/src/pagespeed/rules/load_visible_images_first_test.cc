// Copyright 2011 Google Inc. All Rights Reserved.
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

#include <string>

#include "base/scoped_ptr.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/load_visible_images_first.h"
#include "pagespeed/testing/pagespeed_test.h"

using pagespeed::rules::LoadVisibleImagesFirst;
using pagespeed_testing::FakeDomDocument;
using pagespeed_testing::FakeDomElement;
using pagespeed_testing::PagespeedRuleTest;

namespace {

class LoadVisibleImagesFirstTest
    : public PagespeedRuleTest<LoadVisibleImagesFirst> {
 protected:
  static const char* kRootUrl;
  static const char* kIframeUrl;
  static const char* kImg1Url;
  static const char* kImg2Url;
  static const char* kAboveTheFoldUrl;

  static const int kEarlyResourceLoadTimeMillis;
  static const int kLateResourceLoadTimeMillis;
  static const int kLatestResourceLoadTimeMillis;
  static const int kOnloadMillis;

  virtual void DoSetUp() {
    SetViewportWidthAndHeight(1024, 768);
    NewPrimaryResource(kRootUrl);
    CreateHtmlHeadBodyElements();
    SetOnloadTimeMillis(kOnloadMillis);
    AddVisibleImageLoadedLate();
  }

  void AddImage(const char* url, FakeDomElement* parent,
                int x, int y, int width, int height,
                int request_start_time_millis) {
    FakeDomElement* img = NULL;
    NewPngResource(url, parent, &img)->SetRequestStartTimeMillis(
        request_start_time_millis);
    img->SetCoordinates(x, y);
    img->SetActualWidthAndHeight(width, height);
  }

  void AddImage(const char* url, int x, int y, int width, int height,
                int request_start_time_millis) {
    AddImage(url, body(), x, y, width, height, request_start_time_millis);
  }

  void AddVisibleImage(int request_start_time_millis) {
    AddImage(kAboveTheFoldUrl, 5, 5, 10, 10, request_start_time_millis);
  }

  void AddVisibleImageLoadedLate() {
    AddVisibleImage(kLateResourceLoadTimeMillis);
  }
};

const char* LoadVisibleImagesFirstTest::kRootUrl = "http://test.com/";
const char* LoadVisibleImagesFirstTest::kIframeUrl =
    "http://test.com/frame.html";
const char* LoadVisibleImagesFirstTest::kImg1Url =
    "http://test.com/a.png";
const char* LoadVisibleImagesFirstTest::kImg2Url =
    "http://test.com/b.png";
const char* LoadVisibleImagesFirstTest::kAboveTheFoldUrl =
    "http://test.com/atf.png";
const int LoadVisibleImagesFirstTest::kOnloadMillis = 100;
const int LoadVisibleImagesFirstTest::kEarlyResourceLoadTimeMillis = 1;
const int LoadVisibleImagesFirstTest::kLateResourceLoadTimeMillis =
    kOnloadMillis - 2;
const int LoadVisibleImagesFirstTest::kLatestResourceLoadTimeMillis =
    kLateResourceLoadTimeMillis + 1;

TEST_F(LoadVisibleImagesFirstTest, EmptyDom) {
  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, ImageMissingDimensions) {
  FakeDomElement* img1 = NULL;
  NewPngResource(kImg1Url, body(), &img1);
  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, ImageAboveTheFold) {
  AddImage(kImg1Url, 0, 0, 10, 10, kEarlyResourceLoadTimeMillis);
  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, ImageAboveTheFoldNoWidth) {
  AddImage(kImg1Url, 0, 0, 0, 10, kEarlyResourceLoadTimeMillis);
  CheckOneUrlViolation(kImg1Url);
}

TEST_F(LoadVisibleImagesFirstTest, ImageBelowTheFold) {
  AddImage(kImg1Url, 0, 768, 10, 10, kEarlyResourceLoadTimeMillis);
  CheckOneUrlViolation(kImg1Url);
}

TEST_F(LoadVisibleImagesFirstTest, TwoImagesBelowTheFold) {
  AddImage(kImg1Url, 0, 768, 10, 10, kEarlyResourceLoadTimeMillis);
  AddImage(kImg2Url, 0, 1000, 10, 10, kEarlyResourceLoadTimeMillis);
  CheckTwoUrlViolations(kImg1Url, kImg2Url);
}

TEST_F(LoadVisibleImagesFirstTest, ImageOverlappingTheFold) {
  AddImage(kImg1Url, 0, 760, 10, 10, kEarlyResourceLoadTimeMillis);
  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, SameImageAboveAndBelowTheFold) {
  AddImage(kImg1Url, 0, 768, 10, 10, kEarlyResourceLoadTimeMillis);

  FakeDomElement* img2 = FakeDomElement::NewImg(body(), kImg1Url);
  img2->SetCoordinates(0, 0);
  img2->SetActualWidthAndHeight(10, 10);

  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, OneImageVisibleOneNotVisible) {
  AddImage(kImg1Url, 1024, 100, 10, 10, kEarlyResourceLoadTimeMillis);
  AddImage(kImg2Url, 100, 100, 10, 10, kEarlyResourceLoadTimeMillis);
  CheckOneUrlViolation(kImg1Url);
}

TEST_F(LoadVisibleImagesFirstTest,
       ImageBelowTheFoldBeforeOnloadAfterLatestAboveTheFold) {
  AddImage(kImg1Url, 1024, 100, 10, 10, kLatestResourceLoadTimeMillis);
  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, ImageBelowTheFoldAfterOnload) {
  AddImage(kImg1Url, 1024, 100, 10, 10, kOnloadMillis + 1);
  CheckNoViolations();
}

TEST_F(LoadVisibleImagesFirstTest, RedirectedImage) {
  FakeDomElement* img1 = NULL;
  NewRedirectedPngResource(
      kImg1Url, kImg2Url, body(), &img1)->SetRequestStartTimeMillis(
          kEarlyResourceLoadTimeMillis);
  img1->SetCoordinates(1024, 100);
  img1->SetActualWidthAndHeight(10, 10);
  CheckOneUrlViolation(kImg2Url);
}

TEST_F(LoadVisibleImagesFirstTest, ImageInIframe) {
  FakeDomElement* iframe = FakeDomElement::NewIframe(body());
  iframe->SetCoordinates(200, 200);
  iframe->SetActualWidthAndHeight(200, 200);
  FakeDomDocument* iframe_doc = NULL;
  NewDocumentResource(kIframeUrl, iframe, &iframe_doc);
  FakeDomElement* html = FakeDomElement::NewRoot(iframe_doc, "html");

  // 200, 200 + 0, 0 = 200, 200, which is above the fold.
  AddImage(kImg1Url, html, 0, 0, 10, 10, kEarlyResourceLoadTimeMillis);

  // 200, 200 + 0, 700 = 200, 900, which is below the fold.
  AddImage(kImg2Url, html, 0, 700, 10, 10, kEarlyResourceLoadTimeMillis);

  CheckOneUrlViolation(kImg2Url);
}

}  // namespace
