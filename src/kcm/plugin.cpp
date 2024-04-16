#include "ColorCorrectionFilterKCM.h"

// KF5
#include <KPluginFactory>

#if (QT_VERSION_MAJOR >= 248)
K_PLUGIN_CLASS(ColorCorrectionFilterKCM)
#else
K_PLUGIN_FACTORY_WITH_JSON(ColorCorrectionFilterConfigFactory,
                           "metadata.json",
                           registerPlugin<ColorCorrectionFilterKCM>();)
#endif

#include "plugin.moc"
