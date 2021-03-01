/* 
    This program is free software: you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or 
    (at your option) any later version. 
 
    This program is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
    GNU General Public License for more details. 
 
    You should have received a copy of the GNU General Public License 
    along with this program.  If not, see <http://www.gnu.org/licenses/> 
*/

// local includes
#include "Tools.h"
#include "DegeneracyAlgorithm.h"
#include "CliqueTools.h"

// system includes
#include <map>
#include <list>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

/*! \file main.cpp

    \brief Main entry point for quick cliques software. This is where we parse the command line options, read the inputs, and decide which clique method to run.

    \author Darren Strash (first name DOT last name AT gmail DOT com)

    \copyright Copyright (c) 2011-2016 Darren Strash. This code is released under the GNU Public License (GPL) 3.0.

    \image html gplv3-127x51.png

    \htmlonly
    <center>
    <a href="gpl-3.0-standalone.html">See GPL 3.0 here</a>
    </center>
    \endhtmlonly
*/

bool   bQuiet;
bool   bOutputLatex;
bool   bOutputTable;
string inputFile;
string algorithm;
bool   staging;

bool   bTableMode;
int K;

const int BLOCK = 30000;


void PrintDebugWarning()
{
    cout << "\n\n\n\n\n" << flush;
    cout << "#########################################################################" << endl << flush;
    cout << "#                                                                       #" << endl << flush;
    cout << "#    WARNING: Debugging is turned on. Don't believe the run times...    #" << endl << flush;
    cout << "#                                                                       #" << endl << flush;
    cout << "#########################################################################" << endl << flush;
    cout << "\n\n\n\n\n" << flush;
}

void PrintHeader()
{
    cout << "NOTE: Quick Cliques v2.0beta." << endl;
}

string basename(string const &fileName)
{
    string sBaseName(fileName);

    size_t const lastSlash(sBaseName.find_last_of("/\\"));
    if (lastSlash != string::npos) {
        sBaseName = sBaseName.substr(lastSlash+1);
    }

    size_t const lastDot(sBaseName.find_last_of("."));
    if (lastDot != string::npos) {
        sBaseName = sBaseName.substr(0, lastDot);
    }

    return sBaseName;
}


int n; // number of vertices
int m; // 2x number of edges
class ETKC_Solver {
    class Solution {
    private:
        vector<int> vertex_cover;
        vector<list<int>> choose;
    public:
        int value;
        void init() {
            vertex_cover.resize(n);
            for (int i = 0; i < n; ++i) {
                vertex_cover[i] = 0;
            }
            value = 0;
            choose.reserve(K);
        }
        bool operator< (const Solution &rhs) const {
            return value < rhs.value;
        }
        void add(int x) {
            if (++vertex_cover[x] == 1) ++value;
       }
        void del(int x) {
            if (--vertex_cover[x] == 0) --value;
        }
        void proc(list<int> &clique, const char &ch) {
            if (ch == 'A') choose.push_back(clique);
            else choose.pop_back();
            for (const int &x : clique) {
                if (ch == 'A') add(x);
                else if (ch == 'D') del(x);
                else assert(false);
            }
        }
    } now_solution, global_best;
private:
    vector<list<int>> adjacencyList;
    vector<long> vertex_clique;
    vector<Algorithm *> vpAlgo;
    vector<list<list<int>>> vcliques;
    vector<list<list<int>>::iterator> vit;
    vector<int> vnow_vertex;
    vector<long> vclique_left, vclique_right;
    vector<bool> vfinish;

