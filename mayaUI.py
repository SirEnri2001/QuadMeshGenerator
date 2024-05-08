import maya.cmds as cmds
import maya.mel as mel

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
    eigenSlider = cmds.intSliderGrp(label="Eigenvalue (different values yield different layouts)", field=True, minValue=5, maxValue=100, value=eigenValue)
    tessellationSlider = cmds.intSliderGrp(label="Tessellation (mesh density)", field=True, minValue=1, maxValue=10, value=3)
    relaxitionSlider = cmds.intSliderGrp(label="Relaxation Iters (lower value for faster computation)", field=True, minValue=1, maxValue=200, value=100)
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