#include <mpi.h>
#include <cassert>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <chrono>
#include <cstring>
#include <cmath>
using namespace std;
using namespace std::chrono;

#define MATRIX_TAG 0

constexpr int N = 1e3 + 5;
constexpr int mod = 1e9 + 7;

class Setting {
public:
    int numThread;
    string input1, input2;
    string output;
private:
    Setting() {
        numThread = 1;
    }
public:
    static Setting& getInstance() {
        static Setting instance;
        return instance;
    }

    void parseArg(int argc, char **argv) {
        for (int i = 0; i < argc; ++i) {
            if (strcmp(argv[i], "--nt") == 0 ||
                strcmp(argv[i], "--num-threads") == 0 ||
                strcmp(argv[i], "-nt") == 0 ||
                strcmp(argv[i], "-num-threads") == 0) {

                numThread = atoi(argv[i + 1]);
            }

            if (strcmp(argv[i], "--i1") == 0 ||
                strcmp(argv[i], "--input1") == 0 ||
                strcmp(argv[i], "-i1") == 0 ||
                strcmp(argv[i], "-input1") == 0) {
                    
                input1 = argv[i + 1];
            }

            if (strcmp(argv[i], "--i2") == 0 ||
                strcmp(argv[i], "--input2") == 0 ||
                strcmp(argv[i], "-i2") == 0 ||
                strcmp(argv[i], "-input2") == 0) {
                    
                input2 = argv[i + 1];
            }

            if (strcmp(argv[i], "--o") == 0 ||
                strcmp(argv[i], "--output") == 0 ||
                strcmp(argv[i], "-o") == 0 ||
                strcmp(argv[i], "-output") == 0) {
                    
                output = argv[i + 1];
            }
        }
    }
};

#define MASTER 0

class MPIHelper {
private:
    int rank, numProcs;
    MPIHelper() {}
public:
    static MPIHelper& getInstance() {
        static MPIHelper instance;
        return instance;
    }

    void init(int argc, char **argv) {
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    }

    int getRank() {
        return rank;
    }

    int getNumProcs() {
        return numProcs;
    }

    bool isMaster() {
        return rank == 0;
    }

    bool isWorker() {
        return !isMaster();
    }

    void barrier() {
        MPI_Barrier(MPI_COMM_WORLD);
    }

    void finalize() {
        MPI_Finalize();
    }

    void sendString (string message, int dest, int tag) {
        char *buf = (char *)message.c_str();
        MPI_Send(message.c_str(), message.size() + 1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }

    bool gotMessage() {
        MPI_Status status;
        int flag;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        return flag;
    }

    pair<int, int> receiveString (string &message, int source = MPI_ANY_SOURCE, int tag = MPI_ANY_TAG) {
        MPI_Status status;
        MPI_Probe(source, tag, MPI_COMM_WORLD, &status);
        int msgCount;
        MPI_Get_count(&status, MPI_CHAR, &msgCount);

        char *recvBuffer = new char[msgCount];
        MPI_Recv(recvBuffer, msgCount, MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);
        message = recvBuffer;

        return {status.MPI_SOURCE, status.MPI_TAG};
    }
};

#define mpiout MPIOut::getInstance()
class MPIOut {
    private: 
        bool disableOutput;
        MPIOut() {
                disableOutput = false;
        }
	public:

		template<class TArg>
		MPIOut &operator<<(TArg arg) {
			if (MPIHelper::getInstance().isMaster() && !(this)->disableOutput) cout << arg;
			return (*this);
		}

		static MPIOut &getInstance() {
			static MPIOut instance;
			return instance;
		}

        void setDisableOutput(bool disableOutput) {
            this->disableOutput = disableOutput;
        }
};

class Matrix {
public:
    int nRows, nCols;
    int a[N][N];

    Matrix(int nRows = 0, int nCols = 0) {
        this->nRows = nRows;
        this->nCols = nCols;
        memset(a, 0, sizeof a);
    }

    Matrix operator * (const Matrix &b) {
        assert(nCols == b.nRows);
        Matrix ans(nRows, b.nCols);

        for (int i = 0; i < ans.nRows; ++i)
            for (int j = 0; j < ans.nCols; ++j)
                for (int t = 0; t < nCols; ++t)
                    (ans.a[i][j] += 1ll * a[i][t] * b.a[t][j] % mod) %= mod;

        return ans;
    }
    
    // overload operator >> to read matrix
    friend istream& operator >> (istream &is, Matrix &x)
    {
        is >> x.nRows >> x.nCols;
        for (int i = 0; i < x.nRows; ++i)
            for (int j = 0; j < x.nCols; ++j)
                is >> x.a[i][j];
        
        return is;
    }
};

Matrix a, b, ans;

void Read() {
    ifstream in(Setting::getInstance().input1);
    in >> a;
    in.close();

    in.open(Setting::getInstance().input2);
    in >> b;
    in.close();
}

void Solve() {
    long pid = MPIHelper::getInstance().getRank();
    int numProcs = MPIHelper::getInstance().getNumProcs();
    int p = (a.nRows + numProcs - 1) / numProcs;

    Matrix a2;
    a2.nCols = a.nCols;
    for (int i = p * pid; i < p * (pid + 1) && i < a.nRows; ++i) {
        ++a2.nRows;
        for (int j = 0; j < a.nCols; ++j) {
            a2.a[i - p * pid][j] = a.a[i][j];
        }
    }

    ans = a2 * b;

    if (MPIHelper::getInstance().isWorker()) {
        stringstream ss;
        ss << ans.nRows << " " << ans.nCols << "\n";
        for (int i = 0; i < ans.nRows; ++i) {
            for (int j = 0; j < ans.nCols; ++j) {
                ss << ans.a[i][j] << " ";
            }
            ss << "\n";
        }
        MPIHelper::getInstance().sendString(ss.str(), MASTER, MATRIX_TAG);
    } else {
        ans.nRows = a.nRows;
        ans.nCols = b.nCols;

        int cnter = numProcs - 1;

        while (cnter > 0) {
            if (MPIHelper::getInstance().gotMessage()) {
                string message;
                auto [source, tag] = MPIHelper::getInstance().receiveString(message);
                
                stringstream ss(message);
                int nRows, nCols;
                ss >> nRows >> nCols;

                assert(nCols == ans.nCols);

                for (int i = source * p, j = 0; j < nRows; ++i, ++j) {
                    for (int k = 0; k < nCols; ++k) {
                        ss >> ans.a[i][k];
                    }
                }

                --cnter;
            }
        }
    }
}


int main(int argc, char **argv)
{
    Setting::getInstance().parseArg(argc, argv);
    MPIHelper::getInstance().init(argc, argv);

    Read();

    freopen(Setting::getInstance().output.c_str(), "w", stdout);

    // int startTime = 1000 * clock() / CLOCKS_PER_SEC;
    // auto startTime = high_resolution_clock::now();
    struct timespec startTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    Solve();

    MPIHelper::getInstance().barrier();
    MPIHelper::getInstance().finalize();

    mpiout << "Main: All processes completed!!!\n";

    // Print result
    mpiout << ans.nRows << " " << ans.nCols << "\n";
    for (int i = 0; i < a.nRows; ++i) {
        for (int j = 0; j < b.nCols; ++j) {
            mpiout << ans.a[i][j] << " ";
        }
        mpiout << "\n";
    }

    // int endTime = 1000 * clock() / CLOCKS_PER_SEC;
    // auto endTime = high_resolution_clock::now();
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &endTime);
    double elapsed = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;
    mpiout << "Time: " << round(elapsed * 1000) << "ms\n";
    return 0;
}