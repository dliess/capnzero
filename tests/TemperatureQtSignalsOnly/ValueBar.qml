import QtQuick 2.14

Item{
    id: root
    property real actualValue
    property real commandedValue
    property color backgroundColor : "black"
    property color commandedValueColor : "blue"
    property color actualValueColor : "red"

    Rectangle {
        id: commandedValueLine
        anchors {top: parent.top; bottom: parent.bottom}
        color: root.commandedValueColor
        border.width: 0
        width: 3
        x: parent.width * root.commandedValue
    }

    Rectangle {
        anchors.centerIn: parent
        width: parent.width
        height: parent.height * 0.5
        border {width: 1; color: "white"}
        color: root.backgroundColor
        Rectangle {
            anchors {left: parent.left; top: parent.top; bottom: parent.bottom}
            width: parent.width * root.actualValue
            color : root.actualValueColor
        }
    }
}
