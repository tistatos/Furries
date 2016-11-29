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

class FurriesSpringNode : public MPxNode{
public:
  FurriesSpringNode();
  virtual ~FurriesSpringNode();

  virtual MStatus compute(const MPlug& plug, MDataBlock& data);

  static void* creator() { return new FurriesSpringNode(); }
  static MStatus initialize();

public:
  static MObject input;
  static MObject output;
  static MTypeId id;
};

#endif
