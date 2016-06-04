
set(LIB morrf-awa-viz)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${LIB} SHARED
            morrf_awa_viz/multiobjpathplanninginfo.h
            morrf_awa_viz/morrfvisualizer.h
            multiobjpathplanninginfo.cpp
            morrfvisualizer.cpp
           )

include_directories(
                    ${Qt5Widgets_INCLUDE_DIRS}
                   )

target_link_libraries(${LIB} 
                      morrf-awa
                      ${Qt5Widgets_LIBRARIES}
                     )
