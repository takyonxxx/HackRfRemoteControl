#include "hackrfmanager.h"

HackRfManager::HackRfManager(QObject *parent) :
    QObject(parent), m_stop(false), m_ptt(false)
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

void HackRfManager::setBuffer(const QByteArray& buffer)
{
    if(!m_ptt)
    {        
        if(audioOutputThread)
            audioOutputThread->writeBuffer(buffer);
    }
}
