function(add_paracl_compile_options target_name)
    target_compile_features("${target_name}"
        PRIVATE
            cxx_std_20
    )

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options("${target_name}"
            PRIVATE
                -fdiagnostics-color=always

                -Wall
                -Wextra
                -Wpedantic
                -Wshadow
                -Wconversion
                -Wsign-conversion
                -Wunused
                -Wformat=2
                -Wnon-virtual-dtor
                -Wswitch-enum

                -fstack-protector-strong

                $<$<CONFIG:Debug>:
                    -Og
                    -g3
                    -ggdb
                    -fsanitize=address,leak,undefined,float-divide-by-zero
                    -fno-omit-frame-pointer
                >

                $<$<CONFIG:Release>:
                    -g
                    -O2
                    -march=native
                    -flto
                >
        )

        target_link_options("${target_name}"
            PRIVATE
                -rdynamic

                $<$<CONFIG:Debug>:
                    -fsanitize=address,leak,undefined,float-divide-by-zero
                >

                $<$<CONFIG:Release>:
                    -flto
                >
        )

        target_compile_definitions("${target_name}"
            PRIVATE
                $<$<CONFIG:Release>:NDEBUG>
        )
    endif()

endfunction()
