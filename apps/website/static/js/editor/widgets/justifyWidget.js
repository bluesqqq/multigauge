import { openDropdown } from "../../dropdown.js";
import { createFieldShell, getEnumOptions } from "./shared.js";

const JUSTIFY_PREVIEW_POSITIONS = {
	"flex-start": [16, 36, 56],
	center: [30, 50, 70],
	"flex-end": [44, 64, 84],
	"space-between": [16, 50, 84],
	"space-around": [20, 50, 80],
	"space-evenly": [24, 50, 76],
};

function getPreviewPositions(value) {
	return JUSTIFY_PREVIEW_POSITIONS[String(value)] || JUSTIFY_PREVIEW_POSITIONS["flex-start"];
}

export function renderJustifyWidget({ container, meta, path, targetId, setPropertyValue }) {
	const options = getEnumOptions(meta);
	if (!options.length) return false;

	const field = createFieldShell(meta, false);
	const widget = document.createElement("div");
	widget.className = "justifyWidget";

	const preview = document.createElement("div");
	preview.className = "justifyPreview";

	const previewRail = document.createElement("div");
	previewRail.className = "justifyPreviewRail";
	preview.appendChild(previewRail);

	const previewBoxes = [0, 1, 2].map((index) => {
		const box = document.createElement("span");
		box.className = "justifyPreviewBox";
		box.style.transitionDelay = `${index * 24}ms`;
		preview.appendChild(box);
		return box;
	});

	const controls = document.createElement("div");
	controls.className = "justifyControls";

	const leftButton = document.createElement("button");
	leftButton.type = "button";
	leftButton.className = "justifyStepButton";
	leftButton.textContent = "<";
	leftButton.setAttribute("aria-label", "Previous justify option");

	const nameButton = document.createElement("button");
	nameButton.type = "button";
	nameButton.className = "justifyNameButton";

	const rightButton = document.createElement("button");
	rightButton.type = "button";
	rightButton.className = "justifyStepButton";
	rightButton.textContent = ">";
	rightButton.setAttribute("aria-label", "Next justify option");

	controls.appendChild(leftButton);
	controls.appendChild(nameButton);
	controls.appendChild(rightButton);

	widget.appendChild(preview);
	widget.appendChild(controls);
	field.appendChild(widget);
	container.appendChild(field);

	let currentValue = meta.value;

	const getSelectedIndex = () => {
		const currentIndex = options.findIndex((option) => String(option.value) === String(currentValue));
		return currentIndex >= 0 ? currentIndex : 0;
	};

	const applyPreview = (option) => {
		const positions = getPreviewPositions(option?.value);
		nameButton.textContent = option?.name || "Select";
		nameButton.title = option?.name || "Select";
		preview.dataset.value = String(option?.value || "");

		previewBoxes.forEach((box, index) => {
			box.style.left = `${positions[index] ?? 50}%`;
		});
	};

	const commitIndex = (index) => {
		if (index < 0 || index >= options.length) return;
		const option = options[index];
		currentValue = option.value;
		applyPreview(option);
		setPropertyValue(targetId, path, option.value, { refresh: false });
	};

	const cycle = (direction) => {
		const currentIndex = getSelectedIndex();
		const nextIndex = (currentIndex + direction + options.length) % options.length;
		commitIndex(nextIndex);
	};

	leftButton.addEventListener("click", () => cycle(-1));
	rightButton.addEventListener("click", () => cycle(1));
	nameButton.addEventListener("click", () => {
		const rect = nameButton.getBoundingClientRect();
		openDropdown(
			options.map((option) => ({
				name: option.name,
				value: option.value,
				selected: String(option.value) === String(currentValue),
			})),
			rect.left + window.scrollX,
			rect.bottom + window.scrollY,
			(option) => {
				const nextIndex = options.findIndex((entry) => String(entry.value) === String(option.value));
				commitIndex(nextIndex);
			},
			true,
			false,
			{
				anchorEl: nameButton,
			}
		);
	});

	applyPreview(options[getSelectedIndex()]);
	return true;
}
