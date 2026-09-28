#pragma once

#include <QObject>

class TestSingboxCustomConfig : public QObject
{
    Q_OBJECT

private slots:
    void testParseVlessWs();
    void testParseTrojanGrpc();
    void testParseShadowsocks();
    void testGenerateConfigJsonValid();
    void testParseMultipleUrlsFromFile();
};
