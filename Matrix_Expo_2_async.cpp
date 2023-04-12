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

struct Matrix
{
    int nRows, nCols;
    int a[N][N];

    Matrix(int nRows = 0, int nCols = 0)
    {
        this->nRows = nRows;
        this->nCols = nCols;
        memset(a, 0, sizeof a);
    }

    Matrix operator*(const Matrix &b)
    {
        assert(nCols == b.nRows);
        Matrix ans(nRows, b.nCols);

        for (int i = 0; i < ans.nRows; ++i)
            for (int j = 0; j < ans.nCols; ++j)
                for (int t = 0; t < nCols; ++t)
                    (ans.a[i][j] += 1ll * a[i][t] * b.a[t][j] % mod) %= mod;

        return ans;
    }
    friend void readMatrix(Matrix &x)
    {
        cout << "Moi nhap kich thuoc ma tran (Hang - cot): ";
        cin >> x.nRows >> x.nCols;

        cout << "Moi nhap ma tran:\n";
        for (int i = 0; i < x.nRows; ++i)
            for (int j = 0; j < x.nCols; ++j)
                cin >> x.a[i][j];
    }
} A, B, res;

// send Matrix
void sendMatrix(const int destProcess, int &cntTag, const Matrix &A, const pair<int, int> &leftUp, const pair<int, int> &rightDown)
{
    int nRows = rightDown.first - leftUp.first + 1,
        nCols = rightDown.second - leftUp.second + 1;

    int encodeSize = (nRows <= 0 || nCols <= 0) ? 0 : nRows * 10000 + nCols;

    MPI_Request request_size;
    MPI_Isend(&encodeSize, 1, MPI_INT, destProcess, ++cntTag, MPI_COMM_WORLD, &request_size);
    MPI_Wait(&request_size, MPI_STATUS_IGNORE);

    if (encodeSize != 0)
    {
        int *encodeRow = new int[nCols];
        for (int x = leftUp.first; x <= rightDown.first; ++x)
        {

            for (int y = leftUp.second; y <= rightDown.second; ++y)
            {
                encodeRow[(y - leftUp.second)] = A.a[x][y];
            }

            MPI_Request request_row;
            MPI_Isend(encodeRow, nCols, MPI_INT, destProcess, ++cntTag, MPI_COMM_WORLD, &request_row);
            MPI_Wait(&request_row, MPI_STATUS_IGNORE);
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

void Read()
{
    readMatrix(A); // Read A
    readMatrix(B); // Read B

    res.nRows = A.nRows;
    res.nCols = B.nCols;

    int len = (A.nCols + numProcess - 1) / numProcess;

    // Now, distribute matrix
    for (int i = 0; i < numProcess; ++i)
    {
        int cntTag = 0;
        sendMatrix(i, cntTag, A, {0, i * len}, {A.nRows - 1, min(A.nCols - 1, (i + 1) * len - 1)});
        sendMatrix(i, cntTag, B, {i * len, 0}, {min(B.nRows - 1, (i + 1) * len - 1), B.nCols - 1});
    }
}

void Solve()
{
    int cntTag = 0;
    recvMatrix(MASTER_PROC, cntTag, A);
    recvMatrix(MASTER_PROC, cntTag, B);

    // cerr << A.nRows << " " << A.nCols << " " << B.nRows << " " << B.nCols << "\n";
    //  {
    //      cout << "Report from process " << rankProcess << ":\n------------------------\n";
    //      cout << "Matrix A: " << A.nRows << " " << A.nCols << "\n";
    //      for(int i = 0; i < A.nRows; ++i)
    //          for(int j = 0 ; j < A.nCols; ++j)
    //              cout << A.a[i][j] << (j == A.nCols - 1 ? "\n" : " ");
    //      cout << "----------------\n";
    //      cout << "Matrix B: " << B.nRows << " " << B.nCols << "\n";
    //      for(int i = 0; i < B.nRows; ++i)
    //          for(int j = 0 ; j < B.nCols; ++j)
    //              cout << B.a[i][j] << (j == B.nCols - 1 ? "\n" : " ");
    //              cout << "----------------\n";
    //  }

    A = A * B;

    cntTag = 0;
    sendMatrix(MASTER_PROC, cntTag, A, {0, 0}, {A.nRows - 1, A.nCols - 1});

    if (rankProcess == MASTER_PROC)
    {
        for (int src = 0; src < numProcess; ++src)
        {
            cntTag = 0;
            recvMatrix(src, cntTag, B);

            //assert(res.nCols == B.nCols && res.nRows == B.nRows);
            // Don't assert because some processes compute null matrix
            if(res.nCols == B.nCols && res.nRows == B.nRows)
                for (int i = 0; i < res.nRows; ++i)
                    for (int j = 0; j < res.nCols; ++j)
                        (res.a[i][j] += B.a[i][j]) %= mod;
        }

        // Print answer
        for (int i = 0; i < res.nRows; ++i)
            for (int j = 0; j < res.nCols; ++j)
                cout << (res.a[i][j] + mod) % mod << (j == res.nCols - 1 ? "\n" : " ");
    }
}

void init(int &argc, char **&argv);
int main(int argc, char **argv)
{
    init(argc, argv);

    if (fopen("testMatrix.INP", "r"))
    {
        freopen("testMatrix.INP", "r", stdin);
        freopen("testMatrix.OUT", "w", stdout);
    }

    if (rankProcess == MASTER_PROC)
        Read();

    // MPI_Barrier(MPI_COMM_WORLD); // wait until Master send all datas

    Solve();

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