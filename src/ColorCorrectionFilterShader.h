//
// Created by matin on 20/07/22.
// Adaptded by juanma1980 27/04/24
//

#ifndef KWIN4_SHAPECORNERS_CONFIG_SHADERMANAGER_H
#define KWIN4_SHAPECORNERS_CONFIG_SHADERMANAGER_H

#include <qconfig.h>
#if QT_VERSION_MAJOR >= 6
    #include <effect/effect.h>
#else
    #include <kwineffects.h>
#endif

namespace KWin {
    class GLShader;
    class ShaderManager;
}

inline QRectF operator *(const QRect& r, const qreal scale) { return {r.x() * scale, r.y() * scale, r.width() * scale, r.height() * scale}; }
inline QRectF operator *(const QRectF& r, const qreal scale) { return {r.x() * scale, r.y() * scale, r.width() * scale, r.height() * scale}; }
inline QVector2D toVector2D(const QSizeF& s) { return {static_cast<float>(s.width()), static_cast<float>(s.height())}; }

class ColorCorrectionFilterShader {
public:
    /**
     * \brief Loads the correct shader based on the GL platform in KWin.
     *        Then it assigns the location references for all its uniform variables so they could be written in.
     */
    explicit ColorCorrectionFilterShader();

    /**
     * \return True if the shader is valid and loaded
     */
    [[nodiscard]] bool IsValid() const;

    /**
     * \brief This function assigns the required variables to the shader.
     *        Then it pushes the shader to the stack of rendering.
     *        This needs to be called before each window is rendered.
     * \param w The window that the effect will be rendering on
     * \return A reference to the unique pointer of the loaded shader.
     */
    const std::unique_ptr<KWin::GLShader>& Bind() const;

    /**
     * \brief Pop the shader from the stack of rendering.
     *        This needs to be called after each window is rendered.
     */
    void Unbind() const;

    /**
     * \return A reference to the unique pointer of the loaded shader.
     */
    std::unique_ptr<KWin::GLShader>& GetShader() { return m_shader; }

private:
    /**
     * \brief A pointer to the loaded shader used to assign the value of uniform variables.
     */
    std::unique_ptr<KWin::GLShader> m_shader;

    /**
     * \brief An instance of `ShaderManager` used to load shader from file and push/pop the shader for each render.
     */
    KWin::ShaderManager* m_manager;

    /**
     * \brief Used only for its `palette()` function which holds the currently active highlight colors.
     */
    std::shared_ptr<QWidget> m_widget;

    /**
     * \brief Reference to `uniform vec2 windowSize;`
     *        Containing `window->frameGeometry().size()`
     */
    int m_shader_windowSize = 0;

    /**
     * \brief Reference to `uniform vec2 windowExpandedSize;`
     *        Containing `window->expandedGeometry().size()`
     * \note  When `expandedGeometry = frameGeometry`, it means there is no shadow.
     */
    int m_shader_windowExpandedSize = 0;

    /**
     * \brief Reference to `uniform vec2 windowTopLeft;`
     *        Containing the distance between the top-left of `expandedGeometry` and the top-left of `frameGeometry`.
     * \note  When `windowTopLeft = {0,0}`, it means `expandedGeometry = frameGeometry` and there is no shadow.
     */
    int m_shader_windowTopLeft = 0;

    /**
     * \brief Reference to `uniform vec4 colorCorrectionFilter;`
     *        Containing the filter to apply
     */
    int m_shader_colorCorrectionFilter = 0;

    /**
     * \brief Reference to `uniform vec4 applyColotCorrection;`
     *        Containing the bool that controls correction of transformed colors
     */
    int m_shader_applyColorCorrection = 0;

    /**
     * \brief Reference to `uniform sampler2D front;`
     *        Containing the active texture. When assigned to zero, it will contain the painted contents of the window.
     */
    int m_shader_front = 0;
};


#endif //KWIN4_SHAPECORNERS_CONFIG_SHADERMANAGER_H
