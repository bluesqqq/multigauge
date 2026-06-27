import { createEditorSession, makeBlankFaceTemplate, makeDefaultElementTemplate } from "./api.js";
import { createGaugeView } from "./view.js";
import { createHierarchyView } from "./hierarchy.js";
import { createPropertiesView } from "./properties.js";
import { closeAllDropdowns, openMenu } from "../dropdown.js";

const FACE_TEMPLATE_DEFS = [
	{
		key: "empty",
		label: "Empty",
		sourceUrl: "/static/json/templates/empty.json",
	},
	{
		key: "rectangle",
		label: "Rectangle",
		sourceUrl: "/static/json/templates/rectangle.json",
	},
	{
		key: "graph",
		label: "Graph",
		sourceUrl: "/static/json/templates/graph.json",
	},
	{
		key: "tachometer",
		label: "Tachometer",
		sourceUrl: "/static/json/templates/tachometer.json",
	},
];
const FACE_TEMPLATE_DEFS_BY_KEY = new Map(FACE_TEMPLATE_DEFS.map((entry) => [entry.key, entry]));

const statusEl = document.getElementById("status");
const menuButtonEl = document.getElementById("menuButton");
const historyButtonEl = document.getElementById("historyButton");
const historyPanelEl = document.getElementById("historyPanel");
const historyListEl = document.getElementById("historyList");
const undoButtonEl = document.getElementById("undoButton");
const redoButtonEl = document.getElementById("redoButton");
const resetViewButtonEl = document.getElementById("resetViewButton");
const viewRenderWidthEl = document.getElementById("viewRenderWidth");
const viewRenderHeightEl = document.getElementById("viewRenderHeight");
const viewCircularEl = document.getElementById("viewCircular");
const openInputEl = document.getElementById("openInput");
const hierarchyEl = document.getElementById("hierarchy");
const viewportEl = document.getElementById("viewViewport");
const workspaceEl = document.getElementById("viewWorkspace");
const propertiesEl = document.getElementById("properties");

const EMPTY_HIERARCHY_MODEL = {
	roots: [],
	nodes: new Map(),
	parentById: new Map(),
	depthById: new Map(),
	rootById: new Map(),
};

let session = null;
let hierarchyView = null;
let gaugeView = null;
let propertiesView = null;
let hierarchyPayload = null;
let hierarchyModel = EMPTY_HIERARCHY_MODEL;
let historyEntries = [];
let historyHeadIndex = 0;
let elementTypeEntries = [];
let selectedId = 0;
let refreshQueued = false;
let historySyncQueued = false;

function setStatus(message) {
	if (statusEl) {
		statusEl.textContent = message;
	}
}

function isEditableTarget(target) {
	return target instanceof HTMLElement && (target.isContentEditable || ["INPUT", "TEXTAREA", "SELECT"].includes(target.tagName));
}

function closeHistoryMenu() {
	historyPanelEl?.classList.add("hidden");
}

function toggleHistoryMenu() {
	if (!historyPanelEl) return;
	historyPanelEl.classList.toggle("hidden");
}

function getSelectedRootId() {
	return hierarchyModel.rootById.get(Number(selectedId)) || Number(selectedId) || 0;
}

function normalizePackageInfo(raw) {
	const source = raw && typeof raw === "object" && !Array.isArray(raw)
		? (raw.data && typeof raw.data === "object" ? raw.data : raw)
		: {};

	return {
		name: String(source.name ?? source.title ?? "").trim(),
		author: String(source.author ?? "").trim(),
		description: String(source.description ?? source.summary ?? ""),
	};
}

function getCurrentPackageInfo() {
	try {
		return normalizePackageInfo(session?.getPackageInfo?.());
	} catch {
		return { name: "", author: "", description: "" };
	}
}

function normalizeFaceName(raw) {
	if (typeof raw === "string") {
		return raw.trim();
	}
	if (raw && typeof raw === "object") {
		return String(raw.name ?? raw.value ?? raw.data ?? "").trim();
	}
	return "";
}

