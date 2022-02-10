/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "videoplayer.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>
#include <QtCore/QDir>
#include <QFontDatabase>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QFile fontFile(":resources/font/Roboto-Regular.ttf");
    if(fontFile.exists() == true)
    {
        int newfontId = QFontDatabase::addApplicationFont(fontFile.fileName());
        QString fontname = QFontDatabase::applicationFontFamilies(newfontId).at(0);
        QFont font(fontname);
        QApplication::setFont(font);
    }

    QCoreApplication::setApplicationName("Video Widget Example");
    QCoreApplication::setOrganizationName("QtProject");
    QGuiApplication::setApplicationDisplayName(QCoreApplication::applicationName());
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt Video Widget Example");

    QCommandLineOption waylandSinkOption("w", "use waylandsink");
    parser.addOption(waylandSinkOption);

    QCommandLineOption intervalOption("i", "interval between paly & pause", "interval", "500");
    parser.addOption(intervalOption);

    QCommandLineOption autoRepeatOption("a", "auto play repeat");
    parser.addOption(autoRepeatOption);

    QCommandLineOption fullScreenOption("f", "use fullscreen");
    parser.addOption(fullScreenOption);

    QCommandLineOption thresholdOption("t", "threshold positionChanged event count", "threshold", "10");
    parser.addOption(thresholdOption);

    parser.process(app);

    bool waylandSink = parser.isSet(waylandSinkOption);
    QString interval = parser.value(intervalOption);
    bool autoRepeat = parser.isSet(autoRepeatOption);
    bool fullScreen = parser.isSet(fullScreenOption);
    QString threshold = parser.value(thresholdOption);

    VideoPlayer player(waylandSink, interval.toInt(), autoRepeat, fullScreen, threshold.toInt());


    const QRect availableGeometry = QApplication::desktop()->availableGeometry(&player);
    qWarning() << "availableGeometry: " << availableGeometry << availableGeometry.width() << availableGeometry.height();

    if (fullScreen) {
        player.showFullScreen();
    } else {
        player.resize(availableGeometry.width() / 6, availableGeometry.height() / 4);
        player.show();
    }

    return app.exec();
}
