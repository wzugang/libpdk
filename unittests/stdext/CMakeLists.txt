add_custom_target(StdextUnittests)
set_target_properties(StdextUnittests PROPERTIES FOLDER "StdExtUnittests")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/preprocessor)
set(PDK_STDEXT_TEST_SRCS)
pdk_add_files(PDK_STDEXT_TEST_SRCS
    optional/OptionalTest.cpp
    optional/OptionalValueAccessTest.cpp
    optional/OptionalFailureTest.cpp
    optional/OptionalIoTest.cpp
    optional/MakeOptionalTest.cpp
    optional/MinimumRequirementTest.cpp
    optional/OptionalMoveTest.cpp
    optional/OptionalRefTest.cpp
    optional/OptionalSfinaeFriendluCtorTest.cpp
    optional/OptionalStaticPropertiesTest.cpp
    optional/OptionalSwapTest.cpp
    AnyTest.cpp
    preprocessor/ArithmeticTest.cpp
    preprocessor/ArrayTest.cpp
    preprocessor/ComparisonTest.cpp
    preprocessor/ControlTest.cpp
    preprocessor/IterationTest.h
    preprocessor/IterationTest.cpp
    preprocessor/RepetitionTest.cpp)

pdk_add_unittest(StdextUnittests StdExtTest ${PDK_STDEXT_TEST_SRCS})
