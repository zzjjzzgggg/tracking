add_library(dim dim.cpp)
target_link_libraries(dim ${LIBRARIES})

add_executable(exam_dim_dg exam_dim.cpp)
target_link_libraries(exam_dim_dg dim dyn_dgraph_mgr)
target_compile_definitions(exam_dim_dg PRIVATE DGRAPH=1)

add_executable(test_dim test_dim.cpp)
target_link_libraries(test_dim dim)
