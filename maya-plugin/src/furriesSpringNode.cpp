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
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnNumericData.h>
#include <maya/MFnUnitAttribute.h>

#include <maya/MDoubleArray.h>
#include <maya/MMatrix.h>
#include <maya/MTime.h>
#include <maya/MFloatMatrix.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MArrayDataBuilder.h>


#include "furriesSpringNode.h"

#define PERRORfail(stat,msg) \
                                                                if (!(stat)) { \
                                                                        stat.perror((msg)); \
                                                                        return (stat); \
                                                                }
                                                                
MString FurriesSpringNode::name = "furrySpringNode";
MTypeId FurriesSpringNode::id(0x00001);

MObject FurriesSpringNode::meshInput;
MObject FurriesSpringNode::matrixInput;
MObject FurriesSpringNode::timeInput;
MObject FurriesSpringNode::gravityInput;
MObject FurriesSpringNode::stiffnessInput;
MObject FurriesSpringNode::springAnglesInput;

MObject FurriesSpringNode::outputSpringPositions;
MObject FurriesSpringNode::outputSpringAngles;

FurriesSpringNode::FurriesSpringNode() {}
FurriesSpringNode::~FurriesSpringNode() {}

MStatus FurriesSpringNode::initialize() {
  MStatus status;
  MFnTypedAttribute typedAttr;
  MFnMatrixAttribute  matrixAttr;
  MFnNumericAttribute numericAttr;
  MFnUnitAttribute unitAttr;

  //  Inputs
  FurriesSpringNode::meshInput = typedAttr.create("inputMesh", "in",
  MFnData::kMesh, &status);
  typedAttr.setWritable(true);
  addAttribute(meshInput);

  FurriesSpringNode::matrixInput = matrixAttr.create("inputMatrix", "mat");
  matrixAttr.setWritable(true);
  addAttribute(matrixInput);

  FurriesSpringNode::stiffnessInput = numericAttr.create("springStiffness", "stiff", MFnNumericData::kFloat, 1.0);
  numericAttr.setWritable(true);
  addAttribute(stiffnessInput);

  FurriesSpringNode::gravityInput = numericAttr.create("gravity", "grav", MFnNumericData::kFloat, 9.8);
  numericAttr.setWritable(true);
  addAttribute(gravityInput);

  FurriesSpringNode::timeInput = unitAttr.create("inputTime", "time",
      MFnUnitAttribute::kTime);
  unitAttr.setWritable(true);
  addAttribute(timeInput);

  FurriesSpringNode::springAnglesInput = numericAttr.create("inputSpringAngles", "anglesIn", MFnNumericData::k3Double);
  numericAttr.setWritable(true);
  numericAttr.setArray(true);
  numericAttr.setUsesArrayDataBuilder(true);
  addAttribute(springAnglesInput);

  // Output
  FurriesSpringNode::outputSpringAngles = numericAttr.create("springAngles", "angles", MFnNumericData::k3Double);
  numericAttr.setWritable(false);
  numericAttr.setReadable(true);
  numericAttr.setArray(true);
  numericAttr.setUsesArrayDataBuilder(true);
  addAttribute(outputSpringAngles);

  FurriesSpringNode::outputSpringPositions = numericAttr.create("springPositions", "positions", MFnNumericData::k3Double);
  numericAttr.setWritable(false);
  numericAttr.setReadable(true);
  numericAttr.setArray(true);
  numericAttr.setUsesArrayDataBuilder(true);
  addAttribute(outputSpringPositions);

  //Affecting attributes
  status = attributeAffects(meshInput, outputSpringAngles);
  status = attributeAffects(meshInput, outputSpringPositions);

  status = attributeAffects(matrixInput, outputSpringPositions);
  status = attributeAffects(matrixInput, outputSpringAngles);

  status = attributeAffects(timeInput, outputSpringAngles);
  status = attributeAffects(timeInput, outputSpringPositions);

  status = attributeAffects(stiffnessInput, outputSpringAngles);
  status = attributeAffects(stiffnessInput, outputSpringPositions);

  status = attributeAffects(gravityInput, outputSpringAngles);
  status = attributeAffects(gravityInput, outputSpringPositions);

  return MStatus::kSuccess;
}

MStatus FurriesSpringNode::compute(const MPlug& plug, MDataBlock& data) {
  MStatus status = MStatus::kSuccess;

  if(plug == outputSpringPositions || plug == outputSpringAngles) {
    MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
    MObject inputMeshObject(inputMeshHandle.asMesh());
    MFnMesh inputMesh(inputMeshObject);

    MTime currentTime = data.inputValue(timeInput, &status).asTime();

    MTransformationMatrix matrix = data.inputValue(matrixInput, &status).asMatrix();

    MFloatPointArray vertices;
    MFloatVectorArray normals;
    inputMesh.getPoints(vertices, MSpace::kWorld);
    unsigned int springCount = vertices.length();
    inputMesh.getVertexNormals(false, normals, MSpace::kWorld);

    MArrayDataHandle outputPositions = data.outputArrayValue( FurriesSpringNode::outputSpringPositions);

    MArrayDataBuilder positionBuilder(FurriesSpringNode::outputSpringPositions, springCount);

    for(unsigned int i = 0; i < springCount; i++) {
      MDataHandle outPoint  = positionBuilder.addLast();
      MPoint point;
      point.x = vertices[i].x;
      point.y = vertices[i].y;
      point.z = vertices[i].z;
      point.w = 1.0;

      point = point * matrix.asMatrix();
      outPoint.set3Double(point.x, point.y, point.z);
    }


    //angle calculations goes here
    MArrayDataHandle outputAngles = data.outputArrayValue( FurriesSpringNode::outputSpringAngles);
    MArrayDataBuilder angleBuilder(FurriesSpringNode::outputSpringAngles, springCount);

    MArrayDataHandle inputAngles = data.inputValue( springAnglesInput);

    for(unsigned int i = 0; i < springCount; i++) {

      MFloatVector inAngle;
      bool foundAngle = false;
      if(inputAngles.elementCount() > 0 && inputAngles.elementIndex() == i) {
        inAngle = inputAngles.inputValue().asDouble3();
        inputAngles.next();
        foundAngle = true;
      }

      MDataHandle outangle  = angleBuilder.addLast();
      MFloatVector normal = normals[i];
      MFloatVector up(0, 1.0, 0);

      MMatrix m = matrix.asRotateMatrix();
      normal = MPoint(normal) * m;
      MQuaternion q(up, normal);

      if(currentTime.value() <= 1) {
        MFloatVector angles = q.asEulerRotation().asVector();
        angles *= 180/3.14;
        outangle.set3Double(angles.x, angles.y, angles.z);
      }
      else if(foundAngle) {
        outangle.set3Double(inAngle.x, inAngle.y, inAngle.z+sin(currentTime.value()*0.1));
      }
    }

    outputPositions.set(positionBuilder);
    outputAngles.set(angleBuilder);
    data.setClean(plug);
  }

  return status;
}
