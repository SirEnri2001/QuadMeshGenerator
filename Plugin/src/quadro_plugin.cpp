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
#define TTK_CELL_ARRAY_NEW
#include <TopologyToolKit.h>
#include <Triangulation.h>

#define checkStatM(stat, message) if (stat != MS::kSuccess) {MGlobal::displayWarning(message); return stat;}
#define checkStat(stat) if (stat != MS::kSuccess) {return stat;}



class QuadroPlugin : public MPxCommand
{
public:
    int eigen = 1;
    int tessellation = 1;
    MStatus doIt(const MArgList& args);
    static void* creator();

private:
    MStatus quadrangulate(MPointArray& vertices, MIntArray& facesVertsCount, MIntArray& facesConnectivity);

    // helper functions
    MStatus testfunc();
    MStatus createAndDisplayMesh(const MPointArray& vertices, const MIntArray& facesVertsCount, const MIntArray& facesConnectivity);
    MStatus extractMeshData(MDagPath& dagPath, 
            MPointArray& vertices, MIntArray& facesVertsCount, MIntArray& facesConnectivity);
};
struct hash_pair {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);

        if (hash1 != hash2) {
            return hash1 ^ hash2;
        }

        // If hash1 == hash2, their XOR is zero.
        return hash1;
    }
};
int loadFile(const std::string& inputPath,
    std::vector<double>& pointSet,
    std::vector<long long int>& triangleSetCo,
    std::vector<long long int>& triangleSetOff) {

    // load some terrain from some OFF file.

    if (inputPath.empty())
        return -1;

    int vertexNumber = 0, triangleNumber = 0;
    std::string keyword;

    std::ifstream f(inputPath.data(), std::ios::in);

    if (!f) {
        return -1;
    }

    f >> keyword;

    if (keyword != "OFF") {
        return -2;
    }

    f >> vertexNumber;
    f >> triangleNumber;
    f >> keyword;

    pointSet.resize(3 * vertexNumber);
    triangleSetCo.resize(3 * triangleNumber);
    triangleSetOff.resize(triangleNumber + 1);

    for (int i = 0; i < 3 * vertexNumber; i++) {
        f >> pointSet[i];
    }

    int offId = 0;
    int coId = 0;
    for (int i = 0; i < triangleNumber; i++) {
        int cellSize;
        f >> cellSize;
        if (cellSize != 3) {
            std::cerr << "cell size " << cellSize << " != 3" << std::endl;
            return -3;
        }
        triangleSetOff[offId++] = coId;
        for (int j = 0; j < 3; j++) {
            int cellId;
            f >> cellId;
            triangleSetCo[coId++] = cellId;
        }
    }
    triangleSetOff[offId] = coId; // the last one

    f.close();

    return 0;
}
int load(
    MPointArray& vertices, 
    MIntArray& facesVertsCount, 
    MIntArray& facesConnectivity, 
    std::vector<double>& pointSet,
    std::vector<long long int>& triangleSetCo,
    std::vector<long long int>& triangleSetOff) {
    int vertexNumber = vertices.length(), triangleNumber = facesVertsCount.length();
    std::string keyword;

    pointSet.resize(3 * vertexNumber);
    triangleSetCo.resize(3 * triangleNumber);
    triangleSetOff.resize(triangleNumber + 1);

    for (int i = 0; i < vertexNumber; i++) {
        pointSet[3 * i + 0] = vertices[i].x;
        pointSet[3 * i + 1] = vertices[i].y;
        pointSet[3 * i + 2] = vertices[i].z;
    }
    triangleSetOff[0] = 0;
    for (int i = 1; i < triangleNumber + 1; i++) {
        triangleSetOff[i] = facesVertsCount[i - 1] + triangleSetOff[i - 1];
    }

    for (int i = 0; i < 3 * triangleNumber; i++) {
        triangleSetCo[i] = facesConnectivity[i];
    }

    return 0;
}

int dumpOff(MPointArray& vertices,
    MIntArray& facesVertsCount,
    MIntArray& facesConnectivity, std::string outputPath) {
    // save the simplified terrain in some OFF file
    std::string fileName(outputPath);

    std::ofstream f(fileName.data(), std::ios::out);

    if (!f) {
        return -1;
    }

    const int nbTriangles = facesVertsCount.length();

    f << "OFF" << std::endl;
    f << vertices.length() << " " << nbTriangles << " 0" << std::endl;

    for (int i = 0; i < (int)vertices.length(); i++) {
        f << vertices[i].x<<" " << vertices[i].y<<" " << vertices[i].z;
        f << std::endl;
    }
    int offset = 0;
    for (int i = 0; i < nbTriangles; i++) {
        int cellSize = facesVertsCount[i];
        f << cellSize << " ";
        for (int j = 0; j < facesVertsCount[i]; j++) {
            f << facesConnectivity[j + offset];
            f << " ";
        }
        f << std::endl;
        offset += cellSize;
    }

    f.close();

    return 0;
}

