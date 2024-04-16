#include "ColorCorrectionFilterKCM.h"
#include "ui_ColorCorrectionFilterKCM.h"
#include "kwineffects_interface.h"
#include "ColorCorrectionFilterConfig.h"
#include <QDialog>

#if (QT_VERSION_MAJOR >= 6)
ColorCorrectionFilterKCM::ColorCorrectionFilterKCM(QObject* parent, const KPluginMetaData& args)
    : KCModule(parent, args)
    , ui(new Ui::Form)
{
    ui->setupUi(widget());
    addConfig(ColorCorrectionFilterConfig::self(), widget());
#else
ColorCorrectionFilterKCM::ColorCorrectionFilterKCM(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , ui(new Ui::Form)
{
    ui->setupUi(this);
    addConfig(ColorCorrectionFilterConfig::self(), this);
#endif
}

void
ColorCorrectionFilterKCM::save()
{

    ColorCorrectionFilterConfig::self()->save();
    KCModule::save();

    const auto dbusName = QStringLiteral("kwin4_effect_colorcorrectionfilter");
    OrgKdeKwinEffectsInterface interface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Effects"),
                                         QDBusConnection::sessionBus());
    interface.reconfigureEffect(dbusName);

    // It was expected that the Apply button would repaint the whole screen, but it doesn't.
    // Even calling KWin::effects->addRepaintFull(); didn't do the trick.
    // Maybe it is a bug on the KWin side. Need to check and delete these lines later.
    interface.unloadEffect(dbusName);
    interface.loadEffect(dbusName);
}

void ColorCorrectionFilterKCM::load() {
    KCModule::load();
    ColorCorrectionFilterConfig::self()->load();
   // ui->InclusionList->addItems(ColorCorrectionFilterConfig::inclusions());
    //ui->ExclusionList->addItems(ColorCorrectionFilterConfig::exclusions());
}

void ColorCorrectionFilterKCM::defaults() {
    KCModule::defaults();
    ColorCorrectionFilterConfig::self()->setDefaults();
}
