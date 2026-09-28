#include "singboxconnection.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include "utils/log/logger.h"

SingboxConnection::SingboxConnection(QObject *parent, Helper *helper, types::Protocol protocol)
    : IConnection(parent, protocol), helper_(helper)
{
}

SingboxConnection::~SingboxConnection()
{
    teardown();
}

void SingboxConnection::teardown()
{
    startDisconnect();
    waitForDisconnect();
}

void SingboxConnection::prepareImpl()
{
    effectiveIp_ = descr_.ip;
    effectiveHostname_ = descr_.hostname;

    QString jsonConfig = descr_.singbox.runtimeConfigJson;
    if (jsonConfig.trimmed().isEmpty())
    {
        qCCritical(LOG_CONNECTION) << "SingboxConnection: runtime config is empty";
        emit prepareFailed(ConnectError::kCustomConfigInvalid);
        return;
    }

    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    configFilePath_ = tempDir + "/windscribe_singbox_config.json";

    QFile file(configFilePath_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        qCCritical(LOG_CONNECTION) << "SingboxConnection: cannot write to" << configFilePath_;
        emit prepareFailed(ConnectError::kInternalClientError);
        return;
    }

    file.write(jsonConfig.toUtf8());
    file.close();

    qCInfo(LOG_CONNECTION) << "SingboxConnection: prepared config at" << configFilePath_;
    emit prepared();
}

void SingboxConnection::startConnect()
{
    isStopRequested_ = 0;
    isRunning_ = 1;
    start();
}

void SingboxConnection::startDisconnect()
{
    isStopRequested_ = 1;
}

bool SingboxConnection::isDisconnected() const
{
    return isRunning_ == 0;
}

void SingboxConnection::waitForDisconnect()
{
    if (isRunning_)
    {
        isStopRequested_ = 1;
        wait(5000);
    }
}

void SingboxConnection::run()
{
    qCInfo(LOG_CONNECTION) << "SingboxConnection: starting sing-box through helper...";

    if (!helper_->startSingbox(configFilePath_))
    {
        qCCritical(LOG_CONNECTION) << "SingboxConnection: failed to start sing-box via helper";
        isRunning_ = 0;
        emit error(ConnectError::kTunnelEstablishmentFailure);
        return;
    }

    emit interfaceUpdated("ws-tun0");

    AdapterGatewayInfo adapterInfo;
    adapterInfo.setAdapterName("ws-tun0");
    if (!effectiveIp_.isEmpty())
        adapterInfo.setRemoteIp(types::IpAddress(effectiveIp_));

    qCInfo(LOG_CONNECTION) << "SingboxConnection: connected to" << effectiveIp_;
    emit connected(adapterInfo);

    while (!isStopRequested_)
    {
        msleep(500);
    }

    qCInfo(LOG_CONNECTION) << "SingboxConnection: stopping sing-box...";
    helper_->stopSingbox();

    isRunning_ = 0;
    emit disconnected();
}
