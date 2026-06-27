import { makeSelectMenu } from "../../dropdown.js";

export function joinPath(base, key) {
	if (!base) return key || "";
	if (!key) return base;
	return base + "." + key;
}

export function stringifyStable(value) {
	try {
		return JSON.stringify(value, null, 2);
	} catch {
		return String(value);
	}
}

function isPlainObject(value) {
	return value != null && typeof value === "object" && !Array.isArray(value);
}

export function inferLeafKind(meta) {
	if (!meta || typeof meta !== "object") return "text";
	const widget = meta.widget || "";
	if (widget === "number") return "number";
	if (widget === "bool" || widget === "boolean") return "bool";
	if (widget === "text" || widget === "string") return "text";
	if (widget === "json") return "json";
	if (widget === "color") return "color";
	if (widget === "color-selector") return "color-selector";
	if (widget === "gradient") return "gradient";
	if (widget === "enum" || widget === "select") return "text";

	const value = meta.value;
	if (typeof value === "boolean") return "bool";
	if (typeof value === "number") return "number";
	if (isPlainObject(value) || Array.isArray(value)) return "json";
	return "text";
}

export function splitProperties(metaArr) {
	const leaves = [];
	const groups = [];
	for (const meta of metaArr) {
		if ((meta?.widget || "") === "group") groups.push(meta);
		else leaves.push(meta);
	}
	return { leaves, groups };
}

export function getEnumOptions(meta) {
	if (!meta || typeof meta !== "object") return [];

	const sources = [
		meta.options,
		meta.values,
		meta.choices,
		meta.enum,
		meta.items,
	];

	for (const source of sources) {
		if (!Array.isArray(source)) continue;
		return source
			.map((entry, index) => {
				if (entry == null) return null;
				if (typeof entry === "string" || typeof entry === "number" || typeof entry === "boolean") {
					return {
						name: String(entry),
						value: entry,
						index,
					};
				}
				if (typeof entry === "object") {
					const optionValue = "value" in entry ? entry.value : ("name" in entry ? entry.name : index);
					const optionName = entry.label ?? entry.name ?? String(optionValue);
					return {
						name: String(optionName),
						value: optionValue,
						index,
					};
				}
				return null;
			})
			.filter(Boolean);
	}

	return [];
}

export function appendSharedSelect(field, { labelText, options, onChange }) {
	const select = makeSelectMenu(options, labelText, onChange);
	select.classList.add("propertySelect");
	field.appendChild(select);
}

function normalizeSegmentKey(value) {
	return String(value ?? "")
		.trim()
		.toLowerCase()
		.replace(/([a-z0-9])([A-Z])/g, "$1-$2")
		.replace(/[\s_]+/g, "-");
}

function getSegmentIconSource(option) {
	const explicitIcon = typeof option?.icon === "string" ? option.icon.trim() : "";
	if (explicitIcon) {
		if (explicitIcon.startsWith("/")) return explicitIcon;
		return `/static/images/${explicitIcon}`;
	}

	const key = normalizeSegmentKey(option?.value ?? option?.name);
	const iconMap = {
		static: "/static/images/static_color.png",
		value: "/static/images/value_color.png",
		time: "/static/images/cycle_color.png",
		user: "/static/images/user_color.png",
		variable: "/static/images/variable_color.png",
		forward: "/static/images/loop_forward.png",
		reverse: "/static/images/loop_reverse.png",
		pingpong: "/static/images/loop_pingpong.png",
		"ping-pong": "/static/images/loop_pingpong.png",
		up: "/static/images/up.png",
		down: "/static/images/down.png",
		left: "/static/images/left.png",
		right: "/static/images/right.png",
	};

	return iconMap[key] || "";
}

function shouldUseIconOnlySegments(options, explicitPreference) {
	if (explicitPreference === true) return options.every((option) => !!getSegmentIconSource(option));
	if (explicitPreference === false) return false;
	return options.length > 0 && options.length <= 5 && options.every((option) => !!getSegmentIconSource(option));
}

