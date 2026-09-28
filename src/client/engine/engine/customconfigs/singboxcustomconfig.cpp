#include "singboxcustomconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

namespace customconfigs {

SingboxCustomConfig::SingboxCustomConfig(const SingboxParsedNode &node, const QString &sourceFile, int index)
    : node_(node)
{
    QFileInfo fi(sourceFile);
    name_ = node_.tag.isEmpty() ? QString("%1-%2").arg(fi.baseName()).arg(index + 1) : node_.tag;
    nick_ = QString("%1:%2").arg(node_.server).arg(node_.port);
    filename_ = QString("%1#%2_%3").arg(fi.fileName()).arg(index).arg(node_.tag.replace(" ", "_"));
    isCorrect_ = node_.isValid();
    if (!isCorrect_)
        errMessage_ = "Invalid sing-box URL or parameters";
}

CUSTOM_CONFIG_TYPE SingboxCustomConfig::type() const
{
    return CUSTOM_CONFIG_SINGBOX;
}

QString SingboxCustomConfig::name() const
{
    return name_;
}

QString SingboxCustomConfig::nick() const
{
    return nick_;
}

QString SingboxCustomConfig::filename() const
{
    return filename_;
}

QStringList SingboxCustomConfig::hostnames() const
{
    QStringList list;
    if (!node_.server.isEmpty())
        list << node_.server;
    return list;
}

bool SingboxCustomConfig::isAllowFirewallAfterConnection() const
{
    return true;
}

bool SingboxCustomConfig::isCorrect() const
{
    return isCorrect_;
}

QString SingboxCustomConfig::getErrorForIncorrect() const
{
    return errMessage_;
}

QString SingboxCustomConfig::generateRuntimeJsonConfig(const QString &tunInterface) const
{
    return SingboxUrlParser::generateConfigString(node_, tunInterface);
}

QVector<QSharedPointer<const ICustomConfig>> SingboxCustomConfig::makeFromUrlFile(const QString &filepath)
{
    QVector<QSharedPointer<const ICustomConfig>> configs;
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return configs;

    QTextStream in(&file);
    int index = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#") || line.startsWith("//"))
            continue;

        SingboxParsedNode node;
        if (SingboxUrlParser::parseUrl(line, node))
        {
            auto *cfg = new SingboxCustomConfig(node, filepath, index++);
            configs.append(QSharedPointer<const ICustomConfig>(cfg));
        }
    }
    return configs;
}

ICustomConfig *SingboxCustomConfig::makeFromJsonFile(const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        return nullptr;

    QJsonObject obj = doc.object();
    SingboxParsedNode node;
    node.protocol = "singbox_json";
    QFileInfo fi(filepath);
    node.tag = fi.baseName();
    node.server = "127.0.0.1";
    node.port = 443;

    return new SingboxCustomConfig(node, filepath, 0);
}

} // namespace customconfigs
