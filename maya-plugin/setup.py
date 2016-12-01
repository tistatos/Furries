import maya.cmds as cmds
import maya.api.OpenMaya as omn
import math

cmds.unloadPlugin("libfurries")
cmds.loadPlugin("libfurries")
cmds.polySphere()
cmds.createNode("furrySpringNode")
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh')
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime')
cmds.connectAttr('pSphere1.worldMatrix', 'furrySpringNode1.inputMatrix')

cgroup = cmds.group(em=True, n="cubegroup")

for i in range(0,54):
    points = []
    pointsPerCurve = 5
    
    # Coordinate System for strand
    w = omn.MVector(0, 0.2, 0.2)
    n = omn.MVector(0,0, 1)
    z = omn.MVector(w)
    z.normalize()
    # crossproduct
    x = (z ^ n)
    x.normalize()
    y = (x ^ z)
    y.normalize()
    # (x, y, z) now forms a coordinate system as in the paper
    theta = w.length()
    l = 1.0
    lxy = 0.2
    for j in range(0, pointsPerCurve):
        u = j/float(pointsPerCurve) 
        zi = omn.MVector(u*z)
        xi = omn.MVector(((1 - math.cos(theta*u))/theta) * x)
        yi =  omn.MVector((lxy*math.sin(u*theta)/theta)*l*y)
        point = omn.MVector(zi+xi+yi)  
        points.append((point.x, point.y, point.z))
        print point
    

    cmds.curve(p=points)
    cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % (i*7)), ('curve%i.translate' % (i+1)))
    cmds.connectAttr(('furrySpringNode1.springAngles[%i]' % (i*7)), ('curve%i.rotate' % (i+1)))
