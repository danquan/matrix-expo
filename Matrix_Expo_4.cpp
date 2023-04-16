#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <ctime>
using namespace std;

#define MASTER_PROC 0
int rankProcess, numProcess;
int len_name;
char name[MPI_MAX_PROCESSOR_NAME];

constexpr int N = 1e3 + 5;
constexpr int mod = 1e9 + 7;

struct Matrix{
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

        for(int i = 0; i < ans.nRows; ++i)
            for(int j = 0; j < ans.nCols; ++j)
                for(int t = 0; t < nCols; ++t)
                    (ans.a[i][j] += 1ll * a[i][t] * b.a[t][j] % mod) %= mod;
        
        return ans;
    }
    friend void readMatrix (Matrix &x) {
        cout << "Moi nhap kich thuoc ma tran (Hang - cot): ";
        cin >> x.nRows >> x.nCols;
        
        cout << "Moi nhap ma tran:\n";
        for(int i = 0; i < x.nRows; ++i)
            for(int j = 0; j < x.nCols; ++j)
                cin >> x.a[i][j];
    }
} A, B, res;

// send Matrix
void sendMatrix(const int destProcess, int &cntTag, const Matrix &A, const pair<int, int> &leftUp, const pair<int, int> &rightDown)
{
    int nRows = rightDown.first - leftUp.first + 1,
        nCols = rightDown.second - leftUp.second + 1;

    int encodeSize = (nRows <= 0 || nCols <= 0) ? 0 : nRows * 10000 + nCols;

    MPI_Send(&encodeSize, 1, MPI_INT, destProcess, ++cntTag, MPI_COMM_WORLD);

    if (encodeSize != 0)
    {
        int *encodeRow = new int[nCols];
        for (int x = leftUp.first; x <= rightDown.first; ++x)
        {

            for (int y = leftUp.second; y <= rightDown.second; ++y)
            {
                encodeRow[(y - leftUp.second)] = A.a[x][y];
            }

            MPI_Send(encodeRow, nCols, MPI_INT, destProcess, ++cntTag, MPI_COMM_WORLD);
        }
        // Free memory
        delete[] encodeRow;
    }
}

void recvMatrix(const int &srcProcess, int &cntTag, Matrix &A)
{
    int decodeSize;
    MPI_Recv(&decodeSize, 1, MPI_INT, srcProcess, ++cntTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    A.nRows = decodeSize / 10000;
    A.nCols = decodeSize % 10000;

    if (decodeSize != 0)
    {

        int *decodeRow = new int[A.nCols];

        for (int i = 0; i < A.nRows; ++i)
        {
            MPI_Recv(decodeRow, A.nCols, MPI_INT, srcProcess, ++cntTag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < A.nCols; ++j)
                A.a[i][j] = decodeRow[j];
        }

        // Free memory
        delete[] decodeRow;
    }
}

void Read() {

    readMatrix(A); //Read A
    readMatrix(B); //Read B
    
    res.nRows = A.nRows;
    res.nCols = B.nCols;
}

void Solve() {

    int cntTag = 0;

    if(rankProcess != MASTER_PROC) { // receive matrix
        cntTag = 0;
        recvMatrix( (rankProcess - 1) / 2, cntTag, A );
        recvMatrix( (rankProcess - 1) / 2, cntTag, B );
    }

    //send matrix using idea divide and conquer
    if(rankProcess * 2 + 1 < numProcess) {
        cntTag = 0;
        sendMatrix( rankProcess * 2 + 1, cntTag, A, {0, 0}, {A.nRows - 1, A.nCols - 1});
        sendMatrix( rankProcess * 2 + 1, cntTag, B, {0, 0}, {B.nRows - 1, B.nCols - 1});
    }
    if( rankProcess * 2 + 2 < numProcess ) {
        cntTag = 0;
        sendMatrix( rankProcess * 2 + 2, cntTag, A, {0, 0}, {A.nRows - 1, A.nCols - 1});
        sendMatrix( rankProcess * 2 + 2, cntTag, B, {0, 0}, {B.nRows - 1, B.nCols - 1});
    }

    // Let's resize matrices
    {
        int len = (A.nRows + numProcess - 1) / numProcess;
        int l = len * rankProcess,
            r = min( len * (rankProcess + 1) - 1, A.nRows - 1);
        
        if(l > r) {
            A.nRows = 0;
        }
        else {
            for(int i = l; i <= r; ++i) 
                for(int j = 0; j < A.nCols; ++j)
                    A.a[i - l][j] = A.a[i][j];
            
            A.nRows = r - l + 1;
        }
    }

    A = A * B;

    cntTag = 0;
    sendMatrix(MASTER_PROC, cntTag, A, {0, 0}, {A.nRows - 1, A.nCols - 1});

    if(rankProcess == MASTER_PROC) 
    {
        res.nRows = 0;
        for(int src = 0; src < numProcess; ++src) 
        {
            cntTag = 0;
            recvMatrix(src, cntTag, B);

            // assert(res.nCols == B.nCols);
            // Don't use assert because some process compute null matrix
            if(res.nCols == B.nCols)
                for(int i = 0; i < B.nRows; ++i)
                    for(int j = 0; j < res.nCols; ++j)
                        (res.a[i + res.nRows][j] += B.a[i][j]) %= mod;

            res.nRows += B.nRows;
        }

        //Print answer
        for(int i = 0; i < res.nRows; ++i)
            for(int j = 0; j < res.nCols; ++j)
                //cout << res.a[i][j] << (j == res.nCols - 1 ? "\n" : " ");
                cout << (res.a[i][j] + mod) % mod << (j == res.nCols - 1 ? "\n" : " ");
    }
}

void init(int &argc, char **&argv);
int main(int argc, char **argv)
{
    init(argc, argv);

    if(fopen("testMatrix.INP", "r")) {
        freopen("testMatrix.INP", "r", stdin);
        freopen("testMatrix.OUT", "w", stdout);
    }

    if (rankProcess == MASTER_PROC)
        Read();

    Solve();
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rankProcess == MASTER_PROC)
        cerr << "\nTime elapsed: " << 1000 * clock() / CLOCKS_PER_SEC << "ms\n";

    MPI_Finalize();
    return 0;
}

void init(int &argc, char **&argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcess);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankProcess);
    MPI_Get_processor_name(name, &len_name);
}