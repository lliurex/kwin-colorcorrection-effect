/*
 *   Adapted by juanma1980 Copyright © 2024
 *   Original work:
 *
 *   Copyright © 2015 Robert Metsäranta <therealestrob@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; see the file COPYING.  if not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 */

#include "ColorCorrectionFilterEffect.h"
#include "ColorCorrectionFilterConfig.h"
#include <QtDBus/QDBusConnection>
#include <QDBusError>
#include <KX11Extras>

#if QT_VERSION_MAJOR >= 6
    #include <opengl/glutils.h>
    #include <effect/effecthandler.h>
    #include <core/output.h>
    #include <core/renderviewport.h>
#else
    #include <kwinglutils.h>
#endif

ColorCorrectionFilterEffect::ColorCorrectionFilterEffect()
    : KWin::OffscreenEffect()
{
    reconfigure(ReconfigureAll);

    if (auto connection = QDBusConnection::sessionBus();
        !connection.isConnected()) {
        qWarning("ColorCorrectionFilter: Cannot connect to the D-Bus session bus.\n");
    }
    else {
        if (!connection.registerService(QStringLiteral("org.kde.ColorCorrectionFilter"))) {
            qWarning("%s\n", qPrintable(connection.lastError().message()));
        }
        else {
            if (!connection.registerObject(QStringLiteral("/ColorCorrectionFilterEffect"), this, QDBusConnection::ExportAllSlots)) {
                qWarning("%s\n", qPrintable(connection.lastError().message()));
            }
        }
    }

    if(m_shaderManager.IsValid()) {
        for (const auto& win: KWin::effects->stackingOrder())
            windowAdded(win);
        connect(KWin::effects, &KWin::EffectsHandler::windowAdded, this, &ColorCorrectionFilterEffect::windowAdded);
        connect(KWin::effects, &KWin::EffectsHandler::windowDeleted, this, &ColorCorrectionFilterEffect::windowRemoved);
#if QT_VERSION_MAJOR < 6
        connect(KWin::effects, &KWin::EffectsHandler::windowFrameGeometryChanged, this, &ColorCorrectionFilterEffect::windowResized);
#endif
    }
}

ColorCorrectionFilterEffect::~ColorCorrectionFilterEffect() = default;

void
ColorCorrectionFilterEffect::windowAdded(KWin::EffectWindow *w)
{
    qDebug() << w->windowRole() << w->windowType() << w->windowClass();
    //const QSet<QString> hardExceptions { "plasmashell", "kscreenlocker_greet", "ksmserver", "krunner" };
    const auto name = w->windowClass().split(QChar::Space).first();
    //if (hardExceptions.contains(name))
    //    return;
    if (const auto& [w2, r] = m_managed.insert({w, false}); r) {
#if QT_VERSION_MAJOR >= 6
        connect(w, &KWin::EffectWindow::windowFrameGeometryChanged, this, &ColorCorrectionFilterEffect::windowResized);
#endif
        redirect(w);
        setShader(w, m_shaderManager.GetShader().get());
        checkTiled();
    }
}

void ColorCorrectionFilterEffect::windowRemoved(KWin::EffectWindow *w)
{
    m_managed.erase(w);
    checkTiled();
}

void
ColorCorrectionFilterEffect::reconfigure(const ReconfigureFlags flags)
{
    Q_UNUSED(flags)
    ColorCorrectionFilterConfig::self()->read();
}

void ColorCorrectionFilterEffect::prePaintWindow(KWin::EffectWindow *w, KWin::WindowPrePaintData &data, std::chrono::milliseconds time)
{
    if (!hasEffect(w))
    {
        Effect::prePaintWindow(w, data, time);
        return;
    }

    const auto size = 1;

#if QT_VERSION_MAJOR >= 6
    const auto geo = w->frameGeometry() * w->screen()->scale();
    data.setTranslucent();
#else
    const auto geo = w->frameGeometry() * KWin::effects->renderTargetScale();
    data.setTranslucent();
#endif
    QRegion reg {};
    reg += QRect(geo.x(), geo.y(), size, size);
    reg += QRect(geo.x()+geo.width()-size, geo.y(), size, size);
    reg += QRect(geo.x(), geo.y()+geo.height()-size, size, size);
    reg += QRect(geo.x()+geo.width()-size, geo.y()+geo.height()-size, size, size);
    data.opaque -= reg;
    data.paint += reg;

    OffscreenEffect::prePaintWindow(w, data, time);
}

