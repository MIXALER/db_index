cmake_minimum_required(VERSION 3.17)
project(db_index)

set(CMAKE_CXX_STANDARD 11)

add_executable(b_plus_tree b_plus_tree/main.cpp b_plus_tree/BTree.h b_plus_tree/BTreeIndex.h b_plus_tree/DataBase.h)