int loadOff(MPointArray& vertices,
    MIntArray& facesVertsCount,
    MIntArray& facesConnectivity, std::string inputPath) {
    // load some terrain from some OFF file.

    if (inputPath.empty())
        return -1;

    int vertexNumber = 0, faceNumber = 0;
    std::string keyword;

    std::ifstream f(inputPath.data(), std::ios::in);

    if (!f) {
        return -1;
    }

    f >> keyword;

    if (keyword != "OFF") {
        return -2;
    }

    f >> vertexNumber;
    f >> faceNumber;
    f >> keyword;
    vertices = MPointArray(vertexNumber);
    facesVertsCount = MIntArray(faceNumber);
    facesConnectivity = MIntArray();

    for (int i = 0; i < vertexNumber; i++) {
        f >> vertices[i].x;
        f >> vertices[i].y;
        f >> vertices[i].z;
        vertices[i].w = 1.0;
    }

    int offId = 0;
    int coId = 0;
    for (int i = 0; i < faceNumber; i++) {
        int cellSize;
        f >> cellSize;
        facesVertsCount[i] = cellSize;
        for (int j = 0; j < cellSize; j++) {
            int cellId;
            f >> cellId;
            facesConnectivity.append(cellId);
        }
    }

    f.close();
    return 0;
}

int save(MPointArray& vertices,
    MIntArray& facesVertsCount,
    MIntArray& facesConnectivity, 
    const std::vector<ttk::Quadrangulation::Point>& pointSet,
    const std::vector<std::array<long long int, 4>>& quadSetCo) {
    const int nbQuads = quadSetCo.size();
    vertices.clear();
    for (int i = 0; i < (int)pointSet.size(); i++) {
        vertices.append(
            {
            pointSet[i][0],
            pointSet[i][1],
            pointSet[i][2] 
            }
        );
    }
    facesVertsCount = MIntArray(quadSetCo.size(), 4);
    facesConnectivity = MIntArray(quadSetCo.size() * 4, 4);
    for (int i = 0; i < quadSetCo.size(); i++) {
        facesConnectivity[4 * i + 0] = quadSetCo[i][0];
        facesConnectivity[4 * i + 1] = quadSetCo[i][1];
        facesConnectivity[4 * i + 2] = quadSetCo[i][2];
        facesConnectivity[4 * i + 3] = quadSetCo[i][3];
    }
    return 0;
}


int scalarFieldNormalize(std::vector<double>& input,
    std::vector<double>& output) {
    double min = 0, max = 0;
    for (int i = 0; i < input.size(); i++) {
        double const value = input[i];

        if ((!i) || (value < min)) {
            min = value;
        }
        if ((!i) || (value > max)) {
            max = value;
        }
    }

    for (int i = 0; i < input.size(); i++) {
        double value = input[i];
        value = (value - min) / (max - min);
        output[i] = value;
    }
    return 0;
}

MStatus QuadroPlugin::quadrangulate(MPointArray& vertices, MIntArray& facesVertsCount, MIntArray& facesConnectivity)
{
    MStatus stat = MS::kSuccess;
    // TODO: quadrangulate
    this->eigen;
    this->tessellation;
        // MPointArray vertices;
        // MPoint& pt = vertices[i];
        // pt.x, pt.y, pt.z
    dumpOff(vertices, facesVertsCount, facesConnectivity, "C:\\Users\\SirEnri\\1.off");
    MGlobal::executeCommand("system \"C:\\Users\\SirEnri\\quad.exe -i C:\\Users\\SirEnri\\1.off\";");
    loadOff(vertices, facesVertsCount, facesConnectivity, "C:\\Users\\SirEnri\\output.off");
    return stat;
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
        list.getDagPath(index, dagPath);
        MPointArray vertices;
        MIntArray faceVertCount;
        MIntArray faceConnectivity;
        // retrieve the mesh of current node
        stat = extractMeshData(dagPath, vertices, faceVertCount, faceConnectivity);
        if (stat != MS::kSuccess) {
            return stat;
        }
        try {
            stat = quadrangulate(vertices, faceVertCount, faceConnectivity);
        }
        catch (const std::runtime_error& e) {
            assert(0);
        }

        stat = createAndDisplayMesh(vertices, faceVertCount, faceConnectivity);
        if (stat != MS::kSuccess) {
            return stat;
        }
    }
    return stat;
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
    // retrieving the mesh of this selected node
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

MStatus QuadroPlugin::createAndDisplayMesh(const MPointArray& vertices, const MIntArray& facesVertsCount, const MIntArray& facesConnectivity)
{
    MStatus stat = MS::kSuccess;

    // Create the mesh
    MFnMesh meshFn;
    MObject newMesh;
    newMesh = meshFn.create(vertices.length(), facesVertsCount.length(), vertices, facesVertsCount, facesConnectivity, MObject::kNullObj, &stat);
    if (stat != MStatus::kSuccess)
    {
        MGlobal::displayError("Failed to create mesh");
        return stat;
    }

    // Attach to DAG
    MFnDagNode myNode(newMesh, &stat);
    if (stat != MStatus::kSuccess)
    {
        MGlobal::displayError("Failed to create DagNode");
        return stat;
    }

    return stat;
}