    Algorithm* new_algorithm(int id) {
        vnow_vertex[id] = 0;
        vclique_left[id] = 0;
        vclique_right[id] = 0;
        vfinish[id] = false;
        vcliques[id].clear();
        Algorithm *pAlgorithm(nullptr);
        if (algorithm == "degeneracy") {
            pAlgorithm = new DegeneracyAlgorithm(adjacencyList);
        } else {
            cout << "ERROR: unrecognized algorithm name: " << algorithm << endl;
            exit(1);
        }

    #ifdef PRINT_CLIQUES_ONE_BY_ONE
        auto printClique = [bOneBasedVertexIds](list<int> const &clique) {
            list<int>::const_iterator it = clique.begin();

            int offset = bOneBasedVertexIds ? 1 : 0;

            if (it != clique.end()) {
                printf("%d", *it + offset); //cout << *it;
                ++it;
            }
            while (it != clique.end()) {
                printf(" %d", *it + offset); //cout << " " << *it;
                ++it;
            }
            printf("\n"); //cout << endl;
        };

        pAlgorithm->AddCallBack(printClique);
    #endif //PRINT_CLIQUES_ONE_BY_ONE

        // Run algorithm
        

    #ifdef RETURN_CLIQUES_ONE_BY_ONE
        auto storeCliqueInList = [&cliques](list<int> const &clique) {
            cliques.push_back(clique);
        };
        pAlgorithm->AddCallBack(storeCliqueInList);
    #endif //RETURN_CLIQUES_ONE_BY_ONE

        pAlgorithm->SetQuiet(bQuiet);

    ////    cout << "Last clique has size: " << cliques.back().size() << endl << flush;
        return pAlgorithm;
    }
    
    void init() {
        if (!bTableMode) {
            PrintHeader();
    #ifdef DEBUG_MESSAGE
            PrintDebugWarning();
    #endif //DEBUG_MESSAGE
        }

        bool bOneBasedVertexIds(false);
        if (inputFile.find(".graph") != string::npos) {
            if (!bTableMode) cout << "Detected .graph extension, reading METIS file format. " << endl << flush;
            adjacencyList = readInGraphAdjListEdgesPerLine(n, m, inputFile);
            bOneBasedVertexIds = true;
        } else {
            if (!bTableMode) cout << "Reading .edges file format: one edge per line. " << endl << flush;
            adjacencyList = readInGraphAdjList(n, m, inputFile);
        }

        bool const bComputeAdjacencyMatrix(adjacencyList.size() < 20000);
        bool const bShouldComputeAdjacencyMatrix(algorithm == "tomita");

        if (bShouldComputeAdjacencyMatrix && !bComputeAdjacencyMatrix) {
            cout << "ERROR!: unable to compute adjacencyMatrix, since the graph is too large: " << adjacencyList.size() << " vertices." << endl << flush;
            exit(1);
        }

        vertex_clique.resize(n);
        fill(vertex_clique.begin(), vertex_clique.end(), -1);

        vpAlgo.resize(K);
        vcliques.resize(K);
        vnow_vertex.resize(K);
        vclique_left.resize(K);
        vclique_right.resize(K);
        vfinish.resize(K);
        vit.resize(K);

        now_solution.init();
        global_best.init();
    }
    void get_next_block(int x) {
        vclique_left[x] = vclique_right[x];
        vclique_right[x] += BLOCK;
        // if (x == 0) {
        //     cerr << "x: " << x << "[" << vclique_left[x] << ", " << vclique_right[x] << "]" << endl;
        // }
        vcliques[x].clear();
        bool done;
        RunAndPrintStats(vpAlgo[x], vcliques[x], bTableMode, vertex_clique, vnow_vertex[x], vclique_left[x], vclique_right[x], done);
        // cerr << "RunAndPrintStats done!" << endl;
        // cerr << "vcliques size: " << vcliques[x].size() << endl;
        
        vit[x] = vcliques[x].begin();
        if (done) {
            vfinish[x] = true;
        }
        // cerr << "get_next_block done!" << endl;
    }
    void get_next_clique(int x, bool &done, list<int> &clique) {
        done = false;
        clique = *vit[x];
        ++vit[x];
        if (vit[x] == vcliques[x].end()) {
            if (vfinish[x]) {
                done = true;
            } else {
                get_next_block(x);
            }
        }
        // cerr << "get_next_clique done!" << endl;
    }
    void dfs(int x) {
        if (x == K) {
            if (global_best < now_solution) {
                global_best = now_solution;
            }
            return ;
        }
        vpAlgo[x] = new_algorithm(x);
        get_next_block(x);
        // cerr << "!!! x: " << x << endl;
        // int cnt = 0;
        while (true) {
            list<int> now_clique;
            bool done;
            get_next_clique(x, done, now_clique);
            // cerr << "now clique: ";
            // for (const int &vtx : now_clique) {
            //     cerr << vtx << " ";
            // }
            // cerr << endl;
            if (done) break;
            now_solution.proc(now_clique, 'A');
            dfs(x + 1);
            now_solution.proc(now_clique, 'D');
            // if (x == 0) {
            //     ++cnt;
            //     cerr << "cnt: " << cnt << endl;
            // }
        }
    }
public:


