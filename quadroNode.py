import os
import re
import subprocess

# import maya.cmds as cmds
# cmds.createNode("transform", name="animCube1")
# cmds.createNode("mesh", name="animCubeShape1", parent="animCube1")
# cmds.sets("animCubeShape1", add="initialShadingGroup")
# cmds.createNode("spAnimCube", name="animCubeNode1")
# cmds.connectAttr("time1.outTime", "animCubeNode1.time")
# cmds.connectAttr("animCubeNode1.outputMesh", "animCubeShape1.inMesh")

import sys
import maya.OpenMaya as OpenMaya
import maya.OpenMayaMPx as OpenMayaMPx
import maya.cmds as cmds
import maya.mel as mel

kPluginNodeName = "quadroNode"
kPluginNodeId = OpenMaya.MTypeId(0x00080052)


class animCube(OpenMayaMPx.MPxNode):
    eigenFunction = OpenMaya.MObject()
    tessellation = OpenMaya.MObject()
    relaxation = OpenMaya.MObject()
    outputMesh = OpenMaya.MObject()
    meshInput = OpenMaya.MObject()

    def __init__(self):
        OpenMayaMPx.MPxNode.__init__(self)
        self.meshInitialized = False

    def createMesh(self, vertices, facesVertsCount, facesConnectivity, outData):
        numFaces = facesVertsCount.length()
        numVertices = vertices.length()
        numFaceConnects = facesConnectivity.length()
        meshFS = OpenMaya.MFnMesh()
        newMesh = meshFS.create(numVertices, numFaces, vertices, facesVertsCount, facesConnectivity, outData)
        self.meshInitialized = True
        return newMesh

    def extractSharpEdgeAsList(self, meshInputObject) -> list:
        fnMesh = OpenMaya.MFnMesh(meshInputObject)
        fnMeshData = OpenMaya.MFnMeshData(meshInputObject)
        itEdge = OpenMaya.MItMeshEdge(meshInputObject)
        hardEdges = []
        hardEdgesIndices = []
        while not itEdge.isDone():
            if itEdge.isSmooth() or not len(itEdge.getConnectedFaces())==2:
                continue
            hardEdges.append((itEdge.vertexId(0), itEdge.vertexId(1), itEdge.getConnectedFaces()))
            itEdge.next()
        itFaceVertex = OpenMaya.MItMeshFaceVertex(meshInputObject)
        lastVertexId = None
        lastFaceId = None
        while not itFaceVertex.isDone():
            if itFaceVertex.vertId()==hardEdges[0]:
                lastVertexId = itFaceVertex.vertId()
                lastFaceId = itFaceVertex.faceId()
                continue
            if itFaceVertex.faceId()==hardEdges[1] and itFaceVertex.vertId()==lastFaceId:
                hardEdgesIndices.append((1, lastFaceId, ))
            lastVertexId = None
            lastFaceId = None

    def dumpOff(self,
                vertices: OpenMaya.MPointArray,
                facesVertsCount:OpenMaya.MIntArray,
                facesConnectivity:OpenMaya.MIntArray,
                outputPath):
        nbFaces = len(facesVertsCount)
        with open(outputPath, "w") as outputFile:
            outputFile.write("OFF\n")
            outputFile.write("{} {} {}\n".format(vertices.length(), nbFaces, 0))
            for i in range(vertices.length()):
                outputFile.write("{} {} {}\n".format(vertices[i].x, vertices[i].y, vertices[i].z))
            offset = 0
            for i in range(nbFaces):
                cellSize = facesVertsCount[i]
                outputFile.write("{} ".format(cellSize))
                for j in range(facesVertsCount[i]):
                    outputFile.write("{} ".format(facesConnectivity[j+offset]))
                outputFile.write("\n")
                offset += cellSize

    def loadOff(self, inputPath: str):
        with open(inputPath, "r") as inputFile:
            inputString = ' '.join(inputFile.readlines())
            inputs = re.split(r"\s+", inputString)
            inputIter = iter(inputs)
            if next(inputIter) != "OFF":
                raise RuntimeError("Invalid input")
            vertexNumber = int(next(inputIter))
            faceNumber = int(next(inputIter))
            k = next(inputIter)
            vertices = OpenMaya.MPointArray(vertexNumber)
            facesVertsCount = OpenMaya.MIntArray(faceNumber)
            facesConnectivity = OpenMaya.MIntArray()
            for i in range(vertexNumber):
                vertices[i].x = float(next(inputIter))
                vertices[i].y = float(next(inputIter))
                vertices[i].z = float(next(inputIter))
                vertices[i].w = 1.0
            for i in range(faceNumber):
                cellSize = int(next(inputIter))
                facesVertsCount[i] = cellSize
                for j in range(cellSize):
                    cellId = int(next(inputIter))
                    facesConnectivity.append(cellId)
            return vertices, facesVertsCount, facesConnectivity


    def extractMeshData(self,
        fnMesh: OpenMaya.MFnMesh,):
        _vertices = OpenMaya.MPointArray()
        _facesVertsCount = OpenMaya.MIntArray()
        _facesConnectivity = OpenMaya.MIntArray()
        fnMesh.getPoints(_vertices, OpenMaya.MSpace.kObject)
        fnMesh.getVertices(_facesVertsCount, _facesConnectivity)
        return _vertices, _facesVertsCount, _facesConnectivity

    def quadrangulateMesh(self, meshInputObject: OpenMaya.MObject, e, t, r) -> OpenMaya.MObject:
        fnMesh = OpenMaya.MFnMesh(meshInputObject)
        vertices, facesVertsCount, facesConnectivity = self.extractMeshData(fnMesh)
        self.dumpOff(vertices, facesVertsCount, facesConnectivity, "inputdump.off")
        print("Quadro Node: quadrangualte using -e {} -t {} -r {}".format(e, t, r))
        p = subprocess.Popen(["quad_msc", "-i", "inputdump.off", "-e", str(e), "-t", str(t), "-r", str(r)])
        p.communicate()
        vertices, facesVertsCount, facesConnectivity = self.loadOff("output.off")
        dataCreator = OpenMaya.MFnMeshData()
        newOutputData = dataCreator.create()
        self.createMesh(vertices, facesVertsCount, facesConnectivity, newOutputData)
        return newOutputData

    def compute(self, plug, data):
        if plug == animCube.outputMesh:
            print("actual compute")
            if self.meshInitialized:
                print("skip mesh already Initialized")
                return
            e = data.inputValue(self.eigenFunction).asInt()
            t = data.inputValue(self.tessellation).asInt()
            r = data.inputValue(self.relaxation).asInt()
            meshInputObject = data.inputValue(animCube.meshInput).asMesh()  # MObject
            newOutputData = self.quadrangulateMesh(meshInputObject, e, t, r)
            outputHandle = data.outputValue(animCube.outputMesh)
            outputHandle.setMObject(newOutputData)
            data.setClean(plug)
        else:
            return OpenMaya.kUnknownParameter


