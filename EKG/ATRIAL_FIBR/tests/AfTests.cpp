#include <QString>
#include <QtTest>

#include <cmath>
#include <numeric>
#include <iostream>
#include "../src/RRIntervals.h"
#include "../src/AtrialFibrApi.h"
#include "../src/PWave.h"
#include <stdlib.h>
#include <QVector>

using namespace std;
using namespace Ecg::AtrialFibr;

class RRSanityTest : public QObject {
  Q_OBJECT
  RRIntervalMethod rrmethod;

public:
  RRSanityTest();

private
Q_SLOTS:
  void initTestCase();
  void countRRIntervalsOneInterval();
  void countRRIntervalsThreeIntervals();
  void classifyIntervalsTest();
  void countTransitionsTest();
  void normalizeMarkovTableTest();
  void RRRunTest();
  void entropyBig();
  void entropySmall();
  void KLDivergenceEqualMatrix();
  void KLDivergenceTest();
  void JKDivergenceEqualMatrix();
  void JKDivergenceTest();
  void correlation_ObviousCases();
  void pWaveOccurence_AllFound();
  void pWaveOccurence_HalfFound();
  void pWaveOccurence_ThrowIfPWaveStartTooCloseToEndOfSignal();

  void closestP();
  void closestP_SinglePPeak();
  void closestP_TwoPPeaks();
  void calcRWaveSets_SingleR();
  void calcSets_SingleR();
};

RRSanityTest::RRSanityTest() {}

void RRSanityTest::initTestCase() {}

void RRSanityTest::countRRIntervalsOneInterval() {
  // Arrange
  QVector<double> signal(10);
  QVector<QVector<double>::const_iterator> RRTime{ signal.begin() + 1,
                                                   signal.begin() + 2 };

  // Act
  QVector<int> intervals =
      rrmethod.countRRInvervals(begin(RRTime), end(RRTime));

  // Assert
  QCOMPARE(intervals.front(), 1);
  QCOMPARE(intervals.size(), RRTime.size() - 1);
}

void RRSanityTest::countRRIntervalsThreeIntervals() {
  QVector<double> signal(10);

  // Arrange
  QVector<QVector<double>::const_iterator> RRTime = {
    signal.begin() + 1, signal.begin() + 2, signal.begin() + 5,
    signal.begin() + 7, signal.begin() + 9
  };
  QVector<int> ExpIntervals = { 1, 3, 2, 2 };

  // Act
  QVector<int> intervals =
      rrmethod.countRRInvervals(begin(RRTime), end(RRTime));

  // Assert
  QCOMPARE(intervals.size(), RRTime.size() - 1);
  QCOMPARE(intervals, ExpIntervals);
}

void RRSanityTest::classifyIntervalsTest() {
  // Arrange
  QVector<int> intervals = { 10, 10, 15, 5 };
  QVector<classification> expectedIntervals = { Regular, Regular, Long, Short };

  // Act
  const int averageInterval = countAverageInterval(intervals);
  QVector<classification> classifiedIntervals =
      classifyIntervals(intervals, averageInterval);

  // Assert
  QVERIFY(classifiedIntervals == expectedIntervals);
}

void RRSanityTest::countTransitionsTest() {
  // Arrange
  QVector<int> intervals = { 10, 10, 15, 05 };
  std::array<std::array<double, 3>, 3> ExpectedArray = {
    { { { 0, 0, 0 } }, { { 0, 1, 1 } }, { { 1, 0, 0 } } }
  };

  // Act
  int interval = countAverageInterval(intervals);
  QVector<classification> classifiedIntervals =
      classifyIntervals(intervals, interval);
  rrmethod.countTransitions(classifiedIntervals);

  // Assert
  QVERIFY(rrmethod.getMarkovTable() == ExpectedArray);
}

void RRSanityTest::normalizeMarkovTableTest() {
  // Arrange
  QVector<int> intervals = { 10, 10, 10, 15, 5 };
  std::array<std::array<double, 3>, 3> ExpectedArray = {
    { { { 0, 0, 0 } }, { { 0, 0.5, 0.25 } }, { { 0.25, 0, 0 } } }
  };

  // Act
  const int avgInterval = countAverageInterval(intervals);
  QVector<classification> classifiedIntervals =
      classifyIntervals(intervals, avgInterval);
  rrmethod.countTransitions(classifiedIntervals);
  rrmethod.normalizeMarkovTable();

  // Assert
  QVERIFY(rrmethod.getMarkovTable() == ExpectedArray);
}

