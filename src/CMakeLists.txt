set(CMAKE_INSTALL_RPATH "\$ORIGIN:/usr/local/lib")

# Build and install libvinbero_core
add_library(vinbero_core SHARED vinbero_core.c)
list(APPEND vinbero_core_public_headers "vinbero_core.h")
set_target_properties(vinbero_core PROPERTIES VERSION ${vinbero_version}
                                              SOVERSION ${vinbero_soversion}
                                              PUBLIC_HEADER "${vinbero_core_public_headers}")
target_link_libraries(vinbero_core fastdl gaio jansson pthread dl)
install(TARGETS vinbero_core 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/vinbero)

# Build and install vinbero)
add_executable(vinbero vinbero.c vinbero_Help.c vinbero_Options.c)
target_link_libraries(vinbero vinbero_core vinbero_common)
install(TARGETS vinbero 
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/vinbero)
