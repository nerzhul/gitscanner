set(SOURCE_FILES
		gitupdater.cpp
		gitscanner.cpp
		main.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} git2)
install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)