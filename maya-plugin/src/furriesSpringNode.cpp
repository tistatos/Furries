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

#include <maya/MRenderUtil.h>

#include "furriesSpringNode.h"

MString FurriesSpringNode::name = "furrySpringNode";
MTypeId FurriesSpringNode::id(0x00001);

MObject FurriesSpringNode::meshInput;
MObject FurriesSpringNode::matrixInput;
MObject FurriesSpringNode::timeInput;
MObject FurriesSpringNode::gravityInput;
MObject FurriesSpringNode::stiffnessInput;
MObject FurriesSpringNode::hairDensityInput;
MObject FurriesSpringNode::airResistanceInput;
MObject FurriesSpringNode::maxThetaInput;
MObject FurriesSpringNode::timestepInput;
MObject FurriesSpringNode::epsilonInput;
MObject FurriesSpringNode::dampingInput;

MObject FurriesSpringNode::outputSpringPositions;
MObject FurriesSpringNode::outputSpringNormals;
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

	FurriesSpringNode::stiffnessInput = numericAttr.create("springStiffness", "stiff", MFnNumericData::kFloat, 0.6);
	numericAttr.setWritable(true);
	addAttribute(stiffnessInput);

	FurriesSpringNode::gravityInput = numericAttr.create("gravity", "grav", MFnNumericData::kFloat, 9.8);
	numericAttr.setWritable(true);
	addAttribute(gravityInput);

	FurriesSpringNode::timeInput = unitAttr.create("inputTime", "time", MFnUnitAttribute::kTime);
	unitAttr.setWritable(true);
	addAttribute(timeInput);

	FurriesSpringNode::dampingInput = numericAttr.create("damping", "damp", MFnNumericData::kFloat, 0.99f);
	numericAttr.setWritable(true);
	numericAttr.setMax(1.0f);
	numericAttr.setMin(0.01f);
	addAttribute(dampingInput);

	FurriesSpringNode::airResistanceInput = numericAttr.create("airResistance", "airR", MFnNumericData::kFloat, 0.5f);
	numericAttr.setWritable(true);
	numericAttr.setMax(1.0f);
	numericAttr.setMin(0.01f);
	addAttribute(airResistanceInput);

	FurriesSpringNode::epsilonInput = numericAttr.create("epsilon", "epsilon", MFnNumericData::kFloat, 0.001f);
	numericAttr.setWritable(true);
	numericAttr.setMax(1.0f);
	numericAttr.setMin(0.0f);
	addAttribute(epsilonInput);

	FurriesSpringNode::hairDensityInput = numericAttr.create("hairDensityInput", "density", MFnNumericData::kFloat, 1.0f);
	numericAttr.setWritable(true);
	numericAttr.setSoftMax(1.0f);
	numericAttr.setMin(0.01f);
	addAttribute(hairDensityInput);

	FurriesSpringNode::maxThetaInput = numericAttr.create("hairTheta", "theta", MFnNumericData::kFloat, 90.0f);
	numericAttr.setWritable(true);
	numericAttr.setSoftMax(180.0f);
	numericAttr.setMin(0.01f);
	addAttribute(maxThetaInput);

	FurriesSpringNode::timestepInput = numericAttr.create("timeStep", "timestep", MFnNumericData::kFloat, 0.05f);
	numericAttr.setWritable(true);
	addAttribute(timestepInput);
	// Output
	FurriesSpringNode::outputSpringNormals = numericAttr.create("springNormals", "normals", MFnNumericData::k3Double);
	numericAttr.setWritable(false);
	numericAttr.setReadable(true);
	numericAttr.setArray(true);
	numericAttr.setUsesArrayDataBuilder(true);
	addAttribute(outputSpringNormals);

	FurriesSpringNode::outputSpringAngles = numericAttr.create("springAngles", "angles", MFnNumericData::k3Double);
	numericAttr.setWritable(false);
	numericAttr.setReadable(true);
	numericAttr.setCached(false);
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
	status = attributeAffects(meshInput, outputSpringNormals);
	status = attributeAffects(meshInput, outputSpringPositions);

	status = attributeAffects(matrixInput, outputSpringPositions);
	status = attributeAffects(matrixInput, outputSpringAngles);
	status = attributeAffects(matrixInput, outputSpringNormals);

	status = attributeAffects(timeInput, outputSpringPositions);
	status = attributeAffects(timeInput, outputSpringAngles);
	status = attributeAffects(timeInput, outputSpringNormals);

	status = attributeAffects(stiffnessInput, outputSpringAngles);
	status = attributeAffects(stiffnessInput, outputSpringNormals);

	status = attributeAffects(gravityInput, outputSpringAngles);
	status = attributeAffects(gravityInput, outputSpringNormals);

	status = attributeAffects(dampingInput, outputSpringAngles);
	status = attributeAffects(dampingInput, outputSpringNormals);

	status = attributeAffects(airResistanceInput, outputSpringAngles);
	status = attributeAffects(airResistanceInput, outputSpringNormals);

	status = attributeAffects(hairDensityInput, outputSpringAngles);
	status = attributeAffects(hairDensityInput, outputSpringNormals);

	status = attributeAffects(maxThetaInput, outputSpringAngles);
	status = attributeAffects(maxThetaInput, outputSpringNormals);
	return MStatus::kSuccess;
}

