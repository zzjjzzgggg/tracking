add_library(config config.cpp)
set(LIBRARIES config ${LIBRARIES})

add_library(walk_engine walk_engine.cpp)
target_link_libraries(walk_engine ${LIBRARIES})

add_library(greedy_manager greedy_manager.cpp)
target_link_libraries(greedy_manager ${LIBRARIES})


add_executable(convert_graph main_convert_graph.cpp)
target_link_libraries(convert_graph ${LIBRARIES})

add_executable(simulate_walks main_simulate_walks.cpp)
target_link_libraries(simulate_walks walk_engine)

add_executable(simulate_walks_async main_simulate_walks_async.cpp)
target_link_libraries(simulate_walks_async walk_engine)

add_executable(estimate_ht main_estimate_hitting_time.cpp)
target_link_libraries(estimate_ht walk_engine)

add_executable(greedy_ht main_greedy_algorithm.cpp)
target_link_libraries(greedy_ht greedy_manager walk_engine)

add_executable(greedy_ap main_greedy_algorithm.cpp)
target_link_libraries(greedy_ap greedy_manager walk_engine)
target_compile_definitions(greedy_ap PRIVATE GAIN_AP=1)

add_executable(test-me test.cpp)
#target_link_libraries(test-me greedy_manager)

add_executable(refine_gain_ap main_refine_gain.cpp)
target_link_libraries(refine_gain_ap greedy_manager walk_engine)
target_compile_definitions(refine_gain_ap PRIVATE GAIN_AP=1)

add_executable(refine_gain_ht main_refine_gain.cpp)
target_link_libraries(refine_gain_ht greedy_manager walk_engine)
# target_compile_definitions(refine_gain_ht PRIVATE GAIN_HT=1)

add_executable(refine_gain_ap_var main_refine_gain_var.cpp)
target_link_libraries(refine_gain_ap_var greedy_manager walk_engine)
target_compile_definitions(refine_gain_ap_var PRIVATE GAIN_AP=1)

add_executable(refine_gain_ht_var main_refine_gain_var.cpp)
target_link_libraries(refine_gain_ht_var greedy_manager walk_engine)

add_executable(simulate_cascades main_simulate_cascades.cpp)
target_link_libraries(simulate_cascades ${LIBRARIES})

add_executable(stat_cascades main_stat_cascades.cpp)
target_link_libraries(stat_cascades ${LIBRARIES})
