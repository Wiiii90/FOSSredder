.pragma library

function findObject(root, objectName) {
    return findObjectInternal(root, objectName, [])
}

function findObjectInternal(root, objectName, visited) {
    if (!root)
        return null

    if (visited.indexOf(root) !== -1)
        return null
    visited.push(root)

    if (root.objectName === objectName)
        return root

    var specialChildren = [root.contentItem, root.background, root.indicator, root.popup]
    for (var i = 0; i < specialChildren.length; ++i) {
        var special = specialChildren[i]
        if (!special)
            continue
        var specialMatch = findObjectInternal(special, objectName, visited)
        if (specialMatch)
            return specialMatch
    }

    if (root.data) {
        for (var dataIndex = 0; dataIndex < root.data.length; ++dataIndex) {
            var dataMatch = findObjectInternal(root.data[dataIndex], objectName, visited)
            if (dataMatch)
                return dataMatch
        }
    }

    if (!root.children)
        return null

    for (var childIndex = 0; childIndex < root.children.length; ++childIndex) {
        var match = findObjectInternal(root.children[childIndex], objectName, visited)
        if (match)
            return match
    }

    return null
}
