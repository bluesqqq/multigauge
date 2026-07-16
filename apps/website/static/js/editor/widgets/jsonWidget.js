import { createFieldShell, stringifyStable } from "./shared.js";

export function renderJsonWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, true);
	const textarea = document.createElement("textarea");
	let baseline = stringifyStable(meta.value);
	textarea.value = baseline;
	textarea.spellcheck = false;

	const actions = document.createElement("div");
	actions.className = "actions";

	const applyBtn = document.createElement("button");
	applyBtn.className = "smallBtn";
	applyBtn.textContent = "Apply";
	applyBtn.addEventListener("click", () => {
		try {
			const nextValue = JSON.parse(textarea.value);
			baseline = stringifyStable(nextValue);
			setPropertyValue(targetId, path, nextValue);
		} catch (error) {
			alert(`Invalid JSON for "${path}": ${error.message || error}`);
		}
	});

	const resetBtn = document.createElement("button");
	resetBtn.className = "smallBtn";
	resetBtn.textContent = "Reset";
	resetBtn.addEventListener("click", () => {
		textarea.value = baseline;
	});

	actions.appendChild(applyBtn);
	actions.appendChild(resetBtn);
	field.appendChild(textarea);
	field.appendChild(actions);
	container.appendChild(field);
}