async function renamePackage() {
	if (!session) return;
	const current = getCurrentPackageInfo();
	const next = window.prompt("Rename package", current.name || "Package");
	if (next == null) return;
	const trimmed = String(next).trim();
	if (!trimmed || trimmed === current.name) return;

	await mutate(async () => {
		session.setPackageInfo(trimmed, current.author, current.description);
	}, "Renamed package.");
}

async function renameFace(faceId) {
	if (!session || !faceId) return;
	const current = normalizeFaceName(session?.getFaceName?.(faceId));
	const next = window.prompt("Rename face", current || `Face ${faceId}`);
	if (next == null) return;
	const trimmed = String(next).trim();
	if (!trimmed || trimmed === current) return;

	await mutate(async () => {
		session.setFaceName(faceId, trimmed);
	}, "Renamed face.");
}

function queueRefresh() {
	if (refreshQueued) return;
	refreshQueued = true;
	window.requestAnimationFrame(() => {
		refreshQueued = false;
		renderAll();
	});
}

function queueHistorySync() {
	if (historySyncQueued) return;
	historySyncQueued = true;
	window.requestAnimationFrame(() => {
		historySyncQueued = false;
		syncUndoRedo();
		renderHistoryMenu();
	});
}

function extractHistoryEntries(history) {
	if (Array.isArray(history)) return history.slice();
	if (Array.isArray(history?.entries)) return history.entries.slice();
	if (Array.isArray(history?.history)) return history.history.slice();
	if (Array.isArray(history?.items)) return history.items.slice();
	if (Array.isArray(history?.list)) return history.list.slice();
	return [];
}

function coerceHistoryFlag(value) {
	if (typeof value === "boolean") return value;
	if (typeof value === "number") return value !== 0;
	if (typeof value === "string") {
		const normalized = value.trim().toLowerCase();
		return normalized === "true" || normalized === "1" || normalized === "yes";
	}
	if (typeof value === "function") {
		try {
			return !!value();
		} catch {
			return false;
		}
	}
	return !!value;
}

function updateUndoRedo(history, currentIndex = null, entries = []) {
	const normalizedIndex = Number.isFinite(Number(currentIndex)) ? Number(currentIndex) : null;
	let canUndo = false;
	let canRedo = false;

	if (Array.isArray(entries) && entries.length > 0 && normalizedIndex !== null) {
		const safeIndex = Math.max(0, Math.min(normalizedIndex, Math.max(0, entries.length - 1)));
		canUndo = safeIndex > 0;
		canRedo = safeIndex < entries.length - 1;
	} else {
		canUndo = coerceHistoryFlag(history?.canUndo);
		canRedo = coerceHistoryFlag(history?.canRedo);
	}
	if (undoButtonEl) undoButtonEl.disabled = !canUndo;
	if (redoButtonEl) redoButtonEl.disabled = !canRedo;
}

function syncUndoRedo() {
	try {
		const history = session?.getHistory?.();
		const indexValue = Number(session?.historyIndex?.());
		historyEntries = extractHistoryEntries(history);
		historyHeadIndex = Number.isFinite(indexValue)
			? indexValue
			: Math.max(0, historyEntries.length - 1);
		updateUndoRedo(history, historyHeadIndex, historyEntries);
	} catch (error) {
		console.error("Failed to sync undo/redo state:", error);
	}
}

function renderHistoryMenu() {
	if (!historyListEl) return;
	historyListEl.innerHTML = "";

	if (!historyEntries.length) {
		const empty = document.createElement("div");
		empty.textContent = "No history yet.";
		historyListEl.appendChild(empty);
		return;
	}

	const safeHeadIndex = Math.max(0, Math.min(historyHeadIndex, historyEntries.length - 1));
	for (let index = historyEntries.length - 1; index >= 0; index -= 1) {
		const entry = historyEntries[index];
		const button = document.createElement("button");
		button.type = "button";
		const isCurrent = index === safeHeadIndex;
		const isNewerThanHead = index > safeHeadIndex;
		button.className = "historyItem" + (isCurrent ? " current" : "") + (isNewerThanHead ? " ahead" : "");
		button.textContent = String(entry);
		button.addEventListener("click", async () => {
			closeHistoryMenu();
			await jumpToHistoryIndex(index);
		});
		historyListEl.appendChild(button);
	}
}

