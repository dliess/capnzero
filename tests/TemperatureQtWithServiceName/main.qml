
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

    Slider {
        id: slider1
        x: 37
        y: 25
        value: 0.8
        Binding {
            target: backend
            property: "commandedTemperature"
            value: slider1.value
        }
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