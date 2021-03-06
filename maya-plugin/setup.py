import maya.cmds as cmds
import maya.api.OpenMaya as omn
import math
cmds.file(new=True, force=True)
cmds.unloadPlugin("libfurries")
cmds.loadPlugin("libfurries")
cmds.polySphere()


makeNormals = False;
makeStrands= True;

cmds.createNode("furrySpringNode")
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh')
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime')
cmds.connectAttr('pSphere1.worldMatrix', 'furrySpringNode1.inputMatrix')

if makeStrands:
    cmds.createNode("furryFurNode")
    cmds.connectAttr('pSphereShape1.outMesh', 'furryFurNode1.inputMesh')
    cmds.connectAttr('furrySpringNode1.springAngles', 'furryFurNode1.springAngles')
    cmds.connectAttr('furrySpringNode1.springPositions', 'furryFurNode1.springPositions')
    cmds.connectAttr('pSphere1.worldMatrix', 'furryFurNode1.inputMatrix')
    
    curves = cmds.getAttr('furryFurNode1.numberOfCurves')

for i in range(0,curves):
    if makeStrands:
        hair = cmds.createNode("nurbsCurve")
        cmds.connectAttr(('furryFurNode1.outputCurves[%i]' % i), hair + '.create')
    if makeNormals:
        normalPoints = []
        for j in range(0,4):
            point = omn.MVector(0, 0.1*j, 0)
            normalPoints.append((point.x, point.y, point.z))
        normal = cmds.curve(p=normalPoints, n="curveNormal1")
        normalShape = maya.cmds.listRelatives(normal, children=True)[0]
        cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % i), normal+ '.translate')
        cmds.connectAttr(('furrySpringNode1.springNormals[%i]' % i), normal+ '.rotate')
