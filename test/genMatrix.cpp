#include <bits/stdc++.h>

using ll = long long;
using namespace std;
mt19937_64 Rand(chrono::high_resolution_clock::now().time_since_epoch().count());

ll rand(ll l, ll r)
{
    return Rand() % (r - l + 1) + l;
}

int main (int argc, char **argv) {
    int m, n;
    // cin >> m >> n;
    m = atoi(argv[2]);
    n = atoi(argv[3]);
    int l, r;
    // cin >> l >> r;
    l = atoi(argv[4]);
    r = atoi(argv[5]);

    freopen(((string)argv[1]).c_str(), "w", stdout);

    cout << m << " " << n << "\n";
    for(int i = 1; i <= m; ++i)
        for(int j = 1; j <= n; ++j)
            cout << rand(l, r) << (j == n ? "\n" : " ");
    // cout << m << " " << n << "\n";
    // for(int i = 1; i <= m; ++i)
    //     for(int j = 1; j <= n; ++j)
    //         cout << rand(l, r) << (j == n ? "\n" : " ");
}