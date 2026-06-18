#include <QtTest>
#include "domain/RingBuffer.h"

class RingBufferUnitTest : public QObject
{
    Q_OBJECT

private slots:
    void testInitialState();
    void testPushAndOverflow();
    void testClear();
};

void RingBufferUnitTest::testInitialState()
{
    Saiko::Domain::RingBuffer buffer;
    QCOMPARE(buffer.size(), 0U);
    QCOMPARE(buffer.maxBytes(), 0U);
}

void RingBufferUnitTest::testPushAndOverflow()
{
    Saiko::Domain::RingBuffer buffer;
    buffer.setMaxBytes(5);
    QCOMPARE(buffer.maxBytes(), 5U);

    uint8_t data[] = {1, 2, 3, 4, 5};
    buffer.push(data, 5);
    QCOMPARE(buffer.size(), 5U);
    QCOMPARE(buffer.data()[0], 1);
    QCOMPARE(buffer.data()[4], 5);

    uint8_t extra[] = {6, 7};
    buffer.push(extra, 2);
    QCOMPARE(buffer.size(), 5U); // capped at 5
    QCOMPARE(buffer.data()[0], 3); // 1 and 2 dropped
    QCOMPARE(buffer.data()[4], 7);
}

void RingBufferUnitTest::testClear()
{
    Saiko::Domain::RingBuffer buffer;
    buffer.setMaxBytes(10);
    uint8_t data[] = {1, 2, 3};
    buffer.push(data, 3);
    QCOMPARE(buffer.size(), 3U);

    buffer.clear();
    QCOMPARE(buffer.size(), 0U);
}

QTEST_APPLESS_MAIN(RingBufferUnitTest)
#include "ringbuffer_test.moc"
