#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

namespace customconfigs {

struct SingboxParsedNode
{
    QString rawUrl;
    QString protocol;       // "vless", "trojan", "ss", "vmess", "hysteria2"
    QString tag;            // Name from fragment #tag
    QString server;         // IP or domain
    uint port = 0;
    QString uuidOrPassword;
    QString method;         // For shadowsocks

    // Security & TLS
    QString security;       // "tls", "reality", "none"
    QString sni;
    QString fingerprint;    // "chrome", "firefox", etc.
    QString flow;           // "xtls-rprx-vision", etc.
    QString realityPublicKey;
    QString realityShortId;
    bool insecure = false;
    QString alpn;

    // Transport
    QString transportType;  // "ws", "grpc", "httpupgrade", "tcp"
    QString path;
    QString serviceName;    // For gRPC
    QString headerType;

    bool isValid() const { return !server.isEmpty() && port > 0 && !protocol.isEmpty(); }
};

class SingboxUrlParser
{
public:
    static bool parseUrl(const QString &rawUrl, SingboxParsedNode &node);
    static QJsonObject generateConfigJson(const SingboxParsedNode &node, const QString &tunInterface = "ws-tun0");
    static QString generateConfigString(const SingboxParsedNode &node, const QString &tunInterface = "ws-tun0");

    static QString sanitizeTag(const QString &tag);

private:
    static bool parseVless(const QString &url, SingboxParsedNode &node);
    static bool parseTrojan(const QString &url, SingboxParsedNode &node);
    static bool parseShadowsocks(const QString &url, SingboxParsedNode &node);
    static QJsonObject buildOutbound(const SingboxParsedNode &node);
};

} // namespace customconfigs
