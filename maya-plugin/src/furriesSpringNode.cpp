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

MString FurriesSpringNode::name = "furrySpringNode";
MTypeId FurriesSpringNode::id(0x00001);

MObject FurriesSpringNode::meshInput;
MObject FurriesSpringNode::matrixInput;
MObject FurriesSpringNode::timeInput;
MObject FurriesSpringNode::gravityInput;
MObject FurriesSpringNode::stiffnessInput;

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

  FurriesSpringNode::timeInput = unitAttr.create("inputTime", "time",
      MFnUnitAttribute::kTime);
  unitAttr.setWritable(true);
  addAttribute(timeInput);

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
  status = attributeAffects(matrixInput, outputSpringPositions);

  status = attributeAffects(timeInput, outputSpringAngles);
  status = attributeAffects(timeInput, outputSpringNormals);

  status = attributeAffects(stiffnessInput, outputSpringAngles);
  status = attributeAffects(stiffnessInput, outputSpringNormals);

  status = attributeAffects(gravityInput, outputSpringAngles);
  status = attributeAffects(gravityInput, outputSpringNormals);

  return MStatus::kSuccess;
}

MStatus FurriesSpringNode::calculatePositions(MDataBlock& data) {
  MStatus status = MStatus::kSuccess;

  //get mesh vertices
  MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
  MObject inputMeshObject(inputMeshHandle.asMesh());
  MFnMesh inputMesh(inputMeshObject);

  MFloatPointArray vertices;
  inputMesh.getPoints(vertices, MSpace::kWorld);
  unsigned int springCount = vertices.length();

  //get mesh matrix
  MTransformationMatrix matrix = data.inputValue(matrixInput, &status).asMatrix();

  //Output positions
  MArrayDataHandle outputPositions = data.outputArrayValue( FurriesSpringNode::outputSpringPositions);
  MArrayDataBuilder positionBuilder(FurriesSpringNode::outputSpringPositions, springCount);

  //generate a position from each vertex
  for(unsigned int i = 0; i < springCount; i++) {
    MPoint point;
    point.x = vertices[i].x;
    point.y = vertices[i].y;
    point.z = vertices[i].z;
    point.w = 1.0;

    point = point * matrix.asMatrix();
    MDataHandle outPoint  = positionBuilder.addLast();
    outPoint.set3Double(point.x, point.y, point.z);
  }
  outputPositions.set(positionBuilder);
  return status;
}


