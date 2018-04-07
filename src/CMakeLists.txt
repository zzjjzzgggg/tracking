add_library(dyn_dgraph_mgr dyn_dgraph_mgr.cpp)
target_link_libraries(dyn_dgraph_mgr ${LIBRARIES})

add_library(greedy_alg greedy_alg.cpp)
target_link_libraries(greedy_alg ${LIBRARIES})






add_executable(test-me test.cpp)
#target_link_libraries(test-me greedy_manager)

add_executable(test_dyngraph_mgr test_dyngraph_mgr.cpp)
target_link_libraries(test_dyngraph_mgr dyn_dgraph_mgr)

add_executable(exam_sieve_adn exam_sieve_adn.cpp)
target_link_libraries(exam_sieve_adn dyn_dgraph_mgr)

add_executable(exam_basic exam_basic.cpp)
target_link_libraries(exam_basic dyn_dgraph_mgr)

add_executable(exam_greedy_adn exam_greedy_adn.cpp)
target_link_libraries(exam_greedy_adn dyn_dgraph_mgr greedy_alg)

add_executable(exam_greedy_basic exam_greedy_basic.cpp)
target_link_libraries(exam_greedy_basic dyn_dgraph_mgr greedy_alg)
