import { createLayoutSizeEditor } from "./layoutSizeWidget.js";
import { createFieldShell, joinPath, parseLayoutSizeValue, serializeLayoutSizeValue } from "./shared.js";

const BOX_LAYOUT_MODES = [
	{ value: "paired", symbol: "[]", label: "Paired Edges" },
	{ value: "expanded", symbol: "<>", label: "Individual Edges" },
];

function normalizeLayoutSizeValue(value) {
	return serializeLayoutSizeValue(parseLayoutSizeValue(value));
}

function inferBoxLayoutMode(edges) {
	const left = normalizeLayoutSizeValue(edges.left);
	const right = normalizeLayoutSizeValue(edges.right);
	const top = normalizeLayoutSizeValue(edges.top);
	const bottom = normalizeLayoutSizeValue(edges.bottom);

	if (top === bottom && left === right) return "paired";
	return "expanded";
}

function createBoxInput({ labelText, value, className = "", onCommit }) {
	const wrap = document.createElement("div");
	wrap.className = "boxEditorItem";
	if (className) wrap.classList.add(className);

	const label = document.createElement("span");
	label.className = "boxEditorLabel";
	label.textContent = labelText;

	const control = createLayoutSizeEditor({
		value,
		className: "boxLayoutSizeEditor",
		compact: true,
		onCommit,
	});

	wrap.appendChild(label);
	wrap.appendChild(control);
	return wrap;
}

export function renderBoxWidget({ container, meta, path, targetId, state, setMultiplePropertyValues, refreshInspector }) {
	const field = createFieldShell(meta, true);
	const edges = Array.isArray(meta.properties) ? meta.properties : [];
	const edgeMeta = {
		left: edges.find((entry) => entry?.key === "left"),
		top: edges.find((entry) => entry?.key === "top"),
		right: edges.find((entry) => entry?.key === "right"),
		bottom: edges.find((entry) => entry?.key === "bottom"),
	};

	const boxStateKey = `${targetId}:${path}`;
	const storedMode = state.inspectorState.boxLocks.get(boxStateKey);
	const currentMode = BOX_LAYOUT_MODES.some((mode) => mode.value === storedMode)
		? storedMode
		: inferBoxLayoutMode({
			left: edgeMeta.left?.value,
			right: edgeMeta.right?.value,
			top: edgeMeta.top?.value,
			bottom: edgeMeta.bottom?.value,
		});
	state.inspectorState.boxLocks.set(boxStateKey, currentMode);

	const layout = document.createElement("div");
	layout.className = "boxEditorSimple";
	const applyLayoutMode = (modeValue) => {
		layout.classList.toggle("boxEditorSimpleExpanded", modeValue === "expanded");
	};
	applyLayoutMode(currentMode);

	const lockButton = document.createElement("button");
	lockButton.type = "button";
	lockButton.className = "boxLockButton";
	lockButton.classList.add("boxLockButtonSimple");
	const applyLockMode = (modeValue) => {
		const mode = BOX_LAYOUT_MODES.find((entry) => entry.value === modeValue) || BOX_LAYOUT_MODES[0];
		lockButton.textContent = mode.symbol;
		lockButton.title = `${mode.label} (click to cycle)`;
		lockButton.setAttribute("aria-label", `${meta.name || meta.key || "Box"} lock mode: ${mode.label}`);
		lockButton.dataset.mode = mode.value;
		applyLayoutMode(mode.value);
	};
	applyLockMode(currentMode);
	let liveMode = currentMode;
	lockButton.addEventListener("click", () => {
		const currentIndex = BOX_LAYOUT_MODES.findIndex((mode) => mode.value === (lockButton.dataset.mode || "paired"));
		const nextMode = BOX_LAYOUT_MODES[(currentIndex + 1 + BOX_LAYOUT_MODES.length) % BOX_LAYOUT_MODES.length];
		const previousMode = liveMode;
		state.inspectorState.boxLocks.set(boxStateKey, nextMode.value);
		liveMode = nextMode.value;
		applyLockMode(nextMode.value);
		const updates = [];
		if (nextMode.value === "paired" && previousMode === "expanded") {
			updates.push(
				{ path: joinPath(path, "right"), value: edgeMeta.left?.value ?? edgeMeta.right?.value ?? "0" },
				{ path: joinPath(path, "bottom"), value: edgeMeta.top?.value ?? edgeMeta.bottom?.value ?? "0" }
			);
		}
		if (updates.length) {
			setMultiplePropertyValues(targetId, updates);
			return;
		}
		if (typeof refreshInspector === "function") {
			refreshInspector(targetId);
		}
	});

	if (currentMode === "expanded") {
		layout.appendChild(createBoxInput({
			labelText: "T",
			value: edgeMeta.top?.value,
			onCommit: (nextValue) => setMultiplePropertyValues(targetId, [{ path: joinPath(path, "top"), value: nextValue }]),
		}));
		layout.appendChild(createBoxInput({
			labelText: "R",
			value: edgeMeta.right?.value,
			onCommit: (nextValue) => setMultiplePropertyValues(targetId, [{ path: joinPath(path, "right"), value: nextValue }]),
		}));
		layout.appendChild(createBoxInput({
			labelText: "B",
			value: edgeMeta.bottom?.value,
			onCommit: (nextValue) => setMultiplePropertyValues(targetId, [{ path: joinPath(path, "bottom"), value: nextValue }]),
		}));
		layout.appendChild(createBoxInput({
			labelText: "L",
			value: edgeMeta.left?.value,
			onCommit: (nextValue) => setMultiplePropertyValues(targetId, [{ path: joinPath(path, "left"), value: nextValue }]),
		}));
	} else {
		layout.appendChild(createBoxInput({
			labelText: "V",
			value: edgeMeta.top?.value,
			onCommit: (nextValue) => setMultiplePropertyValues(targetId, [
				{ path: joinPath(path, "top"), value: nextValue },
				{ path: joinPath(path, "bottom"), value: nextValue },
			]),
		}));
		layout.appendChild(createBoxInput({
			labelText: "H",
			value: edgeMeta.left?.value,
			onCommit: (nextValue) => setMultiplePropertyValues(targetId, [
				{ path: joinPath(path, "left"), value: nextValue },
				{ path: joinPath(path, "right"), value: nextValue },
			]),
		}));
	}

	layout.appendChild(lockButton);

	field.appendChild(layout);
	container.appendChild(field);
}
