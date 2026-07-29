#include "axesmodel.h"
#include "buttonmodel.h"
#include "hatmodel.h"

#include <QAbstractItemModelTester>
#include <QStandardItemModel>
#include <QTest>

class GameControllerModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptyModelsRespectModelContract()
    {
        AxesModel axes;
        ButtonModel buttons;
        HatModel hats;
        QAbstractItemModelTester axesTester(&axes, QAbstractItemModelTester::FailureReportingMode::Fatal);
        QAbstractItemModelTester buttonsTester(&buttons, QAbstractItemModelTester::FailureReportingMode::Fatal);
        QAbstractItemModelTester hatsTester(&hats, QAbstractItemModelTester::FailureReportingMode::Fatal);

        QStandardItemModel foreignModel(1, 1);
        const QModelIndex parent = foreignModel.index(0, 0);
        QCOMPARE(axes.rowCount(parent), 0);
        QCOMPARE(buttons.rowCount(parent), 0);
        QCOMPARE(hats.rowCount(parent), 0);
        QCOMPARE(axes.columnCount(parent), 0);
        QCOMPARE(buttons.columnCount(parent), 0);
        QCOMPARE(hats.columnCount(parent), 0);
    }
};

QTEST_MAIN(GameControllerModelTest)

#include "modeltest.moc"
