#include "NFCManager.h"
#include <QtNfc/qndefnfctextrecord.h>
#include <QtNfc/qndefrecord.h>
#include <QtNfc/qndefmessage.h>
#include <QtNfc/qnearfieldmanager.h>
#include <QtNfc/qnearfieldtarget.h>

#include <QDebug>

NFCManager::NFCManager(QObject *parent)
    : QObject(parent)
    , m_manager(new QNearFieldManager(this))
{
    connect(m_manager, &QNearFieldManager::targetDetected,
            this, &NFCManager::onTargetDetected);
    connect(m_manager, &QNearFieldManager::targetLost,
            this, &NFCManager::onTargetLost);

    qDebug() << "NFC available ?" << m_manager->isAvailable() << "supported ?" << m_manager->isSupported();
    if (!m_manager->isAvailable()) {
        Q_EMIT nfcError("NFC disabled");
    }
}

bool NFCManager::hasTagInRange() const
{
    return m_hasTagInRange;
}

NFCManager::ActionType NFCManager::actionType() const
{
    return m_actionType;
}

void NFCManager::setActionType(NFCManager::ActionType actionType)
{
    if (m_actionType == actionType) {
        return;
    }

    m_actionType = actionType;
    Q_EMIT actionTypeChanged(m_actionType);
}

void NFCManager::setHasTagInRange(bool hasTagInRange)
{
    if (m_hasTagInRange == hasTagInRange) {
        return;
    }

    m_hasTagInRange = hasTagInRange;
    Q_EMIT hasTagInRangeChanged(m_hasTagInRange);
}

void NFCManager::startReading()
{
    setActionType(ActionType::Reading);
    m_manager->setTargetAccessModes(QNearFieldManager::NdefReadTargetAccess);
    m_manager->startTargetDetection();
}

void NFCManager::stopDetecting()
{
    setActionType(ActionType::None);
    m_manager->setTargetAccessModes(QNearFieldManager::NoTargetAccess);
    m_manager->stopTargetDetection();
}

void NFCManager::saveRecord(const QString &record)
{
    m_record= record;

    setActionType(ActionType::Writing);
    m_manager->setTargetAccessModes(QNearFieldManager::NdefWriteTargetAccess);
    m_manager->startTargetDetection();
}

void NFCManager::onTargetDetected(QNearFieldTarget *target)
{
    setHasTagInRange(true);

    switch (m_actionType) {
    case None:
        break;
    case Reading:
        connect(target, &QNearFieldTarget::ndefMessageRead, this, &NFCManager::onNdefMessageRead);
        connect(target, &QNearFieldTarget::error, this, &NFCManager::handleTargetError);

        m_request = target->readNdefMessages();
        if (!m_request.isValid()) {
            handleTargetError(QNearFieldTarget::NdefReadError, m_request);
        }
        break;
    case Writing:
        connect(target, &QNearFieldTarget::ndefMessagesWritten, this, &NFCManager::onNdefMessageWritten);
        connect(target, &QNearFieldTarget::error, this, &NFCManager::handleTargetError);

        m_request = target->writeNdefMessages(QList<QNdefMessage>() << generateNdefMessage());
        if (!m_request.isValid()) {
            handleTargetError(QNearFieldTarget::NdefWriteError, m_request);
        }
        break;
    }
}

void NFCManager::onTargetLost(QNearFieldTarget *target)
{
    setHasTagInRange(false);
    target->deleteLater();
}

void NFCManager::onNdefMessageRead(const QNdefMessage &message)
{
    bool recordFound = false;
    for (const QNdefRecord &record : message) {
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            QNdefNfcTextRecord textRecord(record);
            m_record = textRecord.text().toUtf8();
            Q_EMIT recordChanged(m_record);
        }
    }

    stopDetecting();
    m_request = QNearFieldTarget::RequestId();

    if (recordFound) {
        Q_EMIT recordChanged(m_record);
    } else {
        Q_EMIT nfcError("Tag does not contain desired record or is malformed");
    }
}

void NFCManager::onNdefMessageWritten()
{
    stopDetecting();
    m_request = QNearFieldTarget::RequestId();

    Q_EMIT wroteSuccessfully();
}

void NFCManager::handleTargetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id)
{
    if (m_request == id) {

        QString errorMsg;

        switch (error) {
        case QNearFieldTarget::NoError:
            break;
        case QNearFieldTarget::UnsupportedError:
            errorMsg = "Unsupported tag";
            break;
        case QNearFieldTarget::TargetOutOfRangeError:
            errorMsg = "Tag removed from field";
            break;
        case QNearFieldTarget::NoResponseError:
            errorMsg = "No response from tag";
            break;
        case QNearFieldTarget::ChecksumMismatchError:
            errorMsg = "Checksum mismatch";
            break;
        case QNearFieldTarget::InvalidParametersError:
            errorMsg = "Invalid parameters";
            break;
        case QNearFieldTarget::NdefReadError:
            errorMsg = "NDEF read error";
            break;
        case QNearFieldTarget::NdefWriteError:
            errorMsg = "NDEF write error";
            break;
        default:
            errorMsg = "Unknown error";
        }

        qDebug() << "error" << errorMsg;

        m_manager->setTargetAccessModes(QNearFieldManager::NoTargetAccess);
        m_manager->stopTargetDetection();
        m_request = QNearFieldTarget::RequestId();

        if (!errorMsg.isEmpty()) {
            Q_EMIT nfcError(errorMsg);
        }
    }
}

QString NFCManager::record() const
{
    return m_record;
}

QNdefMessage NFCManager::generateNdefMessage() const
{
    if (m_record.isEmpty()) {
        return QNdefMessage();
    }

    QNdefMessage message;

    QNdefNfcTextRecord record;
    record.setEncoding(QNdefNfcTextRecord::Utf8);
    record.setText(m_record);

    message.append(record);

    return message;
}
