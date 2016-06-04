set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(morrf-awa-viz-demo
               configobjdialog.h
               configobjdialog.cpp
               mainwindow.h
               mainwindow.cpp
               main.cpp
               )

include_directories(
                   ${Qt5Widgets_INCLUDE_DIRS}
                   ${Qt5Core_INCLUDE_DIRS}
                   )

target_link_libraries(morrf-awa-viz-demo
                      morrf-awa
                      morrf-awa-viz
                      ${Qt5Widgets_LIBRARIES}
                      ${Qt5Core_LIBRARIES}
                     )
