#include "Controller.hpp"
#include "Simulation.hpp"
#include "PathAlgorithms.hpp"

Controller::Controller(int nonSatelliteNodeCount, int satelliteCount)
: _graph(nonSatelliteNodeCount, satelliteCount) 
{
    initGraph();
}

void Controller::run() {
    // std::vector<Transaction> transactions = parseTransactionFile("test.simc");
    // printTransactions(transactions);

    while (true)
    {
        cout << "Folainer$: ";
        string commandString;
        getline(cin, commandString);

        istringstream ss(commandString);

        vector<string> command;
        string token;

        while (ss >> token) {
            command.push_back(token);
        }

        try {
            if (command.size() <= 0 || command.size() > 4) {
                throw "prohibited";
            }

            string commandName = command[0];

            if (command.size() == 1) {
                if (commandName == "h" || commandName == "help") {
                    help();
                } 
                else if (commandName == "e" || commandName == "exit") 
                {
                    exit(0);
                }
                else 
                {
                    throw "prohibited";
                }
            }
            else if (command.size() == 2)
            {
                if (commandName == "show")
                {
                    if (command[1] == "graph")
                    {
                        _graph.output(cout);
                    }
                    else if (command[1] == "weight")
                    {
                        displayGraphWeights();
                    }
                    else if (command[1] == "mtu")
                    {
                        cout << "The current MTU value: " << MTU << endl;
                    }
                    else if (command[1] == "buffer_size")
                    {
                        cout << "The current router buffer size value: " << ROUTER_BUFFER_SIZE << endl;
                    }
                    else if (command[1] == "ttl")
                    {
                        cout << "The current global ttl for new packet has value: " << TTL << endl;
                    }
                    else if (command[1] == "full_weight")
                    {
                        displayFullGraphWeights();
                    }
                    else
                    {
                        throw "prohibited";
                    }
                }
            }
            else if (command.size() == 3)
            {
                if (commandName == "show")
                {
                    if (command[1] == "distance")
                    {
                        displayNodeDistance(stoi(command[2]));
                    }
                    else if (command[1] == "path_table")
                    {
                        displayPathTable(stoi(command[2]));
                    }
                    else if (command[1] == "backup_path_table")
                    {
                        displayBackDistanceTable(stoi(command[2]));
                    }
                    else if (command[1] == "error_from")
                    {
                        displayErrorFrom(stoi(command[2]));
                    }
                }
                else if (commandName == "change")
                {
                    if (command[1] == "mtu")
                    {
                        MTU = stoi(command[2]);
                    }
                    else if (command[1] == "buffer_size")
                    {
                        ROUTER_BUFFER_SIZE = stoi(command[2]);
                    }
                    else if (command[1] == "ttl")
                    {
                        TTL = stoi(command[2]);
                    }
                }
                else if (commandName == "simulate")
                {
                    Simulation(command[1], _graph, *this).run(command[2]);
                }
            }
            else if (command.size() == 4)
            {
                if (commandName == "show")
                {
                    if (command[1] == "path")
                    {
                        displayPath(stoi(command[2]), stoi(command[3]));
                    }
                    else if (command[1] == "backup_path")
                    {
                        displayBackDistance(stoi(command[2]), stoi(command[3]));
                    }
                }
            }
        }
        catch (...) {
            cout << "Incorect command, please use 'help' command" << endl;
        }
    }
}

