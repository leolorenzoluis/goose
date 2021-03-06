// Copyright 2015 Google Inc. All rights reserved.
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
package com.google.devtools.build.lib.query2;

import static com.google.devtools.build.lib.packages.Type.BOOLEAN;
import static com.google.devtools.build.lib.packages.Type.TRISTATE;

import com.google.common.base.Preconditions;
import com.google.common.collect.ImmutableList;
import com.google.devtools.build.lib.packages.AggregatingAttributeMapper;
import com.google.devtools.build.lib.packages.Attribute;
import com.google.devtools.build.lib.packages.NonconfigurableAttributeMapper;
import com.google.devtools.build.lib.packages.Rule;
import com.google.devtools.build.lib.packages.Target;
import com.google.devtools.build.lib.packages.TargetUtils;
import com.google.devtools.build.lib.packages.Type;
import com.google.devtools.build.lib.query2.engine.QueryEnvironment.TargetAccessor;
import com.google.devtools.build.lib.query2.engine.QueryEnvironment.TargetNotFoundException;
import com.google.devtools.build.lib.query2.engine.QueryException;
import com.google.devtools.build.lib.query2.engine.QueryExpression;
import com.google.devtools.build.lib.syntax.Label;

import java.util.ArrayList;
import java.util.List;

/**
 * Implementation of {@link TargetAccessor&lt;Target&gt;} that uses an
 * {@link AbstractBlazeQueryEnvironment &lt;Target&gt;} internally to report issues and resolve
 * targets.
 */
final class BlazeTargetAccessor implements TargetAccessor<Target> {
  private final AbstractBlazeQueryEnvironment<Target> queryEnvironment;

  BlazeTargetAccessor(AbstractBlazeQueryEnvironment<Target> queryEnvironment) {
    this.queryEnvironment = queryEnvironment;
  }

  @Override
  public String getTargetKind(Target target) {
    return target.getTargetKind();
  }

  @Override
  public String getLabel(Target target) {
    return target.getLabel().toString();
  }

  @Override
  public List<Target> getLabelListAttr(QueryExpression caller, Target target, String attrName,
      String errorMsgPrefix) throws QueryException {
    Preconditions.checkArgument(target instanceof Rule);

    List<Target> result = new ArrayList<>();
    Rule rule = (Rule) target;

    AggregatingAttributeMapper attrMap = AggregatingAttributeMapper.of(rule);
    Type<?> attrType = attrMap.getAttributeType(attrName);
    if (attrType == null) {
      // Return an empty list if the attribute isn't defined for this rule.
      return ImmutableList.of();
    }
    for (Object value : attrMap.visitAttribute(attrName, attrType)) {
      // Computed defaults may have null values.
      if (value != null) {
        for (Label label : attrType.getLabels(value)) {
          try {
            result.add(queryEnvironment.getTarget(label));
          } catch (TargetNotFoundException e) {
            queryEnvironment.reportBuildFileError(caller, errorMsgPrefix + e.getMessage());
          }
        }
      }
    }

    return result;
  }

  @Override
  public List<String> getStringListAttr(Target target, String attrName) {
    Preconditions.checkArgument(target instanceof Rule);
    return NonconfigurableAttributeMapper.of((Rule) target).get(attrName, Type.STRING_LIST);
  }

  @Override
  public String getStringAttr(Target target, String attrName) {
    Preconditions.checkArgument(target instanceof Rule);
    return NonconfigurableAttributeMapper.of((Rule) target).get(attrName, Type.STRING);
  }

  @Override
  public Iterable<String> getAttrAsString(Target target, String attrName) {
    Preconditions.checkArgument(target instanceof Rule);
    List<String> values = new ArrayList<>(); // May hold null values.
    Attribute attribute = ((Rule) target).getAttributeDefinition(attrName);
    if (attribute != null) {
      Type<?> attributeType = attribute.getType();
      for (Object attrValue : AggregatingAttributeMapper.of((Rule) target).visitAttribute(
          attribute.getName(), attributeType)) {

        // Ugly hack to maintain backward 'attr' query compatibility for BOOLEAN and TRISTATE
        // attributes. These are internally stored as actual Boolean or TriState objects but were
        // historically queried as integers. To maintain compatibility, we inspect their actual
        // value and return the integer equivalent represented as a String. This code is the
        // opposite of the code in BooleanType and TriStateType respectively.
        if (attributeType == BOOLEAN) {
          values.add(Type.BOOLEAN.cast(attrValue) ? "1" : "0");
        } else if (attributeType == TRISTATE) {
            switch (Type.TRISTATE.cast(attrValue)) {
              case AUTO :
                values.add("-1");
                break;
              case NO :
                values.add("0");
                break;
              case YES :
                values.add("1");
                break;
              default :
                throw new AssertionError("This can't happen!");
            }
        } else {
          values.add(attrValue == null ? null : attrValue.toString());
        }
      }
    }
    return values;
  }

  @Override
  public boolean isRule(Target target) {
    return target instanceof Rule;
  }

  @Override
  public boolean isTestRule(Target target) {
    return TargetUtils.isTestRule(target);
  }

  @Override
  public boolean isTestSuite(Target target) {
    return TargetUtils.isTestSuiteRule(target);
  }
}
