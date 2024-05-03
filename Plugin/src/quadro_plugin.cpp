#include <stdio.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MGlobal.h>

#include <maya/MFnMesh.h>
#include <maya/MIntArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPointArray.h>
#include <maya/MDagModifier.h>
#include <maya/MItMeshPolygon.h>

#define checkStat(stat, message) if (stat != MS::kSuccess) {MGlobal::displayWarning(message); return stat;}


class QuadroPlugin : public MPxCommand
{
public:
    int eigen = 1;
    int tessellation = 1;
    MStatus doIt(const MArgList& args);
    static void* creator();

private:
    void quadrangulate();

    // helper functions
    MStatus testfunc();
    MStatus createAndDisplayMesh();
    MStatus extractMeshData(MDagPath& dagPath, 
            MPointArray& vertices, MIntArray& facesVertsCount, MIntArray& facesConnectivity);
};

void QuadroPlugin::quadrangulate()
{

}

MStatus QuadroPlugin::testfunc() {
    MStatus stat = MS::kSuccess;

    MDagPath dagPath;
    MSelectionList list;
    MGlobal::getActiveSelectionList(list);

    // Only quadrangulate the first selected mesh, may change later
    int len = list.length() > 1 ? 1 : list.length();
    for (unsigned int index = 0; index < len; index++)
    {
        // retreave the mesh of this selected node
        list.getDagPath(index, dagPath);
        unsigned int numShapes;
        stat = dagPath.numberOfShapesDirectlyBelow(numShapes);
        if (stat != MS::kSuccess) {
            MGlobal::displayWarning("No shapes attached");
            return stat;
        }

        MDagPath tmpDagPath = dagPath;
        if (!tmpDagPath.hasFn(MFn::kMesh)) {
            for (unsigned int i = 0; i < numShapes; ++i) {
                tmpDagPath = dagPath;
                tmpDagPath.extendToShapeDirectlyBelow(i);
                if (tmpDagPath.hasFn(MFn::kMesh)) {
                    break;
                }
            }
            dagPath = tmpDagPath;
        }
        if (!dagPath.hasFn(MFn::kMesh, &stat)) {
            MGlobal::displayWarning("No mesh found in selected object.");
            return stat;
        }

        MFnMesh meshFn(dagPath, &stat);
        if (stat != MS::kSuccess) {
            MGlobal::displayWarning("Failed to attach mesh function set");
            return stat;
        }

        // Get vertex positions
        MPointArray vertexArray;
        meshFn.getPoints(vertexArray, MSpace::kWorld);
        // Print vertex positions
        for (unsigned int i = 0; i < vertexArray.length(); ++i) {
            MPoint& pt = vertexArray[i];
            MGlobal::displayInfo(MString("Vertex ") + i + ": (" + pt.x + ", " + pt.y + ", " + pt.z + ")");
        }

        // Iterate over each polygon to get connectivity
        MIntArray facesVertsCount;
        MIntArray facesConnectivity;
        MItMeshPolygon polyIter(dagPath, MObject::kNullObj, &stat);
        if (stat != MS::kSuccess) {
            MGlobal::displayWarning("Fail to get surface");
            return MS::kFailure;
        }
        while (!polyIter.isDone()) {
            MIntArray vertexList;
            polyIter.getVertices(vertexList);
            facesVertsCount.append(vertexList.length());
            MString vertexIndices = "Face Vertex Indices: ";
            for (unsigned int i = 0; i < vertexList.length(); ++i) {
                vertexIndices += vertexList[i];
                facesConnectivity.append(vertexList[i]);
                if (i < vertexList.length() - 1)
                    vertexIndices += ", ";
            }
            MGlobal::displayInfo(vertexIndices);
            polyIter.next();
        }

    }
}

// In more complex command plug-ins, doIt() parses arguments, 
// sets internal data, and does other housekeeping before it calls the redoIt() function. 
// The redoIt() function then performs the command's actions.
MStatus QuadroPlugin::doIt(const MArgList& args) {
    MStatus stat = MS::kSuccess;

    // get eigen and tessellation from arguments
    stat = args.get(0, eigen) ;
    if (stat != MS::kSuccess) {
        return stat;
    }
    stat = args.get(1, tessellation);
    if (stat != MS::kSuccess) {
        return stat;
    }
    cout << eigen << " " << tessellation << endl;
    testfunc();
    return stat;
}


