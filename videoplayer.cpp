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

#include <QtWidgets>
#include <QVideoWidget>

VideoPlayer::VideoPlayer(bool waylandSink, int interval, bool autoRepeat, bool fullScreen, int threshold, QWidget *parent)
    : QWidget(parent)
{
    useWaylandSink = waylandSink;
    playInterval = interval;
    autoPlayRepeat = autoRepeat;
    useFullScreen = fullScreen;
    thresholdTimerCount = threshold;
    qWarning() << "waylandSink: " << useWaylandSink << ", playInterval: " << playInterval << ", autoPlayRepeat: " << autoPlayRepeat << ", fullScreen: " << useFullScreen << ", threshold: " << thresholdTimerCount;

    m_mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);

    QVideoWidget *videoWidget = new QVideoWidget;
    QAbstractButton *openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::openFile);

    QAbstractButton *autoButton = new QPushButton(tr("Auto"));
    connect(autoButton, &QAbstractButton::clicked, this, &VideoPlayer::autoTestStart);

    m_playButton = new QPushButton;
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

    connect(m_playButton, &QAbstractButton::clicked,
            this, &VideoPlayer::play);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setRange(0, 0);

    connect(m_positionSlider, &QAbstractSlider::sliderMoved,
            this, &VideoPlayer::setPosition);

    m_errorLabel = new QLabel;
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QBoxLayout *controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(autoButton);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_positionSlider);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(videoWidget);
    layout->addWidget(m_errorLabel);
    if (fullScreen) {
        layout->addLayout(controlLayout);
    } else {
        QWidget *controlWidget = new QWidget(nullptr);
        controlWidget->setLayout(controlLayout);
        controlWidget->show();
    }
    setLayout(layout);

    m_mediaPlayer->setVideoOutput(videoWidget);
    connect(m_mediaPlayer, &QMediaPlayer::stateChanged,
            this, &VideoPlayer::mediaStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &VideoPlayer::mediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),
            this, &VideoPlayer::handleError);

    timer.start();

    playTimer = new QTimer(this);
    connect(playTimer, SIGNAL(timeout()), this, SLOT(timerCallback()));

}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::timerCallback()
{
    qWarning() << "timer callback";
    if (timerCount > thresholdTimerCount) {
        qWarning() << "=================================== receive the same position " << thresholdTimerCount << " times";
        timerCount = 0;
        m_mediaPlayer->setMedia(originalUrl);
        playTimer->stop();
    }
    timerCount++;
    play();
}

void VideoPlayer::autoTestStart()
{
    if (playTimer->isActive()) {
        playTimer->stop();
        autoPlayRepeat = false;
    } else {
        playTimer->start(playInterval);
        autoPlayRepeat = true;
    }
}

void VideoPlayer::openFile()
{
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    QStringList supportedMimeTypes = m_mediaPlayer->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty())
        fileDialog.setMimeTypeFilters(supportedMimeTypes);

    if (fileDialog.exec() == QDialog::Accepted)
        setUrl(fileDialog.selectedUrls().constFirst());
}

void VideoPlayer::setUrl(const QUrl &url)
{
    m_errorLabel->setText(QString());
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());

    if (useWaylandSink) {
        QString temp = "gst-pipeline: playbin uri=";
        temp.append(url.toString());
        temp.append(" video-sink=waylandsink");
        originalUrl = QUrl(temp);
    } else {
        originalUrl = url;
    }

    m_mediaPlayer->setMedia(originalUrl);
    m_playButton->setEnabled(true);
}

void VideoPlayer::play()
{
    qWarning() << __FUNCTION__ << m_mediaPlayer->state() << timer.nsecsElapsed() / 1000  / 1000;

    if (mediaStatus != QMediaPlayer::BufferedMedia) {
        qWarning() << "play ignored because mediaState: " << mediaStatus;
        return;
    }
    switch (m_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState:
        m_mediaPlayer->pause();
        break;
    default:
        m_mediaPlayer->play();
        break;
    }
}

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    qWarning() << __FUNCTION__ << state << timer.nsecsElapsed() / 1000 / 1000;
    switch(state) {
    case QMediaPlayer::PlayingState:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void VideoPlayer::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qWarning() << __FUNCTION__ << status << timer.nsecsElapsed() / 1000  / 1000;
    mediaStatus = status;
    switch(status) {
    case QMediaPlayer::EndOfMedia:
        m_mediaPlayer->setMedia(originalUrl);
        playTimer->stop();
        break;
    case QMediaPlayer::LoadedMedia:
        m_mediaPlayer->play();
        if (autoPlayRepeat) {
            playTimer->start(playInterval);
        }
        break;
    case QMediaPlayer::InvalidMedia:
        playTimer->stop();
        break;
    }
}

void VideoPlayer::positionChanged(qint64 position)
{
//    qWarning() << "positionChanged: " << position;
    if (previousePosition != position) {
        timerCount = 0;
    }
    previousePosition = position;
    m_positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void VideoPlayer::handleError()
{
    m_playButton->setEnabled(false);
    const QString errorString = m_mediaPlayer->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(m_mediaPlayer->error()));
    else
        message += errorString;
    m_errorLabel->setText(message);
}
