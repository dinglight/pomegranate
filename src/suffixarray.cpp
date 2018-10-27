#include "suffixarray.h"
#include <cstdlib>
#include <vector>

static const char *__text = NULL;

static int suffix_compare(const void *a, const void *b)
{
    const char *t_a = __text + *(int*)a;
    const char *t_b = __text + *(int*)b;
    while (*t_a != '\0' && *t_b != '\0' && *t_a == *t_b) {
        t_a++;
        t_b++;
    }
    return (*t_a - *t_b);
}

static int common_prefix(const char *a, const char *b)
{
    int same = 0;
    while (*a != '\0' && *b != '\0' && *a == *b) {
        same++;
        a++;
        b++;
    }
    return same;
}

int suffixarray_build(const char *text, size_t text_len, int *SA, int *L, int *R, int *D)
{
    // initialize
    for (size_t i = 0; i < text_len; ++i) {
        SA[i] = i;
    }

    __text = text;

    // sort
    qsort(SA, text_len, sizeof(int), suffix_compare);

    // PSI
    int *psi = L;
    psi[SA[0]] = SA[text_len - 1];
    for (int i = 1; i < text_len; ++i) {
        psi[SA[i]] = SA[i - 1];
    }

    int *plcp = R;
    int h = 0;
    for (int i = 0; i < text_len; ++i) {
        int j = psi[i];
        while (i + h < text_len && j + h < text_len && text[i + h] == text[j + h]) {
            ++h;
        }
        plcp[i] = h;
        if (h > 0) --h;
    }

    // init H
    int *H = L;
    for (int i = 0; i < text_len; ++i) {
        H[i] = plcp[SA[i]];
    }
    H[0] = -1;

    // node_num
    size_t node_num = 0;

    std::vector<std::pair<int, int>> S;
    S.push_back(std::make_pair(-1, -1));
    for (int i = 0; ; ++i) {
        std::pair<int, int> cur(i, (i == text_len) ? -1 : H[i]);
        std::pair<int, int> cand(S.back());
        while (cand.second > cur.second) {
            if (i - cand.first > 1) {
                L[node_num] = cand.first;
                R[node_num] = i;
                D[node_num] = cand.second;
                ++node_num;
            }
            cur.first = cand.first;
            S.pop_back();
            cand = S.back();
        }
        if (cand.second < cur.second) {
            S.push_back(cur);
        }
        if (i == text_len) break;
        S.push_back(std::make_pair(i, text_len - SA[i] + 1));
    }
    return node_num;
}
