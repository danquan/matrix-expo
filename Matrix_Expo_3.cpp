#include <bits/stdc++.h>
#include <mpi.h>
#define MASTER_PROC 0

using namespace std;

int numProcess, rankProcess;
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

void init(int &argc, char **&argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcess);
    MPI_Comm_rank(MPI_COMM_WORLD, &rankProcess);
    MPI_Get_processor_name(name, &len_name);
}

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

    int p = (int) sqrt(numProcess);
    int len = (A.nRows + p - 1) / p;

    //Now, distribute matrix
    for(int i = 0; i < p; ++i)
        for (int j = 0; j < p; ++j) {
            int cntTag = 0;
            sendMatrix(i * p + j, cntTag, A, {i * len, 0}, {min(A.nRows - 1, (i + 1) * len - 1), A.nCols - 1});
            sendMatrix(i * p + j, cntTag, B, {0, j * len}, {B.nRows - 1, min(B.nCols - 1, (j + 1) * len - 1)});
        }
}

void Solve() {
    int cntTag = 0;
    recvMatrix(MASTER_PROC, cntTag, A);
    recvMatrix(MASTER_PROC, cntTag, B);

    // {
    //     cout << "Report from process " << rankProcess << ":\n------------------------\n";
    //     cout << "Matrix A: " << A.nRows << " " << A.nCols << "\n";
    //     for(int i = 0; i < A.nRows; ++i)
    //         for(int j = 0 ; j < A.nCols; ++j)
    //             cout << A.a[i][j] << (j == A.nCols - 1 ? "\n" : " ");
    //     cout << "----------------\n";
    //     cout << "Matrix B: " << B.nRows << " " << B.nCols << "\n";
    //     for(int i = 0; i < B.nRows; ++i)
    //         for(int j = 0 ; j < B.nCols; ++j)
    //             cout << B.a[i][j] << (j == B.nCols - 1 ? "\n" : " ");
    //             cout << "----------------\n";   
    // }
    
    A = A * B;

    cntTag = 0;
    sendMatrix(MASTER_PROC, cntTag, A, {0, 0}, {A.nRows - 1, A.nCols - 1});
    MPI_Barrier(MPI_COMM_WORLD); // wait until all workers send datas

    if(rankProcess == MASTER_PROC)  {
        res.nCols = 0;
        int p = (int) sqrt(numProcess);

        for(int src1 = 0; src1 < p; ++src1) {
            res.nRows = 0;
            int prevCol = 0;

            for (int src2 = 0; src2 < p; ++src2) {
                cntTag = 0;
                recvMatrix(src1 * p + src2, cntTag, B);

                for(int i = 0; i < B.nRows; ++i)
                    for(int j = 0; j < B.nCols; ++j)
                        (res.a[res.nRows + i][res.nCols + j] += B.a[i][j]) %= mod;
                
                res.nRows += B.nRows;
                prevCol = B.nCols;
            }

            res.nCols += prevCol;
        }

        //Print answer
        for(int i = 0; i < res.nRows; ++i)
            for(int j = 0; j < res.nCols; ++j)
                cout << res.a[i][j] << (j == res.nCols - 1 ? "\n" : " ");
    }
}

int main(int argc, char* argv[]) {
    init(argc, argv);

    if (fopen("testMatrix.INP", "r"))
    {
        freopen("testMatrix.INP", "r", stdin);
        freopen("testMatrix.OUT", "w", stdout);
    }
    if (rankProcess == MASTER_PROC) Read();


    MPI_Barrier(MPI_COMM_WORLD);

    Solve();

    MPI_Finalize();

    return 0;
}