void* QuadroPlugin::creator() {
    return new QuadroPlugin;
}

MStatus initializePlugin(MObject obj) {
    MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");

    // Register command
    plugin.registerCommand("Quadro", QuadroPlugin::creator);

    // Set name
    plugin.setName("Quadro");

    return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj) {
    MFnPlugin plugin(obj);
    plugin.deregisterCommand("Quadro");
    return MS::kSuccess;
}

MStatus QuadroPlugin::extractMeshData(MDagPath& dagPath, 
        MPointArray& vertices, MIntArray& facesVertsCount, MIntArray& facesConnectivity)
{   
    vertices.clear();
    facesVertsCount.clear();
    facesConnectivity.clear();

    MStatus stat = MS::kSuccess;
    // retreaving the mesh of this selected node
    unsigned int numShapes;
    stat = dagPath.numberOfShapesDirectlyBelow(numShapes);
    if (stat != MS::kSuccess) {
        MGlobal::displayWarning("No shapes attached");
        return stat;
    }
    
    MDagPath tmpDagPath = dagPath;
    if (!tmpDagPath.hasFn(MFn::kMesh)) {
        for (unsigned int i = 0; i < numShapes; ++i) {
            tmpDagPath = dagPath;
            tmpDagPath.extendToShapeDirectlyBelow(i);
            if (tmpDagPath.hasFn(MFn::kMesh)) {
                break;
            }
        }
        dagPath = tmpDagPath;
    }
    if (!dagPath.hasFn(MFn::kMesh, &stat)) {
        MGlobal::displayWarning("No mesh found in selected object.");
        return stat;
    }
    MFnMesh meshFn(dagPath, &stat);
    if (stat != MS::kSuccess) {
        MGlobal::displayWarning("Failed to attach mesh function set");
        return stat;
    }

    // Get mesh's vertex positions
    MPointArray vertices;
    meshFn.getPoints(vertices, MSpace::kWorld);
    
    // Print vertex positions
    for (unsigned int i = 0; i < vertices.length(); ++i) {
        MPoint& pt = vertices[i];
        MGlobal::displayInfo(MString("Vertex ") + i + ": (" + pt.x + ", " + pt.y + ", " + pt.z + ")");
    }

    MItMeshPolygon polyIter(dagPath, MObject::kNullObj, &stat);
    if (stat != MS::kSuccess) {
        MGlobal::displayWarning("Fail to get surface");
        return MS::kFailure;
    }

    // Iterate over each polygon to get each face's number of vertices and connectivity 
    // Print connectivity
    while (!polyIter.isDone()) {
        MIntArray vertexList;
        polyIter.getVertices(vertexList);
        facesVertsCount.append(vertexList.length());
        MString vertexIndices = "Face Vertex Indices: ";
        for (unsigned int i = 0; i < vertexList.length(); ++i) {
            vertexIndices += vertexList[i];
            facesConnectivity.append(vertexList[i]);
            if (i < vertexList.length() - 1)
                vertexIndices += ", ";
        }
        MGlobal::displayInfo(vertexIndices);
        polyIter.next();
    }

    return MStatus();
}

MStatus QuadroPlugin::createAndDisplayMesh()
{
    MStatus stat;
    MPointArray vertices;
    MIntArray faceCounts;
    MIntArray faceConnectivity;
    /*------- receive quadro results ---------------------------------------------------------------------------------*/
    // Define vertices
    vertices.append(MFloatPoint(0.0, 0.0, 0.0));
    vertices.append(MFloatPoint(1.0, 0.0, 0.0));
    vertices.append(MFloatPoint(0.5, 1.0, 0.0));

    // Define the face structure
    faceCounts.append(3); // The face has three vertices

    // Connect vertices
    faceConnectivity.append(0);
    faceConnectivity.append(1);
    faceConnectivity.append(2);
    /*--------------------------------------------------------------------------------------------------*/

    // Create the mesh
    MFnMesh meshFn;
    MObject newMesh;
    newMesh = meshFn.create(vertices.length(), faceCounts.length(), vertices, faceCounts, faceConnectivity, MObject::kNullObj, &stat);
    if (stat != MStatus::kSuccess)
    {
        MGlobal::displayError("Failed to create mesh");
        return stat;
    }

    MFnDagNode myNode(newMesh, &stat);
    if (stat != MStatus::kSuccess)
    {
        MGlobal::displayError("Failed to create DagNode");
        return stat;
    }
}