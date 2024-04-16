//
// Created by matin on 20/07/22.
// Adaptded by juanma1980 27/04/24
//

#include "ColorCorrectionFilterShader.h"
#include "ColorCorrectionFilterEffect.h"
#include "ColorCorrectionFilterConfig.h"
#include <QFile>
#include <QWidget>

#if QT_VERSION_MAJOR >= 6
    #include <opengl/glutils.h>
#else
    #include <kwinglutils.h>
#endif

ColorCorrectionFilterShader::ColorCorrectionFilterShader():
        m_manager(KWin::ShaderManager::instance()),
        m_widget(new QWidget)
{
    const QString fragmentshader = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kwin/shaders/Daltonize.frag"));
    qCritical() << fragmentshader;
    QFile file(fragmentshader);
    if (!file.open(QFile::ReadOnly))
        qCritical() << "ColorFilterCorrection: no shaders found! Exiting...";

    const QByteArray frag = file.readAll();
    auto shader = m_manager->generateShaderFromFile(KWin::ShaderTrait::MapTexture, QStringLiteral(""), fragmentshader);
    m_shader = std::move(shader);
    file.close();
    if (!m_shader->isValid())
        qCritical() << "ColorCorrectionFilter: no valid shaders found! ColorCorrectionFilter will not work.";

    m_shader_colorCorrectionFilter = m_shader->uniformLocation("colorCorrectionFilter");
    m_shader_applyColorCorrection = m_shader->uniformLocation("applyColorCorrection");
    qInfo() << "ColorCorrection: shaders loaded. Option";
}

bool ColorCorrectionFilterShader::IsValid() const {
    return m_shader && m_shader->isValid();
}

const std::unique_ptr<KWin::GLShader>&
ColorCorrectionFilterShader::Bind() const {
	auto filter=static_cast<int>(ColorCorrectionFilterConfig::colorCorrectionFilter());
	auto correction=static_cast<bool>(ColorCorrectionFilterConfig::applyColorCorrection());
    m_manager->pushShader(m_shader.get());
    m_shader->setUniform("colorCorrectionFilter",filter);
    m_shader->setUniform("applyColorCorrection",correction);
    return m_shader;
}

void ColorCorrectionFilterShader::Unbind() const {
    m_manager->popShader();
}
