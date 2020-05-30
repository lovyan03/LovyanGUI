set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS src/*.cpp)
set(COMPONENT_SRCS ${SRCS})

register_component()
