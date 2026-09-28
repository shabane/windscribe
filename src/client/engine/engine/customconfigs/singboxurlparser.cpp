#include "singboxurlparser.h"

#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QByteArray>

namespace customconfigs {

bool SingboxUrlParser::parseUrl(const QString &rawUrl, SingboxParsedNode &node)
{
    QString trimmed = rawUrl.trimmed();
    if (trimmed.startsWith("vless://", Qt::CaseInsensitive))
    {
        return parseVless(trimmed, node);
    }
    else if (trimmed.startsWith("trojan://", Qt::CaseInsensitive))
    {
        return parseTrojan(trimmed, node);
    }
    else if (trimmed.startsWith("ss://", Qt::CaseInsensitive))
    {
        return parseShadowsocks(trimmed, node);
    }
    return false;
}

bool SingboxUrlParser::parseVless(const QString &urlStr, SingboxParsedNode &node)
{
    QUrl url(urlStr);
    if (!url.isValid())
        return false;

    node.rawUrl = urlStr;
    node.protocol = "vless";
    node.uuidOrPassword = url.userName();
    node.server = url.host();
    node.port = url.port(443);
    node.tag = QUrl::fromPercentEncoding(url.fragment().toUtf8());
    if (node.tag.isEmpty())
        node.tag = QString("%1:%2").arg(node.server).arg(node.port);

    QUrlQuery query(url);
    node.security = query.queryItemValue("security");
    node.sni = query.queryItemValue("sni");
    node.fingerprint = query.queryItemValue("fp");
    node.flow = query.queryItemValue("flow");
    node.realityPublicKey = query.queryItemValue("pbk");
    node.realityShortId = query.queryItemValue("sid");
    node.transportType = query.queryItemValue("type").toLower();
    if (node.transportType.isEmpty())
        node.transportType = "tcp";
    node.path = QUrl::fromPercentEncoding(query.queryItemValue("path").toUtf8());
    node.serviceName = query.queryItemValue("serviceName");
    node.alpn = query.queryItemValue("alpn");
    node.headerType = query.queryItemValue("headerType");

    QString insecureStr = query.queryItemValue("insecure");
    if (insecureStr.isEmpty())
        insecureStr = query.queryItemValue("allowInsecure");
    node.insecure = (insecureStr == "1" || insecureStr.compare("true", Qt::CaseInsensitive) == 0);

    return node.isValid();
}

bool SingboxUrlParser::parseTrojan(const QString &urlStr, SingboxParsedNode &node)
{
    QUrl url(urlStr);
    if (!url.isValid())
        return false;

    node.rawUrl = urlStr;
    node.protocol = "trojan";
    node.uuidOrPassword = url.userName();
    node.server = url.host();
    node.port = url.port(443);
    node.tag = QUrl::fromPercentEncoding(url.fragment().toUtf8());
    if (node.tag.isEmpty())
        node.tag = QString("%1:%2").arg(node.server).arg(node.port);

    QUrlQuery query(url);
    node.security = query.queryItemValue("security");
    if (node.security.isEmpty())
        node.security = "tls";
    node.sni = query.queryItemValue("sni");
    node.fingerprint = query.queryItemValue("fp");
    node.alpn = query.queryItemValue("alpn");
    node.transportType = query.queryItemValue("type").toLower();
    if (node.transportType.isEmpty())
        node.transportType = "tcp";
    node.path = QUrl::fromPercentEncoding(query.queryItemValue("path").toUtf8());
    node.serviceName = query.queryItemValue("serviceName");

    QString insecureStr = query.queryItemValue("insecure");
    if (insecureStr.isEmpty())
        insecureStr = query.queryItemValue("allowInsecure");
    node.insecure = (insecureStr == "1" || insecureStr.compare("true", Qt::CaseInsensitive) == 0);

    return node.isValid();
}

bool SingboxUrlParser::parseShadowsocks(const QString &urlStr, SingboxParsedNode &node)
{
    QString stripped = urlStr;
    if (stripped.startsWith("ss://", Qt::CaseInsensitive))
        stripped = stripped.mid(5);

    QString fragment;
    int hashIdx = stripped.indexOf('#');
    if (hashIdx >= 0)
    {
        fragment = QUrl::fromPercentEncoding(stripped.mid(hashIdx + 1).toUtf8());
        stripped = stripped.left(hashIdx);
    }

    int atIdx = stripped.lastIndexOf('@');
    if (atIdx >= 0)
    {
        QString userinfo = stripped.left(atIdx);
        QString hostPortPart = stripped.mid(atIdx + 1);

        // Try decoding userinfo if it is base64
        QByteArray decodedUserinfo = QByteArray::fromBase64(userinfo.toUtf8());
        QString resolvedUserinfo = userinfo;
        if (decodedUserinfo.contains(':'))
        {
            resolvedUserinfo = QString::fromUtf8(decodedUserinfo);
        }

        int colonUser = resolvedUserinfo.indexOf(':');
        if (colonUser > 0)
        {
            node.method = resolvedUserinfo.left(colonUser);
            node.uuidOrPassword = resolvedUserinfo.mid(colonUser + 1);
        }

        int queryIdx = hostPortPart.indexOf('?');
        if (queryIdx >= 0)
            hostPortPart = hostPortPart.left(queryIdx);

        int colonHost = hostPortPart.lastIndexOf(':');
        if (colonHost > 0)
        {
            node.server = hostPortPart.left(colonHost);
            node.port = hostPortPart.mid(colonHost + 1).toUInt();
        }
    }
    else
    {
        // Whole url could be base64
        int queryIdx = stripped.indexOf('?');
        QString b64 = (queryIdx >= 0) ? stripped.left(queryIdx) : stripped;
        QByteArray decoded = QByteArray::fromBase64(b64.toUtf8());
        return parseShadowsocks("ss://" + QString::fromUtf8(decoded) + (fragment.isEmpty() ? "" : "#" + fragment), node);
    }

    node.rawUrl = urlStr;
    node.protocol = "shadowsocks";
    node.tag = fragment.isEmpty() ? QString("%1:%2").arg(node.server).arg(node.port) : fragment;

    return node.isValid();
}

QJsonObject SingboxUrlParser::buildOutbound(const SingboxParsedNode &node)
{
    QJsonObject outbound;
    outbound["tag"] = "proxy-out";
    outbound["server"] = node.server;
    outbound["server_port"] = static_cast<int>(node.port);

    if (node.protocol == "vless")
    {
        outbound["type"] = "vless";
        outbound["uuid"] = node.uuidOrPassword;
        if (!node.flow.isEmpty())
            outbound["flow"] = node.flow;
    }
    else if (node.protocol == "trojan")
    {
        outbound["type"] = "trojan";
        outbound["password"] = node.uuidOrPassword;
    }
    else if (node.protocol == "shadowsocks")
    {
        outbound["type"] = "shadowsocks";
        QString m = node.method.toLower();
        if (m == "chacha20-poly1305" || m == "chacha20-ietf-poly1305")
            outbound["method"] = "chacha20-ietf-poly1305";
        else
            outbound["method"] = node.method.isEmpty() ? "chacha20-ietf-poly1305" : node.method;
        outbound["password"] = node.uuidOrPassword;
        return outbound;
    }

    // TLS / Reality
    if (node.security == "tls" || node.security == "reality")
    {
        QJsonObject tls;
        tls["enabled"] = true;
        if (!node.sni.isEmpty())
            tls["server_name"] = node.sni;
        else
            tls["server_name"] = node.server;

        tls["insecure"] = node.insecure;

        if (!node.alpn.isEmpty())
        {
            QJsonArray alpnArr;
            for (const QString &a : node.alpn.split(','))
                alpnArr.append(a.trimmed());
            tls["alpn"] = alpnArr;
        }

        if (!node.fingerprint.isEmpty())
        {
            QJsonObject utls;
            utls["enabled"] = true;
            utls["fingerprint"] = node.fingerprint;
            tls["utls"] = utls;
        }

        if (node.security == "reality")
        {
            QJsonObject reality;
            reality["enabled"] = true;
            reality["public_key"] = node.realityPublicKey;
            reality["short_id"] = node.realityShortId;
            tls["reality"] = reality;
        }

        outbound["tls"] = tls;
    }

    // Transport
    if (node.transportType == "ws")
    {
        QJsonObject transport;
        transport["type"] = "ws";
        transport["path"] = node.path.isEmpty() ? "/" : node.path;
        if (!node.sni.isEmpty())
        {
            QJsonObject headers;
            headers["Host"] = node.sni;
            transport["headers"] = headers;
        }
        outbound["transport"] = transport;
    }
    else if (node.transportType == "grpc")
    {
        QJsonObject transport;
        transport["type"] = "grpc";
        transport["service_name"] = node.serviceName;
        outbound["transport"] = transport;
    }
    else if (node.transportType == "httpupgrade")
    {
        QJsonObject transport;
        transport["type"] = "httpupgrade";
        transport["path"] = node.path.isEmpty() ? "/" : node.path;
        if (!node.sni.isEmpty())
        {
            QJsonObject host;
            host["Host"] = node.sni;
            transport["headers"] = host;
        }
        outbound["transport"] = transport;
    }

    return outbound;
}

QJsonObject SingboxUrlParser::generateConfigJson(const SingboxParsedNode &node, const QString &tunInterface)
{
    QJsonObject root;

    // Log
    QJsonObject log;
    log["level"] = "info";
    log["timestamp"] = true;
    root["log"] = log;

    // Inbounds (TUN)
    QJsonArray inbounds;
    QJsonObject tun;
    tun["type"] = "tun";
    tun["tag"] = "tun-in";
    tun["interface_name"] = tunInterface;
    QJsonArray addressArr;
    addressArr.append("172.19.0.1/30");
    tun["address"] = addressArr;
    tun["auto_route"] = true;
    tun["strict_route"] = true;
    tun["stack"] = "gvisor";
    inbounds.append(tun);
    root["inbounds"] = inbounds;

    // Outbounds
    QJsonArray outbounds;
    outbounds.append(buildOutbound(node));

    QJsonObject direct;
    direct["type"] = "direct";
    direct["tag"] = "direct";
    outbounds.append(direct);

    root["outbounds"] = outbounds;

    // DNS
    QJsonObject dns;
    QJsonArray dnsServers;
    QJsonObject remoteDns;
    remoteDns["tag"] = "remote-dns";
    remoteDns["type"] = "udp";
    remoteDns["server"] = "1.1.1.1";
    remoteDns["detour"] = "proxy-out";
    dnsServers.append(remoteDns);

    QJsonObject localDns;
    localDns["tag"] = "local-dns";
    localDns["type"] = "local";
    localDns["detour"] = "direct";
    dnsServers.append(localDns);

    dns["servers"] = dnsServers;
    root["dns"] = dns;

    // Route
    QJsonObject route;
    route["default_domain_resolver"] = "remote-dns";
    QJsonArray rules;

    QJsonObject dnsRule;
    dnsRule["protocol"] = QJsonArray() << "dns";
    dnsRule["action"] = "hijack-dns";
    rules.append(dnsRule);

    QJsonObject privateIpRule;
    privateIpRule["ip_is_private"] = true;
    privateIpRule["outbound"] = "direct";
    rules.append(privateIpRule);

    route["rules"] = rules;
    route["auto_detect_interface"] = true;
    root["route"] = route;

    return root;
}

QString SingboxUrlParser::generateConfigString(const SingboxParsedNode &node, const QString &tunInterface)
{
    QJsonObject json = generateConfigJson(node, tunInterface);
    QJsonDocument doc(json);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

} // namespace customconfigs
