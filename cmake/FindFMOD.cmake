if(WINDOWS)
  set(FMOD_INSTALL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/External/fmod/lib/${OLDUNREAL_CPU}/fmod.dll)
  set(FMOD_LINK_LIBRARY    ${CMAKE_CURRENT_SOURCE_DIR}/External/fmod/lib/${OLDUNREAL_CPU}/fmod_vc.lib)
else()
  set(FMOD_INSTALL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/External/fmod/lib/${OLDUNREAL_CPU}/libfmod${SHARED_LIB_EXT})
  set(FMOD_LINK_LIBRARY    ${FMOD_INSTALL_LIBRARY})
endif()

include_directories(External/fmod/inc)