def nodeCreator():
    return OpenMayaMPx.asMPxPtr(animCube())


def nodeInitializer():
    unitAttr = OpenMaya.MFnNumericAttribute()
    typedAttr = OpenMaya.MFnTypedAttribute()
    animCube.eigenFunction = unitAttr.create("eigenfunction", "ef", OpenMaya.MFnNumericData.kInt, 80)
    animCube.tessellation = unitAttr.create("tessellation", "ts", OpenMaya.MFnNumericData.kInt, 3)
    animCube.relaxation = unitAttr.create("relaxation", "rl", OpenMaya.MFnNumericData.kInt, 100)
    animCube.meshInput = typedAttr.create("meshInput", "in", OpenMaya.MFnData.kMesh)
    animCube.outputMesh = typedAttr.create("outputMesh", "out", OpenMaya.MFnData.kMesh)
    animCube.addAttribute(animCube.outputMesh)
    animCube.addAttribute(animCube.meshInput)
    animCube.addAttribute(animCube.eigenFunction)
    animCube.addAttribute(animCube.tessellation)
    animCube.addAttribute(animCube.relaxation)

    animCube.attributeAffects(animCube.eigenFunction, animCube.outputMesh)
    animCube.attributeAffects(animCube.tessellation, animCube.outputMesh)
    animCube.attributeAffects(animCube.relaxation, animCube.outputMesh)
    animCube.attributeAffects(animCube.meshInput, animCube.outputMesh)


# initialize the script plug-in
def initializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.registerNode(kPluginNodeName, kPluginNodeId, nodeCreator, nodeInitializer)
        createMenuForQuadro()
    except:
        sys.stderr.write("Failed to register node: %s" % kPluginNodeName)
        raise


