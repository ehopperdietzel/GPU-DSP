#include "Window.h"



Window::Window(QWidget *parent):QMainWindow(parent)
{
    // UI conf
    setCentralWidget(widget);
    verticalLayout->addWidget(new QLabel("Frecuency"));
    verticalLayout->addWidget(freqSlider);
    verticalLayout->addWidget(new QLabel("Volume"));
    verticalLayout->addWidget(volumeSlider);
    verticalLayout->addWidget(playbackBtn);

    freqSlider->setRange(0,20000);
    freqSlider->setValue(440);
    volumeSlider->setRange(0,1024);
    volumeSlider->setValue(1024);
    playbackBtn->setText("Play Tone");

    audioWorker->moveToThread(&audioThread);

    connect(playbackBtn, &QPushButton::clicked, audioWorker, &AudioWorker::runAudio);
    connect(freqSlider, &QSlider::valueChanged, audioWorker, &AudioWorker::freqChanged);
    connect(volumeSlider, &QSlider::valueChanged, audioWorker, &AudioWorker::volumeChanged);
    connect(&audioThread, &QThread::finished, audioWorker, &QObject::deleteLater);

    audioThread.start();
    audioThread.setPriority(QThread::Priority::HighestPriority);
}



Window::~Window()
{
    audioThread.quit();
    audioThread.wait();
}

