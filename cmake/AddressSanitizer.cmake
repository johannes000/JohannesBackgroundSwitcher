# AddressSanitizer Option
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    message(STATUS "AddressSanitizer enabled")
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MINGW)
        # Für Clang unter MSYS2/MINGW
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
        
        # Für MSYS2 spezifische Pfade
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            add_compile_options(-g)
        endif()
        
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # Für GCC unter MSYS2 - manchmal problematisch
        message(WARNING "AddressSanitizer with GCC on MSYS2 may have issues")
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address -static-libasan)
        
    elseif(MSVC)
        # Für Visual Studio
        add_compile_options(/fsanitize=address)
        add_compile_definitions(_DISABLE_VECTOR_ANNOTATION _DISABLE_STRING_ANNOTATION)
        
    else()
        message(WARNING "AddressSanitizer not supported for this compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