void Controller::help() const {
    int indent = 30;
    cout << "Information about available commands:" << endl;
    cout << left << setw(indent) << "h|help" << "Shows information about available commands" << endl;
    cout << left << setw(indent) << "show graph" << "Shows information about graph" << endl;
    cout << left << setw(indent) << "show weight" << "Shows information about weights of graph" << endl;
    cout << left << setw(indent) << "show full_weight" << "Shows information about full weights of graph: latency_ms and bandwidth_mbps" << endl;
    cout << left << setw(indent) << "show mtu" << "Shows global mtu in the network" << endl;
    cout << left << setw(indent) << "show buffer_size" << "Shows global router buffer size in the network" << endl;
    cout << left << setw(indent) << "show ttl" << "Shows global ttl for new packet in the network" << endl;
    cout << left << setw(indent) << "show distance <id>" << "Shows information about dijksta's short path" << endl;
    cout << left << setw(indent) << "show path <from> <to>" << "Shows the shortest path using dijksta's algorithm" << endl;
    cout << left << setw(indent) << "show path_table <from>" << "Shows the shortest path table using dijksta's algorithm in a specific router" << endl;
    cout << left << setw(indent) << "show backup_path <from> <to>" << "Shows the backup path" << endl;
    cout << left << setw(indent) << "show backup_path_table <from>" << "Shows the backup path table using modified dijksta's algorithm in a specific router" << endl;
    cout << left << setw(indent) << "show error_from <from>" << "Shows erros in edges from selected node" << endl;
    cout << left << setw(indent) << "change mtu <value>" << "Cahnges global mtu in the network" << endl;
    cout << left << setw(indent) << "change buffer_size <value>" << "Cahnges global router buffer size in the network" << endl;
    cout << left << setw(indent) << "change ttl" << "Changes global ttl for new packet in the network" << endl;
    cout << left << setw(indent) << "simulate <ifile> <ofile>" << "Simulates network pockets transmission using input and output files" << endl;
    cout << left << setw(indent) << "e|exit" << "Exits" << endl;
}

void Controller::initGraph()
{
    Random r(0.3);
    Weight toSat(12, 10);
    Weight betweenSat(17, 10);

    int channel_weights[] = {3, 5, 6, 8, 10, 12, 17, 20, 25, 27};
    
    _graph.addNonDirectedEdge(0, 1, betweenSat, HALF_DUPLEX, (r.getValue() + 0.1) / 100.0);
    _graph.addNonDirectedEdge(0, 6, toSat, HALF_DUPLEX, (r.getValue() + 0.1) / 100.0);
    _graph.addNonDirectedEdge(0, 11, toSat, HALF_DUPLEX, (r.getValue() + 0.1) / 100.0);

    _graph.addNonDirectedEdge(1, 15, toSat, HALF_DUPLEX, (r.getValue() + 0.1) / 100.0);
    _graph.addNonDirectedEdge(1, 25, toSat, HALF_DUPLEX, (r.getValue() + 0.1) / 100.0);

    // up to 22 is done
    int connectionsHalfDuplex[][2] = {{2, 12}, {2, 9}, {3, 12}, {4, 13}, {5, 6}, {6, 7}, {7, 8}, {7, 23}, {11, 12}, {11, 13}, {15, 17}, {16, 18}, {16, 19}, {16, 22}, {18, 21}, {19, 21}, {19, 22}, {20, 23}, {21, 24}, {22, 25}, {23, 24}, {24, 25}};
    int connectionsDuplex[][2] = {{2, 3}, {3, 5}, {4, 5}, {4, 8}, {8, 9}, {9, 10}, {10, 13}, {10, 14}, {14, 15}, {14, 17}, {17, 20}, {18, 20}};

    for (auto& connection : connectionsHalfDuplex)
    {
        int rint = (int)(r.getValue()*100) % 10;
        _graph.addNonDirectedEdge(connection[0], connection[1], Weight(channel_weights[rint], 50), HALF_DUPLEX, (r.getValue() / 10 + 0.01) / 100);
    }
    
    for (auto& connection : connectionsDuplex)
    {
        int rint = (int)(r.getValue()*100) % 10;
        _graph.addNonDirectedEdge(connection[0], connection[1], Weight(channel_weights[rint], 50), DUPLEX, (r.getValue() / 10 + 0.01) / 100);
    }
}

void Controller::displayGraphWeights() const
{
    int index = 0;
    for (auto& nodeNeighbours : _graph.adj)
    {
        for (auto& edge: nodeNeighbours)
        {
            cout << index << "->" << edge.to << ":" << edge.weight.calculate() << ' ';
        }

        cout << '\n';

        index++;
    }
}