# uninitialize the script plug-in
def uninitializePlugin(mobject):
    mplugin = OpenMayaMPx.MFnPlugin(mobject)
    try:
        mplugin.deregisterNode(kPluginNodeId)
    except:
        sys.stderr.write("Failed to deregister node: %s" % kPluginNodeName)
        raise
eigenSlider = ""
tessellationSlider = ""
relaxitionSlider=  ""
eigenValue = 80
tessellationValue = 3
relaxition = 100
mainWindowName = "QuadroMainWindow"
def createMenuForQuadro():
    if cmds.about(batch=True):
        # don't create menu in batch mode
        return
    # destroy any pre-existing shotgun menu - the one that holds the apps
    if cmds.menu("Quadro", exists=True):
        cmds.deleteUI("Quadro")
    # create a new shotgun disabled menu if one doesn't exist already.
    if not cmds.menu("Quadro", exists=True):
        sg_menu = cmds.menu(
            "Quadro",
            label="Quadro",
            # Get the mel global variable value for main window.
            # In order to get the global variable in mel.eval we have to assign it to another temporary value
            # so that it returns the result.
            parent=mel.eval("$retvalue = $gMainWindow;"),
        )
        cmds.menuItem(
            label="Quadrangulate ...",
            command="CreateQuadroMainWindow()",
            parent=sg_menu)


def CreateQuadroMainWindow():
    global mainWindowName
    global eigenSlider
    global tessellationSlider
    global relaxitionSlider
    global eigenValue
    global tessellationValue    
    global relaxitionValue

    cmds.confirmDialog(title="Welcome to Quadro", message="**Please save your work before using**\nWhen you click quadrangulate, "
                                                          "it may take 3-5 minutes to perform the algorithm. "
                                                          "If this tool does not provide desired result, "
                                                          "please manually separate your model into different patches and try again.")
    width = 600
    height = 400
    if cmds.window(mainWindowName, exists=True):
        cmds.deleteUI(mainWindowName)
    quadroMainWindow = cmds.window(mainWindowName, width=width, height=height, title=mainWindowName)
    cmds.columnLayout(adjustableColumn=1, columnAlign="left")
    eigenSlider = cmds.intSliderGrp(label="Eigenvalue", field=True, minValue=5, maxValue=100, value=eigenValue)
    tessellationSlider = cmds.intSliderGrp(label="Tessellation", field=True, minValue=1, maxValue=10, value=3)
    relaxitionSlider = cmds.intSliderGrp(label="Relaxation Iters", field=True, minValue=1, maxValue=200, value=100)
    cmds.setParent("..")
    cmds.rowLayout(numberOfColumns=2, adjustableColumn1=1, adjustableColumn2=1)
    cmds.button(label="Quadrangulate", command="performQuadrangulate()")
    cmds.button(label="Close", command="deleteUI -window "+quadroMainWindow)
    cmds.setParent("..")
    cmds.showWindow(quadroMainWindow)

def performQuadrangulate():
    eigenValue = cmds.intSliderGrp(eigenSlider, q=True, value=True)
    tessellationValue = cmds.intSliderGrp(tessellationSlider, q=True, value=True)
    relaxition = cmds.intSliderGrp(relaxitionSlider, q=True, value=True)
    print("Quadrangulate using parameter of ef: {} ts: {}, rl: {}".format(eigenValue, tessellationValue, relaxition))
    if len(cmds.ls(selection=True))==0:
        print(type(eigenSlider))
        cmds.confirmDialog(title="Error", message="You must select a triangular mesh to perform action.")
        return
    testObject = cmds.ls(selection=True)[0]
    customTransform = cmds.createNode("transform")
    cmds.copyAttr(testObject, customTransform, values=True)
    pluginNode = cmds.createNode("quadroNode")
    cmds.setAttr("{}.ef".format(pluginNode), eigenValue)
    cmds.setAttr("{}.ts".format(pluginNode), tessellationValue)
    cmds.setAttr("{}.rl".format(pluginNode), relaxition)
    cmds.connectAttr(testObject + ".outMesh", pluginNode + ".meshInput")
    pluginMesh = cmds.createNode("mesh", parent=customTransform)
    cmds.sets(pluginMesh, add="initialShadingGroup")
    nodeOutputObject = pluginMesh
    cmds.connectAttr("{}.outputMesh".format(pluginNode), pluginMesh+".inMesh")