async function jumpToHistoryIndex(targetIndex) {
	const safeTarget = Math.max(0, Math.min(Number(targetIndex) || 0, historyEntries.length - 1));
	try {
		await session?.jumpTo?.(safeTarget);
		queueRefresh();
		queueHistorySync();
		setStatus(`Jumped to history ${safeTarget}.`);
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
	}
}

function prettifyLabel(value) {
	return String(value || "")
		.replace(/[_-]+/g, " ")
		.replace(/([a-z0-9])([A-Z])/g, "$1 $2")
		.trim()
		.replace(/\b\w/g, (char) => char.toUpperCase());
}

function normalizeElementTypeEntry(entry) {
	if (typeof entry === "string") {
		return {
			value: entry,
			label: prettifyLabel(entry),
		};
	}

	if (entry && typeof entry === "object") {
		const value = entry.value || entry.type || entry.id || entry.key || entry.name || entry.label || "";
		return {
			value,
			label: entry.label || entry.name || entry.title || prettifyLabel(value),
			template: entry.template || entry.json || entry.defaultTemplate || null,
		};
	}

	return {
		value: "",
		label: "Unknown",
	};
}

function extractElementTypeList(raw) {
	if (Array.isArray(raw)) {
		return raw;
	}

	if (raw && typeof raw === "object") {
		if (Array.isArray(raw.items)) return raw.items;
		if (Array.isArray(raw.types)) return raw.types;
		if (Array.isArray(raw.elements)) return raw.elements;
		if (Array.isArray(raw.value)) return raw.value;
		if (Array.isArray(raw.data)) return raw.data;
		if (Array.isArray(raw.list)) return raw.list;
	}

	return [];
}

function getElementTypeMenuItems(parentId) {
	if (!elementTypeEntries.length) {
		return [
			{
				label: "No element types",
				disabled: true,
			},
		];
	}

	return elementTypeEntries.map((entry) => ({
		label: entry.label,
		onClick: async () => addChild(parentId, entry),
	}));
}

async function ensureElementTypesLoaded() {
	if (elementTypeEntries.length) return elementTypeEntries;
	const raw = session?.listElementTypes?.() ?? session?.listElements?.();
	const list = extractElementTypeList(raw);
	elementTypeEntries = list.map(normalizeElementTypeEntry);
	if (!elementTypeEntries.length && raw && typeof raw === "object") {
		elementTypeEntries = Object.entries(raw)
			.filter(([key, value]) => typeof value === "string" || (value && typeof value === "object"))
			.map(([key, value]) => normalizeElementTypeEntry(value && typeof value === "object" ? { ...value, value: value.value || key, label: value.label || value.name || key } : key));
	}
	return elementTypeEntries;
}

async function loadFaceTemplate(name) {
	const templateDef = FACE_TEMPLATE_DEFS_BY_KEY.get(name);
	const sourceUrl = templateDef?.sourceUrl || `/static/json/templates/${name}.json`;
	const response = await fetch(sourceUrl, { cache: "no-store" });
	if (!response.ok) {
		throw new Error(`Failed to load ${name} template (${response.status}).`);
	}
	const json = await response.json();
	return json?.root || json;
}

function createElementTemplateForType(entry) {
	const template = entry?.template ? structuredClone(entry.template) : makeDefaultElementTemplate();
	const value = entry?.value || entry?.type || entry?.id || entry?.key || entry?.name;
	if (value && typeof template === "object" && template) {
		template.type = value;
	}
	return template;
}

