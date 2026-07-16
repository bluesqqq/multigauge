import { colorToHexValue, createFieldShell, parseColorText } from "./shared.js";

export function renderColorWidget({ container, meta, path, targetId, openColorPanel }) {
	const field = createFieldShell(meta, true);
	const colorValue = typeof meta?.value === "string"
		? meta.value
		: (typeof meta?.value?.color === "string" ? meta.value.color : "");
	const parsedColor = parseColorText(colorValue);

	const button = document.createElement("button");
	button.type = "button";
	button.className = "colorRowButton";
	button.title = meta.description
		? `${meta.description}${colorValue ? ` Current value: ${colorValue}` : ""}`
		: `Open ${meta.name || meta.key || "color"} inspector${colorValue ? ` (${colorValue})` : ""}`;

	const swatch = document.createElement("span");
	swatch.className = "colorRowSwatch";
	swatch.style.background = parsedColor ? colorToHexValue(parsedColor) : "#FFFFFF";

	const textWrap = document.createElement("span");
	textWrap.className = "colorRowText";

	const name = document.createElement("span");
	name.className = "colorRowName";
	name.textContent = meta.name || meta.key || "Color";
	if (colorValue) {
		name.title = colorValue;
	}

	textWrap.appendChild(name);
	button.appendChild(swatch);
	button.appendChild(textWrap);

	button.addEventListener("click", () => {
		if (typeof openColorPanel === "function") {
			openColorPanel({ meta, path, targetId, anchor: button });
		}
	});

	field.appendChild(button);
	container.appendChild(field);
}
