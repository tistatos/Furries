import maya.cmds as cmds
import maya.api.OpenMaya as omn
import math
cmds.file(new=True, force=True)
cmds.unloadPlugin("libfurries")
cmds.loadPlugin("libfurries")
cmds.polySphere()
cmds.createNode("furrySpringNode")
cmds.createNode("furryFurNode")
cmds.connectAttr('pSphereShape1.outMesh', 'furryFurNode1.inputMesh')
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh')
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime')
cmds.connectAttr('pSphere1.worldMatrix', 'furrySpringNode1.inputMatrix')
cmds.connectAttr('furrySpringNode1.springAngles', 'furryFurNode1.springAngles')

for i in range(0,100):
    cmds.createNode("nurbsCurve")
    cmds.connectAttr(('furryFurNode1.outputCurves[%i]' % i), ('curveShape%i.create' % (i+1)))   