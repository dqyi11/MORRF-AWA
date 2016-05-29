
#include <iostream>
#include "objective_knn.h"

int main(int argc, char *argv[]) {

    ObjectiveKNN obj_knn(1);
    float p0[2] = {5.0, 4.0};
    ObjectiveVector vec0(p0, 2);
    obj_knn.insert(vec0);
    float p1[2] = {4.0, 2.0};
    ObjectiveVector vec1(p1, 2);
    obj_knn.insert(vec1);
    float p2[2] = {7.0, 6.0};
    ObjectiveVector vec2(p2, 2);
    obj_knn.insert(vec2);
    float p3[2] = {2.0, 2.0};
    ObjectiveVector vec3(p3, 2);
    obj_knn.insert(vec3);
    float p4[2] = {8.0, 0.0};
    ObjectiveVector vec4(p4, 2);
    obj_knn.insert(vec4);
    float p5[2] = {5.0, 7.0};
    ObjectiveVector vec5(p5, 2);
    obj_knn.insert(vec5);
    float p6[2] = {3.0, 3.0};
    ObjectiveVector vec6(p6, 2);
    obj_knn.insert(vec6);
    float p7[2] = {9.0, 7.0};
    ObjectiveVector vec7(p7, 2);
    obj_knn.insert(vec7);
    float p8[2] = {2.0, 2.0};
    ObjectiveVector vec8(p8, 2);
    obj_knn.insert(vec8);
    float p9[2] = {2.0, 0.0};
    ObjectiveVector vec9(p9, 2);
    obj_knn.insert(vec9); 

    std::cout << "nodes loaded" << std::endl;
    obj_knn._p_index->buildIndex();
    float t0[2] = {3.0, 5.0};
    ObjectiveVector vec_t0(t0, 2);
    obj_knn.find_nearest(vec_t0);

    return 0;
}
