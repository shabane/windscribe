#include <QtTest>
#include <QJsonDocument>

#include "singboxcustomconfig.test.h"
#include "engine/customconfigs/singboxurlparser.h"
#include "engine/customconfigs/singboxcustomconfig.h"

using namespace customconfigs;

void TestSingboxCustomConfig::testParseVlessWs()
{
    QString url = "vless://e804f36a-2975-430f-b1e7-d4fa24be618c@89.22.236.222:8445?encryption=none&security=tls&sni=remove.wiregeek.ir&fp=chrome&insecure=1&allowInsecure=1&type=ws&path=%2F#Arya-VLESS-WS-8445";
    SingboxParsedNode node;
    bool ok = SingboxUrlParser::parseUrl(url, node);
    QVERIFY(ok);
    QCOMPARE(node.protocol, QString("vless"));
    QCOMPARE(node.server, QString("89.22.236.222"));
    QCOMPARE(node.port, 8445u);
    QCOMPARE(node.uuidOrPassword, QString("e804f36a-2975-430f-b1e7-d4fa24be618c"));
    QCOMPARE(node.security, QString("tls"));
    QCOMPARE(node.sni, QString("remove.wiregeek.ir"));
    QCOMPARE(node.fingerprint, QString("chrome"));
    QCOMPARE(node.transportType, QString("ws"));
    QCOMPARE(node.path, QString("/"));
    QCOMPARE(node.tag, QString("Arya-VLESS-WS-8445"));
    QVERIFY(node.insecure);
}

void TestSingboxCustomConfig::testParseTrojanGrpc()
{
    QString url = "trojan://arya-trojan-2026@89.22.236.222:8443?security=tls&sni=remove.wiregeek.ir&fp=chrome&insecure=1&allowInsecure=1&type=grpc&authority=&serviceName=trojan-grpc&mode=gun#Arya-Trojan-gRPC-8443";
    SingboxParsedNode node;
    bool ok = SingboxUrlParser::parseUrl(url, node);
    QVERIFY(ok);
    QCOMPARE(node.protocol, QString("trojan"));
    QCOMPARE(node.server, QString("89.22.236.222"));
    QCOMPARE(node.port, 8443u);
    QCOMPARE(node.uuidOrPassword, QString("arya-trojan-2026"));
    QCOMPARE(node.security, QString("tls"));
    QCOMPARE(node.sni, QString("remove.wiregeek.ir"));
    QCOMPARE(node.transportType, QString("grpc"));
    QCOMPARE(node.serviceName, QString("trojan-grpc"));
    QCOMPARE(node.tag, QString("Arya-Trojan-gRPC-8443"));
}

void TestSingboxCustomConfig::testParseShadowsocks()
{
    QString url = "ss://Y2hhY2hhMjAtcG9seTEzMDU6am1uY2lLM1dGQlFWeU1zZHg5M3V3SEtlZE5EaUhNQXMxQ29RQVJyL0IvYz0@az.wiregeek.ir:35387?#ss-self.corp.ss";
    SingboxParsedNode node;
    bool ok = SingboxUrlParser::parseUrl(url, node);
    QVERIFY(ok);
    QCOMPARE(node.protocol, QString("shadowsocks"));
    QCOMPARE(node.server, QString("az.wiregeek.ir"));
    QCOMPARE(node.port, 35387u);
    QCOMPARE(node.method, QString("chacha20-poly1305"));
    QCOMPARE(node.uuidOrPassword, QString("jmnciK3WFBQVyMsdxg93uwHKedNDiHMAs1CoQARr/B/c="));
    QCOMPARE(node.tag, QString("ss-self.corp.ss"));
}

void TestSingboxCustomConfig::testGenerateConfigJsonValid()
{
    QString url = "vless://e804f36a-2975-430f-b1e7-d4fa24be618c@89.22.236.222:8445?encryption=none&security=tls&sni=remove.wiregeek.ir&fp=chrome&insecure=1&allowInsecure=1&type=ws&path=%2F#Arya-VLESS-WS-8445";
    SingboxParsedNode node;
    SingboxUrlParser::parseUrl(url, node);

    QString configStr = SingboxUrlParser::generateConfigString(node, "ws-tun0");
    QVERIFY(!configStr.isEmpty());

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(configStr.toUtf8(), &err);
    QCOMPARE(err.error, QJsonParseError::NoError);
    QVERIFY(doc.isObject());

    QJsonObject root = doc.object();
    QVERIFY(root.contains("inbounds"));
    QVERIFY(root.contains("outbounds"));
    QVERIFY(root.contains("route"));
    QVERIFY(root.contains("dns"));
}

void TestSingboxCustomConfig::testParseMultipleUrlsFromFile()
{
    // Write sample temp file with mixed configs
    QTemporaryFile tempFile;
    QVERIFY(tempFile.open());
    tempFile.write("vless://e804f36a-2975-430f-b1e7-d4fa24be618c@89.22.236.222:8445?type=ws#Server-1\n");
    tempFile.write("trojan://pass123@89.22.236.222:8443?type=grpc#Server-2\n");
    tempFile.write("# Comment line\n\n");
    tempFile.write("ss://Y2hhY2hhMjAtcG9seTEzMDU6cGFzc3dvcmQ=@89.22.236.222:8000#Server-3\n");
    tempFile.close();

    auto configs = SingboxCustomConfig::makeFromUrlFile(tempFile.fileName());
    QCOMPARE(configs.size(), 3);
    QCOMPARE(configs[0]->name(), QString("Server-1"));
    QCOMPARE(configs[1]->name(), QString("Server-2"));
    QCOMPARE(configs[2]->name(), QString("Server-3"));
    QCOMPARE(configs[0]->type(), CUSTOM_CONFIG_SINGBOX);
}
