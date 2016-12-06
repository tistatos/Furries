/**
* @file furriesSpringNode.h
* @author Erik Sandrén
* @date 29-11-2016
* @brief [Description Goes Here]
*/

#ifndef __FURRIESSPRINGNODE_H__
#define __FURRIESSPRINGNODE_H__

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MFloatVectorArray.h>

class FurriesSpringNode : public MPxNode{
public:
  FurriesSpringNode();
  virtual ~FurriesSpringNode();

  virtual MStatus compute(const MPlug& plug, MDataBlock& data);
  MStatus connectionMade(const MPlug& plug, const MPlug& extPlug, bool asSrc);
  MStatus connectionBroken(const MPlug& plug, const MPlug& extPlug, bool asSrc);

  static void* creator() { return new FurriesSpringNode(); }
  static MStatus initialize();

public:
  static MString name;

  static MObject timeInput;
  static MObject stiffnessInput;
  static MObject gravityInput;
  static MObject meshInput;
  static MObject springAnglesInput;
  static MObject matrixInput;

  static MObject outputSpringPositions;
  static MObject outputSpringAngles;
  static MTypeId id;

private:
  MFloatVector mMeshVelocity;
  MFloatVectorArray mSpringAngularVelocity;
  MFloatVectorArray mSpringW;
  MFloatVectorArray mSpringNormal; //n_i'
};

#endif
