cmake_minimum_required(VERSION 3.22)

set(LIB_NAME "Redbrouk-core")
set(SRC_FILES
	"connection.cpp"
	"io.cpp"

	"hash.cpp"
	"sbtree.cpp"

	"kvobj.cpp"
	"kvt_map.cpp"
	"kvt_set.cpp"
	"kvt_tset.cpp"
)
set(HEADER_FILES
	"network.h"
	"connection.h"
	"io.h"
	"utils.h"

	"hash.h"
	"sbtree.h"

	"kvobj.h"
	"kvt_hash_t.h"
	"kvt_map.h"
	"kvt_tset.h"
)

option(BUILD_SHARED ON)
if(BUILD_SHARED)
	add_library(${LIB_NAME} SHARED ${SRC_FILES} ${HEADER_FILES})
else()
	add_library(${LIB_NAME} STATIC ${SRC_FILES} ${HEADER_FILES})
endif()

target_include_directories(${LIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}")
