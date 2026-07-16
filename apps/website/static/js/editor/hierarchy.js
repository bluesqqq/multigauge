function normalizeNodeList(payload) {
	if (!payload || typeof payload !== "object") {
		return [];
	}

	if (Array.isArray(payload.roots)) {
		return payload.roots.map((value) => Number(value) || 0).filter(Boolean);
	}

	if (Array.isArray(payload.faces)) {
		return payload.faces.map((value) => Number(value) || 0).filter(Boolean);
	}

	return [];
}

function normalizeNodes(payload) {
	const nodes = new Map();

	if (!payload || typeof payload !== "object") {
		return nodes;
	}

	const source = payload.nodes;
	if (Array.isArray(source)) {
		for (const entry of source) {
			if (!entry || typeof entry !== "object") continue;
			const id = Number(entry.id) || 0;
			if (!id) continue;
			nodes.set(id, {
				...entry,
				id,
				children: Array.isArray(entry.children) ? entry.children.map((value) => Number(value) || 0).filter(Boolean) : [],
			});
		}
		return nodes;
	}

	if (source && typeof source === "object") {
		for (const [key, entry] of Object.entries(source)) {
			const id = Number(key) || Number(entry?.id) || 0;
			if (!id) continue;
			nodes.set(id, {
				...entry,
				id,
				children: Array.isArray(entry?.children) ? entry.children.map((value) => Number(value) || 0).filter(Boolean) : [],
			});
		}
	}

	return nodes;
}

export function buildHierarchyModel(payload) {
	let roots = normalizeNodeList(payload);
	const nodes = normalizeNodes(payload);
	const parentById = new Map();
	const depthById = new Map();
	const rootById = new Map();

	if (!roots.length && nodes.size) {
		for (const [id, node] of nodes.entries()) {
			const parentId = Number(node.parentId || node.parent_id || 0) || 0;
			if (!parentId) {
				roots.push(id);
			}
		}
		roots.sort((a, b) => a - b);
	}

	const visit = (id, depth, rootId, parentId) => {
		const node = nodes.get(id) || { id, children: [] };
		parentById.set(id, parentId || 0);
		depthById.set(id, depth);
		rootById.set(id, rootId || id);

		for (const childId of node.children || []) {
			visit(childId, depth + 1, rootId || id, id);
		}
	};

	for (const rootId of roots) {
		visit(rootId, 0, rootId, 0);
	}

	return {
		roots,
		nodes,
		parentById,
		depthById,
		rootById,
	};
}

function labelForNode(node, id) {
	if (!node) {
		return `#${id}`;
	}

	if (node.name) return String(node.name);
	if (node.type) return String(node.type);
	return `#${id}`;
}

function isFaceNode(node, rootId, id) {
	return node?.kind === "face" || Number(rootId) === Number(id);
}

function getChildList(model, id) {
	const node = model.nodes.get(id);
	return Array.isArray(node?.children) ? node.children : [];
}

function getSelectedClass(selectedId, id) {
	return Number(selectedId) === Number(id) ? " selected" : "";
}

export function createHierarchyView({
	containerEl,
	getSelectedId,
	getPackageName,
	onPackageClick,
	onPackageContextMenu,
	onSelect,
	onContextMenu,
}) {
	const renderNode = (model, id) => {
		const node = model.nodes.get(id) || { id, children: [] };
		const depth = model.depthById.get(id) || 0;
		const parentId = model.parentById.get(id) || 0;
		const rootId = model.rootById.get(id) || id;
		const children = getChildList(model, id);
		const li = document.createElement("div");
		li.className = "treeItem";
		li.dataset.nodeId = String(id);
		li.dataset.parentId = String(parentId);
		li.dataset.rootId = String(rootId);
		li.dataset.depth = String(depth);

		const row = document.createElement("div");
		row.className = "treeRow";

		const button = document.createElement("button");
		button.type = "button";
		button.className = "treeButton" + getSelectedClass(getSelectedId(), id);
		button.style.setProperty("--tree-depth", String(depth));

		const chevron = document.createElement("span");
		chevron.className = "treeChevron";
		chevron.textContent = children.length ? "▾" : "";

		const label = document.createElement("span");
		label.className = "treeLabel";
		label.textContent = labelForNode(node, id);

		button.appendChild(chevron);
		button.appendChild(label);

		button.addEventListener("click", () => {
			onSelect?.(id);
		});

		button.addEventListener("contextmenu", (event) => {
			event.preventDefault();
			event.stopPropagation();
			onContextMenu?.({
				x: event.clientX,
				y: event.clientY,
				nodeId: id,
				parentId,
				rootId,
				node,
				isFace: isFaceNode(node, rootId, id),
			});
		});

		row.appendChild(button);
		li.appendChild(row);

		if (children.length) {
			const wrap = document.createElement("div");
			wrap.className = "treeChildren";
			for (const childId of children) {
				wrap.appendChild(renderNode(model, childId));
			}
			li.appendChild(wrap);
		}

		return li;
	};

	function render(payload) {
		const model = buildHierarchyModel(payload);
		containerEl.innerHTML = "";

		const packageWrap = document.createElement("div");
		packageWrap.className = "packageRoot";

		const packageRow = document.createElement("div");
		packageRow.className = "treeRow packageRow";

		const packageButton = document.createElement("button");
		packageButton.type = "button";
		packageButton.className = "treeButton packageButton";
		packageButton.style.setProperty("--tree-depth", "0");
		packageButton.title = "Package";

		const packageChevron = document.createElement("span");
		packageChevron.className = "treeChevron";
		packageChevron.textContent = "▾";

		const packageLabel = document.createElement("span");
		packageLabel.className = "treeLabel";
		packageLabel.textContent = getPackageName?.() || "Package";

		packageButton.appendChild(packageChevron);
		packageButton.appendChild(packageLabel);

		packageButton.addEventListener("click", () => {
			onPackageClick?.();
		});

		packageButton.addEventListener("contextmenu", (event) => {
			event.preventDefault();
			event.stopPropagation();
			onPackageContextMenu?.({
				x: event.clientX,
				y: event.clientY,
				packageName: packageButton.textContent,
				model,
			});
		});

		packageRow.appendChild(packageButton);
		packageWrap.appendChild(packageRow);

		const childrenWrap = document.createElement("div");
		childrenWrap.className = "treeChildren packageChildren";

		for (const rootId of model.roots) {
			childrenWrap.appendChild(renderNode(model, rootId));
		}

		if (!model.roots.length) {
			const empty = document.createElement("div");
			empty.className = "treeEmpty";
			empty.textContent = "No faces yet.";
			childrenWrap.appendChild(empty);
		}

		packageWrap.appendChild(childrenWrap);
		containerEl.appendChild(packageWrap);

		return model;
	}

	return {
		render,
	};
}
