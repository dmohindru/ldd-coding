# this is needed in order for CLion IDE to provide syntax highlightning
# this is independent from the actual kernel object that is built
add_executable(blocky
        # add all *.h and *.c files here that # CLion should cover
        main.c circular_buffer.c
        )

# CLion IDE will find symbols from <linux/*>
target_include_directories("blocky" PRIVATE ${KERNELHEADERS_INCLUDE_DIRS})