    int solve() {
        init();
        cout << "### linxi testing!!!" << endl;
        clock_t start = clock();
        dfs(0);
        clock_t end = clock();
        fprintf(stderr, "cost %f seconds\n", (double)(end-start)/(double)(CLOCKS_PER_SEC));
        cout << "best solution: " << global_best.value << endl;
        ////CommandLineOptions options = ParseCommandLineOptions(argc, argv);

        ////if (options.verify) {
        ////}
        return 0;
    }
} solver;

bool isValidAlgorithm(string const &name)
{
    return (name == "tomita" || name == "adjlist" || name == "hybrid" || name == "degeneracy");
}

void ProcessCommandLineArgs(int const argc, char** argv, map<string, string> &mapCommandLineArgs)
{
    for (int i = 1; i < argc; ++i) {
////        cout << "Processing argument " << i << endl;
        string const argument(argv[i]);
////        cout << "    Argument is " << argument << endl;
        size_t const positionOfEquals(argument.find_first_of("="));
////        cout << "    Position of = " << positionOfEquals << endl;
        if (positionOfEquals != string::npos) {
            string const key  (argument.substr(0,positionOfEquals));
            string const value(argument.substr(positionOfEquals+1));
////            cout << "    Parsed1: " << key << "=" << value << endl;
            mapCommandLineArgs[key] = value;
        } else {
////            cout << "    Parsed2: " << argument << endl;
            mapCommandLineArgs[argument] = "";
        }
    }
}

int main(int argc, char** argv)
{
    int failureCode(0);

    map<string, string> mapCommandLineArgs;

    ProcessCommandLineArgs(argc, argv, mapCommandLineArgs);

    bQuiet = (mapCommandLineArgs.find("--verbose") == mapCommandLineArgs.end());
    bOutputLatex = (mapCommandLineArgs.find("--latex") != mapCommandLineArgs.end());
    bOutputTable = (mapCommandLineArgs.find("--table") != mapCommandLineArgs.end());
    inputFile = ((mapCommandLineArgs.find("--input-file") != mapCommandLineArgs.end()) ? mapCommandLineArgs["--input-file"] : "");
    algorithm = ((mapCommandLineArgs.find("--algorithm") != mapCommandLineArgs.end()) ? mapCommandLineArgs["--algorithm"] : "");
    staging = (mapCommandLineArgs.find("--staging") != mapCommandLineArgs.end());
    
    assert(mapCommandLineArgs.find("--K") != mapCommandLineArgs.end());
    K = stoi(mapCommandLineArgs["--K"]);

    if (inputFile.empty()) {
        cout << "ERROR: Missing input file " << endl;
        // ShowUsageMessage();
        // return 1; // TODO/DS
    }

    if (algorithm.empty()) {
        cout << "ERROR: Missing algorithm" << endl;
        // ShowUsageMessage();
        // return 1; // TODO/DS
    }

    if (argc <= 1 || !isValidAlgorithm(algorithm) || inputFile.empty()) {
        cout << "USAGE: " << argv[0] << " --input-file=<filename> --algorithm=<tomita|adjlist|degeneracy|hybrid>" << endl;
        return 1;
    }



    bTableMode = (bOutputLatex || bOutputTable);

    int ret = solver.solve();

    return ret;
}
