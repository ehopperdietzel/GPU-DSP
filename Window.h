#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QPushButton>

#include "AudioWorker.h"

class Window : public QMainWindow
{
    Q_OBJECT
    QThread audioThread;

public:
    Window(QWidget *parent = nullptr);
    ~Window();
private:
    QWidget *widget = new QWidget(this);
    QVBoxLayout *verticalLayout = new QVBoxLayout(widget);
    QPushButton *playbackBtn = new QPushButton("Play",this);
    QSlider *freqSlider = new QSlider(Qt::Orientation::Horizontal,this);
    QSlider *volumeSlider = new QSlider(Qt::Orientation::Horizontal,this);

    AudioWorker *audioWorker = new AudioWorker;



};
#endif // WINDOW_H
