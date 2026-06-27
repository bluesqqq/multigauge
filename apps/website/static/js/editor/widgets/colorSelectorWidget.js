import {
	clampByte,
	colorToHexValue,
	createFieldShell,
	parseColorText,
} from "./shared.js";

function rgbToHsv({ r, g, b }) {
	const red = r / 255;
	const green = g / 255;
	const blue = b / 255;
	const max = Math.max(red, green, blue);
	const min = Math.min(red, green, blue);
	const delta = max - min;

	let h = 0;
	if (delta > 0) {
		if (max === red) h = ((green - blue) / delta) % 6;
		else if (max === green) h = (blue - red) / delta + 2;
		else h = (red - green) / delta + 4;
		h *= 60;
		if (h < 0) h += 360;
	}

	const s = max === 0 ? 0 : delta / max;
	const v = max;
	return { h, s, v };
}

function hsvToRgb({ h, s, v, a = 255 }) {
	const hue = ((Number(h) % 360) + 360) % 360;
	const saturation = Math.max(0, Math.min(1, Number(s)));
	const value = Math.max(0, Math.min(1, Number(v)));
	const chroma = value * saturation;
	const x = chroma * (1 - Math.abs(((hue / 60) % 2) - 1));
	const m = value - chroma;

	let red = 0;
	let green = 0;
	let blue = 0;

	if (hue < 60) [red, green, blue] = [chroma, x, 0];
	else if (hue < 120) [red, green, blue] = [x, chroma, 0];
	else if (hue < 180) [red, green, blue] = [0, chroma, x];
	else if (hue < 240) [red, green, blue] = [0, x, chroma];
	else if (hue < 300) [red, green, blue] = [x, 0, chroma];
	else [red, green, blue] = [chroma, 0, x];

	return {
		r: Math.round((red + m) * 255),
		g: Math.round((green + m) * 255),
		b: Math.round((blue + m) * 255),
		a: clampByte(a),
	};
}

function getColorFromCanvas(canvas, event, hue, alpha) {
	const rect = canvas.getBoundingClientRect();
	const x = Math.max(0, Math.min(rect.width, event.clientX - rect.left));
	const y = Math.max(0, Math.min(rect.height, event.clientY - rect.top));
	const s = rect.width <= 0 ? 0 : x / rect.width;
	const v = rect.height <= 0 ? 0 : 1 - y / rect.height;
	return {
		color: hsvToRgb({ h: hue, s, v, a: alpha }),
		xRatio: s,
		yRatio: 1 - v,
	};
}

function setCanvasGradient(canvas, hue) {
	const context = canvas.getContext("2d");
	if (!context) return;

	const { width, height } = canvas;
	context.clearRect(0, 0, width, height);

	const hueColor = hsvToRgb({ h: hue, s: 1, v: 1 });
	const saturationGradient = context.createLinearGradient(0, 0, width, 0);
	saturationGradient.addColorStop(0, "rgba(255,255,255,1)");
	saturationGradient.addColorStop(1, `rgba(${hueColor.r}, ${hueColor.g}, ${hueColor.b}, 1)`);
	context.fillStyle = saturationGradient;
	context.fillRect(0, 0, width, height);

	const valueGradient = context.createLinearGradient(0, 0, 0, height);
	valueGradient.addColorStop(0, "rgba(0,0,0,0)");
	valueGradient.addColorStop(1, "rgba(0,0,0,1)");
	context.fillStyle = valueGradient;
	context.fillRect(0, 0, width, height);
}

