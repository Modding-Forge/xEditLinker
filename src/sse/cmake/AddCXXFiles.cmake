function(add_cxx_files TARGET)
	file(GLOB HEADER_FILES
		LIST_DIRECTORIES false
		CONFIGURE_DEPENDS
		"*.h"
		"*.hpp"
		"*.hxx"
		"*.inl"
	)
	source_group("Header Files" FILES ${HEADER_FILES})
	target_sources("${TARGET}" PUBLIC ${HEADER_FILES})

	file(GLOB SOURCE_FILES
		LIST_DIRECTORIES false
		CONFIGURE_DEPENDS
		"*.cpp"
		"*.cxx"
	)
	source_group("Source Files" FILES ${SOURCE_FILES})
	target_sources("${TARGET}" PRIVATE ${SOURCE_FILES})
endfunction()