MStatus FurriesSpringNode::calculatePositions(MDataBlock& data) {
	MStatus status = MStatus::kSuccess;

	//get mesh vertices
	MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
	MObject inputMeshObject(inputMeshHandle.asMesh());
	MFnMesh inputMesh(inputMeshObject);

	MIntArray triangleVertices;
	MIntArray triangleCount;

	MFloatPointArray vertices;
	inputMesh.getTriangles(triangleCount, triangleVertices);
	inputMesh.getPoints(vertices, MSpace::kWorld);

	unsigned int springCount = vertices.length();

	//get mesh matrix
	MTransformationMatrix matrix = data.inputValue(matrixInput, &status).asMatrix();

	//Output positions
	MArrayDataHandle outputPositions = data.outputArrayValue( FurriesSpringNode::outputSpringPositions);
	MArrayDataBuilder positionBuilder(FurriesSpringNode::outputSpringPositions, springCount);

	//generate a position from each triangle
	for(unsigned int i = 0; i < triangleVertices.length(); i++) {

		MDataHandle outPoint = positionBuilder.addElement(triangleVertices[i]);

		MPoint point;
		point.x = vertices[triangleVertices[i]].x;
		point.y = vertices[triangleVertices[i]].y;
		point.z = vertices[triangleVertices[i]].z;

		point = point * matrix.asMatrix();
		outPoint.set3Double(point.x, point.y, point.z);
	}
	outputPositions.set(positionBuilder);
	return status;
}


