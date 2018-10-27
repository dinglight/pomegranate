#include "test_suite.h"
#include "suffixarray.h"

TEST(SuffixArrayTest, buildTest)
{
    const char *text = "banana";
    size_t n = 6;
    // L  left boundary of internal node
    // R  right boundary of internal node
    // D  depth of internal node

    int *SA = (int*)malloc(n * sizeof(int));
    int *L = (int*)malloc(n * sizeof(int));
    int *R = (int*)malloc(n * sizeof(int));
    int *D = (int*)malloc(n * sizeof(int));

    int node_num = suffixarray_build(text, n, SA, L, R, D);
    printf("node_num:%d\n", node_num);
    printf("i\ttext\tSA\tL\tR\tD\n");
    for (size_t i = 0; i < node_num; ++i) {
        printf("%d\t%s\t%d\t%d\t%d\t%d\n", i, text+SA[L[i]], SA[i], L[i], R[i], D[i]);
    }

    free(L);
    free(R);
    free(D);
    free(SA);
}