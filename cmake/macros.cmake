
macro(setup_compiler)
    if(NOT CMAKE_C_COMPILER_ID)
        message(FATAL_ERROR "Compiler not identified")
    endif()

    add_compile_options(
        -Wall
        -Wextra
        -Wsign-compare
        -pedantic
        -Werror
        -Wno-nonportable-include-path
        -Wno-unused-function
        -ffast-math
        -std=c11
    )

    add_compile_definitions($<$<CONFIG:Debug>:DEBUG>
                            $<$<CONFIG:MinSizeRel>:NDEBUG>
                            $<$<CONFIG:RelWithDebInfo>:NDEBUG>
                            $<$<CONFIG:Release>:NDEBUG>
    )
endmacro()

macro(setup_target target_name)
    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE C)

    set_target_properties(${target_name} PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
    set_target_properties(${target_name} PROPERTIES XCODE_ATTRIBUTE_ARCHS "$(ARCHS_STANDARD)")

    set_target_properties(${target_name} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS YES)
endmacro()
