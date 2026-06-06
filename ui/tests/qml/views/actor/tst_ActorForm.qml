/**
 * @file ui/tests/qml/views/actor/tst_ActorForm.qml
 * @brief Provides QML tests for ActorForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import "../../common" as Common
import FossRedder.Views.Actor 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ActorFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedActor: null
        property string selectedActorId: ""
        property int dataRevision: 0
        property var actors: []
        property var contracts: []
        property var actorRows: actors
        property var contractRows: contracts
        onSelectedActorIdChanged: {
            if (selectedActorId.length === 0) {
                selectedActor = null
                if (testCase.actorViewModel)
                    testCase.actorViewModel.reloadFakeFormFromSelection(false)
                return
            }
            if (selectedActor && String(selectedActor.id || "") === selectedActorId) {
                if (testCase.actorViewModel)
                    testCase.actorViewModel.reloadFakeFormFromSelection(false)
                return
            }

            const list = actors || []
            for (let i = 0; i < list.length; ++i) {
                const row = list[i] || ({})
                if (String(row.id || "") === selectedActorId) {
                    selectedActor = {
                        id: String(row.id || ""),
                        name: String(row.name || ""),
                        aliases: row.aliases || [],
                        contractIds: row.contractIds || []
                    }
                    if (testCase.actorViewModel)
                        testCase.actorViewModel.reloadFakeFormFromSelection(false)
                    return
                }
            }

            selectedActor = {
                id: selectedActorId,
                name: "",
                aliases: [],
                contractIds: []
            }
            if (testCase.actorViewModel)
                testCase.actorViewModel.reloadFakeFormFromSelection(false)
        }
        onDataRevisionChanged: if (testCase.actorViewModel) testCase.actorViewModel.reloadFakeFormFromSelection(true)
        onSelectedActorChanged: if (testCase.actorViewModel) testCase.actorViewModel.reloadFakeFormFromSelection(true)

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
            for (let i = out.length - 1; i >= 0; --i) {
                if (String(out[i] || "") === target)
                    out.splice(i, 1)
            }
            return out
        }

        function normalizedStringListKey(values) {
            const list = values ? values.slice(0) : []
            list.sort()
            return JSON.stringify(list.map(function(item) { return String(item || "") }))
        }

        function formStateChanged(savedName, savedAliases, savedContractIds, currentName, aliases, contractIds) {
            return String(savedName || "") !== String(currentName || "")
                    || normalizedStringListKey(savedAliases) !== normalizedStringListKey(aliases)
                    || normalizedStringListKey(savedContractIds) !== normalizedStringListKey(contractIds)
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
                index = fallbackIndex
                return String(rows[index].id || "")
            }
            if (delta > 0)
                return index >= rows.length - 1 ? "" : String(rows[index + 1].id || "")
            if (delta < 0)
                return index <= 0 ? "" : String(rows[index - 1].id || "")
            return String(rows[index].id || "")
        }

        function navigatedSelectionId(rows, currentId, delta, fallbackIndex, key) {
            const list = rows || []
            if (list.length === 0)
                return ""
            const idKey = key || "id"
            let currentIndex = -1
            for (let i = 0; i < list.length; ++i) {
                if (String(list[i][idKey] || "") === String(currentId || "")) {
                    currentIndex = i
                    break
                }
            }
            if (currentIndex < 0) {
                if (delta > 0)
                    return String(list[0][idKey] || "")
                if (delta < 0)
                    return String(list[list.length - 1][idKey] || "")
                const fallback = Math.max(0, Math.min(fallbackIndex, list.length - 1))
                return String(list[fallback][idKey] || "")
            }
            if ((delta > 0 && currentIndex === list.length - 1)
                    || (delta < 0 && currentIndex === 0))
                return ""
            const nextIndex = currentIndex + delta
            if (nextIndex < 0 || nextIndex >= list.length)
                return ""
            return String(list[nextIndex][idKey] || "")
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

        function selectActorState(newId, newName, newAliases, newContractIds) {
            if (selectedActor && selectedActor["setState"])
                selectedActor["setState"](newId, newName, newAliases, newContractIds)
            selectedActorId = String(newId || "")
        }
    }

    property var actorController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""
        property var saveActorOverride: null

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
            saveActorOverride = null
        }

        function defaultSaveActor(id, name, aliases, contractIds) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            }
            return id && id.length > 0 ? id : "actor-new"
        }

        function saveActor(id, name, aliases, contractIds) {
            const override = saveActorOverride
            if (override)
                return override.call(null, id, name, aliases, contractIds)
            return defaultSaveActor(id, name, aliases, contractIds)
        }

        function deleteActor(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var actorViewModel: QtObject {
        property string currentOwnerId: ""
        property string name: ""
        property var aliases: []
        property string aliasInputText: ""
        property int aliasIndex: -1
        property var selectedContractIds: []
        property string savedName: ""
        property var savedAliases: []
        property var savedSelectedContractIds: []
        readonly property bool isEdit: testCase.session.selectedActorId.length > 0
        readonly property bool hasChanges: !isEdit
                ? name.trim().length > 0
                : testCase.session.formStateChanged(savedName,
                                                    savedAliases,
                                                    savedSelectedContractIds,
                                                    name,
                                                    aliases,
                                                    selectedContractIds)
        readonly property bool canSubmit: name.trim().length > 0
        readonly property var actorRows: testCase.session.actorRows
        readonly property var contractRows: testCase.session.contractRows

        function canAddAlias(value) {
            return String(value || "").trim().length > 0
        }

        function clearFormState() {
            name = ""
            aliases = []
            aliasInputText = ""
            aliasIndex = -1
            selectedContractIds = []
        }

        function captureSavedState() {
            savedName = name
            savedAliases = aliases.slice(0)
            savedSelectedContractIds = selectedContractIds.slice(0)
        }

        function reloadFakeFormFromSelection(forceReload) {
            const currentId = testCase.session.selectedActorId || ""
            if (!forceReload && currentOwnerId === currentId)
                return
            currentOwnerId = currentId
            if (currentId.length === 0) {
                clearFormState()
                captureSavedState()
                return
            }
            const current = testCase.session.selectedActor
            if (!current) {
                clearFormState()
                captureSavedState()
                return
            }
            const state = testCase.session.basicFormState(current.name || "",
                                                          current.aliases || [],
                                                          current.contractIds || [])
            name = state.name || ""
            aliases = state.aliases || []
            aliasInputText = state.aliasInputText || ""
            aliasIndex = state.aliasIndex !== undefined ? state.aliasIndex : -1
            selectedContractIds = state.selectedIds || []
            captureSavedState()
        }

        function addAlias(value) {
            const next = testCase.session.addUniqueTrimmed(aliases || [], value || "")
            if (next.length === aliases.length)
                return
            aliases = next
            aliasIndex = next.length - 1
            aliasInputText = ""
        }

        function requestRemoveSelectedAlias() {
            if (aliasIndex < 0 || aliasIndex >= aliases.length)
                return
            const next = testCase.session.removeAt(aliases || [], aliasIndex)
            if (next.length === aliases.length)
                return
            aliases = next
            aliasIndex = next.length > 0 ? Math.min(aliasIndex, next.length - 1) : -1
        }

        function setContractSelected(contractId, selected) {
            const next = selected
                    ? testCase.session.addUniqueTrimmed(selectedContractIds || [], contractId)
                    : testCase.session.removeString(selectedContractIds || [], contractId)
            if (JSON.stringify(next) === JSON.stringify(selectedContractIds))
                return
            selectedContractIds = next
        }

        function clear() {
            currentOwnerId = ""
            clearFormState()
            captureSavedState()
        }

        function enterCreateMode() {
            testCase.session.selectedActorId = ""
            currentOwnerId = ""
            clearFormState()
            captureSavedState()
        }

        function previous() {
            const rows = testCase.workspaceRoles.actorRows || []
            if (rows.length === 0)
                return
            const nextId = testCase.session.navigatedSelectionId(rows,
                                                                  isEdit ? (testCase.session.selectedActorId || "") : "",
                                                                  -1,
                                                                  rows.length - 1)
            testCase.session.selectedActorId = nextId
        }

        function next() {
            const rows = testCase.workspaceRoles.actorRows || []
            if (rows.length === 0)
                return
            const nextId = testCase.session.navigatedSelectionId(rows,
                                                                  isEdit ? (testCase.session.selectedActorId || "") : "",
                                                                  1,
                                                                  0)
            testCase.session.selectedActorId = nextId
        }

        function submit() {
            const id = testCase.workspaceRoles.saveActor(isEdit ? (testCase.session.selectedActorId || "") : "",
                                                          name,
                                                          aliases || [],
                                                          selectedContractIds || [])
            if (id && id.length > 0)
                testCase.session.selectedActorId = id
            captureSavedState()
            return id
        }

        function deleteCurrent() {
            const removedId = testCase.session.selectedActorId || ""
            if (removedId.length === 0)
                return
            testCase.workspaceRoles.deleteActor(removedId)
            const nextId = testCase.session.deleteNextSelectionId(testCase.workspaceRoles.actorRows || [],
                                                                  removedId,
                                                                  0,
                                                                  "id")
            testCase.session.selectedActorId = nextId
        }
    }

    property var workspaceRoles: QtObject {
        property var actorRows: testCase.session.actors || []
        property var contractRows: testCase.session.contracts || []
        property var actorViewModel: testCase.actorViewModel
        function saveActor(id, name, aliases, contractIds) { return testCase.actorController.saveActor(id, name, aliases, contractIds) }
        function deleteActor(id) { testCase.actorController.deleteActor(id) }
    }

    Common.TestTheme {
        id: testTheme
    }

    property var theme: testTheme

    Component {
        id: actorFormComponent

        ActorView {
            width: 960
            height: 640
            actorViewModel: testCase.actorViewModel
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createActorObject(source) {
        return TestSupport.createActorObject(testCase, source)
    }

    function createForm(selectedActor) {
        var actorObject = createActorObject(selectedActor)
        session.selectedActor = actorObject
        session.selectedActorId = actorObject ? actorObject.id : ""
        return createTemporaryObject(actorFormComponent, testCase)
    }

    function init() {
        actorController.reset()
        session.selectedActor = null
        session.selectedActorId = ""
        session.actors = []
        session.contracts = []
    }

    function test_ACT_F_001_createModeSavesActorAndSelectsNewId() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm(null)
        var nameField = findRequired(form, "actorNameField")
        var submitButton = findRequired(form, "actorCreateButton")
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")
        var checkBox = findRequired(form, "actorContractCheckBox")

        form.actorViewModel.name = "Alice"
        aliasInput.text = "Alias One"
        addAliasButton.clicked()
        checkBox.checked = true
        checkBox.toggled()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        actorController.saveActorOverride = function(id, name, aliases, contractIds) {
            var result = actorController.defaultSaveActor(id, name, aliases, contractIds)
            session.selectedActor = createActorObject({
                id: result,
                name: name,
                aliases: aliases,
                contractIds: contractIds
            })
            session.selectedActorId = result
            session.dataRevision += 1
            return result
        }
        try {
            submitButton.clicked()
        } finally {
            actorController.saveActorOverride = null
        }

        compare(actorController.saveCalls, 1)
        compare(actorController.lastSave.id, "")
        compare(actorController.lastSave.name, "Alice")
        compare(actorController.lastSave.aliases.length, 1)
        compare(actorController.lastSave.aliases[0], "Alias One")
        compare(actorController.lastSave.contractIds.length, 1)
        compare(actorController.lastSave.contractIds[0], "contract-1")
        compare(session.selectedActorId, "actor-new")

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        session.selectedActor = createActorObject({ id: "actor-other", name: "Other", aliases: ["Other"], contractIds: [] })
        session.selectedActorId = "actor-other"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedActor = createActorObject({ id: "actor-new", name: "Alice", aliases: ["Alias One"], contractIds: ["contract-1"] })
        session.selectedActorId = "actor-new"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_ACT_F_002_aliasButtonsAddAndRemoveAlias() {
        var form = createForm(null)
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")
        var removeAliasButton = findRequired(form, "actorRemoveAliasButton")

        aliasInput.text = "Alias One"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Alias One")

        removeAliasButton.clicked()
        compare(form.aliases.length, 0)
    }

    function test_ACT_F_003_editModeAliasButtonAddsAliasToFormState() {
        var form = createForm({ id: "actor-4", name: "Alice", aliases: ["Base"] })
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")

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

    function test_ACT_F_004_aliasPanelGetsLayoutSpaceForRenderedAliases() {
        var form = createForm({ id: "actor-4", name: "Alice", aliases: ["Base"] })
        var aliasScroll = findRequired(form, "actorAliasScroll")

        compare(aliasScroll.width > 0, true)
        compare(aliasScroll.height > 0, true)
    }

    function test_ACT_F_004B_aliasChipClickSelectsAliasForRemoval() {
        var form = createForm({ id: "actor-4", name: "Alice", aliases: ["Base", "Alias Two"] })
        var aliasMouse = findRequired(form, "actorAliasMouse_1")
        var removeAliasButton = findRequired(form, "actorRemoveAliasButton")

        aliasMouse.clicked(null)

        compare(form.aliasIndex, 1)
        compare(removeAliasButton.enabled, true)
    }

    function test_ACT_F_005_readModeLoadsSelectedActorState() {
        var form = createForm({ id: "actor-3", name: "Bob", aliases: ["B", "B2"] })
        var nameField = findRequired(form, "actorNameField")

        compare(form.isEdit, true)
        compare(nameField.text, "Bob")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "B")
        compare(form.aliases[1], "B2")
    }

    function test_ACT_F_006_updateModeSavesCurrentActorId() {
        var form = createForm({ id: "actor-5", name: "Old", aliases: ["One"] })
        var nameField = findRequired(form, "actorNameField")
        var updateButton = findRequired(form, "actorUpdateButton")

        form.actorViewModel.name = "New Name"
        updateButton.clicked()

        compare(actorController.saveCalls, 1)
        compare(actorController.lastSave.id, "actor-5")
        compare(actorController.lastSave.name, "New Name")
    }

    function test_ACT_F_007_updateModeKeepsSelectedContractsAndSavesThem() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm({ id: "actor-6", name: "Old", aliases: ["One"], contractIds: [] })
        var nameField = findRequired(form, "actorNameField")
        var updateButton = findRequired(form, "actorUpdateButton")
        var aliasInput = findRequired(form, "actorAliasInput")
        var addAliasButton = findRequired(form, "actorAddAliasButton")
        var checkBox = findRequired(form, "actorContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()
        aliasInput.text = "Alias Two"
        addAliasButton.clicked()
        compare(form.aliases.length, 2)
        compare(form.aliases[1], "Alias Two")
        form.actorViewModel.name = "New Name"
        actorController.saveActorOverride = function(id, name, aliases, contractIds) {
            var result = actorController.defaultSaveActor(id, name, aliases, contractIds)
            if (session.selectedActor && session.selectedActor["setState"])
                session.selectedActor["setState"](id || "actor-6", name, aliases, contractIds)
            session.selectedActorId = id || "actor-6"
            session.dataRevision += 1
            return result
        }
        try {
            updateButton.clicked()
        } finally {
            actorController.saveActorOverride = null
        }

        compare(actorController.saveCalls, 1)
        compare(actorController.lastSave.id, "actor-6")
        compare(actorController.lastSave.name, "New Name")
        compare(actorController.lastSave.aliases.length, 2)
        compare(actorController.lastSave.aliases[0], "One")
        compare(actorController.lastSave.aliases[1], "Alias Two")
        compare(actorController.lastSave.contractIds.length, 1)
        compare(actorController.lastSave.contractIds[0], "contract-1")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectedActor = createActorObject({ id: "actor-7", name: "Other", aliases: ["Other"], contractIds: [] })
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedActor = createActorObject({
            id: "actor-6",
            name: "New Name",
            aliases: ["One", "Alias Two"],
            contractIds: ["contract-1"]
        })
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_ACT_F_008_selectionChangedSignalRefreshesAliasesWithoutReplacingSelectionObject() {
        var form = createForm({ id: "actor-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectActorState("actor-6", "Old", ["One", "Alias Two"], [])
        session.dataRevision += 1
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")

        session.selectActorState("actor-7", "Other", ["Other"], [])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectActorState("actor-6", "Old", ["One", "Alias Two"], [])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
    }

    function test_ACT_F_009_dataRevisionRefreshesAliasesWhenSelectionObjectDoesNotEmitChanged() {
        var form = createForm({ id: "actor-6", name: "Old", aliases: ["One"], contractIds: [] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "One")

        session.selectedActor.aliases = ["One", "Alias Two"]
        session.selectedActor.contractIds = ["contract-1"]
        session.dataRevision += 1

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "One")
        compare(form.aliases[1], "Alias Two")
        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")
    }

    function test_ACT_F_010_contractSelectionUpdatesSelectedIds() {
        session.contracts = [
            { id: "contract-1", name: "Lease" }
        ]
        var form = createForm(null)
        var checkBox = findRequired(form, "actorContractCheckBox")

        checkBox.checked = true
        checkBox.toggled()

        compare(form.selectedContractIds.length, 1)
        compare(form.selectedContractIds[0], "contract-1")

        var updateButton = findRequired(form, "actorUpdateButton")
        var nameField = findRequired(form, "actorNameField")
        nameField.text = "Alice"
        updateButton.clicked()
        compare(actorController.lastSave.contractIds.length, 1)
        compare(actorController.lastSave.contractIds[0], "contract-1")
    }

    function test_ACT_F_011_navigationButtonsMoveSelectionId() {
        session.actors = [
            { id: "actor-1", name: "A" },
            { id: "actor-2", name: "B" }
        ]
        var form = createForm({ id: "actor-1", name: "A", aliases: [] })
        var nextButton = findRequired(form, "actorNextButton")
        var previousButton = findRequired(form, "actorPreviousButton")

        nextButton.clicked()
        compare(session.selectedActorId, "actor-2")

        previousButton.clicked()
        compare(session.selectedActorId, "actor-1")
    }

    function test_ACT_F_013_deleteButtonDeletesCurrentActor() {
        var form = createForm({ id: "actor-7", name: "Charlie", aliases: [] })
        var deleteButton = findRequired(form, "actorDeleteButton")

        deleteButton.clicked()

        compare(actorController.deleteCalls, 1)
        compare(actorController.lastDeleteId, "actor-7")
    }
}
