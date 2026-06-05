/**
 * @file ui/tests/qml/views/contract/tst_ContractForm.qml
 * @brief Provides QML tests for ContractForm behavior.
 */

pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtTest 1.3
import FossRedder.Views 1.0

import "../../common/Lookup.js" as Lookup
import "../../common/TestSupport.js" as TestSupport

TestCase {
    id: testCase
    name: "ContractFormTests"
    when: windowShown
    width: 960
    height: 640

    property var session: QtObject {
        property var selectedContract: null
        property string selectedContractId: ""
        property int dataRevision: 0
        property var contracts: []
        property var actors: []
        property var properties: []
        onSelectedContractIdChanged: if (testCase.contractViewModel) testCase.contractViewModel.reloadFakeFormFromSelection(false)
        onSelectedContractChanged: if (testCase.contractViewModel) testCase.contractViewModel.reloadFakeFormFromSelection(true)
        onDataRevisionChanged: if (testCase.contractViewModel) testCase.contractViewModel.reloadFakeFormFromSelection(true)

        function contractFormState(name, type, selectedActorIds, selectedPropertyIds, aliases) {
            var aliasValues = aliases || []
            return {
                name: name || "",
                type: type || "",
                selectedActorIds: selectedActorIds || [],
                selectedPropertyIds: selectedPropertyIds || [],
                aliases: aliasValues,
                aliasInputText: "",
                aliasIndex: aliasValues.length > 0 ? 0 : -1
            }
        }

        function normalizeStrings(values) {
            var out = []
            if (!values)
                return out
            for (var i = 0; i < values.length; ++i)
                out.push(String(values[i]))
            return out
        }

        function addUniqueTrimmed(values, value) {
            var out = values ? values.slice(0) : []
            var next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }

        function removeAt(values, index) {
            var out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }

        function removeString(values, value) {
            var out = values ? values.slice(0) : []
            var target = String(value || "")
            var index = out.indexOf(target)
            if (index < 0)
                return out
            out.splice(index, 1)
            return out
        }

        function actorRows() {
            return actors || []
        }

        function contractRows() {
            return contracts || []
        }

        function navigatedId(rows, currentId, delta, fallbackIndex) {
            if (!rows || rows.length === 0)
                return ""
            var index = -1
            for (var i = 0; i < rows.length; ++i) {
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
            var kept = []
            for (var i = 0; i < rows.length; ++i) {
                var rowId = String(rows[i][key] || "")
                if (rowId !== String(removedId || ""))
                    kept.push(rows[i])
            }
            if (kept.length === 0)
                return ""
            var idx = Math.max(0, Math.min(fallbackIndex, kept.length - 1))
            return String(kept[idx][key] || "")
        }

        function displayRowsWithEmpty(rows, emptyLabel, textKey) {
            var out = [{ id: "", display: emptyLabel }]
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                out.push({
                    id: String(list[i].id || ""),
                    display: String(list[i][textKey] || "")
                })
            }
            return out
        }

        function indexOfId(rows, targetId) {
            var list = rows || []
            for (var i = 0; i < list.length; ++i) {
                if (String(list[i].id || "") === String(targetId || ""))
                    return i
            }
            return -1
        }

        function selectContractState(newId, newName, newType, newActorIds, newPropertyIds, newAliases) {
            if (selectedContract && selectedContract["setState"])
                selectedContract["setState"](newId, newName, newType, newActorIds, newPropertyIds, newAliases)
            selectedContractId = String(newId || "")
            if (testCase.contractViewModel)
                testCase.contractViewModel.reloadFakeFormFromSelection(true)
        }
    }

    property var contractController: QtObject {
        property int saveCalls: 0
        property int deleteCalls: 0
        property var lastSave: ({})
        property string lastDeleteId: ""
        property var saveContractOverride: null

        function reset() {
            saveCalls = 0
            deleteCalls = 0
            lastSave = ({})
            lastDeleteId = ""
            saveContractOverride = null
        }

        function defaultSaveContract(id, name, type, actorIds, propertyIds, aliases) {
            saveCalls += 1
            lastSave = {
                id: id,
                name: name,
                type: type,
                actorIds: actorIds,
                propertyIds: propertyIds,
                aliases: aliases
            }
            return id && id.length > 0 ? id : "contract-new"
        }

        function saveContract(id, name, type, actorIds, propertyIds, aliases) {
            const override = saveContractOverride
            if (override)
                return override.call(null, id, name, type, actorIds, propertyIds, aliases)
            return defaultSaveContract(id, name, type, actorIds, propertyIds, aliases)
        }

        function normalizeStrings(values) {
            var out = []
            if (!values)
                return out
            for (var i = 0; i < values.length; ++i)
                out.push(String(values[i]))
            return out
        }

        function addUniqueTrimmed(values, value) {
            var out = values ? values.slice(0) : []
            var next = String(value || "").trim()
            if (next.length === 0 || out.indexOf(next) !== -1)
                return out
            out.push(next)
            return out
        }

        function removeAt(values, index) {
            var out = values ? values.slice(0) : []
            if (index < 0 || index >= out.length)
                return out
            out.splice(index, 1)
            return out
        }

        function deleteContract(id) {
            deleteCalls += 1
            lastDeleteId = id
        }
    }

    property var contractViewModel: QtObject {
        readonly property string currentId: testCase.session.selectedContractId
        property string currentOwnerId: ""
        property string name: ""
        property string type: ""
        property var aliases: []
        property string aliasInputText: ""
        property int aliasIndex: -1
        property var selectedActorIds: []
        property var selectedPropertyIds: []
        property string savedName: ""
        property string savedType: ""
        property var savedAliases: []
        property var savedSelectedActorIds: []
        property var savedSelectedPropertyIds: []
        property string allocatableMode: "mixed"
        readonly property bool isEdit: testCase.session.selectedContractId.length > 0
        readonly property bool hasChanges: !isEdit
                ? name.trim().length > 0 || type.trim().length > 0 || aliases.length > 0 || selectedActorIds.length > 0 || selectedPropertyIds.length > 0
                : savedName !== name
                  || savedType !== type
                  || JSON.stringify(savedAliases) !== JSON.stringify(aliases)
                  || JSON.stringify(savedSelectedActorIds) !== JSON.stringify(selectedActorIds)
                  || JSON.stringify(savedSelectedPropertyIds) !== JSON.stringify(selectedPropertyIds)
        readonly property bool canSubmit: name.trim().length > 0
                                          && type.trim().length > 0
                                          && (selectedActorIds.length > 0 || selectedPropertyIds.length > 0)
        readonly property var contractRows: testCase.workspaceRoles.contractRows
        readonly property var actorRows: testCase.workspaceRoles.actorRows
        readonly property var actorDisplayRows: testCase.session.displayRowsWithEmpty(actorRows, "No actor", "name")
        readonly property var propertyRows: testCase.workspaceRoles.propertyRows
        readonly property int selectedActorIndex: {
            const id = selectedActorIds.length > 0 ? selectedActorIds[0] : ""
            const index = testCase.session.indexOfId(actorDisplayRows, id)
            return index >= 0 ? index : 0
        }

        function canAddAlias(value) { return String(value || "").trim().length > 0 }

        function clearFormState() {
            name = ""
            type = ""
            aliases = []
            aliasInputText = ""
            aliasIndex = -1
            selectedActorIds = []
            selectedPropertyIds = []
            allocatableMode = "mixed"
        }

        function captureSavedState() {
            savedName = name
            savedType = type
            savedAliases = aliases.slice(0)
            savedSelectedActorIds = selectedActorIds.slice(0)
            savedSelectedPropertyIds = selectedPropertyIds.slice(0)
        }

        function reloadFakeFormFromSelection(forceReload) {
            const currentId = testCase.session.selectedContractId || ""
            if (!forceReload && currentOwnerId === currentId)
                return
            currentOwnerId = currentId
            if (currentId.length === 0 || !testCase.session.selectedContract) {
                clearFormState()
                captureSavedState()
                return
            }
            const current = testCase.session.selectedContract
            const state = testCase.session.contractFormState(current.name || "",
                                                             current.type || "",
                                                             current.actorIds || [],
                                                             current.propertyIds || [],
                                                             current.aliases || [])
            name = state.name || ""
            type = state.type || ""
            aliases = state.aliases || []
            aliasInputText = state.aliasInputText || ""
            aliasIndex = state.aliasIndex !== undefined ? state.aliasIndex : -1
            selectedActorIds = state.selectedActorIds || []
            selectedPropertyIds = state.selectedPropertyIds || []
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
        function selectPrimaryActor(actorId) {
            const id = String(actorId || "").trim()
            selectedActorIds = id.length > 0 ? [id] : []
        }
        function setPropertySelected(propertyId, selected) {
            const id = String(propertyId || "").trim()
            const next = selected
                    ? testCase.session.addUniqueTrimmed(selectedPropertyIds || [], id)
                    : testCase.session.removeString(selectedPropertyIds || [], id)
            selectedPropertyIds = next
        }
        function clear() { currentOwnerId = ""; clearFormState(); captureSavedState() }
        function enterCreateMode() {
            testCase.session.selectedContractId = ""
            testCase.session.selectedContract = null
            currentOwnerId = ""
            clearFormState()
            captureSavedState()
        }
        function previous() {
            const rows = testCase.workspaceRoles.contractRows || []
            if (rows.length === 0) return
            testCase.session.selectedContractId = testCase.session.navigatedId(rows, isEdit ? testCase.session.selectedContractId : "", -1, rows.length - 1)
            syncSelectionObject()
            reloadFakeFormFromSelection(true)
        }
        function next() {
            const rows = testCase.workspaceRoles.contractRows || []
            if (rows.length === 0) return
            testCase.session.selectedContractId = testCase.session.navigatedId(rows, isEdit ? testCase.session.selectedContractId : "", 1, 0)
            syncSelectionObject()
            reloadFakeFormFromSelection(true)
        }
        function syncSelectionObject() {
            const id = testCase.session.selectedContractId || ""
            if (id.length === 0) {
                testCase.session.selectedContract = null
                return
            }
            const rows = testCase.workspaceRoles.contractRows || []
            for (var i = 0; i < rows.length; ++i) {
                if (String(rows[i].id || "") === id) {
                    testCase.session.selectedContract = testCase.createContractObject(rows[i])
                    return
                }
            }
        }
        function submit() {
            const id = testCase.workspaceRoles.saveContract(isEdit ? testCase.session.selectedContractId : "",
                                                             name,
                                                             type,
                                                             selectedActorIds || [],
                                                             selectedPropertyIds || [],
                                                             aliases || [])
            if (id && id.length > 0)
                testCase.session.selectedContractId = id
            captureSavedState()
            return id
        }
        function deleteCurrent() {
            const removedId = testCase.session.selectedContractId || ""
            if (removedId.length === 0) return
            testCase.workspaceRoles.deleteContract(removedId)
            testCase.session.selectedContractId = testCase.session.deleteNextSelectionId(testCase.workspaceRoles.contractRows || [], removedId, 0, "id")
        }
    }

    property var workspaceRoles: QtObject {
        property var contractViewModel: testCase.contractViewModel
        property var session: testCase.session
        property var contractRows: testCase.session.contracts
        property var actorRows: testCase.session.actors
        property var propertyRows: testCase.session.properties
        function saveContract(id, name, type, actorIds, propertyIds, aliases) { return testCase.contractController.saveContract(id, name, type, actorIds, propertyIds, aliases) }
        function deleteContract(id) { testCase.contractController.deleteContract(id) }
    }

    property var theme: QtObject {
        property int pageContentMargin: 8
        property int viewFormSpacing: 8
        property int formLabelWidth: 120
        property int viewAliasGroupSpacing: 6
        property int viewAliasPanelMinHeight: 140
        property int viewAliasPanelPreferredHeight: 180
        property int viewAliasChipHeight: 24
        property int viewAliasChipRadius: 3
        property int spacingSmall: 6
        property int spacingLarge: 20
        property int spacing: 8
        property int borderWidthThin: 1
        property color textMuted: "#666666"
        property color selectionHighlight: "#dddddd"
        property color surfaceAlt: "#f5f5f5"
        property color surface: "#ffffff"
        property color border: "#cccccc"
        property color textPrimary: "#000000"
        property int viewSelectionPanelMinHeight: 160
        property int viewSelectionPanelPreferredHeight: 220
        property int viewActionButtonWidth: 120
        property int viewCompactActionButtonSize: 28
        property int radius: 3
    }

    Component {
        id: contractFormComponent

        ContractView {
            width: 960
            height: 640
            contractViewModel: testCase.contractViewModel
            theme: testCase.theme
        }
    }

    function findRequired(root, objectName) {
        return TestSupport.findRequired(Lookup, root, objectName)
    }

    function createContractObject(source) {
        return TestSupport.createContractObject(testCase, source)
    }

    function createForm(selectedContract) {
        var contractObject = createContractObject(selectedContract)
        session.selectedContract = contractObject
        session.selectedContractId = contractObject ? contractObject.id : ""
        contractViewModel.reloadFakeFormFromSelection(true)
        return createTemporaryObject(contractFormComponent, testCase)
    }

    function init() {
        contractController.reset()
        session.selectedContract = null
        session.selectedContractId = ""
        session.contracts = []
        session.actors = []
        session.properties = []
        contractViewModel.clear()
    }

    function test_CON_F_001_createModeSavesContractAndSelectsNewId() {
        session.actors = [{ id: "actor-1", name: "Alice" }]
        session.properties = [{ id: "property-1", name: "Lot" }]
        var form = createForm(null)
        var nameField = findRequired(form, "contractNameField")
        var createButton = findRequired(form, "contractCreateButton")
        var actorCombo = findRequired(form, "contractActorComboBox")
        var propertyCheckBox = findRequired(form, "contractPropertyCheckBox")
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")

        compare(createButton.enabled, false)
        nameField.text = "Lease 2026"
        compare(createButton.enabled, false)
        var typeField = findRequired(form, "contractTypeField")
        typeField.text = "lease"
        typeField.textEdited()
        compare(createButton.enabled, false)

        actorCombo.currentIndex = 1
        actorCombo.activated(1)
        propertyCheckBox.checked = true
        propertyCheckBox.toggled()
        aliasInput.text = "Main"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
        compare(createButton.enabled, true)

        contractController.saveContractOverride = function(id, name, type, actorIds, propertyIds, aliases) {
            var result = contractController.defaultSaveContract(id, name, type, actorIds, propertyIds, aliases)
            session.selectedContract = createContractObject({
                id: result,
                name: name,
                type: type,
                actorIds: actorIds,
                propertyIds: propertyIds,
                aliases: aliases
            })
            session.selectedContractId = result
            session.dataRevision += 1
            return result
        }
        try {
            createButton.clicked()
        } finally {
            contractController.saveContractOverride = null
        }

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "")
        compare(contractController.lastSave.name, "Lease 2026")
        compare(contractController.lastSave.aliases.length, 1)
        compare(contractController.lastSave.aliases[0], "Main")
        compare(contractController.lastSave.actorIds.length, 1)
        compare(contractController.lastSave.actorIds[0], "actor-1")
        compare(contractController.lastSave.propertyIds.length, 1)
        compare(contractController.lastSave.propertyIds[0], "property-1")
        compare(session.selectedContractId, "contract-new")

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")

        session.selectedContract = createContractObject({
            id: "contract-other",
            name: "Other",
            type: "legacy",
            actorIds: [],
            propertyIds: [],
            aliases: ["Other"]
        })
        session.selectedContractId = "contract-other"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedContract = createContractObject({
            id: "contract-new",
            name: "Lease 2026",
            type: "lease",
            actorIds: ["actor-1"],
            propertyIds: ["property-1"],
            aliases: ["Main"]
        })
        session.selectedContractId = "contract-new"
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
    }

    function test_CON_F_002_aliasButtonsAddAndRemoveAlias() {
        var form = createForm(null)
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")
        var removeAliasButton = findRequired(form, "contractRemoveAliasButton")

        aliasInput.text = "Main"
        addAliasButton.clicked()
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Main")

        removeAliasButton.clicked()
        compare(form.aliases.length, 0)
    }

    function test_CON_F_003_editModeAliasButtonAddsAliasToFormState() {
        var form = createForm({ id: "contract-4", name: "Lease", type: "core", actorIds: ["actor-1"], propertyIds: [], aliases: ["Base"] })
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")

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

    function test_CON_F_004_aliasPanelGetsLayoutSpaceForRenderedAliases() {
        var form = createForm({ id: "contract-4", name: "Lease", type: "core", actorIds: ["actor-1"], propertyIds: [], aliases: ["Base"] })
        var aliasScroll = findRequired(form, "contractAliasScroll")

        compare(aliasScroll.width > 0, true)
        compare(aliasScroll.height > 0, true)
    }

    function test_CON_F_004B_aliasChipClickSelectsAliasForRemoval() {
        var form = createForm({ id: "contract-4", name: "Lease", type: "core", actorIds: ["actor-1"], propertyIds: [], aliases: ["Base", "Alias Two"] })
        var aliasMouse = findRequired(form, "contractAliasMouse_1")
        var removeAliasButton = findRequired(form, "contractRemoveAliasButton")

        aliasMouse.clicked(null)

        compare(form.aliasIndex, 1)
        compare(removeAliasButton.enabled, true)
    }

    function test_CON_F_005_readModeLoadsSelectedContractState() {
        var form = createForm({
            id: "contract-3",
            name: "Lease",
            type: "core",
            actorIds: ["actor-1"],
            propertyIds: ["property-1"],
            aliases: ["L1", "L2"]
        })
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")

        compare(form.isEdit, true)
        compare(nameField.text, "Lease")
        compare(typeField.text, "core")
        compare(form.aliases.length, 2)
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedPropertyIds.length, 1)
    }

    function test_CON_F_006_updateModeSavesCurrentContractIdWithType() {
        var form = createForm({ id: "contract-5", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: [] })
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var updateButton = findRequired(form, "contractUpdateButton")

        nameField.text = "New Contract"
        typeField.text = "modern"
        typeField.textEdited()
        updateButton.clicked()

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "contract-5")
        compare(contractController.lastSave.name, "New Contract")
        compare(contractController.lastSave.type, "modern")
    }

    function test_CON_F_007_updateModeKeepsSelectedRelationsAndSavesThem() {
        session.actors = [
            { id: "actor-1", name: "Alice" }
        ]
        session.properties = [
            { id: "property-1", name: "Lot" }
        ]
        var form = createForm({ id: "contract-6", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: ["Base"] })
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var updateButton = findRequired(form, "contractUpdateButton")
        var aliasInput = findRequired(form, "contractAliasInput")
        var addAliasButton = findRequired(form, "contractAddAliasButton")
        var actorCombo = findRequired(form, "contractActorComboBox")
        var propertyCheckBox = findRequired(form, "contractPropertyCheckBox")

        actorCombo.currentIndex = 1
        actorCombo.activated(1)
        propertyCheckBox.checked = true
        propertyCheckBox.toggled()
        aliasInput.text = "Alias Two"
        addAliasButton.clicked()
        compare(form.aliases.length, 2)
        compare(form.aliases[1], "Alias Two")
        nameField.text = "New Contract"
        typeField.text = "modern"
        typeField.textEdited()
        contractController.saveContractOverride = function(id, name, type, actorIds, propertyIds, aliases) {
            var result = contractController.defaultSaveContract(id, name, type, actorIds, propertyIds, aliases)
            if (session.selectedContract && session.selectedContract["setState"])
                session.selectedContract["setState"](id || "contract-6", name, type, actorIds, propertyIds, aliases)
            session.selectedContractId = id || "contract-6"
            session.dataRevision += 1
            return result
        }
        try {
            updateButton.clicked()
        } finally {
            contractController.saveContractOverride = null
        }

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.id, "contract-6")
        compare(contractController.lastSave.name, "New Contract")
        compare(contractController.lastSave.type, "modern")
        compare(contractController.lastSave.aliases.length, 2)
        compare(contractController.lastSave.aliases[0], "Base")
        compare(contractController.lastSave.aliases[1], "Alias Two")
        compare(contractController.lastSave.actorIds.length, 1)
        compare(contractController.lastSave.actorIds[0], "actor-1")
        compare(contractController.lastSave.propertyIds.length, 1)
        compare(contractController.lastSave.propertyIds[0], "property-1")
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")

        session.selectedContract = createContractObject({
            id: "contract-7",
            name: "Other",
            type: "legacy",
            actorIds: [],
            propertyIds: [],
            aliases: ["Other"]
        })
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectedContract = createContractObject({
            id: "contract-6",
            name: "New Contract",
            type: "modern",
            actorIds: ["actor-1"],
            propertyIds: ["property-1"],
            aliases: ["Base", "Alias Two"]
        })
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")

        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
    }

    function test_CON_F_008_updateModeReplacesPreviouslyAssignedActorWithNewSelection() {
        session.actors = [
            { id: "actor-1", name: "Alice" },
            { id: "actor-2", name: "Bob" }
        ]
        var form = createForm({ id: "contract-8", name: "Old", type: "legacy", actorIds: ["actor-2"], propertyIds: [], aliases: [] })
        var actorCombo = findRequired(form, "contractActorComboBox")
        var updateButton = findRequired(form, "contractUpdateButton")

        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-2")

        actorCombo.currentIndex = 1
        actorCombo.activated(1)
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")

        updateButton.clicked()

        compare(contractController.saveCalls, 1)
        compare(contractController.lastSave.actorIds.length, 1)
        compare(contractController.lastSave.actorIds[0], "actor-1")
    }

    function test_CON_F_009_selectionChangedSignalRefreshesAliasesWithoutReplacingSelectionObject() {
        var form = createForm({ id: "contract-6", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: ["Base"] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Base")

        session.selectContractState("contract-6", "Old", "legacy", [], [], ["Base", "Alias Two"])
        session.dataRevision += 1
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")

        session.selectContractState("contract-7", "Other", "legacy", [], [], ["Other"])
        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Other")

        session.selectContractState("contract-6", "Old", "legacy", [], [], ["Base", "Alias Two"])
        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")
    }

    function test_CON_F_010_dataRevisionRefreshesAliasesWhenSelectionObjectDoesNotEmitChanged() {
        var form = createForm({ id: "contract-6", name: "Old", type: "legacy", actorIds: [], propertyIds: [], aliases: ["Base"] })

        compare(form.aliases.length, 1)
        compare(form.aliases[0], "Base")

        session.selectedContract.aliases = ["Base", "Alias Two"]
        session.selectedContract.actorIds = ["actor-1"]
        session.selectedContract.propertyIds = ["property-1"]
        session.dataRevision += 1

        compare(form.aliases.length, 2)
        compare(form.aliases[0], "Base")
        compare(form.aliases[1], "Alias Two")
        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")
    }

    function test_CON_F_011_actorSelectionUpdatesSelectedActorIds() {
        session.actors = [
            { id: "actor-1", name: "Alice" }
        ]
        var form = createForm(null)
        var actorCombo = findRequired(form, "contractActorComboBox")

        actorCombo.currentIndex = 1
        actorCombo.activated(1)

        compare(form.selectedActorIds.length, 1)
        compare(form.selectedActorIds[0], "actor-1")
    }

    function test_CON_F_012_propertySelectionUpdatesSelectedPropertyIds() {
        session.properties = [
            { id: "property-1", name: "Lot" }
        ]
        var form = createForm(null)
        var propertyCheckBox = findRequired(form, "contractPropertyCheckBox")

        propertyCheckBox.checked = true
        propertyCheckBox.toggled()

        compare(form.selectedPropertyIds.length, 1)
        compare(form.selectedPropertyIds[0], "property-1")

        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var createButton = findRequired(form, "contractCreateButton")
        nameField.text = "Lease 2026"
        typeField.text = "lease"
        typeField.textEdited()
        compare(createButton.enabled, true)
        createButton.clicked()
        compare(contractController.lastSave.propertyIds.length, 1)
        compare(contractController.lastSave.propertyIds[0], "property-1")
    }

    function test_CON_F_013_createButtonStaysDisabledUntilAtLeastOneRelationIsSelected() {
        session.actors = [{ id: "actor-1", name: "Alice" }]
        session.properties = [{ id: "property-1", name: "Lot" }]
        var form = createForm(null)
        var nameField = findRequired(form, "contractNameField")
        var typeField = findRequired(form, "contractTypeField")
        var createButton = findRequired(form, "contractCreateButton")

        nameField.text = "Lease 2026"
        typeField.text = "lease"
        typeField.textEdited()

        compare(createButton.enabled, false)
        contractViewModel.selectedActorIds = ["actor-1"]
        compare(createButton.enabled, true)
    }

    function test_CON_F_014_navigationButtonsMoveSelectionId() {
        session.contracts = [
            { id: "contract-1", name: "A" },
            { id: "contract-2", name: "B" }
        ]
        var form = createForm({ id: "contract-1", name: "A", type: "x", actorIds: [], propertyIds: [], aliases: [] })
        var nextButton = findRequired(form, "contractNextButton")
        var previousButton = findRequired(form, "contractPreviousButton")

        nextButton.clicked()
        compare(session.selectedContractId, "contract-2")

        previousButton.clicked()
        compare(session.selectedContractId, "contract-1")
    }

    function test_CON_F_015_createShortcutButtonClearsSelectionAndSwitchesToCreateMode() {
        var form = createForm({ id: "contract-9", name: "Selected", type: "base", actorIds: [], propertyIds: [], aliases: [] })
        var createModeButton = findRequired(form, "contractCreateModeButton")

        createModeButton.clicked()

        compare(session.selectedContractId, "")
        compare(form.isEdit, false)
    }

    function test_CON_F_016_deleteButtonDeletesCurrentContract() {
        var form = createForm({ id: "contract-7", name: "Legacy", type: "base", actorIds: [], propertyIds: [], aliases: [] })
        var deleteButton = findRequired(form, "contractDeleteButton")

        deleteButton.clicked()

        compare(contractController.deleteCalls, 1)
        compare(contractController.lastDeleteId, "contract-7")
    }
}
