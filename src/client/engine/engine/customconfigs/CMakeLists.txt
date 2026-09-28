target_sources(engine PRIVATE
    customconfigs.cpp
    customconfigs.h
    customconfigsdirwatcher.cpp
    customconfigsdirwatcher.h
    customovpnauthcredentialsstorage.cpp
    customovpnauthcredentialsstorage.h
    icustomconfig.h
    ovpncredentialinliner.cpp
    ovpncredentialinliner.h
    ovpncustomconfig.cpp
    ovpncustomconfig.h
    parseovpnconfigline.cpp
    parseovpnconfigline.h
    wireguardcustomconfig.cpp
    wireguardcustomconfig.h
    singboxurlparser.cpp
    singboxurlparser.h
    singboxcustomconfig.cpp
    singboxcustomconfig.h
)

if(IS_BUILD_TESTS)
    add_subdirectory(tests)
endif(IS_BUILD_TESTS)
