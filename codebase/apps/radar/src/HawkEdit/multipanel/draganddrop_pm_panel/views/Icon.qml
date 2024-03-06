// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Rectangle {
    id: icon
    required property Item dragParent

    property int visualIndex: 0
    width: 72
    height: 72
    anchors {
        horizontalCenter: parent.horizontalCenter
        verticalCenter: parent.verticalCenter
    }
    radius: 3
    Column {
    Rectangle {
        color: "white"
        width: 20
        height: 20

    MouseArea {
        anchors.fill: parent
        onClicked: { parent.color = 'green' }
    }
    Item {
        Text {
            anchors.centerIn: parent
            color: "black"
            //text: "+"
            text: parent.visualIndex
        }
    }
    Rectangle {
        color: "yellow"
        width: 20
        height: 20
      
        MouseArea {
            anchors.fill: parent
            onClicked: { 
                focus: true
                //colorModel.remove(3,1)  // works
                console.log("colorModel.count:" + colorModel.count) 
                console.log("root.currentIndex:" + root.currentIndex) 
                console.log("icon.visualIndex:" + icon.visualIndex) 
                colorModel.remove(icon.visualIndex, 1)
                //colorModel.remove(parent.visualIndex, 1)
                // parent.color = 'red' 
            }
        }
        Item {
            Text { text: "-" }
//            focus: true
//            Keys.onPressed: (event)=> {
//                if (event.key == Qt.Key_Left) {
//                    console.log("move left");
//                    event.accepted = true;
//                }
 //           }
//            Keys.onReturnPressed: console.log("Pressed return");
        }
    }
    }
    } // Column

    //Image {
    //    source: "images/qt-logo.png"
        // https://doc.qt.io/qt-5/qquickimageprovider.html#details
    //}
    Image { source: "image://colors/yellow" }
    Image { source: "image://colors/red" }


    DragHandler {
        id: dragHandler
    }

    Drag.active: dragHandler.active
    Drag.source: icon
    Drag.hotSpot.x: 36
    Drag.hotSpot.y: 36

    states: [
        State {
            when: dragHandler.active
            ParentChange {
                target: icon
                parent: icon.dragParent
            }

            AnchorChanges {
                target: icon
                anchors {
                    horizontalCenter: undefined
                    verticalCenter: undefined
                }
            }
        }
    ]
}
