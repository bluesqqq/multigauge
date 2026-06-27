import { appendSharedSelect, createFieldShell } from "./shared.js";

export function renderValueWidget({ container, meta, path, targetId, setPropertyValue, state }) {
	const valueIds = Array.isArray(state?.valueIds) ? state.valueIds : [];
	if (!valueIds.length) return false;

	const field = createFieldShell(meta, false);
	const currentValue = meta.value == null ? "" : String(meta.value);
	const selectOptions = valueIds.map((valueId) => ({
		name: String(valueId),
		value: String(valueId),
		selected: String(valueId) === currentValue,
	}));
	const selectedOption = selectOptions.find((option) => option.selected) ?? selectOptions[0];

	appendSharedSelect(field, {
		labelText: selectedOption?.name ?? "Select value",
		options: selectOptions,
		onChange: (next) => setPropertyValue(targetId, path, next),
	});

	container.appendChild(field);
	return true;
}
