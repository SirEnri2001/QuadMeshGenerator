/// \ingroup examples
/// \author Julien Tierny <julien.tierny@lip6.fr>
/// \date October 2017.
///
/// \brief Minimalist C++-only TTK example pipeline, including:
///  -# The computation of a persistence curve
///  -# The computation of a persistence diagram
///  -# The selection of the most persistent pairs of the diagram
///  -# The pre-simplification of the data according to this selection
///  -# The computation of the Morse-Smale complex on this simplified data
///  -# The storage of the output of this pipeline to disk.
///
/// This example reproduces the Figure 1 of the TTK companion paper:
/// "The Topology ToolKit", J. Tierny, G. Favelier, J. Levine, C. Gueunet, M.
/// Michaux., IEEE Transactions on Visualization and Computer Graphics, Proc.
/// of IEEE VIS 2017.
///
/// See the individual VTK wrappers (core/vtk/) to see how to use each ttk::base
/// (C++-only) TTK component.

// include the local headers
#include <TopologyToolKit.h>

#include <cassert>
#include <iostream>
#include <ttkAlgorithm.h>
#include <Eigen/Dense>

int load(const std::string& inputPath,
    std::vector<double>& pointSet,
    std::vector<long long int>& triangleSetCo,
    std::vector<long long int>& triangleSetOff) {

    // load some terrain from some OFF file.

    if (inputPath.empty())
        return -1;

    ttk::Debug dbg;
    dbg.setDebugLevel(ttk::globalDebugLevel_);
    dbg.setDebugMsgPrefix("main::load");
    dbg.printMsg("Reading input mesh...");

    int vertexNumber = 0, triangleNumber = 0;
    std::string keyword;

    std::ifstream f(inputPath.data(), std::ios::in);

    if (!f) {
        dbg.printErr("Cannot read file `" + inputPath + "'!");
        return -1;
    }

    f >> keyword;

    if (keyword != "OFF") {
        dbg.printErr("Input OFF file `" + inputPath + "' seems invalid :(");
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

    dbg.printMsg("... done! (read " + std::to_string(vertexNumber) + " vertices, "
        + std::to_string(triangleNumber) + " triangles)");

    return 0;
}

int save(const std::vector<double>& pointSet,
    const std::vector<long long int>& triangleSetCo,
    const std::vector<long long int>& triangleSetOff,
    const std::string& outputPath) {

    // save the simplified terrain in some OFF file
    std::string fileName(outputPath);

    std::ofstream f(fileName.data(), std::ios::out);

    if (!f) {
        ttk::Debug dbg;
        dbg.setDebugLevel(ttk::globalDebugLevel_);
        dbg.setDebugMsgPrefix("main::save");
        dbg.printErr("Could not write output file `" + fileName + "'!");
        return -1;
    }

    const int nbTriangles = triangleSetOff.size() - 1;

    f << "OFF" << std::endl;
    f << pointSet.size() / 3 << " " << nbTriangles << " 0" << std::endl;

    for (int i = 0; i < (int)pointSet.size() / 3; i++) {
        for (int j = 0; j < 3; j++) {
            f << pointSet[3 * i + j];
            f << " ";
        }
        f << std::endl;
    }

    for (int i = 0; i < nbTriangles; i++) {
        int cellSize = triangleSetOff[i + 1] - triangleSetOff[i];
        assert(cellSize == 3);
        f << cellSize << " ";
        for (int j = triangleSetOff[i]; j < triangleSetOff[i + 1]; j++) {
            f << triangleSetCo[j];
            f << " ";
        }
        f << std::endl;
    }

    f.close();

    return 0;
}

int saveQuadPoints(const std::vector<ttk::Quadrangulation::Point>& pointSet,
    const std::vector<std::array<long long int, 4>>& quadSetCo,
    const std::string& outputPath) {

    // save the simplified terrain in some OFF file
    std::string fileName(outputPath);

    std::ofstream f(fileName.data(), std::ios::out);

    if (!f) {
        ttk::Debug dbg;
        dbg.setDebugLevel(ttk::globalDebugLevel_);
        dbg.setDebugMsgPrefix("main::save");
        dbg.printErr("Could not write output file `" + fileName + "'!");
        return -1;
    }

    const int nbQuads = quadSetCo.size();

    f << "OFF" << std::endl;
    f << pointSet.size() << " " << nbQuads << " 0" << std::endl;

    for (int i = 0; i < (int)pointSet.size(); i++) {
        for (int j = 0; j < 3; j++) {
            f << pointSet[i][j];
            f << " ";
        }
        f << std::endl;
    }

    for (int i = 0; i < nbQuads; i++) {
        std::array<long long int, 4> quad = quadSetCo[i];
        f << 4 << " ";
        for (int j = 0; j < 4; j++) {
            f << quad[j];
            f << " ";
        }
        f << std::endl;
    }

    f.close();

    return 0;
}

template<class T>
int saveQuad(const std::vector<T>& pointSet,
    const std::vector<std::array<long long int, 4>>& quadSetCo,
    const std::string& outputPath) {

    // save the simplified terrain in some OFF file
    std::string fileName(outputPath);

    std::ofstream f(fileName.data(), std::ios::out);

    if (!f) {
        ttk::Debug dbg;
        dbg.setDebugLevel(ttk::globalDebugLevel_);
        dbg.setDebugMsgPrefix("main::save");
        dbg.printErr("Could not write output file `" + fileName + "'!");
        return -1;
    }

    const int nbQuads = quadSetCo.size();

    f << "OFF" << std::endl;
    f << pointSet.size() / 3 << " " << nbQuads << " 0" << std::endl;

    for (int i = 0; i < (int)pointSet.size() / 3; i++) {
        for (int j = 0; j < 3; j++) {
            f << pointSet[3 * i + j];
            f << " ";
        }
        f << std::endl;
    }

    for (int i = 0; i < nbQuads; i++) {
        std::array<long long int, 4> quad = quadSetCo[i];
        f << 4 << " ";
        for (int j = 0; j < 4; j++) {
            f << quad[j];
            f << " ";
        }
        f << std::endl;
    }

    f.close();

    return 0;
}

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

int main(int argc, char** argv) {

    std::string inputFilePath;
    ttk::CommandLineParser parser;

    ttk::globalDebugLevel_ = 3;

    // register the arguments to the command line parser
    parser.setArgument("i", &inputFilePath, "Path to input OFF file");
    // parse
    parser.parse(argc, argv);

    std::vector<double> pointSet;
    std::vector<long long int> triangleSetCo, triangleSetOff;
    ttk::Triangulation triangulation;

    // load the input
    load(inputFilePath, pointSet, triangleSetCo, triangleSetOff);
    triangulation.setInputPoints(pointSet.size() / 3, pointSet.data(), true);
    long long int triangleNumber = triangleSetOff.size() - 1;
#ifdef TTK_CELL_ARRAY_NEW
    triangulation.setInputCells(
        triangleNumber, triangleSetCo.data(), triangleSetOff.data());
#else
    LongSimplexId* triangleSet;
    CellArray::TranslateToFlatLayout(triangleSetCo, triangleSetOff, triangleSet);
    triangulation.setInputCells(triangleNumber, triangleSet);
#endif

    // 1. compute eigen scalar function
    int eigenNumber = triangulation.getNumberOfVertices() - 80;
    ttk::EigenField eigenField;
    std::vector<double> eigenFunction;
    eigenFunction.resize(triangulation.getNumberOfVertices() * triangulation.getNumberOfVertices());
    std::vector<double> stats;
    stats.resize(4 * triangulation.getNumberOfVertices());

    eigenField.preconditionTriangulation(triangulation);
    eigenField.setThreadNumber(32);
    eigenField.execute<double>(triangulation,
        static_cast<double*>(eigenFunction.data()),
        triangulation.getNumberOfVertices(), false,
        static_cast<double*>(stats.data()));
    std::vector<double> extractComponentOutput(
        eigenFunction.begin() + eigenNumber * triangulation.getNumberOfVertices(),
        eigenFunction.begin() + (1 + eigenNumber) * triangulation.getNumberOfVertices());
    std::vector<double> normalizedScalarField;
    normalizedScalarField.resize(extractComponentOutput.size());
    scalarFieldNormalize(extractComponentOutput, normalizedScalarField);
    
    // 2. computing the persistence diagram
    ttk::PersistenceDiagram diagram;
    std::vector<ttk::PersistencePair> diagramOutput;
    diagram.preconditionTriangulation(&triangulation);
    std::vector<ttk::SimplexId> order;
    order.resize(triangulation.getNumberOfVertices());
    ttk::preconditionOrderArray(triangulation.getNumberOfVertices(), normalizedScalarField.data(), order.data());
    diagram.execute(
        diagramOutput, normalizedScalarField.data(), 0, order.data(), &triangulation);
    // 3. computing the persistence curve from the persistence diagram
    ttk::PersistenceCurve curve;
    //std::array<ttk::PersistenceCurve::PlotType, 4> outputCurve;
    //curve.execute(outputCurve, diagramOutput);

    // 4. selecting the critical point pairs
    //std::vector<double> simplifiedField = normalizedScalarField;
    std::vector<ttk::SimplexId> authorizedCriticalPoints, simplifiedOrder = order;
    for (int i = 0; i < (int)diagramOutput.size(); i++) {
        if (diagramOutput[i].persistence() > 0.001 && diagramOutput[i].persistence() < 999999999) {
            // 5. selecting the most persistent pairs
            authorizedCriticalPoints.push_back(diagramOutput[i].birth.id);
            authorizedCriticalPoints.push_back(diagramOutput[i].death.id);
        }
    }

    // 6. simplifying the input data to remove non-persistent pairs
    ttk::TopologicalSimplification simplification;
    simplification.preconditionTriangulation(&triangulation);
    simplification.execute<double>(
        normalizedScalarField.data(), normalizedScalarField.data(), authorizedCriticalPoints.data(),
        order.data(), order.data(), authorizedCriticalPoints.size(),
        false, triangulation);

    //// assign the simplified values to the input mesh
    //for (int i = 0; i < (int)simplifiedHeight.size(); i++) {
    //    pointSet[3 * i + 2] = simplifiedHeight[i];
    //}

    //// 7. computing the Morse-Smale complex
    ttk::MorseSmaleComplex morseSmaleComplex;
    // critical points
    ttk::MorseSmaleComplex::OutputCriticalPoints outCriticalPoints{};
    // 1-separatrices
    ttk::MorseSmaleComplex::Output1Separatrices out1Separatrices{};
    // 2-separatrices
    ttk::MorseSmaleComplex::Output2Separatrices out2Separatrices{};
    // segmentation
    std::vector<ttk::SimplexId> ascendingSegmentation(
        triangulation.getNumberOfVertices(), -1),
        descendingSegmentation(triangulation.getNumberOfVertices(), -1),
        mscSegmentation(triangulation.getNumberOfVertices(), -1);
    ttk::MorseSmaleComplex::OutputManifold outSegmentation{
      ascendingSegmentation.data(), descendingSegmentation.data(),
      mscSegmentation.data() };

    morseSmaleComplex.preconditionTriangulation(&triangulation);
    //morseSmaleComplex.execute(
    //    outCriticalPoints, out1Separatrices, out2Separatrices, outSegmentation,
    //    simplifiedField.data(), 0, simplifiedOrder.data(), triangulation);
    morseSmaleComplex.execute(
        outCriticalPoints, out1Separatrices, out2Separatrices, outSegmentation,
        normalizedScalarField.data(), 0, order.data(), triangulation);

    ttk::MorseSmaleQuadrangulation morseSmaleQuadrangulation;
    morseSmaleQuadrangulation.preconditionTriangulation(&triangulation);
    morseSmaleQuadrangulation.setCriticalPoints(
        outCriticalPoints.points_.size(),
        outCriticalPoints.points_.data(),
        outCriticalPoints.PLVertexIdentifiers_.data(),
        outCriticalPoints.cellIds_.data(),
        outCriticalPoints.cellDimensions_.data());
    morseSmaleQuadrangulation.setSeparatrices(
        out1Separatrices.pt.numberOfPoints_,
        out1Separatrices.pt.cellIds_.data(),
        out1Separatrices.pt.cellDimensions_.data(),
        out1Separatrices.pt.smoothingMask_.data(),
        out1Separatrices.pt.points_.data()
    );
    morseSmaleQuadrangulation.setDualQuadrangulation(false);
    morseSmaleQuadrangulation.execute(triangulation);
    //triangulation.setInputPoints(
    //    out1Separatrices.pt.points_.size()/3, 
    //    out1Separatrices.pt.points_.data());
    //triangulation.setInputCells(
    //    out1Separatrices.cl., triangleSetCo.data(), triangleSetOff.data());
    struct Tree {
        std::array<long long int, 4>* current;
        std::vector<Tree*> children;
        Tree* parent;
        bool flip = false;
    };
    std::unordered_map<std::array<long long int, 4>*, bool> hasTree;
    std::unordered_map<std::pair<long long int, long long int>, std::vector<std::array<long long int, 4>*>, hash_pair> edges;
    for (auto& q : morseSmaleQuadrangulation.outputCells_) {
        for (int i = 0; i < 4; i++) {
            long long int idx1 = q[i], idx2 = q[(i + 1) % 4];
            if (idx1 > idx2) {
                std::swap(idx1, idx2);
            }
            edges[{idx1, idx2}].push_back(&q);
        }
    }

    Tree* root = new Tree({ &morseSmaleQuadrangulation.outputCells_[0], {}, nullptr });
    hasTree[&morseSmaleQuadrangulation.outputCells_[0]] = true;
    std::queue<Tree*> treeQueue;
    treeQueue.push(root);
    while (treeQueue.size() != 0) {
        std::array<long long int, 4>* rootQ = treeQueue.front()->current;
        for (int i = 0; i < 4; i++) {
            long long int idx1 = (*rootQ)[i], idx2 = (*rootQ)[(i + 1) % 4];
            if (idx1 > idx2) {
                std::swap(idx1, idx2);
            }
            for (auto& q : edges[{idx1, idx2}]) {
                if (hasTree[q] || q==rootQ) {
                    continue;
                }
                Tree* t = new Tree({ q, {}, treeQueue.front() });
                hasTree[q] = true;
                treeQueue.push(t);
                treeQueue.front()->children.push_back(t);
            }
        }
        treeQueue.pop();
    }
    treeQueue = std::queue<Tree*>();
    treeQueue.push(root);
    while (treeQueue.size() != 0) {
        Tree* cur = treeQueue.front();
        for (auto& child : cur->children) {
            bool nextChild = false;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if ((*cur->current)[i] == (*child->current)[j] && (*cur->current)[(i + 1) % 4] == (*child->current)[(j + 1) % 4]) {
                        std::reverse(child->current->begin(), child->current->end());
                        nextChild = true;
                        break;
                    }
                }
                if (nextChild) {
                    break;
                }
            }
            treeQueue.push(child);
        }
        treeQueue.pop();
    }

    ttk::QuadrangulationSubdivision quadrangulationSubdivision;
    quadrangulationSubdivision.preconditionTriangulation(&triangulation);
    quadrangulationSubdivision.setInputQuads(
        morseSmaleQuadrangulation.outputCells_.data(), 
        morseSmaleQuadrangulation.outputCells_.size()
    );
    quadrangulationSubdivision.setInputVertices(
        morseSmaleQuadrangulation.outputPoints_.data(),
        morseSmaleQuadrangulation.outputPoints_.size()/3
    );
    quadrangulationSubdivision.setInputVertexIdentifiers(
        morseSmaleQuadrangulation.outputPointsIds_.data(),
        morseSmaleQuadrangulation.outputPointsIds_.size()
    );
    quadrangulationSubdivision.setSubdivisionLevel(3);
    quadrangulationSubdivision.setRelaxationIterations(100);
    quadrangulationSubdivision.execute(triangulation);

    // save the output
    saveQuadPoints(quadrangulationSubdivision.outputPoints_, quadrangulationSubdivision.outputQuads_, "output.off");

    return 0;
}