async function createFaceFromPreset(presetKey) {
	if (!FACE_TEMPLATE_DEFS_BY_KEY.has(presetKey)) {
		await createFace(`Face ${hierarchyModel.roots.length + 1}`);
		return;
	}

	const template = await loadFaceTemplate(presetKey);
	await mutate(async () => {
		const result = session.createFace(structuredClone(template));
		selectedId = result?.id || selectedId;
	}, `Created ${presetKey}.`);
}

function ensureSelectedNodeExists() {
	if (!selectedId) return;
	if (!hierarchyModel.nodes.has(Number(selectedId))) {
		selectedId = hierarchyModel.roots[0] || 0;
	}
}

function syncViewControls() {
	if (!gaugeView) return;
	const faceId = getSelectedRootId();
	const faceConfig = faceId ? gaugeView.getFaceRenderConfig?.(faceId) : null;
	const hasFace = !!faceConfig;

	if (viewRenderWidthEl) viewRenderWidthEl.disabled = !hasFace;
	if (viewRenderHeightEl) viewRenderHeightEl.disabled = !hasFace || !!faceConfig?.circular;
	if (viewCircularEl) viewCircularEl.disabled = !hasFace;

	if (hasFace) {
		if (viewRenderWidthEl) viewRenderWidthEl.value = String(faceConfig.width || 250);
		if (viewRenderHeightEl) viewRenderHeightEl.value = String(faceConfig.height || 240);
		if (viewCircularEl) viewCircularEl.checked = !!faceConfig.circular;
	}
}

function applySelectedFaceSize() {
	const faceId = getSelectedRootId();
	if (!faceId || !gaugeView) return;
	const width = Number.parseInt(viewRenderWidthEl?.value || "0", 10);
	const circular = !!viewCircularEl?.checked;
	const height = circular ? width : Number.parseInt(viewRenderHeightEl?.value || "0", 10);
	if (!Number.isFinite(width) || width <= 0 || !Number.isFinite(height) || height <= 0) {
		setStatus("Width and height must be positive numbers.");
		return;
	}
	if (gaugeView.setFaceRenderSize?.(faceId, width, height)) {
		gaugeView.setFaceCircular?.(faceId, circular);
		queueRefresh();
		syncViewControls();
		setStatus(`Updated face size to ${width}x${height}.`);
	}
}

function applySelectedFaceCircular() {
	const faceId = getSelectedRootId();
	if (!faceId || !gaugeView) return;
	const circular = !!viewCircularEl?.checked;
	if (circular && viewRenderWidthEl?.value) {
		const width = Number.parseInt(viewRenderWidthEl.value, 10);
		if (Number.isFinite(width) && width > 0 && viewRenderHeightEl) {
			viewRenderHeightEl.value = String(width);
		}
	}
	if (gaugeView.setFaceCircular?.(faceId, circular)) {
		queueRefresh();
		syncViewControls();
	}
}

async function undoLastChange() {
	try {
		const ok = session?.undo?.();
		if (!ok) {
			setStatus("Nothing to undo.");
			return;
		}
		queueRefresh();
		queueHistorySync();
		setStatus("Undid.");
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
	}
}

async function redoLastChange() {
	try {
		const ok = session?.redo?.();
		if (!ok) {
			setStatus("Nothing to redo.");
			return;
		}
		queueRefresh();
		queueHistorySync();
		setStatus("Redid.");
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
	}
}

async function mutate(action, message) {
	try {
		await action();
		setStatus(message || "Updated.");
		queueRefresh();
		queueHistorySync();
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
	}
}

async function createFace(name = "Face") {
	await mutate(async () => {
		const result = session.createFace(makeBlankFaceTemplate());
		selectedId = result?.id || selectedId;
	}, `Created ${name}.`);
}

function buildCreationMenuItems() {
	return [
		{
			label: "New",
			onClick: async () => createFace(`Face ${hierarchyModel.roots.length + 1}`),
		},
		{
			label: "New from template",
			submenu: buildFaceTemplateSubmenuItems(),
		},
	];
}

