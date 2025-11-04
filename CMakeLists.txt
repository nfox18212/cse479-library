target_sources(cse479-library

    PRIVATE
        asm_library.s
        debug-asm.s
        debug.c
        library.c
    PUBLIC
        FILE_SET HEADERS
        FILES
            debug_config.h
            debug.h
            library.h

)