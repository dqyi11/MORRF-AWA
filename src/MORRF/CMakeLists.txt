
set(LIB morrf-awa)

set(${HDR} morrf_awa/kdtree++/allocator.hpp
           morrf_awa/kdtree++/function.hpp
           morrf_awa/kdtree++/iterator.hpp
           morrf_awa/kdtree++/kdtree.hpp
           morrf_awa/kdtree++/node.hpp
           morrf_awa/kdtree++/region.hpp
           morrf_awa/KDTree2D.h
           morrf_awa/utilities.h
           morrf_awa/subtree.h
           morrf_awa/morrf.h
           morrf_awa/objective_knn.h
    )

add_library(${LIB} SHARED
            ${HDR}
            subtree.cpp
            morrf.cpp
           )

target_link_libraries(${LIB})

install(TARGETS ${LIB}
        LIBRARY
        DESTINATION lib
        ARCHIVE
        DESTINATION lib)

install(DIRECTORY morrf DESTINATION include)
