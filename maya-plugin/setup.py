import maya.cmds as cmds
import maya.api.OpenMaya as om
import math

cmds.unloadPlugin("libfurries");
cmds.loadPlugin("libfurries");
cmds.polySphere();
cmds.createNode("furrySpringNode");
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh');
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime');

for i in range(0,54):
    points = []
    pointsPerCurve = 5
    
    # Coordinate System for strand
    w = om.MVector(0, 0.2, 0.2)
    n = om.MVector(0,0, 1)
    z = om.MVector(w)
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
        zi = om.MVector(u*z)
        xi = om.MVector(((1 - math.cos(theta*u))/theta) * x)
        yi =  om.MVector((lxy*math.sin(u*theta)/theta)*l*y)
        point = om.MVector(zi+xi+yi)  
        points.append((point.x, point.y, point.z))
        print point
    

    cmds.curve(p=points);
    cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % (i*7)), ('curve%i.translate' % (i+1)));