bool ColorCorrectionFilterEffect::supported()
{
    return KWin::effects->isOpenGLCompositing();
}

#if QT_VERSION_MAJOR >= 6
void ColorCorrectionFilterEffect::drawWindow(const KWin::RenderTarget &renderTarget, const KWin::RenderViewport &viewport,
                                    KWin::EffectWindow *w, int mask, const QRegion &region,
                                    KWin::WindowPaintData &data) {
#else
void ColorCorrectionFilterEffect::drawWindow(KWin::EffectWindow *w, int mask, const QRegion &region,
                                    KWin::WindowPaintData &data) {
#endif
    if (!hasEffect(w))
    {
        unredirect(w);
#if QT_VERSION_MAJOR >= 6
        OffscreenEffect::drawWindow(renderTarget, viewport, w, mask, region, data);
#else
        OffscreenEffect::drawWindow(w, mask, region, data);
#endif
        return;
    }

    redirect(w);
    setShader(w, m_shaderManager.GetShader().get());
    m_shaderManager.Bind();
    glActiveTexture(GL_TEXTURE0);

#if QT_VERSION_MAJOR >= 6
    OffscreenEffect::drawWindow(renderTarget, viewport, w, mask, region, data);
#else
    OffscreenEffect::drawWindow(w, mask, region, data);
#endif
    m_shaderManager.Unbind();
}

bool ColorCorrectionFilterEffect::hasEffect(const KWin::EffectWindow *w) const {
    const auto name = w->windowClass().split(QChar::Space).first();
    return m_shaderManager.IsValid()
           && m_managed.contains(w);
           //&& (w->isNormalWindow() || w->isDialog());
}

QString ColorCorrectionFilterEffect::get_window_titles() const {
    QStringList response;
    for (const auto& [win, tiled]: m_managed) {
        const auto name = win->windowClass().split(QChar::Space).first();
        if (!response.contains(name))
            response.push_back(name);
    }
    return response.join("\n");
}

template<bool vertical>
bool ColorCorrectionFilterEffect::checkTiled(double window_start, const double& screen_end, double gap) {
    if (window_start == screen_end) {
        return true;    // Found the last chain of tiles
    } else if (window_start > screen_end) {
        return false;
    }

    const bool firstGap = (gap == -1);

    bool r = false;
    for (auto& [w, tiled]: m_managed) {

        if (firstGap) {
            gap = std::get<vertical>(std::make_pair(w->x(), w->y())) - window_start;
            if(gap > 40)        // There is no way that a window is tiled and has such a big gap.
                continue;
            window_start += gap;
        }

        if (std::get<vertical>(std::make_pair(w->x(), w->y())) == window_start) {
            if (std::get<vertical>(std::make_pair(w->width(), w->height())) + gap > 0) {
                if (checkTiled<vertical>(window_start + std::get<vertical>(std::make_pair(w->width(), w->height())) + gap, screen_end, gap)) {
                    tiled = true;   // Mark every tile as you go back to the first.
                    r = true;
                }
            }
        }

        if(firstGap) {
            window_start -= gap;    // Revert changes.
        }
    }
    return r;
}

void ColorCorrectionFilterEffect::checkTiled() {
    for (auto& [w, tiled]: m_managed) {     // Delete tile memory.
        tiled = false;
    }

    for (const auto& screen: KWin::effects->screens()) {        // Per every screen
        const auto& geometry = screen->geometry();
        checkTiled<false>(geometry.x(), geometry.x() + geometry.width()); // Check horizontally
        checkTiled<true>(geometry.y(), geometry.y() + geometry.height()); // Check vertically
    }
}