void RRSanityTest::RRRunTest() {
  // Arrange
  QVector<double> signal;
  for (int i = 0; i < 100; i++)
    signal.push_back(i);

  QVector<CIterators> RRPeaksIterators;
  RRIntervalMethod a;
  for (QVector<double>::const_iterator iters = signal.begin();
       iters < signal.end(); iters += 10) {
    RRPeaksIterators.push_back(iters);
  }
  // Act
  a.RunRRMethod(begin(RRPeaksIterators), end(RRPeaksIterators));
  Matrix3_3 markovTable = a.getMarkovTable();
  Matrix3_3 ExpectedArray = { { { { 0, 0, 0 } }, { { 0, 1, 0 } },
                                { { 0, 0, 0 } } } };
  // Assert
  QVERIFY(markovTable == ExpectedArray);
}
void RRSanityTest::entropyBig() {
  // Arrange
  std::array<std::array<double, 3>, 3> arr = { { { { 0.11, 0.11, 0.11 } },
                                                 { { 0.11, 0.11, 0.11 } },
                                                 { { 0.11, 0.11, 0.11 } } } };

  // Assert
  QVERIFY(abs(entropy(arr) - 1.0) < 0.1);
}

void RRSanityTest::entropySmall() {
  // Arrange
  std::array<std::array<double, 3>, 3> arr = {
    { { { 0.0, 0.0, 0.0 } }, { { 0.0, 1.000, 0.0 } }, { { 0.0, 0.0, 0.0 } } }
  };

  // Assert
  QVERIFY(entropy(arr) < 0.1);
  QVERIFY(entropy(arr) > -0.1);
}

void RRSanityTest::KLDivergenceEqualMatrix() {
  // Arrange
  std::array<std::array<double, 3>, 3> pattern = {
    { { { 0.11, 0.11, 0.11 } }, { { 0.11, 0.11, 0.11 } },
      { { 0.11, 0.11, 0.11 } } }
  };
  std::array<std::array<double, 3>, 3> arr = pattern;
  // Assert
  QVERIFY(KLdivergence(arr, pattern) == 0);
}

void RRSanityTest::KLDivergenceTest() {
  std::array<std::array<double, 3>, 3> pattern = {
    { { { 0.11, 0.11, 0.11 } }, { { 0.11, 0.11, 0.11 } },
      { { 0.11, 0.11, 0.11 } } }
  };
  std::array<std::array<double, 3>, 3> arr = {
    { { { 0.001, 0.001, 0.001 } }, { { 0.001, 1.000, 0.001 } },
      { { 0.001, 0.001, 0.001 } } }
  };
  // Assert
  QVERIFY(KLdivergence(arr, pattern) > 3.8);
  QVERIFY(KLdivergence(arr, pattern) < 3.9);
}

void RRSanityTest::JKDivergenceEqualMatrix() {
  // Arrange
  std::array<std::array<double, 3>, 3> pattern = {
    { { { 0.11, 0.11, 0.11 } }, { { 0.11, 0.11, 0.11 } },
      { { 0.11, 0.11, 0.11 } } }
  };
  std::array<std::array<double, 3>, 3> arr = pattern;

  // Assert
  QVERIFY(JKdivergence(arr, pattern) == 0);
}

void RRSanityTest::JKDivergenceTest() {
  std::array<std::array<double, 3>, 3> pattern = {
    { { { 0.11, 0.11, 0.11 } }, { { 0.11, 0.11, 0.11 } },
      { { 0.11, 0.11, 0.11 } } }
  };
  std::array<std::array<double, 3>, 3> arr = {
    { { { 0.0, 0.0, 0.0 } }, { { 0.0, 1.000, 0.0 } }, { { 0.0, 0.0, 0.0 } } }
  };
  // Assert
  QVERIFY(JKdivergence(arr, pattern) > 0.51);
  QVERIFY(JKdivergence(arr, pattern) < 0.52);
}

void RRSanityTest::correlation_ObviousCases() {
  // Assert
  QCOMPARE(correlation({ 1, 2, 3, 4, 5 }, { 2, 4, 6, 8, 10 }), 1.0);
  QCOMPARE(correlation({ 1, 2, 3, 4, 5 }, { 5, 4, 3, 2, 1 }), -1.0);
}

