
#include <iostream>
#include "morrf_awa/objective_knn.h"

int main(int argc, char *argv[]) {

    /*
    std::vector<int> arr (9);
    arr = {1,2,3,4,5,6,7,8,9};

    std::vector<int>::const_iterator begin = arr.begin();
    std::vector<int>::const_iterator last = arr.begin() + arr.size();
    std::vector<int> new_arr(begin, last);
    std::cout << "Size: "<< new_arr.size() << std::endl;

    for (int i = 0 ;  i < new_arr.size(); i++){
        std::cout << new_arr[i] << std::endl;
    }
    return 0;
    */

    float p0[2] = {5.0, 4.0};
    flann::Matrix<float> vec0(p0, 1, 2);
    flann::Index<flann::L2<float> > index(vec0, flann::KDTreeIndexParams(4));
    float p1[2] = {4.0, 2.0};
    flann::Matrix<float> vec1(p1, 1, 2);
    index.addPoints(vec1);
    float p2[2] = {7.0, 6.0};
    flann::Matrix<float> vec2(p2, 1, 2);
    index.addPoints(vec2);
    float p3[2] = {2.0, 2.0};
    flann::Matrix<float> vec3(p3, 1, 2);
    index.addPoints(vec3);
    float p4[2] = {8.0, 0.0};
    flann::Matrix<float> vec4(p4, 1, 2);
    index.addPoints(vec4);
    float p5[2] = {5.0, 7.0};
    flann::Matrix<float> vec5(p5, 1, 2);
    index.addPoints(vec5);
    float p6[2] = {3.0, 3.0};
    flann::Matrix<float> vec6(p6, 1, 2);
    index.addPoints(vec6);
    float p7[2] = {9.0, 7.0};
    flann::Matrix<float> vec7(p7, 1, 2);
    index.addPoints(vec7);
    float p8[2] = {2.0, 2.0};
    flann::Matrix<float> vec8(p8, 1, 2);
    index.addPoints(vec8);
    float p9[2] = {2.0, 0.0};
    flann::Matrix<float> vec9(p9, 1, 2);
    index.addPoints(vec9);

    std::cout << "nodes loaded" << std::endl;
    index.buildIndex();
    float t0[2] = {3.0, 5.0};
    flann::Matrix<float> vec_t0(t0, 1, 2);

    std::vector<std::vector<int> > indices;
    std::vector<std::vector<float> > dists;
    index.knnSearch( vec_t0 , indices, dists, 1, flann::SearchParams(128));

    for(unsigned int i=0;i<indices.size();i++) {
        std::cout << indices[i].size() << std::endl;
        std::cout << indices[i][0] << " (" << dists[i][0] << ")" << std::endl;
    }

    return 0;
}
