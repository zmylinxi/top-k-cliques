#include <bits/stdc++.h>
using namespace std;

class Solution {
private:
    vector<int> vertex_cover;
    vector<list<int>> choose;
public:
    int value;
    void init(int n, int K) {
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
    void proc(const list<int> &clique, const char &ch) {
        if (ch == 'A') choose.push_back(clique);
        else choose.pop_back();
        for (const int &x : clique) {
            if (ch == 'A') add(x);
            else if (ch == 'D') del(x);
            else assert(false);
        }
    }
};