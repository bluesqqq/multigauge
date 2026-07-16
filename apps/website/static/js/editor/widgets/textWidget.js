import { createFieldShell } from "./shared.js";

export function renderTextWidget({ container, meta, path, targetId, setPropertyValue }) {
	const field = createFieldShell(meta, false);
	const input = document.createElement("input");
	input.type = "text";
	input.value = meta.value == null ? "" : String(meta.value);
	if (meta?.placeholder != null) input.placeholder = String(meta.placeholder);
	if (meta?.description) input.title = meta.description;
	const commit = () => setPropertyValue(targetId, path, input.value);
	input.addEventListener("keydown", (event) => {
		if (event.key === "Enter") commit();
	});
	input.addEventListener("blur", commit);
	field.appendChild(input);
	container.appendChild(field);
}
