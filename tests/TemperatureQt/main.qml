
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

    Slider {
        id: slider1
        x: 37
        y: 5
        value: 0.8
        Binding {
            target: backend
            property: "commandedTemperature"
            value: slider1.value
        }
    }

    ValueBar {
        x: 37
        y: 55
        width: 200
        height: 30
        actualValue : backend.actualTemperature
        commandedValue: backend.actualTemperature
    }
}