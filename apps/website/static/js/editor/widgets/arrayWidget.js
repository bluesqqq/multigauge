import { createFieldShell, stringifyStable } from "./shared.js";

export function renderArrayWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, true);
	const textarea = document.createElement("textarea");
	let baseline = stringifyStable(meta.items ?? meta.value ?? []);
	textarea.value = baseline;
	textarea.spellcheck = false;

	const actions = document.createElement("div");
	actions.className = "actions";

	const applyBtn = document.createElement("button");
	applyBtn.className = "smallBtn";
	applyBtn.textContent = "Apply Array";
	applyBtn.addEventListener("click", () => {
		try {
			const nextValue = JSON.parse(textarea.value);
			baseline = stringifyStable(nextValue);
			setPropertyValue(targetId, path, nextValue);
		} catch (error) {
			alert(`Invalid JSON for "${path}": ${error.message || error}`);
		}
	});

	actions.appendChild(applyBtn);
	field.appendChild(textarea);
	field.appendChild(actions);
	container.appendChild(field);
}
