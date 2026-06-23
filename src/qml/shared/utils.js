function formatTimestamp(date) {
    return date.getFullYear() +
        ("0" + (date.getMonth() + 1)).slice(-2) +
        ("0" + date.getDate()).slice(-2) + "_" +
        ("0" + date.getHours()).slice(-2) +
        ("0" + date.getMinutes()).slice(-2) +
        ("0" + date.getSeconds()).slice(-2)
}

function openDialog(qmlFile, properties, onAccepted) {
    var component = Qt.createComponent(qmlFile)
    if (component.status === Component.Ready) {
        var win = component.createObject(null, properties || {})
        if (onAccepted) {
            win.accepted.connect(function() { onAccepted(win) })
        } else {
            win.accepted.connect(function() { win.close() })
        }
        win.rejected.connect(function() { win.close() })
        win.show()
    } else if (component.status === Component.Error) {
        console.error(qmlFile + " error:", component.errorString())
    }
}