function buildTopMenuActions() {
	return [
		...buildCreationMenuItems(),
		{ type: "divider" },
		{
			label: "Open",
			onClick: async () => {
				openInputEl?.click();
			},
		},
		{ type: "divider" },
		{
			label: "Save",
			onClick: async () => {
				try {
					const text = getSaveText();
					downloadText("multigauge.json", text);
					setStatus("Saved.");
				} catch (error) {
					setStatus(`Error: ${error.message || error}`);
				}
			},
		},
		{
			label: "Export",
			onClick: async () => {
				try {
					const text = getSaveText();
					downloadText("multigauge-export.json", text);
					setStatus("Exported.");
				} catch (error) {
					setStatus(`Error: ${error.message || error}`);
				}
			},
		},
	];
}

function buildHierarchyPanelActions() {
	return [
		{
			label: "Rename package",
			onClick: async () => renamePackage(),
		},
		{ type: "divider" },
		...buildCreationMenuItems(),
		{ type: "divider" },
		{
			label: "Paste",
			onClick: async () => mutate(() => session.pasteFace(hierarchyModel.roots.length), "Pasted."),
		},
	];
}

function buildFaceTemplateSubmenuItems() {
	const items = [];
	for (const template of FACE_TEMPLATE_DEFS) {
		if (items.length) {
			items.push({ type: "divider" });
		}
		items.push({
			label: template.label,
			onClick: async () => createFaceFromPreset(template.key),
		});
	}
	return items;
}

async function addChild(parentId = selectedId, typeEntry = null) {
	const normalizedParentId = Number(parentId) || 0;
	if (!normalizedParentId) return;
	await mutate(async () => {
		const result = session.createElement(normalizedParentId, createElementTemplateForType(typeEntry));
		selectedId = result?.id || selectedId;
	}, "Added child.");
}

async function openDocumentFromFile(file) {
	if (!file) return;
	const text = await file.text();
	await mutate(async () => {
		session.loadDocument(text);
	}, `Opened ${file.name}.`);
}

function getSaveText() {
	const result = session.saveDocument();
	return session.formatSaveText(result);
}

function downloadText(filename, text) {
	const blob = new Blob([text], { type: "application/json" });
	const url = URL.createObjectURL(blob);
	const link = document.createElement("a");
	link.href = url;
	link.download = filename;
	document.body.appendChild(link);
	link.click();
	link.remove();
	URL.revokeObjectURL(url);
}

