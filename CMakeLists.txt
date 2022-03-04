set(COMPONENT_ADD_INCLUDEDIRS
    src
    )
file(GLOB SRCS
     src/*.cpp
     src/lgui/*.cpp
    )
set(COMPONENT_SRCS ${SRCS})

set(COMPONENT_REQUIRES LovyanGFX)

register_component()
