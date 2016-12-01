import maya.cmds as cmds

cmds.unloadPlugin("libfurries");
cmds.loadPlugin("libfurries");
cmds.polySphere();
cmds.createNode("furrySpringNode");
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh');
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime');
cmds.connectAttr('pSphere1.worldMatrix', 'furrySpringNode1.inputMatrix');

cgroup = cmds.group(em=True, n="cubegroup");
for i in range(0,54):
    cube = cmds.polyCube(w=0.1, d=0.1);
    #cmds.parent(cube, cgroup);
    cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % (i*7)), ('pCube%i.translate' % (i+1)));
    cmds.connectAttr(('furrySpringNode1.springAngles[%i]' % (i*7)), ('pCube%i.rotate' % (i+1)));
