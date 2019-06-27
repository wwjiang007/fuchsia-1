// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_UI_EXAMPLES_ESCHER_WATERFALL_SCENES_APP_TEST_SCENE_H_
#define SRC_UI_EXAMPLES_ESCHER_WATERFALL_SCENES_APP_TEST_SCENE_H_

#include "src/lib/fxl/macros.h"
#include "src/ui/lib/escher/gl/mesh.h"
#include "src/ui/lib/escher/scene/model.h"
#include "src/ui/lib/escher/scene/viewing_volume.h"

class AppTestScene {
 public:
  AppTestScene();
  ~AppTestScene();

  // Initialize once OpenGL context is available.
  void InitGL();

  escher::Model GetModel(const escher::ViewingVolume& volume, const escher::vec2& focus);

 private:
  escher::Material app_bar_material_;
  escher::Material canvas_material_;
  escher::Material card_material_;
  escher::Material fab_material_;
  escher::Material green_material_;
  escher::Material checkerboard_material_;
  escher::Material null_material_;
  fxl::RefPtr<escher::Mesh> circle_mesh_;

  FXL_DISALLOW_COPY_AND_ASSIGN(AppTestScene);
};

#endif  // SRC_UI_EXAMPLES_ESCHER_WATERFALL_SCENES_APP_TEST_SCENE_H_
