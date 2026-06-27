import {
	colorToHexValue,
	createFieldShell,
	joinPath,
	parseColorText,
} from "./shared.js";

function clampStopPosition(value, fallback = 0) {
	const numeric = Number(value);
	if (!Number.isFinite(numeric)) return fallback;
	return numeric;
}

function findKeyframesMeta(meta) {
	if (!meta || !Array.isArray(meta.properties)) return null;
	return meta.properties.find((entry) => entry?.key === "keyframes") || null;
}

function getStopHex(stop) {
	const colorValue = stop?.color;
	if (typeof colorValue === "string") {
		const parsed = parseColorText(colorValue);
		if (parsed) return colorToHexValue(parsed);
	}

	if (colorValue && typeof colorValue === "object") {
		if (typeof colorValue.color === "string") {
			const parsed = parseColorText(colorValue.color);
			if (parsed) return colorToHexValue(parsed);
		}
	}

	return "#808080FF";
}

function isStaticStopColor(stop) {
	const colorValue = stop?.color;
	if (!colorValue || typeof colorValue !== "object") return typeof colorValue === "string";
	return !colorValue.type || colorValue.type === "static";
}

function normalizeStop(stop, index) {
	return {
		pos: clampStopPosition(stop?.pos ?? stop?.position, index),
		color: stop?.color && typeof stop.color === "object"
			? { ...stop.color }
			: (typeof stop?.color === "string" ? { type: "static", color: stop.color } : { type: "static", color: "#FFFFFFFF" }),
	};
}

function sortStops(stops) {
	return [...stops]
		.map((stop, index) => ({ stop, index }))
		.sort((a, b) => {
			const diff = clampStopPosition(a.stop?.pos, a.index) - clampStopPosition(b.stop?.pos, b.index);
			if (diff !== 0) return diff;
			return a.index - b.index;
		})
		.map(({ stop }, index) => normalizeStop(stop, index));
}

function getStops(meta) {
	const keyframesMeta = findKeyframesMeta(meta);
	const rawStops = Array.isArray(keyframesMeta?.value) ? keyframesMeta.value : [];
	return sortStops(rawStops.map(normalizeStop));
}

function buildPreviewGradient(stops) {
	if (!stops.length) {
		return "linear-gradient(90deg, rgba(255,255,255,0.08), rgba(255,255,255,0.03))";
	}

	if (stops.length === 1) {
		const hex = getStopHex(stops[0]);
		return `linear-gradient(90deg, ${hex}, ${hex})`;
	}

	const sorted = [...stops].sort((a, b) => a.pos - b.pos);
	const min = sorted[0].pos;
	const max = sorted[sorted.length - 1].pos;
	const span = max - min;

	const segments = sorted.map((stop, index) => {
		const ratio = span === 0 ? (sorted.length === 1 ? 0 : index / (sorted.length - 1)) : (stop.pos - min) / span;
		const percent = Math.max(0, Math.min(100, ratio * 100));
		return `${getStopHex(stop)} ${percent.toFixed(2)}%`;
	});

	return `linear-gradient(90deg, ${segments.join(", ")})`;
}

function createDefaultStop(stops) {
	if (!stops.length) {
		return {
			pos: 0,
			color: {
				type: "static",
				color: "#FFFFFFFF",
			},
		};
	}

	const last = stops[stops.length - 1];
	const nextPos = clampStopPosition(last?.pos, 0) + 1;
	return {
		pos: nextPos,
		color: {
			...(last?.color && typeof last.color === "object" ? last.color : { type: "static", color: getStopHex(last) }),
		},
	};
}

function createStaticColorPayload(hex) {
	return {
		type: "static",
		color: hex,
	};
}

