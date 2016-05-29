
add_executable(test-kdtree test_kdtree.cpp)

target_link_libraries(test-kdtree morrf)

add_executable(test-objective-knn test_objective_knn.cpp)

target_link_libraries(test-objective-knn morrf)

add_executable(test-flann test_flann.cpp)

target_link_libraries(test-flann morrf)
