#pragma once

#include <QVector>
#include <QSharedPointer>

#include "icustomconfig.h"
#include "singboxurlparser.h"

namespace customconfigs {

class SingboxCustomConfig : public ICustomConfig
{
public:
    SingboxCustomConfig() = default;
    explicit SingboxCustomConfig(const SingboxParsedNode &node, const QString &sourceFile, int index = 0);

    CUSTOM_CONFIG_TYPE type() const override;
    QString name() const override;
    QString nick() const override;
    QString filename() const override;
    QStringList hostnames() const override;
    bool isAllowFirewallAfterConnection() const override;

    bool isCorrect() const override;
    QString getErrorForIncorrect() const override;

    const SingboxParsedNode &parsedNode() const { return node_; }
    uint getEndpointPort() const { return node_.port; }
    QString generateRuntimeJsonConfig(const QString &tunInterface = "ws-tun0") const;

    static QVector<QSharedPointer<const ICustomConfig>> makeFromUrlFile(const QString &filepath);
    static ICustomConfig *makeFromJsonFile(const QString &filepath);

private:
    SingboxParsedNode node_;
    QString name_;
    QString nick_;
    QString filename_;
    QString errMessage_;
    bool isCorrect_ = false;
};

} // namespace customconfigs
