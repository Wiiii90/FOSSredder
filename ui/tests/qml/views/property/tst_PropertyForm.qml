/**
 * @file ui/tests/qml/views/property/tst_PropertyForm.qml
 * @brief Provides QML tests for PropertyForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Property 1.0

import "../../common/Lookup.js" as Lookup

TestCase {
    id: testCase
    name: "PropertyFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedProperty: null
        property string selectedPropertyId: ""
        property int dataRevision: 0
        property var properties: []
        property var contracts: []

        function basicFormState(name, aliases, selectedIds) {
            const aliasValues = aliases || []
            return {
                name: name || "",
                aliases: aliasValues,
                aliasInputText: "",
                aliasIndex: aliasValues.length > 0 ? 0 : -1,
                selectedIds: selectedIds || []
            }
        }

        function normalizeStrings(values) {
            const out = []
            if (!values)
                return out
            for (let i = 0; i < values.length; ++i)
                out.push(String(values[i]))
            return out
        }

        function addUniqueTrimmed(values, value) {
            const out = values ? values.slice(0) : []
            const next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }

        function removeAt(values, index) {
            const out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }

        function removeString(values, value) {
            const out = values ? values.slice(0) : []
            const target = String(value || "")
            const index = out.indexOf(target)
            if (index < 0)
                return out
            out.splice(index, 1)
            return out
        }

        function contractRows() {
            return contracts || []
        }

        function indexOfId(rows, id) {
            const list = rows || []
            const target = String(id || "")
            for (let i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === target)
                    return i
            }
            return -1
        }

        function navigatedId(rows, currentId, delta, fallbackIndex) {
            if (!rows || rows.length === 0)
                return ""
            let index = -1
            for (let i = 0; i < rows.length; ++i) {
                if (String(rows[i].id || "") === String(currentId || "")) {
                    index = i
                    break
                }
            }
            if (index < 0) {
                index = delta > 0 ? 0 : (delta < 0 ? rows.length - 1 : fallbackIndex)
                return String(rows[index].id || "")
            }
            if (delta > 0)
                return index >= rows.length - 1 ? "" : String(rows[index + 1].id || "")
            if (delta < 0)
                return index <= 0 ? "" : String(rows[index - 1].id || "")
            return String(rows[index].id || "")
        }

        function deleteNextSelectionId(rows, removedId, fallbackIndex, key) {
            if (!rows || rows.length === 0)
                return ""
            const kept = []
            for (let i = 0; i < rows.length; ++i) {
                const rowId = String(rows[i][key] || "")
                if (rowId !== String(removedId || ""))
                    kept.push(rows[i])
            }
            if (kept.length === 0)
                return ""
            const idx = Math.max(0, Math.min(fallbackIndex, kept.length - 1))
            return String(kept[idx][key] || "")
        }

        function selectPropertyState(newId, newName, newAliases, newContractIds) {
            if (selectedProperty && selectedProperty["setState"])
                selectedProperty["setState"](newId, newName, newAliases, newContractIds)
            selectedPropertyId = String(newId || "")
            if (testCase.propertyViewModel && testCase.propertyViewModel["setState"])
                testCase.propertyViewModel["setState"](newId, newName, newAliases, newContractIds)
        }
    }

    property var propertyViewModel: QtObject {
        signal changed()
        property string currentId: ""
        property string name: ""
        property var aliases: []
        property string aliasInputText: ""
        property int aliasIndex: -1
        property var selectedContractIds: []
        property string savedName: ""
        property var savedAliases: []
        property var savedSelectedContractIds: []
        property bool isEdit: false
        property bool hasChanges: false
        readonly property bool canSubmit: String(name || "").trim().length > 0
        readonly property var propertyRows: testCase.propertyController.propertyRows
        readonly property var contractRows: testCase.propertyController.contractRows
        function setState(newId, newName, newAliases, newContractIds) {
            currentId = String(newId || "")
            name = String(newName || "")
            aliases = (newAliases || []).slice(0)
            aliasInputText = ""
            aliasIndex = aliases.length > 0 ? 0 : -1
            selectedContractIds = (newContractIds || []).slice(0)
            savedName = name
            savedAliases = aliases.slice(0)
            savedSelectedContractIds = selectedContractIds.slice(0)
            isEdit = currentId.length > 0
            hasChanges = false
            changed()
        }
        function canAddAlias(value) { return String(value || "").trim().length > 0 }
        function clear() {
            currentId = ""
            name = ""
            aliases = []
            aliasInputText = ""
            aliasIndex = -1
            selectedContractIds = []
            savedName = ""
            savedAliases = []
            savedSelectedContractIds = []
            isEdit = false
            hasChanges = false
            changed()
        }
        function enterCreateMode() {
            testCase.session.selectedPropertyId = ""
            testCase.session.selectedProperty = null
            clear()
        }
        function addAlias(value) {
            const next = testCase.session.addUniqueTrimmed(aliases, value)
            if (next.length === aliases.length)
                return
            aliases = next
            aliasIndex = next.length - 1
            aliasInputText = ""
            hasChanges = true
            changed()
        }
        function requestRemoveSelectedAlias() {
            if (aliasIndex < 0 || aliasIndex >= aliases.length)
                return
            const next = testCase.session.removeAt(aliases, aliasIndex)
            if (next.length === aliases.length)
                return
            aliases = next
            aliasIndex = next.length > 0 ? Math.min(aliasIndex, next.length - 1) : -1
            hasChanges = true
            changed()
        }
        function setContractSelected(contractId, selected) {
            const next = selected ? testCase.session.addUniqueTrimmed(selectedContractIds, contractId) : testCase.session.removeString(selectedContractIds, contractId)
            if (next.length === selectedContractIds.length)
                return
            selectedContractIds = next
            hasChanges = true
            changed()
        }
        function previous() {
            const rows = testCase.propertyController.propertyRows || []
            if (rows.length === 0)
                return
            const nextId = testCase.session.navigatedId(rows, isEdit ? currentId : "", -1, rows.length - 1)
            testCase.session.selectedPropertyId = nextId
            setStateFromRows(nextId)
        }
        function next() {
            const rows = testCase.propertyController.propertyRows || []
            if (rows.length === 0)
                return
            const nextId = testCase.session.navigatedId(rows, isEdit ? currentId : "", 1, 0)
            testCase.session.selectedPropertyId = nextId
            setStateFromRows(nextId)
        }
        function setStateFromRows(id) {
            const rows = testCase.propertyController.propertyRows || []
            for (let i = 0; i < rows.length; ++i) {
                if (String(rows[i].id || "") === String(id || "")) {
                    setState(rows[i].id || "", rows[i].name || "", rows[i].aliases || [], rows[i].contractIds || [])
                    return
                }
            }
            clear()
        }
        function submit() {
            const id = testCase.propertyController.saveProperty(isEdit ? currentId : "",
                                                                name,
                                                                aliases || [],
                                                                selectedContractIds || [])
            setState(id && id.length > 0 ? id : currentId, name, aliases || [], selectedContractIds || [])
            testCase.session.selectedPropertyId = currentId
            return currentId
        }
        function deleteCurrent() {
            if (currentId.length === 0)
                return
            testCase.propertyController.deleteProperty(currentId)
        }
    }

    property var propertyController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""
        property var savePropertyOverride: null
        property var propertyViewModel: testCase.propertyViewModel
        property var propertyRows: []
        property var contractRows: []

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
            savePropertyOverride = null
        }

        function defaultSaveProperty(id, name, aliases, contractIds) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            }
            return id && id.length > 0 ? id : "property-new"
        }

        function saveProperty(id, name, aliases, contractIds) {
            const override = savePropertyOverride
            if (override)
                return override(id, name, aliases, contractIds)
            return defaultSaveProperty(id, name, aliases, contractIds)
        }

        function normalizeStrings(values) {
            const out = []
            if (!values)
                return out
            for (let i = 0; i < values.length; ++i)
                out.push(String(values[i]))
            return out
        }

        function addUniqueTrimmed(values, value) {
            const out = values ? values.slice(0) : []
            const next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }

        function removeAt(values, index) {
            const out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }

        function deleteProperty(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: propertyFormComponent

        PropertyView {
            width: 960
            height: 640
            propertyViewModel: testCase.propertyViewModel
            theme: testCase.theme
        }
    }

    Component {
        id: propertyObjectComponent

        QtObject {
            signal changed()
            property string id: ""
            property string name: ""
            property var aliases: []
            property var contractIds: []

            function setState(newId, newName, newAliases, newContractIds) {
                id = newId || ""
                name = newName || ""
                aliases = newAliases || []
                contractIds = newContractIds || []
                changed()
            }
        }
    }

    function findRequired(root, objectName) {
        const match = Lookup.findObject(root, objectName)
        verify(match !== null, "Missing object: " + objectName)
        return match
    }

    function createPropertyObject(source) {
        if (!source)
            return null
        const propertyObject = createTemporaryObject(propertyObjectComponent, testCase)
        propertyObject.setState(source.id || "", source.name || "", source.aliases || [], source.contractIds || [])
        return propertyObject
    }

    function createForm(selectedProperty) {
        const propertyObject = createPropertyObject(selectedProperty)
        session.selectedProperty = propertyObject
        session.selectedPropertyId = propertyObject ? propertyObject.id : ""
        propertyController.propertyRows = session.properties || []
        propertyController.contractRows = session.contractRows()
        propertyViewModel["setState"](propertyObject ? propertyObject.id : "",
                                  propertyObject ? propertyObject.name : "",
                                  propertyObject ? propertyObject.aliases : [],
                                  propertyObject ? propertyObject.contractIds : [])
        return createTemporaryObject(propertyFormComponent, testCase)
    }

    function init() {
        propertyController.reset()
        propertyController.propertyRows = []
        propertyController.contractRows = []
        session.selectedProperty = null
        session.selectedPropertyId = ""
        session.properties = []
        session.contracts = []
    }

    function test_PROP_F_001_createModeSavesPropertyAndSelectsNewId() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        const form = createForm(null)
        const nameField = findRequired(form, "propertyNameField")
        const createButton = findRequired(form, "propertyCreateButton")
        const aliasInput = findRequired(form, "propertyAliasInput")
        const addAliasButton = findRequired(form, "propertyAddAliasButton")
        const checkBox = findRequired(form, "propertyContractCheckBox")

        nameField.text = "Flat 12"
        aliasInput.text = "Alias One"
        addAliasButton.clicked()
        checkBox.checked = true
        checkBox.toggled()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        propertyController.savePropertyOverride = function(id, name, aliases, contractIds) {
            const result = propertyController.defaultSaveProperty(id, name, aliases, contractIds)
            session.selectPropertyState(result, name, aliases, contractIds)
            session.dataRevision += 1
            return result
        }
        try {
            createButton.clicked()
        } finally {
            propertyController.savePropertyOverride = null
        }

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "")
        compare(propertyController.lastSave.name, "Flat 12")
        compare(propertyController.lastSave.aliases.length, 1)
        compare(propertyController.lastSave.aliases[0], "Alias One")
        compare(propertyController.lastSave.contractIds.length, 1)
        compare(propertyController.lastSave.contractIds[0], "contract-1")
        compare(session.selectedPropertyId, "property-new")

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        session.selectPropertyState("property-other", "Other", ["Other"], [])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectPropertyState("property-new", "Flat 12", ["Alias One"], ["contract-1"])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_PROP_F_005_aliasButtonsAddAndRemoveAlias() {
        const form = createForm(null)
        const aliasInput = findRequired(form, "propertyAliasInput")
        const addAliasButton = findRequired(form, "propertyAddAliasButton")
        const removeAliasButton = findRequired(form, "propertyRemoveAliasButton")

        aliasInput.text = "A1"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "A1")

        removeAliasButton.clicked()
        compare(form.aliases.length, 0)
    }

    function test_PROP_F_005B_editModeAliasButtonAddsAliasToFormState() {
        const form = createForm({ id: "property-4", name: "Flat", aliases: ["Base"] })
        const aliasInput = findRequired(form, "propertyAliasInput")
        const addAliasButton = findRequired(form, "propertyAddAliasButton")

        aliasInput.text = "Alias Two"
        compare(aliasInput.text, "Alias Two")
        compare(addAliasButton.enabled, true)
        compare(addAliasButton.width > 0, true)
        compare(addAliasButton.height > 0, true)
        addAliasButton.clicked()

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")
    }

    function test_PROP_F_005C_aliasPanelGetsLayoutSpaceForRenderedAliases() {
        const form = createForm({ id: "property-4", name: "Flat", aliases: ["Base"] })
        const aliasScroll = findRequired(form, "propertyAliasScroll")

        compare(aliasScroll.width > 0, true)
        compare(aliasScroll.height > 0, true)
    }

    function test_PROP_F_005D_aliasChipClickSelectsAliasForRemoval() {
        const form = createForm({ id: "property-4", name: "Flat", aliases: ["Base", "Alias Two"] })
        const aliasMouse = findRequired(form, "propertyAliasMouse_1")
        const removeAliasButton = findRequired(form, "propertyRemoveAliasButton")

        aliasMouse.clicked(null)

        compare(form.aliasIndex, 1)
        compare(removeAliasButton.enabled, true)
    }

    function test_PROP_F_002_readModeLoadsSelectedPropertyState() {
        const form = createForm({ id: "property-3", name: "House", aliases: ["H1", "H2"] })
        const nameField = findRequired(form, "propertyNameField")

        compare(form.isEdit, true)
        compare(nameField.text, "House")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "H1")
        compare(form.aliases[1], "H2")
    }

    function test_PROP_F_006_updateModeSavesCurrentPropertyId() {
        const form = createForm({ id: "property-5", name: "Old", aliases: ["One"] })
        const nameField = findRequired(form, "propertyNameField")
        const updateButton = findRequired(form, "propertyUpdateButton")

        nameField.text = "New Property"
        updateButton.clicked()

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "property-5")
        compare(propertyController.lastSave.name, "New Property")
    }

    function test_PROP_F_008_updateModeKeepsSelectedContractsAndSavesThem() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        const form = createForm({ id: "property-6", name: "Old", aliases: ["One"], contractIds: [] })
        const nameField = findRequired(form, "propertyNameField")
        const updateButton = findRequired(form, "propertyUpdateButton")
        const aliasInput = findRequired(form, "propertyAliasInput")
        const addAliasButton = findRequired(form, "propertyAddAliasButton")
        const checkBox = findRequired(form, "propertyContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()
        aliasInput.text = "Alias Two"
        addAliasButton.clicked()
        compare(form.aliases.length, 2)
        compare(form.aliases[1], "Alias Two")
        nameField.text = "New Property"
        propertyController.savePropertyOverride = function(id, name, aliases, contractIds) {
            const result = propertyController.defaultSaveProperty(id, name, aliases, contractIds)
            session.selectPropertyState(id || "property-6", name, aliases, contractIds)
            session.dataRevision += 1
            return result
        }
        try {
            updateButton.clicked()
        } finally {
            propertyController.savePropertyOverride = null
        }

        compare(propertyController.saveCalls, 1)
        compare(propertyController.lastSave.id, "property-6")
        compare(propertyController.lastSave.name, "New Property")
        compare(propertyController.lastSave.aliases.length, 2)
        compare(propertyController.lastSave.aliases[0], "One")
        compare(propertyController.lastSave.aliases[1], "Alias Two")
        compare(propertyController.lastSave.contractIds.length, 1)
        compare(propertyController.lastSave.contractIds[0], "contract-1")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectPropertyState("property-7", "Other", ["Other"], [])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectPropertyState("property-6", "New Property", ["Property Alias", "Property Fresh Alias"], ["contract-1"])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Property Alias")
        compare(form.aliases[1], "Property Fresh Alias")

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_PROP_F_010_selectionChangedSignalRefreshesAliasesWithoutReplacingSelectionObject() {
        const form = createForm({ id: "property-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectPropertyState("property-6", "Old", ["One", "Alias Two"], [])
        session.dataRevision += 1
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectPropertyState("property-7", "Other", ["Other"], [])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectPropertyState("property-6", "Old", ["One", "Alias Two"], [])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
    }

    function test_PROP_F_011_selectionRefreshesAliasesWhenSelectionObjectDoesNotEmitChanged() {
        const form = createForm({ id: "property-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectPropertyState("property-6", "Old", ["One", "Alias Two"], ["contract-1"])

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_PROP_F_007_contractSelectionUpdatesSelectedIds() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        const form = createForm(null)
        const checkBox = findRequired(form, "propertyContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
        compare(propertyViewModel.hasChanges, true)
    }

    function test_PROP_F_012_navigationButtonsMoveSelectionId() {
        session.properties = [
            { id: "property-1", name: "A" },
            { id: "property-2", name: "B" }
        ]
        const form = createForm({ id: "property-1", name: "A", aliases: [] })
        const nextButton = findRequired(form, "propertyNextButton")
        const previousButton = findRequired(form, "propertyPreviousButton")

        nextButton.clicked()
        compare(session.selectedPropertyId, "property-2")

        previousButton.clicked()
        compare(session.selectedPropertyId, "property-1")
    }

    function test_PROP_F_014_deleteButtonDeletesCurrentProperty() {
        const form = createForm({ id: "property-7", name: "Lot", aliases: [] })
        const deleteButton = findRequired(form, "propertyDeleteButton")

        deleteButton.clicked()

        compare(propertyController.deleteCalls, 1)
        compare(propertyController.lastDeleteId, "property-7")
    }
}
