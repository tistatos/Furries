
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

	//  Inputs
	FurriesFurNode::meshInput = typedAttr.create("inputMesh", "in", MFnData::kMesh, &status);
	typedAttr.setWritable(true);
	addAttribute(meshInput);

	FurriesFurNode::distanceBetweenStrands = numericAttr.create("distanceBetweenStrands", "dist", MFnNumericData::kDouble, 0.1, &status);

	numericAttr.setWritable(true);
	numericAttr.setMin(0.0001);
	numericAttr.setMax(1.0000);
	addAttribute(distanceBetweenStrands);

	FurriesFurNode::inputSpringPositions = numericAttr.create("springPositions", "positions", MFnNumericData::k3Double);
	numericAttr.setWritable(true);
	numericAttr.setArray(true);
	addAttribute(inputSpringPositions);

	FurriesFurNode::inputSpringAngles = numericAttr.create("springAngles", "angles", MFnNumericData::k3Double);
	numericAttr.setWritable(true);
	numericAttr.setArray(true);
	addAttribute(inputSpringAngles);

	// Output
	FurriesFurNode::outputCurves = typedAttr.create( "outputCurves", "oc", MFnNurbsCurveData::kNurbsCurve, &status );
	typedAttr.setArray( true );
	typedAttr.setReadable( true );
	typedAttr.setWritable( false );
	typedAttr.setUsesArrayDataBuilder( true );
	addAttribute(outputCurves);

	FurriesFurNode::numberOfCurves = numericAttr.create( "numberOfCurves", "n", MFnNumericData::kInt, 0, &status );
	addAttribute(numberOfCurves);

	//Affecting attributes
	status = attributeAffects(meshInput, outputCurves);
	status = attributeAffects(meshInput, numberOfCurves);

	status = attributeAffects(inputSpringAngles, outputCurves);
	status = attributeAffects(inputSpringPositions, outputCurves);

	status = attributeAffects(distanceBetweenStrands, outputCurves);
	status = attributeAffects(distanceBetweenStrands, numberOfCurves);

	return MStatus::kSuccess;
}