void Controller::displayFullGraphWeights() const
{
    int index = 0;
    for (auto& nodeNeighbours : _graph.adj)
    {
        for (auto& edge: nodeNeighbours)
        {
            cout << index << "->" << edge.to << ":" << edge.weight.latency_ms << "ms:" << edge.weight.bandwidth_mbps << "mbps ";
        }

        cout << '\n';

        index++;
    }
}

void Controller::displayNodeDistance(int id) const
{
    Graph& graph = const_cast<Graph&>(_graph);
    Node& node = const_cast<Node&>(graph.nodes.at(id));

    if (!node.isCalculated())
    {
        node.fillTable(graph, id);
    }

    for (size_t i = 0; i < node.distTable.size(); i++)
    {
        cout << id << "->" << i << ": " << node.distTable[i] << '\n';
    }
}

vector<int> Controller::findPath(int from, int to) const
{
    Graph& graph = const_cast<Graph&>(_graph);
    Node& node = const_cast<Node&>(graph.nodes.at(from));

    if (!node.isCalculated())
    {
        node.fillTable(graph, from);
    }

    return PathAlgorithm::reconstruct_nodes_from_parentEdges(to, node.parentTable, graph.edgeEndpoints);
}


void Controller::displayPath(int from, int to) const
{
    vector<int> path = findPath(from, to);

    cout << "Path " << from << "->" << to << ": ";

    for (auto& nodeId : path)
    {
        cout << nodeId;
        if (nodeId == path.at(path.size() - 1))
        {
            break;
        }
        cout << "->";
    }

    cout << '\n';
}

void Controller::displayPathTable(int from) const
{
    Graph& graph = const_cast<Graph&>(_graph);
    Node& node = const_cast<Node&>(graph.nodes.at(from));

    int n = graph.n;

    if (!node.isCalculated())
    {
        node.fillTable(graph, from);
    }

    cout << "Path table for " << from << " router" << endl;

    for (int i = 0; i < n; i++)
    {
        
        auto path = PathAlgorithm::reconstruct_nodes_from_parentEdges(i, node.parentTable, graph.edgeEndpoints);
        
        cout << "Path " << from << "->" << i << ": ";

        for (auto& nodeId : path)
        {
            cout << nodeId;
            if (nodeId == path.at(path.size() - 1))
            {
                break;
            }
            cout << "->";
        }

        cout << '\n';
    }
}


void Controller::displayBackDistance(int from, int to) const
{
    Graph& graph = const_cast<Graph&>(_graph);
    vector<int> reservedPath = graph.nodes.at(from).findReservedPath(from, to, graph);

    if (reservedPath.size() == 0)
    {
        cout << "There are no backup path " << from << "->" << to << endl;
        return;
    }

    cout << "Backup path " << from << "->" << to << ": ";

    for (auto& nodeId : reservedPath)
    {
        cout << nodeId;
        if (nodeId == reservedPath.at(reservedPath.size() - 1))
        {
            break;
        }
        cout << "->";
    }

    cout << '\n';
}

void Controller::displayBackDistanceTable(int from) const
{
    cout << "Backup path table for " << from << " router" << endl;
    for (int i = 0; i < _graph.n; i++)
    {
        displayBackDistance(from, i);
    }
}

void Controller::displayErrorFrom(int from) const
{

    cout << "Edges errors from " << from << " node:" << endl;
    for (auto edge : this->_graph.adj.at(from))
    {
        string typeStr;
        if (edge.type == DUPLEX)
        {
            typeStr = "Duplex";
        }
        else 
        {
            typeStr = "Half duplex";
        }

        cout << '[' << from << "->" << edge.to << "] Error probalility: " << round(edge.p_error * 1000 * 100) / 1000.0 << "%, Channel type: " << typeStr << endl;
    }
}