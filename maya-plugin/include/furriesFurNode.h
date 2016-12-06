
#ifndef __FURRIESFURNODE_H__
#define __FURRIESFURNODE_H__

#include <maya/MPxNode.h>
#include <maya/MObject.h>

class FurriesFurNode : public MPxNode {
public:
  FurriesFurNode();
  virtual ~FurriesFurNode();

  virtual MStatus compute(const MPlug& plug, MDataBlock& data);

  static void* creator() { return new FurriesFurNode(); }
  static MStatus initialize();


  static  MObject outputCurves;


public:

  static MStatus createHairCurve(MFloatPointArray positions, MDataBlock& data);
  
  static MString name;
  
  static MObject meshInput;

  static MObject output;
  static MTypeId id;
};
#endif