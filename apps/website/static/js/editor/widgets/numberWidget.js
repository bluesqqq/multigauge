import { createFieldShell } from "./shared.js";

export function renderNumberWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, false);
	const input = document.createElement("input");
	input.type = "number";
	input.step = Number.isFinite(Number(meta?.step)) ? String(meta.step) : "any";
	if (meta?.min != null) input.min = String(meta.min);
	if (meta?.max != null) input.max = String(meta.max);
	if (meta?.placeholder != null) input.placeholder = String(meta.placeholder);
	input.value = typeof meta.value === "number" ? String(meta.value) : (Number.isFinite(Number(meta?.value)) ? String(Number(meta.value)) : "");

	const commit = () => {
		if (input.value.trim() === "") {
			if (meta?.value == null) setPropertyValue(targetId, path, null);
			return;
		}
		const next = Number(input.value);
		if (!Number.isNaN(next)) setPropertyValue(targetId, path, next);
	};

	input.addEventListener("keydown", (event) => {
		if (event.key === "Enter") commit();
	});
	input.addEventListener("blur", commit);
	field.appendChild(input);
	container.appendChild(field);
}
