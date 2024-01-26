#include "hackrfmanager.h"

HackRfManager::HackRfManager(QObject *parent) :
    QThread(parent), m_stop(false), m_ptt(false)
{
    audioOutputThread = new AudioOutputThread(this);
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
