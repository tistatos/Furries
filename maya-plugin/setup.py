import maya.cmds as cmds

cmds.unloadPlugin("libfurries");
cmds.loadPlugin("libfurries");
cmds.polySphere();
cmds.createNode("furrySpringNode");
cmds.connectAttr('pSphereShape1.outMesh', 'furrySpringNode1.inputMesh');
cmds.connectAttr('time1.outTime', 'furrySpringNode1.inputTime');

for i in range(0,38):
    cmds.polyCube(w=0.1, d=0.1);
    cmds.connectAttr(('furrySpringNode1.springPositions[%i]' % (i*10)), ('pCube%i.translate' % (i+1)));
