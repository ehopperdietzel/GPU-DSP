#ifndef AUDIOWORKER_H
#define AUDIOWORKER_H

#include <QObject>

class AudioWorker : public QObject
{
    Q_OBJECT

private:
    void setupCL();

public slots:
    void runAudio();
    void volumeChanged(int val);
    void freqChanged(int val);

};

#endif // AUDIOWORKER_H
