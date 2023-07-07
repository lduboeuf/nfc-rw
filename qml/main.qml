import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtMultimedia 5.12

ApplicationWindow {
    id: root
    visible: true
    width: 640
    height: 480
    title: qsTr("NFC Tag reader writer")
    color: theme.backgroundColor

    Theme {
        id: theme
    }

    FontMetrics {
        id: fontMetrics
    }

    header: RowLayout {
        width: parent.width
        Label {
            Layout.fillWidth: true;
            text: "NFC Tag reader-writer" ;
            color: theme.textColor
            font.pixelSize: fontMetrics.font.pixelSize * 2
            horizontalAlignment: Text.AlignHCenter
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 24
        RowLayout {
            Layout.margins: 24
            spacing: 20

            CustomButton {
                id: cbBtn
                text: "Read Tag"
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                onClicked: {
                    busyIndicator.visible = true
                    nfcManager.startReading()
                }
            }

            CustomButton {
                text: "Save Tag"
                enabled: txtArea.text.length > 0
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                onClicked: {
                    busyIndicator.visible = true
                    nfcManager.saveRecord(txtArea.text)
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 24
            color: theme.secondaryBackgroundColor
            TextArea {
                id: txtArea
                color: theme.textColor
                verticalAlignment: TextEdit.AlignTop
                cursorPosition: 0
                selectionColor:theme.tintColor
                selectByMouse: true
                background: null
                placeholderTextColor: theme.textColor
                anchors.fill: parent
                placeholderText: qsTr("Enter text")
                font.pixelSize: fontMetrics.font.pixelSize * 1.2
                cursorDelegate: Rectangle {
                        color: "salmon"
                        width:2
                }
            }
        }
    }

    Rectangle {
        id: busyIndicator
        anchors.fill: parent
        color: "white"
        opacity: 0.8
        visible: false
        onVisibleChanged: {
            if (visible) {
                pulsatingImage.startAnimation()
            } else {
                pulsatingImage.stopAnimation()
            }
        }
        MouseArea {
            anchors.fill: parent
        }
        PulsatingImage {
          id: pulsatingImage
          width: parent.width * 0.5
          height: width

          anchors.centerIn: parent
          imageSource: "qrc:/assets/readTagGreen.png"
        }

        CustomButton {
            text: "Cancel"
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottomMargin: 24
            onClicked: {
                nfcManager.stopDetecting()
                busyIndicator.visible = false
            }
        }

    }

    Connections {
        target: nfcManager
        enabled: root.visible

        onRecordChanged: {
            console.log("onRecordChanged", record)
            readEffect.play()
            busyIndicator.visible = false
            txtArea.text = record
        }

        onNfcError: {
            busyIndicator.visible = false
            console.log("onNfcError", error)
        }

        onWroteSuccessfully: {
            saveEffect.play()
            busyIndicator.visible = false
            console.log("NFC Tag Saved Successfully")
        }
    }

    SoundEffect {
      id: readEffect
      source: "qrc:/assets/beepread.wav"
    }

    SoundEffect {
          id: saveEffect
          source: "qrc:/assets/beepsave.wav"
    }
}
