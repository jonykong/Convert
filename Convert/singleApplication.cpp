#include "singleApplication.h"
#include <QtNetwork/QLocalSocket>
#include <QFileInfo>
#include <QLocalServer>
#include <QMessageBox>
#include <QIcon>

#define TIME_OUT                (500)    // 500ms

SingleApplication::SingleApplication(int &argc, char *argv[])
    : QApplication(argc, argv)
    , w(NULL)
    , m_bRunning(false)
    , m_pLocalServer(NULL)
{
    m_qsServerName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    initLocalConnection();
}

bool SingleApplication::isRunning()
{
    return m_bRunning;
}

void SingleApplication::newLocalConnection()
{
    QLocalSocket *socket = m_pLocalServer->nextPendingConnection();
    if (socket)
    {
        socket->waitForReadyRead(2 * TIME_OUT);
        delete socket;

        activateWindow();
    }
}

void SingleApplication::initLocalConnection()
{
    m_bRunning = false;
    QLocalSocket socket;
    socket.connectToServer(m_qsServerName);
    if (socket.waitForConnected(TIME_OUT))
    {
        fprintf(stderr, "%s already running.\n", m_qsServerName.toLocal8Bit().constData());
        m_bRunning = true;
        return;
    }
    newLocalServer();
}

void SingleApplication::newLocalServer()
{
    m_pLocalServer = new QLocalServer(this);
    connect(m_pLocalServer, SIGNAL(newConnection()), this, SLOT(newLocalConnection()));
    if (!m_pLocalServer->listen(m_qsServerName))
    {
        if (m_pLocalServer->serverError() == QAbstractSocket::AddressInUseError)
        {
            QLocalServer::removeServer(m_qsServerName);
            m_pLocalServer->listen(m_qsServerName);
        }
    }
}

void SingleApplication::activateWindow()
{
    if (w)
    {
        w->show();
        w->raise();
        w->activateWindow();
    }
}
