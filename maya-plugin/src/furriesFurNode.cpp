
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

  for (int i = 0; i < triCount.length; i++) {
    MFloatPoint p0 = pointList[triVert[3 * i]];
    MFloatPoint p1 = pointList[triVert[(3 * i) + 1]];
    MFloatPoint p2 = pointList[triVert[(3 * i) + 2]];
  }

  return status;
}