MStatus FurriesFurNode::createHairCurve( MFloatPointArray positions,  MFloatVectorArray normals, MFloatVectorArray wArray, MDataBlock& data){

	MStatus stat;

	int numCurves=positions.length();

	MArrayDataHandle outputArray = data.outputArrayValue(outputCurves,&stat);
	MArrayDataBuilder builder(outputCurves, numCurves, &stat);

	for (int curveNum = 0; curveNum < numCurves; curveNum++) {

		MDataHandle outHandle = builder.addLast();

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
	MStatus status = MStatus::kSuccess;

	if (plug == outputCurves || plug == numberOfCurves){
		//get mesh inputs
		MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
		MObject inputMeshObject(inputMeshHandle.asMesh());
		MFnMesh inputMesh(inputMeshObject, &status);

		//angle and position inputs
		MArrayDataHandle inputAngles = data.inputValue(inputSpringAngles, &status);
		if(status != MStatus::kSuccess) {
      cout << "Error getting angles" << endl;
			return status;
		}
		MArrayDataHandle inputPositions = data.inputValue(inputSpringPositions, &status);
		if(status != MStatus::kSuccess) {
      cout << "Error getting positions" << endl;
			return status;
		}

		MDataHandle inputStepSize = data.inputValue(distanceBetweenStrands, &status);

		//output curves
		MDataHandle nCurvesHandle = data.outputValue( FurriesFurNode::numberOfCurves, &status);


		//get vertices, normals and triangles
		MIntArray triangleCount;
		MIntArray triangleVertices;
		MFloatPointArray pointList;
		MFloatVectorArray normalList;
		inputMesh.getPoints(pointList, MSpace::kWorld);
		inputMesh.getNormals(normalList, MSpace::kWorld);
		inputMesh.getTriangles(triangleCount, triangleVertices);

		MVectorArray angles;
		MVectorArray positions;

		//get angles from input
		for (int i=0; i < inputAngles.elementCount() && i < inputPositions.elementCount(); i++) {
      angles.append(inputAngles.inputValue().asDouble3());
      positions.append(inputPositions.inputValue().asDouble3());
      inputAngles.next();
      inputPositions.next();
		}


    unsigned int numberOfSpringInputs =
      inputAngles.elementCount() > inputPositions.elementCount() ?
      inputAngles.elementCount() : inputPositions.elementCount();

    double stepSize = inputStepSize.asDouble();
    double ap[4];

    MFloatVector p0, p1, p2, p3, p4;
    MFloatVector p2p0, p1p0;
    MFloatVector p2p1, p0p1;
    MFloatVector p2p3;
    MFloatVector p2p4;
    MFloatVector p4p3;

		cout << "triangle count: " << triangleCount.length() << endl;
		cout << "Angles size: " << angles.length() << endl;

    MFloatPoint resultPoint;
    MFloatPointArray pointArray;
    MFloatVectorArray resultNormalArray;
    MFloatVectorArray wArray;

    int currentTriangle = 0;
    for (int n = 0; n < triangleCount.length(); n++){
      for(int i = 0; i < triangleCount[n]; i++){

        int index = currentTriangle;
        currentTriangle += 3;

        pointList[triangleVertices[index]].get(ap);
        p0 = arrToVec(ap);

        pointList[triangleVertices[index + 1]].get(ap);
        p1 = arrToVec(ap);

        pointList[triangleVertices[index + 2]].get(ap);
        p2 = arrToVec(ap);


        p2p0 = (p2 - p0).normal();
        p2p1 = (p2 - p1).normal();
        p0p1 = (p0 - p1).normal();
        p1p0 = (p1 - p0).normal();

        //w angles from spring node
        unsigned int angleIndex= triangleVertices[index]+3;
        unsigned int positionIndex = triangleVertices[index]+3;
        if( angleIndex > inputAngles.elementCount()) {
          cout << "not enough angles!" << endl;
          break;
        }

        p3 = p0 + (p2p0 * (stepSize/2)) + (p1p0 * (stepSize / 2));
        p4 = p1 + (p2p1 * (stepSize / 2)) + (p0p1 * (stepSize / 2));

        float A = ((p1 - p0 ) ^ (p2 - p0)).length()/2.0;

        //Get normals
        MVector n0,n1,n2;
        inputMesh.getFaceVertexNormal(n,triangleVertices[index],n0,MSpace::kWorld);
        inputMesh.getFaceVertexNormal(n,triangleVertices[index+1],n1,MSpace::kWorld);
        inputMesh.getFaceVertexNormal(n,triangleVertices[index+2],n2,MSpace::kWorld);

        //Get w-vector
        MVector w0,w1,w2;

        w0 = angles[angleIndex];
        w1 = angles[angleIndex + 1];
        w2 = angles[angleIndex + 2];

        for (size_t j = 0; (j*stepSize) < (p0 - p2).length(); j++) {
          p2p3 = (p2 - p3).normal();
					p2p4 = (p2 - p4).normal();

          for (int k = 0; (k*stepSize) < (p4-p3).length(); k++) {
            // Add nurbcurve at p3 + ((p4-p3).normalize() * k * stepSize)
            p4p3 = (p4 - p3).normal();
            MFloatVector interpolatedPosition = p3 + p4p3 * k * stepSize;

						////calculate interpolation values
            float A0 = ((interpolatedPosition - p1) ^ (p2 - p1)).length()/2.0;
            float A1 = ((interpolatedPosition - p0) ^ (p2 - p0)).length()/2.0;
            float A2 = ((p1 - p0) ^ (interpolatedPosition - p0)).length()/2.0;


            float weight0 = A0/A;
            float weight1 = A1/A;
            float weight2 = A2/A;

            pointArray.append(interpolatedPosition);

            MFloatVector interpolatedNormal = ( weight0 * n0 +  weight1 * n1 +  weight2 * n2);
            MFloatVector interpolatedW = ( weight2 * w0 +  weight2 * w1 +  weight2 * w2);

            interpolatedNormal.normalize();
            resultNormalArray.append(interpolatedNormal);
            wArray.append(interpolatedW);
          }

          p3 += p2p0 * (stepSize);
          p4 += p2p1 * (stepSize);
        }
      }
    }

		for (int i=0; i < pointArray.length(); i++) {
      cout << i << endl;
      cout << "Position: " << pointArray[i] << endl;
      cout << "Normal: " << resultNormalArray[i] << endl;
      cout << "W: " << wArray[i] << endl;
		}

    createHairCurve(pointArray, resultNormalArray, wArray, data);
    nCurvesHandle.set((int)pointArray.length());
		data.setClean(outputCurves);
		data.setClean(numberOfCurves);
	}
	return status;
}