export function appendSharedSegmentedSelect(field, { options, onChange, iconOnly = undefined }) {
	const segmented = document.createElement("div");
	segmented.className = "segmentedSelect";

	const useIconOnly = shouldUseIconOnlySegments(options, iconOnly);
	if (useIconOnly) segmented.classList.add("segmentedSelectIconOnly");

	for (const option of options) {
		const button = document.createElement("button");
		button.type = "button";
		button.className = "segmentedSelectOption";
		button.title = option.name;
		button.setAttribute("aria-label", option.name);
		if (option.selected) {
			button.classList.add("selected");
			button.setAttribute("aria-pressed", "true");
		} else {
			button.setAttribute("aria-pressed", "false");
		}

		const iconSource = getSegmentIconSource(option);
		if (iconSource) {
			const icon = document.createElement("img");
			icon.className = "segmentedSelectIcon";
			icon.src = iconSource;
			icon.alt = "";
			button.appendChild(icon);
		}

		if (!useIconOnly || !iconSource) {
			const text = document.createElement("span");
			text.className = "segmentedSelectText";
			text.textContent = option.name;
			button.appendChild(text);
		}

		button.addEventListener("click", () => {
			if (typeof onChange === "function") onChange(option.value);
		});

		segmented.appendChild(button);
	}

	field.appendChild(segmented);
}

export function createFieldShell(meta, wide = false) {
	const field = document.createElement("label");
	field.className = "propertyField" + (wide ? " propertyFieldWide" : "");

	const label = document.createElement("div");
	label.className = "propertyLabel";
	label.textContent = meta.name || meta.key || "Property";
	if (meta.description) {
		label.title = meta.description;
	}

	field.appendChild(label);
	return field;
}

export function clampByte(value, fallback = 255) {
	const numeric = Number(value);
	if (!Number.isFinite(numeric)) return fallback;
	return Math.max(0, Math.min(255, Math.round(numeric)));
}

function toHexByte(value) {
	return clampByte(value).toString(16).padStart(2, "0").toUpperCase();
}

export function parseColorText(value) {
	if (typeof value !== "string") return null;

	let text = value.trim();
	if (!text) return null;
	if (text.startsWith("#")) text = text.slice(1);
	else if (text.startsWith("0x") || text.startsWith("0X")) text = text.slice(2);

	if (![3, 4, 6, 8].includes(text.length)) return null;
	if (!/^[0-9a-fA-F]+$/.test(text)) return null;

	if (text.length === 3 || text.length === 4) {
		text = text.split("").map((char) => char + char).join("");
	}
	if (text.length === 6) {
		text += "FF";
	}

	return {
		r: parseInt(text.slice(0, 2), 16),
		g: parseInt(text.slice(2, 4), 16),
		b: parseInt(text.slice(4, 6), 16),
		a: parseInt(text.slice(6, 8), 16),
	};
}

export function colorToPickerValue(color) {
	if (!color) return "#000000";
	return `#${toHexByte(color.r)}${toHexByte(color.g)}${toHexByte(color.b)}`;
}

export function colorToHexValue(color) {
	if (!color) return "#000000FF";
	return `#${toHexByte(color.r)}${toHexByte(color.g)}${toHexByte(color.b)}${toHexByte(color.a)}`;
}

export function parseLayoutSizeValue(value) {
	if (typeof value === "number" && Number.isFinite(value)) {
		return { unit: "px", value };
	}

	if (typeof value !== "string") {
		return { unit: "auto", value: 0 };
	}

	const text = value.trim();
	if (!text) return { unit: "auto", value: 0 };
	if (text.toLowerCase() === "auto") return { unit: "auto", value: 0 };

	const trailingMatch = text.match(/^(-?(?:\d+(?:\.\d+)?|\.\d+))(%)$/);
	if (trailingMatch) {
		return { unit: "%", value: Number(trailingMatch[1]) || 0 };
	}

	const prefixedPercentMatch = text.match(/^%(-?(?:\d+(?:\.\d+)?|\.\d+))$/);
	if (prefixedPercentMatch) {
		return { unit: "%", value: Number(prefixedPercentMatch[1]) || 0 };
	}

	const pxMatch = text.match(/^(-?(?:\d+(?:\.\d+)?|\.\d+))px$/i);
	if (pxMatch) {
		return { unit: "px", value: Number(pxMatch[1]) || 0 };
	}

	const numeric = Number(text);
	if (Number.isFinite(numeric)) {
		return { unit: "px", value: numeric };
	}

	return { unit: "auto", value: 0 };
}

export function serializeLayoutSizeValue(state) {
	if (!state || state.unit === "auto") return "auto";
	const numeric = Number(state.value);
	const safeValue = Number.isFinite(numeric) ? numeric : 0;
	return state.unit === "%" ? `${safeValue}%` : safeValue;
}
