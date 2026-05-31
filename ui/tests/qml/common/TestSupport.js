.pragma library

function findRequired(lookup, root, objectName) {
    const found = lookup.findObject(root, objectName)
    if (!found)
        throw new Error("Missing object: " + objectName)
    return found
}

function createActorObject(testCase, source) {
    if (!source)
        return null

    const actorObject = Qt.createQmlObject('import QtQml 2.15; QtObject { signal changed(); property string id: ""; property string name: ""; property var aliases: []; property var contractIds: []; function setState(newId, newName, newAliases, newContractIds) { id = newId || ""; name = newName || ""; aliases = newAliases || []; contractIds = newContractIds || []; changed(); } }', testCase)
    actorObject["setState"](source.id || "", source.name || "", source.aliases || [], source.contractIds || [])
    return actorObject
}

function createContractObject(testCase, source) {
    if (!source)
        return null

    const contractObject = Qt.createQmlObject('import QtQml 2.15; QtObject { signal changed(); property string id: ""; property string name: ""; property string type: ""; property var actorIds: []; property var propertyIds: []; property var aliases: []; function setState(newId, newName, newType, newActorIds, newPropertyIds, newAliases) { id = newId || ""; name = newName || ""; type = newType || ""; actorIds = newActorIds || []; propertyIds = newPropertyIds || []; aliases = newAliases || []; changed(); } }', testCase)
    contractObject["setState"](source.id || "", source.name || "", source.type || "", source.actorIds || [], source.propertyIds || [], source.aliases || [])
    return contractObject
}