MStatus FurriesSpringNode::calculateSprings(MDataBlock& data) {
  MStatus status = MStatus::kSuccess;

  //get current time
  MTime currentTime = data.inputValue(timeInput, &status).asTime();

  //mesh matrix
  MTransformationMatrix matrix = data.inputValue(matrixInput, &status).asMatrix();
  MMatrix m = matrix.asRotateMatrix();

  //get mesh vertices and normals
  MDataHandle inputMeshHandle = data.inputValue(meshInput, &status);
  MObject inputMeshObject(inputMeshHandle.asMesh());
  MFnMesh inputMesh(inputMeshObject);

  MFloatPointArray vertices;
  MFloatVectorArray normals;
  inputMesh.getPoints(vertices, MSpace::kWorld);
  inputMesh.getVertexNormals(false, normals, MSpace::kWorld);
  unsigned int springCount = vertices.length();

  //gravity
  MFloatVector g(0, -data.inputValue(gravityInput).asFloat(), 0);

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

  MFloatVector acceleration = 2 * change / (FRAME_TIME_STEP*FRAME_TIME_STEP) - 2 * mMeshVelocity / FRAME_TIME_STEP;

  mMeshVelocity  = change / FRAME_TIME_STEP;

  for(unsigned int i = 0; i < springCount; i++) {
    //calculate normal in world space
    MFloatVector normal = normals[i];
    normal = MPoint(normal) * m;

    if(currentTime.value() <= 1) {
      MDataHandle outnormal  = normalBuilder.addLast();
      MDataHandle outAngle  = angleBuilder.addLast();

      //FIXME USE THESE
      //calculate rotation for normal direction and reset curves
      //outnormal.set3Double(normal.x, normal.y, normal.z);
      //outAngle.set3Double(0.0, 0.0, 0.0);

      //reset velocities, w and modified normals
      mSpringW[i] = MFloatVector::zero;
      mSpringNormal[i] = normal;
      mSpringAngularVelocity[i] = MFloatVector::zero;
      mPrevMatrix = matrix;
      mMeshAcceleration = MFloatVector::zero;
      mMeshVelocity = MFloatVector::zero;

      //FIXME VISUAL NORMAL OUTPUT DONT USE
      MFloatVector up(0, 1.0, 0);
      MQuaternion q(up, normal);
      MFloatVector orientedNormal = q.asEulerRotation().asVector();
      orientedNormal *= 180/3.14; //FIXME only rotational stuff
      outnormal.set3Double(orientedNormal.x, orientedNormal.y, orientedNormal.z);
    }
    else {

      MDataHandle outnormal  = normalBuilder.addLast();
      MDataHandle outAngle  = angleBuilder.addLast();
      //Get stored variables
      MFloatVector wAngle = mSpringW[i];
      MFloatVector velocity = mSpringAngularVelocity[i];
      MFloatVector wNormal = mSpringNormal[i];

      float rho = 1.0f; //FIXME: hair density per unit
      float ka = 1.0f; //FIXME: air-resistance coefficient
      float kft = 0.9f;
      float kmft = 0.4f;
      float hi = 1 - fmax(kft+(kmft-kft),0);
      hi = 0.2;

      double const epsilon = 0.1;
      float ks = data.inputValue(stiffnessInput).asFloat();

      MFloatVector ami, aa, as;

      as = acceleration;
      //Equation 1
      ami = wNormal^(g-as);
      // Equation 2
      //aa = ka * (-mMeshVelocity)^wNormal/rho;
      // Equation 3
      as = -ks*wAngle;

      //sum all accelerations (Eq. 8)
      MFloatVector ai = ami + aa + as;

      //acceleration time step (Eq. 9)
      velocity = velocity + ai*FRAME_TIME_STEP*hi;

      double angle = wAngle.length();

      //Equation 11
      if(angle < THETA_MAX * 0.5) {
      }
      else if (angle > THETA_MAX * 0.5 && angle < THETA_MAX) {
        velocity = (1.0-(1.0+epsilon)*(angle/THETA_MAX))*velocity.normal();
      }
      else {
        velocity = -epsilon*velocity.normal();
      }

      //store new velocity
      mSpringAngularVelocity[i] = velocity;

      //update wAngle to the new angle and store it (Eq. 10)
      MFloatVector newAngle = wAngle+velocity*FRAME_TIME_STEP;
      mSpringW[i] = newAngle;
      
      //create a rotation from the new angle
      MTransformationMatrix angleWmatrix;
      angleWmatrix.setToRotationAxis(newAngle.normal(), newAngle.length());
      MFloatVector newWNormal = MPoint(normal) * angleWmatrix.asRotateMatrix();
      mSpringNormal[i] = newWNormal.normal();

      //FIXME correct output
      //outnormal.set3Double(newWNormal.x, newWNormal.y, newWNormal.z);
      outAngle.set3Double(newAngle.x, newAngle.y, newAngle.z);

      //FIXME VISUAL NORMAL OUTPUT DONT USE
      MFloatVector up(0, 1.0, 0);
      MQuaternion q(up, newWNormal);
      MFloatVector orientedNormal = q.asEulerRotation().asVector();
      orientedNormal *= 180/3.14;
      outnormal.set3Double(orientedNormal.x, orientedNormal.y, orientedNormal.z);

    }
    outputNormals.set(normalBuilder);
    outputAngles.set(angleBuilder);
    }

  data.setClean(outputSpringNormals);
  data.setClean(outputSpringAngles);
  return status;
}

MStatus FurriesSpringNode::compute(const MPlug& plug, MDataBlock& data) {

  MStatus status = MStatus::kSuccess;

  MTime currentTime = data.inputValue(timeInput, &status).asTime();

  if(plug == outputSpringPositions) {
    status = calculatePositions(data);
  }

  if(plug == outputSpringAngles || plug == outputSpringNormals) {
    //skip evaluation if we are on the same time step
    //if(fabs(mLastTimeUpdate - currentTime.value()) < 0.001) {
      //data.setClean(plug);
      //return status;
    //}

    status = calculateSprings(data);
    //mLastTimeUpdate = currentTime.value();
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
