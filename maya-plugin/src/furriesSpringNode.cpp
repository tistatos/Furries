/**
* @file furriesSpringNode.cpp
* @author Erik Sandrén
* @date 29-11-2016
* @brief [Description Goes Here]
*/

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnComponentListData.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>

#include "furriesSpringNode.h"


MTypeId FurriesSpringNode::id(0x00001);
MObject FurriesSpringNode::input;
MObject FurriesSpringNode::output;

FurriesSpringNode::FurriesSpringNode() {}
FurriesSpringNode::~FurriesSpringNode() {}

MStatus FurriesSpringNode::initialize() {
  MStatus status;
  MFnTypedAttribute typedAttr;
  MFnNumericAttribute numericAttr;

  FurriesSpringNode::input = typedAttr.create("inputMesh", "in",
  MFnData::kMesh, &status);

  output = numericAttr.create("output", "out", MFnNumericData::kFloat, 0.0);
  numericAttr.setWritable(false);
  numericAttr.setStorable(false);

  status = addAttribute(input);
    if(!status) { status.perror("input addAttribute"); return status; }

  status = addAttribute(output);
    if(!status) { status.perror("output addAttribute"); return status; }

  status = attributeAffects(input, output);
    if(!status) { status.perror("attributeAffects"); return status; }
  return status;
}

MStatus FurriesSpringNode::compute(const MPlug& plug, MDataBlock& data) {
  return MStatus::kSuccess;
}
