/**
* @file furriesSpringNode.cpp
* @author Erik Sandrén
* @date 29-11-2016
* @brief [Description Goes Here]
*/

#include <maya/MFnTypedAttribute.h>
#include <maya/MFnComponentListData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MArrayDataBuilder.h>

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnUnitAttribute.h>

#include "furriesSpringNode.h"


MTypeId FurriesSpringNode::id(0x00001);
MObject FurriesSpringNode::meshInput;
MObject FurriesSpringNode::timeInput;
//MObject FurriesSpringNode::gravityInput;
//MObject FurriesSpringNode::stiffnessInput;

MObject FurriesSpringNode::output;
MObject FurriesSpringNode::outputSpringPositions;
//MObject FurriesSpringNode::outputSpringAngles;

FurriesSpringNode::FurriesSpringNode() {}
FurriesSpringNode::~FurriesSpringNode() {}

MStatus FurriesSpringNode::initialize() {
  MStatus status;
  MFnTypedAttribute typedAttr;
  MFnNumericAttribute numericAttr;
  MFnUnitAttribute unitAttr;

  //  Inputs
  FurriesSpringNode::meshInput = typedAttr.create("inputMesh", "in",
  MFnData::kMesh, &status);
  typedAttr.setWritable(true);

  //FurriesSpringNode::stiffnessInput = numericAttr.create("springStiffness", "stiff", MFnNumericData::kFloat, 1.0);
  //FurriesSpringNode::gravityInput = numericAttr.create("gravity", "grav", MFnNumericData::kFloat, 9.8);
  FurriesSpringNode::timeInput = unitAttr.create("inputTime", "time",
      MFnUnitAttribute::kTime);
  //numericAttr.setWritable(true);

  // Output
  FurriesSpringNode::output = numericAttr.create("output", "out", MFnNumericData::kFloat, 0.0);
  numericAttr.setWritable(false);

  //FurriesSpringNode::outputSpringAngles = typedAttr.create("springAngles", "angles", MFnData::kVectorArray);
  //typedAttr.setWritable(false);
  //typedAttr.setArray(true);
  //typedAttr.setUsesArrayDataBuilder(true);
  FurriesSpringNode::outputSpringPositions = numericAttr.create("springPositions", "positions", MFnNumericData::k3Double, (0.0, 0.0, 0.0));
  numericAttr.setReadable(true);
  numericAttr.setArray(true);
  numericAttr.setUsesArrayDataBuilder(true);

  //Add attributes
  // Inputs
  //addAttribute(stiffnessInput);
  //addAttribute(gravityInput);
  addAttribute(timeInput);
  addAttribute(meshInput);

  // Outputs
  addAttribute(output);
  addAttribute(outputSpringPositions);
  //addAttribute(outputSpringAngles);

  //Affecting attributes
  status = attributeAffects(meshInput, output);
  //status = attributeAffects(meshInput, outputSpringAngles);
  status = attributeAffects(meshInput, outputSpringPositions);

  status = attributeAffects(timeInput, output);
  //status = attributeAffects(timeInput, outputSpringAngles);
  status = attributeAffects(timeInput, outputSpringPositions);

  return MStatus::kSuccess;
}

MStatus FurriesSpringNode::compute(const MPlug& plug, MDataBlock& data) {
  MStatus status = MStatus::kSuccess;

  if(plug == outputSpringPositions) {
    MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
    MObject inputMeshObject( inputMeshHandle.asMesh() );
    MFnMesh inputMesh(inputMeshObject);

    MFloatPointArray vertices;
    inputMesh.getPoints(vertices, MSpace::kWorld);
    unsigned int springCount = vertices.length();

    cout << "spring count: " << springCount << endl;
    MArrayDataHandle outputPositions = data.outputArrayValue( FurriesSpringNode::outputSpringPositions);

    MArrayDataBuilder positionBuilder(FurriesSpringNode::outputSpringPositions, springCount);

    for(unsigned int i = 0; i < springCount; i++) {
      MDataHandle outPoint  = positionBuilder.addLast();
      outPoint.set3Double(vertices[i].x, vertices[i].y, vertices[i].z);
    }

    outputPositions.set(positionBuilder);
    data.setClean(plug);
  }

  if(plug == output) {
    MFloatPointArray vertices;
    MFloatVectorArray normals;

    MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
    MObject inputMeshObject( inputMeshHandle.asMesh() );
    MFnMesh inputMesh(inputMeshObject);

    //MDataHandle inputTimeHandle = data.inputValue(timeInput, &status);

    inputMesh.getPoints(vertices, MSpace::kWorld);
    inputMesh.getVertexNormals(false, normals, MSpace::kWorld);

    MDataHandle outputHandle = data.outputValue( FurriesSpringNode::output);
    outputHandle.setFloat(vertices.length());
    data.setClean(plug);
  }

  return status;
}