export function renderGradientWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, true);
	const widget = document.createElement("div");
	widget.className = "gradientWidget";

	const preview = document.createElement("div");
	preview.className = "gradientWidgetPreview";

	const previewBar = document.createElement("div");
	previewBar.className = "gradientWidgetPreviewBar";

	const rows = document.createElement("div");
	rows.className = "gradientWidgetRows";

	const footer = document.createElement("div");
	footer.className = "gradientWidgetFooter";

	const addButton = document.createElement("button");
	addButton.type = "button";
	addButton.className = "gradientWidgetAdd";
	addButton.textContent = "+";
	addButton.title = `Add ${meta?.name || meta?.key || "gradient"} stop`;

	footer.appendChild(addButton);
	preview.appendChild(previewBar);
	widget.appendChild(preview);
	widget.appendChild(rows);
	widget.appendChild(footer);
	field.appendChild(widget);
	container.appendChild(field);

	let stops = getStops(meta);

	const commitStops = () => {
		stops = sortStops(stops);
		setPropertyValue(targetId, joinPath(path, "keyframes"), stops, { refresh: false });
	};

	const rerender = () => {
		previewBar.style.background = buildPreviewGradient(stops);
		rows.innerHTML = "";

		if (!stops.length) {
			const empty = document.createElement("div");
			empty.className = "gradientWidgetEmpty";
			empty.textContent = "No gradient stops";
			rows.appendChild(empty);
			return;
		}

		stops.forEach((stop, index) => {
			const row = document.createElement("div");
			row.className = "gradientWidgetRow";

			const main = document.createElement("div");
			main.className = "gradientWidgetRowMain";

			const swatchButton = document.createElement("button");
			swatchButton.type = "button";
			swatchButton.className = "gradientWidgetSwatchButton";
			swatchButton.title = isStaticStopColor(stop)
				? "Edit stop color"
				: "Editing here converts this stop to a static color";

			const swatch = document.createElement("span");
			swatch.className = "gradientWidgetSwatch";
			swatch.style.background = getStopHex(stop);
			swatchButton.appendChild(swatch);

			const positionInput = document.createElement("input");
			positionInput.type = "number";
			positionInput.step = "any";
			positionInput.className = "gradientWidgetPosition";
			positionInput.value = String(stop.pos);
			positionInput.title = "Gradient stop position";

			main.appendChild(swatchButton);
			main.appendChild(positionInput);

			const removeButton = document.createElement("button");
			removeButton.type = "button";
			removeButton.className = "gradientWidgetRemove";
			removeButton.textContent = "-";
			removeButton.title = "Remove stop";

			const popup = document.createElement("div");
			popup.className = "gradientWidgetColorPopup hidden";

			const popupRow = document.createElement("div");
			popupRow.className = "gradientWidgetColorPopupRow";

			const colorInput = document.createElement("input");
			colorInput.type = "color";
			colorInput.className = "gradientWidgetColorInput";
			colorInput.value = getStopHex(stop).slice(0, 7);

			const hexInput = document.createElement("input");
			hexInput.type = "text";
			hexInput.className = "gradientWidgetHexInput";
			hexInput.value = getStopHex(stop);
			hexInput.placeholder = "#RRGGBBAA";

			const popupHint = document.createElement("div");
			popupHint.className = "gradientWidgetColorHint";
			popupHint.textContent = isStaticStopColor(stop)
				? "Static stop color"
				: "Changing this stop converts it to a static color";

			popupRow.appendChild(colorInput);
			popupRow.appendChild(hexInput);
			popup.appendChild(popupRow);
			popup.appendChild(popupHint);

			const syncPopup = (hex) => {
				const parsed = parseColorText(hex);
				if (!parsed) return;
				const normalizedHex = colorToHexValue(parsed);
				colorInput.value = normalizedHex.slice(0, 7);
				hexInput.value = normalizedHex;
				stops[index] = {
					...stops[index],
					color: createStaticColorPayload(normalizedHex),
				};
				swatch.style.background = normalizedHex;
				previewBar.style.background = buildPreviewGradient(stops);
				popupHint.textContent = "Static stop color";
				commitStops();
			};

				swatchButton.addEventListener("click", () => {
					popup.classList.toggle("hidden");
				});

				colorInput.addEventListener("input", () => {
					const current = parseColorText(hexInput.value);
					const alpha = current ? colorToHexValue(current).slice(7, 9) : "FF";
					syncPopup(`${colorInput.value}${alpha}`);
				});

			const commitHex = () => {
				const parsed = parseColorText(hexInput.value);
				if (!parsed) {
					hexInput.value = getStopHex(stops[index]);
					return;
				}
				syncPopup(colorToHexValue(parsed));
			};

			hexInput.addEventListener("keydown", (event) => {
				if (event.key === "Enter") commitHex();
			});
			hexInput.addEventListener("blur", commitHex);

				positionInput.addEventListener("keydown", (event) => {
					if (event.key !== "Enter") return;
					stops[index] = { ...stops[index], pos: clampStopPosition(positionInput.value, stops[index].pos) };
					commitStops();
					rerender();
				});

				positionInput.addEventListener("blur", () => {
					stops[index] = { ...stops[index], pos: clampStopPosition(positionInput.value, stops[index].pos) };
					commitStops();
					rerender();
				});

			removeButton.addEventListener("click", () => {
				stops = stops.filter((_, stopIndex) => stopIndex !== index);
				commitStops();
				rerender();
			});

			row.appendChild(main);
			row.appendChild(removeButton);
			row.appendChild(popup);
			rows.appendChild(row);
		});
	};

	addButton.addEventListener("click", () => {
		stops = sortStops([...stops, createDefaultStop(stops)]);
		commitStops();
		rerender();
	});

	rerender();
}
