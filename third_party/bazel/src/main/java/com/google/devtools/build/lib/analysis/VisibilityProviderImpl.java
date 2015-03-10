// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.devtools.build.lib.analysis;

import com.google.devtools.build.lib.collect.nestedset.NestedSet;
import com.google.devtools.build.lib.concurrent.ThreadSafety.Immutable;
import com.google.devtools.build.lib.packages.PackageSpecification;

/**
 * Visibility provider implementation.
 */
@Immutable
public final class VisibilityProviderImpl implements VisibilityProvider {
  private final NestedSet<PackageSpecification> visibility;

  public VisibilityProviderImpl(NestedSet<PackageSpecification> visibility) {
    this.visibility = visibility;
  }

  @Override
  public NestedSet<PackageSpecification> getVisibility() {
    return visibility;
  }
}
