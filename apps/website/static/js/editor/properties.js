import { renderArrayWidget } from "/static/js/editor/widgets/arrayWidget.js";
import { renderBoxWidget } from "/static/js/editor/widgets/boxWidget.js";
import { renderLeafWidget } from "/static/js/editor/widgets/renderLeafWidget.js";
import { renderTypeSelectorWidget } from "/static/js/editor/widgets/typeSelectorWidget.js";
import { joinPath, splitProperties } from "/static/js/editor/widgets/shared.js";

function stringifySelection(value) {
	if (value == null) return "";
	if (typeof value === "string") return value;
	try {
		return JSON.stringify(value);
	} catch (_) {
		return String(value);
	}
}

export function createPropertiesView({
	containerEl,
	editorSession,
	getSelectedId,
	onStatus,
	onDocumentMutated,
}) {
	const state = {
		inspectorState: {
			boxLocks: new Map(),
			nullableValues: new Map(),
			panels: new Map(),
		},
	};

	const setPropertyValue = async (targetId, path, value, options = {}) => {
		const result = editorSession.setProperty(targetId, path, JSON.stringify(value));
		if (result && result.ok === false) {
			throw new Error(result.error || "Property update failed");
		}
		await onDocumentMutated?.({
			targetId,
			path,
			refresh: options.refresh,
			reloadDocument: options.reloadDocument,
		});
	};

	const setMultiplePropertyValues = async (targetId, updates, options = {}) => {
		for (const update of Array.isArray(updates) ? updates : []) {
			if (!update?.path) continue;
			const result = editorSession.setProperty(targetId, update.path, JSON.stringify(update.value));
			if (result && result.ok === false) {
				throw new Error(result.error || `Property update failed for ${update.path}`);
			}
		}
		await onDocumentMutated?.({
			targetId,
			paths: Array.isArray(updates) ? updates.map((update) => update?.path).filter(Boolean) : [],
			refresh: options.refresh,
			reloadDocument: options.reloadDocument,
		});
	};

	const openColorPanel = async ({ meta, path, targetId }) => {
		const current = stringifySelection(meta?.value);
		const next = globalThis.prompt(`Set ${meta?.name || meta?.key || "color"}`, current);
		if (next == null) return;
		const trimmed = String(next).trim();
		if (!trimmed) return;
		try {
			const parsed = trimmed.startsWith("{") || trimmed.startsWith("[") ? JSON.parse(trimmed) : trimmed;
			await setPropertyValue(targetId, path, parsed);
		} catch (error) {
			alert(`Invalid color value: ${error.message || error}`);
		}
	};

	const renderNode = (container, meta, path, targetId) => {
		if (!meta || typeof meta !== "object") return;
		if ((meta.widget || "") === "box" && Array.isArray(meta.properties)) {
			renderBoxWidget({
				container,
				meta,
				path,
				targetId,
				state,
				setMultiplePropertyValues,
				refreshInspector: render,
			});
			return;
		}

		if ((meta.widget || "") === "array") {
			renderArrayWidget({
				container,
				meta,
				path,
				targetId,
				setPropertyValue,
			});
			return;
		}

		if (Array.isArray(meta.properties) && meta.properties.length) {
			const section = document.createElement("section");
			section.className = "propertySection";

			const title = document.createElement("div");
			title.className = "propertySectionTitle";
			title.textContent = meta.name || meta.key || path || "Group";
			section.appendChild(title);

			const fields = document.createElement("div");
			fields.className = "propertyFields";

			if (meta.types?.all?.length) {
				renderTypeSelectorWidget({
					container: fields,
					meta,
					path,
					targetId,
					setPolymorphicType: async (_, typePath, typeId) => {
						await setPropertyValue(targetId, typePath, { type: typeId });
						render();
					},
				});
			}

			const { leaves, groups } = splitProperties(meta.properties);
			for (const leaf of leaves) {
				renderNode(fields, leaf, joinPath(path, leaf.key || ""), targetId);
			}
			for (const group of groups) {
				renderNode(section, group, joinPath(path, group.key || ""), targetId);
			}

			section.appendChild(fields);
			container.appendChild(section);
			return;
		}

		renderLeafWidget({
			container,
			meta,
			path,
			targetId,
			setPropertyValue,
			openColorPanel,
			state,
		});
	};

	const createEmptyState = (title, helper) => {
		const empty = document.createElement("div");
		empty.id = "propertiesEmpty";

		const emptyTitle = document.createElement("div");
		emptyTitle.className = "propertiesEmptyTitle";
		emptyTitle.textContent = title;

		const emptyText = document.createElement("div");
		emptyText.className = "propertiesEmptyText";
		emptyText.textContent = helper;

		empty.appendChild(emptyTitle);
		empty.appendChild(emptyText);
		return empty;
	};

	function render() {
		const selectedId = Number(getSelectedId()) || 0;
		containerEl.innerHTML = "";

		if (!selectedId) {
			containerEl.appendChild(
				createEmptyState(
					"No selection",
					"Select a face or element to edit its properties."
				)
			);
			return;
		}

		let payload;
		try {
			payload = editorSession.getPropertiesMeta(selectedId, "");
		} catch (error) {
			onStatus?.(`Failed to load properties: ${error.message || error}`);
			containerEl.appendChild(
				createEmptyState(
					"Properties unavailable",
					"Could not load properties for the selected item."
				)
			);
			return;
		}

		const metaList = Array.isArray(payload?.meta) ? payload.meta : [];
		if (!metaList.length) {
			containerEl.appendChild(
				createEmptyState(
					"No editable properties",
					"This selection does not expose editable properties."
				)
			);
			return;
		}

		for (const meta of metaList) {
			renderNode(containerEl, meta, meta.key || "", selectedId);
		}
	}

	return {
		render,
		refreshInspector: render,
	};
}
