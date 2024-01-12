/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CUBIC_CURVE_H_
#define _KIS_CUBIC_CURVE_H_

#include <QList>
#include <QPointF>
#include <QQmlListProperty>
#include <QVariant>
#include <QVector>

class QPointF;

/**
 * Hold the data for a cubic curve.
 */
class KisCubicCurve : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QPointF> points READ points CONSTANT)
public:
    KisCubicCurve();
    KisCubicCurve(const QList<QPointF> &points);
    KisCubicCurve(const QString &curveString);
    KisCubicCurve(const KisCubicCurve &curve);
    ~KisCubicCurve();
    KisCubicCurve &operator=(const KisCubicCurve &curve);
    bool operator==(const KisCubicCurve &curve) const;

    Q_INVOKABLE qreal value(qreal x) const;
    Q_INVOKABLE const QList<QPointF> &points() const;
    void setPoints(const QList<QPointF> &points);
    Q_INVOKABLE void setPoint(int idx, const QPointF &point);
    // QQmlListProperty<QPointF> pointsListProperty();
    /**
     * Add a point to the curve, the list of point is always sorted.
     * @return the index of the inserted point
     */
    Q_INVOKABLE int addPoint(const QPointF &point);
    void removePoint(int idx);

    /*
     * Check whether the curve maps all values to themselves.
     */
    bool isIdentity() const;

    /*
     * Check whether the curve maps all values to given constant.
     */
    bool isConstant(qreal c) const;

    /**
     * This allows us to carry around a display name for the curve internally. It is used
     * currently in Sketch for perchannel, but would potentially be useful anywhere
     * curves are used in the UI
     */
    void setName(const QString &name);
    const QString &name() const;

    static qreal interpolateLinear(qreal normalizedValue, const QVector<qreal> &transfer);

    const QVector<quint16> uint16Transfer(int size = 256) const;
    const QVector<qreal> floatTransfer(int size = 256) const;

    QString toString() const;
    Q_DECL_DEPRECATED void fromString(const QString &);

private:
    struct Data;
    struct Private;
    Private *const d{nullptr};
};

#endif
