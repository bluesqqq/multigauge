import { appendSharedSegmentedSelect, appendSharedSelect, createFieldShell, getEnumOptions } from "./shared.js";

export function renderEnumWidget({ container, meta, path, targetId, setPropertyValue }) {
	const enumOptions = getEnumOptions(meta);
	if (!enumOptions.length) return false;

	const field = createFieldShell(meta, false);
	const selectOptions = enumOptions.map((option) => ({
		name: option.name,
		value: option.value,
		selected: String(option.value) === String(meta.value),
	}));
	const selectedOption = selectOptions.find((option) => option.selected) ?? selectOptions[0];

	if ((meta.widget || "") === "segmented-select") {
		appendSharedSegmentedSelect(field, {
			options: selectOptions,
			onChange: (next) => setPropertyValue(targetId, path, next),
		});
	} else {
		appendSharedSelect(field, {
			labelText: selectedOption?.name ?? "Select",
			options: selectOptions,
			onChange: (next) => setPropertyValue(targetId, path, next),
		});
	}
	container.appendChild(field);
	return true;
}
