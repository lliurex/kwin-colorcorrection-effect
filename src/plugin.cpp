#include "ColorCorrectionFilterEffect.h"

KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED(  ColorCorrectionFilterEffect,
                                        "metadata.json",
                                        return ColorCorrectionFilterEffect::supported();,
                                        return ColorCorrectionFilterEffect::enabledByDefault();)

#include "plugin.moc"