function buildNodeActions(context) {
	const addMenu = {
		label: "Add",
		submenu: getElementTypeMenuItems(context.nodeId),
		search: {
			placeholder: "Search element types",
			emptyText: "No element types found.",
		},
	};

	if (context.isFace) {
		const faceIndex = hierarchyModel.roots.indexOf(context.nodeId);
		return [
			addMenu,
			{ type: "divider" },
			{
				label: "Rename",
				onClick: async () => renameFace(context.nodeId),
			},
			{ type: "divider" },
			{
				label: "Copy",
				onClick: async () => {
					session.copyFace(context.nodeId);
				},
			},
			{
				label: "Cut",
				onClick: async () => mutate(() => session.cutFace(context.nodeId), "Cut."),
			},
			{
				label: "Paste",
				onClick: async () => mutate(() => session.pasteFace(Math.max(0, faceIndex + 1)), "Pasted."),
			},
			{
				label: "Duplicate",
				onClick: async () => mutate(() => {
					session.copyFace(context.nodeId);
					return session.pasteFace(Math.max(0, faceIndex + 1));
				}, "Duplicated."),
			},
			{ type: "divider" },
			{
				label: "Delete",
				onClick: async () => mutate(() => {
					session.removeFace(context.nodeId);
					selectedId = hierarchyModel.roots[0] || 0;
				}, "Removed."),
			},
			{ type: "divider" },
			{
				label: "Move Up",
				disabled: faceIndex <= 0,
				onClick: async () => mutate(() => {
					if (faceIndex > 0) {
						return session.reorderFace(context.nodeId, faceIndex - 1);
					}
				}, "Moved."),
			},
			{
				label: "Move Down",
				disabled: faceIndex < 0 || faceIndex >= hierarchyModel.roots.length - 1,
				onClick: async () => mutate(() => {
					if (faceIndex >= 0 && faceIndex < hierarchyModel.roots.length - 1) {
						return session.reorderFace(context.nodeId, faceIndex + 1);
					}
				}, "Moved."),
			},
		];
	}

	const parentId = context.parentId || 0;
	const parentNode = parentId ? hierarchyModel.nodes.get(Number(parentId)) || null : null;
	const siblings = Array.isArray(parentNode?.children) ? parentNode.children : [];
	const index = siblings.indexOf(context.nodeId);

	return [
		addMenu,
		{ type: "divider" },
		{
			label: "Copy",
			onClick: async () => {
				session.copyElement(context.nodeId);
			},
		},
		{
			label: "Cut",
			onClick: async () => mutate(() => session.cutElement(context.nodeId), "Cut."),
		},
		{
			label: "Paste",
			onClick: async () => mutate(() => {
				const childCount = Array.isArray(context.node?.children) ? context.node.children.length : 0;
				return session.pasteElement(context.nodeId, childCount);
			}, "Pasted."),
		},
		{
			label: "Duplicate",
			onClick: async () => mutate(() => {
				session.copyElement(context.nodeId);
				return session.pasteElement(parentId, Math.max(0, index + 1));
			}, "Duplicated."),
		},
		{ type: "divider" },
		{
			label: "Delete",
			onClick: async () => mutate(() => {
				session.removeElement(context.nodeId);
				selectedId = hierarchyModel.roots[0] || 0;
			}, "Removed."),
		},
		{ type: "divider" },
		{
			label: "Move Up",
			disabled: index <= 0,
			onClick: async () => mutate(() => {
				if (index > 0) {
					return session.reorderElement(context.nodeId, index - 1);
				}
			}, "Moved."),
		},
		{
			label: "Move Down",
			disabled: index < 0 || index >= siblings.length - 1,
			onClick: async () => mutate(() => {
				if (index >= 0 && index < siblings.length - 1) {
					return session.reorderElement(context.nodeId, index + 1);
				}
			}, "Moved."),
		},
	];
}

function openNodeContextMenu(context) {
	openMenu(buildNodeActions(context), context.x, context.y);
}

function openHierarchyPanelContextMenu(event) {
	const target = event.target;
	if (target instanceof Element && target.closest(".treeButton")) {
		return;
	}

	event.preventDefault();
	event.stopPropagation();
	openMenu(buildHierarchyPanelActions(), event.clientX, event.clientY);
}

function renderHierarchy() {
	if (!hierarchyView) return;
	hierarchyModel = hierarchyView.render(hierarchyPayload) || EMPTY_HIERARCHY_MODEL;
}

function renderView() {
	if (!gaugeView) return;
	gaugeView.render();
	syncViewControls();
}

function renderProperties() {
	if (!propertiesView) return;
	propertiesView.render();
}

function renderAll() {
	if (!session) return;
	try {
		hierarchyPayload = session.getHierarchy();
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
		return;
	}

	ensureSelectedNodeExists();
	syncUndoRedo();
	renderHistoryMenu();
	renderHierarchy();
	renderView();
	renderProperties();
	syncViewControls();
}

