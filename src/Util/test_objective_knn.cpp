
#include <iostream>
#include "morrf_awa/objective_knn.h"

int main(int argc, char *argv[]) {

    float data[2*10];
    data[0] = 5.0; data[1] = 4.0;
    data[2] = 4.0; data[3] = 2.0;
    data[4] = 7.0; data[5] = 6.0;
    data[6] = 2.0; data[7] = 2.0;
    data[8] = 8.0; data[9] = 0.0;
    data[10] = 5.0; data[11] = 7.0;
    data[12] = 3.0; data[13] = 3.0;
    data[14] = 9.0; data[15] = 7.0;
    data[16] = 2.0; data[17] = 2.0;
    data[18] = 2.0; data[19] = 0.0;
    flann::Matrix <float> vec(data, 10, 2);
    ObjectiveKNN knn(5, vec);


    float t0[2] = {3.0, 5.0};
    flann::Matrix<float> vec_t0(t0, 1, 2);
    std::vector<float> res = knn.get_sparse_diversity(vec_t0);

    for(unsigned int i=0; i < res.size(); i++ ) {
        std::cout << res[i] << std::endl;
    }

    return 0;
}