export function renderColorSelectorWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, true);
	const parsed = parseColorText(typeof meta.value === "string" ? meta.value : "") || {
		r: 255,
		g: 255,
		b: 255,
		a: 255,
	};

	let state = {
		...parsed,
		...rgbToHsv(parsed),
	};

	const wrap = document.createElement("div");
	wrap.className = "colorSelectorWidget";

	const canvasWrap = document.createElement("div");
	canvasWrap.className = "colorSelectorCanvasWrap";

	const canvas = document.createElement("canvas");
	canvas.className = "colorSelectorCanvas";
	canvas.width = 240;
	canvas.height = 160;

	const canvasThumb = document.createElement("div");
	canvasThumb.className = "colorSelectorCanvasThumb";

	canvasWrap.appendChild(canvas);
	canvasWrap.appendChild(canvasThumb);

	const hueRow = document.createElement("div");
	hueRow.className = "colorSelectorHueRow";

	const hueSlider = document.createElement("input");
	hueSlider.type = "range";
	hueSlider.min = "0";
	hueSlider.max = "360";
	hueSlider.step = "1";
	hueSlider.className = "colorSelectorHueSlider";

	hueRow.appendChild(hueSlider);

	const inputs = document.createElement("div");
	inputs.className = "colorSelectorInputs";

	const makeChannelInput = (labelText, key, options = {}) => {
		const group = document.createElement("label");
		group.className = "colorSelectorInputGroup";
		const label = document.createElement("span");
		label.className = "colorSelectorInputLabel";
		label.textContent = labelText;
		const input = document.createElement("input");
		input.type = "text";
		input.className = "colorSelectorInput";
		if (options.placeholder) input.placeholder = options.placeholder;
		group.appendChild(label);
		group.appendChild(input);
		inputs.appendChild(group);
		return { key, input };
	};

	const channelInputs = [
		makeChannelInput("R", "r"),
		makeChannelInput("G", "g"),
		makeChannelInput("B", "b"),
		makeChannelInput("HEX", "hex", { placeholder: "#RRGGBBAA" }),
	];

	const getCurrentColor = () => ({
		r: clampByte(state.r),
		g: clampByte(state.g),
		b: clampByte(state.b),
		a: clampByte(state.a),
	});

	const syncVisuals = () => {
		const color = getCurrentColor();
		const hsv = rgbToHsv(color);
		state = { ...state, ...color, ...hsv };
		hueSlider.value = String(Math.round(state.h));
		setCanvasGradient(canvas, state.h);
		canvasThumb.style.left = `${state.s * 100}%`;
		canvasThumb.style.top = `${(1 - state.v) * 100}%`;
		for (const channel of channelInputs) {
			if (channel.key === "hex") channel.input.value = colorToHexValue(color);
			else channel.input.value = String(color[channel.key]);
		}
	};

	const commit = () => {
		setPropertyValue(targetId, path, colorToHexValue(getCurrentColor()));
	};

	const applyRgb = (nextColor, { shouldCommit = false } = {}) => {
		state = { ...state, ...nextColor };
		syncVisuals();
		if (shouldCommit) commit();
	};

	const applyHsv = ({ h = state.h, s = state.s, v = state.v }, { shouldCommit = false } = {}) => {
		const nextColor = hsvToRgb({ h, s, v, a: state.a });
		state = { ...state, ...nextColor, h, s, v };
		syncVisuals();
		if (shouldCommit) commit();
	};

	let draggingCanvas = false;

	const handleCanvasPointer = (event, shouldCommit) => {
		const sample = getColorFromCanvas(canvas, event, state.h, state.a);
		applyRgb(sample.color, { shouldCommit });
	};

	canvasWrap.addEventListener("pointerdown", (event) => {
		draggingCanvas = true;
		canvasWrap.setPointerCapture?.(event.pointerId);
		handleCanvasPointer(event, false);
	});

	canvasWrap.addEventListener("pointermove", (event) => {
		if (!draggingCanvas) return;
		handleCanvasPointer(event, false);
	});

	const stopCanvasDrag = (event) => {
		if (!draggingCanvas) return;
		draggingCanvas = false;
		canvasWrap.releasePointerCapture?.(event.pointerId);
		handleCanvasPointer(event, true);
	};

	canvasWrap.addEventListener("pointerup", stopCanvasDrag);
	canvasWrap.addEventListener("pointercancel", stopCanvasDrag);

	hueSlider.addEventListener("input", () => {
		applyHsv({ h: Number(hueSlider.value) }, { shouldCommit: false });
	});

	hueSlider.addEventListener("change", () => {
		applyHsv({ h: Number(hueSlider.value) }, { shouldCommit: true });
	});

	for (const channel of channelInputs) {
		if (channel.key === "hex") {
			const commitHex = () => {
				const parsedColor = parseColorText(channel.input.value);
				if (!parsedColor) {
					syncVisuals();
					return;
				}
				applyRgb(parsedColor, { shouldCommit: true });
			};
			channel.input.addEventListener("keydown", (event) => {
				if (event.key === "Enter") commitHex();
			});
			channel.input.addEventListener("blur", commitHex);
			continue;
		}

		const commitChannel = () => {
			applyRgb({ [channel.key]: clampByte(channel.input.value, state[channel.key]) }, { shouldCommit: true });
		};

		channel.input.addEventListener("input", () => {
			applyRgb({ [channel.key]: clampByte(channel.input.value, state[channel.key]) }, { shouldCommit: false });
		});
		channel.input.addEventListener("blur", commitChannel);
		channel.input.addEventListener("keydown", (event) => {
			if (event.key === "Enter") commitChannel();
		});
	}

	wrap.appendChild(canvasWrap);
	wrap.appendChild(hueRow);
	wrap.appendChild(inputs);
	field.appendChild(wrap);
	container.appendChild(field);

	syncVisuals();
}
