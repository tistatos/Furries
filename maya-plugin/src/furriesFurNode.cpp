
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
#include <maya/MDagModifier.h>

#include "furriesFurNode.h"

MString FurriesFurNode::name = "furryFurNode";
MTypeId FurriesFurNode::id(0x00002);

MObject FurriesFurNode::meshInput;
MObject FurriesFurNode::springInput;

MObject FurriesFurNode::outputCurves;
MObject FurriesFurNode::numberOfCurves;  
MObject FurriesFurNode::distanceBetweenStrands;
MObject FurriesFurNode::inputSpringAngles;
MObject FurriesFurNode::inputSpringPositions;

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
  MGlobal::displayInfo("FURRIES BURRIES");

  //  Inputs
  FurriesFurNode::meshInput = typedAttr.create("inputMesh", "in", MFnData::kMesh, &status);
  typedAttr.setWritable(true);
  addAttribute(meshInput);


  FurriesFurNode::distanceBetweenStrands = numericAttr.create("distanceBetweenStrands", "dist", MFnNumericData::kDouble, 0.1, &status);
  numericAttr.setWritable(true);
  numericAttr.setMin(0.0001);
  numericAttr.setMax(1.0000);
  addAttribute(distanceBetweenStrands);

  // Output
  FurriesFurNode::outputCurves = typedAttr.create( "outputCurves", "oc", MFnNurbsCurveData::kNurbsCurve, &status );
  CHECK_MSTATUS ( typedAttr.setArray( true ) );
  CHECK_MSTATUS ( typedAttr.setReadable( true ) );
  CHECK_MSTATUS ( typedAttr.setWritable( false ) );
  CHECK_MSTATUS ( typedAttr.setUsesArrayDataBuilder( true ) );

  FurriesFurNode::numberOfCurves = numericAttr.create( "numberOfCurves", "n", MFnNumericData::kInt, 0, &status );


  FurriesFurNode::inputSpringAngles = numericAttr.create("springAngles", "angles", MFnNumericData::k3Double);
  numericAttr.setWritable(true);
  numericAttr.setArray(true);
  numericAttr.setUsesArrayDataBuilder(true);
  addAttribute(inputSpringAngles);

  FurriesFurNode::inputSpringPositions = numericAttr.create("springPositions", "positions", MFnNumericData::k3Double);
  numericAttr.setWritable(true);
  numericAttr.setArray(true);
  numericAttr.setUsesArrayDataBuilder(true);
  addAttribute(inputSpringPositions);

  //Add attributes
  // Inputs

  // Outputs
  addAttribute(outputCurves);
  addAttribute(numberOfCurves);

  //Affecting attributes
  status = attributeAffects(meshInput, outputCurves);
  status = attributeAffects(meshInput, numberOfCurves);
  status = attributeAffects(inputSpringAngles, outputCurves);
  status = attributeAffects(distanceBetweenStrands, outputCurves);
  status = attributeAffects(distanceBetweenStrands, numberOfCurves);

  return MStatus::kSuccess;
}

MStatus FurriesFurNode::createHairCurve( MFloatPointArray positions,  MFloatPointArray normals, MFloatPointArray wArray, MDataBlock& data){

  MStatus stat;

  int numCurves=positions.length();

  MArrayDataHandle outputArray = data.outputArrayValue(outputCurves,&stat);
  MArrayDataBuilder builder(outputCurves, numCurves, &stat);

  for (int curveNum = 0; curveNum < numCurves; curveNum++) {

    MDataHandle outHandle = builder.addElement(curveNum);

    MFnNurbsCurveData dataCreator;
    MObject outCurveData = dataCreator.create();

    double k[] = {0.0, 0.0, 0.0, 1.0, 2.0, 2.0, 2.0};
    MDoubleArray knots(k, 7);
    MVector w = wArray[curveNum];
    MPoint pos = MPoint(positions[curveNum]);
    MPoint normal = MPoint(normals[curveNum]);

    MVector z = w.normal();
    double theta = w.length();
    double length = 1.0;
    double lxy = 0.2;
    MVector x = (w ^ normal).normal();
    MVector y = (w ^ x).normal();
    MPointArray cvs;
    for (float u = 0.0; u <= 1.0; u += 0.2){
      cvs.append(pos + 
                  u*z + lxy*(1-cosf(u * theta)/theta)*x +
                  length*lxy * sin(u*theta)/theta * y
                );
    }

    MFnNurbsCurve curve;
    MObject newCurve = curve.create(cvs,knots,3, MFnNurbsCurve::Form::kOpen, false, true, outCurveData,&stat);
    outHandle.set(outCurveData);

  }
  stat = outputArray.set(builder);
  return stat;
}

