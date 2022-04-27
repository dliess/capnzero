
import QtQuick 2.14
import QtQuick.Window 2.14
import QtQuick.Controls 2.13

Item {
    width: 300
    height: 100
    Rectangle {
        anchors.fill: parent
        color: "white"
    }

    Text {
        x: 70
        y: 5
        font.family: "Helvetica"
        font.pointSize: 17
        text: backend.sensorName
    }

    ValueBar {
        x: 37
        y: 75
        width: 200
        height: 30
        actualValue : backend.actualTemperature
        commandedValue: backend.actualTemperature
    }
}