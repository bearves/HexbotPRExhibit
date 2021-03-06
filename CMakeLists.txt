cmake_minimum_required(VERSION 2.8)

set( CMAKE_EXPORT_COMPILE_COMMANDS 1)

# add prefix to every string in a variable
FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

project(PushRecoveryTest)

# set compile options
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O2")
endif(UNIX)

# set binary and library folders
set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)

# set include and lib folders
if(UNIX)
    set(ARIS_SOURCE_LOCATION "/home/hex/Aris_with_force")
    include_directories("/usr/lapack/include")
    include_directories(${ARIS_SOURCE_LOCATION}/src/Aris_Control)
    include_directories(${ARIS_SOURCE_LOCATION}/src/Aris_Core)
    include_directories(${ARIS_SOURCE_LOCATION}/src/Aris_Plan)
    include_directories(${ARIS_SOURCE_LOCATION}/src/Aris_DynKer)
    include_directories(${ARIS_SOURCE_LOCATION}/src/Aris_ExpCal)
    include_directories(${ARIS_SOURCE_LOCATION}/src)

    include_directories("/usr/xenomai/include")
    include_directories("/opt/etherlab/include")
    include_directories("/usr/Robots/include")
    link_directories(/usr/lapack/lib)
    link_directories(${ARIS_SOURCE_LOCATION}/build/lib)
    link_directories(/usr/xenomai/lib)
    link_directories(/opt/etherlab/lib)
    link_directories(/usr/Robots/lib)
endif(UNIX)

# set link files
set(SYSTEM_LINK_LIBS pthread gfortran)
set(XENOMAI_LINK_LIBS native rtdm xenomai)
#set(XENOMAI_LINK_LIBS m)
set(ETHERLAB_LINK_LIBS ethercat rt)
#set(ETHERLAB_LINK_LIBS dl)
set(LAPACK_LINK_LIBS lapacke lapack cblas refblas)
set(ARIS_LINK_LIBS Aris_Plan Aris_DynKer Aris_ExpCal Aris_Control Aris_Core)
set(ARIS_NORT_LINK_LIBS Aris_Plan Aris_DynKer Aris_ExpCal Aris_Core)
set(ROBOTS_LINK_LIBS  HexapodIII)

# build programs
set(SOURCE_FILES PRT.cpp VirtualBipedPlanner.cpp PushRecoveryPlanner.cpp RetreatGait.cpp)
PREPEND(FULL_SRC PRT ${SOURCE_FILES})
add_executable(PRT ${SOURCE_FILES})
target_link_libraries(PRT ${ROBOTS_LINK_LIBS} ${ARIS_NORT_LINK_LIBS} ${LAPACK_LINK_LIBS} ${SYSTEM_LINK_LIBS})

# build another test
set(SOURCE_FILES main.cpp VirtualBipedPlanner.cpp PushRecoveryPlanner.cpp RetreatGait.cpp)
PREPEND(FULL_SRC PRT ${SOURCE_FILES})
add_executable(main ${SOURCE_FILES})
target_link_libraries(main ${ROBOTS_LINK_LIBS} ${ARIS_LINK_LIBS} ${LAPACK_LINK_LIBS} ${ETHERLAB_LINK_LIBS} ${XENOMAI_LINK_LIBS} ${SYSTEM_LINK_LIBS})

set(SOURCE_FILES ArisControl.cpp VirtualBipedPlanner.cpp PushRecoveryPlanner.cpp Server.cpp Gait.cpp RetreatGait.cpp)
PREPEND(FULL_SRC PRT ${SOURCE_FILES})
add_executable(ArisControlPRT ${SOURCE_FILES})
target_link_libraries(ArisControlPRT ${ROBOTS_LINK_LIBS} ${ARIS_LINK_LIBS} ${LAPACK_LINK_LIBS} ${ETHERLAB_LINK_LIBS} ${XENOMAI_LINK_LIBS} ${SYSTEM_LINK_LIBS})

set(SOURCE_FILES ClientPRT.cpp Client.cpp)
PREPEND(FULL_SRC PRT ${SOURCE_FILES})
add_executable(ClientPRT ${SOURCE_FILES})
target_link_libraries(ClientPRT ${ROBOTS_LINK_LIBS} ${ARIS_NORT_LINK_LIBS} ${LAPACK_LINK_LIBS} ${SYSTEM_LINK_LIBS})

set(SOURCE_FILES ./LogDataReader.cpp)
PREPEND(FULL_SRC PRT ${SOURCE_FILES})
add_executable(LogDataReader ${SOURCE_FILES})
target_link_libraries(LogDataReader ${ROBOTS_LINK_LIBS} ${ARIS_LINK_LIBS} ${LAPACK_LINK_LIBS} ${ETHERLAB_LINK_LIBS} ${XENOMAI_LINK_LIBS} ${SYSTEM_LINK_LIBS})

