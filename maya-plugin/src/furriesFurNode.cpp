
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
MObject FurriesFurNode::matrixInput;

MObject FurriesFurNode::outputCurves;
MObject FurriesFurNode::numberOfCurves;
MObject FurriesFurNode::distanceBetweenStrands;
MObject FurriesFurNode::hairLength;
MObject FurriesFurNode::hairLengthXY;
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

	FurriesFurNode::numberOfCurves = numericAttr.create( "numberOfCurves", "n", MFnNumericData::kInt, 0, &status );
	addAttribute(numberOfCurves);
	//  Inputs
	FurriesFurNode::meshInput = typedAttr.create("inputMesh", "in", MFnData::kMesh, &status);
	typedAttr.setWritable(true);
	addAttribute(meshInput);

	FurriesFurNode::distanceBetweenStrands = numericAttr.create("distanceBetweenStrands", "dist", MFnNumericData::kDouble, 0.12, &status);
	numericAttr.setWritable(true);
	numericAttr.setMin(0.0001);
	numericAttr.setMax(1.0000);
	addAttribute(distanceBetweenStrands);

	FurriesFurNode::matrixInput = matrixAttr.create("inputMatrix", "mat");
	matrixAttr.setWritable(true);
	addAttribute(matrixInput);

	FurriesFurNode::hairLength = numericAttr.create("hairLength", "l", MFnNumericData::kDouble, 0.2, &status);
	numericAttr.setWritable(true);
	numericAttr.setMin(0.01);
	numericAttr.setMax(2.0000);
	addAttribute(hairLength);

	FurriesFurNode::hairLengthXY = numericAttr.create("hairLengthXY", "lxy", MFnNumericData::kDouble, 0.9, &status);
	numericAttr.setWritable(true);
	numericAttr.setMin(0.00);
	numericAttr.setMax(1.0000);
	addAttribute(hairLengthXY);

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
	typedAttr.setCached(false);
	addAttribute(outputCurves);


	//Affecting attributes
	status = attributeAffects(meshInput, outputCurves);
	status = attributeAffects(meshInput, numberOfCurves);

	status = attributeAffects(inputSpringAngles, outputCurves);
	status = attributeAffects(inputSpringPositions, outputCurves);

	status = attributeAffects(distanceBetweenStrands, outputCurves);
	status = attributeAffects(hairLength, outputCurves);
	status = attributeAffects(hairLengthXY, outputCurves);
	status = attributeAffects(distanceBetweenStrands, numberOfCurves);

	return status;
}