async function initialize() {
	try {
		setStatus("Loading Multigauge...");
		session = await createEditorSession();
		await ensureElementTypesLoaded();

		hierarchyView = createHierarchyView({
			containerEl: hierarchyEl,
			getSelectedId: () => selectedId,
			getPackageName: () => getCurrentPackageInfo().name || "Package",
			onPackageClick: renamePackage,
			onPackageContextMenu: (context) => openMenu([
				{
					label: "Rename package",
					onClick: async () => renamePackage(),
				},
			], context.x, context.y),
			onSelect: (id) => {
				selectedId = Number(id) || 0;
				syncViewControls();
				queueRefresh();
			},
			onContextMenu: openNodeContextMenu,
		});
		hierarchyEl?.addEventListener("contextmenu", openHierarchyPanelContextMenu);

		gaugeView = createGaugeView({
			viewportEl,
			workspaceEl,
			editorSession: session,
			getHierarchyModel: () => hierarchyModel,
			getSelectedId: () => selectedId,
			onSelect: (id) => {
				selectedId = Number(id) || 0;
				syncViewControls();
				queueRefresh();
			},
			onContextMenu: openNodeContextMenu,
			onStatus: setStatus,
			onViewChange: syncViewControls,
		});

				propertiesView = createPropertiesView({
					containerEl: propertiesEl,
					editorSession: session,
					getSelectedId: () => selectedId,
					onStatus: setStatus,
					onDocumentMutated: (options = {}) => {
						if (options?.reloadDocument || options?.refresh !== false) {
							queueRefresh();
						}
						queueHistorySync();
					},
				});

			await createFace("Face 1");
			queueRefresh();
			queueHistorySync();
			syncViewControls();
			setStatus("Ready.");
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
		console.error(error);
	}
}

menuButtonEl?.addEventListener("click", (event) => {
	event.stopPropagation();
	const rect = menuButtonEl.getBoundingClientRect();
	openMenu(buildTopMenuActions(), rect.left + window.scrollX, rect.bottom + window.scrollY, {
		anchorEl: menuButtonEl,
	});
});
historyButtonEl?.addEventListener("click", () => {
	toggleHistoryMenu();
});

undoButtonEl?.addEventListener("click", async () => {
	await undoLastChange();
});

redoButtonEl?.addEventListener("click", async () => {
	await redoLastChange();
});

resetViewButtonEl?.addEventListener("click", () => {
	gaugeView?.resetView?.();
});

viewRenderWidthEl?.addEventListener("input", () => {
	if (viewCircularEl?.checked && viewRenderWidthEl && viewRenderHeightEl) {
		viewRenderHeightEl.value = viewRenderWidthEl.value;
	}
	applySelectedFaceSize();
});

viewRenderWidthEl?.addEventListener("change", () => {
		applySelectedFaceSize();
});

viewRenderHeightEl?.addEventListener("input", () => {
	if (!viewCircularEl?.checked) {
		applySelectedFaceSize();
	}
});

viewRenderHeightEl?.addEventListener("change", () => {
		applySelectedFaceSize();
});

viewCircularEl?.addEventListener("change", () => {
		applySelectedFaceCircular();
});

viewRenderWidthEl?.addEventListener("keydown", (event) => {
	if (event.key === "Enter") {
		event.preventDefault();
		applySelectedFaceSize();
	}
});

viewRenderHeightEl?.addEventListener("keydown", (event) => {
	if (event.key === "Enter") {
		event.preventDefault();
		applySelectedFaceSize();
	}
});

openInputEl?.addEventListener("change", async () => {
	const file = openInputEl.files?.[0];
	openInputEl.value = "";
	if (!file) return;
	try {
		await openDocumentFromFile(file);
	} catch (error) {
		setStatus(`Error: ${error.message || error}`);
	}
});

document.addEventListener("click", (event) => {
	if (historyPanelEl && !historyPanelEl.contains(event.target) && event.target !== historyButtonEl) {
		closeHistoryMenu();
	}
});

document.addEventListener("keydown", (event) => {
	if (event.key === "Escape") {
		closeHistoryMenu();
		closeAllDropdowns();
		return;
	}
	if (isEditableTarget(event.target)) {
		return;
	}

	if ((event.ctrlKey || event.metaKey) && !event.altKey && event.key.toLowerCase() === "z") {
		event.preventDefault();
		if (event.shiftKey) {
			redoLastChange();
		} else {
			undoLastChange();
		}
	}
});

initialize();