void RRSanityTest::pWaveOccurence_AllFound() {
  // Arrange
  QVector<double> signal(200);
  QVector<QVector<double>::iterator> pWaveStarts = { signal.begin() + 10,
                                                     signal.begin() + 70 };
  QVector<QVector<double>::const_iterator> pWaveStartsC = {
    signal.begin() + 10, signal.begin() + 70
  };
  for (auto it : pWaveStarts)
    copy(begin(averagePWave), end(averagePWave), it);
  // Assert
  QCOMPARE(
      pWaveOccurenceRatio(begin(pWaveStartsC), end(pWaveStartsC), end(signal)),
      1.0);
}

void RRSanityTest::pWaveOccurence_HalfFound() {
  // Arrange
  QVector<double> signal(200);
  QVector<QVector<double>::const_iterator> pWaveStarts = {
    signal.begin() + 10, signal.begin() + 70
  };
  copy(begin(averagePWave), end(averagePWave), begin(signal) + 10);

  // Assert
  QCOMPARE(
      pWaveOccurenceRatio(begin(pWaveStarts), end(pWaveStarts), end(signal)),
      1.0 / 2);
}

void RRSanityTest::pWaveOccurence_ThrowIfPWaveStartTooCloseToEndOfSignal() {
  // Arrange
  QVector<double> signal(100);
  copy(begin(averagePWave), begin(averagePWave) + 10, begin(signal) + 90);
  QVector<QVector<double>::const_iterator> pWaveStarts = { signal.begin() +
                                                           90 };
  bool thrown = false;

  // Act
  try {
    pWaveOccurenceRatio(begin(pWaveStarts), end(pWaveStarts), end(signal));
  }
  catch (PWaveStartTooCloseToEndOfSignal) {
    thrown = true;
  }

  // Assert
  QVERIFY(thrown);
}

void RRSanityTest::closestP() {
  // Arrange
  QVector<double> signal = { 0.0, 0.0, 0.0 };
  Cit rpeak = signal.end() - 1;
  QVector<Cit> pPeaks = { signal.begin() + 1 };

  // Assert
  QCOMPARE(pPeaks.begin(), closestPWave(begin(pPeaks), end(pPeaks), rpeak));
}

void RRSanityTest::closestP_SinglePPeak() {
  // Arrange
  QVector<double> signal(4);
  Cit rpeak = signal.end() - 1;
  QVector<Cit> pPeaks = { signal.begin() + 1 };

  // Assert
  QCOMPARE(pPeaks.begin(), closestPWave(begin(pPeaks), end(pPeaks), rpeak));
}

void RRSanityTest::closestP_TwoPPeaks() {
  // Arrange
  QVector<double> signal(4);
  Cit rpeak = signal.end() - 1;
  QVector<Cit> pPeaks = { signal.begin() + 1, signal.begin() + 2 };

  // Assert
  QCOMPARE(pPeaks.end() - 1, closestPWave(begin(pPeaks), end(pPeaks), rpeak));
}

void RRSanityTest::calcRWaveSets_SingleR() {
  // Arrange
  QVector<Cit> rpeaks(140);

  // Act
  const auto answer = calcRWaveSets(begin(rpeaks), end(rpeaks), 60);

  // Assert
  QCOMPARE(2, answer.size());
  QCOMPARE(rpeaks.begin(), answer[0]);
  QCOMPARE(rpeaks.begin() + 60, answer[1]);
}

void RRSanityTest::calcSets_SingleR() {
  // Arrange
  QVector<double> signal(2000);
  QVector<Cit> rpeaks(140);
  QVector<Cit> ppeaks(140);
  for (int i = 0; i < 140; i++) {
    rpeaks[i] = signal.begin() + i * 10 + 10;
    ppeaks[i] = signal.begin() + i * 10 + 5;
  }

  // Act

  auto sets =
      calcSets(begin(ppeaks), end(ppeaks), begin(rpeaks), end(rpeaks), 60);

  // Assert
  QCOMPARE(2, sets.size());
  QCOMPARE(rpeaks.begin(), get<0>(sets[0]));
  QCOMPARE(ppeaks.begin(), get<1>(sets[0]));
  QCOMPARE(rpeaks.begin() + 60, get<0>(sets[1]));
  QCOMPARE(ppeaks.begin() + 60, get<1>(sets[1]));
}

QTEST_APPLESS_MAIN(RRSanityTest)

#include "AfTests.moc"