MStatus FurriesFurNode::createHairCurve( MFloatPointArray positions,  MFloatVectorArray normals, MFloatVectorArray wArray, MDataBlock& data){

	MStatus stat;

	int numCurves=positions.length();
	MDataHandle inputHairLength = data.inputValue(hairLength, &stat);
	MDataHandle inputHairLengthXY = data.inputValue(hairLengthXY, &stat);
	MArrayDataHandle outputArray = data.outputArrayValue(outputCurves,&stat);
	MArrayDataBuilder builder(outputCurves, numCurves, &stat);
	MTransformationMatrix m = data.inputValue(matrixInput).asMatrix();

	for (int curveNum = 0; curveNum < numCurves; curveNum++) {

		MDataHandle outHandle = builder.addLast();

		MFnNurbsCurveData dataCreator;
		MObject outCurveData = dataCreator.create();

		double k[] = {0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0,
			8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 18.0, 19.0
				, 20.0, 20.0 ,20.0};
		MDoubleArray knots(k, 22);

		MVector w = wArray[curveNum];
		MPoint pos = positions[curveNum];
		pos = pos * m.asMatrix();
		MVector normal = MPoint(normals[curveNum]) * m.asRotateMatrix();

		MVector z = w.normal();
		double theta = w.length();
		double length = inputHairLength.asDouble();
		double lxy = inputHairLengthXY.asDouble();

		MVector x = (z ^ normal).normal();
		MVector y = (x ^ z).normal();
		MPointArray cvs;


		for (int j = 0; j < 20; j++){
			float u = j/20.0;
			cvs.append(pos +
								(u * z*(1-lxy) + (lxy * (1 - cos(u * theta)) / theta) * x +
								lxy * sin(u * theta) / theta * y) * length );
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

		//angle inputs
		MArrayDataHandle inputAngles = data.inputValue(inputSpringAngles, &status);
		if(status != MStatus::kSuccess) {
			cout << "Error getting angles" << endl;
			return status;
		}

		//input step size
		MDataHandle inputStepSize = data.inputValue(distanceBetweenStrands, &status);

		//output curves count
		MDataHandle nCurvesHandle = data.outputValue( FurriesFurNode::numberOfCurves, &status);

		//get vertices, normals and triangles
		MIntArray triangleCount;
		MIntArray triangleVertices;

		MFloatPointArray pointList;

		inputMesh.getPoints(pointList, MSpace::kWorld);
		inputMesh.getTriangles(triangleCount, triangleVertices);

		MVectorArray angles;

		//get angles from input
		for (unsigned int i=0; i < inputAngles.elementCount(); i++) {
			angles.append(inputAngles.inputValue().asDouble3());
			inputAngles.next();
		}


		double stepSize = inputStepSize.asDouble();
		double ap[4];

		MFloatVector p0, p1, p2, p3, p4;
		MFloatVector p2p0, p1p0;
		MFloatVector p2p1, p0p1;
		MFloatVector p2p3;
		MFloatVector p2p4;
		MFloatVector p4p3;

		MFloatPointArray pointArray;
		MFloatVectorArray resultNormalArray;
		MFloatVectorArray wArray;
		MIntArray indexArray;


		for(int i = 0; i < triangleVertices.length(); i += 3){

			int index = i;

			p0 = pointList[triangleVertices[index]];
			p1 = pointList[triangleVertices[index+1]];
			p2 = pointList[triangleVertices[index+2]];

			p2p0 = (p2 - p0).normal();
			p2p1 = (p2 - p1).normal();
			p0p1 = (p0 - p1).normal();
			p1p0 = (p1 - p0).normal();

			p3 = p0 + (p2p0 * (stepSize/2)) + (p1p0 * (stepSize / 2));
			p4 = p1 + (p2p1 * (stepSize / 2)) + (p0p1 * (stepSize / 2));

			//Triangle area
			float A = ((p1 - p0 ) ^ (p2 - p0)).length()/2.0;

			//Get normals
			MVector n0,n1,n2;
			inputMesh.getVertexNormal(triangleVertices[index], n0, MSpace::kWorld);
			inputMesh.getVertexNormal(triangleVertices[index+1], n0, MSpace::kWorld);
			inputMesh.getVertexNormal(triangleVertices[index+2], n0, MSpace::kWorld);

			//Get w-vector
			MVector w0,w1,w2;

			w0 = angles[triangleVertices[index]];
			w1 = angles[triangleVertices[index+1]];
			w2 = angles[triangleVertices[index+2]];

			unsigned strandsPerArea = 100; //FIXME should be set here

			unsigned int strandsInTriangle = 100*A;
			strandsInTriangle = 1;

			for (size_t j = 0; (j*stepSize + (stepSize/2)) < (p0 - p2).length(); j++) {
				p2p3 = (p2 - p3).normal();
				p2p4 = (p2 - p4).normal();

				for (int k = 0; (k*stepSize) < (p4-p3).length(); k++) {
					// Add nurbcurve at p3 + ((p4-p3).normalize() * k * stepSize)
					p4p3 = (p4 - p3).normal();

					MVector interpolatedPosition = p3 + p4p3 * k * stepSize;

					//interpolatedPosition = p0;
					// calculate interpolation values
					MVector v0 = p1-p0;
					MVector v1 = p2-p0;
					MVector v2 = interpolatedPosition - p0;

					float d00 = v0*v0;
					float d01 = v0*v1;
					float d11 = v1*v1;
					float d20 = v2*v0;
					float d21 = v2*v1;
					float denom = d00 * d11 - d01 * d01;
					float v = (d11 * d20 - d01 * d21) / denom;
					float w = (d00 * d21 - d01 * d20) / denom;
					float u = 1.0f - v - w;

					float weight0 = u;
					float weight1 = v;
					float weight2 = w;

					if ( u > -0.001 && u <= 1.001 &&
							 v > -0.001 && v <= 1.001 &&
							 w > -0.001 && w <= 1.001){

						MFloatVector interpolatedNormal = ( weight0 * n0 +  weight1 * n1 +  weight2 * n2);
						MFloatVector interpolatedW = ( weight2 * w0 +  weight2 * w1 +  weight2 * w2);

						// interpolatedPosition = p0;
						// interpolatedNormal = n0;
						// interpolatedW = w0;
						interpolatedNormal.normalize();

						indexArray.append(triangleVertices[index]);
						pointArray.append(interpolatedPosition);
						resultNormalArray.append(interpolatedNormal);
						wArray.append(interpolatedW);
						}
				}

				p3 += p2p0 * (stepSize);
				p4 += p2p1 * (stepSize);
			}
		}

		createHairCurve(pointArray, resultNormalArray, wArray, data);
		nCurvesHandle.set((int)pointArray.length());
		data.setClean(outputCurves);
		data.setClean(numberOfCurves);
	}
	return status;
}
