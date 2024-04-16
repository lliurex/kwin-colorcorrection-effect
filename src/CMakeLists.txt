add_subdirectory(kcm)

set(effect_SRCS
    ColorCorrectionFilterEffect.h
    ColorCorrectionFilterEffect.cpp
    ColorCorrectionFilterShader.h
    ColorCorrectionFilterShader.cpp
    plugin.cpp
)

kconfig_add_kcfg_files(effect_SRCS ColorCorrectionFilterConfig.kcfgc)
kcoreaddons_add_plugin(kwin4_effect_colorcorrectionfilter SOURCES ${effect_SRCS} INSTALL_NAMESPACE "kwin/effects/plugins/")

target_link_libraries(kwin4_effect_colorcorrectionfilter
    Qt${QT_MAJOR_VERSION}::DBus
    KF${QT_MAJOR_VERSION}::ConfigGui
)
if(${QT_MAJOR_VERSION} EQUAL 6)
    target_link_libraries(kwin4_effect_colorcorrectionfilter
            KWin::kwin
            Qt${QT_MAJOR_VERSION}::CorePrivate
    )
else()
    target_link_libraries(kwin4_effect_colorcorrectionfilter
            KWinEffects::kwineffects
            KWinEffects::kwinglutils
    )
endif()

install(DIRECTORY shaders DESTINATION ${KDE_INSTALL_DATADIR}/kwin)
