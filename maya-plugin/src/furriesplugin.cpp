#include <maya/MStatus.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

#include "simplecmd.h"
#include "furriesSpringNode.h"

MStatus initializePlugin(MObject obj) {
  MStatus status;
  MFnPlugin plugin(obj, "FURRIES" , "0.1");

  plugin.registerCommand("SimpleCmd", SimpleCmd::creator);
  plugin.registerNode("furrySpringNode", FurriesSpringNode::id, FurriesSpringNode::creator, FurriesSpringNode::initialize);

  //Register stuff here
  return status;
}

MStatus uninitializePlugin(MObject obj) {
  MStatus status;
  MFnPlugin plugin(obj);
  plugin.deregisterCommand("SimpleCmd");
  plugin.deregisterNode(FurriesSpringNode::id);

  //deregister stuff here
  return status;
}
