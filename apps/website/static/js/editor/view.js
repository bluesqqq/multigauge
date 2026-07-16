function clampScale(value) {
	return Math.max(0.35, Math.min(3, value));
}

function rootIdForNode(model, nodeId) {
	return model.rootById.get(Number(nodeId)) || Number(nodeId) || 0;
}

function normalizeSize(value, fallback) {
	const parsed = Number(value);
	if (!Number.isFinite(parsed) || parsed <= 0) {
		return Math.max(1, fallback | 0);
	}
	return Math.max(1, Math.round(parsed));
}

export function createGaugeView({
	viewportEl,
	workspaceEl,
	editorSession,
	getHierarchyModel,
	getSelectedId,
	onSelect,
	onContextMenu,
	onStatus,
	onViewChange,
}) {
	const DEFAULT_FACE_WIDTH = 250;
	const DEFAULT_FACE_HEIGHT = 240;
	const faceStates = new Map();
	const viewState = {
		scale: 1,
		x: 0,
		y: 0,
	};
	let pan = null;

	const notifyViewChange = () => {
		onViewChange?.({
			scale: viewState.scale,
			x: viewState.x,
			y: viewState.y,
		});
	};

	const applyTransform = () => {
		workspaceEl.style.transform = `translate(${viewState.x}px, ${viewState.y}px) scale(${viewState.scale})`;
		notifyViewChange();
	};

	const resetView = () => {
		viewState.scale = 1;
		viewState.x = 0;
		viewState.y = 0;
		applyTransform();
	};

	const zoomAtPoint = (clientX, clientY, nextScale) => {
		const rect = viewportEl.getBoundingClientRect();
		const localX = clientX - rect.left;
		const localY = clientY - rect.top;
		const worldX = (localX - viewState.x) / viewState.scale;
		const worldY = (localY - viewState.y) / viewState.scale;
		viewState.scale = clampScale(nextScale);
		viewState.x = localX - (worldX * viewState.scale);
		viewState.y = localY - (worldY * viewState.scale);
		applyTransform();
	};

	const applyFaceRuntimeConfig = (faceState) => {
		if (!faceState?.runtime) return;
		const nextWidth = normalizeSize(faceState.renderWidth, DEFAULT_FACE_WIDTH);
		const nextHeight = faceState.circular
			? nextWidth
			: normalizeSize(faceState.renderHeight, DEFAULT_FACE_HEIGHT);
		const nextCircular = !!faceState.circular;
		if (
			faceState.appliedRenderWidth === nextWidth &&
			faceState.appliedRenderHeight === nextHeight &&
			faceState.appliedCircular === nextCircular
		) {
			return;
		}
		try {
			faceState.renderWidth = nextWidth;
			faceState.renderHeight = nextHeight;
			faceState.circular = nextCircular;
			faceState.runtime.setRenderMode("intrinsic", nextWidth, nextHeight);
			faceState.appliedRenderWidth = nextWidth;
			faceState.appliedRenderHeight = nextHeight;
			faceState.appliedCircular = nextCircular;
			faceState.canvas.style.borderRadius = nextCircular ? "50%" : "0";
		} catch (error) {
			onStatus?.(`Runtime error for face ${faceState.faceId}: ${error.message || error}`);
		}
	};

	const syncFaceCanvasSize = (faceState) => {
		if (!faceState?.canvas) return;
		faceState.canvas.style.width = `${faceState.displayWidth}px`;
		faceState.canvas.style.height = `${faceState.displayHeight}px`;
	};

	const ensureFaceState = (faceId, node) => {
		let faceState = faceStates.get(faceId);
		if (faceState) {
			faceState.node = node;
			return faceState;
		}

		const artboard = document.createElement("div");
		artboard.className = "faceArtboard";
		artboard.dataset.faceId = String(faceId);

		const canvas = document.createElement("canvas");
		canvas.className = "faceCanvas";
		canvas.dataset.faceId = String(faceId);

		canvas.addEventListener("click", () => {
			onSelect?.(faceId);
		});

		canvas.addEventListener("contextmenu", (event) => {
			event.preventDefault();
			event.stopPropagation();
			onContextMenu?.({
				x: event.clientX,
				y: event.clientY,
				nodeId: faceId,
				parentId: 0,
				rootId: faceId,
				node,
				isFace: true,
			});
		});

			faceState = {
				faceId,
				node,
				artboard,
				canvas,
				runtime: null,
				started: false,
				renderWidth: DEFAULT_FACE_WIDTH,
				renderHeight: DEFAULT_FACE_HEIGHT,
				circular: false,
				displayWidth: DEFAULT_FACE_WIDTH,
				displayHeight: DEFAULT_FACE_HEIGHT,
				appliedRenderWidth: 0,
				appliedRenderHeight: 0,
				appliedCircular: false,
			};

		syncFaceCanvasSize(faceState);
		faceStates.set(faceId, faceState);
		return faceState;
	};

	const destroyMissingFaces = (activeIds) => {
		for (const [faceId, faceState] of faceStates.entries()) {
			if (activeIds.has(faceId)) continue;
			try {
				faceState.runtime?.destroy?.();
			} catch (_) {
			}
			faceState.artboard.remove();
			faceStates.delete(faceId);
		}
	};

	const getFaceState = (faceId) => faceStates.get(Number(faceId)) || null;

	const setFaceRenderSize = (faceId, width, height) => {
		const faceState = getFaceState(faceId);
		if (!faceState) return false;
		faceState.renderWidth = normalizeSize(width, faceState.renderWidth);
		faceState.renderHeight = faceState.circular
			? faceState.renderWidth
			: normalizeSize(height, faceState.renderHeight);
		faceState.displayWidth = faceState.renderWidth;
		faceState.displayHeight = faceState.circular ? faceState.displayWidth : faceState.renderHeight;
		syncFaceCanvasSize(faceState);
		applyFaceRuntimeConfig(faceState);
		return true;
	};

	const setFaceCircular = (faceId, circular) => {
		const faceState = getFaceState(faceId);
		if (!faceState) return false;
		faceState.circular = !!circular;
		if (faceState.circular) {
			faceState.renderHeight = faceState.renderWidth;
			faceState.displayHeight = faceState.displayWidth;
		} else {
			faceState.displayHeight = faceState.renderHeight;
		}
		applyFaceRuntimeConfig(faceState);
		return true;
	};

	function render() {
		const model = getHierarchyModel();
		const selectedId = Number(getSelectedId()) || 0;
		const selectedRootId = rootIdForNode(model, selectedId);
		const activeIds = new Set(model.roots);

		destroyMissingFaces(activeIds);

		const fragment = document.createDocumentFragment();
		for (const rootId of model.roots) {
			const node = model.nodes.get(rootId) || { id: rootId };
			const faceState = ensureFaceState(rootId, node);
			faceState.node = node;
			faceState.canvas.classList.toggle("selected", selectedRootId === rootId);
			faceState.artboard.classList.toggle("selected", selectedRootId === rootId);
			syncFaceCanvasSize(faceState);
			faceState.artboard.innerHTML = "";
			faceState.artboard.appendChild(faceState.canvas);
			fragment.appendChild(faceState.artboard);
		}

		workspaceEl.innerHTML = "";
		workspaceEl.appendChild(fragment);

		for (const faceState of faceStates.values()) {
			if (!faceState.artboard.isConnected) {
				faceState.artboard.innerHTML = "";
				faceState.artboard.appendChild(faceState.canvas);
				workspaceEl.appendChild(faceState.artboard);
			}
			try {
				if (!faceState.runtime) {
					faceState.runtime = globalThis.Multigauge.createRuntime(faceState.canvas);
					faceState.runtime.bindEditor(editorSession.editor, faceState.faceId);
				}
				if (!faceState.started) {
					faceState.runtime.start?.();
					faceState.started = true;
				}
				applyFaceRuntimeConfig(faceState);
			} catch (error) {
				onStatus?.(`Runtime error for face ${faceState.faceId}: ${error.message || error}`);
			}
		}
	};

	const handlePointerDown = (event) => {
		if (!viewportEl.contains(event.target)) return;
		if (event.target.closest?.(".faceCanvas")) return;
		if (event.button !== 0) return;
		pan = {
			id: event.pointerId,
			startX: event.clientX,
			startY: event.clientY,
			baseX: viewState.x,
			baseY: viewState.y,
		};
		viewportEl.setPointerCapture?.(event.pointerId);
		viewportEl.style.cursor = "grabbing";
	};

	const handlePointerMove = (event) => {
		if (!pan || pan.id !== event.pointerId) return;
		viewState.x = pan.baseX + (event.clientX - pan.startX);
		viewState.y = pan.baseY + (event.clientY - pan.startY);
		applyTransform();
	};

	const stopPan = (event) => {
		if (!pan || pan.id !== event.pointerId) return;
		viewportEl.releasePointerCapture?.(event.pointerId);
		pan = null;
		viewportEl.style.cursor = "grab";
	};

	const handleWheel = (event) => {
		event.preventDefault();
		const zoomFactor = event.deltaY < 0 ? 1.08 : 0.92;
		zoomAtPoint(event.clientX, event.clientY, viewState.scale * zoomFactor);
	};

	viewportEl.addEventListener("pointerdown", handlePointerDown);
	viewportEl.addEventListener("pointermove", handlePointerMove);
	viewportEl.addEventListener("pointerup", stopPan);
	viewportEl.addEventListener("pointercancel", stopPan);
	viewportEl.addEventListener("wheel", handleWheel, { passive: false });
	workspaceEl.addEventListener("wheel", handleWheel, { passive: false });

	applyTransform();

	return {
		render,
		resetView,
		setFaceRenderSize,
		setFaceCircular,
		getFaceState,
		getFaceRenderConfig(faceId) {
			const faceState = getFaceState(faceId);
			if (!faceState) return null;
			return {
				width: faceState.renderWidth,
				height: faceState.renderHeight,
				circular: !!faceState.circular,
				displayWidth: faceState.displayWidth,
				displayHeight: faceState.displayHeight,
			};
		},
	};
}
