add_library(dyngraph_mgr dyngraph_mgr.cpp)
target_link_libraries(dyngraph_mgr ${LIBRARIES})

add_executable(test-me test.cpp)
#target_link_libraries(test-me greedy_manager)

add_executable(test_dyngraph_mgr test_dyngraph_mgr.cpp)
target_link_libraries(test_dyngraph_mgr dyngraph_mgr)
