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

  status = attributeAffects(stiffnessInput, outputSpringAngles);

  status = attributeAffects(gravityInput, outputSpringAngles);

  return MStatus::kSuccess;
}

MStatus FurriesSpringNode::compute(const MPlug& plug, MDataBlock& data) {

  MStatus status = MStatus::kSuccess;


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

  if(plug == outputSpringPositions) {


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
    outputPositions.set(positionBuilder);
  }

  if(fabs(mLastTimeUpdate - currentTime.value()) < 0.001) {
      data.setClean(plug);
      return status;
    }

  if( plug == outputSpringAngles) {

    mLastTimeUpdate = currentTime.value();

    //angle calculations goes here
    MArrayDataHandle outputAngles = data.outputArrayValue( FurriesSpringNode::outputSpringAngles);
    MArrayDataBuilder angleBuilder(FurriesSpringNode::outputSpringAngles, springCount);
    MArrayDataHandle inputAngles = data.inputValue( springAnglesInput);
    MFloatVector g(0, -data.inputValue(gravityInput).asFloat(), 0);


    MMatrix m = matrix.asRotateMatrix();

    MFloatVector prevPosition = mPrevMatrix.getTranslation(MSpace::kWorld);
    MFloatVector currentPosition = matrix.getTranslation(MSpace::kWorld);
    MFloatVector change = currentPosition-prevPosition;

    MFloatVector acceleration = 2 * change / (FRAME_TIME_STEP*FRAME_TIME_STEP) - 2 * mMeshVelocity / FRAME_TIME_STEP;

    mMeshVelocity  = change / FRAME_TIME_STEP;
    cout << currentTime.value() << " " << mMeshVelocity <<  " " << acceleration << endl;
    for(unsigned int i = 0; i < springCount; i++) {

      MFloatVector inAngle;
      bool foundAngle = false;
      if(inputAngles.elementCount() > 0 && inputAngles.elementIndex() == i) {
        inAngle = inputAngles.inputValue().asDouble3();
        inputAngles.next();
        foundAngle = true;
      }
      MDataHandle outangle  = angleBuilder.addLast();

      //calculate normal in world space
      MFloatVector normal = normals[i];

      if(currentTime.value() <= 1) {
        normal = MPoint(normal) * m;
        //calculate rotation for normal direction and reset curves
        MFloatVector up(0, 1.0, 0);
        MQuaternion q(up, normal);
        MFloatVector angles = q.asEulerRotation().asVector();
        angles *= 180/3.14;
        outangle.set3Double(angles.x, angles.y, angles.z);

        //reset velocities, w and modified normals
        mSpringW[i] = MFloatVector::zero;
        mSpringNormal[i] = normal;
        mSpringAngularVelocity[i] = MFloatVector::zero;
        mPrevMatrix = matrix;
        mMeshAcceleration = MFloatVector::zero;
        mMeshVelocity = MFloatVector::zero;
      }
      else if(foundAngle) {
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


        //as = 2 * change / (FRAME_TIME_STEP*FRAME_TIME_STEP) - 2 * mMeshVelocity / FRAME_TIME_STEP;

        //Equation 1
        ami = wNormal^(g-as);

        // Equation 2
        //aa = ka * (-mMeshVelocity)^springNormal/rho;
        // Equation 3
        as = -ks*wAngle;

        //sum all accelerations (Eq. 8)
        MFloatVector ai = ami + aa + as;


        //acceleration time step (Eq. 9)
        velocity = velocity + ai*FRAME_TIME_STEP*hi;


        double angle = wAngle.length();
        //Equation 11

        if(angle < THETA_MAX * 0.5) { }
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

        //calculate new modified angle
        MFloatVector newWNormal = MPoint(normal) * angleWmatrix.asRotateMatrix();
        newWNormal.normalize();
        mSpringNormal[i] = newWNormal;

        MQuaternion q(wNormal, newWNormal);

        inAngle *= 3.14/180;
        //calculate new normal angle with old value
        MFloatVector outAngle = (MEulerRotation(inAngle).asQuaternion()*q).asEulerRotation().asVector();
        outAngle *= 180/3.14;
        outangle.set3Double(outAngle.x, outAngle.y, outAngle.z);
      }

    mPrevMatrix = matrix;
    }
    outputAngles.set(angleBuilder);
    data.setClean(outputSpringAngles);
  }

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
    //cout << "Connection!" << endl;
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
