
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
#include <maya/MPointArray.h>
#include <maya/MFloatPoint.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnNurbsCurveData.h>
#include <maya/MGlobal.h>

#include "furriesFurNode.h"

#define PERRORfail(stat,msg) \
                                                                if (!(stat)) { \
                                                                        stat.perror((msg)); \
                                                                        return (stat); \
                                                                }

MString FurriesFurNode::name = "furryFurNode";
MTypeId FurriesFurNode::id(0x00002);
MObject FurriesFurNode::meshInput;

MObject FurriesFurNode::outputCurves;

FurriesFurNode::FurriesFurNode() {}
FurriesFurNode::~FurriesFurNode() {}

MStatus FurriesFurNode::initialize() {
  MStatus status;
  MFnTypedAttribute typedAttr;
  MFnMatrixAttribute  matrixAttr;
  MFnNumericAttribute numericAttr;
  MFnUnitAttribute unitAttr;
  MGlobal::displayInfo("FURRIES BURRIES");

  //  Inputs
  FurriesFurNode::meshInput = typedAttr.create("inputMesh", "in", MFnData::kMesh, &status);
  typedAttr.setWritable(true);

  // Output
  FurriesFurNode::outputCurves = typedAttr.create( "outputCurves", "oc", MFnData::kNurbsCurve, &status );

  //Add attributes
  // Inputs
  addAttribute(meshInput);

  // Outputs
  addAttribute(outputCurves);

  //Affecting attributes
  status = attributeAffects(meshInput, outputCurves);

  return MStatus::kSuccess;
}

MStatus FurriesFurNode::createHairCurve(MFloatPoint pos, MDataBlock& data ){
  
  MStatus stat;
  int numCurves=1;
  MArrayDataHandle outputArray = data.outputArrayValue(outputCurves,&stat);
  MArrayDataBuilder builder(outputCurves, numCurves, &stat);
  PERRORfail(stat, "BUILDER");
  for (int curveNum = 0; curveNum < numCurves; curveNum++) {
    MDataHandle outHandle = builder.addElement(curveNum);

    
    MFnNurbsCurveData dataCreator;
    MObject outCurveData = dataCreator.create();

    double k[] = {1.0,1.0,1.0,1.0,1.0};
    MDoubleArray knots(k, 5);
    MPointArray cvs;
    cvs.append(MPoint(0,0,0));
    cvs.append(MPoint(0,0,0.25));
    cvs.append(MPoint(0,0,0.5));
    cvs.append(MPoint(0,0,0.75));
    cvs.append(MPoint(0,0,1));

    MFnNurbsCurve curve;
    MObject newCurve = curve.create(cvs,knots,1, MFnNurbsCurve::Form::kOpen, false, true, outCurveData,&stat);
    cout << "Created curve with status" << stat << endl;
    outHandle.set(outCurveData);

    PERRORfail(stat, "HEJ");
  }
  stat = outputArray.set(builder);
  return stat;
}

MStatus FurriesFurNode::compute(const MPlug& plug, MDataBlock& data) {
  MGlobal::displayInfo("FURRIES"); 
  cout << "COMPUTE!!!!";
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
  //cout << "N�MEN VAD �R DET H�R F�R EN LITEN SKIT?" << endl;
  //cout << dist << endl;
/*
  for (int i = 0; i < triCount.length(); i++) {
    MFloatPoint p0 = pointList[triVert[3 * i]];
    MFloatPoint p1 = pointList[triVert[(3 * i) + 1]];
    MFloatPoint p2 = pointList[triVert[(3 * i) + 2]];
  }*/

  MFloatPoint p0(0,1,0);
  createHairCurve(p0, data);
  return status;
}
