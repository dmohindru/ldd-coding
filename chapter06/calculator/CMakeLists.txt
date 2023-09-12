# this is needed in order for CLion IDE to provide syntax highlightning
# this is independent from the actual kernel object that is built
add_executable(calculator
        # add all *.h and *.c files here that # CLion should cover
        calculator.c
        )

# CLion IDE will find symbols from <linux/*>
target_include_directories("calculator" PRIVATE ${KERNELHEADERS_INCLUDE_DIRS})