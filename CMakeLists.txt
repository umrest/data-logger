cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 11)
project( data_saver )

file(GLOB data_saver_SRC
    "include/*.h"
    "src/*.cpp"
)

find_package( Boost COMPONENTS filesystem system REQUIRED )
find_package (Threads REQUIRED)

link_directories( /usr/local/lib/comm )

include_directories( include 
 ${BOOST_INCLUDE_DIRS})


add_executable( data_saver ${data_saver_SRC} )

target_link_libraries( data_saver comm 
${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})