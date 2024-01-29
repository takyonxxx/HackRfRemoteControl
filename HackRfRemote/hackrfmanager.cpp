#include "hackrfmanager.h"

HackRfManager::HackRfManager(QObject *parent) :
    QThread(parent), m_stop(false), m_ptt(false)
{

}

HackRfManager::~HackRfManager()
{
    if (audioOutputThread) {
        delete audioOutputThread;
    }
}

void HackRfManager::setPtt(bool newPtt)
{
    m_ptt = newPtt;   
}

void HackRfManager::setStop(bool newStop)
{
    m_stop = newStop;
}

void HackRfManager::setDemod(Demod newDemod)
{
    if (audioOutputThread) {
        delete audioOutputThread;
    }

    auto samplingRate = getSamplingRate(newDemod);
    if (samplingRate != -1) {
        audioOutputThread = new AudioOutputThread(this, samplingRate);
    }
}

void HackRfManager::setBuffer(const QByteArray& buffer)
{
    if(!m_ptt)
    {
        m_bufferQueue.enqueue(buffer);
    }
}

void HackRfManager::run()
{
    while (!m_stop)
    {
        if (!m_bufferQueue.isEmpty())
        {
            if(audioOutputThread)
                audioOutputThread->writeBuffer(m_bufferQueue.dequeue());
        }
        QThread::msleep(10);
    }
}
