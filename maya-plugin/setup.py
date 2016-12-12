import maya.cmds as cmds
import maya.api.OpenMaya as omn
import math
cmds.file(new=True, force=True)
cmds.unloadPlugin("libfurries")
cmds.loadPlugin("libfurries")
cmds.polySphere()
cmds.createNode("furrySpringNode")
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh')
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime')
cmds.connectAttr('pSphere1.worldMatrix', 'furrySpringNode1.inputMatrix')

#cmds.createNode("furryFurNode")
#cmds.connectAttr('pSphereShape1.outMesh', 'furryFurNode1.inputMesh')
#cmds.connectAttr('furryFurNode1.outputCurves[0]','curveShape1.create')
#cmds.createNode("nurbsCurve")


for i in range(0,382):
    points = []
    normalPoints = []
    pointsPerCurve = 5
    for j in range(0, pointsPerCurve):
        point = omn.MVector(0, .1*j, 0)
        points.append((point.x, point.y, point.z))
    for j in range(0, pointsPerCurve):        
        point = omn.MVector(0, 0.2*j, 0)      
        normalPoints.append((point.x, point.y, point.z))
    
    cmds.curve(p=normalPoints, n="curveNormal1")
    cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % (i)), ('curveNormal%i.translate' % (i+1)))
    cmds.connectAttr(('furrySpringNode1.springNormals[%i]' % (i)), ('curveNormal%i.rotate' % (i+1)))

    cmds.curve(p=points, n="curveSpring1")
    cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % (i)), ('curveSpring%i.translate' % (i+1)))
    cmds.connectAttr(('furrySpringNode1.springAngles[%i]' % (i)), ('curveSpring%i.rotate' % (i+1)))