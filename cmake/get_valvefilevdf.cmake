if( TARGET valve-file-vdf::valve-file-vdf )
    return()
endif()

include( ./dl_unpack )

add_library( valve-file-vdf::valve-file-vdf INTERFACE IMPORTED )
target_include_directories( valve-file-vdf::valve-file-vdf INTERFACE "${EXTERNAL_SRC_ROOT}/ValveFileVDF" )
