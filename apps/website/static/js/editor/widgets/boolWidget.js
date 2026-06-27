import { appendSharedSelect, createFieldShell } from "./shared.js";

export function renderBoolWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, false);
	appendSharedSelect(field, {
		labelText: meta.value ? "true" : "false",
		options: [
			{ name: "true", value: true, selected: meta.value === true },
			{ name: "false", value: false, selected: meta.value === false },
		],
		onChange: (next) => setPropertyValue(targetId, path, next === true),
	});
	container.appendChild(field);
}