MStatus FurriesSpringNode::calculateSprings(MDataBlock& data) {
	MStatus status = MStatus::kSuccess;

	//get current time
	MTime currentTime = data.inputValue(timeInput, &status).asTime();

	double delta = currentTime.value() - mLastTimeUpdate;
	//mesh matrix
	MTransformationMatrix matrix = data.inputValue(matrixInput, &status).asMatrix();
	MMatrix m = matrix.asRotateMatrix();

	//get mesh vertices and normals
	MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
	MObject inputMeshObject(inputMeshHandle.asMesh());
	MFnMesh inputMesh(inputMeshObject);

	MIntArray triangleCount;
	MIntArray triangleVertices;

	MFloatPointArray pointList;
	MFloatVectorArray normalList;
	inputMesh.getPoints(pointList, MSpace::kWorld);
	inputMesh.getVertexNormals(false, normalList, MSpace::kWorld);
	inputMesh.getTriangles(triangleCount, triangleVertices);
	unsigned int springCount = pointList.length();

	// coefficients
	MFloatVector g(0, -data.inputValue(gravityInput).asFloat(), 0);
	float rho = data.inputValue(hairDensityInput).asFloat();
	float ka = data.inputValue(airResistanceInput).asFloat();
	float hi = (1.0f-data.inputValue(dampingInput).asFloat());

	float epsilon = data.inputValue(epsilonInput).asFloat();
	float ks = data.inputValue(stiffnessInput).asFloat();
	float timestep = data.inputValue(timestepInput).asFloat() * delta;
	float maxTheta = data.inputValue(maxThetaInput).asFloat() * (3.14f/180.0f);

	//get Normal output
	MArrayDataHandle outputNormals = data.outputArrayValue( FurriesSpringNode::outputSpringNormals);
	MArrayDataBuilder normalBuilder(FurriesSpringNode::outputSpringNormals, springCount);

	//get angle output
	MArrayDataHandle outputAngles = data.outputArrayValue( FurriesSpringNode::outputSpringAngles);
	MArrayDataBuilder angleBuilder(FurriesSpringNode::outputSpringAngles, springCount);

	//calculate velocity and acceleration
	MFloatVector prevPosition = mPrevMatrix.getTranslation(MSpace::kWorld);
	MFloatVector currentPosition = matrix.getTranslation(MSpace::kWorld);
	MFloatVector change = currentPosition-prevPosition;

	MFloatVector acceleration = 2 * change / (timestep*timestep) - 2 * mMeshVelocity / timestep;

	mMeshVelocity  = change / timestep;

	MQuaternion rotation;
	rotation = matrix.asMatrix();

	for(unsigned int i = 0; i < triangleVertices.length(); i++) {
		//calculate normal in world space
		int index = triangleVertices[i];
		MFloatVector normal = normalList[index];

		if(currentTime.value() <= 1) {
			MDataHandle outnormal  = normalBuilder.addElement(index);
			MDataHandle outAngle  = angleBuilder.addElement(index);

			//reset velocities, w and modified normals
			mSpringW[index] = MFloatVector::zero;

			//Avoid Zero vector
			MFloatVector fakeW = (normal^g).normal() * 0.0001;
			outAngle.set3Double(fakeW.x , fakeW.y, fakeW.z );

			mSpringNormal[index] = normal;
			mSpringAngularVelocity[index] = MFloatVector::zero;
			mPrevMatrix = matrix;
			mMeshAcceleration = MFloatVector::zero;
			mMeshVelocity = MFloatVector::zero;

			//FIXME VISUAL NORMAL OUTPUT DONT USE
			MFloatVector up(0, 1.0, 0);
			MQuaternion q(up, MPoint(normal)*matrix.asMatrix());
			MFloatVector orientedNormal = q.asEulerRotation().asVector();
			orientedNormal *= 180/3.14; //FIXME only rotational stuff
			outnormal.set3Double(orientedNormal.x, orientedNormal.y, orientedNormal.z);
		}
		else {

			MDataHandle outnormal  = normalBuilder.addElement(index);
			MDataHandle outAngle  = angleBuilder.addElement(index);
			//Get stored variables
			MFloatVector wAngle = mSpringW[index];
			MFloatVector velocity = mSpringAngularVelocity[index];
			MFloatVector wNormal = mSpringNormal[index];

			MFloatVector ami, as;

			ami = as = MFloatVector::zero;
			if(delta > 0) {
			//Equation 1
			ami = wNormal^(g-acceleration);

			// Equation 3
			as = -ks*wAngle;
			}
			else {
			}
			//sum all accelerations (Eq. 8)
			MFloatVector ai = ami + as;

			//acceleration time step (Eq. 9)
			velocity = velocity + ai*timestep*hi;

			double angle = wAngle.length();

			//Equation 11
			if(angle < maxTheta * 0.5) { }
			else if (angle > (maxTheta / 2.0f) && angle <= maxTheta) {

				if ((wAngle + velocity*timestep).length() > angle) {
					//only scale if we are increasing angle
					double scaleFactor = 1.0 - (1+epsilon) * (angle / maxTheta);
					velocity = scaleFactor * velocity.normal();
				}
			}
			else {
				velocity = -epsilon*velocity.normal();
			}

			//store new velocity
			mSpringAngularVelocity[index] = velocity;

			//update wAngle to the new angle and store it (Eq. 10)
			MFloatVector newAngle = wAngle+velocity*timestep;

			mSpringW[index] = newAngle;

			//create a rotation from the new angle
			MTransformationMatrix angleWmatrix;
			angleWmatrix.setToRotationAxis(newAngle.normal(), newAngle.length());

			MFloatVector newWNormal = MPoint(normal) * rotation *  angleWmatrix.asRotateMatrix();
			mSpringNormal[index] = newWNormal.normal();

			outAngle.set3Double(newAngle.x, newAngle.y, newAngle.z);


			//FIXME VISUAL NORMAL OUTPUT DONT USE
			MFloatVector up(0, 1.0, 0);
			MQuaternion q(up, newWNormal);
			MFloatVector orientedNormal = q.asEulerRotation().asVector();
			orientedNormal *= 180/3.14;
			outnormal.set3Double(orientedNormal.x, orientedNormal.y, orientedNormal.z);

		}
	}

	mLastTimeUpdate = currentTime.value();

	outputNormals.set(normalBuilder);
	outputAngles.set(angleBuilder);
	data.setClean(outputSpringNormals);
	data.setClean(outputSpringAngles);
	return status;
}

MStatus FurriesSpringNode::compute(const MPlug& plug, MDataBlock& data) {
	MStatus status = MStatus::kSuccess;

	MTime currentTime = data.inputValue(timeInput, &status).asTime();

	if(plug == outputSpringPositions) {
		status = calculatePositions(data);
		data.setClean(outputSpringPositions);
	}

	if(plug == outputSpringAngles || plug == outputSpringNormals) {
		status = calculateSprings(data);
	}

	//store previous matrix for acceleration and velocity
	mPrevMatrix = data.inputValue(matrixInput, &status).asMatrix();

	return status;
}

MStatus FurriesSpringNode::connectionMade(const MPlug& plug, const MPlug& extPlug, bool asSrc) {
	if(plug == meshInput) {
		MObject otherNode = extPlug.node();
		MFnMesh inputMesh(otherNode);
		MFloatPointArray vertices;
		MFloatVectorArray normals;
		inputMesh.getPoints(vertices, MSpace::kWorld);
		unsigned int springCount = vertices.length();
		inputMesh.getVertexNormals(false, normals, MSpace::kWorld);

		mSpringAngularVelocity.setLength(springCount);
		mSpringW.setLength(springCount);
		mSpringNormal.setLength(springCount);
	}
	return MStatus::kUnknownParameter;
}

MStatus FurriesSpringNode::connectionBroken(const MPlug& plug, const MPlug& extPlug, bool asSrc) {
	if(plug == meshInput) {
		mSpringNormal.clear();
		mSpringAngularVelocity.clear();
	}
	return MStatus::kUnknownParameter;
}
