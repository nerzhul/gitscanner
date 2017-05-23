set(SOURCE_FILES
		gitscanner.cpp
		main.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} git2 stdc++fs)