MStatus FurriesFurNode::compute(const MPlug& plug, MDataBlock& data) {
  if (plug == outputCurves || plug == numberOfCurves){

  MStatus status = MStatus::kSuccess;

  MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
  MObject inputMeshObject(inputMeshHandle.asMesh());
  MFnMesh inputMesh(inputMeshObject);

  MArrayDataHandle inputAngles = data.inputValue(inputSpringAngles, &status);
  MDataHandle inputStepSize = data.inputValue(distanceBetweenStrands, &status);
  MDataHandle nCurvesHandle = data.outputValue( FurriesFurNode::numberOfCurves, &status);
  
  MIntArray triCount;
  MIntArray triVert;

  MFloatPointArray pointList;
  MFloatVectorArray normalList;

  inputMesh.getPoints(pointList, MSpace::kWorld);
  
  MFloatVectorArray angles = MFloatVectorArray(pointList.length());
  
  for (int i=0; i < pointList.length(); i++){
    if(inputAngles.elementCount() > 0 && inputAngles.elementIndex() == i) {
      angles[i] = inputAngles.inputValue().asFloat3();
      inputAngles.next();
    }
  }

  cout << angles.length() << endl;
  inputMesh.getNormals(normalList, MSpace::kWorld);  
  inputMesh.getTriangles(triCount, triVert);

  double stepSize = inputStepSize.asDouble();
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
  MFloatVector p4p3;
  MFloatVector resultVec;

  MFloatPoint resultPoint;
  MFloatPointArray pointArray;
  MFloatPointArray resultNormalArray;
  MFloatPointArray wArray;
  int curTri = 0;
  for (int n=0; n < triCount.length(); n++){
    for(int i=0; i < triCount[n] ; i++){

      int index = curTri;
      curTri += 3;

    pointList[triVert[index]].get(ap0);
    pointList[triVert[index + 1]].get(ap1);
    pointList[triVert[index + 2]].get(ap2);


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

    float A = ((p1 - p0 ) ^ (p2 - p0)).length()/2.0;
    
    //Get normals 
    MVector n0,n1,n2;
    inputMesh.getFaceVertexNormal(n,triVert[index],n0,MSpace::kWorld);	 
    inputMesh.getFaceVertexNormal(n,triVert[index+1],n1,MSpace::kWorld);	 
    inputMesh.getFaceVertexNormal(n,triVert[index+2],n2,MSpace::kWorld);

    //Get w-vector
    MVector w0,w1,w2;
    w0 = angles[triVert[index]];
    w1 = angles[triVert[index]];
    w2 = angles[triVert[index]];


    for (size_t j = 0; (j*stepSize) < (p0 - p2).length(); j++)
    {

      p2p3 = p2 - p3;
      p2p3.normalize();
      p2p4 = p2 - p4;
      p2p4.normalize();


      for (int k = 0; (k*stepSize) < (p4-p3).length(); k++) {
        
        // Add nurbcurve at p3 + ((p4-p3).normalize() * k * stepSize)
        p4p3 = (p4 - p3);
        p4p3.normalize();
        resultVec = p3 + p4p3 * k * stepSize;
        MFloatPoint pos = MFloatPoint(resultVec);

        //calculate interpolation values
        float A0 = ((resultVec - p1) ^ (p2 - p1)).length()/2.0;
        float A1 = ((resultVec - p0) ^ (p2 - p0)).length()/2.0;
        float A2 = ((p1 - p0) ^ (resultVec - p0)).length()/2.0; 

        float v0 = A0/A;
        float v1 = A1/A;
        float v2 = A2/A;

        //cout << "Area : " << A << " A1: "<< A0 << " A2: " << A1 << " A3: " << A2 << endl;
        
        pointArray.append(MFloatPoint(resultVec));

        MFloatVector normalz = ( v0 * n0 +  v1 * n1 +  v2 * n2);
        MFloatVector w = ( v0 * w0 +  v1 * w1 +  v2 * w2);
        normalz.normalize();
        resultNormalArray.append(MFloatPoint(normalz));
        wArray.append(MFloatPoint(w));
      }
      
      p3 += p2p0 * (stepSize);
      p4 += p2p1 * (stepSize);

    }

  }
  
}
  createHairCurve(pointArray, resultNormalArray, wArray, data);
  nCurvesHandle.set((int)pointArray.length());
  data.setClean(outputCurves);
  data.setClean(numberOfCurves);
  return status;
  }

}