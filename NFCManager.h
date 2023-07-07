#ifndef NFCMANAGER_H
#define NFCMANAGER_H

#include <QObject>
#include <qqml.h>
#include <QtNfc/qnearfieldtarget.h>

class QNearFieldManager;
class QNdefMessage;
class QNdefNfcTextRecord;

class NFCManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasTagInRange READ hasTagInRange NOTIFY hasTagInRangeChanged)
    Q_PROPERTY(ActionType actionType READ actionType WRITE setActionType NOTIFY actionTypeChanged)
    Q_PROPERTY(QString record READ record NOTIFY recordChanged)

public:
    explicit NFCManager(QObject *parent = nullptr);

    enum ActionType
    {
        None = 0,
        Reading,
        Writing
    };
    Q_ENUM(ActionType)

    bool hasTagInRange() const;
    ActionType actionType() const;
    QString record() const;
    Q_INVOKABLE void saveRecord(const QString &record);
    QNdefMessage generateNdefMessage() const;

public Q_SLOTS:
    void startReading();
    void stopDetecting();


Q_SIGNALS:
    void hasTagInRangeChanged(bool hasTagInRange);
    void actionTypeChanged(ActionType actionType);
    void recordChanged(const QString &record);

    void wroteSuccessfully();
    void nfcError(const QString &error);

private Q_SLOTS:
    void setActionType(ActionType actionType);
    void setHasTagInRange(bool hasTagInRange);

    void onTargetDetected(QNearFieldTarget *target);
    void onTargetLost(QNearFieldTarget *target);

    void onNdefMessageRead(const QNdefMessage &message);
    void onNdefMessageWritten();
    void handleTargetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

private:
    bool m_hasTagInRange = false;
    ActionType m_actionType = ActionType::None;

    QString m_record;
    QNearFieldManager *m_manager;
    QNearFieldTarget::RequestId m_request;
};

#endif // NFCMANAGER_H
