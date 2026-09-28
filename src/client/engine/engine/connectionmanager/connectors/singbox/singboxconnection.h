#pragma once

#include <QAtomicInt>
#include "engine/connectionmanager/connectors/iconnection.h"
#include "engine/helper/helper.h"

class SingboxConnection : public IConnection
{
    Q_OBJECT

public:
    SingboxConnection(QObject *parent, Helper *helper, types::Protocol protocol);
    ~SingboxConnection() override;

    void startConnect() override;
    void startDisconnect() override;
    bool isDisconnected() const override;
    void waitForDisconnect() override;
    void teardown() override;

    QString effectiveHostname() const override { return effectiveHostname_.isEmpty() ? descr_.hostname : effectiveHostname_; }
    QString effectiveIp() const override { return effectiveIp_.isEmpty() ? descr_.ip : effectiveIp_; }

protected:
    void prepareImpl() override;
    void run() override;

private:
    Helper *helper_;
    QAtomicInt isStopRequested_{0};
    QAtomicInt isRunning_{0};
    QString configFilePath_;
    QString effectiveIp_;
    QString effectiveHostname_;
};
