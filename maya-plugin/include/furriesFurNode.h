
#ifndef __FURRIESFURNODE_H__
#define __FURRIESFURNODE_H__

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
class FurriesFurNode : public MPxNode {
public:


  struct FurNode {
    MFloatPoint position;
    int springIndices[3];
    int springWeights[3];
  };

  FurriesFurNode();
  virtual ~FurriesFurNode();

  virtual MStatus compute(const MPlug& plug, MDataBlock& data);

  static void* creator() { return new FurriesFurNode(); }
  static MStatus initialize();


  static MObject outputCurves;
  static MObject numberOfCurves;
  static MStatus createHairCurve( MFloatPointArray positions,  MFloatVectorArray normals, MFloatVectorArray wArray, MIntArray index, MDataBlock& data);

  static MString name;

  static MObject meshInput;
  static MObject distanceBetweenStrands;

  static MObject springInput;
  static MObject inputSpringAngles;
  static MObject inputSpringPositions;
  static MObject output;
  static MTypeId id;

};
#endif
