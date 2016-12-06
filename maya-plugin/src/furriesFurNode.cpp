
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
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MArrayDataBuilder.h>

#include "furriesFurNode.h"


MString FurriesFurNode::name = "furryFurNode";
MTypeId FurriesFurNode::id(0x00002);
MObject FurriesFurNode::meshInput;

MObject FurriesFurNode::output;

FurriesFurNode::FurriesFurNode() {}
FurriesFurNode::~FurriesFurNode() {}

MFloatVector arrToVec(double theArray[]) {
  MFloatVector vec;
  vec.x = theArray[0];
  vec.y = theArray[1];
  vec.z = theArray[2];
  return vec;
}

MStatus FurriesFurNode::initialize() {
  MStatus status;
  MFnTypedAttribute typedAttr;
  MFnMatrixAttribute  matrixAttr;
  MFnNumericAttribute numericAttr;
  MFnUnitAttribute unitAttr;

  //  Inputs
  FurriesFurNode::meshInput = typedAttr.create("inputMesh", "in",
    MFnData::kMesh, &status);
  typedAttr.setWritable(true);

  // Output
  FurriesFurNode::output = numericAttr.create("output", "out", MFnNumericData::kFloat, 0.0);
  numericAttr.setWritable(false);


  //Add attributes
  // Inputs
  addAttribute(meshInput);

  // Outputs
  addAttribute(output);

  //Affecting attributes
  status = attributeAffects(meshInput, output);

  return MStatus::kSuccess;
}

MStatus FurriesFurNode::compute(const MPlug& plug, MDataBlock& data) {
  MStatus status = MStatus::kSuccess;

  MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
  MObject inputMeshObject(inputMeshHandle.asMesh());
  MFnMesh inputMesh(inputMeshObject);
  
  MIntArray triCount;
  MIntArray triVert;

  MFloatPointArray pointList;

  inputMesh.getPoints(pointList, MSpace::kWorld);

  inputMesh.getTriangles(triCount, triVert);

  double stepSize = 0.01;

  //MFloatPoint p1 = pointList[triVert[0]];
  //MFloatPoint p2 = pointList[triVert[1]];
  //MFloatPoint p3 = pointList[triVert[2]];
  //float dist = p1.distanceTo(p2);
  //cout << "NÄMEN VAD ÄR DET HÄR FÖR EN LITEN SKIT?" << endl;
  //cout << dist << endl;

  double ap0[4];
  double ap1[4];
  double ap2[4];

  MFloatVector p0;
  MFloatVector p1;
  MFloatVector p2;
  MFloatVector p3;
  MFloatVector p4;
  MFloatVector p2p0;
  MFloatVector p2p1;
  MFloatVector p0p1;
  MFloatVector p1p0;
  MFloatVector p2p3;
  MFloatVector p2p4;

  for (int i = 0; i < triCount.length(); i++) {
    pointList[triVert[3 * i]].get(ap0);
    pointList[triVert[(3 * i) + 1]].get(ap1);
    pointList[triVert[(3 * i) + 2]].get(ap2);
    p0 = arrToVec(ap0);
    p1 = arrToVec(ap1);
    p2 = arrToVec(ap2);

    p2p0 = p2 - p0;
    p2p0.normalize();
    p2p1 = p2 - p1;
    p2p1.normalize();
    p0p1 = p0 - p1;
    p0p1.normalize();
    p1p0 = p1 - p0;
    p1p0.normalize();
    
    p3 = p0 + (p2p0 * (stepSize/2)) + (p1p0 * (stepSize / 2));
    p4 = p1 + (p2p1 * (stepSize / 2)) + (p0p1 * (stepSize / 2));
    for (size_t i = 0; (i*stepSize) < (p0 - p2).length(); i++)
    {
      p2p3 = p2 - p3;
      p2p3.normalize();
      p2p4 = p2 - p4;
      p2p4.normalize();

      p3 = p2p3 * (stepSize * i);
      p4 = p2p4 * (stepSize * i);
      for (int k = 0; (k*stepSize) < (p4-p3).length(); k++) {
        // Add nurbcurve at p3 + ((p4-p3).normalize() * k * stepSize)
      }
    }

    (stepSize / 2) + stepSize * i;
  }

  return status;
}
