add_library(dyn_dgraph_mgr dyn_dgraph_mgr.cpp)
target_link_libraries(dyn_dgraph_mgr ${LIBRARIES})

# test
add_executable(test-me test.cpp)
#target_link_libraries(test-me greedy_manager)

add_executable(test_dyn_dgraph_mgr test_dyn_dgraph_mgr.cpp)
target_link_libraries(test_dyn_dgraph_mgr dyn_dgraph_mgr)

# experiments
## SieveADN
### bipartite graph
add_executable(exam_sieve_adn_bg exam_sieve_adn.cpp)
target_link_libraries(exam_sieve_adn_bg ${LIBRARIES})
### general graph
add_executable(exam_sieve_adn_dg exam_sieve_adn.cpp)
target_link_libraries(exam_sieve_adn_dg dyn_dgraph_mgr)
target_compile_definitions(exam_sieve_adn_dg PRIVATE DGRAPH=1)


## BasicReduction
### bipartite graph
add_executable(exam_basic_bg exam_basic.cpp)
target_link_libraries(exam_basic_bg ${LIBRARIES})
### general graph
add_executable(exam_basic_dg exam_basic.cpp)
target_link_libraries(exam_basic_dg dyn_dgraph_mgr)
target_compile_definitions(exam_basic_dg PRIVATE DGRAPH=1)

## HistApprox
### bipartite graph
add_executable(exam_hist_bg exam_hist.cpp)
target_link_libraries(exam_hist_bg ${LIBRARIES})
### general graph
add_executable(exam_hist_dg exam_hist.cpp)
target_link_libraries(exam_hist_dg dyn_dgraph_mgr)
target_compile_definitions(exam_hist_dg PRIVATE DGRAPH=1)


# Greedy
## ADN
### bipartite graph
add_executable(exam_greedy_adn_bg exam_greedy_adn.cpp)
target_link_libraries(exam_greedy_adn_bg ${LIBRARIES})
### general graph
add_executable(exam_greedy_adn_dg exam_greedy_adn.cpp)
target_link_libraries(exam_greedy_adn_dg dyn_dgraph_mgr)
target_compile_definitions(exam_greedy_adn_dg PRIVATE DGRAPH=1)


# BasicReduction
### bipartite graph
add_executable(exam_greedy_basic_bg exam_greedy_basic.cpp)
target_link_libraries(exam_greedy_basic_bg ${LIBRARIES})
### general graph
add_executable(exam_greedy_basic_dg exam_greedy_basic.cpp)
target_link_libraries(exam_greedy_basic_dg dyn_dgraph_mgr)
target_compile_definitions(exam_greedy_basic_dg PRIVATE DGRAPH=1)


# Random
### bipartite graph
add_executable(exam_random_bg exam_random.cpp)
target_link_libraries(exam_random_bg ${LIBRARIES})
### general graph
add_executable(exam_random_dg exam_random.cpp)
target_link_libraries(exam_random_dg dyn_dgraph_mgr)
target_compile_definitions(exam_random_dg PRIVATE DGRAPH=1)


# add_executable(exam_greedy_eval exam_greedy_eval.cpp)
# target_link_libraries(exam_greedy_eval dyn_graph_mgr)



### bipartite graph
add_executable(exam_input_mgr_bg exam_input_mgr.cpp)
target_link_libraries(exam_input_mgr_bg ${LIBRARIES})
### general graph
add_executable(exam_input_mgr_dg exam_input_mgr.cpp)
target_link_libraries(exam_input_mgr_dg dyn_dgraph_mgr)
target_compile_definitions(exam_input_mgr_dg PRIVATE DGRAPH=1)
