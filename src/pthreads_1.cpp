#include <pthread.h>
#include <stdio.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <ctime>
using namespace std;

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

void *thread_main(void *thread_id) {
    long tid = (long) thread_id;
    int numThread = Setting::getInstance().numThread;
    int p = (a.nRows + numThread - 1) / numThread;

    Matrix a2;
    a2.nCols = a.nCols;
    for (int i = p * tid; i < p * (tid + 1) && i < a.nRows; ++i) {
        ++a2.nRows;
        for (int j = 0; j < a.nCols; ++j) {
            a2.a[i - p * tid][j] = a.a[i][j];\
        }
    }

    Matrix c = a2 * b;
    for (int i = p * tid; i < p * (tid + 1) && i < a.nRows; ++i) {
        for (int j = 0; j < b.nCols; ++j) {
            ans.a[i][j] = c.a[i - p * tid][j];
        }
    }

    pthread_exit(NULL); // Kết thúc luồng

    return NULL;
}


int main(int argc, char **argv)
{
    Setting::getInstance().parseArg(argc, argv);

    Read();

    freopen(Setting::getInstance().output.c_str(), "w", stdout);

    int startTime = 1000 * clock() / CLOCKS_PER_SEC;

    int numThread = Setting::getInstance().numThread;
    pthread_t *threads = new pthread_t[numThread]; // Mảng chứa ID của các luồng

    // Tạo các luồng
    for (int t = 0; t < numThread; t++) {
        printf("In main: creating thread %ld\n", t);
        int rc = pthread_create(&threads[t], NULL, thread_main, (void *)t);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    // Đợi tất cả các luồng hoàn thành
    for (int t = 0; t < numThread; t++) {
        pthread_join(threads[t], NULL);
    }

    printf("Main: All threads completed!!!\n");

    cout << ans.nRows << " " << ans.nCols << "\n";
    for (int i = 0; i < a.nRows; ++i) {
        for (int j = 0; j < b.nCols; ++j) {
            cout << ans.a[i][j] << " ";
        }
        cout << "\n";
    }

    int endTime = 1000 * clock() / CLOCKS_PER_SEC;

    cout << "Time: " << (endTime - startTime) << "ms\n";
    return 0;
}