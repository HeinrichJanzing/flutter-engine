// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell_test_external_view_embedder.h"

namespace flutter {

ShellTestExternalViewEmbedder::ShellTestExternalViewEmbedder(
    const EndFrameCallBack& end_frame_call_back,
    PostPrerollResult post_preroll_result,
    bool support_thread_merging)
    : end_frame_call_back_(end_frame_call_back),
      post_preroll_result_(post_preroll_result),
      support_thread_merging_(support_thread_merging),
      submitted_frame_count_(0) {}

void ShellTestExternalViewEmbedder::UpdatePostPrerollResult(
    PostPrerollResult post_preroll_result) {
  post_preroll_result_ = post_preroll_result;
}

int ShellTestExternalViewEmbedder::GetSubmittedFrameCount() {
  return submitted_frame_count_;
}

SkISize ShellTestExternalViewEmbedder::GetLastSubmittedFrameSize() {
  return last_submitted_frame_size_;
}

std::vector<int64_t> ShellTestExternalViewEmbedder::GetVisitedPlatformViews() {
  return visited_platform_views_;
}

MutatorsStack ShellTestExternalViewEmbedder::GetStack(int64_t view_id) {
  return mutators_stacks_[view_id];
}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::CancelFrame() {}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::BeginFrame(
    SkISize frame_size,
    GrDirectContext* context,
    double device_pixel_ratio,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::PrerollCompositeEmbeddedView(
    int view_id,
    std::unique_ptr<EmbeddedViewParams> params) {
  SkRect view_bounds = SkRect::Make(frame_size_);
  std::unique_ptr<EmbedderViewSlice> view;
  if (params->display_list_enabled()) {
    view = std::make_unique<DisplayListEmbedderViewSlice>(view_bounds);
  } else {
    view = std::make_unique<SkPictureEmbedderViewSlice>(view_bounds);
  }
  slices_.insert_or_assign(view_id, std::move(view));
}

// |ExternalViewEmbedder|
PostPrerollResult ShellTestExternalViewEmbedder::PostPrerollAction(
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  FML_DCHECK(raster_thread_merger);
  return post_preroll_result_;
}

// |ExternalViewEmbedder|
std::vector<SkCanvas*> ShellTestExternalViewEmbedder::GetCurrentCanvases() {
  return {};
}

// |ExternalViewEmbedder|
std::vector<DisplayListBuilder*>
ShellTestExternalViewEmbedder::GetCurrentBuilders() {
  return {};
}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::PushVisitedPlatformView(int64_t view_id) {
  visited_platform_views_.push_back(view_id);
}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::PushFilterToVisitedPlatformViews(
    std::shared_ptr<const DlImageFilter> filter) {
  for (int64_t id : visited_platform_views_) {
    EmbeddedViewParams params = current_composition_params_[id];
    params.PushImageFilter(filter);
    current_composition_params_[id] = params;
    mutators_stacks_[id] = params.mutatorsStack();
  }
}

EmbedderPaintContext ShellTestExternalViewEmbedder::CompositeEmbeddedView(
    int view_id) {
  return {slices_[view_id]->canvas(), slices_[view_id]->builder()};
}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::SubmitFrame(
    GrDirectContext* context,
    std::unique_ptr<SurfaceFrame> frame) {
  if (!frame) {
    return;
  }
  frame->Submit();
  if (frame->SkiaSurface()) {
    last_submitted_frame_size_ = SkISize::Make(frame->SkiaSurface()->width(),
                                               frame->SkiaSurface()->height());
  } else {
    last_submitted_frame_size_ = SkISize::MakeEmpty();
  }
  submitted_frame_count_++;
}

// |ExternalViewEmbedder|
void ShellTestExternalViewEmbedder::EndFrame(
    bool should_resubmit_frame,
    fml::RefPtr<fml::RasterThreadMerger> raster_thread_merger) {
  end_frame_call_back_(should_resubmit_frame, raster_thread_merger);
}

// |ExternalViewEmbedder|
SkCanvas* ShellTestExternalViewEmbedder::GetRootCanvas() {
  return nullptr;
}

bool ShellTestExternalViewEmbedder::SupportsDynamicThreadMerging() {
  return support_thread_merging_;
}

}  // namespace flutter
